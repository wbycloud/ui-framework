# XuanShu P2 C1独立验收记录

日期：2026-10-22。状态：C1已实现并完成下列本地限定验证，待代码评审。整体设计仍为Proposed，完整P2未完成，C2未获实施授权。

## 1. 实际基线与保留范围

框架分支`codex/p1-process-boundaries`，HEAD `e0eca03db25930be55bdb45477027d1793a61904`；接手时已有A+B及评审收口的源码、版本、文档和未跟踪文件，暂存为空。`.gitattributes`为用户既有修改。此次没有reset、提交、推送或刷新远程。参考工程`D:\XuanShu`的`main`、HEAD `67eacf5c47a856f5d3d68262f247659a15a13f60`，本次只读检查根工作区干净；九模块身份仍引用原审计，不冒称重新逐模块审计。

新证据目录为[本轮本地证据](../../build/xuanshu-p2c1-20261022-91d7ac)，不随clone提供。接手`baseline.json`覆盖17356个既有非忽略文件及根规划记录；`git-entry.txt`保存原工作区状态。原A+B及其整改证据目录不写入、不清理，首次失败不覆盖。

保持SDK0.10-dev/API10/标准10、应用ABI1/包1、旧公开布局和原v1只读语义；未修改`capability.h`、JSON实现、yyjson锁定源码/许可证、固定Lexbor/QuickJS、冻结SDK、参考工程、维护示例业务源码或browser元数据。

## 2. C1改动与接口合同

| 文件范围 | 本批改动 |
| --- | --- |
| [capability_entry.h](../../include/ui_framework/capability_entry.h) | 独立version1、size24 binding；enable/publish/request_call三项入口，未导出v2人工执行或AI Service |
| [capability.c](../../src/capability.c) | 权威ID冲突、生成投影、目标复制、旧request关联及终态桥接、result/visitor租约、目录/调用预算及宿主共享请求额度 |
| [ui.c](../../src/ui.c)、[ui_internal.h](../../src/ui_internal.h) | 生成命令pending标记、源长度与UI线程/ID耗尽门、阻止公共reply跳过校验、host销毁挂接清理；旧公开结构不变 |
| [assistant.c](../../src/assistant.c)、[assistant_internal.h](../../src/assistant_internal.h) | 动态只读视图、legacy修改拒绝、稳定call取消/释放桥接、宿主挂接及终态安全回收 |
| [workspace.c](../../src/workspace.c) | 原dispatch/callback门之后的安全poll入口回收workspace拥有的Assistant终态记录 |
| [入口夹具](../../tests/capability_entry.c)、[workspace夹具](../../tests/capability_workspace.c) | C1专用合同及实际DLL入口路径；原workspace配置的路径和断言继续保留 |
| [CMake](../../CMakeLists.txt)、公共头C/C++测试 | 两个独立C1测试目标，原测试不减少；新增binding布局检查；没有放宽超时/警告/断言 |
| 三份设计、当前交接及根规划 | 区分已确认、已实现和后续建议；原主线状态仅追加独立checkpoint |

单一能力描述管理ID、版本、Schema、业务callback、资源及两阶段注销。发布只复制能力ID与完整target，不接受第二份handler、权限或Schema。普通命令是引用投影，Assistant直接遍历权威目录，晚创建的Assistant无需同步第二目录。UI标题副本仅用于显示翻译，最多4095字节预留进目录预算，能力名称和指纹保持。

初次enable在旧请求/回调在途时PENDING；启用后不可逆，legacy Assistant READ隐藏、EDIT/DESTRUCTIVE直接invoke拒绝，旧validator/confirm/handler不能成为回退。`max_permission=DESTRUCTIVE`和`source_id="human"`也不能恢复编辑。普通legacy人工命令仍可用；未迁移host保持旧协议。本批没有提供可信人工来源证明，不能把普通host-invoke暴露成AI修改工具。

