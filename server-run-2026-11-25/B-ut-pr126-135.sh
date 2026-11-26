#!/bin/bash
R=/home/hww/projects/ops-ras/ops-ras-handoff/ops-ras-npu-handoff-2026-11-23
. /home/hww/env.sh >/dev/null 2>&1
export CPLUS_INCLUDE_PATH="/home/hww/projects/ops-ras/deps-include"
for pr in 126 135; do
  cd /home/hww/projects/ops-ras/pr-work/pr$pr || exit 9
  echo "=== pr$pr host UT $(date -Is) ==="
  bash build.sh -u --ophost --ops=matmul_abft_verify --soc=ascend910_93 -j8 2>&1
  echo "PR${pr}_UT_EXIT=$?"
done
echo ALL_DONE
