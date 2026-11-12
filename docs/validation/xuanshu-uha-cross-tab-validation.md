# UH-A第二批跨标签固定目标限定补测

2026-11-04最终：跨标签固定目标补项通过；承接此前第二批已经通过的历史选择、预览、本地取消及记录恢复证据，集中人工第二批零Key、零联网限定验收通过。实际RP-C→Web Counter #2→RP-C由用户截图证明，中间诊断与切换前任务/目标完全一致，返回后取消落库再次一致。PID13776实际exit0，新库1任务/2消息/3事件/0工具，无确认/请求保存或工具派发。不是全部人工、真实Provider或系统完整验收。下方准备和等待段落按发生时状态保留，以本段及最终结果为准。


2026-11-04：准备核对已完成，人工补测尚未执行。现有安全第二标签已找到，下一步仅按本轮授权启动一次匹配正式宿主，由用户逐步完成本地未确认任务、切换标签、取消及正常关闭。此前第二批通过分项继续有效，整批结论仍待本次动态证据，不预填通过。

## 基线与已有覆盖

匹配快照为`build/xuanshu-uha-visibility-20261103-e57b29`，delivery-identity.json SHA256为`FF84C151A97784832D932AAE72F55BF9107DDE7E81DFF97F006FF41887664AB0`。本轮只读核对417项源码、153项程序和21项身份关联清单全部匹配。HEAD为`e0eca03db25930be55bdb45477027d1793a61904`，既有工作区修改不归为本轮。参考工程README可读，仅只读；未使用子代理或computer use，没有构建或运行测试。

此前[第二批记录](xuanshu-uha-manual-second-validation.md)及`build/xuanshu-uha-manual-second-20261104-89bc95`保持。依据已有退出和关后观察记录，初始PID61528与恢复PID64548实际exit0；3任务、6消息、9事件、0工具，恢复前后记录与库/配置哈希一致。A/B合成历史、只选user A、未确认提案、完整预览显示、主动取消、唯一终态及persisted诊断已分项通过。本轮没有重开或读取旧数据库，仅读取已有脱敏取证文件。

唯一必要补项是实际跨标签前后固定目标，不把场景缺失写成已证实的目标缺陷。第二预览块全文未独立归档及摘要难定位的原限制保持；本轮不重做原流程、不修界面。后续易用性建议仍为中文分区、可读范围摘要及JSON详情，属于另行授权的产品修改。

## 第二标签的选择依据

选择同一冻结目录的**普通生产版web_counter.uapp**，而非p4-web_counter或uha_memory。包内manifest为org.ui.example.web-counter、Web Counter 0.2.0、API10/ABI1、multiple_instances=true；只有web_counter_app.dll。通过只读解析UAPP索引和PE导入，实际导入为ui_framework.dll、KERNEL32.dll，与CMakeLists.txt的web_counter_app→ui_framework_shared链接一致。包及源码均匹配冻结清单。

`examples/web_counter/app.c`内页面是编译内置HTML，初始计数为0，没有外部URL或业务文件。create只分配合成内存状态、注册目录和菜单；mount创建Light视图；active回调为空；关闭允许并释放本实例资源。`examples/p2_support.h`注册和poll路径仅管理本实例能力/版本/待完成调用，不自动发起业务调用；无待执行事务时p4_try_complete直接返回。不存在启动/激活时读取用户业务文件或原有数据库的调用。

该应用本身含counter.read及counter.increment，不宣称整个应用是只读包。本轮限定为**被动查看合成页面和切换标签**，不点“+1”、Counter/Increment菜单，不选择其能力，不调用snapshot、编辑、图或代理入口；只有rpc_memory的memory.document/read/revision/value进入本地提案范围。应用注册能力不是本轮执行或授权这些能力。

排除uha_memory：包内rpc_memory.dll实际导入ui_framework_edit_test.dll；排除p4-web_counter：实际模块同样导入测试DLL。browser涉及浏览器及另案副作用，不选。未复制、改名或修改rpc_memory，保持其multiple_instances=false。宿主main.c的wWinMain逐个处理包路径参数，因此可用两个不同现有包建立场景；不以加号存在推定已打开第二实例。实际两个标签及切换结果仍待用户观察。

## 程序与新隔离目录

