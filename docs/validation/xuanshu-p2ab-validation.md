# XuanShu P2 A批+B批最小只读闭环：本地验证

日期：2026-10-22。状态：已实现，等待代码评审；不是完整P2或稳定发布。只记录本轮本地源码、构建和测试，不继承历史通过结论。

## 1. 基线与授权

框架：`codex/p1-process-boundaries`，HEAD `e0eca03db25930be55bdb45477027d1793a61904`。接手SDK0.9-dev/API9/标准9，本轮工作区升为SDK0.10-dev/API10/标准10，ABI1/包1及旧公开结构布局保持。没有commit、push、tag、release或远程工作流。

参考工程`D:/XuanShu`的main仍为`67eacf5c47a856f5d3d68262f247659a15a13f60`，只读核对tracked工作区干净；没有刷新远程、安装或修改参考依赖。原`.gitattributes`用户修改、历史未跟踪文件、冻结SDK和业务应用均保留。

新构建及日志根目录：[本轮本地目录](../../build/xuanshu-p2ab-20261022-6e7d9c)。它不随clone提供。MSVC19.50.35727、VS18 BuildTools、Windows x64、C11、CMake/Ninja，Release；新增模块和测试保留`/W4 /WX`。固定Lexbor `7fb22cf5664a331d7c24b113489e566767c9c25a`、QuickJS-NG `2f0aa72a6b09cf69ff06399bcf9f4c083ac1a278`均核对一致，没有替换或升级。

## 2. 实现与文件范围

| 范围 | 文件与结果 |
| --- | --- |
| 公共合同 | [capability.h](../../include/ui_framework/capability.h)：version1独立结构、21个公共导出；没有AI Service/写入接口 |
| 解析与指纹 | [capability_json.c](../../src/capability_json.c)、[内部声明](../../src/capability_internal.h)：固定池严格解析、重复键、最小Schema、JCS、Windows CNG SHA-256 |
| 目录与调用 | [capability.c](../../src/capability.c)：注册、分类分页、资源枚举/绑定、prepare/dispatch、稳定ID完成、查询/forget、取消、两阶段注销 |
| 生命周期 | [ui.c](../../src/ui.c)、[ui_internal.h](../../src/ui_internal.h)、[workspace.c](../../src/workspace.c)、[app_data.h](../../include/ui_framework/app_data.h)：host所有权、实际实例身份、加载门、在途关闭门、WAIT后REFUSE恢复接纳 |
| 固定依赖 | [yyjson锁](../../third_party/yyjson/source-lock.json)、原始C/H/LICENSE及局部.gitattributes；[CMake离线哈希检查](../../cmake/yyjson.cmake) |
| 新增测试 | [JSON](../../tests/capability_json.c)、[核心](../../tests/capability_core.c)、[独立JCS参考](../../tests/capability_jcs_reference.js)、[真实DLL夹具](../../tests/capability_fixture.c)、[夹具清单](../../tests/capability_fixture.ini)、[workspace](../../tests/capability_workspace.c) |
| 当前版本 | 根CMake/ui.h、五维护示例包声明、browser app.ini、当前api7/session0/C++动态测试包清单、package_format未来版本边界、公共C/C++头测试、language_core版本断言、windows-ci maintained_api=10 |
| 文档与规划 | 三份设计、README/CHANGELOG、版本政策/开发标准/构建指南、[迁移](../migration-v0.9-to-v0.10.md)、当前交接与三份示例README；根task_plan/progress/findings只追加独立checkpoint |

yyjson0.13.0的tag对象为`138b4a7e757426bbbaa11ca37f257ce8d8489007`，完整源码commit为`6447536015f3d600f3d65323b10976103b337ca7`。三份上游文件不修改，MIT许可证保留，SHA256：

| 文件 | SHA256 |
| --- | --- |
| yyjson.c | `d2d58ef0a3b2267862dc363832c4f3185fd375aea933d5d3419d1a940d6ece2e` |
| yyjson.h | `ef803cda5c06b8962face6dfa39c3284b3bcf3e73f4d7317664690cc779679ca` |
| LICENSE | `45e384d3d52c73cba3a64d6e6c25d47cd738cd8a55c30629e3201046eda62947` |

## 3. 冻结的首批合同

应用使用context.host；workspace在create前设置实际instance ID，成功mount后才能prepare。独立host有框架分配身份。host_epoch是进程UUID，资源打开代次为UUID，revision为非空不透明UTF-8；绑定是调用方拥有的完整副本，不读取活动标签来重新定向。

