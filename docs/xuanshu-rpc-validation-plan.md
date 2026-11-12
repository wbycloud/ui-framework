# RP-C首次真实DeepSeek服务验证规格

> 2026-11-02最新收尾：RPC-K1冻结追记完成，独立T2/T3补测已停止并正常关闭。T2实际一次POST收到xs_read_discover/page=1，原page0-only适配器拒绝；公布Schema仅integer的说明缺口已定位但本轮未修，工具0、M1/R1/S1/C1/U0、usage494/90及本地参考USD0.0002562、唯一failed终态保存。T3未验，不重复T1、不重试/退额/新会话追加。详[补测记录](validation/xuanshu-rpc-real-service-validation-t2t3.md)。须先另授权最小工具声明整改及离线回归，不能继续旧会话；下方授权/准备为历史。

> 2026-11-02新授权：RPC-K1限定独立复审通过及匹配快照冻结已[落盘追记](validation/xuanshu-rpc-credential-independent-review.md)。仅以原程序开展[独立T2/T3补测](validation/xuanshu-rpc-real-service-validation-t2t3.md)：T1=0、T2≤3、T3≤1、合计≤4，其他数据/费用/期限/来源合同不变。源码支持直接T2，界面T1仍显示但不操作。旧轮事实/额度保留，不恢复过期会话；本轮无产品修改或离线重跑。

> 2026-11-02第二轮最终：RPC-K1整改离线Native66/66、Light68/68、ASAN66/66；用户实际原生Key保存/同ref读回、复制诊断及T1生产文本成功，M1/R1/S1/C1/U0，usage467/6，参考费用USD0.0001473。T2准备时validation_deadline，本轮T2/T3未执行、没有延长/重置/追加请求；用户正常关闭，关后库唯一成功终态及身份已只读核对。整轮RP-C未完成，不能报告通过，详[第二轮最终记录](validation/xuanshu-rpc-real-service-validation-round2.md)。下方授权/准备/失败均属各轮历史，不是重新启动许可。

> 2026-11-02新增独立授权：按[RPC-K1](xuanshu-rpc-credential-closeout-contract.md)核实并整改原生凭据入口/后台确认/诊断复制，离线完成后才以新匹配程序开展[第二轮受控验证](validation/xuanshu-rpc-real-service-validation-round2.md)。用户未见旧Key界面，聊天“已录入”不是保存证据。旧轮R=1及首败保留，新轮5按1/3/1是独立授权，不续用旧会话；来源、外发白名单、费用/期限和既有公共合同不变。

> 2026-11-02实际结论：T1完成可信确认后，生产路径在发送前credential_unavailable失败，R已消费1但S=0；T2/T3未执行，禁止本轮重试/新目录续跑。用户正常关闭，进程退出及隔离库唯一失败终态已只读核对；M1/R1/S0/C0/U0。RP-C首次真实服务限定验证未通过，不能宣称DeepSeek/TLS/真实read已验；未修改冻结代码或合同排障。详[实际记录](validation/xuanshu-rpc-real-service-validation.md)，后续凭据链路诊断及新尝试另授权。

> 2026-11-02：准备整改已由上一轮限定独立复审通过，匹配快照冻结记录见[追记](validation/xuanshu-rpc-preparation-independent-review.md)。用户已另授权本轮匹配程序及三合成任务最多5次真实POST，仍须用户原生录入隔离Key及逐任务可信确认后才能发送；不授权代码/构建/新功能。实际过程见[真实验证记录](validation/xuanshu-rpc-real-service-validation.md)。下文旧未授权/待评审状态均为历史，不用新授权倒填旧轮。

> 2026-10-31限定整改：原准备独立复审暂不通过后，已最小修复旧入口client ID校验顺序并补精确10/r1断言。新候选及实际离线结果见[整改记录](validation/xuanshu-rpc-preparation-closeout-validation.md)，待限定独立复审；真实执行仍未授权。下文原准备身份/结果保留为历史，不自动纳入新验收，新的候选启动方式见真实清单第6节。

