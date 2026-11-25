#!/bin/bash
R=/home/hww/projects/ops-ras/ops-ras-handoff/ops-ras-npu-handoff-2026-11-23
V=/home/hww/projects/ops-ras/opp/abft-kmin-install/vendors/verifyraskmin_ras
for K in 0 64 128 256 512 1024; do
  export ABFT_KMIN=$K
  bash $R/results/harness/run_probe_v2.sh $V 2 0 kmin$K > $R/results/C1kmin-$K-nofault.log 2>&1
  echo "kmin=$K nofault_rc=$?"
  bash $R/results/harness/run_probe_v2.sh $V 2 3 kmin${K}f > $R/results/C1kmin-$K-rowfault.log 2>&1
  echo "kmin=$K rowfault_rc=$?"
done
echo SWEEP_DONE
