# CHANGELOG

## 0.7.0 — 本地开发交付，未创建稳定标签

API7/标准7，支持API1–7，ABI1/包格式1保持。轻量overflow新增框架绘制横纵条，共同树/表格/列表新增完整数据范围条、宽表横条、uint64精确映射和捕获失效处理，保留分页/预算。连续RGBA/透明度预览及文本沿用草稿/一次业务提交。

底部停靠与同区标签复用内容槽和GL；布局格式2读取1、完整异常验证、缺失/新增面板恢复和reset。ui_panel_layout在48/56追加字段，旧48前缀兼容；原host布局大小/COUNT8不改，BOTTOM9/CANCEL8追加。冻结SDK6及原API1–6包分别验证。

修复关闭WAIT期间仍启动Runtime环境创建，保留内部文档确认/延后Close/BrowserProcessExited合同。原资源阈值不改，保留复现、中间失败和独立连续周期证据；API5基线与最终都要求框架关闭排空；修复异步模态先聚焦容器、文档就绪后漏转交Runtime的缺口，只转交仍有效的当前焦点。详情见[稳定性](docs/validation/runtime-stability-validation.md)和[本轮验收](docs/validation/api7-validation.md)。

四配置CI继续，增加真实Runtime滚动和API7独立DLL必跑项。本轮未推送；整机无登录/真实IME/物理跨屏/桌面合成/长期人工及业务试点不继承历史通过。没有硬件无窗口GL范围扩展。迁移见[0.6→0.7](docs/migration-v0.6-to-v0.7.md)。

API7交付收敛：修正当前文档仍称标签式停靠/颜色拾取未实现及API6接手提示；CI记录精确清单/二进制与原包缺项，拒绝空测试/遗漏/重复/异常跳过，原API5/6 Runtime用例进入相应阶段。修复关闭WebView2时滚动测试仍引用可选函数的链接错误，仅测试编译条件变化，API7/ABI1不变。[实测和保留失败](docs/validation/api7-delivery-validation.md)、[业务试点准备](docs/business-pilot.md)分别交付；未授权修改业务应用或触发远程工作流。

## 0.6.0 — 开发版，未创建稳定标签

API6/标准6，运行库接受API1–6，ABI1/包格式1保持。追加应用负责存储的布局格式1、稳定面板ID、恢复/重置、约束分隔条与左右栈/浮动拖拽，内容槽和GL context沿用原生命周期；Windows无窗口提供逻辑布局。

共同轻量/WebView2模板实现枚举下拉、RGBA选择与文本校验、表格键盘/提交取消、完整数据源排序和稳定ID多选/异步范围。修复Runtime键盘事件目标、DOM重排焦点、非法颜色CSS阻止校验，以及新增描述初始化失败清理；原资源门槛保持。

**兼容**：冻结SDK5与原断言，追加完整字段读取；旧API1–5调用方/原包分别验收。旧表格点击编辑保留，API6表格体验显式启用。新DLL和清单同步声明6，ABI/导出/包格式无需升级。

新增独立API6测试.uapp、真实Runtime/OSMesa组合回归与四种Windows C11 CI。独立框架证据不代表请求方业务试点；Session0已有成功，整机无登录、物理/长期人工项目仍待验。见[迁移](docs/migration-v0.5-to-v0.6.md)、[布局](docs/workspace-layout.md)、[验收](docs/validation/api6-validation.md)。

补验修复工具提示可见时裸Alt被吞；Runtime取消时仅等待所启动内部空白导航ID完成，忽略初始about:blank并只释放一次事件所有权，等待完成文档的renderer确认，在UI消息中停止导航并延后Close。测试比较同样已排空的生命周期，并要求STA内部清理归零后退出；原资源门槛保留，新增快照诊断引起的目标异常也保留记录，失败与最终结果见验收。Runtime桥接最低138.0.3351.48；临时CI虚拟桌面、显式软件WGL与Runtime平台图形分阶段记录，不计作物理或硬件验收。

