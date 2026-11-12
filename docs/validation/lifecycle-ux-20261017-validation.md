# 生命周期、显示与交互性能查证 — 2026-10-17

作者：Codex。范围：A Mesa 生命周期；B 显示、重绘与 Runtime；C 性能、窄布局、主题与资源。本文区分历史事实、本轮实测、产品修复、测试校准及缺条件项。最终回归与归档证明见文末；冷Runtime子进程退出失败及未完成项独立列账。

## 入口与保护

实际 HEAD `1af1f5a9f2efd6627ffea300ef90c41171cb917b`，分支 `codex/p1-process-boundaries`；入口已有 27 个 tracked 修改、6600 个未跟踪文件。上一轮源码/资源身份 `599800c363e101a6179dcc1c709a8d721cd74eafb47b21b22f986664324c2b94` 的 249 项代码清单全部匹配，均属本轮受保护入口。

[入口清单](lifecycle-ux-20261017-f6c42b91-evidence/raw/entry.json)记录583份源码/资源/当前说明、6600条旧未跟踪路径、15份历史哈希索引和3项保护产物。原始字节快照为 `build/lifecycle-ux-20261017-f6c42b91/snapshots/entry-source-resources.zip`；入口 Git diff 另存，回退不能恢复到 HEAD 覆盖上一轮修改。

SDK/API/标准仍0.9.0-dev/9/9，ABI/包格式1，无本轮新增公共接口或默认 provider 改动。固定 Lexbor、QuickJS、WebView2 SDK、Mesa 均未 prepare、升级或覆盖。业务应用、PERF-001、冻结SDK及历史材料只读；无提交、推送、远程执行、标签、服务改动、注销、硬件GL、子代理、CUA或私有调用方入口。

机器 Windows10.0.26300、Session1、Default交互桌面、2560×1440、单显示器；窗口/系统及显示器报告DPI均96，程序96/144/192单列。MSVC19.50、实际Runtime155.0.4283.45。新的构建、验证、交付、解压目录均在 `build/lifecycle-ux-20261017-f6c42b91/`。前台已获用户明确空闲解锁答复；输入串行，自动SendInput与硬件人工分开。

## A. Mesa：有边界的本地对照

历史基准：[目标A/C原记录](p1-mesa-hosted-control-validation-20261009.md)。默认供应方A的退出等待已在目标托管复现，候选C完成过一次目标A/C并自然退出。该事实保留；本轮不是重新把所有超时标成完全未定位，也不把本地通过当成默认A修复。

### A1 同构建B/C

A是供应方原件，SHA `c633b820bba8ec0dcb05466505e3c246e4c4028eaec7caf1054e707c8eb3ca1f`，与B/C工具链不同，独立列。B/C复用并复制已封存的精确DLL/PDB，没有冒称本轮重新编译Mesa。

| 项目 | B：无补丁 | C：进程终止边界候选 |
|---|---|---|
| DLL SHA256 | `3bd1280a2d72052c3ef673428dde75c8d3a07ab0ca905c048ea0c054d6169159` | `99d8142434c3abfacce969aa3731220314d99c53ee0afc546896c0afad92f5f5` |
| PDB SHA256 | `41abb9787bf910960f06978b208d065949b4021b055033389747f84e3c646517` | `c5c15c5cc6673b795d21fa721d87a6abb382005417129685903937e744f73bbd` |
| 构建 | MSVC19.50，静态CRT，LLVM19.1.7 | 相同 |
| 源码 | 10640文件逐项核对 | 仅osmesa.c、osmesa_target.c两文件差异 |
| Meson选项及导出 | 全部选项一致、370导出 | 相同 |

[身份/选项/源码差异](lifecycle-ux-20261017-f6c42b91-evidence/raw/mesa-identities.json)及完整清单保留。PDB RSDS逐个匹配。A的353导出均在B/C中，无A原导出缺失。

新独立公共OSMesa调用方不加载业务、不调用私有RVA。A/B/C各9个新进程：默认exit6、LP_NUM_THREADS=0 exit1、默认8轮context循环1、默认FreeLibrary1，共27个，均自然OS退出0。512×512、120帧×64三角形，原像素、alpha、错误码和全缓冲FNV断言保持。默认路径本地未重现目标退出等待，因此本轮“默认失败而0配置通过”的因果对照不成立，不能据此认定LP0修复同一个本地等待。

