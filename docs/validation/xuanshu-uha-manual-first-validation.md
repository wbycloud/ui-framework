# UH-A集中人工第一批：零Key、零联网验收记录

2026-11-03。状态：唯一一次受监督重试完成并正常退出；第一批人工部分覆盖，因收起残留/交互异常暂不通过，停止后续操作。首次启动取证不足及旧记录保留。本轮授权文档、匹配正式宿主启动、新隔离目录本地配置/合成任务和取证，不授权源码修改、构建、离线测试、真实凭据或Provider请求。

## 身份与隔离

冻结追记见[xuanshu-uha-buildgate-independent-review.md](xuanshu-uha-buildgate-independent-review.md)。匹配delivery-identity SHA256为B3351D1DB62743F5E5EFC67739976CB2A470B7D32C5855375B25B02A8C68007A；本轮没有重新运行离线测试。启动只用原根light/framework_host.exe、ui_framework.dll及生产链接rpc_memory.uapp，程序哈希见本轮preflight.json，不用测试DLL、窗口驱动或私有可信事件。

新证据根build/xuanshu-uha-manual-first-20261103-5d8a41，启动前不存在，唯一session/conversation.sqlite；模型配置固定派生为session/conversation.sqlite.model.json。未覆盖默认配置、未改变LOCALAPPDATA、未复制凭据或旧库，不复用RPCB引用。新配置不存在时只加载本地配置文件；不设置Key、不运行凭据读取检查、不点击外发确认。保存offline-fixture仅为本地合成模型名，不是启用传输替身或允许联网。

## 操作与取证安排

助手每次仅给当前一个操作，说明实际控件、预期和短反馈，等待用户完成。依据匹配host.html/宿主源码核对控件，不制造人工输入或确认，不使用computer use。用户观察与软件脱敏诊断、关后库证据分别记录，不以聊天反馈代替保存确认。

| 检查 | 当前结果与证据 |
| --- | --- |
| 冻结身份、启动前隔离 | preflight完成；19项身份关联、13增量、416源码、102最终产物无漂移 |
| 启动、未配置、隔离路径、零自动任务 | PID43480创建后已退出；未捕获退出码/栈，UI就绪未由用户观察；不作通过 |
| 真实鼠标/键盘、焦点/滚动、来源诊断 | 待用户操作；仅覆盖实际路径，不扩为Windows来源分类全部通过 |
| stub合成历史与默认不选 | 待操作，不使用真实数据或文件；脚本只在本地stub诊断 |
| read资源/能力/字段选择 | 待操作，唯一生产memory.document、memory.read、value/revision |
| 本地接纳/保存、完整外发预览 | 待操作；永不点击“确认此任务外发范围（一次）” |
| 固定目标、标签/折叠/刷新/页面重建 | 待核对实际公开控件；无入口就记录限制，不使用私有接口补成通过 |
| 输入/选择草稿、错误与诊断复制 | 待操作，优先可靠复制，不要求手抄大量JSON |
| 取消、正常关闭及原隔离库恢复 | 待操作；关闭/重启逐步等待用户反馈，不强杀 |
| 零联网及持久化证据 | 待正常关闭后只读核对本库；运行中不在线绕锁读库 |

取证保留步骤时间、用户简短反馈/必要截图、复制的脱敏诊断、稳定run/client/query ID、保存状态和错误、匹配程序、PID/退出及本轮独有文件。日志不含Key、Authorization、凭据Blob或完整内存。复制内容仅合成任务且可能包含已选历史，不宣称没有任务内容。

零联网判断结合新配置/未确认状态、原生产门和关后请求/事件/任务记录；普通宿主没有RPC专用M/R/S/C/U策略，不虚构计数或完整网络抓包。瞬时无TCP连接只作有限佐证，不能单独证明全程无联网。完整外发预览不是实际发送记录。严格区分收到、保存、等待确认、工具结果及终态保存。

## 安全收尾与限制

