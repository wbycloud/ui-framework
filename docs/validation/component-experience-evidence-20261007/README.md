# 2026-10-07 API8 实际运行证据

产品807828147bdfb6959abb49b4668dfa90c34eda56，测试收敛0fe08640c3008874449dce77fd7445ef91f6a578。接手c62753d、SDK0.7/API7，本轮公开列宽和help集中升级SDK0.8/API8，ABI1/包1。完整结论、失败和边界见[验收](../component-experience-validation.md)；准确产物身份见[artifact-identity.json](artifact-identity.json)，文件校验见[sha256.json](sha256.json)。均为本地交付，不是托管CI或完整业务验收。

## 真实截图对照

同完整独立DLL/.uapp、100k/64列、树/属性/实际OSMesa视口、1200×800、96程序DPI。默认列宽保持应用注册值，只有用户显式操作才改变。截图观察器用公共set_column_width展示调宽状态；真实表头拖动由原生SendInput用例另行验收。

| 主题/后端 | 修改前 | 修改后 |
| --- | --- | --- |
| Light浅色 | [之前](screenshots/before-light-host-light.png) | [之后](screenshots/after-light-host-light.png) |
| Light深色 | [之前](screenshots/before-light-host-dark.png) | [之后](screenshots/after-light-host-dark.png) |
| 实际Runtime浅色 | [之前](screenshots/before-runtime-desktop-host-light.png) | [之后](screenshots/after-webview2-host-light.png) |
| 实际Runtime深色 | [之前](screenshots/before-runtime-desktop-host-dark.png) | [之后](screenshots/after-webview2-host-dark.png) |

| 当前体验 | Light | 实际WebView2 Runtime |
| --- | --- | --- |
| 调宽后完整窗口 | [查看](screenshots/after-light-columns-resized.png) | [查看](screenshots/after-webview2-columns-resized.png) |
| 适配/重置入口 | [查看](screenshots/after-light-columns-tools.png) | [查看](screenshots/after-webview2-columns-tools.png) |
| 显式缓存适配 | [查看](screenshots/after-light-columns-fit.png) | [查看](screenshots/after-webview2-columns-fit.png) |
| 完整可复制文本 | [查看](screenshots/after-light-complete-text.png) | [查看](screenshots/after-webview2-complete-text.png) |
| 属性与反馈 | [查看](screenshots/after-light-form-light.png) | [查看](screenshots/after-webview2-form-light.png) |
| 长枚举 | [查看](screenshots/after-light-enum.png) | [查看](screenshots/after-webview2-enum.png) |
| RGBA | [查看](screenshots/after-light-color-picker.png) | [查看](screenshots/after-webview2-color-picker.png) |
| 工作区长名称/停靠 | [查看](screenshots/after-light-workspace-details.png) | [查看](screenshots/after-webview2-workspace-details.png) |
| 窄窗 | [查看](screenshots/after-light-narrow.png) | [查看](screenshots/after-webview2-narrow.png) |
| 144程序DPI | [查看](screenshots/after-light-host-dpi-144.png) | [查看](screenshots/after-webview2-host-dpi-144.png) |
| 192程序DPI | [查看](screenshots/after-light-host-dpi-192.png) | [查看](screenshots/after-webview2-host-dpi-192.png) |
| AI用户展开 | [查看](screenshots/after-light-assistant.png) | [查看](screenshots/after-webview2-assistant.png) |

当前只读业务副本：浅色[完整窗口与实际WGL四形状](screenshots/after-current-business-light.png)、[深色](screenshots/after-current-business-dark.png)、[AI](screenshots/after-current-business-ai.png)；真实鼠标[首项](screenshots/current-business-drag-first-cell.png)、[末项](screenshots/current-business-drag-last-cell.png)、[未缓存末列STYLE](screenshots/current-business-drag-last-column.png)。当前业务只有本轮修改后观察，不冒充本轮前后对照；原完整回归29/30仍有旧坐标失败。

[screenshots.json](screenshots.json)记录105张PNG的原BMP、各自SHA256及实际尺寸：40张有效前基线、57张当前实图、8张受限Runtime失败过程图。`before-runtime-desktop-*`是成功真实Runtime基线；`before-runtime-*`其余8张属于失败过程，不可用于成功对照。PrintWindow/实际桌面BitBlt来自真实宿主和DLL，PNG只作无损RGB转换，未使用生成图或静态页面效果图。程序DPI、自动输入和实际Runtime截图不代替IME、物理跨屏、桌面合成及人工验收。

## 原始失败、修复与执行来源

`raw/`保留本轮初始和中间失败，不以名字中的green/final宣称通过：

- `full-current.log/xml`：46通过/9失败/1物理跳过；Light有限几何/静态属性与Runtime初次超时。
- `framework-8078281*.log/xml`：两次54通过/1宿主超时/1物理跳过。`features-pump-red.log`和`features-sequence-trace.log`保存WM_PAINT队列诊断，`features-pump-green.log/xml`实际仍有一次命令失败；最后`features-pump-ready-green.log`独立3次通过、`framework-delivery-final.log/xml`55通过/1物理跳过。持续重绘来源未归因。
- `business-current`、`business-final`、`business-tip-fix`、`business-pointer-green`保留原业务失败/超时；最后`business-8078281.log/xml`仍29/30，`business-real-scroll-pixels.log`是另一个框架维护只读观察器的102/102，不能互相替代。
- `before-runtime.log`是受限环境失败/未完整结束；`before-runtime-desktop.log`才是成功基线。错误CTest路径/空测试选择、编译错误和中间诊断cleanup未等待WAIT均保留，不能算功能通过。

`raw/build.cmd`、`native-build.cmd`、`business-after-build.cmd`等记录真实构建环境；`api8-rich-observer.c`、`business-observe.c`、`business-scroll-observation.c/.inc`和对应构建cmd是框架维护的截图/只读观察工具，原业务源码和断言保持原样。`raw/final-plan.json`是早期阶段计划，最终实际执行选择见[delivery-run-plan.json](raw/delivery-run-plan.json)。依赖、Session1/服务权限错误5、SDK离线构建/消费及产物哈希分别记录，不把历史Session0成功继承给当前源码。

SDK本地目录`build/experience-20261007/sdk`，归档`build/experience-20261007/ui-framework-sdk0.8.0-dev-api8-8078281-windows-x64.zip`；`SDK-README.md`说明匹配头文件/库/宿主、固定依赖和离线构建，SDK内部`sha256.json`覆盖所有文件（除自身）。ZIP哈希和解包校验在本地`build/experience-20261007/sdk-archive.json`，不把归档的自身哈希放入被归档内容制造循环。
