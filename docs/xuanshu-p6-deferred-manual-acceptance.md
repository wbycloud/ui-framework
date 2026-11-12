# P6软件整体集中人工验收准备

状态：延期未验，2026-10-27。沿用P2/P3/P4/P5原延期项目，不删除、不署名。

## 1. 匹配程序和未来启动

本轮证据build/xuanshu-p6-20261027-4a92d1；以最终delivery-identity.json/source-after.json/binary-files.json核对。正式Light/framework_host.exe、ui_framework.dll与原名称uapp用于将来人工；ui_framework_edit_test.dll/p4-*.uapp仅隔离自动测试，不能作为生产来源正例。

得到集中人工授权后，从仓库根使用全新隔离库，当前未执行：

```powershell
$env:UI_AI_DB_PATH = Join-Path (Get-Location) 'build/xuanshu-p6-20261027-4a92d1/manual-future/conversation.sqlite'
& 'build/xuanshu-p6-20261027-4a92d1/light/framework_host.exe' 'build/xuanshu-p6-20261027-4a92d1/light/web_counter.uapp'
```

按实际路径、schema和备份位置逐次确认1→2→3；不能用用户库。退出后清本进程环境变量，不删在用WAL/SHM。

## 2. 延期矩阵

| 项目 | 当前替代证据与不足 | 未来操作及结果 |
| --- | --- | --- |
| 真实鼠标/键盘、Windows分类 | 私有隔离接纳后的业务/负例，不证明物理正例，HARDWARE非物理证明 | 原菜单/按钮/快捷键/表单/画布与正式AI确认逐项操作；程序消息和可识别注入拒绝。记录origin/device/链，不声称隔离UIAccess/恶意DLL |
| 整体任务/审批/UNKNOWN体验 | 自动模型/revision/事务/回执/重启保护；用户可理解性未判 | 单主对话直接read与跨五应用图，切标签/收起/重建，问题只给所属节点；范围30分钟/风险批准5分钟。取消不清门，UNKNOWN仅新的人工只读退出/回读证明解除 |
| IME、DPI、视觉与辅助技术 | 模型/DOM/有界面板循环，不覆盖真实桌面 | 中日文组合输入、屏幕阅读器/焦点/长范围分页、跨不同DPI屏幕；无截断/误触。保留原P2详细要求 |
| 物理断电/设备写缓存 | 子进程退出、SQLite VFS错误和隔离坏副本，不是实际断电 | 整体后另定专机/可恢复镜像和明确授权；绝不在用户设备强断电或操作用户库。核对意图/消费/真实业务状态和唯一终态 |
| 用户长期历史/保留选择 | 本轮不自动删除历史；数据库增长按保留记录解释 | 用户决定归档/保留策略后另评审，不删未完成/UNKNOWN/未确认/必要关联；不能把压平DB曲线当稳定性 |
| 人工长时及交互资源 | 600秒/60轮/有界采样仅有限自动样本 | 软件整体完成后另定实际时长、负载和资源阈值，记录应用装卸、REFUSE、慢设备/取消交互；不宣传无限稳定 |

## 3. 失败材料

保存匹配源码/程序SHA、操作顺序、run/node/operation/question/call、目标/epoch/revision、JCS参数/descriptor指纹、预算、事务/undo/退出证明、原错误与资源采样。正常关闭后保存一致库副本/必要WAL/SHM/备份；故障现场先保留原文件，不重建/降级、自动重试修改、强杀worker或覆盖恢复。不使用测试钩子伪造生产人工来源。

## 4. P6与browser后续集中人工步骤（未执行）

1. 对照delivery-identity核对正式Light目录八项SHA。按上面的新隔离库启动正式宿主，確認stub标识，展示路径、schema与备份后分别确认1→2、2→3；不选择真实用户库。
2. 打开web_counter及其余四维护包。真实菜单/按钮/工具栏/快捷键/表单/对话框/画布按原P2、P4、P5延期清单逐项操作；具体模型值、对象ID、name/独立草稿、revision、事务/undo回执对照原业务读能力，不靠控件或计数变化判断成功。
3. 主对话提交明确stub read/有限图，确认前切标签、收起/重建面板，原任务及目标不变。逐项检查范围/风险批准、问题ID、过期、取消、busy/modal、REFUSE和关闭；UNKNOWN不能由取消或重提交清门，退出/业务回读证明后显式核对。
4. 打开browser.uapp只使用browser:start/help内建页验证本轮新增read和10个view：地址/查找焦点、查找提交、缩放100→110→100、settings/diagnostics与关闭；对照browser.snapshot的address/zoom/panel/find_text/revision。生产真实输入链仍需独立确认；程序消息和可识别注入应拒绝非read，不以私有测试钩子代替。
5. browser导航/停止/home/help网络副作用及settings-save/reset共9项完整能力迁移另案评审；本次人工清单不授权新网络、文件执行器或副作用迁移，也不把旧人工可用当迁移已完成。
6. Windows分类真实鼠标/键盘正例、可识别注入拒绝及UIAccess限制分别记录。IME/DPI/屏幕阅读器/视觉与人工长时逐项按本清单第2节；真实物理断电只在未来专机方案明确授权后开展。当前均延期未验。

失败时保留匹配SHA、步骤、实际来源链、run/node/call/operation、目标/epoch/revision/指纹、原错误、资源CSV及正常关闭后的完整一致库/必要WAL/备份。不删除旧失败，不自动恢复业务执行，不把私有接纳正例写成真实设备验证。
