# P1–P6 本地根因查证，2026-10-09

本轮确认并修复一处产品缺陷：完整范围表格在数据末端错误地把两行可见数据计为0。P6只读普通Home/End观察由同Runtime原生textarea对照解释为浏览器行为与探针起点期待错误。P1托管超时、P2泛化叠层、P3历史残影、P4历史持续重绘及P5未归因Runtime异常继续分别保留，当前通过没有替代根因证据。

## 1. 执行基线、身份与保全

实际分支main，HEAD `82a774aaec41d0f6a0adc0f7c25107724ca94bc3`，执行前已有六个tracked修改和上一轮报告/两目录。初始完整tracked补丁SHA256为`9c945c7c9999c81ccda2e204c4617db68aa8fcc9d77c20373fa4f797b2311a29`；前轮源码、公共头和产物逐项匹配。[初始核对](root-cause-investigation-evidence-20261009/raw/baseline-identities.json)、[初始补丁](root-cause-investigation-evidence-20261009/raw/baseline.patch)与[最终完整身份](root-cause-investigation-evidence-20261009/raw/final-identities.json)区分各次dirty源码和PE，不把本地构建标为干净82a产物。新增未跟踪诊断源码逐文件绑定在source目录及SHA256索引中，不能只用tracked补丁识别它们。

新构建在`build/root-cause-investigation-20261009/current`。SDK0.9.0-dev/API9/标准9、应用ABI1/包格式1不变，公共头未改，无新公共接口。最终产品DLL2190848字节，SHA256 `ea42c07618f4e6e2c801073c93ac0ac5cb1c9142f7034ce9d80d1454c67685d1`；导入库`ui_framework_runtime.lib`61690字节，SHA256 `fa14d15ec556363c2af99b176b24926bb9e1b5e1eedad4d3dba78fa9cd44ab0a`。宿主257024字节，SHA256 `43661413b1e1165fb2faf39a2c5fe8a9b43a6530400f2e9b4ae939f7bf944773`；完整示例包38799字节，SHA256 `e6667beb3de3e20a5c01109b0d0fdcb99be7c6e9d2f97b9ae200483fc3d92024`。所有调用方、DLL、静态库与应用包见完整身份，逐次运行另有patch/identities/source-snapshot。

Windows家庭版中文版10.0.26300，Session1，非管理员，实际Windows UI语言zh-CN，Runtime154.0.4258.62、MSVC19.50。一个2560×1440显示器，有效DPI96/raw108；窗口报告96，程序192另记。[独立环境核对](root-cause-investigation-evidence-20261009/raw/environment-confirmed.json)。最初environment.json有系统PowerShell编码乱码，原件保留，以后续独立pwsh核对为准。Lexbor7fb22cf、QuickJS-NG2f0aa72、WebView2 SDK1.0.4129.50及Mesa24.3.4维持固定版本，依赖Git工作区干净；未执行prepare或升级。

已读最新两份证据目录及报告、handoff21–25、ui-repair、ci-recovery、component-experience、api8-stability、Runtime合同和相关源码。旧29项/455帧只作历史。历史保全覆盖7830文件/28017059122字节，包括原143份、18张PNG、437份CI材料、完整原BMP、旧local-investigation构建和两目录。最终核验见[前清单](root-cause-investigation-evidence-20261009/raw/preservation-before.json)与[后核验](root-cause-investigation-evidence-20261009/raw/preservation-after.json)。冻结44c039f SDK保持117891276字节、3602文件和原SHA256 `28ca0b8409b321477f1aa3adce2680fda8cf2cd8a9a35dd4308a5f08ef1f79a2`，未补写本轮内容。业务、冻结包、PERF-001、历史raw和失败均未作为输出路径。

## 2. 问题来源及判定标准

执行前[来源与假设表](root-cause-investigation-evidence-20261009/raw/sources-and-experiments.md)记录每项候选、实验及排除条件。来源没有记名时统一为“原始观察者未记录”，不从Git作者推断；P6明确观察者为上一轮Codex。

