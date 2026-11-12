# RP-C凭据整改后的第二轮受控真实验证

> 2026-11-02后续：本轮T1成功与T2/T3期限停止及原M/R保持。RPC-K1已限定独立复审通过并[记录冻结](xuanshu-rpc-credential-independent-review.md)；用户另授权[新T2/T3补测](xuanshu-rpc-real-service-validation-t2t3.md)，不是恢复本轮、不退款或改写本记录，也不重复T1。

> 新补测实际为T2一次真实POST后page=1被仅0适配器拒绝、failed保存，工具0，T3停止；与本记录T1成功分账。该新结果不覆盖本轮期限停止或更早credential_unavailable首败，也不能合并宣称完整RP-C通过。

2026-11-02。最终状态：真实原生输入、后台保存、本地读取和T1生产文本完成；准备T2时Policy已validation_deadline，T2/T3未执行。用户正常关窗，PID76012已退出，关后只读库确认唯一成功终态及M1/R1/S1/C1/U0。本轮部分完成，不能报告整轮RP-C限定验证通过；不延长期限或续跑。

## 1. 新轮与旧轮隔离

本轮开发证据根`build/xuanshu-rpc-credentials-20261102-e48d73`，真实证据使用其`real`目录，启动器只创建新的`real/session`；数据库为conversation.sqlite，显式模型配置为conversation.sqlite.model.json，ref由该配置路径派生，不修改LOCALAPPDATA或用户默认数据。

旧轮`build/xuanshu-rpc-real-20261102-b5e731`的M1/R1/S0/C0/U0、工具0、失败终态/库/截图/程序保留，不退款、不改写、不据此继续旧会话。用户纠正未看到可输入Key的界面，聊天“已录入”不等于软件保存确认；旧轮不是DeepSeek认证失败。本轮新授权最多5次不是旧轮额度重置。

实际分支codex/p1-process-boundaries，HEAD e0eca03db25930be55bdb45477027d1793a61904。新代码/程序身份以本轮source-after/binary-files/delivery-identity为准，不自动继承原冻结验收，也不把本轮自审写成独立复审。

已核对Light候选：probe SHA-256 `307E8E783D167CC38AA151C6C5275A420D7CDF0D2144EF2D930F084D684A9806`，生产DLL `8B2DCB82FA7F72F5E6974FFAFD427DA71CC706B336D09D00C97D854DA0F7F1E0`，只读包 `04FAA57382D17BBF81463B34FF6A8199E9524D0317819C7B2565852248A3D33F`。程序不带私有输入或传输替身，启动不自动提任务。新状态/原生窗/复制验证不表示真实服务已验。

离线delivery-identity SHA-256 `0CBE4D1CA7A43E6557F7783402E033D10B3FA8FB6B978324326D809FEC1D3317`，403源码、21项交付、139最终程序/链接/PDB、118失败归档；仅清理新根559个obj，红测及匹配程序保留。启动前real/preflight再次核对源/程序/依赖及18清单、45证据无差异。此后本轮运行文档/规划追加属于另授权运行记录变化，不自动并入离线测试快照，产品身份不变。

launch.json保存准确命令、隔离session及时间；新进程PID76012，标题正确、Responding=true，未自动提交。显式模型配置已生成，model=deepseek-flash、revision=1、ref与新配置路径哈希一致；这些不是Key保存证据。没有对独占在线库绕锁查询，也没有操作凭据或读取原有Key。已请用户只点击“设置DeepSeek API Key”，看到明确遮罩框后录入/保存，待实际“保存成功，本地可读取；服务端未验证”再用“复制诊断”提供状态；暂不点任务或确认外发。若没有窗口/保存确认先诊断，不推进T1。

## 2. 本轮使用的操作步骤（已结束，不是再次启动许可）

