# Windows C/Web UI 框架应用开发标准

开发标准修订：**4**。对应 **SDK 0.4.0 开发版、框架 API 4**；运行库接受 API 1/2/3/4，应用 ABI、导出 ui_app_query_v1 和包格式仍为 1。没有创建稳定标签，最近稳定基准仍为 v0.1.0。开发者应记录实际 SDK commit，而不是只记录 main。

面向能阅读 C/C++ 头文件、Win32 和 OpenGL 示例的开发者。新应用通过公共 C 接口注册组件、提供数据、绑定已有语义命令，通用 HTML/CSS/JavaScript、草稿、焦点和交互由框架维护。菜单、工具、面板内组件、状态、参数及确认界面使用 Web；Win32 仅承载窗口、消息、输入法及绘制。旧 API1/2 应用自有原生内容和原生嵌入式保留，系统文件/目录选择器是例外。

## 0. 阅读入口与版本约定

先读[通用 Web UI 接入约定](generic-web-ui.md)，再参考[通用纯 C 示例](../examples/generic_components/README.md)。新接口见 [components.h](../include/ui_framework/components.h)、[images.h](../include/ui_framework/images.h)；命令、面板、内容槽、助手和应用生命周期仍以原公共头文件为准。

[0.3 → 0.4 迁移指南](migration-v0.3-to-v0.4.md)说明当前菜单及离屏接口，[0.2 → 0.3 迁移指南](migration-v0.2-to-v0.3.md)保留通用组件和废弃输入开关的迁移要求；[CHANGELOG](../CHANGELOG.md)区分兼容、可选和必须迁移；[当前验收](validation/api4-validation.md)记录构建、自动化与真实宿主证据。API4 包不能加载到只支持 API1/2/3 的运行库，清单与 DLL 声明必须相同。

受控轻量后端实现本版框架组件；WebView2 保留已有自定义 HTML/消息能力，不提供新增 C 图片 ID 和组件呈现。OpenGL 仍由应用自行选择、通过内容槽挂载，框架没有应用文档模型或业务渲染器。具体子集、预算及未实现能力以接入约定的能力清单为准，不把 HTML 支持视为完整浏览器。

本版追加[分组菜单与离屏约定](framework-menu-offscreen.md)、[0.3 → 0.4迁移](migration-v0.3-to-v0.4.md)及[API4验收](validation/api4-validation.md)。公共菜单、呈现查询、复制像素输出、显式无窗口workspace及隐藏WGL均遵守原线程、所有权和卸载规则。离屏是可选运行模式，旧包可保持原API；新的可构建接入示例见[framework_features](../examples/framework_features/README.md)。

## 1. 架构和应用职责

框架由独立 Web 宿主、C 核心、Win32/OpenGL 适配层和 Web 内容后端组成。`framework_host.exe` 拥有顶层窗口、Web 标签/菜单及全局助手。它加载 `.uapp` 内的 C ABI DLL，为每个实例创建独立 host、带真实内容容器的兼容 shell 和 assistant；应用维护文档数据、业务行为和内容。应用不应包含 `src/ui_internal.h`，也不应依赖 Lexbor、QuickJS 或 WebView2 的私有类型。

同一进程中的应用共享 `ui_framework.dll`。应用 DLL 链接对应 import library，不能再静态链接一份独立框架。此模型不是进程隔离：应用的非法指针或崩溃可能影响整个宿主。第一目标是 Windows x64；框架不自动恢复上次会话，也不把任意现成 EXE 嵌入标签。

宿主不提供私有依赖 DLL 的命名空间或版本隔离。除共享框架和系统 DLL 外，应用应静态链接依赖，或给私有 DLL 使用应用专属文件名并让 module 的 imports 引用该名称，避免不同应用的同名依赖互相影响。

| 层 | 当前能力 | 应用负责的内容 |
| --- | --- | --- |
| 独立宿主 | 应用包验证和加载、标签切换、实例 ID、关闭和 DLL 卸载、全局助手 | 应用清单、多实例声明、生命周期回调 |
| C 核心 | host 生命周期、布局与 DPI、命令和异步结果、事件、surface 回调、Web 后端接口 | 文档模型、业务校验、工作任务、文件格式 |
| 外壳与内容容器 | 独立宿主的 Web 菜单/工具入口/面板外框、借用内容槽；原生嵌入式的菜单/工具栏/面板和消息转发 | 注册框架组件和业务数据；绘图 surface；旧内容兼容 |
| OpenGL 层 | 显式版本/profile/MSAA/debug 配置、兼容路径、GPU/context 信息查询 | 绘制、资源、文档坐标、缩放、滚动、选择和拾取 |
| 助手协议 | 命令允许列表、schema 元数据、校验和确认回调、进度、取消、快照、事务接口 | 模型服务连接、权限界面、实际事务和撤销数据 |
| 轻量 Web 后端 | Lexbor HTML 解析、QuickJS-NG 脚本、GDI 绘制、受控动态 DOM/布局/控件和 JSON 消息 | 遵守受控子集、页面资源、语义命令 |
| WebView2 后端 | 纯 C COM 适配、异步 HTML/JS、可信页面命令桥接 | Runtime 部署、消息循环、页面和浏览器能力选择 |

构建同时保留静态框架和原有嵌入式样例，并提供独立宿主所需的共享框架。Windows 默认开启独立宿主与轻量 Web 后端，WebView2 默认关闭。只构建原生/OpenGL 时，同时设置 `UI_BUILD_STANDALONE_HOST=OFF` 与 `UI_FRAMEWORK_ENABLE_LIGHT_WEB=OFF`，无需 HTML/JS 引擎；此配置不生成独立宿主。原ui_framework_headless目标仍只验证核心状态；真实轻量无HWND和隐藏WGL由Windows完整运行库的显式路径提供，两者分别验收。

框架没有内置 EDA、Markdown、画板、PPT、Excel 文档模型，也没有自动加载 `app://` 资源、完整停靠管理器、网络模型客户端或 TypeScript 编译器。将这些能力实现为应用逻辑，保持公共 C 接口作为接入边界。

## 2. 应用包和生命周期

### 2.1 清单与资源

`.uapp` 是版本化的单文件容器，包含包头、UTF-8 INI 清单、文件索引和未压缩内容。宿主验证格式、架构、ABI、长度、偏移、名称和路径后，将 DLL 提取到独立临时目录，交给 Windows 加载器。应用包不用自己提供框架运行库。

最小 EDA 的 [`manifest.ini`](../examples/minimal_eda/manifest.ini) 使用以下字段：

```ini
[application]
app_id=org.ui-framework.minimal-eda
name=Minimal EDA
version=1.0.0
architecture=x64
abi_version=1
framework_api_version=1
module=minimal_eda_app.dll
multiple_instances=true
```

`app_id` 是稳定身份，`version` 标识版本；同一 ID 的不同版本不能同时加载。`multiple_instances=false` 时，重复打开包激活已有标签；为 true 时创建新的私有状态和标签。不要通过进程全局变量保存实例数据。

清单必须有且仅有一个 `[application]` 节，以上八个字段必须各出现一次。UTF-8 可带 BOM，接受 LF/CRLF、空行和以 `#`/`;` 开头的注释；不接受未知字段。当前支持 `architecture=x64`、应用 ABI 1 和框架 API 1/2/3/4。清单 API 必须与 DLL descriptor 一致；使用当前头文件构建的新应用声明 4，旧 API 1/2/3 包保持原值。`app_id` 使用字母、数字、点、下划线和连字符，首字符为字母或数字。文件名为有效 UTF-8 相对路径，以 `/` 分隔；不得使用绝对路径、反斜杠、`.`/`..`、Windows 设备名或大小写冲突的同名文件。同包不能同时含文件 `assets` 和路径 `Assets/icon.txt`，避免文件/目录前缀冲突。打包源目录不得含符号链接或 reparse points，输出包必须在源目录之外。

