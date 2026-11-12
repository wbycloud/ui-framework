# UH-A集中人工第二批：本地任务、取消与记录恢复

2026-11-04跨标签补测最终通过：同冻结生产web_counter只作被动合成第二标签；用户截图证明RP-C→Web Counter #2→RP-C，两个实时诊断task/model逐值一致，run/client/instance/host-open epoch/resource/r1/read scope及字段不变，仅展示代次21→23。返回后取消，PID13776实际exit0；新库1任务/2消息/3事件/0工具，唯一cancelled/committed终态，无确认/请求保存/工具派发；DB SHA25697886179…8B3BC0D，immutable前后不变、无WAL/SHM。承接此前第二批两次exit0及历史选择/取消/恢复通过证据，集中人工第二批零Key、零联网限定验收通过。不是全程抓包、真实Provider或系统完整验收。 该结论来自独立新补测，原本轮“部分完成/跨标签未覆盖”段落按当时事实保留，不回写当时结果。详[跨标签记录](xuanshu-uha-cross-tab-validation.md)。


2026-11-04最终：本地合成历史、结构化范围选择、未确认模型提案、主动取消及同库记录恢复分项通过；第二批仍有跨标签固定目标场景未覆盖，不报告整批全部通过。两次正式宿主均由用户正常关闭，实际exit0；恢复前后库及配置哈希相同，未新增任务、消息、事件或工具。本轮只做必要文档、只读身份核对和已授权的生产宿主人工验证，没有修改产品、构建或运行离线测试，没有读取凭据或调用Provider。无子代理、computer use、程序输入或私有可信事件。以下准备、等待状态均按当时历史保留，以本段及最终结果为准。

## 身份与隔离

沿用已独立限定通过并冻结的build/xuanshu-uha-visibility-20261103-e57b29，identity `FF84C151A97784832D932AAE72F55BF9107DDE7E81DFF97F006FF41887664AB0`；417源码、153程序和21项身份关联清单匹配。HEAD仍为e0eca03db25930be55bdb45477027d1793a61904。当前6份旧文档编辑前副本/哈希和Git状态保存，不把既有全部修改归于本轮。参考工程README可读，仅只读；原证据和SDK不修改。

本轮唯一新根`build/xuanshu-uha-manual-second-20261104-89bc95`，显式数据库`session/conversation.sqlite`，配置精确派生`session/conversation.sqlite.model.json`；启动前二者不存在。不操作前两轮人工库，不迁移用户库、复制旧ref或改变LOCALAPPDATA。普通宿主最近应用列表仍走其原系统路径，隔离AI数据库不等于隔离所有普通宿主偏好；本轮不额外读取/修改该列表。

匹配程序：生产light/framework_host.exe、ui_framework.dll及生产链接rpc_memory.uapp；精确SHA256见本轮preflight.json。模型名offline-fixture仅为本地合成配置，不是传输替身，更不代表允许联网；ready不能表示凭据可用或认证成功。

## 准备时的已有覆盖、本轮补项与限制

| 项目 | 已有覆盖 | 本轮补项或未覆盖边界 |
| --- | --- | --- |
| 显隐、未提交草稿、明确空状态及编辑 | 最近短复核已通过，PID80044 exit0，库0任务/事件/消息/工具 | 不单独重复，仅自然涉及 |
| 历史选择及默认不选 | 旧完整人工未完成 | 用两个最小stub回显任务生成A/B；只选user A，不选assistant回显或任何B |
| 本地模型接纳、保存及完整预览 | 原第一批曾覆盖无历史提案，后因显隐问题整体暂不通过 | 本次独立验证A＋当前输入＋memory固定目标/read字段，查看全部初始request，不确认 |
| 实际标签切换 | 既有离线双实例证据，不是本轮人工 | 指定rpc_memory包multiple_instances=false，仅有一个可用标签；＋为打开包入口。不得引入其他包/数据、伪造第二标签，跨标签暂列未覆盖 |
| 主动取消及唯一终态 | 旧任务随关闭取消 | 本次点“请求取消”，等待cancelled及最终保存，再核对库 |
| 同库恢复及历史诊断 | 尚无该人工闭环 | 第一次正常退出和关后核对后仅重开同库一次，选历史任务，核对persisted诊断、无授权恢复/联网/重放 |

