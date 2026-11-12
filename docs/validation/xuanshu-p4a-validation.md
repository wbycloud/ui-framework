# P3收口及P4-A独立验收

2026-10-26。限定范围：P3 continue租约整改及单资源专用内存夹具的P4-A。文档不署名。不是完整P4、正式审批聊天面板、真实设备或生产用户库迁移验收。

## 1. 基线与交付身份

实际HEAD `e0eca03db25930be55bdb45477027d1793a61904`，分支`codex/p1-process-boundaries`。证据根`build/xuanshu-p4a-20261026-a61d4e`，baseline17427项；入口原P3 delivery32/source308/binary101身份全部匹配。参考工程之前不可读，本轮未扩大权限或写入，不影响本仓库独立工作。

本轮归属以baseline→delivery-files比较，不以HEAD→工作区全量差异代替。保留.gitattributes及此前P2/P3未提交实现、未跟踪历史材料；固定依赖、SQLite3.54.1、yyjson、JCS/SHA-256、冻结SDK、版本和旧公开结构不变。文件逐项归属及最终源码/二进制SHA-256见delivery-files、source-after、binary-files、preservation和audit清单；build/test前输入另存，不能把清单生成时间当成测试时的源码身份。

## 2. 先冻结、再实现及自审结论

先新增P4A-1及实施计划和P3独立复审，再进入P3修复；P3新Native9/9通过后才开展编辑。P4-A新增两个独立公共头，完整size/version=1，10个公开函数。同目录原子注册原v2描述与事务适配，execute唯一；人工与AI共用resource_busy，AI独立结构化授权不借用C2凭据，不开放legacy修改。

UI负责资源/事务/回读，原Service串行worker负责SQLite；输入与输出复制，稳定call/stage ID先建立，UI无等待worker的join。每调用对应应用事务及opaque撤销回执，fixture实际值和revision作证据。意图和scope/approval消费同一SQLite事务，收到确认后再查门，最后结果确认才释放。UNKNOWN取消/forget/重登记不可清除，显式只读核对与持久化解除记录后释放。

自审及范围内整改包括：

- P3 methods外层租约及回调后READY重检，保留合法重入；没有宣称复现崩溃。
- 原describe_v2把完整metadata塞入256字节格式缓冲，导致合法描述错误返回OUT_OF_MEMORY，改为有界动态追加，不提高限额或改变错误合同。
- 提案先RECEIVED，数据库确认才SCOPE_WAIT，拒绝提前确认；失败接纳回收live标记、确认入队失败撤回未生效scope，防止关不掉或无法重新确认。
- 显式核对不把原未知终态重新变成RUNNING；持有额外cap/resource/回调租约，核对后没有第二终态。意图/结果commit确认丢失可能实际落库，绝不重放业务。
- BEGIN只接纳本次事务号，COMMIT要求撤销回执；回读的旧事务号不能污染新调用。RECONCILE缺失/冲突原事务回执保持UNKNOWN。同步完成仍等待外层执行栈/worker/回调退出。
- 专用Light测试运行库跟随正式shared的CRT配置；不改QuickJS或其他固定依赖，不将测试私有钩子导出到生产DLL。

## 3. P4-A逐项自动断言与边界

所有用例在隔离临时库及真实uapp DLL上进行；两个真实workspace实例仅验证固定目标，一次仍只写一个资源。业务值通过统一read能力回读，不以计数或控件变化代替。计数只用来证明未执行/未重复。

