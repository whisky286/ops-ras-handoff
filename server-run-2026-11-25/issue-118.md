# 118: [Bug-Report|缺陷反馈]: aclnnMatmulAbftVerify 在 FLOAT16 且 K<255 时把正确结果大面积判为故障（K=3 时 54/64 行误报），FP32/BFLOAT16 无此问题

## 标题

`[Bug-Report|缺陷反馈]: aclnnMatmulAbftVerify 在 FLOAT16 输入且 K<255 时把正确结果大面积判为故障（K=3 时 54/64 行误报），FP32/BFLOAT16 无此问题`

## 问题描述

`aclnnMatmulAbftVerify` 的功能是判定「预先算好的 matmul 结果 C 是否可信」。
在 `checksumWeight` 全 1（文档所述「对应普通的分块行校验和」的标准用法）、
`C` 由主机以 fp64 累加精确算出的**无故障输入**上，FLOAT16 路径会把大量正确的行
判定为「检测到错误」，而 FP32 与 BFLOAT16 路径在同一批输入上完全正常。

误报随归约维 K 单调恶化：K=3/5 时超过 80% 的行被判错。全程 `phase1=0, phase2=0, sync=0`，
无任何报错，属于**静默地给出错误的安全结论**。

## 环境信息

- 仓库：`cann/ops-ras`，`version.cmake` → `set_cann_package(ops-ras VERSION "9.2.0")`
- 芯片：Atlas A2 训练服务器 910B3（`npu-smi 26.1.1`，firmware 7.8.0.6.201）
- CANN：9.2.0
- 内核：`opp/built-in/op_impl/ai_core/tbe/kernel/ascend910b/ops_ras/matmul_abft_verify/`
  （`MatmulAbftVerify_4c2b041a18632a7ff6cc68ad7df896fb_high_performance.o` 已安装）
- 编译器：g++ (GCC) 10.3.1，`-std=c++17`

## 重现步骤

1. 取 `M=64, N=256`，`checksumWeight` 全 1（长度 N，dtype 与 A/B 一致）。
2. A、B 用均匀分布 U(−0.5, 0.5) 生成，并按 A/B 的精度（FLOAT16 / BFLOAT16 / FLOAT32）
   做一次 round-to-nearest 到目标精度。
3. `C = A × B` 在**主机 fp64** 累加后转 float32 写入 device（即完全无故障的 C）。
4. `eMax` 取文档推荐值：FLOAT32 / FLOAT16 → `0.00002`，BFLOAT16 → `0.001`
   （`docs/aclnnMatmulAbftVerify.md` 的 eMax 使用说明）。
5. `compRow` 长度按文档 `ceil(M/8) * ceil(N/256)` 分配。
6. 调用 `aclnnMatmulAbftVerifyGetWorkspaceSize` → `aclnnMatmulAbftVerify` → `aclrtSynchronizeStream`。
7. 按文档语义解码：**bit=1 表示该行正确，bit=0 表示检测到错误**；统计 bit=0 的行数。
8. 对 K ∈ {1,2,3,5,9,17,33,65,129,255,256,257,512} 重复，且三种精度并行对照。

## 预期结果

C 完全正确（由 fp64 精确计算），所有行都应判为正确：`bad = 0`。

## 实际结果

`bad` = 被判为「检测到错误」的行数（M=64，共 64 行）：

| K | FLOAT32 | **FLOAT16** | BFLOAT16 |
|---:|---:|---:|---:|
| 1 | 0 | **2** | 1 |
| 2 | 0 | 0 | 0 |
| 3 | 0 | **54** | 0 |
| 5 | 0 | **56** | 0 |
| 9 | 0 | **45** | 0 |
| 17 | 0 | **32** | 0 |
| 33 | 0 | **13** | 0 |
| 65 | 0 | **8** | 0 |
| 129 | 0 | **5** | 0 |
| 255 | 0 | 0 | 0 |
| 256 | 0 | 0 | 0 |
| 257 | 0 | 0 | 0 |
| 512 | 0 | 0 | 0 |

每次调用均 `phase1=0, phase2=0, sync=0`，无错误码。

## 关键对照（用于排除构造错误）

1. **FP32 与 BFLOAT16 在全部 13 个 K 上均为 0 误报** —— 同一套主机侧构造、
   同一套 Tiling 推导、同一个 kernel 二进制，只有 A/B dtype 不同。
   因此可排除「标杆 C 算错」「compRow 解码错位」「workspace 申请不足」等共同因素。
2. **FLOAT16 与 BFLOAT16 位宽相同、精度接近**，BFLOAT16 用文档推荐 eMax=0.001 完全干净，
   FLOAT16 用推荐 eMax=0.00002 却误报 80% —— 说明 FLOAT16 的阈值估计路径本身有问题，
   而不是「fp16 精度不够」这种预期内现象。
