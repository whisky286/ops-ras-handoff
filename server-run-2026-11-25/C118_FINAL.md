# #118 MatmulAbftVerify 小 K 误报 — 最终修复与验收（2026-11-27）

## 结论

FP16/BF16 小 K 误报已修复，交付形态为两个相互独立、可单独回滚的补丁：

1. `patches/issue118-tiling-threshold-scaling.patch`（Host tiling + UT）
2. `patches/issue118-k1-extrema-fix.patch`（Kernel CE block）

固化后（无环境变量）在 Phy 8 重跑 117 用例全矩阵：

| 配置 | 无故障误报 | 行和故障检出 | 内部失败 |
|---|---|---|---|
| 基线 | 891 | 117/117 | 0 |
| 阶段 1（log-clamp + kmin=512，实验钩子） | 14（仅 K=1） | 116/117 | 0 |
| **最终（kmin 常量化 + K=1 extrema 修复）** | **0** | **117/117** | **0** |

Host UT：12/12 PASSED（`C1final-hostut.log`）。

## 根因

K=1 残余误报**不是比较逻辑问题**，而是 **threshold = NaN**：

- K=1 时 AIV 的 A 侧 max/min 归约对部分行没有取到该行唯一的元素，读到未初始化 UB
  （清零 A tile 后 aMax/aMin=0，而 host 非 0），得到 `aMax/aMin = NaN`。
- `threshold = f(aMax,aMin,aMean,...) = NaN`，比较 `|X-C| - NaN <= 0` 恒为假，结果位 0。
- 同一行 AIC 单独计算的 `aMean` 始终正确（`add_ae_op` 写入），故 K=1 时可用 mean 覆盖 extrema。
- K>=2 的误报由阈值估计偏小导致，用 `sqrt(kMin/k)` 放大阈值解决（阶段 1 已验证）。

## 补丁 1：tiling 阈值缩放（Host）

文件：`op_host/matmul_abft_verify_tiling.cpp`、`tests/ut/op_host/test_matmul_abft_verify_tiling.cpp`

- `stdEstARowRatio = 1/sqrt(2*log(max(k,2)))`：K=1 时 log(1)=0 会除零，钳位到 log(2)。
- `kMin = 512.0f` 为**编译期常量**（已移除 `ABFT_KMIN` 环境变量与 `<cstdlib>`/`getenv`/`atof`）；
  当 `k < kMin` 时 `eMax *= sqrt(kMin/k)`，即小 K 阈值下限等价于按 kMin 计算。
- UT 同步更新 eMax 与 stdEstARowRatio 期望值。

## 补丁 2：K=1 extrema 修复（Kernel）

文件：`op_kernel/FT_self/gemv/block/block_sum_aiv_ceft_no_splitk_thre_robust_preload.hpp`、
`op_kernel/matmul_abft_verify.h`

- CE block 入口新增 `uint32_t kActual`，调用点传 `params.problemGemmShape.k()`。
- `kActual == 1` 时令 `aMax = aMin = aMean`（单元素时 min==max==mean 严格成立），再算 std/阈值。

## 补丁可用性

- 基线：`6943b10`（ops-ras 代码化迁移）。
- 两个补丁分别及合并 `git apply --check` 均通过（在干净 worktree 验证）。
- 无其它源码改动；实验草稿已归档到 `patches/experiments/`。

## 环境与复现

- 服务器 sg144，驱动 26.1.1，CANN 9.0.0，SoC ascend910_93。
- 设备：Phy 8（备用 9），程序内逻辑设备 0；运行前 `npu-smi` 确认无进程。
- 构建：`B-pkg-pr118-final.log.gz`（vendor `verifyrasdbg`，安装到 `opp/abft-dbg-install`）。
- 运行（**不设置 ABFT_KMIN**）：
  ```bash
  . /home/hww/env.sh
  V=/home/hww/projects/ops-ras/opp/abft-dbg-install/vendors/verifyrasdbg_ras
  bash harness/run_probe_v2.sh $V 8 0 final0   # 无故障
  bash harness/run_probe_v2.sh $V 8 3 final3   # 行和故障
  ```
- 日志：`C1final-nofault.log`（117 行，bad 全 0）、`C1final-rowfault.log`（117 行，bad 全 1、faultBit 全命中）。
- Host UT：`C1final-hostut.log`（12/12 PASSED）。

## 验收矩阵（最终，Phy 8）

| dtype | 用例数 | 无故障 bad>0 | 行和故障检出 | internal_failures |
|---|---|---|---|---|
| float32 | 39 | 0 | 39/39 | 0 |
| float16 | 39 | 0 | 39/39 | 0 |
| bfloat16 | 39 | 0 | 39/39 | 0 |
| 合计 | 117 | 0 | 117/117 | 0 |

## 回滚

两个补丁互不依赖，可单独 `git apply -R` 回滚；tiling 补丁回滚后仅恢复原阈值公式，
K=1 extrema 补丁回滚后仅恢复原 extrema 归约路径。

## 未决 / 后续

- A 侧 extrema 归约对 `1 < n_actual < repeat_size` 的 padding 读取机制仍不严谨
  （K>=2 数值正常，未复现 NaN）；建议在 `TileFaultSum<MAX_MIN>` 入口按 `n_actual` 钳位并补 UT。
- !235（FP16 K>=121 的 507015）与本修复独立，需另行处理。
- 未对 GitCode 做任何写操作。
