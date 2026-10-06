# 框架开发交接记录

更新日期：2026-10-07。当前原生滚动两处修复源码为2e87e02，上一单元9df202f；[本轮记录](validation/native-component-scroll-validation.md)单列原生真实鼠标与当前业务应用结果，不能用此前离屏结果替代。当前接手确认SDK0.7/API7/标准7；本轮共同组件分页清理见[验收](validation/component-scroll-only-validation.md)及第14节。共同数据组件只保留完整范围滚动，底层按需批次不变；菜单、最近应用及标签溢出的分页仍保留。从本轮起执行[当前版本维护政策](version-policy.md)，历史SDK/API/调用方和原包不再保证兼容或列入专项回归。本文第3–5、7–13节保存历史身份与结果，不能当作当前承诺或运行证据。现行基础功能、size/线程/所有权/卸载合同继续维护。当前Session0、严格无登录runner、真实IME/物理与长期人工仍分别待验，本机运行不冒充托管CI或完整业务验收。

## 1. 恢复顺序与版本

1. 检查 `git status --short`、`git branch --show-current` 和 `git log -3 --oneline`；先保留接手时的用户改动。
2. 阅读本文、[应用开发标准](application-development-standard.md)、[API7验收](validation/api7-validation.md)、[Runtime稳定性](validation/runtime-stability-validation.md)、[构建与CI](build-and-validation.md)、[API6验收](validation/api6-validation.md)、[API5验收](validation/api5-validation.md)、[API4记录](validation/api4-validation.md)及[API3未验证项](validation/api3-validation.md#41-未验证项目与补验清单)。
3. 修改代码前核对[公共头文件](../include/ui_framework/ui.h)、[布局](workspace-layout.md)、[菜单/离屏合同](framework-menu-offscreen.md)、[通用Web](generic-web-ui.md)、[0.6→0.7迁移](migration-v0.6-to-v0.7.md)及相关测试。无需重做已完成的实现。

| 项目 | 交接状态 |
| --- | --- |
| 仓库 | [wbycloud/ui-framework](https://github.com/wbycloud/ui-framework) |
| API5功能代码 | [87478fa20d7bb46809c0ef81dd44f972dc193a24](https://github.com/wbycloud/ui-framework/commit/87478fa20d7bb46809c0ef81dd44f972dc193a24)，四项实现及回归；文档另行提交，实际HEAD以Git为准 |
| 浏览器式宿主 | d7030b3：单行标签标题栏、第二行应用菜单、原Windows/关闭合同及12项最近成功包；完整结果/限制见第12节，文档提交不改执行代码 |
| API7当前交付 | 产品源码2c0c52b、原生测试cb42660；本次CI/测试修正ae4dc7f、2fde15e、e4a33bf；运行身份和待验见第11节及收敛记录，实际HEAD以Git为准 |
| API6实现/最终测试与CI条件 | [1cdc50e](https://github.com/wbycloud/ui-framework/commit/1cdc50e)、[9705565](https://github.com/wbycloud/ui-framework/commit/9705565)，完整提交序列及失败修复见API6验收；文档提交不改运行代码 |
| 菜单后续本地实现 | `6bf59f8`、`2917b44`、`d907fbd`、`d2e4a61`及测试收敛`b1f4c7b`；最终产品源码d2e4a61，仅内部菜单改动，API6/ABI1不变；最终证据另提交 |
| API4历史实现/补验 | [e83a008](https://github.com/wbycloud/ui-framework/commit/e83a0087f0ef1017c9cd99db3e2f6754b503311c)、[48d6aaf](https://github.com/wbycloud/ui-framework/commit/48d6aafa451802b2a057fa68ce1f687fb2f2a004)，模态原生输入、STYLE回收、redock焦点；[记录](validation/api4-validation.md#5-2026-10-04-接手补验与修复) |
| API6阶段最后核对的远端分支（历史） | `main`仍为`ea5b108`，`codex/menus-offscreen`为`9705565`；最终API6证据`559b802`已本地提交，未推送；交付状态修订另行提交，实际HEAD以Git为准 |
| SDK / 框架 API / 标准修订 | `0.7.0` 本地开发版 / `7` / `7` |
| 维护范围 | 只维护和验收本轮确认的API7；运行库现存API1–7接受行为暂存，不作未来旧包兼容保证；清单与DLL声明必须一致 |
| 应用 ABI / 导出入口 / 包格式 | `1` / `ui_app_query_v1` / `1` |
| 稳定标签 | 只有 `v0.1.0`；未创建新的稳定标签 |

API6本轮起点为干净ea5b108，与当时origin/main及工作分支一致，没有需覆盖的用户改动；API5起点1f77b2e属于历史记录。后续以Git核对实际HEAD，应用应锁定SDK commit。

## 2. 长期要求与责任边界

第一平台为 Windows x64，框架使用 C11，应用接口为公共 C ABI。独立宿主加载多个 `.uapp`，以标签切换实例；当前API7原生嵌入式接口及应用内容继续维护。UI 外壳及新通用业务组件由框架维护 HTML/CSS/JavaScript，应用提供描述、数据和语义命令。OpenGL 是否使用由应用决定。

新业务 UI 不使用 TreeView、ListView、Button、Edit、MessageBox 或隐藏 EDIT 代理；系统文件/目录选择器是例外。Win32 可用于窗口、消息、IME、字体和绘制适配。框架不拥有具体应用的文档模型、算法或业务渲染器，不是完整浏览器，也没有连接大模型服务。

优先复用原菜单、工具、面板、内容槽、命令、队列和生命周期。新增接口不暴露第三方私有类型；保持旧字段偏移、枚举值和默认行为，新字段按 size 判断。64位身份在 JSON 中为十进制字符串，图像像素通过 C 接口复制传递。线程、所有权和 DLL 卸载合同不能绕过。

文档面向 C/C++ 应用开发者，不署名；给其他应用的[升级提示词](application-upgrade-prompt.md)只使用 GitHub 入口，不包含个人本地目录。历史任务曾授权推送，API6阶段最终推送曾被自动审批拒绝（第8节）；该拒绝属于历史记录。本轮API7收敛没有新的推送或远程执行授权，先保留本地提交。不要强推或创建新的稳定标签。

交接准备及后续补验未启用 graph-engineering，也未修改请求方 KLayout C 应用、冻结应用包或 PERF-001。接手时不要把应用适配或性能重新测量自动扩展进框架任务。后续已完成本机自动补验和三项针对性修复，没有新建会话或移动当前会话。

## 3. API4历史实现与定位

| 能力 | 公共接口 / 实现 / 验证入口 |
| --- | --- |
| 分组菜单、公共弹窗、工具溢出 | [menus.h](../include/ui_framework/menus.h)、[menus.c](../src/menus.c)、[menu.html](../src/menu.html)、[宿主 HTML](../examples/framework_host/host.html)、[宿主 C](../examples/framework_host/main.c) |
| 树标题、通用组件及离屏模板 | [components.h](../include/ui_framework/components.h)、[components.c](../src/components.c)、[components.html](../src/components.html) |
| 实际 Web 命中/输入、呈现查询、RGBA、flush | [ui.h](../include/ui_framework/ui.h)、[轻量后端](../src/backends/light_web.c)、[menu_offscreen 测试](../tests/menu_offscreen.c) |
| 显式无 HWND workspace | [application.h](../include/ui_framework/application.h)、[workspace.c](../src/workspace.c)、[Win32 shell](../src/platform/win32_shell.c)、[workspace_offscreen 测试](../tests/workspace_offscreen.c) |
| 第一阶段隐藏 WGL/FBO | [opengl.h](../include/ui_framework/opengl.h)、[Win32 surface](../src/platform/win32_surface.c)、[offscreen_gl 测试](../tests/offscreen_gl.c) |
| 通用 API4 应用与真实宿主 | [样例说明](../examples/framework_features/README.md)、[共享样例源码](../examples/generic_components/app.c)、[真实宿主测试](../tests/framework_features_host.c) |
| 旧 ABI / 原包证据 | [SDK3冻结头文件](../tests/sdk_v3/README.md)、[变更前断言](../tests/api3_baseline/README.md)，SDK1/2基线也保留 |

菜单支持根分组、嵌套、状态、HOST/SLOT/ROW 锚点、分页和失效关闭；路径保留4095字节预算，单层最多128项。宿主打开包为 Ctrl+Shift+O，应用 Ctrl+O 独立进入原语义命令。工具可选 COMPACT，树不保留不存在的图标空位。

离屏 Web 使用正常轻量引擎及同一组件模板，未创建 HWND；输入经过正常命中、焦点和脚本。捕获输出不透明顶向下 RGBA8，调用方拥有缓冲；像素尺寸、stride 和预算显式检查。workspace 的默认 NULL parent 仍非法，只有显式 `UI_RUN_OFFSCREEN` 开启无窗口路径；原包加载、资源、复制投递、关闭与卸载继续复用。

GL 离屏显式要求至少3.3 compatibility，仍依赖隐藏 HWND/DC，不接受 legacy/core/MSAA 静默降级。私有 FBO 复用应用 frame/input，RGBA 同步读回并恢复相关 WGL/FBO/viewport/像素打包/PBO 状态。它不是 Session0 或真正无窗口 GL 的交付。

## 4. API4历史验证事实及复现

原交接的验证代码为e83a008，Windows已登录会话、MSVC Build Tools18、Release、单显示器；当时实际离屏驱动返回Intel / Intel(R) UHD Graphics 770 / `3.3.0 - Build 32.0.101.7079`，compatibility profile，没有推断硬件/软件类别。最新补验代码为48d6aaf，结果和资源数据见[新增记录](validation/api4-validation.md#5-2026-10-04-接手补验与修复)。

| 验证 | 结果与边界 |
| --- | --- |
| 完整轻量 + 可选 WebView2 | 最新补验34项，33通过，1个 `ui_monitor_transition` 因物理条件跳过；含新增resilience及本地原API3包。原交接e83a008为33项/32通过/1跳过 |
| 干净源码原生配置 | 未复制 .git/.deps/build；18项，17通过，同一跨屏项跳过 |
| 原 API1/2/3 二进制包混合 | 最新补验再次指定三个原包运行真实宿主，0失败；另保留冻结SDK重新构建的调用方，二者不等同 |
| UV-03/04 自动补验 | 十分钟10155轮、50次重开0失败，GDI峰值2003；六种草稿/选区/撤销/实际焦点组合通过。仍不等于长期人工与完整编辑验收 |
| 新真实路径 | 七组菜单、末项/工具溢出、双实例、普通右键、模板草稿/模态、按需大表格、队列、REFUSE/WAIT、初始化失败、卸载重开、GL读回通过 |
| 公开交付 | main/工作分支 SHA 已核对，README/标准/提示词/迁移/验收的公开文件 HTTP200；文件链接已检查 |

详情和截图在[API4验收记录](validation/api4-validation.md)。程序 DPI、程序中文输入、输入注入和客户区捕获不能证明物理跨屏、真实 IME 或桌面交换呈现。

准备 README 中固定 Lexbor/QuickJS-NG 依赖后，在 x64 Visual Studio 开发终端从仓库根目录执行：

```powershell
cmake -S . -B build/web-shell -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/web-shell
ctest --test-dir build/web-shell --output-on-failure
& .\build\web-shell\framework_host.exe .\build\web-shell\framework_features.uapp .\build\web-shell\framework_features.uapp
```

API4历史口径：新增resilience后，当时默认轻量配置为31项；可选 WebView2为33项。只有设置 `UI_LEGACY_COMPONENT_PACKAGE` 指向原API3包才额外产生第34项，不得把未提供的原包写成已复验。WebView2需固定 SDK1.0.4129.50 和 Runtime；受限环境曾两项创建超时，在允许浏览器子进程环境复验通过，不能把超时记为通过。

原生配置需要同时关闭宿主、轻量后端和 WebView2；不产生 Web 宿主：

```powershell
cmake -S . -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_BUILD_STANDALONE_HOST=OFF -DUI_FRAMEWORK_ENABLE_LIGHT_WEB=OFF -DUI_FRAMEWORK_ENABLE_WEBVIEW2=OFF
cmake --build build/native
ctest --test-dir build/native --output-on-failure
```

API4历史验证曾使用`build/fw-next/release`，菜单阶段同目录重建为API6、配置53项/原生21项（第9节）；当前已重建API7，最终配置及身份见第10节；`build/web-shell` 等旧目录可能残留旧版本，使用前重建，不混用运行库。本地保留 `build/fw-next/ctest-final.log`、`build/fw-next/clean-test.log`、`build/fw-next/build.log`、`build/fw-next/build.cmd` 和原生构建脚本。它们被忽略，不属于远程可获取证据；公开证据以验收文档和可构建测试为准。

本次补验日志另存 `build/acceptance-20261004/`，历史失败日志保留。最终回归日志为 `final-regression.log`、`native-regression.log`、`original-packages.log`；十分钟数据为 `stress-fixed-600s.log`。公开[采样CSV](validation/api4-resilience-20261004.csv)及[曲线](validation/api4-resilience-20261004.png)已保存。十分钟曲线在最终焦点补丁前采集；该补丁随后通过原复现环境和完整回归。

以下原包尚在本地忽略目录，不应清理或用新编译结果覆盖：

| 原包 | SHA256 |
| --- | --- |
| `build/web-shell/baseline/minimal_eda.uapp` | `86988546516e17b4783c5a9ed1583dfaa9e73ecdfcd1f5b8fad9a4910a5a3503` |
| `build/api3/baseline/web_counter.uapp` | `2cf7a9619365184211cb636db6537c6119f03bc40d41d92edfbe6c235d3ccb2f` |
| `build/fw-next/baseline/generic_components.uapp` | `d024c191b6fcae0da5eecc71b29c3429e27a74db9ec339a118a9577c691c0745` |

原包、匹配运行库和本地日志不在 Git 中。换机器时需要单独保全；缺少它们就明确写旧原包未复验。不得重新构建后冒充原二进制。

## 5. 尚未完成的验收

| 编号 | 待做 | 补验条件 |
| --- | --- | --- |
| UV-01 | 实际中文 IME | 真实输入法组合、候选、提交/取消及焦点/标签/DPI变化，记录操作证据 |
| UV-02 | 物理跨屏 DPI | 至少两台不同缩放显示器，记录 WM_DPICHANGED、矩形、framebuffer和候选位置 |
| UV-03 | 长期人工滚动/资源压力；十分钟自动补验通过 | 仍需连续人工行列/树/缩略图/双实例操作，记录时长、受管资源统计和进程曲线 |
| UV-04 | 完整编辑与复杂布局；六种自动组合及三项缺陷修复通过 | 仍需完整编辑手势、草稿/选择期间停靠、浮动、折叠、DPI和模态人工组合，核对状态保留 |
| UV-05 | 整机无登录/远程驱动及其他后端目标环境；OSMesa Session0已补验 | 实际CI LocalSystem Session0的测试DLL/OSMesa通过，整机仍有登录会话1，缺无登录runner；轻量Web、隐藏WGL、WebView2另验，不用替代图片冒充GL |
| UV-06 | 请求方应用完整离屏业务接入 | 应用独立适配窗口依赖、路径/调度和业务renderer；本框架未替应用完成 |
| UV-07 | 物理屏幕边缘菜单及GL遮挡 | 不同DPI显示器边缘测试三种锚点、翻转、外点关闭、焦点恢复和桌面合成 |

API4未实现的Alt访问键、真正无窗口GL、离屏MSAA、WebView2呈现/捕获/组件桥接已在API5实现，历史证据见第7节。布局持久化、约束分隔条与左右栈/浮动拖拽已在API6实现，见第8节。API7已实现底部与同区域标签式停靠、完整范围滚动和连续RGBA；完整浏览器、Canvas/SVG、富文本、可变行高、任意嵌套分割树及硬件EGL未实现；目标环境或人工待验项不能写成通过。

## 6. 接手后的执行规则

先核对用户下一项要求与上述待补验，报告能验证的项目和实际缺少的条件。发现缺陷先保留复现、再做针对性修复，随后运行相关回归；代码未变且无新疑点时无需重跑全部测试。追加证据时记录日期、框架/应用commit、环境、步骤、观察和结果，不覆盖旧的未验证记录。

特别保留以下已解决的问题：菜单模型在堆上分配，避免嵌套 QuickJS 回调耗尽原栈预算；不能靠提高 DOM/脚本预算代替虚拟化。普通右键统一经过公共输入路径。GL CPU读回先清除 skip/PBO 影响后恢复相关状态。workspace flush 有界但空闲不等于 worker 已停止；REFUSE/WAIT、join和回调寿命仍决定 DLL 何时可卸载。

本地 `task_plan.md`、`findings.md`、`progress.md` 是忽略的辅助记录，旧章节含历史状态；先读当前摘要，不沿用“原生是默认宿主”“API2是当前版本”“无需依赖即可默认构建”等旧判断。跨机器接手以本文和当前公共文档为入口。

可直接给下一会话以下指令：

```text
请接手 https://github.com/wbycloud/ui-framework 。先读docs/handoff.md、docs/application-development-standard.md、docs/build-and-validation.md、docs/generic-web-ui.md、docs/workspace-layout.md、docs/framework-menu-offscreen.md及API7、Runtime稳定性、API5/6、菜单、浏览器式宿主、视觉和Session0验收，并读docs/visual-design.md、docs/standalone-host.md、docs/version-policy.md及docs/validation/component-scroll-only-validation.md。核对实际Git状态、HEAD、API7公共头文件和现有四行CI/严格Session0工作流，保留用户改动。API5/6/7及菜单已实现，不重复开发。按docs/version-policy.md只维护接手时确认的当前版本，停止历史SDK/API/调用方/原包兼容专项；当前基础功能及完整size字段/偏移、线程、所有权和卸载合同继续验证，共同数据组件无分页或列箭头按钮但完整范围滚动/按需查询保留。本地运行不冒充托管CI，历史Session0不替代当前源码。先给简短计划，持续补验/修复和文档交付，缺环境或授权最后汇总。业务试点按docs/business-pilot.md先核对指定应用及修改授权；没有授权只准备方案。不得修改冻结SDK、原包、PERF-001或未授权请求方应用，不使用graph-engineering、子代理或新会话；没有明确授权不推送、不触发远程工作流、不强推或创建稳定标签。
```

## 7. API5本轮交付与接续

2026-10-04优先补验Session0／无登录Windows CI：[独立DLL、OSMesa与服务测试记录](validation/session0-osmesa-validation.md)。真实源码3e1fca5在CI LocalSystem Session0通过200轮、400实例、1200frame、2000命令，HWND0、句柄148→148。整机登录会话1，无登录门槛明确失败；用户确认当前没有无登录runner，继续待验。修复MSVC19.44包加载器编译警告及测试运行器无控制台日志初始化，不改公共ABI或版本，原业务应用/SDK/原包保留。

四项实现、依赖、取舍、结构尺寸、失败复现、真实像素和最终日志见[API5验收](validation/api5-validation.md)。完整42项为41PASS/1跨屏SKIP；原生20项为19PASS/1SKIP。原API1/2/3混合宿主及原API4包均0失败，四个原包哈希不变。SDK4完整12个头文件从1f77b2e冻结，原SDK1/2/3未修改。

真正无窗口路径为固定OSMesa24.3.4/llvmpipe软件GL，实际运行应用DLL frame，不创建隐藏HWND或取得窗口DC。旧隐藏WGL独立保留；精确MSAA在两条路径实际为4。OSMesa模块/worker缓存保持到进程结束，不强制卸载私有提供方模块。实际Session0已补验，无登录仍缺条件；没有硬件EGL实现。

WebView2使用SDK1.0.4129.50/实际Runtime154.0.4258.53。组件须显式framework_components，借用后端由调用方保持到host销毁。PENDING通过外层STA消息循环完成，不保留调用方输出缓冲。销毁先切断应用回调，SDK初始化尚未完成时等待内部无副作用脚本完成，再异步Close；环境保留至BrowserProcessExited。应用DLL可卸载，框架DLL/UI线程仍须处理关闭消息。

本轮修复Runtime视口反馈循环、菜单窗口IME干扰/焦点覆盖、提前关闭生命周期及残缺新增指针字段读取。32次实际DLL重开及48次创建期取消资源检查通过，失败记录保留，未提高上限或改skip。当前本地构建为build/fw-next/release与native，新增证据在build/api5-evidence；不混用旧build目录产物。

Session0补验后的必要回归为43项42PASS/1SKIP，原生21项20PASS/1SKIP，四个原包再次通过且哈希不变；前文42/20项为此前历史结果。新增日志在build/session0-evidence，永久证据见专门记录。严格CI工作流默认托管runner会因无登录门槛失败，不能仅看job红绿判断独立GL用例；无登录runner到位后手动填真实runner_label重验。

最终执行源码78c24cb已再次在实际CI验证：Session0 GL0失败、HWND0、句柄147→147，8张原始帧哈希与3e1fca5证据一致；整机无登录仍失败。最新原始摘要和源码/CI链接见专门记录，随后文档提交不改变运行代码。

下一步获取UV-01真实IME、UV-02物理跨屏、UV-03/04人工长时/编辑布局、UV-05整机无登录及其他后端目标环境条件。继续独立补验，不宣称全阶段已完整验收。请求方应用升级另按[提示词](application-upgrade-prompt.md)执行，不自动修改该应用。

## 8. API6独立开发交付与后续

2026-10-05依次完成保存/恢复、分隔条、拖拽停靠，再完善共同组件。应用存储ULYT格式1、稳定面板ID至多63字节；64KiB/512记录、完整输入先验、未知/新增面板、work area夹紧与reset均有测试。左右栈高度/侧栏宽度受约束，Esc/捕获丢失/实例失活取消；布局更新保留原容器、草稿、选择、焦点、组件及GL context。Windows无窗口workspace执行同一逻辑布局，不创建HWND；原生手势/屏幕预览只适用有窗口路径。

枚举下拉、RGBA调色/文本、表格方向/Home/End/Tab/Enter/F2/Esc由两后端共同模板维护。完整排序由source按query状态提供窗口，不排序缓存冒充全数据；稳定ID多选至多512、内存计入缓存，异步范围用UI_QUERY_SELECTION及复制投递，旧结果按代次丢弃。所有既有DOM/缓存/图片/队列/JS门槛保持。API6 opt-in表格行为与旧点击编辑兼容，SDK5已冻结，原API1–5包分别通过并保持哈希。

独立api6_fixture.uapp是公共C DLL，组合树/十万行表格、属性、真实OSMesa MSAA frame C图片、后台缩略图、语义命令、双实例、初始化失败、模态、关闭/join/unload和8次重开。轻量完整离屏生命周期CBT HWND0；真实Runtime另执行相同组件体验及实际C图片捕获。它不代表请求方真实业务应用试点。完整本机51项50PASS/1物理SKIP，原生21项20PASS/1SKIP，原五包与冻结调用方PASS；精确输出、CI实际矩阵及失败复现见[API6验收](validation/api6-validation.md)。

构建仍为build/fw-next/release，独立原生build/api6/native，新证据build/api6-evidence；永久日志/像素在docs/validation/api6-*。CI使用固定依赖、临时1920×1080虚拟桌面、四个软件WGL必需DLL、显式llvmpipe及兼容Runtime至少138.0.3351.48，检查实际环境并保留失败。虚拟桌面、Session0交互对照和矩阵绿色均不能代替严格整机无登录或物理人工验收。

真实CI额外暴露创建期取消回收失败：12轮句柄301→316超过原+12，本机相同Mesa部署综合用例一次487→500也超限。最终[9705565](https://github.com/wbycloud/ui-framework/commit/9705565)仅接受内部空白导航ID，忽略初始about:blank，在完成文档上等待renderer确认，再在UI消息中Stop/Close；事件所有权只释放一次。取代第7节历史初始文档脚本排空方案；没有应用内容/回调、ABI变化或预算增加。测试要求初始/重复周期同样排空、内部STA清理归零及正常退出；新增快照诊断在目标系统干扰回归后已移除，原断言保留。完整必要回归和实际CI结果见API6验收，历史失败保留。

最后审计：用户当前无无登录Windows服务runner；真实中文IME、不同缩放物理屏幕/边缘/桌面合成和长期人工条件缺失；未指定并授权真实业务应用试点；没有实测OSMesa瓶颈和明确硬件GL需求。继续保留这些待验，不注销用户、不改既有服务、不扩大到硬件无窗口方案。可独立完成的实现、回归和文档交付与这些待验分别报告。

本轮最终交付状态：实现及最终回归源码9705565已在工作分支发布，四行实际CI通过；559b802保存最终文档、失败修复、完整本机日志、CI摘录和实际像素，已本地提交。2026-10-05最终推送自动审批拒绝，理由为当前可核验用户授权不足以覆盖默认分支发布；操作未执行。实查origin/main仍ea5b108、工作分支9705565，未强推或创建标签。待明确发布授权后才快进推送完整交付。本轮API6源码未重新触发Session0工作流；已通过的Session0证据严格属于专门记录中的78c24cb，不冒充当前源码目标环境验收。


## 9. 2026-10-05 菜单外观与鼠标修正

接手基线干净c4c5a91，已读合同/API5/API6并核对实现，保留既有工作。本轮按用户要求仅本地提交：6bf59f8引入作用域菜单/工具/标签样式及缓存侧向菜单，2917b44补齐框架☰外点/Esc/重复点击与F1名称，d907fbd按实际行数去掉无分页旧页脚；d2e4a61补充hook初始化失败借用宿主引用回收；产品源码d2e4a61、最终验收源码b1f4c7b，完整SHA见专门记录，文档提交不变更运行代码。

菜单打开时顶层悬停切换；嵌套侧展、work area翻转、160ms跨层宽限、外点关闭、重复入口关闭；键盘、模态、焦点、实例和一次命令继续原合同。单层128项及原分页/节点/像素预算保留，缓存链按共同预算最多13层；完整深路径仍可直接打开。呈现/输入/捕获当前最深可见层；无窗口逻辑展开不创建菜单HWND。公共头文件、冻结SDK、原包及PERF-001未改，不升级公共API。

最终完整53项52PASS/1物理SKIP（264.36s），原生21项20PASS/1物理SKIP（4.78s），结果与源码/条件见[菜单验收](validation/menu-desktop-validation.md)：包括实际宿主GL内容上方菜单窗口命中、三级展开、480px窄窗/程序DPI、双实例/模态/焦点/40次重开、128项末项/预算、原生及完整矩阵。KLayout现有原包仅只读观察，菜单命令0调用，前后哈希一致；正常桌面用例采用受控测试窗口与可恢复鼠标位置，保留原窗口命中断言及同类工具提示验证；只读观察分支不作这些桌面准备。真实前后PrintWindow客户区图保留，不代替GL业务、桌面交换或IME验收。

失败记录不能丢：首版样式/选择错误、旧关闭按钮用例、缓存首次建立口径、F1字符映射与无分页空白均有复现及修正。2917b44完整矩阵出现一次API6 WebView2重开句柄374→416超原+12，相同源码独立复验386→387通过，原因未定位；保留原断言及完整失败，继续追查，不把菜单改动说成其修复。受限环境Runtime超时也独立保留。

仅本地交付，不推送或创建稳定标签；旧API6四行CI属于9705565，不能算本轮源码的新CI。整机无登录runner仍缺，历史Session0成功仍归专门记录；真实IME、不同缩放物理显示器/边缘、桌面合成和长期人工待验。本轮有KLayout只读观察授权，没有业务修改/适配授权，独立DLL仅算框架集成。缺少这些条件不阻塞本轮菜单独立开发，但不能宣布全部验收通过。

## 10. API7稳定性、滚动与平面布局本地交付

接手干净658b4972f78c4c52fad86ff8c379567dd108d277，既有API5/6及菜单保留。独立开发依次完成关闭门控、完整范围横纵滚动、底部/标签、连续RGBA、独立DLL与文档。Windows x64/C11，SDK0.7/API7/标准7、ABI1/包格式1；不扩展嵌套分割树或硬件无窗口GL。

本地单元：3055bab（关闭门控/资源复现）、87f829a（SDK6冻结）、21e94e5（API7实现/测试/CI）、d79a59f（捕获变化测试）、1262cc0（直接注册BOTTOM/无窗口初始布局）、2c0c52b（稳定复现的异步模态原生焦点修复）、ae4ee3a/cb42660（仅原生输入就绪/失败诊断）。最后可执行源码HEAD为cb426600d25175261eeaee3ccd23ad859b138b16；文档和证据是包含本节的后续提交，以git log/status为准。没有推送/强推/新标签、代理/新会话、请求方/PERF-001/冻结SDK或原包编辑。

旧21e矩阵60PASS/1SKIP仍归原身份。1262矩阵又复现原API5超限358→395、USER15→17、pending0，不能用之前通过称已根治。观察到Windows输入适配窗口/模块延迟初始化，另稳定复现模态容器在controller未创建时得到焦点而文档就绪后未转交；2c只转交仍有效当前焦点。三个原包独立32周期均390→387；当前完整矩阵built/original各32均394→391。最新API7两个独立64周期390→390和391→392、pending0、逐轮DLL卸载通过。原资源门槛不改，历史全部波动精确归因不主张。

2c产品61项矩阵初跑为59PASS/1FAIL/1物理SKIP（348.69秒）；ui_api5_native_host受到未由用例发送的VK_PACKET输入，其后独立运行也保留前台获取失败。原生配置22项21PASS/1SKIP（5.35秒）。当前原生输入等待实际呈现就绪，所有owned/次数/焦点断言保留，最后cb42660三个独立CTest各1/1通过。没有把初跑失败改写成全绿，也没有把后续通过称共享桌面竞态永久解决；随后冻结cb42660在后台启动器中完整61项60PASS/1物理SKIP、0失败（327.82秒）；原生输入另连续10个独立进程全部通过（60.74秒）。这些通过限定于实际条件，不能宣布共享桌面竞态永久消失。精确每次身份、失败、步骤、依赖及hash见[API7验收](validation/api7-validation.md)、[最终清单](validation/api7-frozen-binaries.json)和[资源记录](validation/runtime-stability-validation.md)。

滚动条按完整total/列宽，用BigInt精确处理uint64，支持空批总量缩小/树缓存外折叠、换源/排序/捕获失效、窗口/无窗口输入，保持分页和预算。底部9/旧COUNT8、CANCEL8追加、panel旧48前缀及完整字段兼容；格式2读1、旧host写1、新host写2，应用负责存储。标签槽、草稿/选择/焦点/GL复用；连续颜色只改草稿、业务提交一次。SDK1–6调用方及六个原二进制分别通过且hash不变，旧布局1断言保留。[迁移](migration-v0.6-to-v0.7.md)、开发标准、构建、接口、README/CHANGELOG和升级提示词同步。

最后审计：cb42660源码的Session0服务脚本第11行非管理员检查退出1，未创建服务或注销用户/修改既有服务，历史78c24cb不冒充当前验收。最终冻结Session1实际DLL/OSMesa200轮、400实例、2000命令、HWND0、原128MiB门槛通过（185→186句柄，private峰值109,375,488），只算INTERACTIVE_CONTROL。严格无登录runner没有；真实中文IME、物理不同DPI/屏幕边缘/桌面合成、长期人工、当前托管CI与授权业务试点仍待验。没有OSMesa瓶颈/硬件需求证据，不扩大范围。独立实现和本地文档已交付；当前自动矩阵已通过，但历史失败仍保存；上述目标环境和人工限制意味着不宣布整个阶段验收完成。

归档提交39bcedf；41fb643针对current/frozen证据保留原始字节，实测日志哈希与Git blob零差异。后续仅说明/检查记录提交，不改变cb42660已验执行源码。最终HEAD和清洁状态以git log/status核对。

## 11. API7交付收敛、CI复验与业务准备

2026-10-05/06接手干净68c4e9c，当前API7产品功能不重复开发。CI单元ae4dc7f补清单/身份/原包缺项及结果校验；2fde15e修复关闭WebView2时滚动测试仍链接可选函数；e4a33bf把显式OSMesa对照与软件WGL部署隔离，保持原资源断言。没有修改公共接口/ABI、请求方应用、SDK1–6、原包或PERF-001。

当前结果、原失败及精确运行身份见[交付收敛验收](validation/api7-delivery-validation.md)。四行脚本的本机运行与GitHub托管CI分别报告；本轮无远程推送/工作流触发授权，保留本地提交。当前Session0权限与严格无登录runner、真实IME/物理多屏/桌面边缘合成/长期人工待验，不继承78c24cb历史成功。

真实业务接入按[试点准备](business-pilot.md)先取得指定应用和明确修改范围。旧KLayout只读观察不授权迁移；框架DLL集成不作为业务验收。继续复用API7布局2读取1、旧API1–7/C ABI及线程/所有权/卸载合同，不扩大硬件GL。

## 12. 浏览器式独立宿主本地交付

2026-10-06接手干净7ce1f2c，已有API5/6/7、菜单和关闭实现复用。产品/测试单元d7030b3f85e73e98284fdaec15c88b77ac0789b8：融合标签标题栏、第二行活动菜单、Windows命中/拖动/缩放/系统菜单、12项成功绝对路径历史、窄窗独立标签分页；主题与布局重置迁到宿主选项，AI和REFUSE/WAIT保持原合同。API7/ABI1不升级，public/src/冻结SDK/原包/PERF-001与请求方应用保留。

[宿主使用](standalone-host.md)记录存储和能力边界；[当前验收](validation/browser-host-validation.md)含前后真实截图、失败/修复和全部原字节证据。冻结源码本机62项61PASS/1物理SKIP、0失败340.52秒，正常桌面专项6/6；实际拖动、最大化/还原/边缘、右键及Alt+Space、窄窗程序DPI、双实例/最后标签、模态/焦点/一次命令、24次实际DLL与原六包分别通过。公共接口和原阈值保留；本机结果不冒充托管CI。

OSMesa当前Session1独立DLL200轮、HWND0/原预算通过；Session0严格服务脚本非管理员检查退出1、未创建服务，历史成功不替代。无无登录runner/真实中文IME/不同DPI物理多屏/边缘Snap及DWM/长期人工/授权业务试点；均待验，无硬件无窗口扩展。首版样式UNSUPPORTED、初始标题区域和系统菜单失败及受限桌面输入失败保留；普通桌面及最终完整矩阵随后通过，不把共享桌面或历史Runtime不确定性称永久根治。本轮只本地提交，不推送/远程工作流/新稳定标签。


## 13. 2026-10-06 框架自有视觉与AI收敛

接手7e87f55及四文件AI默认收起未提交修改，审查保留。内部单元485ef0d（统一中性浅深规范、宿主/共同组件/菜单/浮动/AI及回归）、f733bcc（现有CI分类）、0b306e6（同HWND重复捕获及基本参数即时反馈）、c5df8c0（工具密度/标签对齐及真实尺寸回归）、7d81b8f（非法草稿保持/禁止旧值执行）、01c8780（原参数预算及JSON数值保真）。公共API7/ABI1、布局格式2读1、菜单/编辑/线程/所有权及关闭合同、原预算不变。最后执行源码01c87801aa86d70759c45e333af7ecc6e31699fc，核心完整矩阵绑定0b306e6，不改写身份；后续文档/证据提交不变运行代码。没有推送、远程工作流、新稳定标签、代理或新会话。

[视觉规范](visual-design.md)、[宿主](standalone-host.md)及[验收/前后真实截图](validation/visual-ui-validation.md)给出角色、数字、状态和原生边界。独立宿主同步全部共同组件/菜单/Web浮动标题主题，自有HTML/原生/GL仍由应用管理。AI每次启动收起，仅手动切换，普通视图是目标/可读操作/基本参数/状态结果，事务/快照/schema/原JSON/日志在初始折叠高级区，保留原高级操作和validator，不接模型服务。收起回收原生/GL宽度，resize/实例切换保留手动状态。

初始收起AI暴露右边缘被原生子窗口覆盖，保留系统侧/底缩放框后原实际动作通过。像素断言复现并修复边框节点黑底；完整矩阵稳定复现重复SetCapture同一HWND触发pressed取消，避免重复获取后原点击断言通过。参数红例和初始化用例错误分别记录；未知JSON值保留、无效数字草稿保持并禁用执行、合并参数原4095字节预算及大整数保真高级路径、布尔反馈及一次命令通过；参数边界失败与修复分别保留。工具原240上限用例通过，排除最初flex判断；真实240短项密度红例修正估算后通过，未提高预算或删除失败。

0b306e6本机完整64项63PASS/1物理SKIP，376.85秒；正常WGL Intel/真实Runtime/OSMesa。原生/Light复用目录自动选择Mesa D3D12，2/9项崩溃，事件指向libgallium_wgl偏移0xde41aa；四DLL哈希与固定提供方一致。按既有CI显式GALLIUM_DRIVER=llvmpipe的同二进制对照：22项21PASS/1SKIP（4.94秒）、43项42PASS/1SKIP（47.92秒）。不删除DLL、不改断言，D3D12组合未修复；软件通过不代替D3D12。最后工具单元14/14（68.41秒）、非法草稿单元14/14（68.14秒）、参数边界单元14/14（67.76秒）；01c8780两后端/EDA最终截图各0失败；原SDK1–6/六包哈希再次PASS。测试与原图的精确二进制/计划/原日志在本轮证据包，历史失败不覆盖。

完整矩阵0b306e6中Session1实际DLL/OSMesa200轮、HWND0及原预算通过，最后源码01c8780再次调用Session0服务脚本，非管理员检查退出1且未创建输出/服务；历史成功不替代当前Session0。无严格无登录runner、真实IME/物理不同DPI/桌面边缘Snap及DWM/长期人工/授权业务试点；均待验。没有实测OSMesa瓶颈/硬件需求，不扩大范围。只有本地交付，不能宣称当前托管CI、所有目标环境或完整业务验收通过。

## 14. 2026-10-06 共同组件分页入口清理与当前版本政策

接手干净bfa5b120216acde0bc33fd03ffa6f40e7c0c78db，SDK0.7/API7确认不变；此前视觉和AI修改已在基线中。本地单元a3181d8删除共同TREE/TABLE/LIST分页与列箭头按钮，隐藏数据操作容器并回收36逻辑像素；FORM/DIALOG提交取消和菜单/最近/标签溢出分页保留。完整total_count/列宽横纵条、uint64/BigInt、first/count按需窗口、稳定ID、草稿/焦点/选择及原预算不变，API7应用无需修改。702cd05撤下冻结SDK/API1–6/历史集成/原包专项，当前功能夹具用当前头文件及API7声明；4b3fe3d补当前Session0 GL对照包API7声明与实际日志/CSV/帧归档。公共头文件/运行库旧版本分支不清理，API/ABI不升级。

本轮起仅维护和验收各轮接手确认的当前版本，[政策](version-policy.md)取代持续保证旧SDK/API/调用方/原包兼容的承诺；现存可加载行为不等于未来承诺。当前基础接口仍维护，不能按引入版本删掉；冻结SDK、历史记录与原包原样保全，不再作兼容专项或交付门槛。后续使用匹配当前SDK/API的头文件、导入库、运行库与清单/DLL声明。

[本轮验收](validation/component-scroll-only-validation.md)提供同一未修改API7包、实际宿主及轻量/Runtime两主题/96/144/192 DPI前后图，40张before和40张after原BMP及无损PNG像素索引。测试先行4失败→4通过，当前smoke因格式2夹具仍用旧80字节头出现1失败；按实际96/112只修测试后9/9通过，不改产品布局。最终矩阵身份与详细结果在验收和证据ZIP，不把dirty文档状态改写为clean。

Runtime首轮关闭排空15秒断言失败，后续12取消pending0、句柄291→288/USER18→17；同二进制三个独立进程与Runtime阶段随后通过，完整行两次通过。原因未确定，没有声称根治或改资源/时间门槛，不杀Runtime或清用户数据。启动器解析/PowerShell环境失败另保存，不混同产品结果。当前API7对照包在Session1执行200周期/400实例/2000命令、HWND0和原资源门槛通过，仅为INTERACTIVE_CONTROL；当前Session0/整机无登录不继承历史成功。

仅本地提交，未推送/触发托管CI或创建稳定标签；文档/原字节证据另行提交，最终HEAD以Git为准。没有请求方/PERF-001/原包/冻结SDK修改、代理/新会话、额外依赖或预算提升。严格无登录runner仍无，真实IME/不同缩放物理屏幕/屏幕边缘DWM/长期人工及授权业务试点待验；当前分页开发与本机回归完成不表示全部目标验收完成。

最终执行源码4b3fe3d四行完整重跑：native20PASS/1SKIP、light40PASS/1SKIP、osmesa44PASS/1SKIP、webview2 51PASS/1SKIP；合计155次通过/4次同一物理跨屏跳过，0失败。276.50秒阶段测试时间。四行manifest记录4b3fe3d及仅文档dirty，两提供方行共24个Session1原文件逐字节归档一致；本机证据不冒充远程CI。原失败仍保留，不宣称Runtime不确定性根治。


## 15. 2026-10-07 原生共同组件滚动修复

起点0744d17。9df202f避免同HWND重复捕获，2e87e02将TREE异步加载/空状态移进行视口覆盖层，稳定轨道几何；API7/ABI1/标准7和预算不变。真实原生TREE/LIST/64列鼠标、first3回首、末列按需、重复/resize/焦点、取消及程序DPI通过。框架41通过/1物理跳过，真实Runtime相关3/3；只读重建当前KLayout C原代码a727103（文档HEAD42d8624）完整27/27，观察补证95/95。首次业务26/27及4条回首/键盘失败保留，并由第二修复定位；不以单次通过覆盖。

完整SDK产品快照为2e87e02，匹配头文件/导入库/运行库/宿主并含离线源码依赖；文档提交另行记录。真实截图、所有哈希、命令、失败和限制见[验收](validation/native-component-scroll-validation.md)。应用仓库/冻结SDK/原包/历史证据只读，未执行历史包或修改PERF；未推送/发布/稳定标签。物理IME/跨屏/DWM、长期人工、托管CI和当前Session0分别待验，未将GDI/Shift/前台其他现象归责本轮。
