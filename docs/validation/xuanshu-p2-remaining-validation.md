# XuanShu P2 剩余范围：代码、自动验证与人工准备

日期：2026-10-23。状态：**代码和限定自动验证已交付，准备进入集中人工验收；不宣称P2完整验收通过或全部CTest绿色**。真实鼠标/键盘、完整Windows来源动态矩阵及其他人工项目均未执行。

## 1. 独立基线和归属

实际HEAD `e0eca03db25930be55bdb45477027d1793a61904`，分支`codex/p1-process-boundaries`，本轮没有提交。证据根`build/xuanshu-p2-remaining-20261023-b84e6f`。入口`baseline.json`有17381个文件身份，含三根规划、已有Git可见及此前交付路径；`identity.json`、`git-entry.txt`保留初态。工作区原有大量修改与未跟踪文件，不将HEAD至当前差异整体称为本轮实现。

前轮C2交付35项、源码242项、二进制392项在接手时全部匹配，见`prior-evidence-check.json`。C1通过限定验收，只沿用其边界，不重做全面复审。A+B/C1/C2AB/C2集中收口原目录、首次失败及交付清单保持；本轮独立`delivery-files.json`按入口文件哈希计算增量，`preservation.json`逐项区分改变、原有未改、新文件及缺失，`source-after.json`锁定最终源码。

参考工程一次读取被权限拒绝，没有反复申请、刷新或修改；旧main/67eacf5身份只是历史记录。入口baseline中446项路径含sdk字样（含脚本、截图和历史材料），不等于冻结SDK目录的全量清单；没有被忽略`.deps`的完整清单，不能宣称对整个冻结SDK/固定依赖树作过入口/出口全量哈希比较。本轮没有向这些目录写入。固定yyjson源码、source-lock及许可证未改，离线CMake哈希门仍有效；Lexbor/QuickJS/Mesa/WebView2固定身份没有升级。

## 2. 实现及复审整改

| 项目 | 实现与边界 | 证据 |
| --- | --- | --- |
| HMENU-1 | 框架自有MNS_NOTIFYBYPOS菜单、私有菜单输入链、匹配叶项和一次性消费；程序WM_COMMAND不能授权；菜单loop租约、退出后安全刷新 | capability_native_menu，真实菜单句柄；物理来源待验 |
| CI-1 | 标准单元格编辑/布尔格、树重命名、选择/排序/context，完整ID及动态参数；编辑base/generation、cached disabled/readonly门；旧消息不得重复回退 | capability_component_inputs最终64/0，组件两分支、实际应用 |
| AP-1 | 显式应用UI安全poll，真实worker退出确认后完成/关闭；不join、不自动跟踪任意线程；poll私有宿主租约与能力回调租约分开 | app_poll真实DLL32/0及原生命周期回归 |
| 目录metadata | discover_v2不再把完整描述塞入256字节临时格式缓冲；仍按原目录限额追加 | capability_v2新增4096字节描述回归，原限额不变 |
| 本地选择continuation | 选中后的树展开、焦点/导航和mouseup→click保持；不携带可复用授权；右键down/up完整来源链 | generic/features各自编译分支、组件回归 |
| 动态核对菜单 | UNKNOWN出现后的菜单刷新使用保留content的ui_shell_refresh；避免删除应用原生子窗口 | p2_support_unknown实际HWND，修前17检查/1失败；修后20/0 |
| 示例模型 | 五分支55项描述；共享辅助器只投影同一目录并完成同一业务run，不建第二执行器；read 11项，人工非read 44项 | p2_apps真实五DLL267/0，应用专用回读见下 |
| 文本安全/原功能 | C模型拒绝嵌入NUL与超出原字节容量；框架合法JSON/JCS不改。保留真实选择anchor/Ctrl/Shift和原容量 | 各业务测试；stateful最后201/0 |

准确功能—入口—能力ID—业务实现—测试矩阵见[五示例覆盖](../xuanshu-p2-application-coverage.md)，新增接口的实现前设计见[本轮规格](../xuanshu-p2-remaining-implementation.md)。stateful的UST1/UIL1仍只保存原偏好，不是新业务文档格式。首轮合法模型name=Untitled；required字段合同不因旧空初值测试而放宽。

## 3. 公共合同和兼容边界

SDK0.10-dev/API10/标准10、ABI1/包1不变。旧公开结构、v1只读及错误语义、冻结C2A-1/B-1/C-1未改。新增独立`capability_component_inputs.h`中两个CI-1入口、独立`app_poll.h`中一个AP-1入口及必要独立结构；同目录、原预算/队列/注销机制。AP-1是生命周期通知，不是人工授权。

Assistant/普通程序调用只能执行read，source自称human没有资格。资源固定实例/open epoch/revision；非read只经已接入人工链；busy不排队，取消/forget不清UNKNOWN。共享示例对失败的异步完成只重投同一次completion，绝不重执行业务。worker由应用确认真正退出；同进程合作边界不承诺恶意DLL隔离。

现有stateful偏好写文件属已授权保留功能，采用固定路径、原格式后台I/O，保守结果未知与显式核对；没有引入通用文件/网络/external/AI写入适配。程序Assistant修改测试转为明确拒绝加实际readback；合法人工语义在专用夹具验证。旧物理输入脚本不冒充新正例。

## 4. 本轮自动证据

全量阶段索引`test-index.json`和每阶段`*-sources.json`、原始`*.log`、各配置JUnit保留。MSVC19.50.35727/Ninja/Release；Light使用本地Mesa24.3.4显式OSMesa路径，WebView2关闭；ASAN原生独立配置。新测试TIMEOUT15秒；既有超时/容量/范围未放宽。使用专用新构建目录，没有写冻结SDK。

