> 2026-11-11。PB-2匹配快照冻结记录已归档，PB2-REAL-20261111-8f3c2a唯一真实正文任务已完成并正常关闭。生产PID28300实际exit0；147字节正文、revision、12/9/3、林青与周明及复核三台设备逐值对应。关后1任务、2消息、31事件、2工具，唯一completed；3次占用、非模拟传输入口和完整响应，unknown0；本地保守费用USD0.0010926，未结预留0。该包已消费，当前没有本轮运行进程、待Key、待确认或继续执行操作。后续新真实任务需独立具体授权，原默认模板维持Proposed锁停。 详见[本次完整结果](xuanshu-page-body-policy-real-20261111-validation.md)和[证据目录](../../build/xuanshu-page-body-real-run-20261111-8f3c2a)。

## 以下历史记录按各自时点保留

> 2026-11-11当前状态：PB-2匹配快照已限定独立通过，冻结记录已归档。见[冻结记录](xuanshu-page-body-policy-independent-review.md)及[本日准备](xuanshu-page-body-policy-real-preparation-20261111-validation.md)。PB2-REAL-20261111-8f3c2a仍Proposed，等待一次具体批准；没有生产执行或真实请求。11月10日正文包已失效，以下原文按当时状态保留，不构成当前操作或授权。

# PB-2正文验证保护开发验收

2026-11-10。开发、自审及离线验证记录，不是独立复审、冻结、真实调用或人工验收。前置PB-1 identity CEFE1130580DE40480F5D5963F4DFD172A45CD7E59FB0EB10C1175E19AD764B0在本轮开始匹配，原快照冻结已单独[归档](xuanshu-page-body-independent-review.md)。新根build/xuanshu-page-body-policy-20261110-d42b8e，551份选定入口源码/文档及Git状态界定增量，不是全Git内容快照。

## 实现及直接证据

先冻结[最小合同](../xuanshu-page-body-policy-contract.md)，再取得原运行时拒绝正文提案的1例1断言受控红例：PERMISSION_DENIED，0模型替身请求。同期代码、DLL、程序和输入包保留于red-source/red-program。该证据证明原固定策略不支持正文，不外推真实故障。

产品只改4份私有实现：ai_validation.inc增加场景枚举及开启/状态JSON，ai_business_validation.inc按场景选择固定输入/能力/字段，ai_model.inc只为正文任务保存validation_scene，ai_business_restore.inc在READY前交叉校验两份标记和固定范围。缺省及显式snapshot-v1保持旧范围；dom-body-v1只允许固定PB-1输入、history=[]、browser.view、browser.body、body/revision。未知场景或正文使用旧估算旁路锁停。没有新增Service、Provider、执行器、线程或公开结构。

请求总额、逐轮完整计量、av_reserve/av_before_send、费用、期限和POST不重试均沿原链，没有第二个发送入口。所有初始与续轮共用3次额度，第4次传输前拒绝，失败不退占用；可信确认起10分钟，费用unknown/计量/存储失败禁止后继。公开价格和新真实包另外准备，开发中的价格/凭据/网络均为替身。

36项专用用例直接读取实际界面、请求和关后记录：本地接纳/完整预览，147字节正文及revision进入模型续轮和工具存储；目录模型结果只含允许browser.body，持久化保存原始read目录数组，二者分别比较。初始request的model/instructions/tools/reasoning/max_output_tokens/stream及前两条输入逐值相同；计划中的输入、history、场景、能力/字段和目标也直接核对。

多标签、草稿、原revision、PENDING解绑/隐藏拒绝、重复确认和同步回调重入保持。取消、费用/usage/计量/断流/存储停止及cleanup-only通过。完整恢复保留known/estimated/unknown、manifest、占用及传输入口等逐值；缺开启、场景缺失/改写、任务标记缺失/改写、误分类、范围矛盾和孤立占用通过实际Service打开在READY前拒绝。恢复后新提案和旧确认均拒绝，新增传输/占用/确认为0。

