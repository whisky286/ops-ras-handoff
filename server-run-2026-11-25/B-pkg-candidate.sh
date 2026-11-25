#!/bin/bash
R=/home/hww/projects/ops-ras/ops-ras-handoff/ops-ras-npu-handoff-2026-11-23
. /home/hww/env.sh >/dev/null 2>&1
export CPLUS_INCLUDE_PATH="/home/hww/projects/ops-ras/deps-include"
cd $R/ops-ras-candidate || exit 9
echo "=== pkg candidate matmul_abft_verify $(date -Is) ==="
bash build.sh --pkg --ops=matmul_abft_verify --soc=ascend910_93 --vendor_name=verifyras -j8 2>&1
echo "PKG_EXIT=$?"
find build/packages -maxdepth 3 -type d 2>/dev/null | head -20
df -h /home | tail -1
echo ALL_DONE
