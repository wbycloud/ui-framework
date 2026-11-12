# RP-C首次真实DeepSeek服务限定验证记录

> 2026-11-02用户纠正（保留原记录）：用户没有看到可实际输入Key的界面，聊天“已录入”不应解释为已完成原生录入或保存。原文已区分反馈与证据，这里进一步明确不能归因用户未配置/Key无效或厂商拒绝。旧M1/R1/S0/C0/U0及原失败事实不变。随后用户另授权[凭据/体验整改](xuanshu-rpc-credential-validation.md)及[独立新一轮验证](xuanshu-rpc-real-service-validation-round2.md)，不是本轮退款/重试，不覆盖本记录。

2026-11-02。最终状态：T1在生产路径发送前以credential_unavailable失败，后继请求已停止；用户正常关窗后进程退出，隔离库只读对账完成。RP-C首次真实服务限定验证未通过，停止在本轮结论位置。没有重试或执行T2/T3，没有产品修复、重新构建或新增功能。

## 1. 基线与证据位置

分支`codex/p1-process-boundaries`，HEAD `e0eca03db25930be55bdb45477027d1793a61904`。准备交付根`build/xuanshu-rpc-prep-closeout-20261031-f27a64`，delivery-identity SHA-256 `D10E28A6AB8DE63FC854518F75C67B8803E41708701084083691D4A0B32459B7`。匹配程序和冻结依据见[独立复审追记](xuanshu-rpc-preparation-independent-review.md)。本轮仅核对身份，不重跑离线测试。

本轮新证据根`build/xuanshu-rpc-real-20261102-b5e731`，preflight.json保存文档修改前身份核对；document-baseline保存7份获准既有文档的原文。本轮只写9份授权文档及必要验证证据，原交付根不改。参考工程README曾可读，本轮Git访问仍Permission denied，未扩权或修改。

计划新会话目录为上述证据根的`session`，启动器自行创建；库`session/conversation.sqlite`，模型配置`session/conversation.sqlite.model.json`，凭据引用由模型配置路径派生。不开第二会话重置额度，不修改LOCALAPPDATA或用户默认配置。

## 2. 官方协议及费用核对