出现确认资格、目标错误、越界内容、存储不明或关闭异常时停止相关步骤，记录后只读定位，本轮不修产品、不反复点击或重开追求通过；其余独立安全检查可继续。不强杀、不提前卸载、不清UNKNOWN。关闭用右上角正常关闭，等待用户确认及进程真实退出；之后仅对本轮库使用mode=ro&immutable=1（确认无活动WAL，若仍有WAL则先停止并保留诊断），不创建WAL/SHM、不重开Service取证。恢复检查另次启动同一隔离库，绝不迁移/覆盖/复制其他库。

全部历史首败及证据限制保持，含ASAN30.08秒和ninja首次stderr缺失。IME、跨屏DPI、视觉专项、辅助技术、人工长时后续批次；物理断电仅专用环境，browser九副作用另案。第一批通过不等于真实Provider、全部人工或整个系统验收通过。

后续结果随实际观察追加，未执行项不预填通过。不自动进入有Key或真实请求阶段。


## 2026-11-03首次启动异常与本轮停止

15:30:17左右在正常用户桌面请求显示匹配正式宿主，PID43480，显式新隔离库，参数仅生产rpc_memory.uapp；Start-Process返回成功不等于UI就绪。15:30:58只读检查进程已不在，stdout/stderr均0字节；新库4096字节、WAL53592字节，模型配置未创建。尚未请求用户任何点击/键入/Key/外发确认，助手未提交任务或请求真实API。没有可用诊断快照或RPC策略计数，不能把未测的发送计数写成精确0。

首次受限上下文查询Application日志被拒绝；后在正常用户上下文针对framework_host.exe和近15分钟1000/1001事件只读查询，没有匹配事件。未捕获退出码、异常栈或实际加载模块，不推定为崩溃、环境问题、正常关闭或产品根因。监督退出码不足是本次取证限制；不因复审程序身份匹配而自动推断运行成功。

保留preflight、launch、startup-observation、application-events、early-exit-observation、stdout/stderr及原始自有DB/WAL；只作文件哈希，不对未checkpoint的库做忽略WAL的查询、不创建SHM、不回放/恢复、不重开Service。未自动重新启动或另建目录。第一批其余项目均未执行，正常关闭/恢复及全程零联网的独立证据尚未完成，不能报告第一批通过。

冻结追记已经完成且原源码/程序不改；本次启动异常尚无证据定位到已限定复审的窗口，不自动改写原快照结论。下一步最小建议是限定启动监督取证：保留旧尝试，捕获匹配程序真实退出码/初始化阶段及执行上下文；任何重启须另明确安排、保持原隔离库、不提交任务、不读凭据、不联网。若需源码或构建整改，必须另授权；本轮不执行。


## 2026-11-03唯一一次受监督启动取证重试（进行中）

用户另明确授权唯一一次启动取证重试，仍用原匹配正式宿主和原session/conversation.sqlite，不另建会话/重置状态。启动前程序/416源码身份匹配；原库53248字节、已无WAL/SHM，正常用户上下文确认原PID43480不在；只读immutable核对schema1、runs/messages/events/tools各0，文件哈希不变。当前库及首次日志/记录独立复制到startup-retry-1-before。原先WAL曾有哈希但未及时独立复制、重试前已消失，不能补造其内容或倒推精确退出时刻；此前进程未见的观察也不能独立证明准确退出时间，原取证限制追加保留。

本次监督脚本仅位于本轮证据根，不改产品。独占retry-1-intent标记保证无重启循环；Popen持续持有进程句柄、等待真实退出，分别保存PID/启动参数/心跳/退出码及stdout/stderr，不强制结束。监督进程仅作用于寿命取证，不制造输入、不签发授权、不读Key、不联网。

PID8708于本次启动后持续运行；正常用户上下文只读窗口元数据确认主窗口“C 应用框架 · Web”，生产ui_framework.dll路径/hash与冻结一致，生产rpc_memory.dll已加载。此元数据不证明UI内容或实际人工通过，等待用户第一步点击右上角AI。监督exec会话51648继续运行，最终退出码尚未产生；不在运行中读库。相关材料见retry-1-preflight/launch/heartbeat/window-observation及将来的exit。

当前第1步待用户观察：只点击AI，预期展开工作台并看到“玄枢 · 单一主对话”；不要求配置、Key、确认或提交。若再次提前退出则保留退出码/日志并停止，不再启动。


