# API7 稳定性、滚动、工作区及颜色验收

日期2026-10-05。接手时工作区干净，分支codex/menus-offscreen、HEAD 658b4972f78c4c52fad86ff8c379567dd108d277。先核对handoff、开发标准、通用Web、工作区、菜单及API5/6、菜单、Session0记录；既有API5/6和菜单功能继续回归。本轮只本地提交，未推送、未创建标签，没有启用graph-engineering、子代理或新会话。

## 1. 实施与源码身份

先复现关闭资源问题，再冻结SDK6、建立各项失败用例；复用原生命周期实现完整范围滚动、底部停靠、同区标签和连续RGBA；最后完整回归、旧包兼容和环境检查。新增公开接口升级SDK0.7/API7/标准7，ABI1、包格式1、导出入口不变；内部关闭修复本身没有单独升级ABI。

| 单元 | 本地提交 |
| --- | --- |
| Runtime关闭门控、资源诊断和6次64轮复验 | 3055bab6a115740a05e37a0decf102deae445c39 |
| 冻结SDK6原公共头文件 | 87f829aa6d6e4a404a2d374831872907d54f24cd |
| API7实现、测试和CI回归入口 | 21e94e5a5f99964fde0de2ab4a32fa5c65858b82 |
| 补充捕获/换源/resize/关闭测试 | d79a59f9f38b7d31997a44175f965e9010ad5302（仅两测试文件） |
| 补齐直接注册底部默认面板与初始无窗口布局 | 1262cc084e5ef9a20eb20a4557dbb0c05099b0b3 |
| 异步模态就绪焦点与资源诊断 | 2c0c52baa345a3234bc10ec39e7458afbefd6a8c |
| 原生输入就绪及失败所有权诊断（仅测试） | ae4ee3a、cb426600d25175261eeaee3ccd23ad859b138b16 |
| 文档与最终证据 | 包含本文的后续提交，Git log核对 |

本轮没有覆盖接手用户改动，没有修改请求方应用、PERF-001、SDK1–5、原包。新增SDK6是658b497全部12个公共头文件的冻结副本，兼容性检查按Git换行转换归一比较；它也不在后续编辑范围内。

## 2. 实现与成功标准

| 项目 | 实现及验收入口 |
| --- | --- |
| 稳定性 | 延后启动Runtime环境/controller，WAIT期间检查dispatch_blocked；已开始异步操作只持有失效内部view，特定内部文档确认后退出回调栈Close，环境等待BrowserProcessExited。保留句柄+12、GDI/USER+4、pending cleanup归零、逐轮DLL卸载断言；[资源记录](runtime-stability-validation.md) |
| 轻量overflow | 框架GDI绘制12逻辑像素横纵轨道、最小24滑块；无溢出隐藏、点击一视口、两轴交汇留白、悬停/焦点绘制；无额外DOM或系统滚动控件。有窗口及NULL-HWND同一路径，捕获/释放/Esc/CANCEL/捕获丢失/resize/关闭；[测试](../../tests/light_scroll.c) |
| TREE/TABLE/LIST | 纵条使用完整total_count和树展开计数，宽表横条使用全部列宽、按列边界停靠，浏览器overflow不冒充虚拟范围。BigInt精确映射uint64、首末行/列可达；窗口查询、分页、滚轮、Shift滚轮、表格键盘、稳定ID及原预算保留；[测试](../../tests/component_scroll.c) |
| 数据变化 | 空批报告缩小后的总量，夹紧并重查；非空批仍严格验证first/count。树移出缓存的已知分支可折叠，失去祖先的元数据不计总量。换源/排序/代次/模态/实例失活释放捕获，保留最新有效状态；显式Esc/CANCEL在当前有效拖动恢复起点，迟到代次拒绝 |
| 底部与标签 | 平面左右/底部栈，底部高度/分隔条、标题标签命中预览/提交/取消、窄窗、关闭活动页后选可用成员、activate重开、reset。组ID为shell内稳定uint64，0无组、UINT64_MAX保留；活动页共享框，所有槽/组件/GL context继续存活；[测试](../../tests/workspace7.c) |
| 持久化 | 格式2（96头/112记录）保留格式1前缀并读取1（80/104），未来版本拒绝、整份异常验证后应用；应用存储/恢复时机、缺失/新增面板、DPI/work area及重置合同见[布局](../workspace-layout.md)。未扩展嵌套分割树 |
| 连续颜色 | 共用RGBA四通道0..255、暗/白底透明度合成预览、六/八位文本，拖动/键盘只改草稿；保留颜色关闭选择器，业务只在表单提交一次。取消/选择器Esc恢复打开前值，捕获取消恢复拖动前值；只读/禁用拒绝、非法提交校验；[两后端测试](../../tests/component_experience.c) |

