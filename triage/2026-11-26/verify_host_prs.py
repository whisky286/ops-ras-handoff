import argparse, importlib.util, json, os, re, subprocess, sys, tempfile
from pathlib import Path
def run(args,cwd,**kw):
    return subprocess.run(args,cwd=cwd,text=True,capture_output=True,**kw)

def must(args,cwd,**kw):
    p=run(args,cwd,**kw)
    if p.returncode:raise RuntimeError(p.stderr or p.stdout)
    return p

def load(path,name):
    spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);sys.modules[name]=m;spec.loader.exec_module(m);return m

def probe(n,repo,tmp):
    if n==219:
        module=load(repo/"scripts/check_build_dependencies.py","dependency_probe")
        cases=[("8.1","8.1.3",False),("8.1.3","8.1",False),("8.1","8.1.0",True),("8.1.0","8.1",True),("8.1.3","8.1.3",True),("8.1.3",">=8.1",True),("8.1",">8.1",False),("8.1","<8.1.3",True),("8.2","<=8.1",False)]
        result=[{"version":v,"constraint":c,"expected":e,"actual":module.check_build_dep_item(v,c)} for v,c,e in cases]
        return {"cases":result,"mismatches":sum(c["expected"]!=c["actual"] for c in result)}
    if n==220:
        cases=[]
        for value in [None,"abc","a_b","tag_20261125_013000000_hash"]:
            env=dict(os.environ);env.pop("tagInfo",None)
            if value is not None:env["tagInfo"]=value
            output=tmp/"version.info"
            p=must([sys.executable,str(repo/"scripts/generate_version_info.py"),"1.0","--output",str(output)],repo,env=env)
            ts=next(l.split("=",1)[1] for l in output.read_text().splitlines() if l.startswith("timestamp="))
            expected="20261125_013000000" if value and value.startswith("tag_") else None
            ok=ts==expected if expected else bool(re.fullmatch(r"\d{8}_\d{9}",ts))
            cases.append({"tagInfo":value,"timestamp":ts,"expected":"preserve valid tag" if expected else "nonempty timestamp format","passed":ok})
        return {"cases":cases,"mismatches":sum(not c["passed"] for c in cases)}
    source=tmp/"fixture_def.cpp"
    source.write_text('std::map<std::string,int> cfg = {{"ascend910b",1},{"ascend310p",2}};\nSetAscendConfig("x","ascend910b");\nSetAscendConfig("x","ascend310p");\nthis->AICore().AddConfig("ascend950");\nFOREACH_OPDEF(A2, Example);\n')
    command="import importlib.util,json,sys;s=importlib.util.spec_from_file_location('soc_probe',sys.argv[1]);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);print(json.dumps(m.extract_ai_core_configs(sys.argv[2])))"
    cases=[]
    for seed in range(1,13):
        env=dict(os.environ,PYTHONHASHSEED=str(seed))
        p=must([sys.executable,"-c",command,str(repo/"scripts/ci/gen_ops_soc.py"),str(source)],repo,env=env)
        value=json.loads(p.stdout);assert set(value)=={"ascend310p","ascend910b","ascend950"}
        cases.append({"hash_seed":seed,"configs":value})
    return {"cases":cases,"distinct_orders":len({tuple(c["configs"]) for c in cases}),"sorted_every_time":all(c["configs"]==sorted(c["configs"]) for c in cases)}
def main():
    parser=argparse.ArgumentParser()
    parser.add_argument("--baseline",required=True,type=Path,help="Local ops-ras repository containing the pinned master commit")
    parser.add_argument("--output",type=Path,default=Path("HOST_PR_RERUN.json"))
    args=parser.parse_args()
    root=Path(__file__).resolve().parent
    index=json.loads((root/"PATCH_INDEX.json").read_text())
    results=[]
    with tempfile.TemporaryDirectory(prefix="ops-ras-host-rerun-") as temporary:
        tmp=Path(temporary);repo=tmp/"source"
        must(["git","clone","-q","--shared",str(args.baseline.resolve()),str(repo)],tmp)
        for n in [219,220,244]:
            must(["git","reset","--hard","-q",index["master_sha"]],repo)
            must(["git","clean","-fdq"],repo)
            before=probe(n,repo,tmp)
            entry=next(p for p in index["patches"] if p["pr"]==n)
            must(["git","apply",str(root/entry["patch"])],repo)
            after=probe(n,repo,tmp)
            if n==244:
                assert before["distinct_orders"]>1 and after["distinct_orders"]==1 and after["sorted_every_time"]
            else:
                assert before["mismatches"]==2 and after["mismatches"]==0
            results.append({"pr":n,"head_sha":entry["head_sha"],"before":before,"after":after})
    args.output.write_text(json.dumps({"master_sha":index["master_sha"],"reviews":results},ensure_ascii=False,indent=2)+"\n")
    print("Three baseline/candidate host reviews passed; no CANN/NPU used.")
if __name__=="__main__": main()
