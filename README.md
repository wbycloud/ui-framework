# Windows C/Web UI Framework

面向 Windows x64 的轻量 C 应用框架。独立宿主 `framework_host.exe` 加载 `.uapp` 应用包，以浏览器风格的 Web 外壳提供多应用标签、菜单、工具入口、侧栏外框和全局助手。外壳使用 Lexbor、QuickJS-NG 与 GDI 实现受控 HTML/CSS/JS 子集，支持浅色/深色切换，并保留 Windows 标准标题栏。

新应用通过公共 C ABI 注册框架 Web 组件、提供数据并绑定语义命令；绘图内容按需要使用 OpenGL 内容槽，文档和业务逻辑由应用维护。旧 API1/2 应用自有原生内容继续兼容。框架同时保留静态库及应用自行创建窗口的原生嵌入式模式。WebView2 是可选的应用内容后端。

## 当前版本与开发入口

| 项目 | 当前开发值 | 定义位置 |
| --- | --- | --- |
| SDK | `0.3.0` 开发版，尚未发布对应稳定标签 | [CMakeLists.txt](CMakeLists.txt) |
| 最近稳定标签 | `v0.1.0` | [稳定源码](https://github.com/wbycloud/ui-framework/tree/v0.1.0) |
| 框架 API | `3`；当前运行库接受 `1`、`2`、`3` | [ui.h](include/ui_framework/ui.h) |
| 应用 ABI / 包格式 | `1` / `1` | [application.h](include/ui_framework/application.h)、[package.h](include/ui_framework/package.h) |
| 开发标准修订 | `3`，对应当前 `0.3.0` 开发源码 | [应用开发标准](docs/application-development-standard.md) |

当前工作源码为 0.3.0 开发版，不创建稳定标签。采用开发版本须记录确切 commit；稳定基准仍为 v0.1.0。清单与 DLL 的 API 声明必须一致，API3 应用不能加载到只支持 API1/2 的旧运行库。

建议按以下顺序阅读：

1. [CHANGELOG](CHANGELOG.md)：版本变化和兼容性分类。
2. [0.2 → 0.3 迁移](docs/migration-v0.2-to-v0.3.md)：通用组件、图片资源及输入行为；更早版本参考 [0.1 → 0.2](docs/migration-v0.1-to-v0.2.md)。
3. [应用开发标准](docs/application-development-standard.md)：生命周期、线程、所有权、DPI、内容后端与助手约定。
4. [构建与验证](docs/build-and-validation.md)：固定依赖、构建开关、测试和发布检查。
5. [通用 Web UI](docs/generic-web-ui.md)及[纯 C 示例](examples/generic_components/README.md)：框架管理的树、表格、表单、对话框、缩略图和绘图内容槽。
6. [Web Counter](examples/web_counter/app.c)及其[清单](examples/web_counter/manifest.ini)：API 2 的纯 Web 内容示例；[最小 EDA](examples/minimal_eda/app.c)展示 API 1 原生/OpenGL 内容兼容路径。
7. [应用升级提示词](docs/application-upgrade-prompt.md)：交给其他应用项目的适配指令，包含版本核对和未验证项的处理要求。

## 获取、构建与运行

需要 Git、Visual Studio MSVC C/C++ 工具、Windows SDK、CMake 3.20 或更新版本，以及 Ninja。以下命令在选择 x64 工具链的 Visual Studio Developer PowerShell 中执行。

Windows 默认开启 `UI_BUILD_STANDALONE_HOST` 和 `UI_FRAMEWORK_ENABLE_LIGHT_WEB`。新 clone 不包含 `.deps/`，构建不会自动下载依赖；首次构建先准备固定版本：

```powershell
git clone https://github.com/wbycloud/ui-framework.git
cd ui-framework
git rev-parse HEAD
New-Item -ItemType Directory -Path .deps -Force | Out-Null
git clone --no-checkout https://github.com/lexbor/lexbor.git .deps/lexbor
git -C .deps/lexbor checkout --detach 7fb22cf5664a331d7c24b113489e566767c9c25a
git clone --no-checkout https://github.com/quickjs-ng/quickjs.git .deps/quickjs
git -C .deps/quickjs checkout --detach 2f0aa72a6b09cf69ff06399bcf9f4c083ac1a278
cmake -S . -B build/web-shell -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/web-shell
ctest --test-dir build/web-shell --output-on-failure
& .\build\web-shell\framework_host.exe
```

宿主空启动后点击“打开应用”，选择 `build/web-shell/web_counter.uapp`，无需 OpenGL。再次打开会创建另一个独立计数器标签。也可同时打开纯 Web 计数器与 OpenGL EDA：

```powershell
& .\build\web-shell\framework_host.exe .\build\web-shell\web_counter.uapp .\build\web-shell\minimal_eda.uapp
```

打开两个 API3 通用示例：

```powershell
& .\build\web-shell\framework_host.exe .\build\web-shell\generic_components.uapp .\build\web-shell\generic_components.uapp
```

示例无需编写组件 HTML，展示按需树、表格/复选框/图片/样式、单位和混合值表单、Web 参数对话框、后台缩略图及绘图浮窗。菜单“Web 对话框”或 Ctrl+D 打开参数界面；Enter 提交表格编辑，Esc 取消；表格 Shift+滚轮切换列。它的 OpenGL 三角形仅为内容槽验证，不是框架业务渲染器。

EDA 需要驱动支持 OpenGL 3.3 compatibility profile。点击画布后，`A` 添加矩形、`Z` 放大、`C` 清空。不同实例的数据、缩放和属性笔记独立保存。`Ctrl+O` 打开应用，`Ctrl+W` 关闭标签，`Ctrl+Tab` / `Ctrl+Shift+Tab` 切换标签。全局助手可选目标实例、查看命令/schema、提交 JSON、查看结果与快照；危险命令经过权限和确认接口。

分发宿主时，保留同目录的 `framework_host.exe` 与匹配的 `ui_framework.dll`，并提供应用包、工具链运行依赖及第三方许可。默认轻量引擎静态链接到运行库；当前共享运行库使用动态 CRT，部署要求见[构建与验证](docs/build-and-validation.md#21-打包自己的应用)。Git 仓库不提交构建产物。

## 原生嵌入式构建

只构建原生/OpenGL、打包器和核心测试时，必须同时关闭独立宿主与轻量 Web 后端。这条路径不需要 `.deps/`：

```powershell
cmake -S . -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_BUILD_STANDALONE_HOST=OFF -DUI_FRAMEWORK_ENABLE_LIGHT_WEB=OFF -DUI_FRAMEWORK_ENABLE_WEBVIEW2=OFF
cmake --build build/native
ctest --test-dir build/native --output-on-failure
& .\build\native\minimal_eda.exe
```

该配置不产生 `framework_host.exe`。[嵌入式 EDA](examples/minimal_eda/main.c)自行创建顶层窗口和原生外壳；显式添加 `--legacy` 可验证旧式 WGL 路径。这个参数不适用于宿主加载的 EDA 模块。

## 开发应用

应用 DLL 使用公共头文件，链接共享框架的 `ui_framework_runtime.lib`，导出 `ui_app_query_v1`，实现实例生命周期并注册 UI 和语义命令。CMake 模块链接 `ui_framework_shared` 可获得公共 include 路径和 shared 编译定义。应用不重复静态链接框架，也不携带替换宿主的 `ui_framework.dll`。

通用组件通过 [components.h](include/ui_framework/components.h) 注册，框架复制界面描述和数据，使用 [images.h](include/ui_framework/images.h) 发布 RGBA/PNG 图片。默认图片资源上限每实例 32 MiB，新数据/图片投递队列 8 MiB，超限明确报错；详见[接入约定](docs/generic-web-ui.md)。

在 `mount` 中通过 [shell.h](include/ui_framework/shell.h) 获取借用的 shell 和内容槽，挂载原生/OpenGL surface 或 Web view；在 `unmount` 中释放应用内容。`ui_shell_refresh()` 更新注册项并保留已有内容容器。旧 `ui_native_shell_refresh()` 的重建语义仍保留，调用后必须重新创建面板内容。

应用包是版本化、未压缩的单文件容器，由清单、DLL 和资源组成。构建产生的 EDA staging 目录可直接重新打包：

```powershell
& .\build\web-shell\uapp_pack.exe .\examples\minimal_eda\manifest.ini .\build\web-shell\eda_package .\build\web-shell\minimal_eda-copy.uapp
```

实例私有状态由应用分配和释放；宿主管理框架对象。工作线程只通过复制投递接口交付结果、事件、进度和关闭完成通知。完整关闭顺序、回调 scope 和模块卸载要求见[开发标准](docs/application-development-standard.md)。

## 已有应用如何升级

在应用项目中记录框架仓库地址、已适配的 SDK 标签或 commit、开发标准修订号及 API/ABI。SDK 记录不放进 `.uapp` 清单：包格式 1 严格接受已有八个字段，额外字段会被拒绝。

已有框架 clone 可更新 `main` 并记录新的开发源码基准：

```powershell
git switch main
git pull --ff-only origin main
git rev-parse HEAD
```

符合 v0.1.0 公共接口约定、通过框架注册 UI 并在内容容器中挂载的 API 1 二进制包，属于无需重新编译的兼容目标。直接安装/修改 Win32 菜单、枚举原生工具栏或依赖外壳窗口类的应用必须迁移。独立宿主只提供 Web 外壳；原生嵌入式 API 保留。

升级时先保存原应用包，对照[迁移指南](docs/migration-v0.1-to-v0.2.md)核查依赖，并对照 [0.2 → 0.3 指南](docs/migration-v0.2-to-v0.3.md)，在目标宿主验证加载、多实例、助手路由、DPI/绘制和卸载。采用 API3 新组件或重新构建应用时，同步清单与 DLL 的 API 版本。旧包无需修改的兼容目标与本次实际验收结果分别记录，不能用重新编译的示例代替原二进制兼容证据。

## Web 后端和能力边界

默认外壳使用 Lexbor v2.5.0 对应 commit `7fb22cf5664a331d7c24b113489e566767c9c25a` 与 QuickJS-NG commit `2f0aa72a6b09cf69ff06399bcf9f4c083ac1a278`。后端具备受控动态 DOM、基础 flex/absolute 布局、按钮/文本输入、JSON 双向消息与 GDI 绘制；完整支持清单见[开发标准](docs/application-development-standard.md#92-轻量受控后端的实际子集)。它不提供完整浏览器兼容性。

应用需要现代浏览器内容时，可另行启用 `UI_FRAMEWORK_ENABLE_WEBVIEW2=ON`，准备固定 WebView2 SDK `1.0.4129.50` 并部署 Runtime；该开关默认 OFF，不改变独立宿主的轻量 Web 外壳。TypeScript 需要离线编译为 JavaScript，框架不直接执行 `.ts`。

本轮完整可选构建测试为 27 通过、1 跳过；跳过为真实跨显示器条件。实际宿主已验证两实例、十万行/十万节点、异步图片、草稿、实例模态和旧应用混合加载；实际中文 IME、物理跨屏及长时间手动滚动仍未验证，见[API3 验收记录](docs/validation/api3-validation.md)。

应用在同一进程执行，应用崩溃可能影响宿主。助手 UI 尚未连接模型服务。框架不实现 EDA、Markdown、画板、PPT 或电子表格的完整文档模型，也不直接加载任意现成 EXE。

## 许可证

框架本身的许可证尚未选定。第三方组件有各自的许可证和再分发条款，来源与文件说明见[依赖许可](docs/build-and-validation.md#7-依赖许可和分发)。
