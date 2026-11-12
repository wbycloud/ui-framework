# RPAB-1：DeepSeek Responses只读接入冻结合同

2026-10-29，实施前冻结。仅RP-A＋RP-B；不含RP-C、OpenAI、OAuth、AI写入或新业务执行器。保持SDK0.10-dev/API10/标准10、ABI1/包1、v1/C2/P4/P5及全部旧额度。与提案差异以本合同明确项为准，不改写历史结论。

## 1. 接口与执行

独立ai_model.h/ai_model_ui.h，所有公开方法UI线程、size/version=1。model_open指定用户配置路径（空值采用用户默认位置）；configure复制模型及框架自有credential_ref，后台原子保存；get_state只返回脱敏JSON。real_propose/continue复制当前输入、明确历史选择、固定read目标及外发投影，received关联先于回调。旧submit始终stub。配置加载/保存/取密钥/网络/协议/SQLite在后台，UI仅投影。

普通技术细节由本轮授权冻结：DeepSeek官方https://api.deepseek.com/responses，非思考reasoning.effort=none，stream=true，max_output_tokens=4096；不发store/metadata/background/previous_response_id。不依赖parallel_tool_calls；完整批次校验后在现有单工具邮箱串行执行。远端内置执行工具、写入/P4/P5、其他origin、重定向和自动POST重试全部拒绝。

外发确认由框架现有私有Light输入链或原生宿主入口一次消费绑定，不能由页面消息/布尔/source字符串证明。确认固定run请求摘要、profile revision、目标与投影，最长30分钟，从确认时计；任务结束/取消/关闭/条件变化失效，重启不恢复。调用者只能配置自有命名空间及origin匹配的引用，不枚举/读取其他密钥；正式密钥输入采用宿主原生凭据对话框，明文不经过页面。

## 2. 数据、预算和存储

历史选择为最多32个消息sequence的JSON数组，由后台从本库加载；仅显式选择的user/assistant消息进入上下文。外发投影为封闭JSON对象capabilities（能力ID数组）和fields（顶层结果字段数组）；显式星号表示用户允许所列能力的完整结果，不隐含其他资源。未匹配字段不发送；未知/无效投影拒绝。面板展示具体能力和字段范围再确认，默认不选历史。

沿用输入16KiB、目标16、上下文128KiB、回复64KiB、模型16/工具64/步骤64、Service8MiB、队列16、cache64、command32、事件64/2MiB、控制256KiB、P2输入64KiB/输出256KiB。HTTP编码请求128KiB、SSE单事件256KiB/单请求累计2MiB、深度16/节点4096/数组512/对象128字段；新私有缓冲在分配前预留到原Service8MiB，不能另造预算。超限拒绝，不截断必要配对数据。输出usage缺失为unknown。

保存顺序：本地任务接纳→外发确认→模型请求manifest及必要输入确认→网络发送→完整响应/工具批次记录→原tool_request→prepare/bind确认→dispatch→实际read结果/forget→结果保存→下一模型请求→最终消息/唯一终态。优先schema3版本化事件payload，不新增DDL；旧schema仍按原合同处理。每步确认后重查存储，COMMIT成功后checkpoint失败不回滚已提交事实但禁止后继执行。存储不可用仅cleanup-only。重启NEEDS_REVIEW，不联网、不重放；显式继续新任务和目标。

## 3. 线程及生命周期

使用WinHTTP异步及原Service串行worker，无额外常驻执行线程。回调仅复制有限数据/发布稳定ID并唤醒；UI、网络回调不等待队列。请求引用覆盖发起栈、回调、完成待消费及保存；关闭等待HANDLE_CLOSING和回调计数归零，不以取消/超时伪造退出。UI poll唤醒覆盖网络/配置及完成槽，引用移交与P6R-1一致。应用只在UI解析ID并走原read链，真实DLL原门保持。

超时为连接10秒、首字节30秒、有效进度空闲30秒、总120秒；取消5秒仅停滞提示。旧测试门槛不改。请求取消后实际连接退出、工具租约排空才能终态/释放。远端处理/费用未知与SQLite未确认、业务UNKNOWN分别表达，不能清除P4未知门。

## 4. 实施安排和验证

RP-A核心稳定后接RP-B。内部模块可同翻译单元.inc复用ai_tool_begin/ai_store_tool，避免第二执行器。网络/凭据替身及私有确认只编入UI_AI_FIXTURE，生产不导出；本轮只运行离线夹具，不访问DeepSeek/模型列表/余额或真实Key。配置测试写新临时目录，凭据仅内存替身。原P6产品374/102身份入口匹配；上一轮文档增补不归新产品变更。

覆盖非法/碎片/重复SSE、多工具整批预检、只读实际模型回读、固定workspace目标、取消/背压/存储关键窗口/冷开、回调/worker卸载、Light输入保留/折叠/REFUSE及静态嵌入。保留首次失败与匹配程序。无法完成的真实TLS/代理、凭据OS交互、实际模型和物理设备单列未验，不冒充通过。

本合同冻结常规技术选择，不自行扩大冻结C2/P4/P5。若实现证明必须突破其生命周期/额度/公开语义，停止相关部分并集中报告。


## 5. 实际接口及实现冻结（2026-10-29）

RP-A/B实现采用`include/ui_framework/ai_model.h`的model_open/configure/get_state、real_propose/continue和`ai_model_ui.h`的bind_light/bind_button/unbind。独立size/version=1结构；原v1/C2/P4/P5公开结构、错误数值和版本均不改。没有公开confirm、Key明文参数、Provider注册表、AI写入或图调度入口。旧submit仍只运行stub。模型字符串由用户配置，本轮测试仅用offline-fixture，不代选收费模型。

