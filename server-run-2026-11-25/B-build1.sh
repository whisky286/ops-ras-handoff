#!/bin/bash
R=/home/hww/projects/ops-ras/ops-ras-handoff/ops-ras-npu-handoff-2026-11-23
. /home/hww/env.sh >/dev/null 2>&1
export CXXFLAGS="-I/home/hww/projects/ops-ras/deps-include"
cd $R/ops-ras-candidate || exit 9
echo "=== B1 ophost matmul_abft_verify $(date -Is) ==="
bash build.sh -u --ophost --ops=matmul_abft_verify --soc=ascend910_93 -j8 2>&1
echo "B1_EXIT=$?"
df -h /home | tail -1
echo ALL_DONE
