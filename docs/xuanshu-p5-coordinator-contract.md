# P5-1 Coordinator、应用Agent与共享根控制合同

2026-10-26实施前冻结。状态：本轮已授权的实现规格，完成状态以独立验收证据为准；整体其余设计仍Proposed。无署名。

## 1. 唯一所有者与增量接口

一个workspace仍仅一个Service/主对话；P3主Agent简单read路径保留。复杂任务显式提交stub结构化任务图；它是测试脚本，不是自然语言理解。Coordinator是唯一依赖、并发和预算权威，应用不创建Provider/线程/第二调度器。ai_coordinator.h独立size/version=1结构，不修改P3/P4/v1/C2布局、错误值、SDK0.10-dev/API10/std10 ABI1/package1或固定依赖。

接口范围：ui_ai_agent_profile_register/unregister/get（host、纯数据复制）；ui_ai_coordinator_enable/get_state；ui_ai_graph_submit/get/answer/cancel/continue；ui_ai_graph_bind_light/unbind。没有直接授权API、source证明、Provider注册或任意代码执行器。输入全部UI线程，复制后返回；JSON输出由ui_ai_release释放；同步received可早于submit返回，run ID先写出。旧P4提案仍一次具体调用，绝不自动扩成多步骤。

## 2. 任务图、权限和上下文

graph JSON：nodes数组1..32；每节点稳定id/operation_id、parent（空为root）、dependencies数组、agent_profile（空为主Agent直接工具）、instruction、context、target索引、capability、params、constraint；可选question（人工关联补充，不改变params）和fail用于stub失败验证。父先于子，依赖可前向；全图查环（含父边）、唯一ID、存在性、深度和权限在接纳前验证，执行前再核对。动态spawn只可接纳预声明图中的节点，不能从工具结果或应用提示增加节点/权限；需要新节点必须新提案/宿主确认。失败spawn保留其Agent预留并结算为失败，不退款循环。

有效权限为配置能力集合∩根确认具体步骤∩父向下委派∩宿主read或独立P4编辑政策。父子target必须相同，子能力须在父委派列表；明确跨应用的兄弟步骤分别从root委派，不共享兄弟上下文。每子仅获得自己的instruction/context、目标、能力Schema及自己的问题答案；不注入主对话、兄弟私有上下文或依赖的完整结果。依赖只提供状态，显式摘要列为数据；输入和应用角色文字永远不能签发权限。记录输入manifest/hash与必要摘要，不广播完整私有内容。

全部目标在提交时深复制instance/resource/kind/host epoch/open epoch/revision/scope；目录descriptor hash和JCS参数/constraint固定。执行前最后应用回调后复核；不随标签切换/重开/版本变化改绑，不自动重试。各步骤明确revision；前一步修改同资源后，后一步旧revision应失败，须新图重新确认，不能替模型递增。

## 3. 授权、审批和业务执行

含write图由宿主展示全部结构化具体步骤、范围和参数再通过私有人类输入链确认，一次确认授权图中有限步骤，不是无限循环。30分钟从确认起算，不随子Agent启动/排队/对话框续期；取消/终态/关闭/重启失效。破坏性/external/不可撤销每具体步骤另5分钟审批；只用P4既有memory reversible/approved-destructive或policy-only拒绝，不新增外发执行器。只读图不需要写授权。

AI任务授权独立于C2人工业务凭据。页消息/公开模拟输入只能提案或回答问题，不能确认授权/审批/迁移/UNKNOWN证明。Light绑定仅可信workspace chrome；内部一次性消费绑定完整root/节点/会话/hash，重复/过时拒绝。同进程合作，不隔离恶意DLL，不证明物理设备。

编辑复用同目录、同P4阶段控制器与业务handler；每在途节点可有独立私有控制状态，不创建Service或第二业务执行器。写前意图+该步骤单次消费原子持久确认才begin/execute；调用完成、commit、readback、最终存储和回调退栈分别处理。人工/AI共用资源门；busy明确失败不排队重试。取消不等于rollback或解门；提交结果保留；UNKNOWN按原P4证明合同阻止资源，不由root终态/forget/重登记/重启清除。主对话中的显式核对通过原READBACK/RECONCILE及存储确认，不能猜未修改。

## 4. 本轮冻结安全上限与预算

根steps64/model16/tool64/agent32；单次model输入128KiB且仅估算token（stub不伪造真实usage），单子private context16KiB、全根128KiB；单子result16KiB、全根result/最终summary64KiB；图JSON64KiB；持久快照512KiB。根内并发4、深度3、直接子8、后代32，普通根排队16沿原P3。应用profile每项64KiB、每实例8项/总256KiB，文字16KiB，不提升P2目录/调用/Schema限额。内存实际复制、活动编辑控制器、profile、结果、请求/缓存均在所属原Service8MiB或profile独立256KiB内预先计账；控制仍原256KiB/事件2MiB，不拿P2预算补额度。