| 专用case | 主要实际断言 | 仍未覆盖的边界 |
|---|---|---|
| basic | 人工适配同handler先33，AI改40；活动切到另一实例仍只改原实例，另一实例10；原请求重取不执行；显式undo恢复33；同ID换参数异步拒绝；两次消费/两条唯一终态 | 非真实物理人工接纳 |
| policy | 越界101及自报source参数拒绝；read不能伪装write；同ID冲突；可见原生Button上的BM_CLICK/程序down-up不授权；30分钟/5分钟精确边界；三类高风险审批消费后effect_not_enabled，值10 | Windows物理来源正例/所有注入分类未验 |
| cancellation | 开始前取消值10；execute后commit前真实rollback并回读10；commit后取消保留33/applied；每run唯一终态 | 无整任务自动回滚 |
| unknown | rollback失败实际77；cancel/forget/注销不清门；显式只读证明、事务及revision后解除；在途注销PENDING | 跨进程业务回执需真实应用适配 |
| storage | 备份失败、生成副本后校验失败、事务迁移失败均不改原schema1；备份可读且保留；新路径重新确认后schema2；意图失败执行0/消费0 | 用户数据库和实际宿主确认未操作 |
| lifecycle | DLL自有worker及外部回调均在途，关闭不能卸载；worker独退仍非终态；两者退出才complete/rollback/存储/释放；队列满明确LIMIT；旧实例迟到post被拒绝 | 无任意应用线程自动追踪 |
| storage-result | 实际commit66而最终保存失败，UNKNOWN/write门不释放，不伪装未修改 | 真实磁盘故障窗口只用故障注入 |
| storage-ack | 最终事务已提交却确认丢失，保留66；核对幂等且原终态仍只有一条 | 非断电耐久性测试 |
| output | 提交67后业务回读Schema非法，保持UNKNOWN；纠正只读证明后方可解除 | 应用真实性属合作合同 |
| restart-scope | 子进程仅保存scope后退出；父进程新内存10，不恢复授权或执行业务 | 不是恢复旧业务快照 |
| restart-intent | 原子意图/消费已保存、BEGIN未开始即退出；恢复只读记录，执行0，不重放 | 不自动解除旧epoch隔离 |
| restart-commit | 应用提交后、最终记录前退出；恢复未确认记录且不执行 | 内存fixture不提供跨进程业务数据 |
| restart-final | 最终落库后退出，恢复同对话/终态；不重放或重复消费 | 不导入参考工程数据库 |
| boundaries | 最后revision回调打开modal/过期、busy、版本冲突均execute0；意图受控未确认时实际10；人工同门拒绝；之后53；真实workspace关闭重开拒绝旧instance/epoch，新实例10 | modal哨兵为专用门夹具，真实Light modal回归另列 |
| sync-retire | execute内同步complete后forget拒绝，注销/关闭仍PENDING，参数和模块仍有效；外层退出及rollback持久化后实际卸载 | 未新增恶意DLL隔离 |
| large-scope | 合法63000字节Schema描述与完整参数独立JCS/SHA，不因二次封装降低原64KiB；实际49 | 非提高现有限额 |
| intent-ack | 意图实际提交但确认丢失，值10/execute0；保持UNKNOWN，新的显式只读核对后才允许65；两条唯一终态/两次消费 | 无自动重试或审批复用 |
| receipt | commit实际70但错误事务回执保持UNKNOWN；核对缺失事务号仍拒绝，正确回执后解除；终态唯一 | 无应用外业务真实性证明 |
| undo-gate | 新显式撤销在BEGIN后出现modal或check改变revision，UNDO次数0，实际59/60保持且真实rollback；每调用仍唯一终态 | modal是专用受控哨兵 |
| receipt-backpressure | 提案接纳前队列故障返回LIMIT且ID为空；确认入队失败不留下scope；重新明确确认后55；只一条终态/一次消费 | 受控队列故障非真实malloc压力崩溃复现 |

本轮共20个专用case。每个stage还检查非零ID、executor_exited=false时PENDING、重复完成ALREADY_EXISTS；实际输出仍过原Schema/256KiB和调用8MiB。旧1024/8MiB、多Assistant和只读失败槽额度由未弱化的C1/core/closeout回归支持，不冒称本批重写了全部额度测试。

## 4. 最终新验证结果

结论：**P4-A开发阶段模拟及自动验收通过**，P3本次租约阻断窗口闭环。不是设备/真实人工验收、正式面板或完整P4通过。源码冻结后review-final三配置均通过，P4-A专用20项各分支全部通过：

| 配置 | 联合结果 | 耗时 | 原始证据 |
|---|---|---|---|
| Native | 44/44，0失败 | 38.72秒 | review-final-native-test.log，native/review-final.xml |
| Light | 49/49，0失败 | 40.79秒 | review-final-light-test.log，light/review-final.xml |
| ASAN | 44/44，0失败 | 47.81秒 | review-final-asan-test.log，asan/review-final.xml |

这些是本轮新结果，不是原P3或P2历史结果。ASAN未报告新内存错误；不以检查数量替代业务断言或推断所有资源压力情况均已验证。各配置最终build/test前318项源码输入与当前文件全匹配；实际运行二进制另列131项（含red/native），失败副本对其阶段源清单零差异。

回归范围含P3 Service/continue/静态embedded/control/storage/limits/tools，Light正式面板；原A+B/C1真实DLL workspace两分支、v2真实DLL、Assistant、公共C/C++头、JSON/JCS、目录/entry/closeout/v2/forms、来源/HMENU，以及Light modal/forms/component输入分支。所有既有断言、超时和生产限额未放宽。

最终生产DLL导出核对以export-audit.json及dumpbin原文为准：Native/ASAN311、Light325，其中本批10接口；生产无ui_ai_edit_fixture*、私有ui_cap_edit_*或人工接纳导出。隔离ui_framework_edit_test.dll才导出三个私有接纳/故障/人工比较钩子。其普通程序输入与私有接纳分开，不冒充生产物理来源。

## 5. 首次失败及修正分账