LP0仅在子进程首次初始化前设置。实际线程证据不是环境变量回显：PSS记录默认48个Mesa线程句柄，加main为49；独立live gate匹配DLL/PDB转储各确认24个rasterizer与24个compute栈，另有3个Windows线程。LP0实际只剩main和3个Windows线程，没有Mesa工作线程。PSS释放后目标HANDLE计数恢复，未将观察器复制句柄留在目标中。gate、PSS、调试断点会扰动时序，单列，不混入未调试退出或界面性能样本。

### A2 259→0：尚未定位原子时序

新外部观察器记录同一原始HANDLE、观察器DuplicateHandle、GetExitCodeThread调用前后QPC、零等待结果及观察器副本关闭。默认B/C终止跟踪和6次A的1线程跟踪均自然退出，但未捕获原查询259→随后同对象0的原子转变。历史dump保存的259、调试器稍后真实查询0、缓存值与当前查询仍分账，不追加猜测性HANDLE修补。

保留一个反证：A的全断点观察首次在初始化 `_beginthreadex/create_thread_parameter → lp_cs_tpool_create` 停住，45秒原诊断期限强制结束254；匹配转储显示尚未进入manager/rasterizer退出。它是受观察扰动的初始化失败，不能冒充本次退出等待，也未用后续通过抹去。其dump位于新构建目录，匹配栈日志纳入证据。

### A3 三个清理边界

| 边界 | 本轮实测 | 当前结论 |
|---|---|---|
| OSMesaDestroyContext及暖态8轮 | 三provider均OS0，默认HANDLE每轮167；48Mesa线程仍存活 | 普通context释放通过；进程级screen/manager缓存仍存在 |
| 默认多线程FreeLibrary | 自有引用2→1→0，GetModuleHandle仍映射 | CRT工作线程持有模块引用；不能称真正UNLOAD |
| LP0正常FreeLibrary，reserved=NULL | A/B/C各记录manager/rast清理、真实DEBUG_MODULE_UNLOAD及GetModuleHandle=NULL，OS0 | 仅LP0正常卸载通过，C正常分支flag0未跳过清理 |
| 默认整个进程终止，reserved!=NULL | B进入compute/rast清理并OS0；C先通知termination、manager flag1跳过screen等待，OS0 | C候选符合已知Windows/MSVC静态CRT终止范围，不能外推正常卸载 |

固定公共OSMesa没有显式screen清理入口；本轮禁止私有调用方，因此没有使用旧RVA清理探针，也不强行卸载未知引用。默认多工作线程下的运行时screen清理及真正UNLOAD仍未闭环。LP0正常卸载和OS最终回收不能填补该项。

### A4 历史24句柄余量

源码中rasterizer拥有 `thrd_create` 返回HANDLE，Windows分支等待后未走 `thrd_detach/CloseHandle`；已有历史正常清理/真正卸载诊断将24个已退出但未关闭的rasterizer线程句柄定位于此。当前默认缓存中的48线程仍存活，与“线程已退出但句柄未关闭”不同；PSS观察器自身副本已关闭。LP0没有该24线程集合，不能作为句柄修复。受A3公共清理边界限制，本轮未新增句柄补丁，仍列“历史归属明确、当前公开多线程清理/修复待验”。

C不默认纳入SDK，LP0不固化配置。默认A、候选C、单线程绕行的路径和成本分开。固定负载默认渲染中位A/B/C为168.860/166.715/166.038ms，LP0单样本为855.394/868.270/868.215ms（约5倍），像素hash均38b57cea08bb9bf9。LP0样本仅1次/provider，不能作一般性能保证，也不以离屏吞吐代替应用体验。目标同条件B/C方案与执行器已完成本地只生成计划检查，保留原存储180秒、语言240秒，无重试到通过；需要新的明确远程授权，未执行。

## B. 显示、重绘、Runtime

### B1 192叠层与窄表头：当前公共场景未复现泛化产品缺陷

