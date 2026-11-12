> 2026-11-10本次已完成：[BR2-REAL-20261110-111dd1真实业务只读限定验证通过](validation/xuanshu-br2-discovery-real-validation.md)，目录分类匹配快照冻结记录和本地交付收尾已完成。唯一PID35088正常exit0；实际空分类发现后执行browser.snapshot，标题/browser:start/revision逐值一致。关后1任务/2消息/31事件/2工具、唯一completed，3次占用/非模拟传输入口/完整响应，unknown0；本地保守绑定价USD0.0014961，不是账户账单。该包已消费，不再启动/提交/确认/读Key；后续真实调用需新具体授权。原模板仍Proposed锁停，旧真实失败/遗留保持。以下历史状态各按原时点保留。

> 2026-11-10最新准备：目录分类快照冻结记录已落盘；[BR2-REAL-20261110-111dd1具体授权包](../build/xuanshu-br2-discovery-real-preparation-20261110-111dd1/authorization-package.md)已准备，尚未批准。执行目录/可执行监督命令未创建，不启动、读凭据或发送。用户仅负责本包批准、必要原生Key及一次完整范围可信确认，其余普通操作和收尾由助手处理。[准备记录](validation/xuanshu-br2-discovery-real-preparation-validation.md)。旧真实失败及旧包消费保持。以下历史状态不自动继承。

> 2026-11-10分类契约限定整改候选已完成开发、自审及离线验证。新根build/xuanshu-br2-discovery-20261110-7e4a96，identity 3DFFCFC515A9958A69396AE703307D6B9E84575D63DD921322D1882E5086F545。分类说明明确空分类只枚举允许read，非空无匹配返回明确提示；直接相关的私有池缩容越界已用独立哨兵和空白名单ASAN因果定位后最小修复。最终Light118/118、ASAN118/118、Native49/49分次通过。
> [最新开发候选](xuanshu-br2-discovery-entry.md)与[完整证据](validation/xuanshu-br2-discovery-validation.md)。旧正式冻结入口仍保留原范围；不自行冻结新候选或重验真实Provider。

> 2026-11-10本次真实业务验证已结束：包BR2-REAL-20261110-4da059已明确批准并仅执行一次，PID23608实际exit0。唯一任务运行时completed并保存，但只执行category=browser的目录查询，browser.snapshot注册于browser.read，实际返回空目录且未调用snapshot，标题/来源/业务返回revision未取得。本次真实业务验收及交付收口暂不通过；原限定冻结和离线结论保持原适用范围，不追加请求、不重启、不修改产品。
> [最新真实执行记录](validation/xuanshu-br2-meter-real-20261110-validation.md)；当前生产组合仍按计量冻结交付清单核对，原入口与历史库不改。任何后续启动或真实补测需要独立具体授权。

> 2026-11-09当前交付：build/xuanshu-br2-meter-20261106-1f8148匹配计量快照冻结记录已落盘，后续文档身份与原快照分开。下文旧费用/恢复入口保留历史范围，当前生产文件按计量交付清单核对。
> [当前计量交付清单](xuanshu-br2-meter-delivery-manifest.md)及[一次集中授权准备](validation/xuanshu-br2-meter-real-preparation-validation.md)。现有本地入口仍未激活，不执行真实请求。

> 2026-11-06 BR-2逐轮完整计量开发完成。候选build/xuanshu-br2-meter-20261106-1f8148接入官方完整Responses转换及每轮实际body发送前门，新增组件独立128MiB限制，原确认、Service及预算合同保持。Light主轮127/127，ASAN主轮121/127并另次原程序6/6对照，最终计量取证补项各20/20，Native38/38，组件15/15。首轮frontend中断/超时根因未定位，旧计量日志的中文路径截断已修为UTF-8，相关旧源码/程序保留。新身份等待限定独立复审，真实策略仍Proposed锁停。详[计量开发记录](validation/xuanshu-br2-meter-validation.md)及[候选入口](xuanshu-br2-meter-entry.md)。原冻结及历史结论保持其原适用范围。

> 2026-11-06 编码规范续查。已找到官方Recipe完整转换/渲染/tokenizer来源，并取得固定初始内容496 token的参考计数。当前仍缺动态续轮发送前计量闭环，真实入口保持锁停，未修改原程序或启动宿主。后续最小计量开发涉及冻结发送链，须另行授权；详[编码链续查记录](validation/xuanshu-br2-encoding-preparation-validation.md)。下文历史状态保留。

> 2026-11-06 BR-2真实验证前置核实。本轮已核对官方模型、Responses协议及当前价格，可靠计费input上界仍缺充分依据，真实启动保持锁停。执行停在集中授权包提交之前，详[本轮前置记录](validation/xuanshu-br2-real-preparation-validation.md)。原匹配快照冻结及本地交付结论保留。下文未核价、待复审等描述保留其历史时点。

