# XuanShu P2 C2-A：接口与验收规格

当前状态增补（2026-10-23）：下文“未实现/仅设计”描述保留原冻结阶段，不是最新交付状态。C2已实现并在本轮补HMENU及标准组件入口、五示例接入；原规范正文不变。见[独立验证](validation/xuanshu-p2-remaining-validation.md)、[增量设计](xuanshu-p2-remaining-implementation.md)。物理验收尚未开始，不宣称完整通过。


规格身份：**C2A-1，2026-10-23设计冻结；未实现，验收未运行**。读者为框架及应用开发者。本规格是已确认C2-A子范围的唯一规范来源；三份整体设计继续为Proposed。此次授权仅允许八份文档落盘，不允许创建公共头、修改产品/测试/构建/版本或执行构建测试。公共接口和下列C声明均为待实现规格，不是现有导出。

## 1. 基线、范围与证据

框架实际分支为`codex/p1-process-boundaries`，HEAD为`e0eca03db25930be55bdb45477027d1793a61904`。接手工作区有33项tracked修改、5334个未跟踪文件，暂存为空；其中包含用户修改、A+B、原C1及其整改，不将HEAD差异整体归为本轮。当前版本保持SDK0.10-dev/API10/标准10、ABI1/包1，使用匹配开发快照，不覆盖冻结SDK。

C1已在本会话通过限定验收。本轮仅以其单一目录、read闭环、legacy只读政策、预算和租约作为前置条件，不重新进行全面复审。已有证据见[C1第7、8节](validation/xuanshu-p2c1-validation.md)及[A+B第7节](validation/xuanshu-p2ab-validation.md)。参考工程上次已核对身份为`main / 67eacf5c47a856f5d3d68262f247659a15a13f60`；本次C2规划读取`D:\XuanShu`被权限拒绝，不能声称重新确认其当前HEAD或干净状态。

C2-A只接通Windows私有输入链、内置Light菜单叶项和统一能力执行，以专用内存夹具验证view、draft、单资源write。read继续使用C1；external仅保留设计，不支持执行。单资源由单一业务所有者负责，首批不开放跨实例共享文档写入。

排除通用表单/对话框绑定、业务草稿冲突实现、stateful/EDA业务模型、五示例迁移、WebView2/专业画布适配、文件/网络/外发适配，以及AI Service、Provider、Coordinator、审批、事务/撤销、存储和恢复。P2不引入SQLite、凭据依赖、新文档格式、Agent预算或并发实现。

### 1.1 已有源码事实与待实现项

| 类别 | 事实或约束 | 依据 |
| --- | --- | --- |
| 源码已实现 | v1仅read，目标副本、prepare/dispatch、稳定ID完成、取消、结果与关闭门 | [capability.h](../include/ui_framework/capability.h)、[capability.c](../src/capability.c) |
| 源码已实现 | 生成read投影、请求桥接、legacy策略及独立回调租约 | [capability_entry.h](../include/ui_framework/capability_entry.h)、[ui.c](../src/ui.c)、[assistant.c](../src/assistant.c) |
| 源码缺口 | 菜单选择目前经消息及source字符串invoke，没有人工凭据 | [menus.c](../src/menus.c) |
| 源码缺口 | Light原生与公开模拟输入汇入同一路径；不能由函数入口推断人工来源 | [light_web.c](../src/backends/light_web.c) |
| 源码事实 | 每实例独立host，现有read登记不提供跨host写门 | [workspace.c](../src/workspace.c) |
| 已确认、待实现 | 本文v2非read合同、内部凭据、UI绑定、写门和未知核对 | 仅冻结设计，不计入C1实现 |

## 2. 单一目录及公共边界

v1、v2共用能力ID、版本、Schema、资源、目录代次、调用ID序列、预算和注销生命周期。普通命令及Assistant为投影，不保存第二业务执行器。双向重复ID返回ALREADY_EXISTS；退休期间仍占ID，无覆盖或legacy回退。

C2接入先调用现有enable_read_entries并确认OK，再登记v2能力；未启用只读模式时register_v2返回PERMISSION_DENIED，不隐式激活或重试。初次enable因旧请求/回调在途返回PENDING时仍未激活，须依原C1合同在退栈后显式重试。这样新接入host不能保留legacy Assistant修改旁路；未迁移host的原legacy行为不因本批全局改变。