第1步用户反馈：已展开，看到单一主对话。记录为真实鼠标点击AI后的视觉/交互观察；不由此推定全部来源分类、保存或联网计数。监督PID8708仍运行，无exit记录。下一步只展开DeepSeek用户配置区域，核对未配置/凭据未确认及本轮隔离路径，不保存模型、不设置Key、不提交或确认外发。


第2步用户表示未找到目录名，改用截图逐步核对。截图user-step-02-model-unconfigured.png可见模型未配置/配置已读回/凭据未确认可用/配置代次0、无目标、预览未就绪，密钥及提案/确认控件灰色；路径只见D盘仓库前缀，右侧截断，精确隔离路径待诊断核对，不能直接判为隔离错误。用户偏好截图引导，不要求手抄JSON；下一步仅向下小幅滚动找到复制诊断按钮，不设置Key或外发。


第3步截图user-step-03-copy-control-visible.png确认小幅滚动后“复制脱敏诊断（不授权、不发送）”完整可见；模型仍未配置、无目标、无就绪预览，外发确认灰色。下一步只由用户点击该只读复制按钮并直接粘贴输出，不要求手抄。精确配置路径仍待实际诊断核对。


用户随后截图显示“已复制脱敏诊断；未授权、未提交、未发送”，并直接粘贴实际诊断。config_location精确匹配本轮session/conversation.sqlite.model.json，credential_ref匹配该路径派生的af4c…6cbd；模型空、revision0、ready/pending=false，凭据not_configured/last_entry=not_received、未打开/提交/保存/读回，错误码null表示未观测而非成功。当前task=null且run/query空，未发现隔离错误；display_valid=true只是宿主状态展示，不等于确认或发送。原复制截图/JSON及精确校验结果保留。下一步只输入本地合成模型名offline-fixture，不设置Key、不确认外发，输入动作本身不保存。


用户截图user-model-name-entered.png确认模型输入框已准确输入offline-fixture；状态仍模型未配置，尚无保存确认。记录为实际键盘/文本输入观察，不扩大为完整Windows来源动态正例。下一步仅点击保存用户模型配置一次，不设置Key、不提交任务或确认外发。


用户保存后截图user-model-config-saved.png显示offline-fixture、配置已读回、配置代次1，凭据仍未确认可用。助手只读本轮.model.json确认model=offline-fixture、revision=1、credential_ref匹配隔离路径，保存快照及哈希；未读取Windows凭据，也不把配置ready解释为Key可用/服务端有效。下一步仅从当前生产只读夹具列出资源和read能力，不提交任务或确认外发。


用户点击只读目录后截图user-read-resource-list.png列出memory.document，当前仍选无目标（纯文本）；未出现任务或就绪预览，尚未选择资源/能力/字段。下一步仅点击memory.document，不提交或确认外发。


用户截图user-read-resource-selected.png确认实例1/memory.document被选中，无目标已取消；出现memory.read及revision/value字段，三者仍未勾选。此为新提案草稿选择，尚不证明已接纳任务、固定epoch/r1或持久化；下一步仅选择memory.read。


用户截图user-read-capability-selected.png确认memory.read已选，实例1/memory.document保持；revision/value仍未选，尚未提交提案或确认外发。下一步仅选择两个结果字段。


用户截图user-read-fields-selected.png确认memory.document、memory.read、revision/value均已勾选，无其他字段或星号；预览仍未就绪，尚无提案。接下来只填写合成输入；零联网依赖未确认及生产门，不以输入中“不要发送”等文本充当授权控制。


用户按“输入；默认仅测试回显”提示未找到控件；顶部截图user-shared-input-location.png表明灰色回复此问题按钮下面第一块大空白textarea才是共享任务输入框（本图约y570–640），本图没有可见占位提示/明确标签。记录易发现性限制，原因尚未定位，不能归因用户或伪报修复；改按实际截图位置指引，不改产品。Service显示2、排队0，消息/输出空；下一步仅输入合成文字，未请求提交。


