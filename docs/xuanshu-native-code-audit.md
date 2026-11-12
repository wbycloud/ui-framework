> 2026-10-26最新状态：本轮先冻结[P4A-1](xuanshu-p4-controlled-edit-contract.md)，已完成P3租约收口及P4-A单资源夹具实现；Native44/44、Light49/49、ASAN44/44，限定为**P4-A开发阶段模拟及自动验收通过**。见[P3独立复审](validation/xuanshu-p3-independent-review.md)、[本轮验收/身份/首次失败](validation/xuanshu-p4a-validation.md)。整体架构仍Proposed，P4-B/C、正式面板、应用接入和真实人工均未完成；下文历史状态保留。

# XuanShu 原生 AI 子系统代码审计

> 2026-10-24 P3状态增补优先于历史阶段的“P3仅规划/未授权”表述；整体架构仍Proposed。[P3-1合同](xuanshu-p3-service-contract.md)与[独立验收](validation/xuanshu-p3-validation.md)记录本轮新增原生Service、SQLite、stub只读循环和全局Light面板，不能归为原XuanShu已可直接复用的C实现。旧P0/P1引用及A+B/C1/C2证据不重写。

状态：Proposed（整体）。审计日期：2026-10-22。下列基线表和F/X结论描述原接手源码；后获P2 A+B授权的增量见第7节，C1独立增量见第8节，不回写历史测试。

本文面向框架及应用开发者。配套文档：[集成设计](xuanshu-native-integration-design.md)、[应用能力合同](application-capability-contract.md)。这里的“已实现”指在下列源码基线上找到实现，不代表本轮验证运行成功。

## 1. 审计基线与证据规则

### 1.1 双工程身份

| 工程 | 实际身份 | 接手状态 |
| --- | --- | --- |
| ui-framework | `D:\应用软件框架\应用层序框架`；`codex/p1-process-boundaries`；`e0eca03db25930be55bdb45477027d1793a61904` | 本地 HEAD 与 origin 跟踪引用一致；未联网刷新。唯一已跟踪修改是 `.gitattributes` 的 interaction 历史证据规则；暂存为空 |
| XuanShu | `D:\XuanShu`；`main`；`67eacf5c47a856f5d3d68262f247659a15a13f60` | 根工作区干净；相对本地 origin/main 领先4次提交；未联网刷新 |

框架接手有5307个未跟踪文件，集中于既有 `docs/validation` 历史报告和证据树，均不属于本轮交付。根目录三份规划记录的旧末尾停留在早期语言轮，不能代替 [当前交接](handoff-current.md) 对当前状态的判断。本轮只追加独立 checkpoint。

XuanShu 的九个模块实际 HEAD 均与根工程 gitlink 相同，逐模块 `git status --porcelain --untracked-files=normal` 为空：

| 模块 | 实际 HEAD |
| --- | --- |
| agent-kernel | `34da2bd61bf4b1c6999c73a5a9c641c8e3a03334` |
| runtime | `9b890b4d2a4f1187f3eeb698e46a814026be7545` |
| context | `143b7e832b258bcd3742621bffb94898809f03dc` |
| storage-sqlite | `68fa21c8e13bc7c64ad8da82fee4b3a6a35c5726` |
| providers | `f37256f652a6d761ce2678da0a01f208965b14d2` |
| security | `ecee850ef3b073ee1b53fb1fedd967f8294d6baf` |
| artifacts | `b662cfe786feee592a9e936d447a881fc58c0109` |
| tools | `45b0c0295187a45b58c15f1b56066cc5d6bfb2eb` |
| cli | `3f273dfd1f59b878d47a187f66be1e2c80794004` |

初次受限 Git 读取 XuanShu 报 Permission denied；随后以获准的只读命令取得上述结果。根 `submodule status` 曾显示未初始化标记，不能据此推断磁盘模块无源码或无 Git；逐模块实际查询才是本表依据。没有初始化、更新或修复参考仓库。

### 1.2 版本与依赖

| 项目 | 声明或合同 | 本地核对与边界 |
| --- | --- | --- |
| 框架 | SDK0.9.0-dev/API9/标准9；应用ABI1/包1；Windows x64/C11 | 原接手时根CMake/公共头/政策一致；本轮API10增量见第7节 |
| Lexbor | `7fb22cf5664a331d7c24b113489e566767c9c25a` | 固定 checkout HEAD 相同，已有 tracked 状态干净 |
| QuickJS-NG | `2f0aa72a6b09cf69ff06399bcf9f4c083ac1a278` | 固定 checkout HEAD 相同，已有 tracked 状态干净 |
| WebView2 SDK | `1.0.4129.50` | 本地 header 和 x64 static loader 存在；未查询或启动当前 Runtime |
| Mesa | `24.3.4` | 本地 x64 osmesa.dll 存在；本轮不加载、不重新校验提供方归档、不处理历史退出问题 |
| XuanShu workspace | 根及审阅模块 package 为1.1.0；Node要求至少20 | package 版本不是行为对等证明；本轮不执行 Node/npm 版本或工具链检查 |
| XuanShu锁定依赖 | zod4.1.12、TypeScript5.9.3、vitest4.1.10、better-sqlite3 12.9.0 | package.json 与根 package-lock 条目一致；没有 npm install 或测试 |

