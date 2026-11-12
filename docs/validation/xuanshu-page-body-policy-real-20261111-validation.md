# PB-2一次真实网页DOM正文只读验证

2026-11-11，Asia/Shanghai。PB2-REAL-20261111-8f3c2a已按具体批准执行一次并正常关闭。147字节正文、框架revision、模型摘要、工具结果和最终保存逐值对应，本次限定验证通过。该包已消费，当前没有运行中的本轮宿主、待录Key或待确认操作。

## 冻结快照与本轮文档状态

承接上一轮限定独立只读复审通过及[本轮冻结归档](xuanshu-page-body-policy-independent-review.md)。匹配根为build/xuanshu-page-body-policy-20261110-d42b8e，delivery-identity.json SHA256为46813CE1DAB1C2DD5620D58A5FFC90306076AFCF26AEDED2BEC620F5BA1D7496。启动前及退出后537份源码、241项程序及链接/调试产物均与清单匹配，Git HEAD保持e0eca03db25930be55bdb45477027d1793a61904。入口范围仅对应选定文件，既有Git修改和未跟踪文件继续保留。

| 生产文件 | SHA256 |
| --- | --- |
| light/framework_host.exe | D9870808CB9A22C86BD1D424448FAFF1BFDFFE8D09559C436F4BF8111211EDE7 |
| light/ui_framework.dll | 124E7BB6F8B3B0B470DC242CA1B6E5A7F76BA370FBA0915BDA9227F5F397154A |
| light/body-production.uapp | 5CB06AEE16E08611E621E1A24871349EEE2DCD30C8482A32EDB0F3FF327F2F48 |
| light/xuanshu_br2_meter.exe | 7E2C2FF2508A83F05E1DB5B891635DABFA843812B4078BC0D7D4FEC3CB6C0EBB |

本轮完成身份核对、真实执行、证据观察和文档整理。产品源码、测试、构建配置、依赖、版本、公开布局、DDL、来源政策和预算保持，未重新构建或运行离线测试。原交付identity、源码副本、程序及旧证据不改写，后续文档状态独立记录。原开发和五次离线运行仍按[冻结记录](xuanshu-page-body-policy-independent-review.md)各自范围引用。

## 具体批准与唯一启动

[具体授权包](../../build/xuanshu-page-body-real-preparation-20261111-8f3c2a/authorization-package.md)的SHA256为2F81761A899F853D31D14AD109BC1DF4812944EF15DBB074697457B2EC9C1096。用户明确批准“明确批准本日包，仅执行这一次”。批准记录approved.json与监督命令绑定，命令SHA256为087767F748BDF27FD848B4CD059D5B181958729D9E2ECD294F883812F1DC06DE。初始准备清单和preparation-identity保留当时Proposed事实，后续批准和执行分别见approved.json及本记录。

唯一新执行根为build/xuanshu-page-body-real-run-20261111-8f3c2a。显式数据库为session/conversation.sqlite，模型配置按DB路径追加.model.json，来源配置和BR-2策略均限该新会话。未改变LOCALAPPDATA、迁移用户数据或复用旧库、旧额度及旧凭据引用。

唯一生产PID为28300。stdout.txt和stderr.txt均为空，监督result.json记录实际exit_code=0、exit_observed=true、environment_restored=true、error_count=0。结束使用正式窗口WM_CLOSE，监督等待实际退出后才观察数据库。没有循环重启、强杀或提前卸载。原监督8项及辅助2项按相同机制身份复用，本轮新增命令的实际正常执行另有本次证据。

## 人员操作与普通界面操作

助手通过外部脚本操作生产普通控件，配置deepseek-flash、选择范围、输入固定任务、保存一次本地提案、查看完整请求详情、复制脱敏诊断并正常关闭。输入和点击记录在actions.jsonl，唯一提案使用CreateNew意图记录约束。没有测试DLL、私有可信入口、页面内容改写或直接业务数据库写入。

用户原生录入Key后反馈“已保存”，关闭原生密钥窗口后的只读软件状态为本地可读取、配置代次2。原生交接期间截图、剪贴板和输入暂停，助手没有读取密码框、Key、环境Key、Authorization或凭据Blob。用户核对完整范围后亲自确认一次并反馈“已确认”；助手没有点击外发确认。软件接纳及确认保存由实际状态和关后记录分别核对。

