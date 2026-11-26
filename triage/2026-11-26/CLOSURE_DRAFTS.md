# 可关闭候选的说明草稿

分类日期：2026-11-26。**仅准备文案，未发表评论或执行关闭。** 当前仍需遵守用户暂缓 GitCode 远端操作的决定。

## #45：已修复 / 重复

已在当前 master（6943b100）核对：中英文 op_debug_prof.md 的 Simulator 链接均指向实际存在的 cann_sim.md，相关中英文锚点与标题一致。修复 !112 已合入，讨论确认与 #42 同一问题。可按已解决/重复关闭，不再单独提交修复。

依据：evidence/closure-evidence.json、evidence/closure-evidence.json、原仓 PR 页面及 PATCH_INDEX.json。

## #51：当前目标分支已正确

当前 master 中英文 API 清单均指向实际存在的 aclnnMatmulAbftVerify.md / aclnnMatmulAbftVerify_en.md，旧文件名失配已不再存在。可按当前版本已解决关闭；关联 !125 仍 open，需另行由维护者判断其是否已无必要，不自动关闭该 PR。

依据：evidence/closure-evidence.json。纯文档链接，无需 NPU。

## #49：误报

讨论已明确确认：反馈将内置 OpsRas 安装器要求 --full 的规则套用于自定义算子包；实际示例的自定义包不要求该操作类型。关联 !123 已关闭，不应继续补 --full。可按误报关闭。

依据：evidence/closure-evidence.json。关闭说明应保留纠正后的结论，不沿用讨论中早期错误分析。

## #63：与 #59 重复

讨论已确认本条与 #59 是同一份孤立量化文档副本及图片路径问题。可按重复关闭本条，保留 #59 / !139 继续跟踪。主问题尚待合入，不能写为已经修复。

依据：evidence/closure-evidence.json。

## #105：与 #64 重复

本条与 #64 是相同中文迁移指南中的四处图片问题；可合并跟踪并按重复关闭本条，主问题保留 #64 / !137。!190 已 closed，不把其当作已合入；图片补充和 !141 的命名规范仍需协调。

依据：evidence/closure-evidence.json、原仓 PR 页面及 PATCH_INDEX.json。

## 为什么本轮暂不列入 #116

当前 master 确实已不再引用失效的 invocation#环境准备，转而指向存在的 quick_install.md；但 issue 还要求直达“环境变量配置”章节，而当前链接没有章节 fragment。
为避免将部分改善当成全部满足，本轮保守归为“已有 !188，待核对范围/章节定位”，不直接关闭。这是对旧快照“master_fixed_pending_close”结论的收紧。

