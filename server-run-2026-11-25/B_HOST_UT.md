# B 阶段: CANN 构建与 Host UT (2026-11-25)

环境: sg144, CANN 9.0.0, SoC=ascend910_93 (Ascend910_9382), g++ 10.3.1
候选: ops-ras-candidate @ 6943b100 + functional.patch(diff sha256 c49d61cd...) + 本机补充的 PR160 移植(见 A 阶段)
构建适配: /home/hww/projects/ops-ras/deps-include + CPLUS_INCLUDE_PATH(见 ENV_NOTES.md), 未改仓库文件

## B1: Host UT matmul_abft_verify
命令: bash build.sh -u --ophost --ops=matmul_abft_verify --soc=ascend910_93 -j8
日志: results/B-build1.log
结果: B1_EXIT=0; [==========] 12 tests from 3 test suites ran. [ PASSED ] 12 tests.
注意: 这是 Host tiling/infershape UT, 不是设备数值验证; 也不能代表 #118 已修复。

## B2: opapi UT crypto / obfuscation_setup / obfuscation_calculate
命令: bash build.sh -u --opapi --ops=crypto,obfuscation_setup,obfuscation_calculate --soc=ascend910_93 -j8
环境注入: CPLUS_INCLUDE_PATH 增加三个算子 op_host/op_api 目录(CMake 的 OPAPI_INCLUDE 未包含)
日志: results/B-build2.log
结果: B2_EXIT=0; [==========] 29 tests from 5 test suites ran. [ PASSED ] 29 tests.

## 尚未完成
- --valgrind 验证: 服务器未安装 Valgrind, 记为依赖缺失(HANDOFF 允许)
- #109 --bisheng_flags / --kernel_template_input 的真实编译参数传递: 待做
- 设备侧: 自定义 OPP 包构建与 #118 复现(进行中, 见 C_*)
