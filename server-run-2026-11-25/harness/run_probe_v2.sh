#!/bin/bash
# usage: run_probe_v2.sh <vendor_dir> <devId> <mode> <tag>
set +u
OPP=$1; DEV=${2:-2}; MODE=${3:-0}; TAG=${4:-v2}
R=/home/hww/projects/ops-ras/ops-ras-handoff/ops-ras-npu-handoff-2026-11-23
. /home/hww/env.sh >/dev/null 2>&1
export ASCEND_CUSTOM_OPP_PATH="$OPP"
export ASCEND_RT_VISIBLE_DEVICES="$DEV"
export LD_LIBRARY_PATH="$OPP/op_api/lib:$LD_LIBRARY_PATH"
A=/usr/local/Ascend/cann-9.0.0/aarch64-linux
g++ -std=c++17 -O1 -fPIC -I"$OPP/op_api/include" -I"$A/include" -I"$A/include/aclnnop" \
  "$R/results/harness/abft_verify_probe_v2.cpp" \
  -L"$OPP/op_api/lib" -L"$A/lib64" -L/usr/local/Ascend/driver/lib64 \
  -lcust_opapi -lopapi_math -lacl_rt -lnnopbase \
  -Wl,-rpath="$OPP/op_api/lib" -Wl,-rpath,/usr/local/Ascend/driver/lib64 \
  -o "$R/results/harness/probe_v2_$TAG" || { echo "COMPILE_FAILED"; exit 9; }
"$R/results/harness/probe_v2_$TAG" 0 "$MODE"
rc=$?
echo "PROBE_EXIT=$rc"
exit $rc