迁移要在应用源码移除同ID旧普通/Assistant注册，再注册能力并发布；所有方向重复ID明确ALREADY_EXISTS，退休期间仍占ID。目标包含instance/process/open epoch、资源、revision和read scope；活动标签不参与重新绑定。资源重开或revision变化后不能静默更新发布快照；需两阶段注销并重新注册/绑定。

关联顺序为旧pending/request→prepare call→写入request/call关联→dispatch→execute。同步result observer中即可查关联；异步仍用实例/call稳定ID和复制app_data返回UI。成功输出为验证后的原JSON，终态失败为code；prepare失败为status且无call ID。旧invoke的OK不等于业务成功。公共reply对生成请求拒绝，完成只能走原输出/revision校验。嵌套调用遇到dispatch PENDING时取消该PREPARED记录，返回可观察的cancelled_before_dispatch，不泵UI、不重复执行。

result/visitor及外层execute租约阻止回调内forget、finish注销或关闭提前释放。独立host/Assistant调用方显式释放；workspace Assistant在下一安全poll自动回收终态记录，观察者需复制跨poll数据。销毁Assistant不代替应用排空worker，也不取消仍运行的call；桥接不保存Assistant对象指针。host释放时断开内部挂接；剩余Assistant仅可销毁，不可继续操作已销毁host。

每host共享1024条生成入口Assistant请求额度，失败但未释放的请求也占用，不能靠多个Assistant绕过。节点计入8MiB调用预算；关联pending及有界source也保守预留。生成入口source最多255字节。目录仍2MiB、最多256能力；发布目标/投影同样计入，不能保证256个最大描述都同时放入。输出预算满时complete明确LIMIT_EXCEEDED，保持RUNNING、无伪成功observer；释放终态结果后才可重投。

## 3. 实际合同验证

| 场景 | 本轮证据 |
| --- | --- |
| 单一来源、双向ID冲突、退休占ID、发布重复、目录代次 | `ui_capability_entry`；失败发布不改代次，成功发布使旧分页代次失效；既有core继续验证分类分页 |
| 生成视图一致、晚创建Assistant、动态注销、标题与metadata分离 | 同一规范化Schema，UI标题更新不改能力名称；零二次业务回调 |
| 明确目标、目标数据复制、revision冲突、资源重开 | 清空调用方binding原目标后仍正确；旧open epoch不重定向；实际workspace第二标签激活后仍调用第一实例 |
| 非法参数、disabled/busy/modal/check/关闭 | execute不进入；component source不能绕过最终modal门；回调内更新disabled同样拒绝；原真实Light modal回归59/0 |
| 同步/异步、关联先于execute、重复派发/完成、非法输出 | execute与同步result均可查关联；重复不重复执行；invalid_output失败；公共reply绕过被拒绝 |
| 取消、未终态forget、嵌套调用、结果预算与请求预算 | 取消保持协作语义；回调内forget PENDING；实际填满1024条、8MiB结果预算、2MiB目录及跨Assistant总额度 |
| 在途注销、Assistant销毁、回调寿命 | finish注销PENDING；终态回调中host_destroy不释放；销毁Assistant后仍可稳定ID完成；host先销毁后Assistant析构安全 |
| DLL自有worker/外部回调、同步栈、复制队列、迟到与重开 | 独立`ui_capability_entry_workspace`经生成入口进入原真实DLL夹具：worker未退关闭受阻，外部回调退出后才complete；队列LIMIT后重投；旧实例消息不命中同call ID的新实例 |
| Assistant修改旁路与人工入口 | 高权限旧配置及human来源仍拒绝旧修改命令；人工普通命令在能力发布、退休前后继续完成；原legacy Assistant合同测试保持通过 |

不改变同进程合作边界：框架跟踪调用及框架callback租约，应用负责实际worker/外部回调退出；不阻塞UI等待worker、不强停线程、不自动追踪任意应用线程。