已有07a66a8等浮动、提示、多行、对话框修复及上一轮Runtime144捕获边界修复属于入口，不重复开发。

新构建Light与真实Runtime公共display探针在96/144/192执行：0→全选逐字0.002、相同粘贴后续输入、长文本Home/End、900/350宽度、表头resize与fresh同尺寸逐像素对比。全部OS0，Runtime观察到的6个子进程均signaled/exit0。截图与公共RGBA分别验收，不把捕获正确称为桌面合成根因修复。

视觉复核另外发现：Runtime192的原表头resized截图实际空白，而之后的离屏header_capture通过。旧探针在异步就绪查询前拍摄，属于验收顺序缺口，不把它当作显示叠层根因。新增公开就绪/首次RGBA完成→正常Dispatch→DwmFlush→实际桌面ROI/完整画面，未强刷或增加sleep/原10秒查询预算，保留旧图与断言。Light/Runtime各9个表头阶段（96/144/192前/resize/fresh）共18个非空白桌面检查通过，resize=fresh原像素断言保留。该测试修正已进入现有display探针，CMake只补系统dwmapi链接。

补验整项结果Light OS0，Runtime OS1：在第一96DPI、尚未执行新表头路径前，CtrlA/相同CtrlV/Y输入后API值仍0.002x，完整真实画面也显示原选区高亮且Y未出现；不是仅凭缓存getter猜测已输入。原断言未删、没有增大80ms输入cadence或重试等到通过。144/192后续同项通过不能抹去首个失败；框架/Runtime原生粘贴/输入驱动责任仍尚未定位。它与历史0→0.002修复、Home/End及新表头时序分账。该次观察的24个子进程均正常退出。

混合公共探针组合native按钮、公共FORM/Web、固定软件WGL；实际renderer为 `llvmpipe (LLVM19.1.7,256bits), Mesa24.3.4 git-1950a8b78c`。Light/Runtime×96/144/192真实鼠标经过GL、持有预览、目标切换、释放、7px边角命中、Esc、受控捕获丢失、resize、dock/undock、关闭恢复，核对焦点/草稿/HWND/context身份。补充自有非激活遮挡窗口的前/中/后、原生paint正常Dispatch、恢复像素和折叠恢复；不强刷整窗、不重建控件、不扩大缓冲。逻辑几何、popup实际DPI、clientOrigin、screen RECT、父子/前序窗口及失效矩形有日志。

保留新诊断首次失败：192程序DPI被错用于实际96DPI的浮动面板，草稿点击失效；Runtime首次presentation pending被误用。核对公开panel.dpi并遵守原10秒pending期限后六组通过，未修改产品实现。另一次命令参数顺序错误走错场景且63个截图写入失败，单列命令错误，不作功能/视觉通过。遮挡观察再补正常消息Dispatch，避免仅DwmFlush而本进程WM_PAINT尚未处理；无额外sleep或超时放宽。

结论范围是本机单96DPI显示器上的程序DPI及指定窗口组合。泛化192叠层、窄表头残影仍未独立复现，未凭猜测改合成架构。正常提示、已知遮挡、自有捕获和真正合成异常不混为一谈。

### B2 持续重绘：一条冗余布局链确认，其余边界保留

Light后端 `lw_set_rect` 在尺寸/DPI相同且文档无dirty时仍每次重建字体、SetWindowPos、布局。公共调用方200次同矩形：入口DLL发送200次WM_WINDOWPOSCHANGING、200次布局，223.095ms；WM_WINDOWPOSCHANGED=0。相同EXE与候选DLL：0次changing、0次重复布局、0.00590ms。隐藏窗口的UpdateRect=0不能证明桌面没有损伤。

独立真正宿主当前空闲三段2秒及GL后2秒均paint0，CPU每段15.625–31.250ms；三次resize各6次paint/3layout，GL两次各4paint/1layout，所有检查的invalid-after0；完整LastTest日志另存，未将JUnit截断内容当完整诊断。最小产品修复仅在Light稳定路径复用字体/布局，并核对实际HWND矩形；位置改变仍SetWindowPos，尺寸/DPI、dirty、reload仍完整处理。没有对所有backend/host的set_dpi加入全局早退，保留失败重试和自定义后端语义。新增公共回归 `ui_light_rect_stability` 覆盖96/144/192、实际HWND被外移后校正、resize、reload及presentation。

