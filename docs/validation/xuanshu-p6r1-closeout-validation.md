# P6R-1 三项限定整改验收记录

2026-10-28。本轮是三项源码窗口的实现收口及自审，停止在再次独立复审之前；不是独立验收通过或正式冻结。

## 1. 基线、归属及保护

实际分支 `codex/p1-process-boundaries`，HEAD `e0eca03db25930be55bdb45477027d1793a61904`，未提交。新证据根 `build/xuanshu-p6r1-closeout-20261028-91c4e7`，入口17470项Git可见文件哈希。此前P2—P6已有修改不归本轮；以baseline、delivery-files和incremental-source-diff核对增量。原P6根 `build/xuanshu-p6-20261027-4a92d1` 保持只读，原373源码及120程序另核对。参考工程D:/XuanShu未写入；沿用Git读取Permission denied记录，不声称本轮重新核验其HEAD，不扩权。

保持SDK0.10-dev/API10/标准10、ABI1/包1、公开布局、v1只读语义、冻结C2/P4/P5、固定依赖、JCS/SHA-256及所有生产额度。新增字段仅在不透明Coordinator私有结构，仍计入原Service预算。没有新执行器、生产测试授权、来源政策变化、用户库操作或新功能。未修改原有测试断言、超时及数据范围。

## 2. 三项核实及最小实现

### 2.1 Coordinator引用移交

原ag_queue增加run.refs，ai_graph_io在发布done时却减回0，ai_graph_ui尚未消费。真实64槽压力下原root_refs=0、原ID已被新提交回收；测试立即有界退出，不继续危险解引用，未制造或宣称崩溃。

现在私有run_id/run_ref随排队、worker、done一起移交。worker不再释放该引用；UI核对slot、稳定ID及graph后消费，再恰好释放一次。身份异常直接记录UNAVAILABLE/错误，不让UI等待通知队列空间。job1/2/5无run引用；成功、失败和关闭沿同一消费门。原ai_evict额度和64槽不变。

cr-pressure先经真实存储建立64个普通记录，建立等待关联答复的老图，再重新读取63个已淘汰身份，使它们的缓存order晚于图。生产64槽实际满后暂停UI消费done4，再提交新任务迫使真实淘汰；要求图身份及唯一引用保留，消费后refs=0、finished=true，再poll不重复释放，后续任务及关闭正常。没有降低生产缓存额度。首版夹具128预热及较宽的等待条件超时，保留首次20.04秒结果；最终用最小真实64槽置换建立同一个边界，断言并未退化成缓存未满的固定指针演示。

### 2.2 存储失败后的cleanup-only

原worker清active，而图UI按active/c.failed过滤，独立接纳事务失败时c.failed仍为0。受控在子意图、真实DLL执行及最终确认位置暂停消费、等待失败worker退出及active清除后，旧代码的live/holding仍为1，5秒排空断言失败。是有界收尾失败，未宣称永久死锁或数据损坏。

新增ae_unavailable，独立于active、根终态及正常调度：只取消、消费已接纳阶段完成、记录实际事务/undo/revision事实，再经原finish/release等待应用executor_exited及回调退栈。没有新begin/execute/commit/rollback或正常存储推进；存储job只消费失败结果，不重试。UNKNOWN资源门及原未确认意图保留，释放执行租约不等于解除UNKNOWN。

同范围自审补充：只读call的应用已关闭后，旧ag_read仍保留call ID；受控cr-read-close一条排空断言失败。现在关闭中的既有read可取消/查询/forget；若实例已安全移除，workspace原能力/回调静默门证明执行者已退出，仅清理失效read关联，不接纳新调用。分别验证关闭中和已移除两条路径，不引入新执行器。

三个编辑窗口均要求存储失败后提交计数不再增加、无新业务执行、worker与外部回调未退出时destroy=PENDING。应用确认两者退出后holding/live/job/busy归零；意图及执行窗口实际值10，已提交窗口值42且committed保留，仍为UNKNOWN，不把结果记为未修改。原UNKNOWN/重登记/forget/重启合同由相称回归复核。

### 2.3 COMMIT与checkpoint后续推进

原ai_commit在COMMIT成功后PASSIVE失败仍返回成功，这是已提交事实；缺口是ai_begin未拒绝不可用存储，Provider同轮会写入下一终态。真实第32次提交精确落在running记录，旧实现多出一次提交及终态，三条断言失败。

保持ai_commit成功返回和BUSY/LOCKED分支；ai_begin拒绝store_failed。Provider在工具请求/绑定/结果、关联答复及running提交后的后继阶段重查；Coordinator停止正常排队；子编辑存储入口和应用阶段派发前重查。数据库I/O仍仅后台，UI只查状态，不join。

除running外，新增第32次提交为图快照、第64次提交为子意图的精确故障。全部使用真实前置提交，不调整32周期。证明前置running/graph/intent及单次消费确已提交，后继业务未开始，无重复终态；关闭后专用子进程冷重开只恢复记录、无自动执行。子意图故障还要求io_ok=1，防止把COMMIT成功误记成回滚。

## 3. 新受控用例与断言

