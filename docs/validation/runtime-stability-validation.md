# WebView2 重开稳定性修复

接手基线658b497，Windows x64/C11，SDK1.0.4129.50，实际Runtime154.0.4258.53、Mesa24.3.4。运行既有公共API6独立应用DLL，原包SHA256 `b493adda01f6dc5d8562a52e3648b2ce8b4e233a07d9c8def28646241cf084e4`。没有修改应用包，原句柄+12及GDI/USER+4、待清理窗口归零断言保留。

## 复现与定位

[原64周期](api7-runtime-baseline64.log)句柄384→491；[仅首尾快照对照](api7-runtime-baseline64-minimal.log)384→453。框架待清理窗口为0、GDI/USER不增长；PSS记录增量主要是已退出Runtime进程的宿主进程句柄。未把消失的browser进程与宿主句柄回收混为一项。

创建后但首导航未完成的关闭分支此前直接Close；补齐内部导航后，[64轮仍失败](api7-runtime-drain64.log)，384→407；[不使用PSS的对照](api7-runtime-drain64-control.log)384→405，排除快照工具解释全部增量。随后发现匹配ID的完成可能取消；特定文档确认与观察器重试修复保留[导航诊断](api7-runtime-navigation64.log)、[文档屏障失败](api7-runtime-document64.log)和[重试失败](api7-runtime-document-retry64.log)。这些中间修复不单独宣称解决资源问题。

关键关闭门控缺口：应用request_close返回WAIT后正常STA消息循环仍启动新的环境/controller工作。现在把创建排入下一STA周期，并在开始时核对实例dispatch_blocked；取消前尚未开始的请求不启动Runtime。WAIT被拒绝后，下一有效view操作重新排队创建。已经启动的操作继续持有失效内部view、确认真实内部文档、退出回调栈后Close，浏览器退出后释放环境；不回调已卸载应用。没有人为等待或放宽预算。

## 重复运行

每次为新的宿主进程。快速关闭每轮运行实际应用GL与正常WAIT关闭；呈现关闭另外要求Runtime表单字段真实呈现后再关闭。两种模式各3次，每次64连续周期，共384次重开，均保持原断言。应用DLL每轮卸载，末次框架pending cleanup=0。

| 模式 | 证据 | 句柄基线→最终 | 结果 |
| --- | --- | --- | --- |
| 快速1 | [日志](api7-runtime-close-gate64.log) | 384→386 | PASS |
| 快速2 | [日志](api7-runtime-fast-2.log) | 385→387 | PASS |
| 快速3 | [日志](api7-runtime-fast-3.log) | 385→387 | PASS |
| 呈现1 | [日志](api7-runtime-ready64.log) | 384→386 | PASS |
| 呈现2 | [日志](api7-runtime-ready-2.log) | 385→386 | PASS |
| 呈现3 | [日志](api7-runtime-ready-3.log) | 385→387 | PASS |

GDI均12→12、USER17→17；部分周期因共享浏览器仍活动而有多个pending backend，末次均归零。[精确复验二进制](api7-runtime-repeat-binaries.json)。历史菜单阶段374→416原失败仍保留，单次通过从未改写为已修复。本记录只证明所列真实自动用例与范围，不替代长期人工压力、IME或桌面合成；最终阶段完整回归另行记录。

