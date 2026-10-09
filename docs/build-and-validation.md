# 构建与验证

本项目的实际 UI 目标是 Windows x64。当前 SDK 为 0.9.0 开发源码，最近稳定标签仍是 `v0.1.0`。Windows 默认构建 Web 独立宿主 `framework_host.exe`、共享运行库 `ui_framework.dll`、打包器、以当前SDK/API9构建的EDA、Web Counter、generic_components和framework_features功能应用包；同时保留 C11 静态框架、原生嵌入式 EDA 和核心/Win32 测试。默认轻量 Web 引擎需要另行准备固定依赖，WebView2 默认关闭。应用 API/ABI、包格式和生命周期见 [应用开发标准](application-development-standard.md)，当前升级见[0.8→0.9迁移](migration-v0.8-to-v0.9.md)；更早版本按对应历史指南迁移。

API5已有能力见[菜单与离屏](framework-menu-offscreen.md)、[通用Web](generic-web-ui.md)及[历史验收](validation/api5-validation.md)。当前API9接口迁移见[0.8→0.9](migration-v0.8-to-v0.9.md)、[布局](workspace-layout.md)及[当前API9验收](validation/instance-language-validation.md)。历史验收节和冻结源码保留原结果，不作为当前测试状态或未来兼容承诺。当前维护范围见[版本政策](version-policy.md)，本轮结果见[实例语言验收](validation/instance-language-validation.md)。

最新实例语言源码复核和本地SDK交付见[2026-10-09记录](validation/instance-language-current-validation.md)，API9首次实现验收保持原版本/失败/环境。使用同一开发快照的头文件、导入库、DLL、宿主和.uapp；先核对归档及逐文件哈希，再执行解包产物。本机结果不能覆盖b6a36fee的托管失败或替代当前源码Session0/严格无登录/物理条件。

## 1. 构建环境

使用安装了 MSVC C/C++ 工具、Windows SDK、CMake 和 Ninja 的 Visual Studio Developer PowerShell，选择 x64 工具链，并进入仓库根目录。WebView2 loader 根据目标架构选择；本轮实际 UI、GPU 和 Runtime 验证覆盖 Windows x64，历史panel兼容编译结果仅属于历史记录，本轮不列为交付门槛。

CMake 项目最低版本为 3.20。C11 用于框架源码；安装了 C++ 编译器时，测试配置会额外构建 C++ 公共头文件调用方。C++ 测试被配置出来不表示框架源代码改成了 C++。

本项目恢复 CMake 误解码的中文 MSVC `showIncludes` 前缀，确保 Ninja 能跟踪公共头文件依赖；旧工作区发生过依赖跟踪问题，迁移验证应使用新构建目录，确认公共头文件变化确实重编译。下文命令使用 `build/web-shell` 与 `build/native`，历史记录中的目录只用于标识当时的本地证据。

## 2. 默认 Web 宿主构建

Windows 默认 `UI_BUILD_STANDALONE_HOST=ON`，轻量 Web 开关随宿主默认开启；`UI_FRAMEWORK_ENABLE_WEBVIEW2=OFF`。先按第3节准备 Lexbor/QuickJS-NG。独立宿主只提供 Web 外壳，不能仅关闭轻量 Web 而保持宿主开启，否则 CMake 明确报错。

```powershell
cmake -S . -B build/web-shell -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/web-shell
ctest --test-dir build/web-shell --output-on-failure
```

运行独立宿主：

```powershell
& .\build\web-shell\framework_host.exe
```

空启动后点击“打开应用”，选择 `build/web-shell/web_counter.uapp` 验证纯 Web 内容，无需 OpenGL；也可选择 `minimal_eda.uapp` 验证原生/OpenGL 内容。四个示例都允许多实例。API3 优先打开 generic_components.uapp，组件 UI 不依赖原生业务控件。直接启动多个包：

```powershell
& .\build\web-shell\framework_host.exe .\build\web-shell\web_counter.uapp .\build\web-shell\minimal_eda.uapp
```

Web 外壳提供打开/关闭入口，应用菜单和工具入口跟随当前标签，支持浅色/深色切换；顶层采用融合标题区的浏览器式标签栏，下方为活动应用菜单；AI位于窗口按钮左侧，主题/布局重置位于第二行右端宿主选项。最近历史与当前标签分开，详见[宿主说明](standalone-host.md)。`Ctrl+Shift+O` 打开应用包、`Ctrl+W` 关闭、`Ctrl+Tab` / `Ctrl+Shift+Tab` 切换标签。全局助手默认收起，点击AI展开后选择目标实例和可读操作，基本参数可在普通视图编辑；高级区保留schema、原始JSON、快照、事务及详细日志。输入 `{}` 可调用 EDA 命令并观察结果；`eda.clear` 请求确认。事务按钮调用应用的 begin/commit/rollback/undo callback，EDA 未提供时返回 UNSUPPORTED。这个区域用于协议验证，尚未连接模型服务。

宿主日志存储上限为 32000 字节，日志/schema 的显示预览按 UTF-8 边界限制到 4095 字节。危险操作确认参数采用完整分块显示和滚动，不能用被截断的日志预览代替确认内容。应用自身仍负责参数 schema 的业务校验。

点击 EDA 画布后用 `A` 添加 block、`Z` 放大、`C` 清空。修改属性区笔记后切换标签或浮动/停靠，文字与画布数据继续保留；两实例互不影响。关闭最后一个实例后模块卸载，再次打开重新初始化状态。模块要求 OpenGL 3.3 compatibility，不隐式降级。

也可运行保留的嵌入式样例：

```powershell
& .\build\web-shell\minimal_eda.exe
```

嵌入式样例默认请求 OpenGL 3.3 compatibility profile。若驱动不支持请求配置，样例会提示失败；需要验证旧式 WGL 路径时明确指定：

```powershell
& .\build\web-shell\minimal_eda.exe --legacy
```

`--legacy` 是嵌入式 EXE 单独选择的兼容路径，不适用于独立宿主加载的 EDA 模块。两个样例验证外壳、绘制和生命周期，尚未实现完整 EDA 编辑功能或模型服务。

### 2.1 打包自己的应用

应用模块使用 [`application.h`](../include/ui_framework/application.h)，导出 `ui_app_query_v1`，链接共享框架的 `ui_framework_runtime.lib`，并用 `UI_FRAMEWORK_BUILD_SHARED` 编译；不要链接供嵌入式应用使用的静态 `ui_framework.lib`。CMake 模块目标链接 `ui_framework_shared`，公共 include 路径与 shared 定义由该目标传递。当前头文件的 API 宏是 8；使用当前接口的应用将清单与 DLL descriptor 一致声明为8，历史SDK/API和原包不再属于维护及专项回归范围。清单存于 staging 目录外，staging 只放 module 和资源。构建自动生成 `build/web-shell/eda_package`，内容为：

```text
minimal_eda_app.dll
readme.txt
```

打包器接口是 `uapp_pack.exe manifest.ini source-directory output.uapp`，所有输入 Windows 路径均转为 UTF-8 接入 C API。输出必须在 source-directory 之外。包文件不是 ZIP，版本1未压缩，不能用压缩软件重新保存；容器编码和清单限制见标准第2节。