XuanShu 是 TypeScript/Node 工程。Node Promise、AbortSignal、zod 和 better-sqlite3 不是可直接链接的原生 C 实现。本文使用“移植”表示重新实现行为及相应验收，不表示复制其运行时依赖到框架。

### 1.3 证据类别

| 标记 | 含义 |
| --- | --- |
| 源码已实现 | 找到函数体和可追踪调用；若只到接口，明确写为接口/局部实现 |
| 文档规划 | ADR、README或规划中的目标；Accepted 也不等于端到端已实现 |
| 本次建议 | 本案提出的 C、能力层、生命周期及存储合同，尚未进入产品源码 |

菜单专项原报告、本地424张PNG、历史SDK ZIP及10月22日文本核验目录可取得；菜单ZIP SHA256与原记录 `f035711b0c3350d1d5cba9a08cb11bad5c1ba211e72a57748cd481b54f8d3f0e` 相同。浏览器M2外部 verification/INDEX/SDK也存在，其SDK SHA256为 `5dd74ffda529ea2782c3c8117b248817dd427f66ad4d944622f9b85daad61880`。它们都不是 AI 子系统的新验证或新SDK。

10月22日329文件清单在磁盘无缺失；223份字节相同，106份仅LF/CRLF表示不同，规范到该清单的CRLF后相同。清单身份 `abbc8a65187a198b3b70422b108bbe7b83a839b05f2e7da65dfb3a7ba3d46297` 属于当时导出，不冒称当前全部工作树逐字节身份。原始材料不随clone提供，本轮不补写、复制或重新生成。

## 2. 框架源码审计

相对链接指向本基线的实际文件；行号是审阅定位提示，不是新测试结果。

| 编号 | 源码依据 | 源码已实现及限制 | 本次建议 |
| --- | --- | --- | --- |
| F01 | [宿主](../examples/framework_host/main.c)，host_window_t、visit_command、on_confirm、on_message、WM_CLOSE | 全局协议操作面板、独立目标实例、菜单、事务入口及关闭轮询；没有模型循环或常驻AI服务 | 面板只投影服务状态；不能用面板可见性作为任务寿命 |
| F02 | [workspace](../src/workspace.c):381、668 | 加载包/创建实例；invoke按稳定实例ID进入assistant，拒绝closing实例 | 增加文档/资源身份、版本及访问范围绑定；标签切换不参与派发目标选择 |
| F03 | [应用公共合同](../include/ui_framework/application.h):37、49、118 | create/mount/unmount/destroy、ALLOW/REFUSE/WAIT、module_shutdown、UI callback scope | 扩展服务与能力在途租约；旧scope不能用于后台保活 |
| F04 | [命令实现](../src/ui.c):679、983、1029 | 注册、pending、同步handler、单次reply；invoke接受调用方source字符串 | 普通source只作历史诊断；新可信来源由路由器确定 |
| F05 | [assistant](../src/assistant.c):139、174 | 第二份允许列表；validator可选；READ/EDIT/DESTRUCTIVE及同步confirm；先跟踪再调用host | 单一能力目录派生适配；不把缺validator解释成参数有效 |
| F06 | 同上:283、344、360 | 协作取消标记、应用snapshot、begin/commit/rollback/undo回调 | 不宣称杀线程、自动回滚、文档版本管理或自动撤销栈 |
| F07 | [组件](../src/components.c):432、454、693；[组件接口](../include/ui_framework/components.h) | committed控件值与draft分离；提交校验、选择、按需查询及generation；accept_fields复制draft | 控件“已接纳”不等于业务文档已保存；新适配需显式业务提交 |
| F08 | [workspace](../src/workspace.c):702、729、746 | 普通post_copy复制result/event/progress/close后入队；组件及app_data经预算检查 | 普通post_copy未扣8MiB预算、也未在入队时查目标；不能称所有队列有界。AI单独提供有界复制队列 |
| F09 | [app_data](../include/ui_framework/app_data.h)、workspace_poll_impl | 数据复制；UI接收、WAIT期间收尾、移除后丢弃；不携带模块函数指针 | 服务post携带ID/代次和纯数据，UI重新解析并查存活；不让worker保存host指针 |
| F10 | [workspace](../src/workspace.c):790、899 | depth防当前回调内卸载；模块清理失败保留DLL，workspace销毁失败保持有效 | AI引用、排队写入及pending审批须在unmount前处理；超时不授予强制卸载权限 |
| F11 | [最小EDA](../examples/minimal_eda/app.c):59、81、93、176、218 | add/zoom/clear确实改私有数组/视图，键盘复用命令；笔记仅原生EDIT；参数只支持空对象 | 专业能力显式表达对象/几何参数；现状没有对象稳定ID、文件持久化或撤销 |
| F12 | [stateful](../examples/stateful_components/app.c)、[夹具](../tests/api7_fixture.c):75、87 | wrapper引用测试源码；保存列宽/布局、语言偏好；test.edit计数，test.submit只accept_fields | 试点用于接入验证；新增业务状态适配及回读断言后才能证明编辑一致 |