公共追加：BOTTOM=9，COUNT=8保持旧哨兵且8保留；CANCEL=8追加，ui_input_event和ui_layout_desc尺寸/偏移不变；ui_panel_layout原48字节之后追加group（偏移48）/active（56），当前64字节。size≥56访问group，≥60访问active，55/59部分字段不访问。旧get/set前缀兼容，旧同区set保留组、移区/浮动脱组。旧API1–6未启用新布局能力仍保存1，API7保存2，旧运行库不能读取2；应用保留迁移备份。详见[迁移](../migration-v0.6-to-v0.7.md)。

DOM1024、缓存2MiB、批512、列64、字段64、图片32MiB、投递8MiB、JS8MiB/栈256KiB/运行时限均未提高，没有全数据预载。面板可在panel_desc.dock_region直接声明BOTTOM，NONE保留原RIGHT默认，reset返回注册默认底部。Windows无窗口用实际轻量模板和逻辑拖拽/布局，分隔条可直接提交逻辑尺寸；没有原生捕获/物理显示器。ui_framework_headless是核心适配器，UNSUPPORTED不作为功能证据。

## 3. 失败与修复证据

| 失败 | 定位/修复与复验 |
| --- | --- |
| 旧Runtime64轮384→491，首尾PSS对照384→453 | 关闭门控及逐步失败均保存，[资源记录](runtime-stability-validation.md)中的6次独立64轮通过；随后当前完整回归与API7独立64轮复验 |
| [首轮完整矩阵](api7-full-first-failure.log)3失败 | 未来API测试仍把7当非法，改为8；颜色选择器挤出基础色末项，改为连续轨道加紧凑6色及bounded overflow；API5资源425→449保留原失败，见下述关闭基线说明 |
| [第二轮完整矩阵](api7-full-second-failure.log)3失败 | 原生键盘owned断言因前台政策失败；新增总量缩小断言也真实失败。后者用空批总量通知/重查及树展开元数据边界修复；此轮进行时新增用例被构建，不能把它当作同一冻结源码的最终矩阵 |
| [总量缩小](api7-total-red.log) | UINT64 endpoint后总量1：原空批被拒、first越界；现在允许空批报告新总量、夹紧重查，树100k+100k展开、移出缓存折叠和LIST端点通过 |
| [真实Runtime视口反馈](api7-viewport-failure.log) | body自动高度/行伸缩反馈使rows高度增长，明确body视口和行最小高度；同时修正Light测试对固定高度/单子节点自动填充的假设。未提高预算 |
| [轻量CSS失败](api7-unsupported-style-failure.log) | 新模板曾使用未支持的flex-shrink，移除该属性，保持既有受控CSS子集 |
| [原生面板头失败](api7-native-header-frame-failure.log) | 内容槽frame遗漏标签头，修正实际frame/内容分配；旧布局与新组可见性通过 |
| [集成焦点失败](api7-integration-focus-failure.log) | 新滚动动作后原测试仍假定编辑字段拥有焦点；先实际点击编辑字段再验证布局保留焦点，原焦点断言保留 |
| [最终边界复现](api7-boundary-red.log) | 六位颜色拖alpha丢RGB、宿主下方仍命中底部；颜色解析接受六位并默认alpha255，底部命中限制host范围。Light/真实Runtime各颜色失败、三种shell拖拽失败随后专项7项通过 |
| [底部直接注册失败](api7-bottom-registration-red.log) | panel_desc.dock_region仍只接受左右，三种shell注册都失败；允许BOTTOM并在无窗口初始reflow准备底部空间，初始/移动/reset[专项通过](api7-bottom-registration-fixed.log)，旧默认RIGHT保持 |
| 测试设置错误 | [多传set_field参数](api7-boundary-test-compile-failure.log)后改为真实签名；[首次边界fixture](api7-boundary-fixture-first.log)漏接受已提交草稿，按原合同accept_fields后再回填六位颜色。没有削弱对应产品断言 |

