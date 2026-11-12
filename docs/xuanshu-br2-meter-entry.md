> 2026-11-10分类契约限定整改候选已完成开发、自审及离线验证。新根build/xuanshu-br2-discovery-20261110-7e4a96，identity 3DFFCFC515A9958A69396AE703307D6B9E84575D63DD921322D1882E5086F545。分类说明明确空分类只枚举允许read，非空无匹配返回明确提示；直接相关的私有池缩容越界已用独立哨兵和空白名单ASAN因果定位后最小修复。最终Light118/118、ASAN118/118、Native49/49分次通过。
> [候选匹配程序及Proposed入口](xuanshu-br2-discovery-entry.md)。这不是将旧冻结结论转移到新代码；新候选等待限定独立复审，当前未授权真实启动或发送。原当日真实包已消费，不再执行旧命令。下文旧冻结/真实失败记录保持。

> 2026-11-10本次真实业务验证已结束：包BR2-REAL-20261110-4da059已明确批准并仅执行一次，PID23608实际exit0。唯一任务运行时completed并保存，但只执行category=browser的目录查询，browser.snapshot注册于browser.read，实际返回空目录且未调用snapshot，标题/来源/业务返回revision未取得。本次真实业务验收及交付收口暂不通过；原限定冻结和离线结论保持原适用范围，不追加请求、不重启、不修改产品。
> [实际过程、固定目标、费用及退出证据](validation/xuanshu-br2-meter-real-20261110-validation.md)。原start-readonly/Proposed模板、封装包和程序不改；已执行的单次命令不得再运行。下一轮最小建议仅为只读发现分类可发现性与参数契约及对应离线回归，尚未授权实施。下文准备/未执行记录保留历史时点。

> 2026-11-10当前入口状态：冻结生产组合未变，11月9日包已过期。本日唯一包BR2-REAL-20261110-4da059已准备、尚未批准；固定一次执行命令只作静态审阅，未运行或动态验证。
> [本日准备与可审阅命令](validation/xuanshu-br2-meter-real-preparation-20261110-validation.md)。原锁停入口/模板保持，不通过改布尔值直接执行。下文旧日期记录保留历史状态。

> 2026-11-09当前入口：匹配计量快照已限定冻结，本地交付材料齐备；真实验证的公开依据和具体集中包已准备，尚未批准或执行。原start-readonly及Proposed模板继续锁停。
> [交付清单](xuanshu-br2-meter-delivery-manifest.md)；[冻结记录](validation/xuanshu-br2-meter-independent-review.md)；[准备与待确认包](validation/xuanshu-br2-meter-real-preparation-validation.md)。不能直接修改原模板布尔值启动真实验证。

# BR-2逐轮计量候选交付入口

当前候选位于build/xuanshu-br2-meter-20261106-1f8148。它承接费用恢复冻结快照，新增逐轮完整请求计量与失败锁停，形成独立程序身份。开发自审和离线证据与后续限定独立复审、冻结、真实服务验证分别记录。

## 本地文件与命令

从仓库根目录核对同根delivery-identity.json、launch-files.json和源码清单。未来取得本地启动授权后可使用下列入口，本轮只整理说明。

```powershell
Set-Location -LiteralPath 'D:\应用软件框架\应用层序框架'
& '.\build\xuanshu-br2-meter-20261106-1f8148\start-readonly.ps1'
```

正式运行组合为light/framework_host.exe、light/ui_framework.dll、light/br1-production.uapp及light/xuanshu_br2_meter.exe。该组件与包含Service的模块同目录，其SHA256已编入生产DLL。Windows/VC运行库按同根dependents证据核对，运行不需要Rust工具链。component目录保留匹配组件的交付副本和独立组件测试依据。

br1-production.uapp使用实际apps/browser及包内合成交接单。只读目标browser.view，能力browser.snapshot，字段state和revision。state是完整顶层对象，当前场景读取标题、来源与框架版本，样本正文12/9/3没有被该接口汇总。隔离包br1-isolated.uapp、ui_framework_edit_test.dll和fixture计量组件属于离线验证材料。

入口按既有监督机制创建唯一sessions目录，UI_AI_DB_PATH指定新库，模型配置派生为数据库路径追加.model.json，UI_BROWSER_SETTINGS_PATH指定本会话空来源配置，UI_AI_BR2_POLICY指定与该库绑定的派生策略。它不迁移旧库或配置，不改变LOCALAPPDATA。prestart、PID、stdout/stderr、阶段异常和实际退出码均由监督脚本记录，失败不循环重启，辅助记录失败仍保留监督。

## 真实验证保持Proposed

policy-proposed.json的verified=false、费率0及input_ceiling=0保持锁停。新增meter_profile及meter_evidence_sha256固定完整Responses转换依据。该本地入口仍只接受未激活模板，不能通过修改布尔值把它变为真实调用入口。

后续真实验证须先完成本候选的限定独立复审，并再次核对官方模型、协议、价格和编码版本，准备与唯一新库绑定的受信策略及监督配置，取得集中授权及逐任务可信确认。候选计量上界可设16384 input token，由程序对每轮实际body执行硬检查，超界即停止。该值不缩减普通任务的数据范围。[Proposed授权包](xuanshu-br2-real-proposal.md)

日常本地提案仍使用“保存任务并查看范围”，实际发送仍要求“确认此任务外发范围（一次）”。完整详情、实际文本及布局、run/hash、展示代次和末次回调条件保持。计量通过和display_valid都不会单独签发外发资格。

## 停止与保存

取消请求、终态及最终保存按正式界面分别判断。计量失败、组件身份不符、计数超界、usage异常、期限或存储错误均停止后继，保留既有占用与unknown。计量辅助进程由worker持有至真实退出，UI关闭保持原PENDING及cleanup-only语义。存储错误先展示并确认错误，再完成正常关闭。

关后只读核对当前自有库。存在非空WAL时保留并报告证据范围，不以immutable忽略未合并记录。无全程网络遥测时只报告模型路径的软件和存储事实，额度占用、manifest和传输入口分别计数。

BR1-KI-001仍未解决、用户接受非阻断。实际再次出现模型名不一致时保留现场并停止受影响任务。已取消的临时控件测试保持取消，真实来源、IME、DPI、辅助技术及其他范围外事项继续按原记录分账。