## 最终运行分账

| 配置/运行 | 实际结果 | 最终适用范围 |
| --- | --- | --- |
| Light closure-final | 201/201 | 其中新36例后来加强断言，其余165例程序和产品不变 |
| Light strengthened-final | 另次36/36 | 最终正文策略夹具，替代对应较早36例 |
| ASAN-Light direct-final | 36/36 | 最终正文策略直接用例 |
| ASAN-Light compatibility-final | 另次165/165 | 共享运行时、旧策略、费用、显示及生命周期 |
| Native closure-final | 7/7 | 公共C/C++、静态嵌入、解析/池及真实DLL |

最终逐项Light201、ASAN201、Native7；不是首次单次全部通过。最终程序、测试前源码/程序清单、原始日志、JUnit和编译依赖由verification-summary/final-test-match关联；较早通过程序单独作为上下文保留，不称为失败程序。Native期间新测试源的局部变化未进入其编译依赖。

兼容运行包括旧RPCP/RPCB合法known高于estimate及达到/超过阈值、unknown与矛盾对照，BR-2恢复缺口/费用/期限/计量及3次门，正文/原browser生产公开read，完整长历史112/48页、缓存中断、旧代次/显示失败、取消、存储、回调、真实DLL和安全关闭。原时限、合法数据范围及断言未放宽。

## 首败与限制

- direct-smoke的目录1项期待错误、direct-all的8项拒绝打开后返回值期待错误属于新夹具；实际产品已分别给出原始目录和UNAVAILABLE。修正为准确层次和返回值，补新提案/旧确认拒绝，没有改变产品过滤或打开条件。原程序/源码/日志/JUnit/自有库保留。
- 增强断言时局部request同名触发C4456/C2220及/WX，未形成新程序；仅改局部名称，编译失败源和日志保留，不降低警告等级。
- PB-1的20.03秒初始化首超时及其他历史期限/复制风险、首败和后补源码恢复限制保持。新通过不解释或解决它们。BR1-KI-001仍未解决、用户接受非阻断，本轮未观察到实际模型名不一致，不主动重复定位。

正式宿主页隔离DLL/可信事件/模型替身全链与生产DLL公开read分账，不拼为生产真实人工授权。脚本摘要不证明真实模型质量。种子库有隔离确认、请求和工具；零增量只描述恢复后尝试，不称整库零模型路径，不虚构M/R/S/C/U或精确HTTP次数。

## 身份、关后及交付

72份最终正文策略主库在实际测试退出后检查旁文件，用immutable/query_only核对前后哈希及范围、工具和额度事实。破损副本和其他旁文件独立保留，不扩展无WAL/SHM/journal结论；原历史库未操作。原错误和unknown不清除，不恢复授权或重放工具。

生产358项导出与PB-1一致，无私有入口。公开头、布局、错误值、DDL、版本、来源、C2/P4/P5和固定依赖不变；运行时99份中仅4份私有增量，另95份匹配。50ms、8MiB JS堆、256KiB脚本栈、128KiB模型、2MiB宿主、8MiB Service、Light256KiB及原测试时限保持。依赖在对象清理前捕获，清理后不声称obj仍在。

最终源码/程序/文档与证据以新根delivery-identity.json为准；仅清理本轮无用途对象和未运行的早期ASAN夹具副本，保留实际红例、失败、较早通过的必要上下文、原始日志和库/旁文件。匿名文档、交接和索引更新，根三规划只追加。

新[交付入口](../xuanshu-page-body-policy-entry.md)和[具体真实包](../../build/xuanshu-page-body-real-preparation-20261110-d42b8e/authorization-package.md)准备。当前开发必须修正项为空，具备限定独立复审条件；新程序未冻结。真实包Proposed，独立复审及具体批准未完成，不读取真实凭据、不发送真实请求、不启动人工验收。