最初bottom12失败和scroll17失败曾在工具输出出现，但对应本地文件后被通过运行覆盖，未伪造为保留的原始日志。上述实际保存的失败与最终输出提供可复验依据。其他中间日志在本地build/api7-evidence/保留。

API5首轮资源失败的旧基线没有记录pending，不能断言这一次波动的精确来源已完全证明。新测试在基线及结束均等待框架pending cleanup=0（至多15秒），末次必须原阈值内，独立诊断过程PSS资源类型也记录。原2秒每轮等待、32轮和+12/+4断言不变。诊断444→443、第二轮built/original均410→409以及后续完整矩阵重复通过；这证明所列关闭合同下资源回收，不能用后续单次通过擦掉425→449。

原生键盘失败保留[桌面诊断](api7-native-desktop-failure.log)及[首次激活尝试](api7-native-activation-attempt-failure.log)。测试只在SetForegroundWindow失败时把自身窗口临时放到最前、验证WindowFromPoint确属自身标题后点击，恢复层级和指针；绝不注入其他应用。owned、真实SendInput、重复键只执行一次及焦点断言不改。[隔离通过](api7-native-focus.log)和最终完整矩阵均通过。这是测试前台前置条件，未修改产品快捷键逻辑。

## 4. 各次回归与源码身份

下面第一组结果属于21e94e5主体实现及d79a59f测试补充；后续1262cc0/2c0c52b的实际结果另外列出，不把早期二进制身份覆盖为新结果。

| 回归 | 实测结果 |
| --- | --- |
| 全部必要项：原生/Light/真实Runtime/OSMesa、WGL及原包 | 61项：60 PASS、ui_monitor_transition物理条件SKIP、0失败，342.10秒 |
| 关闭Web的原生独立配置 | 22项：21 PASS、同一物理项SKIP、0失败，5.33秒 |
| 最后新增边界及滚动/布局/颜色专项 | 7/7 PASS，20.20秒，包含实际Runtime |
| 当前API7快速关闭独立64周期 | [日志](api7-final-fast64.log)：句柄397→399、GDI12→12、USER17→17、pending0、逐轮DLL卸载，0失败 |
| 当前API7实际呈现后关闭独立64周期 | [日志](api7-final-ready64.log)：句柄397→398、GDI12→12、USER17→17、pending0、逐轮DLL卸载，0失败 |
| 当前API5 built/original各32周期 | 最终两个独立进程均399→398、GDI9→9、USER17→17、pending0，原+12/+4断言通过 |
| 最后明确覆盖拖动中换源/总量变化/resize/捕获关闭 | [补充测试7/7 PASS](api7-transitions-final.log)，20.97秒，测试提交d79a59f，产品语句不变 |
| 格式整理后重建API7 DLL再次组合验证 | [两后端2/2 PASS](api7-post-trim-integration.log)，22.44秒；[完整输出](api7-post-trim-integration-output.log) |
| 当前API7完整集成Light/Runtime各8重开 | Light238→238、Runtime401→401，GDI/USER不增长、pending0 |

64轮实际计数以日志reopen 0..63为准；资源汇总行沿用测试中的reopen8文字标签，不把标签当作本次周期数量。呈现关闭运行有共享Runtime退出前的临时pending/句柄峰值（末周期pending64、句柄550），随后实际归零且最终398；原终值阈值不改，没有把仍存活进程数量等同于宿主句柄数。

[最终完整日志](api7-full-final.log)、[JUnit含应用摘录](api7-full-final.xml)、[完整应用测试输出](api7-full-final-output.log)、[原生日志](api7-native-final.log)、[原生JUnit](api7-native-final.xml)、[专项边界日志](api7-focused-final.log)。本机自动用例和工作流配置不是当前commit已在GitHub托管CI运行，本轮未推送。该早期运行只有物理跨屏SKIP；后续失败另列，不能把本句当作最新矩阵无失败。