### 2.2 UAPP v1 容器格式

所有整数按 little-endian 编码，不直接写 C struct。总文件大小不超过 256 MiB，清单不超过 64 KiB，条目最多 4096 个，条目名称最多 255 个 UTF-8 字节。版本 1 无压缩、CRC、内容哈希或数字签名；验证包头、索引、路径和版本，不检测保持长度不变的内容位翻转，也不证明 DLL 来源或隔离其代码。

| 64 字节包头偏移 | 类型 | 含义 |
| --- | --- | --- |
| 0 | 8 bytes | magic：`55 41 50 50 0D 0A 1A 0A` |
| 8 | uint32 | 格式版本，1 |
| 12 | uint32 | 包头长度，64 |
| 16 | uint64 | 总文件长度，必须等于实际文件大小 |
| 24 | uint64 | 清单偏移，64 |
| 32 | uint64 | 清单字节数，不含终止零 |
| 40 | uint64 | 索引偏移，紧接清单 |
| 48 | uint32 | 文件条目数 |
| 52 | uint32 | 保留，0 |
| 56 | uint64 | 内容偏移，紧接完整索引 |

每条索引由 `uint32 name_length`、保留的零 `uint32`、`uint64 content_offset`、`uint64 content_length` 和恰好 `name_length` 字节的 UTF-8 名称组成。内容按索引顺序紧密连续，第一条从内容偏移开始，最后一条结束于文件末尾；零长度文件允许。清单的 module 名必须与索引名称完全相同。打包器按名称排序，应用通常调用 [`package.h`](../include/ui_framework/package.h) 或 `uapp_pack`，无需自己编码容器。

通过 `ui_app_resource_read(context, "readme.txt", &data, &size)` 在 mount 或已挂载实例的 UI 回调中读取包资源。create 时实例尚未注册进 workspace，资源读取不可用；在 mount 分配所需数据，并在 unmount/destroy 使用已保存内容清理。读取前将输出清零并检查返回状态，只有成功后读取内容。返回数据是框架分配的 `size` 字节，末尾额外补零，可用于 UTF-8 文本；成功后必须用 `ui_app_resource_release(data)` 释放，不能交给应用自己的 `free()`。路径是包内资源名，不是自动加载的 `app://` URL。外部图片或脚本加载仍由选择的 Web 后端能力决定。

### 2.3 模块入口和实例创建

DLL 导出固定的 C 入口 `ui_app_query_v1`，返回 `ui_app_descriptor_t`。函数表存储在模块内，设置 `size`、`abi_version = UI_APPLICATION_ABI_VERSION` 和 `framework_api_version = UI_FRAMEWORK_API_VERSION`；C++ 实现入口需使用 `extern "C"`。不得改变结构 packing，或向公共结构插入私有字段。

`ui_app_query_v1` 只返回函数表。DllMain、query 和 C++ 静态构造均不得启动线程、创建 UI 或注册外部异步回调；它们发生在实例生命周期建立前，ABI 拒绝时也没有可执行的实例清理协议。将业务初始化放到 create/mount，使失败路径仍能清理。

宿主按以下顺序创建实例：

1. 分配不会复用的 `instance_id` 和实例 host，建立当前 DPI 和内容几何，再调用 `create(context, &state, &assistant_config)`。
2. 应用分配私有状态，复制 context，注册命令、菜单、工具栏和面板，设置布局；此时 context 中的 shell/assistant 尚不可用。
3. 宿主根据应用配置创建 shell 和 assistant，调用 `mount(state, context)`。
4. 应用更新私有 context，获取面板容器，创建内容与 OpenGL/Web 视口，在 assistant 注册允许列表。
5. 宿主通过 `set_active(state, active)` 通知前台/后台变化。应用保留文档和资源，后台停止持续绘制；不要用整体 refresh 来切换标签。

`create` 失败后，非空 state 仍会交给 `destroy`。`mount` 可能只完成一部分：应用的 `unmount` 必须能清理已创建的资源和子窗口。初始化失败不会进入 request_close；create/mount 返回失败前须停止已启动的异步工作和外部回调。所有生命周期回调在宿主 UI 线程执行，应用不另开顶层消息循环。`ui_app_context_t` 借用宿主对象，应用可以复制结构但不能销毁其中的 host、shell、assistant 或 workspace。

### 2.4 关闭、拒绝与等待

`request_close` 返回 ALLOW、REFUSE 或 WAIT。未保存内容由应用提示保存；ALLOW 保证工作线程、定时器、外部回调已经停止。WAIT 时宿主停止新命令，应用完成停止后使用 `ui_workspace_post_close_complete(workspace, instance_id, decision)` 投递 ALLOW 或 REFUSE。WAIT 不是强制终止线程。

确认 worker 已实际结束、COM/异步订阅已解除且回调不会再进入模块后，才能完成 ALLOW。不能由仍在 DLL 代码栈上的 worker 把“即将返回”当成“已经停止”；可由 UI 线程确认 join/停止完成后投递。回调 scope 只保护在途 UI 调用，不能代替停止和等待后台任务。关闭中的标签不可重新激活，关闭被拒绝后才恢复输入和激活状态。

正常关闭时，宿主先调用 `unmount`，再销毁 shell、assistant、host，随后调用 `destroy`。`unmount` 在 surface/context 仍有效时释放应用 GPU 对象、native 子窗口、Web backend 等；应用如自行销毁 surface/view，必须清空保存指针，避免重复使用。`destroy` 只释放私有状态，不再访问已失效的框架对象。

最后一个实例结束后调用 `module_shutdown`，用于清理模块共用窗口类和其他全局资源。返回成功后宿主在全部调用退出时卸载 DLL；失败则保留模块并报告错误，不强制卸载。模块自行注册的窗口类、线程入口、timer callback、subclass 和外部函数指针都不能在 DLL 卸载后残留。ALLOW 已保证任务和外部回调静默，module_shutdown 不应再用 WAIT 延后这些任务。框架不会为应用强制杀线程。

### 2.5 原有嵌入式接入

[`examples/minimal_eda/main.c`](../examples/minimal_eda/main.c) 保留由应用创建窗口的模式。应用必须在首个 HWND 前调用 `ui_framework_initialize()` 请求 Per-Monitor V2 DPI；已设置 unaware/system-aware 时返回 `UI_STATUS_UNSUPPORTED`。随后创建 host、注册 UI、创建 shell/surface，再运行自己的消息循环。

这种模式中顶层 HWND 属于应用；host 拥有 surfaces/views，但不拥有 shell、assistant 和独立 Web backend。退出前停止任务，释放应用 GPU/native/backend 资源，先销毁 assistant 和 shell，再销毁 host，最后销毁顶层 HWND。专用 Web backend 使用对应的 `ui_light_web_backend_destroy()` 或 `ui_webview2_backend_destroy()`，不要混用通用销毁函数。嵌入式模式没有独立 workspace 的跨线程投递队列。

## 3. UI 线程、UTF-8 和 C ABI

### 3.1 UI 线程和回调

