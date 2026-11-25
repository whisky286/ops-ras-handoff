#!/bin/bash
R=/home/hww/projects/ops-ras/ops-ras-handoff/ops-ras-npu-handoff-2026-11-23
. /home/hww/env.sh >/dev/null 2>&1
export CPLUS_INCLUDE_PATH="/home/hww/projects/ops-ras/deps-include"
cd $R/ops-ras-baseline || exit 9
echo "=== pkg baseline matmul_abft_verify $(date -Is) ==="
bash build.sh --pkg --ops=matmul_abft_verify --soc=ascend910_93 --vendor_name=verifyrasbase -j8 2>&1
echo "PKG_BASE_EXIT=$?"
I=/home/hww/projects/ops-ras/opp/baseline-install
rm -rf $I; mkdir -p $I
bash build_out/cann-ops-ras-verifyrasbase_linux-aarch64.run --quiet --install-path=$I 2>&1 | tail -3
echo "INSTALL_BASE_EXIT=$?"
df -h /home | tail -1
echo ALL_DONE