不因当前缺少标签入口而增加产品功能或将已有自动结果写成本轮人工通过；最终须分别报告实际补齐、未覆盖及其对整批结论的影响。

## 现有控件与安全顺序

依据匹配host.html及ai_panel.inc：顶部“AI”打开工作台；常驻“任务输入（填写要本地保存的任务内容）”对应ai-input；“stub可选资源ID（诊断用；模型目标在下方选择）”保持空；“stub诊断脚本（测试模型，不是自然语言）”保持默认`{"steps":[]}`。旧submit仍为本地stub空steps回显，不请求Provider、不派发工具。A/B分别通过“提交stub任务”一次提交，等待completed，不能将点击/清空输入冒充最终保存。

A为“UH-A第二批合成历史A：允许用于本地范围预览。”；B为“UH-A第二批合成历史B：本次不选择。”。生产保存会同时形成user及assistant消息；结构化选择只选择user A，不能默认把它的assistant回显或B一起选中。

“DeepSeek 官方 Responses · 用户配置／外发确认 ▸”内用offline-fixture并点“保存用户模型配置”；不触碰“原生窗口设置密钥”。在结构化历史中只选A；“选择当前应用的只读目标与能力”→memory.document→memory.read→revision、value。输入“UH-A第二批本地验收：仅预览已选历史与合成内存只读范围，不发送到模型。”，只点“保存真实模型任务提案”。查看当前输入、已选历史、固定instance/host epoch/open epoch/resource/r1/scope、read能力/字段，以及完整instructions/tools。禁止“确认此任务外发范围（一次）”、显式继续及编辑/图诊断。

等待提案持久化且未确认，再使用“请求取消”；取消请求不等于终态保存。画面状态区具有请求保存、远端及最终保存区分，确需稳定ID/精确字段时仅用“复制脱敏诊断（不授权、不发送）”，不要求手抄JSON。用户每次只收到当前一步，截图仅按需；助手不读在线数据库、密码框或进程内存。

## 监督、关闭和恢复

最多一次初始启动及一次有前置门的同库恢复启动，不循环重试。监督分别保存PID、stdout/stderr、心跳、实际退出码；窗口稳定不代表所有控件正常。提前退出保留并停止。第一次必须用户正常关闭并确认实际退出，再以mode=ro&immutable=1读本轮自有库；有未合并WAL时不忽略、不checkpoint或改写，停止相关恢复并说明。

依实际接纳数核对runs/messages/events/tools、唯一终态和所选历史/完整固定目标。若计划完整执行，应有两个stub完成任务和一个未确认模型取消任务，最终数量以库为准。确认/模型请求事件与模型工具应无；stub的model_calls是本地模拟步骤，不能当成HTTP次数。

第一次证据完成后，只重开同一第二批数据库一次。由用户点击“任务记录”并选择对应输入的历史任务，复用异步query核对run_id、来源persisted、终态/保存/error/error_recorded和历史固定目标；不得自动改绑新host/open epoch，不重新提案或继续。之后再次用户正常关闭，对比两次库中任务/消息/事件/工具，判断无重复/重放。两次退出码均待实际退出，不能预填。

## 证据边界与停止

用户观察、软件投影/脱敏诊断与关后SQLite分别记录。无全程网络遥测，不冒充抓包或精确HTTP次数；普通宿主无RPC配额策略，不生成M/R/S/C/U。没有读取Windows凭据来证明零Key，依据禁止入口、隔离配置、操作和未进入模型前置门；保存配置不认证Key。

出现显示/输入异常先保留只读定位；确认资格、错误目标、越界内容、意外任务/发送、存储或关闭异常则停止相关步骤，不修产品、不强杀、不提前卸载、不清UNKNOWN。原人工失败、原进程退出码未知、首red BMP限制及ASAN20.02/30.08秒等历史风险保留，不默认环境或用新通过覆盖。