创建、注册、布局、消息处理、Web 脚本调用、直接结果回复和销毁都应发生在 host 的 UI 线程。独立宿主中的后台线程只使用 workspace 与 instance ID，通过 `ui_workspace_post_result()`、`post_event()`、`post_progress()` 或 `post_close_complete()` 复制投递数据；不能把 host、HWND、应用函数指针或借用的字符串传进队列。

宿主在 `UI_WORKSPACE_WAKE_MESSAGE` 后调用 `ui_workspace_poll()`，在 UI 线程交付消息；已关闭的实例 ID 对应消息被丢弃。应用保证投递期间 workspace 仍存活，所有 worker 停止后才能允许关闭。嵌入式应用需使用自己的线程安全队列，再在 UI 线程直接调用 host 接口。

框架自动追踪命令、surface frame/input/resize、窄窗口策略以及 Web backend ops 的在途回调；workspace 在这些回调尚未返回时延后实例销毁和 DLL 卸载。应用自己注册的 WndProc、subclass 和外部 UI/COM 回调不经过这些入口，必须成对调用 `ui_app_callback_enter(context)` 和 `ui_app_callback_leave(context)`。

enter 覆盖整个调用：应用处理、`DefWindowProc`/`DefSubclassProc`、同步发送消息和模态消息循环都在 scope 内；每条返回分支调用 leave，leave 后直接返回，不再执行会泵消息的 UI 操作。scope 可以嵌套，必须平衡；context/host 在整个范围内有效。最小 EDA 的 `properties_proc` 和 `panel_resize_proc` 展示完整用法，包括默认窗口过程。嵌套消息循环可能收到关闭和 WAKE 消息，scope 保证卸载推迟到原调用退出。

这两个接口只用于宿主 UI 线程，不是引用计数或跨线程寿命保证。后台 worker、COM MTA 回调和非 UI 的外部通知不能用 scope 保活 host；使用 workspace post 复制投递，并在关闭时通过 WAIT 完全停止它们。COM STA/UI 回调可以使用 scope，但同样必须解除订阅并等待不会再有回调，才能允许关闭。

回调可能同步触发。例如 `ui_host_invoke()` 内部会直接进入命令 handler，handler 又可以立刻回复，进入 result callback；`ui_surface_set_callbacks()` 会立即调用新注册的 resize callback。

回调不得销毁正在 dispatch 的 host、shell、assistant、surface 或 Web view，也不得在布局回调内重新配置该布局。在 Web 命令 handler 中销毁当前 view、重载当前轻量页面，或在 frame callback 中销毁当前 surface 都会破坏正在执行的对象。将这些动作延迟到回调返回后的 UI 消息处理阶段。允许在命令 handler 中回复结果、报告进度，以及在结果回调中释放已完成的助手请求记录。

### 3.2 字符串和所有权

公共命令 ID、标题、HTML、事件、JSON 和脚本文本使用以零结尾的有效 UTF-8。ID 应使用稳定命名空间，如 `eda.add_block`；标题可翻译，ID 不应随显示语言变化。WebView2 的 `user_data_folder` 是一个明确的 Windows 路径例外，配置类型使用 `const wchar_t *`。

| 数据 | 所有权和有效期 |
| --- | --- |
| 命令、菜单、工具栏、面板描述中的字符串 | 注册时复制，调用结束后应用可释放原字符串 |
| 布局描述和 Web backend ops | 框架复制描述/函数表，不保留调用方栈上的结构地址 |
| callback 和 `user_data` | 仅保存指针，应用负责其有效期；框架不释放应用数据 |
| command handler 的 `params_json`、source 和回调参数 | 借用；异步保存时必须自行复制 |
| `ui_result_t` 及其字符串、事件 payload、WebView2 JS 结果 | 仅在当前回调内有效，不得保存为悬空指针 |
| 助手 snapshot 回调返回的字符串 | 应用拥有；获取方不得释放，更新前需要自行复制 |
| `ui_app_context_t` 和 `ui_app_instance_info_t` 中的框架对象、名称 | 借用；实例或模块关闭后失效，不能从 worker 访问 |
| `ui_app_resource_read()` / `ui_package_read()` 返回的数据 | 框架分配；分别使用 resource release / package release 释放 |
| workspace post 的字符串 | 调用时复制；返回后调用方可释放自己的原始数据 |
| shell、content slot、native panel/surface/view handle | 借用；slot 到 shell 销毁为止有效，应用不得销毁框架拥有的容器 HWND |
| Web JSON 消息回调参数 | 仅在本次回调内借用；异步保留时复制，后端按数据解析，不拼接为脚本 |
| `ui_opengl_info_t` 和矩形查询输出 | 写入调用方结构，调用方拥有该副本 |

### 3.3 size 和兼容性

使用当前头文件时，先将描述结构清零，再设置 `size = sizeof(结构)`。host 配置还需设置 `api_version = UI_FRAMEWORK_API_VERSION`，当前值为 4。可用 `ui_framework_supports_api()` 查询运行库是否接受某个 API 版本；当前接受 1、2、3、4，拒绝 0 和未支持的更高版本。新增可选描述字段放在原有字段之后；枚举已有数值保持不变。不要改变公共结构的 packing，也不要把应用私有字段插入公共结构。

布局接口接受原始六字段布局描述，省略的新字段按零值处理；Web ops 支持旧尺寸，缺少追加的定位、消息或能力回调时对应操作返回 UNSUPPORTED。`ui_workspace_config_t` 在保留完整 v1 布局后追加 `shell_mode`；清零时选择 `UI_WORKSPACE_SHELL_NATIVE`，自建 Web workspace 可显式选择 `UI_WORKSPACE_SHELL_WEB`。独立宿主固定使用 WEB。并非所有结构都允许截断，不能人为缩小 `size` 来假装某个版本。C++ 应用通过 `extern "C"` 调用同一接口；[`tests/public_headers.cpp`](../tests/public_headers.cpp) 覆盖公共头文件调用路径。

## 4. 命令、结果和事件

先用 `ui_host_register_command()` 注册语义命令，再让菜单、工具栏、快捷键或可信 Web 页面引用其 ID。注册重复 ID 返回 `UI_STATUS_ALREADY_EXISTS`；菜单和工具栏项引用的 command/toolbar 必须已经存在。当前没有注销或修改注册项的 API。

`ui_host_invoke()` 返回非零请求 ID；无法找到或派发命令时返回 0。handler 可立即调用 `ui_host_reply()`，也可保存请求 ID，完成工作后在 UI 线程回复。`params_schema_json` 是元数据，host 不解析 JSON，也不替应用校验参数。异步 handler 需要自行复制参数，不能保留借用字符串。

每个请求只回复一次。`ui_host_reply()` 从 pending 列表移除请求并调用配置中的 result callback，重复回复返回 `UI_STATUS_NOT_FOUND`。同步结果回调可能在 invoke 返回前已经发生，不应等 invoke 返回后才创建用于识别请求的状态。

用 `ui_host_emit_event()` 发布应用变化，如样例中的 `eda.state.changed`。框架同时发布 `ui.host.layout_changed` 和 `ui.host.dpi_changed`；Web shell 的增量刷新成功后发布 `ui.shell.changed`，通知宿主更新外壳。应用定义增量事件的字段和版本，并在需要完整状态时提供 snapshot；框架不自动生成文档差异，也不存储事件历史。

最小 EDA 样例的 `eda.add_block`、`eda.zoom_in`、`eda.clear` 均经过同一 host 命令路径。应用业务功能应遵循这个模式，使菜单、快捷键和助手可以复用行为。

