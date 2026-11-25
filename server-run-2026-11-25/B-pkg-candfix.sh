#!/bin/bash
R=/home/hww/projects/ops-ras/ops-ras-handoff/ops-ras-npu-handoff-2026-11-23
. /home/hww/env.sh >/dev/null 2>&1
export CPLUS_INCLUDE_PATH="/home/hww/projects/ops-ras/deps-include"
cd $R/ops-ras-candidate || exit 9
echo "=== pkg candidate+pr129 $(date -Is) ==="
bash build.sh --pkg --ops=matmul_abft_verify --soc=ascend910_93 --vendor_name=verifyrasfix -j8 2>&1
echo "PKG_FIX_EXIT=$?"
I=/home/hww/projects/ops-ras/opp/candidate-fix-install
rm -rf $I; mkdir -p $I
bash build_out/cann-ops-ras-verifyrasfix_linux-aarch64.run --quiet --install-path=$I 2>&1 | tail -2
echo "INSTALL_FIX_EXIT=$?"
echo ALL_DONE
