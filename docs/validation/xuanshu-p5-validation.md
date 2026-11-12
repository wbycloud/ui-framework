# P5独立开发验收与P4限定收口

2026-10-26开始，2026-10-27收尾。结论由本轮ownership最终JUnit、身份与保全核对确认；不把开发阶段模拟/自动证据写成真实模型、真实设备或无条件完整P5验收。无署名。

## 1. 实际基线与改动归属

HEAD e0eca03db25930be55bdb45477027d1793a61904，codex/p1-process-boundaries；无提交。新证据build/xuanshu-p5-20261026-e63b8a，17450项入口baseline。原P4 delivery43/source323/binary352入口全部匹配；P4、P3、P2历史不是本轮增量。当前本轮精确增量以delivery-files.json为准，不用HEAD diff推断。参考工程此前读取拒绝，本轮未重试扩权，其当前HEAD无法新核实；没有修改。

## 2. P4限定前置核对与收口

按Service方法/回调→目标绑定→P4范围/审批→原子意图消费→BEGIN/唯一业务execute/COMMIT→READBACK/undo回执→最终确认→释放及UNKNOWN/关闭门核对。旧P3方法租约、最后回调后门、真实DLL退出和原P4资源门已存在，不重复开发；原先129/2等红证据继续属于前轮。

本轮为并发子步骤参数化同一个私有P4控制器（显式e参数、Service非拥有引用），原公开方法仍调用其原实例；没有第二业务handler/目录，未改P4独立结构布局/错误/额度。新的图控制器以独立范围持有限具体步骤，不把旧单调用确认扩大。

自审发现正式宿主新绑定可先于排队的范围投影生效，为源码窗口，未声称已动态复现用户错误写入。最小宿主投影代次门：新快照先使旧控制解绑，只有Light同步post_json成功才重新绑定；失败不生效。P4和P5同守此门，后端源码lw_post_json在JS异常返回失败。专用测试显式使代次未投影，要求绑定=0及私有接纳也UNSUPPORTED，投影后才成功；这是状态机受控回归，不是物理来源证明。原P3/P4正式面板另回归。

## 3. 已实现合同

独立ai_coordinator.h size/version1的profile注册/注销/复制读取、Coordinator启用/查询、图submit/get/answer/cancel/continue及可信Light绑定/解绑。保持SDK0.10-dev/API10/std10 ABI1/package1、v1/C2/P4布局及语义、固定依赖/JCS/SHA/原额度。

根图只有一个调度/预算权威，复用同Service/主对话/后台SQLite worker及原新任务队列。显式stub图不冒充NLU；直接主Agent工具与可选子Agent都受根预算，子上下文/角色只作数据，兄弟/主对话不自动复制。图先查唯一ID/父/依赖/环/深度/委派/Schema/目标，模型输出不能增权。根32后代/直接子8/深度3/并发4，steps64/model16/tool64/Agent32；model16同时限制可运行的profile Agent数量，不能宣称32个子模型均可各运行一轮。上下文128KiB、结果合计64KiB、每子16KiB，原Service8MiB和控制/事件/P2额度不提高。

含write的有限图由宿主私有事件确认，30分钟不续期；每风险调用原P4另5分钟。固定descriptor、参数/constraint、目标/epoch/revision/scope；不改绑、自动排队重试业务或开放legacy。图中的每在途编辑复用P4原子意图/消费、应用自有事务和实际业务回读；同资源busy拒绝，跨资源并发，部分失败保留成功。UNKNOWN保留资源门，取消/forget/重登记/根终态不清除，新的宿主只读核对才解除。

接纳持久化事实与当前阶段持久化确认分离。读调用stable call ID在dispatch前随图确认，forget后仍保留last_call关联。等待问题/审批/依赖不占模型worker；取消停滞只可观察，不终止线程，最终必须子结果/预算/回调收尾。AI线程只持复制数据/稳定ID，UI调用应用，无UI join。

P3入口字符串规则继续用于Service图（不接纳嵌入NUL的字符串值）；未改能力层JSON/JCS对U+0000的合同，五维护应用的C业务文本本身也明确拒绝该值。未将此服务子集宣传为可直接复用全部TS Provider/上下文语义。

## 4. 持久化、身份与恢复

schema3通过宿主确认后，后台一致备份/quick_check/application_id/schema/run计数核对再事务迁移。故障注入的DDL后rollback保持原schema2且ai_graphs不存在；备份失败根本不开始迁移。仅隔离临时库/副本，无用户库。