> 2026-11-06 BR-2费用恢复匹配快照冻结与本地交付收尾。上一轮限定独立复审通过，匹配identity为1EF3D1643AE1DF94ED6B6CC163F061F1376D82E3A5CC540823F1E197687105FB，冻结记录已落盘，本地材料齐备。当前入口为build/xuanshu-br2-fee-20261106-54d9a8/start-readonly.ps1，完整身份、依赖和监督说明见[本地交付清单](xuanshu-br2-delivery-manifest.md)，结论见[独立复审与冻结记录](validation/xuanshu-br2-fee-restore-independent-review.md)。真实验证继续Proposed、未核价锁停。

从仓库根目录使用以下当前命令。此处仅写入说明，启动需相应授权。

```powershell
Set-Location -LiteralPath 'D:\应用软件框架\应用层序框架'
& '.\build\xuanshu-br2-fee-20261106-54d9a8\start-readonly.ps1'
```

下文保留各旧候选及其当时结论，当前路径与冻结状态以上述清单为准。

> 当前费用恢复候选为build/xuanshu-br2-fee-20261106-54d9a8，入口是该根start-readonly.ps1，文件身份按同根delivery-identity.json及launch-files.json核对。开发及离线验证完成，等待再次限定独立复审，Proposed模板继续锁停真实发送。下文保留历史入口记录。

# BR-2 本地交付入口

本轮候选：build/xuanshu-br2-20261105-9b7f14。开发及离线证据与独立复审、真实服务授权分开；未自行冻结或发布。

## 文件与启动

从仓库根目录，在未来获得本地启动授权后运行：

```powershell
& .\build\xuanshu-br2-20261105-9b7f14\start-readonly.ps1
```

本轮未运行生产宿主人工流程。入口核对launch-files.json中的生产framework_host.exe、ui_framework.dll、br1-production.uapp和策略/监督脚本。新建唯一sessions目录，UI_AI_DB_PATH指向conversation.sqlite；模型文件为其追加.model.json；UI_BROWSER_SETTINGS_PATH指向空origins.txt。UI_AI_BR2_POLICY指向派生且绑定此数据库路径的business-policy.json。三个临时变量均恢复，不改变LOCALAPPDATA。

br1-production.uapp使用实际apps/browser及本轮包内合成交接单；不是rpc_memory，也不等于任意真实用户网页覆盖。测试包br1-isolated.uapp、ui_framework_edit_test.dll和私有可信事件只用于离线测试，不得替代生产文件。

## 默认锁停

policy-proposed.json的价格/计费上界未核实，默认禁止接纳BR-2验证任务及发送。可以查看业务应用和本地设置，但“未核实而锁停”不是正常业务失败。入口拒绝已标记verified的模板，不能靠改布尔值把本地入口改成真实调用入口。

独立限制接口ui_ai_business_validation_enable仅安装更严格策略，不签发可信确认。未来真实验证须另行核对官方资料、生成一次数据库绑定策略并集中授权；该操作不得复用已打开/锁停的库，也不得在已执行后另建库继续同一预算。见[待授权包](xuanshu-br2-real-proposal.md)。

## 证据与停止

每个会话保存prestart、PID、stdout/stderr、result及实际退出码。无进程时PID/退出码为空；辅助记录失败仍继续监督；不自动重启、不强杀。启动或记录失败按stage、PowerShell异常及可取得的系统码区分。主目录不可写时只能报未落盘，不伪造记录。

正式界面仍以“保存任务并查看范围”进行本地提案，以“确认此任务外发范围（一次）”独立授予逐任务许可。摘要和display_valid单字段都不代表可发送；完整详情、实际文本/布局、run/hash/代次和末次回调检查仍适用。请求取消不等于远端停止，unknown不清零。存储错误首次关闭须展示错误；安全收尾后才能退出，不以清错或恢复派发解除等待。

关后只核对自有库，保留未合并WAL时不得用immutable忽略它。本轮测试自有库与启动辅助进程目录独立；辅助进程结果不冒充生产宿主验收。

BR1-KI-001仍未解决，按用户决定非阻断保留；发现模型读回/绑定不一致立即停对应任务。不恢复已取消的临时页面控件测试。


## 2026-11-06 恢复整改候选入口

新的本地候选根为build/xuanshu-br2-restore-20261106-3c8d42。入口采用该根start-readonly.ps1，按同根launch-files.json核对正式宿主、生产DLL及实际浏览器样本包。监督脚本、未核价策略模板沿用已验证字节，监督8项及策略辅助2项按身份复用。新程序的恢复审计在READY之前拒绝不完整或误分类账本。独立复审及集中授权待另行完成，当前入口保持Proposed锁停。对应源码与程序身份以新根delivery-identity.json为准。


## 2026-11-06 费用恢复兼容候选

最新候选根为build/xuanshu-br2-fee-20261106-54d9a8，对应start-readonly.ps1及launch-files.json。生产宿主、DLL及br1-production.uapp在新根匹配，不覆盖前置交付。监督和Proposed未核价模板保持原验证字节，启动监督8项及辅助2项按身份复用。入口默认锁停真实验证，真实授权前继续核实官方协议/价格、可靠计费上界及独立集中授权。新费用语义和证据见[费用恢复整改记录](validation/xuanshu-br2-fee-restore-validation.md)，最终源码、程序身份以同根delivery-identity.json为准。
