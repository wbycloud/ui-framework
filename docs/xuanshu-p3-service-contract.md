# XuanShu P3-1：宿主服务、stub与最小持久化合同

2026-10-24实现前冻结，承接P2开发阶段模拟验收。本文的P3范围已获本会话实现授权；整体架构继续Proposed。未实现状态只有在独立验证记录中有实际证据后才更新。无署名。

## 1. 单一所有者和阶段边界

一个workspace至多一个ui_ai_service，workspace内部持有非拥有型注册；Service未安全销毁时workspace_destroy拒绝。不是每个应用ui_host_t一个服务。Service持有一条SQLite中固定conversation_id的主对话，面板仅投影；切标签、折叠/刷新面板不取消、不重绑任务。

Service、SQLite、stub和应用同进程，遵守合作生命周期；越界内存、进程崩溃和不合作回调仍可能影响宿主，不承诺硬隔离。取消停滞继续可观察，不通过强杀线程或提前卸载DLL处理。

独立ai_service.h，不修改既有公开结构、v1、C2合同、错误数值、SDK0.10-dev/API10/标准10、ABI1/包1。公开范围：create/open/get_state/poll/quiesce/resume/request_close/destroy；submit/get_run/query_run/cancel/answer/history/continue。不导出Provider注册、审批、Coordinator、子Agent、AI写入或密钥接口。支持Windows native/static及现有Light framework_host；WebView2不在本轮。

静态供应必须闭合已有依赖：ui_framework纳入workspace.c，测试用headless静态库纳入workspace.c/package.c。嵌入调用者只链接公开库，不要求另编框架私有实现文件；ui_ai_service_test和同源ui_ai_embedded_test分别以headless/native库验证同一Service合同。旧显式编译workspace的维护夹具不因此新增第二份执行器，静态链接按已有符号解析。

## 2. 接纳、ID、状态与控制

config/request/snapshot/event完整size/version=1；所有公共方法在workspace所属UI线程。create只分配和登记，不I/O；open后台验证/恢复，READY前不接任务。输入字符串在返回前复制，目标完整复制为P2 v1 read target，最多16项。client_request_id为调用方预建1…64字节ASCII ID；run_id由conversation_id/client_request_id确定性SHA-256得出，建立关联后才能通知/执行。received可以同步早于submit返回，sequence=0/durability=unconfirmed；OK只表示内存收到，不等于保存。

worker确认SQLite接纳后发accepted（committed），才进入普通任务FIFO；同ID/同完整canonical请求返回原run，不重复执行；同ID异内容ALREADY_EXISTS。重启时同ID幂等也由数据库核对，不靠当前cache。普通新任务一次只运行一个，其余有界排队。

Service：CREATED→OPENING→READY；失败UNAVAILABLE；quiesce封接纳/请求取消但保留库；app REFUSE可resume恢复接纳，旧取消不复活；最终request_close→CLOSING→CLOSED。destroy仅CLOSED、worker实际退出且poll/回调/工具租约清空时OK，否则PENDING，对象保持有效。workspace_destroy原错误合同不变，服务存在时仍CANCELLED。

Run：RECEIVED→QUEUED（保存确认）→RUNNING；WAITING_INPUT/WAITING_TOOL→RUNNING；CANCEL_REQUESTED/CANCELLATION_STALLED为非终态；执行上下文/工具清理后FINISHED，outcome唯一completed/failed/cancelled/limit。存储确认失败进入UNCONFIRMED，不能伪造持久终态。5秒仅停滞提示，不强杀/自动释放。cancel通过run ID直接控制，不排在普通队列后；answer通过run/question/operation ID直接控制，绝不从自由文本推断审批。answer≤16KiB，同操作ID同值幂等、异值拒绝。

callbacks只借用至返回；可get_state/get_run/cancel/answer/quiesce，嵌套poll或新普通请求受背压，不递归无限通知。关闭和取消不需要普通队列空间。返回错误不隐式改绑/排队重试业务。

