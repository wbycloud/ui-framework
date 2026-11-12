> 2026-11-06 官方编码链续查更新。已取得DeepSeek官方Recipe生产API转换规范及Flash-1017模型参考编码，文本tokenizer字段对应。固定初始内容的离线参考计数为496，尚未建立全部动态续轮上界。剩余范围收窄为完整Responses计量对齐与实际每轮发送前接入；原准备授权要求冻结产品保持原身份，相关最小开发需另行授权。详[本轮续查记录](xuanshu-br2-encoding-preparation-validation.md)。下文保留上一轮仅查演示资料时的判断与工具首败。

# BR-2一次真实业务验证前置核实记录

2026-11-06。当前状态为前置条件未满足，真实验证未开始。本轮完成冻结身份核对、官方公开资料与生产发送链对照、独立准备材料及其辅助自检。执行准备停在可靠计费input上界确认之前，集中授权包尚未达到提交条件。

## 已完成范围及匹配身份

沿用已冻结的build/xuanshu-br2-fee-20261106-54d9a8，delivery-identity.json SHA256为1EF3D1643AE1DF94ED6B6CC163F061F1376D82E3A5CC540823F1E197687105FB。433项源码、113项程序产物、5项launch-files及30份identity关联清单匹配。前置限定独立复审和本地交付结论继续按原范围保留。[冻结记录](xuanshu-br2-fee-restore-independent-review.md)

本轮新建独立准备目录build/xuanshu-br2-real-preparation-20261106-d83d0e。validation_id预定为BR2-real-20261106-d83d0e，执行目录预定为该根execution-session-d83d0e，数据库为conversation.sqlite，模型配置为该数据库路径追加.model.json。执行目录和数据库均未创建，PID和实际退出码为空。[准备目录](../../build/xuanshu-br2-real-preparation-20261106-d83d0e/README.md) [路径与状态](../../build/xuanshu-br2-real-preparation-20261106-d83d0e/planned-session.json)

## 官方资料与实际实现对照

| 核对项 | 已确认的内容 | 对应本地实现或限制 |
|---|---|---|
| 模型与端点 | deepseek-flash当前映射DeepSeek-V4.1-Flash-1017，官方支持POST /responses | 生产固定https://api.deepseek.com及/responses，协议为deepseek-responses-v1 |
| 输入与工具 | 支持instructions、input、function工具及call_id配对 | 复用同一请求序列化器及两条read包装工具，结构化范围限定browser.snapshot |
| 流与续轮 | SSE以completed/incomplete/failed结束；无状态续轮需回传input | 生产追加实际工具结果至items后重新构建下一轮body，原发送门覆盖全部续轮 |
| 输出上限 | max_output_tokens同时限制可见和推理输出 | 生产每轮4096，reasoning.effort=none |
| 费率 | 峰时未命中输入USD0.30/百万token，输出USD1.20/百万token | 候选保守费率为300/1200 nano USD/token，按全部未命中及峰时考虑 |
| 可计费input上界 | 页面公开上下文量级为1M，字符换算仅近似，最终usage给出实际量 | 本次所需的更小可靠上界尚无充分依据，verified保持false |

