# P6独立开发验证记录

2026-10-27，本轮开发交付和联合评审记录。限定可靠性结果与原P5迁移缺口分账，无署名。

## 1. 基线与归属

codex/p1-process-boundaries，HEAD e0eca03db25930be55bdb45477027d1793a61904。独立build/xuanshu-p6-20261027-4a92d1，入口17463；P5 delivery37/source331/binary114、before-queue-ownership362入口无差异。此前P2/P3/P4/P5不归本轮。参考工程元信息可见而Git Permission denied，无扩权或写入。

## 2. P5限定复审

核对workspace唯一Service、原新任务队列、方法/observer租约，图接纳与admitted/committed，operation唯一/依赖/预算，配置和上下文隔离，P4意图消费/真实事务/UNKNOWN，复制队列关联、正式投影代次、REFUSE/DLL关闭及schema3记录恢复。原ownership源码和二进制匹配，不重做历史完整验收。

新连续面板在第15轮复现图已确认/子编辑SCOPE_WAIT停滞，业务未变。旧poll唤醒判断遗漏子编辑job，是源码可确认的空闲退出窗口；首日志未含所有内部job字段，不将推断写成原日志已采集。新增ai_graph_jobs锁内遍历子job，UI poll重启退出worker；原空闲线程退出规则保持，无busy-spin或UI join。受控child-wakeup使真实存储线程带待处理job退出一次，要求恢复到值42且退出计数1；初次129检查/0失败。原面板首日志/源/程序和15轮库副本保留。

P6新增启动逻辑一致性审计，补P5只有quick_check的物理核对范围。不是声称P5测试曾验证这些逻辑损坏，也不是自动修复库。新schema无变化，授权/版本/额度不变。

## 3. 实现与故障边界

ai_reliability.inc在恢复写入前检查conversation唯一、终态/事件、消息/tools/编辑/消费引用、intent绑定和图最新版本事件/预算。非法结果UNAVAILABLE，原库不重建/覆盖；只读审计不承诺识别所有恶意本地篡改。

成功写事务每32次由原worker执行PASSIVE checkpoint；UI不访问SQLite。DB/审计采用显式保留策略，不自动删用户历史；内存页/队列/缓存仍原额度。存储失败停止确认，未确认/UNKNOWN不变成“未修改”。固定依赖和旧接口不变。

测试VFS仅隔离运行库，故障由原ui_ai_edit_fixture私有控制的新增mode选择，生产不编译/导出。实际拦截SQLite xWrite/xSync返回IOERR_WRITE/IOERR_FSYNC/FULL，不冒称真实磁盘被填满。二十个私有受控退出点只在专用子进程，不强杀应用线程或用户进程；ExitProcess不是物理断电。SQL核对和业务正例另分账。

## 4. 故障窗口矩阵

| 点 | 受控阶段 | 核心断言 |
| --- | --- | --- |
| 1/2 | 接纳事务前/后 | 无接纳或已保存NEEDS_REVIEW；不恢复执行 |
| 3/4 | 意图与消费事务前/后 | 消费0/1，begin及execute尚未开始；重启不重放 |
| 5/6/7 | begin后/真实DLL execute worker在途/commit前 | 原意图保留，未确认结果待核对；不因内存夹具重置声称rollback |
| 8/9/10 | commit返回/回读前/回读后 | 没有final确认则保守未完成；实际修改/回执由原业务回归证明 |
| 11/12/13 | final事务前/后未ack/已ack | 终态0/1/1且唯一；无新执行权 |
| 14/15 | 图预算charged确认/settled确认 | 原预算和图关系保留；子结果成功不自动重放 |
| 16/17 | 根终态前/后 | 根终态0/1，后代记录不丢 |
| 18/19/20 | checkpoint前/后/SQLite close后 | 已保存任务保留，重复打开不改变终态/自动执行 |

每窗口独立子进程、新库、两次冷重开；SQL核对恢复事件幂等、唯一终态及未完成phase8。物理断电/磁盘控制器缓存不在本轮实测中。