## 3. 独立安全上限

沿用Proposed根步骤64、模型续轮16、工具调用64，P3只是单任务安全上限，不实现P5共享Agent预算。stub无真实token计算承诺，按UTF-8字节/操作数限额。输入16KiB、脚本64KiB，复制目标和上下文合计128KiB；最终/流式合并回复64KiB。16个排队任务（另1活动），64个有界cache槽；终态无借用槽可逐出，所有历史仍可异步查询。

普通命令32项，任务/cache/命令内存共8MiB；控制预留256KiB单独分配；worker→UI通知64项/2MiB；单个查询结果最多512KiB；工具邮箱一个、上限2MiB，输入沿P2 64KiB、输出256KiB，Service独立记账，不挤占或提高P2额度。history分页1…32项，遇字节上限返回较短页及明确next，不截断单条消息。

流式展示允许合并，权威partial在Service中可查询；最终、存储失败及终态通知不得静默丢弃，满时worker保留状态等待poll排空。UI不等待队列空间或join。回调期间普通请求背压可观察，输入保留到persisted accepted，不能收到received就清空。

## 4. 后台执行与只读Agent

采用一个按需启动的串行后台执行通道，负责SQLite和stub状态机；空闲/等待UI或问题时可退栈退出，UI仅零时检查线程已退出后关闭句柄/启动下一worker。SQLite serialized connection在这些互斥worker之间顺序使用，UI绝不访问库。Provider是内部stub运行/分片/取消/清理合同，无真实网络、连接、凭据。

stub明确标为测试模型。默认只回显；高级脚本显式给出text、discover、tool、ask、fail步骤，禁止用关键词路由假称自然语言理解。最多64步骤；每次模型启动/续轮及工具各记对应安全计数。脚本及工具参数经严格JSON/JCS完整聚合后校验，tool支持显式参数碎片用于测试；缺少终结、非法JSON、重复tool_call_id或超限在派发前失败。

UI执行目录分类分页、describe、绑定和应用回调；只使用P2 read目录/prepare/dispatch/result/forget，不调用旧Assistant/普通修改入口。目录输出也过滤非read。目标在submit时复制固定实例、资源、host/open epoch、revision和read scope；普通提交先验证形状和scope，允许持久化幂等请求携带上一进程目标完成身份核对。新执行在发现/派发前重新核对，绝不改成最新活动标签；显式continue还在提交时要求当前重新bind的完整目标。工具请求先存储选中目标/关联，P2 prepare建立稳定call后再存储绑定确认并发布tool_bound，确认后才dispatch。同步complete可以早于dispatch返回；异步只按ID查询，不保留应用裸指针。

read调用运行时取消只请求P2取消，等待真实完成与forget租约释放；关闭中的实例拒绝新派发，在途调用保持P2卸载门。后台只持Service数据、稳定ID和复制字符串，不持ui_host/组件/文档指针。数据结果复制后仍须排空P2租约才允许清理。

## 5. SQLite固定供应和原始库保护

选定SQLite3.54.1，官方2026/sqlite-amalgamation-3540100.zip；source ID `2026-10-16 14:45:20 555e07ef33f69a0957d24d8e555b702464a131d6f1607f4c3c1e4551e8e6ca76`。官方压缩包SHA3-256 `92281c86e2957f9b9305890f9ac04204caa7b677cc868dc326d942689df28e30`；文件SHA-256、来源和完整版权页见third_party/sqlite/source-lock.json。公有领域声明保留，不购买/伪造许可证。只编译sqlite3.c，不编译shell/网络/扩展；THREADSAFE=1、OMIT_LOAD_EXTENSION，离线CMake逐文件哈希门，无构建时下载。其他固定依赖不改。

新库schema1，固定application_id，包含conversation、runs、messages、tools、events。事件sequence为SQLite单调主键，operation_id唯一，终态部分唯一索引；用户消息/接纳、结果/最终消息/唯一终态在同一事务提交。commit返回前不通知committed。流式片段为非权威展示；最终消息不能只留在展示队列。持久化不保存应用业务文档，不导入参考工程库。