新结构必须完整size/version=1；Windows x64静态断言固定target4576字节、resource desc48字节、capability desc112字节、result224字节。旧结构断言保持。描述字符串深复制，函数/user_data保留到注销完成；UI回调入参借用到返回。JSON输出用ui_capability_release，结果output借用至forget/host销毁。

prepare先复制并建立ID/结果记录，不执行；dispatch重新检查目标、revision、目录代次、modal/关闭和应用check。同步complete可早于dispatch返回；重复dispatch不重做，重复terminal complete为ALREADY_EXISTS，forget后为NOT_FOUND。完成只使用稳定ID；不让worker保存call/host指针。

host接口及回调限UI线程。异步夹具使用真实后台线程经app_data复制instance/call ID，在UI接收器完成。框架跟踪调用/UI回调租约，不自动追踪任意应用线程；应用须在worker/外部回调真实退出后才complete。夹具在poll前确认测试worker已返回。取消prepared可终态；取消running只提出请求，忽略取消时保持可观察并阻止提前销毁。显式quiesce不可逆；workspace另有可撤回关闭门，WAIT后REFUSE允许新调用，但不复活已取消调用。

只有effect=read与固定read scope。它是合作同进程程序接口，不是ACL、用户任务授权或AI审批。业务check拒绝为REJECTED/unavailable；revision不相等才为STALE/revision_conflict；非法输出为FAILED/invalid_output；完成时revision读取失败为FAILED/revision_unavailable。读取effect恒为none，不模拟事务或回滚。

Schema不支持组合、引用、pattern/format及未知约束。integer限安全整数；uint64身份传字符串。JCS按binary64/UTF-16键序处理，不做Unicode归一化。输入64KiB、输出256KiB、深度16、节点4096、数组512、对象128字段；每host256能力/256资源、合计目录复制2MiB；调用记录另限1024条/8MiB。完成记录预算不足保留RUNNING并返回LIMIT_EXCEEDED，生产者保留结果，释放旧记录后重交，不丢完成。

## 4. 本轮测试

| 配置/范围 | 结果与边界 |
| --- | --- |
| Native完整配置 | 27项中26通过、1物理跨屏跳过；包括新增四项，未把跳过算通过 |
| Native最后收口复验 | Schema规范化后的存储长度也限制64KiB（防止短指数展开）；新增/公共头/workspace共7/7通过，未重写前轮完整配置结果 |
| Light相关范围 | 14/14通过，包含新增四项、公共C/C++、应用合同、workspace、标题、文字、hover、主题和矩形稳定性；不是完整Light55项矩阵 |
| ASAN | JSON/调用核心/JCS独立对照3/3通过；`/fsanitize=address /Z7`，未运行共享DLL/实际GL的ASAN专项 |
| JCS | 独立Node24.20.0 ECMAScript参考268向量通过；Node只作测试oracle，不是产品依赖 |
| ABI/导出 | C/C++公共调用方编译运行；21个新增capability/JSON导出可从实际DLL取得，无AI Service导出 |

最后收口日志为`quota-final-native.log`、`quota-final-light.log`及`quota-final-asan.log`；完整Native轮为`review-native.log`，首次失败与中间限定复验另存。

新增JSON用例覆盖重复/转义键、无效UTF-8和代理项、尾随及NUL后垃圾、非标准JSON、浮点边界、负零、Unicode排序、不归一化、深度16/17、节点/对象/数组上限和Schema拒绝。核心用例覆盖分类分页、目录代次、scope/实例/打开代次、同步/异步、重复完成、业务拒绝与版本冲突区分、取消停滞、非法输出、在途注销、256能力、2MiB目录、1024调用和8MiB完成背压。

真实DLL用例覆盖两实例/切换标签后目标固定、后台复制完成、WAIT后REFUSE、ALLOW但在途引用未释放时保持实例、完成后真正DLL卸载、迟到投递拒绝及重开后旧目标失效。没有使用真实Provider或模型连接。

本地命令采用本机VS x64开发环境；目录均为本轮新建：

```powershell
cmake -S . -B build/xuanshu-p2ab-20261022-6e7d9c/native-checked -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TRY_COMPILE_CONFIGURATION=Release -DUI_BUILD_STANDALONE_HOST=OFF -DUI_FRAMEWORK_ENABLE_LIGHT_WEB=OFF -DUI_FRAMEWORK_ENABLE_WEBVIEW2=OFF -DUI_OSMESA_LIBRARY=
cmake --build build/xuanshu-p2ab-20261022-6e7d9c/native-checked
ctest --test-dir build/xuanshu-p2ab-20261022-6e7d9c/native-checked --output-on-failure
```

