# 服务器环境与构建适配记录 (2026-11-25)

## 环境事实
- 主机: node73-144 (sg144), aarch64, openEuler 22.03 SP4, Linux 5.10
- 登录: l00899010; 代理/CANN 环境来自 `/home/hww/env.sh`
- CANN: /usr/local/Ascend/cann-9.0.0 (ASCEND_HOME_PATH), nnal/atb latest 已装
- 驱动 npu-smi 26.0.rc1; NPU 型号 Ascend910_9382 (torch_npu 报告), 16 die (Phy 0-15, HBM 64G/颗)
- 编译器: g++ 10.3.1; cmake 3.22.0; Python 3.9.9; Bash 5.1.8; Valgrind 未安装
- 设备占用(2026-11-25 查): Phy 0,1 用户自用; 6,7,8,9 有他人 python 进程; 其余空闲
- 本任务设备验证计划使用 Phy 10 (备用 11), 不碰其他卡

## 版本边界(与 HANDOFF 第4节一致)
- ops-ras 基线 version.cmake 声明 9.2.0; 服务器实际 CANN 9.0.0
- opbase 固定 tag 32b1c2ce 的 `op_common/log/log.h` 引用 `graph/error_codes.h`,
  该头在 CANN 9.0.0 中不存在(9.0 的 graphStatus 常量在 `graph/ge_error_codes.h`)

## 构建适配(仅本机环境层, 未修改仓库文件)
1. `/home/hww/projects/ops-ras/deps-include/graph/error_codes.h`
   - 自写 shim(3 行转发 `#include "graph/ge_error_codes.h"`),sha256=3a63fd9848bc2b2fce29f71ba61139c90cff9e58fc2959ef6b7df5841f4e674a
   - 通过 CMakeCache 的 CMAKE_CXX_FLAGS 注入 `-I/home/hww/projects/ops-ras/deps-include`
   - 先试过 metadef master 的 error_codes.h: 与 9.0 ge_error_codes.h 重复定义 ge::GRAPH_* 而编译失败;
     metadef v9.0.1 中该文件不存在, 说明这是 9.2 布局对 9.0 SDK 的依赖缺口
2. crypto/obfuscation 的 op_api UT: CMake 的 OPAPI_INCLUDE 未包含各算子 `op_host/op_api` 目录,
   导致 `aclnn_crypto.h` 等找不到; 通过 CMAKE_CXX_FLAGS 追加三个目录解决(环境层规避)
- 以上两条均为环境层规避, 不计为仓库修复; 结果台账中会单独注明

## 未做 / 禁止
- 未对 gitcode 提交评论/PR/关闭 issue; 未改系统 CANN; 未升级驱动