操作期公共调用方三轮交错前后见C1。稳定空闲及独立真正宿主的WM_PAINT、invalid-before/after、位置消息、wake、layout、CPU由原 `ui_host_repaint` 新跑并保留完整日志；单个idle paint0不作为历史持续重绘已根治证据。宿主全客户区绘制范围、重复消息源仍是独立边界；本轮没有第二条足够证据支持的产品性能补丁。

### B3 三类Runtime分账

公共Runtime生命周期：预定6个独立进程，程序DPI96/144/192/192/144/96；每进程8次创建→ready/navigation→公开首帧RGBA→销毁，以及一次ready前取消。48次重开、6次取消均通过，父及观察子进程均OS0。ready原10秒期限未增加。首个冷首帧约2703ms，另有暖轮约4343ms，最差保留；它们是公开捕获端点，不等于实际屏幕呈现或50ms达标。

历史首次C restore144的4条语言/活动切换断言失败不是退出卡死，旧日志未包含充分活动时序，仍尚未定位。新回归在原点击40/150ms和8轮精确断言不变条件下补 `TAB_INPUT before/after` 的目标ID、公开active、tick、Dispatch；暗色校准改用现有 `ui_host_set_theme`，不能再以被状态同步覆盖的 `h.dark=1` 冒充主题覆盖。限定3对144 write/restore每后端、独立新进程、首失败停，不重试解释历史失败。

本轮限定restore144实际执行：Light三对write/restore六进程全部通过；Runtime首个write功能断言0、主OS0，但10秒子进程检查中PID24328、13736未signaled且退出码259，runner124，按计划停止，其余五个Runtime样本未执行。不是restore功能断言失败，也不是Mesa主进程等待。取证时两PID已不存在，DMP未取得、后续实际退出码未知，不能以“后来消失”升级为通过。

为查责任另预定四个同EXE/DLL、独立新用户目录的A/B/B/A控制：A原CoUninitialize路径，B在CoUninitialize前按原10秒边界分发UI消息。四个均自然OS0、各14子进程退出0；B进入检查时已经没有待退出子进程（elapsed0），所以因果对照不成立。另对入口与候选DLL各一次、同原EXE/包且不启用Runtime资源日志，均通过；不能归因或排除本轮时序影响。没有据此增大sleep/超时或修补产品。新增观察器在首次发现时记录PID/parent/创建时间/命令行，持有真实句柄，在10秒失败边界立即取dump；首次失败原记录不覆写。