| 项目 | 最早追回的材料/原操作 | 证据强度及成功标准 | 当前分类 |
| --- | --- | --- | --- |
| P1 | 原CI437份、各artifact ctest.xml/LastTest/test-plan、original-ci-terminal-stages.json | 有真实Timeout；须定位所属进程和具体阻塞操作，原门槛同序列复验 | 本地相关停止前缀退出，目标CI仍待授权，根因未定位 |
| P2 | handoff23、ui-repair调查段、前轮公共探针 | 目前只找到历史描述，症状原图/操作顺序未追回；须公共调用方独立重复错误层并解释坐标或生命周期 | 有界排查未复现，不能判根治 |
| P3 | 历史描述；独立f0b161c红例及前轮resized/fresh对照 | 列序/调宽捕获红例已修，不能混作泛化残影；须重复同一旧像素与空间差异 | 历史残影未定位；本轮另发现并修复末端页脚计数 |
| P4 | component-experience raw/features-pump-red.log，30.05s Timeout | 真正绘制计数1983，但无HWND/失效栈；须追到重复产生下一次失效的闭环 | 当前原触发链未复现闭环，历史根因未定位 |
| P5 | API6/7资源、导航/清理日志及instance-language raw/focused-retest.log | 多种独立失败，须分别解释生命周期和真实退出，不把PENDING当失败 | 既有close gate/焦点修复保留；当前定向路径通过，残余历史异常未归因 |
| P6 | 前轮interaction_probe CtrlHome→19Down→普通Home/End | 没有DOM起点/选择证据；须相同textarea对照与逐键状态确定责任 | 已解释为测试期待及浏览器正常只读行为 |

## 3. P1：原托管超时的进程边界

三个artifact仍分开记录。main-webview2存储180.019秒，最后profile-b失败数0；语言240.015秒，最后restore96。main-osmesa存储34.6911秒通过，语言240.024秒，最后restore192。branch-webview2存储180.023秒，最后write192；语言70.2146秒通过。原源码b6a36fee、Windows10.0.20348、Session2、镜像20260927.320.1、1920×1080/96DPI；观察者未记录。直接来源在`build/github-sync-20261008`，摘要只读保留于[原终点表](local-investigation-evidence-20261009/raw/original-ci-terminal-stages.json)。

原父子链为CTest→PowerShell7→ui_stateful_components_test/ui_instance_language_test→宿主/应用DLL；真实Runtime可再创建browser子进程。原计划工作目录`D:/a/ui-framework/ui-framework/build/ci-webview2`或`ci-osmesa`，命令为PowerShell7 -NoProfile -File tests/run-stateful-components.ps1（或run-instance-language.ps1），-BuildDirectory对应构建、-Provider固定osmesa.dll、-Backend light。原RUN_SERIAL与TIMEOUT180/240不变。

当前Run-Case沿用2fa6f84：CASE_BEGIN/manifest running→原`& exe ... 2>&1 | Tee-Object`→流水线结束后才写LASTEXITCODE、elapsedMs、exited和CASE_END。C侧close-begin/close-request-return/close-state-checked/COM-uninitialize-begin/process-return覆盖到return前；末行failures0仍不能证明CRT/OS退出或输出EOF。跨进程存储锁、启动/等待、输出句柄和卸载/COM路径未找到可确认导致原Timeout的错误；候选“孩子仍活着”“父已在管道等EOF”“下一孩子未启动”“DLL/COM/CRT阻塞”均保留待验。

新外部观察器保留原Tee与实际Dispatch，记录父/子PID、OS真实exitCode及stdout EOF，查询句柄属于观察器，不计入产品资源。只重放原停止点相关存储前缀9进程（96/144/192 write/restore、isolation、profile-b、reload）和语言原7进程，分别15.578/66.687秒：孩子9/9及7/7退出0，CASE_END齐全、父退出0并EOF。[进程边界](root-cause-investigation-evidence-20261009/raw/p1/boundaries-summary.json)。它们在页脚修复前的本轮产物运行，精确身份见各trace对应identities，不冒称最终DLL重测。