IME/DPI/完整Windows来源分类/辅助技术/预定义人工长时继续延期，断电仅专用环境方案，browser九副作用另案。当前只准备并等待第一步；没有证据前不报告整批通过。完成后停在本轮结论，不进入有Key、真实调用或其他阶段。根三规划只追加；新文档状态不冒充原冻结测试时身份。


## 初始启动历史与当时等待

2026-11-04本轮初始启动已进行：正式宿主PID61528，监督工具会话12024持续持有子进程句柄；实际加载生产ui_framework.dll匹配E0ECEF43…6ADBE5。正常用户上下文10:02:20只读观察标题“C 应用框架 · Web”和稳定窗口；不以Responding替代控件健康。初始退出码仍待实际正常关闭，恢复启动未执行。新库由程序创建，不在线读库，模型配置尚未创建。

当前仅引导第1步点击顶部AI打开工作台，等待用户反馈。没有提交合成历史、保存配置/提案、录入Key或外发确认。已完成所有不依赖人工的准备，后续每步待实际反馈推进；不因为等待而自动重启。


## 第一次关闭前的实际结果与同库恢复前置核对

用户截图分别显示A/B本地stub completed，默认4项历史均未选；保存offline-fixture后配置代次1、凭据仍未确认可用；只选user A、memory.document、memory.read、revision/value。模型提案run=700a51c2d5f97bc0e7ad404b18585c5b78e236e3500dd21b6bfffa3d257d8921，诊断committed=true/phase3/display_valid=true，history=[1]、model_calls/tool_calls=0、所有外发确认和请求保存标记false。固定实例1，host_epoch=8d015ae4-87ef-49f2-8d48-10a3663cf8b6，open_epoch=eb1b32de-8268-4651-9e7e-0308d3683857，memory.document/r1/read scope；仅A＋投影＋当前输入，B和assistant回显不进入context_preview。

完整预览分块已显示；用户寻找范围摘要/英文项困难，经定位version框提供第一块文本。保留user-preview-first-block.txt，已核对instructions、实际选取内容、discover完整page0 schema及两个工具名；片段从version开始、结尾在xs_read_call描述中间，不补造另一块、不冒充全文导出或网络抓包。软件display_valid及两块截图支持当前完整显示资格，第二块全文未独立复制，易用性观察与证据限制另列。

用户点击请求取消后截图显示cancelled，并按指引正常关闭。初始PID61528监督实际exit0（10:40:51，2344.953秒），无强杀/自动重启；61528及监督35824均随后不存在。关后仅本轮库mode=ro&immutable=1读取，无WAL/SHM、前后SHA256一致872BAEAAFFDBC82BABC7222747E4C1E19B3ADDE24BAB50417A37BCA26BDB978F。实际3任务/6消息/9事件/0工具，每run一个唯一终态，A/B completed、模型cancelled。模型的持久化final仍committed=true、确认/请求保存false、model_calls/tool_calls=0、not_sent；范围和fixed_targets与取消前诊断逐值一致。没有disclosure/provider请求/响应事件。配置仅合成model/ref/revision，无Key；未调用任何凭据API。

第一次关闭核对已完成，before-restore保留同库及配置副本；restore-ready.json只允许已授权的同库重开一次，不创建新库/重置。恢复启动及历史诊断实际结果仍待执行，不能把第一次关后通过当成整批完成。


## 唯一恢复启动历史与当时等待

已按前置门重开同一库，PID64548，监督会话90761。10:43:57正常用户上下文窗口稳定、生产DLL匹配；不在线读库。仅待第14步点击AI观察历史记录，未重新提交/确认，第二次实际退出码待关闭。已用完本轮一次恢复启动，不自动再试。


## 最终结果矩阵

