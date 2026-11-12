# P3开发阶段模拟及自动验收

2026-10-24。读者：框架维护者、接口和应用开发者。无署名。结论：**P3开发阶段模拟及自动验收通过**，限定为本文的stub、只读、隔离存储及程序UI范围。不是设备人工验收、全部历史CTest绿色或无条件P3完整验收。停止于本轮交付和代码评审，不进入P4。

## 1. 基线、归属和交付身份

实际分支`codex/p1-process-boundaries`，HEAD `e0eca03db25930be55bdb45477027d1793a61904`。证据根`build/xuanshu-p3-20261024-f19c6a`：入口baseline17408文件；此前P2模拟交付17项、源码280项、二进制77项入口均匹配，见`prior-evidence-check.json`。没有将HEAD至工作区的A+B/C1/C2/五应用接入全部归为本轮。已有修改、未跟踪、历史材料和冻结SDK保留；参考工程历史不可读，本轮没有重复申请访问或更新其身份。

本轮产品增量是独立`ai_service.h`、`src/ai_service.c`/`ai_internal.h`/`ai_store.inc`、workspace私有单Service登记、framework_host的`ai_panel.inc`/main.c/host.html，以及必要CMake和SQLite离线供应。新增`tests/ai_service.c`、`ai_tools.c`、`ai_panel.c`及legacy生命周期包清单；公共C/C++头测试仅增加独立P3头及前缀断言。旧断言、超时、P2数据范围和限额没有放宽。完整逐文件归属、SHA-256及二进制身份以`delivery-files.json`、`source-after.json`、`binary-files.json`为准，保全与清理另有清单。

SDK0.10-dev/API10/标准10、ABI1/包1、原公开结构/v1只读/冻结C2均保持。Light生产DLL315导出（原299加16个P3函数）；native配置301（Web配置差异，不拿Light的299当native原值）。两者没有ui_ai_fixture、内部workspace登记或私有授权/route导出，见`production-exports.json`及原始exports文本。Provider、SQLite和测试时钟接口不公开。

## 2. 冻结合同与实际闭环

实现前先冻结[P3-1](../xuanshu-p3-service-contract.md)。一个workspace一个Service、一条固定conversation；宿主不为各ui_host创建AI服务。create无I/O，后台open恢复后才READY；REFUSE后的resume若仍在打开库，保持OPENING，不提前用空conversation身份接纳。Service常驻，串行worker按需启动/退出，UI只零时观察线程句柄，不join依赖UI的worker。

普通任务16排队加1活动，cache64/命令32、保留数据8MiB；控制预留256KiB、通知64项/2MiB、工具邮箱一个/2MiB、查询512KiB。输入16KiB、计划64KiB、上下文128KiB、回复64KiB，64步/16模型续轮/64工具安全上限。P2额度独立，未实现P5共享预算或并发。限额是队列、复制数据与任务的逻辑安全门，不是SQLite/图形后端整体RSS上限；长期资源验收仍属P6。

client_request_id在调用方建立，确定性run_id在同步received前写回。received/内存入队不等于已保存；SQL事务提交后accepted。相同ID/完整canonical内容幂等，异内容明确拒绝；终态和最终消息同事务，唯一终态索引。流式UTF-8为合并前缀快照；通知拥塞时最终状态仍可查询，不能伪造完成或丢关联。

stub明确为测试模型；默认回显，高级脚本显式text/discover/tool/ask/fail，非自然语言路由。工具参数完整聚合、严格JSON/JCS校验后才进UI；目录也过滤非read。固定instance/resource/host epoch/open epoch/revision/scope，P2 prepare→稳定call绑定持久化→dispatch→结果复制→forget，后台不持应用裸指针。修改能力拒绝，不回退Assistant/legacy。取消/关联问题是按明确ID的控制操作，不是文本授权；取消后真实执行者未退出就不终态。

## 3. SQLite固定供应和恢复

