# BR-2费用恢复兼容限定独立复审与冻结记录

2026-11-06。本记录追记本对话上一轮已经完成的限定独立只读复审，并落实本轮文档授权。上一轮完成源码、文件、哈希及既有证据核对。本轮进行对应身份复核和文档收尾，测试结果沿用各自已核对的原始运行。

## 验收及冻结结论

旧策略费用恢复兼容整改已闭环，BR-2本轮限定验收通过，匹配快照可以冻结。必须修正项为空，具备冻结记录落盘和本地交付整理条件。本记录落实该快照的冻结状态。

冻结范围为旧RPCP/RPCB费用恢复兼容、直接涉及的BR-2可靠上界、安全算术、READY前审计、恢复锁停、零新增执行及安全关闭。本轮结论对应以下唯一快照，真实验证准备保持Proposed。

| 身份项 | 值 |
|---|---|
| 匹配交付 | `build/xuanshu-br2-fee-20261106-54d9a8` |
| delivery-identity.json SHA256 | `1EF3D1643AE1DF94ED6B6CC163F061F1376D82E3A5CC540823F1E197687105FB` |
| Git HEAD | `e0eca03db25930be55bdb45477027d1793a61904` |
| 前置交付 | `build/xuanshu-br2-restore-20261106-3c8d42` |
| 前置identity SHA256 | `8FAB0325528D69685E71CB43193B0CEBC68DAF22458091AD2E88C67C2581FDE4` |

本轮编辑前，433项source-final、113项binary-files、5项launch-files及16项delivery-files均与实际文件匹配，identity关联的30份清单哈希匹配。入口455条记录对应449个唯一文件，6组别名内容相同。入口范围及Git工作区状态分别记录，产品增量按独立前态副本界定。[原交付身份](../../build/xuanshu-br2-fee-20261106-54d9a8/delivery-identity.json)

| 实际生产文件 | SHA256 |
|---|---|
| `light/framework_host.exe` | `FA7DB4741BD3F7DBA95ED67142A4FA5F4F4CF9F238BB8AF888D9911ACB988C7F` |
| `light/ui_framework.dll` | `40F74E512FC3D45130D41F4536DEA8D2C7276240759CA5CDB26D6A4FD71492E6` |
| `light/br1-production.uapp` | `C4BA8A621136EA0F1D40F791FA81588CB6C5C129C4C66784A137F489405E0AC9` |

表内路径相对于匹配交付根。参考目录`D:\XuanShu`可读取，保持只读。[本地交付清单](../xuanshu-br2-delivery-manifest.md)

## 闭环依据

