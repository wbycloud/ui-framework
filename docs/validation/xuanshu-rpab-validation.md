# RP-A＋RP-B开发交付及离线验收

2026-10-29。结论：**RP-A＋RP-B开发阶段模拟及自动验收通过**，停止联合代码评审和首次真实服务验证之前。本结论只适用于本轮明确授权的代码及离线合同；不是独立评审通过、真实DeepSeek服务通过或真实人工验收通过。RP-C未执行。

## 1. 实际基线、增量与身份

分支codex/p1-process-boundaries，HEAD e0eca03db25930be55bdb45477027d1793a61904，未提交。入口当前源码/直接文档476项见新根baseline.json，含既有修改及ignored根规划；不是历史全量Git17470项。一次本轮自有全历史树读取耗时过长被停止，没有据其宣称全量基线。参考D:/XuanShu仍Permission denied，未扩权、不修改。

新证据：build/xuanshu-rpab-20261029-6f38a1。source-after、delivery-files、preservation、incremental-source-diff、baseline-source、final-source、binary-files、red-binary-files、delivery-identity、verification-summary、compatibility-coverage、exports、owned-data-files、failure-source/binary-comparison及cleanup共同标识本轮。不能把HEAD至工作区的既有P2—P6差异都算作本轮。

P6R-1入口374源码/102最终程序与原清单匹配，详p6-identity.json；原根build/xuanshu-p6r1-closeout-20261028-91c4e7只读。原入口自动17470＋3份事后补充ignored根规划的来源界限、Native96/96、ASAN96/96和Light55通过/1未运行后补1/1均为历史证据，不是本轮新跑。原冻结不自动覆盖RP增量。

## 2. 实现及兼容合同

先冻结[RPAB-1](../xuanshu-rpab-contract.md)，后实现。独立ai_model.h/ai_model_ui.h，不改SDK0.10-dev/API10/标准10、ABI1/包1、旧公开布局/v1/C2/P4/P5错误和固定依赖。Windows系统WinHTTP/凭据/原生凭据窗口，不引入新库。

一个workspace复用原Service和主对话。新接口复制配置、历史序号、投影、目标及输入；旧submit仍stub。模型配置后台读取和原子保存，未来/损坏格式封闭写入；固定DeepSeek origin及框架自有引用，不读环境Key或其他应用凭据。原生Key输入不经页面，后台保存；本轮只用内存替身。

后台先形成有界实际context_preview，宿主分页展示当前输入、明确历史实际文本、目标和read投影。人工外发确认只由框架私有匹配事件一次消费，最长30分钟；摘要固定Service session、run ID、请求、profile、目录、目标/epoch/revision及参数。保存确认后再次经UI核对才POST。模型文本、工具结果、source、页面布尔和程序消息都不能授权。没有AI写入、审批或图接口。

网络使用固定HTTPS路径、SSE/UTF-8严格解析、完整批次校验、原P2串行read执行。发起/回调/完成待消费/存储确认均保留引用；取消和到期不提前释放。工具结果保存后才续轮，所有失败无自动POST或业务重试。COMMIT与checkpoint区别沿用P6；存储不可用仅cleanup-only。唯一终态、幂等client ID、run/request/tool关联沿用原存储。

网络缓冲使用已计入原8MiB的2MiB任务arena，无耗尽回退堆；所有原额度不放宽。生产禁用重定向、自动认证/cookie/跨session池和失败连接重发；系统不支持安全选项时明确失败。真实系统网络行为未由替身证明。

正式Light保留原stub、受控编辑、图及命令诊断。用户模型配置、原生Key入口、范围确认、流式回复/错误/历史/排队/取消都是Service投影；收起不取消，失败保留输入。新消息封装在原七字段/64KiB桥接内，未提高解析限额。静态嵌入复用同一模块。

## 3. 最终匹配自动结果

| 构建／原始证据 | 结果 | 实际范围 |
|---|---|---|
| native/native-release-matching.xml及同名根日志/源码与程序清单 | 61/61 | 全部RP协议/核心/静态＋原Service/continue、C1入口/closeout、Assistant、公共C/C++头、两真实DLL分支、受影响P4/P5/P6R-1收尾 |
| light/light-release-matching.xml及同名日志/清单 | 40/40 | 全部RP＋正式Light模型/旧P3/P4/P5面板、组件布局、公共头、Service/continue和静态 |
| asan/asan-matching.xml及同名日志/清单 | 50/50 | 全部RP、公共头/Assistant/静态、真实DLL、Service/continue及受影响P4/P5收尾 |

每批configure/build/test输入身份在对应sources.json；测试前程序哈希在test-binaries.json。最终源码逐项与以上三份输入匹配，不用测试数量替代合同。相同源码不同编译分支分别执行。首次及中间结果全部保留，最终全绿不覆盖它们。

