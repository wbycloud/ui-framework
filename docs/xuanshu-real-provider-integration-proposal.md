> 2026-10-31冻结追记：上一轮限定独立复审已通过，两项整改匹配快照可以冻结；本轮仅核对身份并落盘，无新测试/联网。见[独立复审记录](validation/xuanshu-rpab-independent-review.md)。用户已确认RP-C模型、隔离凭据、合成数据及5次/费用/超时方案，详[RP-C规格](xuanshu-rpc-validation-plan.md)；准备实现和真实调用均未授权。下文旧状态保留为历史。

> 2026-10-31限定整改：发现数组/完整描述投影及cleanup扫描引用已最小修复，离线证据见[整改记录](validation/xuanshu-rpab-closeout-validation.md)。没有改变RPAB-1公共合同、授权真实调用或重新开放AI写入；待下一轮限定独立复审。

> 2026-10-29 RP-A/B实现增量：本提案的接口草图及建议值已由本轮明确授权细化为[RPAB-1冻结合同](xuanshu-rpab-contract.md)，并实现独立模型配置、内部外发确认、DeepSeek生产传输和Light投影。实现/离线结果见[独立验收](validation/xuanshu-rpab-validation.md)。下文“尚无实现”等描述保留为提案时点事实，不能套用为当前状态。RP-C真实服务、实际Key和费用均未授权/未执行；P6R-1原快照验收不自动覆盖新增代码。

# XuanShu真实Provider接入实施提案

状态：Proposed；日期：2026-10-28。已确认的产品选择单独列出，其余接口、默认值和分批方案均待规格评审，不是实现或联网授权。读者为框架、宿主及应用能力开发者。P6R-1限定冻结见[独立复审记录](validation/xuanshu-p6r1-independent-review.md)。

## 1. 本轮结论与已确认选择

用户本轮确认：沿Responses协议推进，首批先完成DeepSeek官方配置，OpenAI后补；外发仅为当前任务、明确选取的历史、所选目标必要的只读能力说明与结果，由宿主逐任务展示并确认。不默认上传整条主对话、其他应用快照或文件。此前“OpenAI官方Responses首批”的选择由后续DeepSeek优先决定修订，不能继续将OpenAI列为首批必交项。

凭据沿用已确认的用户级Windows凭据引用方向；用户明确可能使用DeepSeek API Key，不把认证限定为OpenAI密钥。推荐首批使用DeepSeek的Bearer API Key，不同时加入ChatGPT登录/OAuth、第三方中转或任意兼容地址。模型ID由用户显式配置，不选定或承诺某个账号可用的型号。本轮没有读取、索取密钥、枚举凭据、验证账号或发送模型请求。

一个workspace仍只有一个Service、一条可恢复主对话及同一能力目录。新增真实只读请求种类不改变既有stub、P4编辑及P5图的行为。真实模型没有调用P4/P5修改、授权、审批、图调度或来源签发接口的工具。接通模型不意味着修改授权。

## 2. 实际源码与缺口

本轮文档修改前，框架为`codex/p1-process-boundaries` / `e0eca03db25930be55bdb45477027d1793a61904`。374项产品源码、102项最终产物及15项原整改交付与P6R-1清单匹配；本轮只核对身份和下列链路，没有重做P2—P6全面复审或运行测试。`D:/XuanShu` Git读取Permission denied，无扩权；下列实现事实均来自框架，不假称参考工程具有已移植的C Provider。

