# SDK0.6 / API6 独立开发验收

日期：2026-10-05。接手时为干净的`codex/menus-offscreen`、HEAD `ea5b10816681c43b662adf351dcfe57e8ae00e7a`，与当时main/工作分支相同；先读取handoff、标准、菜单/离屏、共同组件和API3/4/5及Session0记录。保留API5已有实现，不修改请求方应用、PERF-001、冻结SDK和原二进制包，未使用graph-engineering、子代理或新会话。

SDK0.6.0开发版、框架API6、标准修订6；运行库接受API1–6，ABI1、`ui_app_query_v1`及包格式1不变。没有新增稳定标签。独立开发覆盖交付收敛、布局和组件体验；目标机器及人工项目分别列出，不能宣称全部验收通过。

## 1. 设计、依赖和成功标准

| 顺序 | 设计与兼容策略 | 可复验成功标准 |
| --- | --- | --- |
| API5收敛 | 核对实际源码修正文档；固定依赖，四种C11 CI配置；Session0与严格无登录分开 | 实际原生/轻量/Runtime/OSMesa回归，保留失败及环境条件，不以编译或模拟通过替代 |
| 布局保存/恢复 | 应用存储格式1字节，稳定面板ID；验证全部输入后更新现有容器 | 格式/长度/版本/重复ID/异常字段、缺失新增面板、重置、DPI/work area夹紧；失败输入不改变布局 |
| 分隔条、拖拽 | 先约束侧栏/栈尺寸，再命中、预览、提交/取消；复用原内容和生命周期 | 窄窗、捕获取消、Esc、双实例/模态、520次重排；保留草稿/焦点/选择/组件和GL context |
| 共同组件 | 原options下拉、RGBA文本/调色；表格键盘/编辑，完整数据源排序、稳定ID范围查询 | 同一测试运行轻量及实际Runtime；100000完整行、>2^53 ID、跨页导航、一次命令、迟到结果失效，预算不增加 |
| 独立应用集成 | 公共C ABI DLL/.uapp，树/表/属性/OSMesa C图片、后台缩略图及语义命令 | 初始化失败、双实例、模态/焦点、布局/resize、图片更新释放、WAIT/join/unload、实际frame和回收 |

