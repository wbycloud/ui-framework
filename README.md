# Windows C/Web UI Framework

面向 Windows x64 的轻量 C 应用框架。独立宿主 `framework_host.exe` 加载 `.uapp` 应用包，使用标签同时运行多个应用，并提供菜单、工具栏、侧栏、悬浮窗、Per-Monitor DPI、OpenGL 内容区和全局助手命令面板。应用通过公共 C ABI 接入，维护自己的文档和业务逻辑。

默认构建采用 Win32/OpenGL，不包含第三方 HTML/JavaScript 引擎。Lexbor/QuickJS-NG 和 WebView2 是可选后端。框架同时保留静态库与应用自行创建窗口的嵌入式使用方式。

## 当前版本与开发入口

| 项目 | 当前值 | 定义位置 |
| --- | --- | --- |
| SDK 发布版本 / 稳定标签 | `0.1.0` / `v0.1.0` | [CMakeLists.txt](CMakeLists.txt)、Git 标签 |
| 框架 API | `1` | [ui.h](include/ui_framework/ui.h) |
| 应用 ABI | `1` | [application.h](include/ui_framework/application.h) |
| 应用包格式 | `1` | [package.h](include/ui_framework/package.h) |
| 开发标准修订 | `1`，对应 `v0.1.0` | [应用开发标准](docs/application-development-standard.md) |