独立[API7 DLL](../../tests/api7_fixture.c)/.uapp在Light无窗口与真实Runtime组合树/64列100k表、属性/连续RGBA、实际OSMesa4xMSAA GL frame/C图片（[应用实际GL读回](api7-application-frame.png)）、异步缩略图、语义命令、bottom/tab/same slot/context、草稿/选择/焦点、双实例、模态、初始化create/mount失败、迟到结果、WAIT关闭及每轮DLL卸载。Light钩子断言全生命周期新增HWND为0；Runtime必须实际查询到字段，不能由Light通过代替。测试DLL结果仅是框架集成证据。

API5菜单Alt、OSMesa、精确MSAA、Runtime查询/捕获/图片/桥接及旧非MSAA/WGL、菜单侧向鼠标、编辑、模态、预算、原生/GL均在完整矩阵中实际回归。源码构建使用C11 /W4 /WX /utf-8，公共C/C++头文件调用方及旧完整size断言保留。

### 最新产品2c0c52b与原生测试cb42660

1262cc0完整矩阵再次出现API5原包358→395、USER15→17、pending0的超限，保留[原失败](api7-latest-full-failure-output.log)。独立三次诊断中第三次354→378失败，并观察到SystemUserAdapterWindowClass/CoreMessaging加载；实际Windows输入创建栈与稳定复现的异步模态焦点缺口见[资源记录](runtime-stability-validation.md)。修复没有增加固定预热、放宽阈值或修改原包。

产品2c0c52b修复文档就绪后的有效原生焦点转交。API5原包三个独立进程各32周期均390→387、GDI9→9、USER17→17、pending0、逐轮DLL卸载：[运行1](api7-modal-focus-fixed-1.log)、[运行2](api7-modal-focus-fixed-2.log)、[运行3](api7-modal-focus-fixed-3.log)及[当时二进制](api7-modal-focus-binaries.json)。同一产品完整矩阵中的built/original两次32周期均394→391。

| 最新检查 | 原始结果与身份 |
| --- | --- |
| 61项完整矩阵初跑 | **59 PASS / 1 FAIL / 1 SKIP**，348.69秒；失败ui_api5_native_host日志含用例未发送的VK_PACKET Unicode，原生前台/菜单断言保留；[摘要](api7-current-full.log)、[完整输出](api7-current-full-output.log)、[JUnit](api7-current-full.xml) |
| 原生独立配置 | 22项21 PASS / 1物理SKIP，5.35秒；[日志](api7-current-native.log)、[输出](api7-current-native-output.log)、[JUnit](api7-current-native.xml) |
| 最新API7快速64周期 | [独立进程](api7-current-fast64.log)：390→390、GDI12→12、USER17→17、pending0，0失败 |
| 最新API7实际呈现后关闭64周期 | [独立进程](api7-current-ready64.log)：391→392、GDI12→12、USER19→17、pending0，0失败；退出前临时pending64/handles552，最终必须原阈值内 |
| 最新原生真实输入 | 只改测试为等待实际Runtime呈现就绪，记录前台命中及被拒绝窗口PID/owner；owned/次数/焦点断言不改。cb42660三个独立CTest进程：[1](api7-native-owner-1.log)、[2](api7-native-owner-2.log)、[3](api7-native-owner-3.log)，各1/1 PASS，三个原始日志均通过，时间分别6.14、6.06、6.07秒 |

### 最终冻结cb42660完整矩阵

在同一已提交源码cb426600d25175261eeaee3ccd23ad859b138b16上，释放交互工具后重新执行整个61项矩阵：**60 PASS / 1物理SKIP / 0失败，327.82秒**。ui_api5_native_host本次6.10秒通过，完整结果见[摘要](api7-frozen-full.log)、[逐项输出](api7-frozen-full-output.log)、[JUnit](api7-frozen-full.xml)。此前59/1/1失败未覆盖；后续通过只证明对应条件下的回归，不能宣称共享桌面争抢前台永久消失。