| 用例 | 必须成立的证据 |
| --- | --- |
| cr-pressure | 64实际槽、待消费refs=1、ID不变、消费后refs=0且不重复 |
| cr-checkpoint | running实际提交，后续终态0，提交计数只增加2，冷重开不执行 |
| cr-checkpoint-graph | 图快照已提交，未创建子编辑，无业务修改，消费引用归零 |
| cr-checkpoint-child | 第64次真实提交是意图；io_ok=1、消费1、started=0、业务值10 |
| cr-intent | 独立存储失败后意图完成可排空，不开始业务，UNKNOWN保留 |
| cr-execute | DLL自有worker/外部回调在途不卸载；各自退出后才能收齐，非阻塞关闭 |
| cr-final | 业务值42、已提交事实保留，结果未确认仍UNKNOWN、无后续提交 |
| cr-read-close | 应用安全移除后旧read关联排空，其他实例值10 |
| cr-read-closing | 尚可查询的关闭中实例只收取既有read，不派发新read |

全部复用现有隔离测试DLL和原私有控制，没有新增生产测试入口。状态轮询有原5秒界限，CTest保持原20秒；没有使用固定延时猜完成，也没有操作用户数据库。

## 4. 首次失败与夹具整改分账

- red：5项中2失败，含首版压力夹具20.04秒超时和running检查点3条断言；3个子窗口尚未充分命中active清除阶段，不能以当时通过当作覆盖。
- red-controlled：真实64槽保留/身份两条断言失败；running仍3条失败。不得把它们写成已复现崩溃。
- red-drain：等待没有包含worker再次退出，active前置断言失败，属于夹具时序不足。
- red-active：三个窗口各自命中active=-1/c.failed=0，live/holding和UNKNOWN两条断言失败；本组才是第二项修前排空证据。
- reviewed-native：图checkpoint用例在done尚未消费时过早要求refs=0；保留失败，追加明确worker发布及UI消费屏障，仍要求最终refs=0，不删除断言。
- read-close-red：应用已移除时read call=1、refs=0，排空断言失败，归第二项最小修复。
- final-asan：新执行窗口夹具未观测store_failed，后续断言失败，原始日志/JUnit/匹配程序保留。原夹具暂停UI后没有空闲worker退出及故障命令接纳屏障；cr_fail_accept先有界核对真实退出，再在隔离夹具锁内一次发布故障与命令，避免依赖被故意暂停的UI poll唤醒。未修改生产策略、延长5秒或用新通过覆盖该首次结果。不把这个未命中的故障窗口算作生产存储错误复现。
- reviewed-light-build：编排使用不存在的ui_framework_dll目标，Ninja明确拒绝；改为实际ui_framework_shared，不改构建规则。构建失败与产品断言分账。

每次失败在各自failures目录，开测源和二进制清单与复制件核对。red/native独立于最终native/asan/light；不重建红色程序冒充原始二进制。未同时修改测试输入或重建正在运行的程序。最终源码和运行库身份以随后完整清单为准，早期绿色中间结果不冒充最终身份。

- verified-light原筛选56项中55通过，五应用graph_apps未构建而Not Run；随后只补目标及该用例，1/1、416检查/0失败。它不是产品断言失败，也不是第一次已通过；原未运行日志和JUnit保留。

## 5. 最终自动验证

| 最终组 | 实际结果 | 范围 |
| --- | --- | --- |
| Native verified | 96/96，136.93秒 | 9新case＋原Service、嵌入/工具、P4/P5、37存储故障、公共头、Assistant、三能力DLL用例 |
| ASAN verified | 96/96，138.94秒 | 同组真实业务、所有权、后台存储及关闭；无ASAN错误 |
| Light verified | 55通过／1未运行，90.44秒 | 9新case、P4/P5、两能力workspace编译分支、v2 DLL、P3/P4/P5正式投影、头/Assistant |
| Light graph-apps | 1/1，4.49秒，416检查/0失败 | 补齐目标，五实际维护应用协作；与上组逐项覆盖56/56，不是一次56全绿 |

所有结果来自新的独立构建，最后实现未修改后再验证。版本、依赖、来源/权限、公开布局及原限额保持；不相加重叠组为独立测试总数。原有测试函数及断言/时间/数据未放宽，9个新case保持真实64缓存、32提交周期及5秒状态/20秒CTest门。早期绿色/红色程序属于其各自源码阶段，最终身份由verified开测输入与source-after/binary-files核对。未重复无关browser、设备或旧600秒/60轮测试；原样本仅属其原身份和有限条件。

## 6. 历史风险及未验

原并行graph_boundaries超过20秒、ASAN控制任务5秒内未结束、P2五包首次15.04秒、面板180.44秒：保持尚未定位的期限风险。本轮串行与原并行负载、60轮面板不是同一复现，不据此归因环境或称已解决。原日志没有足够时序证据把这些期限失败归为本轮三个窗口；本轮窗口的正反受控证据只支持其限定修复，正式冻结仍需下一次独立判断。

旧frontend63、ASAN-Light脚本中断、Light surface_input、browser20、workspace首次20.05秒（门槛20）、Native输入首次失败继续保留。无新的直接证据支持改变其结论，不以新通过覆盖原失败，不扩大复审。

