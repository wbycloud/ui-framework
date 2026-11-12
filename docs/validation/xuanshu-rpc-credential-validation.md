# RPC-K1凭据与验证体验限定整改记录

> 2026-11-02后续：上一轮限定独立复审已通过，匹配RPC-K1快照可以冻结、无必须修正项；本轮仅核对身份并[追记冻结](xuanshu-rpc-credential-independent-review.md)，没有重跑测试或重新全面复审。非阻断文案建议不改程序。用户另授权[仅T2/T3独立补测](xuanshu-rpc-real-service-validation-t2t3.md)，不重复T1、不重置旧R/期限。下文无独立复审/待启动等为当时历史状态。

> 本次补测已结束：凭据再次成功，直接T2真实返回page=1而仅0适配门拒绝，发现包装Schema未公布该限制；工具0、M1/R1/S1/C1/U0，唯一failed终态已保存，T3停止未验。无代码修改或重试，RPC-K1限定冻结记录不扩大为RP-C成功；具体缺口与后续最小建议见补测记录。

2026-11-02。最终状态：限定整改、离线自动验证及自审完成；新轮实际原生录入、保存/本地读取和T1生产文本完成。T2/T3因validation_deadline未执行，程序正常关闭并完成关后只读对账。不署名。本记录不是独立复审通过或新快照正式冻结，T1成功也不等于RP-C整轮通过。

## 1. 基线及问题归类纠正

实际codex/p1-process-boundaries / e0eca03db25930be55bdb45477027d1793a61904，未提交。新证据根build/xuanshu-rpc-credentials-20261102-e48d73，baseline.json自动保存408项本轮范围（399源码及9相关文档/根规划），不称全工作区、也不归并HEAD全部差异。本轮入口核对原准备399源码/122最终/27red/8依赖及旧真实根28份证据全部匹配。历史修改、ignored规划、冻结SDK和参考工程保持；不使用子代理/computer use。

用户明确纠正：上轮没有看到可实际输入Key的界面。聊天“已录入”是操作反馈，不能作为软件保存/本地读取证据；不能归因用户未配置、Key无效或厂商拒绝。旧轮M1/R1/S0/C0/U0、工具0、credential_unavailable及唯一失败终态原样保留，不退款或覆写。旧记录中聊天反馈及截图事实作为历史保留，本条纠正其解释。

## 2. 核实及修前受控结果

原probe每15ms无条件SetWindowText，选择和滚动不稳定已源码核实。新专用ui_rpc_probe_ui在生产链接DLL/公开memory夹具上检查绑定和真实控件选区；修前Key绑定非零，选区保持断言失败。还发现没有凭据保存/本地读取证据时窗口会提交T1。没有用固定指针或私有传输替身冒充生产绑定。

原am_action调用通用CredUI但不保存其返回类别/系统错误；后台job3没有分别确认写凭据、配置保存、读回，配置ready不证明Key可用，host_status还会被确认操作覆盖。旧弹窗未出现的具体Windows原因仍无法追溯；不能由当前夹具重建原本未捕获的错误。新ck-state明确要求独立凭据状态，修前缺失失败，属于可观察性缺口而非已证明Key读取根因。

red-controlled/native两项均有有界断言失败，无异常终止：ck-state两项缺状态断言；probe三项缺状态/选区/未配置T1防误操作断言。原20秒超时及真实预算不改。red/native源码、程序、开测身份、原始日志和JUnit保留，此后不重建red目录。

自审追加selfreview-capture：在可见但屏外的真实标准按钮上，程序消息只用于负例和控件收尾验证，确认DOWN取得捕获、UP未释放，1项GetCapture断言失败。原来源检查仍拒绝注入、没有打开Key窗/发请求。源码因抑制默认UP命令漏掉捕获收尾，最小增加ReleaseCapture（仅自己的捕获）及BM_SETSTATE复位，再执行既有资格检查后的动作。嵌套WM_CAPTURECHANGED仍由原方法租约覆盖，未开放程序消息授权。修前程序/源码/日志/JUnit另存，不改写red-controlled证据。该机制缺陷已证实，但旧CredUI实际返回仍不可追溯，不能称唯一根因。

