# 2026-10-09 本地界面查证、提示边界修复与验证

## 范围与实际基线

接手实际为干净 `main` / `82a774aaec41d0f6a0adc0f7c25107724ca94bc3`，没有用户未提交改动。SDK0.9.0-dev/API9/开发标准9、ABI1、包格式1、Windows x64/C11保持。44c039f、419290b、82a774a是说明/证据提交，原产品测试身份8765882和最后产品修复07a66a8仅作为历史依据。本轮最终源码身份为该HEAD加[准确代码补丁](local-investigation-presentation-evidence-20261009/raw/final-code.patch)，产物哈希见[最终身份](local-investigation-presentation-evidence-20261009/raw/final-identities.json)。各次输入及CTest清单另存当时的探针补丁和二进制哈希；产品menus.c在最终主输入、29项CTest及全部补充输入期间相同，探针后续只补长行输入校准、GL重新绘制和连续呈现画面。没有提交或新SDK归档；不能把脏工作区产物称为原82a774a干净源码产物。

新构建及完整原始画面保存在 `build/local-investigation-20261009/current`（修复前）和 `after`（修复后），未覆盖旧构建。没有推送、Git远端查询、托管工作流、发布、标签、业务应用/冻结SDK修改，未使用graph-engineering、子代理或新会话。缓存origin/main仍为b6a36fee，不能当作实时远端。

系统产品名Windows11家庭版中文版、版本10.0.26300、Session1、非管理员、MSVC19.50.35727、Runtime154.0.4258.62。窗口报告DPI96；单显示器有效DPI96、系统报告raw DPI108；程序96/144/192单独记录。UI culture为zh-CN、显示语言覆盖为空；偏好列表包含英文不表示实际英文Windows。固定Lexbor7fb22cf、QuickJS-NG2f0aa72、WebView2 SDK1.0.4129.50、Mesa24.3.4沿用，未prepare覆盖。实际记录见[环境](local-investigation-evidence-20261009/raw/environment-final.json)和[CTest二进制/源码身份](local-investigation-evidence-20261009/raw/after-targeted-summary.json)。

## 已有实现、复现与责任

历史依据分别为[已有界面修复验收](ui-repair-validation.md)、[实例语言当前验收](instance-language-current-validation.md)、[公共布局合同](../dialog-layout.md)。这些结果不计入本轮新测试。

| 问题 | 当前已有实现及历史覆盖 | 本轮结果与修改 |
|---|---|---|
| 浮动拖动/停靠/尺寸 | af36e02/e0dcf78/07a66a8已有独立反馈窗口、抓取偏移、取消、扩展边角命中及生命周期修复 | GL和草稿两组各38帧；GL最终43帧补验明确验证再浮动/关闭后重新打开仍能使用同一上下文绘制。连续按下/经过实际GL/左右底部目标/释放/取消/受控捕获丢失/尺寸调整/再浮动正确。尺寸390×240仅在明确调整后变440×290，抓取86/20偏移保留；八个边角7px内命中和方向光标通过。原内容HWND、草稿、焦点、GL上下文保留。未重写拖动实现。 |
| 公开FORM只读多行 | 已有98px默认高度、LF/CRLF范围、内层文字及外层字段滚动、更新清除旧布局 | Light/真实Runtime滚轮、实际滚动条、Ctrl+End最终可读第41行；42/65/160高度、350px窄窗、三行更新及第六字段可达。Ctrl+A/C在清空旧剪贴板后精确复制全部41行，真实输入不能改变只读值。Light长行Home/End可见；Runtime实际滚轮独立补验稳定显示第20行前缀和完整换行尾部。Runtime普通Home/End的行内目标另有定位限制，见下文。没有重复多行产品开发。 |
| 提示尺寸/生命周期/公开隐藏 | 短内容测量、范围预算、离开/更新/移动/拖动/停靠/关闭/销毁及ui_host_hide_tooltip所属范围已实现 | 短标题/短值尺寸正确，但原测试只比较宽高。程序192边缘提示完整矩形超出拥有者客户区，确认框架责任并修复；连带的提示覆盖区域点击拦截也有独立失败和修复。其他host提示保留，普通菜单保留/操作另验。 |
| 公共对话框布局 | 三个既有公共函数可表达首选/最小/最大客户区、visible_rows/逻辑height；单/多字段初高、长说明独立滚动、顶部按钮已实现 | 公共C探针及布局回归覆盖单字段、多字段、长说明/错误、只读/可编辑多行、窄窗。实际最小客户区240×120、最大600×230；模态拥有者禁用、Tab、普通Enter提交一次、多行Enter换行、Ctrl+Enter提交一次、Esc及取消按钮恢复拥有者通过。没有新增接口。 |
| 长可编辑文字End/Home | 07a66a8已有Uniscribe最后字符trailing计算修复，原红例和尾部强断言保留 | Light公共最小复现900/350px及程序96/192的START/TAIL标记和真实光标通过，保留18帧及几何。提示定位修正的中间版本曾导致点击未进编辑框，未归因于Uniscribe；修正提示命中后原强断言恢复。 |
| 泛化192叠层/窄表头残影 | 历史仅有调查，未宣称根治 | 无业务DLL的公共C程序保存宽→窄→同尺寸新建表头；两种程序DPI下像素一致，未独立复现残影。已区分本轮可复现的预期提示层与泛化叠层；后者仍尚未定位，未猜测修改。 |
| 历史持续重绘 | 既有阶段/实际Dispatch/每HWND更新区域和资源诊断 | 新构建检查空闲、打开/挂载、中文切换、错误打开/关闭、调整尺寸、显式GL帧及其后空闲。触发期确有绘制，最后空闲paint/queued/dispatch/wake为0；这只支持该触发链本机通过。旧中文断言失败已校准，不代表持续重绘根因。 |
| 瞬态Runtime | 既有异步清理、资源预算和正常CRT退出诊断 | 就绪后64次重开在原120秒测试超时和原handle/GDI/USER门槛内完成，pending清理0、实际退出0；不宣称全部历史瞬态问题根治。 |
| 原托管180/240秒超时 | 2fa6f84已有阶段、PID/tick/elapsed、running/exited和真实exitCode诊断 | 原CI只读分析可确定最后输出检查点，不能证明是哪一个进程退出或下一次启动卡住。根因仍尚未定位，本机通过不表示托管修复。 |

