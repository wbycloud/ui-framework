# 框架开发交接记录

更新日期：2026-10-04。当前SDK0.5/API5四项已实现，本机自动验收及实际CI的OSMesa Session0补验通过；整机无登录及人工验收尚未全部完成。先读[API5验收](validation/api5-validation.md)、[Session0记录](validation/session0-osmesa-validation.md)与第7节，API3/API4历史证据继续保留。不要把“已推送”“测试通过”和“完整验收完成”混为一谈。

## 1. 恢复顺序与版本

1. 检查 `git status --short`、`git branch --show-current` 和 `git log -3 --oneline`；先保留接手时的用户改动。
2. 阅读本文、[应用开发标准](application-development-standard.md)、[API5验收](validation/api5-validation.md)、[API4记录](validation/api4-validation.md)及[API3未验证项](validation/api3-validation.md#41-未验证项目与补验清单)。
3. 修改代码前核对[公共头文件](../include/ui_framework/ui.h)、[菜单/离屏合同](framework-menu-offscreen.md)、[通用Web](generic-web-ui.md)、[0.4→0.5迁移](migration-v0.4-to-v0.5.md)及相关测试。无需重做已完成的实现。

| 项目 | 交接状态 |
| --- | --- |
| 仓库 | [wbycloud/ui-framework](https://github.com/wbycloud/ui-framework) |
| API5功能代码 | [87478fa20d7bb46809c0ef81dd44f972dc193a24](https://github.com/wbycloud/ui-framework/commit/87478fa20d7bb46809c0ef81dd44f972dc193a24)，四项实现及回归；文档另行提交，实际HEAD以Git为准 |
| API4历史实现/补验 | [e83a008](https://github.com/wbycloud/ui-framework/commit/e83a0087f0ef1017c9cd99db3e2f6754b503311c)、[48d6aaf](https://github.com/wbycloud/ui-framework/commit/48d6aafa451802b2a057fa68ce1f687fb2f2a004)，模态原生输入、STYLE回收、redock焦点；[记录](validation/api4-validation.md#5-2026-10-04-接手补验与修复) |
| 已推送分支 | `main`、`codex/menus-offscreen`；交接文档另行提交，后续以实际 HEAD 为准 |
| SDK / 框架 API / 标准修订 | `0.5.0` 开发版 / `5` / `5` |
| 兼容范围 | 运行库接受 API1/2/3/4/5；包清单和 DLL descriptor 必须一致 |
| 应用 ABI / 导出入口 / 包格式 | `1` / `ui_app_query_v1` / `1` |
| 稳定标签 | 只有 `v0.1.0`；未创建新的稳定标签 |

本轮起点为干净的1f77b2e，与当时origin/main及工作分支一致，没有需覆盖的用户改动。当前记录不承诺后续 `main` 仍是同一个提交；应用应锁定 SDK commit。

## 2. 长期要求与责任边界

第一平台为 Windows x64，框架使用 C11，应用接口为公共 C ABI。独立宿主加载多个 `.uapp`，以标签切换实例；原生嵌入式接口和旧应用内容保留兼容。UI 外壳及新通用业务组件由框架维护 HTML/CSS/JavaScript，应用提供描述、数据和语义命令。OpenGL 是否使用由应用决定。

新业务 UI 不使用 TreeView、ListView、Button、Edit、MessageBox 或隐藏 EDIT 代理；系统文件/目录选择器是例外。Win32 可用于窗口、消息、IME、字体和绘制适配。框架不拥有具体应用的文档模型、算法或业务渲染器，不是完整浏览器，也没有连接大模型服务。

优先复用原菜单、工具、面板、内容槽、命令、队列和生命周期。新增接口不暴露第三方私有类型；保持旧字段偏移、枚举值和默认行为，新字段按 size 判断。64位身份在 JSON 中为十进制字符串，图像像素通过 C 接口复制传递。线程、所有权和 DLL 卸载合同不能绕过。

文档面向 C/C++ 应用开发者，不署名；给其他应用的[升级提示词](application-upgrade-prompt.md)只使用 GitHub 入口，不包含个人本地目录。此前授权包括提交并推送框架和说明；不要强推或创建新的稳定标签。

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

新增resilience后，新 clone 默认轻量配置为31项；可选 WebView2为33项。只有设置 `UI_LEGACY_COMPONENT_PACKAGE` 指向原API3包才额外产生第34项，不得把未提供的原包写成已复验。WebView2需固定 SDK1.0.4129.50 和 Runtime；受限环境曾两项创建超时，在允许浏览器子进程环境复验通过，不能把超时记为通过。

原生配置需要同时关闭宿主、轻量后端和 WebView2；不产生 Web 宿主：

```powershell
cmake -S . -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_BUILD_STANDALONE_HOST=OFF -DUI_FRAMEWORK_ENABLE_LIGHT_WEB=OFF -DUI_FRAMEWORK_ENABLE_WEBVIEW2=OFF
cmake --build build/native
ctest --test-dir build/native --output-on-failure
```

当前机器的 API4 验证构建位于 `build/fw-next/release`；`build/web-shell` 等旧目录可能残留旧版本，使用前重建，不混用运行库。本地保留 `build/fw-next/ctest-final.log`、`build/fw-next/clean-test.log`、`build/fw-next/build.log`、`build/fw-next/build.cmd` 和原生构建脚本。它们被忽略，不属于远程可获取证据；公开证据以验收文档和可构建测试为准。

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

API4未实现的Alt访问键、真正无窗口GL、离屏MSAA、WebView2呈现/捕获/组件桥接已在API5实现，当前证据见第7节。完整浏览器、Canvas/SVG、富文本、可变行高、拖拽停靠和布局持久化仍在既定范围之外；硬件EGL未实现。目标环境或人工待验项不能写成通过。

## 6. 接手后的执行规则

先核对用户下一项要求与上述待补验，报告能验证的项目和实际缺少的条件。发现缺陷先保留复现、再做针对性修复，随后运行相关回归；代码未变且无新疑点时无需重跑全部测试。追加证据时记录日期、框架/应用commit、环境、步骤、观察和结果，不覆盖旧的未验证记录。

特别保留以下已解决的问题：菜单模型在堆上分配，避免嵌套 QuickJS 回调耗尽原栈预算；不能靠提高 DOM/脚本预算代替虚拟化。普通右键统一经过公共输入路径。GL CPU读回先清除 skip/PBO 影响后恢复相关状态。workspace flush 有界但空闲不等于 worker 已停止；REFUSE/WAIT、join和回调寿命仍决定 DLL 何时可卸载。

本地 `task_plan.md`、`findings.md`、`progress.md` 是忽略的辅助记录，旧章节含历史状态；先读当前摘要，不沿用“原生是默认宿主”“API2是当前版本”“无需依赖即可默认构建”等旧判断。跨机器接手以本文和当前公共文档为入口。

可直接给下一会话以下指令：

```text
请接手 https://github.com/wbycloud/ui-framework 。先读 docs/handoff.md，再核对实际Git状态、HEAD、API5开发标准及API3/4/5验收记录。四项API5功能已实现并有本机真实Runtime/GL证据，不重做或把目标环境未验写成通过。不修改请求方应用、冻结SDK、原包及PERF-001，不使用graph-engineering、子代理或新会话。继续可执行的补验和针对性修复，保留API1/2/3/4兼容、size/字段偏移及线程/卸载合同；同步标准、迁移、验收、handoff与GitHub升级提示词。
```

## 7. API5本轮交付与接续

2026-10-04优先补验Session0／无登录Windows CI：[独立DLL、OSMesa与服务测试记录](validation/session0-osmesa-validation.md)。真实源码3e1fca5在CI LocalSystem Session0通过200轮、400实例、1200frame、2000命令，HWND0、句柄148→148。整机登录会话1，无登录门槛明确失败；用户确认当前没有无登录runner，继续待验。修复MSVC19.44包加载器编译警告及测试运行器无控制台日志初始化，不改公共ABI或版本，原业务应用/SDK/原包保留。

四项实现、依赖、取舍、结构尺寸、失败复现、真实像素和最终日志见[API5验收](validation/api5-validation.md)。完整42项为41PASS/1跨屏SKIP；原生20项为19PASS/1SKIP。原API1/2/3混合宿主及原API4包均0失败，四个原包哈希不变。SDK4完整12个头文件从1f77b2e冻结，原SDK1/2/3未修改。

真正无窗口路径为固定OSMesa24.3.4/llvmpipe软件GL，实际运行应用DLL frame，不创建隐藏HWND或取得窗口DC。旧隐藏WGL独立保留；精确MSAA在两条路径实际为4。OSMesa模块/worker缓存保持到进程结束，不强制卸载私有提供方模块。实际Session0已补验，无登录仍缺条件；没有硬件EGL实现。

WebView2使用SDK1.0.4129.50/实际Runtime154.0.4258.53。组件须显式framework_components，借用后端由调用方保持到host销毁。PENDING通过外层STA消息循环完成，不保留调用方输出缓冲。销毁先切断应用回调，SDK初始化尚未完成时等待内部无副作用脚本完成，再异步Close；环境保留至BrowserProcessExited。应用DLL可卸载，框架DLL/UI线程仍须处理关闭消息。

本轮修复Runtime视口反馈循环、菜单窗口IME干扰/焦点覆盖、提前关闭生命周期及残缺新增指针字段读取。32次实际DLL重开及48次创建期取消资源检查通过，失败记录保留，未提高上限或改skip。当前本地构建为build/fw-next/release与native，新增证据在build/api5-evidence；不混用旧build目录产物。

Session0补验后的必要回归为43项42PASS/1SKIP，原生21项20PASS/1SKIP，四个原包再次通过且哈希不变；前文42/20项为此前历史结果。新增日志在build/session0-evidence，永久证据见专门记录。严格CI工作流默认托管runner会因无登录门槛失败，不能仅看job红绿判断独立GL用例；无登录runner到位后手动填真实runner_label重验。

下一步获取UV-01真实IME、UV-02物理跨屏、UV-03/04人工长时/编辑布局、UV-05整机无登录及其他后端目标环境条件。继续独立补验，不宣称全阶段已完整验收。请求方应用升级另按[提示词](application-upgrade-prompt.md)执行，不自动修改该应用。
