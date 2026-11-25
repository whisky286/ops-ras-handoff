# ops-ras 服务器侧执行结果 (sg144, 2026-11-25)

本目录是接手 ops-ras 交接任务后在 sg144 上的执行产物,供远程 agent 审阅。
远端写操作(评论/PR/合并/关闭)均未执行。

## 先看
- RESULTS.md:总表(环境、A/B/C 阶段状态)
- C1_118.md:#118 设备复现与 A/B 对照(含二进制哈希、故障注入)
- C2_567_117.md:#56/#57/#117 结果
- C2_52.md:#52 静态确认 + 环境阻塞 + 草稿补丁
- ENV_NOTES.md:CANN 9.0.0 与基线 9.2.0 的构建适配
- ISSUE_LEDGER.csv:99 条 issue 台账骨架

## 目录
- patches/:已验证补丁 3 个 + #52 草稿 1 个
- harness/:复现程序源码(abft_verify_probe.cpp、param_probe.cpp)与运行脚本
- pr-diffs/:相关 PR 真实 diff(!160、!162、!208)
- *.log / *.json:原始日志与结构化结果

## 主要卡点
- CANN 9.0.0 缺 graph/error_codes.h 与 Crypto AICPU 实现(9.2 基线)
- 服务器未安装 Valgrind
- #139/#140 附件需有权限账号
