# P4A-1：单资源受控编辑合同

2026-10-26。状态：P4A-1先冻结后实现，本轮单资源夹具代码已完成；自动验收以独立记录最终JUnit为准，整体架构仍Proposed。读者为框架和应用开发者。P4-B/C、正式聊天审批面板和应用迁移不在本轮。

## 1. 问题、选择与兼容

P3只有read执行链；C2只证明人工UI调用，complete_v2不是应用事务提交。采用独立编辑描述、阶段完成和Service编辑接口，在原目录内管理同一ID、Schema、资源及唯一业务execute。禁止同时以v2和编辑描述注册同ID。不采用legacy修改入口、source字符串、应用自报授权或第二AI执行器。

旧公开布局、v1语义、C2A-1、错误数值、SDK0.10-dev/API10/标准10、ABI1/包1及依赖/限额不变。独立新结构完整size/version=1。应用拥有真实事务、业务与撤销数据；框架仅调度并核对回执，不建立通用撤销栈。

## 2. 授权和来源

宿主展示具体目标、能力、允许参数Schema、风险和有效期，用户确认后产生任务授权；自由文本、脚本、工具结果和注册描述不能签发权限。原生宿主确认按钮由框架私有事件链识别并一次消费，独立于C2调用凭据；不接受BM_CLICK、WM_COMMAND、source或参数自报。测试只使用编入隔离测试运行库的私有接纳入口，生产DLL不导出它。Windows来源分类不是物理设备证明，不隔离恶意同进程DLL。

任务授权最多30分钟、逐项审批最多5分钟，从确认起算，不续期；排队、对话框和重试不延长。任务结束/取消、关闭/重启和条件变化使授权失效，历史保留。普通可撤销write在范围内执行；破坏性/external/不可撤销操作仍逐项审批，P4-A即使审批有效也不执行其业务，仅验证绑定、消费及拒绝。

授权绑定能力版本/descriptor_hash、资源实例/host/open epoch、revision、scope、参数约束和政策版本。审批另绑定具体operation ID及JCS参数SHA-256。每次派发重新检查。由本调用确认且持久化的成功结果才能推进任务预期revision，模型不可自改。到期只封闭新派发，已开始事务按原生命周期收尾。

## 3. 调用与事务状态机

提案→待任务确认→待调用审批（若需要）→取得共用资源门→保存意图及单次消费→最后检查→BEGIN→EXECUTE→COMMIT→READBACK→结果保存→完成。提交前失败/取消→ROLLBACK→READBACK；提交后取消仍报告applied。UNDO是新的显式宿主操作，核对资源、当前revision及应用undo回执，再经过同一意图/门/结果链，不自动回滚整任务。

应用阶段接口为BEGIN、EXECUTE、COMMIT、ROLLBACK、READBACK、UNDO、RECONCILE。EXECUTE只调用目录中唯一业务handler；其他阶段调用应用事务适配。阶段完整结果按稳定call/stage ID交回，可同步早于派发返回，也可异步；重复完成明确拒绝。回读结果须通过原输出Schema并有实际revision和事务/撤销回执。不能以计数、草稿、控件更新证明业务成功。

UNKNOWN及存储未确认保持共用资源门。取消、forget、重登记不清门；明确只读核对必须证明执行者及外部回调退出，业务回读、revision和操作回执一致，核对记录保存后才解除。无法证明或重启后仅剩旧epoch记录时不重放、不推断未修改。缺少跨重开身份/回执的资源不开放恢复后的AI编辑。

## 4. 存储和迁移

UI建立client/run/operation/call关联先于任何执行。SQLite worker在同一事务保存写前意图和授权/审批单次消费；确认前execute=0。确认后再次检查，失效则保存未执行结论，消费不回退。应用commit、业务回读和结果持久化是三个阶段；最终结果保存前不释放资源门。运行和权威事件唯一终态、operation幂等；失败不重放业务。

schema2增补编辑任务、意图/消费及阶段事件，仍保留P3主对话和记录。schema1不自动迁移；宿主展示路径/schema/影响/备份位置并由用户确认。暂停新写入、排空存储操作，后台生成并校验一致备份后事务迁移。备份/校验失败不迁移；迁移失败不重建、不降级、不覆盖恢复。损坏/未来schema直接拒绝。实现和测试仅使用隔离临时库或副本，不操作用户库。

## 5. 线程、寿命和预算