离线通过及身份核对后，启动匹配Light/xuanshu_rpc_probe.exe和同目录ui_framework.dll/rpc_memory.uapp。标题为“玄枢 RP-C受控验证（仅三项合成任务）”，默认没有任务或发送。任务入口在凭据保存和本地可读取前不可用。

1. 用户点击“设置DeepSeek API Key”。应出现同名原生模态窗，唯一输入框明确标注DeepSeek API Key且遮罩，没有用户名/登录密码问题。
2. 仅在此输入框输入真实Key，点击“保存”，不在聊天、命令行或配置文本提供Key。
3. 等主窗口显示“保存成功，本地可读取；服务端未验证”，credential_saved/config_saved/local_readable均true，saved_revision与模型revision相同、pending=false。正在保存或失败不算成功，不推进T1。
4. 如窗口不出现或没有保存确认，点击“复制诊断”提供脱敏快照；先核对入口last_entry、prompt_code、write_error、config_error、read_error，不要求填写不存在的窗口，不自动绕过来源或重试真实请求。

用户反馈、软件提交job、实际写入、配置保存、本地读取和服务端验证分别记证。复制只读，不取密码控件或授权，不发送。实际原生/设备正例只记录本次实际覆盖，不外推IME/DPI/辅助技术等全面验收。

## 3. 唯一允许的请求与顺序

只使用DeepSeek官方POST https://api.deepseek.com/responses、deepseek-flash、reasoning.effort=none、stream及max_output_tokens=4096。发送前只读核对官方公开协议/价格，不访问模型列表/余额/账户API。若实质冲突则停，不换端点、型号或协议。