[开发标准](application-development-standard.md)规定UI对象调用限UI线程，但不是每个函数体都有线程检查。未来桥接必须主动遵守该合同，不能靠“函数未报错”证明跨线程安全。UI中的enabled/busy、modal和应用前提也不能仅从菜单灰态推导完整执行许可。

现有双重注册示例包括 [counter](../examples/web_counter/app.c):66/91、[generic](../examples/generic_components/app.c):141/206、[EDA](../examples/minimal_eda/app.c):316/378。generic的framework_features变体增加菜单和关闭控制；stateful/夹具未注册同等Assistant能力。目录覆盖不等于业务覆盖。

## 3. XuanShu 源码审计

以下绝对链接只在有参考工程的本机有效。新的clone应报告缺失，不能用旧文档填补源码事实。

| 编号 | 依据 | 分类与结论 |
| --- | --- | --- |
| X01 | [Agent](D:/XuanShu/agent-kernel/src/agent.ts):60、82、109、162、180 | 源码已实现：普通run/resume和显式runDurable/resumeDurable分开。模型—工具循环、校验及终态处理存在；持久入口要求专用store/lease/authorizer/ledger |
| X02 | [工具协议](D:/XuanShu/agent-kernel/src/tool.ts) | 源码已实现：zod输入校验、生成模型Schema、tool policy、before/after、进度/产物；不能直接替代C应用业务前提和文档版本检查 |
| X03 | [上下文](D:/XuanShu/context/src/workspace-context.ts) | 源码已实现：读取workspace AGENTS、按轮保留历史和字符预算。不是精确token预算，也不是自动生成最小授权子上下文 |
| X04 | [预算](D:/XuanShu/agent-kernel/src/budget.ts) | 源码已实现：共享计数、depth、RootControlLedger序列化预留、幂等operation及不确定写核对；不能简化成每个Agent各自计数 |
| X05 | [协调器](D:/XuanShu/agent-kernel/src/coordinator/coordinator.ts)、[类型](D:/XuanShu/agent-kernel/src/coordinator/types.ts) | 源码已实现：静态DAG和动态admission，两者不能按README旧MVP误判；动态路径使用根ledger、lineage、队列/并发/深度上限及取消收集 |
| X06 | [取消](D:/XuanShu/agent-kernel/src/abort.ts)、agent.ts中的runJoinable | 源码已实现：信号、超时和joinable等待；durable停滞不伪造终态。C须使用事件/原子状态与真实worker退出，不能移植Promise.race后丢弃执行者 |
| X07 | [审批类型](D:/XuanShu/agent-kernel/src/types.ts):96、[取消/审批合同测试](D:/XuanShu/agent-kernel/tests/cancellation-approval.contract.test.ts):191 | 源码已实现：approval-v2绑定schema/input/principal/scope和一次消费；裸approved布尔只属legacy。绑定还需补入框架资源与revision |
| X08 | [事件](D:/XuanShu/agent-kernel/src/events.ts)、[持久协议](D:/XuanShu/agent-kernel/src/persistence.ts) | 源码已实现：普通同步observer及durable RunEvent是两种路径；事件head/owner generation、有限payload、append确认/核对协议存在。接口本身不证明有生产store实现 |
| X09 | [Runtime](D:/XuanShu/runtime/src/runtime.ts):131、175、325 | 源码已实现：ProviderRegistry、Session、context/toolset、Agent调用及discussion/project入口；当前普通入口仍调用run/resume，未调用runDurable/resumeDurable |
| X10 | [SqliteRunStore](D:/XuanShu/storage-sqlite/src/run-store.ts)、[迁移](D:/XuanShu/storage-sqlite/src/migrations.ts):3 | 源码已实现：snapshot load/save/delete和幂等键；schema最高5。未找到durable RunEventStore和ADR-018所述migration6生产接入，不能外推审批-v2恢复已端到端完成 |
| X11 | [Provider](D:/XuanShu/providers/src/openai-compatible.ts)、response/serialization | 源码已实现：TS HTTP适配、流解析、工具/结构化输出及请求指纹；C需重做传输、解析、取消和内存管理。本轮未调用 |
| X12 | [权限](D:/XuanShu/security/src/capability-policy.ts) | 源码已实现：缺risk/capabilities拒绝；scope匹配，低风险allow，其余审批。未绑定框架文档/版本，也不提供同进程恶意C代码沙箱 |
| X13 | [CLI配置](D:/XuanShu/cli/src/config.ts):9、85 | 源码已实现：支持文件apiKey和环境变量；不等于已有Windows凭据存储。新C配置改用凭据引用，禁止照搬明文配置 |
| X14 | [conversation](D:/XuanShu/agent-kernel/src/conversation/engine.ts)、Runtime.discuss | 源码已实现：多参与者讨论工具；不等于本案单主对话+私有任务子Agent。不要把讨论transcript广播模型当成本案上下文隔离 |
| X15 | [ADR-018](D:/XuanShu/docs/adr/018-run-event-observability-and-session-evolution.md)、[ADR-019](D:/XuanShu/docs/adr/019-bounded-child-agent-lifecycle-and-authority.md) | 文档规划：Accepted的可观测性/子Agent权威边界；部分内核已实现，Runtime/Storage尚有缺口；根规划历史checkpoint也不能当当前实现清单 |