先建立失败用例再实施；最终合同见[布局](../workspace-layout.md)、[共同组件](../generic-web-ui.md#api6-组件交互完整排序与选择)、[迁移](../migration-v0.5-to-v0.6.md)。当前布局仅左右侧栏有序栈与浮动，不宣称任意嵌套或标签式停靠。无窗口Windows workspace执行逻辑布局及手势提交，不产生窗口预览或原生指针捕获。

## 2. 实际提交与依赖

| 提交 | 可审查单元 |
| --- | --- |
| [1cdc50e](https://github.com/wbycloud/ui-framework/commit/1cdc50e) | API6追加接口、SDK5冻结、布局/组件及独立DLL用例 |
| [214b957](https://github.com/wbycloud/ui-framework/commit/214b957cd4b181a27bbb4e54633e32b9abea0885) | 固定依赖的四配置Windows CI |
| [a6580a8](https://github.com/wbycloud/ui-framework/commit/a6580a8c003517bd13ef3d595542034744ef5c11) | 公开有界编译诊断 |
| [f269cd8](https://github.com/wbycloud/ui-framework/commit/f269cd8) | 工具提示可见时裸Alt仍进入菜单，保留复现断言 |
| [be7910e](https://github.com/wbycloud/ui-framework/commit/be7910e2ba3740c724d39505196049394e40ed45) | cmd解析实际批处理路径 |
| [9460d5d](https://github.com/wbycloud/ui-framework/commit/9460d5df6f371c4f47901ae15d3c334c53cf867c) | CI显式llvmpipe、真实呈现矩形诊断 |
| [8c5208f](https://github.com/wbycloud/ui-framework/commit/8c5208fce64f8e18e13280345397d4d411d70925) | 最小WGL DLL部署、CI桌面/Runtime条件、实际GL C图片捕获断言 |
| [98ad176](https://github.com/wbycloud/ui-framework/commit/98ad176c4fa9919b8a221dc473d22b661d9b5d0b) | Runtime即时阶段日志，保留创建期取消资源失败 |
| [376bf6b](https://github.com/wbycloud/ui-framework/commit/376bf6b6bfce6bdca45d2b1fea94f88af24c748a) | 取消创建后以内部分离的空白导航完成启动，再延后Close；原资源门槛不变 |
| [44809cd](https://github.com/wbycloud/ui-framework/commit/44809cd6d686353c447076c92a46c9803806adc9) | 显式软件WGL与实际Runtime平台图形分阶段，合并全部结果及完整输出 |
| [fc6a626](https://github.com/wbycloud/ui-framework/commit/fc6a6267b30811bb8f6f3015be3d78a8404d8f69) | 严格STA清理归零，自身句柄类型与异常/正常退出诊断 |
| [4d41dc5](https://github.com/wbycloud/ui-framework/commit/4d41dc5ec1fdf6292787decad3d76fcda770b596) | 延后Close前Stop导航，资源比较前后同样排空；本机通过但CI仍失败，保留记录 |
| [5479105](https://github.com/wbycloud/ui-framework/commit/5479105b43e40879de9521e7a9a3fad5db0e6718) | 取消启动只接受指定内部导航ID、忽略初始about:blank、事件所有权仅释放一次 |
| [9705565](https://github.com/wbycloud/ui-framework/commit/9705565) | 指定空白导航后等待renderer确认，再延后Stop/Close；保留原断言、移除干扰正常回归的快照诊断，并直接验证GL对象跨布局保留 |
| [a56036f](https://github.com/wbycloud/ui-framework/commit/a56036f) | README/标准/构建/迁移、布局与共同组件合同收敛 |

文档提交不更改产品实现；最新HEAD由Git核对，不把文档版本当作执行源码。

本机Windows x64、已登录普通账户、单显示器，MSVC19.50、Windows SDK10.0.26100、C11 Release/CMake/Ninja；公共头文件C11/C++及框架新增代码`/W4 /WX /utf-8`通过。实际WebView2 Runtime154.0.4258.53；窗口WGL为Intel UHD770、GL3.3 compatibility，真正无窗口为Mesa24.3.4 llvmpipe LLVM19.1.7、GL4.5 compatibility软件GL。

| 依赖 | 固定身份 |
| --- | --- |
| Lexbor | `7fb22cf5664a331d7c24b113489e566767c9c25a` |
| QuickJS-NG | `2f0aa72a6b09cf69ff06399bcf9f4c083ac1a278` |
| WebView2 SDK | 1.0.4129.50，归档SHA256 `d3934f482d484b89fb4825df720c710664e1143a1e90f7b3a60794ef33f473d2` |
| Mesa24.3.4 MSVC x64 | 归档SHA256 `7ebc711ad1896ac88ab21e142f1017f8ff035f0f342bdb72fbb5e2eb881ba363` |

组件Runtime最低138.0.3351.48并检查实际ControllerOptions4；此前托管镜像131不满足输入桥接条件。OSMesa是真实软件桌面GL，不冒充GPU；提供方模块/线程缓存可驻留，surface/context/FBO及应用DLL按生命周期回收。硬件无窗口路径不在本次范围内。

## 3. 本机回归与实际应用输出

最终执行源码为9705565；完整配置包含两个Web后端、显式OSMesa及五个事先保全的原包。此前376bf6b、4d41dc5、5479105的本机完整回归通过不代替它们仍失败的实际CI结果。

| 检查 | 结果与证据 |
| --- | --- |
| 完整必要回归 | 51项，50PASS、1物理跨屏SKIP、0FAIL；[执行摘要](api6-full-regression.log)、[逐项记录](api6-full-tests.xml)、[完整单项输出](api6-full-output.log)；JUnit单项输出有限长，资源诊断以完整日志为准 |
| 无Web依赖的原生配置 | 21项，20PASS、1同项SKIP、0FAIL；[输出](api6-native-regression.log) |
| 明确Mesa软件WGL的原生对照 | 21项，20PASS、1同项SKIP、0FAIL；[输出](api6-native-mesa-regression.log) |
| 早期七项过程回归 | 布局、冻结SDK5、两后端共同体验/独立DLL、原API2包PASS；[过程输出](api6-targeted-regression.log)未单独记录HEAD，最终证据以上述明确源码的完整回归为准 |
| 独立API6应用真实GL | 64×64、实际samples4、123个中间值抗锯齿像素；C图片更新及两后端实际捕获有白色GL像素；应用display-list对象在浮动、恢复、分隔条和拖拽后仍有效，不能仅凭samples相同推断context保留；[完整输出](api6-full-output.log) |
| 完整无窗口应用生命周期 | 轻量离屏+实际OSMesa frame、后台队列、双实例、模态、布局及8次重开，CBT记录创建HWND0 |
| 关闭回收 | 每次DLL卸载，GL/context及测试DLL计数归零；8次重开沿用句柄≤+12、GDI/USER≤+4断言；实际数值见详细输出 |
| API5已有四项/非MSAA | Alt/重复键/嵌套/溢出/焦点、隐藏WGL与NO_WINDOW OSMesa、精确MSAA/resolve/状态/预算、Runtime查询捕获/C图片/组件/32次卸载、旧非MSAA边界均PASS |

日志按原UTF-8或Windows编码转为UTF-8，规范化行尾空白，并将工作区路径替换为`<repo>`；失败记录另行保存，不改数值或删除断言。完整无窗口测试不是`ui_framework_headless`的UNSUPPORTED适配器。

以下两张PNG来自独立DLL实际PPM读回的无损转换，既未生成图片也未重绘。Runtime路径的GL仍由同一实际OSMesa frame运行，发布C图片到共同组件后另做真实CapturePreview验证；不能把图片显示当成Runtime拥有无窗口GL context。

![轻量离屏应用实际GL frame](api6-light-frame.png)
![Runtime集成应用实际GL frame](api6-runtime-frame.png)

## 4. CI配置、运行条件和结果

[工作流](../../.github/workflows/windows-regression.yml)使用Windows-2022 x64、固定工具/依赖，四个独立构建目录。软件WGL仅部署四个必需DLL并显式llvmpipe；实际renderer在测试中查询。Runtime行先执行40个非Runtime用例，再移除本行部署的四个WGL DLL，以平台图形执行7个真实Runtime用例；合并全部结果，任一阶段失败仍失败。OSMesa继续使用绝对提供方路径。托管GUI仅在临时VM设置1920×1080虚拟桌面，记录原/实际尺寸及DPI；宽窗和窄窗断言均保留。这些虚拟条件不算物理显示器验收。

| 配置 | 能力条件 | 最新结果 |
| --- | --- | --- |
| native | 无两个Web依赖，无OSMesa配置；显式软件WGL | 21项，20PASS/1物理SKIP/0FAIL；[输出](api6-ci-native-regression.log) |
| light | Lexbor/QuickJS，共同组件，未配置OSMesa | 37项，36PASS/1同项SKIP/0FAIL；[输出](api6-ci-light-regression.log) |
| webview2 | 两Web、真实兼容Runtime、显式OSMesa实际DLL | 47项，46PASS/1同项SKIP/0FAIL，7个实际Runtime用例全部PASS且正常退出；[输出](api6-ci-webview2-regression.log) |
| osmesa | 轻量与显式OSMesa，实际应用DLL/GL，关闭Runtime | 40项，39PASS/1同项SKIP/0FAIL；[输出](api6-ci-osmesa-regression.log) |

实际源码9705565、[运行37231692624](https://github.com/wbycloud/ui-framework/actions/runs/37231692624)四行绿色。本机完整50PASS/1SKIP用时256.11秒；取消重开重复三次均383→384，原+12门槛保留。CI为Windows Server20348、镜像20260927.320.1、Session2、DPI96、1920×1080虚拟桌面，Runtime154.0.4258.53；[环境](api6-ci-webview2-manifest.json)、[全部公开注释](api6-ci-results.json)、[原始日志/二进制/像素身份](api6-evidence-manifest.json)。该次通用manifest的osmesa描述为静态字串，实际仅表中两行启用；交付脚本已按配置区分禁用行。工作流上传完整输出/合并JUnit/环境/实际PPM为30天artifact；GitHub单步骤notice数量受限，公开test-output文件是摘录，不能冒称完整远端日志。本机[完整单项日志](api6-full-output.log)永久保留。

只允许缺物理条件的`ui_monitor_transition`跳过；Runtime创建、导航、资源或像素失败仍失败，组件/应用DLL实际Runtime用例必须存在。CI不带本地原二进制包，它的冻结调用方及重编译fixture不能代替第5节本地原包证据。

失败运行[37222965425](https://github.com/wbycloud/ui-framework/actions/runs/37222965425)、[37223445891](https://github.com/wbycloud/ui-framework/actions/runs/37223445891)保留批处理调用错误；[37224257053](https://github.com/wbycloud/ui-framework/actions/runs/37224257053)保留宽窗被1024×768桌面约束、属性栏隐藏和Runtime131不兼容的失败。取消资源与退出失败一直保留到[37229816453](https://github.com/wbycloud/ui-framework/actions/runs/37229816453)，[37230663007](https://github.com/wbycloud/ui-framework/actions/runs/37230663007)保留新增快照诊断的异常栈，随后9705565实际复验通过。没有把失败改成通过。[失败与修复](api6-failures-and-fixes.log)及[CI失败原注释](api6-ci-failure-checks.json)保留原始摘录、源码和最终用例入口。

## 5. ABI和原二进制兼容

旧SDK1–4未改，SDK5头文件及四份历史测试逐字冻结于ea5b108。API6只追加字段/枚举/函数；旧字段偏移和默认行为保持。SDK5调用方在PAGE_NOACCESS边界运行旧descriptor/state/source前缀，0失败。部分新字段不完整时不读/不写；残缺指针毒值、旧尾padding及非法selection_flags均有回归。没有为内部菜单修复无故另升ABI。

完整字段偏移和应用升级责任见[迁移](../migration-v0.5-to-v0.6.md)。旧API5的“API6必须拒绝”版本断言相应更新为拒绝API7并加API6成功，其余失败预算保留原断言。

| 原包API | SHA256（保全后未重建） | 实际验证 |
| --- | --- | --- |
| 1 minimal_eda | `86988546516e17b4783c5a9ed1583dfaa9e73ecdfcd1f5b8fad9a4910a5a3503` | 真实宿主原包/混合实例PASS |
| 2 web_counter | `2cf7a9619365184211cb636db6537c6119f03bc40d41d92edfbe6c235d3ccb2f` | 原包测试及混合宿主PASS |
| 3 generic_components | `d024c191b6fcae0da5eecc71b29c3429e27a74db9ec339a118a9577c691c0745` | 真实宿主/编辑/虚拟化/混合实例PASS |
| 4 framework_features | `ccc1ddd03068aaf8d68682ffe9a6d16afbb93939ded65c6925e5deee60ddd568` | 原API4包菜单/宿主PASS |
| 5 api5_fixture | `285431bddc0d00bebed153c6441c00fc9be98793a79a50dd0a31c5b800645f19` | 原API5 DLL/实际Runtime/GL/32次关闭PASS |

[冻结及哈希输出](api6-original-packages.log)保留身份；新clone缺这些原包时必须报告缺失，不将新fixture顶替旧包。

## 6. 失败、修复和复验

| 失败用例 | 观察与修复 | 最终证据 |
| --- | --- | --- |
| 先写布局用例 | 缺9个接口链接失败；fixture漏必需entry_url后41断言失败 | 实施真实入口并修正fixture；格式/手势/边界用例PASS |
| source/布局边界 | 六处残缺字段/非法flags失败；新增flags拒绝路径错误释放借用ID造成heap异常 | 按完整字段末端判断，在取得所有权前验证flags；PAGE_NOACCESS及原包PASS |
| 实际Runtime键盘 | 六处键盘导航/编辑失败，桥接把keydown命中在指针0,0 | KEY_DOWN取activeElement，指针保留Ctrl/Shift/Alt；相同Runtime测试0失败 |
| 非法颜色文本 | 轻量将非法颜色赋给CSS在校验前抛异常，2失败；Runtime0失败 | 只预览合法CSS，非法草稿仍投递C校验；两后端一致PASS |
| 模态综合fixture | 测试错误地调用普通命令代替模态提交，3失败 | 经真实dialog按钮输入提交，草稿/焦点和门控PASS |
| 裸Alt+工具提示 | 原API4宿主与menu_access可复现共3失败 | 只有正式菜单弹窗优先，工具提示不吞裸Alt；同复现及完整回归PASS |
| CI调用、桌面与Runtime | 相对带斜线cmd误解析；Help溢出及form零宽；Runtime131无Options4 | 解析绝对cmd路径，临时虚拟桌面1920×1080，安装并核验兼容Runtime；不放宽原断言 |
| CI复制整个Mesa目录 | 本机5个原生窗口用例异常c0000409，实际GL本身可运行 | 仅部署4个WGL必需DLL，同21项20PASS/1物理SKIP；保留原失败XML |
| Runtime创建期取消资源 | 更新Runtime后CI12轮句柄301→316超过+12；本机相同Mesa部署API6一次487→500同样超限 | 内部空白NavigationCompleted取代初始文档ExecuteScript排空，再异步Close；不装载应用文档/回调，不增加预算，复验原取消及综合门槛 |
| 取消导航错误完成/迟到IPC | 初始about:blank也会完成；仅接受任意完成事件或NavID完成后立即Close仍有残留 | 只接受内部导航ID、清空事件所有权后再调用SDK、等待已完成空白文档的renderer脚本确认；早期脚本在初始文档上执行的失败方案不重用 |
| 新增快照诊断 | 5479105实际CI两项资源断言均只增1，但随后快照诊断调用栈在ntdll出现c0000005 | 移除本轮新增Pss诊断，保留原全部资源/清理断言、即时日志、异常栈及正常CRT退出检查；目标实际退出仍必须复验 |

受限执行沙箱曾禁止Runtime子进程，使导航超时；在正常允许子进程环境运行同一测试通过，未关闭浏览器沙箱。测试的相对DLL路径/清单初始化错误属于fixture修复，不冒称产品缺陷。

## 7. 可复验步骤及最后环境审计

按[构建说明](../build-and-validation.md#api6-回归矩阵与独立应用)准备固定依赖，在x64 VS开发终端构建。保全原包并用五个UI_LEGACY_*配置项指向它们；无原包则缺失项单列。执行完整CTest，再按需要用`-R "ui_workspace_layout|ui_component_experience|ui_api6_integration|ui_api5_compat"`复验。Runtime需要STA消息循环及正常子进程权限，OSMesa用明确绝对提供方路径。

预算保持原值：DOM1024、组件缓存2MiB、单批512、图片32MiB、每实例投递8MiB、JS8MiB；选择内存计入缓存，不靠提高上限完成体验。迟到结果按实例/generation/request/内容身份丢弃，关闭仍停止并join worker、排空有效回调后卸载DLL。

| 最后审计条件 | 结果和后续要求 |
| --- | --- |
| Session0 GL | 已有实际LocalSystem CI200轮/400实例/1200frame/2000命令、HWND0和回收成功；[原始记录](session0-osmesa-validation.md)保留精确源码。交互对照不计作Session0 |
| 整机无登录 | 用户确认无合适服务模式Windows x64 runner；托管runner登录会话1、严格NO_LOGIN门槛失败，保持待验。取得真实标签后运行同一严格工作流，不注销用户或改既有服务 |
| 中文IME/物理不同缩放跨屏/边缘/桌面合成/长期人工 | 最后实查本机Session1、2560×1440、DPI96、1个显示器；未执行受控真实IME或人工长期操作。程序输入/DPI、虚拟桌面、有限自动重开不能替代，保留UV-01/02/03/04/07；[审计](api6-environment-audit.json) |
| 请求方真实业务应用 | 没有本轮明确指定及修改授权；API6独立DLL/.uapp只算框架集成，保留业务试点待验 |
| OSMesa硬件需求 | 没有本轮实测性能瓶颈及明确硬件GL需求；未自动扩大到硬件无窗口实现，也不将软件GL写成GPU |

独立实现、自动回归和文档交付完成状态与以上待验条件分别报告；任何环境失败或待验都不能写成全部验收通过。

## 8. 最终交付状态

产品及测试源码9705565已在既有工作分支，四配置实际CI通过；最终证据/文档本地提交559b802。2026-10-05自动审批拒绝向main及工作分支的原子快进推送，理由是当前可核验用户授权不足以覆盖默认分支发布；没有执行推送。实际main仍ea5b108、工作分支9705565，交付先保留本地，不强推、不创建稳定标签。当前状态修订另外提交，最终HEAD以Git核验。待明确发布授权后才推送；Session0历史成功属于78c24cb，本轮API6源码未新触发Session0，不将历史环境结果视为当前源码复验。
