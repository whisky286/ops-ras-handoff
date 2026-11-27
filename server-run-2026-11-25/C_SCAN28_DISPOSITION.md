# 28 条扫描项逐条处置（2026-11-27）

来源：`snapshots/2026-11-23/gist-review.json`（三份 Gist：接口缺文档 8、参数不一致 1、构建检查 19）。
本表给出逐条处置建议；不把 28 条扫描记录直接视为 28 个已确认 bug，未据此关闭任何 issue。

| # | 报告 | 行 | 对象 | 分类 | 结论 | 处置 |
|---|---|---|---|---|---|---|
| 1 | no-doc | 1 | CastOnlyForConvBackward(const aclTensor* self, op::DataType dstDtype,  | API_documentation_scope | 需确认公开文档范围，不能仅凭头文件中存在就认定漏文档。 | **需决策**：确认公开文档范围；CanOptimizeContiguous/CanOptimizeView 为内部接口不补文档；其余若确为公开接口再补文档，不批量补用户 API 文档。 |
| 2 | no-doc | 2 | PickViewAsContiguous(const aclTensor* x, aclOpExecutor* executor) | API_documentation_scope | 需确认公开文档范围，不能仅凭头文件中存在就认定漏文档。 | **需决策**：确认公开文档范围；CanOptimizeContiguous/CanOptimizeView 为内部接口不补文档；其余若确为公开接口再补文档，不批量补用户 API 文档。 |
| 3 | no-doc | 3 | ReViewToOut(const aclTensor* x, const aclTensor* y, aclOpExecutor* exe | API_documentation_scope | 需确认公开文档范围，不能仅凭头文件中存在就认定漏文档。 | **需决策**：确认公开文档范围；CanOptimizeContiguous/CanOptimizeView 为内部接口不补文档；其余若确为公开接口再补文档，不批量补用户 API 文档。 |
| 4 | no-doc | 4 | CanOptimizeContiguous( const op::Shape& viewShape, const op::Strides&  | API_documentation_scope | 需确认公开文档范围，不能仅凭头文件中存在就认定漏文档。 | **需决策**：确认公开文档范围；CanOptimizeContiguous/CanOptimizeView 为内部接口不补文档；其余若确为公开接口再补文档，不批量补用户 API 文档。 |
| 5 | no-doc | 5 | CanOptimizeView(const op::Shape& viewShape, const op::Strides& strides | API_documentation_scope | 需确认公开文档范围，不能仅凭头文件中存在就认定漏文档。 | **需决策**：确认公开文档范围；CanOptimizeContiguous/CanOptimizeView 为内部接口不补文档；其余若确为公开接口再补文档，不批量补用户 API 文档。 |
| 6 | no-doc | 6 | CheckDims(const aclTensor* tensor) | API_documentation_scope | 需确认公开文档范围，不能仅凭头文件中存在就认定漏文档。 | **需决策**：确认公开文档范围；CanOptimizeContiguous/CanOptimizeView 为内部接口不补文档；其余若确为公开接口再补文档，不批量补用户 API 文档。 |
| 7 | no-doc | 7 | CheckReduceOutShape(const aclTensor* inferOut, const aclTensor* out) | API_documentation_scope | 需确认公开文档范围，不能仅凭头文件中存在就认定漏文档。 | **需决策**：确认公开文档范围；CanOptimizeContiguous/CanOptimizeView 为内部接口不补文档；其余若确为公开接口再补文档，不批量补用户 API 文档。 |
| 8 | no-doc | 8 | CheckNCDimValid(const aclTensor* self, const aclTensor* out) | API_documentation_scope | 需确认公开文档范围，不能仅凭头文件中存在就认定漏文档。 | **需决策**：确认公开文档范围；CanOptimizeContiguous/CanOptimizeView 为内部接口不补文档；其余若确为公开接口再补文档，不批量补用户 API 文档。 |
| 9 | parameters | 1 | IsNullptr(const T* param, const char* name) | API_signature_and_version_consistency | 需先对齐CANN版本及头文件来源，再判断文档是否需要同步。 | **需决策**：先对齐 CANN 版本与头文件来源，再判断文档同步或扫描误判；不为匹配文档修改代码签名。 |
| 10 | buildcheck | 1 | ops/ops-ras/scripts/package/ops_ras/scripts/opp_uninstall.sh | script_path_maintainability | 路径变量依赖层数的工程规范问题；该条本身未证明运行失败。 | **规范建议**：将路径变量改为按脚本自身位置推导，去除对目录层数的依赖；低风险。 |
| 11 | buildcheck | 2 | ops/ops-ras/scripts/package/ops_ras/scripts/opp_uninstall.sh | script_path_maintainability | 路径变量依赖层数的工程规范问题；该条本身未证明运行失败。 | **规范建议**：将路径变量改为按脚本自身位置推导，去除对目录层数的依赖；低风险。 |
| 12 | buildcheck | 3 | ops/ops-ras/scripts/kernel/binary_script/build_binary_opc_gen_task.sh | script_error_policy_review | 确实缺少显式set -e，但这不足以证明失败会误报成功；需逐条检查关键子命令、显式退出码及已有容错分支。 | **规范建议**：逐条审计关键子命令与已有容错；仅对确认失败不传播的路径加显式非零返回，正常路径不退化。 |
| 13 | buildcheck | 4 | ops/ops-ras/scripts/kernel/binary_script/build_env.sh | sourced_helper_error_policy_review | 被source的环境/公共函数脚本，不能机械追加set -e，以免改变调用方错误处理；线程数辅助函数会把正常数值放入非零返回码。 | **规范建议**：被 source 的公共脚本不机械加 set -e；保持函数返回码语义，必要时显式处理。 |
| 14 | buildcheck | 5 | ops/ops-ras/scripts/package/ops_ras/scripts/opp_uninstall.sh | script_error_policy_review | 确实缺少显式set -e，但这不足以证明失败会误报成功；需逐条检查关键子命令、显式退出码及已有容错分支。 | **规范建议**：逐条审计关键子命令与已有容错；仅对确认失败不传播的路径加显式非零返回，正常路径不退化。 |
| 15 | buildcheck | 6 | ops/ops-ras/scripts/package/ops_ras/scripts/opp_install.sh | script_error_policy_review | 确实缺少显式set -e，但这不足以证明失败会误报成功；需逐条检查关键子命令、显式退出码及已有容错分支。 | **规范建议**：逐条审计关键子命令与已有容错；仅对确认失败不传播的路径加显式非零返回，正常路径不退化。 |
| 16 | buildcheck | 7 | ops/ops-ras/scripts/util/gen_compile_option.sh | functional_risk_static_evidence | 静态确认call_write_scripts失败未被传播，main末尾固定exit 0；未在本次主机环境执行该CLI，系统Bash为3.2，而脚本含Bash4语法。 | **已静态确认**：call_write_scripts 失败未传播且 main 固定 exit 0；改为传播失败并加回归。 |
| 17 | buildcheck | 8 | ops/ops-ras/scripts/ci/ops_run_repackage.sh | script_error_policy_review | 确实缺少显式set -e，但这不足以证明失败会误报成功；需逐条检查关键子命令、显式退出码及已有容错分支。 | **规范建议**：逐条审计关键子命令与已有容错；仅对确认失败不传播的路径加显式非零返回，正常路径不退化。 |
| 18 | buildcheck | 9 | ops/ops-ras/scripts/util/generate_cpp_cov.sh | script_error_policy_review | 确实缺少显式set -e，但这不足以证明失败会误报成功；需逐条检查关键子命令、显式退出码及已有容错分支。 | **规范建议**：逐条审计关键子命令与已有容错；仅对确认失败不传播的路径加显式非零返回，正常路径不退化。 |
| 19 | buildcheck | 10 | ops/ops-ras/scripts/kernel/binary_script/get_threadnum_with_op.sh | sourced_helper_error_policy_review | 被source的环境/公共函数脚本，不能机械追加set -e，以免改变调用方错误处理；线程数辅助函数会把正常数值放入非零返回码。 | **规范建议**：被 source 的公共脚本不机械加 set -e；保持函数返回码语义，必要时显式处理。 |
| 20 | buildcheck | 11 | ops/ops-ras/scripts/package/ops_ras/scripts/opp_common.sh | sourced_helper_error_policy_review | 被source的环境/公共函数脚本，不能机械追加set -e，以免改变调用方错误处理；线程数辅助函数会把正常数值放入非零返回码。 | **规范建议**：被 source 的公共脚本不机械加 set -e；保持函数返回码语义，必要时显式处理。 |
| 21 | buildcheck | 12 | ops/ops-ras/scripts/util/merge_aicpu_info_json.sh | verified_functional_failure_propagation | 已在主机复现：后续JSON输入不存在时打印ERROR，脚本仍返回0，结果不完整。 | **已修复**：缺输入时 exit 1；before/after 主机验证通过（见 C_SCAN28_FAILURE_PROPAGATION.md）。 |
| 22 | buildcheck | 13 | ops/ops-ras/scripts/kernel/binary_script/build_binary_opc.sh | verified_functional_failure_propagation | 已用受控子进程复现：生成脚本返回7，包装脚本打印ERROR后仍返回0。正常子进程返回0的控制用例通过。 | **已修复**：生成失败时 return $gen_res 传播真实退出码；before/after 主机验证通过（见 C_SCAN28_FAILURE_PROPAGATION.md）。 |
| 23 | buildcheck | 14 | ops/ops-ras/scripts/kernel/binary_script/build_binary_gen_opc_info.sh | script_error_policy_review | 确实缺少显式set -e，但这不足以证明失败会误报成功；需逐条检查关键子命令、显式退出码及已有容错分支。 | **规范建议**：逐条审计关键子命令与已有容错；仅对确认失败不传播的路径加显式非零返回，正常路径不退化。 |
| 24 | buildcheck | 15 | ops/ops-ras/scripts/custom/upgrade.sh | script_error_policy_review | 确实缺少显式set -e，但这不足以证明失败会误报成功；需逐条检查关键子命令、显式退出码及已有容错分支。 | **规范建议**：逐条审计关键子命令与已有容错；仅对确认失败不传播的路径加显式非零返回，正常路径不退化。 |
| 25 | buildcheck | 16 | ops/ops-ras/scripts/package/ops_ras/scripts/install.sh | script_error_policy_review | 确实缺少显式set -e，但这不足以证明失败会误报成功；需逐条检查关键子命令、显式退出码及已有容错分支。 | **规范建议**：逐条审计关键子命令与已有容错；仅对确认失败不传播的路径加显式非零返回，正常路径不退化。 |
| 26 | buildcheck | 18 | ops/ops-ras/scripts/package/ops_ras/scripts/ver_check.sh | script_error_policy_review | 确实缺少显式set -e，但这不足以证明失败会误报成功；需逐条检查关键子命令、显式退出码及已有容错分支。 | **规范建议**：逐条审计关键子命令与已有容错；仅对确认失败不传播的路径加显式非零返回，正常路径不退化。 |
| 27 | buildcheck | 19 | ops/ops-ras/examples/add_example/op_graph/fusion_pass/CMakeLists.txt | CMake_placeholder_scope | 当前目录仅有.gitkeep及只含许可注释的CMakeLists.txt，需按占位目录/扫描范围核实，不直接视为构建错误。 | **需决策**：核实为占位目录（仅 .gitkeep 与许可注释）；不移动文件，必要时调整扫描范围。 |
| 28 | buildcheck | 22 | ops/ops-ras/scripts/package/ops_ras/scripts/uninstall.sh | script_error_policy_review | 确实缺少显式set -e，但这不足以证明失败会误报成功；需逐条检查关键子命令、显式退出码及已有容错分支。 | **规范建议**：逐条审计关键子命令与已有容错；仅对确认失败不传播的路径加显式非零返回，正常路径不退化。 |

## 汇总
- 接口缺文档 8 条：均为公开范围待确认，暂不批量补文档。
- 参数不一致 1 条：版本对齐后再判，不改签名。
- 构建检查 19 条：2 条路径层级（规范修正）、16 条缺 set -e（逐条审计，不机械加）、1 条 CMake 占位目录（不移动）。
- 其中 2 条（merge_aicpu_info_json.sh、build_binary_opc.sh）已在主机复现并**修复**：补丁 `patches/scan28-failure-propagation.patch`，before/after 验证通过（`C_SCAN28_FAILURE_PROPAGATION.md`）。
