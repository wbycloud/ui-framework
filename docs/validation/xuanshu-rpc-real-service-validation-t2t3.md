# RP-C仅T2/T3独立受控真实补测

2026-11-02。最终状态：RPC-K1冻结记录已落盘；新轮原生Key保存/同ref读取成功，直接T2接纳与可信外发确认成功。T2收到完整真实响应，但page=1触发仅接受page=0的适配器拒绝，唯一failed终态已保存；M1/R1/S1/C1/U0，工具0。按停止规则未执行T3、未重复T1或重试；用户正常关闭后只读库已核对。T2/T3限定补测未通过，不署名。

## 1. 身份、隔离与历史保全

承接[RPC-K1限定验收及冻结记录](xuanshu-rpc-credential-independent-review.md)。只使用build/xuanshu-rpc-credentials-20261102-e48d73/light的原probe、生产DLL及rpc_memory.uapp，哈希见该冻结记录和本轮program-identity.json；不修改产品/测试/构建/版本/依赖，不重新构建或跑离线测试。

新证据根为build/xuanshu-rpc-t2t3-20261102-53b8a6，只创建该根下唯一session。启动器创建conversation.sqlite，显式配置为conversation.sqlite.model.json，凭据引用严格由传给model_open的实际UTF-8配置路径派生，不覆盖默认用户配置，不改LOCALAPPDATA。

本轮预期ref为ui-framework/xuanshu/deepseek/d15f7883aeeac194fe15029c233b44bdeb1f20dcd3d2f680373ed5e70803ab38；只记录引用，不读取Windows凭据或Key明文。启动后须核对实际配置/ref及零任务零发送，再由用户原生录入。

更早credential_unavailable且S0的旧轮M1/R1/S0/C0/U0保持；上一轮真实T1成功的M1/R1/S1/C1/U0、usage467/6/473、本地参考USD0.0001473、正常关闭及期限停止也保持。本次不是退款、旧期延期或恢复旧执行；旧库/凭据/截图/清单/程序不改。

## 2. 本轮授权及既有配额门

仅POST https://api.deepseek.com/responses、deepseek-flash、reasoning.effort=none、stream=true、max_output_tokens=4096。一个workspace/Service/主对话、原能力目录和read执行器，不接P4/P5修改/授权/图调度或legacy旁路。

| 任务 | 本轮上限 | 数据与顺序 |
|---|---:|---|
| T1 | 0 | 不点击、不提交、不确认、不重复 |
| T2 | 3 POST，含发现/读取及所有续轮 | 直接从T2开始，绑定新实例及host/open epoch、memory.document/r1/只读scope |
| T3 | 1 POST | 仅T2成功、保存及费用允许后，尝试取消 |
| 合计 | 4 POST | 失败/取消/发送未知仍消费，不返还、不挪用、不自动重试 |

源码tools/xuanshu_rpc_probe.c:27/35及src/ai_validation.inc:59—66按任务独立接纳，没有T1解锁条件；继承已完成T1事实后可直接T2。原产品全局门仍为5、任务门1/3/1，界面T1仍可见；本轮明确不操作T1，T2/T3各自硬上限使这两类合计最多4。不声称已新增产品级总4或禁用T1功能，不改冻结合同。如果实际T2不可用，先诊断，不能通过重复T1解锁。

## 3. 官方核对、费用及停止

