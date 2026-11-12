# RP-A＋RP-B两项限定评审整改验收记录

2026-10-31。范围仅为能力发现结果适配和cleanup/cache竞争的两项修复与必要回归；不包含RP-C。当前为实施者自审及离线验证记录，不能替代下一轮独立复审，也不宣称正式冻结或真实DeepSeek／真实人工验收通过。

## 1. 基线与增量归属

实际分支`codex/p1-process-boundaries`，HEAD `e0eca03db25930be55bdb45477027d1793a61904`，未提交。已有P2—P6、原RP、用户修改和未跟踪文件保留。参考工程`D:/XuanShu`读取仍Permission denied，未扩权或写入。

新证据根`build/xuanshu-rpab-closeout-20261031-a19c42`。入口`baseline.json`捕获491项当前范围文件，不是全历史Git清单。三份原RP新增文档未在该入口集合内，`baseline-supplement.json`事后按原RP delivery-files精确长度及SHA-256补核；已追加合同从原长度截取的前缀亦与原交付哈希一致。不得将494项写成全部入口自动捕获。原RP源码389项在入口与原source-after匹配；原RP/P6快照结论不自动覆盖本次增量。

交付归属依据本轮baseline、补充清单、incremental-source-diff和delivery-files；不依据HEAD至工作区全部差异。源码、最终/失败程序及PDB、原始日志/JUnit、兼容性、生产导出、保全及清理分别见新根对应清单。

## 2. 源码核实与最小修复

### 2.1 发现结果

核实生产调用链：原P3/v1的`ai_tool_ui`将分页read摘要聚合为根数组，原RP `am_result`误读根对象items，所以合法目录变成空items。原目录用例的整个body子串正例可被初始投影中的memory.read满足，不能证明工具输出。

另核实`ui_capability_discover`按原合同只返回摘要，不含Schema。仅将RP改为遍历数组仍不足以交付完整描述。本次只在RP适配调用同一目录`ui_capability_describe`取得完整read描述，仍返回原根数组到结果消费者；P3/v1摘要和公共发现接口不改。RP将数组按既有白名单投影为items/next，保留ID、版本、说明、Schema和描述指纹。完整描述累计仍受原256KiB工具结果上限约束，失败不截断。

非数组、非法项、缺失必要描述字段返回失败；真正无匹配能力的合法数组才返回空目录。没有第二目录、业务执行器或新授权。非read仍拒绝，Assistant/AI政策不变。

### 2.2 cleanup扫描

核实`am_config_io`由worker在Service锁外调用，原扫描没有run引用；已取消终态且网络完成的任务可同时被UI提交引发的64槽缓存回收释放。属于真实源码竞争窗口。

现在锁内选择候选、复制run ID并取得临时扫描引用，解锁后处理网络cleanup，再锁内核对ID/对象并归还一次。扫描引用与原网络引用独立，完成待消费、回调退出及存储确认原门不变。身份异常封闭后继执行并保留不可用诊断，不错减复用slot引用。未跨网络或应用回调持锁，UI无等待worker/队列空间；未永久保留终态任务或提高额度。

存储不可用/取消/关闭仍仅收尾，不能启动新的业务/HTTP、清除UNKNOWN或伪造完成。相关既有P4/P5/P6收尾用例纳入相称回归。

## 3. 新增受控用例与修前证据

新增`tests/ai_model_closeout.inc`，由现有真实workspace及DLL模型夹具复用；CMake在原20秒门槛下注册`ui_ai_model_cr-discover`及`ui_ai_model_cr-scan`。

| 用例 | 真实断言 | 修前证据 |
|---|---|---|
| cr-discover | 直接解析出站请求input内function_call_output.output；与统一目录describe比较ID、能力版本、说明、name、effect、指纹和输入输出Schema；memory.hidden、write/external不可见；真实不存在分类为空；7种非法上游格式失败 | red-controlled Native 31项断言失败；不是模型真实服务行为 |
| cr-scan | 建立已取消终态且网络完成的真实模型任务，再持久化63个普通任务填满真实64槽；候选访问前暂停worker，UI新提交触发真实回收；候选refs=1/ID/slot保持；释放屏障后refs=0且恰好消费一次，后续新提交可正常回收旧候选；安全关闭 | red-controlled Native 4项断言失败；occupied=64、refs_during=0、retained=0 |

修前屏障在恢复时先核对身份；发现已复用则报告失败并跳过危险解引用，避免为了留证故意崩溃。该证据证明对象提前回收及缺少引用，不宣称实际UAF崩溃、死锁或数据损坏。同步脚本源及匹配程序在`red/native`和`failures/red-controlled-native-test`独立保存，不会用最终二进制覆盖。

测试屏障/投影探针仅编入UI_AI_FIXTURE；额外探针仅隔离ui_framework_edit_test.dll导出。生产DLL逐项PE导出核对，不能将夹具当作生产授权入口或真实网络正例。

## 4. 本轮验证

本轮修复后离线结果：

| 构建/原始JUnit | 结果 | 范围 |
|---|---|---|
| native/fixed-controlled.xml | 2/2，10.32秒 | 新增真实发现结果/非法格式、64槽扫描引用屏障 |
| native/matching-regression.xml | 61/61，99.29秒 | RP目录/文本/read/多工具/异步/取消/缓存/关闭/存储失败/回调；Service/continue；原P4/P5/P6相关收尾；Assistant/C1；公共C/C++、两个真实DLL编译分支及静态嵌入 |
| light/light-matching.xml | 42/42，58.46秒 | 原40项相称Light投影/模型/布局/静态＋两新增受控用例 |
| asan/asan-matching.xml | 52/52，73.20秒 | 原50项相称Service/模型/目录/存储/生命周期/真实DLL/公共头/静态＋两新增用例；无ASAN报告 |

