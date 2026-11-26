# 服务器 Agent 从这里接手

先阅读本目录 README.md、ISSUE_TRIAGE.csv、SERVER_QUEUE.md、PATCH_INDEX.json。全量分类由本地 Agent 维护；不要重新给全部 issue 跑一轮 NPU 测试。

1. 优先完成 #53/!126 和 #65/!135 的小型 Host 测试，以及 #23/!87 的阶段一参数检查。
2. #118 已有 !235 的具体 kernel/checksum 修复候选，先用 patches/pr-235.patch 在独立基线验证，勿混入未采纳的 kmin 实验。
3. #109/!233 需固定该 PR head 补参数传递证据。#52/#16 先确认环境/支持范围，阻塞时继续其他项。
4. 复用 #56/#57 / !129、#117 等已有证据；#150/#151/#154 本地已有真实主机对照，见 HOST_PR_REVIEW.json，无需再占用 NPU。
   #81/#83/#84 的现有 PR 回归也已在本地主机复核，见 BUILD_SCRIPT_REVIEW.json 与相应日志。
5. 每个候选独立应用，校验 PATCH_INDEX.json 的 SHA256 与 head。不要一次叠加所有补丁；!87/!146 含共享 UT/CMake 变化需审查范围。
6. 将后续结果追加到交接仓库独立结果目录，写明 issue/PR/head、基线/候选、预期/实测、证据和剩余条件；不要覆盖原日志。
7. GitCode 评论、推送 PR、合并及关闭仍暂缓。

完整 102 条是本次刷新结果；出现新增 issue 或 PR head 变化时，由本地 Agent 更新全量台账。