不同实例可以注册相同 ID，请求 ID 也可以相同。跨标签调用/日志必须同时携带 instance ID，不能用 command ID 或 request ID 单独识别目标。调用 `ui_workspace_invoke(workspace, instance_id, command_id, params_json, &request_id)` 后，目标固定为该实例，即使用户切换标签，结果、事件和进度仍属于原目标；关闭中的实例拒绝新命令。

## 5. 菜单、工具栏、面板和内容槽

### 5.1 通过注册项接入外壳

`ui_host_register_menu_item()` 的 `menu_path` 使用 `/` 表示层级，例如 `File/Actions`，`order` 决定排列。先注册业务 command，再注册引用其 ID 的菜单或工具项。工具栏先注册 `ui_toolbar_desc_t`，再注册属于该 toolbar ID 的 item。`icon_url` 是注册元数据，当前没有自动下载或解码图标。

独立宿主从活动实例的注册项生成 Web 菜单和工具入口，切换标签保留应用内容。workspace 负责激活/隐藏内容及浮窗，应用通过 `set_active` 调整任务和绘制。兼容 shell 保留供旧接口查询的借用 HMENU 镜像，但不把它安装到宿主窗口；应用不得通过安装或修改它改变 Web 外壳，也不应枚举原生 toolbar/window class。

原生嵌入式 shell 仍创建真实 HMENU 和 Common Controls toolbar，将点击转发为 `ui_host_invoke(..., "{}", "native")`。HMENU 属于非客户区，原生菜单模式将 `menu_bar_height` 设为 0。managed native shell 的 `menu_owner`、激活/移除菜单和后备菜单约定保持原有行为；[`tests/native_shell_integration.c`](../tests/native_shell_integration.c) 与 [`tests/native_activation.c`](../tests/native_activation.c) 覆盖这条保留路径。

### 5.2 内容槽与面板挂载

面板通过 `ui_host_register_panel()` 注册。`UI_PANEL_SIDEBAR` 按 `dock_region` 停靠到左/右侧，`NONE` 保留默认右侧行为；`UI_PANEL_FLOATING` 创建独立浮动面板。`preferred_width` 控制浮动初始宽度，停靠侧栏总宽由 host 布局决定。`entry_url` 是兼容描述的必填非 NULL 字段，原生面板设置为 `""`；它不自动读取资源、打开页面或创建 backend。

新应用在 mount 后通过 `ui_host_get_shell(host)` 取得借用 shell。调用 `ui_shell_get_content_slot(shell, NULL)` 选主内容区，传已注册的 panel ID 选面板内容区；不存在时返回 NULL。slot 到 shell 销毁时才失效。`ui_content_slot_native_handle()` 返回借用容器 HWND，新应用在其中挂载框架组件或绘图内容；原生 child 仅作为旧应用兼容路径，禁止销毁容器。

`ui_content_slot_attach_surface()` 挂载原生/OpenGL surface，`ui_content_slot_attach_web_view()` 挂载 Web view；内容必须属于该槽的 host，传 NULL 解除对应绑定。这些操作不转移所有权，应用仍在 unmount 中销毁内容、清空保存指针，并在销毁 view 后销毁其专用 backend。主内容槽的 Web backend 使用 host 的 native parent 和 host 坐标；面板 Web backend 必须以槽容器为 parent，view 使用容器局部坐标。一个槽只能绑定一种内容，内容对象也不能同时绑定到其他槽。

`ui_content_slot_get_rect()` 返回相对实例 host 的逻辑矩形，`get_pixel_rect()` 返回设备像素矩形。浮动内容的矩形以 `(0,0)` 为原点，尺寸属于浮窗内容区。`ui_shell_get_slot_state()` 可查询 frame/content、visible 和 floating；停靠矩形使用 host 坐标。OpenGL framebuffer 仍使用 surface 的像素查询结果，不能把逻辑宽高直接用于 viewport。

旧应用继续用 `ui_native_shell_panel_handle(context->shell, panel_id)` 获取真实容器。停靠/浮动及标签切换保留容器 HWND、内容 child 和 GL context；原生 child 的内部布局仍由应用根据实际容器尺寸调整。多个同侧停靠面板按垂直空间分配；折叠标签可临时打开浮窗，`ui_native_shell_set_panel_floating()` 可主动浮动/返回停靠，返回时仍受窄窗口策略限制。浮窗独立处理 DPI 和 suggested RECT。当前没有拖拽停靠树、分隔条调整或布局持久化。

### 5.3 增量刷新与旧重建接口

运行中新增注册项后，新接口 `ui_shell_refresh()` 更新外壳并保留已有内容容器和绑定；新增面板可在刷新后查询内容槽。resize/DPI 使用 reflow，不需要重建或重新 mount。内容槽行为对应 [`tests/web_shell_slots.c`](../tests/web_shell_slots.c)，完整 Web 挂载示例见 [`examples/web_counter/app.c`](../examples/web_counter/app.c)。

旧 `ui_native_shell_refresh()` 仍重建菜单、工具栏和面板，原 panel HWND 及其应用 child 会失效。继续使用该接口的应用在刷新前释放内容资源，刷新后重新查询 handle 并创建内容。不要将旧 refresh 当作增量更新，也不要用它切换标签。`ui_native_shell_reflow()` 只应用布局，不重建内容。

## 6. 响应式布局和 DPI

### 6.1 逻辑矩形和侧栏约束

`ui_host_resize()` 接受逻辑客户区尺寸，96 DPI 对应 100% 缩放。`ui_host_get_rect()` 查询 ROOT、MENU_BAR、TOOLBAR、MAIN、左右 SIDEBAR、STATUS_BAR 的逻辑矩形。顶部和底部区域先占用高度，主区与左右侧栏共享剩余区域。

侧栏宽度按首选宽度、旧的 width 字段、最小/最大约束计算。`preferred_width` 为 0 时使用旧 width；`max_width` 为 0 时不设置额外上限。`main_min_width` 为 0 时采用 400 的策略目标。这个目标不能在任意微小窗口下强制保证；可用空间不足且侧栏不可折叠时，主区仍会压缩。

下列设置摘自已编译的 [`tests/responsive_layout.c`](../tests/responsive_layout.c)：

```c
layout.size = sizeof(layout); layout.toolbar_height = 36; layout.status_bar_height = 24;
layout.right_sidebar_preferred_width = 320; layout.right_sidebar_min_width = 260;
layout.right_sidebar_max_width = 360; layout.main_min_width = 400;
layout.collapsed_tag_width = 32;
```

该测试中的主区逻辑尺寸如下；96、144、192 DPI 下逻辑布局相同，设备像素矩形随 DPI 改变。

| 逻辑客户区 | 主区 | 右侧栏 |
| --- | --- | --- |
| 1920 × 1080 | 1600 × 1020 | 可见，320 宽 |
| 1280 × 720 | 960 × 660 | 可见，320 宽 |
| 800 × 600 | 480 × 540 | 可见，320 宽 |
| 640 × 480 | 608 × 420 | 折叠，32 宽标签轨道 |

### 6.2 窄窗口策略

| `ui_narrow_window_policy_t` | 行为 |
| --- | --- |
| `UI_NARROW_MAIN_PRIORITY` | 默认策略；先尝试缩小侧栏到 min，必要时依次折叠右侧、左侧 |
| `UI_NARROW_KEEP_SIDEBARS` | 保留侧栏的请求宽度，允许主区压缩；总宽度仍不能超出客户区 |
| `UI_NARROW_COLLAPSE_LEFT` | 需要折叠时只允许折叠左侧 |
| `UI_NARROW_COLLAPSE_RIGHT` | 需要折叠时只允许折叠右侧 |
| `UI_NARROW_FLOAT_SIDEBARS` | 空间不足时把需要让出的侧栏变为浮动状态 |
| `UI_NARROW_DISALLOW_SHRINK` | 结合最小窗口尺寸限制，让正常交互缩放保留请求区域 |

