# P2 集中人工验收清单

2026-10-24当前口径：用户已将真实设备及必须人工判断的项目延期到软件整体开发完成后集中验收，**现在不执行本页物理操作**。原清单和启动步骤完整保留；每行A/B可自动/模拟部分的实际证据见[开发阶段验收](validation/xuanshu-p2-simulated-acceptance.md)，C剩余部分及未来步骤见[延期清单](xuanshu-p2-deferred-manual-acceptance.md)。下文“待验/由验收人开始”是原物理验收要求，不表示要求当前开始。开发阶段模拟通过不等于物理或P2完整验收。

状态：**准备完成，全部人工项目待验**。本文不代表P2完整验收通过。操作对象为本轮匹配开发快照，不是冻结SDK或已发布版本。

## 1. 身份与启动

证据根：`build/xuanshu-p2-remaining-20261023-b84e6f`。`manual/matched-binaries.json`逐项锁定实际交付二进制，`manual/provider-identity.json`锁定现有本地OSMesa路径及哈希；`delivery-files.json`与`source-after.json`记录源码身份。匹配SDK为0.10-dev/API10/标准10、ABI1/包1。不可混用其他目录的DLL。

在仓库根的PowerShell执行（**由验收人开始，本轮未执行**）：

```powershell
& .\build\xuanshu-p2-remaining-20261023-b84e6f\Start-P2Manual.ps1 -Chrome native
```

先完成native宿主；退出后用`-Chrome web`验Web宿主chrome。后者使用Light，不是WebView2。脚本先核验二进制和固定provider，创建独立`manual/runs/<时间-GUID>/profile`，再启动六个实例：stateful A、EDA、generic、features、counter、stateful B。记录控制台OPEN实例ID和状态，不能把相同应用的A/B当同一实例。非OK装载、缺失GL或provider须先记录失败，不继续假设正常初态。已存在的用户偏好文件不受影响。

native验收驱动仅用公开API，不含私有人工授权钩子。F6读取活动实例真实业务和视图资源；Ctrl+Tab换实例；Ctrl+F4关闭；F9显式选择匹配包重新打开。F7普通invoke自称human、F8程序WM_COMMAND是**负例**。F11取消、F12 forget均指向最后接纳调用的原实例，不重定向到活动标签。Web宿主没有这些驱动快捷键，使用自身标签/关闭和界面信息；深入回读在native驱动进行。

控制台记录`p2.capability`调用、结果及SOURCE。`GetCurrentInputMessageSource`仅表示采样点Windows分类，可能在回调阶段已经不可用，不是物理来源证明；真实事件授权仍由框架私有匹配链决定。不要用屏幕变化替代F6业务回读。

## 2. 全部待验操作

每行记录时间、实例ID、输入、F6前后结果、实际状态及通过/失败。空白均表示未验。