| 文件（冻结目录light下） | SHA256 |
| --- | --- |
| framework_host.exe | D386E229872295CE604196AF456CD3B4FB3100F39687377D7012B011FA5F6E17 |
| ui_framework.dll | E0ECEF4362DA77690F849F8B9415BDF82C1640B860656DDEE531C18DFF6ADBE5 |
| rpc_memory.uapp | 235F8CBEC2ED287F52AA05B5EEC92F3667D3344A4DF0A11A98BB262FA9F8769F |
| web_counter.uapp | 13ABD22BDDAF2DE5481CE47C30DCE4608D5F4D5764AC60D6957479C956CA856A |

新唯一根`build/xuanshu-uha-cross-tab-20261104-d3c057`；显式UI_AI_DB_PATH指向其`session/conversation.sqlite`，配置由生产代码派生为同路径加`.model.json`。创建时二者均不存在，不操作前两批库、不复制旧配置或凭据、不改变LOCALAPPDATA。普通宿主最近应用列表仍沿既有系统路径工作，AI数据库隔离不等于隔离全部宿主偏好，本轮不额外读取或更改该列表。

只允许本轮一次启动，顺序加载rpc_memory.uapp和web_counter.uapp；第二标签可能初始激活，首个人工步骤先确认两标签并选择rpc_memory。启动默认不创建任务或联网。独立监督持有子进程句柄，记录PID、stdout/stderr、心跳和实际退出码；提前退出即保留并停止，不重启、不强杀。新监督脚本仅控制这次授权启动和记录进程状态，不执行产品测试或注入输入。

## 限定流程与判定

每次只向用户给当前一步；以下是取证准备，不一次发送整份操作清单。

1. 用户确认两个实际标签，选择RP-C synthetic read-only memory。Web Counter仅查看，不点击业务按钮。
2. 在既有DeepSeek配置区保存offline-fixture合成模型名，不进入密钥窗口。ready只表示配置可用，不代表凭据或服务端认证。
3. 选择memory.document、memory.read、revision和value；无历史、无其他应用范围。
4. 常驻“任务输入（填写要本地保存的任务内容）”输入“UH-A跨标签固定目标补测：仅保存和查看合成内存只读范围，不发送到模型。”，只点“保存真实模型任务提案”，等待持久化接纳和完整预览。
5. 记录同一run/client及固定instance、host/open epoch、resource_id、revision、scope、能力和字段。需要精确值时使用既有“复制脱敏诊断”，不让用户手抄或查找无标签JSON框。
6. 用户切到Web Counter标签，取得实际切换观察/截图及原任务诊断；返回rpc_memory，再核对原任务身份与固定目标。当前活动应用或新草稿目标可变化，不等于原已接纳目标改绑；不点击“以当前目标”重新提案。
7. 用户请求取消，等待cancelled及保存确认；正常关闭后由助手确认真实退出，再只读核对本轮库。

禁止提交stub任务、生成历史、确认外发或任何密钥入口；没有工具业务执行或真实模型调用。仅最终库相同不能证明中间切换，必须保留用户观察和前后对应诊断。复制操作不授权、提交或联网。目标/确认资格/越界/意外请求/存储/关闭异常时停止相关步骤，只读定位，不能在本轮修产品。

## 关闭、证据及停止边界

用户关闭并确认实际退出后，只用mode=ro&immutable=1及query_only核对本轮自有库；先检查WAL/SHM，存在未合并WAL则不忽略、不checkpoint，不宣称完整结果。预期仅一项本地未确认模型任务、唯一cancelled终态，具体数量以实际库为准；核对无外发确认、模型请求保存、模型工具、重复任务或改绑。退出码取监督句柄，不用用户“已关闭”代替。

普通宿主无RPC验证配额，不虚构M/R/S/C/U；无全程网络遥测，不冒充抓包或精确HTTP次数。以禁止操作、凭据not_configured、模型路径前置门及持久化事件证明本轮未进入模型请求路径，助手不读取任何凭据。用户观察、软件状态与持久化证据分开记录。

保留旧人工异常、未知退出码、ASAN20.02/30.08秒首超时、red BMP首次缺失及其他历史风险，不归因环境或覆盖。IME/DPI/Windows完整来源/辅助技术/人工长时继续延期；物理断电仅专用环境；browser九项副作用另案。当前只是补测准备，不宣称独立复审、整批或系统验收通过。

本轮新增记录及必要索引/交接更新；根三规划仅追加。没有产品、依赖、公开结构、来源政策、预算/限额/超时或版本修改。新文档状态与原冻结交付分开。必要证据和新库保留，不清理历史材料；当前无无用途过程产物需要删除。