`collapsed_tag_width == 0` 时折叠侧栏不保留标签轨道；非零值保留可点击标签。`left_sidebar_no_collapse` 和 `right_sidebar_no_collapse` 阻止对应侧栏折叠，主区可能因此更窄。通过 `ui_host_get_sidebar_state()` 查询可见、折叠或浮动状态，通过 `ui_host_set_narrow_policy_callback()` 让应用按尺寸/DPI返回策略。该 callback 也可能在最小尺寸查询中执行，必须保持可重复调用且不得修改 host。

禁止继续缩小时，应用仍需把 `WM_GETMINMAXINFO` 转发给 `ui_host_handle_message()`，让框架将最小逻辑客户区尺寸转换为含边框/菜单的最小跟踪尺寸。直接调用 `ui_host_resize()` 或程序性修改窗口可以绕过交互跟踪限制，不能将该策略当成尺寸校验器。

### 6.3 Windows 消息接入

独立应用由宿主负责首个 HWND 前的 DPI 初始化和根窗口消息。宿主 Web 布局先划分标签/全局助手/实例工作区，再调用 `ui_workspace_set_rect()`，为包括后台标签在内的实例同步逻辑尺寸和 DPI。实例内部由 C 核心计算布局，`ui_host_get_rect()` 保持相对实例容器的逻辑坐标。应用不将主窗口 WM_SIZE 的完整客户区再次应用到实例 host；workspace 内容矩形相对宿主客户区，Windows 标题栏不计入其中。

以下转发要求用于嵌入式应用，或实现自定义 workspace 外壳的开发者。未被框架处理的消息继续自己的派发或 `DefWindowProc`：

| 消息 | 应用接入要求 |
| --- | --- |
| `WM_COMMAND` | 转发给 `ui_native_shell_handle_message()`，处理 native shell 的菜单、toolbar 和标签按钮 |
| `WM_SIZE` | 转发 host 消息，框架把物理客户区尺寸转为逻辑尺寸，再调用 shell reflow；最小化时尺寸可为 0 |
| `WM_DPICHANGED` | 转发新的 DPI，然后由应用用 `lParam` 的 suggested RECT 调整顶层窗口；重新布局 native shell 和内容 |
| `WM_GETMINMAXINFO` | 转发 host 消息，支持 `DISALLOW_SHRINK` 的最小跟踪尺寸 |

shell 的消息接口也可以直接处理 `WM_SIZE`/`WM_DPICHANGED`：它内部转发 host 并执行 reflow。选择“host 转发 + shell reflow”或“shell 转发”一种路径，避免同一消息重复更新。suggested RECT 的应用仍由顶层窗口所有者完成。EDA 样例采用第一种路径。

DPI 变化会更新已存在 surface 的像素矩形，包括手动放置的 surface，并通知变化。已绑定内容槽或 layout region 的 Web view 跟随布局与 DPI；手动放置的 view 仍由应用更新逻辑尺寸并检查返回码。后端 DPI 同步失败时 `ui_host_set_dpi()` 返回错误，host 或其他 surface 可能已经更新；应用应记录失败并重试相同 DPI，不能假设这是整体回滚事务。不要把 physical 像素值直接传给逻辑尺寸 API。

自动测试中的模拟 `WM_DPICHANGED` 证明消息路径和像素换算；它不证明 Windows 在真实跨显示器移动时的 DPI、字体、非客户区和驱动行为。发布前必须在实际不同缩放的显示器间移动窗口完成手动验证。

## 7. OpenGL surface 接入

使用 [`opengl.h`](../include/ui_framework/opengl.h) 的 `ui_opengl_surface_create()` 显式请求 OpenGL version、profile、MSAA 样本数和 debug context。现代配置不可用时返回 NULL，并通过 status 返回 `UI_STATUS_UNSUPPORTED`，不会静默降级。

最小 EDA 的模块和嵌入样例默认请求 OpenGL 3.3 compatibility profile；它们使用 compatibility 绘制示范，不是 core profile renderer。只有嵌入式 `minimal_eda.exe --legacy` 明确选择旧式 WGL；模块不做隐式降级。旧的 `ui_surface_create()` 创建 OpenGL surface 时仍使用 legacy 路径，应用要请求现代配置应使用新 API。

legacy 配置要求 `legacy_context = 1`，其他 version/profile/sample/debug 字段全为 0。混合这两类设置返回 `UI_STATUS_INVALID_ARGUMENT`。`samples == 0` 禁用 MSAA，正值请求至少对应样本数；是否可用由驱动决定。

将 surface 绑定 `UI_LAYOUT_REGION_MAIN` 后，框架在 layout/DPI 变化时调整它；`UI_LAYOUT_REGION_NONE` 保留手动矩形。`ui_surface_get_rect()` 返回逻辑矩形，`ui_surface_get_pixel_rect()` 返回实际设备像素矩形；OpenGL viewport 必须使用后者的宽高。

`ui_surface_set_callbacks()` 注册 resize 和 frame callback。resize 同时提供逻辑矩形、framebuffer 矩形和 DPI，注册时会先回调一次当前尺寸。`ui_surface_invalidate()` 请求绘制；Win32 surface 通过绘制消息执行 frame callback，不提供固定帧率。EDA 模块在内容变化、布局和激活时按需绘制，后台保留 context 和数据；嵌入样例仍用应用定时器请求失效。frame callback 调用 make-current、绘制和 swap-buffers。

应用应在 0 宽/高时避免无意义的投影除法和重型绘制，在恢复后继续按新 framebuffer 绘制。EDA 用 viewport、文档坐标、保持比例的投影和 zoom 适配；画板应维护画布变换，PPT/Excel 应维护文档与视口关系。框架不会用 CSS 拉伸 OpenGL 内容。

`ui_surface_set_input_callback()` 为原生/OpenGL surface 注册输入回调。事件坐标是相对 surface 客户区的逻辑像素；鼠标左/右/中键的 `pointer_button` 分别为 1/2/3。`modifiers` 是位标志：CONTROL=1、SHIFT=2、ALT=4。按键 `key_code` 当前使用 Win32 virtual-key 值，`wheel_delta` 保留 Win32 wheel 增量，通常一个刻度为 120。框架在鼠标按下时处理焦点与 capture，并在释放/失焦时清理；应用仍负责工具状态和文档命中。

surface 使用 Unicode 窗口，从 `WM_CHAR`/`WM_SYSCHAR` 将提交的 UTF-16 文本转换为 `UI_INPUT_TEXT` 的 UTF-8，包括 surrogate pair。事件和 `text_utf8` 只在 input callback 内借用；需要保留时复制。当前没有公共 IME composition、触控或 pen 压感协议，应用不要把提交文本事件当成这些完整能力。[`tests/surface_input.c`](../tests/surface_input.c) 验证 native/OpenGL 输入回调路径。

`ui_opengl_surface_get_info()` 返回 vendor、renderer、版本、profile、实际 MSAA 和 debug/legacy 属性；查询会恢复此前 current context。扩展函数通过 `ui_opengl_surface_get_proc_address()` 获取，并在对应 context current 时调用。读取 renderer 再判断当前机器实际能力，不要把 legacy 创建成功当成硬件加速已证实。