UI线程解析workspace及应用对象、调度事务/业务/回读；SQLite及耗时准备后台。worker仅持稳定ID和复制数据。输入返回前复制，回调数据借用至返回，输出完成时复制。所有外层方法及应用回调持租约；同步完成不表示外层execute退栈。写门贯穿结果存储与回调收尾，注销/关闭保持PENDING，不强杀、不UI join。

编辑记录和宿主确认绑定纳入Service既有8MiB；单次能力记录、输出和资源门纳入P2既有1024/8MiB及目录2MiB。任务一次单资源编辑，输入64KiB、输出256KiB，P3控制256KiB/通知2MiB不提高。满时显式背压，不丢关联或递归通知。P4-A只允许一个在途编辑控制器；普通read运行期间不抢占，不自动排队或重试编辑。

## 6. 验收和停止

必须覆盖假授权/越界/legacy拒绝；30分钟/5分钟边界；重复消费；固定目标/重开/版本；非法参数/disabled/busy/modal/末次回调；意图未确认execute=0；同步异步/重复完成；提交前后取消/回滚失败；真实值/revision/回执；UNKNOWN/核对；各存储中断窗口和不重放；备份/迁移/损坏/未来schema；背压/迟到/注销/真实DLL worker退出。

P3租约缺口必须先验证并收口。任何无意图先修改、假回滚、重复执行、未知结果被清除、生产私有授权导出、提前卸载或超限均停止交付验收。真实设备、Windows动态正例、IME/DPI、视觉/长时继续延期。自动通过仅能称P4-A开发阶段模拟及自动验收通过。

## 7. 独立接口冻结清单

capability_edit.h：ui_cap_edit_desc_t一次注册base v2描述、风险和应用事务stage回调；ui_capability_register_edit不重复注册ID。唯一业务执行仍是base.execute。ui_capability_edit_get_stage提供借用期内的阶段/operation/transaction信息；ui_capability_edit_complete复制阶段回执，stage ID重复返回ALREADY_EXISTS，旧complete_v2不能提交AI编辑阶段。内部prepare/check/stage/finish/release只供Service，不导出授权执行旁路。

ai_edit.h：ui_ai_edit_enable异步检查schema；ui_ai_edit_propose只创建待确认范围，含client ID、capability、固定target、参数约束Schema和具体参数。ui_ai_edit_get_state返回复制JSON；ui_ai_edit_cancel只请求取消；ui_ai_edit_forget不能清UNKNOWN。ui_ai_edit_bind_button把具体提案指纹与CONFIRM_SCOPE、APPROVE、UNDO、RECONCILE、MIGRATE等动作绑定框架原生按钮；不导出接受approved/source/token的执行API。专用测试可用隔离运行库的私有确认/时钟/故障钩子，不属于生产DLL。

Service仅一个编辑槽，所有内存复制按既有预算接纳；普通read任务在途时拒绝新编辑，编辑在途时新普通任务明确背压。query/cancel/close控制仍可用。编辑槽可在已知终态后释放，历史/operation幂等保留在库；UNKNOWN或未确认保留隔离，关闭可以保存记录并安全退出，但不能重启后自动执行或凭新提案解除隔离。

编辑业务阶段的应用数据、事务和撤销回执不进入通用框架撤销栈。AI库只保存必要结果、身份及回执引用；暂存模型/undo快照在夹具DLL内。P4-A没有正式授权聊天面板，原生专用宿主按钮是最小生产来源适配；物理正例仍延期。

迁移失败后的新尝试必须重新准备备份位置及绑定hash，再经新的宿主确认；ui_ai_edit_enable在schema1/空闲且前次I/O退栈时可重新准备，不覆盖前次备份。原调用UNKNOWN的只读核对增加阶段事件，不把原唯一终态重新变成RUNNING或写出第二终态。

请求身份以JCS参数、JCS约束Schema分别SHA-256，再对固定身份包（client、capability/descriptor、policy、完整目标/scope、两指纹）JCS＋SHA-256，避免将两份64KiB合法数据再次塞入单输入64KiB解析器。消费绑定还含Service authority epoch；重启不恢复授权。阶段effect沿none/applied/unknown，最终rolled_back布尔及回执明确区分已验证回滚和从未执行。


## 8. 已实现接口细则和错误语义

