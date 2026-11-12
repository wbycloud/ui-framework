# P6R-1限定独立复审结论与冻结记录

记录日期：2026-10-28。状态：限定验收通过，匹配交付快照可以冻结。本文追记本会话上一轮独立只读复审，不是本轮重新运行测试或重做P2—P6全面复审。冻结仅为交付身份和结论记录，不执行commit、tag或release。

## 1. 结论及适用范围

上一轮结论为：“三项整改闭环，P6R-1限定验收通过，当前交付快照可以冻结。”必须修正项为无。适用范围是Coordinator完成结果引用、存储失败后的在途cleanup-only及PASSIVE checkpoint错误后的推进门，以及相称生命周期和交付证据。

本结论不等于真实人工验收、无限负载稳定性、无条件P6完整验收或原P5所有副作用迁移完成。真实Provider尚未实现，不能由本次冻结推导其已获实现或联网授权。

## 2. 冻结身份与本轮有限核对

框架分支`codex/p1-process-boundaries`，HEAD `e0eca03db25930be55bdb45477027d1793a61904`，保留未提交修改和未跟踪材料。原P2—P6与整改归属按本地基线和增量清单判断，不采用HEAD到工作区全差异作为整改范围。

证据根：`build/xuanshu-p6r1-closeout-20261028-91c4e7`。以下路径均相对此根；完整逐文件身份留在原清单，不重新生成或改写旧证据。

| 清单 | SHA-256 |
| --- | --- |
| baseline.json | `7D10A1097F469A77276537A6B2D6047084F0562D0A67549C31E55E32714BADC7` |
| baseline-supplement.json | `89D9A2FA4D203015E968FA5351CB988C0085CC616A983036911BB7A89C4E7F56` |
| delivery-files.json | `FE32B61DB2D34C0690C83BE1614DC444898D3D19BCC0CEF4F0ADC702DDB18C83` |
| source-after.json | `1664A6BBC5F37A4B51F99DCF853298B6D4A62E8FE23B2A4DD76E03BD786EFAF4` |
| binary-files.json | `28DC53EF78A8EF67993A4BEFAC3BCE5F4115E66BE3F1019880A1060960AC2B33` |
| red-binary-files.json | `267AF974C75618FB46E1580420A7B90B865967C186428277A48CC7B737621B0F` |
| delivery-identity.json | `BCCD9780B787AD7A5BAFBE8EB7368CE38BD2BCC5D8E56B15D3414EC985EF69C2` |
| verification-summary.json | `A5D794C0F4F380C6476ACA06E89E5799A7C0B518F67270B6DD7AC663D9EC92FB` |
| preservation.json | `8AA0626B5E3B4EC707D766794BBFE49F53782BAFD7FBD35FF1C9EB0BD81F4276` |

| 代表性产品二进制 | SHA-256 |
| --- | --- |
| native/ui_framework.dll | `141D4224424AFAAC75ED43C5D9595F971B5A7D338CFB0A8B532DDF026A6C9FBA` |
| light/ui_framework.dll | `4E9B2109B49C93CCB8AB48CCA5D5FD93360F9960CB94F7025104EF8D2DE05AD9` |

本轮在文档修改前只读复算：374项源码、102项最终运行/链接/PDB、9项red程序及15项原整改交付全部匹配。不是新测试，也没有更新旧清单。本轮随后仅新增两份文档、更新五份授权记录；原delivery中验收/交接/根三规划的文档哈希因此有预期差异，新增评审/提案不属于旧交付。冻结产品源码与程序身份不变，文档增补不能自动纳入旧测试结论。

入口自动Git清单为17470项；三份被忽略的根规划由事后补充恢复原始前缀并与原P6归档核对。17473是合计保全核对数，不能描述为全部入口自动捕获。原P6对照根`build/xuanshu-p6-20261027-4a92d1`及全部旧失败保持原样。参考工程`D:/XuanShu`本轮Git读取仍Permission denied，未扩权、未写入，不声称核验其新HEAD。

## 3. 上一轮源码判断