日期：2026-10-31。状态：验证准备已获独立授权，RPCP-1已冻结并实现，离线验证交付见[准备验收](validation/xuanshu-rpc-preparation-validation.md)。真实API及用户凭据操作仍未授权、未执行。读者为宿主/框架开发者及验证执行者，不署名。

## 0. 准备交付增量（当前状态）

证据根`build/xuanshu-rpc-prep-20261031-c913e8`，仅将入口497项与本轮增量对照，不将原RP/P2—P6归为准备修改。原冻结390源码入口匹配，原114产物及旧冻结清单保留。新增[RPCP-1合同](xuanshu-rpc-preparation-contract.md)、ai_validation.h限制API、生产rpc_memory.uapp和xuanshu_rpc_probe。旧结构/错误/依赖/额度不改，无数据库DDL。

启动器显式新目录conversation.sqlite及conversation.sqlite.model.json，ref由配置路径派生，默认0任务/0请求；原生Key/确认仍由原生产可信人工链完成，限制API不签发授权。生产夹具公开read实际返回10/r1；并非隔离测试DLL。发送前保存并限制R=5、任务1/3/1，所有续轮同门；S在发送API返回后登记，在途或保存未知如实显示，离线模拟单列。10分钟/费用阈值/usage未知封闭和重开保留消费计数均已实现。不得将model_calls当作精确HTTP次数。

后续启动模板及真实授权范围见[真实服务清单](xuanshu-rpc-real-service-checklist.md)。本轮仅编译启动器和离线夹具，不启动生产窗口、不读真实Key、不调用任何真实API；仍须准备代码评审及另一次真实执行授权。下文第2节的“当前缺口”和第8节的“下一轮准备”保留为准备之前的规格依据，以本节及新合同的实际状态为准；不是仍缺同一实现，也不扩大原授权。

## 1. 授权分层与已确认规格

| 层次 | 内容 | 当前状态 |
|---|---|---|
| A：原规格轮 | 原冻结身份核对、8份文档、官方资料及实际源码只读探索 | 已完成，原轮不启动程序/读取Key/联网API |
| P：本轮验证准备 | 显式隔离配置的启动器、生产链接只读夹具、发送前计数/拒绝及必要脱敏观测；离线验收 | 已另授权并交付，停在准备代码评审 |
| B：首次真实调用 | 准备验收后，以下3项任务及最多5次POST | 未授权，必须另行明确执行范围 |
| C：用户操作 | 原生凭据录入、可信人工外发确认、必要界面/账单检查 | 当前不要求操作；将来逐项执行，不由程序注入或页面布尔替代 |

已确认模型`deepseek-flash`，官方origin `https://api.deepseek.com`，唯一请求`POST /responses`；Bearer Key、非思考`reasoning.effort=none`、流式，保持当前固定`max_output_tokens=4096`。不调用模型列表/余额接口探测账号，不保证具体账号已开通，不自动换型号或端点。

凭据只使用新隔离验证配置路径派生的框架自有引用，由用户在宿主原生界面录入；不在聊天中提供Key，不枚举/读取/覆盖其他已有凭据，不读环境Key。每任务history恒为`[]`，不发业务文件、整段主对话、其他应用快照，不开放AI写入/审批/图调度、外部网络工具或浏览器副作用。

整轮最多5次POST：文本1、发现/读取及续轮合计最多3、取消尝试1；失败和发送未知也占额，不自动重试或挪用剩余额度另开任务。费用停止阈值USD0.10，不是服务端硬扣费保障。保留原10/30/30/120秒网络合同；整轮10分钟停止新派发并请求取消，安全收尾仍等待实际退出，不能强杀线程。

## 2. 已冻结快照与准备前缺口（历史依据）