| 补项 | 实际结果 | 支持证据与边界 |
| --- | --- | --- |
| 合成历史及默认不选 | 通过 | A/B各一项stub completed，用户/助手共4条历史；截图默认全不选，实际只选择user A（sequence1），不包含assistant回显或B |
| 本地模型配置与提案保存 | 通过 | offline-fixture/revision1，凭据not_configured；提案phase3/committed=true，事件accepted→awaiting_disclosure，无确认保存或请求保存 |
| 输入、范围、指令及工具预览 | 显示与核心内容核对通过，全文取证限制保留 | 截图为两块，宿主诊断display_valid=true；用户首块含完整固定instructions、A＋投影＋当前输入、固定目标、discover page0 schema及两个read工具名。另一块全文未独立复制，不能说已归档全部原始预览或网络抓包；用户对摘要/分块定位困难另记易用性问题 |
| 固定目标跨标签 | 未覆盖 | 本轮唯一允许的rpc_memory生产包为multiple_instances=false，没有第二个获准应用/标签；不把取消前后/重开记录相同冒充跨标签动态正例，不引入其他应用/私有接口 |
| 未确认任务主动取消及保存 | 通过 | 用户截图cancelled；关后唯一finished终态terminal1/committed=true，error空，原确认及请求保存均false，model_calls/tool_calls0、remote_delivery=not_sent |
| 正常关闭及同库恢复 | 通过 | 初始61528 exit0；唯一恢复64548 exit0。两次关后仅本轮库immutable只读，记录及整库/配置文件哈希不变，无新任务/消息/事件/工具 |
| 持久化历史诊断复制 | 通过 | 恢复后选中原run，task_source=persisted、query_id=host-run-1、terminal1/committed=true/error_recorded=true，非task:null；run/client、scope及原target字段与第一次库逐值一致 |
| 不恢复授权、不自动模型/工具重放 | 本轮证据支持 | 恢复后仅历史查询/复制，无新业务事件；task_source=persisted不是可执行任务恢复。display_valid只表示宿主状态投影，不签发外发资格；模型配置ready不证明凭据可读/认证有效 |

本次没有发现新的任务取消、存储或关闭产品故障；整批结论的剩余缺口是跨标签覆盖条件，并非已发现错误目标。按用户“不靠缩小范围宣称全部通过”的要求，明确为部分完成，不报告“集中人工第二批零Key、零联网限定验收通过”。已完成分项可以复用，不需为了获得整批措辞重做A/B、取消或恢复。

## 两次退出与关后记录

| 启动 | 宿主/监督PID | 启动与退出（Asia/Shanghai） | 实际结果 |
| --- | --- | --- | --- |
| 初始 | 61528 / 35824 | 10:01:46.358418 → 10:40:51.309906 | exit0，2344.953秒；用户“已关闭”，监督未请求强制终止 |
| 唯一同库恢复 | 64548 / 43664 | 10:43:29.073539 → 10:47:31.534799 | exit0，242.469秒；用户“已关闭”，无再次启动 |

两次均确认对应宿主和监督进程随后不存在。stdout/stderr各为空，不把空日志冒充详细生命周期日志；存活时长不是预定义人工长时验收。实际退出码来自监督子进程句柄，用户反馈与软件证据分开保存。

关后数据库同为61440字节、schema1、conversation1、runs3、messages6、events9、tools0。两个stub各有accepted/running/finished，模型任务为accepted/awaiting_disclosure/finished；每run只有一个唯一终态，模型任务的assistant最终消息为空。未把SQLite内部sqlite_sequence计数当成业务消息。初始关闭、恢复前副本及第二次关闭数据库SHA256均为：

`872BAEAAFFDBC82BABC7222747E4C1E19B3ADDE24BAB50417A37BCA26BDB978F`

恢复前后conversation、runs、messages、events、tools逐项完全一致，配置文件也完全一致。两次读取前后无WAL/SHM、没有新建它们，使用mode=ro&immutable=1＋query_only，没有忽略未合并日志、checkpoint、改写库或重开Service取证；恢复的那一次正式宿主启动有独立授权和前置门。

