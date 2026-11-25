#!/bin/bash
R=/home/hww/projects/ops-ras/ops-ras-handoff/ops-ras-npu-handoff-2026-11-23
V=$1
. /home/hww/env.sh >/dev/null 2>&1
export ASCEND_CUSTOM_OPP_PATH=$V ASCEND_RT_VISIBLE_DEVICES=2
export LD_LIBRARY_PATH=$V/op_api/lib:$LD_LIBRARY_PATH
A=/usr/local/Ascend/cann-9.0.0/aarch64-linux
g++ -std=c++17 -O1 -I$V/op_api/include -I$A/include -I$A/include/aclnnop \
  $R/results/harness/param_probe.cpp -L$V/op_api/lib -L$A/lib64 -L/usr/local/Ascend/driver/lib64 \
  -lcust_opapi -lopapi_math -lacl_rt -lnnopbase -Wl,-rpath=$V/op_api/lib -o $R/results/harness/param_probe 2>&1
echo "COMPILE_EXIT=$?"
$R/results/harness/param_probe 0 2>&1