3. **与随机种子无关**：LCG 与 mt19937 两种生成器、各 3 个种子，FLOAT16 结果一致。
4. **与 C 的累加精度无关**：把 C 从 fp64 累加改为 fp32 累加，FLOAT16 的 `bad` 完全不变，
   排除「C 精度不足导致误判」这一解释。
5. **放宽 eMax 可以让误报消失，但需要放大约 5 个数量级**：
   在 M=64,N=256,K=256 下，eMax `2e-05 → 64/64`、`1e-03 → 64/64`、
   `1e-02 → 59/64`、`1e-01 → 3/64`、`1.0 → 0/64`。
   这本身也说明阈值估计对 FLOAT16 偏离极大。

## 静态线索

`reliability/matmul_abft_verify/op_host/matmul_abft_verify_tiling.cpp`

- `:145` `tiling.eMax = eMax * 6 * std::sqrt(static_cast<float>(k) / 1024.0f);`
  —— 阈值随 sqrt(K) 缩放，K 越小阈值越小，与观测到的「K 越小误报越严重」方向一致。
- `:146` `tiling.stdEstARowRatio = 1.0f / std::sqrt(2.0f * std::log(static_cast<float>(k)));`
  —— `std::log(k)` 在 K=1 时为 0，`1/0` 得 inf；K=2 时 `log(2)` 很小，
  比值被显著放大。K=1 时 FLOAT16 出现 2/64 误报、BFLOAT16 出现 1/64，
  而 K=2 时两者都为 0，与该式在 K=1 处退化、K=2 处偏向另一侧的行为吻合。
- `:118-128` 的 `stdEstRatios` / `knRatios` 一族常量按 `L1TileShape_N=256` 与
  `nRemainSplit` 分支计算，当 `nRemainSplit == 0`（N 恰为 256 的整数倍）时
  走「无剩余块」分支；本组用例 N=256 正好命中该分支，
  而 FLOAT16 的误报恰在此分支下最严重。
- 同一文件 `:238-245` 把 `1.0f/K` 转成目标精度写入 `aMeanFactorBits`：
  FLOAT16 用 `FloatToFp16Bits`，FLOAT32 用 `FloatBits`，BFLOAT16 用 `FloatToBf16Bits`。
  K 较大时 `1/K` 在 FP16 下接近下溢（相关已提 issue #53 讨论 K=2e7 时round 到 0），
  K 较小时 `1/K` 本身无下溢风险，因此本缺陷与 #53 的机制不同：
  **#53 是 K 极大时的下溢，#本 issue 是 K 较小时的阈值/统计量偏斜。**

上述为静态线索，用于指向修复方向；本 issue 的结论以第 3–5 节的真机对照为准。

## 影响与规避方式

ABFT 的唯一职责是判定 matmul 结果可否信。FLOAT16 是推理主用精度，
decode 阶段小 batch / 短序列场景下 K 往往远小于 255 —— 此时每次校验都会把
绝大多数正常行判成故障：

- 调用方按 `compRow` 触发重算 → 白白付出重算开销；
- 调用方据此上报 RAS 告警 → 告警风暴，真实故障被淹没。

规避：当前只能调大 eMax（需放大约 5 个数量级），但这会同步降低真实故障的检出率，
等于用检出能力换误报率，不能作为调用方侧的常规规避手段。

## 建议修复

1. 按 dtype 分别标定阈值尺度，FLOAT16 不应直接复用 FLOAT32 的 eMax 推荐值；
   或者文档补上 FLOAT16 的推荐值并明确其与 FP32 不在同一量级。
2. 检查 `stdEstARowRatio`、`stdEstRatios`、`knRatios` 在 `nRemainSplit == 0`
   与 K < 256 两个退化分支下的取值，确认除数与被除数没有跨精度混用。
3. 建议在仓内加一条主机可跑的回归：三种精度 × K ∈ {3,5,65,255} 全 1 权重的
   **无误报** 基线（`bad == 0`），这是不需要设备也能拦住本类回归的最低门槛。

## 备注

本 issue 为**新增缺陷**。提交前已检索本仓全部 issue，含关键词
`MatmulAbftVerify`（9 条）、`eMax`（2 条）、`ABFT`（9 条）。
已有的相关 issue 与本条的区别：

- **#53**「FP16 mean factor can round to zero」：K=2e7 时 `1/K` 下溢到 0，机制是极大 K 的下溢。
  本条是 K<255 时的阈值偏斜，方向相反，且已用 fp64 精确 C 复现。
- **#57**「接受负 eMax」：参数校验缺失，属错误码契约问题；本条是校验通过后的计算结果错误。
- **#56**「未校验 B/C/weight 输入类型与 shape 合同」：本条所有输入均**完全合法**
  （A/B 同 dtype、C=[M,N]、weight=[N]，rank 与 shape 全对），问题在合法输入上算错。
- **#65**「InferShape 动态输出长度」：本条是静态 shape，不涉及动态推导。

本条与本轮同批提交的另一条 issue（V2 头文件 include guard 重名）无关联。