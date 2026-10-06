# 分组菜单与离屏测试接口

SDK0.7.0开发版、框架API7、开发标准修订7。API5四项已有实现继续保留，当前源码以实际commit为准。只维护接手确认的当前API7；[版本政策](version-policy.md)取代旧版兼容承诺，现存加载分支暂不清理。应用ABI、`ui_app_query_v1`和包格式保持1；不创建稳定标签。

本轮只修改框架和通用验收样例，没有修改请求方应用、文档模型、业务渲染器或PERF-001。当前能力及证据见[API7验收](validation/api7-validation.md)，历史结果见[API6验收](validation/api6-validation.md)与[API5历史验收](validation/api5-validation.md)；[API4记录](validation/api4-validation.md)保留历史结果。

## 1. 菜单与工具栏

[menus.h](../include/ui_framework/menus.h)复用已有菜单、工具和命令注册。`ui_host_register_menu_group`可选指定路径、显示名称与顺序；未指定时按子项的最小order和路径排序。路径使用 `/` 分隔，访问根路径用空字符串；组内按order和稳定ID排序。组是容器，其下命令的enabled/visible/checked/busy与界面项状态按原合同合成。

独立宿主常驻显示活动实例顶层分组；宽度不足进入“更多”。工具保留所属toolbar与组序；`ui_toolbar_desc_t.display=UI_TOOLBAR_COMPACT`在有图标时只显示图标和公共提示，无图标时保留标题。旧描述缺少尾部display字段时使用原默认值。

宿主“打开应用包”快捷键改为Ctrl+Shift+O；Ctrl+O留给应用。内容Web、绘图surface和框架外壳使用既有语义命令；外壳消息固定实例目标，处理时检查仍是活动实例。编辑区保留编辑快捷键。菜单支持左右切换根组、上下选择及翻页、Enter执行、Esc关闭并恢复焦点。API5 的 Alt 访问键合同见第6节。

`ui_host_show_menu`显示已注册路径，复制popup描述和路径。anchor选择HOST、CONTENT_SLOT或COMPONENT_ROW：HOST.rect相对应用host客户区；SLOT.rect相对该内容槽；ROW使用组件当前可见行的裁剪矩形。业务target_id独立于锚点，以十进制字符串发送为params.id及params.target，action为menu。ROW要求当前缓存中的有效项目；slot和component必须属于同一host。描述清零、设置size，矩形使用逻辑像素。路径最多4095 UTF-8字节，保持旧组件菜单路径预算。

Windows使用独立的框架Web弹窗承载连续菜单行，嵌套组侧向展开，父层仍可见，可覆盖原生OpenGL子窗口。按锚点所在显示器work area翻转、夹紧位置；内容分页，单层最多128项，超限返回LIMIT_EXCEEDED。分页和键盘可到达最后一项，不悄悄丢弃项目。`ui_component_show_menu`兼容入口转发到这个共同实现，已移除固定在组件内部的旧菜单模板。

点击重新检查实例、行代次和命令状态；切换实例、关闭、删除锚点行、换源、组件注销、shell销毁、尺寸/DPI改变会关闭旧菜单。`ui_host_show_toolbar_menu`重用已注册工具和命令；提示使用show_tooltip/hide_tooltip。菜单本身没有新建业务命令系统，也不代替业务校验和助手权限。

### 桌面菜单样式与指针交互

宿主应用菜单使用独立的`app-menu`样式：26逻辑像素高、平面文字项，默认无独立边框/圆角；悬停、聚焦及展开高亮。当前第一行为最近箭头、实例标签、加号、AI及窗口按钮；第二行菜单右端“⋯”为宿主选项，主题/布局重置迁到其中。应用工具保持工具样式，其他业务按钮不随菜单变化。加号提示为Ctrl+Shift+O。[当前宿主](standalone-host.md)及[验收](validation/browser-host-validation.md)补充外壳合同；历史☰截图和结果保留。浅色/深色通过宿主同一主题同步到菜单及框架共同组件，应用自有内容主题仍由应用管理。中性表面、MDL2子菜单箭头和对齐细节见[视觉规范](visual-design.md)及[本轮验收](validation/visual-ui-validation.md)，分页和命令合同不变。

共同弹窗使用28逻辑像素连续行，勾选/忙碌、图片、文字、已注册快捷键和子菜单箭头各有固定对齐区域。缺图也保留对齐空间；若标题尾部与注册的组合快捷键重复，仅在展示中去掉重复后缀，注册字符串和命令保持。默认隐藏路径标题及返回/关闭按钮；无分页只留8px外沿，有分页保留上下入口和原键盘导航。快捷键按虚拟键名称显示（含F1–F24与导航键），不把F1当作ASCII字符。

