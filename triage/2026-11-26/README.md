# ops-ras 全量 issue 分类

分类日期：2026-11-26（Asia/Shanghai）。已联网刷新 GitCode 全部 open issue、所有状态 PR 和 master；清单为 **102 条**，不是旧快照的 99 条。master 仍为 `6943b10042266ee3dbee3e17237919a0ded600bf`。服务器报告使用 GitHub `36847ff`。

这是全量的**关联与状态初筛**：每条均有分类、理由、已有 PR 和下一步；不代表已审完所有 PR diff、重新跑完 CI，或全部修复通过。分类不会因为缺 NPU 而停下。

## 谁来做

- 本地 Agent（本对话）：维护全量清单，核对当前 master、PR 对应关系、重复/误报、文档/脚本范围，整理可关闭说明及后续任务。
- 服务器 Agent：只接收需要 SDK/Host UT/设备的具体任务；复用已有环境和测试工具，不为每个 PR 重建一套验证。
- 维护者：评审和合入；用户恢复授权后才执行 GitCode 评论/关闭等远端动作。

## 分类结果

| 分类 | 数量 | 含义 |
| --- | --- | --- |
| 可准备关闭 | 5 | 问题现状已消失，或有明确重复/误报依据；只准备说明，尚不关闭 |
| 已有PR，待范围审查/合入 | 88 | 已有明确对应的开放 PR；按需审查范围和已有测试，等合入；不是全部已验收 |
| 已有PR，需补针对性验证 | 5 | 有候选 PR，但缺关键的最小验证；按任务补证，不重新从零实现 |
| 缺条件或范围待确认 | 4 | 缺附件/匹配后端或支持范围尚需确认；先记录条件，继续其他项 |

## 当前可以准备关闭的 5 条

- #45：Simulator 链接和锚点当前有效，!112 已合入，与 #42 重复。
- #51：中英文 Matmul API 链接已指向存在的文档；!125 仍 open 不改变 master 当前已正确的事实。
- #49：讨论已明确确认为误报（混淆内置包与自定义包安装器），!123 已关闭。
- #63：与 #59 重复，主问题仍由 #59 / !139 跟踪。
- #105：与 #64 重复，主问题仍由 #64 / !137 跟踪；不能把重复关闭描述成主问题修复。

其中链接现状见 evidence/closure-evidence.json；讨论见 evidence/closure-evidence.json。关闭文案见 CLOSURE_DRAFTS.md。未执行任何 GitCode 远端写入。

## 先改变的工作顺序

1. 不让 #118 阻塞全量分类；本次 102 条已全部写入台账，不再留空。
2. 先审查已有 PR 的真实范围、目标分支、对应测试和当前状态。足够的现有证据可以复用，不强制逐 PR 全量复测。
3. 已有 PR 未合入，issue 保持 open；本地验证通过只是一项证据，不等于原仓已修复。
4. 同一根因/同一 PR 覆盖多个 issue，只建立一份验证证据。例如 !129 对应 #56/#57/#58；!160 对应 #87/#88。
5. 需要服务器的最小任务见 SERVER_QUEUE.md。除列明事项外，本地先做关联/文档/脚本检查，不默认送到 NPU。

## 刚发现的新增修复

- #118 已有 !235：FP16 checksum 改为 FP32，并包含 K=1 保护。该 PR 的作者尚未设备验证，应优先测试这个现有候选，避免持续进行没有候选实现依据的阈值试探。
- #16 已有 !236，属于 INT64/BF16 示例能力扩展，需确认支持范围及 SoC 后验证。
- #107/#108/#109 和多条文档反馈已有 !223–!234；不要把旧本地草稿直接认作这些新 PR 的测试证据。
- 新增 #152/#153 各有 !237/!238；#152 与 #44 / !223 同一量化链接修正有重叠。
- 新扫描的 19 条构建检查已有 master PR !243，!242 的目标是 9.3.0。不能把 9.3.0 的处理当成 master 已修；批量 set -e 的行为影响仍需有针对性证据，不凭 bash -n 判定功能通过。

## 文件

