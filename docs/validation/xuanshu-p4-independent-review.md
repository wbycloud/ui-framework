# P3整改及P4-A独立源码复审

2026-10-26。基线为实际HEAD e0eca03db25930be55bdb45477027d1793a61904及P4-A工作区，不把HEAD全部差异归本轮。证据build/xuanshu-p4-final-20261026-c84f12。P4-A 30/318/131项入口匹配；44/49/44为已有证据，不代替源码审查。

## 1. 核对路径及原合同

按Service submit/continue→目标应用回调→方法租约/关闭门；edit propose→授权→意图/消费事务→BEGIN/EXECUTE/COMMIT→业务回读→最终SQLite确认→释放；以及UNDO/RECONCILE/注销逐项核对。P3 methods门和回调后READY检查已存在；BEGIN/EXECUTE/UNDO最后应用回调后modal/revision/关闭重检、复制阶段结果、回调及worker退出门均存在。固定依赖、v1/C2/P4A布局/错误/限额不扩展。

## 2. 待核实的局部缺口

源码发现：continue的fresh revision回调可重入接纳新edit，而外层submit只在回调前检查编辑互斥，回调后仅检查READY；需要受控回归区分源码窗口与动态结果。最小方案在最后目标回调后再次检查同一编辑门，不禁止正常重入或改变既有错误数值。未预称崩溃或业务并发修改已经复现。

## 3. 后续复审边界

P4-A内存夹具不证明真实应用事务，生产面板尚未接入。stateful已有固定路径UST1/UIL1副作用需明确不可撤销政策，不能假称普通内存write。原生批准按钮只能证明合作输入链，不是恶意DLL隔离。最终回归、范围内修正、失败及未验项在本文件后追加；当前不预填独立验收通过。

## 4. 独立核实和本轮最小修正

continue的新目标revision回调重入接受edit后，旧外层继续接受普通read的窗口由`review-red`真实DLL受控case证明：129检查/2失败；没有声称崩溃、并发业务修改或越权write已经发生。`ai_submit_impl`最后一次target callback后重查原edit互斥，仍用LIMIT_EXCEEDED，read ID留空，edit维持原关联。`review-fixed`Native30/30后才进入应用及面板。原P3 methods门不重复开发。

新增`ui_ai_edit_bind_target`方法租约覆盖真实revision→request_close/poll/destroy重入：内层destroy PENDING，回调返回后CANCELLED且不输出新目标，外层退出后正常关闭。Light绑定沿现有human guard且共享16条预算，页消息/程序点击拒绝；同步/异步、UNDO末回调门、模块租约、UNKNOWN和SQLite消费继续复用原链路。

自审源码另发现新POLICY_ONLY允许省略transaction的组合：若最终存储失败进入UNKNOWN，后续核对不能盲调空回调。已在阶段派发明确UNSUPPORTED；stateful现有偏好另提供只读RECONCILE，worker退出后回读原资源、effect NONE、不生成虚假事务号。它不是新的文件执行器；本项为源码发现并最小封闭，未故意触发或声称复现空指针崩溃。

## 5. 应用及正式面板自审

沿授权→原子意图/消费→共用写门→应用BEGIN/唯一run/staged→COMMIT→实际READBACK→最终确认→释放重新核对。应用快照只含其业务值，不含输入草稿、组件指针或授权；revision单调，EDA ID高水位不回退，undo不得跨revision。原人工handler同源，AI不从计数/显示/草稿判成功。空值未改变的调用明确回滚/未应用；EDA原clear含视图复位，即使对象已空仍保留该行为，undo不复原历史视图。

面板测试定位并修正严格Schema默认值不合法、长历史触发单控件RangeError。保留4095字节上限，完整范围/结果/目录/历史明确展示分页，不截断权威记录。第一次分页实现使用Light未支持的insertBefore；即使测试断言绿色，JS诊断仍不接受。改为已有DOM支持的静态分页控件，补投影断言；最终不得有此TypeError/RangeError。程序点击的PERMISSION_DENIED是来源负例的正确结果，不改产品来源门使之返回OK。

范围内构建依赖耦合（workspace不必要包含yyjson私有头）、专用夹具WGL/通知/异步谓词及链接遗漏均独立保留首次证据。见[最终验收](xuanshu-p4-validation.md)的失败分类；编译失败后旧EXE不冒充新源红运行。

## 6. 审查限制与停止位置

不重做C1全面复审，不将P2已有差异归本轮。新正式应用与面板路径采用同源隔离接纳测试及正式生产DLL旧人工/只读回归；不是物理来源证明。原frontend63、surface_input、browser20、workspace首次20.05秒/20秒门槛、Native输入首次仍独立待核实，不能以相称回归通过消除。最终结论以本轮验收清单、实际源码和匹配二进制为准，停止P4，不进入P5。

补充：正式面板回调在栈时unbind PENDING的缓存就绪窗口已最小修复；受控methods租约模型断言在途保留旧ID/不更新缓存、退栈后换新ID，Light及ASAN-Light面板通过。原生/Light合计16槽回归通过。新应用测试改GUID隔离数据库，PID复用导致schema/请求冲突的20秒测试超时保留，不错误归为产品执行超时。

ASAN-Light新应用路径出现脚本InternalError: interrupted（原50ms限制）。减少本轮应用事务投影对无关表/树的重刷，真实树名/undo投影另加断言；诊断仍在，原非AI人工generic204/features389对照亦有。它说明共享投影在该仪器构建存在风险，不证明物理环境根因或全部旧版本已有。本轮生产Light对应回归无该诊断，业务/回执与ASAN内存检查通过，因此列非阻断展示风险，后续集中视觉/性能核对，不扩成框架限额放宽。


## 2026-10-27 P5直接前置限定复核

本轮仅核对方法/应用回调租约、授权与原子意图/消费、同资源应用事务/回读/UNKNOWN和正式面板关闭门；没有重做全部P4或把旧HEAD差异归为本轮。入口43/323/352身份匹配。旧方法/最后回调门和真实DLL合同沿用，新增P5实例明确复用原私有控制器，不重复业务执行器。

源码自审发现正式控制绑定可早于排队范围投影；为源码窗口，未制造或声称真实用户误写red。宿主现增加投影代次门，Light同步投影成功才重新绑定P4/P5控制。最初新门缓存/测试等待缺陷在本轮projection-gate/cache失败中保留，随后正确清缓存/有界等实际投影；原P3/P4面板与新P5面板最终通过。P4旧公开结构/凭据/30分钟与5分钟、限额及来源政策不变。

其余本轮图的查环、describe分流、client缓冲、接纳/阶段确认和取消饥饿整改均属P5增量，详见P5验收，不冒充原P4历史崩溃修复。旧ASAN-Light脚本中断等仍独立未定位；本轮生产Light专项通过不替代设备/视觉或旧全量前端验收。