有菜单打开时，宿主悬停其他顶层入口切换根菜单；重复点击当前入口关闭，其他入口打开对应菜单。悬停可用子组向侧面展开，右侧不足则翻到左侧并按work area夹紧。已有子层时，父层其他行的切换保留160毫秒跨层宽限；进入子层或离开候选行取消待切换。外部点击关闭，不吞原点击；失活、模态、实例切换、锚点失效及销毁关闭整链。鼠标悬停本身不执行命令，点击仍重新校验状态和身份。

Alt/助记键/方向键/Enter沿原合同；键盘的根组选择直接打开组内容，Left返回父层，Esc关闭整链。焦点只恢复到仍活动、可见且可接收输入的原窗口；切到其他顶层窗口时不夺回焦点。组合输入及旧宿主/应用快捷键路由保持，真实IME仍另验。

每层128项、路径4095字节、原分页窗口最多10项保持。整条缓存展开链按每层最坏76个节点预留，合计不超过1024，最多13层；可见层按实际DPI共同计入32MiB像素预算。超预算关闭链，不静默提高限制或丢弃注册项；程序仍可按完整注册路径直接打开深层组。子层缓存复用，host销毁回收全部view、窗口、timer和本UI线程消息hook。无窗口路径执行同样的逻辑展开、输入和当前层捕获，不创建菜单HWND或原生hook，不把它当作桌面合成证据。

本次是内部模板和交互修正，公共C头文件、size、字段偏移、枚举、ABI及菜单注册/命令参数合同未改。当前实现、失败复现、真实宿主/原包回归和请求方只读前后截图见[菜单修正验收](validation/menu-desktop-validation.md)。

## 2. 真实轻量Web离屏接口

NULL parent轻量view是已有能力。本版增加正常布局/绘制代码的公共输出及交互接口，不复制测试专用渲染器。

- `ui_web_view_get_presentation`返回view本地逻辑rect、有效clip、visible/enabled/focused、text_overflow和复制文本。它是受控模板的元数据，不是完整浏览器无障碍树。文字溢出以实际文本布局及截图补验。
- `ui_web_view_capture_rgba`使用显式逻辑尺寸、DPI和调用方缓冲；轻量输出顶向下RGBA8、不透明alpha255。NULL pixels查询宽高/stride；否则必须给足capacity及正stride，调用结束后应用拥有缓冲。
- `ui_web_view_flush`执行至多budget个脚本job（1..1024）并完成当前布局，仍有job返回CANCELLED。这个状态不是业务任务完成。
- 组件提供mount_offscreen、dispatch_input、get_presentation、capture_rgba和flush。TREE/TABLE/LIST/FORM/STATUS/DIALOG使用相同模板；输入仍通过命中、焦点、脚本、数据源与语义命令。
- 菜单提供对应input/presentation/capture/flush，均针对当前最深的展开层；get_item_presentation按注册ID查找，避免应用依赖内部DOM键。原生鼠标按各层实际窗口命中；无窗口调用方的坐标相对当前层。页面元素ID仍可用于公共呈现诊断。

