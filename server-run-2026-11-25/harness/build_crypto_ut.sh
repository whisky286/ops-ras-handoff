#!/bin/bash
R=/home/hww/projects/ops-ras/ops-ras-handoff/ops-ras-npu-handoff-2026-11-23
. /home/hww/env.sh >/dev/null 2>&1
export CPLUS_INCLUDE_PATH="/home/hww/projects/ops-ras/deps-include:$R/ops-ras-candidate/reliability/crypto/op_host/op_api"
cd $R/ops-ras-candidate || exit 9
bash build.sh -u --opapi --ops=crypto --soc=ascend910_93 -j8 2>&1
echo "CRYPTO_UT_EXIT=$?"
