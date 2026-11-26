# 续接说明(下个会话先读这里)

更新时间:2026-11-26(服务器驱动 26.1.1,CANN 9.0.0)
仓库:whisky286/ops-ras-handoff,当前 HEAD 以 main 最新提交为准
GitCode 侧禁止任何写操作(评论/PR/合并/关闭)

## 当前任务
#118 MatmulAbftVerify FP16/BF16 小 K 误报。已排除:统一放大阈值。
候选(log(max(k,2)) + kmin=512):误报 891->14,目标行检出 116/117;K=1 残余 3/11,未采纳。
上游 !235 已验失败:FP16 K>=121 sync 507015;小 K 未改善。诊断见 server-run-2026-11-25/C118_PR235_DIAGNOSIS.md。

## 下一步(唯一未完成项)
K=1 残余的定位:
- 已用 outputThre=1 dump 了 workspace 的 zRow/dRow/thre,按 |z-d|>thre 判定 64 行全通过,
  但 compRow 仍报 1~2 行失败,说明实际比较不在已 dump 的 block。
- 需在 reliability/matmul_abft_verify/op_kernel/FT_self/gemv/block/block_sum_aiv_ceft_no_splitk_thre_robust_preload.hpp
  的 tileCompare 调用处临时打印或 dump 三个输入与结果位。

## 环境与操作
- ssh sg144,先 . /home/hww/env.sh(NPU 用 Phy 6,备用 7;程序内逻辑设备 0)
- 工作树:ops-ras-candidate(主)、pr-work/pr118(阶段1实验)、pr-work/pr235/126/135/87/233
- OPP:opp/pr118-install、pr235-install、candidate-fix-install、baseline-install、candidate-install
- 验证:results/harness/abft_verify_probe_v2.cpp + run_probe_v2.sh(退出码 0/3);参数探针 param_probe_v2.cpp(0/3/4/5)
- 推送:https + credential store,先 source env.sh;单次请求体 <100000 字节,大提交要拆

## 其他进行中/已交付
- #56/#57:!129 本地应用验证通过(非法 8->0,UT 28/28);未合入
- #53/!126、#65/!135、#23/!87:Host/OpAPI UT 通过
- #109/!233:参数层与 bisheng_flags 透传通过;模板真实键、Valgrind 缺
- 报告在 server-run-2026-11-25/;台账 ISSUE_LEDGER.csv

## 快速自检
1. ssh sg144, cd /home/hww/projects/ops-ras/ops-ras-handoff && git pull
2. . /home/hww/env.sh && npu-smi info(确认 6 号空闲)
3. 阅读 server-run-2026-11-25/C118_STAGE1.md 与 RESULTS.md
