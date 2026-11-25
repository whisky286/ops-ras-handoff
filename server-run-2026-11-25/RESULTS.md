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
