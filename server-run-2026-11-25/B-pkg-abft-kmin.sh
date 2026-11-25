#!/bin/bash
R=/home/hww/projects/ops-ras/ops-ras-handoff/ops-ras-npu-handoff-2026-11-23
. /home/hww/env.sh >/dev/null 2>&1
export CPLUS_INCLUDE_PATH="/home/hww/projects/ops-ras/deps-include"
cd $R/ops-ras-candidate || exit 9
bash build.sh --pkg --ops=matmul_abft_verify --soc=ascend910_93 --vendor_name=verifyraskmin -j8 2>&1
echo "PKG_KMIN_EXIT=$?"
I=/home/hww/projects/ops-ras/opp/abft-kmin-install
rm -rf $I; mkdir -p $I
bash build_out/cann-ops-ras-verifyraskmin_linux-aarch64.run --quiet --install-path=$I 2>&1 | tail -2
echo "INSTALL_KMIN_EXIT=$?"
echo ALL_DONE