## 本轮提示红例与最小修复

修复前Light和真实Runtime均在程序192重现：拥有者可见客户区为`2218,1003..2552,1384`，提示实际为`2198,935..2532,1315`。旧宽高和监视器约束通过，新增完整矩形断言失败；两个进程各实际退出1。[原始红例](local-investigation-evidence-20261009/raw/tooltip-bound-red-manifest.json)保留命令、PID、二进制、代码差异及退出状态，失败日志未删除。

`src/menus.c`仅对detail提示把完整设备像素矩形夹在拥有者客户区与监视器工作区交集内；交集为空时公开显示返回NOT_FOUND，未启动生命周期timer。有效显示后使用原80ms生命周期timer。普通菜单沿用其原定位规则。

约束位置后，原提示会覆盖拥有者内的输入区域。真实显示探针发现点击后焦点没有进入编辑视图，192下五个原光标/尾部断言失败，实际退出1；简单子窗口HTTRANSPARENT处理也未解决，保留两份中间失败日志。最终detail窗口使用`WS_EX_LAYERED|WS_EX_TRANSPARENT|WS_EX_NOACTIVATE`和alpha255，画面仍完全不透明，输入穿过整层提示；回到普通菜单时去除这些提示专用style。此行为有[Windows layered hit-testing文档](https://learn.microsoft.com/en-us/windows/win32/winmsg/window-features)支持，具体通过由本轮真实点击、编辑焦点、末尾文本和进程退出共同证明。

最终192提示为`2218,1003..2552,1383`，部分拥有者出屏时为`2378,1003..2560,1383`，均落在实际可见客户区交集内；完全在工作区外时NOT_FOUND且没有可见提示窗口。短标题、短值、长值换行、拥有者移动/关闭/销毁、更新、内部浮动标题公开隐藏、独立host及普通菜单语义分别核对；未隐藏业务内容或关闭全部提示。

公共头文件、结构、版本号和三个布局函数没有新增变化。调用方继续使用`ui_host_hide_tooltip(host)`以及已有的`ui_component_set_dialog_layout`、`ui_component_get_dialog_layout`、`ui_component_set_field_layout`；例如先清零并设置size，指定preferred/min/max客户区逻辑尺寸，再为多行字段指定id及visible_rows或height。可运行公共C示例仍为[dialog_layout](../../examples/dialog_layout/main.c)。新构建包含匹配的headers、导入库、DLL、framework_host及应用包；冻结SDK保持原字节，不能混用库。

## 真实输入、画面与失败记录

用户明确确认保持解锁且桌面空闲。输入串行执行，默认输入桌面检查、所属PID/前台保护、真实Dispatch计数、命中类名/HWND、按下状态、捕获、焦点、presentation和最终状态、实际进程退出均记录。GL为Mesa24.3.4软件WGL实际窗口上下文；OSMesa/provider与Runtime阶段前移除仅部署在新构建的四个WGL DLL，没有扩大到硬件GL。

最终产品的[主输入进程记录](local-investigation-evidence-20261009/raw/complete-input-manifest.json)：两组浮动各38帧、Light/Runtime表单各56帧、显示18帧，共206帧，五个独立进程全部退出0。[长行独立补验](local-investigation-evidence-20261009/raw/long-line-input-manifest.json)两进程各32帧、[GL重新绘制首次补验](local-investigation-evidence-20261009/raw/gl-final-input-manifest.json)一进程40帧也实际退出0；这310帧仍含下文明确拒绝的早期空白画面，不以退出0宣布逐帧通过。最后[GL连续呈现](local-investigation-presentation-evidence-20261009/raw/visual-final-input-manifest.json)43帧、[Light/Runtime只读对话框呈现](local-investigation-presentation-evidence-20261009/raw/visual-dialog-input-manifest.json)39/63帧补全实际可读性，三个进程实际退出0。Runtime运行器的字符串替换意外选择完整公共表单路径，实际63帧而非计划的39帧；准确命令及未改写运行器保留，不再重复执行。

合计455份最终产品输入画面，另两个提示边界红例，共457份来源记录。完整2560×1440原BMP保存在新build目录；Git保存按真实拥有者区域裁切的原像素PNG及[主截图来源/坐标/哈希](local-investigation-evidence-20261009/raw/screenshots.json)、[补验截图来源](local-investigation-presentation-evidence-20261009/raw/screenshots.json)，无绘制补写或缩放。缩略预览仅供浏览，不能代替原尺寸可读性。三张拥有者完全在屏外的帧不生成伪黑图，因此两目录共454张PNG；完整桌面原帧仍本地保存。

首轮有外部浏览器遮挡，actual hit属于其他PID且Dispatch未增长；该轮无效，保留本地原始画面和日志，不作为框架失败。首个扩展Runtime套件退出0仍有未到末行/滚动条第38行/空白首次呈现，画面项被拒绝。随后校正扩展键标志、扫描码/修饰键Dispatch、清除旧剪贴板、滚动块真实端点，以及既有PENDING呈现等待；CtrlEnd动画用连续真实帧记录，不用扩大CTest超时或进程等待预算。旧帧和原失败均保留。

Runtime只读长行普通Home/End连续画面最终回到页首/页末，未证明行内起点/终点；该自动键序列的行内定位仍尚未确认框架责任，也不计为Light可编辑End/Home缺陷复现。原六格滚轮越过第20行，保留画面而不按文件名判通过。独立补验将Runtime输入校准为三格，四张连续原像素画面稳定可读LINE18–22、第20行前缀及`ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ`完整换行尾部；Light六格同样可读。第41行轮/条/CtrlEnd、只读复制及更新结果分别有效。

主GL序列在调整尺寸后未给面板主动提交新帧，再浮动截图中的白色不能作为GL内容呈现通过。首次补验明确make-current/clear/swap后再浮动已是蓝色，但关闭后重开的第一帧早于桌面呈现，仍不计通过。最终43帧补验保留四张连续重开画面，实际拥有者可见、同一上下文及蓝色内容均成立。只读Runtime对话框的主序列初帧也空白；最后补验等待既有公共field-text presentation返回就绪，再保存连续初始及CtrlEnd画面，完整第41行可读，提交/取消按钮仍可见。未修改框架绘制或生命周期代码，没有用接口成功替代可读性，也未提高等待/CTest预算。

自动SendInput与硬件鼠标人工验收分开；SetCapture/ReleaseCapture为受控捕获丢失，未验证外部软件抢占。显示探针明确区分公共presentation/view-local逻辑、宿主客户区、设备像素与screen；截图源、单实例所属PID和窗口DPI同时记录。

可直接查看的代表画面如下；完整连续序列及各帧状态仍以两个截图索引为准，单张代表图不能代替整项交互验收。

| 项目 | 原像素画面 | 对应输入及最终状态 |
|---|---|---|
| 按住经过GL/切换目标 | [经过GL](local-investigation-evidence-20261009/screenshots/complete-float-gl/007-held-gl.png)、[底部目标](local-investigation-evidence-20261009/screenshots/complete-float-gl/029-target-bottom.png) | [GL完整日志](local-investigation-evidence-20261009/raw/complete-float-gl.log) |
| 草稿及GL保留 | [再浮动草稿](local-investigation-evidence-20261009/screenshots/complete-float-draft/036-refloated.png)、[重新打开GL](local-investigation-presentation-evidence-20261009/screenshots/visual-final-float-gl/042-reopened-panel-gl.png) | [草稿日志](local-investigation-evidence-20261009/raw/complete-float-draft.log)、[GL呈现日志](local-investigation-presentation-evidence-20261009/raw/visual-final-float-gl.log) |
| Runtime长行/末行 | [第20长行](local-investigation-evidence-20261009/screenshots/long-line-runtime/015-readonly-long-line-wheel.png)、[滚动条末行](local-investigation-evidence-20261009/screenshots/complete-form-runtime/038-field-scrollbar-end.png) | [长行日志](local-investigation-evidence-20261009/raw/long-line-runtime.log)、[Runtime表单日志](local-investigation-evidence-20261009/raw/complete-form-runtime.log) |
| 只读模态/窄说明 | [Runtime对话框末行](local-investigation-presentation-evidence-20261009/screenshots/visual-dialog-runtime/038-readonly-dialog-key-end.png)、[Light完整说明尾部](local-investigation-evidence-20261009/screenshots/complete-form-light/049-dialog-help-end.png) | [模态补验日志](local-investigation-presentation-evidence-20261009/raw/visual-dialog-runtime.log)、[Light表单日志](local-investigation-evidence-20261009/raw/complete-form-light.log) |
| 提示短值/192边缘 | [短值7](local-investigation-evidence-20261009/screenshots/complete-form-runtime/050-short-value-tip.png)、[192边缘](local-investigation-evidence-20261009/screenshots/complete-form-runtime/052-edge-tip-dpi-192.png) | [提示红例](local-investigation-evidence-20261009/raw/tooltip-bound-red-form-runtime.log)、上述Runtime最终日志 |
| 窄编辑尾部/表头 | [192尾部](local-investigation-evidence-20261009/screenshots/complete-display-light/013-dpi-192-width-350-End.png)、[缩窄表头](local-investigation-evidence-20261009/screenshots/complete-display-light/016-dpi-192-header-resized.png)、[同尺寸新表头](local-investigation-evidence-20261009/screenshots/complete-display-light/017-dpi-192-header-fresh.png) | [显示日志](local-investigation-evidence-20261009/raw/complete-display-light.log) |

## 构建、相关回归与准确命令

两次独立Release/Ninja配置均完成404步，使用同一固定依赖；产品及探针迭代随后执行必要增量构建，build-after.log记录最后一次增量构建，不冒充最初404步日志。执行命令由新目录保存的ASCII CRLF批处理加载`VsDevCmd.bat -arch=x64 -host_arch=x64`，再运行：

```text
cmake -S . -B build/local-investigation-20261009/after -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_FRAMEWORK_ENABLE_WEBVIEW2=ON -DUI_OSMESA_LIBRARY="%CD%\.deps\mesa-24.3.4\x64\osmesa.dll"
cmake --build build/local-investigation-20261009/after
python -B build/local-investigation-20261009/run-complete.py
python -B build/local-investigation-20261009/run-after-targeted.py
python -B build/local-investigation-20261009/run-long-line.py
python -B build/local-investigation-20261009/run-gl-final.py
python -B build/local-investigation-20261009/run-visual-gl-final.py
python -B build/local-investigation-20261009/run-visual-dialog-final.py
powershell -NoProfile -ExecutionPolicy Bypass -File tools/test-windows-ci.ps1
python -B build/local-investigation-20261009/preserve.py after
git diff --check
```

运行器对CTest使用已解析的绝对路径、`-j1 --output-on-failure --output-junit`和精确测试名正则，校验每项名称、缺失/重复/失败/跳过以及实际进程退出。日志、JUnit、准确命令和二进制哈希在新证据目录；没有重新执行全部历史64项矩阵或声称物理条件跳过属于本轮通过。

| 阶段 | 修复前新构建 | 修复后新构建 |
|---|---:|---:|
| WGL/公共头/布局/完整范围滚动/菜单 | 15通过 | 17通过 |
| provider/完整应用Light存储与语言/重绘 | 6通过 | 7通过 |
| 真实Runtime布局/宽度/滚动/视觉/64重开 | 5通过 | 5通过 |

修复前26项全部通过，只作本轮前基线；最终产品29项通过、0失败、0跳过，三个CTest进程均实际退出0。修复后另外包括普通桌面菜单、离屏菜单及级联菜单，用于验证提示专用style退出后的菜单语义。Runtime64次就绪重开后handles361→358、GDI13→13、USER17→17、pending cleanup0，并记录正常CRT退出。原完整范围滚动、uint64/BigInt、按需查询、原断言、超时和资源预算均保留；没有增加资源容许值、删除失败或恢复分页。

## 原CI与尚未定位项目

[原CI最后阶段摘录](local-investigation-evidence-20261009/raw/original-ci-terminal-stages.json)来自只读原437份材料，具体并非同一终点：

| 原artifact | Light存储 | Light语言 |
|---|---|---|
| main-webview2 | 180.019s超时，最后profile-b输出0 failures | 240.015s超时，最后restore96输出failures0 |
| main-osmesa | 34.6911s通过 | 240.024s超时，最后restore192输出failures0 |
| branch-webview2 | 180.023s超时，最后write192输出0 failures | 70.2146s通过 |

旧b6a日志没有各case实际exitCode及running/exited边界，因此无法区分该进程退出卡住与下一个进程启动卡住；部分JUnit输出还有原1024字节截断。最近的2fa6f84诊断已覆盖本机各阶段，不应重复新增另一套诊断或据此宣称托管根因。要确认目标托管环境需另行明确远程授权；本轮没有提高180/240秒上限、加sleep、扩大资源预算或去掉断言。

泛化192叠层、窄表头残影、历史持续重绘的完整触发链、历史瞬态Runtime根因、上述托管超时仍尚未定位或尚未独立复现。Runtime只读长行普通Home/End行内定位未确认，实际长行可读性已由独立滚轮画面证明；不能合并为已解决的Light Uniscribe问题。

## 保全、文件改动与剩余条件

新增证据目录与既有材料分开。主目录封存后追加的呈现补验使用第二新目录，不覆盖先前raw或空白帧；[主哈希索引](local-investigation-evidence-20261009/sha256.json)459份记录、[补验哈希索引](local-investigation-presentation-evidence-20261009/sha256.json)157份记录均逐文件核验。[历史逐文件保全](local-investigation-evidence-20261009/raw/preservation-after.json)重新核对1265份记录、8个SDK归档，差异0。包含143份实例语言证据、18张PNG及对应原BMP、437份原CI材料、既有界面证据及封存SDK。最终44c039f SDK仍117891276字节、3602文件、SHA256`28ca0b8409b321477f1aa3adce2680fda8cf2cd8a9a35dd4308a5f08ef1f79a2`；未补写本轮内容。

产品仅[menus.c](../../src/menus.c)提示完整矩形、不可见拥有者及命中穿透修正；[interaction_probe.c](../../tests/interaction_probe.c)扩展真实输入/可读性/呈现证据，[display_probe.c](../../tests/display_probe.c)补Dispatch、所属PID、扩展键标志及触发前画面，原断言保留。[开发标准](../application-development-standard.md)3.3、[构建说明](../build-and-validation.md)2.1/视觉段/当前声明校正为API9；标准末段补准确提示范围及命中语义。历史API8版本验收事实未修改。新报告、两个新证据目录及`.gitattributes`的原字节保全属性用于本轮可审查交付；本地运行器/原BMP/构建/规划记录均在新build目录。无相邻重构、新存储、语言开关或公共接口。

以下继续独立待验：当前源码Session0服务、严格整机无登录、实际英文Windows空宿主、真实中文IME、物理混合DPI、桌面合成专项和长期人工稳定性/硬件鼠标。当前仅Session1非管理员、一个有效96DPI显示器；程序192、受控语言映射、自动输入、历史b6a Session0 GL均不能替代这些条件。没有注销用户、修改现有服务、降低无登录门槛或扩大硬件GL范围。