公开增量恰为10个函数，声明在`include/ui_framework/capability_edit.h`和`include/ui_framework/ai_edit.h`。独立结构要求完整size及version=1，不改嵌入的v2字段。所有API在所属UI线程；调用方释放get_state的复制JSON。事务context仅借用至回调返回；应用异步必须复制稳定call_id/stage_id及必要数据，不在worker保留context/host裸指针。完成应由UI线程送回，executor_exited仅是合作应用对worker和外部回调均已退出的声明。

- register_edit原子复制原v2描述与risk/transaction，ID冲突ALREADY_EXISTS，资源与目录代次/注销仍同一来源。metadata增加edit_contract=1与risk，原v2描述不改变；普通AI/read发现不纳入它。
- get_stage返回当前阶段副本；complete仅接纳匹配非零stage_id，重复完成ALREADY_EXISTS，错ID NOT_FOUND，executor未退出PENDING。输出超限LIMIT_EXCEEDED且保留阶段；非法Schema/冲突事务回执归为阶段验证失败、进入保守核对，不能假成功。旧complete_v2/get_result_v2不接受AI编辑阶段，旧forget拒绝绕过。
- enable异步检查schema；schema1等待独立宿主迁移确认。propose的OK仅表示已建立身份与内存接纳，RECEIVED(15)经存储确认才SCOPE_WAIT(1)，不等于已授权。入口分配/入队失败清理未接纳标记，返回原错误，out ID为空，不生成假任务。
- 同缓存同ID同内容返回原ID；不同内容ALREADY_EXISTS。仅在数据库发现冲突时异步返回code=request_id_conflict、rejected=true、storage_confirmed=false，不覆盖旧任务。已忘记的相同请求读回原结果，不重复消费或执行。
- get_state中的call/instance ID用十进制字符串，effect仍none/applied/unknown，transaction_committed和rolled_back独立表示应用阶段事实。storage_confirmed表示结果记录获确认，不能从内存phase推断SQLite成功。
- cancel仅请求；forget只接受已知DONE且I/O/回调/执行门均退出，UNKNOWN为PENDING。资源被占用沿C2的PERMISSION_DENIED门处理，提案槽或迁移在途为PENDING；不增加新的错误数值。
- bind_button限所属UI线程的原生Button，引用当前binding_hash与具体宿主动作。每个绑定最多接纳一次动作，最多16条，含分配计入原Service预算；失效hash、其他控件、程序消息不取得授权。确认队列拒绝时不留下已确认scope；再次操作须重新人工确认，不自动重试。

阶段ID在应用回调前建立；BEGIN仅接纳本次transaction_id，COMMIT必须给出undo_id，其他阶段不能把上一笔回读的事务号变为新事务。EXECUTE仍调用原描述的唯一execute；UNDO调用应用事务适配以应用回执执行新的显式操作，不建立第二业务目录或通用undo系统。

phase值0..15分别为IDLE、SCOPE_WAIT、APPROVAL_WAIT、GATE、INTENT_WAIT、BEGIN、EXECUTE、COMMIT、ROLLBACK、READBACK、RECORD、DONE、UNKNOWN、PROOF、BASELINE、RECEIVED。BASELINE在BEGIN前只读核对当前值与revision；回滚后以输出JCS指纹和实际revision/事务证明核对，不能只信rollback返回OK。既有v1/P3 task枚举不变。

## 9. 实现边界和后续依赖

已实现：单Service编辑控制器、同目录注册、宿主按钮绑定、单资源门、后台schema检查/一致备份/事务迁移、原子意图及单次消费、实际应用事务/回读/撤销回执、唯一终态、确认丢失和UNKNOWN核对。SQLite只增补schema2的ai_edits及ai_edit_consumptions，P3消息/有序事件继续复用。

风险为destructive/external/irreversible的首批注册仍使用write载体，只测试政策和逐项消费后effect_not_enabled，绝不执行文件、网络或外发。普通read任务与编辑互斥接纳；UNKNOWN后允许只读核对，但AI编辑不排队。编辑控制器已有固定容量、原预算不提高；临时/调用方查询副本按借用与释放规则管理。

重启后的旧epoch编辑记录不重放、不恢复授权；单资源内存夹具不能提供跨进程真实业务回执，因此recovered_unresolved保持阻止新AI编辑，不假设新内存值10证明旧进程未修改。当前进程显式核对必须等应用worker/外部回调退出，核对记录持久化后才解门。原最终记录未获确认但实际上可能已落库时，幂等核对只补一条解除事件；数据库唯一终态不得重复。