PSS观察依据[Microsoft PSS_HANDLE_ENTRY文档](https://learn.microsoft.com/en-us/windows/win32/api/processsnapshot/ns-processsnapshot-pss_handle_entry)；异步环境/浏览器退出生命周期依据[Microsoft Controller合同](https://learn.microsoft.com/en-us/microsoft-edge/webview2/reference/win32/icorewebview2controller)。诊断快照、walk marker和临时进程查询句柄在资源断言前全部释放。

## API7最终源码复验与API5基线

最终产品21e94e5a5f99964fde0de2ab4a32fa5c65858b82：[快速64](api7-final-fast64.log)397→399，[实际呈现后关闭64](api7-final-ready64.log)397→398，均GDI12→12、USER17→17、pending cleanup0、逐轮卸载DLL，原阈值保留。共享Runtime最后退出前可出现临时pending峰值；它与未归还的宿主进程句柄分别记录，最终必须归零。

[API7首轮完整失败](api7-full-first-output.log)保留API5资源425→449、GDI9→9、USER15→17。旧基线未测pending，不能据此断言这次精确来源已全部定位。API5测试现在在基线/末次均等待框架pending0，至多15秒；原32轮、每轮2秒、句柄+12及GDI/USER+4不改，未把等待中的对象当作回收完成。

[独立PSS诊断](api7-api5-diagnose.log)444→443、pending0；后续独立矩阵built/original重复通过，最终二者均399→398、GDI9→9、USER17→17、pending0，见[完整输出](api7-full-final-output.log)。PSS只作可选资源类型诊断，所有快照句柄释放后测断言。不用随后单次通过改写首轮失败，最终本机61项60通过/1物理跳过与原生21通过/1跳过见[API7总记录](api7-validation.md)。当前源码Session0服务权限不足，历史Session0与无登录另记。

## 后续失败与异步模态焦点复现

1262cc0 完整回归再现 API5 原包资源超限：358→395、GDI9→9、USER15→17、pending0，最终等待15秒仍未归还；[失败完整输出](api7-latest-full-failure-output.log)和[JUnit](api7-latest-full-failure.xml)保留，不能用此前通过声称这一情况已修复。三次独立原包诊断中，两次通过，第三次354→378、USER15→17失败；[窗口/模块诊断](api7-input-adapter-reproduction.log)显示新增 SystemUserAdapterWindowClass、CoreMessaging/CoreUIComponents。MSCTFIME UI 和 IME 窗口此前已经存在，因此不能笼统归因为首次 IME 创建。

随后增加异步模态原生焦点断言，[修复前](api7-modal-focus-red.log)容器与实际focus相同、descendant=0。现有WM_SETFOCUS发生在controller创建之前被略过，文档就绪后未转交；这是可稳定复现的独立缺口。修复只在同一容器仍持有焦点、活动且输入未被门控时 MoveFocus，不增加等待/预热、放宽资源阈值或初始化全局输入服务。资源波动与这项缺口的关系以随后独立/完整运行及可选创建调用栈为准；该类窗口/模块出现本身不能证明所有泄漏来源已定位。

2c0c52b之后，原API5三次独立32轮均390→387、GDI9/USER17平稳、pending0；当前完整矩阵built/original各32均394→391。API7两个独立64周期分别390→390、391→392，原阈值及DLL卸载通过，详见[最新产品复验](api7-validation.md)。这证明所列场景的回收，没有把历史所有句柄差值都归为同一种原因。

[实际创建调用栈](api7-input-adapter-creation.log)记录textinputframework/MSCTF→CoreMessaging→USER32，SystemUserAdapter属于Windows输入基础设施；它在该次完整初始场景内已创建。诊断CBT hook仅在UI_RUNTIME_INVENTORY下安装，USER多1前后均计入，结束释放；正常重复运行没有该开关。首帧中文路径受wide fprintf编码影响不完整，其余Windows DLL帧可识别；不借此断言具体框架调用点或解释全部内核句柄差额。PSS只在可选UI_RUNTIME_RESOURCES下运行，UI_RUNTIME_ONLY_SETTLED可避开基线快照影响。

原生输入测试显式等待实际呈现完成后记录焦点，safe title点击后仅重试标准SetForegroundWindow；微软仍允许拒绝前台请求，见[合同](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setforegroundwindow)。没有更改输入设置、桌面、所有者断言或对其他应用发键。共享桌面的额外VK_PACKET/前台丢失失败均保留，最新三个独立通过不称永久消除竞态。下一次资源超限继续保存pending、窗口类/PID和PSS类型，不能以一次通过、未知类型或缓存假说宣布根治。

最终冻结cb42660完整61项60PASS/1物理SKIP、327.82秒，本次原生输入通过；另10个独立原生输入进程60.74秒全通过。API5 built/original各360→357、GDI9/USER17不增长、pending0，原32周期/+12/+4不变；[冻结原输出](api7-frozen-full-output.log)与[完整身份](api7-frozen-binaries.json)。之前全部失败仍保留，当前目标Session0/无登录及长期人工待验。

## 2026-10-06交付收敛复验

执行代码e4a33bf（本次仅CI/测试变化，产品/公共头文件相对68c4e9c未改）：两个独立API7 Runtime进程各64周期，快速及实际呈现后关闭均406→406、GDI12/USER17不增长、末次pending0、逐轮DLL卸载；呈现关闭暂存pending64/handles峰值564，实际排空后仍满足原+12/+4。四行Runtime API5内置32周期384→380/GDI9/USER15平稳，原API5包另32周期实际通过；原生真实输入6.19秒通过。历史未完全归因差值与共享桌面失败继续保留，不宣称永久根治；源码/产物/完整证据见[收敛验收](api7-delivery-validation.md)。

另外，软件WGL部署会影响独立OSMesa资源对照；原341→346/USER99→103失败及三次同二进制移除本行WGL部署后的237→238/USER2对照均保留，CI现隔离WGL/provider/Runtime。该环境修正没有修改关闭产品代码，也不解释历史全部WebView2波动。