继承[RP-A/B冻结记录](validation/xuanshu-rpab-independent-review.md)。根`build/xuanshu-rpab-closeout-20261031-a19c42`，source-after清单SHA-256 `DE9DD6C31991971676C58995F319AC337130D61315D2205F78B213B608C718C8`；binary-files清单`7416C6FA7E5F27C222CAD4FDB0C315941929EC9B6D1D883E188E8E583843855B`；delivery-identity清单`B2F9941B437964071D775345F3390C847A5A6ADC3190618CC500EAB93197DAA0`。正式Light程序为light/framework_host.exe、ui_framework.dll，分别为A55617390C4FB2854881DB838B4D08B2BAE96BB880E36AA584EEE6B7D7BB550A和E04987A3AED11C3CE44AF584DBACEFAF4308E469AF486CD7CE47A3E179A56C53。

不得直接把离线EXE或ui_framework_edit_test.dll当成生产联网路径。当前存在以下RP-C准备缺口，并非推翻RP-A/B限定验收：

1. `examples/framework_host/ai_panel.inc`的正式分支调用`ui_ai_model_open(service,NULL)`，使用用户级默认model-config.json；UI_AI_DB_PATH仅改变对话库路径。它不会自动隔离模型配置或凭据引用。验证启动器必须显式调用model_open指定新目录内配置路径，再按该路径SHA-256生成框架自有引用。不得改LOCALAPPDATA来伪装隔离，也不得覆盖默认配置。
2. `tests/ai_edit_fixture.c`及ai_model测试链接隔离运行时，既有read输出只有value，不能冒称已有生产链接、同时输出revision的新夹具。需准备一个只使用公开API、链接生产运行时的专用只读uapp，回读固定value=10与revision=r1，没有修改/事务/外发handler及任何私有接纳入口。
3. 当前`model_calls`在provider_request保存时递增，计的是已保存请求manifest，不是精确的WinHttpSendRequest次数。当前每任务16轮是安全上限，没有整轮5次POST或每任务1/3/1的独立硬门。只靠提示词或UI轮询无法保证这些小限额；必须先补发送前接纳门和计数证据。
4. 当前最终Responses JSON会保存usage，但面板没有权威实时费用计价器；流式中间结果和取消可能没有usage。不能把模型回复、最后一个remote_delivery字段或零usage当成账单事实。

准备接口/实现需先独立记录影响，不提前冻结新增C API、数据库DDL或更改既有公共合同。可复用Service公开API、原生按钮绑定及同一能力目录；必要增量保持独立，不能使用fixture授权或传输替身替代真实生产调用。预计触及验证启动器/夹具、`src/ai_model_net.inc`唯一发送点的必要私有准入/脱敏观测及相关离线测试，准确清单由下一轮准备授权冻结。本轮不写这些源码。

准备后的源码、启动器、生产DLL、uapp、配置策略及导出必须形成新的匹配身份并做相称离线回归；新增准备差异不会自动获得原快照验收。尚未准备的文件没有可用启动命令，不能虚构一个可直接执行的RP-C命令。原正式Light程序也不应现在启动。

## 3. 官方协议核对与当前实现

只读核对链路：Service/real_propose→本地保存与可信外发确认→provider_request保存及再校验→am_net_start/WinHTTP→UTF-8/SSE聚合→完整工具批次验证→P2 read prepare/dispatch/result/forget→工具结果保存→有界续轮→最终保存/关闭。一个workspace仍只有一个Service及一条主对话。

官方资料支持当前选用的Responses端点、function调用、语义SSE和非思考方向，但不构成真实兼容验证。多工具响应须整批校验再串行，不能依赖parallel_tool_calls。官方API无服务端会话续接；客户端仅传本任务已确认的上下文和配对工具结果。没有把其他服务商或OpenAI行为套用为DeepSeek保证。