| 阶段 | 实际结果 | 说明 |
| --- | --- | --- |
| joint-regression Native | 25/25 | C1/A+B、Assistant、公共头、真实DLL/新增生命周期及原生合同 |
| joint-regression ASAN | 25/25 | 相同选定原生范围，非Light渲染ASAN承诺 |
| joint-regression Light | 41/43 | resilience旧策略断言与surface_input失败；不是43/43 |
| final-component Native / ASAN | 各10/10 | 后续公共组件/原生修改相称回归，非重跑全部 |
| final-view-contract Light | 8/8 | 已改应用/组件/韧性范围 |
| unknown-content-fixed Light | 7/7 | 五应用、真实包及原生content存活回归 |
| business-text-final Light | 7/7 | 五业务分支及实际DLL，C字符串前提/长度/非法值 |
| selection-parity-final Light | 2/2 | 真实包267/0及stateful179/0，选择与排序保留 |
| unknown-text-final Native / ASAN | 各1/1，20/0 | NUL失败先于效果、UNKNOWN保持及实际HWND |
| acceptance-binary-load Light | 1/1，267/0 | 匹配五包真实装载，8.29秒；非人工输入 |
| preference-failure-final Light | 1/1，201/0 | 实际固定临时路径失败、worker完成UNKNOWN、取消/forget/重提交不清门、核对无法证明仍阻止 |

最后业务分支检查数是stateful201、EDA109、generic106、features123、counter37、真实包267；数量仅用于识别运行，不代替合同。共享generic默认/API4分别验证，真实DLL能力生命周期两个编译分支覆盖。最末stateful只增加测试，产品二进制没有被这一测试修改。

原C1的Native17/17、Light17/17、ASAN11/11、20923检查，以及此前C2的22/30/22等是历史证据，不算本轮新通过。

## 5. 首次失败和修正证据

`failures/<阶段-配置-类型>`保留当时源码及产物，原log/JUnit不删除。每次test前来源清单对应当时磁盘；runner不在编译失败后伪造成功。`unknown-content-red-light-test`曾在前置编译失败后尝试，属于Not Run，**不是红色复现**；其后`unknown-content-red-include`真实执行17/1才是content缺陷证据。

真实失败包括：stateful早期错误Schema初始化后未短路导致测试进程失败（修fixture初始化与guard，不放宽Schema）、组件动态collector覆盖base、AP-1复用能力租约阻止安全退休、长metadata发现失败、EDA原生WM_SETTEXT测试未生成预期通知、树展开continuation丢失、UNKNOWN动态菜单删除content。分别保留初次日志、源码、匹配执行二进制及修正后结果。编译失败（API名/链接/共享CRT/unused等）没有新执行二进制；快照里的旧二进制不能作为“失败编译的匹配产物”。

resilience失败先尝试保留旧空name，但这违反已冻结C2B投影，已回退。最终仅SelectAll再输入保留原精确值断言，并把旧程序Assistant修改断言改为PERMISSION_DENIED+实际view未改变；合法view执行在两业务分支专用回归覆盖，未删业务功能或放宽时限/范围。

Light联合`ui_surface_input`的thread modifiers首次失败仍未定位。随后限定重跑及Native/ASAN通过只说明有不同观测，不证明已解决。相同断言在原A+B closeout日志已有记录；本轮win32_surface.c与入口哈希一致，但不能据此排除所有共享路径影响。当前优先级为后续输入稳定性核对项，集中物理验收重点观察，不默认归因环境。

原browser20项失败、workspace首次20.05秒（实际门槛20秒）、Native输入首次失败继续独立分账。本轮未执行browser20项，不修复其未定位根因。workspace/输入后续通过均不覆盖首次；没有证据将原浏览器失败定位到本轮新路径。Web模板旧textContent diagnostic在20261008/09日志也存在，本轮没有借机修复或称其无害。

## 6. 人工准备、未验和停止位置

`manual/`含匹配两个宿主、运行库、五包、当前开发SDK头/导入库及哈希；固定OSMesa使用原路径哈希引用，不把整套Mesa/opengl32.dll复制到宿主旁改变WGL。`Start-P2Manual.ps1`、`Invoke-P2InjectedNegative.ps1`只做语法/结构核对，未运行人工宿主或注入。详见[逐步人工清单](../xuanshu-p2-manual-acceptance.md)。

真实鼠标/键盘正例、完整Windows动态分类、菜单各物理路径、IME/跨屏DPI、长时运行及未捕获的真实busy/取消窗口均待验。私有来源夹具、程序编辑和SendInput不能替代物理正例。源码/自动合同的准备不等于P2完整验收；联合Light曾失败的输入项仍明确列出。

没有P3/AI Service/Provider/Coordinator/存储/恢复/通用事务撤销/跨实例共享文档写入；没有远程工作流、提交推送或参考工程改动。整体架构文档仍Proposed；本轮实现状态单列。终点为集中人工验收开始之前，后续由用户操作，不自动启动P3。

## 7. 保全和清理

`preservation.json`、`binary-files.json`、`failure-files.json`、`evidence-files.json`与`cleanup.json`分别记录入口文件保全、最终及失败二进制、失败源码/材料、原日志及本轮清理。清理只限本轮无用构建中间产物/模板脚本，不动先前证据、用户改动、固定依赖及冻结SDK。保留失败匹配产物、源身份、CTest/JUnit和CMakeCache以便复核；这些诊断保留不算无用途垃圾。完整改变文件列表以交付清单为准。
