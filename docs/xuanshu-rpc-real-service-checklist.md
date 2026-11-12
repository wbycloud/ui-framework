> 2026-11-02当前：RPCB-1预览展示与启动诊断限定独立验收通过，匹配快照冻结已[追记落盘](validation/xuanshu-rpcb-display-independent-review.md)。最终UI为bin/final-projection.xml，最终启动检查为production-startup-final.xml；本轮没有重跑测试、启动程序或追加真实请求。[阶段收尾](xuanshu-native-stage-closeout.md)集中索引延期/另案及Proposed下一批。下文旧“待复审/本轮停止”分别保留当时阶段，不代表再次授权执行历史启动命令。

> 2026-11-02本轮当前入口为[RPCB-1批次验证](xuanshu-rpc-batch-contract.md)，已由用户明确授权验证专用合同增量。新的xuanshu_rpc_batch替代本轮逐任务操作：指定引用可读时，只需阅读完整固定范围后点击一次“开始受控补测”；不可读时才通过原生遮罩API Key窗口保存并等后台确认，然后点击开始。不要求复制JSON、逐任务确认或手动关窗。T1=0、T2≤3、T3≤1、合计≤4，失败即停止，无退款/挪用/重试。下文旧模式及历史记录保留，不用于本轮操作。

# RP-C首次真实服务验证操作清单

> 2026-11-02补测已结束：仅T2一次POST，page1与适配器仅0接受域不一致，任务failed已保存、工具0；T3按停止规则未执行，T1未重复。用户正常关窗、PID75568退出、关后库只读对账完成。M1/R1/S1/C1/U0，详[最终记录](validation/xuanshu-rpc-real-service-validation-t2t3.md)。本清单不再授权重启/换ID/另建目录重试；后续工具声明整改及再验证须另授权。

> 2026-11-02当前授权仅[新T2/T3补测](validation/xuanshu-rpc-real-service-validation-t2t3.md)，沿RPC-K1原程序：不得点击T1，先原生Key保存/同ref读取软件确认，再直接T2≤3及费用允许后的T3≤1，总≤4。新唯一session位于build/xuanshu-rpc-t2t3-20261102-53b8a6，启动不自动提交；不使用下方旧目录或恢复旧额度。身份及[RPC-K1冻结记录](validation/xuanshu-rpc-credential-independent-review.md)已核对，未改代码/构建。真实操作结果后续追加，不预写通过。

> 2026-11-02第二轮已结束：原生Key实际保存/同ref读回及“复制诊断”可用，T1真实文本与唯一保存终态成功，M1/R1/S1/C1/U0。T2/T3因validation_deadline未执行；用户正常关窗，进程退出及关后库已核对，不延长期限或另建目录续跑。详[第二轮最终记录](validation/xuanshu-rpc-real-service-validation-round2.md)。本清单保留操作依据，不授权再次启动、自动填Key或追加请求；整轮真实限定验收未完成。

> 2026-11-02凭据整改后新轮：先完成[RPC-K1离线验收](validation/xuanshu-rpc-credential-validation.md)，再启动新根build/xuanshu-rpc-credentials-20261102-e48d73/light的匹配程序，创建独立real/session；不要使用下文旧候选模板。本轮按钮为“设置DeepSeek API Key”，原生窗唯一遮罩API Key框及保存/取消；等后台实际“保存成功，本地可读取；服务端未验证”，再逐任务“确认外发”。“复制诊断”只读，可可靠提供阶段/错误/计数，不要求抄刷新JSON。准确新步骤/状态见[第二轮记录](validation/xuanshu-rpc-real-service-validation-round2.md)；旧失败和额度不退、不覆盖。

> 2026-11-02本轮收尾：T1已接纳/确认但在发送前credential_unavailable失败；截图及关后库均为M1/R1/S0/C0/U0，Policy stopped。T2/T3未执行，不重试或另开目录退还额度；用户正常关窗、进程已退出、唯一失败终态已持久化。真实服务限定验证未通过，后续须先另授权排查凭据链路。详[实际记录](validation/xuanshu-rpc-real-service-validation.md)。下文未执行为原清单历史，不可据此再次启动。

> 2026-11-02执行状态：上一轮准备限定复审通过，当前匹配第6节新根；本轮用户已明确授权必要文档和程序启动，以及其原生Key录入/逐任务确认后的最多5次请求。新隔离路径为`build/xuanshu-rpc-real-20261102-b5e731/session`，不得复用或另开目录重置额度。实际状态以[真实验证记录](validation/xuanshu-rpc-real-service-validation.md)为准。以下“未授权/尚未运行”说明保留原轮历史；没有因此授权其他功能或追加请求。

> 限定整改候选已生成，待代码评审；新产物以第6节为准。第1—5节保留原准备交付的范围及操作说明，原产物模板不可据此直接执行。本轮与原轮均未获真实调用/Key录入/人工验收许可。

