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
| 快速1 | [日志](api7-runtime-close-gate64.log) | 383→386 | PASS |
| 快速2 | [日志](api7-runtime-fast-2.log) | 385→387 | PASS |
| 快速3 | [日志](api7-runtime-fast-3.log) | 385→387 | PASS |
| 呈现1 | [日志](api7-runtime-ready64.log) | 384→386 | PASS |
| 呈现2 | [日志](api7-runtime-ready-2.log) | 385→386 | PASS |
| 呈现3 | [日志](api7-runtime-ready-3.log) | 385→387 | PASS |

GDI均12→12、USER17→17；部分周期因共享浏览器仍活动而有多个pending backend，末次均归零。[精确复验二进制](api7-runtime-repeat-binaries.json)。历史菜单阶段374→416原失败仍保留，单次通过从未改写为已修复。本记录只证明所列真实自动用例与范围，不替代长期人工压力、IME或桌面合成；最终阶段完整回归另行记录。

PSS观察依据[Microsoft PSS_HANDLE_ENTRY文档](https://learn.microsoft.com/en-us/windows/win32/api/processsnapshot/ns-processsnapshot-pss_handle_entry)；异步环境/浏览器退出生命周期依据[Microsoft Controller合同](https://learn.microsoft.com/en-us/microsoft-edge/webview2/reference/win32/icorewebview2controller)。诊断快照、walk marker和临时进程查询句柄在资源断言前全部释放。

