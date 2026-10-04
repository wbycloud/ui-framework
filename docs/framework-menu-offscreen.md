# 分组菜单与离屏测试接口

SDK0.4.0开发版、框架API4、开发标准修订4。审查基准为 `074a70fa5d2e04dde3b872a76641571c158db7e6`。运行库接受API1/2/3/4，应用ABI、`ui_app_query_v1`和包格式保持1；不创建稳定标签。

本轮只修改框架和通用样例，没有修改请求方应用、文档模型、业务渲染器或PERF-001。能力状态和实际证据见[验收记录](validation/api4-validation.md)。

## 1. 菜单与工具栏

[menus.h](../include/ui_framework/menus.h)复用已有菜单、工具和命令注册。`ui_host_register_menu_group`可选指定路径、显示名称与顺序；未指定时按子项的最小order和路径排序。路径使用 `/` 分隔，访问根路径用空字符串；组内按order和稳定ID排序。组是容器，其下命令的enabled/visible/checked/busy与界面项状态按原合同合成。

独立宿主常驻显示活动实例顶层分组；宽度不足进入“更多”。工具保留所属toolbar与组序；`ui_toolbar_desc_t.display=UI_TOOLBAR_COMPACT`在有图标时只显示图标和公共提示，无图标时保留标题。旧描述缺少尾部display字段时使用原默认值。

宿主“打开应用包”快捷键改为Ctrl+Shift+O；Ctrl+O留给应用。内容Web、绘图surface和框架外壳使用既有语义命令；外壳消息固定实例目标，处理时检查仍是活动实例。编辑区保留编辑快捷键。菜单支持左右切换根组、上下选择及翻页、Enter执行、Esc关闭并恢复焦点。当前没有Alt访问键。

`ui_host_show_menu`显示已注册路径，复制popup描述和路径。anchor选择HOST、CONTENT_SLOT或COMPONENT_ROW：HOST.rect相对应用host客户区；SLOT.rect相对该内容槽；ROW使用组件当前可见行的裁剪矩形。业务target_id独立于锚点，以十进制字符串发送为params.id及params.target，action为menu。ROW要求当前缓存中的有效项目；slot和component必须属于同一host。描述清零、设置size，矩形使用逻辑像素。路径最多4095 UTF-8字节，保持旧组件菜单路径预算。

Windows使用独立的框架Web弹窗，可覆盖原生OpenGL子窗口。按锚点所在显示器work area翻转、夹紧位置；内容分页，单层最多128项，超限返回LIMIT_EXCEEDED。分页和键盘可到达最后一项，不悄悄丢弃项目。`ui_component_show_menu`兼容入口转发到这个共同实现，已移除固定在组件内部的旧菜单模板。

点击重新检查实例、行代次和命令状态；切换实例、关闭、删除锚点行、换源、组件注销、shell销毁、尺寸/DPI改变会关闭旧菜单。`ui_host_show_toolbar_menu`重用已注册工具和命令；提示使用show_tooltip/hide_tooltip。菜单本身没有新建业务命令系统，也不代替业务校验和助手权限。

## 2. 真实轻量Web离屏接口

NULL parent轻量view是已有能力。本版增加正常布局/绘制代码的公共输出及交互接口，不复制测试专用渲染器。

- `ui_web_view_get_presentation`返回view本地逻辑rect、有效clip、visible/enabled/focused、text_overflow和复制文本。它是受控模板的元数据，不是完整浏览器无障碍树。文字溢出以实际文本布局及截图补验。
- `ui_web_view_capture_rgba`使用显式逻辑尺寸、DPI和调用方缓冲；轻量输出顶向下RGBA8、不透明alpha255。NULL pixels查询宽高/stride；否则必须给足capacity及正stride，调用结束后应用拥有缓冲。
- `ui_web_view_flush`执行至多budget个脚本job（1..1024）并完成当前布局，仍有job返回CANCELLED。这个状态不是业务任务完成。
- 组件提供mount_offscreen、dispatch_input、get_presentation、capture_rgba和flush。TREE/TABLE/LIST/FORM/STATUS/DIALOG使用相同模板；输入仍通过命中、焦点、脚本、数据源与语义命令。
- 菜单提供对应input/presentation/capture/flush；get_item_presentation按注册ID查找，避免应用依赖内部DOM键。页面元素ID仍可用于公共呈现诊断。