请求body仍含当前固定系统指令、xs_read_discover/xs_read_call两个封闭包装工具定义和本任务input；即使文本任务没有目标，也会发送这两个包装定义，不能写成“完全没有工具定义”。允许的是必要本地function声明，不是启用厂商内置web_search等工具。半JSON、非法参数、重复ID异参、非read或越界目标均不得派发。模型自然语言或工具数据不能生成授权。

原输入16KiB、上下文/编码请求128KiB、回复64KiB、单事件256KiB、累计流2MiB、2MiB任务arena计入原8MiB，以及P2/线程/生命周期门保持。RP-C小请求数是额外的验证限制，不能提高生产原限额。正式路径关闭重定向、自动认证/cookie及失败连接重发；任何所需系统安全选项不支持时停止，不降级重试。

## 4. 合成数据白名单与逐任务确认

固定系统指令按`src/ai_model.inc:am_body`原文，内容是仅使用获准read工具、target-0等别名、范围不足时询问、不写入/审批/外部工具/建图。两个包装工具的名称、说明、参数Schema亦作为确认范围；不在验证时临时扩充。每个任务展示实际输入、history=[]、目标与epoch/revision、能力和字段投影，确认与其JCS/描述指纹绑定。

| 任务 | 当前输入及准确数据 | 固定目标/投影 | POST配额 |
|---|---|---|---:|
| T1合成文本 | `只用中文回复“玄枢流式验证成功”。不要调用工具。`；固定系统指令和两个read包装定义 | 无目标；`{"capabilities":[],"fields":[]}`；history=[] | 1 |
| T2发现与读取 | `请先发现target-0的memory类只读能力，再调用memory.read读取数值与版本，用一句中文报告；不得修改或调用其他能力。`；仅下述描述/Schema及实际只读结果 | 唯一memory.document实例，target-0；投影`{"capabilities":["memory.read"],"fields":["value","revision"]}`；history=[] | 3，续轮全部计入 |
| T3取消尝试 | `请用中文解释什么是流式输出，最多三句；不要调用工具。`；无其他历史/业务内容 | 无目标；空投影同T1 | 1 |

T2夹具规格：capability ID memory.read、能力版本1、category memory、effect read、target kind document；名称`Read synthetic memory value and revision`，说明`Read-only RP-C fixture; value 10 and revision r1.`；输入Schema为`{"type":"object","additionalProperties":false}`，参数`{}`；输出Schema为object，properties为value(integer)及revision(string)，required=[value,revision]，additionalProperties=false。实际输出应严格为`{"value":10,"revision":"r1"}`，值及版本来自应用内存模型回读，不是执行计数/控件变化。发现结果可外发该项name/description/category、contract_version=1、capability_version=1、target_kind、effect、cancellable=false、descriptor_hash及上述输入输出Schema；不外发其他描述。

本地固定instance ID、resource ID、host/open epoch、revision及scope用于执行核对，模型工具只使用target-0别名。未出现在实际body中的本地ID不额外上传；不能将允许的本地诊断ID理解为可任意外发。T2只接受该目录与字段白名单；不加载其他业务包，不点击任何修改按钮。

续轮会重发本任务的明确上下文、已接纳function_call及配对function_call_output；这已在任务范围内，不包含整个主对话。若真实请求body新增字段或内容，停止后继派发并重新确认；工具结果中的自由文本不扩权。

## 5. 请求计数、期限和费用

准备门必须位于真实发送前、覆盖所有模型续轮及任务，关联Service session、run_id、provider_request_id和ordinal。在不阻塞UI的路径上先登记配额/请求摘要并获得保存确认，再允许唯一WinHttpSendRequest；相同请求ID不得第二次发送。计数或保存失败一律不发送，超出任务1/3/1或整轮5的请求明确拒绝。不替换WinHTTP，不放宽认证/来源，不增加业务工具自动重试。

