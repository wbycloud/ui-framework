# UH-A正式宿主开发与离线联合验收

2026-11-03。开发和自审记录，不是独立复审或真实服务/人工验收。新根build/xuanshu-uha-20261103-71d0c9。HEAD e0eca03db25930be55bdb45477027d1793a61904，分支codex/p1-process-boundaries，原有Git修改/未跟踪文件保留，不能把HEAD全部差异归本轮。

## 1. 基线及证据限制

承接RPCB-1冻结identity41D15CFF8A2E0F639D02B6748C0F90AB95E9D6B4CAB7368221CF357BA4E8FF7C；其3项源码入口哈希及8项程序匹配。旧清单/副本/真实记录不改，新UH-A代码不继承旧验收。参考工程本轮只列出根目录，未修改或扩大权限。适用用户提供AGENTS，无额外磁盘AGENTS。

入口顺序哈希共17534项（Git列出已有文件，另含3根规划）；不是原子快照。捕获耗时期间实施开始，末尾对当前CMake的前置检查遇到本轮已授权变化而失败。入口CMake/launcher/UI-test三项仍匹配冻结；main.c、ai_panel.inc、ai_service.c、ai_model.inc捕获与修改交错，不能将这些行称作修改前自动捕获。baseline.json原样保留，baseline-note.txt/baseline-supplement.json列明差异；source-before-from-archive从对应历史source-final取得比较前文，明确为归档对照而非本轮自动捕获。最终增量使用该对照及本轮动作，不将所有HEAD差异计入。

## 2. 冻结规格及实现

[UH-A合同](../xuanshu-uha-host-contract.md)先记录必要设计后实施。新独立公共头ai_model_view.h只新增ui_ai_model_get_disclosure，返回version1复制JSON，使用生产初始POST同一构造器，完整模型参数、instructions/tools/input及固定目标/scope。方法和任务引用覆盖目标回调/外层栈；回调返回重检关闭/取消/存储/config/revision，不能查询即不确认。旧公开布局、错误值、v1只读、C2/P4/P5、依赖/限额不变，无DDL/网络/凭据/写入授权变更。

正式页面用历史片段、分页资源/read目录及Schema属性字段按钮代替日常JSON；默认空选，不自动星号/全历史/文件。提案复制固定目标和选择，不随标签切换重定向。完整预览每块≤3000 UTF-8字节，宿主逐块读回presentation比较并核对run/binding，才绑定原可信确认；页面消息不能证明显示或批准。稳定内容不重复改文本，保留选择/捕获；失败/旧hash/关闭禁止旧确认。UI只读/有界处理，库/网络/凭据仍后台，原租约/cleanup-only和UNKNOWN不变。

草稿由宿主内存复制，同页刷新/折叠及页面重建保持；不是进程重启后的草稿存储。显式UI_AI_DB_PATH在正式/测试构建都派生.model.json，默认用户路径保持，不迁移或复用RPCB凭据。配置ready和凭据readable分开，诊断复制不读密码框、不授权/提交/联网，复制失败显示状态和系统码，保留原任务/凭据错误。stub/edit/graph诊断仍在。

## 3. 自动验证（不同运行分别记录）

| 最终证据 | 结果 | 范围 |
| --- | --- | --- |
| light/final.xml | 95/95（路径保护前的联合基线） | 正式Light/原生和嵌入、协议/RP状态机、公共C/C++头、原面板/编辑/图、5项新UI及2项新API租约/revision |
| light/control.xml | 3/3 | 独立补充控制/存储/限额，未与95合并成首次单次通过 |
| light/diagnostic-path-final.xml | 1/1 | 正常用户上下文路径保护后匹配程序，只复制测试自有脱敏快照，无凭据/网络 |
| native/final.xml | 29/29 | 另一编译分支的同源Service/RP/静态嵌入/头/协议及新API回调 |
| asan-light/final.xml | 33/33（路径保护前） | 新API/正式Light页面/真实DLL异步/取消、存储失败cleanup-only等，运行时与测试均此分支编译插桩 |

| light/path-guard.xml | 9/9 | 最后路径保护后正式/编辑/图/模型面板及5项新UI；含超长路径明确拒绝 |
| asan-light/path-guard.xml | 6/6 | 同一局部变更后的ASAN新UI及模型面板 |
| asan-light/diagnostic-path-fixed.xml | 1/1 | 正常用户上下文补齐已安装ASAN运行库查找路径后，同程序、同30秒阈值复制用例 |

最后自审检查snprintf派生路径可能在原4095字节附近截断，增加明确LIMIT_EXCEEDED和错误投影，不回退用户配置、不增加上限；正常/过长路径、配置代次不改变均加入新UI用例。只是启动宿主局部修正，不修改运行时。95/33已运行的UI/生产DLL原程序在pre-path-guard-binaries归档；private DLL和非UI程序未变，生产DLL因既有生成头依赖重新链接，最新9项真实正式面板回归覆盖新DLL，ASAN运行时未变。path-guard-binary-changes.json逐项列明，不将旧UI程序结果自动算作最终新UI通过，也不机械重跑无源变化的协议全部用例。

先前joint93/93与各阶段UI通过是历史阶段证据，新增API用例/最终重新链接后以final为准，不混称首次全部通过。诊断复制在受限上下文Win32 error5失败、未读取非本测试成功写入的剪贴板；未重建的同程序在正常用户上下文对照通过，最终程序另1/1通过。仅支持这项具体上下文差异，不能据此解释所有历史期限/输入失败；没有降低复制成功断言。

