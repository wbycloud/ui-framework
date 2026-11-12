# XuanShu P2 C2-B：标准表单和对话框合同

当前状态增补（2026-10-23）：下文“未实现/仅设计”描述保留原冻结阶段，不是最新交付状态。C2已实现并在本轮补HMENU及标准组件入口、五示例接入；原规范正文不变。见[独立验证](validation/xuanshu-p2-remaining-validation.md)、[增量设计](xuanshu-p2-remaining-implementation.md)。物理验收尚未开始，不宣称完整通过。


状态：**C2B-1设计冻结，2026-10-23；实现与验收随后记录**。本会话已授权先更新设计再实施本增量，无需再次授权常规结构和测试细节。本文件不更改[C2A-1](xuanshu-p2c2a-interface-and-acceptance.md)的公开结构、11个接口或错误合同；同一能力目录、执行器、预算和来源边界继续有效。

## 1. 范围与责任

C2-A核心已有本轮5679检查/0失败及Light编译结果，详见本轮独立日志；这不是完整A验收或物理输入证据。C2-B只适配框架拥有的Light标准FORM/DIALOG：完整收参、提交快照、字段代次、base revision、草稿保留/冲突、modal所属入口、业务回读及选择性接纳。专用内存夹具承载业务，不修改stateful/EDA模型或五示例。

业务提交只引用应用已注册的同一v2能力及resource ref，不新增业务handler或权限副本。草稿编辑、取消和冲突处置是框架控件本地状态管理，绝不修改应用业务；不据此宣称通用draft能力目录或五应用功能迁移完成。程序字段/模型投影不是业务提交，也不授予人工来源。

## 2. 独立公共增量

拟新增`include/ui_framework/capability_forms.h`，四个函数、独立完整version1结构。现有v1、C2A-1及SDK0.10-dev/API10/标准10、ABI1/包1保持。新增绑定ID来自同一UI绑定序列，解绑使用C2-A begin_unbind/finish_unbind，不另建注册中心。

```c
typedef struct ui_cap_form_projection {
    uint32_t size, version;
    const char *revision;
    const char *fields_json;
    size_t length;
} ui_cap_form_projection_t;

typedef struct ui_cap_form_field_state {
    uint32_t size, version;
    uint64_t generation;
    uint32_t modified, conflict;
    char base_revision[128], model_revision[128];
} ui_cap_form_field_state_t;

typedef struct ui_cap_form_state {
    uint32_t size, version;
    uint64_t pending_call;
    uint32_t dirty_fields, conflict_fields;
    char base_revision[128], model_revision[128];
} ui_cap_form_state_t;

typedef struct ui_cap_form_binding_desc {
    uint32_t size, version;
    const char *component_id, *capability_id;
    ui_cap_resource_ref_v2_t resource;
    ui_status_t (*read_model)(char revision[128], char *fields_json,
        size_t capacity, size_t *length, void *user_data);
    void (*observe)(const ui_cap_ui_notice_t *, void *user_data);
    void *user_data;
} ui_cap_form_binding_desc_t;

UI_API ui_status_t ui_capability_form_bind(ui_host_t *,
    const ui_cap_form_binding_desc_t *, uint64_t *binding_id);
UI_API ui_status_t ui_capability_form_project(ui_component_t *,
    const ui_cap_form_projection_t *);
UI_API ui_status_t ui_capability_form_get_field_state(ui_component_t *,
    const char *field_id, ui_cap_form_field_state_t *);
UI_API ui_status_t ui_capability_form_get_state(ui_component_t *,
    ui_cap_form_state_t *);
```

UI线程；size至少完整结构，未知version拒绝。绑定复制身份/元数据，read_model/observe/user_data持有至finish_unbind成功。只支持框架默认Light后端，应用指定后端及WebView2不支持；允许在mount前绑定，不能因此切换成未适配后端。

read_model只读回应用真实业务，写框架提供的有界缓冲和revision；不得保存缓冲、执行业务或等待worker。投影fields_json为字段ID到类型化值的对象，长度不含NUL，最大64KiB；需完整覆盖绑定表单的非只读语义字段，不接受未知ID或错误类型。TEXT/ENUM/COLOR为字符串、NUMBER为有限JSON数、BOOLEAN为布尔；只读IMAGE/GROUP等不构成提交参数。单字段保留现有4096字节等限制。输入参数为`{"fields":{...}}`，最终仍由能力Schema/业务check验证。

## 3. 草稿、快照及显式解决

初始绑定从read_model取得业务状态/版本；不将控件初值冒充业务。投影原子验证后才更新模型显示，保留modified草稿。每次草稿变化递增字段generation；业务revision变化时，所有基于旧资源revision的脏字段保守标冲突并保留base revision。单资源版本策略不暗中按字段合并。

提交前捕获完整类型化字段、每字段generation和base revision。expected revision来自草稿base，不在点击时刷新到最新模型来绕过冲突。已知冲突明确拒绝且不execute；未投影的外部业务变化由C2-A revision门拒绝，再只读刷新模型并标冲突。标签切换不更换resource ref。