最小化首批增量：read继续以v1登记和执行；`register_v2`仅接受`view`、`draft`、`write`，传read或external返回UNSUPPORTED。后续若要提供v2 read执行器，须另审互操作，不能预先导出未实现入口。v2发现可查询共同目录中的v1/read和v2元数据，但发现不授予执行权限；v1发现、描述及Assistant视图仍仅提供read。Assistant/AI调用旧修改入口、事务辅助API以及v2非read均不得取得执行权限。

### 2.1 结构及版本规则

能力相关新结构`version=2`，独立UI相关结构`version=1`。`size`至少覆盖对应版本完整结构；未知version拒绝，不读取未知尾部。保持平台默认布局，不修改任何既有结构、枚举数值、packing或旧错误合同。实际新结构size/offset尚未经编译验证，未来公共C/C++调用方必须以本声明的字段顺序验证Windows x64布局，不能在本轮虚构已通过的布局结果。

所有host API和回调在创建UI线程执行。元数据、Schema、资源身份和绑定复制；注册回调/user_data在相应finish成功前由应用保持有效。回调入参仅借用至返回。JSON分配由框架释放函数释放，禁止跨DLL用应用free。

### 2.2 能力结构声明

以下声明拟进入`include/ui_framework/capability_v2.h`；本轮不创建该文件。独立枚举不改变v1枚举。

```c
typedef enum ui_cap_outcome_v2 {
    UI_CAP2_OK = 0, UI_CAP2_REJECTED = 1, UI_CAP2_FAILED = 2,
    UI_CAP2_CANCELLED = 3, UI_CAP2_STALE = 4, UI_CAP2_UNKNOWN = 5
} ui_cap_outcome_v2_t;

typedef enum ui_cap_effect_v2 {
    UI_CAP2_EFFECT_NONE = 0, UI_CAP2_EFFECT_APPLIED = 1,
    UI_CAP2_EFFECT_UNKNOWN = 2
} ui_cap_effect_v2_t;

typedef enum ui_cap_resolution_v2 {
    UI_CAP2_UNRESOLVED = 0, UI_CAP2_RESOLVED_APPLIED = 1,
    UI_CAP2_RESOLVED_NOT_APPLIED = 2
} ui_cap_resolution_v2_t;

typedef struct ui_cap_resource_ref_v2 {
    uint32_t size, version;
    uint64_t instance_id;
    ui_cap_resource_kind_t kind;
    char host_epoch[37], resource_id[256], open_epoch[37];
} ui_cap_resource_ref_v2_t;

typedef struct ui_cap_target_v2 {
    uint32_t size, version;
    ui_cap_resource_ref_v2_t resource;
    char expected_revision[128];
    char scope_json[4097];
} ui_cap_target_v2_t;

typedef struct ui_cap_completion_v2 {
    uint32_t size, version;
    ui_cap_outcome_v2_t outcome;
    ui_cap_effect_v2_t effect;
    const char *output_json;
} ui_cap_completion_v2_t;

typedef struct ui_cap_result_v2 {
    uint32_t size, version;
    ui_cap_call_state_t state;
    ui_cap_outcome_v2_t outcome;
    ui_cap_effect_v2_t effect;
    uint32_t cancel_requested;
    char code[64];
    char revision_before[128], revision_after[128];
    ui_cap_resolution_v2_t resolution;
    char reconciled_revision[128], evidence_sha256[65];
    const char *output_json;
} ui_cap_result_v2_t;

typedef struct ui_cap_reconciliation_v2 {
    uint32_t size, version;
    ui_cap_resolution_v2_t resolution;
    char observed_revision[128];
} ui_cap_reconciliation_v2_t;

typedef struct ui_capability_desc_v2 {
    uint32_t size, version, capability_version;
    const char *id, *name_utf8, *description_utf8, *category;
    const char *input_schema_json, *output_schema_json;
    const char *reconcile_schema_json;
    ui_cap_resource_kind_t target_kind;
    const char *effect;
    ui_status_t (*check)(const ui_cap_target_v2_t *,
        const char *params_json, void *user_data);
    void (*execute)(uint64_t call_id, const ui_cap_target_v2_t *,
        const char *params_json, void *user_data);
    void (*cancel)(uint64_t call_id, void *user_data);
    ui_status_t (*reconcile)(uint64_t call_id,
        const ui_cap_target_v2_t *original_target,
        const char *original_params_json,
        ui_cap_reconciliation_v2_t *result,
        char *evidence_json, size_t capacity, size_t *length,
        void *user_data);
    void *user_data;
} ui_capability_desc_v2_t;
```

