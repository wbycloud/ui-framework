# P5延期集中人工验收

状态：延期未验。2026-10-27。真实设备及人工判断在软件整体完成并另获授权后集中执行；本轮只自动/模拟验证。保留P2/P3/P4原清单，不署名。

## 匹配构建和未来启动

先核对build/xuanshu-p5-20261026-e63b8a中的delivery-identity.json、source-after.json、binary-files.json。正式Light目录中的framework_host.exe、ui_framework.dll及五个原名称.uapp是待人工产物；ui_framework_edit_test.dll和p4-*.uapp仅隔离测试，不用于生产输入正例。

未来获授权后从仓库根设置全新隔离路径并启动，当前没有执行：

```powershell
$env:UI_AI_DB_PATH = Join-Path (Get-Location) 'build/xuanshu-p5-20261026-e63b8a/manual-future/conversation.sqlite'
& 'build/xuanshu-p5-20261026-e63b8a/light/framework_host.exe' 'build/xuanshu-p5-20261026-e63b8a/light/web_counter.uapp'
```

新库先走P4 schema1→2确认，再按Coordinator显示的路径、schema2→3、备份位置和影响确认。不得用真实用户库或替换原库冒充测试。程序正常关闭后清除此进程环境变量，不删除在用WAL/SHM。

## 原人工要求的延期与替代证据

| 延期项目/原因 | 当前替代证据、剩余风险 | 将来操作与预期 |
| --- | --- | --- |
| 真实鼠标、键盘/正式来源正例 | 私有隔离接纳后的业务链、程序页消息拒绝、原C2来源负例；不证明物理设备 | 打开全局面板，目标每行实例ID:资源ID，提交明确stub图；真实点击范围确认。计数0→1、revision增加且只执行一次。程序消息/注入不能批准 |
| Windows分类动态矩阵 | 原HARDWARE/可识别注入/系统未知分类边界及私有链源码；不保证排除UIAccess或恶意DLL | 分别真实鼠标/键盘、可识别注入、程序调用，保存分类和具体事件。Windows分类不等同物理证明，不扩合作故障边界 |
| 多应用与主对话体验 | 五真实包/真实workspace切标签自动断言，不是交互体验判断 | 在A固定目标提案后切B、收面板/重建；对话及任务仍存在，只有A修改。看清父/依赖/等待、预算及部分失败，成功结果不被失败覆盖 |
| 问题、范围、逐项审批 | 明确root/node/question/operation关联和重放/过期负例；无真实用户可理解性判断 | 复制正确问题ID回答；错误ID/取消后迟到答案拒绝。修改先展示有限图与完整目标/参数，30分钟不续期；破坏性步骤另5分钟，拒绝不执行 |
| 最后一次范围投影/分页 | 生产Light同步post成功后才重新绑定控制；未投影时解绑的受控测试；完整长文本分页 | 在范围快速切换、长参数/长结果时观察旧按钮失效，新完整范围显示后才可接受。未显示/投影失败不得执行，不凭控件变化判业务成功 |
| 取消、UNKNOWN、关闭/REFUSE | 真实DLL worker/callback、写前意图/提交/回读、排队取消停滞与安全卸载自动证据 | 取消只请求停止；已提交仍显示实际修改，UNKNOWN保留资源阻止。显式只读核对需退出与业务/revision/回执证明。退出遇REFUSE恢复接纳，不复活旧任务 |
| 重启与存储迁移说明 | 隔离子进程/临时库、备份失败/事务rollback和旧epoch查身份自动证据 | 只恢复记录；未完成显示待核对，新授权和新目标才继续。不得自动重放已charged步骤，损坏/未来schema不重建/覆盖 |
| IME、跨屏DPI、视觉、辅助技术 | 模型/接口和部分真实DOM读回，不覆盖物理输入/可访问性 | 中日文组合输入、焦点/屏幕阅读器、不同DPI屏间移动；范围、问题和长结果完整可读、分页不误触 |
| 长时间和资源稳定性 | 有界自动/ASAN通过不等于长期稳定；旧ASAN-Light脚本中断仍未定位 | 整体完成后另定时长/指标并明确授权；本轮不执行benchmark或P6长期工程 |

## 失败保留与限制

保存最小操作顺序、run/node/parent/question/operation/call ID、目标/epoch/revision、参数/descriptor/hash、事务及undo回执、预算reserve/charge/settle、最后投影/错误和匹配EXE/DLL/uapp SHA。数据库只在正常关闭后保留一致副本及必要WAL/SHM/备份。不要自动重建、重试写入、强杀worker或删除历史证据。真实模型质量不在stub验收口径中。
