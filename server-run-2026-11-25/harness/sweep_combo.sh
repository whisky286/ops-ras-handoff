#!/bin/bash
R=/home/hww/projects/ops-ras/ops-ras-handoff/ops-ras-npu-handoff-2026-11-23
V=/home/hww/projects/ops-ras/opp/pr118-install/vendors/verifyras118_ras
for K in 128 256 512 1024; do
  export ABFT_KMIN=$K
  bash $R/results/harness/run_probe_v2.sh $V 2 0 combo$K > $R/results/C118c-kmin$K-nofault.log 2>&1
  echo "kmin=$K nofault_rc=$?"
  bash $R/results/harness/run_probe_v2.sh $V 2 3 combo${K}f > $R/results/C118c-kmin$K-rowfault.log 2>&1
  echo "kmin=$K rowfault_rc=$?"
done
echo COMBO_DONE
