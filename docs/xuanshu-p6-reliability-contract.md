# P6R-1 可靠性及故障验收合同

状态：本轮授权范围冻结，2026-10-27；实现状态由独立验收更新。无署名。SDK0.10-dev/API10/标准10、ABI1/包1，旧布局、v1、C2/P4/P5和固定依赖不变。不新增Provider/恢复执行/授权接口。

## 1. 前置事实与范围

P5最终37交付/331源/114程序及修前362项入口匹配。限定复审单Service/对话、图/预算/委派、admitted/committed、队列所有权、P4意图与消费、UNKNOWN、投影代次和生命周期，不重复全面历史验收。程序模拟和物理正例分账。内部应用清单包括五维护示例与browser，其他dialog_layout、light_web_probe、webview2_probe、minimal_eda单体main/headless为开发/诊断入口，不是另一个uapp业务系统；外部应用不写。

browser只读快照可以使用独立资源/revision和同目录read，原navigation/back/forward/reload/stop具网络副作用、settings-save/reset具文件副作用，不能按旧READ分类。external与页面/文档新来源不在冻结C2执行范围；相关人工行为保留，不新增执行器，完整副作用迁移已由用户确认另案评审，未完成时不得宣布原P5全迁移完成。

## 2. 持久化与兼容

保持schema1/2/3，不因可靠性新建schema。SQLite quick_check之外，启动在修改恢复状态之前核对：唯一主对话、runs/messages/events/tools关联、终态事件与任务终态、P4编辑/消费关联、P5图JSON/版本/预算及关联。失败UNAVAILABLE，不重建/降级/覆盖；原库及诊断保留。校验是阻止错误恢复，不是修复用户库工具。

收到/持久化接纳/业务执行/事务提交/回读/最终确认分开。写前意图与单次消费原子确认后才允许begin；确认未知仍阻止。根终态不代表UNKNOWN资源解除。旧授权/审批/实例/epoch不恢复；明确继续使用新目标、权限、业务核对且不得重放charged operation。应用仍负责真实业务持久性，内存夹具重启归初始值不等于业务已回滚。

WAL采用原FULL同步和单worker；在已确认写事务后采用SQLite PASSIVE checkpoint调节WAL，无UI I/O；正常关闭原FULL checkpoint/close保持。检查点不承诺物理断电测试：专用进程ExitProcess验证进程中断，真实断电/设备缓存仍延期。

## 3. 故障窗口与停止

专用子进程/新库按以下受控点退出：收到尚未保存、接纳已保存、scope保存、intent未确认/已确认、begin后、execute worker在途、commit前/后、readback前/后、final未确认/已保存未ack/已确认，以及图charged/settled/根终态前后。另检查checkpoint/flush/close故障。每点保存源码、父子PID/阶段、关联ID、库、日志和匹配程序；只退出测试自身进程，不强杀线程或用户进程。

重启两次须记录不自动执行：无active/queued/恢复授权，未完成NEEDS_REVIEW，消费及UNKNOWN/未确认编辑持久阻止继续；读核对不写业务。前意图窗口executes=0；commit后保留提交/回执或未知，不自动重放。

SQLITE_BUSY/IOERR_WRITE/FULL用隔离连接/本轮VFS故障注入，不冒称真实磁盘已满；坏头/逻辑坏副本/未来schema、备份校验/迁移失败各明确拒绝。VFS仅测试EXE/隔离库；故障控制不生产导出。唯一终态、幂等op、严格递增sequence、预算charge/settle及恢复次数均用SQL/业务值核对。

## 4. 资源、保留和验证规模

沿用Service8MiB、事件2MiB/64、根队列16、cache64、command32、P2调用1024/8MiB、P5并发4/后代32及冻结字节额度。不提高或降低。数据库/审计历史默认保留，不自动删除用户历史；WAL可checkpoint而非删除审计。无无限磁盘稳定承诺，磁盘失败停止新写入/未确认保留。生产删除/压缩/留存期限须未来用户选择，不擅自执行。

新增自动规模（测试预算，不是产品额度）：服务/应用完整装卸60轮、连续任务至少256次（跨cache64的四轮）、持续运行600秒、每任务/受控阶段有界轮询，单次完成上限原5秒、连续测试总上限720秒，装卸180秒。600秒是本轮可重复的十分钟稳定性样本，不能称无限或人工长时通过。正常测试不sleep猜完成；持续时限仅决定采样结束。

