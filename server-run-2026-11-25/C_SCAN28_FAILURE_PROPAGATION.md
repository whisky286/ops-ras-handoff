# 28 条扫描项：2 条已复现失败传播的修复（2026-11-27）

## 范围

对应扫描项 buildcheck 12/13（`merge_aicpu_info_json.sh`、`build_binary_opc.sh`），
均为“失败被吞、脚本仍返回 0”的已复现缺陷。工作树 `pr-work/scan28`（基线 `6943b10`）。

## 修复

1. `scripts/util/merge_aicpu_info_json.sh`
   - 后续 JSON 输入不存在时，原来只打印 `[ERROR]` 仍返回 0；改为 `exit 1`。
2. `scripts/kernel/binary_script/build_binary_opc.sh`
   - 生成脚本失败时原来 `return`（返回 0）；改为 `return $gen_res`，传播真实非零码。

补丁：`patches/scan28-failure-propagation.patch`（仅这两个文件，可单独回滚）。

## 验证（主机，无 NPU/OPC 真实执行）

使用交接包 `scripts/verify_failure_propagation.py`：

| 用例 | 修复前 exit | 修复后 exit | 期望 |
|---|---|---|---|
| merge_missing_input | 0（打印 ERROR） | **1** | 非 0 |
| merge_valid_control | 0 | 0 | 0 |
| opc_child_7（受控子进程返回 7） | 0 | **7** | 非 0 |
| opc_child_0（受控子进程返回 0） | 0 | 0 | 0 |

- 修复前 `--expect baseline` 通过（复现缺陷）：`C_scan28-before.json`
- 修复后 `--expect correct` 全部通过：`C_scan28-after.json`（`ALL_PASSED=True`）
- `bash -n` 两文件语法通过。

## 边界

- 仅验证包装脚本的退出码传播；未运行真实 OPC/CANN 构建。
- 未对 GitCode 做任何写操作；未据此关闭 issue。