使用构建好的 EDA staging 重新打包到另一文件：

```powershell
& .\build\web-shell\uapp_pack.exe .\examples\minimal_eda\manifest.ini .\build\web-shell\eda_package .\build\web-shell\minimal_eda-copy.uapp
```

宿主分发目录需要包含 `framework_host.exe`、匹配的 `ui_framework.dll`、应用包和工具链运行依赖。应用依赖的其他 DLL 可以作为包文件，但不能打包替换 `ui_framework.dll`。宿主使用绝对路径加载提取后的 module，同一进程中共享框架对象。

默认 Web 宿主配置中，Lexbor/QuickJS-NG 静态链接到框架，`ui_framework_shared` 使用 MSVC `/MD`，匹配第三方库的动态 CRT 配置；宿主和应用模块使用 `/MT`。共享运行库仍有动态 CRT 部署需求。关闭两种 Web 后端时共享运行库与 EDA 模块使用 `/MT`；原静态目标、旧样例和应用自建 target 以各自设置为准。每个模块的内存都通过提供者的 release 接口释放，不能跨 CRT 交叉 malloc/free。发布时检查实际 DLL 依赖，不以引擎静态链接推断没有运行依赖。

“轻量”应以目标 Release 可执行文件及其需要分发的 DLL/资源总量衡量。QuickJS 的 8 MiB 是 JS runtime 资源上限，不是整个程序、引擎或安装包的大小；当前没有发布固定体积保证。

### 2.2 不含 Web 引擎的原生嵌入式构建

只需要原生/OpenGL 嵌入式样例、静态/共享框架、打包器及核心/Win32 测试时，同时关闭宿主和轻量 Web；为明确配置，也关闭 WebView2。这条路径不需要 `.deps/`，不生成 `framework_host.exe`：

```powershell
cmake -S . -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_BUILD_STANDALONE_HOST=OFF -DUI_FRAMEWORK_ENABLE_LIGHT_WEB=OFF -DUI_FRAMEWORK_ENABLE_WEBVIEW2=OFF
cmake --build build/native
ctest --test-dir build/native --output-on-failure
& .\build\native\minimal_eda.exe
```

使用新目录避免旧 CMake cache 保留轻量后端为 ON。应用自建 workspace 的零初始化 `shell_mode` 继续选择原生嵌入式外壳；这是保留的 API 使用方式，不是独立宿主的原生模式开关。

## 3. 启用两种 Web 后端

Git 仓库不提交 `.deps/` 和 `build/`，也不会自动下载依赖。新 clone 在 Windows 上的默认宿主构建需要 Lexbor 和 QuickJS-NG；WebView2 只在显式开启时需要对应 SDK 和 Runtime。按以下固定版本另行准备：

