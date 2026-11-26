#!/bin/bash
R=/home/hww/projects/ops-ras/ops-ras-handoff/ops-ras-npu-handoff-2026-11-23
. /home/hww/env.sh >/dev/null 2>&1
export CPLUS_INCLUDE_PATH="/home/hww/projects/ops-ras/deps-include:/home/hww/projects/ops-ras/pr-work/pr87/reliability/obfuscation_setup/op_host/op_api:/home/hww/projects/ops-ras/pr-work/pr87/reliability/obfuscation_calculate/op_host/op_api"
cd /home/hww/projects/ops-ras/pr-work/pr87 || exit 9
bash build.sh -u --opapi --ops=obfuscation_setup,obfuscation_calculate --soc=ascend910_93 -j8 2>&1
echo "PR87_UT_EXIT=$?"