每次测试前sources和binaries清单分别记录输入及匹配产物；最终source-after/verification-summary逐项核对。正式Light宿主已构建，仅由隔离面板夹具进行程序/私有接纳模拟；未启动生产宿主或真实服务。

Native的2/2与61/61是两次独立运行，不能写成首次单次63全过。原RP Native61/61、Light40/40、ASAN50/50均是历史证据，不覆盖本轮首败。新测试采用有界轮询、明确状态及可释放屏障，没有固定等待猜测成功。

## 5. 首次失败及编排分账

| 标签 | 事实 | 处理 |
|---|---|---|
| red-core build | 新测试rc_never使用了错误execute签名，编译/WX失败 | 按既有void回调签名修正；不是产品red运行，原日志/输入保留 |
| red-bounded test | 产品发现31失败；扫描5失败，其中一项是测试将received输出写入局部accepted而原事件断言看全局run | 修正夹具关联变量；原失败不删。red-controlled仍观察到产品31＋4失败 |
| fixed-core build | 新RP适配误用不存在的局部r | 改用原工具关联的固定run slot；无公共合同变化；保留编译失败 |
| light-core build | 初次配置Mesa为空，受条件保护的面板目标未生成，ninja报告unknown target | 按原RP Light配置引用既有固定.deps/mesa-24.3.4/x64/osmesa.dll重配；未修改依赖或放宽测试 |
| 证据整理 | 事后补充文档首次按LF分隔定位遇CRLF，未取得条目 | 改用原交付精确长度和SHA-256验证前缀；未改原证据，不是产品失败 |

## 6. 自审、兼容与未验边界

本轮产品增量只有`src/ai_model.inc`和`src/ai_service.c`两处私有适配/租约；`src/ai_model_internal.h`仅增加条件编译的测试屏障字段。测试入口、CMake注册和文档独立记录。SDK0.10-dev/API10/标准10、ABI1/包1、所有公共头、v1/C2/P4/P5语义、错误数值、固定依赖、JCS/SHA-256、64槽/8MiB及原超时均不改。

自审核对：扫描租约与网络引用各有释放点；网络完成后临时引用仍保护访问，终态缓存只在归还后可回收；存储失败cleanup不依赖active，且不恢复正常派发。发现完整描述仍来自原目录，不更改上游夹具格式；投影不把非法结构解释为空目录。最终清单将验证旧头/依赖/导出及输入身份，不以检查数量替代合同。

原四组期限风险（并行graph_boundaries>20秒、ASAN控制5秒、P2五包15.04秒、面板180.44秒）独立未定位。旧frontend63、ASAN-Light中断、Light surface_input、browser20、workspace首次20.05秒/20秒门槛、Native输入首次失败保留原结论。本轮通过不能证明其根因已解决，也不默认归因环境。

RP-C、实际TLS/代理/系统凭据窗口及真实模型未验；不读取真实密钥，不操作用户数据库。browser九副作用另案，OpenAI/OAuth/AI写入未实现。真实设备、Windows来源动态正例、IME、DPI、视觉、辅助技术、物理断电和人工长时继续沿原延期清单，不冒充通过。

## 7. 保全与停止位置

原RP/P6证据只读，根三规划只追加并核对原前缀；旧SDK、参考工程、外部应用及固定依赖不改。只清理本轮新构建根的无用.obj，保留修前/最终源码、原始日志/JUnit、匹配EXE/DLL/uapp/lib/PDB及必要隔离诊断库，详preservation/cleanup/owned-data-files。脚本只处理自身新根，不调用既有recovery或历史验证脚本。

停止在两项整改后的代码评审位置。本轮两项修复及离线相称验证已完成，具备重新开展限定独立复审的条件；此为实施者结论，不自行宣称独立复审通过、正式冻结或RP-C许可。最终交付身份以新根delivery-identity.json为准。


## 8. 2026-10-31限定独立复审通过与快照冻结追记

承接本会话上一轮独立只读复审：“两项整改闭环，RP-A＋RP-B限定验收通过，匹配交付快照可以冻结。”无必须修正项；详[xuanshu-rpab-independent-review](xuanshu-rpab-independent-review.md)。本轮仅核对390源码/14交付/114最终产物/36修前归档及清单身份匹配后记录，没有重新开展完整复审、构建或测试，不修改原证据。

冻结delivery-identity SHA-256为B2F9941B437964071D775345F3390C847A5A6ADC3190618CC500EAB93197DAA0。入口491与事后3份文档、Native新增2/2与相称61/61两次运行、Light42/42/ASAN52/52已有证据，以及修前31/4失败均保持上述来源表述。历史风险、真实服务和延期人工项目未解决/未完成，不由冻结记录消除。

本节及本轮另7份文档是冻结追记与RP-C规格增量，原delivery-files中的授权文档会产生预期差异；原source-after/binary-files和证据清单不更新。没有commit/tag/release、真实Provider/密钥读取或用户库操作。RP-C方案确认见[验证计划](../xuanshu-rpc-validation-plan.md)，准备实施和实际调用分别仍需单独授权。
