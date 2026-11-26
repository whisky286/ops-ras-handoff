#!/bin/bash
R=/home/hww/projects/ops-ras/ops-ras-handoff/ops-ras-npu-handoff-2026-11-23
. /home/hww/env.sh >/dev/null 2>&1
export CPLUS_INCLUDE_PATH="/home/hww/projects/ops-ras/deps-include"
cd /home/hww/projects/ops-ras/pr-work/pr233 || exit 9
echo "=== bisheng flags build $(date -Is) ==="
bash build.sh --opkernel --ops=matmul_abft_verify --soc=ascend910_93 --bisheng_flags=ccec_g -j8 2>&1
echo "BISHENG_EXIT=$?"
