# PB-3开发证据及限定独立复审追记

## 2026-11-11限定独立复审及冻结追记

PB-3限定验收通过，匹配快照可以冻结。冻结记录已落盘，必须修正项为空。原匹配identity为4A892228066D04EB09972594509892AF3B960CF32569DE33A125F11BEF611A42，原清单、源码/文档副本及历史证据保持。详[限定独立复审与冻结](xuanshu-page-reading-independent-review.md)。本次只读核对未重跑测试、构建或生产程序，343条最终case和程序/相关源码匹配，既有运行及首败仍分别记录。

[PB3-REAL-20261111-6c4f82具体授权包](../../build/xuanshu-page-reading-real-preparation-20261111-6c4f82/authorization-package.md)已准备，SHA256为DE37BAC245BB0734161DD00583A118D96DFE3AE2C43630D6A366E3B803D0E8B6。仅2026-11-11北京时间、程序及官方依据不变时有效。12份当前公开官方依据逐字节一致，三份拟议请求本地计量556/899/1236 token、组件均exit0，每轮20000输入/4096输出，初始及全部续轮最多3次、本人可信确认起10分钟、USD0.10停止阈值，三次保守预留USD0.0327456。包保持Proposed，执行目录未创建、策略未激活，等待一次独立集中批准及实际逐任务可信确认。

默认start-readonly.ps1和policy-proposed.json仍保持未核价锁停，具体包的日期/价格依据与候选默认模板分别记录。新监督命令只有静态审阅，尚未动态执行。本次没有真实凭据访问或模型请求，历史PB-2及BR-2批准、库和额度保持原已消费状态。

ASAN首轮五项摘要失败和两次30秒超时、首轮摘要EXE PDB缺失、96主样本与另10旁文件（6非空）的边界、BR1-KI-001及计量任意卡死风险均保留。后续通过支持最终逐项覆盖，未知期限触发因素仍待定位。临时页面控件测试保持取消。

## 原开发验收记录，运行及首败保持

# PB-3公开网页正文阅读开发验收

2026-11-11。实际公开网页阅读场景、最小保护接入、开发自审及相称离线验证完成。候选根build/xuanshu-page-reading-20261111-a18d74，delivery-identity.json SHA256为4A892228066D04EB09972594509892AF3B960CF32569DE33A125F11BEF611A42。当前具备限定独立复审条件，真实验证准备保持Proposed。

## 前置身份与本轮增量

前置PB-2冻结identity为46813CE1DAB1C2DD5620D58A5FFC90306076AFCF26AEDED2BEC620F5BA1D7496，入口时537份源码、241项程序及链接/调试产物匹配，原快照和最近真实证据保持。原147字节交接单、PID28300 exit0、1任务/2消息/31事件/2工具及已消费批准按原记录保留，本轮未重开旧库或复用其凭据引用。

本轮546份最终源码，17项代码/样本增量。其中产品仅src/ai_business_validation.inc、src/ai_business_restore.inc、src/ai_validation.inc和src/ai_model.inc四份私有场景/恢复/任务路由，复用同一Service、Provider、目录、read执行及存储。必要CMake/共享夹具、四份新测试及五份网页样本文件分别列于incremental-files.json。已有Git差异按入口快照保留。

源码、清单、195项程序及链接/调试产物、测试前源码/程序清单和原始日志/JUnit匹配。final-test-match.json逐项核对343个最终case，包括Light168、ASAN168及Native7。生产两配置358项导出与PB-2名称和顺序一致，私有入口保持隔离测试构建范围。

## 实际网页与结果依据