2026-11-02只读查阅[Responses接口](https://api-docs.deepseek.com/api/create-response/)和[当前价格](https://api-docs.deepseek.com/quick_start/pricing/)：支持所选模型/路径、effort=none、语义SSE及function调用配对；Flash峰时缓存未命中输入USD0.30/百万token、输出USD1.20/百万token，与冻结300/1200纳美元参考匹配，无实质冲突。未调用模型列表、余额、账户探测或其他API；不启用web_search、文件或其他服务端工具。

USD0.10仅本地停止阈值，known/estimated/unknown分开，不保证服务端硬扣费。usage缺失/异常或必要费用依据unknown即停后继，不按0费用处理。保持网络10/30/30/120秒及首次可信外发确认起整轮10分钟；排队/续轮不延长，到期停止新派发并请求取消，安全收尾等待真实连接/worker/回调退出。

凭据不可用、配额/费用/期限耗尽、越界外发/非read、重复请求、目标epoch/revision失效、存储或关闭异常均停止后继。不得改代码、换模型/端点、重试、换client或另开目录追加；取消不等于远端停止、回滚或免除费用，不清UNKNOWN、不强杀线程。

## 4. 用户操作及精确外发范围

启动默认零任务/零发送。用户先点“设置DeepSeek API Key”，在同名原生窗唯一遮罩框输入Key并保存；等实际credential_saved/config_saved/local_readable=true、saved_revision=revision、pending=false，再复制诊断。助手不取密码框、环境Key或其他凭据；生产程序仅访问本轮新ref进行保存/本地检查/获准请求。

T2输入严格为：`请先发现target-0的memory类只读能力，再调用memory.read读取数值与版本，用一句中文报告；不得修改或调用其他能力。` history=[]，投影capabilities=[memory.read]、fields=[value,revision]。用户核对固定系统指令、xs_read_discover/xs_read_call两read包装定义和准确预览，亲自点击“确认外发”一次。

唯一描述为Read synthetic memory value and revision / Read-only RP-C fixture; value 10 and revision r1.；memory分类、read效果、document目标、契约/能力版本1。参数为{}，输入Schema为object且additionalProperties=false；输出Schema为object，value为integer、revision为string、两项required、additionalProperties=false。允许同一read描述的Schema/指纹及实际value=10/revision=r1结果，不发送其他应用/业务文件/完整主对话。目标本地身份只用于固定与核对，模型使用target-0别名，不额外上传旧进程ID。

T2成功须有真实发现描述、实际整数10、字符串r1、框架结果revision、原调用关联和工具结果保存；仅模型回复或计数不算成功。全部模型续轮合计不超过3，完整工具批次验证后才串行执行，失败不补请求。

T3固定输入为`请用中文解释什么是流式输出，最多三句；不要调用工具。`，history=[]、无目标/空投影，同一固定系统指令和两read包装定义。用户确认后尝试“取消任务”；若响应已先完成，明确在途取消未覆盖，不追加一次制造场景。

## 5. 取证、关闭及当前状态

分别记manifest M、配额R、实际WinHttpSendRequest进入S、完整响应C、发送未知U，模拟数另列；model_calls不是精确HTTP次数。区分received、保存、外发确认、请求保存、发送、工具完成、最终消息及唯一终态保存。日志仅稳定ID、状态、usage/费用、只读业务结果和程序身份，不含Key、Authorization、Blob或完整内存。

等用户正常关闭且进程退出后，仅在WAL/SHM不存在时只读本轮隔离库核对；不在线绕锁、不重开Service、不操作旧用户库。不自动删除本轮凭据/库/证据。请求摘要重建只作范围核对，不冒充原始抓包；没有原始HTTP头或逐片视觉观察时如实记限制。

本轮入口baseline414项是403源码、8份授权既有文档/规划及3份合同，不称全工作区；两份旧identity/全部源程序证据匹配后才修改授权文档。真实结果待追加。历史首败、期限/frontend/输入/browser风险及延期人工项目不覆盖；只有T2/T3实际条件满足才报告补测通过，取消未覆盖则部分完成。

## 6. 实际启动（等待原生凭据确认）

2026-11-02 14:41:01（北京时间）启动原Light匹配probe，PID75568；准确命令见launch.json。14:41:40只读进程/配置观察为Responding=true、正确中文标题、profile revision1、deepseek-flash及本轮派生ref一致。没有读取凭据、密码框或在线库；配置文件存在不等于Key保存成功，也不替代用户复制的Policy零计数。

唯一session由原启动器创建，没有复用旧目录、默认用户配置或旧凭据。接下来只请用户打开“设置DeepSeek API Key”原生窗、保存并复制实际诊断；未显示保存/本地可读确认前不推进T2，不操作T1。启动记录及预检均是本轮新证据，旧identity未更新或覆盖。

## 7. 实际原生确认、T2及停止

用户复制的软件状态明确window_opened/submitted/credential_saved/config_saved/local_readable=true，stage=readable、error空、四阶段错误码0，模型revision与saved_revision均2、pending=false，ref与本轮派生值一致。此时Policy全0、task_used=[0,0,0]、task=none，证明没有先执行T1；仅聊天“已录入”不作保存证据。记录见credential-user-diagnostic.json。

T2直接创建run `ed833078c229a1db1810ee0d1ea2ebbd7a8c075b9c4238fb4715502804afb9f2`、client `rpc-task-102-1`，初次复制为phase3/committed=true但disclosure_confirmed=false、M/R全0；随后仅继续确认同一run，没有重提T2。固定新实例1、host_epoch `0dc3fce7-8ea3-46fc-a9e8-2af7cc7d89dd`、open_epoch `55a7f8ce-151c-428d-b81a-d5e069ddbc1b`、memory.document/r1/read scope，history=[]及原投影未变。

可信确认后provider_request `91dc0b87273f0686638f0319268e203b3e6ad57067eb2c0fd1f1d27ce3121e2b`仅发送一次，收到完整Responses；服务端响应ID `54408390-bc9d-4709-8b3e-e8df5a1fd9e7`，status=completed/error=null。协议响应完成与本地任务成功不同：本地任务最终phase7、outcome=failed、error=batch_wrapper_invalid、committed/terminal=true，tool_calls=0。disclosure_saved/model_request_saved均true。模型“我会先发现…”只是commentary，不是目录或业务读取证明。

当前Policy stopped=true/error=validation_task_not_completed，M1/R1/S1/C1/U0、task_used=[0,1,0]、模拟计数0；known_nano_usd=256200、estimated=0、fee_unknown=false、send_count_known/send_count_saved=true。没有期限或费用耗尽证据。按约停止T3和后继请求，不返还剩余额度、换client、重开或再建会话。失败诊断见t2-user-failure-diagnostic.json。

## 8. 源码与真实响应定位（未修复）

实际唯一function_call为xs_read_discover，arguments完整JSON为：

```json
{"target_alias":"target-0","category":"memory","page":1}
```

src/ai_model.inc:151的am_step只接受无符号page=0；本次其他包装字段、目标及category匹配，但page=1使其返回NULL，:165记录batch_wrapper_invalid。工具请求尚未进入原P2发现/dispatch，库中tools=0、无tool_requested/tool_result。没有真实发现描述、整数10/字符串r1或框架结果revision的完成证据，不能由固定目标r1和模型文字替代。

同时:138的生产工具定义只公布page.type=integer，既无仅0取值约束，也未说明page=0-only；故本次是RP发现包装的公开参数说明与实际接受域不一致，不是半JSON、Key失效或DeepSeek认证拒绝。tool-rejection-analysis.json保存实际响应/参数、源位置和判断依据；没有动态重跑、模拟请求或修复。

建议下一轮最小方案是先单独授权工具声明整改：保持page0语义及page1拒绝，明确仅允许0的描述/Schema，补直接解析真实序列化工具定义和page0/page1的离线回归，再评审后补T2/T3。不得悄然把page1映射0、放宽门、业务自动重试或追加当前真实请求。RPC-K1冻结结论仍仅覆盖此前凭据/诊断范围，不能当作该兼容缺口或全部RP-C已通过。

## 9. 关后只读对账及交付

用户报告正常关窗；14:49:50只读确认PID75568不存在，没有强杀或重启，也没有捕获数字退出码。仅在WAL/SHM不存在后，以mode=ro&immutable=1读取本轮隔离库，未创建sidecar。65536字节主库SHA-256读前读后均为`D70CFFD5562ACC07702AC2B7F982105C08D5849EF46ED5D8571ED1A04B5E9B1E`。

库中一主对话、一项T2、两条消息、零tools、12条顺序事件、唯一failed终态：rpc_open、accepted、awaiting_disclosure、disclosure、provider_request、rpc_reserved、rpc_send_intent、rpc_send_entered、rpc_network_end、provider_response、rpc_cost、finished。唯一请求/消费为task2 ordinal1，真实发送及完整响应分别保存，未出现T1/T3、重复请求、续轮、工具派发或发送未知。停止原因来自实时Policy；不补造额外持久化停止事件。

usage输入494（缓存256）、输出90（reasoning0）、合计584。冻结保守参考按全部输入494×300＋输出90×1200得到256200纳美元，即USD0.0002562；有效usage后原估算USD0.0056175已释放为0，unknown=false。没有按缓存折扣查询账户账单，本地参考不是实扣或硬费用保证。服务端created_at/completed_at换算北京时间为14:47:49/14:47:50，不冒充本地发送时间。

provider_request与rpc_reserved的body_hash同为`1cf9f5d7a45180c9bad3827e5375f10fd9e567618bd6b85b6507789d7a93ea01`。按未变am_body插入顺序、固定工具定义及持久化初始上下文重建1317字节的t2-expected-body.json，与摘要一致；它不是抓包。实际发送范围仅原合成输入、空历史/原投影、系统指令及两个read包装定义；没有发送业务文件/其他应用或实际业务读取结果。HTTP200/text-event-stream由生产成功门推论，未保存原始响应头。

closed-db-observation.json保留19项对账、原合成消息/事件及usage；final-preservation.json、delivery-files.json和delivery-identity.json关联本轮文档、程序及旧冻结身份。无产品/测试/构建修改、离线重跑、Key明文提取或其他API调用。本轮无清理删除，隔离凭据/配置/库及匹配程序、历史失败和延期项目保留。

本轮最终结论：RPC-K1冻结记录完成；T2/T3限定补测未通过，已定位page声明缺口，T3在途取消未验。停止本轮结论，不自动修复、重启或追加请求。