官方SQLite **3.54.1**，source ID `2026-10-16 14:45:20 555e07ef33f69a0957d24d8e555b702464a131d6f1607f4c3c1e4551e8e6ca76`。供应地址为[官方amalgamation](https://www.sqlite.org/2026/sqlite-amalgamation-3540100.zip)，[发布记录](https://www.sqlite.org/releaselog/3_54_1.html)、[公有领域声明](https://www.sqlite.org/copyright.html)。保留原COPYRIGHT.html、源码头、LICENSE.txt和source-lock.json。

压缩包SHA3-256 `92281c86e2957f9b9305890f9ac04204caa7b677cc868dc326d942689df28e30`与官方清单一致；SHA-256 `0a9c8e34352dad46006aa82836d5383bff9dc6bc9ce5a60553c05aa30e6ad1f9`。sqlite3.c的SHA-256为`0c657666d819f261cc4007526ad57e1669de9949df7a8ea847aa284a2bf0ee52`，其余文件完整哈希见锁文件。`cmake/sqlite.cmake`离线验证，编译THREADSAFE=1、OMIT_LOAD_EXTENSION、DQS=0，不编译shell或构建时下载；第三方自身诊断与框架/WX隔离，其他固定依赖不变。

schema1/application_id固定；conversation/runs/messages/events/tools持久化必要身份、关联、目标和结果，不保存应用业务文档。验证归属/schema/quick_check后使用WAL+synchronous FULL，逻辑独占库，busy后台50ms有界失败，关闭checkpoint/flush。打开损坏、空文件、外来库或未来schema明确失败，不重建冒充恢复。原库字节对照见storage测试；失败注入是受控事务确认失败，不伪称真实磁盘满。

独立子进程仅使用本轮临时库，保存WAITING_INPUT后进程退出。另一进程恢复为NEEDS_REVIEW，active和queued为0，无工具重放；显式continue重新bind当前目标、传新请求ID/脚本，生成关联新run。已有用户库、参考库及用户进程未操作。P3最小持久化已经实现，P4写意图/审批/修改核对和P6迁移/大规模故障窗口仍未实现。

## 4. 本轮自动证据

所有新测试采用明确状态谓词、受控事件及有界poll；1ms MsgWait只是让出执行机会，不以固定延时假定完成。TIMEOUT保持P3核心/工具20秒、面板30秒，原DLL两个分支15秒、旧frontend60秒。动态poll检查次数随调度变化，不当成功能数量。

| 证据 | 结果和真实断言 |
| --- | --- |
| `native/sdk-closure.xml`、`sdk-closure-native-test.log` | P3专项6/6；单Service/会话、UTF-8、同步received关联、重复提交、幂等恢复；控制、storage、limits、真实DLL工具 |
| `light/sdk-closure.xml`、`sdk-closure-light-test.log` | P3专项7/7；前六项加实际framework_host/Light页面/JS桥接，panel专用检查0失败。两counter实例目录/实际count=0、counter-1回读，counter.increment拒绝；收面板/切标签/历史/问题/输入保留/REFUSE收尾 |
| `asan/sdk-closure.xml`、`sdk-closure-asan-test.log` | P3专项6/6，ASAN无报告；不称Light面板也经过ASAN |
| `native/scoped.xml` | 相称旧回归13项全部通过；JSON/核心/C1 entry20923/closeout、v2/FORM、Assistant、公共C/C++、AP-1、真实DLL原入口和C1生成入口两个编译分支。该次总18项中新增tools夹具错误1项失败，后续单独修复，不能把原JUnit改为18/18 |
| `light/joint.xml` | 相称回归14/14，含modal、组件共享测试分支及真实DLL两分支；`frontend.xml`中component_resilience另通过，带旧JS诊断见下节 |
| `asan/scoped.xml` | 旧回归6项和当时P3五项共11/11；最终P3另以sdk-closure重验。静态/动态构建分别核对，不拿一种分支代替另一种 |

### 4.1 合同覆盖和边界

| 项目 | 具体断言/依据 |
| --- | --- |
| 单Service、关联、同步完成 | 同workspace第二create ALREADY_EXISTS；Service存在workspace_destroy CANCELLED；received时out run已写回且get_run可用，嵌套poll/destroy PENDING；同步P2 complete可先于dispatch返回，重复complete ALREADY_EXISTS |
| 静态SDK依赖闭包 | Service测试不再额外编译workspace/package私有源码；同源ui_ai_embedded仅链接native静态库，同样恢复主对话/幂等/安全关闭。headless与native库分别构建并验证，不用测试EXE私自补依赖掩盖SDK链接缺口 |
| 队列和控制 | 1活动+16排队，下一提交LIMIT且输入不变；普通history命令填满后cancel仍接纳。取消已排队任务model_calls=0；停滞仍terminal=false，释放受控执行者后才cancelled；REFUSE恢复接纳不复活旧任务 |
| 流和展示背压 | 中文/emoji每次通知可严格解码；50000字节输出在不poll通知时仍能查到持久终态，通知字节≤2MiB，排空后只发一次finished。history limit33拒绝，limit1明确has_more |
| 只读工具 | 分类发现输出含counter.read、不含counter.increment；真实模型值count0/revision counter-1；碎片`{`+`}`完整才执行，半JSON失败、非法参数不execute、重复tool ID仅第一次执行。非法输出触发P2 invalid_output，Service失败而非假成功 |
| 目标和生命周期 | 真workspace两个实例、切活动标签后投递原实例；revision变更后prepare有稳定call但dispatch前拒绝，execute=0。关闭重开拒旧instance/epoch；旧实例迟到投递CANCELLED |
| 真DLL收尾 | DLL自有worker及外部回调均STARTED后取消/关闭，Service destroy PENDING，模块/实例仍在。worker退出而callback未退出仍非终态；两者确认退出后结果/forget/卸载完成。独立单实例workspace执行同步complete、外层execute在栈时forget/注销/关闭均受原租约门保护 |
| 存储与唯一终态 | 18个控制任务每run唯一终态；事务注入失败不产生假runs/messages，缓存UNCONFIRMED；锁冲突、损坏和未来schema拒绝并核对原字节。专用子进程重启只恢复记录，不执行旧计划；显式继续才新增一个关联run/终态 |
| UI与输入保留 | 公共程序输入通过真实Light控件/生产JS提交回显，确认后清输入；非法脚本保留文本。宿主action也单独验证保留输入。模拟点击仅证明控件/桥接/业务，不是物理来源或非read授权正例 |

P2未改的业务模型、统一目录/严格Schema/JCS、安全来源/UNKNOWN/草稿等沿用已匹配P2证据；没有重复全套应用迁移或伪称P3重做五示例验收。网络Provider、审批、写入、Coordinator、子Agent、存储迁移和长期压力不在本轮完成声明内。

## 5. 首次失败、整改与未定位问题

1. 首轮core编译缺package链接、shared宏重定义、工具测试缺app_data声明，以及一次错误目标名/缺EXE的Not Run，属于构建/组织失败，不冒充红测试执行。日志和编译入口清单保留。
2. `storage-control-first-native-test.log`：control首次20.04秒超时，原5秒状态断言未排空；原DB/热journal、匹配二进制、源码和JUnit保留。诊断只读取复制库，显示持续推进。实现前设计细化为WAL+FULL后在原断言/超时下通过；没有降synchronous或放宽门槛，也不据此解释历史workspace首次超时。
3. tools首次误查“fixture”而实际输出是value42；新增revision测试误以为prepare即拒绝所有版本冲突。按既有合同改为精确value42、稳定call非0且execute=0/dispatch失败；并非放开错误或修改P2。
4. 新增同步DLL收尾测试错误复用要求单实例的原夹具到多实例workspace，导致其既有断言失败；之后失败断言仍继续调用旧host，ASAN实报heap-use-after-free。保留`reviewed-asan`日志和匹配EXE/DLL；源码可见失败open/get_instance后继续用app.host。修复新测试的隔离场景及失败检查，不放宽原夹具断言，不将它宣称为Service产品UAF修复。REFUSE测试原来直接开DLL而非uapp，改为真实fixture包并保留资源；最终均通过。
5. 自审最小产品修复：目录过滤和发现目标revision核对；模型/步骤限额在增加前检查；存储打开期间resume不提前READY；历史实际JSON分页及SQL错误不伪装EOF；已存同请求的existing通知带原accepted序号，展示/查询快照与权威新事件区分。收尾又补工具事务中的runs WAITING_TOOL/结果阶段同步，在真实异步read未完成时通过query_run验证持久phase4/committed=true；最终三配置sdk-closure再次验证。同步/异步关联、取消和旧公开合同未改变。没有宣称这些都曾动态复现崩溃。

   最终静态供应自审还确认：原Service夹具额外编译workspace/package会掩盖库依赖未闭合，不能作为仅链接SDK的证明。现已将已有依赖纳入相应静态库、移除EXE补源码，并新增native静态库同源嵌入测试；这是源码核实后的构建修复，不伪称先前已动态复现公开SDK链接失败。原回归与公开布局不变。
6. **旧完整frontend仍失败**：`frontend-light-test.log`中63个断言（ctest重复输出为126行），含要求旧Assistant修改EDA、旧allowlist/参数注册、旧工具写入等。`comparison`以入口宿主/页面/测试重建、同一当前框架DLL复核，63项失败逐条相同；不是完整旧框架二进制对照，不能扩大结论。相关P2目录/Assistant/应用源码本轮未变；不放开修改旁路或删旧断言使其变绿。优先级为后续前端测试合同迁移/诊断兼容核对，不阻断本文P3专用只读闭环。旧component_resilience通过但输出一次原页面null节点JS诊断，保留为非阻断待核对，不能当作无错误页面验收。

历史Light surface_input首次、浏览器20失败、workspace首次20.05秒（门槛20秒）、Native输入首次继续独立未定位。本轮未重跑这些项目；win32_surface/P2来源政策未改，不能据此推断环境根因或已解决。新宿主timer/poll路径有程序UI和生命周期证据，但物理输入风险仍保留。P3本轮control超时和fixture红证据不能替代或销掉历史失败。

## 6. 保全、复用和停止位置

`preservation.json`区分入口原文件、已授权改动及新增；此前P2证据二进制再次核对，不删除历史失败。`failure-files.json`保存原始失败日志/JUnit、阶段源码/二进制和必要DB/journal；构建前输入清单与执行时产物分开，不能把编译失败后的旧EXE称为新源码匹配红执行。对照harness编译过程错误另留文本，不冒充产品回归。`cleanup.json`只列本轮已无用途的编译/打包过程件，保留首次失败和可运行匹配包。

本轮32项交付由13项入口文件修改和19项新增构成；其余17395个入口文件字节不变，原P2的77项二进制仍匹配。最终每配置构建/测试前307项输入与当前源码核对均无差异，见audit.json；失败运行的相关源码副本也与各自运行入口清单匹配。保留101项正式/对照二进制清单。Light宿主SHA-256 `F604AA0287F99CA499D910DACBD9E19C5DBC3832B9705FF47E23A3B31885A947`，Light ui_framework.dll `42FA1F0F23A90DACEF9EE963C1358257A5B65B94DBD53E0F3BC546B32DABB712`。清理本轮对象/try-compile/重复打包暂存目录；三次清理共30次已核对路径操作、344995550字节；仅作用于本轮证据根，失败/最终EXE、DLL、uapp、静态库、原始日志/JUnit、源身份、隔离DB/journal及SQLite离线供应保留。再次构建可重新配置，不依赖已删除对象缓存。

完整物理来源、Windows动态正例、真实IME、跨屏DPI、视觉/可访问性和长时均**延期未验**，见[P3集中操作清单](../xuanshu-p3-deferred-manual-acceptance.md)及原P2延期表；没有SendInput或私有接纳冒充物理设备。没有真实Provider、密钥读取、模型网络、用户数据库操作、参考工程修改、SDK覆盖、远程工作流、benchmark/旧recovery、Git提交/推送/发布/tag。停止P3代码评审位置，P4等待新授权。
