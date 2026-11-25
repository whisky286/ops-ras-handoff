#!/bin/bash
R=/home/hww/projects/ops-ras/ops-ras-handoff/ops-ras-npu-handoff-2026-11-23
. /home/hww/env.sh >/dev/null 2>&1
export CPLUS_INCLUDE_PATH="/home/hww/projects/ops-ras/deps-include"
cd $R/ops-ras-candidate || exit 9
bash build.sh -u --ophost --ops=matmul_abft_verify --soc=ascend910_93 -j8 2>&1
echo "UT_FIX_EXIT=$?"