P4-B/C仍只属后续依赖：真实应用事务适配、稳定业务回执及恢复核对、正式结构化授权/审批面板。真实设备来源正例/Windows分类动态验证/IME/DPI/视觉和长时延期。本批原生按钮只提供最小宿主来源接口，不宣称已经交付正式聊天审批产品或已执行用户数据库迁移。

显式RECONCILE若原调用已有事务号，必须回传相同的事务回执；缺失或冲突继续UNKNOWN，不能仅以合法输出和executor_exited解门。普通回读不会将旧事务号带入新BEGIN。未获接纳的提案/确认队列拒绝不留下live或scope标记；仍在途调用的预算/门不因此释放。生产Button程序消息负例使用真实可见窗口，但不是物理输入正例。

UNDO与EXECUTE同样在实际阶段派发前核对资源revision、能力状态、modal/关闭和授权取消门；BEGIN通过不能替代UNDO前最后检查。派发前拒绝走应用rollback及实际回读，保持原业务值，不自动换目标或重试。


## 10. P4BC-1 实现前冻结（2026-10-26，剩余P4明确授权）

本节是P4-B/C必要增量，不覆盖P4A-1正文或v1/C2。当前先冻结，实际完成和未验状态随后由独立验证更新。仍单Service、单目录、单业务execute、单资源调用，P5不在范围。

### 10.1 真实应用及风险登记

独立capability_edit_extensions.h新增ui_cap_edit_extension_t：完整size/version=1、嵌入既有ui_cap_edit_desc_t、execution_policy和side_effect。新ui_capability_register_edit_extension原子走同一register_common，统一ID/descriptor_hash/预算/注销。APPROVED_DESTRUCTIVE只允许可撤销内存destructive＋应用事务回调；POLICY_ONLY只允许irreversible/external，AI逐项审批/消费后仍明确拒绝实际执行。既有register_edit的非普通风险拒绝语义不改。目录metadata明确execution_policy与side_effect并进入指纹。

已有偏好文件人工入口需保留：只有新POLICY_ONLY描述在既有C2可信人工路径仍调用原handler；AI不会调用它的事务或执行器，不从source或参数认定人工。不是新文件/网络执行器；保留既有UST1/UIL1及后台I/O。EDA clear可撤销但属破坏性，使用APPROVED_DESTRUCTIVE，必须逐项审批。人工原clear仍走C2一次性语义接纳，不伪装AI审批。

普通内存write直接用原register_edit；应用helper只协助原目录注册/阶段转发，快照格式、复制/恢复/投影均由各应用提供。应用在BEGIN保存真实文档before；唯一run在staging标志下计算工作状态，不投影控件/对话框、不提交业务。保存staged后恢复committed；COMMIT才发布工作状态和递增revision，生成opaque undo回执。UNDO从应用保存的上一份已提交before计算新的staged，另经commit；失败rollback必须回读实际committed。应用数据、undo快照不进入AI数据库，不是框架通用事务/撤销栈。

应用文档revision保持单调，撤销不得恢复旧revision或复用对象ID；EDA next_id为实例分配高水位，不因undo回退。EDA clear提交才重置zoom/选择，undo恢复文档对象/笔记不自动恢复历史视图。stateful和generic表单投影仅在commit/已知rollback后，保留独立草稿、base/generation与冲突；AI不因恰好同值清除用户草稿。EDA note的generation参数保持原范围，AI事务不接纳本地输入草稿，提交后按业务revision标冲突。

每实例保留最近一个可验证撤销回执；后续人工/AI写入改变revision使旧撤销失效。before/staged/undo数据应用自行有界分配（沿现有模型容量，不扩大数据范围），begin失败不执行业务；同步阶段complete仍不释放外层回调/module门。

### 10.2 正式Light面板及可信动作

独立ai_edit_ui.h新增ui_ai_edit_light_binding_t（完整size/version=1、view、control_id、action、binding_hash）及ui_ai_edit_bind_light。原unbind_button作为共同绑定ID释放入口保持原合同；两种绑定共享原16条/8MiB，不加预算。view必须是同workspace已接入C2输入的可信Light chrome，不允许业务应用view冒充宿主。控件ID/hash复制；view借用至解绑或框架detach，重建不保留旧view指针。

workspace_input_semantic在已认证私有语义链内匹配控制ID，再消费host registry的单次human资格；校验原guard、事件种类及current hash，才调用原Service的ae_action。不从WM_COMMAND、页面消息、自由文本、脚本或应用标志签发授权。直接程序语义调用只能拒绝；私有测试正例仅编入隔离运行库。原生bind_button、v1/C2结构和旧错误数值不改。

