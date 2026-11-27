# ops-ras 交接任务:服务器侧阶段结果(截至 2026-11-25)

仓库:https://gitcode.com/cann/ops-ras · 基线提交 6943b10042266ee3dbee3e17237919a0ded600bf
服务器:sg144(node73-144),CANN 9.0.0,Ascend910_9382,SoC=ascend910_93
远端写入:未执行任何评论/PR/推送/合并/关闭操作。

## A 阶段:主机检查
| 项 | 命令 | 结果 |
|---|---|---|
| 8 项构建参数回归 | unittest -p test_build_optional_flags.py | 8/8 OK(A1-candidate-regressions.log) |
| bash 语法 | bash -n build.sh | OK |
| 空白检查 | git diff --check(cr-at-eol) | OK |
| parser 3 项 | verify_parser_scope.py | OK(results/parser.json) |
| 错误不传播(基线) | verify_failure_propagation --expect baseline | PASS=缺陷复现(failure-baseline.json) |
| 错误不传播(候选修复后) | --expect correct | 4/4 符合预期(failure-candidate.json) |

补丁(results/patches/):
- issue88-port-pr160-merge-aicpu.patch(移植 PR !160)
- buildcheck-opc-propagate-exit.patch(return "$gen_res")
说明:PR !162(gen_compile_option.sh)只做了 diff 静态核对,未移植/未验证。

## B 阶段:CANN 构建与 Host UT
| 项 | 结果 |
|---|---|
| Host UT matmul_abft_verify | 12 tests / 3 suites PASSED(B-build1.log) |
| opapi UT crypto+obfuscation | 29 tests / 5 suites PASSED(B-build2.log) |
| Valgrind | 服务器未安装,记为依赖缺失 |
| --bisheng_flags / --kernel_template_input 真实传参 | 未验证 |

环境适配(未改仓库逻辑,详见 ENV_NOTES.md):
- CANN 9.0.0 缺 graph/error_codes.h → 个人 shim + CPLUS_INCLUDE_PATH
- crypto/obfuscation op_api UT 缺 include → CPLUS_INCLUDE_PATH 追加

## C 阶段:设备验证
### #118 FP16 小 K 误报 —— 已复现,候选无修复
- 基线 FP16 K≤129 大面积误报(K=3/seed42=54/64 与 issue 一致);FP32 全 0;BF16 K≤3 亦有误报(issue 未覆盖)。
- 候选结果与基线一致 → functional.patch 不含该修复。
- 注入行和量级单元素故障:117/117 全部检出;10% 相对单元素误差受行和稀释影响有 30/117 漏检(敏感度边界)。
- 详见 C1_118.md;日志 C1-*.log;harness results/harness/abft_verify_probe.cpp
- 状态:**已复现,根因未修**

### #56 / #57 参数校验 —— 已复现,候选无修复
- C rank/列数错、weight 长度/rank 错、eMax<0/0/NaN/Inf 全部 GetWorkspaceSize=0 且执行成功。
- 仅 A/B dtype 不匹配被拒(161002)。
- 基线/候选一致;详见 C2_567_117.md;日志 C2-param-*.log

### #117 V2 include guard —— 已复现,本地修复已验证
- 两套 V2 头与 V1 同名 guard,同 TU 包含时只可见一套。
- 按 PR !208 意图修正 guard 名;4/4 编译顺序验证通过;补丁 issue117-v2-include-guard.patch
- !208 其余部分(ut.cmake、新增 UT)未整包验证。

## 未开始 / 待办
- #52 crypto 非连续 view 六项;需先确认 PMCC/TA 服务可用性(环境依赖)
- #23 obfuscation shape 校验(需打 obfuscation 包并验证服务)
- #53 FP16 mean factor(10^7 K 的 Host tiling 检查)
- #65 动态 InferShape Host 测试
- #16 示例支持范围文档核对
- 28 条扫描项逐条处置;99 条 issue 台账分类
- #139/#140 附件仍需有权限账号(快照阶段 401)
- #109 真实 bisheng/kernel_template 传参与 valgrind 缺失记录已入台账

### #52 aclnnCrypto 非连续 view —— 静态确认,环境阻塞
- 根因在候选源码中存在(MakeContiguous 丢弃连续化结果,输出未回写),见 C2_52.md
- CANN 9.0.0 无 Crypto AICPU 实现,读密钥需 KMS/SPDM → 设备验证阻塞
- 草稿修复编译可过但 UT stub case_success 失败(561103),已回滚;草稿存
  results/patches/draft-issue52-crypto-contiguous.patch
- 回滚后 crypto UT 14/14 PASSED(C2-crypto-ut-clean.log)

## 验证程序补强(v2)
- #118 与 #56/#57 的 harness 已重写:v2 检查全部返回值、哨兵初值、written 校验、精确故障行判定;
  非法参数不再送入设备执行。
