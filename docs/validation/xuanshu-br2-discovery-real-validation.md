# BR-2目录分类整改后一次真实业务只读验证

2026-11-10，Asia/Shanghai。**本次真实业务只读限定验证通过。** 匹配快照冻结记录已落盘，本地交付材料已齐备。本结论仅适用于本次实际browser包内合成交接单、固定只读范围和唯一批准会话，不是整个系统、任意业务应用、全部人工项目或正式发布通过。

## 冻结身份、授权和实际启动

冻结根为`build/xuanshu-br2-discovery-20261110-7e4a96`。delivery-identity.json SHA256：

`3DFFCFC515A9958A69396AE703307D6B9E84575D63DD921322D1882E5086F545`

[限定独立复审及冻结记录](xuanshu-br2-discovery-independent-review.md)追记此前已完成结论，本轮没有重新全面复审、修改产品或重跑离线测试。Light118/118、ASAN-Light118/118、Native49/49仍是三个独立历史运行，新文档状态不回写原快照。

| 生产文件 | SHA256 |
| --- | --- |
| light/framework_host.exe | B8CAAB424048A7F1BADF923630DA856D8978CD700B4AE1DC388F5B0DE79F0C7D |
| light/ui_framework.dll | 9D038E77079D832BD94F0D3E7E89ED47AF8050A356A7291E8B46F580CEF77B99 |
| light/br1-production.uapp | 4855AA94DE99763C59C6E494F29AC2087080B348C524FF473180498D062786F4 |
| light/xuanshu_br2_meter.exe | 7E2C2FF2508A83F05E1DB5B891635DABFA843812B4078BC0D7D4FEC3CB6C0EBB |

用户明确批准[BR2-REAL-20261110-111dd1具体包](../../build/xuanshu-br2-discovery-real-preparation-20261110-111dd1/authorization-package.md)，答复“明确批准本日包，仅执行这一次”。包SHA256为`CE004DE7DA98BAE67EADDFEDB5F9D578E03B870E23DD3E8FC2BD285A97EEC917`。准备记录见[前置准备](xuanshu-br2-discovery-real-preparation-validation.md)，批准前postflight等仍保留原历史时点，不改写成当时已经批准。

新执行根：[build/xuanshu-br2-discovery-real-run-20261110-111dd1](../../build/xuanshu-br2-discovery-real-run-20261110-111dd1/)。唯一生产PID35088，监督exec session99700，启动时间记录为06:31:44.8177762Z。只有一次生产启动；没有换库、重启、重复提案或追加请求。执行命令使用原监督函数及本包固定绑定，实际进程句柄监督至exit0，环境恢复成功，监督错误0，stdout/stderr均0字节。

显式隔离UI_AI_DB_PATH、UI_BROWSER_SETTINGS_PATH和UI_AI_BR2_POLICY，模型配置由数据库路径追加.model.json。未改变LOCALAPPDATA、迁移用户库或复用旧凭据引用。Key仅用户经原生窗口录入；助手在该阶段暂停截图/剪贴板/输入，没有读取Key、环境Key、密码框、Authorization或凭据Blob。正式程序按其合同使用本次原生凭据。

## 实际操作和证据分层

用户仅完成具体包批准、原生Key录入和一次逐任务可信确认。其余普通配置、任务/范围、读回、监督、正常关闭和关后取证由外部脚本完成。未用computer use、子代理、测试DLL、Service内部调用、私有可信事件或伪造source。普通脚本输入为合成输入，不是实际设备来源正例。

| 项目 | 实际结果 | 执行根证据 |
| --- | --- | --- |
| 模型配置 | 输入全文deepseek-flash相等，保存后代次1；用户原生保存后本地可读取、代次2，服务端未验证状态如实区分 | model-input-readback.txt；screens/05-credential-state.png |
| 草稿及结构化范围 | 固定任务全文相等；历史未选；仅browser.view、browser.snapshot、revision/state，find_text未选 | task-input-readback.txt；screens/13-selected.png |
| 本地接纳 | 只点击一次保存；phase3、committed、未确认、模型请求未保存，额度和传输入口0 | proposal-click-intent.json；accepted-diagnostic.json；accepted-check.json |
| 完整预览 | 两块实际只读控件全文复制后可拼接为完整JSON；run/hash/固定目标与接纳值相同，request与授权正文逐值和哈希一致 | detail-0.txt、detail-1.txt；request-readback.json；preconfirm-readback-check.json |
| 可信确认 | 用户反馈“已确认”；随后软件和持久化记录均有一次确认及保存。助手未点击确认 | user-confirmation-report.json；terminal-diagnostic.json；disclosure事件 |
| 目录和业务结果 | 模型实际category为空，发现允许的browser.snapshot，再执行一次实际业务读取 | closed-business-db.json；两条tools记录 |
| 终态和保存 | completed、无任务错误；确认/请求/终态保存是，保护business_task_finished锁停 | screens/24-terminal-facts.png；terminal-check.json |
| 正常关闭 | 助手WM_CLOSE，等待同一进程实际退出0，不强杀/提前卸载 | actions.jsonl；result.json |

唯一任务run_id为`1e17e5a8892b3e906ac06f91b9e64a4ea164e21f8f49d78df8d7f76de893cd07`，client_request_id为`panel-1a1c43558ba-2`。绑定hash为`8cccb9800a6375096207828db538e0471042270d1657f0ae39a09ce4cc432df4`。