## 4. 业务／生命周期验收映射

| 合同 | 断言与方法 | 边界 |
|---|---|---|
| 配置/引用/origin/秘密 | model_setup固定端点与自有引用，错误origin/ref明确拒绝；事件/请求/SQLite无内存Key哨兵；最终sentinel-scan | 不读真实凭据，不证明OS交互 |
| 一次人工确认 | 未确认requests=0；重复确认PermissionDenied；原生BM_CLICK/程序键鼠、页面confirm拒绝；绑定哈希/run ID固定 | 私有夹具只证明接纳后链路；真实来源延期 |
| 外发实际内容 | history用序号选择一项实际消息，body包含explicit-selected、不含private-unselected；preview先于确认；无默认全历史 | 没有任意文件或其他应用快照 |
| 分片/限额 | 协议每个两段切点、逐字节中文；重复键/NUL/坏UTF-8/截断/[DONE]/超长行/精确事件预算拒绝，最终18242检查0失败 | 协议样本有限，不代表厂商全部事件 |
| 全批校验/修改旁路 | 多read实值、read＋write整批拒绝且tools=0；非法Schema参数0工具；跨轮重复ID不重复执行；目录不含memory.set/external | 首版包装discover仅page0，内部原目录分页聚合 |
| 固定目标/revision/epoch | 真实workspace两个DLL实例，切到第二标签后读两个固定目标值均10；改变revision/关闭重开旧目标确认失败且requests=0 | 不是固定指针冒充workspace |
| 关联/同步/异步 | received时run关联存在；同client不重复发送；真实DLL read worker及外部callback分别受控退出后才完成；重复complete返回AlreadyExists | 不追踪任意应用线程 |
| 取消/到期/关闭 | held请求取消保持非终态、destroy Pending；到期期间连接仍在途不释放body/stream；退出后可关闭 | 远端是否已处理/收费仍未知 |
| 网络回调在途 | 隔离DLL中实际am_http函数在发布租约后受控暂停；close/destroy Pending、引用保留；退出和worker消费后关闭 | 测试暂停仅UI_AI_FIXTURE，无真实WinHTTP调用，不能冒充OS回调验证 |
| 队列/缓存/背压 | 原队列/8MiB/64缓存限制；排队取消可排空、排空后恢复；5轮2MiB arena触发原缓存回收仍可完成且bytes≤8MiB | 没有提高额度或降低生产缓存做测试 |
| 网络错误/重试 | 401/429/503、TLS/代理替身、120秒假时钟、截断；每次请求计数固定、失败不重发 | 不是实际外网、代理或证书链测试 |
| 保存顺序/故障 | 实际第32次COMMIT落在provider_request并注入checkpoint IOERR，记录确实存在、requests=0、无终态；在途存储失败不新发请求，保持未确认；原最终确认/UNKNOWN门相称回归 | 不声称已补完所有P6长期故障窗口 |
| 冷开 | 专用子进程保存WAITING_INPUT后自行退出，父进程冷开phase=NEEDS_REVIEW，无恢复授权/网络/工具；既有client只返回记录 | 只操作本轮隔离库和自有子进程 |
| Light/嵌入/原功能 | 正式页面JS配置/提案/投影、输入清空与失败保留、收起后继续；旧P3 REFUSE/恢复、旧编辑/图面板仍通过；静态文本流程同Service | 模拟UI不是真实设备 |

## 5. 首次失败、自审和修正

| 证据标签 | 首次事实 | 修正／归类 |
|---|---|---|
| core-first / core-fixtures / light-rp-first / light-rp-compile / light-panel-link等build | 宏重复、测试无符号字节告警、未定义WinHTTP常量、私有函数不导出及测试JSON库链接；另有错误目标名称 | 常规编译/编排修正，不放宽/WX或断言；未执行的build不能称为修前产品运行 |
| core-first-tests / core-diagnostic | 测试在Service EXCLUSIVE锁内从第二连接查SQLite；截断只删CRLF的一字节仍是完整SSE | 改在安全关闭后查同样SQL；删去完整事件分隔符才是真实截断，未放宽断言 |
| batch-diagnostic | prepare后立即forget尚未执行call返回PENDING | 产品修正：整批纯Schema预检，不造临时调用；真实prepare保留到原闭环 |
| batch-preflight-fixed | PREPARED没有推进到绑定保存/dispatch，WAITING_TOOL有界不排空 | 产品修正：复用原PREPARED→保存→重新检查→DISPATCH；保留失败程序 |
| extended-cases | 排队取消20秒超时；重启夹具正常关闭已取消任务却要求NEEDS_REVIEW | 产品增加“已保存且取消”的模型队列收尾资格；重启改专用子进程退出，不把正常关闭冒充故障恢复 |
| queue-recovery-check / final-core | 夹具未捕获history及查询ID/任务ID参数颠倒 | 修正夹具关联，SQL/模型断言未弱化 |
| light-panel-first / panel-diagnostic / panel-bridge | 提案8字段被原七字段解析器拒绝；不是Light旧输入失败已定位 | 产品按原params封装选择，保留原七字段/64KiB；panel-envelope后正式JS闭环通过 |
| expiry-red | 初版采样未等待worker经过目标窗口，测试通过但不足以证明到期路径 | 补实际tick/worker退出屏障，不冒称这一轮已复现错误 |
| expiry-barrier-red | 524检查2失败：在途到期提前终态/释放body和stream | 最小修正：新派发到期拒绝，在途连接仍待退出；expiry-fixed及最终三构建通过。不是已复现崩溃 |
| sse-boundary-red | 18242检查1失败：事件满额后额外data分隔符未拒绝 | 修复无符号余量下溢，额度不变；最终三构建通过。无数据损坏宣称 |

