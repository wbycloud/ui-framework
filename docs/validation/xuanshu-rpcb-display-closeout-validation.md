# RPCB-1预览接纳门与启动诊断限定整改

2026-11-02。开发/离线验证记录，不是独立复审或正式冻结；无真实请求、真实密钥读取或真实补测。

## 基线及改动归属

HEAD e0eca03db25930be55bdb45477027d1793a61904，分支codex/p1-process-boundaries。旧批次identity341F240F5EB1775FEB8A7B539446327DFF71AE91C24EDD7F1251638EBA46EC58、启动最终755E10D36D7AA0AFA31099722FCF5BB5C1E6B3373AB4B04E3B027F2A7F94A814核对匹配；旧文件及失败记录不改。本轮新根build/xuanshu-rpcb-display-20261102-64c8a2，入口范围基线只记录启动器和部分上下文，不冒充完整Git工作区基线。CMake入口未单独捕获，事后核对其原始前缀与旧source-final字节匹配，见baseline-supplement，不冒充入口自动归档。

产品修改仅tools/xuanshu_rpc_batch.c；新增tests/rpc_batch_ui.c及CMake独立测试注册。全部运行时源码、33份既有公共头、固定依赖、DLL和rpc_memory.uapp保持；新独立构建只编启动器和UI隔离测试，链接已验证的生产导入库及JSON库，不重建运行时。源码/二进制及日志身份由本轮清单记录，不将HEAD全部差异归为本轮。

## 两项修复

1. 全部必需控件即时检查；投影返回明确成功/失败。需要更新或失败时禁用开始并清空显示摘要；同摘要同完整文本的稳定显示不反复禁用，验证当前preview完整对象及JCS指纹、序列化/分配、两次严格UTF-8转换、SetWindowText返回和文本完整长度后，才允许对应hash绑定与开始。旧hash、占位、部分文字均不能通过；失败保持禁用并诊断，恢复后可重新显示。摘要改变解绑旧入口；关闭中不再启用。仅防止启动器自身嵌套刷新，原可信事件链及Service重入合同保持。
2. 启动诊断区分win32/ui_status/launcher，未知code=null、code_known=false，保留负的框架状态。补齐转换/控件/计时器/框架/workspace/Service初始化分支。未创建Service之前失败立即释放窗口；Service已创建时继续原poll关闭，不等待或强杀worker。无公开布局、错误数值、数据范围、来源、版本/依赖/限额或运行时语义变化。

## 修前和自审证据

red.log及red/red.xml：8项用例全部有界失败，19项断言，说明开始启用早于投影或投影失败仍可绑定。red目录保留同期launcher/test源码及匹配exe/DLL。没有动态真实外发、崩溃或越界业务复现。

首个ctest命令因PATH未找到程序而未执行，随后使用VS内绝对路径；工具错误未自动存原文件，此限制如实保留。green-first再次8项失败是测试包含.c的依赖未触发重编，误跑旧测试程序；增加OBJECT_DEPENDS并重编后green8/8，未弱化断言。CMake注册明确声明启动器源码依赖，避免后续同类盲点。

自审发现框架负状态转unsigned显示，保留selfreview-signed同期源码、匹配程序及23项JUnit；纠正为有符号long long。最终diagnostic-check.json直接解析实际stderr，ui_status code=-5而非4294967291，Win32 code8/3/183独立验证。

## 新验证及证明边界

| 本轮证据 | 结果 | 证明范围 |
|---|---|---|
| bin/final.xml | 23/23 | 17项UI/诊断＋6项原批次夹具，符号修正前阶段 |
| bin/final-projection.xml | 17/17 | 最终启动器源码的全部投影/初始化/诊断用例，含捕获保持 |
| bin/final-ui.xml | 17/17 | signed修正后、捕获修正前的中间运行，原结果保留 |
| bin-asan/asan.xml | 17/17 | 最终UI测试及其包含的启动器代码启用ASAN；链接的既有生产DLL未重新插桩，不冒充全运行时ASAN重跑 |
| production-startup-final.xml | 3/3 | 最终匹配生产exe仅--startup-check，中文空格路径/缺失父目录/已存在目录；无Service/凭据/网络 |
| production-startup.xml | 3/3 | 早期生产启动检查，原阶段和结果保留 |

