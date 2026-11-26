#!/bin/bash
R=/home/hww/projects/ops-ras/ops-ras-handoff/ops-ras-npu-handoff-2026-11-23
V=/home/hww/projects/ops-ras/opp/pr118-install/vendors/verifyras118_ras
for S in 0.25 0.5 1 2 4 8; do
  export ABFT_EMAX_SCALE=$S
  bash $R/results/harness/run_probe_v2.sh $V 2 0 scale$S > $R/results/C118s-scale$S-nofault.log 2>&1
  echo "scale=$S nofault_rc=$?"
  bash $R/results/harness/run_probe_v2.sh $V 2 3 scale${S}f > $R/results/C118s-scale$S-rowfault.log 2>&1
  echo "scale=$S rowfault_rc=$?"
done
echo SWEEP_DONE