Light启用standalone/Light和BROWSER_BUILD_CHECKS，WebView2关闭、OSMesa路径为空，完整构建通过；ASAN仅构建两个新增核心测试目标。完整命令、JUnit及失败输出保留在本地目录，受环境影响的首次结果不覆盖。

## 5. 失败、未验项与保全

1. 两次初始CMake编译器探测报MSVC C1902（PDB管理器不匹配），未到产品编译；显式使用Release探测后配置成功。ASAN首次无调试信息的C5072被/WX拒绝，补/Z7后通过，没有关闭警告门槛。
2. 新代码一处未初始化变量警告及新测试strcpy警告已修正。新增测试首轮发现测试脚本CRLF分行、未过滤事件和错误的关闭投递预期，按实际既有合同修正后通过；首轮保留。
3. Native首次生命周期包仍动态声明API9，与新DLL不符；仅同步当前版本元数据后，原断言通过。CMake生成C++包声明也已同步，未来版本拒绝从10推进到11，没有放宽版本门槛。
4. Light的ui_browser_host一次20项断言失败，涉及SendInput、SetCursorPos和最近文件保存/恢复。未修改其交互实现、断言或环境，不能宣称这是已定位的环境根因或已解决的产品问题。
5. 一轮同时存在其他UI测试时，ui_workspace_offscreen原20秒门槛超时（原日志为20.05秒）；保留该次输出。后续独立串行14项通过，未改变超时，未证明超时根因。评审收口纠正原文误写的30秒，CMake与原始日志未修改。
6. 未验：WebView2 Runtime、显式OSMesa、stateful真实GL/模型适配、Session0/整机无登录、真实IME/物理DPI/多屏、长时稳定性、外部业务应用。历史Mesa真正卸载、Alt+F4、Runtime首次退出及同值粘贴问题仍按原交接保留。

原文件保护检查覆盖17300份未授权改动的原文件，缺失和意外变更为0；另四份当前交接/示例版本说明在编辑前单独核对原SHA。原.gitattributes逐字节保持，规划文件原前缀保持。只清理本轮无用依赖临时checkout与失败配置目录，保留最小失败日志、JUnit、当前构建及身份记录供代码评审；不清理用户历史材料。

## 6. 停止位置

三份整体设计仍为Proposed，D01—D05的确认与A+B实现子集已写回。没有实现普通/Assistant生成视图、业务写入、UI绑定、草稿冲突、五示例能力迁移、事务/撤销、AI Service、主对话、Provider、Coordinator、存储或恢复。本轮停在A批+B批最小只读闭环代码评审，不自动进入下一批。

## 7. 2026-10-22评审收口：独立新证据

本节只对应后续明确授权的三项整改，不覆盖第4节原实施结果或第5节首次失败。实际基线仍为`e0eca03db25930be55bdb45477027d1793a61904`、`codex/p1-process-boundaries`及已有A+B未提交工作区。新目录为[评审收口证据](../../build/xuanshu-p2ab-closeout-20261022-4f9c2a)，包含接手哈希、复现脚本、独立日志及各配置JUnit；不随clone提供。参考工程main仍为`67eacf5c47a856f5d3d68262f247659a15a13f60`、工作区干净，固定Lexbor/QuickJS身份未变。

### 7.1 三项处理结果

1. [执行门](../../src/capability.c)在最后一次get_revision回调返回、进入RUNNING前，重新检查关闭、dispatch_blocked、退休状态、目录代次及modal；不增加回调或禁止重入。revision冲突、REJECTED/unavailable、取消和重复派发语义保持。[modal回归](../../tests/capability_modal.c)使用真实Light对话框，分别由check、第一次及最后一次revision回调打开；验证execute为零、重复派发不执行，关闭modal后正常调用仍只执行一次。旧代码红例62检查/15断言失败；修复后59检查/0失败。检查数差异来自旧代码错误执行时额外进入execute内断言，不是删除断言。
2. 原workspace_offscreen门槛核对为20秒，首次日志20.05秒；第5节文字已纠正。CMake的20秒、原始失败日志和后续串行结果均未修改，首次根因仍未定位。
3. [真实DLL夹具](../../tests/capability_fixture.c)自行创建worker及独立的受控回调线程；worker先复制投递结果，再等待退出门，回调线程有独立退出门。应用UI接收器仅使用零超时线程句柄查询；两个线程真实退出前不complete。线程作业只持有workspace、稳定实例/call ID和应用同步对象，不向消息队列复制host/回调/目标指针。框架没有增加任意线程追踪。