| 环节 | 源码已实现 | 需要新增或适配 |
| --- | --- | --- |
| Service/提交 | `include/ui_framework/ai_service.h:28–42,69–87`；`src/ai_service.c:104–128`。单workspace、复制目标、client/run关联、幂等、队列和控制路径 | v1请求字段是stub_plan_json，不能改其含义来暗接网络。独立真实任务结构、配置/外发确认和请求身份 |
| Provider | `src/ai_service.c:233–276`直接解释text/discover/tool/ask/fail脚本；不是独立网络Provider | HTTP、SSE、协议状态机、模型请求/响应关联和退出屏障。整体设计中的xs_provider_ops只是建议，不能当成现成功能 |
| 流式输出 | `src/ai_service.c:202–211`从完整stub字符串切片并发展示快照 | 网络任意字节分片、UTF-8残留、SSE边界、完整事件及协议终态；不能直接把网络chunk传给ai_append |
| 工具 | `src/ai_service.c:213–231,320–349`已有完整JSON/JCS、固定目标、read筛选、prepare/bound/dispatch/result/forget | 从模型function_call生成同一只读桥接请求，按需发现、调用批次完整聚合及去重；不另建目录或业务handler |
| 存储 | `src/ai_store.inc:17–24,97–133`已有接纳、工具阶段、事件和唯一终态；`src/ai_reliability.inc:23–37`恢复前审计 | 模型请求、外发确认、完整响应/工具批次的有界版本化记录及恢复审计。现有持久化不能等同于已有网络审计 |
| UI/关闭 | `examples/framework_host/ai_panel.inc:18–32,71–125`、`main.c:861–877`；Light面板、折叠、poll、REFUSE及安全退出 | 用户级模型设置、密钥原生输入、外发范围确认、真实/Stub区分、网络错误与未确认状态投影 |
| HTTP参考 | `apps/browser/src/browser_net.c:63–151`使用WinHTTP | 可参考OS调用及取消思路，但该模块是browser业务，不是可直接复用的Service传输层；不搬入其等待/下载策略，不修改browser |

SDK0.10-dev/API10/标准10、ABI1/包1、现有公开结构和枚举值、v1只读语义、冻结C2/P4/P5、固定yyjson/SQLite/JCS/SHA-256及现有限额保持。新SDK头只进入匹配开发快照，不覆盖冻结SDK。

## 3. 协议选择与兼容边界

### 3.1 首批与备选

| 方案 | 位置 | 代价及边界 |
| --- | --- | --- |
| DeepSeek官方Responses文本/function子集 | 用户已选首批；建议profile ID为deepseek-responses-v1 | `POST https://api.deepseek.com/responses`；一份显式兼容表，固定来源，不接任意base URL |
| OpenAI官方Responses | 后补候选，不属于首批验收 | 可以复用传输/SSE骨架，但须单独审查参数、推理Items、凭据origin及离线/真实证据 |
| Chat Completions | 备选，当前不实施 | 消息、流式终止及工具关联不同；不在出错时自动切换协议 |
| Anthropic、中转、Azure、OAuth、其他端点 | 后续另评审 | 认证、账户、数据及错误合同需要各自证据，不能由“OpenAI兼容”概括 |