根runs/messages/events唯一终态保持；ai_graphs保存有版本图、预算reserve/charge/settle、问题/答案hash、目标/call/编辑关联、效果/回执/summary和上下文manifest。完整私有context不进runs.plan或accepted事件，测试从SQL核对不会泄漏特定私有串。图在每次下一副作用前确认复制快照，P4业务意图仍独立原子确认；UI只投影，收到/接纳/完成不同。

同ID同内容当前cache幂等；旧epoch或历史目标不再有效时，PENDING只表示异步历史身份查询，out ID先关联，existing/rejected事件区分同内容与冲突，绝不改绑或重放。主界面保留输入到committed existing。隔离子进程证明恢复NEEDS_REVIEW、不自动执行/恢复授权；明确continue用新目标/新client，后台拒绝已charged operation再次执行，原记录不覆盖。长期留存/更多迁移/故障稳定性仍P6，不宣称无界历史存储已解决。

## 5. 自动验证矩阵

| 专项 | 明确断言/本轮证据 |
| --- | --- |
| basic | 一个主Agent直接read得value10；两个子步骤不同资源并发写31/62，revision/txn/undo；标签切换不变、重复提交不重写、同ID异目标冲突 |
| validation | 环、双ID、Schema value101、伪造approved字段拒绝；问题ID/operation关联、重复同答案幂等/异值拒绝、迟到答案不推进；私有串不在公共图 |
| lifecycle | 真实DLL自有worker及外部callback仍在途，根取消后非终态/关闭不卸载；明确退出信号后才完成并允许关闭，未阻塞UI |
| boundaries | profile8及第9拒绝/注销后恢复；直接子9、深度4拒绝；32后代树、峰值4；普通队列背压/全部关联保留，受控+5000ms停滞后仍排空 |
| bytes | 16KiB输入与约48KiB显式上下文图，队列未到16即原8MiB字节额度背压；bytes≤8MiB且>6MiB，已有ID不丢，排空恢复，实际值仍10 |
| partial | 失败spawn保留Agent预留/charge，依赖blocked，独立read成功value10；提示文本不赋权；根授权到期无写 |
| contention/unknown | 同资源两写仅先者27，另一busy；坏事务回执UNKNOWN，取消/forget/新提交不清门，新的只读证明后恢复正常42 |
| approval | 错误/重复批准拒绝；原未开启的破坏/external执行经范围/具体审批消费后仍政策拒绝，值保持10。实际EDA clear/undo沿同P4控制器另复用真实应用回归 |
| storage/migration | 意图前图存储失败execute未开始、Service UNAVAILABLE/原值10；备份失败/DDL后事务rollback；未来/损坏/锁及P4关键窗口复用本轮相称旧专项 |
| restart | 独立seed/inspect子进程，phase NEEDS_REVIEW、无终态/自动工具；私有串不落盘；旧epoch同请求异步existing；新目标显式继续才执行read |
| 五应用/正式面板 | ui_ai_graph_apps真实五包与具体值/对象/树名/revision/回执，峰值4；ui_ai_graph_panel生产host/page、程序页批准拒绝、投影门、可信隔离接纳后业务1，收面板不变 |

最终verified及bytes证据、JUnit与源码/二进制清单列于本目录证据根；分阶段结果不能相加成独立用例数。此前joint Native44/44、Light51/51、ASAN41/41通过；最后新增/整改有closeout Native12/12、ASAN22/22、Light26/26，Native静态/公共头/两真实DLL分支及Assistant另sdk-closure9/9。以verified最终数字和源比较为准，非完整历史CTest全绿。

## 6. 首次失败与修正（原日志不覆盖）