至少分别记录：已保存manifest数M、消耗的发送配额数R、WinHttpSendRequest进入次数S、收到完整响应数C、发送未知数U及服务端response ID（如有）。每个工具续轮独立计数；应有C≤S≤R≤M、总R≤5，失败/取消/发送未知不返还配额。若API未成功进入，仍保守占用已经登记的配额。model_calls只能辅助对账，不得改称实际HTTP数；S计的是应用层发送调用，不声称等于远端计费笔数或TCP报文数。UI轮询不能充当发送前硬门。

整轮只运行一个已确认任务，待其唯一终态及保存确认后再开始下一任务，禁止同时排队多个可发送任务。没有预留自动重试；任务配额未用完也不自动转给其他任务。重启只核对记录，不恢复旧授权或自动联网，剩余配额不得自动重置继续用。欲再次验证需新授权。

系统现有超时设置为解析/连接10秒、发送/接收30秒；应用有效进度空闲30秒、总120秒，保持冻结10/30/30/120合同，不提高值。整轮从首次外发确认接纳时起计10分钟，排队和续轮不延长；到期停止新发送并请求取消。5秒取消停滞只是可观察状态，不是强杀、回滚、免单或可释放模块的证明；收尾允许超出执行期限，以连接/回调/存储真实退出为准。

费用停止阈值USD0.10。准备阶段按执行当天官方价、峰时/未命中缓存的保守口径估算，不依赖缓存折扣；本轮只读官方价格页，未查询账户/余额。当前参考Flash峰时每百万输入token USD0.30、输出token USD1.20，价格会变化，不能将其冻结成厂商长期承诺。usage缺失或费率/账户计价无法核对时记unknown，不记0；未知消耗占用请求额度并停止后继真实发送。累计已知费用或保守预留触及阈值即停止；剩余阈值不足以覆盖下一次保守预留则不发。不能用字节数冒充精确token数。

该阈值是本地停止策略，不是服务端硬限额、实时账单、汇率/税费或退款保证。取消/断流仍可能被远端处理并收费。用户可另用官方账户预算措施，但本轮未核实其存在及强度；如要求绝对不可超过USD0.10而无法证明，真实验证暂停，不以小样本预计便宜代替保障。

## 6. 操作、预期与证据

| 步骤 | 前置条件/操作 | 预期与证据 |
|---|---|---|
| P0准备验收 | 另授权准备实现和离线验证；核对新产物、生产导出/依赖、独立启动方式；准备进程不得联网 | 显式新目录数据库及model-config，不触及用户默认数据；计数门1/3/1及5、重复ID、崩溃/存储失败、无凭据/确认时0发送均有离线证据；无fixture/test传输出现在生产调用路径 |
| P1用户配置 | 准备验收后另授权真实验证；用户通过原生界面录入隔离Key，显式选择deepseek-flash | 脱敏ref、origin、profile revision及保存成功；无Key/Authorization进页面、库、事件或日志；配置行为本身0 POST |
| T1 | 展示白名单并由真实人工路径确认，只提交1个合成任务 | 收到有序文本分片和完整协议终态；回复语义/文字核对；manifest已保存后才发送，最终消息及唯一终态保存；没有实际read派发，无第二次POST |
| T2 | T1无异常后加载新的生产链接内存uapp，绑定target-0的实例/epoch/r1，先本地回读基准，确认范围 | 模型真实提出发现和memory.read；完整批次先校验再串行，function_call_output含完整Schema；实际业务回读10/r1，结果保存后才续轮，最多3 POST；模型不调用工具则记录未覆盖，不把提示文本当成功 |
| T3 | T1/T2无异常且剩余额度和费用可用；新人工确认，在真实文本delta后请求取消 | 只1 POST；观察取消请求与远端状态不确定、连接/回调真实退出、任务及存储状态。若响应在取消前已完成，记录该取消窗口未覆盖，不再付费重试或人为制造迟延 |
| P2保存/关闭 | 停止接纳；继续UI poll，等待连接、P2调用、worker及回调退出，保存/flush/close | destroy在途PENDING，退出后OK；UI不join依赖UI的worker；原DLL卸载门保持，终态唯一且保存确认。成功流结束不能替代数据库确认 |
| P3隔离冷开核对 | 后续同一授权明确允许离线重启时，使用同一隔离库，只恢复历史 | 消息/任务/工具关联可核对，旧授权不恢复，0自动POST/工具重放；不操作用户库或旧recovery工具 |