## 实际启动与当前等待

2026-11-04已执行本轮唯一启动：正式宿主PID13776，监督PID20744，工具会话57720持续持有进程句柄。11:28:16正常用户上下文只读观察窗口稳定，实际加载生产ui_framework.dll、rpc_memory.dll和web_counter_app.dll分别匹配冻结包身份。窗口Responding只作为进程/窗口状态，不替代两个标签、控件健康或用户操作证据。当前未提交任务、未保存模型配置、未操作Key/外发确认；不在线读取数据库。实际退出码待用户正常关闭。

当前只等待用户确认顶部两个标签并点击RP-C synthetic read-only memory；预期主区为明确无业务视图空状态，Web Counter标签仍保留。后续动作逐步等待反馈，不自动提交或切换。本轮启动额度已使用，提前退出即停止，无循环重启。


## 最终执行与结果

| 项目 | 实际证据 | 结论及边界 |
| --- | --- | --- |
| 现有安全第二标签 | 包内manifest/PE导入、匹配生产DLL及源码调用链；用户确认双标签，截图Web Counter #2和count0 | 生产普通web_counter仅被动查看。未改包/单实例属性，不用测试DLL |
| 合成配置及单提案 | 用户截图offline-fixture、配置已读回、revision1、凭据未确认；本地输入及memory范围逐步选择 | 无历史、无stub、无Key；ready不表示凭据可用 |
| 持久化等待确认 | 切换前诊断phase3/committed=true，display_valid=true、generation21；任务context仅投影＋指定合成输入 | 收到与保存分开，以软件committed及关后accepted事件支持保存；不确认外发 |
| 实际切到第二应用 | user-web-counter-active.png显示Web Counter #2激活、count0；其后用户复制原任务诊断 | 不是仅最终库推测切换。两份诊断整个task及model逐值一致，仅顶层display_generation从21变23 |
| 固定目标不随活动标签改变 | run/client、binding、instance、host/open epoch、memory.document/r1/read scope及memory.read/revision/value完全一致 | 当前活动应用变为Web Counter，不重绑定已接纳任务；没有counter字段/能力进入范围 |
| 返回原应用 | user-returned-rpc.png显示RP-C标签激活、Web Counter标签保留、空状态和助手预览仍在 | 返回瞬间未另取全量诊断；随后用户取消及关后accepted/final记录保持原任务和目标，未发现改绑 |
| 本地取消和保存 | 用户反馈已显示cancelled；关后唯一finished/terminal1、phase7、outcome=cancelled、committed=true、error空 | 不将取消按钮点击当作保存。只有一个任务，无重复提案 |
| 正常关闭 | 用户已关闭；监督句柄实际exit0；正常用户只读进程核对宿主和监督均不存在 | 未强杀、自动重启或提前卸载。stdout/stderr各0字节，不冒称详细内部生命周期日志 |
| 未进入模型路径 | 两份实时诊断和持久化final均confirmed/disclosure_saved/model_request_saved=false，request_id空，model_calls/tool_calls0、not_sent | 无模型/工具派发记录；无全程网络遥测，不声称抓包或精确HTTP次数，不生成RPC M/R/S/C/U |

本轮没有发现新的目标、权限、存储或关闭缺陷，也没有重复已经通过的A/B历史和同库恢复流程。此次普通生产包含现有可操作计数功能，但未点击+1/Increment或调用counter能力；观察count0不是对全部业务写入路径的验收。

## 固定目标精确比较

比较来源为user-diagnostic-before-switch.json、user-diagnostic-on-counter.json、closed-db-observation.json中accepted/runs/final；动态切换截图及返回截图同时保留，不能仅凭末次库推导中间操作。switch-target-comparison.json及closed-verification.json保存逐值结果。

| 字段 | 切换前、Web Counter标签下、取消落库 |
| --- | --- |
| run_id | 064750e91c66b60dbb8888c31dc5af13a02217a7f62b7efef8bdeffa8a1dd32a |
| client_request_id | panel-1a1a4b066d0-2 |
| binding_hash | 4d4f571b90d415c3ca4768e2598efd0fd08c0648ab7813c474332245c57113be |
| instance_id / kind | 1 / 1 |
| host_epoch | c3b779a2-2ec0-47f5-bbff-c980f1fc2e4b |
| open_epoch | 912b9785-941f-4790-ae75-b8f73af646f8 |
| resource_id / revision | memory.document / r1 |
| scope | {"access":"read"} |
| capabilities / fields | ["memory.read"] / ["revision","value"] |
| history | [] |

