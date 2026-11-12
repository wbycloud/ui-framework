# XuanShu P2 C2-C 输入绑定与验收合同

当前状态增补（2026-10-23）：下文“未实现/仅设计”描述保留原冻结阶段，不是最新交付状态。C2已实现并在本轮补HMENU及标准组件入口、五示例接入；原规范正文不变。见[独立验证](validation/xuanshu-p2-remaining-validation.md)、[增量设计](xuanshu-p2-remaining-implementation.md)。物理验收尚未开始，不宣称完整通过。


规格身份：**C2C-1，2026-10-23，实现前冻结**。本会话明确授权先设计再实施，本文件不改变C2A-1、C2B-1及v1。读者为框架及应用开发者，不署名。统一目录、执行器、ID/Schema/资源、预算和注销不变；AI/Assistant始终只读。

## 1. 源码盘点与入口矩阵

| 实际入口 | 效果 | 来源/参数/固定目标/执行路径 | 预定证据 |
| --- | --- | --- | --- |
| Light菜单叶项 | v1 read；v2 view/draft/write | 复用A私有节点链/collect/复制ref/共同UI action | A与C入口回归 |
| Light标准FORM/DIALOG与字段 | 同上；输入默认本地草稿 | 复用B提交快照、base/generation和冲突；新提交新事件 | B与Light分支 |
| host快捷键，Light及原生surface聚焦入口 | 同上 | 独立key/modifier绑定；私有原生KEY_DOWN，编辑手势保留，程序dispatch不授权 | input夹具/混合链/真实workspace |
| Light工具栏弹出菜单 | 同上 | 框架toolbar item ID绑定，实际菜单节点匹配，元数据状态最后门 | Light模板 |
| 原生工具栏 | 同上 | 框架私有toolbar subclass命中真实item；先匹配按下/释放，不能凭WM_COMMAND | 原生控件夹具 |
| 标准Light按钮、输入框、TREE/TABLE/LIST行/单元格 | 同上 | 明确view或component＋DOM ID＋提交事件；私有原生链拦截，不采信JS消息；collect给出完整语义参数 | 各控件/表单分支 |
| Win32标准BUTTON | 同上 | 绑定同UI线程/同host父链中的真实BUTTON，框架安装私有子类；BM_CLICK/WM_COMMAND无资格 | 隐藏真实BUTTON负例/私有链正例 |
| 原生2D/OpenGL画布surface | 同上 | 原生surface按下/释放或键盘语义提交、坐标/修饰键快照；collect表达业务语义，execute仍能力回调 | 原生画布专用夹具 |
| 公开输入、host invoke/shortcut、workspace invoke、页面消息、任意应用自报source | read按原合同；非read拒绝 | 不能签发资格，不回退legacy执行v2 | 伪造/重复回归 |
| 原生HMENU | 非read未适配/明确拒绝 | WM_COMMAND不能证明私有匹配人工事件；这是本轮入口覆盖的未完成项，不能凭其他入口通过宣称C2全部完成 | 源码边界；仍缺适配及正例 |
| STATUS、框架面板/标签/拖拽布局内部动作 | STATUS仅显示；原框架view行为保留 | 无业务提交的显示不伪造write；面板布局不声称已业务能力迁移 | 原组件/宿主回归 |
| WebView2/自定义非Light Web后端 | 本轮排除，不授权非read | 不以页面消息/source降级 | UNSUPPORTED/拒绝 |

专业画布仅通用surface事件/参数与夹具，不实现EDA对象、任意几何编辑或专属执行器。五示例业务迁移另批。原生HMENU非read没有可信事件栈适配，是范围内未完成项；不能仅凭当前公开shell消息入口补签人工来源。该缺口单列，未悄然移出本轮完成范围。

## 2. 必要独立增量

新增capability_inputs.h，一个bind函数，其ID/解绑沿用ui_capability_ui_begin_unbind/finish_unbind。新描述没有业务handler或权限字段；只收参和观察。所有非read仍走原UI action、gate、稳定call和complete/result，不新增执行器。C2A的menu_item_id原义不变。

