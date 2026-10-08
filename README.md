# Windows C/Web UI Framework

面向 Windows x64 的轻量 C 应用框架。独立宿主 `framework_host.exe` 加载 `.uapp` 应用包，以浏览器风格的 Web 外壳提供多应用标签、菜单、工具入口、侧栏外框和全局助手。外壳使用 Lexbor、QuickJS-NG 与 GDI 实现受控 HTML/CSS/JS 子集，支持浅色/深色切换；第一行为融合标题区的实例标签栏，第二行为活动应用菜单。[宿主操作与最近记录](docs/standalone-host.md)。框架自有界面采用紧凑中性浅深主题，共同组件同步宿主主题，AI每次启动默认收起。[视觉规范](docs/visual-design.md)与[本轮完整窗口真实截图/回归](docs/validation/component-experience-validation.md)。

新应用通过公共 C ABI 注册框架 Web 组件、提供数据并绑定语义命令；绘图内容按需要使用 OpenGL 内容槽，文档和业务逻辑由应用维护。当前API9应用可挂载自有原生内容。框架同时保留静态库及应用自行创建窗口的原生嵌入式模式。WebView2 是可选的应用内容后端。

## 当前版本与开发入口

| 项目 | 当前开发值 | 定义位置 |
| --- | --- | --- |
| SDK | `0.9.0` 开发版，尚未发布对应稳定标签 | [CMakeLists.txt](CMakeLists.txt) |
| 最近稳定标签 | `v0.1.0` | [稳定源码](https://github.com/wbycloud/ui-framework/tree/v0.1.0) |
| 框架 API | `9`；只维护和验收当前版本，现存低版本加载行为不作未来保证 | [ui.h](include/ui_framework/ui.h) |
| 应用 ABI / 包格式 | `1` / `1` | [application.h](include/ui_framework/application.h)、[package.h](include/ui_framework/package.h) |
| 开发标准修订 | `9`，对应当前 `0.9.0` 开发源码 | [应用开发标准](docs/application-development-standard.md) |

当前工作源码为 0.9.0 开发版，不创建稳定标签。采用开发版本须记录确切 commit；v0.1.0仅是历史稳定标签。[当前版本政策](docs/version-policy.md)撤销持续旧版兼容承诺。清单与 DLL 的 API 声明必须一致，应用不能加载到不支持其声明 API 的旧运行库。

本轮开发保留本地提交，未推送；GitHub main不代表这些本地变更。取得源码后记录实际 commit，再读[API9验收](docs/validation/instance-language-validation.md)、[0.8 → 0.9迁移](docs/migration-v0.8-to-v0.9.md)和[应用升级提示词](docs/application-upgrade-prompt.md)。API3/4/5历史记录保留原结果，当前菜单及离屏合同见[接口说明](docs/framework-menu-offscreen.md)。

API8 后续收敛提供[完整应用存储示例](examples/stateful_components/README.md)：应用自己保存 UCW1 列宽与工作区布局，稳定 A/B 档案跨进程恢复、锁隔离、显式保存／重置和异常拒绝。共同表格按稳定列 ID 对齐，调宽改变可见列数时保留指针捕获。[此前API8收敛证据](docs/validation/api8-stability-state-validation.md)分别记录重绘诊断、实际 Runtime、业务原固定坐标失败及 SDK；历史持续重绘尚不能宣称根治。

此前API7[交付收敛与CI审计](docs/validation/api7-delivery-validation.md)区分本机运行、托管CI和待验条件；真实应用准备见[业务试点](docs/business-pilot.md)。当前本地API9提交尚未发布，新clone必须先核对`UI_FRAMEWORK_API_VERSION=9`和CMake版本0.9.0；不符合时取得维护方明确提供的API9 commit后再接入，不能把main或稳定标签当作该提交。

建议按以下顺序阅读：

1. [CHANGELOG](CHANGELOG.md)：版本变化和兼容性分类。
2. [0.8 → 0.9迁移](docs/migration-v0.8-to-v0.9.md)：当前API9；[0.7 → 0.8](docs/migration-v0.7-to-v0.8.md)：此前列宽/help；[0.6 → 0.7](docs/migration-v0.6-to-v0.7.md)：此前滚动/停靠/颜色；[0.5 → 0.6 迁移](docs/migration-v0.5-to-v0.6.md)：布局和组件交互；[0.4 → 0.5](docs/migration-v0.4-to-v0.5.md)：Alt、OSMesa、MSAA和Runtime；更早版本按原API阅读相应迁移指南。
3. [应用开发标准](docs/application-development-standard.md)：生命周期、线程、所有权、DPI、内容后端与助手约定。
4. [构建与验证](docs/build-and-validation.md)：固定依赖、构建开关、测试和发布检查。
5. [通用 Web UI](docs/generic-web-ui.md)及[纯 C 示例](examples/generic_components/README.md)：框架管理的树、表格、表单、对话框、缩略图和绘图内容槽。
6. [Web Counter](examples/web_counter/app.c)及其[清单](examples/web_counter/manifest.ini)：使用当前SDK/API9的纯Web内容示例；[最小 EDA](examples/minimal_eda/app.c)展示当前原生/OpenGL内容路径。
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

当前菜单/工具溢出示例使用 `framework_features.uapp`；可打开两个实例。通用示例也以当前SDK/API9构建：

```powershell
& .\build\web-shell\framework_host.exe .\build\web-shell\generic_components.uapp .\build\web-shell\generic_components.uapp
```

示例无需编写组件 HTML，展示按需树、表格/复选框/图片/样式、单位和混合值表单、Web 参数对话框、后台缩略图及绘图浮窗。菜单“Web 对话框”或 Ctrl+D 打开参数界面；Enter 提交表格编辑，Esc 取消；表格 Shift+滚轮切换列。它的 OpenGL 三角形仅为内容槽验证，不是框架业务渲染器。

EDA 需要驱动支持 OpenGL 3.3 compatibility profile。点击画布后，`A` 添加矩形、`Z` 放大、`C` 清空。不同实例的数据、缩放和属性笔记独立保存。`Ctrl+Shift+O` 打开应用包，`Ctrl+W` 关闭标签，`Ctrl+Tab` / `Ctrl+Shift+Tab` 切换标签。全局助手点击AI展开，普通视图显示目标、可读操作、基本参数、执行状态及结果；schema、原始JSON、快照、事务和详细日志位于可折叠高级区。危险命令经过权限和确认接口。

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

Session0／无登录 Windows CI使用独立实际应用DLL、OSMesa frame和资源回收测试，入口及环境硬性检查见[专门验收记录](docs/validation/session0-osmesa-validation.md)。CTest的 `ui_session0_interactive_control` 仅为准备性对照，目标结果单独记录。

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

本轮起只维护接手确认的当前版本。历史SDK/API、旧调用方和旧二进制包不再保证兼容，也不作为CI必跑项；冻结SDK、原包与历史结果不改。当前仍接受部分旧声明属于现存实现，不是持续承诺。原生嵌入式等当前基础接口仍受维护。

接入当前维护版本应锁定确切commit，使用匹配的API9头文件、导入库及运行库，同步清单与DLL声明，在目标宿主验证加载、多实例、助手、DPI/绘制和卸载。历史迁移指南仅供迁移参考，维护范围以[当前政策](docs/version-policy.md)为准。

## Web 后端和能力边界

默认外壳使用 Lexbor v2.5.0 对应 commit `7fb22cf5664a331d7c24b113489e566767c9c25a` 与 QuickJS-NG commit `2f0aa72a6b09cf69ff06399bcf9f4c083ac1a278`。后端具备受控动态 DOM、基础 flex/absolute 布局、按钮/文本输入、JSON 双向消息与 GDI 绘制；完整支持清单见[开发标准](docs/application-development-standard.md#92-轻量受控后端的实际子集)。它不提供完整浏览器兼容性。

应用需要现代浏览器内容时，可另行启用 `UI_FRAMEWORK_ENABLE_WEBVIEW2=ON`，准备固定 WebView2 SDK `1.0.4129.50` 并部署 Runtime；该开关默认 OFF，不改变独立宿主的轻量 Web 外壳。TypeScript 需要离线编译为 JavaScript，框架不直接执行 `.ts`。

API3历史完整可选构建为27通过、1跳过；API4新增菜单、真实离屏Web/workspace与隐藏WGL的结果见[API4验收](docs/validation/api4-validation.md)。实际宿主已验证两实例、十万行/十万节点、异步图片、草稿、实例模态和旧应用混合加载；实际中文 IME、物理跨屏及长时间手动滚动仍未验证，见[API3 验收记录](docs/validation/api3-validation.md)。

应用在同一进程执行，应用崩溃可能影响宿主。助手 UI 尚未连接模型服务。框架不实现 EDA、Markdown、画板、PPT 或电子表格的完整文档模型，也不直接加载任意现成 EXE。

## 维护与接手

开发者先读[框架开发交接记录](docs/handoff.md)，核对实际 commit、构建目录、证据和 UV-01..07 待补验项。本轮提交保留本地，未推送；测试通过与全部人工验收完成分别记录。

## 许可证

框架本身的许可证尚未选定。第三方组件有各自的许可证和再分发条款，来源与文件说明见[依赖许可](docs/build-and-validation.md#7-依赖许可和分发)。

## API6 工作区与组件交付

支持应用存储的版本化布局、稳定面板ID、左右栈/浮动/折叠状态、重置、约束分隔条和拖拽预览/提交/取消；内容槽、有效草稿、选择、焦点和GL context沿用原生命周期。Windows无窗口workspace提供逻辑布局与手势，不创建HWND。[布局合同](docs/workspace-layout.md)说明格式与适用范围。

共同组件模板增加枚举下拉、颜色选择与RGBA文本、表格键盘导航及Enter/F2编辑、Enter提交/Esc取消、完整数据源排序、稳定ID多选与范围查询。轻量和显式WebView2组件后端使用相同语义；资源预算保持原值。[通用组件](docs/generic-web-ui.md)及[迁移](docs/migration-v0.5-to-v0.6.md)说明应用责任。

四种Windows x64/C11配置由[CI矩阵](.github/workflows/windows-regression.yml)定义；本轮本机实际运行与未执行的目标CI分别记录。WebView2必须启动实际Runtime，OSMesa执行真实测试DLL的GL frame。历史Session0 GL已有成功证据，当前源码须另验；整机无登录仍待专用runner，物理IME/跨屏/桌面合成/长期人工与真实业务试点未验收。独立测试应用不代表请求方业务应用验收。历史结果见[API6记录](docs/validation/api6-validation.md)，此前API8见[记录](docs/validation/component-experience-validation.md)。

菜单外观、侧向鼠标交互与只读前后图见[桌面菜单验收](docs/validation/menu-desktop-validation.md)，公共API6/ABI1不变。

## API7 既有能力

轻量overflow与共同TREE/TABLE/LIST提供可拖动的完整数据范围滚动条；宽表按全部列宽横向导航；共同数据组件不再显示上一页/下一页或列箭头按钮，视口回收原操作栏空间。底部停靠、同区标签组与读取格式1的布局格式2复用内容槽和GL context。连续RGBA选择与文本使用同一草稿/提交合同。关闭稳定性失败、资源诊断和6次独立64轮复验见[记录](docs/validation/runtime-stability-validation.md)，此前API8源码及限制见[验收](docs/validation/component-experience-validation.md)。

本轮[分页清理与当前版本验收](docs/validation/component-scroll-only-validation.md)提供真实前后截图及本地矩阵；菜单、最近应用、标签溢出的分页保持原合同。


原生共同组件滚动首末项与宽表捕获修复见[真实鼠标、业务应用和SDK交付记录](docs/validation/native-component-scroll-validation.md)。该历史修复使用API7匹配产物；当前API9需使用本轮匹配头文件/导入库/运行库/宿主重建；应用仍提供真实total_count与按需批次，本次无需改业务源或预算。

## API8 组件与工作区体验

表头拖动调宽、显式缓存适配、单列/全部重置及UCW1稳定ID状态恢复；应用仍决定初始列宽并负责存储。可选字段帮助、窄特殊列、长文本F1/悬停、128项长枚举、响应式RGBA、长面板/停靠标签查看与溢出、浮动最小尺寸、空心预览及模态焦点。框架保留原预算、无分页共同组件、完整范围滚动和GL/关闭合同。[迁移](docs/migration-v0.7-to-v0.8.md)、[本轮实际证据](docs/validation/component-experience-validation.md)。


## API9 应用实例语言

应用决定并持久化语言，每个host独立支持zh-CN/en-US。宿主跟随活动实例，无应用时跟随Windows显示语言。新增设置、来源查询、通知及原位文本更新能力，使用匹配SDK0.9/API9；不重建DLL、工作区或GL。完整示例通过A/B档案演示语言与原列宽/布局状态的独立持久化。[语言合同](docs/instance-language.md)、[当前验收](docs/validation/instance-language-validation.md)、[完整示例](examples/stateful_components/README.md)。