自审还补齐模型公共方法/确认期间的Service与run租约、完整预检期间同arena互斥、目录描述变化失效、提交关联先于回调、宿主重建重新绑定及存储确认后再次UI核对。reentry受控回调中关闭/poll/destroy返回PENDING，退出后可安全销毁。没有禁止所有回调重入。

一次将新私有scratch从2MiB提高的尝试被自动审批审查拒绝，未落盘；随后保持2MiB及原8MiB，采用预留arena和有界分阶段释放完成验证，无未解决审批阻塞。

## 6. 历史与未验分账

四组未定位期限风险：并行graph_boundaries>20秒、ASAN控制5秒、P2五包15.04秒、面板180.44秒；后续串行同阈值通过不等于首败定位。本轮未测试这些历史负载场景，也不以新通过宣称解决。旧frontend63、ASAN-Light中断、Light surface_input、browser20、workspace首次20.05秒（20秒门槛）、Native输入首次失败保留原结论，不默认归因环境。当前限定路径通过未提供把它们归为已解决的证据。

browser九副作用另案；不实现OpenAI/OAuth、真实模型写入/审批/图、网络工具或应用新业务。真实模型/TLS/代理/凭据系统交互及真实设备/Windows来源正例、IME、DPI、视觉、辅助技术、物理断电和人工长时见[延期清单](../xuanshu-rpab-deferred-validation.md)。这些是明确排除/延期项，不冒充已验证。

## 7. 保全与停止位置

原P6根、冻结SDK、参考工程、其他固定依赖和历史材料均未改。root规划仅追加，preservation核对入口原始前缀。原交付归属靠476入口身份与实际文件哈希、匹配P6源副本及本轮新增文件判定，不依据全部Git差异。首次失败源码、日志/JUnit和匹配程序均保留；清理仅本轮独立根的无用编译obj，匹配EXE/DLL/uapp/lib/PDB、失败材料、配置和隔离库保留为核对证据，详cleanup/owned-data-files。

未使用子代理/computer use，未读取真实Key、调用真实Provider、操作用户数据库、升级依赖或执行Git/远程/benchmark/recovery。最终为RP-A/B开发交付与自审结果；停止联合代码评审及RP-C之前。

证据自检补充：首次哨兵原始字节扫描命中两份旧失败日志，核实均为CHECK打印的测试SQL字面量（EXCLUSIVE锁内查询失败），不是事件、请求或数据库内容泄漏。未删日志或忽略整文件；sentinel-scan逐行保留这些诊断字面量分类，其他命中仍报错。最终事件/请求/库及日志动态内容无哨兵泄漏，不把初期查询失败当成秘密检验通过。


## 8. 2026-10-31独立评审两项问题及限定整改（独立新证据）

上一轮只读复审发现发现数组投影为空及cleanup/cache缺少租约两项阻断，结论为暂不通过/需先整改；原第3节离线结果不等于独立复审通过。本轮按用户限定授权核实、补修前有界回归并最小修复，详[xuanshu-rpab-closeout-validation](xuanshu-rpab-closeout-validation.md)。不覆盖原记录及首败。

新根build/xuanshu-rpab-closeout-20261031-a19c42。修前发现31失败；64槽扫描4失败，未宣称崩溃。修后Native新增2/2＋相称61/61、Light42/42、ASAN52/52。两项只改私有适配/引用，Schema仍来自原目录describe，P3/v1发现格式、SDK/API/ABI/包、公共头/错误、来源政策、依赖及额度不变。原RP/P6证据和延期项目均保持。

这是整改交付及自审证据，具备再次限定独立复审条件；不自行宣称独立验收通过、正式冻结或RP-C许可。真实Provider/Key/用户库均未访问，原期限风险和输入/frontend/browser首次失败继续独立未定位。
