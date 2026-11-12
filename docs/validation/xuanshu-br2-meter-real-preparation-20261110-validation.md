> 2026-11-10后续执行状态：本文件下文保留准备时的未批准/未启动结论。本日具体包随后已明确批准并执行一次，PID23608实际exit0；由于真实目录分类不匹配，仅返回空目录，没有执行browser.snapshot，本次业务验收暂不通过。见[真实运行与关后核对](xuanshu-br2-meter-real-20261110-validation.md)。封装准备包、原命令及冻结文件未改；不再次执行本包。

# BR-2 2026-11-10真实验证当天前置准备

客户端日期2026-11-10，Asia/Shanghai。本轮仅必要文档、只读身份及官方公开资料核对。11月9日BR2-REAL-20261109-4ffee5已过其有效日，本计划转发和普通“继续”均不是批准。

## 当天结论

冻结对象、当天官方模型/协议/价格和编码依据均未变化。当前固定纯文本范围的程序保守预留仍可成立，具体包BR2-REAL-20261110-4da059已准备，等待用户明确批准。没有当前已确认的必须产品修改缺口；新的单次会话命令只作静态审阅，没有动态运行验证，不能称为已验证真实启动入口。

新准备目录为[build/xuanshu-br2-meter-real-preparation-20261110-4da059](../../build/xuanshu-br2-meter-real-preparation-20261110-4da059)。旧准备目录及过期包保持原字节，仅作历史参考。本轮不创建执行数据库或激活策略，不启动宿主/计量组件，不构建或运行测试，不读取真实凭据或调用Provider、模型列表、账户、余额及价格API。

## 冻结身份与已有证据复用

匹配目录build/xuanshu-br2-meter-20261106-1f8148，delivery-identity SHA256为C5A430D7DA479E7706043741FE11D66EE95B976C51B849ADA92A26732BF62596。518项源码、118项程序/链接/调试材料、6项启动清单及91份identity关联均匹配，四个生产文件仍与[冻结交付清单](../xuanshu-br2-meter-delivery-manifest.md)一致。

承接[已落盘的限定独立复审](xuanshu-br2-meter-independent-review.md)，不重新完整复审或验证。Git已有修改/未跟踪项与本轮文档增量分别保留，不将HEAD全部差异归本轮。参考工程仅核对目录元数据，未读取或修改其业务内容。

复用Light127/127、ASAN121/127及同程序原门槛另次6/6、最终计量Light/ASAN各另次20/20、Native38/38、组件15/15及修前5例4失败/1正常对照。它们不合并成首次单次全过。启动监督8项和辅助2项仅按原文件身份及原适用机制复用，不构成下面新命令的动态测试。

## 当天公开资料及差异

12份官方资料均在本日重新取得，URL、访问UTC、客户端日期、版本线索、选定响应头及原始字节SHA保存在public-sources.json；正文与昨日对应文件SHA全部相同。资料未执行、未import、未安装，ZIP仅以数据读取，无解包运行。