其他专项：逻辑双主对话/孤儿事件/缺终态/孤儿消息/孤儿消费/未来schema/缺唯一索引/物理坏头/schema3坏图/消费绑定改变；失败前后主文件逐字节比对（适用副本）和拒绝状态。实际VFS错误要求业务值仍10、消费0、保守UNKNOWN；真实提交坏回执后值69，再冷重开两次，recovered_unresolved=1、修改PENDING且cancel/forget不清除，内存初始10不解释为旧修改回滚。锁冲突、关闭同步失败、备份/迁移及原P4关键窗口分别验证。

## 5. 首次失败与证据

首次30项中20窗口和3 VFS通过，7 audit各两条新夹具文件打开断言失败：UTF8中文路径误传fopen。改_wfopen保留同样逐字节断言；原产品已拒绝逻辑坏库。audit-path9/9包括新坏头/坏图；后续结果不能覆盖首次。

panel-cycles第15轮实际停滞按第2节收口；旧源/匹配程序/库在failures/panel-cycles-light-test。未冒称崩溃、资源泄漏或环境原因。修后原600秒/180秒/旧20秒均不放宽。

## 6. 历史、人工与结论位置

ASAN-Light共享投影中断、frontend63、Light surface_input首次、browser20、workspace20.05秒（门槛20秒）、Native输入首次仍独立。新子job停滞属于P5→P6必经路径，已在本轮最小修复；旧workspace首次没有图调度证据，不归相同根因。其余共享UI路径的相关性未证实，不用新通过覆盖，不无限扩审。

真实输入、Windows动态来源正例、IME/DPI、视觉/辅助技术、真实断电及人工长时延期。browser迁移冲突与最终实际完成范围、最终JUnit/身份/资源汇总及清理待本轮收尾追加。未满足全部冻结条件时不宣称P6完整开发验收通过。

## 7. 新增收口、自审与自动证据解释

- 周期PASSIVE检查点曾忽略错误。checkpoint-red Native 1205检查/3失败：xWrite故障后Service未UNAVAILABLE、没有passive_checkpoint诊断且新请求仍被接纳。修复将非BUSY/LOCKED错误明确置不可用，保留已提交事务。checkpoint-safe Native 816/0、ASAN759/0；实际进程任务数不同导致轮询检查数不同，不以数量证明正确。源码/库/匹配程序在独立red失败目录和原始JUnit。
- 新browser夹具首次加载失败-1（缺少v2所需reconcile/启用顺序/shortcut无view规则），后为-9（输出嵌套Schema不是闭合对象）。按冻结合同补齐应用描述，不改最小Schema验证器。随后旧revision夹具误判派发返回码、读取旧find草稿的时序失败；控制prepare→实际状态变更→dispatch，并严格断言v1既有FINISHED/STALE/revision_conflict；焦点及needle实际DOM/提交前后读回，仍保留原失败。
- browser-contract构建一度使用不存在的error_code成员，/WX构建失败保留；更正为原公开code数组，不改公共结构。
- P4旧面板accepted夹具未等待当前投影代次，联合回归报schema1/确认拒绝。追加有界代次一致断言后才一次调用隔离接纳；不循环尝试授权、不放宽来源。projection-final包括P4面板232/0、P5面板及60轮正式面板、生产/隔离browser共5/5。真实来源仍未验。
- joint-final Native85/85；ASAN77通过/8未运行（首次未构建目标），后已补相应目标。Light31/33，两项为P2原15秒门槛15.04秒超时与上述P4夹具。生产五包串行legacy-package-serial1/1（364/0、7.06秒）并不解释首次超时根因。
- panel-wakeup在同时进行Native/ASAN持续存储测试时180.44秒超时，CSV完成49/60；串行同60轮/180秒门槛73.47秒通过。新最终projection-final保持60/180和完整业务/模块断言。首超时与首15轮业务停滞分别记账，不能仅凭串行通过默认归因环境。
- 最终checkpoint安全处理后兼容性组合与两个持续资源验证同时执行时，Native graph_boundaries 20.08秒超时；ASAN graph_boundaries超时及P3 control末尾排队任务5秒内未终结。原始日志及匹配程序保留。没有改原测试断言、数据范围或20秒门槛；后续受控串行复核及是否阻断以第8节真实结果为准，不预称解决。