## 3. 最小私有实现

先冻结[RPC-K1](../xuanshu-rpc-credential-closeout-contract.md)，然后增加私有原生模态窗和分阶段凭据状态。按钮仍由原bind_button/light的一次性可信输入消费进入，未提供新公开confirm/token/secret接口。窗内明确API Key标签、遮罩、保存/取消，不枚举已有凭据或预填，不再要求猜用户名/登录密码。外层方法租约覆盖模态消息循环；返回重查配置代次/关闭/任务条件，stale不提交保存。

后台同ref的CredWrite、配置文件保存和CredRead校验分别确认，失败点立即保存错误码。没有已知错误码时为null，不伪装0成功。临时明文及读回数据及时清零，页面/数据库/日志/诊断只有ref/阶段/布尔/错误码。模型ready保留配置语义；新增credential JSON对象独立保留阶段、保存/读回结果及saved_revision，不被其他host_status覆盖。网络读失败保留原credential_unavailable任务错误并增加本地细分状态，不改变HTTP/外发授权合同。

probe显示真实中文步骤、绑定结果及凭据阶段；没有本轮保存/读回证据时不接纳任务。文本未变化不刷新，变化时恢复选区/滚动；复制诊断仅取脱敏UI快照，独立于授权/提交。程序标题不再声称NOT authorized。私有copy sink/prompt/store/failure driver仅隔离测试构建，生产无新增私有导出。

## 4. 验证状态与边界

中间受控13/13通过：未配置、提交后尚未保存、保存及同ref读回、取消、窗口/写入/配置/读回失败、配置变化/关闭重入、实际原生窗控件/遮罩及合成保存取消、生产链接probe稳定复制/无凭据拒绝T1。合成原生窗驱动由私有夹具接纳，不是物理输入或生产可信源正例；Windows凭据后台本轮自动测试用内存替身，不读取任何已有用户凭据。物理入口和真实Windows保存/读取留给离线完成后的用户操作。

自审保持原Blob全部字符检查（含拒绝内嵌NUL），区分未知系统错误与0成功，输入窗口在配置/关闭条件变化时不保存；复制及系统消息不签发授权。后续最终Native/Light/ASAN相称回归及实际身份另追加，不将中间通过冒充最终匹配证据。原失败、历史期限/frontend/输入/browser风险和延期清单不覆盖、不默认归因环境。

自审另补ck-request-read，发现新增诊断在普通获准请求的本地读取成功后没有更新代次，2项local_readable/阶段断言失败；业务请求已正常完成，没有重复执行。修复仅把已证明的读取结果与当前代次关联；本进程未保存时标local_readable且不声称credential_saved。失败原始材料独立保存。

中间Native65/65、Light67/67，以及加入诊断后Native66/66、Light68/68、ASAN66/66均为不同代码时点；最终以捕获修复后的closure开测身份/JUnit为准，不相加为单次或冒充最终源匹配。所有重核因本轮私有源码/诊断修正需要，不重跑无关历史全量。原20秒/额度不变。

旧执行驱动多阶段反复导出VS环境时，原工具输出出现“The input line is too long / The syntax of the command is incorrect”；实际build/CTest仍用已初始化环境完成，ASAN flags及runtime导入已核对。此提示只有原工具输出，verification-driver-notes.json为后续如实追记，不冒充同期自动stderr日志。随后新驱动改为按需、仅导出6个工具链变量并严格检查设置退出码，最终closure重新验证；不枚举导出环境Key，不改变编译器/依赖或生产合同。原驱动副本保留。

## 5. 真实新轮及停止位置

用户已另授权本轮离线完成后的新真实验证；不是旧轮退款/重置继续。新匹配程序、库/config/ref和准确步骤须先交付，Key只由用户在原生窗录入，实际保存/本地可读确认后再T1、T2、T3逐项确认。三合成任务/history=[]、5按1/3/1、USD0.10本地停止/usage未知停、网络10/30/30/120及首次确认10分钟原样保持。