| 阶段 | 事实与修正 |
| --- | --- |
| graph-first-native-build | 私有编排脚本按系统编码读取UTF8 CMake失败，尚未注册target；ninja unknown target原日志保留。工具编码异常没有当时独立stdout重定向，不把后写说明伪装成原始日志；无新产品二进制执行 |
| graph-registered-native-build | 新测试误调用不存在shutdown函数，/W4 /WX拒绝；改为实际API，不放宽告警 |
| graph-first-native-test | 3例SegFault为新夹具在先决失败后strlen(NULL)级联；实际产品缺口为图边尚未全部填好就查环、read describe不能取write；分别修最后查环/正确describe分流，加失败保护，不称产品崩溃已复现 |
| graph-gates-native-test | 编辑client/out ID共用缓冲导致propose先清out也清client；改独立缓冲及完整operation hash，原关联先于执行 |
| integration-first/link-light-build | 宿主误用私有未导出ui_strdup、测试误用不存在run mode；用本地CRT副本和既有WINDOWED，未换库或放宽WGL要求 |
| graph-panel-first-light-build | 新manifest局部名遮蔽触发/WX；仅重命名本地变量 |
| graph-panel-first-light-test | schema1初检缓存未随P4迁移刷新，面板进入不了schema3，首超时29.99秒保留；补只读schema状态复核，迁移仍须人确认 |
| restart-first-native-test | 新夹具反复确认同一在途迁移；改有界等待且一次请求，不修改生产消费/原断言 |
| policy-first-native-test | 新测试把审批展示当作存储退栈完成，首20.02秒超时；按PENDING受控等待再确认，业务/错误/期限断言保留 |
| projection-gate/cache-light-test | 新门首次未清旧绑定缓存/夹具未等待投影；清缓存、按真实投影代次有界等待，保留首失败及匹配二进制，不用固定延时猜 |
| final三配置boundaries | 取消排队任务的停滞提示将内存committed清0，调度器因而不选择；SQLite仍QUEUED、部分图从未取得version确认。是源码/动态已证实的P5收尾缺陷，不默认环境。新增独立admitted事实及取消/停滞确认，强制时钟回归通过；未放宽20秒或额度 |

失败目录各自保留源码/输入身份、当时EXE/DLL/uapp、原log/JUnit和必要DB。编译失败没有新执行二进制；不把顺带复制的旧程序当新源码red。早期测试/构建身份与最终分开，不冒充此前历史缺失red二进制。

## 7. 历史独立分账和未验

ASAN-Light共享投影50ms interrupted、frontend63、Light surface_input、browser20、workspace20.05秒/20秒门槛、Native输入首次都未借新通过销账。P4已有ASAN-Light非AI对照不证明所有旧版本根因，本轮未扩大P6或放宽脚本时限；本轮生产Light专项原日志无该诊断，不等于视觉/物理通过。

新排队取消故障在Coordinator图路径有明确关联/已修；旧workspace首次不是该图路径，没有证据归为同一根因。其他旧组共用UI基础但未证实导致P5授权/事务/持久化错误，保留待核实，非自动阻断本轮开发阶段口径。真实物理来源/Windows动态正例、IME/DPI、视觉/辅助技术/长时全部延期，见独立清单。

browser仍legacy，没有本轮统一资源/版本合同；虽为现有Light而非WebView2，也不能回退旧命令接入P5。原“全部其他应用迁移”具体缺口见应用覆盖矩阵；本轮五维护示例P5-1通过不声称原规划全部迁移。没有真实Provider/Coordinator跨进程隔离、通用事务或跨实例共享写入。

## 8. 身份、保全与清理

baseline/delivery/preservation/source/binary/failure/exports/JUnit/cleanup清单记录精确结果。前P4 352项二进制再次核对；旧修改、未跟踪文件、固定依赖和冻结SDK保留。忽略的历史/SDK目录未穷尽重新hash，不夸大保全范围。

verified运行前保留实际二进制清单，收尾再次逐项核对；源码快照对应每轮build输入，改动无关文件不冒称全部字节相同。仅本轮绝对根内无用途obj、CMakeFiles/重复包暂存等允许清理；日志、JUnit、首次失败源码、匹配程序及必要DB/备份保留。清理完成后以cleanup.json列准确数量。

## 9. 停止位置

最终verified Native21/21、Light31/31、ASAN21/21全部通过；包括13项Coordinator核心case、五真实包协作/正式面板及相称回归。Native sdk-closure9/9单列，先前批次不相加。每轮运行前二进制清单和最终保全核对后，本轮口径为：**P5开发阶段模拟及自动验收通过**（P5-1及本轮明确授权范围）。不是设备/真实模型或无条件完整P5验收；停在P4收口及P5联合代码评审，P6需新授权。


## 10. 最终复制队列所有权补充收口（2026-10-27）

独立收尾复核发现：Coordinator队列在锁外看似空闲后，构造快照期间worker可能完成job→busy→done转换；如果未在写入时重新核对，未消费的完成槽或历史查询关联字段可能被覆盖。这是源码已确认的所有权窗口，不声称已复现真实调度下崩溃或业务重复写入。修前完整源及匹配程序另存before-queue-ownership/manifest.json；不替换此前red证据。