[`tests/opengl_integration.c`](../tests/opengl_integration.c) 验证兼容路径、现代配置成功或严格 unsupported、不可实现版本拒绝、context 恢复、DPI 与最小化恢复。测试可能报告某些现代配置 `SKIP` 后整体通过，这表示拒绝行为正确，不能据此宣称该 GPU 支持那些配置。

## 8. 助手命令、权限和撤销

### 8.1 模型输出进入助手允许列表

应用先在 host 注册业务 command，再在 `ui_assistant_t` 注册模型可调用的语义 command、权限和参数 schema。未进入助手允许列表的 command 返回 `UI_STATUS_NOT_FOUND`。用 `ui_assistant_visit_commands()` 向模型工具适配器提供可用 ID、权限和 schema；visitor 内的描述借用到回调结束，不能在遍历时增添命令或销毁 assistant。

`params_schema_json` 供工具说明和应用读取，框架不实现 JSON Schema 校验。用 `validate` callback 检查 JSON 语法、字段类型、范围、文档对象 ID 和业务前提。返回不通过后不派发业务命令；`UI_STATUS_INVALID_ARGUMENT` 会转为 `UI_STATUS_VALIDATION_FAILED`。没有 validator 就没有自动参数校验。

独立宿主的全局助手通过 `ui_workspace_invoke()` 显式选择实例，再进入该实例的 `ui_assistant_invoke()`。助手 UI 的目标与当前活动标签是两个独立选择，不用标签切换改变已提交的调用。目录只展示目标实例允许的 ID、权限和 schema，结果标注原实例和请求 ID。宿主当前提供协议验证 UI，实际模型提供商、网络请求和模型工具适配器另行接入。

模型输出不得绕过这条路径进入 `ui_host_invoke()`、可信 Web 页面的 `ui.invoke()` 或任意 script execution。Web 页面中的 `ui.invoke()` 直接进入 host，它是可信应用 UI 的通路，不能作为模型权限边界。模型只使用对象 ID 和语义参数，不获得 HWND、屏幕坐标或任意 C 指针。

### 8.2 权限、确认和完成时序

权限分为 READ、EDIT、DESTRUCTIVE。`max_permission` 指无需提升确认的权限阈值；高于阈值的操作仍可能由 `confirm` callback 明确批准。DESTRUCTIVE 无论阈值如何都要求确认。缺少 confirm 或确认拒绝时返回 `UI_STATUS_PERMISSION_DENIED`，请求 ID 保持 0，不执行 handler。需要绝对禁止某类操作时，让应用的确认策略始终拒绝它。

确认界面应显示目标实例、实际 command 和已校验参数，并返回用户对这次请求的决定。workspace 合并宿主和应用的权限阈值，确认先由宿主决策；应用另有 confirm 时也须批准。确认回调同步执行；要设计异步确认界面，应先取得决定，再发起调用，不要在确认回调中销毁或重入正在派发的对象。

assistant 会在进入 host 同步 handler 前建立请求跟踪，因此 handler 内报告进度和同步 result callback 都能看到请求。workspace 在交付宿主 result callback 后自动调用 `ui_assistant_forget_request()`；独立应用无需再次清理。直接使用 assistant 的嵌入式应用须在结果回调中 forget 已完成记录。不要等 invoke 返回后才建立识别同步结果所需的应用状态。

### 8.3 取消、进度、快照和事务

`ui_assistant_cancel()` 标记取消并调用应用 cancel callback。它不终止线程，也不替应用撤销已执行修改。任务需定期调用 `ui_assistant_is_cancelled()`，在安全边界停止、释放自己的资源，向 host 回复失败/取消结果，最后忘记请求。进度范围为 0–100；取消后继续报告进度返回 `UI_STATUS_CANCELLED`。

snapshot callback 提供应用拥有的完整状态字符串；变化通过 host 的应用事件发布。大模型连接、上下文裁剪和增量同步由应用适配层实现。

begin、commit、rollback、undo API 只调用应用提供的事务 callback。它们不自动记录数据变化、校验事务嵌套、把一批命令变成原子操作，或生成撤销栈。应用必须保存文档状态/增量，确保 transaction ID 和 undo group 的语义一致。缺少相应 callback 返回 `UI_STATUS_UNSUPPORTED`。

全局助手可操作多个应用，但事务和撤销仍属于单个实例，没有跨应用原子提交。应用 A 的撤销 ID 不能交给应用 B。最小 EDA 模块演示快照、参数校验与危险清空确认，未实现文档持久化或撤销；有实际文档修改的应用应实现自己的事务数据。

[`tests/assistant_lifecycle.c`](../tests/assistant_lifecycle.c) 使用真实的简单文档模型，验证同步结果清理、commit 后 undo、修改后 rollback 和危险操作拒绝/批准。该测试中的数据保存由应用模型实现，可作为各类应用的最小接入参考；框架没有为 EDA/Excel 自动提供撤销。

## 9. 两种 Web 后端

### 9.1 统一 C 接入

两种后端都返回 `ui_web_backend_t *`，使用相同 `ui_web_view_create()`、`load_html()`、`resize()`、`dispatch_input()`、`invalidate()` 接口。第三方 DOM、JS context、COM interfaces 均不出现在公共接口中。自定义后端通过 `ui_web_backend_ops_t` 注册函数，create/destroy 必需，未提供的可选功能返回 `UI_STATUS_UNSUPPORTED`。

Web view 是独立的对象族；`ui_surface_create()` 的 `UI_SURFACE_WEB` 仍不直接创建页面，应用必须先选择 backend 再创建 view。`ui_web_view_set_rect()` 设置相对父客户区的逻辑位置、宽高和 DPI，`ui_web_view_resize()` 保持既有位置。`ui_web_view_set_layout_region()` 可将 view 绑定到 MAIN 或左右 SIDEBAR，使同一个 host 的原生/OpenGL 与 Web 区域共享布局；NONE 解除绑定并保留矩形。`ui_web_view_get_rect()` 返回当前逻辑矩形。两个内置后端均实现定位；旧的自定义后端未实现 set_rect 时只能支持原点处的 resize，非零位置返回 UNSUPPORTED。WebView2 的附加 `get_bounds()` 查询实际 controller 像素矩形，用于验收。创建并放置 view 后，要避免在同一区域重复显示 native 面板容器而遮挡它；面板业务与显示方式由应用选择。

两种后端都支持可信 HTML 中的 `ui.invoke(command, params)` 和 `ui.value(input_id)`。params 可使用 JSON 字符串，或先由 JavaScript 对象转换成 JSON。轻量后端同步返回 host request ID，WebView2 通过消息异步调用并返回 void；可移植页面不依赖这个返回值。业务结果进入 C host result callback，应用可用 JSON 消息将结果或事件回推给页面；框架不自动将每个 command 包装成 Promise。

API 2 使用 `ui_web_view_set_message_callback()` 接收页面的 `ui.postMessage(data)`，用 `ui_web_view_post_json()` 交付 C 侧 JSON。页面通过 `ui.onmessage = function(data) { ... }` 或 `window.addEventListener('message', function(event) { ... })` 接收数据。后端解析 JSON，不把内容拼接成脚本。消息参数在调用/回调期间借用，异步保存时自行复制；回调在 UI 线程执行，不得在其中销毁 view/backend/host，关闭应延迟到回调返回后。