[workspace用例](../../tests/capability_workspace.c)在两线程运行、结果已投递但worker未退、仅worker已退而回调未退三个阶段检查RUNNING、实例及DLL继续存活；两者退出后完成，UI接收器返回后才真正卸载。同步complete用例在外层execute栈上验证forget、能力/资源finish_unregistration均PENDING，嵌套close/poll不卸载，借用的目标、参数及结果仍可读；退出execute后才释放和卸载。

队列用例填满既有8MiB复制队列，DLL worker的结果投递必须实际返回LIMIT_EXCEEDED，调用保持RUNNING。排空后，应用用保留的稳定ID重投复制结果；重投与消费是两个poll阶段，随后安全完成。完成事件中还排入一条旧实例消息，先卸载再重开；新旧实例的call ID刻意相同，旧消息仍被丢弃，新调用保持PREPARED。关闭后的新投递返回CANCELLED。

### 7.2 本轮本地验证

MSVC19.50.35727，Windows x64、Release、C11/C++公共调用方；native及Light完整构建通过。Light使用`UI_BUILD_STANDALONE_HOST=OFF`、Light=ON，WebView2=OFF、OSMesa为空；ASAN使用`/fsanitize=address /Z7`并包含实际共享DLL和夹具。新增代码保留`/W4 /WX`，未降低既有断言、数据规模、预算或超时。

| 本轮运行 | 结果与证据 |
| --- | --- |
| 未修复modal红例 | 1项失败，62检查/15断言失败；`modal-red-light-test.log`、`light/modal-red.xml`保留 |
| Native完整配置 | 27项：25通过、1失败、1物理跨屏跳过；`final-native-native-test.log`、`native/final-native.xml` |
| Native失败后独立核对 | `ui_surface_input`单项1/1通过；`surface-followup-native-test.log`，不覆盖上项失败 |
| Light限定回归 | 15/15通过，新增5项及公共头、workspace、应用合同、主题、文字、hover与几何；`final-light-light-test.log`、`light/final-light.xml` |
| ASAN限定回归 | JSON/core/JCS/真实DLL workspace共4/4通过；`final-asan-asan-test.log`、`asan/final-asan.xml` |
| JSON/JCS及原调用合同 | JSON79、core3747检查通过；独立ECMAScript268向量通过。它们是本轮重新执行，原实现轮结果另列第4节 |

Native首次失败是`ui_surface_input`的“key down and thread modifiers dispatch”断言；该测试没有注册新能力，也未进入本轮修改的target_check。后续单项通过不能解释首次失败，不归因环境或宣称修复；原测试、平台输入实现及断言保持。新增真实DLL用例在native、Light及ASAN均通过，各运行轮询断言数随实际线程调度变化，不作为固定覆盖数量。

首次编译因新测试使用sscanf触发C4996并被/WX拒绝，改用sscanf_s后通过；`red-light-build.log`保留，没有屏蔽警告。依赖原有QuickJS编译警告未修改。UI测试串行运行，没有同时启动不同配置的UI回归。

### 7.3 验收与保护边界

这三项整改已完成对应本地验证，停止在A+B整改代码评审位置，不宣布完整P2或完整宿主回归通过。浏览器原20项失败、本轮之前workspace首次20.05秒超时，以及本轮Native输入首次失败均独立未定位；没有用限定通过覆盖它们。WebView2、显式OSMesa、真实GL模型接入、任意第三方回调实现、长时/物理输入条件仍未验。

保持SDK0.10-dev/API10/标准10、ABI1/包1及全部公共结构。本轮产品代码只修改最终执行门；其他源码改动为测试及测试注册。没有新增AI Service、业务写入、UI绑定、示例迁移、事务/撤销、Provider、Coordinator、存储或恢复；无子代理、computer use、远程工作流或Git写操作。

保全核对以新目录`baseline.json`为接手身份，整改文件及新测试另列`delivery-files.json`；最终差异与原规划前缀检查记录在`preservation.json`。原`.gitattributes`、依赖、公共头、冻结SDK及历史材料不修改。清理仅删除本轮新目录的444份中间obj（61,362,042字节），保留匹配二进制、包、配置/复现脚本和失败/最终日志、JUnit供评审；记录见`cleanup.json`。没有删除首次modal红例、首次编译失败或Native输入失败证据。