## 4. 本轮新结果

MSVC19.50.35727、Windows x64、Release、C11/C++公共头。三个配置均为新的独立构建目录，完整构建通过；`/W4 /WX`等门槛保持。Light为standalone OFF、Light ON、WebView2 OFF、OSMesa空；ASAN为native配置、`/fsanitize=address /Z7`、共享DLL及夹具同样插桩。测试在不同配置间串行运行。

| 最终源码运行 | 结果 | 原始材料 |
| --- | --- | --- |
| Native限定 | 16/16通过，22.35秒 | `final-review-native-test.log`、`native/final-review.xml` |
| Light限定 | 16/16通过，7.23秒；含workspace_offscreen和真实modal | `final-review-light-test.log`、`light/final-review.xml` |
| ASAN限定 | 10/10通过，3.17秒；含两个实际DLL配置 | `final-review-asan-test.log`、`asan/final-review.xml` |
| C1专用单测 | 各配置20923检查/0失败 | 上述对应日志 |
| 原JSON/core/JCS重新执行 | 79/3747检查通过，独立ECMAScript268向量通过 | 上述对应日志；不是历史结果搬用 |

workspace轮询检查数随真实线程调度变化，不作为固定覆盖数量。这里不是Native/Light完整CTest矩阵，也不替代历史验收。所有最终日志对应final-review构建后的源码；源码及匹配二进制身份另见本地交付清单。

首次C1入口测试271检查中2条失败：断言把原Schema文本与JCS排序后的精确文本直接比较，改成规范化的精确预期后通过；未删除断言，`first-native-test.log`、`native/first.xml`保留。实际DLL用例首次即通过。另一次ASAN构建命令把PowerShell `-File`的逗号参数当作一个Ninja目标而失败；PowerShell在stderr处中止，`first-asan-build.log`为空，工具返回错误摘录另存`tooling-failures.txt`，不冒充原始编译日志。随后完整ASAN构建成功，没有修改构建目标或放宽配置。一次裸ctest列表命令因未加载VS PATH未运行，也记入该工具诊断；不是产品测试失败。

## 5. 历史失败与未验项

- 原浏览器20项断言失败：本轮不重跑、不修复。browser没有启用C1模式或发布生成入口，源码未迁移；其共享host/Assistant路径虽有C1增量，限定兼容测试通过不能证明原失败无关或已解决。
- 原workspace_offscreen首次20.05秒超时，实际门槛20秒：历史日志及CMake不变。本轮Light该项2.27秒通过，仍不能定位首次超时，不归因环境或宣称解决。
- 原Native输入首次“key down and thread modifiers dispatch”失败：本轮未执行该输入项；此前单项通过不覆盖首次，根因继续未定位。

未验：完整browser/Native/Light回归、WebView2 Runtime、显式OSMesa、真实GL业务模型、物理IME/DPI/多屏/严格无登录、长时间运行、任意第三方callback实现、外部业务应用、实际五示例迁移。历史Mesa真正卸载、Alt+F4、Runtime首次退出及同值粘贴仍按原交接逐项结论。没有运行真实Provider、benchmark、recovery或任何远程工作流。

## 6. 保护、清理及停止位置

保全结果以本地`preservation.json`为准，逐文件比较接手哈希；三份根规划检查原字节前缀，原主线状态不改。`.gitattributes`、A+B验收第7节及其历史原始日志、固定依赖、冻结SDK保持。`delivery-files.json`列本轮授权文件身份，`binary-files.json`列最终匹配二进制，`cleanup.json`只列新目录无用途中间obj；不清理旧build、历史材料或参考工程。

停止在C1代码评审与验收。C2人工来源、非read效果和通用UI绑定仍需单独冻结并授权；stateful/EDA模型、草稿冲突、五示例完整迁移及后续业务写入均未实施。事务/撤销和AI受控编辑仍属P4；AI Service、主对话、Provider、Coordinator、SQLite、凭据、存储/恢复均未实现。无子代理、computer use、Git写操作、release或tag。