图接纳原子预留所有节点的step/tool/model/agent及context/result容量，重复请求不重复扣。节点启动实际charge；失败spawn/cancel保留已预留记录，未启动部分标settled-unused而非自动退款可复用。usage未知单列并保留原预留，不按0加额度。每次状态变更先复制快照到串行worker，确认后再推进下一副作用；无队列空间则状态可查询/背压，不递归通知或重复派发。等待依赖/问题/审批/收集不占并发执行槽，不轮询模型。

## 5. 状态、取消和生命周期

root RECEIVED/QUEUED→SCOPE_WAIT→RUNNING→COLLECTING→FINISHED；node RESERVED/WAIT_DEPENDENCY/WAIT_INPUT/WAIT_APPROVAL/READY/RUNNING/RECORDING/COLLECTED。失败阻止依赖，独立节点继续（fail-fast=false）；根取消遍历所有后代，等待真实P2调用/编辑阶段/worker和回调退出。5秒只标cancellation_stalled，非终态、不强杀。root唯一终态需所有后代收集、结果持久化、预算结算及租约退出，UNKNOWN允许根结果unknown但资源隔离继续保留。

UI线程解析workspace/目录/目标、应用业务/事务、结果复制和投影；后台只SQLite/stub复制数据，无应用裸指针。方法租约覆盖最后应用回调，poll重入不推进；UI不join或等待队列。应用注销后新spawn拒绝；在途调用仍持P2租约至退出，REFUSE只恢复接纳不复活已取消任务。

## 6. 存储与恢复

schema3新增graph快照/有序操作日志；root仍原runs/messages/events，唯一终态。graph包含拓扑、节点/父/依赖、预算reserve/charge/settle、授权引用/到期、问题/回答hash、目标/效果/回执/summary和context manifest。完整上下文不进入广播；本地受信用户可查询自己的显式节点上下文，数据库仅保留hash/摘要（instruction≤256字节，context不落全量）。历史按原分页/有界512KiB，暂不自动删除审计历史；长期保留策略属P6，不能声称无限历史有界。

schema2→3必须宿主确认路径/schema/影响/备份，停止新根/写入并排空，生成且quick_check/版本/内容计数校验一致备份后事务迁移。失败保留原库/备份/诊断，不重建/覆盖；schema1先原P4迁移，future/损坏拒绝。自动化仅新隔离临时库/副本。重启只恢复记录NEEDS_REVIEW，不恢复图内执行、授权/审批/预算凭据或旧epoch；continue显式新client和新完整目标/图、核对原未完成记录，已提交步骤不得自动重放。

## 7. 应用与边界

五维护示例注册可选pure-data profile，只选择已有read/edit能力；主Agent无profile直接调用仍合法。跨应用测试用实际模型值/revision/对象ID/事务/undo，不用计数冒充业务。browser尚legacy、没有资源/Schema/revision/事务；WebView2/新来源/外部应用明确未授权。最小后续方案是独立browser只读资源/契约迁移评审，不能在本轮通过legacy加入。原P5“全部其他应用迁移”不宣称已完成。

## 8. 验收与停止

见实施计划与独立验收。必须有环/深度/委派/隐私/预算/幂等/背压/问题/审批/过期/存储/重启/真实DLL退出及五真实应用结果；正式面板仅投影，收起/切标签/重建不改任务。设备、Windows动态来源正例、IME/DPI/视觉/辅助技术/长时延期，不假装人工通过。止于P5联合评审，不进入P6。


## 9. 2026-10-27实施及自审收口

本轮P5-1已实现，开发阶段结果见[独立验收](validation/xuanshu-p5-validation.md)，原总体其他条目仍Proposed。实际新增接口仅ai_coordinator.h，旧公开布局/语义/限额保持。PENDING携带run ID时可仅为旧目标的异步历史身份查找，不是新执行接纳；existing/rejected区分幂等与冲突，宿主保留输入到确认。

实际接纳admitted与阶段committed独立，受控取消停滞不会饿死已排队任务；读last_call在forget后保留。图依赖预先声明并验证，运行时按需创建已委派节点，不接受工具文本临时扩图；新增步骤需新提案与确认。角色/model上限取交集，root model16使一次都有模型调用的profile节点至多15，不提高额度。

完整私有上下文仅在有界内存保留，运行记录存manifest/hash；问题显式共享到主对话，答案只给所属节点，持久化答案关联与hash。继续不恢复旧上下文/授权，必要时重新收集。Service图延续P3 C字符串约束，不接纳嵌入NUL的字符串值；不改变能力层JSON/JCS本身及五应用现有业务文本限制。

正式宿主以投影代次封闭旧控制，只有Light同步post成功才启用对应绑定；JS异常/超限保持拒绝。没有物理输入或Windows动态正例验收。browser现有Light legacy路径没有自动进入目录，原其他应用全迁移规划未完成；详见独立覆盖矩阵，不以WebView2排除理由替换真实缺口。P6/真实Provider未实施。


最终私有队列收口：入槽时在Service锁内重新检查job/busy/done；未消费完成通知占有槽，不能被新快照或历史查询覆盖。关联字段与job一次提交，失败保留可观察状态、不重复执行；无公共合同/额度增量。受控发布窗口回归见独立验收第10节。