菜单内部修正：菜单文字入口、工具和标签分开样式；连续行对齐勾选、图片、标题、注册快捷键及箭头，支持两主题。新增顶层悬停切换、侧向展开/边缘翻转、跨子层宽限、外点关闭与重复点击关闭；保留Alt/焦点/一次命令、原分页/128项/资源门槛。公共API6/ABI1不变，无需重编应用。新增真实宿主及无窗口菜单边界测试；[失败、截图和回归](docs/validation/menu-desktop-validation.md)分别记录，未修改请求方应用。

## 0.5.0 — 开发中，未创建稳定标签

API5/标准修订5，接受API1/2/3/4/5，ABI1和包格式1不变。新增显式菜单助记键、正常输入路由与重复键循环；新增显式OSMesa内存上下文入口，准确区分NO_WINDOW与隐藏WGL；离屏支持精确MSAA附件、resolve和RGBA读回；可选WebView2提供实际Runtime呈现查询、捕获、C图片及共同组件模板/数据/语义命令/输入。

**兼容**：完整保留SDK4前缀及padding，原SDK1/2/3不变；旧WebView2配置默认native_handle为空，组件承载须显式开启。新增异步正状态PENDING，调用方正常泵消息后重试，框架不保留调用方输出缓冲；新应用DLL/清单一致声明API5。

**依赖与限制**：实际无窗口提供方是显式固定的x64 OSMesa软件GL，应用所有GL函数经surface解析。实际CI LocalSystem Session0的测试DLL/OSMesa已补验；整机无登录、真实IME/跨屏/长期人工压力仍待验。WebView2是窗口承载Runtime，不等同浏览器无窗口；关闭资源断言及全阶段状态见[API5验收](docs/validation/api5-validation.md)，未通过的项目不能宣称完整交付。

**Session0补验**：新增独立测试DLL、实际frame/输入/resize、临时服务及严格无登录CI；200轮400实例1200frame，HWND0、句柄148→148。修复MSVC19.44/WX下包架构常量警告，位宽合同与ABI不变；测试日志支持无继承控制台。托管runner登录会话1导致无登录门槛失败，缺专用runner；原始像素、资源及失败记录见[专门验收](docs/validation/session0-osmesa-validation.md)。

迁移与构建见 [0.4→0.5](docs/migration-v0.4-to-v0.5.md)及[构建说明](docs/build-and-validation.md)。

## 0.4.0 — 开发版，未创建稳定标签

框架API4/标准修订4，运行库接受API1/2/3/4，ABI1/包格式1不变。分组菜单、独立Web弹层、工具溢出及TREE标题复用原注册、命令、状态和内容槽；移除组件内部旧菜单。新增真实轻量Web/组件输入、呈现查询、RGBA截图及flush；新增显式无HWND workspace和隐藏WGL/FBO读回。

**兼容变化**：旧尺寸及枚举保持，冻结SDK3；原API3包实际复验。NULL parent默认workspace仍无效，离屏必须显式选择。WebView2缺少新增诊断/组件能力时返回UNSUPPORTED。

**可选升级**：应用采用menus.h、COMPACT及离屏测试接口时升级SDK/API声明；旧包可留原API。框架外壳快捷键变为Ctrl+Shift+O，应用Ctrl+O单独路由。

**必须核对**：依赖旧私有node-menu DOM或严格校验菜单params的应用需调整到公共锚点入口；菜单源menu，params保留id并追加target。使用新函数的DLL/清单一致声明API4，链接匹配运行库。

