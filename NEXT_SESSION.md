# 续接说明（下个会话先读这里）

更新时间：2026-11-27（服务器驱动 26.1.1，CANN 9.0.0）
仓库：whisky286/ops-ras-handoff，当前 HEAD 以 main 最新提交为准
GitCode 侧禁止任何写操作（评论/PR/合并/关闭）

## 当前任务
#118 MatmulAbftVerify FP16/BF16 小 K 误报。

**K=1 残余已定位并修复（2026-11-27）**，详见 `C118_K1_DIAGNOSIS.md`：
- 根因：K=1 时 AIV 的 A 侧 max/min 归约对部分行未取到唯一元素，读到未初始化 UB，
  产生 aMax/aMin = NaN -> threshold = NaN -> 结果位 0（并非比较逻辑问题）。
- 修复：CE block 增加 kActual 参数；`k == 1` 时令 aMax = aMin = aMean（严格成立）。
- 与阶段 1 的 log-clamp + kmin=512 组合，117 用例全矩阵：**无故障 0 误报、行和故障 117/117 检出**。
- 补丁：`patches/draft-issue118-k1-extrema-fix.patch`
- 日志：`C1k1final-nofault.log`、`C1k1final-rowfault.log`
- 诊断 harness：`harness/abft_verify_probe_cmp.cpp`

## 下一步（按优先级）
1. 把 tiling 钩子里的 `kmin=512` 固化为常量（去掉 `ABFT_KMIN` 环境变量依赖），
   复核 117 矩阵仍为 0 误报 / 117 检出。
2. 给上游提交：#118 的正式修复 = tiling 阈值缩放 + K=1 extrema 修复（补丁可直接复用）。
3. 跟进 A 侧 extrema 归约对 `1 < n_actual < repeat_size` 的 padding 读取：
   建议在 `TileFaultSum<MAX_MIN>` 入口统一按 `n_actual` 钳位并补 UT。
4. 复核 !235：其 FP16 K>=121 的 507015 问题与本修复独立（CE UB 切片按 FP32 重算）。

## 环境与操作
- ssh sg144，先 `. /home/hww/env.sh`（NPU 用 Phy 6，备用 7；程序内逻辑设备 0）
- 工作树：`/home/hww/projects/ops-ras/pr-work/pr118`（阶段1 + K=1 修复，含 tiling 钩子）
- OPP：`opp/abft-dbg-install`（vendor `verifyrasdbg`，含本次修复）；
  其他：pr118-install、abft-kmin-install、candidate-fix-install、baseline-install、candidate-install
- 构建：`B-pkg-*.sh` 模板；本次命令见 `C118_K1_DIAGNOSIS.md`
- 验证：`results/harness/run_probe_v2.sh`（退出码 0/3）；参数探针 `run_param_v2.sh`（0/3/4/5）
- 推送：https + credential store，先 `source env.sh`；单次请求体 < 100000 字节，大提交要拆

## 其他进行中/已交付
- #56/#57：!129 本地应用验证通过（非法 8->0，UT 28/28）；未合入
- #53/!126、#65/!135、#23/!87：Host/OpAPI UT 通过
- #109/!233：参数层与 bisheng_flags 透传通过；模板真实键、Valgrind 缺
- 报告在 `server-run-2026-11-25/`；台账 ISSUE_LEDGER.csv

## 快速自检
1. `ssh sg144`，`cd /home/hww/projects/ops-ras/ops-ras-handoff && git pull`
2. `. /home/hww/env.sh && npu-smi info`（确认 6 号空闲）
3. 阅读 `server-run-2026-11-25/C118_K1_DIAGNOSIS.md` 与 `RESULTS.md`