`ui_web_view_get_capabilities()` 查询 `UI_WEB_CAP_JSON_MESSAGES`、`DYNAMIC_DOM`、`RESPONSIVE_LAYOUT`、`NATIVE_WINDOW`、`TEXT_INPUT` 位；API 3 增加 `IMAGES/WEB_TEXT_EDIT/COMPONENTS`。能力位表示接口类别可用，不表示完整 DOM/CSS 或所有输入形式。缺少回调的旧自定义后端返回 UNSUPPORTED。`ui_web_view_native_handle()` 返回借用呈现窗口或 NULL，不能销毁它；轻量 Windows 后端在有parent时返回HWND、NULL parent时返回NULL，当前 WebView2 适配器未提供此查询。完整示例见 [`examples/web_counter/app.c`](../examples/web_counter/app.c)。

TypeScript 需在构建阶段离线编译为 JavaScript，再交给后端执行。框架不直接执行 `.ts`，也不附带 Node.js 开发运行时。

### 9.2 轻量受控后端的实际子集

轻量后端将 Lexbor 与 QuickJS-NG 静态链接到所选框架目标，布局和 GDI 绘制由本项目的 C 实现完成。它为宿主外壳和应用工具页提供受控动态 UI，不能用“HTML/CSS/JS 支持”推断任意网站兼容性。

| 类别 | 已实现范围 |
| --- | --- |
| HTML 内容元素 | `div`、`header`、`footer`、`nav`、`aside`、`section`、`main`、`p`、`span`、`button`、文本 `input`、`textarea`、资源 ID `img` |
| 内容属性 | `id`、`class`、`style`、`onclick`、`onmousedown`、`oninput`、`value`、文本 `type`、`disabled`、`readonly` |
| 布局 | `display:block/flex/none`、row/column、整数 flex 权重、整数 px/百分比宽高、absolute 定位、padding/margin/gap、min/max尺寸和基本对齐 |
| 样式 | 十六进制颜色、solid 边框、圆角、字体大小/粗细、受限字体族、裁剪和滚动 |
| 样式表 | 标签/class/ID/后代选择器、hover/focus/disabled、有限优先级与 width min/max media 规则 |
| DOM 脚本 | `document.body`、`getElementById/createElement`、`appendChild/removeChild/remove`、`textContent/className/value/disabled/style`、受限属性访问、`focus()` |
| 事件/消息 | click/input/keydown/focus/blur/mouseenter/mouseleave/mousedown/wheel/contextmenu 监听、document keydown、`ui.invoke/value/postMessage/onmessage` 和 window message |
| 输入 | 逻辑像素命中、按钮/滚轮；Uniscribe 和 IMM32 平台适配的 Web 单行/多行编辑、选择、剪贴板及撤销，无 EDIT 代理 |
| 元素查询 | `ui_web_view_get_element_rect()`，隐藏元素为零矩形，缺失 ID 为 NOT_FOUND |

尚无完整 DOM、完整 CSS cascade/选择器、通用 Flexbox、Grid、任意媒体查询、Canvas/SVG、网络 fetch、外部 script/module 或浏览器定时器/Promise job 调度。CSS/DOM 的同名接口只覆盖表中子集，浏览器框架生成的结构仍可能超出允许列表。不支持的页面结构或样式返回 `UI_STATUS_UNSUPPORTED`，超出资源/格式约束返回 validation 错误，不能把错误当成静默忽略。

当前约束是：HTML 最多 256 KiB、最多 1024 个内部节点和 256 条 CSS 规则，ID 最多 95 UTF-8 字节，文本/value 最多 4095 字节，style/handler 属性最多 2047 字节，单个内联 script/style 文本最多 64 KiB；宽高不超过 32767 逻辑像素，DPI 为 1–768。QuickJS 每个 view 限制 8 MiB JS 内存、256 KiB stack，每次同步脚本约 50 ms 中断预算。这个预算不会抢占应用 C handler，长业务仍需异步实现。增量 DOM 修改保留未删除节点的输入/焦点/滚动状态；重新 load_html 会重建文档和 JS 状态。

`ui_light_web_backend_capabilities()` 的诊断字符串和能力位可辅助识别后端。详细限制见[轻量后端说明](../examples/light_web_probe/README.md)，动态行为对应 [`tests/light_web_dynamic.c`](../tests/light_web_dynamic.c)。旧版本/EDIT/class/193 节点断言保存于 [API 2 基线](../tests/api2_baseline/README.md)，运行测试已经按 API 3 和原生无代理编辑的新合同更新，原因及结果见[验收记录](validation/api3-validation.md)。

### 9.3 WebView2 后端

WebView2 提供由安装的 Runtime 决定的浏览器兼容性，框架适配器仍是 C。SDK 的 headers/loader 在 `.deps`，不等于 Runtime 已随应用打包；目前 config 没有指定 Fixed Version Runtime 的目录选项。部署必须单独解决 Runtime 安装/分发需求，不能宣称该选项已经满足“全部引擎随应用一起打包”。

创建是异步的，UI 线程必须持续 pump Win32 消息。`ui_webview2_view_get_state()` 分别查询 ready 和 navigation completion，ready 包括可信 UI bridge 的安装完成。`load_html()` 返回成功仅表示接受或发起加载，不证明 HTML 已完成导航。`ui_webview2_view_execute_script()` 通过 callback 提供 JSON 结果，`get_error()` 返回异步 HRESULT 诊断。

view 优先嵌入 `host.native_parent`，config 的 parent_window 是后备值。创建时选择正确父容器；创建后没有公共 reparent API。主内容区可使用实例 host；在面板内采用 WebView2 时先核对这个 parent 选择规则，不要假定 config 会覆盖 host 的非 NULL parent。DPI 调整同时更新 raw pixel bounds 和 rasterization scale，以保持逻辑 viewport。

当前仅加载可信应用 HTML；适配器限制主文档导航，并校验 WebMessage 来源后分发 semantic command。不要把任意下载页面或模型生成的 HTML 当成可信 UI。系统 child HWND 自动接收真实输入，所以通用 `dispatch_input()` 返回 `UI_STATUS_UNSUPPORTED`；通用同步 element rect 查询也未实现，可使用异步脚本查询 DOM。应用应检查这些能力差异，不依赖两个后端拥有相同内部行为。

[`examples/web_common/assistant.html`](../examples/web_common/assistant.html) 是两个后端的共用页面。对应 probes 比较 800/500 逻辑 viewport、96/144 DPI 的助手区域显示和宽度，并验证按钮/输入命令；这是已实现子集的行为对照，不是整个浏览器规范的一致性测试。

## 10. 最小 EDA 和其他应用的接入方法

[`examples/minimal_eda/app.c`](../examples/minimal_eda/app.c) 是独立宿主应用模块，构建为 `minimal_eda_app.dll` 并打包为 `minimal_eda.uapp`。CMake 使用冻结的 `tests/sdk_v1` 头文件将它编译为 API 1 兼容调用方，与其清单保持一致；新应用使用当前 `include/` 头文件。它在 create 注册 UI 和配置状态快照，在 mount 注册助手允许列表、创建 OpenGL 主区和属性内容；每实例分配自己的 blocks、zoom、context 和笔记窗口。清单声明允许多实例。

点击画布后，`A` 添加矩形、`Z` 放大、`C` 清空；菜单/工具栏复用同一命令。全局助手使用 `{}` 参数调用，validator 拒绝非空对象和无效 JSON；clear 是 DESTRUCTIVE，需要宿主确认。属性区显示 instance ID、矩形数量、zoom，EDIT 初始文本通过包内 `readme.txt` 资源读取。用户可修改笔记，再切换标签、浮动/停靠面板，检查实际子窗口和内容继续保留。