启动前和确认前各重新核对12份官方公开材料，均HTTP200并与当日准备字节一致。deepseek-flash对应Flash-1017，端点https://api.deepseek.com/responses，协议deepseek-responses-v1，reasoning.effort=none、输出上限4096。公开资料获取使用无认证GET，原始响应和时间分别保留于准备目录及执行目录。依据见[价格](https://api-docs.deepseek.com/quick_start/pricing/)、[Responses接口](https://api-docs.deepseek.com/api/create-response/)和[Responses指南](https://api-docs.deepseek.com/guides/responses_api/)。

## 固定任务、范围与预览

固定任务输入如下。

> PB-1：读取当前合成交接单DOM正文，概括设备总数、已完成、待处理及负责人；不导航或修改。

| 项目 | 实际记录 |
| --- | --- |
| run_id | 360f30e612e4e93903911ced6bfe5ffd16b15339a24eb6a719f1a053a362a9f5 |
| binding_hash | 0404f3a962c78b3f16ed8bc0bc198cfd7fdf0afd107912666a3c351f5c18b97e |
| history | [] |
| 模型与配置代次 | deepseek-flash，2 |
| 场景 | dom-body-v1 |
| 能力与字段 | browser.body，body和revision |
| 实例与资源 | instance_id=1，browser.view |
| host_epoch | 5092e44b-bda2-487d-a204-e91f12524ad8 |
| open_epoch | 5652086e-092a-45b0-87a2-635e00ca170c |
| revision | 77fbdb9e18262e8aa0c807ae275fbfdab249f027c46f7068e4831509755f70af |
| scope | {"access":"read"} |

实际控件中的两块完整详情分别为3000和1094 UTF-8字节，拼接后JSON成功解析，run、binding、目标及请求各值匹配。规范化初始request为1560字节，SHA256为FB8F7A8A7703B6280C194A1D451C449997DEA9F1C3C14FC4EB3486CBA173DD69。准备计量545 token适用于该规范化请求，实际每轮正文使用各自body_hash和计量回执，首轮实际计量544 token与服务端usage一致。完整详情回读见full-preview-readback.json、detail-0.txt和detail-1.txt。

## 实际业务结果及模型摘要

第一轮模型调用xs_read_discover，category为空；第二轮调用xs_read_call，capability_id为browser.body、parameters_json为{}；第三轮返回最终摘要。持久化工具记录共2条，分别为read-directory及browser.body，状态均为0。原始目录记录包含实际应用的body和snapshot两个描述，模型上下文按固定白名单筛选。src/ai_model.inc第214至252行先保存原始工具事实，再按projection筛选目录与字段并加入模型续轮。原始目录行不代表额外业务能力获准或被执行。

browser.body结果为complete=true、bytes=147、semantics=dom-body-v1，revision与固定目标全文一致。实际正文如下。

```text
设备巡检交接单
设备总数：12台。
已完成：9台；待处理：3台。
负责人：林青 & 周明。
下一步
复核三台设备。
```

最终模型回复准确概括12台、已完成9台、待处理3台、林青与周明及复核三台设备，并说明正文来源。模型消息、终态reply及实际结果控件回读对应。正式页面在reply后追加outcome和error展示，该格式已按host.html第339行逐字核对。正文是规范化作者DOM文本，CSS隐藏作者文字可能保留，脚本、样式及表单值按既有正文合同处理；本例未导航、修改或读取其他应用及用户业务文件。

## 请求、计量、费用与停止

本次仅一个固定任务，初始及全部续轮共3个ordinal。每轮provider_request、rpc_reserved、rpc_send_intent、rpc_send_entered、rpc_network_end和rpc_cost按request_id关联；实际正文哈希与计量回执一致。每轮额度先占用，3次传输入口和3个完整响应均非模拟，unknown为0。task model_calls=3与tool_calls=2分别表示模型轮次和工具事实，抓包级HTTP次数未观测。