应用开发和升级以稳定 Git 标签对应的说明及头文件为准。当前稳定基准为 [v0.1.0](https://github.com/wbycloud/ui-framework/tree/v0.1.0)，`main` 用于后续开发。API/ABI 当前采用严格版本校验，尚未提供跨版本兼容范围协商或自动迁移工具。

建议按以下顺序阅读：

1. [CHANGELOG](CHANGELOG.md)：版本变化、兼容性和迁移说明。
2. [应用开发标准](docs/application-development-standard.md)：生命周期、线程、所有权、多实例、DPI、OpenGL/Web 和助手接入约定。
3. [构建与验证](docs/build-and-validation.md)：工具链、可选依赖、测试和发布检查。
4. [最小 EDA 模块](examples/minimal_eda/app.c)及其[清单](examples/minimal_eda/manifest.ini)：可构建的独立应用参考。

## 获取、构建与运行

需要 Git、Visual Studio MSVC C/C++ 工具、Windows SDK、CMake 3.20 或更新版本，以及 Ninja。以下命令在选择 x64 工具链的 Visual Studio Developer PowerShell 中执行。最小 EDA 模块需要驱动支持 OpenGL 3.3 compatibility profile。

```powershell
git clone https://github.com/wbycloud/ui-framework.git
cd ui-framework
git switch --detach v0.1.0
cmake -S . -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/native
ctest --test-dir build/native --output-on-failure
& .\build\native\framework_host.exe
```

宿主空启动后，选择“框架 → 打开应用”，打开 `build/native/minimal_eda.uapp`。再次打开该包会创建第二个独立 EDA 标签，也可通过命令行同时打开两份：

```powershell
& .\build\native\framework_host.exe .\build\native\minimal_eda.uapp .\build\native\minimal_eda.uapp
```

点击画布后，`A` 添加矩形、`Z` 放大、`C` 清空。两个实例的数据、缩放和属性区笔记独立保存。`Ctrl+O` 打开应用，`Ctrl+W` 关闭标签，`Ctrl+Tab` / `Ctrl+Shift+Tab` 切换标签。全局助手可选目标实例、查看命令和参数 schema、提交 JSON、查看结果与快照；危险命令由应用权限和确认接口控制。

分发默认宿主时，保留同目录的 `framework_host.exe` 与 `ui_framework.dll`，并提供应用包。构建自动生成这些文件和 `uapp_pack.exe`；Git 仓库保存源码，不提交构建产物。

嵌入式模式的参考是 [main.c](examples/minimal_eda/main.c)，可运行 `build/native/minimal_eda.exe`。显式添加 `--legacy` 可以验证旧式 WGL 路径；此参数不适用于宿主加载的 EDA 模块。

## 开发应用

应用 DLL 使用公共头文件，链接共享框架的 `ui_framework_runtime.lib`，导出 `ui_app_query_v1`，实现实例生命周期并注册自己的 UI 和语义命令。CMake 应用模块链接 `ui_framework_shared` 可获得公共 include 路径和 shared 编译定义。宿主提供统一的 `ui_framework.dll`。

应用包是版本化、未压缩的单文件容器，由清单、DLL 和资源组成。构建产生的 EDA staging 目录可直接重新打包：

```powershell
& .\build\native\uapp_pack.exe .\examples\minimal_eda\manifest.ini .\build\native\eda_package .\build\native\minimal_eda-copy.uapp
```

实例的私有状态由应用分配和释放；宿主管理框架对象。工作线程只通过复制投递接口交付结果、事件、进度和关闭完成通知。完整的关闭顺序、回调 scope 以及模块卸载要求见[开发标准](docs/application-development-standard.md)。

## 已有应用如何升级

在应用项目中独立记录框架仓库地址、已适配的 SDK 标签或 commit、开发标准修订号及 API/ABI。SDK 记录不放进当前 `.uapp` 清单：格式 v1 严格接受已有字段，额外字段会被拒绝。

升级流程：

1. 选定目标稳定标签，阅读从当前基准到目标版本之间的 CHANGELOG。
2. 对照该标签的开发标准、公共头文件和示例，判断变化属于无需修改、可选升级还是必须迁移。
3. 修改受影响的接口调用与使用约定，使用目标 SDK 重新构建应用 DLL 和应用包。
4. 验证加载、多实例、助手路由、DPI/绘制、异步关闭及完整卸载，再更新应用的适配记录。

开发者或 AI 可从本 README 开始检查升级，但仍需读取应用自己的版本记录和代码。新增可选能力无需由所有旧应用立即采用；接口版本相同也不能替代对行为变化和验收结果的核对。

## 可选 Web 后端

新克隆仓库不包含 `.deps/`，构建不会自动联网下载依赖。先按[依赖准备说明](docs/build-and-validation.md#3-启用两种-web-后端)获取固定版本，再启用所需开关：

```powershell
cmake -S . -B build/web -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_FRAMEWORK_ENABLE_LIGHT_WEB=ON -DUI_FRAMEWORK_ENABLE_WEBVIEW2=ON
cmake --build build/web
ctest --test-dir build/web --output-on-failure
```

轻量后端使用 Lexbor v2.5.0 对应 commit `7fb22cf5664a331d7c24b113489e566767c9c25a` 与 QuickJS-NG commit `2f0aa72a6b09cf69ff06399bcf9f4c083ac1a278`。WebView2 SDK 固定为 `1.0.4129.50`，浏览器 Runtime 需另外安装。两个开关可独立启用；轻量后端是固定 HTML/CSS/JS 子集，WebView2 的现代网页能力由 Runtime 提供。TypeScript 需离线编译为 JavaScript。

## 验证与能力边界

2026-10-02，干净源码在没有 `.deps/` 的情况下完成原生构建，测试为 **15 通过、1 失败、1 跳过**。失败项为本机 Windows 将 1920×1080 客户区加标题栏后的窗口高度限制到 1100 像素，纯 Win32 对照窗口也得到相同结果；其余三组窗口尺寸通过。该失败保留记录，详见[本次干净源码验收](docs/build-and-validation.md#51-本次干净源码验收)。

已有 Windows x64 集成验收记录为：原生配置 16 项通过、1 项跳过；同时启用 Web 后端时 18 项通过、1 项跳过，均无失败。跳过项为真实跨显示器测试，测试环境只有一台显示器。详细条件和历史记录见[构建与验证](docs/build-and-validation.md#5-结果解读和验证记录)。目标机器仍需验证实际显示器 DPI 变化与 GPU 配置。

应用在同一进程执行，应用崩溃可能影响宿主。助手面板目前用于协议验证，尚未连接模型服务。EDA 是验证示例；框架不实现 EDA、Markdown、画板、PPT 或电子表格的完整文档模型，也不直接加载任意现成 EXE。

## 许可证

框架本身的许可证尚未选定。可选第三方组件有各自的许可证和再分发条款，来源与文件说明见[依赖许可](docs/build-and-validation.md#7-依赖许可和分发)。