```c
typedef enum ui_cap_input_kind {
    UI_CAP_INPUT_SHORTCUT=1, UI_CAP_INPUT_TOOLBAR=2,
    UI_CAP_INPUT_LIGHT_CONTROL=3, UI_CAP_INPUT_COMPONENT=4,
    UI_CAP_INPUT_SURFACE=5, UI_CAP_INPUT_NATIVE_BUTTON=6
} ui_cap_input_kind_t;
typedef struct ui_cap_input_context {
    uint32_t size, version;
    ui_cap_ui_context_t invocation;
    ui_input_event_t event;
    const char *control_id, *value_utf8;
} ui_cap_input_context_t;
typedef struct ui_cap_input_binding_desc {
    uint32_t size, version;
    ui_cap_input_kind_t kind;
    ui_web_view_t *view;
    ui_component_t *component;
    ui_surface_t *surface;
    void *native_button;
    const char *control_id;
    ui_input_kind_t trigger;
    uint32_t key, modifiers, pointer_button;
    const char *capability_id;
    ui_cap_resource_ref_v2_t resource;
    ui_cap_ui_purpose_t purpose;
    uint64_t reconcile_call_id;
    ui_status_t (*collect)(const ui_cap_input_context_t *, char *, size_t,
        size_t *, void *);
    void (*observe)(const ui_cap_ui_notice_t *, void *);
    void *user_data;
} ui_cap_input_binding_desc_t;
UI_API ui_status_t ui_capability_input_bind(ui_host_t *,
    const ui_cap_input_binding_desc_t *, uint64_t *binding_id);
```

size覆盖完整version1，未知版本/非法组合INVALID_ARGUMENT，未支持后端/控件UNSUPPORTED；同端点＋trigger/key/modifier/button重复ALREADY_EXISTS，退休期间仍占。SHORTCUT仅key/modifier，TOOLBAR的control_id为已登记toolbar item且命令匹配能力，LIGHT_CONTROL指定本host Light view和明确DOM ID，COMPONENT指定已mount的标准组件和DOM ID，SURFACE指定本host surface，NATIVE_BUTTON为同UI线程同host父链中的标准BUTTON。其他owner指针必须NULL。trigger仅POINTER_UP或KEY_DOWN；pointer button1..3，key1..255，modifiers仅Ctrl/Shift/Alt。新接口注册v2非read引用；read继续已有v1/C1入口，不伪造v2 read。

DOM ID最长255字节，元数据和字符串复制计入原2MiB，临时输入快照及collect在原预留工作区/8MiB内，结果沿用原1024/8MiB。event只借用至collect返回，text_utf8固定NULL；不保存worker可变指针。UI线程、callback/user_data至finish成功有效。控件引用是身份而非凭据，owner销毁/重建/DOM代次失效停止绑定，不自动改绑；回调内销毁被绑定owner拒绝/延后，解绑后可销毁。

## 3. 私有来源与一次接纳

由GetCurrentInputMessageSource在私有WndProc/subclass识别；硬件分类/有效device是已确认合作边界，不等于物理设备证明。普通注入、系统/未知/读取失败不授权；混合按下/释放、不同按钮/修饰键/控件、焦点捕获丢失、公开模拟事件均撤销候选。键盘自动重复不重复提交。元数据匹配也不能代替来源。

框架在匹配原生语义事件栈中签发内部一次性资格，固定owner/节点代次、能力、ref/open epoch、revision和规范参数。私有collect上下文含坐标/key/modifier/控件ID，不允许应用签发资格。最后应用回调后复查来源/事件序列、binding/owner/节点/disabled/busy/modal/关闭以及revision。失败也消费本事件资格；新调用必须新事件。回调中新嵌套事件作废旧资格。

已绑定语义动作不得再触发旧业务handler。非可信或失效事件被明确拒绝，不回退legacy执行v2；未迁移的旧人工UI保持原行为但不宣称已能力迁移。文本输入/字段编辑本地draft；Enter/明确动作才提交。保留编辑器常用复制/粘贴/方向键手势，禁止把任意网页消息当提交。

未知、busy、取消、背压和生命周期完全复用A；UNKNOWN核对是新的人工只读操作，不能借输入绑定强制清门。owner退役不抹去call，解绑后结果仍可查询；观察租约退栈前finish PENDING。UI不阻塞等待worker，框架不追踪任意应用线程。

## 4. 验收 C2C-AC-1

