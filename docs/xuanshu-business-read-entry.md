> 2026-11-05 BR-2最新候选：验证专用3次总额度、10分钟及费用/unknown硬停止开发和离线验证完成；Light73/73、ASAN73/73、Native23/23分次通过。新身份与旧BR-1分开，未独立通过/冻结。当前入口价格及可靠计费上界未核实，真实发送锁停，待集中授权。BR1-KI-001仍未解决、用户接受非阻断。见[开发验收](validation/xuanshu-br2-validation.md)、[入口](xuanshu-br2-entry.md)、[Proposed授权包](xuanshu-br2-real-proposal.md)。下文为历史记录，不改写旧首败或旧结论；已取消临时控件测试不恢复。

> 2026-11-05 最新用户决定：模型名少字符改列BR1-KI-001“未解决、用户接受的非阻断遗留问题”，后续再现时定位；不再主动重复排查或阻断本地交付推进。不是问题已修复，也不追改历史复审为通过。当前本地入口采用consistency候选已验证的启动监督，详[遗留与推进记录](validation/xuanshu-br1-known-issue-disposition.md)。真实请求硬门及真实调用仍需分别授权，本轮不执行。

> 2026-11-05 BR-1一致性整改：启动异常取证已完成并以隔离辅助进程8/8验证；模型名少字符仍未定位，冻结阻断保留。新增逐边界探针及相称回归通过不能替代原根因；新候选build/xuanshu-br1-consistency-20261105-f4c92a仅诊断/监督交付。详[记录](validation/xuanshu-br1-consistency-validation.md)。不进入真实调用或人工验收。

# BR-1 真实浏览器文档属性只读入口

本轮选择仓库实际受限浏览器apps/browser，读取已加载文档标题、来源及版本。样本是本轮新建的包内合成交接单，不是rpc_memory；业务链仍为browser.view → browser.snapshot →实际文档状态。不是任意网页全文提取或表格汇总功能，样本正文数字不作为本场景可读结果承诺。

## 单一启动入口

工作目录：仓库根 `D:\应用软件框架\应用层序框架`。

```powershell
& .\build\xuanshu-br1-consistency-20261105-f4c92a\start-readonly.ps1
```

上述入口已准备，本轮没有启动人工验收。脚本先核对launch-files.json的程序哈希，再新建唯一sessions子目录，设置UI_AI_DB_PATH及UI_BROWSER_SETTINGS_PATH；不改变LOCALAPPDATA、不复用原库或凭据。配置为conversation.sqlite.model.json，来源文件为空，默认不提交任务/确认外发。记录PID、stdout/stderr及实际退出码，不循环重启或强杀。需要兼容Windows及Microsoft VC x64运行库；不是安装器或全新机器部署验收。

运行文件为light/framework_host.exe、light/ui_framework.dll、light/br1-production.uapp；后者是生产browser DLL与本轮样本资源的包。light/br1-isolated.uapp及ui_business_*测试程序用于离线验证，不能替代生产包。样本原文见samples/browser-read/start.html；模型及凭据替身仅在测试构建存在。

## 日常任务路径

展开AI，在设置中仅保存offline-fixture合成配置，不设置Key。新任务输入：

> BR-1：读取本轮合成交接单标题、来源与版本，不导航或修改。

历史不选；选择当前浏览器的browser.view、browser.snapshot及state/revision字段；点击“保存任务并查看范围”。收到不等于持久化接纳，预览不等于发送。state是现有完整顶层对象，包含title、address、阅读状态与诊断，不是只允许title的字段级投影。当前草稿、配置和已接纳任务固定范围各自独立。

“查看本任务范围（只查看，不授权）”与完整请求详情用于检查输入、目标、模型和工具。摘要不能代替完整展示；隐藏、失败、旧代次、配置/目标变化使旧确认失效。display_valid不是独立授权。普通生产仍逐任务可信人工确认，本轮不授权点击外发确认。

离线自动验证通过隔离传输及可信事件夹具完成实际业务read；它不是生产自动批准，也不能在生产脚本中调用。预期业务标题“BR-1 合成交接单：设备巡检”，address=browser:start，业务revision与公开框架返回revision及固定目标一致；精确结果以持久化工具记录为准，模型回复不是业务成功证据。

需要停止时“请求取消当前任务”，等待终态及保存，再正常关闭。取消不等于远端停止；UNKNOWN/cleanup-only不清除。异常保留程序身份、脱敏诊断、任务关联ID与对应日志，不上传Key、Authorization、凭据Blob或内存。关闭后只读核对本轮自有库，不重开Service恢复执行，不擅自checkpoint。

## 当前边界

[接入合同](xuanshu-business-read-contract.md)、[开发验收](validation/xuanshu-business-read-validation.md)、[待授权真实验证包](xuanshu-business-read-real-proposal.md)分开记录。本轮不是独立复审、正式冻结、真实模型或真实人工验收；临时页面控件测试维持取消，不恢复。

同一workspace按app_id复用已加载模块与资源包；不要在一个已有浏览器进程中混装本样本包与同ID其他包来期待资源替换。交付脚本始终新进程，仅加载本样本；不修改包加载规则。


## 2026-11-06 BR-2恢复整改候选

本地候选入口已整理到build/xuanshu-br2-restore-20261106-3c8d42/start-readonly.ps1，生产程序和样本包由同根清单固定。参见[BR-2入口](xuanshu-br2-entry.md)、[恢复整改记录](validation/xuanshu-br2-restore-validation.md)。真实调用继续等待限定独立复审、价格/计费上界核对及集中授权。
