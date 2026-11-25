#!/bin/bash
R=/home/hww/projects/ops-ras/ops-ras-handoff/ops-ras-npu-handoff-2026-11-23
. /home/hww/env.sh >/dev/null 2>&1
export CPLUS_INCLUDE_PATH="/home/hww/projects/ops-ras/deps-include"
cd $R/ops-ras-candidate || exit 9
echo "=== dev opapi+opkernel matmul_abft_verify $(date -Is) ==="
bash build.sh --opapi --opkernel --ops=matmul_abft_verify --soc=ascend910_93 -j8 2>&1
echo "DEV_EXIT=$?"
find $R/ops-ras-candidate/build -name "libopapi_ras.so" -o -name "*.run" -o -name "op_api_ras*.so" 2>/dev/null | head
df -h /home | tail -1
echo ALL_DONE