冲突UI提供两个新的人工动作：保留草稿（只读刷新模型后明确将该草稿base移到当前版本，不提交）、重新载入业务值（放弃旧草稿并更新base）。提交必须另有新人工事件。取消同样只读回当前业务并丢弃本地草稿，必要时关闭当前dialog；这些动作不调用应用修改回调，程序消息不能伪造解决。

业务完成OK且效果明确后必须再次read_model，revision与结果after一致且提交字段实际值吻合，才能接纳对应草稿。只有generation仍等于提交快照的字段可清modified；异步期间用户新输入保留。若实际值不符、读回失败或版本变化，保留草稿/冲突并报告回读问题，不宣称UI已保存。框架不能修正或重执行业务。

UNKNOWN结果不选择性接纳、不清冲突、不解除资源写门，仍依C2-A新人工核对流程。C2-B不新增强制解锁或审批。旧accept_fields原义保持；新绑定不调用它来表示业务成功。

## 4. 人工来源、modal和生命周期

标准提交/取消/冲突按钮和适用的Enter/Escape从Light私有合格动作链进入；脚本消息/公开模拟输入/自报source拒绝。对话框打开不保留提交权限。仅匹配当前modal组件所属的绑定可执行，后台表单、其他组件不能借modal例外；最后回调后重新比较所属组件及modal状态。v1 modal仍统一拒绝。

收参和read_model复制存储计入原8MiB，UI绑定元数据计入原2MiB，调用/输出继续共同额度。模型JSON、字段快照及回读缓冲有界，失败和解绑释放计账。同步完成、异步稳定ID、观察回调、取消、背压及UNKNOWN沿C2-A。finish_unbind在收参/回读/通知仍借用时PENDING；解绑不取消已接纳业务，后续不再回调已解绑表单，其结果仍可按call ID查询。组件解绑前不能销毁，不能让在途回读引用悬空。

## 5. 专用验收 C2B-AC-1

| 编号 | 必须验证 |
| --- | --- |
| B01 | 绑定共用目录/目标/call/预算，无第二业务执行器；同组件重复绑定拒绝 |
| B02 | text/number/boolean/enum/color完整类型化收参；未知/非法/缺失参数和disabled字段拒绝 |
| B03 | 表单与dialog共享内存业务name但草稿独立；提交后实际业务回读一致 |
| B04 | 提交snapshot/generation固定；异步期间新草稿不被完成误接纳 |
| B05 | 外部revision更新保留草稿、标冲突；旧base不被换新；版本失败execute为0 |
| B06 | 保留草稿/重载须新人工动作且只读，不自动提交；再次提交独立凭据 |
| B07 | 同modal所属提交有效；父表单/其他组件/程序消息/过期所属无效；回调新开modal使旧调用拒绝 |
| B08 | 完成回读失败/值不符/版本漂移不假清草稿；UNKNOWN不被UI接纳伪装成功 |
| B09 | 同步/异步/取消/重复、满队列/预算、迟到消息和在途解绑/关闭不丢call或提前释放 |
| B10 | 原组件/表单/模态/布局、C1/A+B、Assistant及公共C/C++头相称回归；物理输入单列未验 |

全部新增验收在实现前未运行；结果记入独立C2-A+B验收记录，保留首次失败及匹配二进制。C2-C、专业画布/WebView2、跨实例文档、应用迁移、AI/P3—P6仍排除。不得放宽旧断言、超时、限额、数据范围或冻结C2-A来实现本合同。

## 6. C2B-1实现状态增补（2026-10-23）

本会话后获连续实现授权，前面的文档冻结时状态保留历史。冻结正文/结构/错误及资源边界不改；实现现已交付并完成联合本地验证。C2B-AC-1逐项证据及未验限制以[独立联合验收记录](validation/xuanshu-p2c2ab-validation.md#4-验收矩阵及证据限制)为准，不能将原“未运行”表回写成历史已运行。

Native17/17、Light23/23、ASAN17/17；真实物理输入、OS来源动态矩阵和v2真实workspace标签切换正例未验。代码完成不等于全部产品验收通过，当前停在联合代码评审。来源、预算、busy/UNKNOWN/生命周期及v1布局保持冻结合同，没有自动进入C2-C、样例模型/五示例迁移或P3—P6。

## 7. 2026-10-23集中收口状态增补

前述冻结正文及原实现证据完整保留。本轮联合审查修正末次回调来源/guard门、失败资格消费及本地草稿回读后modal门，未改变冻结结构、v1语义/错误或限额。[独立集中报告](validation/xuanshu-p2c2-closeout-validation.md)记录新增回归、首次失败和匹配产物。

最终Native22/22、Light30/30、Native-ASAN22/22；A5896/0、B214/0与Light266/0。真实workspace两实例固定目标已补自动验证，不再保留为“只有固定指针”缺证，但没有冒充物理标签点击。C2C-1六类绑定/宿主chrome已实施；原生HMENU非read仍缺适配，真实鼠标/键盘和完整Windows来源动态矩阵未验，C2整体暂不通过完整验收。历史三项首次失败独立保留；停止联合代码评审。