新增backend ops只追加并按size读取；旧后端缺少函数返回UNSUPPORTED。轻量报告OFFSCREEN_CAPTURE/PRESENTATION_QUERY能力位，WebView2不报告。捕获不包括应用的OpenGL或原生内容，不包括输入法候选或桌面交换画面。单次GDI目标限制32MiB、单边32767像素；表格/树继续原DOM和数据预算。

## 3. 显式无窗口workspace

`ui_workspace_config_t`追加reserved_v3和run_mode，保留API3结构尾部填充。清零后默认UI_RUN_WINDOWED，仍要求有效HWND；只有显式UI_RUN_OFFSCREEN接受NULL parent。无窗口模式不接受非NULL parent；host/workspace均提供get_run_mode。无需应用创建隐藏宿主。

复用原包校验、绝对路径加载、共享DLL、资源、实例ID、create/mount/set_active/request_close/unmount/destroy/module_shutdown及复制队列。实例有逻辑/像素内容槽，native_container和slot native handle为NULL。应用查询运行模式，在无窗口路径避免自己的HWND、timer、系统文件选择器及屏幕消息依赖。

组件正常mount到内容槽；无窗口DIALOG使用同一Web模板及实例模态状态，阻止目标实例其他组件输入并保留提交/取消语义，不创建owner窗口。浮动槽拥有虚拟矩形，不提供操作系统浮动窗口；调用原生浮动操作接口返回UNSUPPORTED。

UI线程主动poll或flush，不依赖WAKE_MESSAGE。flush限制本轮复制消息数量和每个view的脚本job数（1..1024）；CANCELLED表示仍有排队/脚本工作或当前回调尚未退出。查询/绘制及应用回调本身由应用保证有界。worker完成仍以正常结果/关闭完成投递确认，不能把flush空闲当成worker已停止。

测试控制台 `ui_workspace_offscreen_test`是可构建的公共API驱动入口，加载匹配DLL和framework_features.uapp、操作真实模板、验证REFUSE/WAIT和重开，不是任意应用的通用自动化脚本。旧应用仍可在正常宿主加载；依赖HWND的旧应用不会自动适配离屏。

## 4. 隐藏WGL离屏绘图

`ui_opengl_offscreen_surface_create`是显式入口；要求至少3.3 compatibility，不接受legacy、core或MSAA，不静默降级。Windows仍创建隐藏WGL drawable/DC及临时bootstrap窗口，get_window_dependency报告HIDDEN_WINDOW；不声称无Windows图形会话支持。实际vendor/renderer/version/profile通过原get_info读取；硬件/软件类别标为unknown，不凭驱动字符串推测。

surface可接入原内容槽。`ui_surface_dispatch_input`复用逻辑输入、关闭/模态门控及原快捷键和input回调；不合成WM消息。`ui_opengl_offscreen_render`调用已有frame一次，绑定框架私有FBO、等待RGBA读回完成，恢复此前WGL context、framebuffer、viewport及读回所用像素打包/PBO状态。NULL pixels只查尺寸，不调用frame。输出顶向下RGBA8；alpha保留应用framebuffer的实际值，不推测业务混合方式。stride/capacity由调用方管理，跨模块不转移像素内存。

应用frame可以调用原make_current，绘制固定管线、纹理及自己的FBO，最终把完整结果合成到调用时的目标FBO；不能把结果留在另一个私有FBO。离屏swap只flush，不交换桌面画面；invalidate不隐式连续绘制，驱动显式调用render。

context、GPU资源、resize及销毁属于创建时UI线程；显式render和make_current拒绝错误线程。颜色/深度附件合计限制32MiB，临时读回和调用方缓冲另外计算，单边限制16384且受驱动最大纹理尺寸限制。卸载前解除surface回调并释放应用GPU资源；host销毁会回收框架FBO和隐藏窗口。

第一阶段不提供完全无窗口的GL context、MSAA离屏或无登录CI承诺。真实输入法、物理跨屏、桌面合成和SwapBuffers呈现仍需单独验收。
