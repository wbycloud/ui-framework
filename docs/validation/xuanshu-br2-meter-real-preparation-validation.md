> 2026-11-10当前索引：下文11月9日准备及其包已过有效日，仅作历史参考，不原位改写或激活。本日身份与12份官方资料已重新核对，最新唯一包BR2-REAL-20261110-4da059仍待明确批准。
> [11月10日当天准备及集中包](xuanshu-br2-meter-real-preparation-20261110-validation.md)。冻结结论保持；不启动、不读凭据、不创建执行库或真实请求。

# BR-2计量冻结后的真实验证前置准备

2026-11-09。仅文档、只读身份和官方公开资料核对。未构建、运行测试、启动宿主或组件，未读取真实凭据，未调用Provider、模型列表、账户、余额或价格API。公开定价页面GET不是价格API。本轮不执行真实调用。

## 当前结论与证据目录

BR-2计量匹配快照冻结记录已落盘，本地交付材料已齐备。真实验证的模型/协议/编码/定价和程序保护依据已按本日核对；一次具体集中包现为Proposed，等待明确批准。没有执行数据库、激活策略或已启动进程。

新目录为[build/xuanshu-br2-meter-real-preparation-20261109-4ffee5](../../build/xuanshu-br2-meter-real-preparation-20261109-4ffee5)。preflight、编辑前文档、官方原始资料与URL/时间/选定响应头/SHA、encoding-match-final、fee-bound、price-basis和集中包均独立保存。原快照及旧准备目录不改写。

匹配identity为C5A430D7DA479E7706043741FE11D66EE95B976C51B849ADA92A26732BF62596；518源码、118产物、6启动项及91关联检查均匹配。本轮只核对前置身份，没有重新完整复审或重跑已有验证。

## 本日官方依据