C01 单一目录/绑定重复及预算、v1/Assistant不可非read；C02 私有来源分类、注入/系统/未知负例、混合动作/按钮/修饰键、公开调用拒绝；C03 一次消费、重复/嵌套/DOM代次/owner销毁失效；C04 快捷键与编辑手势；C05 Light按钮/工具栏/标准组件精确参数；C06 原生BUTTON/toolbar真实句柄和程序点击拒绝；C07 surface语义坐标/键盘与独立业务回读；C08 最后回调modal/disabled/busy/关闭门；C09 真实workspace标签切换后旧call保持原实例/资源，epoch重开拒迟到，revision冲突；C10 同步/异步/重复/取消/UNKNOWN/背压/在途解绑注销卸载；C11 A/B、C1/A+B、Assistant、headers、组件、两个DLL分支Native/Light/ASAN；C12 真实鼠标/键盘及Windows动态来源独立记录，私有钩子/模拟/SendInput不冒充物理正例。

本规格落盘时以上C项尚未运行。固定新构建根保留首失败、当时源码及匹配二进制、原始日志/JUnit。程序分类函数的穷举不能替代Windows实际来源动态验证。没有物理验证时不宣称完整产品验收；实际未适配HMENU、WebView2排除项和应用迁移边界如实列出。停止C2联合评审，不自动进入P3。

## 5. 收参细化（在对应字段实现前冻结）

标准Light输入框需要读取尚未发送为业务命令的当前值；仅event/control ID不足以表达重命名或单元格提交。context补充value_utf8：框架在事件进入时复制最多4096字节的控件当前值，只借用到collect返回；非输入控件/画布/工具栏为NULL，不是业务已提交证据。原预留8MiB工作区内保存，回调重入不改写此快照；应用仍须collect完整参数并通过同一Schema/check。此为新C2C-1增量细化，不修改A/B或v1。原生BUTTON/toolbar标准激活限定左键或Enter/Space，其他组合UNSUPPORTED；其他控制/画布仍按显式trigger。

## 6. 实际宿主chrome入口补充（在桥接实现前冻结）

盘点examples/framework_host/main.c发现主工具栏和chrome快捷键在独立chrome host中显示，原消息排队后只用source调用app。仅适配toolbar弹出菜单不足以覆盖真实入口。新增两个纯元数据函数ui_capability_workspace_input_bind(workspace,view)、ui_capability_workspace_input_unbind(view)，在同一UI线程为框架Light chrome注册/移除内部路由；不公开执行、token或人工自报接口。绑定复制workspace/view身份并计入原2MiB；调用/解绑/销毁仍受租约约束。

框架宿主在每次成功同步投影后记录私有input_instance，失败置0；原生语义动作按此明确已显示实例取其同一TOOLBAR/SHORTCUT绑定，检查实例仍active且未关闭。不能读取最新活动标签去重定向旧显示。源host当前私有资格一次消费，源事件/DOM/投影实例及目标门在回调后仍须有效；源host/view和workspace都持有退栈租约。页面app-command/app-shortcut消息继续不能授权。workspace或view销毁先断开路由，未完成call独立留在原实例。此为C2-C必要host适配，不是跨实例共享文档写入或应用模型迁移。

ui_capability_workspace_input_bind要求框架Light view没有其他semantic绑定，重复ALREADY_EXISTS；不支持后端UNSUPPORTED，错误参数INVALID_ARGUMENT，正在借用时unbind为PENDING。size/version及v1/A/B接口不变。新增两个元数据接口需公共C/C++可链接验证和真实workspace chrome夹具；物理来源仍单列未验。

C2B已拥有的FORM/DIALOG拒绝再绑定C2C通用入口（ALREADY_EXISTS）；已有C2C组件绑定也阻止再建立整表单绑定，退休期间继续占用至finish。核对绑定collect必须NULL；不静默忽略应用字段。原生坐标转为逻辑像素，来源分类只接纳SDK已知有效device枚举。

## 7. 本轮实现结果与未完成项

六类绑定及第6节真实宿主chrome元数据桥已实现；三函数独立增量仍复用同一目录/执行器，不改C2A-1或v1。新增公共C/C++shared链接验证，生产DLL无私有来源/fixture导出。最终Native22/22、Light30/30、Native-ASAN22/22；专用inputs228/0、Light373/0、真实workspace Native257/0和Light282/0。详细入口矩阵、修复和匹配身份见[集中收口验收](validation/xuanshu-p2c2-closeout-validation.md)。

第1节原生HMENU为范围内未完成适配，未将其移出范围；因此不宣称C2代码入口全部完成。真实物理鼠标/键盘和Windows动态来源正例未验，private fixture不证明physical。程序消息OS来源origin0/device0负例已验证，分类逻辑/混合链夹具通过。WebView2按本轮授权排除；应用模型/五示例/AI均未实施。当前停止C2联合代码评审，完整C2验收暂不通过。