资源登记仍使用现有`ui_cap_resource_desc_t`及get_revision；`resource_ref_v2`仅复制已登记身份，不读取活动标签、不签发权限。ref不包含revision，执行target才包含固定expected revision。嵌套ref也须有正确size/version。C2-A scope由框架按效果生成，范围只涵盖该明确资源；对象集合及多资源写入不在本批。

check可选，execute必需，cancel可选；本批所有v2效果要求reconcile及其Schema，以只读回读证明结果。reconcile在UI线程短同步执行，不能产生业务修改或等待worker。capacity包含末尾NUL，length为不含NUL的UTF-8字节数；缓冲由框架提供，应用不可保留。有效APPLIED/NOT_APPLIED核对必须有符合核对Schema的业务证据和有效revision；UNRESOLVED不解除阻止。

### 2.3 UI结构声明

以下声明拟进入`include/ui_framework/capability_ui.h`，包含v2头；不提供公开凭据类型。

```c
typedef enum ui_cap_ui_purpose {
    UI_CAP_UI_INVOKE = 1, UI_CAP_UI_RECONCILE = 2
} ui_cap_ui_purpose_t;

typedef enum ui_cap_ui_stage {
    UI_CAP_UI_ADMITTED = 1, UI_CAP_UI_REJECTED = 2,
    UI_CAP_UI_FINISHED = 3, UI_CAP_UI_RECONCILED = 4
} ui_cap_ui_stage_t;

typedef struct ui_cap_ui_context {
    uint32_t size, version;
    uint64_t binding_id;
    ui_cap_target_v2_t target;
} ui_cap_ui_context_t;

typedef struct ui_cap_ui_notice {
    uint32_t size, version;
    uint64_t binding_id, call_id;
    ui_cap_ui_stage_t stage;
    ui_status_t status;
    char code[64];
    const char *evidence_json;
} ui_cap_ui_notice_t;

typedef struct ui_cap_ui_binding_desc {
    uint32_t size, version;
    const char *capability_id;
    ui_cap_resource_ref_v2_t resource;
    const char *menu_item_id;
    ui_cap_ui_purpose_t purpose;
    uint64_t reconcile_call_id;
    ui_status_t (*collect)(const ui_cap_ui_context_t *,
        char *params_json, size_t capacity, size_t *length,
        void *user_data);
    void (*observe)(const ui_cap_ui_notice_t *, void *user_data);
    void *user_data;
} ui_cap_ui_binding_desc_t;
```

绑定只引用能力和资源，没有业务执行函数或权限副本。menu_item_id必须对应该host的现有框架菜单叶项；同一叶项重复绑定拒绝，不能注册任意网页消息为可信输入。其他控件类型在本批返回UNSUPPORTED。绑定ID由框架生成，host生命周期内不复用；溢出明确LIMIT_EXCEEDED。

INVOKE时reconcile_call_id必须为0；collect为NULL表示参数`{}`，否则只收参、不执行业务。RECONCILE时call ID必须指向同能力、同资源的未知记录，collect必须为NULL；框架自行形成绑定原call ID的规范核对参数。observe可选，不决定权限。通知证据只在RECONCILED回调内借用；业务结果通过get_result_v2查询。

### 2.4 十一个新增函数