| ordinal | 实际计量及usage输入token | usage缓存token | usage输出token | known nano USD |
| --- | --- | --- | --- | --- |
| 1 | 544 | 256 | 85 | 265200 |
| 2 | 887 | 512 | 78 | 359700 |
| 3 | 1083 | 896 | 119 | 467700 |
| 合计 | 2514 | 1664 | 282 | 1092600 |

按绑定峰时非缓存输入300 nano USD/token、输出1200 nano USD/token计算，本地保守known为USD0.0010926，未结预留0、fee_unknown=false。缓存token仍按非缓存单价保守计入，金额适用于本地绑定价核算，账户账单未查询。每轮预留USD0.0109152，三轮保守预留USD0.0327456，实际每轮输入均低于20000硬上界。授权保护为最多3次、本人可信确认起10分钟、USD0.10停止阈值。任务完成后保护stopped=true、原因business_task_finished，计量组件exit0、系统错误0。

本次没有第4次尝试、POST重试、额外提案、换库、重启补额度或继续执行。期限、费用异常和第4次拒绝的故障分支承接匹配离线验证，本次真实执行只提供正常完成路径。没有全程网络抓包或账户扣费观测，运行时请求、传输入口、完整响应及本地计价各按其证据性质记录。

## 关后持久化核对

监督确认实际退出后，先检查本次新库的-wal、-shm及-journal均不存在，再以mode=ro&immutable=1和query_only读取。没有checkpoint、Service重开、恢复执行或数据库写入。关后1任务、2消息、31事件、2工具，唯一finished/completed终态，确认、模型请求、正文工具结果及最终消息均已保存。任务输入、history、固定目标、白名单及场景与接纳和预览记录一致。

数据库观察前后SHA256均为E7A6F2EFF4FBB14717275C274B04712E4006C0BE03E98BC5EEF4D9B1FF2313DB。无旁文件及哈希保持结论仅适用于本次明确观测的新库。主证据为[closed-db-observation.json](../../build/xuanshu-page-body-real-run-20261111-8f3c2a/closed-db-observation.json)、[execution-assessment.json](../../build/xuanshu-page-body-real-run-20261111-8f3c2a/execution-assessment.json)和[result.json](../../build/xuanshu-page-body-real-run-20261111-8f3c2a/result.json)。

## 外部驱动首败、清理与边界

准备脚本曾遗漏新命令哈希替换，并将CRLF与LF误判为不一致，两次在启动前停止并修正。唯一生产进程运行期间，外部驱动还遇到screens目录缺失、前台与Light子窗口焦点未就绪、复制未更新、窗口最小化导致坐标拒绝及OCR后续文件缺失。这些失败均保留，修正普通焦点/取证步骤后继续同一PID，没有通过重新启动或重复提案追求通过。

结果回读首次比较漏掉页面追加的终态展示，原实际文本保留于result-control-mismatch.txt；随后依据实际源码核对全文一致。此项属于外部预期格式错误。driver-errors.md、首次焦点辅助脚本和失败截图分别保留，不将驱动问题归为产品缺陷。未由工具自动落盘的错误只作为明确标注的转录，不冒充原始日志。

清理仅限本次新目录的可再生成OCR放大图，保留对应原始截图、OCR文字、动作日志、失败材料、输入/详情/结果读回、诊断、官方依据及关后证据。原快照、全部历史失败、冻结SDK、旧库和旁文件保全，详见本次cleanup.json及证据清单。

本次人员交接、软件状态、截图/控件全文、持久化工具与模型记录分别提供证据。合成普通输入不证明真实设备来源完整分类。用户原生Key和逐任务确认只适用于本次包，不产生新的通用批准。任意计量辅助进程卡死的有界退出未证明、PB-1初始化20.03秒首超时、BR1-KI-001及其他历史风险继续保留。IME、DPI、辅助技术、长时、断电及browser九项副作用维持原范围，已取消的临时页面控件测试维持取消。

## 收口结论

PB-2一次真实网页正文只读验证通过，匹配交付及本地材料收口完成。本次必要修正项为空，具体批准已消费，原默认模板仍保持独立的Proposed锁停状态。后续新的真实调用需要新的具体范围与独立授权。本结论适用于本次程序身份、合成正文和唯一任务，系统整体、任意网页或其他业务范围沿用各自验收状态。