本节冻结设计时尚无新API/Key读取结果；后续经用户原生操作的实际覆盖见第7节，不能把该历史状态延用为最终结论。没有新增Provider/AI写入、来源放宽、公开布局/错误/版本/依赖/额度变化、Git提交/推送/远程工作流、benchmark或recovery。

## 6. 最终离线结果与交付

捕获修复后同一最终源码的closure三次运行：

| 配置 | 结果 | 时间 | 原始JUnit |
|---|---|---:|---|
| Native | 66/66 | 94.78秒 | native/closure.xml |
| Light | 68/68 | 100.54秒 | light/closure.xml |
| ASAN | 66/66 | 99.19秒 | asan/closure.xml |

覆盖13个ck用例、生产链接probe选区/复制及注入拒绝/捕获释放、原RP目录/文本/read/异步DLL/取消/队列/存储checkpoint/缓存/配额费用/重开/旧ID重入、Service控制及关闭、公共C/C++头、Assistant、真实DLL两工作区编译分支、生产/静态及Light投影/收面板/生命周期。没有Not Run；中间通过及三类有界失败分别保留，不把全部运行数量拼成首次单次结果。最终严格驱动未再重复VS环境设置。

本轮代码/测试/构建增量10项：CMakeLists.txt，src的ai_model.inc/ai_model_internal.h/ai_model_net.inc及新增ai_model_key.inc；tools的xuanshu_rpc_probe.c及新增xuanshu_rpc_probe_ui.inc；tests的ai_model.c及新增ai_credential_cases.inc/rpc_probe_ui.c。其余为11份必要设计/计划/记录/根规划，不修改公开头或应用业务实现。

Light匹配程序SHA-256：xuanshu_rpc_probe.exe `307E8E783D167CC38AA151C6C5275A420D7CDF0D2144EF2D930F084D684A9806`，ui_framework.dll `8B2DCB82FA7F72F5E6974FFAFD427DA71CC706B336D09D00C97D854DA0F7F1E0`，rpc_memory.uapp `04FAA57382D17BBF81463B34FF6A8199E9524D0317819C7B2565852248A3D33F`。source-after/binary-files/exports/preservation及delivery-identity关联源、程序和开测证据，原冻结程序与旧真实根保持。仅清理本轮新根无用obj，保留各失败同期源码/匹配程序/PDB、JUnit/日志和必要隔离数据。

结论为“RPC-K1凭据与验证体验限定整改开发及离线验收通过”，不是独立验收或整轮真实DeepSeek验证通过。实际用户原生输入、Windows存储及同ref本地读回证据已在[第二轮记录](xuanshu-rpc-real-service-validation-round2.md)追加；不将自动夹具冒充真实设备正例。

## 7. 实际凭据与复制验证收尾

新匹配程序PID76012启动后，用户确实看到单一遮罩API Key输入窗并保存；通过“复制诊断”取得window_opened/submitted/credential_saved/config_saved/local_readable=true、四阶段错误码0、revision与saved_revision均3、pending=false。没有让聊天“已录入”替代保存确认。复制诊断随后可连续提供等待外发、T1终态及期限停止状态，未触发网络或授权。此为本次实际路径正例，不外推Windows来源分类、IME/DPI或全部人工体验。

T1生产请求保存准确回复及唯一completed终态，关后库核对M1/R1/S1/C1/U0、tools=0、usage467输入/6输出，参考费用USD0.0001473。准备T2时实际Policy validation_deadline，未执行T2/T3，未延长、返还或另开会话续跑。用户正常关窗，关后只读库哈希不变、无WAL/SHM，软件凭据/复制整改已可使用；整轮RP-C未完成。旧弹窗实际Windows返回仍缺历史捕获，不把确认的捕获收尾缺陷称为唯一旧根因。

启动后的运行文档追加由real/delivery-files.json单列，不覆盖离线开测身份；最终real/delivery-identity.json关联新轮证据和原离线identity。源码403项及139最终程序保持匹配，原122最终/27red及旧真实28项证据不变；新开发失败118项归档、日志/JUnit、隔离凭据和库保留。没有独立复审、正式冻结或后续真实调用授权。