静态核查的契约测试还包括 [lineage/budget](D:/XuanShu/agent-kernel/tests/lineage-budget-manifest.contract.test.ts)、[execution/events](D:/XuanShu/agent-kernel/tests/execution-events.contract.test.ts)、kernel-invariants各轮测试、[Runtime](D:/XuanShu/runtime/tests/runtime.test.ts)、[SQLite](D:/XuanShu/storage-sqlite/tests/storage.test.ts)。它们提供可移植的验收语义；本轮没有运行，未建立当前工具链全绿结论。

## 4. 复用、移植与重写映射

| 子系统 | 处理方式 | 原因与目标阶段 |
| --- | --- | --- |
| C宿主/应用加载/稳定实例ID/关闭scope | 复用并扩展在途租约 | 已有实际实现；P2/P3新增AI引用收尾，不能替代旧关闭责任 |
| host普通命令、Assistant允许列表 | 迁移到单一目录，派生旧入口 | 两份注册会漏项/漂移；P2五个维护示例包同时迁移 |
| 共同组件、按需source、图片和GL槽 | 复用呈现/查询，新增语义适配 | 缓存与draft不是业务数据；不做鼠标自动化替代，P2 |
| Agent模型—工具循环 | 移植必要合同到C | 不能链接TS；P3先stub只读，P4审批编辑 |
| Provider | C接口重写，P3仅stub | 传输与密钥生命周期不同；真实Provider须另获授权 |
| Coordinator/RootControlLedger | 行为移植 | P5统一调度，先建立P3/P4预算和事件基础；不新增第二套调度权威 |
| 上下文/能力目录发现 | 按桌面资源边界重做 | 主对话不是默认子Agent输入；Schema按需获取，P2/P3/P5 |
| approval-v2 | 移植绑定思想并扩展目标/revision | 不能复用旧同步confirm充当持久票据，P4 |
| SQLite snapshot/project store | 存储概念参考，C存储接口重写 | TS V1 store不是durable V2后端；不导入原数据库，P3基础、P4写入核对、P6增强 |
| 权限/文件边界 | 规则参考并收紧 | 原scope通配和trusted host metadata不是系统沙箱；应用范围由宿主发放 |
| CLI/TUI、benchmark/evaluation/recovery | 不纳入产品移植 | 属于另一运行入口和验收体系；不搬runner、旧项目权威或原授权 |

## 5. 关键缺口与停止条件

| 优先级 | 缺口/风险 | 必须满足的进入条件 |
| --- | --- | --- |
| 阻塞P2 | 没有统一目录、完整参数、资源revision合同；试点edit/submit语义不足 | 接受能力合同、完整矩阵及最小真实状态适配；覆盖缺口不得用返回成功抹平 |
| 阻塞P3 | 没有C Agent/Provider/service队列，普通post可能无界 | 有界stub、状态机、关闭及最小会话事件存储；失败关闭不强杀 |
| 阻塞P4 | 旧confirm没有持久审批绑定；store没有新持久协议；无应用真实事务 | 核对intent/结果、版本、单次审批消费及真实rollback/undo；任一未知结果停止该资源写入 |
| 阻塞P5 | 私有上下文、子任务权限缩减、共享根预算和collect尚未接入框架 | P3/P4基础先通过；再验证嵌套取消、失败spawn记账与收尾 |
| 持续边界 | C同进程模块能破坏宿主；凭据仅减少误暴露；本机历史测试不是新验收 | 不承诺内存/进程隔离；需要硬隔离时另立进程设计，不暗改本案架构 |

菜单宽度、tooltip、Light hover及受限浏览器M0/M1/M2已完成，不重复开发。Mesa多线程清理/真正卸载、Alt+F4、Runtime首次退出及同值粘贴等历史未闭环项继续按当前交接分账；AI设计不提供这些问题的根因或修复证据。

## 6. 相对草案的细化与本轮结论

1. “原生集成XuanShu”改为按阶段移植源码合同，不声称现成C内核；保留原生C、同宿主常驻方向。
2. 明确内核durable局部实现与Runtime/Storage端到端缺口，避免把Accepted ADR当可直接接入的后端。
3. 将最小持久运行记录前置至P3、写入结果核对前置至P4；P6完善持久化，不承担第一次防重复写入保证。
4. 试点先是框架示例验证；当前没有真实专业文档/撤销，后续业务应用验收不得用fixture通过替代。
5. 新AI队列独立定义预算；旧8MiB合同不能外推到所有post。

原P0/P1批次仅交付静态审计，不包含实现授权或产品运行结果；后获A+B授权的增量另见第7节及独立验证报告。

## 7. 2026-10-22 P2 A+B增量