同一原生输入测试还以until-fail:10连续启动10个独立进程，全部通过、60.74秒：[摘要](api7-native-owner-stress.log)、[每轮输出](api7-native-owner-stress-output.log)。默认原32周期资源阈值保留，冻结矩阵API5 built/original各360→357、GDI9→9、USER17→17、pending0。API7矩阵Light188→188/Runtime361→361、每种8周期；之前同一2c产品两个独立64周期的身份继续保留，不把测试诊断提交当成额外128周期重新执行。

[最终框架/测试/DLL/原包/依赖/像素及日志清单](api7-frozen-binaries.json)记录cb源码与2c产品代码关系。冻结矩阵Session1对照再次200轮/400实例/2000命令、HWND0通过：[CSV](api7-frozen-session1-resources.csv)、[原始输出](api7-frozen-session1-run.log)、[实际GL帧](api7-frozen-session1-frame.png)及[组合应用GL帧](api7-frozen-application-frame.png)。句柄185→186、GDI0→0、USER2→2；private13,242,368→109,105,152，峰值109,375,488，原128MiB绝对门槛保留。它仍仅是INTERACTIVE_CONTROL，不算当前Session0或无登录通过。

中间共享桌面前台获取失败仍保存：[首次独立失败](api7-native-host-quiet-1.log)、[等待就绪后失败](api7-native-host-ready-1.log)、[标准前台重试仍失败](api7-native-host-activation-1.log)、[隐藏启动器第二次失败](api7-native-controlled-2-output.log)。最后三个通过不是这些竞态已被永久修复的证明。初跑矩阵没有被改写成60通过/0失败；必要用例的后续独立通过与单次整批全绿分别报告。当前真实IME、物理/长期人工仍待验。

最新[二进制、原包、像素及日志身份](api7-current-binaries.json)保留product2c0c52b、native test cb42660区别；ae4ee3a/cb42660只修改原生输入测试，没有改变滚动/布局/颜色/GL/资源产品语句，未用新hash覆盖21e或首次复验。实际[应用GL帧](api7-current-application-frame.png)来自测试DLL读回，独立框架集成不冒充业务应用。工作流语法[检查通过](api7-ci-parse.log)，未推送、未运行当前源码托管CI。


## 5. 兼容、依赖和复现

冻结SDK1–6调用方与原二进制分别运行。API1 EDA、API2 Counter、API3 generic_components、API4 framework_features、API5 fixture、API6 fixture原包都可用，hash再次核对不变：[检查输出](api7-compat-final.log)。原API6额外在Light/Runtime各运行同一原包，未用重编译副本替代。完整矩阵与64周期当时的精确路径/hash、框架/测试/DLL/新包/依赖身份见[清单](api7-binaries.json)。随后只整理自有测试末尾空白、追加两测试中的捕获变化断言，构建发生重编译；[构建日志](api7-transitions-build.log)和[重建二进制清单](api7-transition-binaries.json)分别保留。补充7项和两后端实际DLL集成通过，不用新hash覆盖旧运行身份，也不把它写成64轮在新hash上重新运行。

| 依赖/环境 | 实际条件 |
| --- | --- |
| 系统 | Windows x64 NT10.0.26300，交互Session1，Default桌面，一台1920×1080显示器、DPI96；程序DPI另外覆盖 |
| 编译 | MSVC19.50 / VC14.50.35717、Windows SDK10.0.26100.0、Release/C11 |
| Lexbor | v2.5.0 commit7fb22cf5664a331d7c24b113489e566767c9c25a |
| QuickJS-NG | commit2f0aa72a6b09cf69ff06399bcf9f4c083ac1a278 |
| WebView2 | SDK1.0.4129.50，Runtime 154.0.4258.53，真实子进程、STA消息循环；受限进程沙箱不等同Runtime运行条件 |
| OSMesa | x64 Mesa24.3.4，llvmpipe LLVM19.1.7/4.5 compatibility git1950a8b78c；显式实际提供方，不用生成图片替代GL输出 |