新用例直接解析预览request和实际fixture_body逐值相等，验证固定instructions及两个tool、默认2项input/选历史3项input；破坏显示块/旧hash则不建立确认且requests0。真实workspace双实例切换后read仍在原目标；同一生产rpc_memory.c源码的隔离DLL返回整数10/string r1，框架结果revision=r1，关后tools结果保存为{revision:r1,value:10}。原生产rpc_memory.uapp另保留供人工，不用测试DLL冒充生产来源。诊断扫描秘密哨兵，伪造确认被拒绝；页面重建后未提交输入/任务/预览保留。新API get_revision重入关闭时destroy=PENDING，外层退栈后可销毁；revision变化拒绝且零请求。全部网络/凭据使用私有替身，程序输入仅模拟，不是物理人工正例。

生产exports.json对照旧冻结DLL：仅增加ui_ai_model_get_disclosure，无移除或fixture/uha/test导出。公共旧头逐字节保持。无断言/超时/数据范围/来源或生产额度放宽。

## 4. 首次失败、自审与整改

- 初次构建将目标列表拼成一个名字、后续误用不存在的公共头目标，是编排失败，未运行对应测试；原日志保留。
- 首个实际编译有局部id遮蔽/WX错误；新增API回调夹具另有state遮蔽/WX错误，分别修变量名，保留失败源码和日志。固定依赖的既有编译警告不通过改依赖消除。
- first-ui首次5项3通过/2失败：诊断复制失败；read夹具本来只有value，却按有revision假设访问，受控测试进程异常终止。源码存在未判空strcmp，未取得异常栈，不外推为产品/用户进程崩溃、死锁或数据损坏。后用同rpc_memory.c源码隔离链接，单实例ini导致双标签前提失败后新增自有多实例ini；不修改原生产夹具值或Schema。
- 后续history夹具在DOM投影前点击，首次单次presentation断言失败；加有界存在屏障，未延长超时或放宽断言。显示失效门、同源业务回读及关闭均保持。
- 正常上下文额外ASAN复制首次30.02秒超时，无断言输出。PE导入包含clang_rt.asan_dynamic-x86_64.dll、原PATH查找失败，补齐已安装编译器bin后同程序同30秒门槛1/1通过；支持启动依赖编排关联。原加载器窗口/失败码未捕获，不冒充观测到具体窗口或已复现业务死锁，不用于解释历史ASAN-Light中断。原日志/XML、同期源码和匹配程序归档。
- 复制失败增加失败点系统码后观察error5，正常上下文同程序通过。不是Key有效性/Provider响应证明；所有凭据仍内存替身。

failures子目录、各阶段sources/binaries清单、原始日志/JUnit保留。早期不存在成功链接的对应目标时不能补造匹配程序；源码捕获交错限制不以后补材料掩盖。最终重复联合运行用于匹配新增用例和最终链接身份，不覆盖首次失败。

## 5. 历史风险、人工及结论

原graph_boundaries20秒、ASAN控制5秒、P2五包15.04秒、面板180.44秒首超时及frontend63/ASAN-Light中断/surface_input/browser20/workspace20.05秒/Native输入首败继续独立。当前正式页面和Service必经路径由上述有界用例通过，未发现直接阻断本交付的同类正确性/关闭问题；本轮不宣称定位或解决其原根因，不默认归因环境，不重跑完整历史套件。

旧T1/T2真实结果、T3本地取消/安全退出、远端和费用unknown、483 Tokens待对账及请求重建非抓包均保持；本轮真实POST0、真实凭据访问0、用户原库操作0。browser九副作用另案，真实来源/IME/DPI/视觉/辅助技术/人工长时延期，物理断电仅专用环境后续方案。

结论：本轮正式宿主开发与自动验证完成，具备进入集中人工验收的准备条件。自审未发现必须突破冻结边界的阻断项，不是独立复审通过或系统完整验收。匹配程序、配置位置、启动命令、人工步骤和失败材料见[人工入口](../xuanshu-uha-manual-acceptance.md)。当前不启动验收、不读Key、不联网，等待下一步明确人工授权。交付身份及保全/清理以本轮delivery-identity/preservation/cleanup清单为准。


交付收尾核对：本轮23项明确增量、413项源码/构建输入及93项最终程序/链接产物，失败及路径保护前匹配产物142项分别归档。17534项入口文件核对无授权外变化或丢失，根三规划原前缀完整。只清本轮700个已消费.obj，保留1146项测试自有隔离数据文件和必要日志/原始JUnit、程序/PDB；数据载荷扫描未见两种秘密哨兵。清理不触及历史、用户库/凭据或冻结SDK。数字以实际清单哈希为准，本记录文档与自审结果不构成独立冻结。


## 后续限定复审及整改索引（2026-11-03，原证据不改）

原开发自审结论之后，独立复审发现投影失败旧确认及长历史256KiB溢出两项阻断，判为暂不通过。本轮新身份已完成三项限定整改及相称离线验证，详[xuanshu-uha-closeout-validation.md](xuanshu-uha-closeout-validation.md)，新根build/xuanshu-uha-closeout-20261103-a6f2d8。原本页结果/首败/捕获限制保持历史；新代码不纳入原identity，不以自审替代再次独立复审，不自动进入人工。
