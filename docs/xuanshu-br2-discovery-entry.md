# BR-2当前只读交付入口（阶段已完成）

2026-11-10：**本页是唯一当前交付入口。** 当前匹配快照已冻结，真实业务只读限定验证和最终收口复核通过，必须修正项为空。BR-2阶段已完成，没有进行中的本阶段宿主/Service、待录Key、待确认或待续跑步骤。

基线为`build/xuanshu-br2-discovery-20261110-7e4a96`；delivery-identity.json SHA256为`3DFFCFC515A9958A69396AE703307D6B9E84575D63DD921322D1882E5086F545`。正式宿主、生产DLL、实际browser合成样本包和本地计量组件的完整哈希见下方原[匹配身份](#匹配身份)表，本轮必要核对均匹配。原交付清单和程序不改写，后续文档状态独立于原交付副本。

| 当前结论及说明 | 记录 |
| --- | --- |
| 匹配快照限定验收及冻结 | [独立复审和冻结记录](validation/xuanshu-br2-discovery-independent-review.md) |
| 已完成真实业务验证 | [真实验证及关后证据](validation/xuanshu-br2-discovery-real-validation.md) |
| 最终收口与阶段关闭 | [最终限定收口复核归档](validation/xuanshu-br2-discovery-final-closeout-review.md) |
| 原始脱敏材料索引 | [执行证据身份](../build/xuanshu-br2-discovery-real-run-20261110-111dd1/execution-identity.json) |

## 当前使用说明与停止位置

以下说明现有控件及运行合同，不要求现在启动或再次验证：

- “当前任务与处理状态”展示当前查看任务；“任务输入（填写要本地保存的任务内容）”是新任务草稿，修改草稿不改变已接纳任务。
- 结构化选择历史、目标、read能力及字段；“保存任务并查看范围”只进行本地提案，应分别查看持久化接纳、固定范围及保存状态。
- “查看本任务范围（只查看，不授权）”查看已接纳任务输入、历史、固定目标和白名单；“完整请求详情”保留完整请求。摘要、display_valid或页面投影成功不能单独获得外发资格；仍需run/hash、展示代次、实际文本/布局及末次回调后条件。隐藏、投影失败或条件变化使旧资格失效。
- 外发确认是独立的逐任务可信人工授权。没有新具体授权包和满足的保护条件时，不录Key、不确认、不发送；旧包已结束，不能用于下一任务。
- “请求取消”不等于取消终态、最终保存或远端停止。异常时保留原错误及unknown，停止后继操作，不重试补额度；“复制脱敏诊断”不授权或发送，随后按合同正常关闭。

已完成场景为实际apps/browser包内合成交接单，history=[]，固定browser.view，仅browser.snapshot及state/revision。state是完整顶层对象；验证得到标题、来源和框架revision，不是正文摘要、业务版本或任意应用覆盖。结果、工具及保存证据相互对应，唯一PID35088实际exit0。

具体包BR2-REAL-20261110-111dd1已消费并关闭；默认start-readonly及Proposed模板仍锁停真实验证。下方原启动命令仅作机制说明，不能自动续跑历史命令、复用旧库/额度/凭据引用或恢复旧授权。本轮没有启动程序、读取凭据或打开数据库。

后续仅Proposed“当前网页正文只读摘要”，需独立能力、数据和保护规格及另行开发/真实调用授权；本轮不盘点开发、不核价、不准备新包。历史首败、旁文件及未验边界保留，BR1-KI-001仍未解决，临时页面控件测试保持取消。

## 历史记录（保留原文）

以下内容按各自记录时点保留。旧的“待批准”“待确认”“执行中”和“下一步”均为历史，不构成当前操作或授权；当前状态以上方阶段关闭记录为准。

---

> 2026-11-10本次已完成：[BR2-REAL-20261110-111dd1真实业务只读限定验证通过](validation/xuanshu-br2-discovery-real-validation.md)，目录分类匹配快照冻结记录和本地交付收尾已完成。唯一PID35088正常exit0；实际空分类发现后执行browser.snapshot，标题/browser:start/revision逐值一致。关后1任务/2消息/31事件/2工具、唯一completed，3次占用/非模拟传输入口/完整响应，unknown0；本地保守绑定价USD0.0014961，不是账户账单。该包已消费，不再启动/提交/确认/读Key；后续真实调用需新具体授权。原模板仍Proposed锁停，旧真实失败/遗留保持。以下历史状态各按原时点保留。

> 2026-11-10最新准备：目录分类快照冻结记录已落盘；[BR2-REAL-20261110-111dd1具体授权包](../build/xuanshu-br2-discovery-real-preparation-20261110-111dd1/authorization-package.md)已准备，尚未批准。执行目录/可执行监督命令未创建，不启动、读凭据或发送。用户仅负责本包批准、必要原生Key及一次完整范围可信确认，其余普通操作和收尾由助手处理。[准备记录](validation/xuanshu-br2-discovery-real-preparation-validation.md)。旧真实失败及旧包消费保持。以下历史状态不自动继承。

> 2026-11-10追记：匹配快照限定独立复审通过，[冻结记录已落盘](validation/xuanshu-br2-discovery-independent-review.md)，必须修正项为空。下文开发等待状态为原交付历史，不改原副本。新真实重验包BR2-REAL-20261110-111dd1正在准备，尚未批准、未启动；原入口仍Proposed锁停。

# BR-2目录分类整改候选交付入口

2026-11-10。最小分类契约和直接相关池缩容修复完成，Light118/118、ASAN118/118、Native49/49分别通过。当前等待限定独立复审，不是正式冻结、发布或真实业务重验通过。详[开发验收及首败](validation/xuanshu-br2-discovery-validation.md)。

## 匹配身份

运行根为`D:\应用软件框架\应用层序框架\build\xuanshu-br2-discovery-20261110-7e4a96`。delivery-identity SHA256：

`3DFFCFC515A9958A69396AE703307D6B9E84575D63DD921322D1882E5086F545`

| 必要运行文件 | 用途 | SHA256 |
| --- | --- | --- |
| light/framework_host.exe | 正式DH-1宿主 | B8CAAB424048A7F1BADF923630DA856D8978CD700B4AE1DC388F5B0DE79F0C7D |
| light/ui_framework.dll | 生产运行时 | 9D038E77079D832BD94F0D3E7E89ED47AF8050A356A7291E8B46F580CEF77B99 |
| light/br1-production.uapp | 实际apps/browser及包内合成交接单 | 4855AA94DE99763C59C6E494F29AC2087080B348C524FF473180498D062786F4 |
| light/xuanshu_br2_meter.exe | 同目录匹配纯本地计量组件 | 7E2C2FF2508A83F05E1DB5B891635DABFA843812B4078BC0D7D4FEC3CB6C0EBB |

包是实际业务实现和合成文档，不读取用户旧业务文件，不把它称为任意应用覆盖。Windows/VC依赖沿原交付环境；运行无需Rust工具链。component、asan-light、native、测试DLL、隔离包、fixture组件和失败档案用于验证，不能替换生产DLL或作为人工授权入口。

## 待授权的本地入口

工作目录为仓库根。以下只写入说明，本轮没有执行：

```powershell
Set-Location -LiteralPath 'D:\应用软件框架\应用层序框架'
& '.\build\xuanshu-br2-discovery-20261110-7e4a96\start-readonly.ps1'
```

start-readonly、supervise及未核价Proposed模板与原机制逐字节相同，launch-files更新为本候选身份。入口创建唯一新隔离会话，显式设置UI_AI_DB_PATH、UI_BROWSER_SETTINGS_PATH、UI_AI_BR2_POLICY，模型配置为DB路径加.model.json；不改变LOCALAPPDATA、不迁移配置、不复制凭据。保留PID、stdout/stderr、实际退出码及阶段错误，不循环重启或强杀。

当前Proposed未核价且锁停真实验证，不承诺该锁停入口可提交AI验证任务。启动默认没有任务/发送或凭据访问；任何后续启动仍须相应授权。旧BR2-REAL-20261110-4da059已执行并关闭，不再运行原命令或使用剩余额度。不能通过修改模板布尔值把本候选视为已批准真实调用。

## 使用范围及下一阶段

正式界面仍为“保存任务并查看范围”，本地保存不等于发送。普通任务逐任务可信确认保持；摘要不能替代完整预览，隐藏、旧代次、投影失败或PENDING均拒绝旧确认。新包装说明空分类发现当前目标已授权read能力，命名分类精确匹配；分类未命中提示不扩大权限或自动调用。

业务范围保持history=[]、browser.view、browser.snapshot、state/revision及原固定输入。state是完整顶层对象，不能宣传仅title或正文汇总。请求含初始和全部续轮最多3次，原期限、费用、unknown、唯一终态及cleanup-only保持。

异常时保留任务、原错误和unknown，停止提交/确认，不重试补额度，正常取消与关闭后才读取本次自有库；旁文件存在时不能用immutable忽略未合并WAL。诊断只使用“复制脱敏诊断”，不读取密码框或Key。需要修复或再次真实验证时均另行授权。

下一步仅为本候选限定独立复审，随后才能准备新的当日模型/协议/价格及计费上界核对、唯一隔离策略、具体集中授权包和用户逐任务确认。本轮未授权真实重验，亦未恢复已取消的临时控件测试。