工具请求/绑定事务同时保存runs的WAITING_TOOL；工具结果事务同步恢复RUNNING或当时取消/停滞阶段。不能只有tools表和内存知道在途，而让已保存run查询一直冒充其他阶段；专用回归在真实异步read尚未完成时查询数据库投影，要求phase=WAITING_TOOL且committed=true。

实现细化：在验证库归属/schema/完整性后使用WAL＋synchronous=FULL，关闭执行checkpoint/flush；不把减弱同步作为排队吞吐修复。首轮控制压力运行使用默认rollback journal持续推进但未在既有5秒有界断言内排空，原库/热journal和匹配产物保留；不延长断言或20秒测试超时。WAL选择须经本轮独立重验，不因此宣称旧workspace超时已定位。

现有非本应用库、空/损坏文件、quick_check失败或未来schema均拒绝，不删除/重建/覆盖原库冒充成功。独占逻辑库所有者，busy冲突后台有界返回错误。最小事务幂等核对、checkpoint/flush/close全部后台；错误保留明确UNCONFIRMED/UNAVAILABLE及可查询信息。宿主首次退出遇未确认存储失败先显示错误，不静默销毁；后续明确关闭才可放弃内存展示。

打开恢复历史和未完成run为NEEDS_REVIEW，不自动排队/工具重放。continue显式创建关联新run，必须传新的client ID、当前明确目标重新bind的target及显式脚本；后台核对原run存在、非终态且NEEDS_REVIEW。旧instance/open epoch不能复用。原记录保留，不把继续伪装成原工具幂等重放。历史页返回items/last/next/has_more：next=0表示本页当时EOF，新增记录可用last继续；query快照可committed=true但sequence=0，不代表新建了权威事件。分页按实际JSON编码字节检查，不将SQL读取失败伪装成EOF。

## 6. 面板、退出与验收

framework_host只创建一个Service，新增全局玄枢主对话/stream/queue/cancel/question/history/error区；原命令诊断保留。普通文本默认stub回显，读取/发现需显式测试脚本/目标，UI不会从文本推断授权。折叠仅隐藏，重建后从Service和SQLite投影。退出先quiesce并close_all，REFUSE恢复接纳但不复活旧任务；应用关闭后最终关闭Service、等待worker退出和回调退栈，再销毁workspace/chrome。

实现细化：存储打开尚未完成时REFUSE后的resume回到OPENING，不能提前进入READY或使用空conversation_id。默认路径为LOCALAPPDATA/ui-framework/xuanshu/conversation.sqlite，可显式配置UI_AI_DB_PATH；父目录创建也在后台。现有UI_HOST_TEST夹具无显式隔离路径时不打开用户库，P3面板夹具显式传临时路径，调用同一生产Service；没有测试授权导出。Light页面的输入/脚本及编码后的桥接消息各校验上限，输入只在匹配client的持久接纳确认后清空；普通提交拒绝、桥接超限或存储失败保留草稿。任务记录可选择、分页与显式继续；命令诊断保留独立区域，不作为Service工具执行路径。

专用隔离临时DB/子进程测试：单Service、关联先于回调、FIFO/控制/限额、stream UTF-8、碎片工具、实际read与修改拒绝、target/revision/关闭重开、cancel/stalled/backpressure、存储确认/唯一终态/幂等、跨进程历史不自动执行、锁/失败注入/损坏/未来schema、真实DLL在途卸载及UI非阻塞。私有故障/时钟钩子只编入静态测试，不能从生产DLL导出。

P2充分匹配证据复用，相称公共路径回归另记。首次失败及匹配二进制保留；旧输入/浏览器20/20.05秒超时各独立，不归因环境。物理来源、Windows动态正例、IME/DPI、视觉及长时按原延期清单留后续。完成终点是P3一轮开发交付与代码评审，不自动P4。