最初同目录source-snapshot使语言run覆盖了前缀run的CMake副本；原前缀patch和hash仍在。按后续保留的混合换行字节移除已记录新增的三行Runtime probe配置，派生副本与前缀原CMake SHA256完全相符，见[最终核对](root-cause-investigation-evidence-20261009/raw/final-checks.json)及raw/p1/recovered-from-patch。该副本明确标为派生，不冒称当时原始快照；其他已保存源hash一致。最终观察器为每trace使用独立snapshot目录，并在任何输出已存在时先拒绝，已有证据不会被重复运行覆盖。

合成正对照父进程78ms退出，继承管道holder515ms才EOF，证明两者独立记录。[管道对照](root-cause-investigation-evidence-20261009/raw/observer-control/processes.jsonl)。停滞抓取默认关闭；可显式开启60秒停止输出后外部顺序MiniDumpWriteDump，保存线程栈/模块/句柄信息，不改产品等待或超时。合成存活holder转储34695字节成功；第一抓取遇进程已经退出而失败，原失败保留。使用外部进程及顺序DbgHelp调用符合[Microsoft合同](https://learn.microsoft.com/en-us/windows/win32/api/minidumpapiset/nf-minidumpapiset-minidumpwritedump)，DumpType0x1004范围见[MINIDUMP_TYPE](https://learn.microsoft.com/en-us/windows/win32/api/minidumpapiset/ne-minidumpapiset-minidump_type)。转储和25ms旁观采样可能影响时序，不能当无诊断验收。本地没有产品停滞，未取得产品异常线程栈。

目标CI的可审查补丁及明确授权范围见[CI方案](root-cause-investigation-evidence-20261009/ci-authorization-plan.md)。仓库工作流/CI工具未实际修改，也未查询远端或执行工作流。

## 4. P2/P3：几何、旧像素与独立页脚缺陷

已追查本地交接、旧验收、两轮探针画面和证据索引。P2泛化叠层/P3窄表头残影的原失败图、列参数/顺序、操作时序和具体版本仍未追回，只有历史描述。已修提示层、合法省略号和f0b161c问题不算原症状。旧重排红例Name表头x128/单元格x8；旧第二次调宽360后重挂表头导致捕获取消退到280，均属已修具体缺口，本轮不恢复旧代码。

正常编译的presentation_transition_probe只使用公共C接口，不加载业务。8个稳定列ID/边界标记，h列宽由180改280，总量9007199254741051保持uint64/BigInt；按需完整提供所请求窗口，不以两行缓存冒充全数据。执行同一实例96→192宽→192窄→末列/末行→稳定ID行与标题更新→192宽/窄→同状态新建对照→浮动FORM更新/停靠→96窄/宽。每步保存SetWindowPos、ui_host_set_dpi、ui_host_resize、reflow的参数及先后次序、presentation/clip、slot-host逻辑/设备坐标、screen原点、HWND/PID/父/owner/visible、输入命中与Dispatch。浮动/停靠调用公共布局API，是几何/内容保留实验，没有将它冒称硬件鼠标拖动验收。

例如窄窗客户区334×462设备像素，程序192时view逻辑167×231，窗口/显示器仍96；表头view-local逻辑y40/h28对应设备y80/h56。输入由view-local逻辑按程序DPI转换设备后ClientToScreen，日志明确区分坐标。实际鼠标命中scroll thumb；Runtime preventDefault路径不会因点击自动建立DOM键盘目标，早期End落在body。因此最终序列使用公共dispatch_input明确建立thumb焦点、公共presentation确认focused，再以真实扫描码扩展End验证完整范围。它是公共焦点准备加真实输入，不是纯硬件点击聚焦结论；未为此改变产品浏览器事件语义。

有效最终Light/Runtime均进入firstColumn7和first9007199254741049，末端两行实际可读、稳定内容V2；表头/单元格对齐，浮动更新/停靠保留同内容HWND与generation。缩窄后与同状态新建表头各74816字节（334×56 RGBA），逐像素差异0、无差异坐标，原完整RGBA及桌面BMP保留。这排除所执行路径的静止残影，未证明所有历史操作或物理高DPI情形。

本轮独立红例：尾部查询缓存为2行前缓冲+2行实际数据，原模板无条件再扣不存在的2行后缓冲，页脚显示`0 / 9007199254741051`。修复只去掉额外`+2`，保留span上限和其他完整范围/资源语义。修复前DLL SHA256`c5dafeef80c67efe52cceab770a50ead82b9e47d7620e3cc1e55a357009fe6d6`与同一个最终公共调用方SHA256`2afee3c1fe7e7e4a5c0eacdaf886543189025c8a6951631370e01716baae3008`分别产生Light9失败/Runtime9失败，OS退出1；替换为最终修复DLL后两后端均0失败/OS退出0，9处显示精确范围`9007199254741050–9007199254741051 /`。红例是旧DLL与当前同API调用方的受控组合，各actual_command_files准确记录；没有用当前源码冒充旧DLL源码，旧components.html另存。旧红例静态库尚存，旧导入库未单独封存，未伪造它的身份。

红绿链见[Light红](root-cause-investigation-evidence-20261009/raw/footer-red-light-same-caller/processes.log)、[Runtime红](root-cause-investigation-evidence-20261009/raw/footer-red-runtime-verified/processes.log)、[Light绿](root-cause-investigation-evidence-20261009/raw/footer-fixed-light-final/processes.log)、[Runtime绿](root-cause-investigation-evidence-20261009/raw/footer-fixed-runtime-public-focus/processes.log)。页脚修复没有解释旧像素残影或泛化叠层；原两项保持尚未定位。下一最小实验是在取得历史列/顺序及叠层对象信息后，仅向同调用方添加对应顺序，再取几何、失效源和像素差异。

## 5. P4：真实绘制与历史闭环

直接旧红例[features-pump-red.log](component-experience-evidence-20261007/raw/features-pump-red.log)第65行：一个pump16204ms、messages3001/timers0/paints1983，CTest30.05秒Timeout。原链为双实例→Edit操作→激活第一实例/layout/flush→CtrlO→Alt/Esc→1920/1280/800/640及96/144/192矩阵→tool-more/overflow；观察者未记录，产品/测试阶段见component-experience原身份。日志缺少具体paint HWND、更新区及下一次失效调用栈，不能恢复原闭环。

当前features测试仅在UI_FEATURE_PAINT_TRACE开关下记录按HWND分组的queued/sent WM_PAINT、前后更新区、窗口位置、HOST_REFRESH/WORKSPACE_WAKE；所有取出的消息仍Translate/Dispatch。复用原操作链，前后1秒空闲和主动Invalidate正对照，原30秒门槛不改。诊断运行14.45秒通过；菜单链后14次paint有界，尺寸链后0，主动Invalidate实际paint1。[完整诊断](root-cause-investigation-evidence-20261009/raw/p4/features-diagnostic-valid.log)保存各阶段及更新区，没有观察到再次持续失效闭环。

最终普通构建关闭诊断开关，features12.50秒通过；host_repaint13.41秒通过：三段2秒空闲和GL后2秒空闲均paint/queued/Dispatch/refresh/wake0，resize和显式OSMesa GL产生有界非零刷新，更新区排空，原语言精确断言及资源保留。旧中文断言、消息泵越界/丢Dispatch修复、hook漏队列paint均已分案，不作为持续绘制根因。下一次若当前原链再现持续状态，应按异常HWND及阶段对Invalidate/布局/refresh来源取栈，构造单一触发者对照；当前日志不支持猜测产品补丁。

## 6. P5：Runtime异常逐项拆分

| 历史分支及直接材料 | 已知责任/已有修复 | 本轮证据及剩余缺口 |
| --- | --- | --- |
| API6基线64：384→491，最小对照384→453；drain对照384→405；导航/document/retry红例 | PSS已退出browser句柄与宿主持有句柄分别计；21e94e之前已修close gate、内部导航完成与回调外Close/BrowserProcessExited释放 | 阅读原实现与合同，保留既有修复；当前有效PENDING/快速销毁及回调取消通过，不把历史全部差值归为一个原因 |
| 历史菜单资源374→416、同代码386→387 | 原始观察者未记录，间歇增量未归因 | 本轮新64快速重开及资源对照通过，只支持对应路径 |
| API5原包358→395/pending0、独立354→378；SystemUserAdapter/CoreMessaging/CoreUIComponents | 2c0c52b已修独立异步modal焦点缺口，创建栈属于Windows输入设施；不能据此解释全部内核句柄 | 原日志只读，不重启冻结旧包兼容专项；原资源异常继续未定位 |
| instance-language focused-retest render30.02秒，ready469/78ms后close，!runtime_cleanup_pending失败 | 有直接清理失败/Timeout，缺真实OS退出、线程栈；不能由输出位置判断CRT阻塞 | 当前render同原30秒门槛9.13秒通过，pending关闭x12资源280→280/USER17→17/pending0；历史一次异常仍未归因 |
| runtime-first外部CoreWindow/LockScreenBackstopFrame命中，语言断言失败 | 桌面/输入归属失败，独立于控制器生命周期 | 当前命中检查无此干扰，不宣称消除了外部抢占 |

各历史原文件/身份在[runtime-stability-validation](runtime-stability-validation.md)及[语言旧验收](instance-language-validation.md)。上述失败不合并标签，不从作者推断观察者。源码异步创建排队、dispatch_blocked门控、失效view内部导航/Controller Close、browser退出释放、取消脚本回调均核对，没有找到本轮可确认的新产品生命周期错误。

新runtime_boundary公共调用方先建立正常暖基线，然后在首次pump前create/load/destroy（ready0/nav0/PENDING合法），旧cleanup仍pending时重建backend/view；正常脚本回调1次，接受第二请求后销毁view/backend，取消后没有迟到应用回调，清理窗口归零，宿主/窗口销毁、COM及CRT结束。暖基线237handles/GDI0/USER6，最终236/GDI0/USER4，原+12/+4通过，OS退出0。[操作边界](root-cause-investigation-evidence-20261009/raw/p5/boundary.log)。初始冷112→暖237是初始化对照，未把它作为暖资源循环泄漏。

原api7_integration选择UI_RUNTIME_CYCLES64，未启用UI_RUNTIME_READY_REOPENS，实际日志0–63是快速重开，不能称64次就绪重开。诊断版361→361/GDI13/USER17/pending0，正常最终版360→360/GDI13/USER17/pending0，逐轮原WAIT/DLL卸载及OS退出0。正常消息、render、repaint、features及两后端scroll测试都保留原超时和资源预算；没有增加sleep或放宽条件。仍需在相应目标/触发链重现残余异常后取回调/线程栈及对象等待关系；本轮通过不结清历史瞬态问题。

## 7. P6：只读Home/End的真实目标和原生对照

固定41行、LINE20长行、FINAL41，值长度1649、wrap属性、只读、13px Segoe UI、618×98外框；textarea客户604×96、scrollHeight875。原生对照在同Runtime/同窗口，复制实际computed CSS/内容/几何，匹配框架12px scrollbar，避免初始原生15px滚动条造成601/604宽差。逐键观测实际DOM activeElement/target、选择起止/方向/逻辑行、scrollTop/Left和Win32前台/命中、焦点、低层扫描码/扩展键/修饰键。被动监听不preventDefault；explicit selection只为独立矩阵设置起点，实际导航键仍SendInput并Dispatch。

原序列实际点击后selection1558（逻辑第38行），CtrlHome和19Down没有把只读选择改为第20行；Down后scrollTop760。普通Home仍selection1558但scrollTop0，普通End仍selection1558而scrollTop779。这与相同原生textarea逐项一致。明确第20行折叠/已有选择时，只读普通/Ctrl导航也与原生一致；编辑模式普通Home/End落在长逻辑行的视觉换行段418/504，CtrlHome0/CtrlEnd1649。不能为了“行内定位”假设改写只读浏览器语义。

[诊断对照](root-cause-investigation-evidence-20261009/raw/p6-diagnostic-final/comparison-complete.json)38组最终稳定状态的选择起止/方向、滚动、字体、宽高及实际焦点行为一致（原序列22+矩阵16）。完整rect核对保留8组编辑模式的原点差异：FORM y102、裸textarea y66，宽高618×98仍相同；FORM可编辑时显示28px操作栏及8px间距，原生无此外围布局，未把原点不同抹成“所有状态完全相同”。30组只读rect一致。按键后的80ms中间帧原样保留；后续依据4个RAF scroll位置稳定、原5秒观测门槛判最终状态，不把平滑滚动中间位置当最终错误。该诊断借测试内部getter读取实际公共FORM view，再用现有execute_script取DOM，没有新增产品API。native CSS镜像和观测脚本有时序/布局影响，所以另做无低层hook/无按键监听的原序列22组0差异；此路径仍有只读DOM快照/RAF观察，不能称完全无诊断。

最终未添加DOM脚本/监听/内部getter的原公共interaction调用方`long-line-webview2`在当前同源码静态构建上11.656秒、failures0、实际OS退出0，验证既有长行滚轮、完整末行、复制/更新和公共对话框路径。该调用方链接ui_framework.lib，不将同时记录的DLL哈希冒称实际加载身份；新readonly/presentation/runtime_boundary三个probe链接DLL。该运行是指定模式，未重复或冒称全部历史表单矩阵。[正常公共日志](root-cause-investigation-evidence-20261009/raw/p6-public-current-product/processes.log)。仅校准原探针注释及readonly-ordinary-Home/End帧名，不删除断言，不修改产品键盘语义。P6责任已解释，硬件键盘和其他Runtime版本不由本次自动输入替代。

## 8. 本轮运行、画面和失败保留

首次Release/Ninja/C11 x64完整新构建成功；固定依赖及OSMesa/WebView2显式配置在[build.cmd](root-cause-investigation-evidence-20261009/raw/build.cmd)与final-CMakeCache。后续仅因本轮修改重建。CMake重新configure会重写生成头/资源并改变PE时间戳，各次hash独立记录；最终探针使用build-only target，不混用早期DLL身份。

| 有效新验证 | 命令/模式及范围 | 结果/实际退出 |
| --- | --- | --- |
| P1存储/语言 | p1-state-prefix.ps1（9进程）、原p1-language.ps1（7进程），固定Provider、Backend light | 15.578/66.687秒，全孩子/父exit0，EOF；非完整历史矩阵 |
| P2/P3最终公共调用 | ui_presentation_transition_probe OUTPUT light/webview2；两后端同序列/同caller | 修复前9/9失败、exit1；后Light4.312秒/Runtime10.984秒0失败、exit0；各33帧含原中间帧 |
| P4原链诊断 | ui_framework_features_host，UI_FEATURE_PAINT_TRACE | 14.45秒，1/1，原30秒；14/0空闲、主动1 |
| P5边界/fast64诊断 | ui_runtime_boundary_probe；api7现有fast64 | 0失败/CRT与OS0，资源及pending通过 |
| P6匹配/无监听 | readonly_navigation_probe diagnostic及无监听模式 | 行为38/38、22/22一致，编辑外围原点差异8组另记，0失败，OS0；各84/20原尺寸帧 |
| P6当前普通路径 | ui_interaction_probe OUTPUT long-line-webview2 | 11.656秒，0失败，OS0；无新增DOM诊断 |
| 最终普通相关CTest | 精确7项计划，UI_RUNTIME_CYCLES64；无RESOURCE/PAINT/FEATURE诊断 | 7通过/0失败/0跳过，92.80秒，父与7个native子进程exit0 |

最终7项分别ui_webview2_messages0.496s、ui_webview2_render9.128s、ui_component_scroll0.468s、ui_component_scroll_webview28.749s、ui_host_repaint13.411s、ui_api7_integration_webview248.014s、ui_framework_features_host12.502s；[计划](root-cause-investigation-evidence-20261009/raw/final-regression/plan.json)、[JUnit](root-cause-investigation-evidence-20261009/raw/final-regression/results.xml)、[全日志](root-cause-investigation-evidence-20261009/raw/final-regression/processes.log)及外部OS边界共同核验数量、模式、结果和退出。

messages及两项component_scroll链接同次静态ui_framework.lib，其余上述4项链接当前DLL；最终完整身份同时列静态库和导入库，不能将库类型混用。窄窗中的17位长数字页脚会换行并部分超出当前客户区；本轮页脚计数验收依据公共精确文本及宽窗完整真实画面，不把窄窗DOM文本正确当作完整页脚可读性通过。这项窄窗极长数字呈现限制保留，没有扩大为未经定位的表头修复。

全部本轮1003份原BMP（14476829874字节）保留在新build，其中960份是2560×1440完整桌面，43份是原调用方的窗口/表面截图；均按原尺寸无损转PNG，逐张解码RGB一致，路径/大小/SHA及对应PNG在[bmp-originals](root-cause-investigation-evidence-20261009/raw/bmp-originals.json)。索引包括早期错误/中间/不合格画面，不能把1003张都称验收通过。每序列帧号连续，实际FRAME记录tick/Dispatch/foreground或root/hit/焦点/捕获；采样是连续阶段截图，不冒称视频或硬件鼠标。P1 current-runs中的窗口截图按原来源和设备矩形记录，不能与全桌面probe BMP混用裁切坐标。

所有失败/无效实验保留：[完整运行边界索引](root-cause-investigation-evidence-20261009/raw/run-boundaries.json)。P1首用系统powershell缺Get-FileHash，未启动产品；P4首次ctest不在PATH，未执行测试；P6首轮native为空/焦点不足、后续80ms尚未稳定、第三轮scrollbar宽度不匹配均撤回产品结论；最早P6失败PE未在覆盖前单独封存，只有当时源码/日志/画面，不补造精确PE身份。公共浮动初试entry_url为空指针和未shell_refresh违反/缺公共调用步骤；早期表源只供2行、几何检查未排除不可见header导致级联失败；Runtime部分End实际在body、一次诊断编译失败后旧binary误执行、Tab聚焦实验副作用都按无效探针记录，不当产品根因或末端通过。这些纠正不删除任何原失败。

## 9. 文件影响、结案及剩余条件

| 本轮文件 | 对应问题/影响 |
| --- | --- |
| src/components.html | P3独立确认的末端页脚计数，一处表达式；不改滚动、数据、预算或公共API |
| tests/presentation_transition_probe.c、CMakeLists.txt | P2/P3公共几何/完整数据/旧像素对照；诊断DOM编译宏默认关闭；3个手工probe不加入CTest矩阵 |
| tests/framework_features_host.c | P4可关闭的按HWND真实Dispatch/更新区和正对照；普通用例原30秒不改 |
| tests/runtime_boundary_probe.c | P5合法PENDING/取消/重建/回调与原资源预算；无产品诊断接口 |
| tests/readonly_navigation_probe.c | P6测试专用DOM/原生对照，明确内部getter仅观测 |
| tests/interaction_probe.c | P6注释/帧名校准；上一轮修改和原断言保留 |
| tests/observe_processes.py、tests/process_stall_capture.py | P1外部进程/EOF边界及可选线程转储；默认不开停滞抓取 |
| .gitattributes | 为本轮证据目录追加保持原字节规则，已有历史规则保留 |
| 本报告及新evidence目录 | 来源、失败、坐标、逐次身份和画面索引；旧文档/目录未覆盖 |

上一轮.gitattributes、API9两文档校准、menus提示完整矩形/穿透修复、display_probe及interaction旧修改均保留。菜单/公共头/业务/历史文件没有本轮产品修改；已交付浮动、多行、公共dialog/field layout、实例语言和资源修复不重复开发。

结案分为五类：确认缺陷且红绿验证为末端页脚计数；已解释的期待/默认行为为P6及Runtime滚动键目标准备；当前有界未复现为P2/P3历史显示、P4原链、P5已列当前生命周期；历史直接材料不足为P2/P3原症状图/具体顺序与P4失效闭环栈；目标CI仍缺P1进程/管道/等待栈，须新明确授权。没有把“当前通过”写为其他历史症状根治。

当前源码Session0、严格整机无登录、实际英文Windows、真实中文IME、物理混合DPI、桌面合成专项、长期人工、硬件鼠标/键盘及外部捕获抢占分别待条件。未注销、修改服务或降低门槛；未以程序192、受控映射、自动SendInput或本地通过替代。未提交/推送、查询Git远端、触发CI、发布或创建标签。本轮没有封存新SDK，当前产物以dirty补丁与哈希交付。
