# 完整窗口视觉实测证据（2026-10-07）

产品基线43587c3；本轮产品提交95f5d78、6a0170a，SDK0.7.0-dev/API7/标准7、ABI1。文档提交另以Git历史为准。本地正常Session1输入桌面，未推送或触发托管CI。

- [验收与失败解释](../visual-polish-validation.md)
- [准确产物、依赖及头文件哈希](artifact-identity.json)
- [原始运行源码哈希](final-source-before-commit.json)与[执行命令清单](current-plan.json)
- [原字节ZIP](raw-evidence.zip)与[每文件SHA256/CRC索引](index.json)
- [48张截图原BMP/PNG哈希与尺寸](screenshots.json)
- [环境及待验条件](environment-final.json)
- [最终链接、头文件、产物与原字节复核](verification.json)
- [业务只读哈希before](business-readonly-hashes-before.json)及[after](business-readonly-hashes-after.json)

原ZIP的文件数、字节数及SHA256见index.json，CRC与全部文件SHA256验证通过。每对PNG同应用/数据/外窗尺寸/程序DPI/主题，无损转换实际BMP，不生成或替换图像。

| 内容 | 修改前 | 修改后 |
| --- | --- |
| API7完整DLL：Light/实际OSMesa/树/表格/属性 | [浅色](screenshots/before-light-host-light.png) · [深色](screenshots/before-light-host-dark.png) | [浅色](screenshots/after-light-host-light.png) · [深色](screenshots/after-light-host-dark.png) |
| 实际WebView2 Runtime同DLL | [浅色](screenshots/before-runtime-host-light.png) · [深色](screenshots/before-runtime-host-dark.png) | [浅色](screenshots/after-runtime-host-light.png) · [深色](screenshots/after-runtime-host-dark.png) |
| 只读当前KLayout C/实际原生GL | [浅色](screenshots/before-business-light.png) · [深色](screenshots/before-business-dark.png) | [浅色](screenshots/after-business-light.png) · [深色](screenshots/after-business-dark.png) |
| 框架完整features DLL/实际WGL/工具/工作区 | [浅色](screenshots/before-features-wgl-light.png) · [深色](screenshots/before-features-wgl-dark.png) | [浅色](screenshots/after-features-wgl-light.png) · [深色](screenshots/after-features-wgl-dark.png) |
| AI收起后的主动展开 | [Light](screenshots/before-light-assistant.png) · [业务](screenshots/before-business-ai.png) | [Light](screenshots/after-light-assistant.png) · [业务](screenshots/after-business-ai.png) |
| 窄窗 | [Light](screenshots/before-light-narrow.png) · [Runtime](screenshots/before-runtime-narrow.png) | [Light](screenshots/after-light-narrow.png) · [Runtime](screenshots/after-runtime-narrow.png) |
| 颜色/透明度与枚举 | [颜色](screenshots/before-light-color-picker.png) · [枚举](screenshots/before-light-enum.png) | [颜色](screenshots/after-light-color-picker.png) · [枚举](screenshots/after-light-enum.png) |
| 144/192程序DPI | [144](screenshots/before-light-host-dpi-144.png) · [192](screenshots/before-light-host-dpi-192.png) | [144](screenshots/after-light-host-dpi-144.png) · [192](screenshots/after-light-host-dpi-192.png) |

## 原字节与复验

logs-tools包含本轮构建cmd、配置/编译、JUnit、原失败/最终结果和自有观察器C/inc。run-config保存精确CTest命令、CMake参数、LastTest及业务原锁；raw-captures保存原BMP；diagnostic-only保留错误路径/工作目录/无实际GL等诊断，不纳入最终截图图册。before/webview2-*为沙箱Runtime未就绪，app-business-*为PrintWindow缺GL，不能用作成功证据。

在VS x64开发环境按after-build.cmd建立新目录；业务副本由git archive abc8d870生成，按business-after-build.cmd使用当前框架源码override，不改原仓库。先从正常交互桌面运行CTest完整清单-j1，再运行ui_visual_ui_test的api7_fixture.uapp、显式OSMesa路径、light/webview2参数及截图前缀。原生鼠标用例由UI_NATIVE_SCROLL_EVIDENCE指定新目录。自有业务观察器编译cmd和规定business-after工作目录均存原ZIP，不能执行历史包/程序替代。

业务完整30项中kc_host的旧坐标点击失败保留，独立仍失败；公共presentation鼠标命中折叠/恢复与100项业务观察通过。原业务锁/测试打印43587c3，实际新源码6a0170a的编译override和产物哈希由identity证明，不能将旧打印当作当前构建来源。

当前源码Session0缺服务权限，严格无登录runner无，真实IME/物理跨屏/DWM/实际Snap/长期人工及完整业务试点待验。原生系统控件与应用自有GL配色保留，Light/Runtime字体插值不保证逐像素相同。
