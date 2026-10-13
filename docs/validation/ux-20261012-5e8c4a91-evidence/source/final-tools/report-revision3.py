import pathlib,json,csv,xml.etree.ElementTree as ET
root=pathlib.Path(__file__).resolve().parents[3];base=root/'build/ux-20261012-5e8c4a91';ev=root/'docs/validation/ux-20261012-5e8c4a91-evidence'
data=json.loads((base/'raw/performance-summary-v4.json').read_text(encoding='utf-8'))
lines=['# 2026-10-12 本地按钮、焦点与体验查证','', '记录：Codex；标识：`ux-20261012-5e8c4a91`。仅本地，未提交／推送／触发远程工作流。','',
'## 结果和身份','',
'宿主 glyph 水平居中、悬停与按下区域稳定；鼠标保留内部焦点而无虚线，Tab 使用细实线。Light 显式实现输入模态、受控 `focus-visible`／`outline`，真实 Runtime 使用共同 CSS。未加载共同 CSS 的 Light 公共页面仍保留默认键盘实线环。没有修改公共头文件或新增 API，SDK0.9.0-dev／API9／标准9、ABI1／包1保持。','',
'接手 HEAD `7848a946fcf417eefe87bd63e42f83c8a3f875f3`、分支 `codex/p1-process-boundaries`，初始 tracked clean，11个既有未跟踪路径保留。当前为未提交工作树，不能冒称新 commit：精确差异、源码和产物 SHA 见 [身份](ux-20261012-5e8c4a91-evidence/final-identity.json)。构建均在 `build/ux-20261012-5e8c4a91/`，不封装或补写历史 SDK。','',
'Windows10.0.26300、Session1／Default 解锁桌面、2560×1440；实际 Windows／窗口报告 DPI96，程序分别设置96／144／192。MSVC19.50.35727.0；实际 WebView2 Runtime154.0.4258.37。固定 Lexbor、QuickJS、WebView2 SDK和 Mesa 均未 prepare、升级或替换。实际 OSMesa DLL SHA `c633b820bba8ec0dcb05466505e3c246e4c4028eaec7caf1054e707c8eb3ca1f`。','',
'## 已有能力、复现和修改','',
'历史 af36e02／e0dcf78／07a66a8 的浮动取消、提示生命周期、多行内容和公共对话框布局仍沿用；4fdfcc2 滚动条输入点、同 HWND／异步几何、精确语言及阶段诊断未回退。历史 P1 A/C不计本轮新测试，也未重新开展 Mesa 专项。','',
'| 问题 | 本轮责任和最小修改 | 验证／边界 |','| --- | --- | --- |',
'| 宿主＋、窗口控制的底色偏斜观感 | Light 默认 DT_LEFT；宿主 glyph 未指定居中。共享按钮和 glyph 明确 text-align:center，圆角工具／矩形窗口按钮保持各组尺寸。 | before/hover/down/held-outside/release-outside/Tab 连续桌面 BMP；实际命中、捕获、Dispatch和不变矩形。灰度前景估计：96DPI＋横偏−10.5设备px→0，最小化／最大化约−16→0，关闭−16.5→−0.5。详见 button-geometry.json；阈值排除反锯齿边缘，旧探针未记录原点，旧原点推导有明确标记。 |',
'| 鼠标后的虚线焦点 | 移除无条件 DrawFocusRect；保留 focused节点，按键／指针模态决定实线。outline向内2逻辑px，不占布局。 | Light与实际 Runtime的属性 Submit 真实鼠标／Tab画面；Space／Enter仍激活助手，输入 Home/End和选择合同保持。公共 C 动态测试独立核对像素、焦点和原矩形；无共同 CSS 场景也有键盘环。 |',
'| 短提示占满大框、布局成本 | 宿主自己的300×160模板弹窗仍存在，不是公共tooltip的历史修复未实现。改为 chrome_host 的已有 show/hide 接口，按内容测量；离开／操作／窗口位置变化／关闭隐藏。 | 真实短提示前后图，明暗／边缘／窗口状态／拖动；其他实例菜单和确认语言快照由原回归继续核对。没有禁用全部提示。 |',
'| 重复宿主状态布局 | 同次 layout 两份相同 JSON 重复处理。只抑制完全一致的第二份；每次新的刷新仍发送第一份，包括仅图像变化，保留语言／数据副作用和失败重试。 | 两种组件后端同场景根宿主布局213→182；单字段就绪对照根宿主280→220。全局缓存或跨刷新去重未引入。 |',
'| 最小化零客户区 CSS 错误 | 真实窗口操作捕获 left=-4px；零宽面板仍绘制 side splitter。仅在面板有有效范围时绘制该边。 | 原拒绝日志保留；最终Light96和Runtime192真实最大化／还原／最小化／关闭无原错误。最小化后恢复为受控 ShowWindow，明确不冒称硬件恢复输入。 |',
'| 完整路径和窄窗 | State菜单暴露已有 test.modal；公共dialog layout表达首选宽440、最小宽240／高120、内容自动高度。共同样式和原密度沿用。 | 窄380逻辑px下表格仍在，属性通过已有侧边条浮动工具到达；不修改保存布局。字段草稿、Submit、Tab／Esc可达。对话框草稿独立，不宣称已接入业务回写。 |','',
'连续浮动图覆盖按下、经过实际 OSMesa RGBA内容、Esc、受控捕获丢失、靠近停靠目标、松开；捕获／面板状态、内容 HWND和草稿都核对。该内容是 OSMesa离屏图像，不是硬件 GL或本轮原生WGL遮挡认证。受控 ReleaseCapture 不等同外部软件抢占。','',
'## 实测、命令与画面','',
'410步基线构建完成；外观／计时／优化阶段及最终重建日志全部保留，使用Release、C11、真实WebView2 ON、固定OSMesa显式路径、原 /W4 /WX。最终常规28项：28通过、0失败、0跳过，347.64秒；默认键盘环补齐后7项针对回归：7通过、0失败、0跳过，37.59秒。前一组与最后补验的构建身份分别记录，合计35条执行，去重28项。最后补验新增条件仅无共同 CSS 的默认焦点；旧资源和功能断言未删除。','',
'最终画面矩阵按实际run-inventory记录：Light／webview2 × light／dark × 程序96／144／192共12组，另四组窄窗；串行真实 SendInput，字段就绪后实际操作。宿主始终是产品的 Light chrome，Runtime的焦点证据来自真实 Runtime组件，不以替换宿主为Runtime的失败镜像实验作通过。原始 EOF和OS exit独立于main返回。','',
'复核入口（仓库根目录，x64开发命令行）：','',
'```powershell',r'cmd /d /c build\ux-20261012-5e8c4a91\tools\build-final3.cmd',r'cmd /d /c build\ux-20261012-5e8c4a91\tools\run-checks.cmd',r'cmd /d /c build\ux-20261012-5e8c4a91\tools\run-focus-checks.cmd','python build/ux-20261012-5e8c4a91/tools/run-final3-matrix.py','```','',
'命令对应原目录首次执行，复验应改用新唯一目录。保存的诊断源在 source/final-tools；恢复时复制到相同的新build/tools结构再编译，不能覆盖本次raw。完整命令／产物SHA在各run.json。原180／240等CTest超时均未改；本轮探针120秒诊断期限也未提高。','',
'画面索引：','',
'- [整改前＋悬停](ux-20261012-5e8c4a91-evidence/screenshots/before-light-96/001-open-hover.png)','- [整改后＋及短提示](ux-20261012-5e8c4a91-evidence/screenshots/final3-path-light-96-light-buttons/001-open-hover.png)','- [实际Runtime鼠标释放](ux-20261012-5e8c4a91-evidence/screenshots/final3-path-webview2-96-light-buttons/055-submit-released.png)','- [实际Runtime键盘Tab](ux-20261012-5e8c4a91-evidence/screenshots/final3-path-webview2-96-light-buttons/056-submit-keyboard.png)','',
'所有完整PNG及连续序列均在screenshots/<run>/；原BMP留在同名build/runs目录。PNG转换后RGB逐像素核对，不拼接、修饰或重画画面；BMP被Git忽略，clone不携带它。','',
'## 体验量测的可信边界','',
'固定同机、物理1200×800窗口、程序96、亮主题、100000行／64列、同一操作顺序。每类先记录稳定ROI，再真实输入；独立线程读取实际桌面BitBlt／GdiFlush。输入起点是SendInput前QPC，包含排队／Dispatch／布局／绘制至该ROI首个像素变化；终点是首次采样变化，不是扫描输出或整个画面的第一像素。ROI有范围限制；采样时间位于读回完成后，属于上界估计。最终状态列是C／DOM已验证状态，不冒称最后一帧已呈现或每类都已完全可用。','',
'完整v2各270样本，Light前后OS0且断言通过；Runtime前后均自然OS1、各2项首次对话框未就绪／随后菜单失败，不能称整个套件通过。原measurements.valid仅验证modal，不包含后续关闭，dialog v2数据不作可用性验收。单独把终点改为字段visible且focused后，原／优化Runtime各20次打开、Tab就绪和Esc关闭全部通过，OS0，未由猜测修补产品。','',
'新进程setup记录287.5→292.9ms（Light）、212.5→213.9ms（Runtime），终点仅应用创建／实例取得；OS文件缓存未清空，未验真实冷缓存或首屏呈现。后续重复操作为暖态；不得把上述setup叫作完整冷启动用户体验。','',
'两处性能改动阶段的量测身份为pre-perf和optimized，各run.json精确记录；最后零尺寸边和无CSS默认焦点补丁的最终视觉／回归身份另外记录，未假称重测了最终二进制全性能矩阵。','',
'### 按操作类别的前后数据','',
'单位ms，单元格依次P95／P99／最差；反馈缺样保留−1并列计，不能按0ms通过。n为输入样本数。分位数线性插值，样本量有限。']
def fmt(v):return '/'.join(f'{v[k]:.1f}' for k in ['p95','p99','worst']) if v.get('n') else '未捕获'
for backend in ['light','runtime']:
    before=data[f'perf-before-v4-{backend}-96'];after=data[f'perf-after-v4-{backend}-96']
    lines+=['',f'**{backend}组件（Light宿主）**','', '| 操作 | n | ROI反馈 前→后 | 已验证状态 前→后 | 反馈缺样 前/后 |','| --- | ---: | --- | --- | ---: |']
    for name,old in before['categories'].items():
        new=after['categories'][name]
        lines.append(f'| {name} | {old["samples"]} | {fmt(old["desktop"])} → {fmt(new["desktop"])} | {fmt(old["state"])} → {fmt(new["state"])} | {old["missingDesktopChange"]}/{new["missingDesktopChange"]} |')