| 核对项 | 当天依据与结论 |
| --- | --- |
| 模型/价格 | [官方价格页](https://api-docs.deepseek.com/quick_start/pricing/)和[更新记录](https://api-docs.deepseek.com/updates/)仍将deepseek-flash对应DeepSeek-V4.1-Flash-1017；USD峰时非缓存输入0.30/M、输出1.20/M |
| 缓存/时段 | 缓存价低于非缓存，非峰低于峰时。本包不依赖任何缓存或时段折扣 |
| 其他收费 | Search/Fetch是另收费服务，本任务没有这些服务或图像等额外付费内容 |
| 协议 | [Responses规范](https://api-docs.deepseek.com/api/create-response/)及[指南](https://api-docs.deepseek.com/guides/responses_api/)仍支持/responses、none非思考、纯文本及function续轮；客户端每轮携带完整所需上下文 |
| 输出边界 | max_output_tokens仍涵盖可见与推理输出；固定4096用于全部生成输出，不只正文 |
| 编码 | [模型说明](https://huggingface.co/deepseek-ai/DeepSeek-V4.1-Flash-1017)、[Recipe tokenizer说明](https://github.com/deepseek-ai/deepseek-recipe/blob/main/docs/tokenizer.md)及[官方API对齐说明](https://github.com/deepseek-ai/DeepInfer/blob/main/doc/inference_deployment_qa.md)与固定文本编码链对应 |

当前Recipe归档SHA仍为BF40407077C893BA51DCFE2D015C457198CFF8C9F01923FCBAD4B11ACD01E9CC，commit为935817c66f5d6e64838f419692545f06787e485f。73份原文件按精确归档路径一致。模型参考编码SHA仍为13231C592E958FB39E2AF6001DAE69D90E7A36B265702B9B390236FB387599E2。

模型tokenizer C90DFA01249DB1BE4245780A052EDE752E1361C612AC6D08E2BDADA7D599476B与嵌入tokenizer 81F64D1248A68CE3663E07AB3EE48B851E5DF0E32D27CB98E4C9A268151E8D99的普通编码字段一致；仅129264图像标记不同，不在本次纯文本范围。day-difference.json记录逐项对照，没有用旧包的结论替代当天读取。

## 当天费用预留

完整实际body每轮经过已冻结Recipe转换及tokenizer，计数超过16384或receipt/body/profile不符即在占用/传输之前拒绝。不能用初始496参考计数、提示词、人工计数或三次额度代替该门。

| 预留 | nano USD | USD |
| --- | ---: | ---: |
| 每轮输入16384×300 | 4915200 | 0.0049152 |
| 每轮全部输出4096×1200 | 4915200 | 0.0049152 |
| 每轮合计 | 9830400 | 0.0098304 |
| 初始及全部续轮最多3次 | 29491200 | 0.0294912 |
| 距USD0.10阈值余量 | 70508800 | 0.0705088 |

本日price-basis.json SHA256为01A7CFB528AB6EE1E93180A67EA4F54533A2B69787B8976E96349BDEC67450BB。额度占用不退款，可信确认起10分钟，第四次传输前拒绝，无POST自动重试。known按绑定价对有效usage计算，estimated是预留，unknown不是0；实际usage缺失、超出计量值、价格/版本不匹配、期限、存储或远端未知均禁止后继。

这是当前固定协议/编码与官价的程序保守预留，不是实际账单或服务端绝对扣费保证。跨日或依据变化使包失效；不能把程序内预算计数宣传为精确HTTP次数。

## 可审阅的执行方案与唯一当日包

完整包见[authorization-package.md](../../build/xuanshu-br2-meter-real-preparation-20261110-4da059/authorization-package.md)，完整参数/必要命令见[execution-plan.md](../../build/xuanshu-br2-meter-real-preparation-20261110-4da059/execution-plan.md)。固定输入仍为“BR-1：读取本轮合成交接单标题、来源与版本，不导航或修改。”history=[]；browser.view；browser.snapshot；state、revision；只读scope。

state是完整顶层对象，包含标题/来源及布局、资源、错误/诊断、导航/滚动、性能等状态，不能称为只发送title，也不声称完成正文12/9/3汇总。固定指令、两条read工具包装、允许的目录描述、实际只读结果及必要模型续轮全部进入允许范围和计量；初始完整请求的数据副本单独保存，不是未来实际传输抓包。

批准后才允许创建唯一目录D:\应用软件框架\应用层序框架\build\xuanshu-br2-meter-real-run-20261110-4da059，绑定新库绝对路径、空来源配置、按库路径追加.model.json的模型配置及一次validation_id。当前该目录及拟议监督脚本均不存在，任何既有路径或准备失败都停止，不覆盖、不另建会话补额度。

原start-readonly、supervise及Proposed模板仍保持原字节和锁停。新方案不调用原只接受unverified模板的入口，也不删检查；在明确批准之后才按完整固定参数初始化这一次受信策略，并用原记录函数和单次可见Start-Process监督正式宿主。代码不提交任务、录入凭据或注入确认，用户原生凭据录入及原逐任务可信确认仍须本人完成。

只读调用核对补齐了界面顺序：模型配置及用户原生凭据录入/本地读回必须先完成，待profile revision稳定后才保存唯一提案。现有[配置门](../../src/ai_model.inc#L30)及[凭据门](../../src/ai_model.inc#L331)拒绝活任务期间修改，不能先提案再补Key。这是准备说明的顺序澄清，没有修改产品或动态复现失败，也不增加验证凭据的真实请求。

命令文本仅作静态解析：0语法问题、1处Start-Process，无强杀、Invoke-Expression或调用旧锁停入口。command-text-review.json记录该范围；没有执行该命令或运行启动测试。另存时需UTF-8带BOM以保持中文绝对路径，固定代码SHA为F96191D1BE827337759345D4A7A0E160670523D590FC75E5C85E7DEB74BFB1F3。

当前PowerShell7.6.9及RemoteSigned仅作只读元数据记录，未改执行策略或权限，不能保证未来会话状态。若实际执行时策略或访问控制不允许，应停止，不使用Bypass或提权。静态自审补齐了尚未导入原记录函数时的可取得Win32错误码；这不是产品缺陷修复或动态失败证据。

监督先核对identity/组件/价格依据，再独占创建目录和绑定。进程句柄、PID、stdout/stderr、实际退出码及有界阶段错误持续保留；辅助记录失败不丢失监督，实际退出未确定保持pending，不强杀或伪造。三项临时环境分别恢复，不改变LOCALAPPDATA。新外部命令尚未动态验证，不将说明等同于已验证运行入口；没有当前已知必须改产品的执行缺口。

## 保全与停止位置

ASAN六项首败根因未定位，121/127和另次6/6分账；计量中取消/关闭/到期可以保存failed/validation_send_rejected，不伪称cancelled。release子进程未ASAN插桩，任意卡死时限时关闭未证明，PENDING及不强杀边界保持。

40主样本的无WAL/SHM结论仅适用于原观测范围，2241份自有材料及83份旁文件（含1份非空WAL）保留，本轮不打开历史数据库。manifest、model_calls、占用不是HTTP次数。BR1-KI-001保持未解决、用户接受非阻断；取消的临时控件测试不恢复，已有观察不改成通过。

本轮没有编译、安装、运行或执行会话过程产物。仅保留必要新来源、文档前副本及只读核对材料，历史准备目录和冻结材料不清理。后续仍需本日具体包的明确集中批准以及运行时逐任务可信人工确认，停在批准之前。

Git取证有一项方法边界：为设置只读Git选项曾复制完整继承环境映射，未定向查询、输出、保存或对外发送Key值，后续已改为仅设置命名Git选项、不再枚举环境。没有检查是否存在Key，不将本轮记录当作环境内没有Key的证明。详[准备方法边界](../../build/xuanshu-br2-meter-real-preparation-20261110-4da059/preparation-boundaries.md)；不影响已核对的程序、官价和费用门，也不属于真实凭据使用授权。