模块按需绘制，后台仍可通过助手更新模型，重新激活后呈现最新内容。它无 worker/timer/file persistence，因此 request_close 直接 ALLOW；unmount 移除面板 subclass、销毁自己的子窗口和 surface、释放字体，destroy 只释放状态，module_shutdown 注销共用窗口类。此顺序也能处理 mount 中途失败。示例不实现 netlist、布线、设计规则检查、文件格式、完整鼠标绘图或撤销，不把笔记作为持久化文档。

[`examples/minimal_eda/main.c`](../examples/minimal_eda/main.c) 仍是完整嵌入式 Win32/OpenGL 样例，由应用创建窗口并预留属性/助手容器。它独立启动，不展示多标签，显式 `--legacy` 可验证旧式 WGL；该开关不适用于 `framework_host.exe` 的 EDA 模块。

[`examples/web_counter/app.c`](../examples/web_counter/app.c) 使用冻结的 tests/sdk_v2 公共头文件构建 API 2/ABI 1 DLL，[清单](../examples/web_counter/manifest.ini)声明 API 2。它在 mount 获取主内容槽，创建轻量 backend/view 并设置消息回调，页面发送 increment 数据，C 侧调用业务命令并用 JSON 回推 count。业务数据、助手快照和多实例状态仍由 C 应用维护，unmount 先销毁 view 再销毁 backend。该样例不创建 OpenGL context，可用于验证无 OpenGL 的 Web 内容路径。

Markdown 阅读器可以把解析结果交给自己选择的绘制或 Web 路径；轻量后端当前不支持完整 Markdown HTML 排版。画板将画布作为 OpenGL 文档视口，工具栏命令改变应用工具状态。自研 PPT/Excel 将幻灯片/工作表数据、编辑、布局和撤销留在应用，框架只提供外壳、视口和命令通路。

跨应用复用应首先统一稳定命令 ID、参数验证、结果 JSON 和文档状态事件，再根据实际显示需求选择 native/OpenGL 或 Web 后端。

### 10.3 API 3 通用组件

[generic_components/app.c](../examples/generic_components/app.c) 和[说明](../examples/generic_components/README.md)展示公共接口注册菜单、工具、按需树、100000×16 表格、属性、Web 对话框、异步 RGBA 缩略图、包内 PNG、填充样式及 OpenGL 内容槽。应用不含组件 HTML；独立实例拥有数据、草稿、请求及资源。关闭先停止并 join 后台线程，再销毁绘图内容，宿主统一回收组件与图片。

组件协议、字段所有权、分页、缓存预算和未实测条件完整定义于[通用 Web UI](generic-web-ui.md)。该示例仅有测试数据和有限编辑记录，不构成任何具体应用的业务模型。

## 11. 常见错误

| 表现 | 需要检查的接入错误 |
| --- | --- |
| 包打开后没有标签 | 检查 `[application]`、DLL入口/依赖、架构与ABI、module名和包内资源；读取 workspace last_error |
| 两个标签修改同一份数据 | 应用业务状态应由 create 每实例分配；检查全局/静态 model、callback user_data |
| 切换标签后画布/笔记丢失 | 不应 refresh shell 或重新 mount；保留 native窗口和GLcontext，用 set_active调整绘制 |
| 错误标签收到助手结果 | 同时保存 instance_id/request_id；禁止仅使用当前活动标签路由 |
| 关闭后DLL无法卸载 | 检查后台线程、timer、外部callback、subclass和自有窗口类；module_shutdown必须完成清理 |
| 关闭等待永不结束 | worker停止后是否 post_close_complete ALLOW/REFUSE；宿主是否 poll队列 |
| DPI 切换后控件错位/模糊 | 初始化是否早于首个 HWND；是否采用正确 DPI 模式；是否转发 DPI 消息并应用 suggested RECT |
| 主区缩放比例错误 | 是否把 framebuffer 像素用于 glViewport，并独立维护文档投影/缩放 |
| 窄窗口没有可点击标签 | 检查 collapsed_tag_width 和布局；嵌入式 native shell 是否 reflow，Web 宿主是否刷新状态 |
| 菜单存在但命令不执行 | command 注册顺序和 ID；嵌入式是否转发 WM_COMMAND，Web 内容是否正确接入命令/消息 |
| 面板 refresh 后内容消失 | 旧 ui_native_shell_refresh 会重建内容；新应用可用 ui_shell_refresh 保留既有容器 |
| 模型绕过权限调用命令 | 模型输出必须进入 assistant_invoke，而非 host_invoke 或 UI JS bridge |
| 同步结果没有跟踪记录 | 在 dispatch 前准备应用上下文；使用助手已有的预跟踪，再在结果回调 forget |
| cancel 后数据仍已修改 | cancel 是合作协议；应用必须在安全点停止并执行自己的 rollback |
| WebView2 创建成功但没页面 | pump 消息循环，等待 navigation completion，检查 HRESULT；不要只看 create/load 返回值 |
| 轻量页面加载返回 unsupported | 检查元素/属性/CSS 是否超出固定子集，不能按完整浏览器编写页面 |

## 12. 应用验收清单

1. 用 C11 或 C++ 编译公共接口调用方，所有描述清零并设置正确 size/version；应用 DLL 使用宿主共享框架，入口导出无名称修饰。
2. 验证正常、create失败、mount失败的清理；ALLOW后无异步访问，WAIT可完成或拒绝，最后实例可卸载DLL并重新加载。
3. 在 1920×1080、1280×720、800×600、640×480 逻辑客户区及 96/144/192 DPI 下查询矩形；确认内容不越界且主区策略符合应用选择。
4. 检查折叠、标签打开、浮动、回到停靠和禁止缩小；重新布局和 ui_shell_refresh 保留内容，旧 ui_native_shell_refresh 后正确重建。
5. 在真实不同 DPI 显示器间移动窗口，检查 suggested RECT、字体、工具栏和 framebuffer，再检查最小化/恢复。
6. 检查请求成功/失败、重复回复、同步和异步完成；复制需要异步保存的借用参数。
7. 检查助手允许列表、业务参数校验、权限拒绝/批准、进度、合作取消、状态快照和应用实际 rollback/undo。
8. 启用 Web 后端时验证共用页面，并单独测试目标页面的能力差异；Runtime 缺失、脚本超限和 unsupported 不能被误报为成功。
9. 使用 GPU 信息明确记录现代配置实际支持情况；测试通过和 hardware acceleration 支持是两个需要分别验证的结论。
10. 从菜单/命令行加载包，检查损坏包、非法路径、错误架构/ABI、版本冲突；单实例重复打开只激活已有实例。
11. 两个实例注册同名命令后分别修改，切换保持画布、GLcontext和属性内容，关闭后台实例不改变前台菜单。
12. 助手跨实例调用，切换或关闭标签后结果/进度仍按原ID交付或丢弃，危险确认显示正确目标；等待关闭时拒绝新命令。
13. 应用自有 WndProc/subclass/COM UI callback 的 scope 成对覆盖所有分支和默认窗口过程；嵌套模态消息中关闭后，DLL 仍保留到完整回调退出。确认后台线程不使用 scope，也没有从 query/DllMain/静态构造启动任务。
14. 用未重新构建的 v0.1.0 包与冻结 SDK2 构建包验证兼容，用 API3 通用示例验证新能力；检查清单/DLL API 不一致及不支持版本被拒绝。
15. 检查 Web JSON 的 UTF-8/转义/非法数据、能力差异、内容槽借用与关闭顺序；浅色/深色、窄窗和浮动面板的内容保持可用。

将该清单与应用自身的数据和文件操作测试一起执行，再把应用交给用户使用。
