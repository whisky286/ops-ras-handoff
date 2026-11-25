#!/bin/bash
R=/home/hww/projects/ops-ras/ops-ras-handoff/ops-ras-npu-handoff-2026-11-23
V=/home/hww/projects/ops-ras/opp/abft-exp-install/vendors/verifyrasexp_ras
for F in 0 0.05 0.2 0.5 1.0; do
  export ABFT_EMAX_FLOOR=$F
  bash $R/results/harness/run_probe_v2.sh $V 2 0 exp$F > $R/results/C1exp-floor$F-nofault.log 2>&1
  echo "floor=$F nofault_rc=$?"
  bash $R/results/harness/run_probe_v2.sh $V 2 3 exp${F}f > $R/results/C1exp-floor$F-rowfault.log 2>&1
  echo "floor=$F rowfault_rc=$?"
done
echo SWEEP_DONE
