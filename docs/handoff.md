# 框架开发交接记录

更新日期：2026-10-09。当前SDK0.9.0-dev/API9/标准9，应用ABI1/包格式1。最新实例语言复核接手干净876588246a72d261c88b9e1906372d1b5b91d1ca，已有实现保留，没有重复新增接口；先读第24节与[本轮复核](validation/instance-language-current-validation.md)。此前已核验远端main/codex/menus-offscreen b6a36fee完整源码同步，原托管CI失败及Session0实际GL通过/严格无登录失败见第21节与[同步记录](validation/github-sync-validation.md)，不是当前8765882服务复验。第19节[API9初验](validation/instance-language-validation.md)、第18节及更早各节保留原提交、环境和失败，不继承为当前CI通过。保留95f5d78/6a0170a视觉、9df202f/2e87e02捕获及异步轨道、f0b161c列序与捕获和第23节界面修复。共同组件无分页按钮，原预算与生命周期保持，只维护当前API9；本地交付、托管CI、Session0及物理人工条件分别报告。

本地CI收敛的最新接续见第22节及[本轮验收](validation/ci-recovery-validation.md)：宿主语言断言已有本地修正，原托管存储/语言超时仍待定位。远端原结果、Session0 GL及严格无登录分别保留，不能把本机通过当作远程复验。

本轮只框架及框架维护完整示例，不访问业务仓库。当前完整配置63通过/1物理跳过、独立无Web原生21通过/1物理跳过；SDK归档、实际解包应用、源码快照及最终本地提交见第24节与本轮复核，未经新授权不推送或远程复验。

## 1. 恢复顺序与版本

1. 检查 `git status --short`、`git branch --show-current` 和 `git log -3 --oneline`；先保留接手时的用户改动。
2. 阅读本文、[同步与CI记录](validation/github-sync-validation.md)、[API9验收](validation/instance-language-validation.md)、[应用开发标准](application-development-standard.md)、[当前体验验收](validation/component-experience-validation.md)、[Runtime稳定性](validation/runtime-stability-validation.md)及[构建与CI](build-and-validation.md)。API3–8记录用于追踪历史，不重新加入旧版本兼容专项。
3. 修改代码前核对[公共头文件](../include/ui_framework/ui.h)、[布局](workspace-layout.md)、[菜单/离屏合同](framework-menu-offscreen.md)、[通用Web](generic-web-ui.md)、[0.7→0.8迁移](migration-v0.7-to-v0.8.md)及[0.8→0.9迁移](migration-v0.8-to-v0.9.md)、[实例语言](instance-language.md)和相关测试。无需重做已完成的实现。