新增能力仍调用原ai_tool_begin→tool_requested→UI prepare→tool_bound确认→dispatch→稳定ID结果/forget→tool_result确认。整批预检仅校验封闭包装、白名单、目标和原Schema，不创建再遗忘尚未执行的临时P2调用。跨轮remote call ID按SHA-256关联，重复拒绝；单轮多调用完整校验后串行执行。发现包装首版page=0，内部复用原分类目录的最多16页聚合；只外发明确允许的read描述，next=0，不承诺远端自行翻页。

一次确认摘要绑定Service session、run ID、请求JCS摘要、profile revision、目标与目录描述指纹；目录/目标/版本变化要求新提案，不静默改绑。确认未保存时不得发送。provider_request提交确认后，另经UI重查目标/目录/期限和关闭门，再发起网络。正在进行的连接不能因确认到期提前终态/释放；仍等待真正退出。超时/取消的remote_processing_unknown与业务UNKNOWN分别表示，绝不重发POST或工具。

每个执行任务预留2MiB固定arena，计入原Service8MiB；缓存可按原规则回收已终态且无引用任务。JSON解析池最多512KiB；池不足明确失败，不能另建无上限回退堆。绑定最多16，分配前在Service锁内计账。UI完整批次验证与worker通过短锁发布阶段/引用，锁不跨应用回调；提交借用外层方法租约，绑定在received之前形成。网络回调只发布有限状态，数据由单个16KiB读缓冲串行交接，引用一直保留到HANDLE_CLOSING、回调退出和worker消费。

WinHTTP固定HTTPS、系统代理、证书默认校验；关闭重定向、自动认证、cookie、跨session全局池和失败连接重发。明确设置失败连接重试次数0；若系统不支持所需选项，返回no_retry_transport_unavailable，不能默默降级。没有ERROR_WINHTTP_RESEND_REQUEST重发分支。不承诺离线夹具已证明实际TLS/代理/服务兼容。官方依据：[WinHTTP异步返回合同](https://learn.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpreceiveresponse)、[选项及自动重试](https://learn.microsoft.com/en-us/windows/win32/winhttp/option-flags)。

配置后台读取/原子保存，损坏/未来格式封闭写入，不覆盖原配置。凭据只允许固定origin、自有路径派生引用；未配置/失效Key明确失败，不读环境Key或其他应用凭据。原生凭据输入后由后台CredWrite，成功同时保存新profile revision；临时明文/用户名清零。OS内部缓冲不承诺物理擦除。配置/Key不自动发请求。

Light面板保留stub、受控编辑、图诊断；真实模型区域独立折叠，输入不因刷新丢失。新提案仍使用原七字段/64KiB桥接，上下文选择封装在params中，不放宽宿主解析器。Key和外发确认没有page-message授权处理器，只能经私有匹配输入链；原生按钮同样拒绝BM_CLICK/程序消息和混合注入。生产来源正例仍延期。静态嵌入复用同一Service模块，没有第二聊天执行器。

## 6. 交付和验证边界

新证据根为build/xuanshu-rpab-20261029-6f38a1；详细结果以独立验收记录和原始JUnit为准。测试替身/接纳函数仅隔离测试构建；生产ui_framework.dll导出需核对无fixture/accept/http回调私有符号。协议夹具逐字节及每个两段切点验证中文、SSE和工具JSON；实际DLL/双workspace实例/隔离库证明read值与固定目标。专用子进程退出后恢复记录，明确不恢复确认、不联网、不重放。

这次交付只完成RP-A/B代码和离线合同验证。真实DeepSeek响应兼容性、真实WinHTTP回调/TLS/代理及凭据UI交互仍须RP-C和集中人工授权；不能由替身或公开模拟输入证明。历史期限风险、browser另案和P6冻结原记录不改。

确认前由后台从本库生成明确历史及当前输入的固定context_preview，正式面板分页展示实际内容，不能只显示序号。预留同一个2MiB arena，不增加额度；初始上下文超过原128KiB时本地拒绝且不发送。预览在Service锁下脱离后释放；worker在UI完整预检租约期间不操作同一arena，包括取消收尾。get_run分别投影确认收到/确认保存、请求保存、远端处理不确定状态，不以committed旧字段混称所有阶段已保存。


## 7. 2026-10-31两项限定整改（私有实现，无公共合同变更）

原P3/v1发现链继续返回根数组摘要，不更改公开发现接口、布局或错误数值。RP发现适配仅从同一目录的describe取得完整read描述，累计大小仍受原256KiB工具结果上限约束；RP投影按原数组读取，再按已确认能力白名单输出items/next。ID、能力版本、说明和输入输出Schema保持目录原值。无匹配数组可输出空目录；非数组、非法项或缺失必要描述字段明确失败，不能伪装正常空目录。未增加目录、业务handler或授权途径。

cleanup扫描在Service锁内选择候选，复制稳定run ID并取得临时扫描引用；解锁后才处理网络退出，处理结束锁内核对ID/对象并准确释放一次。扫描引用独立于网络引用，覆盖已取消终态及cache回收窗口。取消、存储不可用及关闭仍仅收尾，不恢复普通派发、不清除UNKNOWN；UI不等待worker或通知空间。身份不变量异常进入既有不可用状态并保留诊断，不错误归还其他slot引用。原64槽/8MiB、网络回调退出门、公共布局及固定依赖均不变。

专用扫描屏障和结果投影探针仅UI_AI_FIXTURE编译，新增探针仅隔离测试DLL导出，生产DLL无此入口。修前时序检测到候选复用时停止危险解引用，只取得有界失败证据，不宣称已复现崩溃、永久死锁或数据损坏。新增用例和实际验证以独立整改验收记录为准；不由本节自行宣称独立复审通过、正式冻结或RP-C授权。