已确认的D01—D05和草稿政策见[集成设计第8节](xuanshu-native-integration-design.md#8-proposed-决议未决项与交付边界)。本轮API10已增加独立[capability.h](../include/ui_framework/capability.h)，不改既有公开结构；普通命令/Assistant旧路径仍原样存在，尚未生成统一UI/Assistant注册视图，不能宣称双重注册已迁移完成。

已实现增量为[JSON/Schema/JCS](../src/capability_json.c)、[只读目录/资源/调用](../src/capability.c)及workspace关闭门；仅read效果，无UI绑定、示例模型、写入、AI Service和存储。yyjson0.13.0固定commit和三个原始文件哈希见[依赖锁](../third_party/yyjson/source-lock.json)。原Lexbor/QuickJS、XuanShu参考工程与冻结SDK保持只读。

本轮本地构建/测试、新增用例、失败和未验项统一见[独立A+B验证](validation/xuanshu-p2ab-validation.md)。原F/X审计与AC/XS未来矩阵不等同这些测试已经全部通过；完整P2仍未交付。

后续评审收口已在最后一次应用回调后重查派发前提，补真实DLL自有worker、受控回调退出、同步完成栈上租约和复制队列背压验证；没有改变公共合同。新结果和Native输入首次失败独立见[收口证据](validation/xuanshu-p2ab-validation.md#7-2026-10-22评审收口独立新证据)，不能据此关闭浏览器旧20项失败或workspace首次20秒超时。

## 8. 2026-10-22 C1独立增量

基线仍为`codex/p1-process-boundaries`、`e0eca03db25930be55bdb45477027d1793a61904`加已有A+B及评审收口工作区；不是干净HEAD实现。参考工程`main`、`67eacf5c47a856f5d3d68262f247659a15a13f60`根状态干净，只读核对，不刷新远程。既有用户修改、历史证据、固定依赖及冻结SDK保留。

| 分类 | C1源码事实与边界 | 依据 |
| --- | --- | --- |
| 已实现 | 能力ID与普通/Assistant旧注册双向冲突检查；退休期间ID仍保留，直到finish成功；无静默覆盖、alias或legacy回退 | `src/capability.c`、`src/ui.c`、`src/assistant.c` |
| 已实现 | 独立`capability_entry.h`，显式启用宿主只读兼容模式；一个能力发布一个同ID入口，复制完整目标，目录代次推进；普通记录是投影，Assistant动态读取同一目录 | [新入口合同](../include/ui_framework/capability_entry.h) |
| 已实现 | prepare后、execute前关联旧request ID和稳定call ID；终态向旧result回调投影，公共reply不能跳过输出校验；重复完成、取消、结果查询/释放沿用A+B | `src/capability.c`、`src/ui.c` |
| 已实现 | 结果/visitor回调纳入框架租约；workspace在下一安全poll回收其Assistant终态请求；普通host-invoke及独立Assistant调用方显式释放；不保留Assistant回调指针 | `src/assistant.c`、`src/workspace.c` |
| 已实现 | 模式启用后隐藏全部legacy Assistant项；旧EDIT/DESTRUCTIVE直接invoke拒绝，即使旧配置允许修改、source自称human；普通legacy人工命令仍可调用 | [专用入口用例](../tests/capability_entry.c) |
| 已确认、未实现 | 后续v2独立结构/入口属于同一能力合同增量；仅可信人工UI调用非read，AI/Assistant继续只读；共享name业务属性但各自草稿，EDA笔记显式提交/取消 | C2及后续授权前不导出这些接口 |
| 后续建议 | C2人工来源、effects/results与UI绑定；stateful/EDA真实模型及五示例完整迁移；P3—P6服务与执行体系 | 整体Proposed设计，不计入C1完成 |

迁移必须在应用源码中移除同ID旧普通/Assistant业务注册，再以新开发快照创建宿主、注册能力并发布目标；本批没有提供热替换旧执行函数或自动推断旧Schema。模式启用前有旧请求/回调在途时返回PENDING，不能把已运行旧编辑静默纳入新合同。未启用模式的legacy宿主保持原行为；五个示例和browser本批均未迁移，因此不能宣称所有历史Assistant入口已全局改为只读。

注册描述的名称/Schema/执行回调只有一份权威记录。普通命令的UI标题有可翻译副本，改变显示不改变能力元数据或指纹；其最多4095字节的占用预留进2MiB目录预算。关联字段、旧pending/request节点及有界source副本预留进8MiB调用预算，1024条上限保持。新结果走同步host observer，没有新增无界结果消息队列；后台仍使用既有复制app_data队列。

本批专用用例包含实际DLL自有worker/外部回调、队列满后重投和旧实例消息，复用A+B真实夹具但从生成入口进入；不冒称任意第三方线程自动追踪。新结果、首次失败、源码身份及保护检查见[C1独立验收](validation/xuanshu-p2c1-validation.md)。原浏览器20项失败、workspace20.05秒首次超时、Native输入首次失败保持独立未定位。

## 9. C1评审收口：源码事实与证据边界

本次仅修复三项评审问题，整体设计仍为Proposed。旧实现中validate/confirm/visitor等回调没有阻挡初次enable的租约，四个Assistant事务辅助API仍直接调用应用，prepare失败前的pending/source未实际计账；这些是源码缺口。新增有界回归在修复前得到48条失败，观察到错误激活、事务回调执行及失败通知重入；没有进行无界递归或宣称复现崩溃。

当前[assistant.c](../src/assistant.c)为validate、confirm、visitor、progress、cancel、snapshot及事务回调设置独立旧回调租约，初次enable返回PENDING，派发前重查策略；该租约不占能力dispatch的嵌套标记。[capability.c](../src/capability.c)和[ui.c](../src/ui.c)对生成pending及ID/source副本在分配前预留、observer返回后释放，Assistant跟踪单独计账；不再依赖prepare成功后的保守桥接估算。原8MiB内预留一个固定失败槽；失败observer中的生成提交在接纳前背压，不能递归制造失败通知。

启用模式后begin/commit/rollback/undo明确PERMISSION_DENIED，应用回调不执行。现有调用点为测试和framework_host的旧Assistant面板事务控件；这些控件在启用模式的host中同样被拒绝。普通人工业务命令没有依赖这些辅助API，仍可用；未迁移host保持legacy。没有借source证明人工来源，也没有引入P4事务或C2接口。

新增[收口用例](../tests/capability_closeout.c)与原测试独立；运行、身份及失败记录见[C1验收第7节](validation/xuanshu-p2c1-validation.md#7-2026-10-22-c1评审收口独立证据)。ABI1/包1、公开布局、版本、JSON/JCS与固定依赖不变；本次不迁移示例、不进入C2。


## 10. 2026-10-23 C2-A规格冻结：源码依据与未实现项

C1在本会话已通过限定验收；本节不是重新全面复审，也不是C2实现审计。当前框架仍为codex/p1-process-boundaries、HEAD e0eca03db25930be55bdb45477027d1793a61904；已有33项tracked修改、5334个未跟踪文件和空暂存保留，不将其归为本轮文档增量。参考工程上次身份main/67eacf5c47a856f5d3d68262f247659a15a13f60仅作历史记录；本次规划读取被权限拒绝，不能重报为当前已核实或干净。

| 分类 | 本轮只读依据与结论 |
| --- | --- |
| 源码已实现、复用 | capability.h/capability_entry.h及capability.c已有read目录、绑定、执行、稳定ID、桥接、预算/生命周期；C1第7/8节整改不重复开发 |
| 源码缺口、待实现 | menus.c以消息/字符串source调用旧命令；light_web.c私有原生和公开模拟输入合流，没有可信人工凭据；WndProc入口不能自证来源 |
| 源码缺口、待实现 | v1 target scope固定read，完成要求revision不变，不可伪装view/draft/write；独立v2字段和错误合同以C2A-1为准 |
| 源码事实、后续限制 | workspace.c每实例独立host；当前没有跨host共享资源写门，首批只单一资源所有者 |
| 源码事实、后批工作 | components.c保留modified草稿但accept_fields接纳当前所有字段；提交快照、base revision/冲突及选择性接纳归C2-B |
| 源码事实、后批工作 | stateful的test.edit仅计数，generic编辑/重命名已有内存修改记录；不能一概当成真实业务或一概重写 |

[C2A-1唯一规范](xuanshu-p2c2a-interface-and-acceptance.md)已冻结首批11接口/结构、私有一次性凭据、busy拒绝、未知只读核对、有界预算及A01—A15。read继续v1，v2只登记view/draft/write，不支持external；同一目录而非第二注册中心。Windows识别边界、单资源/单所有者、无AI写入及未迁移legacy边界均为明确限制，不是遗漏已实现能力。

本轮只有八份文档授权，无公共头/源码/测试/CMake/版本/固定依赖变更，无构建或测试运行。新验收矩阵全为未运行。整体审计设计继续Proposed，后续接口虽冻结仍未导出。原A+B/C1日志、red源码和匹配二进制及旧red缺失限制保持；原浏览器20失败、workspace首次20.05秒超时（20秒门槛）、Native输入首次失败独立未定位，不默认归因环境或用限定通过覆盖。

## 2026-10-23 C2-A＋C2-B实施状态（本次授权）

下列是本轮源码状态，前面“仅文档/未实现/未授权”段落保留为当时历史，不再表示当前范围。C1限定验收不重开；整体设计仍Proposed，C2A-1冻结合同及先于实现落盘的C2B-1独立增量保持。当前停在两批联合代码评审，未授权C2-C/五示例业务迁移/P3—P6。

源码已实现：同一registry中的v2 view/draft/write、Windows私有Light菜单语义接纳、一次消费/固定目标/版本/效果、资源busy门、稳定ID/取消/UNKNOWN人工只读核对、共享原预算与生命周期；标准Light FORM/DIALOG收参、字段generation/base snapshot、私有草稿/显式冲突处置、modal所属、真实业务回读及选择性接纳。已绑定表单不再经残留legacy组件命令执行；未绑定组件保持原行为。没有可公开自报人工来源的token或第二业务执行器。

新增公共头capability_v2.h和capability_ui.h共11函数、capability_forms.h四函数；v1头/布局/只读和错误合同、SDK0.10-dev/API10/标准10、ABI1/包1、固定依赖/JCS/SHA-256/资源限额保持。C2-B本地草稿生命周期不冒充完整draft业务迁移。业务模型仅在专用内存夹具，真实stateful/EDA及其他三示例尚未迁移，不将UST1/UIL1当成新业务格式。

本轮独立最终Native17/17、Light23/23、ASAN17/17；C2-A5876/0、B headless212/0、Light模板264/0、新DLL53/0。首次崩溃/断言/编译失败、自审修正、源/二进制身份及未验项见[联合实施验证](validation/xuanshu-p2c2ab-validation.md)。保留原日志和失败匹配二进制，不用最终通过覆盖首次失败。

真实鼠标/键盘正例、OS输入来源动态矩阵、v2真实workspace标签切换仍未验；私有夹具不证明物理来源。同进程合作边界不隔离恶意DLL。原浏览器20失败、workspace20.05秒/20秒门槛和Native输入首次失败继续独立未定位。后续建议是联合代码评审与列明未验的补验，不自动进入下一批。

## 2026-10-23 C2集中收口：联合评审事实

整体仍为Proposed；此前“C2-C未授权”是历史阶段。实际HEAD仍e0eca03db25930be55bdb45477027d1793a61904，新增证据根build/xuanshu-p2c2-closeout-20261023-a93d5e，以17373项baseline区分原A+B、C1、C2-A/B与本轮；参考工程访问被拒，历史身份不当作新核对。

**源码已实现**：A/B末次应用回调后的来源/guard门、失败资格消费和本地草稿modal收尾已修；C2C-1六类输入元数据和真实宿主chrome路由复用同一registry/call/execute。新增capability_inputs.h三个函数；原15个A/B增量及v1布局/错误不改。输入值快照不是业务结果，模型/五示例仍未迁移。

**已验证**：最终Native22/22、Light30/30、Native-ASAN22/22；原C1/A+B、Assistant、公共C/C++头、组件与两个真实DLL分支分别覆盖，新增真实workspace双实例固定目标。新surface raw回调移除后WM_CHAR空调用在Native/ASAN独立复现，最小修复后通过，修复前匹配产物保留；不混同历史Native首次失败。

**未完成/未验**：原生HMENU非read没有私有来源适配，是本轮范围内缺口；真实物理输入和Windows来源完整动态矩阵未验。没有将其移出范围后宣称C2全部完成。WebView2按授权排除。C2暂不通过完整验收，细节和首次失败见[集中收口报告](validation/xuanshu-p2c2-closeout-validation.md)。历史browser20、workspace20.05秒/20秒、Native首次仍独立未定位。SDK/ABI/包/依赖/限额保持，无Git写或后续阶段。


## 2026-10-23 剩余P2已实现状态（整体仍Proposed）

以独立17381文件入口baseline核对，本轮完成HMENU-1、组件CI-1和AP-1安全poll及五示例接入。长metadata、选择continuation、UNKNOWN菜单content寿命等有源码/红绿证据，不是尚可直接复用的规划。55项描述及真实业务回读见[覆盖矩阵](xuanshu-p2-application-coverage.md)，[独立证据](validation/xuanshu-p2-remaining-validation.md)区分本轮与A+B/C1/C2历史。参考工程再次读取被拒，不能更新其历史身份。

保留SDK0.10-dev/API10/std10 ABI1/package1、旧公开布局/合同、固定依赖/限额；Assistant/AI仍只读。物理来源、真实人工操作及完整P2验收未完成；Light联合输入首次失败独立未定位，不因后续通过销账。P3—P6仍仅规划。

## 2026-10-24 P2开发阶段验收状态增补

整体架构继续Proposed，已确认产品方向/冻结合同保持。当前生产源码、独立公共头、版本/ABI/包/依赖/限额未变；本轮只补专用测试与证据核对，完成当前P2开发阶段模拟及自动验收。实际应用binding/collector及模型回读、共享源码两分支、真实workspace标签/重开/open epoch补强见[20行A/B映射和独立记录](validation/xuanshu-p2-simulated-acceptance.md)。没有增加公开来源证明或执行旁路，生产DLL不导出私有fixture/人工接纳。

真实设备/生产来源正例、Windows动态矩阵、IME/跨屏DPI、视觉与长时按用户2026-10-24指令延期到软件整体开发完成后另获授权集中验收，见[延期清单](xuanshu-p2-deferred-manual-acceptance.md)；原人工要求保留。开发阶段通过不等于物理或P2完整验收。旧输入/浏览器/超时失败仍独立未定位，不因通过销账。P3/AI Service及后续系统没有实施授权。


## 2026-10-26独立源码增量审计：P3收口与P4-A

【源码已实现】Service私有methods外层租约及关闭重检；capability_edit独立阶段合同与原目录共用资源门；ai_edit在原Service/串行worker保存schema2提案、原子意图和消费、唯一终态与核对解除。用户范围/审批只由宿主私有Button动作产生；旧v1/Assistant仍只读。合法描述超过原格式缓冲的问题已最小修复，限额未提高。

【本轮证据】真实内存DLL及两实例workspace、实际read值/revision/应用事务与撤销回执、隔离备份/迁移/子进程中断和确认丢失；三配置44/49/44通过。原P3red48/2及UNDO门red339/3不冒称崩溃；阶段和最终证据分开，参见独立验收。

【仍属后续建议】真实应用可恢复事务/回执、正式审批面板与完整P4/P5/P6；不能从该夹具推导五应用已具有AI编辑事务或从XuanShu TypeScript直接复用现成C模块。参考工程此前不可读，本轮未扩大权限。


## 2026-10-26 剩余P4开发阶段增量

【源码已实现】P3/continue最后目标回调后的edit互斥再次检查；独立风险登记扩展、Light宿主绑定及带方法租约的目标解析。五真实应用13项内存write接入应用自有快照/提交/回滚/回读/undo，3项既有文件副作用明确policy-only，原人工入口保留。原55项矩阵未删减，stateful树原本无rename，generic/features实际树名已接入。

【已验证/限制】Native/Light/ASAN相称回归、同源真实包、正式面板程序负例与隔离接纳后正例，详见本轮独立验收及清单。未读取或移植XuanShu新源码，不宣称TS实现是可链接的C现成模块。ASAN-Light共享表单有50ms脚本中断诊断，非AI人工对照亦出现，保留待核实，不能扩大为已解决或来源环境结论。物理/视觉/长时延期。


## 2026-10-27 P4限定收口与P5-1已实现状态

用户本轮明确授权后，已完成同workspace/同Service/单主对话的Coordinator图、应用可选纯数据profile、私有子上下文manifest、共享根预算、依赖/并发/取消/收集，以及原P4受控事务执行和schema3最小运行记录。独立[P5-1合同](xuanshu-p5-coordinator-contract.md)和[五应用覆盖/原其他应用缺口](xuanshu-p5-application-coverage.md)优先说明当前增量；原总体未实现部分继续Proposed，不由新接口倒推既有能力。

最终verified Native21/21、Light31/31、ASAN21/21，Native静态/Assistant/公共头/两DLL分支另9/9。具体源、二进制、首次失败、原窗口与本轮整改见[P5独立验收](validation/xuanshu-p5-validation.md)；不将这些数字相加冒充独立case。实际五包模型/对象/revision/事务/undo有回读，不是执行计数。旧C2人工资格不用于AI授权，旧Assistant修改不重新开放。

普通有限图修改仍宿主确认30分钟、风险逐项5分钟；写前意图/消费确认先于业务。取消不等于回滚/解门，UNKNOWN继续阻止资源；旧epoch只查记录、不恢复授权/工具重放。P4/P5正式控制在完整投影成功后才绑定，未投影时拒绝。运行记录按字段隐私/预算保留，不广播完整子上下文。

参考工程此前拒读，本轮未扩权或修改，其当前HEAD未重新核验。内置browser仍Light legacy而非必需WebView2，没有统一资源/版本合同，未接入新Service；原“所有其他应用迁移”未完成，具体下一案独立评审，不静默缩减后宣称全部完成。真实设备/Windows动态来源/IME/DPI/视觉/辅助技术/长时继续延期，旧失败独立未定位；P6/真实Provider没有实施。


## 2026-10-27最终所有权回归checkpoint

最终复制队列收口后，ownership Native22/22、Light32/32、ASAN22/22通过，包含14项Coordinator核心case；Native ownership-sdk9/9另核对公共C/C++头、Assistant、静态嵌入/只读工具及两真实DLL分支。修前verified21/31/21保持独立历史。新受控queue-ownership回归保留未消费完成槽，先值10、排空后42；没有声称动态复现崩溃。入槽/关联在同一锁内复查、Coordinator发布/查询同步，仅私有实现变动，无公共合同/版本/限额变化。

最终增量、源码/正式二进制和运行前身份比较、保全与两轮清理由build/xuanshu-p5-20261026-e63b8a清单记录。修前before-queue-ownership匹配源及程序、全部首次失败和必要库/备份保留；其他历史失败独立未定位。P5开发阶段模拟及自动验收通过限本轮P5-1合同/授权范围，原browser及其他应用全迁移不冒称完成。物理/真实模型及P6未执行，停止联合代码评审。

## 2026-10-27 P6限定复审与增量

已实现：同一Service启动在quick_check后、恢复写入前核对逻辑引用/终态/消费intent绑定/图版本和预算；失败UNAVAILABLE且不重建。worker每32次成功COMMIT做PASSIVE checkpoint；检查点非BUSY/LOCKED错误显式停止后续写入，已提交事实保留。新ai_graph_jobs覆盖子编辑存储job的空闲线程唤醒缺口；受控真实线程退出后业务值42，不UI join。无新schema、公共接口、版本、固定依赖或额度。

源码依据新增src/ai_reliability.inc；隔离src/ai_reliability_fixture.inc的VFS和中断点不生产编译/导出；tests/ai_reliability.c、ai_reliability_panel.c及browser_capability.c记录具体值、SQL和资源。P5最终37项交付/331源/114二进制与修前362项入口已核对，不把HEAD全差异归P6。

browser现有Light路径新增原生read快照/资源/revision及10个view绑定，9个导航/文件副作用保留旧人工并拒绝AI/Assistant；用户确认完整副作用迁移另案，原P5全迁移仍未完成。不是WebView2排除，也不新增业务/网络执行器。具体最终结果、失败及证据限制见validation/xuanshu-p6-validation.md；物理来源、断电及人工长时仍延期。