测试VFS只在UI_AI_FIXTURE编译，控制扩展现有私有mode，无生产导出；生产read/非read权限没有测试旁路。初期runner源清单漏apps目录：browser-first失败前补存实际应用源码及browser-first-app-sources.json，之后runner纳入apps。自动第一清单与补充清单分开，不假称初期清单已全面覆盖应用。

## 8. 最终验证与交付

最终相称自动验证如下，均保留各次JUnit和原始日志；不将重叠检查相加为独立样本。

| 范围 | Native | ASAN | Light |
| --- | --- | --- | --- |
| faults-final：20进程窗口、8保护副本及9专项 | 37/37 | 37/37 | 不机械重复同一存储核心 |
| reliability-final：60轮真实DLL装卸＋600秒持续任务 | 2/2；1755任务，601172ms含收尾，164检查点 | 2/2；1824任务，600250ms含收尾，171检查点 | 正式面板60轮另列 |
| compatibility-final＋仅失败项串行recheck | 48/49，随后1/1；逐项覆盖49/49 | 47/49，随后2/2；逐项覆盖49/49 | 原共享分支和实际包单列 |
| production-final：公共头、Assistant、两个真实DLL编译分支、P3面板、五正式包及P4六应用分支/P5五包协作 | 原相称回归 | 原相称回归 | 14/14 |
| projection-final：P4/P5正式面板、60轮面板、生产/隔离browser | 不重复Light UI | 不冒称ASAN-Light | 5/5；全组79.92秒，原60轮/180秒断言保持 |

compatibility的49/49是最终源码逐项覆盖，不是一次全49通过；原并行负载失败仍保留。Native boundaries串行5.66秒、ASAN5.81秒（原20秒），ASAN control3.06秒通过。串行结果不证明并行负载超时/控制末尾延期已定位，仍列期限风险，未来带阶段时延诊断复核，不改门槛。本轮无可重复的串行业务错误或新授权/重复修改/卸载缺陷；有限自动结果不外推无限压力稳定。

资源CSV：Native持续Service最大5475077字节（小于8MiB），事件快照0、队列0；进程private bytes从6471680到最大12021760，handles120..121、threads1..4。ASAN同样逻辑额度与ASAN错误/生命周期断言严格通过，private bytes最大421687296，按冻结合同单列quarantine，不适用Native32MiB阈值或宣称ASAN进程低内存。Native装卸60轮关闭后handle118、worker/队列0、实际DLL不在；ASAN对应122。最终Light60轮handle155、private bytes最大12357632，warm16后仍满足原增量门槛，DLL装卸正反断言俱在。

DB/WAL保留记录有明确增长策略：Native最后采样DB3821568/WAL1174232字节、ASAN3989504/1182472；结束后FULL checkpoint与冷重开核对1755/1824条唯一根终态，不自动恢复任务。日志/审计不被清空以压平曲线。此前sustained Native1869/ASAN1786任务各600秒为周期错误处理修前证据，不冒充最终身份。

以本轮source-after、delivery-files、preservation、binary-files、failure-files、resource-summary、exports及delivery-identity核对。panel-serial因早期测试过滤器名称错误实际0项，不计通过；panel-serial60和projection-final才有有效60轮证据。ASAN未构建目标的8项“不运行”与真实断言失败分账。初期apps源码补充清单和以后完整清单分别保留。

## 9. 范围及最终结论规则

P6R-1可靠性可独立给开发阶段自动结果；原P5完整应用迁移因9项副作用合同仍未完成，用户已明确另案评审。不能因此作无条件P6/软件完整验收通过。没有真实Provider、自然语言理解质量、网络执行器、新文档业务或新公开AI接口。真实设备、Windows来源动态正例、IME、DPI、视觉、辅助技术、物理断电和人工长时保持延期。