配置与任务binding_hash各自意义不同，未混为同一摘要。read选择与r1目标元数据不等于执行了memory.read或验证了业务值10；本轮工具0是要求的结果。

## 实际退出与只读数据库核对

本轮唯一启动11:27:44.772715，正常退出12:08:11.107272（Asia/Shanghai），宿主PID13776、监督PID20744，实际exit0，2426.422秒；并非预定义长时测试。工具会话57720已收取完成输出。12:08:56正常用户上下文再次确认两个PID均不存在。没有再次启动或恢复旧授权。

只核对新库session/conversation.sqlite：53248字节、schema1、1 conversation、1 run、2 messages、3 events、0 tools。事件严格为accepted→awaiting_disclosure→finished，只有一个terminal=1；assistant最终消息为空。模型提案client与run唯一，没有额外stub历史、重放任务或模型工具。配置只含version/model/credential_ref/revision，model=offline-fixture；不含Key。本地配置引用按新路径生成，不复用此前引用，未访问Windows凭据或环境Key。

数据库读取前后均无WAL/SHM，使用mode=ro&immutable=1及query_only，没有忽略未合并WAL、checkpoint或改写。前后SHA256相同：

`97886179C4E44251AF99962AFA3F222BD1135889D8486E0A815E7752B8B3BC0D`

配置SHA256为`87D881924562DACE39C591374523B6CD4C21F0571AF9ABF097BB7616EDA9F98B`。本轮没有重开或读取此前第二批数据库；此前3任务/6消息/9事件/0工具、两次exit0和恢复诊断证据直接承接已有记录。

## 第二批整体收口与剩余边界

此前第二批分项通过、本次补项通过，可以报告“集中人工第二批零Key、零联网限定验收通过”。前一记录在当时只能判部分完成，不能回写成当时已覆盖跨标签；本补测以新独立根和两个真实标签补齐该缺口。

用户截图/反馈、生产复制诊断、监督退出、关后持久化证据各自区分。没有全程网络遥测，零联网限定结论是本轮未使用Key/确认入口、未进入模型请求路径的证据组合，不冒充网卡抓包、精确HTTP计数、服务端认证或真实模型验收。当前证据不涉及其他后台系统网络活动。

原第二批预览第二块全文未单独归档、范围摘要/JSON分区难辨识继续保留为取证限制及非阻断易用性建议，不通过缩小必要检查冒称通过。本次预览软件显示资格与先前匹配代码/人工核心内容证据承接，不新增原始网络抓包说法。后续最小产品建议是常驻中文分区、可读范围摘要、JSON作为详情，需要另行授权，本轮未修改。

IME、DPI、完整Windows来源分类、辅助技术、人工长时及专用环境物理断电仍未验，browser九副作用另案。原人工失败、首进程退出未知、ASAN20.02/30.08秒等首败和red BMP限制均保持；不默认归因环境，不说根因已解决。没有新增阻断缺陷需要在本轮修复。

## 文档、身份与保全

本轮新增本记录，必要更新第二批记录、人工清单、阶段索引、当前交接和根三规划；根三规划仅追加，全部旧结论/首败保留。文档和人工证据是新的状态，不冒充FF84…64AB0离线冻结时的哈希，也不是新独立复审或正式产品重新冻结。

关闭后417源码、153程序、21项身份关联清单再次全部匹配原冻结身份。生产宿主、运行时、包、公开头、版本、依赖、来源及限额未改；没有构建/测试、私有驱动、computer use或子代理，没有Provider/账户/模型列表请求。本轮保留新库/配置、包核对、两份诊断、三张用户截图、所有步骤反馈、监督与退出、关后只读核对和入口文档副本。没有无用途过程产物待删除，未清理任何历史材料。

脱敏证据索引：preflight/package-inspection、switch-target-comparison、closed-verification、verification-summary、post-close-identity、preservation、redaction-check、cleanup、delivery-files/delivery-identity及final-readback。final-readback只核对封装结果，不是新测试/独立验收。

已停在本轮补测结论，不再启动，不进入有Key/真实调用、其他人工批次或产品开发。