lines+=['',
'50ms首反馈为进取目标：悬停P95 Light90.5→39.8ms、Runtime89.1→45.8ms；Runtime最差51.2ms仍超过50。菜单、点击助手、实例切换和长输入未全部达成，且部分指标没有改善。长输入为整段约160字符的真实Unicode键消息突发，不代表单次人工按键或IME延迟。Home/End的最终可见位置另由真实画面核对，不以focused返回替代可读性。','',
'### 帧、CPU、布局与空闲','',
'原始桌面采样间隔通常约17ms，最长约35ms，不能从它认证60fps、精确最长呈现帧或实际漏刷新计数。下表是连续操作ROI不同像素读回之间的间隔；“超过60Hz间隔数”和估计缺槽仅对假设60Hz的图像观察，不是合成器missed-frame计数。停靠ROI经常未变化，保持不可观测，不填造帧率。完整间隔和原始采样见performance-summary.json／desktop-samples.csv。','',
'| 后端／操作（优化后） | 改变间隔n | P95/P99/最长ms | >16.67ms观察间隔数 | 估计未见槽 |','| --- | ---: | --- | ---: | ---: |']
for backend in ['light','runtime']:
    for name in ['scrollbar-drag','float-drag-gl','dock']:
        c=data[f'perf-after-v4-{backend}-96']['categories'][name]
        lines.append(f'| {backend}/{name} | {c["observedChangedPixelIntervalMs"]["n"]} | {fmt(c["observedChangedPixelIntervalMs"])} | {c["gapsOverAssumed60Hz"]} | {c["unseen60HzSlotsEstimate"]} |')