模型与价格依据按查阅日期、映射版本及本地摘录哈希绑定，发送前仍须核对资料是否变化。固定业务范围仍为包内合成交接单，history=[]，browser.view、browser.snapshot及state/revision。state保持完整顶层对象，样本正文汇总沿原覆盖边界处理。[官方价格](https://api-docs.deepseek.com/quick_start/pricing/) [官方Responses定义](https://api-docs.deepseek.com/api/create-response/) [官方Responses指南](https://api-docs.deepseek.com/guides/responses_api/)

## 具体阻断

现有BR-2启用接口要求受信策略已经确认价格和计费上界。av_reserve使用策略input_ceiling乘费率，加4096输出预留，在传输之前核对剩余预算。av_price只在收到响应usage后校验实际input是否超界。该结构依赖准备阶段提供成立的上界依据。[策略入口](../../src/ai_business_validation.inc#L19) [预留检查](../../src/ai_validation.inc#L83) [响应计价](../../src/ai_validation.inc#L113)

按已核实的保守费率计算，每轮输出预留为4915200 nano USD。USD0.10预算能够容纳的单轮input容量为316949；若希望三轮均按相同最坏input和完整输出预留，容量为94727。这些数值只描述预算容量。

官方1M上下文量级即按1000000 input token对照，单轮预留为304915200 nano USD，即USD0.3049152。即使把4096输出从1000000总上下文中扣除，费用量级仍超过本次阈值。当前资料没有建立覆盖instructions、工具定义、服务端封装、工具结果及累计续轮，且落在本次容量内的可靠input计费上界。

官方token页面提供近似字符比例和tokenizer演示。文档链接的deepseek_v4_tokenizer.zip已在内存中按数据检查，归档SHA256为E7310D1DAFE0A86D8A5629FE78A7C763760F651DB9B8682718A1781DCD6FE495。演示配置含model_max_length=16384及消息chat_template，冻结产品并未读取该配置或向服务器施加16384 input限制。资料仍缺当前Flash-1017的Responses完整计费封装对应依据。字符比例、请求字节数、模型返回usage及演示配置值均不能签发所缺的发送前上界。[官方token说明](https://api-docs.deepseek.com/quick_start/token_usage/) [本轮资源观察](../../build/xuanshu-br2-real-preparation-20261106-d83d0e/tokenizer-observation.json)

这属于本次真实验证前置证据缺口。原离线恢复兼容验收及匹配快照冻结继续成立，真实执行保持锁停。

## 准备材料及辅助自检

| 材料 | 用途 |
|---|---|
| official-basis.md | 官方链接、核对日期、必要摘录及资料对照 |
| preflight.json | 冻结身份、源码/程序/清单匹配及准备阶段工具说明 |
| tokenizer-observation.json | 公开归档及配置哈希、必要元数据和适用限制 |
| allowed-request-components.json | 源码逐值提取的固定输入、完整instructions、tools及固定范围 |
| planned-session.json | 一次validation_id、未来执行目录和三个环境路径 |
| policy-blocked.json | 绑定预定库和资料哈希，verified=false、input_ceiling=0 |
| check-preparation.py及helper-result.json | 独立预算算术、路径绑定和锁停状态自检 |

official-basis.md SHA256为88B9E48A175087585E5C7AE31981AE47E59D26A35BB74DC875F4F72D105244E2，policy-blocked.json SHA256为09BB4C5B70D43DE38FE437949E012C4C7DBE14426F80C4446F003DF1CD3287D3。前者哈希绑定本地核实摘录及判断，后者保持未核实策略状态。完整实例、epoch、revision及任务ID待实际启动后按原合同取得，当前组件文件只保存待授权范围。

辅助脚本的12项算术/配置检查通过，结果仍为BLOCKED_RELIABLE_INPUT_BOUND_UNVERIFIED。这是新增准备材料的自检结果，原产品测试和真实发送路径未运行。真实启动入口及其监督配置的准备依赖上界成立，当前未生成可激活的入口。[辅助结果](../../build/xuanshu-br2-real-preparation-20261106-d83d0e/helper-result.json)

首次准备元数据输出因包含完整Git状态而被工具截断，外层JSON解析失败，改为哈希及数量后完成只读核对。Web阅读器不支持ZIP类型，随后用内存归档读取取得上述公开元数据。首次辅助脚本命令误带stdin参数，只显示解释器提示，未执行辅助检查；更正命令后取得12项结果。上述均为准备工具问题，对话中的原工具输出与后续说明分开保留。[工具说明](../../build/xuanshu-br2-real-preparation-20261106-d83d0e/preflight.json)

## 停止位置与最小下一步

1. 先取得能对应当前Flash-1017及Responses协议的权威计费/tokenizer与封装依据，涵盖系统指令、工具Schema、工具调用/结果和全部续轮，并对允许输入建立可证明上界。该资料核实可在独立准备授权下继续，保持当前锁停策略。
2. 若完整依据能给出本次固定流程的保守上界，可继续准备一次库绑定策略、监督配置及可审阅的集中授权包。上界容量须与USD0.10及已知/未结清预留一致。
3. 若必须增加发送前的精确计量或经过验证的上界门，产品改动需另行开发授权、相称验证和限定独立复审。仅增加参数、改verified或依赖人工监控不能替代计费条件。

取得完整授权包并获得明确同意后，才可进入本次原生凭据录入、逐任务可信确认及一次真实验证。当前无真实运行结果、业务工具结果、终态或退出码可报告。当前本地交付结论保留，本次真实验证及其交付收口条件尚未满足。

原冻结产品、原入口、原策略和历史证据保留。BR1-KI-001仍未解决、用户接受非阻断，临时控件测试保持取消。本轮参考工程保持只读，原数据库和旁文件未操作，未读取凭据或发送真实请求。
