#!/bin/bash
R=/home/hww/projects/ops-ras/ops-ras-handoff/ops-ras-npu-handoff-2026-11-23
. /home/hww/env.sh >/dev/null 2>&1
export CPLUS_INCLUDE_PATH="/home/hww/projects/ops-ras/deps-include"
cd /home/hww/projects/ops-ras/pr-work/pr235 || exit 9
bash build.sh --pkg --ops=matmul_abft_verify --soc=ascend910_93 --vendor_name=verifyras235 -j8 2>&1
echo "PKG235_EXIT=$?"
I=/home/hww/projects/ops-ras/opp/pr235-install
rm -rf $I; mkdir -p $I
bash build_out/cann-ops-ras-verifyras235_linux-aarch64.run --quiet --install-path=$I 2>&1 | tail -2
echo "INSTALL235_EXIT=$?"
echo ALL_DONE