用户截图user-shared-input-entered.png确认在正确共享任务框键入合成文字，右侧截断，完整值待实际保存提案/预览诊断核对，不仅凭可见前缀认定逐值相同。下一步仅使用DeepSeek区“保存真实模型任务提案”本地接纳，不点击上方stub提交或真实外发确认；保持零Key/联网边界。


用户点击保存真实模型任务提案后截图user-local-proposal-preview-visible.png显示本地scope和多个完整预览分块，选择保留；截图边缘截断，完整ID/输入、持久化及未确认/未发送状态待复制实际诊断核对，不仅凭显示推定保存成功。下一步仅复制诊断，不确认外发、不读在线数据库。


提案1实际复制诊断user-copied-diagnostic-proposal-1.json确认run490a…13f0/client panel-1a1a089968c-2，phase3/committed=true/terminal=false：本地已保存并等待确认。完整输入逐值相同，history=[]、上下文仅白名单投影及当前输入；memory.read及revision/value精确，唯一实例1/memory.document、host/open epoch与r1及read scope固定。disclosure_confirmed/saved和model_request_saved均false，request ID空、remote not_sent、model_calls/tool_calls0；凭据仍not_configured。软件状态不是关后库证据或精确HTTP/RPC计数，也未执行实际read业务。下一步仅输入未提交草稿以检查面板保留，不确认外发。


用户再次反馈“哪个框？你这框也太多了”，截图user-input-ambiguity-after-proposal.png证实多个无标签空框难以识别；共享输入为回复此问题下方两个空框中上面较高者（本图约y588–657），下面窄框为资源ID。未输入未提交草稿，不能预判草稿保留通过。截图另可见sequence1合成user消息和已保存任务行；改为明确相对位置并允许简短反馈，不要求反复截图。易用性问题保留，原因/修复仍未执行。


用户截图user-unsent-draft-before-collapse.png确认未提交草稿已填入正确共享任务框，焦点/插入光标可见；未请求提交或覆盖原提案。开始检查收起/展开保留；仅同宿主草稿，不承诺进程重启持久化。


收起检查失败：用户报告点击AI后“框残留、点了没用”，全窗口截图user-collapse-residual-unresponsive.png显示右侧工作台仍绘制、草稿及原任务可见。不能按“应该收起了”推定逻辑状态或正常保留通过，也不能仅凭截图宣布UI线程死锁。停止相关交互/提交/确认，不重复AI或重开追求通过；只读检查原收起链assistant_expanded→投影assistant→DOM display→workspace布局/原生region，根因尚未定位。本轮不修产品。原任务最后诊断为已保存等待确认、0模型/工具调用且未发送；异常后实际状态待安全关闭及关后只读库核对。


## 2026-11-03第一批最终结果（暂不通过）

用户按一次Alt+F4并回复已关闭；监督进程持有的实际句柄捕获PID8708于16:48:22（Asia/Shanghai）退出，exit_code=0/0x00000000，未请求强制终止，唯一重试无循环。监督运行3736.75秒不是预定义人工长时验收，不扩大覆盖。retry-1-exit.json的normal_close_proven=false是监督器不自行判断用户操作的固定边界；结合用户反馈、实际exit0及关后库确认正常退出路径，另记closeout-observation-summary，不改写原监督记录。没有生产逐函数service/workspace_destroyed遥测，不冒充专用RPCB启动器标记。

关后确认无WAL/SHM，以mode=ro&immutable=1只读本轮库，读前后SHA256均497E4172800723BA5B115F4C69E781868789A9094B1AC071CD299AA1A391ED45，schema1。1主对话、1任务、2消息、3顺序事件、0工具；accepted→awaiting_disclosure→finished，唯一终态phase7/outcome=cancelled/terminal=1。输入、scope和固定目标与用户复制的原已保存提案逐值匹配，原未提交草稿未变成第二任务；不承诺草稿进程重启持久化。

终态payload仍disclosure_confirmed=false/disclosure_saved=false/model_request_saved=false，provider_request_id空，model_calls=0/tool_calls=0，remote_delivery=not_sent。没有确认、模型请求、发送、响应或usage事件；模型配置和隔离ref正确、实际诊断凭据not_configured。结合未进行Key/外发操作及生产发送前合同，记录零模型派发路径证据；单次TCP快照无匹配连接仅有限佐证，无全程抓包/ETW或精确WinHttp进入遥测，普通宿主无RPC M/R/S/C/U策略，不虚构计数。

