# #118 阶段 1:阈值定位测量(2026-11-26)

## 实验设置
独立工作树 pr-work/pr118(基线 bundle + 临时 tiling 钩子),独立 OPP(opp/pr118-install)。
矩阵:117 用例(3 dtype × 13 K × 3 seed),无故障 + 行和量级单元素故障;设备 2;驱动 26.1.1。

## 实验 A:eMax 统一乘子(ABFT_EMAX_SCALE)
| scale | 总误报 | FP16 K=1/2/3/5/9 | BF16 K=1/2/3 | 目标行检出 |
|---|---|---|---|---|
| 0.25 | 2339 | 94/186/187/177/171 | 62/152/147 | 117/117 |
| 0.5 | 1519 | 58/173/181/159/141 | 65/99/92 | 117/117 |
| 1 | 891 | 33/161/165/127/88 | 54/21/18 | 117/117 |
| 2 | 471 | 33/137/124/68/32 | 11/0/0 | 116/117 |
| 4 | 205 | 32/76/59/20/0 | 11/0/0 | 115/117 |
| 8 | 52 | 22/19/0/0/0 | 11/0/0 | 109/117 |

结论:单纯放大阈值到 8 倍仍留 FP16 K=1/2 误报(22/19),并开始损失检出(109/117);
K=1/2 属 `log(k)` 退化,不是阈值幅度问题。

## 实验 B:log(k) 下界钳位 + kmin 缩放
钩子:stdEstARowRatio 用 log(max(k,2));k<kmin 时 eMax *= sqrt(kmin/k)。
| kmin | 总误报 | FP16 K1/2/3 | FP16 K>=5 | BF16 K1 | 目标行检出 |
|---|---|---|---|---|---|
| 128 | 101 | 4/19/4 | 0~22 | 11 | 117/117 |
| 256 | 31 | 3/4/0 | 0~7 | 11 | 116/117 |
| 512 | 14 | 3/0/0 | 全部 0 | 11 | 116/117 |
| 1024 | 14 | 3/0/0 | 全部 0 | 11 | 113/117 |
| 2048 | 14 | 3/0/0 | 全部 0 | 11 | 113/117 |
| 4096 | 14 | 3/0/0 | 全部 0 | 11 | 112/117 |

- kmin=512:除 K=1 外全部误报清零(FP16 K>=2、全部 BF16 K>=2),检出 116/117
- 漏检用例:FP16 K=1 seed777777
- K=1 残余(FP16 3、BF16 11)对 kmin 不敏感(512→4096 完全不变),说明是结构性问题,而非阈值幅度;
  最可能是 K=1 时方差/统计退化(NaN 或除零),需要单独定位,不能靠调阈值解决

## 结论
- 统一阈值乘子**不可行**(损失检出)
- log 钳位 + kmin=512 是当前最好的 Host 侧候选:误报 891→14、检出 116/117,
  但未达到验收(bad=0 且检出 117/117),因此**不采纳为修复**,作为候选保留
- 下一步(阶段 2/3):定位 K=1 统计退化;再评估是否需要 kernel 侧改动
- 草稿补丁:draft-issue118-logclamp-kmin-experiment.patch(含临时环境变量钩子)
- 日志:C118s-scale*-nofault/rowfault.log、C118c-kmin*-nofault/rowfault.log

## K=1 残余排查(只读代码)
- `tile_std_estimate.hpp: TileStdEstRobust` 的 std 估计为 `sqrt(|(max-mean)*(mean-min)|)`,
  没有除以 (k-1) 或 n 的路径,不存在 K=1 直接除零/NaN 的明显来源。
- K=1 残余对 kmin(512~4096)完全不敏感,说明不是阈值幅度问题;
  需要在 kernel 内观测 K=1 的 threshold / expected / actual 三个量才能定论(需调试输出,尚未实现)。
- 当前最佳候选仍是 log 钳位 + kmin=512:总误报 14(仅 K=1),检出 116/117。

## K=1 内核观测(2026-11-26,调试 harness)
- 利用 tiling 已设 outputThre=1,kernel 会把每行阈值写回 workspace;
  workspace 前段顺序:zRow(64 float) → dRow(64) → threshold(64)。
- 新增调试 harness `results/harness/abft_verify_probe_dbg.cpp`:执行后从 userWorkspace(16MB 偏移)
  读出 zRow/dRow/thre 并打印;仅 K∈{1,2,3}、seed42。
- 观测(FP16/BF16,K=1):
  - zRow 与 dRow 相对差约 1e-3 量级,阈值多数 1e-3~2e-3,个别行阈值很大(24.9、37.2)
  - 用 |zRow-dRow| > thre 判定:**64 行全部通过(0 违反)**
  - 但同一进程的 compRow 对 FP16 K=1 报 1 行失败,B 矩阵全量跑时同配置报 2 行
- 结论:compRow 的失败**不能由 zRow/dRow/thre 这组量解释**,说明实际比较用的是另一组中间量
  (或 compRow 打包/多阶段写入存在细节差异);这是 K=1 残余的真正方向,已超出阈值调参范畴。
- 另:同输入重复运行出现 bad=1 / bad=2 的差异,再次提示运行间不确定性,需要在 tileCompare 内部观测。

## 当前可交付
- 调试手段已建立(workspace dump),下一步应在 `tileCompare` 处记录 X/C/thre 三个输入及比较结果。
- 阶段 1 候选(log clamp + kmin=512)仍为"未采纳";K=1 未解决前不能宣称修复。

## 比较实现核对(2026-11-26)
- `tile_fault_compare.hpp` 的 RSUB 路径:`dst = CompareScalar(|x - y| - thre, <= 0)`,与我们的 dump 判据一致。
- 但仓库里存在多个独立比较点:`block_threshold_compare_fused.hpp`、`block_sum_aiv_ceft_no_splitk_thre_robust_preload.hpp`、`block_large_local_compare.hpp` 各自持有 TileCompare。
- 推论:实际生效的比较不在我们 dump 的那个 block,需要在 `block_sum_aiv_ceft_*` 的 tileCompare 调用处抓 X/C/thre 与结果位。
- 下一步(需新会话):在该 block 加临时 dump,再做 K=1 观测;在此之前 K=1 残余无定论。

## 比较实现核对(2026-11-26)
- tile_fault_compare.hpp 的 RSUB 路径:dst=CompareScalar(|x-y|-thre, LE 0),与 dump 判据一致。
- 仓库存在多个独立比较点:block_threshold_compare_fused、block_sum_aiv_ceft_no_splitk_thre_robust_preload、block_large_local_compare。
- 推论:实际生效的比较不在已 dump 的 block,需在 block_sum_aiv_ceft 的 tileCompare 调用处抓 X/C/thre 与结果位。