任何任务不符合预期时停止后续任务，保留已发生事实；不为取得绿色结果额外提交请求。文本分片仅为展示，response.completed为协议完成，tool_result为工具事实，最终保存确认才是本地权威完成。请求已保存也不等于已发送；远端完成不等于本地最终保存。不同层次分别记录。

标签切换不改变已接纳目标；目标重开、epoch/revision变化必须在派发前失败，不重新绑定。资源只读固定r1，人工也不改值。版本/重开、非法批次、401/429/5xx、TLS/代理错误、存储故障、checkpoint及DLL生命周期等主要复用既有离线证据；真实服务不承担全量故障注入，不故意使用坏Key、坏端点或制造计费错误。

## 7. 脱敏、停止和诊断

只保存本轮合成输入/输出、脱敏配置引用、模型/profile revision、协议版本、run/request/tool/response ID、请求body摘要/字节数、R/S/C/U计数、相对时间、错误码、usage（缺失标unknown）、存储确认/终态/关闭状态和新产物哈希。允许在隔离证据中保留已经逐项确认的合成body用于范围对账，认证头必须在采集前排除；不保存Key、用户名、系统凭据枚举、完整传输原始内存、真实业务文件或其他应用内容。

外发一致性以本地已确认的规范化内容/指纹与实际发送前body摘要及允许元数据对照，不能用模型自报“没有泄漏”证明。新增准备观测不得记录Authorization。服务端账单核对由用户将来在官方界面操作，只记录脱敏汇总/所需最小截图，不读取凭据或自行调用余额API。原隔离库不得上传厂商。

超请求/费用阈值、未知费用不能核对、越界外发/工具、重复请求、目标失效、非法SSE、401/429/5xx、TLS/代理错误、断流、存储不可用/checkpoint异常、取消停滞或关闭异常均停止新发送和新业务派发，继续cleanup-only。COMMIT成功后checkpoint失败保留已提交事实，最终状态未确认不伪装成功；业务UNKNOWN原门不受取消/forget/重启影响。不能强杀线程、提前卸载或关闭证书校验。

保留最小诊断、匹配源码/产物及失败证据；不得为节约额度自动丢终态/请求关联，不在真实环境做“再试一次看看”。真实设备/来源分类、IME、DPI、视觉、辅助技术及长时属于原集中人工清单；本次为完成确认所需的少量用户操作不等同于全部人工项目验收通过。

## 8. 准备授权前的范围建议（历史）及真实执行边界

推荐下一轮只授权RP-C验证准备：显式隔离启动器/原生确认入口、只读合成uapp、唯一发送点的计数拒绝及脱敏观测、独立增量合同、相称离线测试/构建和证据。必须先审查冻结合同影响；如需改变公共布局/错误/来源/额度或新增DDL，则暂停相关部分提交方案，不自行扩展。不得调用真实API、读取真实Key或触及默认用户数据。

准备通过并形成新身份后，才可另授权实际DeepSeek验证：指定新产物及启动命令、deepseek-flash、官方POST端点、隔离ref及用户原生录入、表中三任务/无历史/准确白名单、最多5 POST（1/3/1）、USD0.10停止阈值、原网络超时和整轮10分钟停止新派发、真实退出后收尾、隔离库及必要冷开/账单核对范围。授权应写明是否接受取消/账单未知及无法严格保证服务端硬费用；不能将此段文档当成执行批准。

