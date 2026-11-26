#!/bin/bash
OPP=$1; TAG=$2
R=/home/hww/projects/ops-ras/ops-ras-handoff/ops-ras-npu-handoff-2026-11-23
. /home/hww/env.sh >/dev/null 2>&1
export ASCEND_CUSTOM_OPP_PATH=$OPP ASCEND_RT_VISIBLE_DEVICES=2
export LD_LIBRARY_PATH=$OPP/op_api/lib:$LD_LIBRARY_PATH
A=/usr/local/Ascend/cann-9.0.0/aarch64-linux
g++ -std=c++17 -O1 -I$OPP/op_api/include -I$A/include -I$A/include/aclnnop \
  $R/results/harness/abft_verify_probe_k128.cpp \
  -L$OPP/op_api/lib -L$A/lib64 -L/usr/local/Ascend/driver/lib64 \
  -lcust_opapi -lopapi_math -lacl_rt -lnnopbase \
  -Wl,-rpath=$OPP/op_api/lib -o $R/results/harness/k128_$TAG || { echo COMPILE_FAILED; exit 9; }
$R/results/harness/k128_$TAG 0 0