最小私有修复：复制快照后在同一Service临界区重查job/busy/done，槽仍被占用则背压且保留dirty；历史查询先在局部构造ID/hash，再在同一锁内提交关联和job；Coordinator指针发布及状态查询与worker共享锁。未改公共头、错误语义、额度、固定依赖或线程职责。

新增queue-ownership受控测试在乐观检查和最终复制之间发布旧完成通知，断言job=0/done=1/slot=-1保留、业务值仍10，之后正常排空到42。该钩子仅UI_AI_FIXTURE编译，生产DLL不导出；它验证完成所有权与业务结果，不冒充真实线程随机交错、物理输入或Windows来源正例。首次Native专项211检查/0失败。

补测编排首次错误使用不存在ui_assistant_test目标，ownership-final-native-build原日志/源码/旧程序保留；实际目标是ui_assistant_protocol_test。该次未执行新产品测试，不把复制的旧程序冒称新源码red；后续ownership-sdk构建成功。

最终结果及清理身份以下方交付核对为准；第9节verified 21/31/21保持为修前已通过历史，不用它覆盖最后源码。


### 最后源码验证

ownership Native22/22、Light32/32、ASAN22/22，0失败/0跳过；核心case为14项。ownership-sdk Native9/9，公共C/C++头、Assistant、真实DLL两个分支及P3静态接入；不同阶段有重叠，不相加为独立总用例。未放宽20秒/原限额。修前verified和新增ownership各有源输入与运行前二进制清单，后者与最终交付逐项比较；第9节只代表其当时源码。


### 最终交付与保全核对

本轮37项交付（24修改、13新增），17426项入口文件未变，无缺失/越界改动；root三规划及原整体设计等前缀、旧handoff后缀逐字节保留。331项编译输入及114项实际运行/链接产物有最终SHA清单，ownership三配置/生产包的运行前清单与最终运行产物无差异。正式五包ownership-production 1/1、364检查/0失败，独立核对原名称模块、两个共享分支、重开及旧epoch拒绝，不冒称物理来源。

最终源码清单SHA-256为5129061DD1B502081FE40FAE3A686EA1246772D448ED8F6FDBFC9611D316D244；正式host SHA-256为19717B049C9060BB5B5A7DE2A8CE432C9F637BA2C8658ABB31FE3DB96263A737。完整正式七产物身份见delivery-identity.json，不依靠本说明代替清单。生产Native/ASAN326、Light340导出均无私有接纳接口；隔离库348含8个私有测试入口，不对生产导出。

原P4 352项二进制仍全部匹配；本轮修前before-queue-ownership 362项源/程序匹配，所有首次失败源码清单逐项匹配。保全范围不是对被Git忽略的所有历史/SDK目录穷尽重新hash。累计两次清理1199个文件删除事件、420803398字节（约401MiB），后一次包含重建生成的相同过程路径，不称1199个唯一文件；详见cleanup-pass1/cleanup-ownership与汇总cleanup.json。仅本轮obj、CMakeFiles、链接过程产物及核实重复包暂存删除，失败日志/JUnit、源快照、匹配程序、库/备份和正式交付保留。

结论：**P5开发阶段模拟及自动验收通过**，限本轮P5-1冻结合同和五维护示例范围；browser原全应用迁移规划仍有明确缺口，真实模型及延期人工未验，不写成无条件完整P5通过。停止联合代码评审，不进入P6。

## 2026-10-27 P6前置限定复审补记

原P5最终ownership及37/331/114与修前362项证据入口匹配；本轮不重写原P5结论，也不把原其他应用全部迁移当作已完成。P6连续正式面板第15轮发现子编辑存储job遗漏worker唤醒，修复仅将子job纳入poll检查，真实线程受控退出后业务42和完整收尾另有回归；详情及精确源码身份见本轮P6记录。不是声称P5原运行已覆盖新故障窗口。

browser属于现有Light内部应用，本轮新增同目录read快照/资源/revision、10个既有view绑定和Assistant拒绝旁路；导航/停止/home/help等7项网络副作用及2项来源文件操作共9项仍保留旧人工。用户确认完整副作用迁移另案评审，原P5全应用迁移仍未闭环，不能以WebView2排除或无条件P6完成掩盖。

旧ASAN-Light中断、frontend63、Light输入、browser20、workspace首次20.05秒（20秒门槛）及Native首次仍独立。新P6自己的首次失败、超时、修正和有限资源样本不覆盖旧失败。真实模型/设备输入/物理断电/人工长时未执行。