| 依赖 | 路径和固定版本 | 用途 |
| --- | --- | --- |
| [Lexbor](https://github.com/lexbor/lexbor) | `.deps/lexbor`，v2.5.0，commit `7fb22cf5664a331d7c24b113489e566767c9c25a` | 受控 HTML 解析 |
| [QuickJS-NG](https://github.com/quickjs-ng/quickjs) | `.deps/quickjs`，commit `2f0aa72a6b09cf69ff06399bcf9f4c083ac1a278` | 受控 JavaScript 执行 |
| [WebView2 SDK](https://www.nuget.org/packages/Microsoft.Web.WebView2/1.0.4129.50) | `.deps/Microsoft.Web.WebView2.1.0.4129.50` | COM headers 和 x64 static loader |

这些目录缺失时，可选配置会给出构建错误，不会自动联网下载安装。轻量后端可用 `UI_LEXBOR_SOURCE_DIR` 和 `UI_QUICKJS_SOURCE_DIR` 指向已有源目录；WebView2 SDK 路径目前在 [`cmake/webview2.cmake`](../cmake/webview2.cmake) 中固定。

在仓库根目录的 PowerShell 中执行需要的依赖命令。以下适用于首次准备，已有目录时先核对版本，不覆盖本地修改；只启用一个后端时，只准备它需要的依赖。

```powershell
New-Item -ItemType Directory -Path .deps -Force | Out-Null

git clone --no-checkout https://github.com/lexbor/lexbor.git .deps/lexbor
git -C .deps/lexbor checkout --detach 7fb22cf5664a331d7c24b113489e566767c9c25a

git clone --no-checkout https://github.com/quickjs-ng/quickjs.git .deps/quickjs
git -C .deps/quickjs checkout --detach 2f0aa72a6b09cf69ff06399bcf9f4c083ac1a278
```

WebView2 从官方 NuGet 下载固定版包。`.nupkg` 使用 ZIP 容器，以下保存为 `.zip` 后解压；不要在目标目录内再多套一层同名目录。

```powershell
New-Item -ItemType Directory -Path .deps -Force | Out-Null
Invoke-WebRequest -Uri https://api.nuget.org/v3-flatcontainer/microsoft.web.webview2/1.0.4129.50/microsoft.web.webview2.1.0.4129.50.nupkg -OutFile .deps/Microsoft.Web.WebView2.1.0.4129.50.zip
Expand-Archive -LiteralPath .deps/Microsoft.Web.WebView2.1.0.4129.50.zip -DestinationPath .deps/Microsoft.Web.WebView2.1.0.4129.50
```

准备后核对两份源码的 HEAD 与表中 commit 一致，并检查 SDK 的 headers/loader 路径存在：

```powershell
git -C .deps/lexbor rev-parse HEAD
git -C .deps/quickjs rev-parse HEAD
Test-Path .deps/Microsoft.Web.WebView2.1.0.4129.50/build/native/include/WebView2.h
Test-Path .deps/Microsoft.Web.WebView2.1.0.4129.50/build/native/x64/WebView2LoaderStatic.lib
```

WebView2 SDK 不含浏览器 Runtime。启用该后端还需要安装 Runtime，并按 [Microsoft Runtime 分发说明](https://learn.microsoft.com/en-us/microsoft-edge/webview2/concepts/distribution) 处理部署；本项目不会自动安装 Runtime。

在已准备好依赖的工作区运行：

```powershell
cmake -S . -B build/windows -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_FRAMEWORK_ENABLE_LIGHT_WEB=ON -DUI_FRAMEWORK_ENABLE_WEBVIEW2=ON -DUI_BUILD_TESTS=ON
cmake --build build/windows
ctest --test-dir build/windows --output-on-failure
```

只启用轻量后端时保持 WebView2 为 OFF。只启用 WebView2 内容后端且不需要宿主时，还须设置 `UI_BUILD_STANDALONE_HOST=OFF` 与 `UI_FRAMEWORK_ENABLE_LIGHT_WEB=OFF`。`UI_BUILD_TESTS=OFF` 关闭测试目标，样例和已启用 probes 仍按 CMake 配置构建。

Lexbor 和 QuickJS-NG 构建为静态库并链接所选框架目标；它们是随框架分发轻量引擎的路径。两个选项同时作用于静态框架和宿主的 shared target，应用仍通过同一 C ABI 选择后端，不把引擎再放进每个 `.uapp`。WebView2 SDK 包不包含浏览器 Runtime，当前适配器使用 Windows 上已安装的 Runtime。未实现 Fixed Version Runtime 目录配置，因此不能将 SDK 与 loader 的存在当成整个浏览器引擎已打包。

启用 Web 后端后，CMake 将共用页面 [`examples/web_common/assistant.html`](../examples/web_common/assistant.html) 复制为构建目录中的 `web_parity.html`。CTest 会在正确构建目录运行 probes。手动运行也应先切换到构建目录，避免误读相对资源路径：

```powershell
Push-Location build/windows
.\light_web_probe.exe
.\webview2_probe.exe
Pop-Location
```

## 4. 自动验收目标

Windows 的 CTest 目标由宿主、后端和 C++ 编译器配置决定；选项关闭或编译器缺失时，相应目标不会出现。用 `ctest -N` 检查当前列表，不能把目标存在当成已经通过。

| CTest 名称 | 验证范围 |
| --- | --- |
| `ui_package_format` | 独立编码的UAPP v1样本、打包/读取、截断/偏移/重叠/路径/名称冲突、UTF-8、架构/ABI与格式版本 |
| `ui_native_activation` | managed shell菜单所有权、前台/后台切换、panel/child保留、浮窗隐藏恢复、后台销毁不覆盖前台菜单 |
| `ui_workspace_integration` | 实际包/DLL加载和资源、单/多实例与同名命令隔离、关闭拒绝/等待/完成、异步复制投递/取消/进度、迟到消息丢弃、清理和卸载重载、C++模块ABI、EDA/DPI/布局与回调在途保护 |
| `ui_web_host_frontend` | Web 宿主空启动/加载、多标签、菜单/工具入口、助手/确认/事务、浅色/深色、尺寸/DPI、内容容器/GL 保留和卸载；需独立宿主，C++ fixture 可用时追加覆盖 |
| `ui_application_contract` | 当前API9/非法版本拒绝、当前包加载、多实例、Web及原生/OpenGL内容、清单/DLL API不一致拒绝；不指定历史原包 |
| `ui_web_shell_slots` | 借用内容槽、坐标、surface 绑定、增量刷新、停靠/浮动/DPI 及原生兼容容器 |
| `ui_floating_web_dpi` | 浮动 Web 内容跟随自身窗口 DPI；主 host 的 DPI/布局更新不覆盖浮窗局部尺寸和像素矩形 |
| `ui_light_web_dynamic` | 受控 DOM/CSS、动态按钮、JSON 转义、输入/焦点/滚动保留、重复增量更新、能力位和响应式/DPI |
| `ui_responsive_layout` | 无窗口的 4 个逻辑分辨率 × 3 个 DPI；布局关系、折叠/浮动/保留策略、旧描述尺寸和非法参数 |
| `ui_dpi_web_failure` | Web 后端 DPI 同步失败上报、部分状态更新与相同 DPI 的重试 |
| `ui_assistant_lifecycle` | 简单文档模型、同步请求预跟踪、结果清理、实际 commit/rollback/undo、危险操作确认 |
| `ui_headless_example` | 无窗口 EDA 命令、结果/事件、注册和布局调用 |
| `ui_public_headers_cpp` | C++ 包含公共头文件，链接 C 核心并创建/销毁 host |
| `ui_public_headers_c` | C11 同时包含全部公共头文件，链接 C 核心并创建/销毁 host |
| `ui_webview2_parity` | 实际 Runtime 的异步导航/JS结果、共用页面 800/500 viewport × 96/144 DPI、按钮/输入语义命令和结果 |
| `ui_webview2_messages` | 实际 Runtime 的公共能力位、C→JS→C JSON 双向桥接、中文/引号/反斜杠/换行的数据转义 |
| `ui_light_web_parity` | 共用页面布局/命令、Win32 child/EDIT、滚动和裁剪、JS状态与资源约束；保留的 class/193节点拒绝断言来自旧子集，当前扩展后需单独解释失败 |
| `ui_dpi_integration` | 真实 HWND/child rect，96/144/192 DPI，手动和区域绑定 surface，模拟 resize/DPI/最小化恢复 |
| `ui_opengl_integration` | legacy context、现代请求成功或明确 unsupported、version/profile/MSAA/debug、context 恢复、DPI 后 viewport/clear/swap |
| `ui_surface_input` | native/OpenGL 输入、Unicode文本、逻辑坐标、按键/修饰键、capture和输入callback注销 |
| `ui_monitor_transition` | 真实跨显示器移动与 OS DPI 通知；缺少不同 DPI 的多屏条件时返回 77 |
| `ui_native_responsive` | 左右停靠、标签打开/返回、浮动容器内容保留、字体与独立浮窗DPI、刷新和最小窗口策略 |
| `ui_native_shell_integration` | 实际分层 HMENU、toolbar、panel 容器、命令派发、DPI/reflow/refresh 和菜单恢复 |
| `ui_assistant_protocol` | schema 校验回调、允许列表/权限、进度、合作取消、snapshot 和事务 callback 路径 |

查看当前配置的测试清单或只运行一类验证：

```powershell
ctest --test-dir build/windows -N
ctest --test-dir build/windows --output-on-failure -R ui_responsive_layout
ctest --test-dir build/windows --output-on-failure -R ui_webview2_parity
```

所有返回值都应按实际意义判断。`UI_STATUS_UNSUPPORTED` 表示请求能力不可用；它不能被应用当成已创建页面或已获得现代 GPU context。

## 5. 结果解读和验证记录

### 5.1 0.2.0 开发版本迁移验收

2026-10-02，完成本轮构建、兼容性和实际界面验证。源码以 `v0.1.0` 基准 commit `b89438ab8127c419fde152b5e794bbe509a45de4` 为起点，`main` 提供本轮 0.2.0 开发更新，尚未创建或发布稳定标签。使用时通过 `git rev-parse HEAD` 记录实际源码 commit。环境为 Windows x64、MSVC 19.50.35727.0、CMake/Ninja、Intel UHD Graphics 770、单台 1920×1080/96 DPI 显示器；依赖固定版本见第 3 节。

从源码清单导出不含 `.deps/`、既有构建或工作记录的干净目录，默认构建通过外部路径使用固定 Lexbor/QuickJS 源码；native-only 构建关闭三个 Web/宿主开关，不需要引擎源码。两种配置均构建成功。**原始 CTest 尚未全绿，结果如下，不将旧断言失败改记为通过。**

| 配置 | 通过 | 失败 | 跳过 | 命令结果 |
| --- | --- | --- | --- | --- |
| 干净默认 Web 宿主，LIGHT=ON、WEBVIEW2=OFF | 19 | 2 | 1 | CTest 非零 |
| 干净 native-only，HOST/LIGHT/WEBVIEW2=OFF | 14 | 1 | 1 | CTest 非零 |
| 本地 Release，同时启用 LIGHT 与 WEBVIEW2 | 21 | 2 | 1 | CTest 非零 |

失败边界已逐项核查，原有 18 个测试/探针源文件 hash 均保持不变：

- `ui_package_format` 中 API 2 包应被拒绝的旧断言现在失败，连带同一路径的空 package/非空 error 检查，共 3 条检查。当前运行库明确接受 API 1、2；新的 `ui_application_versions` 验证 0、3 拒绝和清单/DLL 不一致拒绝，其余包校验仍通过。
- `ui_light_web_parity` 仅旧 `class` 拒绝及 193 个节点拒绝两条检查失败。当前受控后端支持 class，节点上限为 1024；其余原 probe 的 3 次 click、2 次 input、4 次 reload 及错误检查通过。
- `ui_monitor_transition` 因缺少不同 DPI 的多屏硬件而跳过。模拟 DPI 与布局矩阵不代替真实跨显示器验收。

新增的 API 版本、动态 DOM、内容槽、独立浮窗 Web DPI、完整 Web 宿主前端及 WebView2 JSON 消息共 6 项测试全部通过。完整前端实际经过页面 JS/C 桥接执行标签、菜单、工具栏、后台助手目标、确认拒绝/允许、事务/撤销、多行/长日志、最小化恢复及 4 组窗口尺寸 × 3 个 DPI 检查；保留 HWND 和 GL context。独立探针还验证 400 节点扩容/回收、复用节点的 DOM 绘制和点击顺序、事件节点移除及原生 CRLF/DOM LF 转换。

`ui_application_versions` 使用升级前保存、未重建的 v0.1.0 EDA 包，SHA256 为 `86988546516e17b4783c5a9ed1583dfaa9e73ecdfcd1f5b8fad9a4910a5a3503`。两个旧 EDA 实例与 API 2 Web Counter 混合加载、状态隔离、切换和关闭通过；该证据不自动覆盖所有第三方旧包。`tests/sdk_v1` 的冻结头文件另外验证 API 1 EDA、生命周期及 C++ fixture 的编译/链接路径；当前全部公共头文件含 `shell.h` 的 C++ 编译和 API 2 运行探针也通过。

正常桌面的浅色/深色截图已检查，Web 外壳与 EDA 网格及矩形的 OpenGL 画布合成正常。受限桌面的全黑截图不能作为视觉证据；WebView2 在受限执行环境初始化超时，普通 Windows 环境的原 parity 和新增 JSON 测试均通过。旧原生 `host_frontend` 用冻结 v0.1.0 EXE 配合新 DLL 另行运行，只有第 5.2 节已有 OS 最大窗口高度限制导致的 1 条尺寸失败，其余原生交互通过。

默认 Release 文件大小为宿主 177,664 字节、运行库 1,886,720 字节，合计约 1.97 MiB，不含应用包、系统库或 MSVC CRT。实际隐藏宿主单次观察：空宿主 working set 约 14.1 MiB/private 4.1 MiB，两个 Web Counter 约 16.3/6.8 MiB，两个 EDA 约 68.8/46.1 MiB；EDA 包含驱动和 GL context 成本。启动到第一次 `WaitForInputIdle` 返回分别约 93/108/171 ms，不表示所有画布首帧完成，也不是冷启动或跨机器性能保证。

本地日志、原包、探针、截图及测量 JSON 保存在忽略的 `build/web-shell/` 和 `build/review/`，不随 Git 源码提交。可用 `UI_LEGACY_EDA_PACKAGE` 指定自己的原包，步骤见[迁移指南](migration-v0.1-to-v0.2.md#6-验证升级)。本轮完成实现验证，但没有宣称原测试全绿、物理多屏已验证或 0.2.0 已稳定发布。

### 5.2 v0.1.0 干净源码验收

2026-10-02，从 Git 暂存清单导出干净源码，不包含 `.deps/`、既有构建目录或本地工作记录。Windows x64 Release 原生配置完成全部 66 个构建步骤，生成宿主、共享运行库、C/C++ 应用模块和 `.uapp`；CTest 为 **15 通过、1 失败、1 跳过**。原测试源码和断言保持不变。

失败的是 `ui_host_frontend` 的 1920×1080 实际客户区尺寸断言。当前屏幕为 1920×1080、96 DPI，Windows 最大窗口跟踪尺寸为 1940×1100；客户区加菜单/标题栏请求的外框为 1936×1139，系统限制后实际外框为 1936×1100，客户区为 1920×1041。没有链接框架、仅使用 `DefWindowProcW` 的 Win32 对照窗口得到同样结果，确认为本机 OS 窗口尺寸限制。1280×720、800×600、640×480 三组客户区尺寸均准确匹配，其他框架集成测试通过。

该失败保留为 FAIL，不改为 SKIP 或报告全通过。需在最大窗口跟踪高度能够容纳请求外框的机器上再次运行完整矩阵。跳过的 `ui_monitor_transition` 仍因只有一台显示器；本次没有重新下载或重建可选 Web 依赖。

本地证据为 `build/publish-clean-ctest.log`、`build/publish-frontend-host.log`、`build/publish-size-diagnostic.log` 和 `build/publish-size-control.log`，不随 Git 源码提交。新 clone 的测试结果以目标机器实际输出为准。

### 5.3 历史独立宿主验收

2026-10-01，独立宿主在当时本机工作区完成最终验收，所有原有测试文件的 SHA256 保持不变。这是已有验证记录，尚不代表新 clone 或本次 Git 发布后的构建已通过；以下日志为本地文件，不随源码提交：

| 配置与日志 | 通过 | 跳过 | 失败 |
| --- | --- | --- | --- |
| Windows x64 Release，默认原生路径；本地日志 `build/standalone-ctest.log` | 16 | 1 | 0 |
| Windows x64 Release，轻量 Web 与 WebView2 同时启用；本地日志 `build/standalone-web-final.log` | 18 | 1 | 0 |

两组分别为 17/19 项，唯一跳过均是 `ui_monitor_transition`：当前只有一台显示器。可选配置在允许 WebView2 Runtime 启动子进程的执行环境中完成导航、脚本和命令测试。自动 DPI 矩阵与模拟消息仍不能代替真实跨显示器验收。

当时本机可直接运行的默认输出是 `build/standalone/framework_host.exe`，同目录包含 `ui_framework.dll` 和 `minimal_eda.uapp`；Git clone 不包含这些二进制，需按第2节自行构建。新增包、managed shell、workspace 和宿主前端测试通过；覆盖的窗口/协议行为以测试表和日志为准，不能据此宣称已经接入模型服务或支持任意浏览器页面。

### 5.4 原有 Web/OpenGL 基线

2026-10-01，新增独立宿主前的 Windows x64 Release、两个 Web 后端启用的基线 CTest：15 项中 **14 通过、1 跳过、0 失败**。跳过的是实际跨显示器测试：当前环境仅有一个显示器。WebView2 在允许 Runtime 启动子进程的环境中完成实际导航、脚本和命令验证。公共头文件的 C11/C++ 调用方均通过；框架平台与后端源码另通过 MSVC C11 `/W4 /WX` 检查。这个基线不证明新增应用加载和多标签能力，后者以本次对应测试为准。

WebView2 在本工作区的真实 Runtime 验证得到：ready 和 NavigationCompleted 均为 1，四组 viewport/DPI 的 JavaScript结果为 `[800,240,false]`、`[500,0,true]`、`[800,240,false]`、`[500,0,true]`，表示宽窗口的助手区域宽 240，窄窗口隐藏。共用页面 button 和 input 各派发一个语义命令，C result callback 收到两个成功结果，异步 HRESULT 为 0。

两个 probes 还验证非零 view 位置、resize 保留位置，以及绑定 host 主区/侧栏后的自动重排和命令路由。WebView2 查询实际 controller bounds；轻量后端检查真实 child HWND 的客户区位置。Release 轻量 probe 为约 1.69 MiB，静态包含 Lexbor/QuickJS-NG；这是当前测试程序大小，尚不包括 MSVC CRT 部署、应用资源或应用业务代码。

受限执行沙箱中的 WebView2 曾在 15 秒内没有收到 environment 初始化 callback；同一探针在允许 Runtime 启动子进程的执行环境中通过。探针现在会在这种超时情况下失败，不将“API接受请求”误报为“网页已运行”。项目没有通过禁用浏览器安全沙箱来绕过问题。

WebView2 Runtime 真正缺失时，probe 返回 77，CTest 明确显示 SKIP。Runtime 已存在但创建失败、导航失败、脚本结果不符或超时，会返回失败。自动测试系统必须能启动 Windows Runtime 子进程才能运行这项测试。

OpenGL 测试会打印实际 vendor、renderer 和 context 信息。现代配置不存在时，测试验证 `UI_STATUS_UNSUPPORTED` 且无隐式降级，并打印对应配置的 `SKIP`。因此整体测试通过只证明接口和拒绝路径正确，不证明所有现代版本、MSAA、debug 或硬件加速都可用。记录测试日志中的实际支持数量，目标 GPU 上再次验证。

本工作区已记录 Intel(R) UHD Graphics 770，legacy context 报告 OpenGL `4.6.0 - Build 32.0.101.7079`；OpenGL 3.3 core、3.3 compatibility、3.3 core + MSAA 4 + debug 三组现代配置均实际创建并通过验证。这是该机器和驱动的证据，不能替代应用其他目标 GPU 的支持记录。

测试输出可在构建目录的 `Testing/Temporary/LastTest.log` 中查看。保存目标机器、工具链、选项、renderer 和结果，作为应用发布的验证记录。

原有基线完整输出保存在本地文件 `build/windows-ctest.log`，不随源码提交。新增宿主前的默认原生 `build/native` 基线为 13 项中 12 通过、1 跳过、0 失败。此前嵌入式 EDA 也完成启动存活检查；渲染、窗口几何与输入的证据来自对应集成测试，启动存活不替代图像或交互验收。

## 6. 必须完成的人工验证

自动矩阵使用的是逻辑客户区尺寸；它没有改变桌面显示器分辨率。96/144/192 DPI 的模拟调用也不能替代 Windows 真实跨显示器消息。

1. 空启动 `framework_host.exe`，通过菜单打开应用包，再从命令行同时打开两份；分别修改矩形、zoom和笔记，检查切换与关闭后台标签不改变另一实例。
2. 在实际不同缩放的两台显示器间移动宿主窗口，确认 Per-Monitor 模式、suggested RECT、公共/应用菜单、标签、助手、面板和 OpenGL framebuffer。
3. 在 1920×1080、1280×720、800×600、640×480 对应目标尺寸下缩放，检查主区、标签/浮动内容可用、AI默认收起，手动展开/收起后resize和标签切换保留选择；同时记录实际逻辑客户区与设备像素尺寸。
4. 最小化/恢复，检查画布重新绘制、比例、zoom 和资源仍正确，后台标签激活后也采用当前尺寸/DPI。
5. 检查面板停靠/浮动及 `ui_shell_refresh()` 保留内容；旧 `ui_native_shell_refresh()` 后重建内容并重新查询 handle，退出不访问旧 handle。
6. 在应用目标 GPU 上验证默认现代配置，另用嵌入式 EXE显式验证legacy路径；检查renderer，确认是否达到硬件加速要求。
7. 选择后台实例的助手命令，切换标签后检查日志目标；清空确认显示正确实例，拒绝时模型不变。
8. 用实际应用测试未保存关闭拒绝、异步等待后完成和最后实例卸载；接入模型后验证权限、校验、异步取消和文档rollback/undo。
9. 使用当前SDK/API9构建的原生/OpenGL和Web应用混合运行；检查浅色/深色、Web 菜单/工具入口、浮动外框及多行/中文输入，长日志预览与完整确认参数都可读取。

尚未完成真实跨显示器和目标 GPU 的验证时，应将这两项标为待验收，不能由模拟消息或整体 CTest 通过代替。

## 7. 依赖许可和分发

依赖许可取自第3节固定版本。`.deps/` 不随源码提交，准备依赖后在以下本地文件中查阅完整许可：

| 组件 | 许可文件 |
| --- | --- |
| Lexbor | Apache-2.0，本地 `.deps/lexbor/LICENSE`；[上游固定版本文件](https://github.com/lexbor/lexbor/blob/7fb22cf5664a331d7c24b113489e566767c9c25a/LICENSE) |
| QuickJS-NG | MIT，本地 `.deps/quickjs/LICENSE`；[上游固定版本文件](https://github.com/quickjs-ng/quickjs/blob/2f0aa72a6b09cf69ff06399bcf9f4c083ac1a278/LICENSE) |
| WebView2 SDK | Microsoft 提供的再分发条款，本地 `.deps/Microsoft.Web.WebView2.1.0.4129.50/LICENSE.txt` 和 `NOTICE.txt`；来源为[官方固定版本包](https://www.nuget.org/packages/Microsoft.Web.WebView2/1.0.4129.50) |

应用分发时应保留对应许可和 notices，并单独处理 WebView2 Runtime 的分发要求。此项目尚未选定框架本身的开源许可证；本文不替项目选择 MIT、Apache-2.0 或其他许可。第三方组件的许可证不自动成为框架和应用的许可证。

## 当前 OSMesa 可选提供方与复验

OSMesa 运行时采用 [mesa-dist-win 24.3.4 MSVC 包](https://github.com/pal1000/mesa-dist-win/releases/tag/24.3.4)：`mesa3d-24.3.4-release-msvc.7z` 的SHA256为 `7ebc711ad1896ac88ab21e142f1017f8ff035f0f342bdb72fbb5e2eb881ba363`。将x64目录原样保留到 `.deps/mesa-24.3.4/x64`（至少osmesa.dll及libglapi.dll及其运行库）；不会自动下载/安装或替换系统opengl32。新版已移除OSMesa，26.2.3包缺少此DLL不能作为复验提供方。

在x64开发终端，完成原轻量/WebView2依赖准备后执行：

```powershell
$osmesaPath = (Resolve-Path .deps/mesa-24.3.4/x64/osmesa.dll).Path
cmake -S . -B build/current-provider -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_FRAMEWORK_ENABLE_WEBVIEW2=ON "-DUI_OSMESA_LIBRARY=$osmesaPath"
cmake --build build/current-provider
ctest --test-dir build/current-provider --output-on-failure
```

`UI_OSMESA_LIBRARY`仅注册实际提供方验收，不是隐式运行模式或编译时硬依赖。当前api7_fixture仅用于框架集成；显式环境路径由integration测试传入。缺少提供方不会注册windowless/integration测试，不得声称通过。旧API集成/冻结SDK/原包注册和UI_LEGACY_*入口已退出当前CMake，历史源码及证据仍保留。

历史API5（78c24cb）在Windows普通账户Session1和实际CI LocalSystem Session0均已运行OSMesa实际frame；当前API9源码Session0仍需另验；整机无登录仍需独立环境，不通过隐藏WGL或替代图片推断。WebView2仍需要实际Runtime、图形会话和原生承载窗口，受限执行环境可能禁止浏览器子进程；通过只表示对应实际环境的证据。测试失败保留并修复，不使用skip或隐瞒资源增长。

Session0／无登录CI现有独立实际应用DLL及临时服务入口，详细构建、命令、环境门槛、资源预算及目标结果见[专门验收](validation/session0-osmesa-validation.md)。该测试配置同时关闭两个Web后端，不依赖WebView2或轻量引擎。默认运行器拒绝Session1；CTest对照不会计入Session0通过数。

CI使用MSVC19.44/W4/WX实际构建通过；x64包检查使用编译期 `UINTPTR_MAX` 比较，避免旧编译器C4127。正式用例保留全部资源采样，进程缓存采用声明的128 MiB预算；初始门槛和失败诊断见专门记录。Windows服务无继承控制台，测试直接重开stdout/stderr文件，不创建控制台窗口。默认托管runner实测有登录用户，严格无登录项会失败；已有无登录Windows x64服务runner可在手动工作流填写其runner_label，不自动创建机器或注销用户。

## 当前 API9 Windows 回归矩阵

[windows-regression.yml](../.github/workflows/windows-regression.yml)与[执行脚本](../tools/windows-ci.ps1)建立四种Windows-2022 x64/C11 Release配置。native关闭两种Web和宿主；light开启轻量但不配置OSMesa；webview2开启两种Web、真实Runtime和显式OSMesa；osmesa开启轻量/实际OSMesa测试DLL，关闭WebView2。四行都执行被配置的必要CTest。只有物理ui_monitor_transition允许缺条件跳过，其余跳过及失败均拒绝。

依赖沿用固定Lexbor/QuickJS-NG commit、WebView2 SDK1.0.4129.50及Mesa24.3.4 SHA256。组件输入桥接需要Runtime至少138.0.3351.48，依据[微软发布记录](https://learn.microsoft.com/en-us/microsoft-edge/webview2/release-notes/sdk/1-0-3351-48)；仅有SDK或旧Runtime不能提供ControllerOptions4。临时CI VM缺失或版本过旧时，用微软签名的官方Evergreen Bootstrapper安装/更新，再核验并记录实际版本及安装器哈希；本地准备脚本不自动安装Runtime。

托管GUI用例需要1920×1080虚拟桌面，脚本仅在GitHub临时VM调用Set-DisplayResolution并核验实际尺寸/DPI，记录原尺寸。这不能替代物理跨屏。依据[runner维护者说明](https://github.com/actions/runner-images/issues/2935)，默认1024×768不能满足宽窗断言。窄窗用例仍按原尺寸矩阵执行。

软件WGL在新build目录仅部署opengl32.dll、libgallium_wgl.dll、libglapi.dll及pipe_swrast.dll，显式GALLIUM_DRIVER=llvmpipe，记录实际renderer；不复制整个Mesa DLL目录污染其他系统库加载，也不修改系统OpenGL或冒充硬件GL。OSMesa行分软件WGL与显式provider两个阶段；WebView2行分WGL、provider、Runtime三个阶段。先运行需要软件WGL的用例，再移除这四个本行部署的DLL，运行显式OSMesa用例，最后使用平台图形运行匹配Runtime过滤器的用例；合并逐项结果和完整输出，任一阶段失败仍失败。OSMesa仍用绝对库路径与实际内存上下文及其提供方依赖，不受WGL部署移除影响。

历史API6最终本机与CI结果见[API6验收](validation/api6-validation.md)，Session0及严格无登录门槛保持[独立工作流](../.github/workflows/session0-osmesa.yml)。GitHub托管runner有已登录用户；矩阵绿色不能替代整机无登录通过。专用runner缺失时保持待验，不注销用户或改既有服务。

准备上述依赖，在x64开发终端使用新目录：

```powershell
$osmesa = (Resolve-Path .deps/mesa-24.3.4/x64/osmesa.dll).Path
cmake -S . -B build/current-api8 -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_FRAMEWORK_ENABLE_WEBVIEW2=ON "-DUI_OSMESA_LIBRARY=$osmesa"
cmake --build build/current-api8
ctest --test-dir build/current-api8 --output-on-failure
```

UI_BUILD_TESTS下生成api7_fixture.uapp，public-C应用包含十万行/64列、树/属性、底部/标签布局、实际OSMesa frame C图片、异步缩略图、语义命令、双实例/失败/模态/卸载。ui_api7_integration与Runtime变体只算框架集成证据；当前Alt/无窗口GL/MSAA/非MSAA基础功能仍独立回归。冻结SDK和历史集成/原包专项不再注册，不用新包冒充旧包。

## 菜单外观与鼠标回归

使用当前API9依赖和配置，执行`ctest --test-dir build/fw-next/release -R "ui_menu_desktop|ui_menu_cascade_offscreen|ui_menu_access|ui_menu_offscreen|ui_framework_features_host|ui_web_host_frontend" --output-on-failure`。`ui_menu_desktop_test`加载框架维护的framework_features.uapp，覆盖实际宿主/GL、三层鼠标、翻转、模态、双实例、焦点和40次重开；`ui_menu_cascade_offscreen_test`覆盖128项末项、超限、展开链预算与零菜单HWND。

只读观察现有应用时，可向`ui_menu_desktop_test.exe`传入原.uapp、证据输出前缀和`--observe`。该分支只打开/关闭菜单和切换宿主主题，不调用应用命令；输出真实PrintWindow客户区BMP，证据PNG只作无损转换。包及原应用目录保持只读，不覆盖它的运行库。菜单截图、窗口命中和程序DPI不能替代真实IME、物理跨屏、桌面合成人工及长期操作验收。详见[菜单验收](validation/menu-desktop-validation.md)。

## 当前 API9 源码交付和必要矩阵

API9/ABI1/包格式1，见[迁移](migration-v0.8-to-v0.9.md)及[验收](validation/instance-language-validation.md)。当前API9开发源码同步main；本地结果、源码同步、稳定发布及托管CI运行分别报告。工作流配置不等于对应commit已在托管CI通过。固定Lexbor/QuickJS与WebView2 SDK1.0.4129.50、显式x64 Mesa24.3.4 OSMesa路径保持。

原生/轻量/Runtime/OSMesa四配置沿用[windows-ci.ps1](../tools/windows-ci.ps1)。Runtime阶段显式纳入ui_component_scroll_webview2、ui_api7_integration_webview2并要求实际执行，仍与显式软件WGL隔离。API7独立DLL/.uapp组合100k/64列、底部/标签、RGBA、GL/缩略图/双实例/卸载；当前功能调用方使用当前头文件与API9声明，停止冻结SDK和旧包专项。

构建后执行ctest --test-dir build/web-shell --output-on-failure；专测使用-R "ui_workspace7|ui_component_scroll|ui_light_scroll|ui_component_experience|ui_api7_integration"。UI_RUNTIME_CYCLES=64选择64连续周期，UI_RUNTIME_READY_REOPENS=1要求实际Runtime呈现后关闭，再运行ui_api7_integration_test.exe package.uapp osmesa.dll webview2；记录实际绝对提供方路径及hash。

Session0用原[服务脚本](../tools/run-session0-validation.ps1)和[严格工作流](../.github/workflows/session0-osmesa.yml)，仅创建自身GUID临时服务，要求管理员/服务管理权限。不注销用户或改既有服务，RequireNoLogin检查整机登录会话；Session0成功不替代该门槛。当前证据必须有当前二进制hash。

## 当前 API9 CI 可复验性与版本门槛

沿用四行工作流，不重复搭建。执行脚本需在x64开发终端使用PowerShell 7、Ninja和CMake/CTest至少3.26（JUnit始于3.21，`--no-tests=error`始于3.26，见[CTest官方手册](https://cmake.org/cmake/help/latest/manual/ctest.1.html)）；普通产品CMake最低3.20不因此改变。先核对取得的确切API9 commit，再在新目录运行：

```powershell
./tools/test-windows-ci.ps1
# 四行分别执行；本地prepare不自动安装Runtime。
foreach ($row in @("native","light","webview2","osmesa")) {
    ./tools/windows-ci.ps1 -Configuration $row -Stage prepare
    # EvidenceDirectory可选；给本轮新目录，保留上轮原始证据。
    $evidence = "build/current-evidence-$row"
    ./tools/windows-ci.ps1 -Configuration $row -Stage build -EvidenceDirectory $evidence
    ./tools/windows-ci.ps1 -Configuration $row -Stage test -EvidenceDirectory $evidence
}
```

`prepare`核验固定源码commit、SDK/Mesa归档SHA256；Evergreen Runtime不是固定镜像，只在临时GitHub VM缺失/过旧时使用微软签名Bootstrapper，并记录实际版本。应使用新的构建目录，不从别处复制Runtime配置或用户数据。构建及CTest错误均非零退出；空测试、缺项、重复项、失败或非物理项跳过均拒绝。`test-plan.json`保存命令/参数及配置清单，合并JUnit必须与它一一对应。当前ui_component_scroll_webview2、ui_component_experience_webview2、ui_api7_integration_webview2及ui_visual_ui_webview2按Runtime阶段分配；旧API集成/compat/original/legacy专项误入清单时校验拒绝。

每行`build/ci-evidence-<row>/`归档源commit/dirty状态、运行ID、OS/Session/桌面/DPI/Runtime、编译输出、实际DLL/EXE/.uapp及OSMesa哈希、精确测试清单、JUnit、完整LastTest和真实帧；工作流always上传30日保留的artifact。本机运行仍是本机证据，不能将无GitHub run ID的结果写为托管CI。

`compatibility-policy.json`取代原包专项摘要，明确maintained_api=9、current_only及现存加载行为不是未来兼容承诺。当前样例全部使用当前SDK、清单及DLL API9；不编译冻结调用方，不验收旧包，也不以新包代替。

历史原包保全位置/哈希仍见[API7历史记录](validation/api7-validation.md#5-兼容依赖和复现)，保持只读。本轮不修改冻结SDK、历史验收或原包；UI_LEGACY_*不再是当前构建开关，遗留CMakeCache条目不产生旧版测试。

[本轮交付及失败记录](validation/api7-delivery-validation.md)提供实际commit、运行/复现步骤和待运行的远程操作；[业务试点](business-pilot.md)先核对指定应用/修改授权。严格[Session0工作流](../.github/workflows/session0-osmesa.yml)复用原服务脚本，`RequireNoLogin`保持整机登录会话门槛，当前权限不足或无目标runner时分别记待验。

显式provider阶段包含ui_windowless_gl、ui_session0_interactive_control、Light的ui_api7_integration/ui_visual_ui_light；Runtime变体只在Runtime阶段。软件WGL DLL即使没有调用WGL也可能初始化进程资源；实际对照曾在200周期资源断言失败，原断言保留，三次同二进制移除本行WGL部署的对照通过。详见收敛记录，不把这项环境隔离称为历史WebView2波动的完整根因。


## 框架视觉复验

公共API为8；内部视觉修正不再次升级接口。统一CSS在CMake配置时嵌入生成的宿主资源、组件/菜单字符串及Web浮动标题。修改src/visual.css或模板后执行cmake --build会触发重配置/RC与DLL构建；仅打开源host.html不能代替运行真实宿主。

使用第3节固定依赖、真实Runtime及显式OSMesa配置，在Windows x64开发终端执行：

```powershell
cmake -S . -B build/visual -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_FRAMEWORK_ENABLE_WEBVIEW2=ON -DUI_OSMESA_LIBRARY="$PWD/.deps/mesa-24.3.4/x64/osmesa.dll"
cmake --build build/visual
New-Item -ItemType Directory -Path build/visual-evidence -Force | Out-Null
ctest --test-dir build/visual --output-on-failure -R '^(ui_visual_ui_light|ui_visual_ui_webview2|ui_browser_host|ui_web_host_frontend|ui_menu_desktop|ui_component_experience|ui_component_scroll|ui_floating_web_dpi)$'
& .\build\visual\ui_visual_ui_test.exe .\build\visual\api7_fixture.uapp "$PWD/.deps/mesa-24.3.4/x64/osmesa.dll" light "$PWD/build/visual-evidence/light"
& .\build\visual\ui_visual_ui_test.exe .\build\visual\api7_fixture.uapp "$PWD/.deps/mesa-24.3.4/x64/osmesa.dll" webview2 "$PWD/build/visual-evidence/webview2"
```

两项视觉目标要求轻量、独立宿主及显式OSMesa；Runtime变体还要求启用WebView2并实际运行Runtime。测试使用框架api7_fixture.uapp，不修改业务应用；真实宿主PrintWindow及C捕获保存BMP，程序DPI96/144/192、双实例、十万行、错误/草稿、颜色/枚举和收起/展开访问均有断言。截图不能替代DWM/物理IME验收，`--baseline`仅是保存旧源码对照的测试模式，不能用于最终通过口径。

现有CI分类把ui_visual_ui_light放入独立OSMesa/provider阶段，把ui_visual_ui_webview2放入实际Runtime阶段；不提高原预算或增加远程执行授权。[历史视觉身份、矩阵及失败](validation/visual-ui-validation.md)区别本机结果和未运行托管CI。

若复用含四个应用本地Mesa WGL DLL的旧build目录，直接CTest前必须按现有CI显式设置GALLIUM_DRIVER=llvmpipe。省略时Mesa可能自动选择D3D12；本轮Windows26300/Intel组合在libgallium_wgl崩溃，严格保留原失败。同二进制显式llvmpipe通过不代表D3D12已修复，不删除测试DLL或改变原断言。[环境对照](validation/visual-ui-validation.md#3-测试与失败)。

此前当前版本政策及[分页清理实测](validation/component-scroll-only-validation.md)替代旧SDK/原包专项门槛。当前Session0对照包也声明API9，CI按注册的ui_session0_interactive_control归档run.log、resources.csv、result.txt和实际帧；运行器不生成manifest.json，先前该文件条件导致漏存原文件，历史CTest结果不因此改写。


## 原生共同组件鼠标复验

当前轻量配置新增 `ui_component_scroll_native`，使用真实SendInput、原生内容槽、同步/受控异步源、TREE/LIST/TABLE和64列。需可用的已登录输入桌面、可见窗口及足够屏幕尺寸（程序192 DPI窗口约1700×1100）；非交互服务环境不能以该用例证明鼠标功能。原断言非零退出，不改预算或使用Home/End替代。

```powershell
ctest --test-dir build/current -R "^ui_component_scroll_native$" --output-on-failure
New-Item -ItemType Directory -Force build/native-scroll-images
$env:UI_NATIVE_SCROLL_EVIDENCE = (Resolve-Path build/native-scroll-images).Path
& .\build\current\ui_component_scroll_native_test.exe
```

该用例与原离屏完整范围测试独立，均列入当前轻量CI必要检查；本轮实测身份、业务只读重建、失败记录及完整SDK见[验收](validation/native-component-scroll-validation.md)。本机结果不算托管CI、Session0或物理/人工验收。

本轮视觉复验增加ui_light_glyph_padding（实际零内距图标像素）、空标题/字段对齐/GL图片比例/只读操作栏和表头宽度断言；现有CI对非native配置要求新像素用例。原生鼠标同HWND捕获及TREE加载覆盖层保持。串行执行当前矩阵，避免独立输入程序抢共享桌面焦点。

真实GL桌面截图需输入桌面和像素读取权限；PrintWindow可能省略原生GL，不能以白色画布证明实际渲染。Sandbox中Runtime可能保持ready0/nav0/HRESULT0而无创建回调，保留原失败后使用同二进制正常桌面对照；不强杀、删目录、延时或提高阈值。必要回归命令：

~~~powershell
ctest --test-dir build/visual -j 1 --output-on-failure --output-junit current.xml
ctest --test-dir build/visual -R "^(ui_light_glyph_padding|ui_component_scroll_native|ui_visual_ui_light|ui_visual_ui_webview2)$" --output-on-failure
./tools/test-windows-ci.ps1
~~~

[完整窗口实测](validation/visual-polish-validation.md)列出准确源哈希、本地commit、初始菜单失败/修复、两后端真实截图和业务只读观察。当前Session1控制不替代当前源码Session0服务或整机无登录，缺权限时保持服务脚本门控待验。

## 此前API8必跑增补（当前继续执行）

现有四行CI复用，不新建工作流。light/osmesa/webview2必须包含ui_component_widths，Runtime行另含ui_component_widths_webview2；tools/test-windows-ci.ps1验证分类与缺失测试拒绝。测试名ui_api7_integration与api7_fixture表示既有基础用例，当前编译宏、清单及DLL声明已为8，不能把文件名视为旧SDK兼容专项。

本轮本地构建build/experience-20261007/after；证据见[当前验收](validation/component-experience-validation.md)。真实Runtime测试需要允许浏览器进程的Windows桌面，沙箱内失败需单独保留；使用构建缓存记录的CTest绝对路径并检查真实退出码。当前服务Session0与整机无登录门槛独立，普通桌面OSMesa通过不算服务或无登录成功。

### API8 稳定性与完整存储示例

指定 UI_OSMESA_LIBRARY 且 UI_BUILD_TESTS=ON 时构建 stateful_components.uapp（完整数据／属性／实际 GL／worker 场景）及其测试；普通无提供方配置不冒充通过。参见[接入示例](../examples/stateful_components/README.md)。当前 provider 阶段必含 ui_host_repaint、ui_stateful_components_light；真实 Runtime 阶段必含 ui_stateful_components_webview2，仍复用四行工作流，不增加旧版本专项。

```powershell
ctest --test-dir build/current -R '^ui_host_repaint$' --output-on-failure
ctest --test-dir build/current -R '^ui_stateful_components_(light|webview2)$' --output-on-failure
./tools/test-windows-ci.ps1
```

新状态脚本串行创建独立进程和证据目录，实际鼠标调宽后关闭、重启恢复；损坏、未来文件、两档案锁与初始化失败均非零失败退出。输出日志、真实窗口 BMP 和 EXE／包／运行库哈希，保留原预算和关闭合同。只在本机实际运行不算托管 CI；当前结果、未定位重绘及环境边界见[本轮验收](validation/api8-stability-state-validation.md)。


## API9 语言回归与SDK

当前构建产出API9头文件、导入库、共享DLL、宿主及清单匹配的应用包。ui_language_core覆盖来源、通知、线程、完整size和旧语言异步失效；ui_instance_language_light/webview2使用实际完整DLL、十万行/64列、GL和真实鼠标，覆盖96/144/192及跨进程档案。run-instance-language.ps1先验证10种偏好文件，再write/restore；Runtime必须实际可用。CI沿用四行矩阵并强制这些当前用例，未触发远程工作流。锁屏/共享输入、真实IME、物理跨屏、Session0及严格无登录与本机结果分开报告。[实际命令、失败和SDK校验](validation/instance-language-validation.md)。

### 托管失败的本地复验与阶段诊断

接手后的语言断言修正及原180/240秒超时定位边界见[CI收敛记录](validation/ci-recovery-validation.md)。空宿主使用公共Windows显示语言查询，不修改机器设置；应用实例测试明确提交zh-CN/en-US，继续精确检查文本及原语义。重绘测试逐断言打印表达式与实际绘制/结果/关闭值，不能把paint0单独作为整项通过。

完整脚本每个独立进程都有CASE_BEGIN/CASE_END和manifest的running/exited、elapsedMs、exitCode；C日志另含PID、tick、实际Dispatch计数及创建/打开/截图/关闭/COM返回阶段。最后一条failures0不等于脚本已取得退出码。`tools/windows-ci.ps1`同时归档state-evidence-*与language-evidence-*，超时必须保留最后running阶段，仍按原TIMEOUT失败，不增等待或放宽资源。

四行配置沿用现有WGL/provider/Runtime分类，依赖已存在时先核验固定提交和目录改动，勿盲目prepare覆盖。独立新build目录、串行真实输入、JUnit精确清单和二进制哈希必须绑定本轮源码；本地结果不改写原托管CI结论。本轮没有新的推送或远程工作流授权。

2026-10-09当前API9完整SDK的源码/说明快照为44c039f；实际解包两后端完整语言套件、包内固定依赖离线构建、三项公共调用/OSMesa及运行后逐文件身份均另有[最终证明和复验命令](validation/instance-language-current-validation.md#sdk与证据保存)。这是本地开发交付，封存归档不补写后续证明，原托管超时和当前服务/物理条件继续分别待验。

## 2026-10-09 公共布局与真实可见性验收

新增 `ui_component_layout` / `ui_component_layout_webview2` 纳入现有WGL/provider/Runtime隔离与完整清单检查；原断言、180/240/300秒及资源门槛保持。`dialog_layout_example` 仅链接当前公共共享库，`ui_interaction_probe` / `ui_display_probe` 为不加载业务DLL的公共C复现，真实输入必须在解锁桌面串行执行。SendInput、实际窗口画面与自动状态检查分列；程序192不代表物理高DPI。当前开发SDK增补需匹配头文件、导入库和运行库，冻结API9归档不包含三个新导出。[命令、实际提交、证据及限制](validation/ui-repair-validation.md)。