| 项目 | 交接状态 |
| --- | --- |
| 仓库 | [wbycloud/ui-framework](https://github.com/wbycloud/ui-framework) |
| 当前已核验GitHub交付 | main与codex/menus-offscreen均为b6a36fee3f4c1a73818cdb01ea7271ad5abd30db；源码与README匹配，自动CI失败，详见第21节；后续本地文档提交不自动代表远端已更新 |
| API5功能代码 | [87478fa20d7bb46809c0ef81dd44f972dc193a24](https://github.com/wbycloud/ui-framework/commit/87478fa20d7bb46809c0ef81dd44f972dc193a24)，四项实现及回归；文档另行提交，实际HEAD以Git为准 |
| 浏览器式宿主 | d7030b3：单行标签标题栏、第二行应用菜单、原Windows/关闭合同及12项最近成功包；完整结果/限制见第12节，文档提交不改执行代码 |
| API8历史交付 | 产品f0b161c、示例／测试d24cdf3；本轮身份、SDK、实测及限制见第18节；前轮8078281／0fe0864与第17节不覆盖，实际HEAD以Git为准 |
| API6实现/最终测试与CI条件 | [1cdc50e](https://github.com/wbycloud/ui-framework/commit/1cdc50e)、[9705565](https://github.com/wbycloud/ui-framework/commit/9705565)，完整提交序列及失败修复见API6验收；文档提交不改运行代码 |
| 菜单后续本地实现 | `6bf59f8`、`2917b44`、`d907fbd`、`d2e4a61`及测试收敛`b1f4c7b`；最终产品源码d2e4a61，仅内部菜单改动，API6/ABI1不变；最终证据另提交 |
| API4历史实现/补验 | [e83a008](https://github.com/wbycloud/ui-framework/commit/e83a0087f0ef1017c9cd99db3e2f6754b503311c)、[48d6aaf](https://github.com/wbycloud/ui-framework/commit/48d6aafa451802b2a057fa68ce1f687fb2f2a004)，模态原生输入、STYLE回收、redock焦点；[记录](validation/api4-validation.md#5-2026-10-04-接手补验与修复) |
| API6阶段最后核对的远端分支（历史） | `main`仍为`ea5b108`，`codex/menus-offscreen`为`9705565`；最终API6证据`559b802`已本地提交，未推送；交付状态修订另行提交，实际HEAD以Git为准 |
| SDK / 框架 API / 标准修订 | `0.9.0` 开发版 / `9` / `9` |
| 维护范围 | 只维护和验收本轮确认的API9；运行库现存API1–9接受行为暂存，不作未来旧包兼容保证；清单与DLL声明必须一致 |
| 应用 ABI / 导出入口 / 包格式 | `1` / `ui_app_query_v1` / `1` |
| 稳定标签 | 只有 `v0.1.0`；未创建新的稳定标签 |

API6本轮起点为干净ea5b108，与当时origin/main及工作分支一致，没有需覆盖的用户改动；API5起点1f77b2e属于历史记录。后续以Git核对实际HEAD，应用应锁定SDK commit。

## 2. 长期要求与责任边界

第一平台为 Windows x64，框架使用 C11，应用接口为公共 C ABI。独立宿主加载多个 `.uapp`，以标签切换实例；当前API9原生嵌入式接口及应用内容继续维护。UI 外壳及新通用业务组件由框架维护 HTML/CSS/JavaScript，应用提供描述、数据和语义命令。OpenGL 是否使用由应用决定。

新业务 UI 不使用 TreeView、ListView、Button、Edit、MessageBox 或隐藏 EDIT 代理；系统文件/目录选择器是例外。Win32 可用于窗口、消息、IME、字体和绘制适配。框架不拥有具体应用的文档模型、算法或业务渲染器，不是完整浏览器，也没有连接大模型服务。

优先复用原菜单、工具、面板、内容槽、命令、队列和生命周期。新增接口不暴露第三方私有类型；保持旧字段偏移、枚举值和默认行为，新字段按 size 判断。64位身份在 JSON 中为十进制字符串，图像像素通过 C 接口复制传递。线程、所有权和 DLL 卸载合同不能绕过。

文档面向 C/C++ 应用开发者，不署名；给其他应用的[升级提示词](application-upgrade-prompt.md)只使用 GitHub 入口，不包含个人本地目录。历史任务曾授权推送，API6阶段最终推送曾被自动审批拒绝（第8节）；该拒绝属于历史记录。第19节API9本地验收时没有远程授权；2026-10-08用户另行明确授权普通推送并更新main，允许自然触发现有工作流（第20节）。不额外workflow_dispatch，不强推或创建稳定标签。

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

API4历史验证曾使用`build/fw-next/release`，菜单阶段同目录重建为API6、配置53项/原生21项（第9节）；此前重建为API7的身份见第10节；当前API8使用独立build/experience-20261007/after，身份见第17节；`build/web-shell` 等旧目录可能残留旧版本，使用前重建，不混用运行库。本地保留 `build/fw-next/ctest-final.log`、`build/fw-next/clean-test.log`、`build/fw-next/build.log`、`build/fw-next/build.cmd` 和原生构建脚本。它们被忽略，不属于远程可获取证据；公开证据以验收文档和可构建测试为准。

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
请接手 https://github.com/wbycloud/ui-framework 。先读docs/handoff.md、docs/application-development-standard.md、docs/build-and-validation.md、docs/generic-web-ui.md、docs/workspace-layout.md、docs/framework-menu-offscreen.md及当前component-experience-validation、历史API7、Runtime稳定性、API5/6、菜单、浏览器式宿主、视觉和Session0验收，并读docs/visual-design.md、docs/standalone-host.md、docs/version-policy.md及docs/validation/component-scroll-only-validation.md。核对实际Git状态、HEAD、API8公共头文件和现有四行CI/严格Session0工作流，保留用户改动。API5/6/7及菜单已实现，不重复开发。按docs/version-policy.md只维护接手时确认的当前版本，停止历史SDK/API/调用方/原包兼容专项；当前基础功能及完整size字段/偏移、线程、所有权和卸载合同继续验证，共同数据组件无分页或列箭头按钮但完整范围滚动/按需查询保留。本地运行不冒充托管CI，历史Session0不替代当前源码。先给简短计划，持续补验/修复和文档交付，缺环境或授权最后汇总。业务试点按docs/business-pilot.md先核对指定应用及修改授权；没有授权只准备方案。不得修改冻结SDK、原包、PERF-001或未授权请求方应用，不使用graph-engineering、子代理或新会话；没有明确授权不推送、不触发远程工作流、不强推或创建稳定标签。
 当前API8新增列宽交互/UCW1应用存储和完整help字段，见docs/migration-v0.7-to-v0.8.md；当前用例ui_api7_integration与api7_fixture文件名保留，编译声明为8，不能当作旧版本专项。
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

## 16. 完整窗口视觉打磨（2026-10-07）

接手干净43587c3e8e0cb4549da314e6c7e15a4654a10100。两个本地单元95f5d78（轻量显式padding0与像素/CI）、6a0170a（宿主/工作区/共同组件/菜单对齐和完整窗口断言），API7/std7/ABI1保持，公共12头文件不变，9df202f/2e87e02原生捕获与异步轨道修复保留。没有推送/远程工作流/标签。

当前完整矩阵54项=53通过/1物理跳过，159.86秒；此前菜单桌面与级联2项失败保留，子节点hover路由到所属行后专项与最终矩阵通过。实际Light/Runtime、完整api7 DLL/OSMesa、framework_features实际WGL和当前业务abc8源码副本只读观察已有同尺寸主题前后截图。两个独立Runtime进程各32实际就绪周期通过，最终cleanup0，原句柄/GDI/USER门槛保持；当前业务abc8副本29/30，固定旧折叠坐标用例独立仍失败，未修改请求方测试。公共presentation定位真实折叠/恢复0失败；包含真实滚轮first3的业务滚动/布局观察100/100。详细失败、版本身份说明及48张配对真实截图见[本轮记录](validation/visual-polish-validation.md)。不将沙箱Runtime未就绪、PrintWindow遗漏GL或BitBlt权限失败改写为成功。

公共视觉值仍集中src/visual.css；空标题和只读无效操作栏回收空间；属性同行/堆叠、单位/色值/勾选密度、表头/内容列宽约束、C图片比例；分组标签置于工具前并有界分配宽度。AI初始收起、高级折叠，不新增命令系统。当前版本政策、不恢复数据分页/历史兼容门槛、不改预算/应用/冻结SDK/原包/PERF。

当前源码Session0服务、严格整机无登录runner、真实IME、物理不同DPI/屏幕边缘合成、实际Snap、长期人工均另列待验。本机不是托管CI，业务只读观察/副本回归不是完整业务试点授权。

## 17. API8 共同组件、工作区与辅助体验

接手干净c62753d，产品提交7051596及最终输入修复8078281。公开列宽六接口及可选help字段集中升级SDK0.8/API8/标准8，ABI1/包1及原字段偏移/枚举不变；当前维护政策不变。初始列宽由应用决定，显式适配只使用表头和预算内缓存；UCW1应用负责存储，异常原子拒绝、稳定ID恢复。两后端共同模板增加窄列/完整文本/长枚举/帮助、响应式RGBA、长面板/停靠标签查看及溢出焦点；浮动/对话框最小逻辑尺寸和空心预览，保留AI默认收起、GL、布局与关闭合同。

首轮失败及修复、冻结产物完整矩阵、只读业务副本、真实前后截图、当前SDK和环境限制见[本轮验收](validation/component-experience-validation.md)。原业务abc8的固定坐标折叠断言与历史29/30失败记录不改；本轮匹配SDK副本仅变更依赖锁/清单元数据，业务列宽或测试改动不属于框架修复。没有推送、远程运行、标签、发布、原业务/冻结SDK/历史包修改。

最终产品8078281、测试收敛0fe0864：完整框架55通过/1物理跳过，原生20通过/1物理跳过；只读abc8业务副本29通过/1原固定坐标失败，框架自有真实业务观察102检查通过（末列STYLE实际像素），Runtime两次独立32周期通过。持续重绘来源未定位，测试消息泵修正不代表根治；原失败均保留。交付匹配SDK include/lib/bin/source及固定依赖、105张实际过程截图（8张失败图不作基线）；源码Session0服务权限错误5、无登录runner、远程CI与物理/人工分别待验。SDK归档位于本地build/experience-20261007，解包逐项sha256验证；最终文档提交不改变已冻结运行产物。

## 18. 2026-10-08 API8 稳定性与完整存储收敛

接手干净660734f。产品f0b161c只修共同模板：稳定列ID对齐，以及可见列数改变时保留顺序正确的捕获表头；没有更改公共头、原生捕获／轨道产品源码、ABI或预算。示例／测试d24cdf3提供stateful_components完整DLL／包，应用拥有UST1容器（UCW1＋ULYT）、跨进程A/B身份／锁、显式保存／加载／重置和异常最终状态回滚；真实表头拖动200→280／360、三个程序DPI、重排列／新旧列、两实例、初始化失败／WAIT／卸载均实际验证。

本轮完整框架58通过／1物理跳过（59项）、原生20通过／1物理跳过（21项）；两个独立真实Runtime各32呈现就绪重开周期，句柄389→391、GDI12／USER17平稳、pending0。WM_PAINT按实际Dispatch观测并验证主动失效正对照；完整场景及两个独立进程闲置均0paint，resize／显式GL触发有界刷新。历史持续重绘未复现／未定位，不能把这两项DOM修复或测试消息泵修正写成根治。

原业务只读HEAD ff44a0f及23项用户tracked diff保留；已登记abc8d87另建副本，仅改SDK锁／清单元数据。最终源码构建29/30（292.60秒）、实际交付SDK导入库重新链接／同DLL及宿主的完整套件也29/30（292.88秒），唯一原kc_host固定坐标折叠失败不改。框架自有观察器在交付SDK上102/102，另12次真实公共presentation折叠／恢复，同HWND、隐藏／显示、焦点及实际WGL context保留。框架提供定位迁移，原应用正式接入／原测试修改仍无授权；无公开chrome view getter，不臆造接口或改断言。

当前依赖、运行条件、失败、命令、哈希、真实完整窗口图和SDK见[本轮验收](validation/api8-stability-state-validation.md)。新增旧布局输入遗漏、诊断hook漏队列paint、Runtime HWND捕获误判、临时Mesa默认D3D12异常均单独保留，不能归为持续重绘根因。原生三项实际拖动及未缓存末列、实际GL／MSAA／非MSAA、布局／输入／关闭回归保持。SDK源码文档快照ace533a，产品f0b161c／示例测试d24cdf3；2457文件／62724776字节ZIP，CRC／逐文件哈希／解包后C调用通过。解包实际宿主在Light和Runtime共8个独立进程保存／恢复／隔离通过，SDK固定依赖离线3/3；[外部交付身份](validation/api8-stability-state-evidence-20261008/raw/sdk-archive.json)另附，不混用此前冻结包。归档后证明另行提交，包内源码快照不自包含自身归档哈希。

本轮服务创建访问准确错误5，未创建／修改服务；当前源码Session0、严格无登录runner、托管CI授权、IME／不同DPI物理屏幕／桌面边缘合成／长期人工分别待验。检测到DWM启用与一屏不替代物理验收。没有硬件无窗口GL扩围、原仓库／冻结SDK／原包／PERF改动、子代理、新会话或远程操作。下一步先读本节与本轮验收，再核对实际Git；第17节及更早记录保持原结果。

## 19. 2026-10-08 API9 应用拥有的实例语言

接手干净cf0e83a。产品ee6a2a8新增每host简中/英文、系统显示语言来源、同步通知和稳定ID原位文本更新；8db942f补充真实校验叶节点验证；6d88770允许关闭/unmount期间清除回调，继续拒绝新增回调。SDK0.9.0-dev/API9/标准9，ABI1/包格式1不变；新增公共能力不再称API8未变化。应用拥有偏好与存储，宿主跟随活动实例，后台隔离，空宿主查询Windows显示语言。没有全局语言开关或进程locale替代。完整stateful_components应用以A/B稳定档案保存UIL1，不更改UST1/UCW1/ULYT；首次呈现前提交，非法文件保持，错误回退由示例应用明确决定。

最终产品6d88770完整矩阵61通过/0失败/1物理跳过（62项，398.63秒）；重新构建的无Web原生21通过/0失败/1物理跳过（22项，6.52秒）。两个独立实际Runtime各32就绪重开，句柄390→391及391→392，GDI12、USER17、末端pending0。Light与实际Runtime的完整DLL语言套件含10种偏好文件及96/144/192跨进程写入/恢复；真实鼠标、草稿/选择、稳定宽度、HWND/GL、双实例/后台、模态、AI空间回收和关闭卸载通过。保留原生三项真实滚动、完整数据范围/按需查询、旧捕获与轨道修复、原预算和断言。

真实系统首选zh-CN；其他语言映射只是可控输入，未修改机器显示语言。锁屏输入失败、Runtime早期导航/异步清理超时、剪贴板失败、语言初版Light解析/翻译函数遮蔽及关闭回调拒绝均有独立原始日志；后续通过不抹去历史失败或宣称Runtime/持续重绘根治。详见[API9验收](validation/instance-language-validation.md)、[合同](instance-language.md)、[迁移](migration-v0.8-to-v0.9.md)及完整示例；真实前后/双实例/浅深/窄窗/DPI图有运行身份和哈希。当前只维护API9，不重启旧SDK专项。

本轮仅框架与框架自有示例，未访问业务仓库。服务创建访问错误5，当前Session0待权限；严格无登录服务runner仍无，托管CI未授权，真实IME/不同DPI物理屏幕/桌面边缘合成/长期人工待条件。本机与历史不能替代；无远程操作、发布、稳定标签、新会话、子代理或新硬件GL范围。匹配SDK产品6d88770、源码/文档快照312bce4，2648文件/96,297,031字节ZIP SHA256 `7c1dbffce1211b2c0654b41d2942b05adbd4c54d111e983d836c27550d8802e1`；[外部证明](validation/instance-language-evidence-20261008/raw/sdk-archive.json)在归档后补充，避免自引用。CRC/逐文件身份/解包C调用通过，解包实际DLL完整宿主两后端三个DPI的12个跨进程保存/恢复用例及偏好异常通过；包内固定依赖独立离线构建3/3，运行后全部交付文件哈希保持。SDK位于本地build/language-20261008；仅本地交付，未推送/发布。

## 20. 2026-10-08 获授权同步 API9 开发源码

本次接手干净a93967fa686710521baf1f030f8c0e63462210d9、codex/menus-offscreen；实际origin为本仓库。fetch后main为ea5b10816681c43b662adf351dcfe57e8ae00e7a，远端独有0/本地领先74，工作分支远端9705565为本地祖先；GitHub查询main未保护。完整提交链包含源码、API9头文件、当前清单、完整示例、测试和已入Git验收证据，不是只修改README版本号。优先普通快进更新main及原工作分支，保留远端祖先。

用户明确授权此次源码同步和自然push工作流。入口README、升级提示词、构建说明与0.8→0.9迁移修正“尚未推送/main不包含”的当前表述；开发源码位于[main](https://github.com/wbycloud/ui-framework/tree/main)，接入记录确切commit。SDK0.9.0-dev/API9/标准9、ABI1/包格式1；v0.1.0仍是历史稳定标签，源码公开不代表稳定SDK或Release发布。已封存本地SDK归档及其哈希不改写；第19节和历史验收保持原版本、失败、环境及当时仅本地交付状态。

实际托管结果只认准确head_sha对应的push运行及结论。本次未额外手动dispatch；四行Windows矩阵及严格Session0工作流已自然运行，最终结果见第21节。API9本机61通过/1物理跳过不冒充托管CI。当前源码服务Session0 GL已通过，严格无登录失败；其他实际Windows显示语言、真实IME/物理跨屏/桌面合成/长期人工继续待验，业务应用修改仍未授权。

没有产品、脚本、冻结SDK、历史包/证据、业务仓库或PERF-001修改；只做本次入口文档修正及链接/diff检查。不提交build/.deps/档案/凭据/未审核临时文件，不强推/reset --hard/删除分支、创建标签或Release。实际远端最终SHA、网页源码核对和自动CI运行记录在本次同步日志及交付报告；发生保护或权限阻塞时以真实结果为准，不绕过。

## 21. 2026-10-08 同步完成后的CI失败交接

接手本节时本地main、origin/main及本地/远端codex/menus-offscreen均为b6a36fee3f4c1a73818cdb01ea7271ad5abd30db，工作区干净。同步以普通快进完成；主页、README、CMake、公共头、当前清单/测试和标准的远端Git blob已核对，SDK0.9.0-dev/API9/标准9一致。v0.1.0仍为历史稳定标签，没有新Release。封存SDK的产品6d88770、文档快照312bce4及SHA256保持第19节原值，不能重写归档补入本次文档。

三次自动push运行都绑定上述提交、attempt1，已完成且结论failure：[main矩阵37753988354](https://github.com/wbycloud/ui-framework/actions/runs/37753988354)、[工作分支矩阵37753988621](https://github.com/wbycloud/ui-framework/actions/runs/37753988621)、[main严格Session0 37753988202](https://github.com/wbycloud/ui-framework/actions/runs/37753988202)。main的通过/失败/物理跳过分别为native 21/0/1、light 42/2/1、osmesa 47/4/1、webview2 56/5/1。真实Runtime专属阶段10/10通过，不覆盖该配置的前置失败。精确环境、分阶段结果与原日志入口见[同步验收记录](validation/github-sync-validation.md)。

下轮优先收敛五个不同失败用例，不能把各配置的重复失败算作独立根因。ui_browser_host及ui_web_host_frontend存在已确认的中文固定断言与英文runner冲突；ui_host_repaint报告1失败但idle paint为0，具体失败断言未定位，源码也含“操作完成”固定中文断言，仅是待核验线索。ui_stateful_components_light在180秒、ui_instance_language_light在240秒超时，需追踪阶段/子进程/关闭，不能以子用例0失败覆盖整项Timeout。此次同步没有改产品、测试、超时或资源门槛，CI缺陷尚未修复。

当前LocalSystem/ServiceMain、SessionId0实际应用DLL与OSMesa已得到GL failures0、commands2000、created_HWNDs0/process_HWNDs0证据。严格无登录因runneradmin的已登录Session2而失败：logged_sessions1、inventory_valid1。用户已确认没有严格无登录服务runner；不注销用户、不改既有服务、不放宽RequireNoLogin，本轮不把这项环境缺失列为产品修复。

本机原始证据位于D:\应用软件框架\应用层序框架\build\github-sync-20261008：report.md、result.json、evidence-sha256.json（437个证据文件）以及main各配置/Session0和工作分支原ZIP、JUnit、manifest、资源与像素。该目录被Git忽略，clone不会带入；同机新会话先保留并核验，其他机器从上述运行下载Artifacts/日志，若已失效则如实标缺失，不重新生成冒充原证据。不要依赖其中临时网络辅助脚本，也不要输出凭据。

本次交接只更新文档、保存提示词并本地提交，不启动修复、构建、测试或远程工作流。本次源码同步授权已执行完，不解释为后续修复无限推送授权；新会话先核对实际状态，完成本地诊断/修复及必要回归，远程验证与推送需新的明确授权。原业务仓库、冻结SDK、历史包/证据、PERF-001保持只读，不使用graph-engineering、子代理或新会话，不扩大到新功能或后端。真实IME、物理不同DPI/桌面合成/长期人工及严格无登录条件仍待验。

## 22. 2026-10-08 API9托管失败的本地收敛

接手干净main/1756b067，live远端main与codex/menus-offscreen均b6a36fee。437份原CI证据长度/SHA256全部匹配，原目录保持只读；新复现、构建和回归在build/ci-recovery-20261008，关键原始日志及产物身份另存[本轮证据](validation/ci-recovery-validation.md)。没有执行prepare覆盖固定依赖，没有推送或远程工作流。

本地f00bbeb790c64ad40c10b3ec1622a65331d3495e修正三项宿主测试的固定中文断言。空宿主依据公共Windows显示语言和来源核验完整空提示；前端两实例提交中英文并交换，保持原参数/草稿/大整数/预算/一次执行语义；repaint逐断言诊断的英文红例准确失败在“操作完成”匹配，失效区清空、Dispatch1、空闲paint0和关闭window0。修正后中英文完整GL结果、request/instance及实际图片呈现通过，未声称历史持续重绘已根治。

本地2fa6f84cb1ca202d54ff822971ca908c2e6dc36f仅补完整存储/语言脚本的阶段、PID/tick/Dispatch、elapsedMs/实际exitCode和running/exited manifest；CI新增language-evidence-*归档。原调用/消息泵/真实SendInput/关闭/DLL卸载及全部存储/语言输入保持，180/240/300秒和资源门槛未改。原托管Timeout本机未复现、根因仍未定位，诊断补充不能写成超时修复。

已完成的真实本机WebView2配置62项61通过/0失败/1物理跳过，分阶段WGL45项、OSMesa/provider7项、Runtime10项；独立无Web原生22项21通过/0失败/1物理跳过，独立OSMesa配置52项51通过/0失败/1物理跳过。每后端完整存储26个子进程、语言7个子进程均核验实际退出码；核心系统查询/实例中英文、双实例、布局/列宽、草稿/选择/焦点、模态、实际GL、AI回收、REFUSE/WAIT/卸载及原生鼠标首末/未缓存末列等原门槛保持。

当前本机为Windows10.0.26300/Session1/MSVC19.50/Runtime154.0.4258.62；真实系统仅zh-CN，实例en-US及受控映射另计，实际英文Windows空宿主仍待环境。产品源码/公共头/工作流/CMake/示例无差异，不升级API或重写SDK；封存6d88770 SDK哈希再次匹配原值。原b6a36fee Session0实际GL通过与严格无登录logged_sessions1失败保持独立，本轮仅Session1对照，无新服务复验。原托管超时、历史瞬态/持续重绘、真实IME/物理跨屏/合成/长期人工及严格无登录环境继续待验。本轮本地提交授权不包含下一次推送或远程复验。

## 23. 2026-10-09 界面修复与公共对话框布局本地交付

接手干净main/359085b58d5c69792533e3b781fd002cd481314b，已有f00bbeb宿主精确中英文断言及2fa6f84完整脚本诊断，不重复修复。437份原托管证据和冻结SDK大小/哈希保持；新原始证据在build/ui-repair-20261009，随Git关键证据及完整逐项结论见[本轮验收](validation/ui-repair-validation.md)。本轮仅框架本地工作/提交，没有推送、远程工作流、业务修改、冻结SDK/历史证据/PERF-001改写、graph-engineering、子代理或新会话。

af36e02f5483dd174d91d32da1868dcbb162dc82修复GL上拖动轮廓、抓取偏移/尺寸、实际边角命中和提示生命周期/所属范围；e0dcf788365c0023b57f7f41734f5e488477a70c修复只读多行、Light滚动保持和End尾部，补公共对话框首选/最小/最大客户区及字段行数/高度；07a66a82370db93c49a66c90db5e217a8812e60e修复Light长说明自身换行范围/绘制，保留窗口/离屏像素红例；4fdfcc2d5f679166064355abe08bc8e855f6a5eb只校准Runtime滚动条实际输入点。所有原失败及“探针0失败但画面不合格”保留，不以状态代替可读性。

最终受影响Light/OSMesa/真实Runtime矩阵重跑，加未受最后Light变更影响的native先前执行，共181通过/0失败/4次物理monitor_transition跳过，原预算及精确清单门槛保持。原生SendInput两组浮动各29张桌面过程图，经过GL、目标切换、Esc、受控捕获丢失、实际角点调整、草稿/焦点/HWND/HGLRC保持通过；Light/Runtime真实滚轮及内层滚动条看到完整41行末行，外层最后字段、窄对话框错误/长说明末行及按钮/Enter/Tab/Esc通过。SendInput不是硬件鼠标人工验收，受控capture loss不是外部软件抢占。

SDK0.9.0-dev/API9/标准9、ABI1/包1保持，新增独立完整size描述及三C导出，现有结构偏移/枚举不动；[合同](dialog-layout.md)、公共C示例、版本/标准要求同步。冻结API9无新导出，必须匹配开发commit的headers/import library/DLL。新独立快照ui-framework-sdk0.9.0-dev-api9-07a66a8-windows-x64.zip，SHA256 b5296fba8d39447603222705ab85edcf446fa5c98088d86c76a153a4fd9c60f0；解包C /W4 /WX编译及实际共享DLL调用通过，不覆盖旧包或称稳定发布。

原托管180/240秒超时和历史持续重绘仍未定位；六份本机完整脚本均实际退出不能证明托管已修复。泛化192叠层及窄表头残影未独立复现，保持尚未定位。程序192/窗口96/monitor effective96、raw108分开记录，不冒充物理高DPI。原b6a严格Session0 GL通过和整机无登录logged_sessions1失败分别保留，本轮无当前源码服务复验。真实英文Windows空宿主、硬件鼠标/外部捕获、IME、物理跨屏/边缘合成/长期人工及严格无登录环境另列待验；未来远程运行需新的明确授权。命令搜索、嵌套分割、冻结列/可变行高只交付[后续范围和验收方案](workspace-feature-candidates.md)，不替代遗留缺陷。

## 24. 2026-10-09 当前源码实例语言复核与完整SDK

接手干净main/876588246a72d261c88b9e1906372d1b5b91d1ca，SDK0.9.0-dev/API9/标准9、ABI1/包1；用户参考API8/cf0e83a已过时。核对已有应用自有UIL1、每host语言/来源/通知/原位文本、活动宿主跟随与空宿主系统UI查询，不重复新增接口或产品实现。保留视觉/原生捕获/异步轨道/列序、f00bbeb精确语言断言、2fa6f84阶段诊断和第23节界面修复。本轮未访问业务仓库，无远程操作或服务修改。

本轮新构建与[独立复核记录](validation/instance-language-current-validation.md)绑定8765882：完整64项63通过/0失败/1物理跳过（436.60秒，46前置/7provider/11实际Runtime），无Web原生22项21通过/0失败/1跳过（10.09秒）。Light及真实Runtime的完整示例均验证10种偏好输入、96/144/192跨进程write/restore、双实例/后台、草稿/选择/宽度/HWND/实际GL、模态、AI、关闭和卸载。原生三项真实鼠标滚动、按需查询/uint64、预算/生命周期断言保持；没有恢复分页按钮。

另用框架自有完整DLL观察器补验两后端各两个中英文权限确认快照：目标A非活动，打开后改变A/B语言，确认完整标题/按钮保持打开时目标语言；真实取消、AI目标/活动实例/HWND与主窗口启用恢复、卸载保持。只测宿主确认适配器，不执行业务命令；正常权限/一次执行另由矩阵覆盖。辅助脚本HOME保留变量、手动观察器缺失/错误资源链接及构建调用错误均保留，修正夹具后原断言通过，不宣称产品缺陷修复。

当前真实系统UI仅zh-CN/provider1/fallback0；实例en-US和其他映射不是实际英文Windows空宿主验收。新截图为完整当前应用/DLL，1200×800/DPI96深色中英文同状态，另含双实例、AI、空宿主和中英文确认；原BMP及PNG哈希保留。开发标准当前声明8残留改为9，菜单/API9验收指向修正，现有README/合同/迁移/构建/宿主/组件/布局/示例/升级提示词同步；历史记录不改写。

本轮归档按精确开发commit提供匹配headers/import/static libraries/shared DLL/host/full packages/source/fixed dependencies/licenses，与旧6d88770和07a66a8归档分开。文档修正前的8765882基准SDK3501文件/117108072字节，SHA141a09ac741f92c7d02b43ea69cef58916b4a09e99f1e23c8d39d61e99d226f8，CRC/逐文件/解包C调用及两后端完整实际解包语言套件通过，保持原归档。

当前最终SDK为ui-framework-sdk0.9.0-dev-api9-44c039f-windows-x64.zip，源码/说明快照44c039fce36c191dba03631a7bcc1c5756e2dd90（首轮文档提交）；3602文件/117891276字节，SHA256 28ca0b8409b321477f1aa3adce2680fda8cf2cd8a9a35dd4308a5f08ef1f79a2。原产品/头文件/测试没有新差异；CRC/逐文件/实际解包C调用通过，最终解包完整应用两后端14个子进程均实际exited0。包内固定依赖离线构建成功，语言core/C++公共头/实际OSMesa三项通过；选错Ninja目标的原失败保持。之后全部交付文件和旧SDK哈希仍匹配。[最终外部证明](validation/instance-language-current-evidence-20261009/raw/sdk-final-archive.json)另作本地提交，不回写已封存归档；最新HEAD见git log，不能把文档提交混作产品测试身份。

全部原证据在build/language-current-20261009，关键原日志、运行器、确认观察器、manifest和18张真实PNG保存到docs/validation/instance-language-current-evidence-20261009，Git属性保留原字节。本机Windows10.0.26300/Session1/MSVC19.50/Runtime154.0.4258.62，固定依赖未改。当前会话非管理员，本轮没有服务Session0复验；原b6a36fee Session0 GL通过和严格无登录logged_sessions1失败分别保留。没有严格无登录runner和新的托管CI授权；真实其他系统UI/IME/物理不同DPI/桌面合成/长期人工待验，原托管超时/历史重绘不宣称根治。仅本地提交，不推送/发布/标签/新会话/子代理/graph-engineering，不改冻结SDK/历史包/证据/PERF-001，不扩大硬件GL。
