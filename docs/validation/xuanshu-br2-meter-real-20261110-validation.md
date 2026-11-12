# BR-2一次真实只读验证与关闭记录（2026-11-10）

## 本次结论

**本次真实业务只读验证暂不通过，尚不满足真实业务交付收口条件。** 唯一任务在运行时正常完成并保存，但模型只查询了分类为`browser`的目录，得到空结果，没有调用`browser.snapshot`，未取得交接单标题、来源及业务读取返回的revision。不能用`completed`、两轮响应、工具调用数或已有离线通过替代业务成功。

本次已完成集中批准、用户原生凭据录入、唯一提案、用户逐任务可信确认、真实响应、费用记录、正常关闭和关后只读核对。没有追加请求、新建补测任务、换库或重启，也没有修改产品、构建或运行测试。原匹配快照的限定冻结和离线结论保持原适用范围；本次失败不能被写成真实业务通过，也不是重新独立复审。

## 身份、授权和隔离

- 集中包：`BR2-REAL-20261110-4da059`。用户明确回复“明确批准本日包，仅执行这一次”。原包和公开依据留在[当天准备目录](../../build/xuanshu-br2-meter-real-preparation-20261110-4da059/authorization-package.md)，不改写其封装字节。
- 冻结运行根：`build/xuanshu-br2-meter-20261106-1f8148`。
- delivery-identity SHA256：`C5A430D7DA479E7706043741FE11D66EE95B976C51B849ADA92A26732BF62596`。
- 唯一执行目录：[build/xuanshu-br2-meter-real-run-20261110-4da059](../../build/xuanshu-br2-meter-real-run-20261110-4da059/)。DB为其`session/conversation.sqlite`；模型配置按DB路径追加`.model.json`。来源配置`session/origins.txt`为空，策略为`session/business-policy.json`，未改变LOCALAPPDATA或迁移旧库。
- 独立批准命令：`build/xuanshu-br2-meter-approved-command-20261110-4da059.ps1`，SHA256为`F96191D1BE827337759345D4A7A0E160670523D590FC75E5C85E7DEB74BFB1F3`，仅执行一次。新命令此前只有静态审阅；本次只证明这一次实际启动、监督和正常退出路径，不将其扩大为全部异常路径动态验证。
- 活动策略SHA256：`21E81009564FD6816EE3D9960CFD8CA41A6A7ABB1FC881431ADCB91CD73EC9EE`。包含全部续轮最多3次、可信确认起10分钟、USD0.10停止阈值、16384输入上界、4096输出上界及逐轮完整计量。

| 生产文件（相对冻结运行根） | SHA256 |
| --- | --- |
| light/framework_host.exe | B5E7FA2FF300297433E8379E83ECB80F34D4D41E9B4CDE5B7490A78D899B148F |
| light/ui_framework.dll | 7FEFFBD5F03EC8116918950C9C7FFDBBECDA3B423C19592B171F7459688B7375 |
| light/br1-production.uapp | 6C23ED0F1AAF3FBC8AA191804C57BABFF131B07F2D7AEEE15933524E0F08BD3B |
| light/xuanshu_br2_meter.exe | 7E2C2FF2508A83F05E1DB5B891635DABFA843812B4078BC0D7D4FEC3CB6C0EBB |

发送前当日日期、四个生产文件、12份官方公开资料及活动策略全部匹配，见[confirm-preflight.json](../../build/xuanshu-br2-meter-real-run-20261110-4da059/confirm-preflight.json)。关后再次核对四个程序、冻结identity及513项对应产品/测试/工具源码，均匹配，见[postflight-identity.json](../../build/xuanshu-br2-meter-real-run-20261110-4da059/postflight-identity.json)。后续文档状态不冒充原交付源码和文档副本。

## 实际界面操作及证据性质

用户通过实际DH-1界面展开AI和设置，保存`deepseek-flash`，通过原生入口录入本次新引用的Key并观察“本地可读取、代次2”。助手未读取密码框、Key值、Authorization、凭据Blob或完整内存；生产程序在批准范围内使用该凭据。本地可读取不能单独证明服务端认证。

历史保持不选，选择`browser.view`、`browser.snapshot`及`revision/state`，填写固定输入：

> BR-1：读取本轮合成交接单标题、来源与版本，不导航或修改。