- ISSUE_TRIAGE.csv：102 条可读表格，含下一步、负责方、最小验证与当前 PR。
- ISSUE_TRIAGE.json：同一清单的结构化完整数据，包含 PR head SHA。
- PR_GROUPS.csv：以 PR 为单位合并对应 issue，避免重复验证。
- SERVER_QUEUE.md：交给服务器 Agent 的有限任务集。
- CLOSURE_DRAFTS.md：5 条可关闭候选的待审查文案。
- PROGRESS.json：补丁审查后的逐条推进记录，生成台账时保留，不把新进展覆盖回初筛状态。
- DOC_PR_REVIEW.json：首批 4 个文档 PR 的实际补丁、链接/锚点、帮助输出及重叠验证。
- MAINTENANCE.md：分类之后的处理责任和完成标准。
- SOURCE_STATE.json：联网来源、目标分支和获取时间。原始 API 响应仅保留本地，不上传整包账户/作者元数据；关闭依据摘录在 evidence/closure-evidence.json。

未获取/复核完整附件的 #139/#140 不宣称全部覆盖；PR open 和 master 状态均为本次获取时的状态。后续合入或新增 issue 后需刷新。

## 已有PR，待范围审查/合入

| Issue | 已有 PR | 理由 / 下一步 |
| --- | --- | --- |
| [#154](https://gitcode.com/cann/ops-ras/issues/154) | [!244](https://gitcode.com/cann/ops-ras/merge_requests/244) | 新增 #154 / !244 已验证：代表性组合输入在 12 个独立 Python 哈希种子进程中，原版产生 5 种顺序，候选均为同一有序列表；集合成员未变。 |
| [#153](https://gitcode.com/cann/ops-ras/issues/153) | [!238](https://gitcode.com/cann/ops-ras/merge_requests/238) | 已取得 !238 真实 diff；build.sh 的 usage 函数之外字节内容未变，bash -n 通过，执行真实 usage 函数的 7 种帮助模式均显示 CPU 数/上限 32；双语说明同步，未运行完整 Linux CLI 或 CANN 构建。 |
| [#152](https://gitcode.com/cann/ops-ras/issues/152) | [!237](https://gitcode.com/cann/ops-ras/merge_requests/237) | !237 和 !223 均修复同一量化链接，分别应用后 basic_concept 的 12 个相对链接目标均存在；两补丁不能原样依次应用。四空格列表渲染问题仍由 #129 / !201 单独跟踪。 |
| [#151](https://gitcode.com/cann/ops-ras/issues/151) | [!220](https://gitcode.com/cann/ops-ras/merge_requests/220) | !220 固定 head 经 4 次真实 CLI 验证：无 tag、短 tag abc/a_b、正常 tag；原版 2 项时间戳格式不符合预期，候选 0 项；正常 tag 输出保持。未覆盖所有三段以上但内容为空的 tag。 |
| [#150](https://gitcode.com/cann/ops-ras/issues/150) | [!219](https://gitcode.com/cann/ops-ras/merge_requests/219) | !219 固定 head 的真实补丁已在独立基线验证：9 个版本比较用例中原版 2 个错误相等判断，候选 0 个失败；包括长短版本互换、补零相等及范围比较控制。 |
| [#149](https://gitcode.com/cann/ops-ras/issues/149) | [!218](https://gitcode.com/cann/ops-ras/merge_requests/218) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#148](https://gitcode.com/cann/ops-ras/issues/148) | [!217](https://gitcode.com/cann/ops-ras/merge_requests/217) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#147](https://gitcode.com/cann/ops-ras/issues/147) | [!216](https://gitcode.com/cann/ops-ras/merge_requests/216) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#146](https://gitcode.com/cann/ops-ras/issues/146) | [!224](https://gitcode.com/cann/ops-ras/merge_requests/224) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#145](https://gitcode.com/cann/ops-ras/issues/145) | [!225](https://gitcode.com/cann/ops-ras/merge_requests/225) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#144](https://gitcode.com/cann/ops-ras/issues/144) | [!230](https://gitcode.com/cann/ops-ras/merge_requests/230) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#143](https://gitcode.com/cann/ops-ras/issues/143) | [!229](https://gitcode.com/cann/ops-ras/merge_requests/229) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#142](https://gitcode.com/cann/ops-ras/issues/142) | [!228](https://gitcode.com/cann/ops-ras/merge_requests/228) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#141](https://gitcode.com/cann/ops-ras/issues/141) | [!231](https://gitcode.com/cann/ops-ras/merge_requests/231) | 新增 !231；需核对是否覆盖全部扫描条目，并协调 !205；不能仅凭改了错误码表就声明全部扫描完成。 |
| [#138](https://gitcode.com/cann/ops-ras/issues/138) | [!215](https://gitcode.com/cann/ops-ras/merge_requests/215) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#137](https://gitcode.com/cann/ops-ras/issues/137) | [!211](https://gitcode.com/cann/ops-ras/merge_requests/211) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#135](https://gitcode.com/cann/ops-ras/issues/135) | [!196](https://gitcode.com/cann/ops-ras/merge_requests/196) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#134](https://gitcode.com/cann/ops-ras/issues/134) | [!195](https://gitcode.com/cann/ops-ras/merge_requests/195) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#133](https://gitcode.com/cann/ops-ras/issues/133) | [!194](https://gitcode.com/cann/ops-ras/merge_requests/194) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#132](https://gitcode.com/cann/ops-ras/issues/132) | [!197](https://gitcode.com/cann/ops-ras/merge_requests/197) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#131](https://gitcode.com/cann/ops-ras/issues/131) | [!202](https://gitcode.com/cann/ops-ras/merge_requests/202) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#130](https://gitcode.com/cann/ops-ras/issues/130) | [!205](https://gitcode.com/cann/ops-ras/merge_requests/205) | 已有 !205，与 #141 / !231 重叠；删除负 eMax 文档不能代替 #57 的实现修复，应与 !129 的实际合同同步。 |
| [#129](https://gitcode.com/cann/ops-ras/issues/129) | [!201](https://gitcode.com/cann/ops-ras/merge_requests/201) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#128](https://gitcode.com/cann/ops-ras/issues/128) | [!206](https://gitcode.com/cann/ops-ras/merge_requests/206) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#127](https://gitcode.com/cann/ops-ras/issues/127) | [!200](https://gitcode.com/cann/ops-ras/merge_requests/200) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#126](https://gitcode.com/cann/ops-ras/issues/126) | [!199](https://gitcode.com/cann/ops-ras/merge_requests/199) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#125](https://gitcode.com/cann/ops-ras/issues/125) | [!204](https://gitcode.com/cann/ops-ras/merge_requests/204) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#124](https://gitcode.com/cann/ops-ras/issues/124) | [!207](https://gitcode.com/cann/ops-ras/merge_requests/207) | 已有 !207；与 #142/#140 的 Setup 文档修正重叠；统一资源释放 cmd=16 的文档和头文件注释。 |
| [#123](https://gitcode.com/cann/ops-ras/issues/123) | [!198](https://gitcode.com/cann/ops-ras/merge_requests/198) | 已有 !198；与 #140 / !227 的 obfCoefficient 类型修正重叠，统一核对，避免重复提交。 |
| [#122](https://gitcode.com/cann/ops-ras/issues/122) | [!203](https://gitcode.com/cann/ops-ras/merge_requests/203) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#117](https://gitcode.com/cann/ops-ras/issues/117) | [!208](https://gitcode.com/cann/ops-ras/merge_requests/208) | 已有 !208；本地 guard 修复四种包含顺序通过，完整 !208 其余部分尚未整包验证；不混入旁引的 !118（它是 EOF 清理且目标为 9.2.0）。 |
| [#116](https://gitcode.com/cann/ops-ras/issues/116) | [!188](https://gitcode.com/cann/ops-ras/merge_requests/188) | 已取得 !188 固定 head 的真实 diff；补丁同时为中英文链接补齐章节 fragment，文件和对应标题均存在，临时基线应用与 diff check 通过；master 尚未包含该变更。 |
| [#115](https://gitcode.com/cann/ops-ras/issues/115) | [!187](https://gitcode.com/cann/ops-ras/merge_requests/187) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#114](https://gitcode.com/cann/ops-ras/issues/114) | [!186](https://gitcode.com/cann/ops-ras/merge_requests/186) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#111](https://gitcode.com/cann/ops-ras/issues/111) | [!181](https://gitcode.com/cann/ops-ras/merge_requests/181) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#110](https://gitcode.com/cann/ops-ras/issues/110) | [!180](https://gitcode.com/cann/ops-ras/merge_requests/180) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#108](https://gitcode.com/cann/ops-ras/issues/108) | [!232](https://gitcode.com/cann/ops-ras/merge_requests/232) | 新增 !232 采用删除死常量并明确解析范围；不等于实现 shapeType/needCompile 枚举校验。 |
| [#107](https://gitcode.com/cann/ops-ras/issues/107) | [!234](https://gitcode.com/cann/ops-ras/merge_requests/234) | 新增 !234 是 EOF 清理；与本地 115 文件草稿范围不同，按真实 diff 和格式门禁确认，无需 NPU。 |
| [#104](https://gitcode.com/cann/ops-ras/issues/104) | [!179](https://gitcode.com/cann/ops-ras/merge_requests/179) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#103](https://gitcode.com/cann/ops-ras/issues/103) | [!168](https://gitcode.com/cann/ops-ras/merge_requests/168) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#102](https://gitcode.com/cann/ops-ras/issues/102) | [!167](https://gitcode.com/cann/ops-ras/merge_requests/167) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#101](https://gitcode.com/cann/ops-ras/issues/101) | [!167](https://gitcode.com/cann/ops-ras/merge_requests/167) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#100](https://gitcode.com/cann/ops-ras/issues/100) | [!166](https://gitcode.com/cann/ops-ras/merge_requests/166) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#99](https://gitcode.com/cann/ops-ras/issues/99) | [!166](https://gitcode.com/cann/ops-ras/merge_requests/166) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#98](https://gitcode.com/cann/ops-ras/issues/98) | [!165](https://gitcode.com/cann/ops-ras/merge_requests/165) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#97](https://gitcode.com/cann/ops-ras/issues/97) | [!165](https://gitcode.com/cann/ops-ras/merge_requests/165) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#96](https://gitcode.com/cann/ops-ras/issues/96) | [!164](https://gitcode.com/cann/ops-ras/merge_requests/164) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#95](https://gitcode.com/cann/ops-ras/issues/95) | [!163](https://gitcode.com/cann/ops-ras/merge_requests/163) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#94](https://gitcode.com/cann/ops-ras/issues/94) | [!163](https://gitcode.com/cann/ops-ras/merge_requests/163) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#93](https://gitcode.com/cann/ops-ras/issues/93) | [!162](https://gitcode.com/cann/ops-ras/merge_requests/162) | 已有 !162，目前仅静态核对；与 #92 同组，按路径引用和失败传播的主机回归审查，不需要 NPU。 |
| [#92](https://gitcode.com/cann/ops-ras/issues/92) | [!162](https://gitcode.com/cann/ops-ras/merge_requests/162) | 与 #93 同属 !162，按同一变更和主机回归合并审查。 |
| [#91](https://gitcode.com/cann/ops-ras/issues/91) | [!161](https://gitcode.com/cann/ops-ras/merge_requests/161) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#90](https://gitcode.com/cann/ops-ras/issues/90) | [!161](https://gitcode.com/cann/ops-ras/merge_requests/161) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#89](https://gitcode.com/cann/ops-ras/issues/89) | [!161](https://gitcode.com/cann/ops-ras/merge_requests/161) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#88](https://gitcode.com/cann/ops-ras/issues/88) | [!160](https://gitcode.com/cann/ops-ras/merge_requests/160) | 已有 !160；本地移植的错误传播对照通过，不能把该有限证据扩写为所有路径/全部 PR head 已验证。 |
| [#87](https://gitcode.com/cann/ops-ras/issues/87) | [!160](https://gitcode.com/cann/ops-ras/merge_requests/160) | 与 #88 同属 !160；优先复用已有带空格路径真实脚本回归，不能当成另一份需要 NPU 的任务。 |
| [#86](https://gitcode.com/cann/ops-ras/issues/86) | [!159](https://gitcode.com/cann/ops-ras/merge_requests/159) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#85](https://gitcode.com/cann/ops-ras/issues/85) | [!158](https://gitcode.com/cann/ops-ras/merge_requests/158) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#84](https://gitcode.com/cann/ops-ras/issues/84) | [!157](https://gitcode.com/cann/ops-ras/merge_requests/157) | !157 的现有 4 个真实 CLI 回归已运行：原版缩短/清空 CSV 残留旧内容，2 项失败；候选 4 项通过，重复生成、多输入和动态筛选控制通过。 |
| [#83](https://gitcode.com/cann/ops-ras/issues/83) | [!156](https://gitcode.com/cann/ops-ras/merge_requests/156) | !156 的现有 3 个真实 CLI 回归已运行：原版缺输入仍返回成功导致 1 个失败，候选 3 个方法全部通过；成功输出、空列表和输入不变控制保留。 |
| [#82](https://gitcode.com/cann/ops-ras/issues/82) | [!155](https://gitcode.com/cann/ops-ras/merge_requests/155) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#81](https://gitcode.com/cann/ops-ras/issues/81) | [!154](https://gitcode.com/cann/ops-ras/merge_requests/154) | !154 的现有回归在固定基线复核：原版空任务场景有 5 处子测试失败，候选 3 个测试方法全部通过，覆盖空/缺省列表、归一化分片数、非空顺序与元数据；未运行 OPC/CANN/NPU。 |
| [#80](https://gitcode.com/cann/ops-ras/issues/80) | [!153](https://gitcode.com/cann/ops-ras/merge_requests/153) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#79](https://gitcode.com/cann/ops-ras/issues/79) | [!153](https://gitcode.com/cann/ops-ras/merge_requests/153) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#78](https://gitcode.com/cann/ops-ras/issues/78) | [!152](https://gitcode.com/cann/ops-ras/merge_requests/152) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#77](https://gitcode.com/cann/ops-ras/issues/77) | [!151](https://gitcode.com/cann/ops-ras/merge_requests/151) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#76](https://gitcode.com/cann/ops-ras/issues/76) | [!151](https://gitcode.com/cann/ops-ras/merge_requests/151) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#75](https://gitcode.com/cann/ops-ras/issues/75) | [!150](https://gitcode.com/cann/ops-ras/merge_requests/150) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#74](https://gitcode.com/cann/ops-ras/issues/74) | [!150](https://gitcode.com/cann/ops-ras/merge_requests/150) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#73](https://gitcode.com/cann/ops-ras/issues/73) | [!149](https://gitcode.com/cann/ops-ras/merge_requests/149) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#72](https://gitcode.com/cann/ops-ras/issues/72) | [!149](https://gitcode.com/cann/ops-ras/merge_requests/149) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#71](https://gitcode.com/cann/ops-ras/issues/71) | [!148](https://gitcode.com/cann/ops-ras/merge_requests/148) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#70](https://gitcode.com/cann/ops-ras/issues/70) | [!148](https://gitcode.com/cann/ops-ras/merge_requests/148) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#69](https://gitcode.com/cann/ops-ras/issues/69) | [!148](https://gitcode.com/cann/ops-ras/merge_requests/148) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#68](https://gitcode.com/cann/ops-ras/issues/68) | [!145](https://gitcode.com/cann/ops-ras/merge_requests/145) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#67](https://gitcode.com/cann/ops-ras/issues/67) | [!143](https://gitcode.com/cann/ops-ras/merge_requests/143) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#66](https://gitcode.com/cann/ops-ras/issues/66) | [!142](https://gitcode.com/cann/ops-ras/merge_requests/142) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#64](https://gitcode.com/cann/ops-ras/issues/64) | [!137](https://gitcode.com/cann/ops-ras/merge_requests/137) | 已有 !137；与重复项 #105 统一跟踪，另与 !141 的图片命名规范方案协调。 |
| [#62](https://gitcode.com/cann/ops-ras/issues/62) | [!134](https://gitcode.com/cann/ops-ras/merge_requests/134) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#61](https://gitcode.com/cann/ops-ras/issues/61) | [!133](https://gitcode.com/cann/ops-ras/merge_requests/133) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#60](https://gitcode.com/cann/ops-ras/issues/60) | [!136](https://gitcode.com/cann/ops-ras/merge_requests/136) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#59](https://gitcode.com/cann/ops-ras/issues/59) | [!139](https://gitcode.com/cann/ops-ras/merge_requests/139) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#58](https://gitcode.com/cann/ops-ras/issues/58) | [!129](https://gitcode.com/cann/ops-ras/merge_requests/129), [!131](https://gitcode.com/cann/ops-ras/merge_requests/131) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#57](https://gitcode.com/cann/ops-ras/issues/57) | [!129](https://gitcode.com/cann/ops-ras/merge_requests/129) | 已有 !129，负值/NaN/Inf 拒绝及 eMax=0 合法已在后续报告说明；与 #56 合并验证，不再为相同 PR 重复建环境。 |
| [#56](https://gitcode.com/cann/ops-ras/issues/56) | [!129](https://gitcode.com/cann/ops-ras/merge_requests/129) | 已有 !129，服务器报告已完成本地阶段一修复对照及 Host UT；修复未进入 master，保持 open，复用对应源码/补丁身份的证据。 |
| [#44](https://gitcode.com/cann/ops-ras/issues/44) | [!223](https://gitcode.com/cann/ops-ras/merge_requests/223) | !223 的真实修改只有量化链接一行，已验证该页 12 个链接目标；与 !237 重叠。原报告声称约 6 处但只列部分位置，原 Simulator 问题已在 master 修复，不能据单行 PR 宣称原 6 项全部验收。 |
| [#40](https://gitcode.com/cann/ops-ras/issues/40) | [!140](https://gitcode.com/cann/ops-ras/merge_requests/140) | PR 正文明确引用本 issue，当前 open、目标 master；这里只确认关联与状态，尚未逐项复核完整 diff/CI，不宣称修复已验证。 |
| [#38](https://gitcode.com/cann/ops-ras/issues/38) | [!141](https://gitcode.com/cann/ops-ras/merge_requests/141) | 已有 !141 的图片命名规范方案；需与 !137 / #64 的补图方案统一，避免相互失效。 |

## 已有PR，需补针对性验证

| Issue | 已有 PR | 理由 / 下一步 |
| --- | --- | --- |
| [#118](https://gitcode.com/cann/ops-ras/issues/118) | [!235](https://gitcode.com/cann/ops-ras/merge_requests/235) | 新增 !235 已提出 FP32 checksum 路径和 K=1 保护，尚无设备验证；优先验证此候选，停止把 Host 阈值试探当作唯一修复路线。 |
| [#109](https://gitcode.com/cann/ops-ras/issues/109) | [!233](https://gitcode.com/cann/ops-ras/merge_requests/233) | 新增 !233 与旧本地草稿实现不同（新增 --debug 别名）；旧草稿 8 项回归不能自动证明 !233；需要针对该 head 核对参数传递及 Valgrind 路径。 |
| [#65](https://gitcode.com/cann/ops-ras/issues/65) | [!135](https://gitcode.com/cann/ops-ras/merge_requests/135) | 已有 !135；先跑 Host InferShape 的未知维度输出 [-1] 和静态 [8] 对照，不把 -1 分配为普通设备张量。 |
| [#53](https://gitcode.com/cann/ops-ras/issues/53) | [!126](https://gitcode.com/cann/ops-ras/merge_requests/126) | 已有 !126；先跑 Host tiling 大 K 元数据用例检查 aMeanFactorBits=0x0001，无需分配巨大设备输入。 |
| [#23](https://gitcode.com/cann/ops-ras/issues/23) | [!87](https://gitcode.com/cann/ops-ras/merge_requests/87) | 已有 !87；先验证四接口的阶段一 shape 拒绝和合法对照，不需要先运行非法 kernel 或等待混淆服务。 |

## 缺条件或范围待确认

| Issue | 已有 PR | 理由 / 下一步 |
| --- | --- | --- |
| [#140](https://gitcode.com/cann/ops-ras/issues/140) | [!227](https://gitcode.com/cann/ops-ras/merge_requests/227) | 新增 !227；可先静态审查，但完整 11 条附件未核验；还与 !198/!207 有内容重叠。 |
| [#139](https://gitcode.com/cann/ops-ras/issues/139) | [!226](https://gitcode.com/cann/ops-ras/merge_requests/226) | 新增 !226；可先静态审查，但完整 17 条附件未核验，不能仅凭 PR 摘要写全部覆盖或关闭。 |
| [#52](https://gitcode.com/cann/ops-ras/issues/52) | [!146](https://gitcode.com/cann/ops-ras/merge_requests/146) | 已有 !146；目前服务器缺匹配的 Crypto AICPU 后端，旧草稿失败已回滚；读密钥路径另需 KMS/SPDM，普通提供密钥路径应单独判断服务需求。 |
| [#16](https://gitcode.com/cann/ops-ras/issues/16) | [!236](https://gitcode.com/cann/ops-ras/merge_requests/236) | 已有 !236，但这是示例能力扩展；需确认目标 SoC/支持范围，再安排 INT64/BF16 最小设备验证，不以静态 pre-commit 代替。 |