lines+=['',
'全套被观察CPU采样累计Light约29859→28828ms，Runtime宿主及其实际子进程合计32859→31734ms。含宿主内桌面观察线程，采样不覆盖最后读数之后的退出成本；不能把它当纯产品CPU或精确OS生命周期总CPU。原始每PID CPU/RSS在process-cpu.csv，峰值不可直接相加成同时峰值。','',
'Light所有视图布局总数11546→11145、布局时间13974.8→13418.1ms、GDI paint2500.2→2406.2ms；真实Runtime内部布局／重绘不可由Light计数推断。宿主全客户区重绘仍存在，实例切换根视图31次paint、累计29069568设备像素处理面积前后相同，未宣称解决大范围重绘。两秒空闲宿主CPU采样0～15.625ms、观察线程不采图；idle trace也含退出清理，不能作为全部视图空闲零绘制认证。原长期持续重绘／瞬态Runtime不宣称根治。','',
'## 保留的失败与责任区分','',
'- 首批Space“未收起”是探针把assistant_expanded=-1误当真，KEYTRACE证实Dispatch和产品三态正确，修正探针而非产品。','- Runtime-mirror临时替换宿主的诊断presentation持续pending、120秒后强制终止；产品宿主未变成Runtime，既不计通过也不归为Mesa退出故障。','- 初版计时器、未稳定ROI和大量KEYTRACE使探针扰动过大；原perf0／v1留存，正式v2统一观察器。','- 初版浮动终点在合法停靠区却要求floating，另一次path字段尚未取得／对话框未ready导致错误点击和后继失败；原画面、实际外部hit PID和断言保留，不当作框架根因。','- 窄窗初次直接点不可见属性字段失败；原布局已提供可见浮动入口，后续按用户可达路径验证，未猜测改布局。','- 有一次探针编译缺browser_shell.h后误启动旧exe，label为perf-before-v3-light-96；身份暴露旧exe，未纳入v2对照。之后编译／启动按exitCode门控。所有构建警告、链接失败和自然退出失败都保留。','',
'## 文件、公共用法与剩余条件','',
'源码改动仅对应上述问题：src/visual.css、src/backends/light_web.c；宿主host.html/main.c；维护示例app.c/language.inc；公共C回归light_web_dynamic.c；视觉规范、开发标准受控CSS表、示例README和本报告；.gitattributes仅保护本轮证据原字节。UI_UX_TRACE／UI_UX_PHASE为默认关闭的内部诊断，无公开API或新资源预算；CSS拒绝信息仅在既有UI_LIGHT_SCRIPT_DIAGNOSTICS开关下补充属性和值。','',
'应用继续使用已有 ui_component_set_dialog_layout／get_dialog_layout／set_field_layout、ui_host_show_tooltip／hide_tooltip；不新增存储、全局语言开关、业务接口或新ABI。工作树改动等待用户审查，未提交。','',
'[保护核验](ux-20261012-5e8c4a91-evidence/raw/final-protection-audit.json)：143＋638＋733＋2675＝4189条历史索引文件及索引SHA复核无差异，固定Mesa和最终44c039f SDK亦保持原SHA／大小。业务应用、PERF-001、冻结SDK、历史BMP和旧构建目录未写；未拥有旧BMP全量接手哈希，不能冒称全部已逐文件比较。新BMP／构建被Git忽略，证据PNG、raw、源码和索引另存；未生成新SDK归档。','',
'| 独立条件 | 本轮结论 |','| --- | --- |','| 当前源码Session0服务／严格整机无登录 | 待验；未注销用户、未改既有服务或降低logged_sessions门槛 |','| 真实英文Windows／中文IME | 待验；当前显示语言中文，ASCII Unicode输入不是中文IME验收 |','| 物理混合DPI／硬件鼠标／外部捕获抢占 | 待验；程序DPI、SendInput、受控ReleaseCapture分别记录 |','| 实际合成器呈现、硬件GL、长期人工 | 待验；GDI桌面读回不是精确呈现时间，OSMesa图像不代替硬件表面 |','| P1 259→0、同条件B/C、正常清理和真正卸载 | 本轮未开展，历史结论与剩余验证独立保留 |','| 原托管180／240超时／持续重绘／瞬态Runtime | 未宣称目标环境解决；需新的明确远程授权后才可补取目标证据 |','',
'可审查的下一步是把最慢的长输入与实例切换分成排队／脚本／布局／屏幕ROI阶段，并在同一观察条件下做单因素定位；真实呈现应另引入可验证的合成器观察端。此报告不授权远程或业务修改。','']
revisions={
 '完整v2各270样本':'最终v4采用同一观察器、字段visible且focused的对话框就绪终点：Light／Runtime修改前后各270样本，共1080；四组均自然OS0、完整断言通过，独立验证Esc关闭和进程退出。v2 Runtime两组各2项首次未就绪／后继菜单失败完整保留，valid=1不涵盖后续关闭，原dialog v2数据不作可用性验收；中间独立各20次就绪对照也通过。',
 '新进程setup记录':'新进程setup记录317.8→338.8ms（Light）、224.8→221.7ms（Runtime），终点仅应用创建／实例取得；OS文件缓存未清空，不验真实冷缓存或首屏呈现。后续重复操作为暖态；不得把setup叫作完整冷启动体验。',
 '两处性能改动阶段':'最终完整性能量测使用pre-perf与final4同一场景／观察器，身份见perf-*-v4-*-96/run.json。final5仅清除被公共提示替代、没有调用方的私有kind5显示分支；重新生成资源和链接产物后DLL／包／宿主SHA也变化，不能称同一二进制。final5另有7项针对回归和2组真实窗口操作；性能结果仍明确属于final4，不冒称final5新量测。',
 '50ms首反馈为进取目标':'50ms首反馈为进取目标：悬停P95 Light90.6→39.2ms、Runtime87.0→43.0ms，修改后各30样本最差44.7／47.5ms；这两类均在观察上界内达成。菜单、点击助手、实例切换和长输入未全部达成，部分指标没有改善。长输入为163字符真实Unicode键消息突发，不代表单次人工按键或IME延迟；Home/End末尾可见性另外由真实画面核对。',
 '全套被观察CPU采样累计':'全套被观察CPU采样累计Light约29047→27891ms，Runtime宿主及其实际子进程32453→28062ms。含宿主内桌面观察线程，采样未覆盖最后读数后的退出成本，不能当纯产品CPU或精确OS总CPU。每PID CPU/RSS在process-cpu.csv，峰值不可相加成同时峰值。',
 'Light所有视图布局总数':'Light视图布局总数11570→11135、布局时间13859.6→13067.3ms、GDI paint3023.6→2808.0ms；Runtime内部不可由Light计数推断，Runtime进程内Light视图paint517.3→536.7ms反而增加。宿主全客户区重绘仍存在，实例切换31次paint、累计29069568设备像素前后相同，未宣称解决大范围重绘。两秒空闲宿主CPU采样0～15.625ms、观察线程不采图；idle trace含退出清理，不能当全部视图空闲零绘制认证。持续重绘／瞬态Runtime不宣称根治。',
 '最终画面矩阵按实际run-inventory记录':'最终final4画面矩阵：12组Light／webview2×明暗×程序96／144／192，另4组窄窗，16组全部自然OS0、断言通过。final5清除无调用方kind5分支后补7项针对回归及2组真实窗口操作。宿主始终产品的Light chrome；Runtime焦点来自真实Runtime组件，不拿失败镜像代替。各组完整EOF和OS exit分别记录。',
 '410步基线构建完成':'410步基线构建完成；各阶段新构建和失败日志保留，Release／C11／WebView2 ON／固定OSMesa／原W4 WX。final2常规28项全通过（347.64s）；final3默认键盘环7项全通过（37.59s）；final5自动按钮测量及收尾7项全通过（时间见JUnit）。合计42条执行、去重28项，0失败／0跳过；最后补验不能冒充重新跑过28项。资源和功能原断言未删。',
 '源码改动仅对应上述问题':'源码改动仅对应上述问题：src/visual.css、src/backends/light_web.c；宿主host.html/main.c；维护示例app.c/language.inc；公共C回归light_web_dynamic.c／light_glyph_padding.c；视觉规范、开发标准受控CSS表、示例README、handoff追加说明和本报告；.gitattributes仅保护本轮证据。UI_UX_TRACE／UI_UX_PHASE为默认关闭内部诊断；CSS拒绝信息只在既有UI_LIGHT_SCRIPT_DIAGNOSTICS开关下追加属性和值。',
}
for index,line in enumerate(lines):
 for prefix,replacement in revisions.items():
  if line.startswith(prefix):lines[index]=replacement