新增backend ops只追加并按size读取；旧后端缺少函数返回UNSUPPORTED。轻量报告OFFSCREEN_CAPTURE/PRESENTATION_QUERY能力位；API5 WebView2也报告并异步完成这两项，运行条件见[共同组件合同](generic-web-ui.md#api5-webview2-与共同组件)。捕获不包括应用的OpenGL或原生内容，不包括输入法候选或桌面交换画面。单次GDI目标限制32MiB、单边32767像素；表格/树继续原DOM和数据预算。

## 3. 显式无窗口workspace

`ui_workspace_config_t`追加reserved_v3和run_mode，保留API3结构尾部填充。清零后默认UI_RUN_WINDOWED，仍要求有效HWND；只有显式UI_RUN_OFFSCREEN接受NULL parent。无窗口模式不接受非NULL parent；host/workspace均提供get_run_mode。无需应用创建隐藏宿主。

复用原包校验、绝对路径加载、共享DLL、资源、实例ID、create/mount/set_active/request_close/unmount/destroy/module_shutdown及复制队列。实例有逻辑/像素内容槽，native_container和slot native handle为NULL。应用查询运行模式，在无窗口路径避免自己的HWND、timer、系统文件选择器及屏幕消息依赖。

组件正常mount到内容槽；无窗口DIALOG使用同一Web模板及实例模态状态，阻止目标实例其他组件输入并保留提交/取消语义，不创建owner窗口。浮动槽拥有虚拟矩形，不提供操作系统浮动窗口；调用原生浮动操作接口返回UNSUPPORTED。

UI线程主动poll或flush，不依赖WAKE_MESSAGE。flush限制本轮复制消息数量和每个view的脚本job数（1..1024）；CANCELLED表示仍有排队/脚本工作或当前回调尚未退出。查询/绘制及应用回调本身由应用保证有界。worker完成仍以正常结果/关闭完成投递确认，不能把flush空闲当成worker已停止。

测试控制台 `ui_workspace_offscreen_test`是可构建的公共API驱动入口，加载匹配DLL和framework_features.uapp、操作真实模板、验证REFUSE/WAIT和重开，不是任意应用的通用自动化脚本。旧应用仍可在正常宿主加载；依赖HWND的旧应用不会自动适配离屏。

## 4. 隐藏WGL离屏绘图

`ui_opengl_offscreen_surface_create`是显式入口；要求至少3.3 compatibility，不接受legacy或core；API5显式正采样数要求精确匹配，不静默降级。Windows仍创建隐藏WGL drawable/DC及临时bootstrap窗口，get_window_dependency报告HIDDEN_WINDOW；不声称无Windows图形会话支持。实际vendor/renderer/version/profile通过原get_info读取；硬件/软件类别标为unknown，不凭驱动字符串推测。

surface可接入原内容槽。`ui_surface_dispatch_input`复用逻辑输入、关闭/模态门控及原快捷键和input回调；不合成WM消息。`ui_opengl_offscreen_render`调用已有frame一次，绑定框架私有FBO、等待RGBA读回完成，恢复此前WGL context、framebuffer、viewport及读回所用像素打包/PBO状态。NULL pixels只查尺寸，不调用frame。输出顶向下RGBA8；alpha保留应用framebuffer的实际值，不推测业务混合方式。stride/capacity由调用方管理，跨模块不转移像素内存。

应用frame可以调用原make_current，绘制固定管线、纹理及自己的FBO，最终把完整结果合成到调用时的目标FBO；不能把结果留在另一个私有FBO。离屏swap只flush，不交换桌面画面；invalidate不隐式连续绘制，驱动显式调用render。

context、GPU资源、resize及销毁属于创建时UI线程；显式render和make_current拒绝错误线程。颜色/深度附件合计限制32MiB，临时读回和调用方缓冲另外计算，单边限制16384且受驱动最大纹理尺寸限制。卸载前解除surface回调并释放应用GPU资源；host销毁会回收框架FBO和隐藏窗口。

API4第一阶段不提供完全无窗口GL和MSAA；API5增补见第6节，仍不承诺无登录CI。真实输入法、物理跨屏、桌面合成和SwapBuffers呈现仍需单独验收。

## 6. API5 菜单、无窗口 GL 与 MSAA 合同

菜单组和项追加 `access_key`：0 无助记键，ASCII A-Z/a-z/0-9，字母不区分大小写；非法值拒绝。原 SDK4 组的32字节完整前缀（含尾 padding）保留，旧描述不读取新字段。裸 Alt 松开进入根菜单，Alt+助记键打开组，上下/左右、Enter、Esc 使用共同弹窗和命令路由。重复助记键循环同层可用项，再用 Enter 执行；禁用/busy 项忽略。键按住不重复提交，松开后才接受下一次。嵌套 Left 返回父组，溢出分页仍可用助记键访问末项。实例切换、模态与关闭取消旧菜单，焦点只恢复到仍活动且可接收输入的原窗口。

原生嵌入应用在 TranslateMessage 之前调用 `ui_host_menu_dispatch_input`，仅 OK 时吞掉该消息及其 SYSCHAR；NOT_FOUND 按原路由处理。宿主和框架原生 Web/surface 已接入。组合输入期间不传菜单键；Ctrl+Alt（AltGr）、Ctrl/Shift组合及既有已注册 Alt 快捷键保持优先权。真实 IME 仍须在目标机器补验，不能由注入字符推断通过。

`ui_opengl_windowless_surface_create` 显式接收 `ui_opengl_windowless_config_t.library_path_utf8` 的绝对 OSMesa DLL 路径。上下文绑定应用内存，私有 FBO 执行应用 frame，不调用 WGL、GetDC 或创建隐藏 HWND，依赖查询为 NO_WINDOW。原 `ui_opengl_offscreen_surface_create` 继续走隐藏 WGL/DC，依赖查询为 HIDDEN_WINDOW，默认行为不改。两者共用 frame/input、resize、get_info、读回和销毁；应用所有 GL 调用（包括 GL1.1）必须经该 surface 的 get_proc_address，不能把 opengl32 导入函数用于 OSMesa。

选择 OSMesa 是可复验的实际软件桌面 GL 方案，不声称 GPU 加速。Windows EGL/WGL 平台不能自动等同于没有 HWND/DC；本轮未实现硬件 EGL。验证固定提供方为 [mesa-dist-win 24.3.4](https://github.com/pal1000/mesa-dist-win/releases/tag/24.3.4) 的 x64 MSVC 包；[维护方说明](https://github.com/pal1000/mesa-dist-win) 已移除新版 OSMesa，应用必须显式部署兼容 DLL、依赖和许可，不能只升级到最新包。上游 OSMesa 的 screen/worker 缓存是进程级，本机最后一个 context 销毁后模块仍驻留；框架释放自己的引用，不强制卸载外部提供方。上下文/附件回收与模块驻留分别验证。

离屏样本数0表示普通附件，正数是精确请求。创建时实际分配多采样 RGBA8 和 depth24/stencil8 renderbuffer，查询两者 GL_RENDERBUFFER_SAMPLES；超过 GL_MAX_SAMPLES、驱动取整或不完整 FBO 都明确失败，不降级。frame 在多采样 FBO 上绘制，blit resolve 到单采样 RGBA8 再读回顶向下 RGBA8。get_info.samples 报告已验证的实际附件采样数。

每 surface 预算32 MiB。旧隐藏WGL单采样继续按8×像素计颜色/深度，临时读回另计、最多16 MiB，保留API4的2048×2048边界。真正无窗口单采样按12×像素计颜色/深度/临时读回；多采样按(8+8×samples)×像素计多采样附件、resolve和临时读回。OSMesa路径再计4字节 provider drawable，调用方缓冲另计。resize 先验预算，失败保持旧尺寸。UI/创建线程操作，销毁需先停止调用；多上下文保留当前提供方 context，恢复 framebuffer、viewport、texture/renderbuffer、pack/unpack PBO、pack 参数及 multisample/scissor。应用自行改变其他 GL 状态仍由应用负责。

实际 Windows 已登录会话、Session0、整机无登录CI分别验收。真实软件 GL frame 输出可以证明无窗口渲染；返回 UNSUPPORTED、替代图片和模拟执行都不能证明成功。历史78c24cb实际测试DLL/OSMesa在CI LocalSystem Session0已通过，当前API7源码仍需单独复验，整机无登录因登录会话1失败，缺专用runner；[专门证据](validation/session0-osmesa-validation.md)保留两个独立结论。其他结果及未验收条件见[API5验收](validation/api5-validation.md)。

## API6 无窗口布局与既有能力回归

workspace在有窗口与Windows UI_RUN_OFFSCREEN路径共享[布局合同](workspace-layout.md)：版本化保存/恢复、分隔条逻辑尺寸和停靠拖拽提交保留内容槽与GL context。无窗口没有实际popup或原生捕获，逻辑布局与frame/input/resize仍走应用原生命周期。真实OSMesa、隐藏WGL、MSAA与普通附件的原能力声明保持准确，不新增硬件无窗口后端。

API5 Alt/嵌套/溢出/模态/焦点、OSMesa实际上下文、离屏MSAA与状态恢复、WebView2真实Runtime查询/捕获/图片/组件继续进入本轮回归。历史Session0成功、当前源码Session0待验和整机无登录未验分别记录，不能把Windows已登录桌面的测试矩阵当作后者通过。

## API7 捕获取消与布局

UI_INPUT_CANCEL=8追加，事件尺寸不变，有窗口捕获丢失和无窗口调用方取消共用；滚动语义见[通用Web](generic-web-ui.md)。底部及同区标签沿用内容生命周期，布局2读取布局1，见[布局](workspace-layout.md)。菜单/Alt/焦点/单次命令及GL/MSAA/普通附件合同不改；没有硬件无窗口扩展。当前GL/Session0与整机无登录分开记于[API7验收](validation/api7-validation.md)。

本轮仅移除共同TREE/TABLE/LIST分页与列箭头按钮；菜单单层128项、页窗口和末项访问合同不变。真实回归见[分页清理验收](validation/component-scroll-only-validation.md)。