真实设备输入、Windows来源动态正例、IME/DPI/视觉/辅助技术、物理断电和人工长时仍按原延期清单，未执行且不冒充。browser9项副作用已由用户确认另案，未新增执行器或把副作用伪装为read/view，也不是本轮整改漏项。未操作用户数据库，所有故障、冷重开和应用DLL测试均用隔离数据/专用进程。

## 7. 交付、保全与停止位置

私有源码5项：src/ai_graph_internal.h、src/ai_graph.inc、src/ai_edit.inc、src/ai_store.inc、src/ai_service.c。测试/构建3项：tests/ai_graph_closeout.inc、tests/ai_coordinator.c、CMakeLists.txt。文档/规划7项：本记录、P6合同、P6计划、当前交接、根三规划。实际增量与源码/程序身份以独立根清单复核，不用HEAD全diff代替。

保留baseline、各阶段源/开测程序清单、红色与最终程序、failures归档、原始日志/JUnit、隔离DB、旧P6记录及新source-after/binary-files/delivery-identity。生产PE导出名单与旧P6完全一致：Native/ASAN326、Light340，无fixture；隔离DLL334/334/348仍原8测试入口，无新增生产授权旁路。

本轮源码核实、受控回归、三项最小修复及自审已经完成，自动结果支持再次开展P6R-1限定独立复审；**不自行宣称独立复审通过或正式冻结**。下一轮仅复审引用移交、cleanup-only、checkpoint推进门及本轮证据即可；不自动进入新功能。


保全清单补充：入口baseline.json自动捕获Git可见17470项，遗漏被Git忽略的三根规划。原baseline不改；baseline-supplement.json通过剥离本轮两段仅追加checkpoint恢复原始前缀，并与原P6归档逐字节匹配，另核对3项。合计17473项保全核对，不把这3项称为入口自动捕获。首次清单KeyError及补充来源见metadata-reconciliation.txt。


最终身份与清理：15项本轮交付、374项源码、102项最终运行/链接及PDB产物；source-after.json SHA-256 `1664A6BBC5F37A4B51F99DCF853298B6D4A62E8FE23B2A4DD76E03BD786EFAF4`。Native ui_framework.dll `141D4224424AFAAC75ED43C5D9595F971B5A7D338CFB0A8B532DDF026A6C9FBA`；Light ui_framework.dll `4E9B2109B49C93CCB8AB48CCA5D5FD93360F9960CB94F7025104EF8D2DE05AD9`；完整EXE/DLL/uapp/lib/PDB身份由binary-files/delivery-identity提供。开测源、最终运行产物及9个失败目录源/程序核对无不符；原P6 373源码/120程序归档仍全部匹配。保全合计17473项（17470入口自动Git可见＋3补充根规划），17460项原文件未变，无基线缺失或授权范围外修改，旧文档原始前缀/交接原始后缀保持。

cleanup.json仅删除本轮独立构建目录中522个可重建对象/临时链接缓存，共225298091字节；保留首次失败、红色/最终匹配程序与PDB、日志/JUnit、隔离数据库及源码归档。未清理旧目录、冻结SDK/固定依赖或无法唯一归属的共享TEMP缓存。无用户进程终止、用户数据库、真实Provider或设备操作。


## 8. 2026-10-28限定独立复审结论追记与快照冻结

本节经用户授权追记本会话上一轮独立只读复审：“三项整改闭环，P6R-1限定验收通过，当前交付快照可以冻结。”必须修正项为无。不是本轮重新运行测试或重做完整复审；旧章节的“等待独立复审”保留为当时实施交付状态。完整依据、冻结清单SHA-256和兼容边界见[xuanshu-p6r1-independent-review.md](xuanshu-p6r1-independent-review.md)。

本轮文档修改前只读复算374项源码、102项最终程序/链接/PDB、9项red程序及15项原交付，均匹配build/xuanshu-p6r1-closeout-20261028-91c4e7原清单。源清单SHA-256仍为1664A6BBC5F37A4B51F99DCF853298B6D4A62E8FE23B2A4DD76E03BD786EFAF4，完整程序及清单身份见独立复审记录。冻结是快照记录，没有commit/tag/release或证据脚本运行；本轮授权的后续文档增补不自动纳入原测试结果，原证据根不写入。

17470项是入口自动Git基线，另3份ignored根规划是事后补充核对，不能写成17473全部入口捕获。Native96/96、ASAN96/96均为已有证据；Light首次55通过/1项Not Run，补构建后该项1/1，逐项56/56，不是首次单次56全通过。

四组未定位期限风险、最终确认/核对证明提交后checkpoint错误的非阻断补测建议、旧历史失败、browser九项副作用另案及全部延期人工项目均保留。该限定结论不等于无条件P6完整/真实人工验收。下一步仅[真实Provider规格评审](../xuanshu-real-provider-integration-proposal.md)：用户已修订为DeepSeek官方Responses优先、OpenAI后补，逐任务确认当前输入/选取历史/必要只读数据；不授权实现、联网、取用密钥或AI修改。