正式framework_host复用原全局面板/Service。输入具体capability、resource、完整params和允许参数Schema只创建未授权提案；展示固定实例/资源/epoch/revision、参数/约束、风险、副作用、有效期、scope/approval状态、原始错误和实际值/回执。确认、逐项批准、undo、只读核对、迁移按钮走私有Light绑定；拒绝/取消只撤销执行资格。页面伪造approve消息明确拒绝。输入仅收到持久提案确认后清除；失败保留。

切标签、折叠、重建只改变投影。历史query扩展可选edit_record，不恢复旧授权；输出复用原reply，不重复复制256KiB业务输出以突破512KiB查询上限。重启旧epoch未核对记录保持阻止，面板解释原因；不提供凭新实例或重提案清UNKNOWN的入口。schema1需展示真实路径/备份/schema/影响后明确确认，测试仅隔离库。

### 10.3 批次及停止条件

P3/P4-A本轮独立重入回归收口后：stateful三项普通内存write→EDA add/note/clear（含审批）→generic/features两分支各三项write及counter→正式面板→自动/自审/证据。13项内存write和3项文件副作用逐项记录；其余39项read/view/draft/显示/生命周期按真实效果保持，不借AI write迁移篡改来源。stateful树本来没有rename命令/业务模型（api7_fixture注册tree无commands.rename），不新增此业务；树名实际编辑由generic/features的demo.rename覆盖。

验收包括committed/staged分离、取消/rollback、单调revision/稳定对象ID/撤销回执、真实workspace固定实例、草稿冲突、文件政策拒绝及原人工入口、Light原页面输入/错误/持久状态投影和伪造批准拒绝、UI/worker/队列/在途关闭。生产DLL无私有测试导出。真实设备及视觉/IME/DPI/长时仍延期；实际范围未交付不得称P4开发阶段通过。

P4BC-1补充（实现前）：新APPROVED_DESTRUCTIVE能力的显式undo同样是新操作；UNDO按钮只确认其任务范围，仍需新的逐项批准和5分钟期限，不沿用原消费记录。原P4A普通可撤销undo合同不变。

P4BC-1实现前补充：ai_edit_ui.h新增ui_ai_edit_bind_target(service,instance,resource,target_v2)。这是UI线程只读目标解析方法，覆盖get_revision期间Service方法租约，回调后重查关闭及实例状态；不是授权或派发。正式host不在Service外借用对象解析目标后再使用可能被重入关闭的Service。旧bind及v1目标布局不改。

P4BC-1面板投影细化：沿用Light单控件4095 UTF-8字节，不放宽上限。结构化范围、结果、目录及历史采用明确的展示分页，完整原文仍在Service/复制模型中；分页不是截断权威记录。单次提案输入仍受实际控件和64KiB桥接双重限制，超限明确拒绝。默认空参数约束必须是严格object（properties={}、additionalProperties=false），不改变Schema子集。

P4BC-1政策类核对：POLICY_ONLY不执行BEGIN/EXECUTE/COMMIT。其最终存储确认失败仍保持UNKNOWN；可选应用只读RECONCILE适配仅回读原偏好资源并证明worker退出/未执行。未提供适配时明确拒绝阶段调用，保留UNKNOWN，不能调用空回调或自动解门。

### 10.4 实际交付状态及最后收口

本节P4BC-1已落实，逐能力状态见覆盖矩阵第7节。独立公共头各完整size/version=1；UI线程/借用期与旧结构/错误不变。Light解绑若回调/方法在途返回PENDING，宿主不得缓存新hash为已就绪；退栈后重试，额度失败明确展示。专用受控租约模型回归不冒充真实物理事件。原生Button与Light共用16条预算。

真实应用事务stage只在COMMIT投影；sample仅查询对应表/树，仍对业务revision变化投影表单冲突。ROLLBACK恢复未提交工作状态后无需重刷未曾改变的界面。每实例保留最新单个undo回执，非通用undo栈。恢复记录不会重新授予授权，重启旧epoch仍需核对，不自动写入或重放。

正式面板显示完整范围/结果的分页，input控件与桥接双上限保留；程序点击/页面approve消息拒绝。真实用户迁移没有执行，所有自动备份/迁移仅隔离库。独立验收/风险/延期见对应文档；不将测试0失败等同所有JS/物理/长时均已验收。
