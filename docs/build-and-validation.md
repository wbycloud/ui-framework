# 构建与验证

本项目的实际 UI 目标是 Windows x64。当前 SDK 为 0.5.0 开发源码，最近稳定标签仍是 `v0.1.0`。Windows 默认构建 Web 独立宿主 `framework_host.exe`、共享运行库 `ui_framework.dll`、打包器、API1 EDA、API2 Web Counter 、API3 generic_components 和 API4 framework_features 应用包；同时保留 C11 静态框架、原生嵌入式 EDA 和核心/Win32 测试。默认轻量 Web 引擎需要另行准备固定依赖，WebView2 默认关闭。应用 API/ABI、包格式和生命周期见 [应用开发标准](application-development-standard.md)，升级见[迁移指南](migration-v0.1-to-v0.2.md)。

当前API5能力见[菜单与离屏](framework-menu-offscreen.md)、[通用Web](generic-web-ui.md)及[验收记录](validation/api5-validation.md)。[API4样例](../examples/framework_features/README.md)、[API4验收](validation/api4-validation.md)及[API3记录](validation/api3-validation.md)保留历史版本证据；新接口迁移见[0.4→0.5](migration-v0.4-to-v0.5.md)。第5节保留旧版本历史结果，不作为当前测试状态。

## 1. 构建环境

使用安装了 MSVC C/C++ 工具、Windows SDK、CMake 和 Ninja 的 Visual Studio Developer PowerShell，选择 x64 工具链，并进入仓库根目录。WebView2 loader 根据目标架构选择；本轮实际 UI、GPU 和 Runtime 验证覆盖 Windows x64，旧 panel 描述兼容性另在 x86/x64 通过独立编译验证。

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

Web 外壳提供打开/关闭入口，应用菜单和工具入口跟随当前标签，支持浅色/深色切换；顶层保留 Windows 标准标题栏。`Ctrl+Shift+O` 打开应用包、`Ctrl+W` 关闭、`Ctrl+Tab` / `Ctrl+Shift+Tab` 切换标签。全局助手可选择目标实例，查看命令/schema，输入 `{}` 调用 EDA 命令并观察结果/快照；`eda.clear` 请求确认。事务按钮调用应用的 begin/commit/rollback/undo callback，EDA 未提供时返回 UNSUPPORTED。这个区域用于协议验证，尚未连接模型服务。

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

应用模块使用 [`application.h`](../include/ui_framework/application.h)，导出 `ui_app_query_v1`，链接共享框架的 `ui_framework_runtime.lib`，并用 `UI_FRAMEWORK_BUILD_SHARED` 编译；不要链接供嵌入式应用使用的静态 `ui_framework.lib`。CMake 模块目标链接 `ui_framework_shared`，公共 include 路径与 shared 定义由该目标传递。当前头文件的 API 宏是 4；使用新接口的应用将清单与 DLL descriptor 一致声明为4，旧 API1/2/3 包可保留原声明。清单存于 staging 目录外，staging 只放 module 和资源。构建自动生成 `build/web-shell/eda_package`，内容为：

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
| `ui_application_versions` | API 1/2 接受、非法版本拒绝、旧/新包加载、多实例、Web 内容与原生/OpenGL 内容、清单/DLL API 不一致拒绝；可指定原 v0.1.0 二进制包 |
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
3. 在 1920×1080、1280×720、800×600、640×480 对应目标尺寸下缩放，检查主区、标签/浮动内容可用、全局助手自动折叠且可手动展开；同时记录实际逻辑客户区与设备像素尺寸。
4. 最小化/恢复，检查画布重新绘制、比例、zoom 和资源仍正确，后台标签激活后也采用当前尺寸/DPI。
5. 检查面板停靠/浮动及 `ui_shell_refresh()` 保留内容；旧 `ui_native_shell_refresh()` 后重建内容并重新查询 handle，退出不访问旧 handle。
6. 在应用目标 GPU 上验证默认现代配置，另用嵌入式 EXE显式验证legacy路径；检查renderer，确认是否达到硬件加速要求。
7. 选择后台实例的助手命令，切换标签后检查日志目标；清空确认显示正确实例，拒绝时模型不变。
8. 用实际应用测试未保存关闭拒绝、异步等待后完成和最后实例卸载；接入模型后验证权限、校验、异步取消和文档rollback/undo。
9. 使用原 v0.1.0 二进制包与 API 2 Web Counter 混合运行；检查浅色/深色、Web 菜单/工具入口、浮动外框及多行/中文输入，长日志预览与完整确认参数都可读取。