| 核对项 | 本日读取的官方依据与判断 |
| --- | --- |
| 模型 | [价格页](https://api-docs.deepseek.com/quick_start/pricing/)及[更新记录](https://api-docs.deepseek.com/updates/)仍将deepseek-flash对应DeepSeek-V4.1-Flash-1017 |
| 协议 | [Responses规范](https://api-docs.deepseek.com/api/create-response/)和[指南](https://api-docs.deepseek.com/guides/responses_api/)支持/responses、完整客户端续轮、function工具和none非思考；max_output_tokens包括可见及推理输出 |
| 计费 | 按USD峰时非缓存输入0.30/M、输出1.20/M保守绑定；不依赖时段或缓存折扣，Search/Fetch另计但本任务不调用这些服务 |
| 编码对应 | [Flash-1017官方模型说明](https://huggingface.co/deepseek-ai/DeepSeek-V4.1-Flash-1017)将生产协议编码指向[Recipe](https://github.com/deepseek-ai/deepseek-recipe)；[DeepInfer官方说明](https://github.com/deepseek-ai/DeepInfer/blob/main/doc/inference_deployment_qa.md)说明处理与官方API对齐 |
| token计数 | [官方tokenizer说明](https://github.com/deepseek-ai/deepseek-recipe/blob/main/docs/tokenizer.md)要求显式控制符与普通文本分别编码；冻结组件执行完整Responses转换而非只编码用户文本 |

本日Recipe归档SHA256仍为BF40407077C893BA51DCFE2D015C457198CFF8C9F01923FCBAD4B11ACD01E9CC，归档commit为935817c66f5d6e64838f419692545f06787e485f，73份原文件按精确根路径与固定来源一致。根Cargo的本地文本组件选择保持原交付身份，不在本轮修改。

模型参考encoding/encoding.py SHA256为13231C592E958FB39E2AF6001DAE69D90E7A36B265702B9B390236FB387599E2。模型tokenizer SHA为C90DFA01249DB1BE4245780A052EDE752E1361C612AC6D08E2BDADA7D599476B；嵌入tokenizer为81F64D1248A68CE3663E07AB3EE48B851E5DF0E32D27CB98E4C9A268151E8D99。model/normalizer/pre_tokenizer/post_processor/decoder/truncation/padding/version一致，仅129264图像added token不同，本任务没有图像。

公开下载代码仅作数据审查，没有import、运行、安装或改变依赖。初次encoding.py根路径404及ZIP后缀匹配歧义分别保留，不能当成产品缺陷或版本变更。一次授权包JSON生成命令在执行前被自动审批拒绝，改用纯Markdown说明；没有生成启动器或激活策略。详driver-errors.md，该说明不冒充未自动落盘的原始日志。

## 可靠预留与停止条件

固定纯文本非思考、完整Recipe编码、每轮硬计量和4096总输出共同支持本范围的程序保守预留。候选input上界16384由已交付发送前门逐轮检查，不能靠初始496参考计数外推动态续轮。

| 项目 | nano USD | USD |
| --- | ---: | ---: |
| 每轮输入预留16384×300 | 4915200 | 0.0049152 |
| 每轮总输出预留4096×1200 | 4915200 | 0.0049152 |
| 每轮合计 | 9830400 | 0.0098304 |
| 最多3次（初始及全部续轮） | 29491200 | 0.0294912 |
| 距0.10停止阈值余量 | 70508800 | 0.0705088 |

这些是纯算术及既有程序门的依据核对，不是新测试、真实usage或账户扣费。实际响应usage仍是服务端观察；本地known按绑定价计算，不保证服务端绝对扣费。无思考且输出上限涵盖全部生成token；没有图像、官方Search/Fetch或其他付费功能。价格或版本变更使本包失效，不能沿用旧费率。

可信确认起10分钟；占用先保存再传输，失败、取消和远端未知不退款，无POST自动重试。每轮input超界、计量/身份/receipt失败、usage缺失或超出计量上界、价格不匹配、费用unknown、期限或存储失败均停止后继。候选manifest可以先于计量保存，不将其、model_calls或占用数当作HTTP次数。

## 一次集中包与未激活的执行方案

具体包为[BR2-REAL-20261109-4ffee5](../../build/xuanshu-br2-meter-real-preparation-20261109-4ffee5/authorization-package.md)。程序、固定输入、全部允许外发内容、目标/能力/字段、价格依据、费用边界、唯一未来目录、凭据与可信确认、监督、关闭及关后证据均在包内。

拟议目录为D:\应用软件框架\应用层序框架\build\xuanshu-br2-meter-real-run-20261109-4ffee5，目前不存在。批准后才允许创建该唯一会话、绑定数据库及本次策略；不操作或恢复旧库，不重新派生会话补额度。

当前start-readonly及Proposed模板仍锁停，监督函数拒绝verified模板。不能用原锁停入口直接修改布尔值续跑。本包提出批准后按同等监督规则执行固定一次受信会话的显式环境/命令流程，不修改冻结脚本，不建立通用自动批准入口，也不把原8+2验证自动当成该未来流程的新测试。若需改产品或原监督机制，须停止并另行说明。

本次集中批准仍不能替代宿主逐任务可信人工确认。用户只通过原生入口录入Key，助手不读取密码内容或环境Key，不注入确认；显示/目标/模型名不一致时停止。未获具体包批准时不能启动程序，普通“继续”不等于批准。

## 保全与未验边界

原修前5例、Light127/127、ASAN121/127及另次6/6、最终计量各20/20、Native38/38、组件15/15分别保持。六项ASAN首败、计量中failed终态、release子进程、任意卡死限时关闭未证明，以及原历史首败不被本次公开核对解释。

40主样本关后无旁文件的结论只适用于原观测范围；2241份自有材料、83份旁文件及1份非空WAL全部保留，本轮不重新打开数据库。BR1-KI-001未解决且用户接受非阻断；临时页面控件测试保持取消。没有真实Provider或本次人工验收，真实来源/IME/DPI/辅助技术/长时/断电及browser九项副作用不扩展。

本轮保留必要新资料、编辑前文档、错误说明和身份记录，无安装、解包执行或编译缓存。最终文档状态与原冻结副本分开核对；停止于具体集中授权确认之前。