- 复核结果与 v1 结论一致,但 v2 为验收依据;日志 C1v2-*.log / C2v2-param-*.log

### 故障统计口径更正
- 行和故障:目标行 117/117 检出(13 例原已误报,新增检出 104/104)
- 相对 10% 故障:目标行 85/117,32 例漏检(新增检出 72/104)
- 以 results/C1_118.md 的更正表为准

## 2026-11-26 增补
### #56/#57(!129 本地应用)
- 修复前:非法参数 8/8 被阶段一接受;defect rc=0,fix rc=5
- 修复后:非法参数 0/8 接受;defect rc=4,fix rc=0;Host UT 28/28
- 库哈希:liboptiling.so eceb666f→9cec83f2;libcust_opapi 未变
- 边界:未合入、未推送 GitCode;文档 hunk 未应用
### #118
- threshold floor 实验证伪:误报 891→14,但目标行检出 117/117→57/117(负结果)
- 根因进一步定位到 kernel half 精度阈值路径(tile_threshold_simplified.hpp),尚未修复
### 验证程序
- 参数探针退出码语义化:0 期望一致 / 3 内部失败 / 4 未复现 / 5 修复未生效
- run_probe_v2.sh / run_param_v2.sh 均传递真实退出码
- #118 第二实验(kmin 校准下限):kmin=512 时误报 892→14、检出 117/117,但相邻点非单调(BF16 K=1 反弹 174),
  作为候选方向保留,不采纳;草稿 draft-issue118-kmin-experiment.patch

## 环境变更提示(2026-11-26)
- 服务器重启后驱动由 26.0.rc1 升级到 26.1.1;CANN 9.0.0 不变。
- 本文档中的设备结果均来自 26.0.rc1,重启后未重测(用户决定)。

## 2026-11-26 分诊增量(服务器侧)
- #118/!235:未通过(FP16 小 K 误报未改善,且 K>=129 同步 507015)
- #109/!233:参数层通过(基线四项非法,!233 正确接受并校验)
- #53/!126:Host UT 13/13(含 mean factor 次正规用例)
- #65/!135:Host UT 15/15(含动态 M/N/K)
- #23/!87:OpAPI UT 19/19(fd=[2], x/y 同 rank 不同维度等拒绝用例)
- 详见 C_TRIAGE_2026-11-26.md;所有 PR 均未合入,不能据此关闭 issue

## #118 阶段 1(2026-11-26)
- 统一 eMax 乘子:scale=8 仍有 FP16 K=1/2 误报(22/19),检出降至 109/117 → 不可行
- log(max(k,2)) + kmin=512:总误报 891→14(仅 K=1),检出 116/117 → 候选未采纳
- K=1 残余对 kmin 不敏感(512~4096 恒为 3/11)→ 疑统计退化(NaN/除零),需单独定位
- 详见 C118_STAGE1.md;草稿 draft-issue118-logclamp-kmin-experiment.patch

## #118 K=1 残余定位与修复（2026-11-27）
- 在 `tileCompare` 处 dump 后确认：K=1 失败行 threshold = NaN，来源是 AIV 的 A 侧 aMax/aMin
  在 K=1 时未取到唯一元素（读到未初始化 UB）；比较 X/C 与结果位本身正确。
- 修复：CE block 增加 kActual；`k == 1` 时 aMax = aMin = aMean。
- 与阶段 1 的 log-clamp + kmin=512 组合，117 用例：**无故障误报 0、行和故障检出 117/117**。
- 详见 `C118_K1_DIAGNOSIS.md`；补丁 `patches/draft-issue118-k1-extrema-fix.patch`；
  日志 `C1k1final-nofault.log`、`C1k1final-rowfault.log`。
- 阶段 1 的 "kmin 候选未采纳（因 K=1 残余）" 结论已被本轮修复解除；剩余工作是固化 kmin 常量并走上游。

## #118 最终修复与固化验收（2026-11-27）
- 交付两个独立补丁（基线 6943b10）：`patches/issue118-tiling-threshold-scaling.patch`（Host + UT）、
  `patches/issue118-k1-extrema-fix.patch`（Kernel CE block）；均通过 `git apply --check`（单独/合并）。
- tiling 已把 `kMin=512` 固化为编译期常量，移除 `ABFT_KMIN` 环境变量。
- Phy 8（无进程）重跑 117 用例：无故障误报 0、行和故障检出 117/117、internal_failures=0。
- Host UT 12/12 PASSED。
- 日志：`C1final-nofault.log`、`C1final-rowfault.log`、`C1final-hostut.log`、`B-pkg-pr118-final.log`；
  详见 `C118_FINAL.md`。
- 阶段 1 的“kmin 候选未采纳”结论已被本轮修复解除。