| 项目 | 最终判断 | 证据与边界 |
| --- | --- | --- |
| 匹配正式程序/隔离/未配置 | 已覆盖 | 冻结identity与生产exe/DLL/uapp匹配，精确配置路径/ref；实际未配置诊断，默认无任务 |
| 真实点击、输入、滚动、焦点 | 局部覆盖 | 用户实际操作截图及复制回传；无完整Windows来源分类证明，未执行IME专项 |
| 本地模型保存 | 已覆盖 | offline-fixture/revision1配置已读回，助手只读自有文件确认；非Key可用或服务端验证 |
| 历史默认不外发 | 已覆盖默认边界，选择未验 | scope history=[]，上下文只有当前输入和明确投影；已出现合成消息sequence1，未完成选历史后再次提案 |
| 结构化read目标/能力/字段 | 已覆盖选择与固定记录 | 实例1/memory.document、memory.read、revision/value；固定host/open epoch/r1及read scope；没有实际工具执行，不宣称本轮10值回读 |
| 本地接纳/保存/分块预览/诊断 | 已覆盖本地状态；视觉全量未完 | committed等待确认、输入/范围逐值正确，预览出现、实际复制可用；完整instructions/tools逐块可读性未全部人工检查 |
| 收起/展开、输入选择保留 | 失败 | 用户报告点击AI后面板残留且控件点击无响应；残留像素不是有效草稿恢复证明，未继续展开追求通过 |
| 标签切换/重开/页面重建 | 未验 | 当前生产rpc_memory包单实例，不使用私有接口或其他测试DLL补成通过；因收起异常停止 |
| 取消和正常退出 | 关闭导致取消及退出已覆盖 | 用户Alt+F4、真实exit0、唯一cancelled已保存；独立取消按钮和重启后UI恢复未验，未再启动 |

### 发现与最小后续范围（未实施）

1. 必须先整改的人工问题：工作台点击收起后残留且用户报告无法交互。已确认的是现象，尚未证明逻辑收起标志/绘制/命中各自状态或根因。只读源码关联main.c assistant_expanded切换（约770行）、send_state/layout/update_region（约497/566/555行）、host.html assistant display投影（约341行）及Light可见性/布局/重绘/命中链。最小下一轮定位并同步这些状态，补受控收起/展开/重建及真实操作相称回归；不允许简单绕过显示门、强制重开或扩大来源政策。
2. 已观察的易用性问题：多个输入框无清晰可见标签/占位，用户两次无法定位任务输入；需给共享输入/问题回复/资源/脚本明确常驻用途提示，并相称验证。底层placeholder未显的原因尚未确定，不归因用户。

上述是下一轮最小建议，不是本轮实现或额外授权。第一批限定人工验收暂不通过；可复用本轮已覆盖结果，但整改后新源码/程序需新身份和相称验证，不能自动继承原冻结快照全部结论。原构建失败回调窗口限定独立通过和冻结追记仍仅指原快照，不代表正式宿主日常体验完整通过。

### 保全与停止

首次启动材料和未知退出码、原WAL只留哈希未及时复制限制保持；本次重试前实际已无WAL，当前库和首败材料已独立复制，没有重置/退款/覆盖。初始evidence-files.json/round-identity.json是当时记录，不更新成当前运行身份；动态数据库变化另记，最终以manual-first-final-identity.json及final-*清单关联。保留用户截图、实际复制诊断、stdout/stderr空文件、监督脚本/意图/PID/心跳/退出、只读库观察和实际自有DB/配置；不清理历史，无本轮需删的无用产物。

原ASAN30.08秒、ninja首次stderr限制、历史期限/frontend/输入/browser/ASAN-Light首败、已有T3远端/费用unknown及483 Tokens等独立保留。IME/DPI/视觉专项/辅助技术/人工长时/专用环境断电和browser九副作用不扩入本批，不预填通过。本轮无源码修改、构建、离线测试、真实凭据或Provider调用，不再启动或自动进入其他阶段。