公开头要求销毁后保持UI线程分发延后清理；现有后端用BrowserProcessExited释放environment，与[微软事件合同](https://learn.microsoft.com/en-us/microsoft-edge/webview2/reference/win32/icorewebview2environment5)一致。该合同不意味着CoUninitialize返回即所有子进程退出。本次未拿到失败时栈/浏览器角色及回调队列，框架/调用方/Runtime责任尚未确认，作为新遗留问题继续列账；不宣称长期资源闭环完成。

诊断mirror pending是另一条历史观察链，不是产品Runtime ready；本轮未通过注入私有JS重跑mirror，也不宣称修复。上述有界成功不能证明所有瞬态Runtime或托管问题已解决。

## C. 体验、布局、主题与资源

### C1 当前源码精简基线

新公共C owner使用原维护完整应用包，100000行×64列、完整范围、uint64/BigInt、按需查询及原预算不变。原生导航按钮/公共菜单和对话框→实例→长输入Home/End→wheel/scrollbar/列宽→浮动穿过软件GL预览→dock/程序resize→idle。该owner不是独立framework_host的Chrome，本表不冒称宿主chrome点击时延；真正宿主行为由独立回归另验。公共RGBA预览与真实WGL分别有证据。

同机器1200×800、96程序DPI、固定数据/主题和动作。12新进程交错运行（3轮×2后端×前后），每进程94样本，共1128；同EXE、同应用包、唯一新输入标记、每次新Runtime用户目录。所有样本有效、0缺样；父及观察子进程自然0。尚未建立所有操作的完整最终可用时延。CSV verifiedPublicState仅为有限条件：Home/End只查焦点、scrollbar只查总量和capture释放、float只查floating、dock只查非floating/HWND同一；不是这些操作的完整最终状态。其它独立功能回归不能被倒用于该时钟。完整端点差距已在性能表列明。

时钟从输入提交前开始；首次可见反馈为独立桌面ROI完整pixel hash首次变化完成的BitBlt/GdiFlush采样上界，包含队列/Dispatch/布局/绘制至该读回端点。实际采样中位间隔约16.6ms；ROI首次变化未逐样本排除光标闪烁/悬停等非目标变化，因此属于候选反馈时间，不能把零缺样当作1128次目标操作呈现已逐帧确认；paint、处理返回、公开状态、桌面读回、真实scanout不混用。没有scanout时间戳，因此不填造60fps、最长呈现帧或错过刷新次数。

[全部类别P95/P99/最差与缺样](lifecycle-ux-20261017-f6c42b91-evidence/raw/performance-summary.json)、[采样间隔与逐PID资源峰值](lifecycle-ux-20261017-f6c42b91-evidence/raw/performance-intervals-resources.json)、每样本CSV及桌面ROI原始CSV均保留。CPU是最后观察到的进程累计CPU，包含桌面观察器，不能当完整进程OS总CPU；逐PID峰值不能相加冒称同一时刻总峰值。Light paintArea是名义view面积，不是合成器damage。

旧观察器dock ROI遗漏浮动/目标区、Runtime滚动只看相同单元格且忽略footer；新ROI取可见表格+footer和owner+浮动/停靠目标，逐pixel hash。首次缺样和功能失败都保留，缺样不记0ms；没有依据消去旧最差样本。另校准真实菜单条目、Esc异步公开状态、长输入桥接全新长度，不更改产品门槛。

### C2 最小性能修复与代价

仅B2一处满足责任明确与稳定工作量收益条件。Light三轮layout均2841→2460（少381，13.4%），布局耗时约3032–3059→2812–2832ms；父CPU中位8625→8109ms。名义paint次数与面积另列原数据。Runtime父CPU与时延没有一致改善，不外推Chromium内部布局。

首次桌面反馈P95/最差ms的局部对照：

| 类别 | Light前→后 P95 / worst | Runtime前→后 P95 / worst |
|---|---|---|
| instance | 47.48/49.12 → 47.85/48.11 | 50.21/50.95 → 48.49/48.95 |
| menu | 48.99/49.47 → 48.89/49.12 | 64.32/66.33 → 61.97/64.57 |
| dialog | 48.39/48.95 → 48.18/50.24 | 54.09/56.14 → 50.23/65.54 |
| wheel | 28.88/29.84 → 29.23/32.11 | 38.79/41.56 → 40.68/42.60 |
| dock | 49.07/49.21 → 48.48/49.16 | 47.93/48.73 → 48.66/49.39 |

样本规模有限，部分后测最差更高；不宣称所有操作提速或达到50ms。保留候选是因为同矩形冗余工作已直接消除、交错Light布局/CPU下降，最终还须通过原资源与功能门槛。没有为了凑两三处修改而猜测架构瓶颈，不增加sleep/预算、不裁数据。

### C3 窄布局与长选项

已有完整内容F1/上下文入口、内外滚动、焦点显露足够表达本场景，无新接口。公共最小FORM使用40条含中文长选项，300物理像素窗口在96/144/192程序DPI缩到284/189/142逻辑宽；真实End到最后项、F1完整只读文本、CtrlEnd/CtrlA/C取得完整 `OPTION_END_39`、Esc/Enter选择最后语义值、Tab后续字段。Light/Runtime均OS0，12画面/后端，192末标记实际可读。窄表头/长编辑Home/End见B1；原对话框/内外滚动新回归另列。该选择探针自身未独立等待Runtime子进程，不能用其父OS0代替子进程回收证明；资源/退出由单独完整生命周期探针验证。

### C4 主题

复核每host范围、SYSTEM查询、WM_SETTINGCHANGE/WM_THEMECHANGED转发→refresh→generation/callback→workspace通知→调用方自绘配合，沿现有公开合同；正常/hover/pressed/focus/selection/disabled语义不靠单改背景替代。新完整循环显式刷新当前真实系统，未变化不重复generation；活动/后台主题与标题隔离、草稿保留，真实display主题切换后焦点/后续输入保留。独立宿主公开应用验证标题完整F1、非活动实例、实际主题及关闭。

没有自动修改Windows主题，因此“真实SYSTEM主题切换”仍待条件。高对比度、系统强调色、调用方自绘/图标自动适配是既有合同限制；关闭最后实例保留显式宿主明暗沿当前约定，不自行改。

### C5 限定资源趋势

预先限定每后端24轮，先保留一个主实例循环，再单独每轮两实例全部关闭；覆盖标题、主题、浮动/停靠、8次create/mount预期失败、images/模块/进程。正常每轮应用DLL真实不再映射；OSMesa全局缓存模块仍在，与应用DLL卸载分开。

该限定循环通过不抵销B3另一次冷进程子进程退出失败。完整循环首次8失败源于新调用方在公开延后清理前检查；补一次正常 `ui_workspace_poll` 后Light及Runtime均OS0，Runtime13个观察子进程均退出0。Light每轮清理HANDLE213/GDI2/USER9稳定；Runtime309/1/20后降302/1/11，未呈线性句柄增长。每实例图片2项18200字节，原32MiB限制未改变。全程私有内存有约13–70MiB波动和低点回收，不能只取结束值宣称泄漏根治；冷/暖/每轮/owner销毁及逐PID曲线均保留。该限定压力不是长期人工验收。

## 失败、候选与未验证

原始失败不删除：受调试A初始化停滞254；公共体验菜单/Esc错误与首个桥接时序失败及ROI缺样；新资源同步清理假设8失败；错误参数63写图失败；混合浮动DPI/pending诊断失败；首次受限CMake ABI检查无进展由本轮已知进程CtrlC结束；构建/脚本路径或UTF8误用均留日志。没有停止未知进程。后续通过只限定校准后的场景，不解释掉历史首次失败。

两份旧BMP CRC原件本轮不读取。只读核对已有disk event7日志、对应PNG及C盘恢复备份SHA；备份存在可读不证明物理盘健康。无修盘、扫描、ACL修改、原件替换或替代截图冒充。

独立未验证：当前源码Session0、严格整机无登录、真实英文Windows、真实中文IME、物理高/混合DPI与多屏、桌面合成专项、硬件鼠标/外部捕获抢占/长期人工体验。程序DPI、受控映射、受控捕获和离屏采样不能替代。

## 命令、最终身份、回归及交付

下列是本轮实际结果；所有命令/环境/身份/OS退出原始manifest保留于新证据目录。Mesa目标同条件B/C只生成可审查计划，未启动远程；默认provider仍供应方A。没有本轮公共能力增补，保持当前开发快照匹配头/库/DLL/宿主/应用包，历史冻结SDK不动。

### 最终源码与差异

HEAD/分支未改变。非Markdown源码/资源清单250项，最终SHA `39418650e488e3fb0d53618a3089ac96baed6101716584e23b37c5d894d06eb7`；入口249项身份599800c3，不能把新的工作树SHA称成Git commit。入口与最终DLL导出集合均248项，完全相同；无本轮公共能力增补，版本/结构/ABI不变。

本轮相对受保护入口仅9个既有文件及1个新源文件：Light后端、CMake、实例语言测试、display公共探针、两份CI清单、build/handoff/next-session三份说明，新增 `tests/light_rect_stability.c`。除此是新专项报告、独立证据和构建/交付目录。入口其余修改不被计作本轮实现，详见entry-relative patch与回退清单。

### 实际命令与结果

命令从仓库根运行。完整参数/环境、文件SHA、PID/OS退出在每run.json/manifest/JUnit及scripts目录；不是只截日志尾部failures0。

```powershell
python -B build/lifecycle-ux-20261017-f6c42b91/tools/mesa-control.py
python -B build/lifecycle-ux-20261017-f6c42b91/tools/mesa-traces.py
python -B build/lifecycle-ux-20261017-f6c42b91/tools/mesa-live-gates.py
python -B build/lifecycle-ux-20261017-f6c42b91/tools/run-perf-pairs.py
python -B build/lifecycle-ux-20261017-f6c42b91/tools/build-finals.py
python -B build/lifecycle-ux-20261017-f6c42b91/tools/run-regressions.py
python -B build/lifecycle-ux-20261017-f6c42b91/tools/run-post-regression.py
python -B build/lifecycle-ux-20261017-f6c42b91/tools/run-runtime-cleanup-control.py
python -B build/lifecycle-ux-20261017-f6c42b91/tools/runtime-exit-entry-candidate.py
python -B build/lifecycle-ux-20261017-f6c42b91/tools/run-sdk.py
```

这些是已经执行的记录入口，复跑必须选择新目录；现有脚本会拒绝已有输出，不能直接覆盖本轮证据。post-regression在首个Runtime子进程检查失败后返回1，standalone随后作为独立case执行并通过，不把原runner改写为0。原Light存储180/语言240、Runtime各原门槛（语言300）均保持。

| 新Release配置 | 精确计划 | 通过 | 失败 | 物理跳过 |
|---|---:|---:|---:|---:|
| native | 23 | 22 | 0 | 1 |
| light | 50 | 49 | 0 | 1 |
| osmesa | 57 | 56 | 0 | 1 |
| webview2 | 70 | 69 | 0 | 1 |

共200项：196通过、0失败、4次单显示器ui_monitor_transition跳过。四配置精确清单、current_only政策自检与审计全部通过。WGL阶段使用隔离固定软件DLL，provider与Runtime阶段没有应用本地WGL替换；不涉及硬件GL。

ASAN新Release优化构建12项全部通过，组件resilience 21.37秒、原30秒期限不变；包含新几何回归、文本、主题、标题、C/C++头、Light/Runtime布局与捕获。ASAN不覆盖所有vendor Mesa/Chromium内部内存，也不替代实际子进程退出。

补充真实输入：display两后端；混合表面六组；长选项两后端；独立实际framework_host公共标题/主题应用；12进程性能；Light三对144；Runtime首write退出失败与后续限定诊断控制。真实Dispatch、最终状态、capture释放及进程记录分别保留；受控capture loss不冒充外部抢占。

### 新SDK及独立解包

新包：`build/lifecycle-ux-20261017-f6c42b91/delivery/ui-framework-sdk0.9.0-dev-api9-1af1f5a9-worktree-39418650e488-windows-x64.zip`

- SHA256：`b1ab8373cfff1389832fe7bba4733253d8e20fc833a9bf60deb7c9ff759e52b9`
- 大小74316410字节，1620文件；ZIP全部CRC通过。
- 独立解压：`build/lifecycle-ux-20261017-f6c42b91/unpack/ready/sdk`；逐文件SHA全部匹配，实际运行与离线构建后仍1620原文件不变、0新增文件。
- 运行使用解包bin的逐文件等SHA外部副本，Runtime profile、日志和截图均在归档/解包外；新公共C调用方只使用解包include/lib，匹配DLL。
- 首份8806快照解包执行12项全部OS0（其原表头截图后续被判为就绪前无效，不能单凭这些退出结果认定可读）：新Light几何、C/C++头、主题、标题、公共C资源回调、两后端替换、真实软件OSMesa/MSAA、两后端96/144/192真实display、实际宿主公共调用方。观察到的子进程全部signaled/exit0。
- 推荐3941快照的5个非交互执行（几何、C++头、主题、公共C消费者、实际windowlessGL）及2个解包源码/头/库表头专用调用方均OS0，后者共18桌面检查、无注入输入、子进程全部退出0。包内固定Lexbor/QuickJS/WebView2 SDK新目录离线244步构建0，包含修正后的display目标；4项theme/public-C++/windowless-GL/geometry检查通过。它们不冒称第二次全矩阵。
- 默认Mesa仍供应方A（SHA c633b820…）；C/LP0不纳入默认。头/导入库/DLL/宿主/应用包同一源码身份。新包加几何源/EXE，旧包两个validation辅助文件未复制；新公共调用方源码与构建证明另存证据，旧包字节不变。

首份SDK `...880646036330...zip`（74314839字节，SHA a09645263efd335c088ce30f1d3848888ec0a49648c120f7bc2540030b70b288）已封存，原字节保留，不再作为推荐诊断交付；没有补写。新3941包只更正表头诊断，bin/lib/include除ui_display_probe.exe外与首包逐文件完全相同，沿用已跑矩阵的产品/库原字节；新探针使用实际补验的精确EXE字节。完整矩阵/ASAN身份是8806；性能对照的实际入口/候选DLL、同EXE/包SHA各在run.json记录，产品代码与该矩阵一致，不把后来快照名冒作当时构建身份；后续仅CMake的手工诊断链接和display探针变更，注册测试、公共头与产品实现未变。相同源码产品重新链接的.text SHA也一致，不能将这个事实冒称新二进制的整矩阵测试。

本包是本地开发快照，README封存前已列Runtime冷子进程退出、原生粘贴失败和Mesa边界，不宣布稳定发布、全部生命周期通过或长期体验完成。封存后只写外部证明，未回填SDK。

### 证据、保护与回退

[逐项状态账](lifecycle-ux-20261017-f6c42b91-evidence/raw/issues.json)、[完整性能表](lifecycle-ux-20261017-f6c42b91-evidence/performance.md)、[关键真实画面前/中/后与失败](lifecycle-ux-20261017-f6c42b91-evidence/visual-review.md)、[原始记录索引](lifecycle-ux-20261017-f6c42b91-evidence/sha256.json)。完整BMP、DMP和PDB在新build目录保留并记录SHA；Git证据含未裁剪RGB无损PNG、日志、源码/参数/manifest/CSV/JUnit及匹配栈。画面生成不自动算可读性验收；首次未paint/参数错误画面仍带原状态保存。

15份历史索引未变；17610条可按证据目录定位的文件记录全部匹配，包含143份当前语言证据及上一轮1800份证据。SDK内容索引初版错误套用证据根目录产生一个无效README比较，原校验记录保留；该SDK索引没有可确定内容根目录，改列未做该索引内容复验，不声称原件损坏。两份CRC故障BMP不读取、不替换。原固定A、44c039f SDK及599800c3 SDK三项SHA原样，6600入口未跟踪文件均仍存在。

安全回退只恢复本轮入口ZIP中的9个文件原字节（保留上一轮未提交内容），并在SHA完全匹配时移除唯一新测试源。新证据、报告、所有build与历史文件保留，不git reset、不clean。先执行只读检查：

```powershell
python -B build/lifecycle-ux-20261017-f6c42b91/tools/rollback-this-round.py --root .
# 只有明确要撤回本轮时才在同命令加 --apply；本轮未执行Apply。
```

### 下一步最小目标验证

Mesa目标同条件B/C执行器仅完成AST/本地生成计划检查，未远程执行：每provider默认3、LP0因果1、独立gate1，及各一次原Light存储/语言，匹配DLL/PDB/完整选项与实际native OS退出、EOF、CASE_END；首失败完整dump与匹配栈。其`--execute`要求明确远程授权和目标CDB。正常多线程screen清理/真正UNLOAD与259→0仍分账，不强释放未知引用。

Runtime新遗留的最小本地后续是预先限定冷进程样本，在首次10秒退出失败边界立即持句柄取证，关联浏览器角色、UserDataFolder、BrowserProcessExited、取消/释放及UI消息顺序；失败已消失前不作责任归因。原始失败不复跑覆盖、不给通过重试。当前没有充分依据作第二处产品修复，亦不扩大SYSTEM/物理/人工条件。

原生粘贴新失败的最小后续是固定Runtime/profile与输入顺序，在公共Win32 EDIT、公开Web原生input及FORM三路作预先限定次数对照，关联公开事件、selection、真实按键队列与Dispatch；不注入组件私有JS，不以增加延迟或后续通过解释首个失败。
