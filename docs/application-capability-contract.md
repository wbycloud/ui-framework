# 应用统一能力协议

2026-11-12。当前匹配开发接口为SDK 0.10.0开发版、API10、标准10、ABI1和包格式1。资源、业务能力、普通UI及通用助手共享同一目录和业务执行器。下面的当前索引对应实际公共头和应用源码，后文保留早期设计及逐阶段状态。完整接入顺序与所有权见指南。[接入指南](xuanshu-general-assistant-integration.md)

## 当前支持矩阵

| 层 | 已实现入口与职责 | 开放边界及依据 |
|---|---|---|
| v1 read | 资源、Schema、JCS、bind/prepare/dispatch/complete/result/forget | capability.h保持read效果及固定read scope，旧数值/布局保持 |
| C1 | 从v1注册生成普通命令及原Assistant只读入口 | capability_entry.h，固定目标，发布/退役与目录代次关联 |
| C2/v2 | 人工菜单、Light控件/快捷键、表单、共同组件绑定同一业务能力 | 独立v2结果含效果及UNKNOWN，来源由框架分类；字符串和页面消息不能签发人工资格 |
| P4 | 单资源受控编辑、实际事务/回读、撤销与人工事实核对 | capability_edit.h和ai_edit.h，UNKNOWN保留资源门及原错误，cleanup-only收尾 |
| P5 | 有限计划、应用专业Profile、权限交集、节点关联及持久化 | ai_coordinator.h，同一Service/目录，具体范围及危险操作按原可信门 |
| 通用助手 | 自然语言草稿、结构化范围、初始准备/有限计划、专业上下文和结果 | ai_assistant.h，初始请求/有限计划分别完整展示并可信确认，宿主统一Provider/计量/存储 |
| 代表应用 | Counter、五维护示例、实际browser快照/正文和维护组件单元格 | [当前应用矩阵](xuanshu-general-assistant-integration.md#当前支持及选择粒度)逐项列读写、结果和示例 |

browser.snapshot的state为完整顶层状态对象，browser.body为dom-body-v1正文及revision。维护组件的test.edit使用实际业务函数与P4回执。浏览器九项导航/设置副作用、完整EDA及未注册业务能力继续按应用分别接入，历史只读结论按原日期保留。当前匹配冻结、生产371项导出及适用范围见记录。[匹配交付](xuanshu-general-assistant-delivery-manifest.md)、[限定复审](validation/xuanshu-general-layout-independent-review.md)

## 当前公共接口索引

| 公共头 | APP/宿主使用的真实入口 | 初始化及寿命要点 |
|---|---|---|
| [application.h](../include/ui_framework/application.h)、[ui.h](../include/ui_framework/ui.h) | ui_app_query_v1、workspace、host、内容与实例回调 | APP拥有模型，宿主拥有框架对象，context借用；当前API10/ABI1 |
| [capability.h](../include/ui_framework/capability.h) | ui_capability_register_resource、ui_capability_register、ui_capability_bind、ui_capability_prepare、ui_capability_dispatch、ui_capability_complete、ui_capability_get_result、ui_capability_forget | v1完整size/version=1；元数据复制、回调应用持有；get_result输出借用至forget |
| [capability_entry.h](../include/ui_framework/capability_entry.h) | ui_capability_enable_read_entries、ui_capability_publish_read_entry | C1生成同ID read入口，固定资源和代次 |
| [capability_v2.h](../include/ui_framework/capability_v2.h)、[capability_ui.h](../include/ui_framework/capability_ui.h) | ui_capability_register_v2、ui_capability_complete_v2、ui_capability_get_result_v2、ui_capability_ui_bind | v2结构version=2，UI绑定结构version=1；begin/finish两阶段解绑 |
| [capability_inputs.h](../include/ui_framework/capability_inputs.h)、[capability_forms.h](../include/ui_framework/capability_forms.h)、[capability_component_inputs.h](../include/ui_framework/capability_component_inputs.h) | ui_capability_input_bind、ui_capability_form_bind、ui_capability_component_input_bind | 业务参数由正式控件收集，来源及回调退栈由框架门约束 |
| [capability_edit.h](../include/ui_framework/capability_edit.h)、[capability_edit_extensions.h](../include/ui_framework/capability_edit_extensions.h) | ui_capability_register_edit、ui_capability_edit_complete、ui_capability_register_edit_extension | 独立事务结构version=1，内嵌v2；executor_exited由实际执行者退出事实支持 |
| [ai_service.h](../include/ui_framework/ai_service.h) | ui_ai_service_create/open/poll/request_close/destroy、ui_ai_query_run | 同一worker及存储，OK收到与committed接纳分开，关闭PENDING继续poll |
| [ai_model.h](../include/ui_framework/ai_model.h)、[ai_model_view.h](../include/ui_framework/ai_model_view.h) | 模型配置、实际预览及显示绑定 | 宿主管理原生凭据、逐任务可信确认和实际完整读回 |
| [ai_edit.h](../include/ui_framework/ai_edit.h)、[ai_edit_ui.h](../include/ui_framework/ai_edit_ui.h) | ui_ai_edit_enable/propose/get_state、受控显示绑定 | 原P4状态/审批/撤销门，终态与保存事实分别查询 |
| [ai_coordinator.h](../include/ui_framework/ai_coordinator.h) | ui_ai_agent_profile_register/unregister、ui_ai_coordinator_enable、ui_ai_graph_answer | Profile是复制的角色数据，能力/目标/根范围取交集 |
| [ai_assistant.h](../include/ui_framework/ai_assistant.h) | ui_ai_assistant_enable/catalog/propose/get/get_disclosure/control、ui_ai_assistant_bind_light_display、ui_ai_assistant_bind_node_undo | open前固定根策略，展示代次及末次回调重检，初始/计划/撤销独立适用确认 |
| [app_data.h](../include/ui_framework/app_data.h)、[app_poll.h](../include/ui_framework/app_poll.h) | ui_workspace_post_app_data、ui_app_set_poll_callback | worker复制数据/稳定ID到UI，poll回调不替代实际worker退出 |

表中带斜线的函数族可在对应头逐项查看完整签名。capability.h与ai_service.h顶部的阶段范围注释分别说明该独立层，后续模型、写入及专业角色通过各自头扩展。JSON分配由所属DLL release接口归还。注册/使用/取消/注销及错误处理的完整顺序见接入指南。

## SDK、授权及应用责任

匹配SDK是公共头、导入库、DLL及宿主的具体文件集合。API10相同仍须核对新增导出和交付清单，旧冻结头/库保留其原能力范围。资源固定实例、host/open epoch、revision及scope；活动标签与新草稿变化不改已接纳任务。

本地提案及保存不触发模型发送。模型和专业续轮受宿主根策略约束，请求上限、期限和费用取具体策略。完整摘要不能替代完整请求显示，隐藏/失败/旧代次/解绑PENDING使旧资格失效。应用仅提供真实数据、业务check/execute及事务，Service、Provider、可信确认及存储由正式宿主统一管理。

## 历史协议设计与分阶段记录

以下保留2026-10-22至10-27的设计、当时“Proposed/未实现”状态及后来逐阶段增补。其日期限定原范围，当前开发入口采用上面的公共头索引及当前应用矩阵。第4节仍是v1只读子合同。

> 2026-10-26最新状态：本轮先冻结[P4A-1](xuanshu-p4-controlled-edit-contract.md)，已完成P3租约收口及P4-A单资源夹具实现；Native44/44、Light49/49、ASAN44/44，限定为**P4-A开发阶段模拟及自动验收通过**。见[P3独立复审](validation/xuanshu-p3-independent-review.md)、[本轮验收/身份/首次失败](validation/xuanshu-p4a-validation.md)。整体架构仍Proposed，P4-B/C、正式面板、应用接入和真实人工均未完成；下文历史状态保留。


> 2026-10-24 P3增补：[宿主Service](xuanshu-p3-service-contract.md)仅消费同一目录的read发现/describe及P2 v1 prepare/dispatch/结果/forget，按固定目标执行，不新增应用业务执行器，不回退legacy或产生人工凭据。v1结构/错误及C2非read人工合同不变；Service独立队列不占用/提高P2预算。实际counter模型回读、修改拒绝、revision及DLL收尾见[P3独立验收](validation/xuanshu-p3-validation.md)。P4 AI写入/审批/事务未实施。

状态：Proposed（整体）；已确认决策与A+B只读子集见第9节，C1生成只读入口的实施合同见第10节。协议草案版本：1。日期：2026-10-22。其他章节按标记保留后续规划。

配套：[源码审计](xuanshu-native-code-audit.md)、[服务与执行设计](xuanshu-native-integration-design.md)。后续“必须”约束相应阶段；现存旧命令/Assistant行为与首批API10只读实现分别以源码为准。

## 1. 目标、注册权威与迁移

应用面向用户的业务功能必须有可发现的语义能力；UI和AI调用同一业务实现。完整覆盖不等于把任意鼠标移动、每次paint、内部诊断和所有控件setter暴露给模型。目录把功能分为业务、视图/草稿、用户设置和诊断四类，诊断默认不向普通Agent开放。

未来注册的唯一来源是应用能力描述。框架从它生成普通命令派发记录、Assistant兼容视图及UI参数绑定；应用不再手工注册第二份Assistant描述。菜单布局、标题、快捷键仍可用现有UI描述，但只能引用已注册的能力ID或纯框架视图操作。

P2同时迁移五个框架维护包：minimal_eda、web_counter、generic_components、framework_features、stateful_components。framework_features与generic_components共享源码，按编译分支分别核对覆盖。独立布局示例只迁移其实际用到的新接口；历史冻结调用方和旧测试夹具保留原义。内置browser及外部业务应用后续逐项迁移，不把“维护示例已迁移”写为“全部应用已迁移”。

未迁移应用走标记为legacy的适配：只能发现已有Assistant允许项，不推断缺失Schema、输出或撤销语义；没有足够验证合同的写入不开放给新AI。旧API仍可能存在不等于恢复历史SDK兼容承诺。迁移后同一host上禁止应用再次手工注册同ID普通/Assistant命令，返回重复注册错误。

## 2. 能力描述与发现

### 2.1 描述字段

| 字段 | 合同 |
| --- | --- |
| id、version | 稳定命名空间ID及单调协议版本；语言/菜单路径变化不改ID；参数或效果语义变化升version |
| name、description、category | 人可读名称、用途及分层分类；描述不承担权限授予 |
| input_schema、output_schema | 本节Schema方言；注册时校验并计算descriptor_hash；错误有独立统一envelope |
| target_kind | document、instance_view、user_settings或workspace；一次调用最多写一个资源，跨资源拆步骤 |
| required_scopes、effect、risk | 访问范围；read、view、draft、write、external或diagnostic；只读仍可能含敏感数据 |
| check | UI线程检查实际目标、版本、关闭/模态/busy/只读及业务前提；返回稳定原因码 |
| execute、cancel、execution_mode | UI短操作或异步协作；取消是否可用及其安全边界；不得标记强制终止线程 |
| transaction_mode、undo_mode | none或application；应用实现及其适用资源；不可用必须如实声明 |
| bindings | UI入口ID、参数收集器、能力ID和结果投影；人工入口也进入统一业务派发 |

可用状态分两次检查：发现时的提示，以及拿到资源写入权后、真实执行前的权威检查。发现结果不能作执行许可。UI disabled、busy、modal必须映射进check；不能仅依赖当前ui_host_invoke的部分拦截。

### 2.2 Schema方言与数据限制

采用JSON Schema形状的明确最小子集 `ui-capability-schema-v1`，不宣称实现完整标准。已确认支持单一type、properties、required、additionalProperties:false、items、min/maxItems、min/maxLength、minimum/maximum、primitive enum/const。对象必须明确additionalProperties:false，数组必须有items；required只能引用已定义字段。title、description及examples仅作注释。组合、$defs/$ref、外部引用、format/pattern及未知关键词注册时拒绝。长度按Unicode标量计数，字节限额另计。

UTF-8有效性、重复JSON键、非有限数、尾随字节、深度及长度由框架检查。嵌套最多16层、节点4096、单数组最多512项、单对象128字段。整数精确范围不超过2^53−1；uint64、行ID、实例ID及业务大整数在线上用十进制字符串，应用validator验证实际取值范围。不得先转double再恢复。revision是应用提供的不透明字符串，模型不得自行递增。

每实例最多256项能力，复制后的目录总预算2MiB；每份Schema最多64KiB；输入最多64KiB、内联成功输出最多256KiB。全部包含UTF-8字节、排除末尾NUL；目录预算包括名称、描述、两份Schema和绑定。超限明确失败，较大结果使用有授权和寿命的产物引用，不能静默截断为成功。这些是新能力层建议预算，不提高现有组件、图片、DOM或JS预算。

注册时结构/Schema有效不表示业务参数有效。调用时先按Schema校验，再由应用check/业务validator验证对象存在性、单位、只读、文档前提和资源边界。输出在发回模型前按output_schema验证；已发生业务修改但输出非法时，返回结果未知并停止该资源后续AI写入。

### 2.3 分类与按需发现

主Agent初始只获得宿主通用操作及目录发现工具，按app/resource/category分页查询摘要；需要时按稳定ID取得完整Schema。摘要含版本、效果、风险、可用原因与catalog_generation，不附完整数据快照。每页最多32项。

模型不能任意指定尚未获得范围的实例或文件；目录查询也经过scope过滤。目录变化使旧descriptor_hash失效，排队调用重新校验；删除/关闭后拒绝新调用。子Agent目录是根任务授权范围的子集，不继承全部主目录。

## 3. 目标、审批与业务执行

### 3.1 固定目标

每次调用携带host_epoch、instance_id、resource_kind、resource_id、open_epoch、expected_revision及scope。resource_kind限定document、instance_view、user_settings、workspace；非文档操作也必须有可检查的资源revision。scope中的对象集合、读写范围和路径许可由框架绑定，模型提出的目标只是候选。

host_epoch防进程重启后旧ID复用，open_epoch防文档关闭重开后误指；文件路径、标题、当前选中项均不能单独充当文档身份。应用负责解析resource_id和实际revision；未提供该合同的业务写入不可开放。写入权按宿主内稳定资源身份分配，跨实例指向同一文档必须返回同一resource_id。

host_epoch/open_epoch使用36字节UUID文本；resource_id为1..255字节的宿主内全局不透明ID，revision为1..127字节的不透明UTF-8；scope_json最多4KiB，scope对象ID仍须遵守数组和总字节双限。资源比较使用登记后的精确字节，不由模型规范化路径或改写ID。应用内部资源采用应用命名空间；共享文档必须登记同一权威ID和revision来源，否则只允许只读接入。每次文档重新打开产生新open_epoch；同一打开文档的多个视图共享该epoch。

调用先取得框架分配的call_id，复制参数/目标，预建关联记录，再排队执行。标签切换只改变未来默认目标提示；排队、运行、审批、回调均保留原绑定。业务执行时再次验证版本，冲突返回STALE，不能自动重定向到活动文档。

新任务可以绑定多个目标供跨应用步骤使用；某个步骤只写一个资源。每次成功写入得到新的revision，后续步骤由服务按已确认结果更新自身预期。任何人工修改也走同一资源写门和revision更新，不得让AI队列之外的编辑绕过并发检查。

### 3.2 权限与审批

宿主路由器确定origin、principal、root/task/call及scope；不信任参数中的source、approved、role或permission。旧host的source字符串保留诊断意义，不转成授权票据。同进程合作模块可遵守此规则，但它不防御恶意C模块直接破坏内存。

只读在已有读取授权内执行。普通可撤销编辑须有明确的用户任务授权，授权绑定资源、能力类别、允许参数范围、过期时间和policy_revision；没有授权则请求确认。破坏性、外发及不可撤销操作逐次展示目标、参数和后果。scope许可和审批都满足才能执行，审批不能绕过业务check。

审批记录绑定principal、task/call、host/open epoch、资源与expected_revision、capability id/version/descriptor_hash、规范参数hash、scope、policy_revision、期限和单次nonce。执行前一次性消费并持久化；目标/参数/版本/政策变化或过期后生成新审批。拒绝后不派发。只取消等待审批不意味着撤销此前步骤。

已确认参数与描述指纹采用RFC8785/JCS规范UTF-8字节的SHA-256小写hex，替换原capcanon-v1建议。对象键按UTF-16代码单元排序，数组保序，不做Unicode归一化；负零规范为0，数字使用ECMAScript binary64规则。JCS数字按binary64解释；integer Schema限制安全整数范围，uint64身份始终传十进制字符串。yyjson普通writer不是JCS实现，框架使用独立字符串、键排序和数字格式适配。依据：[RFC8785](https://www.rfc-editor.org/rfc/rfc8785.html)。

审批绑定和descriptor_hash先组成字段固定的JSON对象，再用同一规则计算；descriptor只含可序列化元数据及Schema，不含指针。不能使用当前uj_get片段读取或普通JSON文本拼接计算安全hash。UI展示解析后与实际执行一致的参数，并提供不截断详情。P2须提供键序、转义、Unicode、-0、指数记法及浮点边界的跨实现向量；向量不一致则不派发写入。

### 3.3 执行与结果

调用顺序为复制/关联→结构校验→scope检查→应用check→审批→取得资源写门→再次check/revision/审批绑定→记录dispatch意图→业务执行→结果记录→UI/模型投影。等待审批期间不占文档写门；取得写门后不泵嵌套UI等待确认。存储确认期间保留逻辑写门，以非阻塞状态机推进，不能阻塞UI线程等磁盘。

应用拥有文档、文件、事务和撤销数据。框架只调度begin/commit/rollback/undo并核对回执，不自动创建撤销栈。相同文档写门一直保持到异步业务操作和结果核对完成；不允许只保护初次handler调用。

结果区分请求状态与修改效果：

| 字段 | 值与意义 |
| --- | --- |
| status | ok、rejected、failed、cancelled、stale、unknown |
| effect | none、applied、rolled_back、unknown；failed/cancelled也可能applied |
| target/revision_before/revision_after | 原资源身份及应用确认的实际版本；不以任务名推断 |
| output | 仅ok且符合output_schema时作为正常模型工具结果 |
| error | 稳定code、可读message、可安全披露的details；不放密钥 |
| transaction_id/undo_id | 仅应用确认可用时提供，带资源与版本约束；不是跨应用ID |
| operation_id/reconciliation | 应用操作回执及核对办法；没有核对能力时写明unsupported |

重复call_id提交或完成不重复执行/产生第二终态；同ID不同参数拒绝。请求是否被接纳由函数返回值判断，业务结果由完成记录判断。取消是协作请求，原子短操作可已提交；必须回报实际effect。超时和通信丢失不证明未修改，unknown会锁住该资源后续AI写入，等待独立回读/应用回执或人工解决。其他资源可继续只读，不全局伪造回滚。

## 4. API10 首批只读 C 接口

源码已实现的唯一公共声明为 [capability.h](../include/ui_framework/capability.h)，新结构完整size/version=1，旧结构布局不变。应用使用context.host；目录归host所有，workspace在create之前绑定实际instance_id。独立host使用框架分配的实例身份。host_epoch是进程UUID，资源重新登记默认产生新open_epoch。

已实现register/resource、两阶段unregistration、discover/describe、resources/bind、prepare/dispatch、complete/cancel/get_result/forget和quiesce。只接受effect=read；没有业务写入、事务、审批、UI binding或AI Service公共入口。其他效果返回UNSUPPORTED，不靠调用者source字符串判断权限。

prepare复制完整目标、参数和目录代次，先建立调用ID与结果记录，不执行应用代码。dispatch可同步完成，重复派发不重复执行；执行前检查modal、关闭、目录代次、业务check和revision。调用源是当前应用的程序调用，v1 scope固定为read，不代表通用ACL、用户任务授权或AI票据。

所有host操作及应用回调限创建UI线程。worker只经既有app_data复制投递workspace+instance+call ID，UI接收器complete(host,ID,result)；不保存host/target/回调到worker。complete复制输入，成功输出校验并再次核对revision。取消运行中调用只置请求并调用可选cancel，不能假造终态。本轮只跟踪能力调用和UI回调引用，不自动发现应用任意线程。应用必须确认本次worker/外部回调真实退出后才complete；投递结果不是退出证明，UI应非阻塞检查就绪。同步complete后，外层execute尚未退栈时不能forget或完成注销。

API返回值描述接纳/接口错误；get_result中PREPARED/RUNNING/FINISHED和outcome描述调用结果。读取结果的effect固定none。重复terminal complete返回ALREADY_EXISTS，忘记后的ID返回NOT_FOUND；没有可重复解引用的裸call指针。未知/过期目标prepare返回NOT_FOUND；派发revision不相等才记STALE/revision_conflict；业务check拒绝记REJECTED/unavailable，不把业务校验错误误称版本冲突。输出非法记FAILED/invalid_output，完成时revision读取失败记FAILED/revision_unavailable。细分code不扩展旧ui_status_t。

目录和schema字符串深复制；函数与user_data由应用拥有，finish_unregistration成功前必须存活。describe/discover/JSON输出由框架分配，用ui_capability_release释放。get_result.output_json借用至forget或host销毁。目标结构是调用方拥有的完整固定数组副本。回调参数仅借用至返回。

每host最多256资源、256能力，资源和描述复制量合计2MiB。描述包含两份Schema和指纹元数据。另设1024条/8MiB调用记录预算，含目标、复制输入及结果；超限在接纳前拒绝，完成结果预算不足保留RUNNING并返回LIMIT_EXCEEDED供生产者保留结果、待释放记录后重交，不丢完成。Schema每份64KiB，发现每页最多32项；generation=0开始扫描，目录变化要求重新扫描。

注销第一步封闭发现及新调用，取消prepared并请求running取消；引用未归零或模块回调未退栈时finish返回PENDING。workspace关闭使用独立可撤回门并等待在途调用；收尾app_data继续交付。WAIT后REFUSE重新接纳新调用，不复活已取消调用；显式quiesce仍不可逆。独立host先quiesce，busy时ui_host_destroy不释放对象，完成后重试。不得从create/mount失败路径遗留异步任务，沿用原应用生命周期要求。

完整写入合同、UI绑定、跨实例共享写门及审批仍属后续建议，不能从首批只读接口推断已经实现。

## 5. UI绑定、草稿与专业画布

| 情形 | 框架负责 | 应用负责 |
| --- | --- | --- |
| 菜单/按钮/快捷键 | 根据绑定解析到同一能力和参数；来源写为manual；执行check | 共用业务函数，不能另有AI分支算法 |
| 标准表格/树/列表 | 虚拟查询、稳定ID、选择/滚动/排序视图适配；保留generation和按需窗口 | 返回真实total；完整源排序；解释对象ID及实际数据修改 |
| 表单/对话框 | draft.read/set/reset、校验、显式submit参数快照 | 验证完整业务参数、执行事务；成功后回填并accept_fields |
| 状态/标题/提示 | 显示投影；表明view效果 | 不把set_text/set_title当作业务修改成功 |
| 画布手势 | 转发语义提交，提供坐标系/选择范围元数据 | 命中对象、稳定ID、单位/变换、几何/约束、业务参数及undo |

视图操作明确标记view/draft；AI改draft不会自动submit。提交必须将全部业务参数冻结到请求，不能让执行handler再从“当前选中对象/当前draft”偷读可变目标。用户菜单操作也采用相同快照。成功后通过业务回读更新UI，不能先改控件再伪造文档成功。

应用若仅有UI编辑器而没有业务模型，覆盖表必须标“UI-only/待接入”，不能生成假的document.update能力。对话框仅收参数，能力不要求对话框可见；鼠标手势能完成的业务修改应能用坐标系、对象和参数描述，AI不需要模仿鼠标。

## 6. 首个试点完整功能矩阵

下表盘点本基线实际入口及隐式控件行为。能力ID均为本次建议；“既有依据”列的测试仅存在于源码，本轮未执行。P2新增验收编号在下一节定义。

| 功能 | 现有UI/命令入口 | 建议能力ID | 当前实现与缺口 | 验收依据/状态 |
| --- | --- | --- | --- | --- |
| 表格按需读取/64列/100000行 | table/source | state.table.read | api7_fixture.source生成固定行；不是可写文档 | api7_integration；AC01，待适配 |
| 单元格编辑 | test.edit | state.table.cell.set | 仅edits++，回查仍editable/42；无真实写入 | api7_fixture.command；AC02，业务缺口 |
| 行选择/范围多选 | test.select及组件状态 | ui.selection.read/set | handler无业务分支；选择由组件维护 | component_scroll；AC03，通用适配 |
| 完整源排序 | test.sort/query.sort_direction | state.table.sort | source按查询方向映射row_id；不是handler排序 | source；AC04，通用+应用适配 |
| 纵横滚动/列定位 | 标准滚动条/键盘 | ui.viewport.reveal | 按需窗口和稳定ID；不应一次取全表 | component_scroll/widths；AC01/03 |
| 列调宽/单列重置 | 拖动、state.reset-column | ui.table.columns.set/reset | 公共列宽API确有状态 | stateful_components；AC05 |
| 树展开/收起/读取 | tree/source | ui.tree.expand/collapse、state.tree.read | 20个根+每根一个子节点，不能误写十万树节点 | api7_fixture.source；AC01 |
| 异步缩略图 | tree/source→worker | state.thumbnail.read | 固定RGBA测试图；真实worker和generation | api7_integration；AC06 |
| 表单name/mode/color草稿 | 标准编辑/枚举/RGBA | ui.form.draft.read/set/reset | 控件draft，不是业务对象 | components_core；AC07 |
| 表单提交 | test.submit | state.properties.apply | 只accept_fields；需应用模型保存name/mode/color并回读 | command；AC02/07，业务缺口 |
| 对话框打开 | State/Edit properties→test.modal | ui.dialog.open | 仅显示实例模态 | component_layout；AC08 |
| 对话框name编辑/关闭 | test.modal.close为submit | ui.dialog.close、state.properties.apply | 当前提交只关闭，未调用accept_fields；不可记为业务保存 | command；AC07/08，业务缺口 |
| GL刷新及图像读取 | test.render | state.viewport.render/read | 实际OSMesa frame→图片；不是专业业务renderer | api7_integration；AC06 |
| 图片释放 | test.image.release | diagnostic.image.release | 诊断资源操作，默认不授普通Agent | api7_integration；AC06 |
| 迟到图片注入 | test.late | diagnostic.thumbnail.late | 测试代次失效，默认不授普通Agent | api7_integration；AC06 |
| 布局重置 | test.reset、state.reset | ui.workspace.layout.reset | 后者同时重置列宽；应合并成明确参数操作 | workspace_layout/stateful；AC05 |
| 停靠/浮动/标签/折叠/调尺寸 | 公共shell交互 | ui.workspace.layout.get/apply | 框架布局状态；复用原槽/GL，不重mount | workspace7/layout；AC05 |
| 界面状态保存 | State→state.save | state.view_state.save | UST1列宽+布局；不含表格业务值/完整文档 | stateful_components；AC09 |
| 界面状态恢复 | State→state.load、mount自动load | state.view_state.load | 格式/档案检查；不是AI任务恢复 | stateful_components；AC09 |
| 标题更新 | State→state.title | state.instance.title.set | handler读取name draft；新参数须显式title | workspace_titles；AC10 |
| 跟随系统主题 | State→state.theme-system | ui.theme.set | 当前host主题及图标通知 | theme_core；AC10 |
| 中文/英文及偏好存储 | language.zh/en | state.language.set | UIL1文件；运行语言可能成功而保存失败，须返回部分效果 | instance_language；AC09/10 |
| 状态、标题及字段翻译 | language/theme callbacks | 查询能力的显示投影 | 无独立业务副作用；不能授权任意set_text假成功 | language/theme；AC10 |
| A/B档案锁及schema变体 | 启动环境/创建 | 启动配置，不默认开放Agent工具 | acquire_profile/schema_variant；属于构造和诊断条件 | stateful_components；AC09 |
| 创建/挂载失败与关闭 | 生命周期入口 | 宿主关闭能力，非绕过应用回调 | stop/WAIT、join、GL释放及module_shutdown | api7_integration；AC11 |
| EDA对象只读枚举 | 画布/摘要 | eda.blocks.read | 当前只有数组及count，无公开稳定对象ID | minimal_eda；AC12，补语义适配 |
| EDA加矩形 | 菜单/工具/A→eda.add_block | eda.block.add | 固定坐标策略、上限64；不能声称可任意建几何 | command_add；AC12 |
| EDA缩放 | 菜单/工具/Z→eda.zoom_in | eda.view.zoom_in | ×1.1，最大4；视图效果，不能误升文档revision | command_zoom；AC12 |
| EDA清空 | 菜单/工具/C→eda.clear | eda.document.clear | 清块并重置zoom；Assistant为危险操作 | command_clear；AC12/13 |
| EDA笔记 | 属性面板原生EDIT | eda.note.read/set | 只有控件文本，未进入业务state/保存/undo | properties_proc；AC12，UI-only缺口 |
| EDA保存/撤销 | 无 | 不登记为已支持 | 当前无文档文件、事务栈；P4必须补合同后才开放受控编辑 | assistant_lifecycle只作参考；AC13 |

试点不会把固定fixture改称真实业务工程。P2需为stateful增加应用私有的最小字段/行修改状态及revision适配，重查询能看到修改；保持原测试夹具“计数模拟”的既有断言，必要的共享场景拆分只在未来P2授权内评审。已有UST1不扩写为文档格式。P4再补真实应用事务/撤销；P2证明manual与能力调用的一致性，不能提前宣称AI编辑已开放。

### 6.1 最小接入示例

建议 `state.properties.apply` 接受完整参数快照，不能只传“点击提交”。下面是未来能力输入，不是当前可运行命令：

```json
{
  "type": "object",
  "properties": {
    "name": {"type": "string", "maxLength": 1024},
    "mode": {"type": "string", "enum": ["low", "medium", "high"]},
    "rgba": {"type": "string", "minLength": 9, "maxLength": 9}
  },
  "required": ["name", "mode", "rgba"],
  "additionalProperties": false
}
```

应用validator再检查rgba为 `#RRGGBBAA`。UI提交收集当前草稿形成同样JSON，调用共同业务函数。应用成功保存私有业务值、更新revision后，投影已提交值并按本次快照选择性确认字段；其他或较新的草稿保留并标冲突，禁止调用accept_fields一并接受当前草稿。失败保留草稿和原文档。AI显式提交同样参数，结果必须与人工提交后的业务回读一致。无持久化时返回in-memory效果，不写“已保存到文件”。

以下为当前API10只读注册用法；resource须先登记。execute由应用提供，完整可编译生命周期实例见[只读夹具](../tests/capability_fixture.c)。上面的写入输入仅是后续建议，不可在首批read接口中伪装执行。

```c
#include "ui_framework/capability.h"

static ui_status_t register_properties_read(
    ui_host_t *host, const char *output_schema,
    void (*read_execute)(uint64_t, const ui_cap_target_t *, const char *, void *),
    void *state)
{
    ui_capability_desc_t d = {0};
    d.size = sizeof(d);
    d.version = UI_CAPABILITY_CONTRACT_VERSION;
    d.capability_version = 1;
    d.id = "state.properties.read";
    d.name_utf8 = "Read properties";
    d.description_utf8 = "Read the bound resource without applying drafts";
    d.category = "document/properties";
    d.input_schema_json = "{\"type\":\"object\",\"additionalProperties\":false}";
    d.output_schema_json = output_schema;
    d.target_kind = UI_CAP_DOCUMENT;
    d.effect = "read";
    d.execute = read_execute;
    d.user_data = state;
    return ui_capability_register(host, &d);
}
```

P2示例为none事务/撤销并不因此开放AI写入；它用于人工/能力路由一致性。P4实现真实事务和undo后更新描述/version、重新验收，再按任务授权放开普通可撤销编辑。不能仅把none改成application就视为已有撤销。

## 7. 迁移与验收门槛

| 编号 | 必需场景与通过条件 | 首次阶段 |
| --- | --- | --- |
| AC01 | 表/树首尾与完整total，按需窗口，无全量预加载；大ID精确往返 | P2 |
| AC02 | 人工入口与能力入口修改同业务值；重新query/重绘后仍一致；计数/文本伪成功必须失败 | P2 |
| AC03 | 选择、过滤/排序后稳定对象与范围；标签切换不改提交目标 | P2 |
| AC04 | 排序覆盖完整源，旧generation数据拒绝；不能只排缓存 | P2 |
| AC05 | 列宽/布局view revision、原槽身份、保存恢复；业务revision不被paint改变 | P2 |
| AC06 | 异步取消、迟到消息、队列预算、图片寿命及真实GL结果；诊断项不可被普通发现 | P2/P3 |
| AC07 | draft/set/reset与业务submit分开；非法/只读字段无写入；一次提交一次业务效果 | P2 |
| AC08 | 模态前提与参数快照；隐藏对话框也能执行合法语义提交，不绕过目标检查 | P2 |
| AC09 | 已有UST1/UIL1/档案锁原合同；失败返回实际部分效果，不宣称业务文档已持久化 | P2 |
| AC10 | 标题/主题/语言有明确参数和target；中英文ID不变；切换语言不重注册目录 | P2 |
| AC11 | 排队/运行/审批期间关闭；WAIT继续收尾；worker及callback全退后DLL才卸载 | P3，P5扩展 |
| AC12 | EDA菜单/快捷键/语义入口的数组、zoom、笔记回读一致；固定加块不冒称任意几何 | P2 |
| AC13 | 危险清空审批，文档版本冲突、真实commit/rollback/undo及未知结果停写 | P4 |
| AC14 | 重复提交/完成、同步完成、乱序/迟到结果、输入/输出Schema失败和关闭后重开 | P2/P3 |
| AC15 | 审批内容被改、过期/重放/跨资源、人工并发修改及子Agent越权均拒绝 | P4/P5 |
| AC16 | 五个维护包功能目录对照无漏项；共享源码不同编译分支分别验收 | P2 |

新增功能必须同时交付能力描述、UI绑定、真实业务实现和验收。覆盖状态只能是已映射、待适配、缺业务实现或明确不适用；不允许因为没有能力ID就从功能清单删除该功能。原P0/P1只有静态设计检查；本轮仅运行第9节列出的只读子集，不宣称以上AC场景已经全部验收。

## 8. 评审事项与差异

本案建议P2集中进行下一版公共API/标准升级，独立新头文件，不修改已有结构布局、应用ABI和包格式；具体版本决议见集成设计。当前源码升级为SDK0.10-dev/API10/标准10，ABI1/包1与冻结SDK不变。

相对原草案，本协议增加非文档资源revision、UI-only/诊断分类、Schema明确子集、输出校验后的未知效果、实例跨文档共享写门及两阶段注销。原因是现有UI状态不等于业务模型、source不是可信权限、C异步回调会跨越卸载边界。

整体合同仍为Proposed；本轮只授权并实现A批+B批只读子集，其他接口/场景仍需后续授权。

## 9. 2026-10-22 A批+B批实施 checkpoint

已确认：D01版本；D02后续内存模型回读且无新文件格式；D03原生SQLite/Windows凭据方向且P2不引入；D04最小Schema、固定yyjson0.13.0、JCS+SHA256及已列P2限额；D05合作同进程边界；草稿保留并显式解决冲突。Agent预算/并发数值未接受，仍属后续建议。

本轮源码子集与测试见 [JSON实现](../src/capability_json.c)、[只读目录与调用](../src/capability.c)、[JSON用例](../tests/capability_json.c)、[调用用例](../tests/capability_core.c)、[workspace用例](../tests/capability_workspace.c)。本轮运行结果单独记录在 [A+B验收](validation/xuanshu-p2ab-validation.md)，不是P0/P1静态检查或历史测试。

UI binding、第6节完整试点矩阵、业务状态适配、五个示例迁移、写入、审批/事务/撤销、AI Service和P3—P6均未实现。第6.1节读取注册使用当前首批接口，写入JSON和UI说明仍为后续建议。

评审收口：源码在最后一次get_revision/check相关回调返回后、RUNNING之前再次检查执行门；回调新打开modal时不执行，仍使用REJECTED/unavailable语义。真实DLL自有worker/受控回调、同步完成外层栈、复制队列达到8MiB后的应用重投，以及旧实例消息不能命中相同call ID的新实例，已完成限定验证。公共结构、版本、取消和两阶段注销合同保持；具体结果及独立未定位失败见[收口记录](validation/xuanshu-p2ab-validation.md#7-2026-10-22评审收口独立新证据)，不表示完整P2完成。

## 10. C1已实现：生成只读兼容入口

### 10.1 独立接口与错误

[公共头](../include/ui_framework/capability_entry.h)新增`ui_cap_entry_binding_t`（version1，Windows x64 size24），不改变任何v1结构：

| 接口 | 合同 |
| --- | --- |
| `ui_capability_enable_read_entries(host)` | UI线程；显式、不可逆启用只读兼容模式；旧请求或callback尚在途时初次返回PENDING，关闭时CANCELLED |
| `ui_capability_publish_read_entry(host,binding)` | binding只引用capability_id和完整target快照；copy，不借用调用方指针；未启用模式PERMISSION_DENIED，重复发布ALREADY_EXISTS，目标身份不匹配NOT_FOUND，非法结构INVALID_ARGUMENT，目录满LIMIT_EXCEEDED |
| `ui_capability_request_call(host,request_id,&call_id)` | 关联在execute前可查；终态仍保留到forget；prepare失败或释放后NOT_FOUND。随后使用原get_result/cancel/dispatch/forget |

不新增权限枚举、source认证、业务执行回调或写入入口。调用都在UI线程；宿主之外的线程不能直接持host/target调用上述函数。输出、Schema、JCS、资源与两阶段注销仍按v1执行。发布推进目录代次，旧分页/旧prepared按原合同失效；UI标题翻译只是显示投影，不修改capability_version或descriptor_hash。

### 10.2 注册及旧入口规则

1. 新宿主中只注册一次能力。其ID与所有旧普通/Assistant注册双向检查，退休未finish期间仍占用ID。迁移需在应用源码移除同ID两次注册；没有自动覆盖或legacy fallback。
2. 注册资源并bind明确目标后publish；一个能力一个同ID入口。普通命令投影借用权威ID/Schema且不接收自定义handler，Assistant动态读取同一记录；晚创建、动态新增及注销使用同一生命周期。
3. 在启用模式的宿主，Assistant只发现/执行生成的read入口；legacy READ隐藏，legacy EDIT/DESTRUCTIVE直接invoke拒绝，不调用旧validator/confirm/handler。提高max_permission、参数/source自称human均不能恢复修改。
4. 普通legacy UI修改入口仍可用；未启用模式的legacy宿主保持旧协议。本批不将这些入口导入AI，也不建立C2人工来源证明。五个维护示例未迁移，不能据此宣称其修改旁路已全部消除。
5. 绑定固定instance/open_epoch/resource/revision/scope；发布不查询活动标签。revision冲突返回原STALE/revision_conflict；资源重开不能自动更新已发布快照。更新绑定须两阶段注销该能力后重新注册/发布。

### 10.3 接入与验收

```c
/* 在已注册资源和read能力、且无旧请求在途的UI线程执行。 */
ui_cap_target_t target = {0};
ui_cap_entry_binding_t binding = {0};
target.size = sizeof(target);
target.version = 1;
/* 每一步必须检查实际status；以下只展示调用顺序。 */
ui_capability_bind(host, "document-a", &target);
ui_capability_enable_read_entries(host);
binding.size = sizeof(binding);
binding.version = UI_CAPABILITY_ENTRY_VERSION;
binding.capability_id = "read";
binding.target = &target;
ui_capability_publish_read_entry(host, &binding);
/* 普通ui_host_invoke或ui_assistant_invoke使用相同的read ID。
 * result observer中查询request_call，随后按宿主所有权规则释放。 */
```

旧invoke返回非零/OK表示接收请求；Schema/业务前提等失败通过失败result投影，不能当作业务成功。prepare前失败输出`{"status":...}`且无call关联；调用终态成功投影验证后的原output，失败为`{"code":...}`。完整state/outcome/revision/cancel_requested仍从get_result读取。结果回调内forget为PENDING；独立调用方退出回调后显式forget，workspace Assistant在下一安全poll自动回收。公共reply对生成请求返回PERMISSION_DENIED，防止绕过输出校验。

专用[入口用例](../tests/capability_entry.c)验证ID冲突、代次、动态视图、metadata与UI标题分离、目标复制、Schema/disabled/busy/modal/check/revision/关闭、同步关联、异步取消、重复及非法输出、资源重开、在途退休、Assistant销毁、1024条和8MiB背压、2MiB目录预算；实际标签切换、DLLworker/回调/同步栈/复制队列及迟到消息由`ui_capability_entry_workspace`配置复用[真实DLL用例](../tests/capability_workspace.c)验证。具体运行和证据限制见[C1验收](validation/xuanshu-p2c1-validation.md)。

已确认但未实现：C2独立v2人工UI增量属于本目录，AI继续只读；stateful表单/对话框共享业务name但保留各自草稿，冲突显式解决；EDA笔记增加明确提交/取消。第6节完整矩阵、五示例迁移、业务写入及P3—P6仍属后续授权范围。

### 10.4 C1收口补充：激活及失败通知

已实现：旧validate/confirm/visitor等应用回调在途时，初次enable返回PENDING；回调退栈后再试才能激活。框架在派发前再次检查策略，正常read入口不因独立旧回调租约被当成嵌套dispatch。启用后旧Assistant begin/commit/rollback/undo也直接拒绝；这些API不构成可信人工入口。未迁移host及普通人工业务命令保持原行为。

生成pending及command/source副本从分配前到结果observer返回一直计入原8MiB，失败prepare也不例外；共享Assistant跟踪保持1024上限。固定失败槽在同一预算内预留，字节耗尽的首个接收请求可返回原status失败，无call ID。所有生成失败结果的observer中再次提交生成请求，框架在接纳前背压，不分配新桥接节点、不递归通知：普通invoke返回0；Assistant返回LIMIT_EXCEEDED和ID0。退出回调、释放记录后再试；不得把0或LIMIT当成已执行，也不得在observer中自动重试。

新增[收口回归](../tests/capability_closeout.c)验证回调激活、事务旁路、非法参数/调用数/字节耗尽的重入、invalid_output通知、共享额度及排空恢复。原C1、A+B、公共C/C++及真实DLL两个编译分支另作回归，结果独立见[C1验收第7节](validation/xuanshu-p2c1-validation.md#7-2026-10-22-c1评审收口独立证据)。这是既有错误与预算合同的封闭，不新增v2、UI绑定或业务写入。


## 11. 2026-10-23 C2-A子合同已冻结，未实现

本会话已接受C1限定验收；本节只冻结下一批设计，不重写第9、10节源码事实及历史证据。唯一规范为[C2A-1接口与验收](xuanshu-p2c2a-interface-and-acceptance.md)，其中C声明、字段顺序、错误映射、状态/预算及A01—A15为后续实现依据；尚未创建公共头或导出函数。本文整体继续Proposed。

已确认：人工凭据仅由框架内部签发/持有/一次消费，绑定具体能力、资源/open epoch、revision及JCS参数；非read仅可信人工路径，AI/Assistant继续只读。busy明确拒绝，不排队/自动重试/复用凭据。取消不释放写门；未知按资源实例/open epoch阻止后续写入，只有新人工只读核对在worker/回调退出并有业务状态和revision依据后解除。

首批为Windows私有输入链及内置Light一个菜单叶项，配专用内存夹具；拒绝可识别注入、系统/未知来源、程序调用、公开模拟输入和消息自证。不把Windows来源分类等同物理设备证明，不承诺排除UIAccess等全部注入或隔离恶意DLL；保留真实鼠标、键盘验收。其他入口不支持v2非read，不回退legacy；旧人工功能保留原行为但不算迁移完成。

| 调用/目录边界 | C2A-1冻结规则（待实现） |
| --- | --- |
| read | 原v1登记/执行及C1投影；不新增第二read执行器 |
| v2登记 | 仅view/draft/write，独立完整结构；read/external不支持此登记入口 |
| v2发现 | 同一目录的v1/v2元数据，统一ID、代次、预算和注销；发现不授权 |
| v1执行/旧invoke执行v2非read | PERMISSION_DENIED；v1 publish非read为UNSUPPORTED |
| 完成/查询/取消/释放跨版本误用 | UNSUPPORTED且记录不变；内部按版本路由 |
| UNKNOWN释放 | forget_v2为PENDING，仍占原1024/8MiB额度；只读核对复用原call |
| 整个实例安全关闭 | worker/回调排空后拆除引用，UNKNOWN不冒充活跃worker且不记为已解决 |

现有第6节功能矩阵及AC编号保留；C2-A不接入stateful/EDA模型或五示例，不能将其标为已覆盖。stateful共享name/私有草稿及冲突、EDA笔记提交取消、通用UI绑定留后批；UST1/UIL1不扩写业务文档。新增合同采用SDK0.10-dev/API10/标准10、ABI1/包1匹配快照，v1公开布局和read语义、固定依赖/JCS/限额保持。

本轮仅文档授权，C2A-AC-1全部未运行。代码、测试、构建和版本未改；后续实现需单独授权，不自动进入C2-B/P3—P6。

## 2026-10-23 C2-B additive form contract before implementation
C2-A core5679checks/0 and Light DLL compile pass; first new test crash/null read and later busy-code failure preserved with source/binaries. No claim of fullA/physical acceptance. C2B-1 at docs/xuanshu-p2c2b-form-contract.md is now frozen before implementation under continuous user authorization:4independent APIs, typed fields, generation/base snapshots, preserved drafts/explicit conflict actions, modal owner, actual business readback and selective accept; no second executor, no C2-C/sample models. Implement B after this design update, continue proportional self-review/regressions without routine permission requests.

## 2026-10-23 C2-A＋C2-B实施状态（本次授权）

下列是本轮源码状态，前面“仅文档/未实现/未授权”段落保留为当时历史，不再表示当前范围。C1限定验收不重开；整体设计仍Proposed，C2A-1冻结合同及先于实现落盘的C2B-1独立增量保持。当前停在两批联合代码评审，未授权C2-C/五示例业务迁移/P3—P6。

源码已实现：同一registry中的v2 view/draft/write、Windows私有Light菜单语义接纳、一次消费/固定目标/版本/效果、资源busy门、稳定ID/取消/UNKNOWN人工只读核对、共享原预算与生命周期；标准Light FORM/DIALOG收参、字段generation/base snapshot、私有草稿/显式冲突处置、modal所属、真实业务回读及选择性接纳。已绑定表单不再经残留legacy组件命令执行；未绑定组件保持原行为。没有可公开自报人工来源的token或第二业务执行器。

新增公共头capability_v2.h和capability_ui.h共11函数、capability_forms.h四函数；v1头/布局/只读和错误合同、SDK0.10-dev/API10/标准10、ABI1/包1、固定依赖/JCS/SHA-256/资源限额保持。C2-B本地草稿生命周期不冒充完整draft业务迁移。业务模型仅在专用内存夹具，真实stateful/EDA及其他三示例尚未迁移，不将UST1/UIL1当成新业务格式。

本轮独立最终Native17/17、Light23/23、ASAN17/17；C2-A5876/0、B headless212/0、Light模板264/0、新DLL53/0。首次崩溃/断言/编译失败、自审修正、源/二进制身份及未验项见[联合实施验证](validation/xuanshu-p2c2ab-validation.md)。保留原日志和失败匹配二进制，不用最终通过覆盖首次失败。

真实鼠标/键盘正例、OS输入来源动态矩阵、v2真实workspace标签切换仍未验；私有夹具不证明物理来源。同进程合作边界不隔离恶意DLL。原浏览器20失败、workspace20.05秒/20秒门槛和Native输入首次失败继续独立未定位。后续建议是联合代码评审与列明未验的补验，不自动进入下一批。

## 2026-10-23 C2-C输入增量与A/B整改状态

整体Proposed及后续应用覆盖义务保持。[C2C-1](xuanshu-p2c2c-input-contract.md)在对应实现前冻结；独立capability_inputs.h新增input_bind和workspace_input_bind/unbind三个函数。一个权威capability、同一execute、同一ID/Schema/资源/注销/预算；无第二执行器，不改变原v1或A/B结构。

已实现SHORTCUT、TOOLBAR、LIGHT_CONTROL、COMPONENT、SURFACE、NATIVE_BUTTON metadata。目标是复制的资源ref/open epoch，执行时固定revision及规范化参数；失效明确拒绝，不改绑/排队/重试。input context只借用至collect返回，Light当前值复制上限4096字节且计入原8MiB，不能凭控件改变宣称文档成功。read继续v1/C1；view/draft/write只走私有人工来源，external仍无可执行入口。全表单binding与通用组件binding不能重叠成为两个权威。

A/B整改在最后应用回调后检查原始事件/guard/modal/关闭，所有失败消费资格；本地冲突/取消回读后也复查所属。公开模拟、程序dispatch、source、页面消息不授权。真实workspace双实例夹具证明旧call不随活动标签改向，chrome依据已投影实例，旧实例迟到消息被拒。

本轮自动Native22/22、Light30/30、Native-ASAN22/22，详细[实际入口—效果—来源—参数—目标—执行—证据矩阵](validation/xuanshu-p2c2-closeout-validation.md#4-实际入口覆盖矩阵)。原生HMENU是范围内未完成适配，非read继续拒绝；真实物理输入与OS分类动态正例未验，不能宣布C2全覆盖/完整验收。STATUS仅显示、内部布局不伪造业务write。WebView2排除；stateful/EDA内存业务模型与五维护示例完整迁移仍属后续P2，不把本轮专用模型计为已迁移。


## 2026-10-23 P2应用接入状态

已确认方向和原规范不变；新增[HMENU-1/CI-1/AP-1实现规格](xuanshu-p2-remaining-implementation.md)只补实际入口与安全生命周期。组件绑定复制固定资源、收集完整语义参数并使用同一能力；display、本地draft与write分开。五个维护示例均已接入，共55项描述/11只读投影，普通Assistant修改及自称human的程序调用拒绝，原人工功能以匹配绑定保留。

逐功能业务实现、UI别名、测试及物理待验状态见[完整矩阵](xuanshu-p2-application-coverage.md)。name/form/dialog共享模型但独立草稿、base与generation，模型变化保留草稿并显式解决；EDA只固定矩形/稳定ID/选择及笔记提交，不扩写任意几何。C字符串模型另在效果前拒绝嵌入NUL/超容量，标准JSON/JCS不改。原UST1/UIL1不保存新业务文档。

[自动交付](validation/xuanshu-p2-remaining-validation.md)不等于[人工验收](xuanshu-p2-manual-acceptance.md)；WebView2、AI写入、跨实例共享写入及通用事务撤销没有实现。整体文档继续Proposed，已实现状态以本增补和实际源码为准。

## 2026-10-24 P2开发阶段验收状态增补

整体架构继续Proposed，已确认产品方向/冻结合同保持。当前生产源码、独立公共头、版本/ABI/包/依赖/限额未变；本轮只补专用测试与证据核对，完成当前P2开发阶段模拟及自动验收。实际应用binding/collector及模型回读、共享源码两分支、真实workspace标签/重开/open epoch补强见[20行A/B映射和独立记录](validation/xuanshu-p2-simulated-acceptance.md)。没有增加公开来源证明或执行旁路，生产DLL不导出私有fixture/人工接纳。

真实设备/生产来源正例、Windows动态矩阵、IME/跨屏DPI、视觉与长时按用户2026-10-24指令延期到软件整体开发完成后另获授权集中验收，见[延期清单](xuanshu-p2-deferred-manual-acceptance.md)；原人工要求保留。开发阶段通过不等于物理或P2完整验收。旧输入/浏览器/超时失败仍独立未定位，不因通过销账。P3/AI Service及后续系统没有实施授权。


## 2026-10-26 P4A-1受控编辑增量

【已实现】独立ui_cap_edit_desc_t嵌入原v2描述并附risk/transaction，原子注册同一ID；Schema、资源、版本、代次及注销仍一个目录。唯一业务execute不复制，BEGIN/COMMIT/ROLLBACK/READBACK/UNDO/RECONCILE为应用事务/证明适配。旧read接口不执行write，旧complete_v2也不能提交AI编辑阶段。

Service私有授权执行链在原资源写门内使用稳定call/stage ID。BEGIN后仍对EXECUTE和UNDO重新查revision、状态、modal/关闭和取消；同步complete不表示应用外层退栈。输出经原Schema和JCS核对，回执不合则UNKNOWN；应用与存储结果确认后才解门。人工UI与AI编辑共用门，AI不冒用C2凭据或legacy入口。

【已确认且本批落实的边界】普通可撤销write仅单资源内存夹具；高风险只验证审批绑定/消费/拒绝，external无实际外发执行器。事务/undo数据在应用，不新增通用事务系统或跨应用原子性。正式应用模型、五示例AI编辑与正式审批UI属于后续P4-B/C，不纳入本次完成。精确头、错误/借用期及验收见[P4A-1](xuanshu-p4-controlled-edit-contract.md)和[独立记录](validation/xuanshu-p4a-validation.md)。


## 2026-10-26 剩余P4开发阶段增量

新增独立capability_edit_extensions.h与ai_edit_ui.h，size/version=1；v2仍同一目录的增量，不改旧register_edit非普通风险拒绝合同。APPROVED_DESTRUCTIVE是具事务的可撤销内存破坏性操作；POLICY_ONLY保留既有可信人工副作用入口，AI审批消费后仍拒绝实际执行，不提供新文件或网络执行器。省略事务回调时阶段明确UNSUPPORTED，不盲调空指针。

五应用13项内存write的唯一run均由应用BEGIN/工作快照/COMMIT/READBACK/UNDO适配复用；业务/草稿分离，undo单调revision、EDA ID不回退。三项偏好文件有明确风险/拒绝及只读退出证明。目录ID/Schema/descriptor_hash/代次/资源/注销和预算仍统一；新投影不成为第二套权限来源。

宿主Light确认绑定复制control_id/hash并借用同workspace可信chrome，消费C2原私有human guard。按钮绑定共用16条/8MiB，不导出生产私有测试入口。解绑PENDING不缓存就绪，退栈后重试；绑定背压可见，不自动重复执行。完整55项功能与业务证据见[矩阵](xuanshu-p2-application-coverage.md)，正式来源正例仍待物理验收。


## 2026-10-27 P4限定收口与P5-1已实现状态

用户本轮明确授权后，已完成同workspace/同Service/单主对话的Coordinator图、应用可选纯数据profile、私有子上下文manifest、共享根预算、依赖/并发/取消/收集，以及原P4受控事务执行和schema3最小运行记录。独立[P5-1合同](xuanshu-p5-coordinator-contract.md)和[五应用覆盖/原其他应用缺口](xuanshu-p5-application-coverage.md)优先说明当前增量；原总体未实现部分继续Proposed，不由新接口倒推既有能力。

最终verified Native21/21、Light31/31、ASAN21/21，Native静态/Assistant/公共头/两DLL分支另9/9。具体源、二进制、首次失败、原窗口与本轮整改见[P5独立验收](validation/xuanshu-p5-validation.md)；不将这些数字相加冒充独立case。实际五包模型/对象/revision/事务/undo有回读，不是执行计数。旧C2人工资格不用于AI授权，旧Assistant修改不重新开放。

普通有限图修改仍宿主确认30分钟、风险逐项5分钟；写前意图/消费确认先于业务。取消不等于回滚/解门，UNKNOWN继续阻止资源；旧epoch只查记录、不恢复授权/工具重放。P4/P5正式控制在完整投影成功后才绑定，未投影时拒绝。运行记录按字段隐私/预算保留，不广播完整子上下文。

参考工程此前拒读，本轮未扩权或修改，其当前HEAD未重新核验。内置browser仍Light legacy而非必需WebView2，没有统一资源/版本合同，未接入新Service；原“所有其他应用迁移”未完成，具体下一案独立评审，不静默缩减后宣称全部完成。真实设备/Windows动态来源/IME/DPI/视觉/辅助技术/长时继续延期，旧失败独立未定位；P6/真实Provider没有实施。


## 2026-10-27最终所有权回归checkpoint

最终复制队列收口后，ownership Native22/22、Light32/32、ASAN22/22通过，包含14项Coordinator核心case；Native ownership-sdk9/9另核对公共C/C++头、Assistant、静态嵌入/只读工具及两真实DLL分支。修前verified21/31/21保持独立历史。新受控queue-ownership回归保留未消费完成槽，先值10、排空后42；没有声称动态复现崩溃。入槽/关联在同一锁内复查、Coordinator发布/查询同步，仅私有实现变动，无公共合同/版本/限额变化。

最终增量、源码/正式二进制和运行前身份比较、保全与两轮清理由build/xuanshu-p5-20261026-e63b8a清单记录。修前before-queue-ownership匹配源及程序、全部首次失败和必要库/备份保留；其他历史失败独立未定位。P5开发阶段模拟及自动验收通过限本轮P5-1合同/授权范围，原browser及其他应用全迁移不冒称完成。物理/真实模型及P6未执行，停止联合代码评审。

## 2026-10-27 P6/BRC-1应用合同落实

原能力合同、C2A-1/P4/P5接口、v1只读结构和错误语义不变。browser统一browser.view资源及SHA-256语义revision，browser.snapshot生成普通/Assistant read投影；view只经现有框架人工链绑定10个旧动作，绑定收集text完整值、不重复handler/权限。目录read条目随视图版本退役/重发；已接纳目标不静默改绑。完成队列背压时保留应用单槽输出并busy，未确认前不丢关联；关闭复用原worker/回调及能力门。

navigation/back/forward/reload/stop/home/help涉及网络/文档取消，settings-save/reset涉及文件；9项不标read/view，不新增external入口，不让source字符串恢复AI修改。其旧人工功能保留，用户已确认完整副作用迁移另案；这仍是原P5完整迁移缺口。五维护示例本轮不扩新业务。

存储只记录运行，不接管业务文档。启动逻辑失败、检查点I/O失败、未确认/UNKNOWN、重启及旧charged操作均按P6R-1保持保守阻止；普通取消/forget/reregister/root终态不清门。原人工解除仍要求执行者退出和实际回读/revision/回执，新测试不成为生产授权旁路。