lines=[line.replace('final3-path-','final4-path-').replace('build-final3.cmd','build-final5.cmd').replace('run-final3-matrix.py','run-final4-matrix.py').replace('run-focus-checks.cmd','run-final-checks.cmd').replace('performance-summary.json','performance-summary-v4.json') for line in lines]
at=lines.index('## 实测、命令与画面')
lines[at:at]=['## 自动按钮的144 DPI截断','',
 '完整路径复核发现Light自动宽度按钮把Submit截成Sub...。仅公共C调用方、同一文字／字体／padding，比较auto和120逻辑px参考按钮：96 DPI两者97个文字深色像素；144 DPI原auto118／参考188，auto宽60逻辑px；192 DPI两者377。根因是文字转逻辑尺寸和两侧paint inset分别取整，144 DPI少了设备像素。按实际paint两侧inset求设备需求，再向上换算逻辑宽；auto变61逻辑px，144两者均188。96／192保持58／59逻辑px及原完整像素。固定width／max-width、用户列宽和布局预算未改变；仍允许显式窄尺寸的原省略语义。',
 '原同源公共调用方在旧DLL自然OS1、新DLL自然OS0，日志glyph-independent-before.log／glyph-after.log及测试源码保留。没有靠额外补几个固定像素或放大所有控件掩盖问题。','']
(root/'docs/validation/ux-validation-20261012.md').write_text('\n'.join(lines),encoding='utf-8')
print('report written')