模型任务run `700a51c2d5f97bc0e7ad404b18585c5b78e236e3500dd21b6bfffa3d257d8921`、client `panel-1a1a463cb64-4`，恢复诊断query为host-run-1。固定目标仍为实例1/kind1、原host_epoch `8d015ae4-87ef-49f2-8d48-10a3663cf8b6`、open_epoch `eb1b32de-8268-4651-9e7e-0308d3683857`、memory.document/r1/read scope；只是历史记录，没有将旧目标改绑新实例或恢复执行。history=[1]，投影仅memory.read及revision/value。恢复诊断保持真实持久化error空且error_recorded=true；不因内存任务不存在丢失状态，不虚构当前网络字段。

## 零Key、未发送与预览限制

模型配置只存version/model/ref/revision，model=offline-fixture；两次用户诊断均显示凭据not_configured、未打开/提交原生窗口、未写入/读回凭据。未点击外发确认，模型持久化final里confirmed/disclosure_saved/model_request_saved均false，request_id空、模型/工具计数0、not_sent；全库没有disclosure/provider请求/响应或工具事件。这些支持未进入模型路径，不是凭据枚举或全程抓包，不能声称精确HTTP统计或RPC M/R/S/C/U。stub本地计数不能当网络请求数。

没有真实模型服务、read业务工具执行或服务端认证，这些不是本轮目标；只读资源、能力和字段只作为预览范围，不能把选择动作说成读取业务值已执行。当前固定目标核对不替代跨标签测试。

预览截图与首块复制证明实际显示和已核对内容，生产宿主display_valid依据实际分块文本读回；但没有对用户提供的片段补上缺失大括号/尾块，也没有将后续源码推导冒充原始完整导出。用户“太难找了，范围摘要在哪？”作为非安全性易用性观察保留。最小后续建议是另行授权明确中文“范围摘要/完整请求”分区标题及可读范围摘要；本轮不修改界面，也不要求用户反复手抄JSON。

## 保全、下一步与停止

本轮唯一新记录加6份必要更新，共7份授权文档；根三规划只追加，原冻结交付的源码、153项程序及身份关联清单保持。新文档和人工证据身份分别见本轮delivery-files/document-files-final/delivery-identity，不冒充原离线或独立复审身份。没有修改源码、测试、构建、版本、依赖、SDK、参考工程或旧证据；不操作旧人工库、复制旧凭据、改LOCALAPPDATA或使用私有输入。普通宿主最近应用偏好仍按其原路径运行，不宣称UI_AI_DB_PATH隔离全部偏好。

本轮必要用户截图、两份复制诊断、预览首块、两次监督/退出/关后只读观察、本轮库/配置、恢复前副本和文档before均保留；没有待清理的无用途构建/测试产物，不删除历史材料。先前首次进程退出码未知、原人工异常、red BMP缺失、ASAN20.02秒/30.08秒及启动/期限/frontend/输入/browser首败独立保留；没有用新通过覆盖或归因环境。已有Provider真实结果及费用unknown/483 Tokens差额不扩审。

最小后续范围：先另行确认允许的第二个本地只读应用或可切换标签场景及匹配程序身份，再仅补已接纳目标跨标签不改绑；没有合适现成入口时提出规格/实现授权，不临时换夹具或扩外发范围。本轮已用完两次启动，停在部分完成结论，不再启动、不修产品、不进入有Key或真实调用。IME/DPI/完整Windows来源分类/辅助技术/人工长时/专用断电仍延期，browser九副作用另案。


## 2026-11-04剩余跨标签补测已获授权

已找到同冻结交付中的普通生产web_counter.uapp，仅被动查看合成页面及切换标签；包内导入ui_framework.dll，不使用uha_memory或p4测试DLL。独立补测根build/xuanshu-uha-cross-tab-20261104-d3c057，详[补测记录](xuanshu-uha-cross-tab-validation.md)。本记录原通过分项、两次exit0、库/配置不变及跨标签未覆盖的历史结论保持。新补测尚未产生实际动态证据，不提前宣称整批通过；未重开/读取本记录的数据库。