固定目标instance=1，host epoch=`b8f63a0b-75a1-4750-ad50-3ff3bfe6fb9e`，open epoch=`32c34093-5576-416f-8b6b-1f98eedbf326`，resource=browser.view，scope={"access":"read"}。revision：

`77fbdb9e18262e8aa0c807ae275fbfdab249f027c46f7068e4831509755f70af`

固定输入为“BR-1：读取本轮合成交接单标题、来源与版本，不导航或修改。”；history=[]，能力仅browser.snapshot，字段仅revision/state。接纳、预览、两条工具target和关后任务逐值一致，没有当前草稿或标签改绑。

## 精确业务结果与实际请求

实际apps/browser业务读取及持久化工具结果为：state.title=`BR-1 合成交接单：设备巡检`，state.address=`browser:start`，state.status=`包内起始页`，policy_count为整数0；返回revision与固定目标相等。最终回复标题、来源和revision与工具值对应，明确快照不提供正文或业务版本号。本轮不声称正文12/9/3汇总。

两条工具为一次read-directory及一次browser.snapshot，status均0。目录为空分类实际枚举，允许项为browser.snapshot、category=browser.read。业务模型上下文仅投影revision/state；原工具结果保留空find_text，目录Schema中的字段名不冒充字段数据。state仍是完整顶层对象，不是仅title。

| 轮次 | 发送前输入计量＝返回input usage | output usage | 返回cached input | 按保守绑定价的known nano USD |
| --- | --- | --- | --- | --- |
| 1 | 538 | 85 | 128 | 263400 |
| 2 | 1129 | 80 | 512 | 434700 |
| 3 | 1472 | 297 | 1152 | 798000 |

每轮实际请求manifest、占用、计量回执和使用量关联一致。三份request重建哈希分别与保存的manifest和meter相同，初始正文哈希仍为`3B2229C22AF89CF0D5948573EC7FD6873E7B5629E7F578342B7BDAE7E0B9BB0B`。这是持久化上下文重建核对，不是传输抓包。

记录为3次占用、3个非模拟传输入口、3个完整响应，unknown=0；不将这些阶段统计写成独立的精确HTTP计数。服务端响应时间字段从14:59:57至15:00:03，不冒充全程网络遥测。没有额外凭据探测、模型列表、账户/余额或价格API。

批准和确认前均重新核对12份当天官方公开依据及四个生产文件。价格按峰时非缓存输入300、输出1200 nano USD/token，16384输入/4096全部输出，每轮预留9830400 nano USD，三次保守总预留USD0.0294912。实际三次known合计1496100 nano USD，即USD0.0014961，estimated归0、fee_unknown=false，低于USD0.10阈值。缓存优惠未用于本地保守计算，账户实际扣费未查询。USD0.0014961不能冒充账户账单或服务端绝对收费保证。

## 关后数据库和保全

实际退出后才打开本次自有库，先确认无WAL/SHM/journal，使用mode=ro、immutable=1及query_only，未重开Service、checkpoint或恢复执行。

数据库SHA256读取前后均为：

`5737520AE778B17E5C58A7287EBA10E32BE4870B9D058481CD16ED2F3C44CACB`

关后1任务、2消息、31事件、2工具，唯一completed终态。确认1次，provider_request/response、reserved、send-intent、send-entered、network-end、cost各3条；原计数、费用、范围和最终回复对应，没有重复提交或工具重放。完整观察见[closed-business-db.json](../../build/xuanshu-br2-discovery-real-run-20261110-111dd1/closed-business-db.json)，新只读观察脚本和输出同根，不运行历史生成脚本。

仅删除本次24份二倍放大OCR派生PNG，共11674237字节。原始截图、OCR文字、普通操作记录、原准备和最终驱动、异常说明、完整预览、批准及监督命令、请求重建、脱敏诊断、关后库与清单保留。原交付、冻结SDK、历史失败及47旁文件（含5份非空WAL）不改；本次无旁文件结论不扩展到历史材料。

## 异常、限制及交付停止

本轮没有已证实产品缺陷。外部驱动的小字/中文OCR定位、Windows PowerShell默认ANSI读取UTF-8授权记录、前台复制受限以及复制按钮/状态行同名分别记录；失败时没有点击确认、重复提案或增加Provider请求。修正仅限外部普通控件驱动。未自动落盘的原工具错误仍以对话为原始来源，后补说明不冒充原始CLI日志。文档写入曾被自动审批拒绝，未执行，后改用文件编辑工具完成。旧快照和预算均不因这些辅助问题改变。

BR1-KI-001仍未解决、用户接受非阻断；P3哨兵日志建议、历史ASAN/期限/复制首败及其他未知根因不由本次解释。真实来源完整分类、IME/DPI、辅助技术、长时及断电等边界保持；已取消临时控件测试不恢复，browser九项副作用另案。

旧BR2-REAL-20261110-4da059仅空目录、未snapshot的真实失败保留，新成功不覆盖旧失败。本次BR2-REAL-20261110-111dd1已消费并结束，不能复用剩余额度、旧命令或凭据引用。原start-readonly入口和模板仍Proposed锁停；本次专用已批准命令不推广为通用批准入口。后续任何真实任务、开发或功能阶段均需独立授权。

[匹配交付入口](../xuanshu-br2-discovery-entry.md)。停在本次验证与交付收尾，不再启动或发送。