**补验修复**：浮动Web组件的原生输入执行同一实例模态/激活/关闭门控，回到停靠后保留原编辑窗口焦点；自动STYLE预览在换页、回填及注销后回收，避免在字节预算内积累位图并耗尽GDI对象。应用提供的图片ID保持借用，公共ABI不变。新增真实宿主编辑布局/资源压力回归，证据见[API4补验](docs/validation/api4-validation.md#5-2026-10-04-接手补验与修复)。

第一阶段GL离屏仍依赖隐藏HWND/DC，拒绝core/legacy/MSAA降级；不提供完全无窗口GL或Session0承诺。Alt访问键、真实中文输入法、物理跨屏、长时压力及请求方业务应用接入边界见[API4验收](docs/validation/api4-validation.md)。详见[迁移](docs/migration-v0.3-to-v0.4.md)。

## 0.3.0 — 开发版，未创建稳定标签

框架 API 3 接受 API1/2/3，标准修订3；应用 ABI1、ui_app_query_v1、包格式1不变。实现基准为 3375459ca2d85d0627e3072e3f1e83fc4fa84aae。

- 新增 components.h/images.h：框架模板、按需树、虚拟窗口表格/列表、属性、Web 对话框/状态、RGBA/WIC PNG、样式预览、异步缩略图及有界资源/投递。
- 轻量后端移除 EDIT 路径，采用 Uniscribe/IMM32 平台适配的 Web 编辑；保留原配置布局，enable_native_input 废弃且忽略。
- 原注册机制追加图标、快捷键、命令/界面状态；宿主按稳定 ID 增量刷新，保留数据、草稿与内容槽。
- 提供纯 C generic_components.uapp 和核心/图片/编辑/真实宿主测试；冻结 SDK2 头文件及原测试合同，不提高 DOM/脚本预算代替按需实现。

**兼容变化**：API1/2 包继续接受，结构和枚举原值保持，原生嵌入式保留。原 API1 二进制包及 SDK2 调用方验证通过。

**可选升级**：旧应用采用通用 Web 组件时改用 API3 公共头文件，同步清单与 DLL；不要求旧包转换为新组件。

**必须迁移**：依赖轻量后端 EDIT 子窗口、原生输入开关或其窗口消息的应用，改用公共 Web 输入/消息。新组件业务界面必须 Web，旧应用自有原生内容单独识别。

完整可选测试 27 通过、1 跳过，依赖之外的干净原生兼容构建通过。中文 IME 实操、物理跨显示器与长时间手动压力未验证。WebView2 不提供新增 C 图片资源/框架组件呈现；详细子集和结果见[能力清单](docs/generic-web-ui.md#7-能力清单)、[验收](docs/validation/api3-validation.md)和[迁移指南](docs/migration-v0.2-to-v0.3.md)。

## 0.2.0 — 开发版，尚未发布稳定标签

当前源码的 SDK 版本为 0.2.0，开发标准修订为 2；最近稳定标签仍是 `v0.1.0`。框架 API 升为 2，运行库接受 API 1 和 2；应用 ABI、`ui_app_query_v1` 入口及包格式仍为 1。清单与 DLL 的 API 声明必须一致。

### 变化

- 独立宿主改用 Lexbor/QuickJS-NG/GDI 的 Web 外壳，提供标签、应用菜单、工具入口、面板外框、全局助手和浅色/深色切换，保留 Windows 标准标题栏。
- Windows 默认开启独立宿主和轻量 Web 后端；首次构建需要另行准备固定 `.deps/`。原生嵌入式构建需同时关闭 `UI_BUILD_STANDALONE_HOST` 和 `UI_FRAMEWORK_ENABLE_LIGHT_WEB`。
- 新增 [shell.h](include/ui_framework/shell.h) 的借用 shell/content-slot 接口；`ui_shell_refresh()` 增量更新注册项并保留已有内容容器。
- 新增统一 Web JSON 消息、能力位和借用 native handle 接口；轻量后端扩展受控动态 DOM、样式/布局、事件与文本输入。
- 新增 API 2 的 [Web Counter](examples/web_counter/app.c) 纯 Web 内容示例及 API 1/2、内容槽、动态 DOM、Web 宿主验证目标。旧应用 fixture 使用冻结的 `tests/sdk_v1` 头文件。

### 兼容性与迁移

**无需修改的兼容目标**：符合 v0.1.0 公共接口约定、通过框架注册菜单/工具栏/面板并在内容容器挂载的 API 1 二进制 `.uapp`。应用继续选择原生、OpenGL 或 Web 内容；标签切换与停靠/浮动保留内容容器和 GL context。原生嵌入式 API 保留。

**可选升级**：重新构建或采用 API 2 时，同步清单与 DLL 的 API 声明，使用新 shell/content-slot 和 JSON 消息接口。自定义 Web backend 可追加能力回调；旧 ops 尺寸继续接受，缺少新增可选回调返回 UNSUPPORTED。

**必须迁移**：依赖独立宿主 Win32 外壳内部结构的应用，包括直接安装/修改 HMENU、枚举 Common Controls toolbar 或依赖外壳窗口类。独立宿主不提供原生外壳选项。旧 `ui_native_shell_refresh()` 仍重建面板并使旧容器/内容句柄失效，调用方继续遵守原有重建约定。

具体步骤见[从 v0.1.0 迁移到 0.2.0](docs/migration-v0.1-to-v0.2.md)。默认 Web 与 native-only 的干净构建成功；原 EDA 二进制兼容、6 项新增集成测试及真实浅色/深色界面通过。完整可选 CTest 为 21 通过、2 失败、1 跳过；失败为保留的旧版本/子集断言，跳过为单屏硬件条件，详见[当前迁移验收](docs/build-and-validation.md#51-020-开发版本迁移验收)。本节不宣称原测试全绿或稳定版本已发布。

## v0.1.0 — 2026-10-02

首个公开 SDK 基准。框架 API 1、应用 ABI 1、应用包格式 1；应用开发标准修订 1。

### 当前能力

- Windows x64 独立宿主、`.uapp` 验证和加载、多实例标签、活动应用菜单及工具栏。
- Win32 侧栏、悬浮窗、主区优先布局、Per-Monitor DPI 和 OpenGL 内容区。
- 应用生命周期、复制的异步结果/事件/进度、允许/拒绝/等待关闭与完整 DLL 卸载。
- 按实例路由的助手协议验证面板、权限确认、取消、快照及应用事务接口。
- 可选受控 Lexbor/QuickJS-NG 和 WebView2 后端；保留静态库及嵌入式模式。
- 独立最小 EDA 应用包、公共 C/C++ 接口样例和集成验收测试。

### 兼容性与迁移

这是首个公开标签，没有更早的公开 SDK 迁移路径。发布前开发的应用按[该标签的开发标准](https://github.com/wbycloud/ui-framework/blob/v0.1.0/docs/application-development-standard.md)与头文件核查，并记录适配基准。

当前加载器严格检查 API/ABI 版本；尚未实现多版本兼容协商或自动迁移工具。异步关闭、内存所有权和应用自有 UI 回调 scope 是必须遵守的使用约定。助手尚未连接模型服务，真实跨显示器行为需要多屏硬件验证。

该版本的干净源码构建成功，原生测试为 15 通过、1 失败、1 跳过；窗口尺寸失败经纯 Win32 对照确认为本机最大窗口跟踪高度限制，真实跨显示器测试因单屏跳过。实际条件见[历史验证记录](docs/build-and-validation.md#52-v010-干净源码验收)。

## 后续版本记录规则

每个稳定标签维护对应的版本说明与开发标准修订号，并列出影响的接口、行为、适用版本和验证方法：

- **兼容变化 / 无需修改**：保留公共接口和使用约定，说明旧应用的验证结果。
- **可选升级**：新增可选能力，说明采用新能力需要的 SDK 与改动。
- **必须迁移**：破坏接口或必要约定，提供从受影响基准到新版本的具体迁移步骤。

SDK 发布版本、API/ABI、应用包格式及开发标准修订分别管理；以实际校验规则和迁移说明判断兼容性。