结论：P6R-1限定可靠性功能及其开发阶段自动项目已完成并通过，停在联合代码评审。**不作无条件的‘P6开发阶段模拟及自动验收通过’宣称**：原P5完整迁移的9项browser副作用已获用户确认另案；并行负载下短测期限风险尚未定位，真实人工/设备/断电仍延期。旧历史失败不被覆盖；未来外部副作用合同、期限诊断和集中人工各需明确后续范围。

证据归档补充：panel-wakeup超时的退出时源码复制中，两份夹具已是后来更正模块名/加入正向加载断言的版本，与开测前哈希不符。已从更早panel-cycles失败档找到两份**逐字节匹配开测哈希**的源码，另存matched-start-source，原退出时复制件不改；panel-wakeup-source-reconciliation.json列出来源和两个哈希。它的旧模块名p4_web_counter.dll缺乏有效卸载证明，不能用于声称该次生命周期正例通过；最终projection-final使用实际web_counter_app.dll加载正断言＋关闭后不存在断言，60轮通过。failure-binary-comparison逐项核对各失败开测清单的根EXE/DLL/uapp/lib均匹配，嵌套应用DLL由匹配uapp保留。没有将新失败程序冒充旧轮缺失二进制。

历史关联与优先级：本轮子job唤醒及checkpoint错误是已证实公共路径缺陷，已最小修复；并行短测期限是P6新风险，串行同阈值通过但根因未定位，建议下次带分阶段调度/存储时延核对。frontend63和输入类与共享UI有关，browser旧脚本部分调用旧Assistant导航，新的只读政策会拒绝这条路径，但不能逆推历史20次失败均由此引起；本轮未运行旧HTTP/browser全套，不以新增本地快照/view通过取代它们。workspace20.05秒旧记录无图子job证据，不归本次唤醒同根因。旧ASAN-Light50ms共享投影中断/其他首次输入保留，当前未证实阻断P6限定路径。

## 10. 精确身份、保全与清理

本轮30项增量（20修改、10新增），17源码/构建/测试项与13文档/规划项，详delivery-files.json；373项源码身份，120项运行/链接产物。Git仍e0eca03db25930be55bdb45477027d1793a61904 / codex/p1-process-boundaries；入口17463项中原未修改17443项，无基线缺失或授权名单外修改。旧三规划/三整体设计/P5覆盖与验证前缀及旧交接完整后缀保全。未穷举全部ignored历史/冻结SDK哈希；操作未触及它们，原P5归档331源/114程序及修前362项逐一匹配。

最终选择的各组源/程序清单与当前产物无身份差异；生产Native/ASAN326和Light340导出均无测试入口，隔离348仍原8个fixture，不新增生产授权接口。失败源/程序归档5456文件，panel-wakeup错时源码的独立匹配来源按第7节单列。源码主清单SHA-256：9A52C4BC8DB62DFCC7051702FED74F021E7BED06577D47FB5A0755A3779B7D4B。正式Light宿主10C32ADF0C7CD1226472179722F0B7195EC4964F7B209A09A99A18A7D19F38FE，ui_framework.dll B0B63AFB6422EA9C4DCA6FDED24294FA7111D0A6D397C7A984C65738E52C9657；其余六uapp由delivery-identity.json提供完整SHA。

cleanup.json删除634个本轮过程文件/234988217字节；cleanup-tail.json在逐项验证生成fixture.txt与实际DLL副本后删除最后一个重复stage及两份临时汇总，另4文件149307字节；累计638个文件235137524字节。CMake对象/缓存和已证明重复stage可重建；旧目录、最终源码/EXE/DLL/uapp/lib、首次失败/原始日志/JUnit、CSV和必要数据库/备份保留。不删除用户历史或在用WAL；没有用户进程终止、Git/远程工作流/真实Provider或设备验收操作。

停止P6开发交付及联合代码评审。未来人工启动和步骤见xuanshu-p6-deferred-manual-acceptance.md；此处不启动人工验收、真实模型或原browser副作用迁移。
