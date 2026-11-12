# 通用助手匹配冻结后的真实质量验证准备

2026-11-12。状态Proposed、尚未批准或执行。候选已按[限定复审](xuanshu-general-layout-independent-review.md)及[冻结记录](../xuanshu-general-layout-freeze.md)收口。独立准备目录：`build/xuanshu-general-layout-review-20261112-a74e20`。本轮仅静态核对和公开无凭据GET；没有Provider调用、账户/模型列表/价格API、计量程序执行或宿主启动。

## 当日官方依据

[官方价格](https://api-docs.deepseek.com/quick_start/pricing/)及[更新记录](https://api-docs.deepseek.com/updates/)仍将deepseek-flash关联Flash-1017。使用峰时未缓存输入USD0.30/百万token、输出USD1.20/百万token作为保守本地费率，不依赖缓存或错峰折扣。

[Responses接口](https://api-docs.deepseek.com/api/create-response/)及[指南](https://api-docs.deepseek.com/guides/responses_api/)支持本轮固定文本请求、函数工具、流式事件、完整usage和每轮完整上下文；输出上限包含可见及推理token。本包固定reasoning.effort=none、max_output_tokens=4096，不使用图片、服务端搜索或其他额外计费工具。

当日12份公开文档、官方recipe及模型编码/tokenizer均下载成功，逐字哈希与2026-11-11对应公开材料一致。profile.json及内嵌tokenizer与冻结计量程序的匹配源码一致；recipe archive hash仍为BF40407077C893BA51DCFE2D015C457198CFF8C9F01923FCBAD4B11ACD01E9CC。模型tokenizer与recipe只差129264图片token定义；本包纯文本，不将该差异扩展为图片计量支持。完整公开来源和时间见public-sources-final.json。首次受限shell DNS失败单独保存，后续公开只读下载成功，不写成模型请求失败。

## 计量与实际预算

源码`src/ai_assistant_meter.inc:33`对每轮最终序列化body调用固定计量，绑定request/body hash、profile/evidence；发送前保存占用及意图。初始、准备read、专业准备/问题、专业结果及根结果共享一个根账本。候选最多8次，不返还已占用次数；初始可信确认起600000ms；本地费用停止阈值100000000 nanoUSD。

当前通用路径的计量接纳上限为131072 input tokens，不能沿用旧BR-2的16384。每轮预留为`实际计量input_tokens×300 + 4096×1200` nanoUSD。单轮极端上限USD0.0442368；八轮极端合计USD0.3538944超过USD0.10。因此本包不声称八轮均能获批：每轮均受剩余预算检查，足额才发送，不足则停止。正常隔离业务六轮只是既有路径证据，不是未来真实轮次数保证。

本地known是绑定峰时价格对有效usage的结算值，不是读取账户账单；缓存优惠可能使实际账户费用更低。unknown、usage超界、计量失败、存储失败、期限到达或超出次数均停止后继发送。不会将unknown记零或承诺服务端绝对扣费上限。

## 可审核的具体包与未执行边界

具体内容见[authorization-package.md](../../build/xuanshu-general-layout-review-20261112-a74e20/authorization-package.md)，附price-basis.json、policy-proposed.json及execution-plan.md。唯一计划会话为`build/xuanshu-general-real-run-20261112-a74e20`，当前不存在，未创建库或Profile。具体策略不加载到环境或宿主。

场景是包内浏览器只读加自有维护单元格可撤销编辑，不能包装成纯只读。输入、两目标、白名单及字段在具体包内固定；模型计划只能在这些范围内，初始及有限计划分别人工确认，撤销另行可信确认。测试用于直接独立读回的state.cell.read不自动成为模型额外权限。

日常start-general.ps1明确移除UI_AI_ASSISTANT_POLICY；不能直接把拟定策略放入环境后调用日常入口并声称保护已启用。execution-plan记录匹配生产文件、唯一会话和监督顺序，独立launch-after-approval.ps1已按固定数据库准备；语法解析零错误、路径逐字相符。新真实命令尚未动态验证，任何前提失败停止，不重启/换库补额度。原8+2项只复用监督设计及相同helper身份，不冒充新命令已动态通过。

真实执行仍需用户对新包另行明确批准，且当日程序/官方依据不变；跨日、模型/价格/协议或范围变化须更新包。当前完成的是核实、冻结及准备，未开展真实质量验收。取消的临时控件测试继续取消。