用户点击“保存任务并查看范围”一次，查看持久化接纳、空历史、固定目标和共2块完整请求详情。一次生产脱敏诊断验证模型、profile2、固定范围、保护就绪及发送前零计数。`display_valid=true`只是投影状态；不是单独的确认资格。用户随后仅点击一次“确认此任务外发范围（一次）”，关后存在唯一`disclosure`记录，其binding、请求hash及profile均匹配；没有使用自动输入、测试DLL或私有可信入口。

用户转录的任务状态为`completed`、持久化接纳/确认保存/请求保存/终态保存均为是、完整响应已收到、工具调用1、模型轮次2、错误无。用户转录结果称目录为空、未读取到数据。数据库中完整回答进一步明确“BR-1 未能完成”，与上述观察相符。用户最后正常关闭主窗口；没有在终态后再次取消或发送。

截图仅证明截图中可见区域，不代替全部实际文本或来源分类；运行时状态和持久化事实另行核对。关键材料在执行目录：`interaction-record.md`、`preconfirm-diagnostic.json`及其核对、`user-confirmation-observation.json`、`user-terminal-state-observation.json`、`user-result-observation.json`，截图在`evidence/ui-02`至`ui-06`。确认反馈的接收时间不是运行时确认起算时间；记录中的单调时钟`until`也不伪造为UTC确认时间。

## 固定任务与真实目录结果

任务run：`f07c794e22ca5bfaadfcece1e013c7f81cb6eca84e38a55025d9d8f450448089`。

| 绑定项 | 实际持久化值 |
| --- | --- |
| client | panel-1a1c35411b8-3 |
| binding_hash | 1a739e0c401fb7e3a18b9e73d0aab152c306264c30d83ea019f81b41a226f33c |
| 模型 / profile | deepseek-flash / 2 |
| history | [] |
| instance / kind | 1 / 2 |
| resource | browser.view |
| host_epoch | 58fb169d-9f0e-46d4-948b-1f7efb61811c |
| open_epoch | 09aa9e3a-eea8-41f5-b058-7b6ad5c389bc |
| 固定框架revision | 77fbdb9e18262e8aa0c807ae275fbfdab249f027c46f7068e4831509755f70af |
| scope | {"access":"read"} |
| 能力 / 字段 | browser.snapshot / revision、state |

固定输入、history、模型、目标、epoch、revision、scope及白名单均与发送前诊断逐值一致。固定revision来自绑定，不冒充一次成功业务工具返回的revision。允许的`state`仍是完整顶层对象，不是只发送title；本次没有取得该对象，也未汇总正文12/9/3。

首轮真实响应要求：

```json
{"name":"xs_read_discover","arguments":{"target_alias":"target-0","category":"browser","page":0}}
```

工具表唯一记录为`capability=read-directory`、`status=0`、`result=[]`，向模型返回`{"items":[],"next":0}`。第二轮只返回无法读取的解释，没有`xs_read_call`，没有`browser.snapshot`执行或业务工具结果。