17项覆盖正常完整显示及重复刷新、内存/序列化/首次及第二次转换/显示失败、部分文本、旧摘要不匹配、渲染中重入/关闭、失效预览控件、四种必需控件创建失败、框架初始化失败和新摘要失败后的恢复。测试使用真实离线Win32控件及启动器源文件，模型/Service状态为隔离外观，不创建真正Service或访问凭据/网络。失败帧不会建立开始绑定，旧绑定保持禁用；不能将这种隔离测试称为真实人工来源或端到端Service故障注入。

六项core-bc-unconfirmed/success/reentry/backpressure/limit/restart使用原匹配的隔离运行时/传输和实际workspace/DLL，在本轮新数据目录运行，分别证明未确认零接纳/消费/真实发送、正常一次接纳/重复拒绝、实际10/r1读结果与保存、回调引用、原64通知背压、总4/0-3-1和封闭重开/自动收尾。生产网络S为0，模拟计数独立。它们与UI禁止绑定/禁用门的证据互补，不冒充所有故障均以真实硬件执行。

原Native96/96及17/17、Light99/99、ASAN97/97是历史充分证据；运行时字节不变，不机械重跑。生产DLL哈希等于旧Light，复用其导出清单无fixture/私有确认/传输导出；生产与测试exe均无PE导出目录。私有Win32/内存/Service外观只在测试源文件中，未编入生产exe或DLL。

## 保留结论和停止位置

之前独立复审的“暂不通过、需先整改”没有被改写为通过。本轮两项整改开发与离线验证完成，具备再次限定独立复审条件，不自行宣称独立复审通过或正式冻结。不启动真实补测，不重跑T1/T2/T3、不读取Key或用户数据库。

既有T2三次真实响应、精确10/r1/framework revision保存及T3真实在途取消/本地退出全部保留；M/R/S/C/U4/4/4/3/1、fee_unknown和保留估算不清除。账户3865与已知3382相差483仍独立待核对，不归给T3。历史期限/frontend/输入/browser首败、browser九副作用及全部延期人工不改变，不默认归因环境。

### 稳定刷新自审补充

自审增加保持鼠标捕获的断言，发现无条件每15ms禁用/重启按钮会丢失捕获；capture-red单项1断言失败及同期程序/源码保留在selfreview-capture。修正为同hash且同完整文本已显示时保留按钮状态，仅更新/失败/摘要失效/关闭时先禁用；不放宽显示检查或可信输入。最终bin/final-projection.xml17/17及最终ASAN17/17包含捕获保持、关闭和显示失败回归；此前final-ui是符号修正后、捕获修正前的阶段证据，不冒充最终源码运行。原6项core运行时路径未受此UI局部修正影响，沿用本轮final.xml中的通过证据。production-startup-final.xml为最终生产exe3/3零Service/凭据/网络检查。

本轮仅清理8个新根无用途obj；原始首败、最终及自审程序/源码、日志/JUnit、旧真实库/凭据和所有历史材料保留。

## 冻结落盘追记（2026-11-02）

承接上一轮独立只读复审：两项整改闭环，限定验收通过，匹配交付快照可以冻结，无必须修正项，无需追加T1/T2/T3。详[独立复审记录](xuanshu-rpcb-display-independent-review.md)及[阶段收尾](../xuanshu-native-stage-closeout.md)。本轮仅身份核对与文档，不重新测试或重做完整独立复审；之前“待复审/暂不通过”的阶段事实保留。

本轮已统一表格最终日志引用，旧final-ui和production-startup的中间结果未删除。冻结identity仍为41D15CFF8A2E0F639D02B6748C0F90AB95E9D6B4CAB7368221CF357BA4E8FF7C；本文件原离线SHA为5729D79FEEA9D05E7C7A619CD9A75FD92BBA513F39E600DC8D7F2D5997316499，原字节在build冻结根source-final副本保留。本轮更新是新文档状态，不能将当前哈希冒充原离线交付。原build清单、程序、日志/JUnit和失败材料不更新。冻结不执行commit/tag/release，下一批建议仍Proposed，当前等待授权。
