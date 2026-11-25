#!/bin/bash
R=/home/hww/projects/ops-ras/ops-ras-handoff/ops-ras-npu-handoff-2026-11-23
. /home/hww/env.sh >/dev/null 2>&1
export CPLUS_INCLUDE_PATH="/home/hww/projects/ops-ras/deps-include:/home/hww/projects/ops-ras/ops-ras-handoff/ops-ras-npu-handoff-2026-11-23/ops-ras-candidate/reliability/crypto/op_host/op_api:/home/hww/projects/ops-ras/ops-ras-handoff/ops-ras-npu-handoff-2026-11-23/ops-ras-candidate/reliability/obfuscation_setup/op_host/op_api:/home/hww/projects/ops-ras/ops-ras-handoff/ops-ras-npu-handoff-2026-11-23/ops-ras-candidate/reliability/obfuscation_calculate/op_host/op_api"
cd $R/ops-ras-candidate || exit 9
echo "=== B2 opapi crypto/obfuscation $(date -Is) ==="
bash build.sh -u --opapi --ops=crypto,obfuscation_setup,obfuscation_calculate --soc=ascend910_93 -j8 2>&1
echo "B2_EXIT=$?"
df -h /home | tail -1
echo ALL_DONE