资源采样每轮/每批记录Service bytes/event bytes/queued/worker、进程private bytes/handles/threads、DB/WAL字节。暖机16轮后Service始终≤原额度，关闭后实例/调用/绑定/请求/worker归零、handle增长≤8、thread增长≤2（覆盖一个执行worker及系统异步清理）；Native private bytes相对暖机≤32MiB（两倍8MiB缓存加SQLite/分配器缓冲余量），ASAN进程RSS只记录因其隔离quarantine，逻辑预算/句柄/线程及ASAN错误仍严格断言。数据库增长按保留记录计数解释，不靠删除未完成/UNKNOWN压平曲线。

## 5. 兼容、验收和未验

公共C/C++头、P3单Service/tools/continue、P4事务/UNKNOWN/真实DLL、P5图/队列/上下文/恢复、五维护包双编译分支、正式Light投影、适用静态嵌入和browser新增read分别验证。复用仍匹配的证据，不相加为独立总数；现有断言/超时/额度不放宽。

停止条件：修改先于确认、未知门误清、重复业务执行、自动恢复旧授权、UI等待worker、先卸载DLL、生产导出测试授权、破坏旧合同、逻辑损坏库被改写。先最小修复/回归，必要合同冲突集中报告。历史ASAN-Light/frontend63/输入/browser20/workspace20.05秒独立，未定位不默认环境。设备、Windows动态来源正例、IME/DPI/视觉/辅助技术/真实断电及人工长时延期。

只有冻结条件均满足且无必要范围冲突，才能P6开发阶段模拟及自动验收通过；否则逐项给已完成与阻断，停止开发交付评审，不启动真实Provider/人工验收。


## 6. 实现中受控收口

连续面板首停滞后新增私有ai_graph_jobs：UI poll在锁内核对全部图子编辑存储job并重启已退出线程，覆盖“旧线程尚未signaled时接到新job”的窗口。worker仍按原规则空闲退出，不因测试hold导致busy-spin；没有UI join或新线程强杀。child-wakeup在隔离构建中令真实存储线程退出一次，之后要求自动排空、业务42及只一次受控退出。不是随机竞态压力替代物。


## 7. BRC-1：既有browser的合同内迁移

仅使用已有来源/业务：browser.view是实例视图资源，open epoch由同目录产生，revision为实际snapshot与语义焦点变化代次的SHA-256。新增browser.snapshot严格空对象read，生成普通/Assistant只读投影；原snapshot回调保留诊断。10个已有view（address/find/find-next/find-prev/plus/minus/reset/settings/diagnostics/panel-close）注册v2，复用原action函数，现有HMENU/Light标准control及适用快捷键只绑定ID/目标与完整text参数；页面消息不为这些view回退旧handler，不自报人工来源。程序调用不得获得v2接纳。

7项网络副作用navigate/back/forward/reload/stop/home/help及2项来源文件settings-save/reset保持原人工业务路径，不进AI/read视图；冻结C2不支持external，未新增执行器或新业务，其完整迁移明确仍有范围冲突。只读迁移和10个view不冒称9项副作用也已迁移；若最终仍未解决该必要范围，不作无条件P6全完成宣称。实际DOM按钮/字段、native menu和快捷键分类按源码单列，文档/画布链接新来源不自动授权。


BRC-1源码细化：home/help也会取消旧网络任务，故与另外5个导航/传输项一起保留原人工路径，不标view；合同内实际迁移10个view，另2项文件副作用，共9项副作用完整迁移未完成。revision对稳定视图语义值（地址/导航/查找/缩放/面板/滚动/状态/政策和诊断）取SHA，易变性能/线程计数只是遥测，不当文档版本。同步完成被背压时应用保留有界单槽输出，后续操作busy，UI timer重试；新回读不覆盖上一非read调用的核对关联。

## 8. 收口自审补充（先冻结、再实现）

PASSIVE检查点发生在已成功COMMIT之后。SQLITE_BUSY/LOCKED仅表示暂不能推进检查点，已提交事实不变；其他检查点错误须显式置Service存储不可用并停止后续新存储写入，不能忽略，也不能把已提交事务改记为回滚。专用隔离VFS在检查点边界一次性注入真实xWrite错误，要求可观察UNAVAILABLE/错误、已保存终态保留、拒绝新提交及安全关闭/冷打开。无公开接口、schema或错误码变更；测试控制只扩展既有私有控制mode，不生产导出。