| 项目 | 物理操作与预期 | 回读/判据 |
| --- | --- | --- |
| 原生HMENU | 鼠标打开菜单选叶项；键盘Alt/方向/Enter及助记键选择。取消菜单不执行；拖出释放不执行；disabled叶项不执行 | 正确binding仅接纳一次，菜单退出后内容窗口和原生EDIT仍有效；F6模型变化符合该能力 |
| Light菜单/按钮/工具栏 | 逐项点击同一功能的菜单和按钮；features七组菜单及13个工具栏别名分别验，含溢出项 | 同一能力ID/业务结果；不出现旧handler再执行一次；具体清单见覆盖矩阵 |
| 程序旁路负例 | counter先F6，F7、F8，再F6；其他应用对修改命令同样做F7 | PERMISSION_DENIED/拒绝，业务revision及值不变；普通read仍可读 |
| 物理与注入区分 | 物理点击counter +1，恰好加一；再按下文负例脚本步骤执行 | 注入不增加count/revision；不得把注入结果当物理正例 |
| 固定实例 | stateful A打开对话框，切B修改其name，再回A提交；记录A/B实例ID和各自F6 | 不因活动标签切换而把A操作写到B；A/B内存状态独立 |
| 关闭/重开 | 关闭A，F9显式重开匹配stateful包；操作旧调用控制键 | 旧目标/epoch/消息不得命中新实例；不自动改绑；新内存业务初态，原偏好格式可恢复 |
| stateful单元格 | F2编辑文本和数值格，Enter提交，Esc丢草稿；输入非法数值 | 合法值从state.cell.read/source回读，文档revision变化；非法值零业务修改；草稿/编辑计数不算成功 |
| stateful选择/排序 | 点击、Ctrl增减、Shift范围、Ctrl+Shift、箭头/Tab/Home/End；表头正反排序 | state.view.read列出实际选择/anchor/sort；保留原100000行/64列范围，不要求新增任意值排序 |
| 共享name冲突 | 表单输入A但不提交；对话框输入B并提交；观察表单 | 业务name=B，表单仍保留A且冲突；直接提交旧base失败；Reload丢A采用B，或Keep后明确再提交A；两处不共享草稿 |
| 对话框modal | 对话框打开时试外部菜单/快捷键；在所属对话框提交/取消 | 外部不绕modal；所属入口合法；关闭后窗口仍可交互。名字提交是业务修改，tab title仅view |
| stateful偏好 | 拖列/改变布局后Save；再改宽度后Load；单列/全部Reset；切语言、主题、draft title | 原UST1/UIL1、A/B隔离保持；宽度/布局/语言实际恢复；不声称保存name或单元格业务文档 |
| EDA实际操作 | 菜单/工具栏/画布A加固定矩形，Z放大，C清空；鼠标点现有对象 | eda.document.read实际ID/几何；ID清空后不复用；view读zoom/selection。仅固定加矩形，不验不存在的任意编辑 |
| EDA笔记 | 原生EDIT输入，F6看已提交note未变；Submit后变化；Cancel/reload丢草稿 | 真实note及revision回读；输入本身不是提交 |
| EDA冲突 | 笔记输入未提交时加矩形，再Submit；Keep draft再Submit，另验Cancel | 旧base失败且保留文本；显式解决后才提交，文档/对象结果均可回读 |
| generic和features | 各验文本格、布尔格、树F2重命名、树展开、右键菜单、表单/对话框提交及冲突 | 实际source/model中值/树名/属性一致；分页仍100000×16，树100×1000；size非法/空必填/超长值不改模型 |
| 快捷键/画布 | generic Ctrl+D、features Ctrl+O；EDA画布焦点A/Z/C；焦点切到编辑框再输入 | 仅匹配入口执行；编辑文本不误触画布命令；程序调用不获得资格 |
| 关闭诊断 | features Close test先Refuse关闭、再Allow关闭 | 原拒绝/允许功能保留；worker未退出时UI继续poll，不提前卸载、不join等待 |
| 异步/busy/取消 | stateful开始Save或语言保存后立即再提交，F11取消；切实例再F11/F12 | 若捕获在途窗口，busy明确拒绝且无第二worker；取消不等于回滚。过快未捕获应记未验，不能虚报通过；自动真实DLL时序证据独立 |
| UNKNOWN | 按下一节受控故障步骤 | 后续写入仍阻止；read可用；forget/取消不解除，显式核对无法证明时仍阻止 |
| resize/DPI/IME及退出 | 中英文IME、跨屏DPI、反复开关modal及应用，最后关宿主 | 无悬空UI/迟到写入；这些物理/长时项目自动回归不能替代；未完成如实记录 |

## 3. 受控UNKNOWN与来源负例

UNKNOWN只在独立profile验。先记录本次脚本输出profile目录，确认`profile-A.ust.tmp`**不存在**后，手动创建该名称的空目录。stateful A执行Save：后台临时文件创建失败，结果保守记UNKNOWN。只删除此次新建的空目录（不递归删用户文件），再Save、F11、F12，必须仍阻止重复写入，F6只读可用。选择新出现的Verify核对入口，应用没有足够证据时仍UNKNOWN，不能把它改写为“未修改”。本轮自动回归已覆盖该路径，但人工栏仍待验。

可选的**注入负例**：先物理激活counter并将指针放到+1按钮，记录count。另开PowerShell执行：

```powershell
$manualProcess = Get-Process ui_p2_manual_host
& .\build\xuanshu-p2-remaining-20261023-b84e6f\Invoke-P2InjectedNegative.ps1 -TargetProcessId $manualProcess.Id
```

3秒内回到native验收驱动。脚本仅在正确PID/窗口类、前台及指针命中该窗口时投递一次SendInput左键down/up；不移动鼠标。不满足条件会拒绝。再物理F6核对业务未变。**本轮只校验脚本语法及40字节输入结构，未运行注入。**不排除所有UIAccess等被Windows分类为HARDWARE的输入；也不隔离恶意同进程DLL。更强物理来源证明另案评审。

## 4. 失败材料与结束

保留最小材料：本次console.txt、匹配清单、实例/call/binding ID、F6前后JSON、操作顺序和时间、必要截图及独立profile的UST1/UIL1相关文件。崩溃另保留dump和对应二进制；不得删除首次失败后只留重跑成功。不要提供凭据/无关用户文件。

业务结果未知或关闭未完成时，停止该实例后续写入；保留现场，UI继续响应，不强杀worker或卸载DLL。关闭成功后才整理本次无用临时产物。集中验收结果另开记录；只有对应人工项目完成且未发现阻断问题后，才可评估P2完整验收。P3没有在本文获得实现授权。
