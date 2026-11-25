#!/bin/bash
# usage: run_probe.sh <repo_dir> <vendor_dir> <devId> <mode> <tag>
set +u
REPO=$1; OPP=$2; DEV=${3:-10}; MODE=${4:-0}; TAG=${5:-run}
VENDOR="$OPP"
R=/home/hww/projects/ops-ras/ops-ras-handoff/ops-ras-npu-handoff-2026-11-23
. /home/hww/env.sh >/dev/null 2>&1
export ASCEND_CUSTOM_OPP_PATH="$OPP"
export ASCEND_RT_VISIBLE_DEVICES="$DEV"
export LD_LIBRARY_PATH="$VENDOR/op_api/lib:$LD_LIBRARY_PATH"
ACL_HOME=/usr/local/Ascend/cann-9.0.0/aarch64-linux
g++ -std=c++17 -O1 -fPIC \
  -I"$VENDOR/op_api/include" -I"$ACL_HOME/include" -I"$ACL_HOME/include/aclnnop" \
  "$R/results/harness/abft_verify_probe.cpp" \
  -L"$VENDOR/op_api/lib" -L"$ACL_HOME/lib64" -L/usr/local/Ascend/driver/lib64 \
  -lcust_opapi -lopapi_math -lacl_rt -lnnopbase \
  -Wl,-rpath="$VENDOR/op_api/lib" -Wl,-rpath,/usr/local/Ascend/driver/lib64 \
  -o "$R/results/harness/probe_$TAG" 2>&1
echo "COMPILE_EXIT=$?"
"$R/results/harness/probe_$TAG" 0 "$MODE"
echo "RUN_EXIT=$?"
