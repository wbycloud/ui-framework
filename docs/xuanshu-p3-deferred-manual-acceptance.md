# P3延期集中人工验收

状态：延期未验。本文只准备步骤，不表示已经启动真实人工验收。读者为框架维护者和验收人员，无署名。延续[P2延期清单](xuanshu-p2-deferred-manual-acceptance.md)，软件整体完成并另获授权后集中执行；当前P3自动及程序UI证据不替代本表。

## 匹配产物和隔离启动

本轮证据根`build/xuanshu-p3-20261024-f19c6a`。用其`binary-files.json`核对Light宿主、ui_framework.dll、web_counter.uapp和其他必要包；`delivery-files.json`核对设计/源码，`source-after.json`核对完整编译输入。以后源码变化必须重建匹配包，不能拿本轮二进制验收未来版本。

未来获授权后，在框架根目录运行：

```powershell
$env:UI_AI_DB_PATH = Join-Path (Get-Location) 'build/xuanshu-p3-20261024-f19c6a/manual/conversation.sqlite'
& 'build/xuanshu-p3-20261024-f19c6a/light/framework_host.exe' 'build/xuanshu-p3-20261024-f19c6a/light/web_counter.uapp'
```

该专用新库与用户默认库隔离；不要复制用户数据库或导入XuanShu数据库。数据库父目录由后台创建。关闭后清除此进程环境变量；不要终止其他进程或删除仍在使用的库、WAL、SHM。出现错误先保留诊断，不能重建原库冒充恢复。

## 延期项目

| 项目/原因 | 当前替代证据及边界 | 未来操作、预期和回读 |
| --- | --- | --- |
| 鼠标/键盘生产链正例，需要真实设备 | ai_panel是明确模拟的Light控件/JS桥接、同一Service和真实counter.read；不是硬件来源证明 | 打开AI面板输入中文并提交，输入只在保存确认后清空；收起再展开，任务/回复/主对话ID保留。键盘焦点不能误触应用快捷键 |
| 固定目标与应用切换 | 专用真实双实例workspace、SQLite选中target和实际count/revision自动回读；没有替代物理点击 | 打开两个counter，用真实应用按钮使值不同。在目标应用提交下方脚本及counter.document，立刻换标签/收面板；回复仍为提交时实例的值/revision，不自动换成活动实例 |
| stub工具与非法参数的可理解展示 | 只读过滤、碎片聚合、修改能力拒绝、revision冲突、重复tool ID已有自动证据 | 明确输入测试脚本，确认界面一直标注Stub测试模型。将counter.read换成counter.increment：应失败且模型不变；非法脚本保留输入，不静默重试 |
| 队列/问题/取消交互 | 有界16排队、控制预留、问题ID、取消停滞和FIFO自动测试 | 用ask脚本占住活动任务，再提交独立任务。回复问题只作用于关联run/question；取消不受普通排队阻塞。任务结束后才显示明确终态，不能凭按钮点下即假报取消完成 |
| 重启历史、显式继续 | 临时库专用子进程退出及另一进程读取；未完成记录NEEDS_REVIEW且无自动执行 | 关闭/重开同一隔离库；确认历史仍在，未完成记录待核对/继续。选择记录，重新选择当前应用并bind当前资源，明确继续生成关联新run；旧epoch不重用，不自动重放工具 |
| 退出REFUSE、错误可见性和关闭体验 | 真实fixture包REFUSE、恢复接纳但旧任务取消、DLL worker/外部回调退出门；真实视觉/用户判断仍未验 | 对明确可拒绝关闭的应用请求退出；拒绝后可提交新任务，已取消任务不复活。存储错误首次退出显示错误，不能静默清历史；明确再次关闭才完成退出。UI保持可交互，不强杀worker |
| Windows来源分类动态矩阵及限制 | P3不更改P2来源政策；聊天/只读工具不产生非read凭据 | 按P2 D03执行并记录采样位置/origin/device及业务结果。普通注入只能作为拒绝负例；HARDWARE分类不是物理设备证明，不承诺排除UIAccess或恶意同进程DLL |
| IME、跨屏DPI、视觉、可访问性和长时 | UTF-8分片、字节限额、程序布局/业务断言不是实际IME或视觉判断 | 中文IME输入、组合取消/确认、粘贴及emoji；不同缩放屏幕移动，核对输入焦点/历史滚动/任务按钮/错误。长时项目另定时长与资源门槛，不能把短时自动测试当长时通过 |

显式工具脚本（target:0对应本次提交捕获的counter.document）：

```json
{"steps":[{"type":"discover","category":"document","target":0},{"type":"tool","tool_call_id":"read-counter","capability_id":"counter.read","target":0,"arguments":{}}]}
```

问题脚本：`{"steps":[{"type":"ask","text":"请补充测试信息"},{"type":"text","text":" · 已处理"}]}`。这些是stub脚本，无真实自然语言理解、审批或写入授权含义。

失败时记录构建清单哈希、client/run/tool/call ID、精确操作、错误码/phase/revision、时间和退出状态，保存对应原始日志；库已安全关闭后复制该隔离DB及仍存在的WAL/SHM。仍在运行时不要仅复制主DB并称其为一致快照。仅在用户授权下采集必要画面，不能删首失败或用后续通过覆盖。