本批最终自检补充：生成调用的cap记录已释放但旧Assistant跟踪尚在时，取消不得回退到legacy取消callback；专用断言验证该路径，最终以`final-review-*`日志为准，先前closure结果另保留。最终保全核对：17339份原文件字节相同，17份仅授权文件有改动，3份授权新增；缺失、意外变更及意外新增均为0，三份规划原字节前缀保持。两次本轮清理共949份中间obj、130880853字节；保留最终匹配二进制、包、配置/复现脚本、首次失败及全部简明日志/JUnit。未清理旧证据和冻结材料。

## 7. 2026-10-22 C1评审收口独立证据

### 7.1 基线、归属及核实

本节是原C1交付后的独立整改，不覆盖第1—6节或A+B第7节。实际框架仍为`codex/p1-process-boundaries`、`e0eca03db25930be55bdb45477027d1793a61904`，暂存为空，接手已有33个tracked修改和大量历史未跟踪材料。原C1的20份delivery源码/文档身份与接手工作区相符；不能把HEAD到工作区的全部差异算成本轮。参考工程`main`、`67eacf5c47a856f5d3d68262f247659a15a13f60`根状态只读核对干净，没有重新运行九模块测试。

新目录：[本轮本地证据](../../build/xuanshu-p2c1-closeout-20261022-c72a91)。`baseline.json`记录17359份既有非忽略文件及三份根规划的长度/SHA-256；`git-entry.txt`记录实际Git状态。原C1、A+B、整改及首次失败目录未写入/清理。所有本节日志均是新目录，不借用历史16/16、16/16、10/10当成本轮结果。

| 问题 | 核实与最小修复 | 新回归 |
| --- | --- | --- |
| 激活与旧回调重入 | 原validate/confirm等没有激活租约，外层缓存策略；为validate、confirm、visitor、progress、cancel、snapshot及四种事务回调增加独立host租约，初次enable PENDING，回调结束后重查策略。能力dispatch嵌套标记不被该租约占用 | validate/confirm尝试启用；成功/拒绝返回路径；外层退栈后重试OK、旧修改handler零新增；visitor/snapshot/progress/cancel/四事务回调；正常read同步完成、重复派发及普通人工入口 |
| 事务修改旁路 | 原begin/commit/rollback/undo直接调用应用；只读模式在调用前PERMISSION_DENIED，begin输出ID0，回调零新增。未启用host按原legacy执行 | 四个真实配置回调，模式前各执行一次、模式后全部拒绝；高max_permission、approve confirm、human来源和label参数均不能授权；普通人工handler仍运行 |
| 失败桥接预算与通知 | 原prepare失败前pending/source未计账；现分配前按实际字节预留，结果回调返回后释放，Assistant跟踪另计账。原8MiB内预留一个固定失败槽，所有生成失败通知内再提交生成请求均在接纳前背压，无递归通知 | 非法参数observer重入；填满1024真实call后保留原关联；64个RUNNING call以最大输出及递减输出填满原8MiB，首个普通失败可观察；invalid_output失败重入；多Assistant共享1024、销毁回收、排空后正常调用 |

修复前新增有界测试5214检查/48失败：观察到错误激活、事务回调执行和通知嵌套；递归深度最多4，没有制造堆/栈耗尽或复现崩溃。`red-native-test.log`、`native/red.xml`保留。预算缺口来自源码所有权核对，不能把这48条断言失败说成48次崩溃或完整泄漏动态诊断。后续回归追加commit/rollback/undo激活、confirm拒绝路径、非法输出通知及借用字符串检查，最终5259检查；没有删除或放宽原测试断言。

### 7.2 文件、合同及兼容影响