2026-10-27用户确认：browser按最小方案推进，完整副作用迁移另案评审。BRC-1第7节最初12/7的分类已由后续源码细化更正为实际10/9；以10个view和9个副作用为准，后者仍阻断原P5全应用迁移，不阻止本轮独立可靠性验证。没有新增网络/文件执行器。

## 2026-10-28 P6R-1三项限定整改（实施前合同）

本轮仅修改私有所有权/收尾/推进门，不改变公开合同。Coordinator排队引用经worker移交给待消费结果，UI按slot和稳定run ID核对、消费后恰好释放一次。存储不可用时独立执行cleanup-only：不依赖active、不创建新业务阶段或存储job，只取消并收取既有完成、等待应用确认执行者/回调退出、保留UNKNOWN资源门。COMMIT成功仍是成功事实；非BUSY/LOCKED checkpoint错误置不可用，事务入口及同轮后继推进再次拒绝。

受控回归保持64缓存、32提交检查点和原期限；暂停UI消费制造真实缓存压力；在子意图/执行/最终确认期间令独立接纳事务发生真实VFS失败；使第32次提交精确落在running前置记录。修前仅称有界断言失败，未复现崩溃。新证据build/xuanshu-p6r1-closeout-20261028-91c4e7，后续实测另记，不覆盖20261027历史。真实人工和旧期限风险保持分账。

## 2026-10-28 P6R-1三项整改完成checkpoint（等待独立复审）

本轮实际e0eca03db25930be55bdb45477027d1793a61904 / codex/p1-process-boundaries未变，无提交；证据build/xuanshu-p6r1-closeout-20261028-91c4e7，入口17470，增量单独核对。先有受控红证据，再最小修复：Coordinator引用移交至done待消费并核对稳定ID；存储失败独立cleanup-only，不依赖active/c.failed；ai_begin及Provider/Coordinator/子编辑提交后门拒绝不可用存储推进。自审补齐关闭read关联排空及UI错误路径不等待通知队列。UNKNOWN、实际提交事实、原线程/回调和执行门保持，没有新执行器。

最终Native96/96、ASAN96/96；Light原批55项通过/1项未构建未运行，随后五应用目标构建并单项1/1（416/0），同源码逐项覆盖56/56，不冒称一次56全绿。9个新增受控case、原Service/P4/P5/37存储故障、两真实DLL编译分支、公共C/C++头/Assistant/静态嵌入及Light正式投影相称覆盖。首版夹具超时/屏障不足、原实现有界失败、自审read排空red、ASAN故障未接纳及Light编排未运行分别保留；不是已复现崩溃/永久死锁，也不以重跑覆盖首次。

SDK0.10-dev/API10/标准10、ABI1/包1、公开布局/v1/C2/P4/P5/固定依赖/JCS及额度不变。原并行boundaries20秒、ASAN控制5秒、P2五包15.04秒、面板180.44秒仍期限风险，未据本轮串行通过定位或解决；旧frontend63/ASAN-Light/输入/browser20/workspace20.05秒等仍独立。browser9副作用另案，真实设备/Windows正例/IME/DPI/视觉/辅助技术/物理断电/人工长时延期。未重复600秒/60轮旧样本，不外推无限稳定。

具备再次开展P6R-1限定复审的开发交付条件；本轮仅自审，不自行宣布独立验收通过或代码正式冻结。详docs/validation/xuanshu-p6r1-closeout-validation.md及新根delivery/preservation/source/binary/exports/日志/JUnit/cleanup。没有参考工程/用户库/外部应用/冻结SDK/固定依赖写入、子代理、computer use、真实Provider或Git/远程操作。原主线及历史仅追加保留。


保全清单补充：入口baseline.json自动捕获Git可见17470项，遗漏被Git忽略的三根规划。原baseline不改；baseline-supplement.json通过剥离本轮两段仅追加checkpoint恢复原始前缀，并与原P6归档逐字节匹配，另核对3项。合计17473项保全核对，不把这3项称为入口自动捕获。首次清单KeyError及补充来源见metadata-reconciliation.txt。