2026-11-02在本轮离线完成后再次只读核对[官方Responses接口](https://api-docs.deepseek.com/api/create-response/)及[价格](https://api-docs.deepseek.com/quick_start/pricing/)，仍支持所选路径/模型/非思考；峰时缓存未命中输入每百万USD0.30、输出USD1.20，与冻结300/1200纳美元参考一致，未见实质冲突。没有调用任何真实API，不以公开资料证明账号可用或账单精确。保守采用峰时，不启用厂商web_search或其他网络工具。

每任务history=[]，固定系统指令及xs_read_discover/xs_read_call两个封闭read工具定义在外发范围内；同Service/主对话、同一能力目录及固定目标。无业务文件、其他应用快照、完整主对话或AI修改/P4/P5授权/图。

| 任务 | 输入/数据 | POST上限 | 当前执行 |
|---|---|---:|---|
| T1 文本 | 只用中文回复“玄枢流式验证成功”。不要调用工具。无目标 | 1 | 用户诊断：completed/terminal/committed，真实S1/C1，回复准确 |
| T2 只读 | 原计划要求先发现target-0的memory类能力，再memory.read；仅memory.document和既定描述/Schema/value=10/revision=r1 | 3，含全部续轮 | 未执行；当前期限已到、入口禁用 |
| T3 取消测试 | 原计划无目标、最多三句的流式解释 | 1 | 未执行；按本轮期限停止 |

T1正常完成且最终保存后才T2，再T3，各项由用户亲自核对预览并“确认外发”。所有失败/取消/发送未知消费不返还、不挪用、不自动POST或工具重试；重复ID不重发，不另开目录或换client续跑。取消若晚于完成只记在途场景未覆盖，不追加请求。

USD0.10本地停止阈值不是服务端硬扣费保证；有效usage的known、保留estimated及unknown分别记录，缺失usage不记0，必要费用依据无法核对则停。网络10/30/30/120秒及首次可信确认起10分钟不延长，安全收尾等待连接/worker/回调真实退出，不强杀或提前卸载。

## 4. 证据、停止与未验

分别记M（manifest）、R（配额）、S（SendRequest进入）、C（完整响应）、U（发送未知），模拟数另列，model_calls不是精确HTTP次数。入队、保存确认、外发确认、发送、远端完成、工具10/r1和最终唯一终态保存分开核对。

凭据不可用、越界外发/重复、额度/期限/费用unknown、目标epoch/revision变化、存储或关闭异常立即停后继发送，cleanup-only，不清UNKNOWN、不自动改代码/重试。运行失败保留脱敏阶段/系统错误码、稳定ID、时间/HTTP/usage、库及程序；不记录Key/Authorization/Blob/内存。

本节起草时真实服务尚未执行，实际覆盖以第5节起的追加记录为准；一次生产HTTPS成功不代表完整TLS/代理故障矩阵已验。尚未执行的发现/read、取消和关闭不得预先宣称通过。历史期限/frontend/输入/browser首败、原tools最早捕获限制及全部延期人工清单保持，browser九副作用另案，不被新结果覆盖。

## 5. 实际原生录入及T1

用户通过新“复制诊断”提供的软件状态：window_opened/submitted/credential_saved/config_saved/local_readable均true，prompt_code/write_error/config_error/read_error均0，stage=readable、last_entry=accepted、dialog_open=false；模型revision与saved_revision均3、pending=false、error空。ref与本轮startup/preflight派生引用一致，M/R/S/C/U及模拟计数均0。该证据不同于聊天“已录入”，证明本轮原生窗口已实际提交、后台写入/配置保存/同ref读回完成；仍不等同于服务端认证。

T1最先复制的是phase=3/committed=true/disclosure_confirmed=false、M0/R0/S0，随后继续同一run等待或完成可信确认，没有重新提交T1。最终复制状态run=e808506b9b974dd5aee3d39fbc6be4e9faae013d2e00d32ab7a4abf7eed843ac、client=rpc-task-101-1、request=8cfd833139eaee5641cd84105a8af3a31acfeca18692f15e89fc22eea561d60e，phase7、completed/terminal/committed、error空、reply=玄枢流式验证成功；disclosure_confirmed/disclosure_saved/model_request_saved均true，remote_delivery=response_received、model_calls1、tool_calls0。

T1 Policy为M1/R1/S1/C1/U0，task_used=[1,0,0]，模拟计数全0，send_count_known/send_count_saved=true、当时stopped=false；known_nano_usd=147300（USD0.0001473），estimated=0、fee_unknown=false。此为有效usage/本地固定费率计价，不是服务端账单实时核对。后续关后只读对账见第7节；单张最终快照不证明用户观察到了每个流式片段。

对应非敏感证据credential-user-diagnostic.json、t1-awaiting-confirmation.json和t1-user-diagnostic.json在real目录。记录时间是取证时间，不伪造精确HTTP开始时间。已给T2当前一步，限制唯一memory.read及字段/10/r1、history=[]和同一目标，最多3次含续轮，不改Key或重开，不推进T3。用户复制诊断实际可用，本轮无需手抄JSON。

## 6. T2禁用及期限停止

用户反馈T2灰后先请求当前复制诊断，没有预设为环境或超时。该实时状态明确为stopped=true、error=validation_deadline；M1/R1/S1/C1/U0、task_used=[1,0,0]保持，T1仍completed/terminal/committed，凭据stage及四项确认正常。当前禁用原因是冻结10分钟期限，不是新的凭据失败；首次反馈灰的更早时刻无同步快照，不补造时间或执行事实。

按合同停止新派发，不延长、返还/挪用额度、换client、另开目录或重启续跑。T2真实发现/read10/r1和T3在途取消均未覆盖，不将T1成功写成整轮通过。用户随后确认正常关窗，PID76012不再存在；没有强杀或操作旧库/凭据。deadline-user-diagnostic.json记录该实时状态，停止原因来自UI，没有伪造独立持久化deadline事件或精确触发时间。

## 7. 关闭后只读对账

仅在用户确认正常关闭、进程不存在且WAL/SHM均不存在后，以SQLite mode=ro&immutable=1读取本轮隔离库；没有在线绕锁、Service重开、恢复执行或新API调用。主库65536字节，读取前后SHA-256均为`B925B8A571C4761562F445F76CA62FFA41E730F9D62CB447954E56B361611E8F`，没有创建sidecar。没有捕获进程退出码，不补写为0。

库中只有一条主对话、一项T1、两条用户/助手消息、零工具记录和12条顺序事件：rpc_open、accepted、awaiting_disclosure、disclosure、provider_request、rpc_reserved、rpc_send_intent、rpc_send_entered、rpc_network_end、provider_response、rpc_cost、finished。op_id唯一，唯一terminal=1，最终任务completed，准确回复“玄枢流式验证成功”；disclosure、模型请求、额度消费、真实发送进入、完整响应和最终保存分别存在，未以model_calls替代HTTP计数。T2/T3无任务或请求记录。

实际usage为input_tokens=467（cached=0）、output_tokens=6（reasoning=0）、total_tokens=473。按冻结参考300/1200纳美元每token计算147300纳美元，即USD0.0001473；保留估算已释放为0，fee_unknown=false。这是有usage依据的本地参考值，不是查询账户得到的账单或硬扣费保障。服务端created_at/completed_at换算北京时间为2026-11-02 12:28:21/12:28:22；不冒充本地HTTP发起时间。

provider_request与rpc_reserved的原始body_hash均为`4b83f2e3fb7ce59ada32894e3dc64813bf1bba8926dd3091581ca36446257ef1`。由未变生产am_body字段插入顺序、固定系统指令/两工具定义及持久化初始上下文重建1201字节请求，与两处摘要相同；history=[]、无目标/能力/字段，只有允许的T1合成输入。real/t1-expected-body.json是重建依据，不是原始网络抓包。取证初稿错误采用键排序导致摘要不匹配，已依据am_mutable_write的flags=0修正重建序列并保留body-reconstruction-notes.json；不是产品失败或重发，既有能力JCS合同不变。

实际保存了完整Responses对象及成功终态；HTTP200/text/event-stream来自未变生产成功门的推论，没有独立保存原始响应头。可确认生产HTTPS/SSE处理路径完成，不外推代理/TLS故障矩阵、逐片视觉体验或全部真实设备验收。

closed-db-observation.json保留原始合成消息/事件、22项对账、usage及证据边界；final-preservation.json核对未变源码/程序、原失败和固定依赖。运行期文档变化及real/delivery-identity.json与启动前离线identity分开记录，不回写离线清单来冒充其覆盖后续文档。

取证字面模式扫描在SQLite二进制中命中1处类似Key形状，初稿有界断言停止。逐项比较确认它仅是已知合成client_request_id的sk-101-1后缀与相邻非敏感任务hash连续存储造成的误匹配；没有读取或输出实际Key。保留redaction-initial-pattern-scan.json及redaction-pattern-notes.json，不删除初次失败来宣称从无命中。最终脱敏检查只证明相应文字/存储字段范围，不是任意进程内存的秘密证明。

## 8. 结论、保全及停止位置

RPC-K1限定整改开发及离线自动验收通过；用户实际原生录入、保存/同ref读取、复制诊断和T1限定生产文本验证有证据。该自审结果不代替新代码的独立复审或正式冻结。RP-C整轮限定真实验证未完成，不能报告通过：T2发现/read10/r1和T3在途取消因validation_deadline停止未验，没有额度或费用耗尽证据，不归因Key或环境。

未追加请求、延长期限、重开目录/会话或换client ID续跑；本轮M1/R1/S1/C1/U0与旧轮M1/R1/S0/C0/U0分别保留。匹配程序、同期失败源码/JUnit/日志、隔离库/配置和本轮凭据引用均保留，助手未提取Key明文。只清理新开发根559个无用obj，真实证据阶段无删除。其余历史期限/frontend/输入/browser首败及延期人工边界保持。

后续只可另行评审剩余T2/T3的最小验证安排；本记录不授权重新启动、重置原额度或自动追加真实调用。停止本轮整改与限定真实验证结论位置。