本轮源码仅`src/assistant.c`、`src/capability.c`、`src/ui.c`、`src/ui_internal.h`；新增`tests/capability_closeout.c`及CMake测试注册；`capability_entry.h`仅补文字合同。另追加三份设计状态、当前交接、本节及根规划checkpoint。完整身份见新目录`delivery-files.json`；不是原C1交付清单。

没有新增导出、公共结构字段、版本或错误码，保持SDK0.10-dev/API10/标准10、ABI1/包1和v1 read。初次enable仍使用PENDING；旧修改及事务辅助API使用既有PERMISSION_DENIED。失败observer中普通生成invoke为0、Assistant为LIMIT_EXCEEDED/ID0，表示未接纳、无新result和call ID；不能自动重试。普通生成入口字节不足时可借固定槽交付原`{"status":...}`；Assistant跟踪分配不足则直接背压。正常终态、输出校验、取消及重复合同不变，原限额未扩大。

兼容核对：`examples/framework_host/main.c`的旧Assistant面板使用事务辅助API，启用模式后也被拒绝；这是封闭已有旁路，不把该面板当成可信人工来源。未迁移host保持legacy，普通业务菜单/命令没有发现依赖这些辅助API，人工入口测试保持通过。没有实现C2来源证明或P4事务/撤销。

### 7.3 本轮新构建与验证

MSVC19.50.35727、Windows x64 Release；Native/Light完整构建，ASAN限定目标构建，均使用独立新目录。Light ON、WebView2 OFF、standalone OFF、OSMesa空；ASAN采用native及`/fsanitize=address /Z7`、`/INCREMENTAL:NO`，共享DLL和夹具同样插桩。测试跨配置串行运行，未改断言、超时或固定依赖。

| 最终源码 | 结果 | 原始证据 |
| --- | --- | --- |
| Native限定 | 17/17，23.69秒 | `acceptance-native-build.log`、`acceptance-native-test.log`、`native/acceptance.xml` |
| Light限定 | 17/17，7.72秒；workspace_offscreen 2.36秒，门槛仍20秒 | `acceptance-light-build.log`、`acceptance-light-test.log`、`light/acceptance.xml` |
| ASAN限定 | 11/11，3.57秒 | `acceptance-asan-build.log`、`acceptance-asan-test.log`、`asan/acceptance.xml` |
| 新收口回归 | 三配置均5259检查/0失败 | 上述三份test日志 |
| 原C1及A+B | C1入口20923/0、JSON79/0、core3747/0、JCS独立268向量通过；真实DLL原workspace/entry两个编译分支均通过 | 上述日志及原测试源码，非历史搬用 |
| 其他关联回归 | Assistant lifecycle/protocol、公共C/C++头三配置通过；Native workspace/layout/7、Light modal/menu_access等按日志范围通过 | 上述JUnit逐项清单 |

一次ASAN目标选择错误：在native ASAN配置选择仅Light存在的`ui_capability_modal_test`，Ninja在编译前报unknown target；原`final-asan-build.log`保留。随后`corrected-asan-build.log`构建实际目标成功，最终增量构建见acceptance日志。没有为此新增不存在的目标、改配置或弱化检查。首轮预期red失败与工具目标错误之外，最终限定测试无失败；不是完整CTest或完整产品验收。

### 7.4 历史问题、未验与停止

浏览器20项失败仍未重跑、未定位；源码公共host/Assistant路径有本轮改动，兼容回归通过不能证明历史失败与公共路径无关，更不能归因环境。原workspace首次20.05秒超时（门槛20秒）保留，当前2.36秒通过不解释首次。Native输入首次失败仍未重跑/定位，历史后续单项通过不覆盖。三项均未借本轮修复改归因或标已解决。

