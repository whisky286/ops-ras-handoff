# #118 K=1 残余定位与修复（2026-11-26/27）

## 结论

K=1 残余误报**不是比较逻辑问题，也不是 X/C 读取竞争**，而是 **threshold = NaN**。
NaN 来自 A 侧逐行 max/min 统计量：K=1 时 AIV 的 A 统计归约对部分行没有取到该行唯一的元素，
读到未初始化 UB（可能是 NaN/inf），而 AIC 单独计算的 aMean 始终正确。

修复：在 CE block 中，当 `k == 1` 时用 `aMean` 覆盖 `aMax/aMin`（单个元素时 min == max == mean 严格成立），
再计算 std / 阈值。

与阶段 1 的 log-clamp + kmin=512 组合后，117 用例全矩阵：
**无故障 0 误报，行和故障 117/117 检出。**

## 证据（tileCompare 处 dump）

在 `block_sum_aiv_ceft_no_splitk_thre_robust_preload.hpp` 的 `tileCompare` 调用后，
把 UB 中比较实际消耗的 ABe 回写 `gmABe`（dRow）；同时从 user workspace 读取
`zRow`(X) / `dRow`(C) / `threshold` / `compRow`(结果位)。

- 比较实现：`UbZ = CompareScalar(|UbY - UbABe| - UbThre, <= 0)`；
  X = zRow（AIV 的 CE），C = dRow（AIC 直乘参考），thre = threshold，结果位 = compRow。
- 失败行（FP16 r=54、BF16 r=32，seed42）：
  `|X-C|` 分别为 2.7e-5 / 2.1e-3，远小于正常阈值；`threshold = NaN`；
  `aMax = NaN`、`aMin = NaN`，但 `aMean` 与 host 的 A 元素完全一致。
- seed 42/1234/777777：NaN 行随数据变化（如 FP16 seed42: 2,13,30,44；seed1234: 无）。
- K=2/3：dump 的 `aMax/aMin` 与 host 重算的 max/min 完全一致，无失败。
- 清零 A UB tile 后，上述行 `aMax=aMin=0`（而 aHost 非 0）：
  说明归约没有读到该行元素 0，读到的是被清零的 padding。
- 源码路径：`BlockFTSumNoSplitK`（`block_b_a_red_no_splitk_robust.hpp`，BF16 走 `_bf` 变体）
  → `TileFaultSum<... MAX_MIN ...>`；aMean 由 AIC 的 `add_ae_op` 写入，所以不受影响。

## 修复

1. CE block 入口增加 `uint32_t kActual`，调用点传 `params.problemGemmShape.k()`。
2. `tileStdEst` 之前：
   ```cpp
   if (kActual == 1) {
       // min == max == mean for a single K element
       SetCounterMask<ElementY>(m_actual);
       Muls<ElementY, false>(UbAMaxTensor, UbAMeanTensor, (ElementY)1.0f, ...);
       Muls<ElementY, false>(UbAMinTensor, UbAMeanTensor, (ElementY)1.0f, ...);
       ResetCounterMask();
       VectorBarrier();
   }
   ```
3. 补丁：`patches/draft-issue118-k1-extrema-fix.patch`
   （其中 tiling 部分是阶段 1 实验钩子：log clamp + `ABFT_KMIN` 环境变量）。

## 验证（Phy6，程序内逻辑设备 0）

```bash
. /home/hww/env.sh
V=/home/hww/projects/ops-ras/opp/abft-dbg-install/vendors/verifyrasdbg_ras
export ABFT_KMIN=512
bash results/harness/run_probe_v2.sh $V 6 0 k1final0
bash results/harness/run_probe_v2.sh $V 6 3 k1final3
```

| 配置 | 用例 | 无故障误报 | 行和故障检出 |
|---|---|---|---|
| 基线 | 117 | 891 | 117/117 |
| 阶段1 kmin=512 | 117 | 14（仅 K=1） | 116/117 |
| **kmin=512 + K=1 修复** | 117 | **0** | **117/117** |

日志：`C1k1final-nofault.log`、`C1k1final-rowfault.log`。
诊断 harness：`harness/abft_verify_probe_cmp.cpp`（逐行打印 z/d/thre/结果位、aMax/aMean/aMin、host 参考）。

## 未决 / 后续

- 补丁的 tiling 部分仍是实验钩子（环境变量 `ABFT_KMIN`）。正式修复需把 `kmin=512` 固化为
  tiling 常量，并评估上游接受度；K=1 修复本身不依赖 kmin，kmin 负责 K>=2 的误报。
- A 侧 extrema 归约对 `1 < n_actual < repeat_size` 的 padding 读取机制仍不严谨
  （目前 K>=2 数值正常，未复现 NaN）；建议上游确认或在归约入口统一按 `n_actual` 钳位。
- 未对 GitCode 做任何写操作。