原规格轮只确认方案，不等于实施/真实调用授权。之后本轮已另授权并完成准备源码、测试、构建和文档交付，新增启动模板仍不执行；没有版本/依赖升级或真实请求证据。停止在准备交付与代码评审位置，真实执行继续需要独立授权。

## 9. 来源与保留风险

官方资料查阅日2026-10-31，仅访问公开文档网页，未访问任何模型/模型列表/余额API：

- [DeepSeek Responses指南](https://api-docs.deepseek.com/guides/responses_api/)：语义SSE、多工具和兼容范围。
- [DeepSeek Responses接口](https://api-docs.deepseek.com/api/create-response/)：POST路径、非思考字段、function配对、usage及终态。
- [DeepSeek Models & Pricing](https://api-docs.deepseek.com/quick_start/pricing/)：当前模型别名、版本及峰/谷计价；账号可用性和费用仍须未来授权核对。

以上厂商事实不代表本地准入、5次配额、费用停止或生命周期保证由厂商提供。离线替身只能证明相应代码/协议样本，不能证明真实TLS、代理、凭据UI或DeepSeek兼容已通过。

历史四组期限、frontend63、ASAN-Light、Light surface_input、browser20、workspace20.05秒/20秒门槛及Native输入首败继续独立未定位，不归因环境，不以后续通过覆盖。browser九副作用另案，OpenAI/OAuth/AI写入/其他网络工具不纳入，参考工程不可读不扩权，冻结SDK和历史材料不改。真实设备/IME/DPI/视觉/辅助技术/物理断电/人工长时保持延期。

## 10. RPCB-1：本轮一次确认与自动收尾

用户已独立授权本轮开发及一轮真实T2/T3，详[批次合同](xuanshu-rpc-batch-contract.md)、[本轮验收记录](validation/xuanshu-rpc-batch-validation.md)。普通RPAB/C2/P4/P5不改变；专用新入口才允许将新验证配置绑定已指定d15f7883…03ab38引用，数据库及session/批次ID仍逐轮隔离。不枚举其他凭据、不复制Key、不改旧库或配置。

操作依赖：离线稳定及自审→锁定匹配Light程序及公开协议/费用→启动唯一新会话，默认零任务/零发送→后台同ref本地检查→只有需要时原生录入/保存/读回→真实鼠标或键盘点击一次开始→批次确认保存→自动T2发现/实际10-r1/框架revision/工具结果保存→T2终态保存且费用允许→T3观测真实在途后取消→停止接纳/cleanup-only→后台脱敏导出→存储及网络回调关闭→destroy→进程退出→只读关后库对账。

真实POST按总4及0/3/1硬门，T2所有续轮包含在3内；失败、取消或发送未知均占额。不得复跑T1、换ID/重开目录增额或在真实失败后改程序续发。T3若先完成，只记录取消窗口未覆盖，不追加请求。费用未知/usage缺失即停止，USD0.10是本地阈值并非硬扣费保障；10分钟自整批可信确认起算，网络10/30/30/120秒不变。

证据由程序自动生成，助手只读文件与关闭后隔离库，无需用户逐次复制诊断。实时报告导出不单独证明关闭成功，必须另核对stdout销毁标记、进程退出与WAL/SHM消失。全部设备、IME/DPI、视觉、辅助技术、物理断电和人工长时仍按历史延期，不将本批次外推整体验收。

### RPCB-1本轮实际停止

匹配Light启动器仅启动一次，PID5916早退；尚未生成会话、配置或自动报告，未要求用户开始批次，真实POST为0，T2/T3未执行。没有库内M/R/S/C/U，不伪造计数/费用/关闭确认。按约停止，不再启动或请求；根因未定位及最小后续启动取证建议见[本轮最终记录](validation/xuanshu-rpc-batch-validation.md)。离线通过不替代这项生产启动/真实服务未验。