未验：完整Native/Light/browser矩阵、WebView2/Mesa、长时间运行、真实物理输入/IME/DPI/多屏、任意第三方回调的合作行为和实际五示例迁移；没有做内存分配器OOM注入或无界栈耗尽实验。真实DLL两分支重新验证原worker/外部回调退出、同步栈、复制队列背压及迟到实例合同，没有扩展为任意应用线程自动追踪。参考工程、固定依赖、冻结SDK及历史材料保持；没有Provider、benchmark、recovery、远程工作流或Git写操作。停止在C1整改后的代码评审与验收，C2及后续未实施。

保全与清理完成情况见本节末尾及新目录`preservation.json`、`binary-files.json`和`cleanup.json`。仅清理新目录无用途中间产物，保留最终匹配二进制、PDB、包、配置、日志和首次失败证据。

最终保全核对：17359份接手文件中17345份字节相同，14份为本轮授权修改，新增仅`tests/capability_closeout.c`；缺失、意外改动、新增均为0。三份规划、三份设计及原C1验收正文的接手字节前缀全部保持。交接只插入本轮最新状态，原历史段保留。固定依赖、版本、公共旧布局测试和所有原测试源码均未改变；原C1/A+B本地证据、用户`.gitattributes`修改保留。

清理限定于本轮绝对路径，经范围核对删除457份中间obj、62921159字节，剩余obj为0；未清理原build或历史目录。保留184份二进制/库/PDB/包身份（含构建支持程序）、配置和复现脚本，以及修复前有界失败、工具失败与最终原始日志/JUnit。最终源码与匹配二进制身份在本轮清单中；没有把重编后的可执行文件冒充修复前red运行的原二进制。最终测试完成后仅更新文档和规划，没有再修改产品或测试源码。

## 8. 2026-10-23 legacy异步结果observer独立整改

本节只核实并修复复审发现的激活窗口，补固定失败槽连续借用回归。原第1—7节、A+B及其整改、历史失败和旧证据目录保持。实际框架仍为`codex/p1-process-boundaries`、HEAD `e0eca03db25930be55bdb45477027d1793a61904`、暂存为空，承接已有33个tracked修改；不将全部HEAD差异归为本轮。新目录为[独立证据](../../build/xuanshu-p2c1-observer-20261023-7d14b2)，接手`baseline.json`覆盖17360份既有文件及三份根规划。

### 8.1 核实、最小修复与合同

源码缺口已动态验证：旧异步handler退出后，UI线程reply先摘除最后一个pending，再直接调用legacy结果observer；原enable门看不到仍在执行的回调。成功及失败结果两种用例，在observer内、释放旧Assistant request前后分别尝试enable，原代码均错误返回OK。修复前5331检查/6失败，仅为上述激活断言；没有崩溃、无界递归或OOM实验。

产品只修改[ui.c](../../src/ui.c)：legacy reply在摘除pending前增加独立host旧回调租约，待observer返回、pending及其字符串释放后解除；覆盖成功、失败和无observer路径，不占能力registry的dispatch回调标记。参数非法、重复reply等提前返回发生在取租约之前。已有read、取消、事务拒绝、错误码、公共结构、版本、依赖与资源限额保持；不新增API或C2人工来源合同。

[收口用例](../../tests/capability_closeout.c)验证外层reply返回后重试enable成功、旧Assistant修改拒绝且handler无新增、普通人工入口仍可用；重复reply不重复通知。最终增加已启用模式的legacy结果observer中调用生成read：同步完成成功，退出后重复dispatch不重复执行，结果正常释放。该补充让最终检查数从修复前5331增至5349，没有削弱原断言。

固定失败槽回归在原8MiB仍满、没有释放任何call/output的条件下连续提交3次；每次不同source及递增request ID，返回原LIMIT status、无call ID、execute不增加。各次observer内重入仍在接纳前背压，借用command/source保持；通知深度始终1。随后逐项核对原调用状态、释放记录并恢复正常调用。该项在产品修复前已经通过，本轮没有修改槽实现或限额。

### 8.2 修复前后证据与匹配二进制