2026-10-31。仅为下一轮授权准备，不是执行许可。当前只完成隔离准备和离线验证；没有读取真实Key、启动验证窗口、调用模型/模型列表/余额API或完成真实人工验收。

## 1. 匹配产物与启动前条件

本轮产物根`build/xuanshu-rpc-prep-20261031-c913e8`，以其中`delivery-identity.json`、`source-after.json`、`binary-files.json`及`exports.json`为准。建议将来选用Light目录的`xuanshu_rpc_probe.exe`、同目录`ui_framework.dll`和`rpc_memory.uapp`；这是生产DLL及公开只读应用接口，不是ui_framework_edit_test.dll。原有Light正式宿主未替换，不把UI_AI_DB_PATH当作模型配置隔离手段。

只有准备交付经代码评审且用户另行授权实际调用后，才可使用以下PowerShell命令模板。`<新的不存在的绝对目录>`必须替换为本轮获准的新位置；当前不执行：

```powershell
& 'D:\应用软件框架\应用层序框架\build\xuanshu-rpc-prep-20261031-c913e8\light\xuanshu_rpc_probe.exe' '<新的不存在的绝对目录>' 'D:\应用软件框架\应用层序框架\build\xuanshu-rpc-prep-20261031-c913e8\light\rpc_memory.uapp'
```

启动器拒绝已存在的目录。库为该目录的conversation.sqlite，模型配置为conversation.sqlite.model.json；凭据引用由后者路径派生，界面展示脱敏ref。新进程只有一个workspace/Service/主对话，只加载memory.read夹具，不自动提交任务或发送。不得改LOCALAPPDATA，不使用默认用户库、默认模型配置或其他应用凭据。

## 2. 另行授权的准确范围

- DeepSeek官方`POST https://api.deepseek.com/responses`、deepseek-flash、非思考、stream、max_output_tokens=4096；不探测其他端点/型号，不自动切换服务商。
- 用户在Native Key原生界面录入本轮独有凭据，随后亲自确认各任务准确外发预览；不得将Key发到聊天、页面、日志或数据库。当前没有该操作。
- 仅[原计划](xuanshu-rpc-validation-plan.md)三项合成输入、固定系统指令/两个封闭read包装工具、history=[]；T2唯一memory.read描述/Schema及实际10/r1。不发业务文件、其他实例或完整主对话。
- 全轮最多5次POST，T1/T2/T3分别1/3/1，续轮计入，失败/取消/发送未知不返还，不挪用或自动重试。USD0.10本地停止策略，接受它不是服务端硬扣费保证；执行前核对官方当天费率，若需更保守费率必须匹配构建/策略及重新评审，不能口头忽略不足。
- 原10/30/30/120秒网络期限，首次可信确认起整轮10分钟后停新派发并请求取消；安全收尾仍等实际退出。无重复试跑、坏Key探测、故障收费请求或新工具。
- 若授权包括关闭后同库离线重开核对，重开仅恢复记录和已消费计数，封闭发送，不续用授权、不重放工具。启动器自身拒绝复用旧目录，不能通过另开窗口重置已消费任务；重开核对需另行指定已有公开Service读取方式，不运行旧recovery工具。

## 3. 操作与预期

| 步骤 | 操作（全部留待授权后） | 预期与记录 |
|---|---|---|
| 启动 | 核对三产物哈希，使用新目录 | config/ref与库同目录；ready后M/R/S/C/U均0；只展示界面，无请求 |
| 配置 | 用户点击Native Key，经生产可信原生输入链录入 | Key不经页面；脱敏origin/ref和配置保存确认；0 POST。模拟消息不能证明该正例 |
| T1 | 点击T1 text，阅读完整预览后亲自Confirm scope | 输入为要求“玄枢流式验证成功”，history=[]；收到流式文字、完整Responses终态及本地最终保存确认；R=1、S≤1，无工具 |
| T2 | T1正常且费用可核对时点击T2 read，再确认准确范围 | memory.document当前epoch/r1；模型实际发现并读取memory.read，配对输出含描述/Schema及10/r1；最多3 POST，不能以模型只回复文字替代工具覆盖 |
| T3 | 前两项正常且仍有额度/费用时点击T3 cancel trial，确认 | 在实际delta后点击Cancel，只占1；若已结束则记取消窗口未覆盖，不另收费重试；取消不等于远端停止或免单 |
| 停止/退出 | Stop round或关窗，继续正常消息循环 | 禁止新发送，等待连接/worker/回调及存储真实退出；不强杀进程或提前卸载；若停滞保留状态和最小日志 |

UI proposal status=OK仅表示received；待本地保存和外发确认后才可能发送。流式delta、完整远端响应、本地业务read、最终保存分别核对，不将流结束当成持久化成功。任务标签/窗口变化不得改变既有固定目标，夹具无人工修改路径。

## 4. 对账、费用及停止条件