```c
UI_API ui_status_t ui_capability_register_v2(
    ui_host_t *, const ui_capability_desc_v2_t *);
UI_API ui_status_t ui_capability_discover_v2(
    ui_host_t *, const char *category, size_t offset, size_t limit,
    uint64_t *generation, char **page_json);
UI_API ui_status_t ui_capability_describe_v2(
    ui_host_t *, const char *capability_id, char **json);
UI_API ui_status_t ui_capability_resource_ref_v2(
    ui_host_t *, const char *resource_id, ui_cap_resource_ref_v2_t *);
UI_API ui_status_t ui_capability_complete_v2(
    ui_host_t *, uint64_t call_id, const ui_cap_completion_v2_t *);
UI_API ui_status_t ui_capability_get_result_v2(
    ui_host_t *, uint64_t call_id, ui_cap_result_v2_t *);
UI_API ui_status_t ui_capability_cancel_v2(ui_host_t *, uint64_t call_id);
UI_API ui_status_t ui_capability_forget_v2(ui_host_t *, uint64_t call_id);
UI_API ui_status_t ui_capability_ui_bind(
    ui_host_t *, const ui_cap_ui_binding_desc_t *, uint64_t *binding_id);
UI_API ui_status_t ui_capability_ui_begin_unbind(ui_host_t *, uint64_t binding_id);
UI_API ui_status_t ui_capability_ui_finish_unbind(ui_host_t *, uint64_t binding_id);
```

无公共非read prepare/dispatch、mint/token、强制核对解除接口。现有能力/资源begin和finish注销同时管理v1/v2；内部取消按记录版本路由，不能依赖跨版本公共cancel。v2发现采用category精确过滤、offset、limit=1..32、generation=0开新扫描及代次不匹配CANCELLED；JSON沿用items/next/generation形状，单项标明contract_version和effect，完整描述含三个适用Schema及JCS/SHA-256指纹。v1项没有核对Schema。元数据不包含回调指针或人工凭据。

## 3. 人工来源与接纳合同

### 3.1 来源识别

只在框架私有Windows输入入口读取来源，记录具体Light视图/菜单叶项及节点代次、绑定ID、输入动作和派发范围。实际命中及按下/释放或键盘激活动作必须匹配；公开ui_surface/ui_web输入API、ui_host_invoke、脚本调用和页面消息均不能创建该记录。进入同一WndProc或lw_dispatch_input本身不构成证明。

按下与释放均须属于同一合格来源链，不接受“注入按下+合格释放”等混合动作。来源不合格、焦点/捕获丢失、菜单/节点/绑定失效时撤销候选动作，不能只看最后一次消息的来源。测试须包含混合来源和失效后再释放场景。