选用SQLite官方Zero-Configuration，原始HTML4209字节，SHA256为D8ECCE18365870C07E60802A76585B9FDE5CCD53714F579A3C8C5BE4E80387EA。官方copyright声明文档属于公共领域，原始网页、许可页、取得时间和HTTP响应分别保存。本轮运行的是原始网页完整隔离副本，包内browser:start使用真实browser文档解析器和browser.body。[页面](https://www.sqlite.org/zeroconf.html)和[许可依据](https://www.sqlite.org/copyright.html)。

独立全文基准为847 UTF-8字节，SHA256为A73C755A8328BD65D7085506C145EACD12760DAA3634A5DEAF42E750D24B773D。正文包含作者导航文本及三段技术内容，script、表单值等按原合同排除，CSS隐藏作者文字可能保留。框架revision是既有不透明状态版本，内容另以全文和哈希核对。真实站点导航、站点样式/图片和任意恶意网页安全性维持原范围。

精确事实基准为无需安装/setup、独立server进程、管理员创建实例或配置文件；原文没有具体待办。模型替身负责回放预期摘要，自动断言实际结果控件、任务输入、白名单、完整请求/工具续轮、固定目标、正文类型/bytes/complete/semantics及保存内容。真实摘要质量留待下一次授权验证。

生产DLL公开read用例先读snapshot状态等待文档就绪，再通过公开browser.body验证847字节全文和revision。正式宿主页完整流程使用隔离DLL、模型/凭据替身及可信事件夹具；对应任务和实际序列化白名单只有browser.body、body/revision。两条路径均索引其实际程序，合成普通输入及隔离可信事件适用于各自测试路径。

## 场景、确认与生命周期

固定输入为“PB-3：阅读SQLite《Zero-Configuration》公开网页隔离副本，概括配置特性、关键事实及待办；原文没有待办时明确说明，不导航或修改。”，history=[]，唯一browser.view，public-sqlite-zeroconf-v1。旧snapshot和合成正文输入保持互斥，未知场景、legacy计量、错能力/字段/资源/历史均拒绝。开启business_scene和任务validation_scene在READY前严格交叉恢复，完整恢复费用及计数后锁停。

正式界面通过草稿、结构化选择、本地保存、持久化接纳、范围和完整详情、实际read、回复与最终保存；任务、配置及未提交草稿保持分离。两实例切换不改原run/目标，关闭重开保持进程host_epoch、生成新instance/open_epoch，旧确认拒绝；revision改变同样拒绝。范围隐藏、PENDING解绑、旧代次/投影失败及恢复一次确认、重复拒绝按匹配回归保持。

初始和全部续轮共用3次硬门，第4次在传输入口前拒绝。费用、usage缺失、unknown、期限、计量失败和存储失败停止保持。原引用、回调重入、cleanup-only、取消、唯一终态、安全关闭和只恢复记录行为通过相称路径。纯文本反例保留指令性文字作为不可信数据，越权browser.navigate模型工具请求被拒绝。

## 修前与最终运行

| 独立运行 | 结果及适用范围 |
| --- | --- |
| Light red-controlled | 2例，公开生产read通过，新场景READY接纳前提1项失败，0任务/请求/工具 |
| Light fixed-direct | 早期37/37，后续共享夹具补项后仅作早期证据 |
| Light closure-final | 167/168，新reopen夹具1项失败，最终覆盖取其中72项 |
| Light direct-final | 另次79/79，替代对应新阅读及旧正文策略用例 |
| Light summary-final | 另次17/17，替代摘要测试程序对应项 |
| ASAN closure-final | 163/168，全部42项新阅读用例通过，最终覆盖取其中151项 |
| ASAN summary-final | 另次17/17，替代摘要测试程序对应项 |
| Native closure | 7/7，真实DLL、静态嵌入及公共C/C++等 |
| 清理后生产公开read | Light及ASAN分别另次1/1 |

最终逐项Light168、ASAN168、Native7，各次运行和首败分账。两条各50000字节ASCII/UTF-8历史及完整112/48页实际文本检查保持，最终ASAN对应22.86秒、23.08秒，cache-page18.90秒，原30秒门槛保持。

## 首败、自审整改和期限限制

修前新场景被旧运行时拒绝，生产read已能读真实网页，这个差异由场景路由最小接入解决。同期源码、失败程序及公开read通过对照按角色保留。首次红库保存，初始失败瞬时字节哈希未额外独立捕获，当前保全身份见owned-data-files.json。

新增重复提案夹具先误用不存在的ai_run_json成员，C2039及后继C2168/C2198保留；改为已有ui_ai_get_run后，局部snapshot遮蔽全局触发C4459/C2220，随后按现有接口及清晰变量名修正。这两次编译没有对应新链接失败程序，失败源及上下文单独标注。

reopen首败错误要求进程级host_epoch随资源重开变化。依据原能力合同修正为host保持、open变化，保留旧确认拒绝、0请求及安全关闭断言，实际前后UUID记录在原始日志。产品路径未因此修改。

ASAN首轮摘要五项失败独立保存。long在脚本预算中断后未取得绑定，clear-back/append-back在显示未完成时比较失败；UTF-8及cache-page分别30.0669/30.0793秒超时。原始日志、同期源码、exe/DLL/UApp、自有库和旁文件保留，首轮测试EXE的PDB未独立归档。没有AddressSanitizer内存错误报告。

隔离缓存夹具改为最多3次同步显示回放，明确等待目标故障或显示完成，期间不泵Service，每次未完成均检查资格失效、旧确认拒绝及零请求。原全文、页码、任务、一次正常确认及重复拒绝断言保持。产品页面、50ms预算和原测试超时均保持。最终17项通过提供功能覆盖，首次预算触发因素及两个期限根因仍未定位，作为独立期限/插桩风险保留，后续通过没有解释全部历史期限问题。

取证脚本的缩进匹配、JUnit1024字节输出截断、Windows/UTF-8混合原始日志及Ninja换行解析错误分别保留并修正。后续自动归档原始CTest字节日志，并完整保留JUnit输出，运行时预算及测试时限保持。详driver-errors.md及各失败归档。

## 持久化与关后证据

Light和ASAN各48份明确记录的阅读主样本，共96份，均先确认测试完成及无旁文件，再以mode=ro&immutable=1、query_only读取，前后哈希一致。48条成功body工具结果逐值符合847字节全文、complete=true、dom-body-v1及对应固定revision，16组损坏恢复副本的确认/请求/占用/传输入口增量为0。种子用过隔离确认和模型替身，恢复零增量按恢复阶段记录。

代表性Light discovery库为light/reading-discovery/body-policy-{D5B2FB08-71A5-490D-82B4-EAC63F6F32DF}.db，1任务、2消息、31事件、2工具、唯一completed。数据库SHA256为D22B4E629DF5436AFB9EF0EB661C0829CD72C93D3C675CFAC58D90CF00219B5F。该记录含3个隔离模型请求和发现/正文工具事实，实际正文与固定任务、模型替身摘要及保存逐值关联。

另10份旁文件独立保留，其中6份非空，96份主样本的无旁文件结论不扩展到其他库。观察器没有checkpoint、Service重开或旧数据库访问。closed-reading-light.json、closed-reading-asan-light.json、owned-data-files.json及retained-sidecars.json给出具体范围。

## 交付、自审及停止位置

生产文件、默认锁停入口及必要依赖见[交付清单](../xuanshu-page-reading-delivery-manifest.md)，[使用入口](../xuanshu-page-reading-entry.md)和[真实验证准备](../xuanshu-page-reading-real-proposal.md)形成单一可定位路径。三份拟议请求纯本地计量为1608/2927/4245字节及556/899/1236输入token，组件exit0；执行前真实模型、协议、价格、完整计量和可靠费用上界仍需核实。

代码自审核对场景解析、旧策略互斥、JSON/字符串所有权、READY前恢复、原发送门及回调/引用路径，限定范围必须修正项已清零。保留SDK0.10-dev/API10/标准10、ABI1/包1，公开头/错误值/DDL/来源政策/C2/P4/P5/依赖及50ms、8MiB JS、128KiB模型、2MiB宿主、8MiB Service、256KiB单包和原数据范围。清理760项本轮编译中间项及包装暂存，433260261字节，必要程序、源码、日志、首败、自有库及旁文件保留；依赖VALID记录对应清理前时点。

真实来源完整分类、IME/DPI、辅助技术、人工长时、断电和browser九项副作用维持原边界。BR1-KI-001保持未解决、用户接受非阻断，临时控件测试维持取消。前置PB-2真实批准已消费，本轮没有真实凭据读取、Provider/账户调用、真实模型请求或人工验收。

一个实际网页阅读场景的只读闭环及离线验证完成，匹配候选和下一次真实验证待授权包已准备，具备限定独立复审条件。当前停在代码复审和具体真实授权准备位置。


固定计量组件任意卡死时的有界结束仍未证明，按原合同保留PENDING、进程和引用，等待真实退出。三次纯本地计量exit0适用于正常路径。PB-1初始化20.03秒、历史30.08/30.06/30.11秒、复制error5及其他首败继续独立保留。