在x64开发终端按[构建说明](../build-and-validation.md)准备固定依赖（[提供方分发文件及源码依赖hash](api7-dependencies.json)）；启用LIGHT_WEB/WEBVIEW2及UI_OSMESA_LIBRARY，提供UI_LEGACY_EDA_PACKAGE、API2、COMPONENT、API4、API5、API6_PACKAGE六个原包路径，再cmake --build、ctest --output-on-failure。不提供原包不产生对应测试，必须明确未验。新目录可避免遗留运行库混用。

专项过滤：ui_workspace7|ui_workspace_layout|ui_light_scroll|ui_component_scroll|ui_component_experience|ui_api7_integration。独立重开：设置UI_RUNTIME_CYCLES=64，运行ui_api7_integration_test.exe api7_fixture.uapp <绝对OSMesa路径> webview2；第二个新进程加UI_RUNTIME_READY_REOPENS=1。原资源阈值不变；失败日志保留，关闭需正常STA消息循环，不清理用户Runtime目录或强杀进程。

## 6. 最后环境检查与未验项

cb42660最后实际只读检查：[环境JSON](api7-environment.json)。本机Session1、machine_user非空、单显示器，DWM进程存在；DWM存在本身不是屏幕边缘合成验收。quser未提供，未由它推断WTS计数。运行账号administrator=false。

尝试当前源码的原服务脚本，在第11行管理员/服务管理权限检查处退出1：[工具输出转录](api7-session0-attempt.txt)。早期重定向文件为空，只保存标明的工具转录。最后cb42660在子PowerShell重新调用同一脚本，取得[真实错误输出](api7-current-session0-attempt.log)，仍在原第11行退出1，未伪造服务run.log。检查发生在GUID服务创建之前，没有创建服务、注销用户或修改已有服务。当前源码Session0仍未验；历史78c24cb的通过只保留在[历史记录](session0-osmesa-validation.md)。

早期21e94e5 Session1实际测试应用DLL 200轮、400实例的GL frame/input/resize/0及4samples、窗口计数和资源回收通过，created_HWNDs=0、process_HWNDs=0；[200轮资源CSV](api7-session1-resources.csv)、[实际GL证据](api7-session1-frame.png)及[完整Session1运行输出](api7-session1-run.log)。它明确标为INTERACTIVE_CONTROL，不冒充Session0或无登录。句柄235→236、GDI0→0、USER2→2；OSMesa/LLVM进程缓存保留，private12,435,456→108,507,136，峰值108,933,120，仍在原128MiB绝对门槛内。每轮应用DLL及GL对象释放，不宣称提供方进程缓存全部退还。

最新产品2c0c52b的Session1对照再次运行200轮/400实例/2000命令、0及4采样实际frame/input/resize，created_HWNDs=0、process_HWNDs=0、逐轮DLL/GL对象释放；[当前资源CSV](api7-current-session1-resources.csv)、[实际帧](api7-current-session1-frame.png)、[完整输出](api7-current-session1-run.log)。句柄219→220、GDI0→0、USER2→2；private12,541,952→99,434,496，峰值105,897,984，原128MiB绝对门槛保留。仅INTERACTIVE_CONTROL，当前Session0仍未验。

严格整机无登录服务runner用户已明确没有；WTS登录会话为0的门槛保留，未注销用户或修改既有服务。真实中文IME候选/组合/提交、不同缩放物理显示器、屏幕边缘桌面合成和长期人工缺少完整条件，程序DPI、自动输入/捕获和短期压力不能替代。

没有新指定且授权修改的真实业务应用；未开展业务试点、未修改请求方包。没有实测OSMesa性能瓶颈和明确硬件GL需求，未自动扩大硬件无窗口方案。独立开发、回归和文档可交付，但上述待验意味着不能宣布全部验收通过。

## 7. 归档字节完整性

39bcedf归档后发现Git换行转换使10份current/frozen证据blob与实测SHA256不同。41fb643只为这两种新前缀在[.gitattributes](../../.gitattributes)设置-text并恢复工作区原始字节；未改变源码、断言、运行结果或原包。两个清单中的日志SHA256与Git blob均[零差异](api7-archive-byte-check.log)。其他历史记录保留，二进制身份不以换行归一比较。