修复前使用专用`red/native`目录；`red-native-configure/build/test.log`及`red/native/red.xml`保留。修复前保存`red/sources/src/ui.c`、`red/sources/tests/capability_closeout.c`、`red/source-before.json`及`red/binary-before.json`，包含原位置EXE和headless静态库身份。最终构建使用另外的native/Light/ASAN目录，未重编或覆盖red二进制。最终两份对应源码另存`final-sources`，全体编译素材身份另列`source-after.json`，最终二进制列`binary-files.json`。

这仅补齐本次observer问题的修复前后证据；不能倒补或冒充2026-10-22原48失败red运行的二进制。旧记录的该证据限制继续保留。`product-change.diff`只描述本轮ui.c变化。

### 8.3 本轮新验证

MSVC19.50.35727、Windows x64 Release，新独立目录；Native/Light完整构建、native ASAN限定目标构建成功。Light为standalone OFF、Light ON、WebView2 OFF、OSMesa空；ASAN为`/fsanitize=address /Z7`及`/INCREMENTAL:NO`，共享DLL/夹具同样插桩。测试跨配置串行运行，没有改超时、警告、断言或固定依赖。

| 独立运行 | 结果 | 原始证据 |
| --- | --- | --- |
| 修复前Native专用回归 | 5331检查/6失败，确认激活窗口；连续槽借用通过 | `red-native-test.log`、`red/native/red.xml`及匹配red二进制 |
| 最终Native限定 | 17/17，24.63秒 | `final-native-build/test.log`、`native/final.xml` |
| 最终Light限定 | 17/17，6.73秒；workspace_offscreen 2.07秒、门槛仍20秒 | `final-light-build/test.log`、`light/final.xml` |
| 最终ASAN限定 | 11/11，3.10秒 | `final-asan-build/test.log`、`asan/final.xml` |
| 本次扩展回归 | 三配置均5349检查/0失败 | 各final test日志 |
| 原合同回归 | C1入口20923/0、JSON79/0、core3747/0、独立JCS268向量通过；Assistant lifecycle/protocol、公共C/C++头和真实DLL workspace两个编译分支通过 | 各final日志/JUnit及原测试源码 |

最终限定无失败或跳过；不是完整产品CTest验收。真实DLL原用例继续核对worker/外部回调退出、同步栈、复制队列背压及迟到实例；没有将本次standalone observer用例说成新增DLL场景。

### 8.4 边界与停止

原浏览器20项失败、workspace首次20.05秒超时（20秒门槛）、Native输入首次失败仍独立未定位。本轮不重跑浏览器或输入项，不以2.07秒限定通过解释原超时，不默认归因环境。未验完整Native/Light/browser、WebView2/Mesa、长稳、物理输入/IME/DPI/多屏、任意第三方callback合作行为及OOM注入。

本轮只改ui.c、现有收口测试、当前交接、本节及三份根规划；三份Proposed整体设计保持，未实现C2、五示例迁移、业务模型/写入、事务/撤销、AI Service、Provider、Coordinator、存储或恢复。未修改XuanShu、外部业务应用、冻结SDK、固定依赖或历史材料，无子代理、computer use、Git写操作或远程工作流。停止在C1复审位置。

最终保全：17360份接手文件中17353份字节相同、7份授权修改，无新增、缺失或意外变更。三份根规划、三份设计及原验收第1—7节的接手字节前缀保持；既有用户修改、公共头、CMake、原C1/A+B测试及依赖保持。新目录`delivery-files.json`列7份修改，`preservation.json`列核对结果。

清理经绝对路径检查，仅删除新目录473份中间obj、65521897字节，剩余obj为0。保留188份二进制/库/PDB/包身份（其中修复前red目录4份）及前后源码快照、配置、脚本、red和final原始日志/JUnit。red二进制哈希在清理后仍与修复前清单一致；最终源码在final运行后只做记录更新，没有再改产品或测试。