拒绝可识别的普通注入、系统来源、未知来源及来源读取失败。Windows的IMO_HARDWARE还包括UIAccess应用注入，因此不将此分类称为物理设备证明，也不承诺排除所有注入或隔离恶意DLL；这是已确认的同进程合作边界。依据：[来源分类](https://learn.microsoft.com/en-us/windows/win32/api/winuser/ne-winuser-input_message_origin_id)、[来源读取](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getcurrentinputmessagesource)。真实鼠标、键盘另行验收，更强来源证明另案评审。

框架精确匹配绑定动作后直接进入能力路由，不能将任意JS postMessage或source自报值升级为凭据；绑定项不再同时触发旧业务执行器。未适配入口拒绝v2非read，不能回退legacy。未迁移普通人工入口保留原行为，但不称为已迁移，不导入未来AI修改路径。

### 3.2 内部一次性凭据

凭据不出框架，绑定能力ID/version/descriptor_hash、完整资源身份、expected revision、规范参数SHA-256、菜单所属对象及代次和本次语义事件。调用方持有context或binding ID不代表持有凭据。

凭据仅限当前语义提交，不跨事件栈保存、异步转交或泵消息延续；发生嵌套输入/绑定失效时撤销旧事件接纳资格。一次消费只接纳一个call，失败后也不复用。打开对话框不延续权限，后续提交须新事件；对话框不在本批适配。异步执行只能完成已经接纳的原call。

### 3.3 固定顺序

1. 确认私有来源、菜单所属对象和绑定代次，固定resource ref及提交revision。
2. collect返回完整参数；严格解析、重复键、Schema/JCS及业务check，不将菜单内部消息自动当业务参数。
3. 在分配前计账，建立稳定call及UI绑定关联，一次消费凭据并取得资源执行门。
4. 通知ADMITTED；此时call ID可查询，记录尚为PREPARED。
5. 最后一次应用回调返回后复查目标/open epoch、revision、目录/绑定代次、disabled/busy/modal和关闭门；之后无应用回调再进入RUNNING/execute。
6. complete_v2核对完成数据和实际revision，形成结果；通知FINISHED后仍保留执行栈和观察回调租约到退栈。

目标/版本变化不重绑、不重试。ADMITTED不代表业务成功，回调可取消PREPARED调用；后续执行门拒绝应产生effect NONE的终态。接纳前失败通知call ID为0；不得伪造关联。资源门在通知及execute前已取得，重入不能插入同资源第二次写入。UI观察租约与业务执行嵌套标记分开，不能导致原正常C1 read观察回调中的同步read全部被拒绝。

## 4. 状态、结果和错误

### 4.1 效果与版本

read保持v1效果none及revision不变检查。view修改视图资源，draft修改私有未提交资源，两者不冒充业务文档修改；write修改单一业务资源。即使共享资源kind，也须用不同权威resource_id区分视图、草稿和业务对象。首批夹具使用内存值及不透明revision，不新建文档文件格式。

成功NONE要求无实际效果且revision未变；成功APPLIED须有业务确认及新revision。框架从已登记get_revision读取最终版本，不接受应用在completion中自填版本。框架可检查Schema和revision，不能仅凭hash普遍证明业务语义；具体值及操作结果必须通过应用回读验收。

失败或取消声明NONE同样要求revision与before相同；APPLIED要求实际版本变化。outcome为OK时output必须符合output_schema；其他outcome的output须为NULL，不将错误负载当正常业务输出。无法确认效果与版本一致性时使用UNKNOWN，而不是猜测NONE。

若执行后输出非法、版本无法确认、版本/效果矛盾或应用声明无法证明结果，终态为UNKNOWN/UNKNOWN，正常output为NULL并阻止该资源后续修改。不能从失败、超时或取消推断NONE。取消后已生效须如实APPLIED；本批不提供rolled_back或撤销承诺。

有效业务失败可为FAILED/NONE或FAILED/APPLIED，但effect须有应用确认及一致revision；非OK结果不投影正常output。明确cancelled也可APPLIED。UNKNOWN无论原始失败原因均保留独立code。原终态不会被后续核对重写，resolution和核对版本另列。

### 4.2 错误映射及互操作

| 条件 | 固定行为 |
| --- | --- |
| 双向重复ID、退休占ID、同一菜单叶项重复绑定 | ALREADY_EXISTS，无覆盖/回退 |
| 不支持effect或UI端点 | UNSUPPORTED |
| 结构、size/version、必需回调、枚举或字符串非法 | INVALID_ARGUMENT |
| 参数Schema失败 | VALIDATION_FAILED / invalid_params，execute为0 |
| 不可信来源、凭据失效或重放 | PERMISSION_DENIED / human_source_required或credential_invalid，call ID为0 |
| disabled/busy/modal不允许 | PERMISSION_DENIED / unavailable；资源门占用为busy，未知阻止为result_unknown |
| 关闭门 | CANCELLED / closing |
| 资源身份/open epoch不匹配 | NOT_FOUND / target_expired，不改绑 |
| 接纳后revision冲突 | 终态STALE / revision_conflict / NONE |
| 目录/绑定代次失效 | 拒绝，code=catalog_changed或binding_expired，不自动重新发现执行 |
| 字节/调用预算耗尽 | LIMIT_EXCEEDED / budget_exceeded，无伪造call或完成 |
| 重复terminal complete | ALREADY_EXISTS，不再次通知或执行 |
| 未知或已释放ID | NOT_FOUND |
| complete结果留存预算不足 | LIMIT_EXCEEDED，仍RUNNING且保留门；可重交完成数据，不得重执行业务 |
| 已知结果外层回调/execute尚未退栈 | forget/finish为PENDING |
| 未核对UNKNOWN | forget_v2为PENDING，不清除阻止或退还记录额度 |
| 核对worker/回调未退出或核对工作区被借用 | PENDING，无业务修改或解除 |

v1 prepare/dispatch或旧invoke不得执行v2非read，返回PERMISSION_DENIED；Assistant执行视图只读。v1 publish非read返回UNSUPPORTED。v1 complete/query/cancel/forget处理v2 ID，以及v2对应函数处理v1 ID，均UNSUPPORTED且不改变记录；未找到任何版本ID仍NOT_FOUND。框架内部按记录版本路由quiesce、注销和取消，不依赖误用这些公共入口。v1原调用的错误和重复dispatch语义不变，v2无公开dispatch。

complete只要接纳并记录了有效完成容器，返回OK不等于业务成功；输出非法等记录为UNKNOWN及invalid_output，记录仍可查。结构错误或预算未满足时不产生伪终态。FAILED/STALE等原因只由框架按检查路径生成code，不由应用任意字符串覆盖。

## 5. 未知核对、预算与生命周期

### 5.1 资源门和核对

同一资源的人工修改共用执行门；busy明确拒绝，无排队、自动重试或凭据复用。后台仅准备耗时工作，业务提交回UI线程，UI不等待worker。取消请求不释放门。确定结果且相关回调退出后才释放；未知结果在执行租约排空后仍保留独立的资源写入阻止状态，键包含实例和open epoch。

UNKNOWN记录为FINISHED但resolution为UNRESOLVED；forget_v2保持PENDING并占原额度。新的人工RECONCILE动作绑定原call，重新取得当前revision快照作为本次核对的预期；reconcile回调接收的original_target仍是原调用快照，不把原expected revision改成当前版本。

只有原worker/外部回调真实退出且框架回调退栈才调用reconcile。应用以原call、参数、业务状态及revision给出APPLIED、NOT_APPLIED或UNRESOLVED；前两项必须有符合reconcile_schema的证据。框架前后读取revision，与observed_revision及新凭据绑定核对，检查身份/关闭/租约，验证并JCS规范化证据后记录SHA-256。缺证据、版本漂移、仍可能迟到写入或UNRESOLVED均不解除。

核对复用原call，不需新调用名额。有效核对仅增加resolution、reconciled_revision、evidence_sha256，原UNKNOWN结果保留；业务证据正文在RECONCILED通知中借用，不保留无界历史。通知返回且相关租约退出后，解除资源阻止，允许forget。失败核对不改变原关联和结果；再次核对须新人工事件，不能在失败observer内自动重试。

### 5.2 预算与背压

继续采用每实例256能力、目录2MiB、输入64KiB、输出256KiB、深度16、节点4096、数组512、对象128字段，以及host共享1024调用和8MiB调用预算。Schema、固定yyjson0.13.0、RFC8785/JCS及SHA-256不变。

v2目录、核对Schema、绑定、目标和标题复制计入2MiB；call、关联、参数、pending/source、新UI回调临时复制及UNKNOWN记录计入8MiB。多个Assistant或UI入口不能各获一份host额度。持有和释放均需可核对，不因prepare/collect失败或observer重入失账。

首个v2接纳前在原8MiB中预留一个有界收参/核对工作区及必要固定终态空间；预留失败则拒绝接纳。核对工作区独占到证据观察回调返回，不因调用数或其他字节已满而需要新call/桥接分配；复用原call的固定结果字段保留结论及hash。其缓冲上限沿用输入/输出限额，JSON临时解析沿既有受限工具规则，不增加现有数值限额。预算验证须覆盖保留工作区后填满剩余预算仍能核对，禁止以已用满额度为由永久锁死全部未知记录。

C1固定失败槽保持原合同。新UI通知也须有界，拒绝嵌套失败通知；observer重入提交在接纳前背压，不递归分配、不伪造成功。队列满时保留原调用，明确失败/背压；worker交付完成只能重投复制结果，不重做业务。排空后新人工操作才可重新提交。

### 5.3 所有权和关闭

complete_v2借用完成容器至返回，复制有效output；get_result_v2的output借用至forget或host销毁。collect/context、notice和核对证据仅借用至回调返回。observe及collect用户数据保留到finish_unbind成功；begin_unbind禁止新动作，finish在相关收参/通知租约未退时PENDING。已接纳调用独立继续，解绑不等于取消或释放call；解绑退栈后不再调用该binding observer，结果由稳定ID查询。

应用worker只经既有复制队列传workspace/instance/call身份和数据，不持host/target/借用回调指针。complete仍代表应用确认相关worker/外部回调真实退出，投递不是退出证明。同步complete后外层execute或observer未退栈，不能forget、finish注销或提前卸载。框架只跟踪自己的调用/回调租约，不自动跟踪任意线程。

活跃host中，未核对UNKNOWN保留核对所需注册引用；begin注销停止新业务调用，专用人工核对仍可服务原未知记录，finish等待核对或整个实例关闭。实例关闭先封门、撤销核对入口并排空执行/回调；没有在途worker/回调后可拆除应用引用，保留复制的未知结果至host销毁。逻辑未知状态不冒充活跃worker租约，不导致安全关闭永久等候；关闭不记录为已解决，不承诺跨进程恢复未知状态。

资源重开使用新open epoch、实例重开使用新instance身份，旧消息不能命中。活动实例不能以注销/同epoch重登绕过未知门。共享文档、多host串行化及持久记录均须后续合同，不能套用首批内存资源关闭规则声称已支持。

## 6. 首批夹具、UI和实施文件边界

独立夹具维护可回读的业务值、视图值和私有草稿，各有revision。Light菜单叶项调用内存业务能力；view/draft通过同一私有接纳机制的专用用例验证。验收必须核对实际值及回读，不以handler计数、控件变化或草稿变化作为业务成功。人工模拟夹具仅验证接纳状态机，测试钩子不导出到生产DLL，也不构成物理来源正例。

产品实现预计涉及两个新公共头、共同能力目录/调用实现、UI内部绑定、menus.c和light_web.c；ui.c/workspace.c只在关联与租约接入必要处改动。未来授权还需专用夹具、公共C/C++头测试及CMake注册。本轮这些文件全部不写。新旧业务注册不能并存；菜单绑定没有另一个handler。版本、依赖、五示例、browser及旧测试业务语义不借机调整。

后续批次依赖：C2-B处理表单/对话框、字段代次、提交快照和草稿冲突；C2-C逐项处理快捷键、其他控件、专业画布及WebView2来源合同。stateful属性表单和对话框共享业务name而保留各自草稿，业务更新保留脏草稿并显式解决冲突；EDA笔记补提交/取消。它们不属于C2-A漏实现。

完整P2仍需五个维护示例及generic/framework_features共享源码两个编译分支分别覆盖。现有[功能矩阵](application-capability-contract.md#6-首个试点完整功能矩阵)不删行；计数夹具保持原义，generic已存在的修改记录应复用而非一律重写。UST1/UIL1不扩写为业务文档格式。P3依赖完整P2后进入常驻服务/stub只读；P4再处理受控AI写入/审批/事务/撤销；P5/P6仅保留协调及持久化/稳定性依赖。

## 7. 验收冻结：C2A-AC-1

下表全部为**未运行**，不是C1测试结果。未来必须逐项记录测试文件、原始日志/JUnit、源码和匹配二进制身份，不能只汇报检查数量。

| 编号 | 触发与必须断言的结果 |
| --- | --- |
| A01 注册权威 | v1/v2/legacy双向重复、退休占ID、共同代次、晚创建/动态发现一致；无第二执行器/静默覆盖 |
| A02 人工来源 | 直接invoke、source/参数伪装、页面消息、公开模拟输入、普通注入及系统/未知/读取失败均不执行非read；绑定动作及真实所属对象须匹配 |
| A03 一次消费 | 同事件重复消息、凭据重放、嵌套事件、解绑/节点代次失效不产生第二execute；异步延续不能签发新调用 |
| A04 固定目标 | 实际标签切换不改绑；open epoch、实例或revision改变明确失败；执行次数0且目标值不变 |
| A05 最后门 | collect/check/get_revision/ADMITTED observer重入改变modal、关闭、disabled/busy后execute为0；正常C1 read仍保持允许的同步观察回调路径 |
| A06 业务效果 | 业务值改后重新查询一致且revision变化；view/draft不改业务值/revision；无操作NONE不虚增revision；实际值而非计数验收 |
| A07 同步/重复 | 关联先于execute；同步完成早于派发返回可查询；重复完成/消息不重复执行或通知；跨版本接口不改变记录 |
| A08 异步/取消 | DLL自有worker准备、UI提交、稳定ID完成；UI不等worker；取消不提前放门；APPLIED不能伪装取消后NONE |
| A09 同资源busy | 不同人工绑定共用资源门；busy拒绝无排队/重试/复用凭据；排空后新事件可执行 |
| A10 未知 | 非法输出/版本不可确认进入UNKNOWN；正常output不投影；后续写、取消、forget、重提交、同epoch重登不能绕过 |
| A11 核对 | worker未退、回调未退、证据非法/缺失、revision漂移、UNRESOLVED均不解除；新人工有效证据才解除，原UNKNOWN不改写 |
| A12 预算 | 256/2MiB、1024/8MiB保持；失败/销毁/释放计账恢复，多Assistant/UI共享；满额仍可复用原call核对；工作区连续借用和重入不复用活数据 |
| A13 生命周期 | 真实DLL worker/外部回调在途、同步complete尚未退栈、observer重入、解绑/注销/关闭不提前释放；迟到消息不命中新实例；UNKNOWN不永久阻塞已排空实例关闭 |
| A14 回归 | 原C1、A+B、Assistant、公共C/C++头及真实DLL两个编译分支相称覆盖；旧v1布局/错误/超时/限额保持，普通legacy人工功能可用 |
| A15 物理输入 | 真实鼠标和键盘正例分别记录；自动化/SendInput不替代；无法执行时列未验，不冒称完整物理验收 |

草稿base revision及冲突前置只冻结合同，不在本批实现字段冲突UI；其真实验收归C2-B。五示例功能仍标待适配，不计为A06已通过。

### 7.1 未来验证及停止条件

获得单独实现授权后才可使用新独立构建目录进行Native、Light、ASAN及相称回归；共享测试源的两个DLL编译分支分别验证。保留首次失败、原始日志/JUnit、源码/二进制哈希及匹配失败二进制。不得修改断言、超时、限额或数据范围来获得通过，不以C1已有17/17、17/17、11/11或20923检查代替新合同证据。

出现v1公共语义变化、来源旁路、未知可绕过、提前卸载、UI阻塞等待worker、预算失账或业务回读不一致即停止验收；需要超出本规范则报告原因、影响和替代方案，先评审再改合同。代码通过与物理未验分开，不将未验写成通过。

## 8. 历史分账与本轮授权

原浏览器20失败、workspace首次20.05秒超时（门槛20秒）、Native输入首次失败继续独立未定位。本轮不重跑、不默认归因环境，不用后续限定通过覆盖首次。原C1 red 48失败缺失匹配二进制的限制保留；后来的5331/6 red及5349/0 final不倒补旧运行。

本轮只落盘本文及三份整体设计、当前交接、三份根规划共8份文档；没有产品实现、构建或测试结果，也没有新build证据目录。参考工程、外部应用、固定依赖、冻结SDK、历史材料只读。无子代理、computer use、真实Provider、benchmark、recovery、远程工作流、commit、push、release或tag。

当前停止在C2-A接口与验收设计冻结位置。文档确认不等于C2-A实现授权；下一步须明确授权首批实现，不能自动进入C2-B或完整P2。

## 9. C2A-1实现状态增补（2026-10-23）

本会话后获连续实现授权，前面的文档冻结时状态保留历史。冻结正文/结构/错误及资源边界不改；实现现已交付并完成联合本地验证。C2A-AC-1逐项证据及未验限制以[独立联合验收记录](validation/xuanshu-p2c2ab-validation.md#4-验收矩阵及证据限制)为准，不能将原“未运行”表回写成历史已运行。

Native17/17、Light23/23、ASAN17/17；真实物理输入、OS来源动态矩阵和v2真实workspace标签切换正例未验。代码完成不等于全部产品验收通过，当前停在联合代码评审。来源、预算、busy/UNKNOWN/生命周期及v1布局保持冻结合同，没有自动进入C2-C、样例模型/五示例迁移或P3—P6。

## 10. 2026-10-23集中收口状态增补

前述冻结正文及原实现证据完整保留。本轮联合审查修正末次回调来源/guard门、失败资格消费及本地草稿回读后modal门，未改变冻结结构、v1语义/错误或限额。[独立集中报告](validation/xuanshu-p2c2-closeout-validation.md)记录新增回归、首次失败和匹配产物。

最终Native22/22、Light30/30、Native-ASAN22/22；A5896/0、B214/0与Light266/0。真实workspace两实例固定目标已补自动验证，不再保留为“只有固定指针”缺证，但没有冒充物理标签点击。C2C-1六类绑定/宿主chrome已实施；原生HMENU非read仍缺适配，真实鼠标/键盘和完整Windows来源动态矩阵未验，C2整体暂不通过完整验收。历史三项首次失败独立保留；停止联合代码评审。
