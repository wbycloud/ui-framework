# P4延期集中人工验收

状态：延期未验。软件整体开发完成并另获授权后再执行；本轮没有启动物理验收。延续原P2/P3清单，不删除原要求，不署名。

## 匹配产物与隔离启动

证据根`build/xuanshu-p4-final-20261026-c84f12`。先核对`delivery-identity.json`、`binary-files.json`及`source-after.json`。正式生产包位于`light`根：framework_host.exe、ui_framework.dll及五个原名称.uapp。`p4-*.uapp`和ui_framework_edit_test.dll是隔离测试产物，**不能作为物理授权验收运行库**。

未来获授权后，从仓库根启动专用新库，不能操作用户库或历史证据DB：

```powershell
$env:UI_AI_DB_PATH = Join-Path (Get-Location) 'build/xuanshu-p4-final-20261026-c84f12/manual-future/conversation.sqlite'
& 'build/xuanshu-p4-final-20261026-c84f12/light/framework_host.exe' 'build/xuanshu-p4-final-20261026-c84f12/light/web_counter.uapp'
```

当前仅提供命令，未执行。父目录由后台创建。退出后清除此进程环境变量。需要schema1→2时确认显示的实际路径、schema、影响、备份位置；**本轮从未授权迁移真实用户库**。不得终止其他进程或删除仍在用的WAL/SHM。

## 延期项目、替代证据、风险与步骤

| 项目及延期原因 | 当前替代证据/剩余边界 | 将来操作与预期 |
|---|---|---|
| 真实鼠标/键盘生产来源正例 | 私有隔离接纳后业务正例；正式页消息和程序输入均拒绝批准。未证明物理设备 | 用真实鼠标确认迁移，再建counter.increment/counter.document/{}提案，严格空object约束。确认范围只接受一次；模型0→1、revision递增、有事务和undo回执；点击新的撤销恢复0且revision继续递增 |
| Windows来源分类动态矩阵 | 现有guard/匹配动作负例与源码；HARDWARE不等于物理设备证明，不保证排除UIAccess注入或恶意DLL | 分别真实鼠标、键盘、可识别注入、程序消息；保存分类/事件关联。非真实来源不得授予资格；分类限制单列 |
| 标签切换、收面板、重建、modal与关闭 | 真实双实例workspace+固定目标+复制绑定+DLL生命周期自动断言；真实交互未验 | 开两counter，在A提案后切B/收面板再确认，只有A变化；应用modal/disabled/busy拒绝写入。关闭中状态可观察，不强杀worker |
| stateful属性与单元格、共享name的独立草稿 | 实际模型/字段代次/冲突/rollback/undo自动证据；IME和视觉未验 | 原单元格人工55、AI66、显式undo55；FORM与DIALOG各留草稿，AI改共享name后两者保留并标冲突，显式Keep/Reload后才提交。UST1/UIL1不保存新业务文档 |
| EDA画布、clear审批、笔记 | 稳定ID/固定矩形/单调revision/独立草稿/取消/undo自动；真实画布手感未验 | 人工加矩形，AI加固定矩形后undo；clear先范围确认，再逐项批准，拒绝不删对象。undo恢复对象，不复原旧缩放。笔记草稿先不改业务；AI笔记提交保留草稿并标冲突 |
| 授权30分钟/批准5分钟的交互可理解性 | 受控时钟边界、条件变化、重复消费和意图存储自动；实际等待/提示未验 | 展示范围/逐项风险/期限，过期明确失败并要求新提案；打开对话框/排队不续期；已开始调用到期不伪称取消或回滚 |
| UNKNOWN人工核对与历史说明 | 存储故障注入、真实业务值、worker退出证明、唯一终态/新只读核对自动 | 在隔离故障构建按步骤使UNKNOWN，取消/forget/重提交不得解门；新的只读核对提供实际值/revision/回执和退出证明才解除。重启只显示历史，不恢复授权或重放旧epoch |
| IME/跨屏DPI/焦点和可访问性 | 程序控件收参/复制投影/分页，不替代输入法与辅助技术 | 中日文组合输入、键盘焦点/屏幕阅读器、不同DPI屏间移动。范围/结果长文本通过前后段完整可读，按钮不截断、不误触 |
| 视觉、长时和资源稳定性 | 本轮有界自动测试/ASAN，未跑长时/benchmark | 整体完成后按明确时长和资源指标验收；先获授权，不将瞬时通过当长期结论 |

## 失败最小材料

保存准确操作顺序、目标实例/资源/open epoch/revision、能力ID和规范化参数、operation/call/事务/undo回执、页面状态及错误、DB/备份/WAL/SHM一致副本、时间和匹配二进制SHA-256。仅在授权的新隔离目录保存截图；失败时先正常关闭/核对，不重建库冒充恢复、不自动重试写入。不触碰历史证据或参考工程。