旧RPCP/RPCB的estimate保持字节代理估算语义。有效usage按开启记录绑定价格生成known，允许高于该次estimate，并保留合法达到或超过100000000 nano USD的事实。对应请求的未结清预留只扣一次，请求关联、usage类型、金额、价格及重复结算检查继续生效。[费用分支](../../src/ai_business_restore.inc#L174) [原费用合同](../xuanshu-rpc-preparation-contract.md)

BR-2继续要求费用落在可靠计费上界、本次预留和总预算内。known加法先核对可表示范围，预留扣减先核对操作数，后继占用先检查known及estimated再计算剩余预算。阈值后矛盾占用和不匹配usage的金额仍拒绝打开。[恢复算术](../../src/ai_business_restore.inc#L157) [增量规格](../xuanshu-br2-fee-restore-contract.md)

Service唯一worker在发布READY前完成恢复审计。缺开启、误分类、孤立、参数及unknown矛盾沿既有拒绝打开路径处理。完整BR-2恢复原计数并锁停，合法RPCP/RPCB沿原显式恢复入口锁停。恢复后的新提案和旧确认拒绝，原事件载荷保持。原逐任务可信确认、完整预览及UNKNOWN语义继续适用。[Service打开边界](../../src/ai_service.c#L289) [显式恢复](../../src/ai_validation.inc#L26) [直接恢复断言](../../tests/br2_fee_restore.c#L141)

方法、任务、worker及回调引用覆盖恢复、错误收尾与退栈，cleanup-only和安全关闭保持。测试主函数命名宏默认仍为wmain，原入口行为保持。生产358项导出与前置一致，私有可信事件、凭据和传输替身限于隔离测试构建。公开布局、错误值、DDL、SDK0.10-dev/API10/标准10、ABI1/包1、来源政策、C2/P4/P5、固定依赖及原预算和测试超时保持。[开发验收](xuanshu-br2-fee-restore-validation.md)

## 各运行及关后证据

| 已有运行 | 结果 | 原始材料 |
|---|---|---|
| 有效修前red-valid | 13例，5失败、8对照通过 | [日志](../../build/xuanshu-br2-fee-20261106-54d9a8/red-valid-light-test.log)、[JUnit](../../build/xuanshu-br2-fee-20261106-54d9a8/light/red-valid.xml) |
| 较早fixed-direct | 13/13 | [日志](../../build/xuanshu-br2-fee-20261106-54d9a8/fixed-direct-light-test.log) |
| 最终Light | 107/107，184.33秒 | [日志](../../build/xuanshu-br2-fee-20261106-54d9a8/final-light-test.log)、[JUnit](../../build/xuanshu-br2-fee-20261106-54d9a8/light/final.xml) |
| 最终ASAN-Light | 107/107，400.66秒 | [日志](../../build/xuanshu-br2-fee-20261106-54d9a8/final-asan-light-test.log)、[JUnit](../../build/xuanshu-br2-fee-20261106-54d9a8/asan-light/final.xml) |
| 最终Native | 36/36，48.76秒 | [日志](../../build/xuanshu-br2-fee-20261106-54d9a8/final-native-test.log)、[JUnit](../../build/xuanshu-br2-fee-20261106-54d9a8/native/final.xml) |

每行属于独立运行。合法反例由原任务及隔离SSE响应经原结算路径生成。RPCP首轮estimate为10570000，known为30020000，累计30260000。RPCB对应值为5628600、9012000、9096000。RPCP恰到100000000及超过至100001000、RPCB超过至100000200的合法记录同样覆盖。这些金额为隔离响应按本地绑定价格计算的费用证据。

27组合法恢复的known、estimated、unknown、manifest、占用、传输入口和各任务用量逐值保持，恢复后stopped=true、ready=false、prior_session=true。恢复尝试期间新确认、占用和传输入口增量均为0，替身请求为0，原事件载荷逐字相同，引用归还并安全关闭。种子库原有隔离确认、模型请求及工具事实按原范围保留。[状态对照](../../build/xuanshu-br2-fee-20261106-54d9a8/fee-restore-state-match.json)

已有51份费用主样本和48份BR-2主样本共99份明确观测库，无WAL/SHM，immutable只读观测前后哈希一致。本轮引用上一轮已核对的关后证据，未重新打开数据库。[费用主样本](../../build/xuanshu-br2-fee-20261106-54d9a8/closed-fee-db.json) [BR-2主样本](../../build/xuanshu-br2-fee-20261106-54d9a8/closed-br2-db.json)

三次最终运行的源码和程序分别关联测试前清单、日志及JUnit。实际浏览器read、revision、工具结果持久化、3次总额度及第4次拒绝、期限和费用unknown停止、存储失败、完整预览、缓存长历史、取消及DLL/静态/公共头路径的相称覆盖已在上一轮复审核对。清理前VALID编译依赖支持对应运行时构建关系，清理后的状态沿用历史编译证据解释。启动监督8项及辅助2项仅按身份和适用范围复用。[最终关联](../../build/xuanshu-br2-fee-20261106-54d9a8/final-test-match.json) [覆盖索引](../../build/xuanshu-br2-fee-20261106-54d9a8/compatibility-coverage.json)

## 非阻断遗留与证据限制

99份主样本之外另有44份旁文件，包括6份非空WAL。对应库的完整状态需要连同WAL判断，主样本的无旁文件结论仅用于其明确观测范围。现有分账保留。[旁文件索引](../../build/xuanshu-br2-fee-20261106-54d9a8/retained-sidecars.json)

C2037/C2198、初次缺包及夹具空访问与有效red-valid产品兼容失败分别保留。packaging-first仅保存首次路径别名重复计数的元数据及文档上下文。前置红库初始字节身份缺失、历史30.08/30.06/30.11秒、复制error5及其他首败继续独立分账。后续通过的适用范围按其实际程序身份记录。[失败分账](../../build/xuanshu-br2-fee-20261106-54d9a8/driver-errors.json)

BR1-KI-001保持未解决、用户接受非阻断的状态，再现实际模型不一致时停止对应任务并保留证据。临时页面控件测试保持取消。真实来源完整分类、IME、跨屏DPI、辅助技术、人工长时和专用环境断电沿原未验边界保留，browser九项副作用继续另案。[遗留记录](xuanshu-br1-known-issue-disposition.md)

## 原快照与当前文档状态

原交付identity、source-final、binary-files、delivery-files、delivery-final及全部历史证据保留原字节。identity中等待复审的status及交付文档副本记录封装时状态，本记录与当前入口记录封装后的限定复审通过和冻结决定。

本轮文档增量为新增本记录和交付清单，更新开发验收、业务入口、Proposed授权包、交付准备、阶段索引及当前交接，根三规划仅追加checkpoint。原16项增量和433项源码的含义继续按原清单解释，当前文档状态单独核对。冻结仅落实匹配快照记录。

## 下一阶段

真实验证仍待官方模型、协议、价格版本和计费规则核实，以及可靠计费上界依据。条件不足时维持锁停。后续一次策略需绑定唯一新隔离库及监督配置，并集中确认合成数据、固定输入、browser.view、browser.snapshot、state/revision、全部续轮总计3次、可信确认起10分钟、USD0.10及失败停止规则。用户原生凭据录入、逐任务可信确认、安全关闭与证据要求均需独立执行授权。当前授权包保持Proposed。[待授权包](../xuanshu-br2-real-proposal.md)