2026-11-02只读查看[官方Responses指南](https://api-docs.deepseek.com/guides/responses_api/)、[接口](https://api-docs.deepseek.com/api/create-response/)及[价格页](https://api-docs.deepseek.com/quick_start/pricing/)，没有访问模型、模型列表、余额或账户API。官方仍列deepseek-flash、POST /responses、reasoning.effort=none、function配对和语义SSE终态，与冻结生产路径未见实质冲突。无服务端会话续接，续轮由客户端发送本任务配对上下文。

价格页当前Flash峰时输入缓存未命中USD0.30/百万token、输出USD1.20/百万token，与启动器300/1200纳美元参考费率一致；以峰时保守口径，不依赖缓存或谷时优惠。USD0.10是本地停止阈值，不是服务端硬扣费保障或实时账单。有效usage计价部分、保留估算和unknown分开，缺失usage不按0处理。资料核对不证明账号可用性、TLS/代理或真实协议兼容。

## 3. 已授权范围和控制

仅`POST https://api.deepseek.com/responses`，deepseek-flash、非思考、stream、max_output_tokens=4096。单Service/主对话、同一目录及read执行链；不开放P4/P5写入、审批、授权、图或legacy修改。

| 任务 | 准确输入/数据 | 最大POST | 当前结果 |
|---|---|---:|---|
| T1 | `只用中文回复“玄枢流式验证成功”。不要调用工具。`；无目标 | 1 | 已确认/保存；发送前credential_unavailable失败，终态failed |
| T2 | `请先发现target-0的memory类只读能力，再调用memory.read读取数值与版本，用一句中文报告；不得修改或调用其他能力。`；memory.document、target-0、仅memory.read描述/Schema及value=10/revision=r1 | 3（发现、read及续轮合计） | 未执行，因T1失败停止 |
| T3 | `请用中文解释什么是流式输出，最多三句；不要调用工具。`；无目标 | 1 | 未执行，因T1失败停止 |

每任务history=[]；固定系统指令及xs_read_discover/xs_read_call两个read包装定义均在确认范围，不发业务文件、其他应用快照或完整主对话。T2目标epoch/revision/scope固定，不随标签变化改绑。详[原数据规格](../xuanshu-rpc-validation-plan.md)。

总额5，分配1/3/1不挪用、不返还、不自动重试；失败、取消及发送未知仍占消费额度。每项前一步成功、费用/额度/期限可用才由用户确认下一项。首次可信确认起10分钟停新派发并请求取消；保持网络10/30/30/120秒，安全收尾等待真实连接/worker/回调退出。取消若晚于完成，只记录未覆盖在途窗口，不追加请求。

## 4. 操作与计数记录

启动前M/R/S/C/U均无发生事实，程序不得自动提交。启动后将核对新库rpc_open、零runs/零provider_request/零rpc_reserved/零rpc_send_entered，及显式配置路径/ref。仅程序可为获准请求读取用户新录入的隔离凭据；助手不提取Key、不枚举或读取其他凭据、不取环境Key。用户点击Native Key后只在原生密码框输入Key，后续每任务自己检查预览并点击Confirm scope。

分别登记manifest M、消费R、WinHttpSendRequest进入S、完整响应C、发送未知U，模拟计数另列。S在API返回后登记，在途/ack未保存时按合同标不确定；model_calls不是精确HTTP次数。本地接纳、保存确认、发送、远端未知、工具事实、最终消息和唯一终态保存分别核对。

| 阶段 | M | R | S | C | U | 状态 |
|---|---:|---:|---:|---:|---:|---|
| 启动前 | 0 | 0 | 0 | 0 | 0 | 未启动，无请求 |
| T1用户截图 | 1 | 1 | 0 | 0 | 0 | stopped=true；failed/terminal/committed；工具0；原额度不退还 |
| 关后持久记录 | 1 | 1 | 0 | 0 | 0 | 唯一T1/唯一失败终态；无send_intent、无tool、无usage；无第二次提交 |

后续记录只填写实际观察事实。脱敏证据仅含合成输入/输出、稳定ID、hash、时间、HTTP/usage、业务值、存储和退出事实；不保存Authorization、Key、完整内存或其他应用数据。读库仅限本轮隔离库的只读查询，不运行既有recovery或历史证据脚本。

2026-11-02 10:16:57+08:00，已使用匹配Light程序启动，PID3056，launch.json记录准确路径/参数。进程只读检查显示Responding=true，窗口存在；启动未自动提交任务。标题仍含冻结源码的旧字样“NOT authorized to run now”，仅为历史提示，本轮用户授权单独记录，不改程序文本。

新目录库和显式model配置已生成，配置revision=1、model=deepseek-flash，ref尾部`7a4ec2cd8f83c17b1bb8e4834325d956923c8ed820fbe52d6f0b82fb5dae7ccc`与配置路径哈希一致（不含Key）。初次外部mode=ro查询遇database is locked；源码ai_store.inc固定locking_mode=EXCLUSIVE，这是外部读取受限事实，不归为产品失败、数据丢失或新的网络请求。没有绕过锁、复制活动库冒充一致快照或修改存储模式。在线Policy零计数由用户界面核对，关窗安全退出后再只读核对持久记录。

已一次给出当前原生录入步骤：先确认Policy五项均0，点击Native Key，在Windows原生密码框录入，待pending=false/ready=true/error空；暂不提交或确认T1/T2/T3。助手不接触密码框/Key，不用程序消息或模拟输入操作。此处仍等待用户完成回报，不把原生来源正例/凭据保存写成已通过。

2026-11-02约10:21+08:00，用户回复“已录入”。这仅记录用户完成录入的反馈，未提供五项计数或配置状态，助手没有读取凭据。已给出T1步骤，要求在ready/pending/error及五项零计数满足时才提交，核对准确预览并亲自Confirm scope一次；等待T1终态、保存、费用及M/R/S/C/U结果，未推进T2/T3。未将用户简短反馈推断为全部来源识别或凭据保存验收通过。

约10:25+08:00取得用户主窗口截图，保存t1-user-main-window.png及SHA-256 `0A6955DE783EF7DA94172B7BDE54DB4E528A986037B6A2CD08E87EB198FF4A5A`。截图没有凭据窗口或Key。用户此前反映主窗口无法复制，故改以截图取证，没有修改刷新/输入或来源代码。不能将复制问题归因用户操作。

截图Task为client=rpc-task-101-1，phase=7、terminal=true、committed=true、outcome=failed、error=credential_unavailable、remote_delivery=not_sent；disclosure_confirmed/disclosure_saved/model_request_saved均true，model_calls=1、tool_calls=0。Policy为manifest=1、reserved=1、send_entries=0、complete=0、unknown=0、task_used=[1,0,0]、stopped=true，错误validation_network_or_send_unknown；模拟计数均0，send_count_known/send_count_saved均true。这不把一个已保存manifest写成一次已发送HTTP。

known_nano_usd=0、estimated_nano_usd=5582700（USD0.0055827保留估算）、fee_unknown=false，是截图的本地状态，不能称实际产生费用或确认服务端账单为0。没有完整远端响应或usage。源码am_net_start在am_secret失败后、WinHttpOpen/SendRequest之前退出；该错误可能来自隔离凭据读取或格式检查，不是DeepSeek拒绝Key/HTTP401，也不能证明用户没有录入。程序未保存细分系统错误，根因待核实，不枚举/读取凭据进一步探测。Model配置仍revision=1、ready=true、pending=false、error空、host_status=0，单凭这些不能证明Key实际保存成功。

已立即停止后继任务，不重试、不退款/挪用R、不新建目录续跑。用户随后回复“窗口已关闭”，2026-11-02 10:28+08:00只读进程检查确认PID3056不存在，无强制结束操作。隔离目录当时仅剩61,440字节库和165字节模型配置，启动stdout/stderr为空；没有从空日志推断成功。

关后仅用sqlite3的mode=ro读取本轮隔离库，没有重启Service或调用recovery。conversation一条；user_version实际为1，本轮没有迁移。run_id=`4caa342e4e00af636729b82503b18e2fc37938c398cc61da0e9ccb25eaa0cf52`，provider_request_id=`8f6d49ffb3e32a9f7c0397bbf8d40b1838acddb5b2b6c9e5182e4c043855e353`，ordinal=1；provider_request与rpc_reserved的body_hash同为`4b83f2e3fb7ce59ada32894e3dc64813bf1bba8926dd3091581ca36446257ef1`。该摘要对应本地待发送内容，不是已外发的证明。

事件严格为rpc_open→accepted→awaiting_disclosure→disclosure→provider_request→rpc_reserved→rpc_network_end→finished，各一次，op_id唯一；网络结束记录complete=false/unknown=false。没有rpc_send_intent、rpc_send_entered、rpc_cost或tools记录，没有HTTP状态、远端response ID或usage。最终failed事件含committed=true及credential_unavailable；messages保存一条合成user输入和一条内容为空的最终assistant记录，空记录不是模型回复。targets和history均[]。这证明本地接纳/确认/请求manifest/配额消费/失败终态已保存，不证明真实文本流、真实工具结果或远端成功。

只读查询前后库SHA-256均`F4650A99650CA9E6A97499182445FC7EA1AA81198E1CDC5D32B646D37554C067`。关窗后的首次目录检查没有WAL/SHM；外部只读SQLite核对后出现空WAL（0字节）及32KiB SHM，是本轮取证连接的辅助文件现象，不能倒记为产品未关闭或未刷库；连同隔离库保留，不擅删。完整查询结果及检查在closed-db-observation.json，未操作用户原库。

## 5. 停止和未验

配额/期限耗尽、费用达阈值或必要费用依据不可核对、越界外发、重复请求、目标/epoch/revision不符、存储失败/确认不明或关闭异常均停止后继发送并cleanup-only；不强杀、不提前卸载、不清UNKNOWN、不临时改代码/协议/模型。失败仅给最小整改建议。

本轮T1发送前失败，不能给出RP-C通过结论。真实流式文本、DeepSeek/TLS/代理兼容、真实发现/read10/r1、取消在途均未覆盖，Key保存可用性亦未证明；没有追加请求制造覆盖。无冷重开执行计划，不通过第二验证目录继续额度。用户原生操作是当前限定验证步骤，不代表全部设备/来源、IME、DPI、视觉、辅助技术、物理断电或长时项目验收；这些仍按原清单延期。

历史期限/frontend/输入/browser首败继续独立保留，不默认归因环境、不被本轮结果覆盖。没有清理历史或自动删除隔离凭据/验证库/必要证据。

## 6. 结论、保全与下一步

准备整改的独立验收与匹配快照冻结已落盘，不是本轮重测或重新完整复审。收尾再次核对399源码、122最终程序/链接/PDB、27red归档、8项依赖及原identity关联18份清单/43份证据全部匹配。三份根规划原始前缀保持；本轮只改9份获准文档和必要新证据，没有代码、测试、构建、版本、依赖、SDK或历史证据变化。

本轮首次真实服务验证未通过，原因是生产路径未能取得符合要求的本轮隔离凭据，尚未进入任何真实SendRequest；费用保留估算不充作实际账单，也不把无usage记作0 token。配额R=1永久计入本轮，剩余额度不据此续跑。进程正常退出及失败持久化已核对；真实网络/worker在途取消和退出路径没有被本次失败覆盖，不外推为完整关闭验证。

最小后续建议是另行授权限定排查“原生录入返回→可信事件绑定→凭据写入job及保存确认→同origin/ref读取”的链路，增加不含Key的分阶段状态和系统错误诊断；现有ready/host_status不应作为凭据可用证明。用户已报告窗口无法复制，宜同时评估稳定可复制的脱敏诊断投影。当前没有证据区分未保存、读取失败或格式拒绝，不归咎用户、账号或厂商，不读取密钥求证。本轮不实施上述建议；需要改代码/构建或新真实尝试均另授权，不复用当前封闭任务自动重试。

本轮证据根保留preflight、launch、用户主窗口截图/哈希、用户步骤记录、closed-db-observation、final-preservation、文档增量清单及完整隔离数据。没有创建新的编译/解压/截图临时副本等无用产物；不清历史、凭据或验证库。真实设备/来源、IME/DPI/视觉/辅助技术/物理断电/人工长时及browser另案保持原边界。停止本轮真实验证，不进入其他阶段。