DeepSeek官方已文档化Responses、语义SSE及完整函数调用；其`parallel_tool_calls`被忽略、部分参数不支持，`developer`角色和推理内容也有兼容差异。因此建议使用instructions承载框架策略，不依赖developer角色提升权限，不把服务端参数当成本地安全门。[DeepSeek兼容说明](https://api-docs.deepseek.com/guides/responses_api/)、[创建响应](https://api-docs.deepseek.com/api/create-response/)。

### 3.2 拟冻结协议子集

请求只含经profile允许的`model`、`instructions`、`input`、`stream:true`、`max_output_tokens`、普通function工具和必要tool_choice。使用客户端保存的明确历史，不依赖远程conversation或previous_response_id。DeepSeek profile不发送其不支持的store/background/include/metadata字段；不把响应store:false解释成全部服务日志零保留。

首版优先明确非思考模式并核对所选模型支持，参数映射按官方`reasoning.effort`规则实现；若需要思考模式，必须保留其要求的有界续轮Items并补独立用例，不能丢弃必需字段再宣称支持。该模式选择与具体模型支持表属于实现前待冻结项，不从模型名称猜测能力。

只开放文本和自有function工具；图片、文件、音频、远程MCP、内置搜索、custom/apply_patch、computer_use及其他远端执行工具不发送。未知执行类输出拒绝，不因服务端忽略某字段就降级放行。首批不承诺OpenAI全协议兼容。

DeepSeek允许一轮产生多个function调用。完整验证整个响应批次后，按原单工具邮箱逐个串行处理；每项计入原64工具/64步骤/16模型调用上限，预算不足则批次在首项执行前拒绝，不提高并发或引入Coordinator。函数调用与输出按call_id配对。[DeepSeek接口合同](https://api-docs.deepseek.com/api/create-response/)。

## 4. 用户配置、凭据与外发确认

### 4.1 一个用户级入口

建议使用`LOCALAPPDATA/ui-framework/xuanshu/model-config.json`保存一份活动profile：配置格式版本、profile ID、revision、协议/profile ID、固定官方origin/path、用户填写的模型ID、凭据引用、下调限额及超时。它是宿主配置，不是应用业务文档，不进入uapp或冻结SDK。异步原子保存成功后才显示“已保存”；不在读取配置时自动联网测试、列模型或查询余额。

普通JSON不含密钥、Authorization头、代理密码或完整模型请求。运行任务固定profile revision、origin、模型ID和外发策略版本；编辑、删除或撤销profile封闭相关新派发并请求取消，既有执行按原生命周期收尾。不把排队任务静默改到新模型、账号或端点。多workspace可读同一配置快照，各自仍只有一个Service。

### 4.2 Windows凭据引用

建议由宿主原生密码输入界面处理DeepSeek Key，后台用Windows Generic Credential保存，采用同用户/本机持久范围；引用绑定profile UUID和origin指纹。密钥不通过Light页面、页面消息或模型上下文传递，不提供给uapp获取明文的公共接口。`CRED_PERSIST_LOCAL_MACHINE`不表示跨用户共享，官方定义为该计算机上同用户后续登录可用。[Windows凭据结构](https://learn.microsoft.com/en-us/windows/win32/api/wincred/ns-wincred-credentialw)。

取用仅针对框架自有命名空间中、所有权及origin均匹配的明确引用，不接受任意Windows TargetName，不枚举或尝试已有密钥，不读环境变量或其他应用配置作为自动回退。输入拒绝CR/LF/NUL及不合法Bearer字符，长度在写入前核对Windows凭据上限和本地预算。临时明文缓冲受原预算约束、使用后清零并CredFree；不记录头部、明文、可逆编码或“用于核对”的密钥哈希。新凭据写入成功再发布引用；中断产生的孤立新项只在能证明本程序所有权时清理，不覆盖旧项。密钥更新/删除失败明确可见。

同进程DLL、调试器或同用户权限攻击者仍可能读取内存/凭据；这是已接受的合作边界，不承诺密钥硬隔离。首批用户自有Key不能作为共享产品内置Key打包。日志只记录run/request、profile revision、HTTP状态、脱敏错误分类、耗时及usage是否已知；原始服务端错误正文不得默认入日志。

### 4.3 逐任务外发确认

宿主展示服务商及完整origin、模型、当前输入、选择的历史消息/字段、目标别名及资源身份、只读能力范围、允许结果字段/类别和字节上限，再取得新的框架人工事件确认。自由文本、stub脚本、工具结果、source字符串、应用自报标志及页面confirm=true不能签发确认。

确认绑定任务请求摘要、profile revision、目标/host/open epoch/revision、历史消息ID及内容摘要、目录/描述指纹、允许外发的结果投影和有效期。首版建议最长30分钟且不续期，仅限本任务；排队不延长，取消、结束、关闭、重启或条件变化使后续外发失效。它是外发同意，不是C2业务调用凭据，也不是P4写授权。期限是本次建议，不能直接借用P4授权对象。

未来结果内容无法在首次确认时全部预览，因此必须明确允许的能力、字段和数据类别；只读结果也是潜在敏感数据，不能因read就默认可上传。超出已确认投影时暂停并重新展示，不能自动扩范围。首版不承诺通用PII自动识别；字段裁剪由宿主已确认的投影规则执行，应用说明和模型要求不是放行依据。未知结构或无法完成投影时拒绝外发。

完整主对话仍留本地，默认无自动近期历史上传；用户选择的历史包含工具数据时同样受投影和上限限制。不读取文件或抓取其他应用内容补全上下文。对发往模型的业务数据只作最小必要暴露，敏感内容不进入普通诊断日志；用户可在本地任务记录查看外发清单。

## 5. 独立接口草案与错误语义

以下名字、字段和布局均为建议，当前仓库不存在这些新入口。实现前须完成公共C/C++头及开发快照审查；不扩写既有ui_ai_task_request_t或把stub_plan_json改成网络配置。

```c
/* Proposed: include/ui_framework/ai_model.h */
typedef struct ui_ai_model_profile {
    uint32_t size, version;
    const char *profile_id, *protocol_profile, *origin, *model_id;
    const char *credential_ref;       /* Reference only; never a secret. */
    uint64_t revision;
} ui_ai_model_profile_t;

typedef struct ui_ai_real_task {
    uint32_t size, version;
    const char *client_request_id, *input_utf8, *profile_id;
    uint64_t profile_revision;
    const ui_cap_target_t *targets;
    size_t target_count;
    const char *history_selection_json, *disclosure_projection_json;
} ui_ai_real_task_t;

ui_status_t ui_ai_model_configure(ui_ai_service_t *,
    const ui_ai_model_profile_t *, const char *operation_id);
ui_status_t ui_ai_model_get_state(ui_ai_service_t *, char **sanitized_json);
ui_status_t ui_ai_real_propose(ui_ai_service_t *,
    const ui_ai_real_task_t *, char run_id[65]);
ui_status_t ui_ai_real_continue(ui_ai_service_t *, const char *old_run_id,
    const ui_ai_real_task_t *, char run_id[65]);
```

profile的origin须与显式协议模板完全匹配；字段不是任意URL执行入口。history/projection字符串采用严格JSON/JCS和size/version规则，总和计入原上下文预算。普通配置请求的OK只表示收到，保存确认通过异步op ID关联；get_state无明文秘密。

real_propose在通知前建立client/run关联，持久化接纳后处于WAITING_INPUT及新的`detail=awaiting_disclosure`，此时零HTTP请求。真实模型提交有独立request_kind，幂等摘要含profile revision、输入、目标、选取历史和外发规则；同client同内容返回既有run，同ID异内容沿ALREADY_EXISTS，不重复发HTTP。旧submit始终走stub。

worker须显式按request_kind路由，真实任务只有在外发确认及模型请求保存后才能进入Provider；未知种类拒绝，不能落回当前ai_provider_tick的stub脚本分支或图调度分支。内部Provider只需私有start/copy-result/request-cancel/try-dispose函数，不导出应用可注册的Provider函数表。模型可见工具集合也不含real_propose、配置、确认、P4编辑或P5图接口。

建议新增独立`ai_model_ui.h`，采用host-owned binding结构绑定Service、待确认摘要、现有Light标准控件及操作种类。只有框架私有匹配人工事件链能够一次消费该绑定；不导出接受任意token/source/布尔值的confirm函数。凭据输入/外发确认均不复用P4批准对象，模型不能取得它们。静态嵌入宿主使用等价受信入口，未适配来源明确不支持确认。来源的物理设备证明限制和延期验收保持。

复用现有cancel/get_run/history/query/poll/close/release，不增改旧状态枚举值。协议细节放在新JSON投影内：awaiting_disclosure、request_saved、sending、streaming、waiting_read、draining、delivery_unknown等；不能把远端生成状态未知伪装为SQLite的UNCONFIRMED或业务UNKNOWN。

沿用INVALID_ARGUMENT、PERMISSION_DENIED、PENDING、LIMIT_EXCEEDED、NOT_FOUND及ALREADY_EXISTS数值和原意义。新增详情code建议包括credential_unavailable、disclosure_expired、profile_changed、protocol_invalid、network_delivery_unknown、provider_rate_limited、request_limit、read_target_changed、storage_unconfirmed；不为方便测试改变旧返回合同。

## 6. 线程、网络及所有权

建议使用Windows自带WinHTTP异步接口，不引入HTTP/AI SDK或更新固定依赖；必要时只增加系统库链接。Service仍保有唯一SQLite串行worker。WinHTTP回调只发布稳定ID和有界复制数据，现有按需worker负责协议解析/存储推进；不在SQLite执行通道上同步等待网络，不在UI或回调中等待队列空间。是否需要额外私有parser worker须由实现前实测设计判断，不能默默突破原资源/生命周期边界。

网络job固定service session、run ID、model_request_id、attempt、job generation和profile revision。签发前建立关联及run引用；引用覆盖发起函数退栈、异步回调、结果待消费及存储消费。回调可能早于发起API返回，不能仅凭完成回调释放尚在发起栈使用的对象。所有新UI公共方法和应用回调继续持有Service方法/回调租约。

一个job拥有session/connection/request句柄及解析缓冲；句柄访问由job状态串行化。取消只向网络控制器发幂等关闭请求，不能再使用已关闭句柄；等待HANDLE_CLOSING及相关回调退栈后才释放context。将callback设NULL不等于回调已经退出。[WinHTTP并发合同](https://learn.microsoft.com/en-us/windows/win32/winhttp/concurrency-in-winhttp)、[回调合同](https://learn.microsoft.com/en-us/windows/win32/api/winhttp/nc-winhttp-winhttp_status_callback)。

拟采用16KiB单个读取块、一个未消费入口槽和单独终态槽；只有槽可用才发下一次read。回调复制失败返回可观察背压并请求停止，不阻塞回调、不丢终态。解析缓冲/请求/结果全部计入原Service/工具邮箱额度；控制预留不能被流正文占满。UI poll唤醒判断须包含新job/完成待消费，防止重现P6漏唤醒；done引用移交和稳定身份检查沿P6R-1，不只保存可复用slot。

生产只允许https和固定DeepSeek origin；标准证书/主机名校验，不忽略TLS错误、不跟随重定向、不跨origin转发Authorization，不自动发送Windows登录凭据。系统代理可由后台读取和解析，但不自动降低安全策略或提示索取代理密码；代理不满足条件时明确错误。任意endpoint、HTTP明文、本机地址及自签名例外只可存在于隔离测试传输，不能导出或改变生产政策。

建议网络阶段超时：连接10秒、首个有效响应30秒、有效流进度空闲30秒、单次请求总120秒；配置只允许下调，排队不偷偷续外发确认。这些是本提案待冻结的网络策略，不改原5秒取消停滞提示、20秒旧CTest及其他历史门槛。超时请求取消并排空，不能以期限到达强杀线程或伪造执行者退出。

## 7. UTF-8、SSE、工具与固定目标

按字节增量解码UTF-8，最多保留未完成码点的残留；不把分片中途的半个码点当错误，也不接受终结时残缺/非法UTF-8。SSE支持CR/LF跨块、空行事件边界、多行data和注释心跳；event名称与JSON type矛盾、重复键、非法JSON、非法控制字段或计量溢出时失败。心跳不代表模型有效进度，不能无限延长总期限。

协议JSON也受深度16、节点4096、数组512、对象128字段约束；工具参数仍使用原64KiB和最小Schema校验，不因外层协议事件更大而扩大业务输入。超出协议子集是明确拒绝，不静默忽略执行类字段。

Responses使用语义终态而非Chat的[DONE]。只有完整response.completed及所需item/arguments完成、终态数据一致、网络退出屏障成立，才接纳工具批次；arguments.done本身不能触发业务。failed/incomplete、断流或只到部分JSON都不派发其中的候选工具。文本delta仅是展示，最终内容必须与完整聚合结果核对。[DeepSeek流合同](https://api-docs.deepseek.com/guides/responses_api/)。

本地关联为`run_id/model_request_id/remote_response_id/item_id/call_id/local_tool_id/P2_call_id`。远端ID是受限不透明数据，不直接塞入现有64字节本地ID；建议远端ID最多256字节，生成SHA-256本地ID并在有界映射中保存原值。事件序号重复且内容一致时幂等忽略；同身份内容冲突、顺序非法或重复工具ID异参则整个未派发批次拒绝。迟到job只释放自己的复制体和引用，不能命中新任务/重开实例。

模型侧建议只提供两个function包装：`xs_read_discover(target_alias, category, page)`与`xs_read_call(target_alias, capability_id, parameters_json)`。所有字段required、包装Schema封闭；parameters_json完整字符串再经原严格解析/重复键/JCS及应用Schema检查。它们是协议映射，不是第二套应用目录或执行器。目录分页、描述版本和白名单来自同一能力源；首批不动态暴露全部256项工具，更不把应用能力ID当作模型函数名随意改写。

target_alias仅映射提交时复制的最多16个目标。模型不能指定新实例、改变epoch/revision/scope或选择当前活动标签；派发前及最后一次应用回调后继续检查现有门。call能力必须已在该任务可见read目录、允许外发范围和当前描述中，非read/未知/退休能力直接拒绝，不能回退legacy、C2人工或P4批准入口。工具说明和结果均是非受信数据，不提升为instructions。

多工具批次先做全体形状、ID、工具预算、目标和可见范围校验；随后每次实际dispatch再次走原prepare/dispatch检查。共享目标revision变化时停止受影响调用，不改绑或重试。结果查询复制后，等forget及相关回调门退出；同步早完成、重复complete、取消、队列满、卸载沿P2已冻结合同。回读出错不能改记成功，也不能从执行计数推断业务结果。

## 8. 上下文、重试与失败

沿用输入16KiB、最多16目标、上下文128KiB、最终回复64KiB、模型调用16、工具64/步骤64、队列16、cache64、命令32、Service8MiB、控制256KiB、事件64项/2MiB、单查询512KiB及P2输入64KiB/输出256KiB。真实任务也受这些额度约束，不从P5或另一个隐形8MiB借额度。

建议将单次编码HTTP请求也限制在128KiB，含instructions、历史、工具Schema及转义开销；单个SSE事件256KiB、单模型请求累计流2MiB，均是原额度内更小的协议子限额，不放宽原值。字段名、ID映射、header解析和尚未消费失败对象同样计账；分配前预留，退出后归还。系统TLS/WinHTTP内部内存另作资源遥测，不能宣称由Service计数穷尽。

工具输出允许本地256KiB，不代表都能装入128KiB外发请求。无法在已确认字段投影和总额度内表达时停止并显示限制，保留本地完整结果；不静默截断JSON、历史或必要的工具/推理配对Items，不自动读其他数据补齐。usage缺失标unknown，字节数不冒充精确token；模型max_output_tokens不能替代本地字节/步骤门。建议输出token子限额4096，仍须模型profile支持和实现前确认。

首批建议应用层HTTP POST不自动重试，包括429/5xx和未收到响应；记录Retry-After但不自动排队。TCP自身重传不是第二次业务请求。明确尚未发送请求体、远端已接收、响应完成、传输退出和工具派发分别记录；请求体可能已发而结果未知时，远端生成/计费不确定，不能称“没有请求过”。

网络错误不得重放已执行read工具，重复provider事件不得重复dispatch。用户明确重试须创建新的关联run、重新确认外发范围并检查当前目标；旧client ID只查询既有请求，不作为重试按钮。只读工具也不能因无写入就允许隐式重试或忽略revision。

401/403、限流、5xx、TLS/代理、空响应、非SSE、非法内容、截断、拒答、输出超限分别映射脱敏详情。连接退出后可记录failed/cancelled终态；远端是否已停止仍可unknown，但不是业务UNKNOWN解除证据。若P4/P5已有UNKNOWN，网络任务的取消、错误、forget或重启不清除其门。

## 9. 持久化顺序与恢复

建议优先使用现有schema3事件表的版本化payload记录模型元数据，避免仅为网络新增schema；如实际实现发现必须DDL迁移，先提交精确理由和迁移合同评审，不自行升级或操作用户库。现有schema1/2/3兼容、备份和坏/未来库保护保持。

1. 建立client/run关联，复制并计账；保存真实请求种类、输入、目标和历史选择，持久接纳后显示等待外发确认。输入不因received清空。
2. 宿主人工确认具体摘要，原子保存外发确认及其绑定；仅在保存确认后准备网络请求。正文是该绑定的确定性投影。
3. 保存`provider_request`：本地请求ID/序号、profile/origin/model、输入与投影manifest/hash、字节上限、工具可见集合及确认引用。引用到的历史和描述必须可追溯到不可变副本/版本；日志不重复保存完整网络正文或密钥。
4. 确认提交后重查存储、取消、profile、期限和目标条件，再交传输层。记录发送阶段；提交成功后checkpoint出错仍保留已提交事实，但后继HTTP/业务不再开始。
5. 文本delta可合并展示；完整响应和工具批次验证后先持久化其关联及必要内容，再逐项复用现有tool_request→prepare→tool_bound保存→dispatch。
6. 收取工具实际结果、释放P2租约并确认tool_result保存；只有已确认、允许外发且未超限的投影才能构造下一模型请求。工具结果保存失败不继续网络或重跑业务。
7. 最终回复、任务完成和唯一终态原子保存后才报告持久化完成。网络结束、内存入队或UI显示都不是此确认。每个模型请求/工具批次都有幂等op ID及摘要，重复通知不重复推进。

存储不可用立刻阻止新HTTP请求和业务调用，已接纳连接/工具仅cleanup-only；取消后排空回调和实际执行者，不等待UI队列空间。完成通知满时保留可查询权威状态，不能为了关闭伪造保存成功。关闭顺序为封新任务/外发→取消网络/工具→继续poll→网络HANDLE_CLOSING及应用回调归零→存储flush/close→安全destroy；REFUSE恢复接纳不复活旧取消/确认。

重启只恢复消息和请求阶段，未完成记录NEEDS_REVIEW，已发送但未确认的网络请求标delivery_unknown；不自动查远端、重连、续流或重放工具。旧外发确认/凭据取用授权和进程目标不恢复；显式continue创建新run，重新绑定当前目标和外发范围。恢复审计须校验模型请求、响应、工具和确认关联，坏记录拒绝，不覆盖原库。

## 10. 分批实施与改动范围建议

推荐下轮只授权RP-A离线核心，完成后评审，再接RP-B设置与正式面板；RP-C真实服务验证需独立授权。两批共用同一真实Provider实现，不创建第二Service或演示聊天执行器。

| 批次 | 目标及预计文件 | 依赖/兼容 | 验收与停止条件 |
| --- | --- | --- | --- |
| RP-A：DeepSeek Responses只读核心 | 新`include/ui_framework/ai_model.h`及私有`src/ai_provider_internal.h`、`ai_provider_responses.c`、`ai_provider_winhttp.c`、`ai_model_config.c`；适配`ai_service.c`、`ai_internal.h`、`ai_store.inc`、`ai_reliability.inc`；新增协议/服务夹具、必要CMake和公共头注册 | 先冻结本提案的接口、超时/子限额及非思考profile；仅OS系统库，不加第三方SDK；v1与stub原行为保持。凭据/传输先通过隔离替身验证，不读取现有Key | 零真实Provider访问，完成配置引用→确认→流→批次read→结果保存→关闭的受控闭环。导出无fixture；任何来源放宽、存储前派发、重复工具或提前释放均停止 |
| RP-B：用户配置与Light接入 | 新`include/ui_framework/ai_model_ui.h`及必要私有绑定；`examples/framework_host/ai_panel.inc`、`host.html`、`main.c`；对应静态嵌入/Light夹具 | RP-A已稳定；原生密码输入不经过页面，配置保存与任务外发分别确认。P4/P5诊断和人工能力保留，但真实工具集合不含它们 | 配置/确认伪造拒绝、草稿保留、模型/Stub显式区分、错误可读、折叠/重建/REFUSE不丢关联。真实设备正例延期，不伪报物理验收 |
| RP-C：授权后的真实服务核对 | 独立验收文档、隔离数据库、明确选定的DeepSeek模型与测试内容；通常不新增协议功能 | 用户另授权服务地址、模型、使用自己的凭据引用、发送内容和调用/费用上限；本轮不读取Key | 真实流式、单个只读业务回读、取消和错误；不导入用户文档，不触发写入，不把账号/模型一次成功扩大为所有兼容服务通过 |

RP-A不接正式配置/聊天UI、不触碰stateful/EDA等业务模型、不迁移browser副作用、不用真实模型驱动P4/P5，不实现新的网络工具、Provider插件注册、OAuth、OpenAI适配、子Agent调度或AI写入。RP-B不扩大RP-A协议。RP-C如暴露协议/来源/预算合同冲突，停止相关调用并单独评审，不临时降级放宽。

RP-A实现供后续RP-B绑定消费的私有确认状态机；离线夹具仅通过隔离测试入口验证接纳后的闭环。RP-B尚未接入时，生产真实任务保持等待外发确认，不提供绕过UI的公开confirm或测试授权。RP-A可包含用户配置的后台存取接口，正式设置入口和实际Key输入仍留RP-B，不读取现有用户配置/凭据来让测试通过。

每批UI对象只在UI线程使用，后台只持稳定ID和复制数据；请求关联在执行前，回调借用期至返回，worker/通知待消费阶段引用均覆盖。解绑/注销/关闭等待真实租约退出，取消不等于已完成。不覆盖冻结SDK，公共头扩展必须匹配开发快照。

## 11. 离线自动验收矩阵

以下均为将来实现的验收规格，本轮零构建/运行。使用新的独立构建目录、隔离临时库及专用进程，保留首失败源码/匹配程序/日志/JUnit；原P6证据只复用未受影响且仍匹配的路径。

| 组 | 方法及必须断言 |
| --- | --- |
| 配置与密钥 | 假凭据后端，引用/服务商origin错配拒绝；缺失/撤销可观察；配置失败不显示保存；秘密哨兵不出现在页面、SQLite、事件、日志及错误。生产DLL无测试后端/接纳导出 |
| 外发范围 | 伪造消息/布尔确认拒绝；新人工事件只确认匹配摘要；历史未选不出请求，字段超范围零外发；profile/目标/revision/确认到期失效，输入保留；同ID同值不重复HTTP |
| SSE/UTF-8 | 在每个UTF-8、CRLF、JSON转义和事件边界拆分；多行data/心跳、空帧、重复序号同/异内容、残缺终止、非法键/深度/超长；只有完整匹配终态可推进，最终文本按具体字符串核对 |
| 多工具与来源 | DeepSeek忽略parallel参数的多调用响应；先整批校验，再按稳定ID串行read；重复ID/异参、未知function、内置执行项、半JSON、越界target及任何非read均零业务派发；真实模型不能获得P4/P5确认 |
| 实际业务 | 使用现有专用DLL/维护示例read值、对象ID和revision核对；真实workspace切标签后仍命中旧固定目标；实例重开/旧epoch及最后应用回调重入均拒绝，不以计数或UI文字证明成功 |
| 预算/通知 | 原队列满、输入/输出/上下文/请求/流/映射边界；分配前计账、失败observer重入有界、控制保留可用；工具256KiB但外发128KiB不能装下时明确失败，不截断/提高额度 |
| 网络错误/取消 | 受控WinHTTP函数表/传输替身发布早于返回、迟到、HANDLE_CLOSING、401/429/5xx、代理/TLS错误、断流、卡住取消；零自动POST重试，5秒停滞可观察，未退出destroy=PENDING |
| 存储及冷开 | request/确认/tool/final提交前后故障，含checkpoint提交成功而不可用；下一HTTP/execute为零、前提交仍存在、唯一终态；专用子进程两次恢复不重放、不恢复外发确认 |
| 生命周期 | 网络回调/工具worker/外部回调分别退出，槽/引用释放一次；真实DLL两个分支及静态嵌入安全卸载，UI无等待；Service关闭后无悬空回调，REFUSE不复活旧任务 |
| 回归/证据 | 公共C/C++头、原Service/tools/continue、P2只读、P4/P5原合同及Light受影响投影；匹配源/程序导出核对。原期限失败和本轮首失败分账，不改原断言/超时 |

内存协议替身可证明解析/控制合同，不能证明生产TLS、代理、Windows凭据或真实模型兼容。WinHTTP回调替身不冒充实际联网；将来如需本地HTTPS夹具，应采用隔离进程/证书配置，不修改系统信任或放宽生产校验。真实设备与体验仍留整体集中人工清单。

## 12. 待冻结事项、授权边界与来源

已确认：DeepSeek优先的Responses方向、OpenAI后补、逐任务外发范围和本轮仅文档/只读探索。仍属本次建议：RP-A/RP-B具体划分、API草案、外发确认30分钟、网络10/30/30/120秒、协议子限额、先非思考模式、事件payload存储方案及无自动POST重试。它们应在下一轮实施授权中明确冻结；不把文档落盘当作产品实现批准。

具体模型ID和账号可用性不阻塞RP-A离线夹具，但阻塞RP-C真实服务验证。当前无需提供任何密钥；未来使用宿主凭据入口，聊天中不索取明文。官方协议资料可变化，下一实现轮只复核选定profile所需字段，不自动升级依赖或开放新工具。

建议下一轮准确授权：仅RP-A、DeepSeek官方Responses文本/function子集、现有限额内的独立C接口/传输/记录/只读工具与离线故障验证；允许必要系统库链接和新独立构建目录；不授权真实API调用、读取已有凭据、正式面板、OpenAI适配、P4/P5真实模型写入或生产数据库迁移。RP-B及RP-C分别后续授权。

官方资料查阅日2026-10-28；下列资料支持协议事实，本文的本地安全门、队列、超时、接口和分批设计属于本次建议，并非厂商保证：

- [DeepSeek Responses指南](https://api-docs.deepseek.com/guides/responses_api/)及[接口](https://api-docs.deepseek.com/api/create-response/)：首批协议与兼容差异。
- [OpenAI流式Responses](https://developers.openai.com/api/docs/guides/streaming-responses)、[函数调用](https://developers.openai.com/api/docs/guides/function-calling)、[迁移对照](https://developers.openai.com/api/docs/guides/migrate-to-responses)：比较资料，不代表首批OpenAI实现。
- [OpenAI Chat Completions](https://developers.openai.com/api/reference/resources/chat/subresources/completions/methods/create)与[数据控制](https://developers.openai.com/api/docs/guides/your-data)：备选协议及不能将store:false等同于零日志保留的依据；不能把OpenAI政策移植成DeepSeek承诺。
- [Windows凭据](https://learn.microsoft.com/en-us/windows/win32/api/wincred/ns-wincred-credentialw)、[WinHTTP并发](https://learn.microsoft.com/en-us/windows/win32/winhttp/concurrency-in-winhttp)、[WinHTTP回调](https://learn.microsoft.com/en-us/windows/win32/api/winhttp/nc-winhttp-winhttp_status_callback)：OS所有权和凭据范围。

停止在真实Provider接入规格评审。未连接任何Provider，未读取密钥，未发送业务内容，未安装依赖、构建、运行测试或启动产品。