尚未完成真实跨显示器和目标 GPU 的验证时，应将这两项标为待验收，不能由模拟消息或整体 CTest 通过代替。

## 7. 依赖许可和分发

依赖许可取自第3节固定版本。`.deps/` 不随源码提交，准备依赖后在以下本地文件中查阅完整许可：

| 组件 | 许可文件 |
| --- | --- |
| Lexbor | Apache-2.0，本地 `.deps/lexbor/LICENSE`；[上游固定版本文件](https://github.com/lexbor/lexbor/blob/7fb22cf5664a331d7c24b113489e566767c9c25a/LICENSE) |
| QuickJS-NG | MIT，本地 `.deps/quickjs/LICENSE`；[上游固定版本文件](https://github.com/quickjs-ng/quickjs/blob/2f0aa72a6b09cf69ff06399bcf9f4c083ac1a278/LICENSE) |
| WebView2 SDK | Microsoft 提供的再分发条款，本地 `.deps/Microsoft.Web.WebView2.1.0.4129.50/LICENSE.txt` 和 `NOTICE.txt`；来源为[官方固定版本包](https://www.nuget.org/packages/Microsoft.Web.WebView2/1.0.4129.50) |

应用分发时应保留对应许可和 notices，并单独处理 WebView2 Runtime 的分发要求。此项目尚未选定框架本身的开源许可证；本文不替项目选择 MIT、Apache-2.0 或其他许可。第三方组件的许可证不自动成为框架和应用的许可证。

## API5 可选提供方与复验

OSMesa 运行时采用 [mesa-dist-win 24.3.4 MSVC 包](https://github.com/pal1000/mesa-dist-win/releases/tag/24.3.4)：`mesa3d-24.3.4-release-msvc.7z` 的SHA256为 `7ebc711ad1896ac88ab21e142f1017f8ff035f0f342bdb72fbb5e2eb881ba363`。将x64目录原样保留到 `.deps/mesa-24.3.4/x64`（至少osmesa.dll及libglapi.dll及其运行库）；不会自动下载/安装或替换系统opengl32。新版已移除OSMesa，26.2.3包缺少此DLL不能作为复验提供方。

在x64开发终端，完成原轻量/WebView2依赖准备后执行：

```powershell
$osmesaPath = (Resolve-Path .deps/mesa-24.3.4/x64/osmesa.dll).Path
cmake -S . -B build/api5 -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_FRAMEWORK_ENABLE_WEBVIEW2=ON "-DUI_OSMESA_LIBRARY=$osmesaPath"
cmake --build build/api5
ctest --test-dir build/api5 --output-on-failure
```

`UI_OSMESA_LIBRARY`仅注册实际提供方验收，不是隐式运行模式或编译时硬依赖。API5 fixture只用于验收；显式环境路径由integration测试传入。缺少提供方不会注册windowless/integration测试，不得声称这两项通过。原API4包可用 `UI_LEGACY_API4_PACKAGE` 加入原包宿主复验；SDK4重新构建测试单独运行。API1/2/3旧包入口保持原构建合同。

Windows普通账户Session1和实际CI LocalSystem Session0均已运行OSMesa实际frame；整机无登录仍需独立环境，不通过隐藏WGL或替代图片推断。WebView2仍需要实际Runtime、图形会话和原生承载窗口，受限执行环境可能禁止浏览器子进程；通过只表示对应实际环境的证据。测试失败保留并修复，不使用skip或隐瞒资源增长。

Session0／无登录CI现有独立实际应用DLL及临时服务入口，详细构建、命令、环境门槛、资源预算及目标结果见[专门验收](validation/session0-osmesa-validation.md)。该测试配置同时关闭两个Web后端，不依赖WebView2或轻量引擎。默认运行器拒绝Session1；CTest对照不会计入Session0通过数。

CI使用MSVC19.44/W4/WX实际构建通过；x64包检查使用编译期 `UINTPTR_MAX` 比较，避免旧编译器C4127。正式用例保留全部资源采样，进程缓存采用声明的128 MiB预算；初始门槛和失败诊断见专门记录。Windows服务无继承控制台，测试直接重开stdout/stderr文件，不创建控制台窗口。默认托管runner实测有登录用户，严格无登录项会失败；已有无登录Windows x64服务runner可在手动工作流填写其runner_label，不自动创建机器或注销用户。
