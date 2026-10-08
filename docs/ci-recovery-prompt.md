# 新会话接手提示词：API9托管CI失败收敛

以下正文可直接发送给新会话。本提示词授权框架本地排查、最小修复、必要回归、文档及本地提交，不包含新一轮推送或远程工作流授权。

以下正文保留1756b06接手时的原失败范围。后续接续先读[本地收敛记录](validation/ci-recovery-validation.md)及handoff第22节；宿主语言断言已有本地修正，原托管存储/语言超时仍待带阶段/退出诊断定位。不要重复修正已提交用例，也不要把本机通过解释为远程CI已通过。

```markdown
请接手 https://github.com/wbycloud/ui-framework，持续完成API9托管CI失败的定位、最小修复、必要回归和文档交付。先核对实际基线，保留用户改动，不重复开发已完成的语言、组件、布局、视觉和滚动功能。

仓库本地路径：D:\应用软件框架\应用层序框架。
参考版本：SDK0.9.0-dev/API9/开发标准9，应用ABI1、包格式1。
已同步至GitHub的基线：b6a36fee3f4c1a73818cdb01ea7271ad5abd30db，main与codex/menus-offscreen均已普通快进到此提交。后续交接文档可能有本地提交，实际HEAD/远端以核对为准。源码与README已完整同步，不再重复“只改远端版本号”的工作。

先读docs/handoff.md（重点第21节）、docs/validation/github-sync-validation.md、docs/validation/instance-language-validation.md、docs/instance-language.md、docs/application-development-standard.md、docs/build-and-validation.md、docs/validation/runtime-stability-validation.md及docs/validation/native-component-scroll-validation.md。
核对git status、HEAD、分支、origin、远端差异、公共头、CMake、固定依赖、.github/workflows/windows-regression.yml、tools/windows-ci.ps1、tools/windows-ci-checks.ps1和相关测试。可以只读获取远端状态；不要覆盖用户修改或混入build/.deps/档案/凭据。先给简短实施计划和可复验成功标准，再实施。

原始自动CI全部绑定b6a36fee、push事件、attempt1：
- main矩阵：https://github.com/wbycloud/ui-framework/actions/runs/37753988354
- 工作分支矩阵：https://github.com/wbycloud/ui-framework/actions/runs/37753988621
- main严格Session0：https://github.com/wbycloud/ui-framework/actions/runs/37753988202
main通过/失败/物理跳过：native 21/0/1、light 42/2/1、osmesa 47/4/1、webview2 56/5/1。真实Runtime专属阶段10/10通过，但WebView2整体配置失败。不能用本机61/1、Runtime子集或工作分支结果覆盖main原失败。

同机原始证据在build/github-sync-20261008：report.md、result.json、evidence-sha256.json，以及原ZIP、JUnit、manifest、日志、资源和真实GL帧。该目录被Git忽略，新clone没有这些文件；新机器从上述运行下载Artifacts/日志，缺失或过期时如实记录，不重生成冒充，不依赖临时网络脚本。先固定产物并核验哈希，保存原失败后再修复。

本轮按依赖顺序完成以下工作：
1. 语言断言：ui_browser_host第90行、ui_web_host_frontend第79/86/88行硬编码中文，而英文Windows runner按API9显示英文。空宿主必须依据Windows显示语言及公共查询合同验证；实例测试可以由测试应用显式提交语言。覆盖zh-CN/en-US、双实例及系统来源，保持精确文本和可见/禁用/草稿/参数/一次命令断言。不要改成任意非空文本、全局setlocale/环境变量或修改机器显示语言获得通过。
2. ui_host_repaint：原日志idle paint0，却报告1失败，具体断言尚未定位。源码第66行“操作完成”固定中文只是线索。先使每个失败断言及实际值可定位，再核验失效区、WM_PAINT实际Dispatch、GL命令结果和关闭状态。确认根因后最小修复，不因名称就宣称持续重绘回归或根治。
3. 完整应用脚本超时：ui_stateful_components_light在webview2配置180秒Timeout；ui_instance_language_light在osmesa/webview2配置240秒Timeout。追踪脚本阶段、子进程耗时/退出、消息循环、真实输入最终状态及异步关闭。保留全套跨进程/DPI/异常状态验证；部分子用例failures0不能代替整项完成。不能加sleep、延长超时、强杀Runtime、清用户目录、提高阈值或删断言遮掩。
4. 修复后运行必要轻量/真实Runtime/OSMesa/原生回归，包含完整stateful_components DLL/.uapp、语言与状态跨进程恢复、双实例、布局/列宽、草稿/选择/焦点、模态、实际GL、AI默认收起及尺寸回收、REFUSE/WAIT、异步清理和DLL卸载。保留三项原生真实鼠标滚动、同HWND捕获、异步轨道几何、uint64/BigInt和完整范围按需查询；不恢复共同组件分页按钮。

现有矩阵脚本分prepare/build/test阶段，构建目录build/ci-配置；不要在已含用户修改的依赖目录盲目执行prepare。核验固定Lexbor 7fb22cf5664a331d7c24b113489e566767c9c25a、QuickJS-NG 2f0aa72a6b09cf69ff06399bcf9f4c083ac1a278、WebView2 SDK1.0.4129.50、Mesa24.3.4。原CI Runtime154.0.4258.62、MSVC19.44、Windows Server2022/image20260927.320.1、Session2；新的实际环境必须另记，不能沿用旧Runtime数值。软件WGL、OSMesa provider与真实Runtime阶段沿用现有隔离，不用离屏输入替代原生鼠标验收。

当前提交实际LocalSystem Session0应用DLL/OSMesa已通过：GL failures0、2000commands、created_HWNDs0/process_HWNDs0。严格整机无登录因logged_sessions1失败，用户没有无登录服务runner。将此环境缺失与五个失败测试分开；不注销用户、修改既有服务或放宽RequireNoLogin。最后汇总真实IME、物理跨屏/不同DPI、桌面合成、长期人工和服务权限条件，缺失不阻塞独立修复。

共同约束：Windows x64/C11、当前API9公共C ABI、size/字段/枚举、UI线程、字符串复制/所有权、异步失效及卸载合同保持。内部修正不无故升级API，只维护当前版本，不恢复历史兼容专项。不修改D:\应用软件框架\klayoutC、冻结SDK、历史包/证据或PERF-001，不新增无关功能/依赖/后端或重构，不提高资源预算、不删除失败断言、不预加载全数据，不使用graph-engineering、子代理或新会话。

封存SDK属于产品6d88770/文档312bce4，ZIP SHA256为7c1dbffce1211b2c0654b41d2942b05adbd4c54d111e983d836c27550d8802e1，不能改写旧包或哈希。产品变更需要新SDK时另建匹配交付并独立记录。

按可审查单元本地提交，同步必要CHANGELOG、验收、构建说明和handoff；未经新的明确授权不推送、不触发/重跑远程工作流、不强推、不创建标签或Release。上一轮同步授权已完成，不能当成未来任意推送授权。
最终逐项报告原失败、已定位根因、修复、实际commit/依赖/命令/产物哈希、复验及仍待定位/环境/授权事项。本机通过不能冒充托管CI；无法复现不能写成根治，存在未通过或待验项时不宣称全部验收完成。
```