逐项记录session/run/request/ordinal、body hash及字节数、manifest M、消费R、真实发送API进入S、完整响应C、发送未知U；工具续轮逐次登记。正常应C≤S≤R≤M且R≤5。S在WinHttpSendRequest返回后登记，调用在途时send_count_known=false；ack保存失败send_count_saved=false。进程中断不能宣称精确S，重开保持计数不确定且封闭。model_calls不是精确HTTP次数，也不等于远端账单笔数。

known_nano_usd为根据有效usage和本地固定费率计算的已知部分；estimated_nano_usd为未核定的保留估算，fee_unknown独立标识。usage缺失的单次费用记录为null，不记0。无有效usage/费率依据、远端状态未知或余额/账单不可核对时停止后继发送。字节代理不能证明准确token或硬费用上限；取消仍可能收费。账单只由用户将来通过官方界面核对，不新增余额API。

任一越界外发、重复请求、非法/截断流、401/429/5xx、TLS/代理错误、过期、配额/费用不满足、目标/版本变化、存储失败或checkpoint异常、取消/关闭异常均停止后续任务，继续cleanup-only。不重新确认后自动重试，不取消/forget清除UNKNOWN，不伪造失败请求“未发送”。

## 5. 最小证据和延期范围

保留本轮获准合成body（不含Authorization）、ID/摘要/时间、计数及保存确认、必要远端response ID/usage、脱敏错误、原始最小日志及本地隔离库、产物哈希；不得采集完整进程内存/真实Key/其他应用内容。关闭异常先保留诊断，不额外请求“再试一次”。

离线传输和私有确认只证明测试状态机/配额及相应模拟路径；生产夹具公开API的10/r1回读不等于真实模型工具兼容已验证。真实DeepSeek、TLS/代理、系统凭据窗口、真实鼠标/键盘和Windows来源正例仍未验。IME/DPI/视觉/辅助技术/物理断电/人工长时保持原延期安排，不因本清单删除。真实执行结束后也只能按实际覆盖给出RP-C结论，不能外推软件完整人工验收。

## 6. 限定整改后的候选程序（尚未运行）

新根build/xuanshu-rpc-prep-closeout-20261031-f27a64，匹配源码/程序/测试见delivery-identity、source-after、binary-files及verification-summary。产品改动只有旧入口client ID前置校验，夹具值/Schema及真实请求范围、费用/期限不变；程序重新链接匹配新生产DLL，不用旧DLL替换。先限定独立复审，再另行取得真实执行授权。

仅供未来授权后的命令模板，当前不执行：

```powershell
& 'D:\应用软件框架\应用层序框架\build\xuanshu-rpc-prep-closeout-20261031-f27a64\light\xuanshu_rpc_probe.exe' '<新的不存在的绝对目录>' 'D:\应用软件框架\应用层序框架\build\xuanshu-rpc-prep-closeout-20261031-f27a64\light\rpc_memory.uapp'
```

仍须用户通过原生界面录入隔离Key并逐任务确认；不发到聊天中。本轮没有启动该程序，不能将测试子进程的离线通过或修前异常冒充真实DeepSeek/真实人工结果。原最早tools编译源未自动归档的证据限制不由本轮新归档消除。

### RPCB-1本轮实际停止

匹配Light启动器仅启动一次，PID5916早退；尚未生成会话、配置或自动报告，未要求用户开始批次，真实POST为0，T2/T3未执行。没有库内M/R/S/C/U，不伪造计数/费用/关闭确认。按约停止，不再启动或请求；根因未定位及最小后续启动取证建议见[本轮最终记录](validation/xuanshu-rpc-batch-validation.md)。离线通过不替代这项生产启动/真实服务未验。

### 当前预览门整改候选（本轮禁止真实补测）

见[限定整改记录](validation/xuanshu-rpcb-display-closeout-validation.md)。候选程序位于build/xuanshu-rpcb-display-20261102-64c8a2/bin，仅通过本轮离线检查，待独立复审。当前hash的完整预览没有成功显示时，不得开始；失败应显示禁止状态。诊断按error_domain区分Win32与框架状态，code_known=false表示未知。不能据旧绑定、旧文字或配置ready认为授权完成。本轮不提供真实启动操作、不录Key、不追加请求；既有真实结果和费用unknown保持。

### 当前冻结与操作停止

匹配根build/xuanshu-rpcb-display-20261102-64c8a2，identity41D15CFF8A2E0F639D02B6748C0F90AB95E9D6B4CAB7368221CF357BA4E8FF7C。冻结记录不启动程序或取得新外发授权；无需为本轮通过结论再执行T1/T2/T3。旧T1、T2及T3覆盖/unknown和483 Tokens账目边界按阶段收尾保留。将来普通正式宿主任务继续原逐任务确认，不能把验证批次接口或固定ref作为通用授权。新交互建议和集中人工均须另获授权。当前检查清单是更新后的文档状态，原离线版本仍在冻结根source-final，未改历史清单或证据。