- P3 red 48检查/2失败/freed=1，见p3-red-native-test.log与red/native/p3-red.xml。测试遇到旧销毁OK立即停止外层访问，未宣称崩溃。修后continue50/0/freed=0。
- 初始C4210、缺entry头、缺yyjson链接属于构建失败；edit-first五项Not Run不是红执行。初次实际edit-executed夹具缺必需unmount，造成五项失败/超时及级联断言，不能归因环境或产品业务成功。
- 合法describe_v2动态格式问题产生edit-basic/edit-diagnostic失败，修复保留原输入范围。随后edit-describe-fixed暴露提前scope接纳，增加RECEIVED后basic通过。
- edit-contracts中的核对状态重回RUNNING是源码缺陷；队列0字节投递期待LIMIT属于新测试错误，改为实际同大小填满并断言LIMIT，不放宽生产预算。
- joint-light-build仅测试运行库CRT不匹配，跟随原shared配置后构建通过，未改固定依赖。
- final-native-test是本轮自审新增回执检查的两项失败：旧回读事务号污染新BEGIN，影响连续取消/undo；其42项中40通过/2失败，匹配源码和二进制保留。限制回执接纳阶段后receipt-fixed三配置通过；后续补接纳失败清理及核对缺回执回归，最终结果另列，不覆盖首次。

编译失败后的旧二进制只作为构建残留诊断，不标为该编译输入的匹配可执行产物；每次实际失败执行的源码输入与失败副本分别核对，差异或缺失由audit.json如实列出。阶段16/16、42/47/42、43/48的通过均保留其实际身份，不作为最后源码的自动证明。

自审末段发现UNDO未复用BEGIN/EXECUTE门，新增undo-red受控运行339检查/3失败：两项实际值/UNDO次数证明执行门遗漏，另一次forget错误使用父ID是新测试关联错误。修复将UNDO纳入同一目标/版本/modal/关闭/授权最后检查后，只剩该ID断言失败；改为查询新的显式undo operation ID后333/0。undo-red、undo-fixed和undo-id的首次材料分别保留，不把测试ID修正包装成产品错误或放宽断言。旧值59/60、UNDO次数0和唯一终态要求不变。

## 6. 历史问题和延期验收

| 项目 | 当前判断及优先级 | 本轮处理 |
|---|---|---|
| frontend63次断言失败 | 同当前DLL的入口host/page/test对照相同，不能断言全部是旧测试错，也不是完整旧框架对照；正式面板接入前优先核对诊断/合同预期 | 不重跑、不删断言、不冒称修复；与本批专用内存闭环无已证实因果 |
| Light surface_input首次失败 | 与正式输入/UI公共路径有潜在关联，待定位；真实输入集中验收前核对 | 本批限定来源/组件通过不销账 |
| browser20失败 | WebView2非本批路径，缺少与新编辑/Service的因果证据；对应浏览器主线处理 | 原失败保留，不归因环境 |
| workspace首次20.05秒超时 | 实际门槛20秒，后续限定通过不能定位根因；生命周期/资源时序风险仍需关注 | 原CMake超时不改，历史不覆盖 |
| Native输入首次失败 | 可能涉及来源/修饰键，未定位；真实输入正例前核对 | 不用本批程序消息负例宣称解决 |

延期到软件整体完成且另获授权：真实鼠标/键盘与生产链正例、Windows分类动态验证及UIAccess/HARDWARE证明限制、真实IME、跨屏DPI、视觉/可访问性与长时。替代证据是私有隔离接纳后的真实模型/事务、可见按钮程序消息拒绝及既有组件/来源自动回归；剩余风险是物理事件匹配、显示/焦点和设备时序。未来应在匹配构建的宿主展示结构化范围→真实点击确认/审批→修改与回读→到期/取消/关闭，再保留源码/二进制hash、原始事件来源、应用revision/回执及错误，不使用SendInput冒充物理输入。正式面板须由后续批次另获授权后提供，当前无用户库/真实设备操作。

## 7. 保全、清理及停止

baseline、delivery、preservation、source-after、binary-files、failure-files、audit、test-results、export-audit及cleanup相互补充。首次失败DB/journal/源副本及匹配EXE/DLL/uapp保留；只清理本轮证据根中可再生的对象、编译暂存和重复打包目录。此前P3 101个二进制与历史材料不清理，冻结SDK不覆盖；依赖源码身份按入口比对保持。

当前停止P3整改及P4-A联合代码评审。P4-B真实应用事务/持久业务回执、P4-C正式授权审批面板均未实现；不接stateful/EDA、不调用真实Provider、不进入Coordinator/子Agent或后续阶段。最终限定结论只按第4节实际证据给出，不是无条件完整P4或真实人工验收。

本轮精确收尾：30项交付（15修改、15新增），17412项入口文件字节未变，无丢失或未授权改动；原P3 101项二进制仍匹配。仅本轮13个已核对绝对目录清理193824571字节，保留所有失败和最终可运行EXE/DLL/uapp/静态库、源快照、日志/JUnit、DB/journal及离线依赖。build缓存可重新配置生成，不把删除对象称为删除失败证据。
