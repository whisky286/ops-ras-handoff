# #109 / !233 `--kernel_template_input` 真实传参验证（2026-11-27）

## 结论

`--kernel_template_input` 已用**真实模板键**验证可透传到 `asc_opc` 并成功编译，非仅参数解析。

- 算子：`matmul_abft_verify`
- 真实模板键：`schMode=0`
  （来源 `op_kernel/matmul_abft_verify_tiling_key.h` 的
  `ASCENDC_TPL_ARGS_DECL(MatmulAbftVerify, ASCENDC_TPL_UINT_DECL(schMode, 1, ..., 0))`）
- 工作树：`pr-work/pr233`（!233 head 的本地固定 diff）
- 命令：
  ```bash
  . /home/hww/env.sh
  export CPLUS_INCLUDE_PATH=/home/hww/projects/ops-ras/deps-include
  cd /home/hww/projects/ops-ras/pr-work/pr233
  bash build.sh --ops=matmul_abft_verify --soc=ascend910_93 \
       --kernel_template_input="schMode=0" -j8
  ```
- 结果：`TPL_EXIT=0`；`[SUCCESS] Build binary success!`

## 证据

编译日志 `C109-template.log`：
- `kernel_template_input is: schMode=0`（3 个 config 各一次）
- `asc_opc ... --kernel-template-input=schMode=0`（任务生成与真实执行命令均含该参数）
- `[SUCCESS] Build binary success!`

## 边界

- 仅验证传参与真实模板键编译成功；未据此关闭 #109。
- `--valgrind` 执行仍缺（服务器未安装 Valgrind），单独记录。
- 未对 GitCode 做任何写操作。