| 项目 | 结论与源码依据 | 受控证据及限制 |
| --- | --- | --- |
| 引用移交 | `src/ai_graph.inc:67,98,149–154`将排队引用移交至待消费结果，UI核对slot、稳定run ID和graph后释放一次；`src/ai_service.c:74`淘汰仍受refs约束。身份异常可观察，UI不等待通知空间 | `tests/ai_graph_closeout.inc:21`经真实64槽压力迫使淘汰，检查待消费身份、refs=1、消费后refs=0及不重复释放。修前是有界断言失败，不是崩溃复现 |
| 失败后收尾 | `src/ai_graph.inc:158–161`独立遍历在途子调用，不依赖active、根终态及c.failed；`src/ai_edit.inc:119–135`只取消/收取已接纳完成并经原finish/release收尾，不创建新业务阶段或存储job。实际提交字段及UNKNOWN资源门保留 | 子意图、真实DLL执行、最终确认，以及关闭中/安全移除实例read两条路径。worker/外部回调未退出时不能提前卸载；5秒排空失败不冒称永久死锁 |
| checkpoint推进门 | `src/ai_store.inc:17–24`保留COMMIT成功事实及BUSY/LOCKED处理，同时拒绝不可用存储的新事务；`src/ai_service.c:229,242–261`及图/子编辑后继阶段再检查可用性 | 第32次running/graph、第64次子意图精确故障；SQL检查前置保存与消费、后继业务为零、无重复终态；冷重开不重放 |

上一轮还核对了只读关闭路径、现有回调/复制/稳定ID和卸载门，未发现阻断整改验收的新源码缺陷。SDK0.10-dev/API10/标准10、ABI1/包1、旧公开布局、v1只读语义、冻结C2/P4/P5、固定依赖和现有限额保持。

## 4. 已有自动证据及失败分账

以下为整改轮已有结果，本轮未构建、未运行测试或程序：

| 组 | 已有结果 |
| --- | --- |
| Native verified | 96/96 |
| ASAN verified | 96/96 |
| Light verified | 首次55通过、1项因目标未构建而Not Run |
| Light graph-apps | 补构建后该项1/1，416检查/0失败；逐项覆盖56/56，不是首次单次56全通过 |

新增9个受控用例和原Service、Coordinator、编辑、存储、公共C/C++头、Assistant、静态嵌入、真实DLL两分支及Light投影提供相称证据，不能以数量替代源码判断。修前/后源码、原始日志/JUnit、失败和匹配程序分别保留；原实现有界失败、新夹具屏障不足、ASAN未接纳故障、Light编排错误/Not Run分别记录。详[原整改记录](xuanshu-p6r1-closeout-validation.md)。

上一轮逐文件核对失败归档及隔离数据，并直接读取PE导出与旧P6比较。生产Native/ASAN各326项、Light340项，无fixture；隔离DLL334/334/348项，仍为原8个测试入口。未发现放宽断言、20秒CTest、5秒状态轮询、数据范围或生产额度。清理记录属于此前整改实施；本轮不执行清理。

## 5. 非阻断事项与延期边界

四组B类期限风险继续未定位：并行graph_boundaries超过20秒；ASAN控制末尾任务未在原5秒内结束；P2五包首次15.04秒超时；面板180.44秒超时。原日志分别为原P6根`compatibility-final-native-test.log:321`、`compatibility-final-asan-test.log:81,325`、`joint-final-light-test.log:29`、`panel-wakeup-light-test.log:15`。串行通过不证明根因已解决，不默认归因环境。

上一轮未找到这些期限失败造成重复修改、UNKNOWN误清或提前卸载的证据，因此不阻断三项整改限定冻结；负载下的完成期限仍不能承诺。建议后续增加阶段时延诊断，保留原阈值，不删除首次失败。

C类非阻断补测建议：`ae_unavailable`在存储不可用时保守转UNKNOWN并保留事务提交字段；当前新增最终确认窗口暂停于AE_FINAL提交前，未单列AE_FINAL/AE_PROOF已提交、随后checkpoint失败的组合。后续可验证成功事实、原门控及冷重开投影，但不是本轮已证实的安全缺陷或冻结前必修项。

旧frontend63、ASAN-Light中断、Light surface_input、browser20、workspace首次20.05秒（门槛20秒）、Native首次输入失败保留原结论，没有足以归因此次整改的新证据。

browser九项副作用navigate/back/forward/reload/stop/home/help/settings-save/settings-reset按已确认另案，不作为本轮漏实现。真实设备、Windows来源动态正例、IME、DPI、视觉、辅助技术、物理断电及人工长时继续[延期集中验收](../xuanshu-p6-deferred-manual-acceptance.md)，不自动阻断本限定结论。

## 6. 停止位置

本轮已将上一轮结论及匹配快照落盘；原证据根不写入。仅开展真实Provider的Proposed规格评审，见[接入提案](../xuanshu-real-provider-integration-proposal.md)。不得据此连接真实模型、读取凭据、发送业务数据或修改产品实现。