本次最早可明确归属的失败路径是：模型传入`browser`分类 → [am_step原样转入discover](../../src/ai_model.inc#L183) → [Service传给目录](../../src/ai_service.c#L347) → [目录按分类精确比较](../../src/capability.c#L333)，而[browser.snapshot注册分类是browser.read](../../apps/browser/src/browser_capabilities.inc#L70)。[包装工具的category仅声明为字符串](../../src/ai_model.inc#L143)，没有列出本目标的合法分类。本次真实响应与源码共同证明分类不匹配导致此查询过滤为空；不能由此声称应用没有注册能力、文档丢失、目标错误或browser.snapshot执行器失败。

后续最小建议为单独授权修正只读发现包装的分类可发现性/参数契约，并以这次真实响应建立离线回归，保持原白名单和权限。修复、重新构建、独立复审及任何下一次真实请求均未由本次执行授权自动批准；本轮没有实施。

## 请求、计量及费用

两轮均由同一任务预算管理；计量body hash、manifest、占用、意图、传输入口、完整响应及结算逐项关联且事件顺序正确。两份重建body与发送前持久化body hash逐字节匹配，详[request-reconstruction.json](../../build/xuanshu-br2-meter-real-run-20261110-4da059/request-reconstruction.json)。这是固定序列化器、接纳上下文和持久化模型/工具结果的重建，不是网络抓包。第一轮输入只含允许投影和固定任务；续轮只追加该函数调用及空目录结果，没有增加业务state、其他历史、目标或数据。

| 轮次 | 输入计量 = usage input | usage output | cached input（包含于input） | 本地known nano USD |
| --- | ---: | ---: | ---: | ---: |
| 1 | 496 | 89 | 0 | 255600 |
| 2 | 592 | 162 | 384 | 372000 |
| 合计 | 1088 | 251 | 384 | 627600 |

本地按绑定峰时非缓存输入300、输出1200 nano USD/token计算，总计**USD0.0006276**。没有依赖缓存折扣，未核对账户账单或承诺实际扣费。两次原预留各9830400 nano USD，经有效usage结算后尚未结清estimated为0；额度占用2次不返还。没有费用unknown或远端unknown记录。

记录为2份manifest、2次占用、2个发送意图、2个非模拟传输入口、2个完整响应、1次目录工具。无第三/第四轮、重复提案、重放或重启；这些软件计数不是独立网络遥测或精确HTTP次数。3次上限、10分钟及费用停止的异常边界仍由既有离线证据支持，本次没有动态触发其边界，不能宣称本次又验证了所有保护拒绝路径。

## 关闭及只读核对

唯一宿主PID **23608**，实际exit **0**。监督记录错误0、证据写入成功、三项临时环境恢复，`stdout.txt`和`stderr.txt`均0字节。见[result.json](../../build/xuanshu-br2-meter-real-run-20261110-4da059/result.json)。没有自动重启或强杀。

用户反馈关闭后，先确认进程实际退出，再检查本次DB无WAL/SHM/journal，使用`mode=ro&immutable=1`和`query_only=ON`。关后读取前后无旁文件、数据库SHA256一致：

`59F45907BAB6AB427D00ABC7B9006B343A6DF5F12C1A5612F0C127B5F5008F57`

关后为**1任务、2消息、21事件、1工具**，唯一`finished`/`completed`终态；终态快照committed、disclosure_saved及model_request_saved为真，错误为空。没有重开Service、checkpoint、恢复旧授权、修改金额/预算或操作历史数据库。完整结构化观察见[closed-db-observation.json](../../build/xuanshu-br2-meter-real-run-20261110-4da059/closed-db-observation.json)，只读观察脚本为本次执行目录的`observe-closed.py`；它不是产品测试或再次独立复审。

## 保全、限制与停止位置

原冻结identity、程序、脚本、准备包、历史失败、数据库及旁文件保留。新增材料仅本次唯一会话、取证及必要文档；根三规划只追加checkpoint。没有无用途过程产物需要删除；必要截图、源材料、日志、库、只读脚本及文档前副本均作为失败证据保留。证据索引和最终只读核对不算新测试。

本次辅助取证曾有PowerShell传给rg的通配路径错误、过宽读取输出截断，以及未显式UTF-8的Python stdout显示乱码；后续采用准确路径和`python -X utf8`后，数据库原始输入/回复与用户观察一致，重建body hash匹配。它们不属于产品崩溃或本轮输入损坏。未自动落盘的原工具输出不以事后说明冒充原始日志，见执行目录`driver-errors.md`。

准备阶段曾复制继承环境映射的既有方法边界继续保留，未定向查询、输出、保存或对外发送Key值；不能将其描述为证明环境没有Key。见[原准备方法边界](../../build/xuanshu-br2-meter-real-preparation-20261110-4da059/preparation-boundaries.md)。本次关闭核对未读取凭据或环境Key。

BR1-KI-001保持未解决、用户接受非阻断，本次模型名完整，不据此宣称遗留修复。ASAN121/127与另次6/6、历史期限、复制及其他首败、release计量子进程非ASAN、任意卡死限时关闭未证明等边界保持。已取消的临时控件测试保持取消；真实来源完整分类、IME/DPI、辅助技术、长时和断电等不扩展为本次通过。browser九项副作用不在范围。

本日具体包已经执行并关闭，不可继续使用剩余额度，不再追加真实请求。冻结快照和本地材料仍保留，但本次真实业务验证及其交付收口**暂不通过**。下一轮只能在独立授权下处理分类契约及离线回归，再决定是否准备新的具体真实验证包；不自动修复、冻结、发布或进入其他功能阶段。
