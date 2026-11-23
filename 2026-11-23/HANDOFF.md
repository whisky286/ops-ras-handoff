# ops-ras：服务器端 Agent 接手与验证任务

交接日期：2026-11-23，Asia/Shanghai。请先完整阅读本文，再按 A → B → C → D 的顺序推进。项目中实际存在的 AGENTS.md 和服务器使用约束同样需要遵守。

## 1. 目标、授权与本轮交付

用户要求处理 [cann/ops-ras 的全部未关闭 issue](https://gitcode.com/cann/ops-ras/issues)：需要修复的进行修复，已经解决的准备关闭。同时新增三份扫描报告，需要核对真问题、文档差异及误报。

**继续保留用户此前的决定：暂不发表评论、提交/推送 PR、合并或关闭 issue。** 本轮在服务器上进行本地复现、修复、验证和整理证据，交付可审查的补丁及逐项结论。接入 NPU 环境不自动恢复远端写入授权。

先完成可以独立执行的任务；某项缺环境、缺权限或无法复现时，准确记录缺少的条件，继续其他项。不能把缺环境写成测试通过，也不能为了得到全绿结果修改用例或放宽阈值。

本轮至少交付：

1. 环境、源码、补丁与实际加载二进制的身份记录。
2. A/B 阶段结果，以及 C 阶段每个算子问题的复现、候选修复和回归状态。
3. 新增三份 Gist 共 28 条扫描的逐条处置建议。
4. 全部 issue 的后续台账；区分已验证、待验证、已有 PR 待核查、待补充环境与可准备关闭。

## 2. 当前真实状态，避免从错误前提开始

| 项目 | 状态 |
| --- | --- |
| 本地代码基线 | 6943b10042266ee3dbee3e17237919a0ded600bf，2026-11-19 的提交 |
| Mac 工作分支 | codex/ops-ras-issue-fixes；修改未提交，也未推送 |
| issues 快照 | 2026-11-22 共 99 条未关闭，76 条仍在候选 PR 待逐条审查队列 |
| 本地草稿 | 13 条 issue 有相关修改，#140 只有部分草稿；没有全部修复 |
| functional.patch | 22 个文件，主要是构建脚本、文档、两个头文件注释及主机回归 |
| eof.patch | 115 个文件仅追加末行换行，单独保存，不混进功能验证 |
| 已运行 | 8 项 Bash/CMake 主机回归；3 项 parser 既有行为检查；bash -n；git diff --check（cr-at-eol） |
| 尚未运行 | CANN/SDK 构建、真实 Valgrind、NPU 算子验证、完整 pre-commit |
| NPU 相关候选修复 | 不是本地补丁的组成部分；需先核对已有 PR，再单独获取或实现 |

**functional.patch 没有修复 #118 的 FP16 数值问题，也没有包含 !87、!126、!129、!135、!146、!208 等在途 PR 的补丁。** snapshots 中保存的是 PR 元数据和 issue 原文，不是这些 PR 的源码补丁。

上表及 PR 的 open/merged 状态是快照，不能当作服务器接手时的线上状态。候选 PR 关联由文字检索获得，可能包含旁引和旧记录；必须核对真实 diff、目标分支及当前源码。

## 3. 交接包有什么、如何恢复

| 文件 | 用途 |
| --- | --- |
| ops-ras-baseline.bundle | 固定基线及其 Git 历史，可离线克隆 |
| patches/functional.patch | 当前功能/文档草稿与 8 项回归测试 |
| patches/eof.patch | 115 个文件的纯末行换行清理 |
| patches/all.patch | 两类修改的总备份；常规验证不要直接整包应用 |
| snapshots/2026-11-22/status.json | 99 条 issue 的工作状态、说明与候选 PR |
| snapshots/2026-11-22/open-issues.json | issue 完整原文和复现条件 |
| snapshots/2026-11-22/pull-requests.json | 当时 PR 元数据，含来源仓库/分支信息 |
| snapshots/2026-11-22/comments.json | 已有讨论；用于判断重复、误报和责任范围 |
| snapshots/2026-11-23/gist-review.json | 新增 28 条扫描的分类与核查结论 |
| reference/ops-nn/ | 少量相关参考文件，不是完整 ops-nn 仓库 |
| scripts/collect_environment.sh | 只读收集工具、CANN 路径和设备状态 |
| scripts/verify_failure_propagation.py | 主机上复现两处错误未传播及正常控制用例 |
| scripts/verify_parser_scope.py | 复核 parser 必填键与 paramType 的 3 项既有行为 |
| VALIDATION_MATRIX.json | 优先验证项和所需证据 |
| results/RESULT_TEMPLATE.md | 服务器 Agent 的结果填写模板 |
| local-proof/ | 本次在 Mac 验证交接包恢复、补丁和主机脚本的日志；不是服务器/NPU 验证结果 |
| SHA256SUMS | 传输完整性校验 |

该包能离线恢复源码和任务上下文，**不等于可以离线完成 CANN 构建**。CANN SDK、驱动、opbase、cann-cmake、其他第三方依赖仍需服务器或其依赖缓存提供。

在解压目录执行，以下会创建两个新目录，不覆盖现有 checkout：

~~~bash
sha256sum -c SHA256SUMS
git clone -b master ops-ras-baseline.bundle ops-ras-baseline
git clone -b master ops-ras-baseline.bundle ops-ras-candidate
git -C ops-ras-candidate switch -c verify/server-candidate
git -C ops-ras-candidate apply --check ../patches/functional.patch
git -C ops-ras-candidate apply ../patches/functional.patch
~~~

检查两个目录的 HEAD 都为上面的固定提交。候选目录还包含未提交补丁，记录其补丁哈希，不得只写 HEAD 然后声称两个版本相同。

local-proof 中的 correctness-mode-detects-existing-defects 预期退出码为 1，表示校验脚本成功发现草稿尚未修复的两处缺陷；不能把它改写为“功能验证通过”。results 目录留给服务器端填写。

联网且允许只读访问 GitCode 时，在独立 checkout 中添加 upstream、fetch 并核查当前 master/PR。保留固定基线作复现对照；不要对脏工作区直接 git pull。若迁移到新基线，先检查补丁适用性，并记录重放前后 SHA 和实际差异。

## 4. 环境与依赖先确认

从交接根目录运行：

~~~bash
bash scripts/collect_environment.sh ops-ras-candidate results/environment.txt
~~~

记录实际 NPU 型号、可使用设备、驱动、固件、CANN toolkit/OPP、CPU 架构、Bash/Python/CMake/编译器版本，以及 ASCEND_HOME_PATH、ASCEND_OPP_PATH、ASCEND_CUSTOM_OPP_PATH。

已知版本边界：

- 基线 version.cmake 声明 ops-ras 9.2.0，多项构建依赖最低为 8.5；原报告曾因 CANN 8.3.RC1 缺少依赖无法构建。最低版本满足不代表所有 SDK/头文件/库自动兼容，仍需检查实际配套。
- 三份 Gist 的扫描日期为 2026-11-11，接口扫描使用 CANN 9.2.2 的 aarch64 头文件。不要把它与 9.2.0 本地 stub 的差异直接判成当前接口错误。
- 源码固定 opbase 为 32b1c2ce574d2adaced2c3d27e92d4dda56eea85；cann-cmake 为 master-057。部分文档写着更早版本，实际构建以源码配置为准。
- opbase.cmake 会 checkout 依赖源码并删除其部分子目录。使用个人依赖目录，避免把构建路径指向其他任务正在使用的共享 opbase checkout。
- 基线脚本存在 Bash 4 语法。服务器不能因使用 Bash 3.2 而把语法不支持误报为本轮缺陷。

使用服务器真实环境脚本加载 CANN，不凭空假定安装目录。仅使用分配给本任务的设备和个人工作目录；验证不需要升级/卸载驱动或覆盖系统 CANN。需要安装候选算子时，优先个人前缀和独立 vendor，并明确当前 shell 的加载路径。

## A. 先跑不需要 CANN/NPU 的主机检查

### A1. 已有草稿的 8 项回归

~~~bash
cd ops-ras-candidate
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s tests/ut/scripts -p test_build_optional_flags.py -v
bash -n build.sh
git -c core.whitespace=cr-at-eol diff --check
~~~

预期：8 项测试全部通过，不能跳过 CMake 用例后仍写“8 项通过”。测试检查参数解析/配置生成，不代表 SDK 编译或设备行为通过。

回到交接根目录，再运行 python3 scripts/verify_parser_scope.py --repo ops-ras-candidate --output results/parser.json，复核 3 项 parser 既有行为。

重点是 #109：valgrind、bisheng_flags、kernel_template_input。照搬 ops-nn 的 ENABLE_UT_EXEC=FALSE 会让本仓 Host UT 跳过执行，因此草稿同时调整了 tests/ut/op_host/CMakeLists.txt 的执行条件；必须保留相应回归。

#108 采用的是“删除无效常量并说清检查范围”的方案，不是新增 shapeType/needCompile 枚举校验。不要把它描述成非法值已被拒绝。

### A2. 新扫描里两处已经复现的实际缺陷

从交接根目录运行：

~~~bash
python3 scripts/verify_failure_propagation.py --repo ops-ras-baseline --expect baseline --output results/failure-baseline.json
~~~

该模式验证“基线仍呈现已知错误”，返回 0 只表示复现符合预期，不表示代码正确：

- merge_aicpu_info_json.sh：缺少后续 JSON 输入，打印 ERROR，却退出 0，输出不完整；正常两输入控制能合并 A/B。
- build_binary_opc.sh：受控的子生成进程返回 7，包装脚本打印 ERROR 后仍返回 0；正常子进程返回 0 的控制通过。这个用例只测试包装脚本错误传播，不调用真实 OPC/NPU。

先查看 #88 / !160 的真实修改，避免重复 JSON 合并修复。gen_compile_option.sh 的类似风险关联 #93 / !162，已静态确认但本轮尚未执行其完整 CLI。

修复后对候选目录运行同一脚本的 --expect correct 模式；失败用例必须非零，正常控制仍为 0。优先显式传播关键子命令状态，不要靠全局增加 set -e 掩盖问题或破坏错误日志。

## B. 在 CANN 环境做真实构建和 Host UT

先从环境识别 SoC：A2 常用 ascend910b，A3 常用 ascend910_93。只能选实际受支持平台，不把不同代芯片结果混为同一组。

加载正确 CANN 后，可按实际 SoC 先执行目标 Host UT：

~~~bash
bash build.sh -u --ophost --ops=matmul_abft_verify --soc=ascend910b -j8
bash build.sh -u --opapi --ops=crypto,obfuscation_setup,obfuscation_calculate --soc=ascend910b -j8
~~~

以上以 A2 举例；先核对当前 checkout 的 help、目标存在性及 SoC，再替换参数。保存命令、退出码、编译日志、用例名称和实际数量。0 tests、全部 skipped、只有构建成功，都不算单测通过。接口 UT 可能使用 stub，不能替代 C 阶段的设备计算。

对 #109 进一步检查：

1. 普通构建成功，再验证 --bisheng_flags 的合法实际编译参数能传到 OPC。
2. --kernel_template_input 使用目标算子实际支持的模板键；主机回归中的 A=0;B=1 只是参数传递夹具，不能直接拿去编译算子。
3. 安装了 Valgrind 时，验证 --valgrind 真正执行 Host UT，且 --asan/--valgrind 不同顺序均关闭 ASAN，不出现重复普通执行。没有 Valgrind 则记录依赖缺失。
4. 文档/头文件改动不等于实际调用成功。#139/#140 仍缺完整 Excel 附件核验，需有权限账号读取后与已有 PR 协调。

### 运行设备测试前必须确认加载的是谁

默认 build.sh --run_example 可能链接 CANN 已安装的 libopapi_ras.so，而不是刚构建的库；自定义模式使用 cust_opapi 和 vendor 路径。先检查本分支 build_single_example()。

记录实际测试进程加载的 libopapi_ras.so/cust_opapi、ops-math、nnopbase 等库的绝对路径及 SHA256，以及算子 kernel/OPP 或自定义包路径。用 ldd、运行时加载日志、进程 maps 或平台工具确认，不仅检查编译命令。

基线和候选在分开的 shell/个人前缀运行，避免 ASCEND_CUSTOM_OPP_PATH 或旧库缓存污染对照。使用真实 ops-math 依赖，不得把 ras_math_test_double/opapi_math 的测试替身当设备参考实现。

## C. NPU / SDK 重点验证矩阵

这些问题不属于“应用 functional.patch 后就已经修好”。先在固定基线复现，再核对已有 PR、审查或实现独立修复，最后用同一用例做候选回归。

包内尚无可直接运行的 NPU 数值/加密复现程序。服务器 Agent 需要根据 issue 原文和现有样例编写或移植对应 harness，并交付源码、编译命令和输入。不要把主机错误传播脚本当成设备用例，也不要仅凭样例文件名判断 tensor 的实际 dtype。

### C1. 首要设备问题：#118，MatmulAbftVerify FP16 小 K 误报

原文见 snapshots/2026-11-22/open-issues.json 的 number=118。基线条件：

- M=64、N=256，checksumWeight 全 1；A/B 同 dtype，C 为 FLOAT32，权重类型按文档支持矩阵。
- A/B 从 U(-0.5, 0.5) 生成，先舍入到真实输入精度，再用舍入后的值在 CPU 以 FP64 累加计算 C，最后转 FLOAT32。不能用未量化的原始输入算参考 C。
- FLOAT32、FLOAT16、BFLOAT16 三种路径；K 至少覆盖 1、2、3、5、9、17、33、65、129、255、256、257、512。
- 每种至少 3 个固定并记录的随机种子；同一用例基线/候选使用完全一致的输入。
- 先按 issue 复现使用的 eMax 测试：FP32/FP16 为 0.00002，BF16 为 0.001。**FP16 的取值来自报告复现条件，不能宣称它已被当前文档单独推荐。**
- 输出 compRow 按当前实现/文档的 ceil(M/8)×ceil(N/256) 分配；bit=1 表示对应有效行通过，bit=0 表示检测到异常。确认打包顺序，只统计有效行/块，不把 padding bit 当错误。

每次保存：完整 shape/dtype/stride、seed、eMax、phase1/phase2/sync 状态、compRow 原始字节和解码、无故障 bad_count、实际库/内核身份。

先做无故障对照，再做明确的单元素 C 故障注入，验证异常位置确实能被检测。不得用“把 eMax 调大后全为 1”作为修复验收，因为这可能只是失去故障检出能力。

验收需要解释 FP16 的误报来源，并证明同条件无故障误报消除、明确注入故障仍可检出，FP32/BF16 控制不退化。K=1 时同时审查 log(K) 造成的退化值。

原报告表格中 K=256 的 FP16 bad_count=0，后文阈值实验却出现同 K 下 bad_count=64 的描述。先澄清 seed、输入、阈值和运行二进制差别，原样记录矛盾，不复制某一个数字充当自己的结果。

### C2. 其他重点问题

| issue / 候选 PR（旧快照） | 最小验证内容 | 验收边界 |
| --- | --- | --- |
| #52 / !146：Crypto 连续化丢弃 | 连续 AES-CTR 与可信参考对照；分别仅让 key、input、IV、opConfig、output、GCM tag 为 stride=2 的 view；检查可写 view 间隙哨兵 | 输出与连续/可信参考一致，哨兵不被覆盖，状态正常；只测 input 不算覆盖六项 |
| #56 / !129：Matmul dtype/shape | 文档内三组合法 dtype；A/B 不匹配、C rank/shape 错、weight rank/长度错；检查 GetWorkspaceSize / InferShape / Tiling | 不合法参数在阶段一被拒绝；非法输入不要继续送入设备 kernel；合法对照保持成功 |
| #57 / !129：负 eMax | eMax<0、0、正常正值；NaN/Inf 先核对 API 约定，记录当前行为 | 负值按约定返回参数错误；不能删除文档要求就宣称已修；非有限数策略需有依据 |
| #23 / !87：Obfuscation shape | Setup/V2 的 fd=[2] 与合法 fd=[1]；Calculate/V2 同 rank 不同维的 x/y，例如 [2,4]/[1,8]；rank 不同控制 | 四接口非法 shape 应返回 161002；合法控制保持预期。PMCC/TA 服务不可用时区分参数检查与真实执行阻塞 |
| #53 / !126：FP16 mean factor | Host tiling 构造 M=1,N=1,K=20000000、FP16，检查 aMeanFactorBits | 预期 0x0001；先做 metadata/Host 回归，无需先分配巨大设备输入；Host 通过不代表设备 compRow 已验证 |
| #65 / !135：动态 InferShape | A=[64,128]、B=[128,-1]、C=[64,-1]、weight=[-1]；另测未知 M；静态 [64,128]×[128,256] 控制 | 动态输出 [-1]，静态输出 [8]；通过 Host InferShape context 测试，不把 -1 shape 分配为普通设备 tensor |
| #117 / !208：V2 include guard | 同一翻译单元同时包含 V1、V2 头文件，并实际引用两者声明；两个包含顺序 | SDK 编译可见四个接口；不是 NPU 数值测试，不能因为其他旁引 PR merged 就跳过核对 |
| #16：示例扩展支持范围 | 当前真实支持 FLOAT/INT32；先核对文档承诺和 owner 的示例定位 | 未确认 BF16/INT64 是必需能力前，不擅自扩展/宣称缺陷关闭；确认扩展后再补 tiling、对齐和设备用例 |

对于 Crypto，必须先确认真实后端和密钥配置可用；若失败来自权限/密钥服务/算子未安装，记录环境阻塞，不把它当作连续化回归结果。优先复用 issue 中已说明的固定输入与验证方式。

## D. 三份新扫描与其余 issues

新扫描共 28 条，三份完整文本和初步分类已随包提供：

- [接口缺文档：8 条](https://gist.github.com/whisky286/36378b45f489add0c44e48d40ac1779a)。包括 CastOnlyForConvBackward、Contiguous 辅助函数和 Check*；CanOptimizeContiguous/CanOptimizeView 在本地明确标为内部接口。先确认正式公开文档范围、真实 SDK 头文件与 stub 的关系，不批量补用户 API 文档。
- [参数不一致：1 条](https://gist.github.com/whisky286/2ca24bf660321615786f887b127a84f8)。SDK 报 IsNullptr(const T*,const char*)，本地有 const aclTensor* 等多个重载；对齐版本后判断模板泛化、文档未同步或扫描误判，不为匹配文档修改代码签名。
- [构建检查：19 条](https://gist.github.com/whisky286/40930e1198fa4ee068b63d493050b814)。2 条路径变量层级、16 条缺少 set -e、1 条 CMake 目录规则。

缺少 set -e 本身不是所有错误未传播的证明。build_env.sh、get_threadnum_with_op.sh、opp_common.sh 等会被 source；辅助函数还会通过非零返回码传递线程数，不能机械地统一加开关。逐条以失败返回非零、正常路径不退化为标准。

CMake 报警指向 examples/add_example/op_graph/fusion_pass：当前仅 .gitkeep 和只含许可注释的 CMakeLists.txt，是占位目录。需核实规则适用范围，不能未经分析当构建错误移动文件。

参考 ops-nn 的固定快照为 b4ea74bd9c5ea158af7d5685ffa0d56904e3eace；对应 16 个脚本有 15 个也没有显式 set -e。它适合对照共享工程实现，不能当作“与之相同就正确”的标准，也没有可直接照搬的 RAS 专属算子修复。

其余 99 条按 status.json 逐条继续：

- #45/#51/#116 是旧基线已修复、待复核后准备关闭的候选。
- #49 为讨论中已确认的误报；#63 与 #59 重复；#105 与 #64 重复但主问题仍需跟踪。
- #139/#140 完整附件在先前读取时返回 401，不应仅看摘要就写全部处理完成。
- #123/#124/#130 的本地文档草稿与在途 PR 有重叠。#130 的文档清理不得替代 #57 的负 eMax 校验。
- 115 个 EOF 改动单独评审，不与数值/运行时修复混合；更不要为了扫描通过顺手改所有脚本的失败策略。

## 5. 结果记录与最终验收

使用 results/RESULT_TEMPLATE.md；每个问题至少包含：

- issue / 扫描行号、状态：未执行、环境阻塞、基线未复现、已复现、候选修复通过、目标分支已修复待准备关闭。
- 实际命令、基线与候选 SHA、补丁 SHA256、环境、设备、已加载库/内核路径及哈希。
- 预期与实测、输入构造、测试数/失败数/跳过数、stdout/stderr、退出码、原始证据路径。
- 修复范围、已有 PR 是否重叠、尚未验证的边界、下一步需要的具体条件。

复现脚本/测试程序、编译命令、输入种子和故障注入参数要和日志一起保存。管道日志需保留实际测试进程退出码，不能用 tee 的成功退出代替测试结果。

最终交付 results/RESULTS.md、结构化逐项结果和按问题拆分的本地补丁。先明确本轮完成与剩余工作；原有 99 条没有全部验证时，不写“全部已解决”。

**完成验证不等于立即关闭。** 只有确认目标分支已包含修复、相关回归有效、重复/误报有依据，并且用户恢复了远端写入授权后，才执行评论、PR 或关闭操作。
