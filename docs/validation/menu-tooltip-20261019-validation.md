# 菜单标题与提示生命周期查证（2026-10-19）

作者：Codex。结果：两项请求均有真实复现、已确认责任的最小修复及前后验证。额外 Alt+F4 失败独立保留，未宣称所有交互路径已解决。

## 身份与范围

入口/最终 HEAD 均为 `689fa03b34f8c4321a6c838f52ca9dcd790a9319`，分支 `codex/p1-process-boundaries`。入口8项旧文档/属性修改及既有未跟踪材料未纳入本轮改动或清理。最终代码/资源清单 SHA256 `9ecb410caca793dcea787f20193f62fb899dfddba4f4393f588963de3bc75328`，261文件，13项本轮差异（11个既有文件、2个新公共调用方源码）。[入口及最终清单](../../build/menu-tooltip-20261019-5a840d43/raw/final-source.json)、[入口字节快照](../../build/menu-tooltip-20261019-5a840d43/entry-source.zip)、[仅本轮差异](../../build/menu-tooltip-20261019-5a840d43/changes.patch)。

SDK/API/标准0.9.0-dev/9/9、ABI/包1；前后DLL导出均248且完全相同。本轮新增的是内部提示所有者记录，没有新增公共接口。Windows10.0.26300，MSVC19.50，实际Runtime155.0.4283.62，2560×1440、系统DPI96；菜单程序DPI96/144/192单列。固定依赖、默认Mesa、业务应用、冻结SDK和历史证据保持原位置与原内容；无提交、推送或远程运行。

## 1. 菜单留白：宿主宽度估算责任，已修复

直接运行入口及候选 `framework_host.exe` 加载框架维护的 `web_counter.uapp`，真实点击打开菜单、Esc关闭、真实关闭按钮，两个正式宿主均自然OS退出0。另用同一正式宿主实现及嵌入资源的观察器，通过公开菜单注册和presentation查询记录8种中文/英文/快捷键标记标题。

当前路径确为原 `Math.max(48, Math.min(220, title.length*14+16))`。实际样式13px Segoe UI、正常字重、按钮左右padding各8、height26；同组gap=0，条带左右padding4。留白来自按钮内部宽度，英文按每字符14px系统性高估，中文字还受48最小宽度影响，括号快捷键标记也被计为等宽字符。

| 192程序DPI，逻辑像素 | 入口按钮宽 | 实际文字加留白后的宽 |
|---|---:|---:|
| File | 72 | 36 |
| Edit | 72 | 38 |
| 文件 | 48 | 43 |
| 编辑(E) | 86 | 58 |
| Window | 100 | 63 |
| III / WWW（同字符数） | 58 / 58 | 32 / 52 |

`examples/framework_host/main.c` 使用与当前宿主按钮一致的字体/字号和程序DPI测量真实字形，向私有宿主模型写入逻辑宽度；`host.html` 使用该宽度。保留16px点击留白、32px最小目标、220px原最大宽度、稳定菜单ID及既有溢出规则，没有缩短标题。

入口48项宽度检查46失败；候选明暗×三DPI的48项全部满足实测尺寸。窄窗口溢出、Alt+F打开指定菜单、Enter执行一次并关闭已由真实SendInput和公开最终状态验证。100轮宿主布局 GDI119→119、HANDLE191→191；桌面菜单40轮开关 HANDLE289→289、GDI857→857、USER50→50。

[原几何/输入日志](../../build/menu-tooltip-20261019-5a840d43/raw/menu-baseline/stdout.log)、[修复后完整宿主输入](../../build/menu-tooltip-20261019-5a840d43/raw/host-normal-loop/stdout.log)、[连续画面](../../build/menu-tooltip-20261019-5a840d43/frames/host-normal-loop/)、[正式EXE前后](../../build/menu-tooltip-20261019-5a840d43/raw/formal-close-control.json)。

## 2. 提示提前消失：分开确认的三条触发链

### 无关组件更新误用host级隐藏（共同组件/菜单模块，已修复）

仅公共C/Win32接口的 `tests/tooltip_lifecycle.c` 注册FORM和两个STATUS，不加载业务代码。真实鼠标进入字段标签，空闲600ms仍显示；之后相同/不同状态文字及销毁无关组件均同步关闭提示。Light与真实Runtime首次各7个断言失败，父进程OS1；Runtime6个观察到的子进程均正常OS0。

更新前后鼠标、焦点及捕获不变，立即查询 `queryDispatch=0`；实际隐藏发生在组件变更通知、工作区刷新之前。匹配PDB的首次隐藏转储解析为 `ui_host_hide_tooltip ← replace_caption+0x10a (component_language.inc:15) ← 公共set_text调用方`；set_text/set_title尾调用优化未各保留独立栈帧。[原始日志](../../build/menu-tooltip-20261019-5a840d43/raw/tooltip-baseline-light/stdout.log)、[匹配栈](../../build/menu-tooltip-20261019-5a840d43/raw/tooltip-first-hide-stack-v2.txt)。

`src/menus.c` 为组件自动提示记录内部所属组件；`component_language.inc`、`components.c` 将组件caption、field、layout、visible、销毁及tip-hide限定为所属组件失效。公开ROW锚点的所属组件也受识别。普通菜单保持独立，公开 `ui_host_hide_tooltip` 的整host/浮动标题语义不变，模态、语言切换及真正关闭仍保留必要清理。粒度是组件，不承诺同一FORM内逐字段的精细失效。

### 父按钮与子节点之间错误的leave/enter（Light后端，已修复）

正式工具栏中心命中其文字span时没有按钮mouseenter；先进入按钮背景再移向文字会发button-leave、关闭提示，指针仍在按钮内部。公开Web最小调用方 `tests/hover_boundaries.c` 在入口Light复现3个失败；正确配置的Runtime对照为0。真实画面、公开消息事件及实际鼠标输入均保留。

`light_web.c` 按旧/新命中节点的共同祖先发送必要边界事件：离开由内到外、进入由外到内；父内移动不再误报离开。事件路径使用稳定UID，真正离开窗口仍发送祖先leave。Runtime原生行为未另行重写。公共输入分发、真实Light和真实Runtime对照均通过；正式工具栏及chrome按钮在无关状态更新及正常Dispatch后保持提示。

### 所属错误文字变更后旧提示残留（共同组件，已修复）

增补对照在入口复现：`set_error("Updated error")` 后仍实际显示 `Original error`，新增1失败（扩展入口合计8失败）。补齐该组件提示的失效；重新悬停后API与实际画面均为 `Updated error`。这条属于必要清理缺口，与前述失效过宽相反，单独记录。

[错误文字原失败](../../build/menu-tooltip-20261019-5a840d43/raw/tooltip-owner-error-baseline/stdout.log)、[Light明暗前后序列](../../build/menu-tooltip-20261019-5a840d43/frames/tooltip-visual-light-dark/)、[Runtime正确新错误画面](../../build/menu-tooltip-20261019-5a840d43/frames/tooltip-visual-runtime/0030-owner-error-update.png)。

### 提示路径边界

应用菜单栏标题使用菜单打开/悬停切换路径，本身不创建文本tooltip；工具栏提示属于活动应用host，宿主按钮/标签/面板标题属于chrome host；公共组件提示属于各自应用host中的组件。状态更新只调度宿主layout，对照中chrome提示未被关闭，未修改其工作区刷新实现。指定场景未发生目标HWND重建，直到显式销毁。未复现的其他异步leave、工作区变动、遮挡/重建或外部输入链仍为“尚未定位”，不以本次通过归因。

## 验证、失败与交付

实际命令见本轮 `tools/build-baseline.cmd`、`build-final.cmd`、`regression.cmd`、`asan.cmd`及各run.json的command；执行路径均为独立新目录，清理后日志保留。主要公开入口：`ui_tooltip_lifecycle_probe light light|dark`、`ui_tooltip_lifecycle_probe runtime light|dark`、`ui_hover_boundaries_probe light`（公共分发）及增加 `--real` 的真实输入。

- 新Release：入口WebView2完整构建、候选WebView2/Light/native完整构建成功；最终桌面fixture在WebView2及默认Light重新编译成功。
- 相关CTest首轮23/24通过；唯一失败为旧desktop菜单fixture假定480px必然溢出。保持480px、原断言/超时，明确增加菜单数据后该项1/1通过。其余23项对应同一最终产品代码。完整初失败/修正结果均保留。
- Release ASAN相关7/7通过，额外真实Light生命周期OS0且无ASAN报错。
- Light/Runtime×明暗四组最终公共生命周期均OS0；Runtime两组12个观察到的子进程均OS0。必要的目标更新、重新悬停、移出、销毁、宿主隐藏/销毁及公开host级hide均验证。
- 独立真实边界对照、菜单鼠标/Alt/Enter最终状态、两份正式EXE关闭按钮均通过。原生父/子窗口及提示消息、焦点/捕获和Dispatch分账记录。
- 32个独立观察运行的全部结果在[总表](../../build/menu-tooltip-20261019-5a840d43/raw/run-summary.json)，含下述诊断失败，不能把该总表解释为全部通过。

额外失败：正式入口/候选的Alt+F4脚本在10秒内均未关闭窗口，前台PID仍为目标；诊断器随后只终止自有目标为254。未捕获该按键的完整Dispatch链，责任**尚未定位**，也不能证明已进入退出清理。正常关闭按钮后续OS0不是该按键路径通过。

观察器修正分账：原始字符串postMessage不符合JSON合同；Runtime未开启framework_components导致native_handle为空、坐标无效；工具栏原布局height0；键盘观察器漏调正式host_menu_key；首次ASAN真实调用缺VS运行库环境（OS0xC0000135，未进入main）。这些诊断/调用方失败保留文本，修正没有更改产品断言或提高等待预算。x86 CDB无法启动x64进程，改用自有x64观察器取转储再离线解析，调试干预不计入无调试时序。

两组PNG采集停顿使原视觉覆盖不完整（最长8.4秒），随后改为独立BitBlt/GdiFlush采集、排队PNG编码，在同一产品及原输入节奏下补齐；新两组44/45帧，0丢帧/编码错误。这里的“帧”是截图样本，不是物理呈现或帧率验收。真实画面已人工目视复核：菜单前后、提示无关更新后保留、新错误文字及明暗/DPI关键帧。

[新开发产物ZIP](../../build/menu-tooltip-20261019-5a840d43/ui-framework-menu-tooltip-20261019-9ecb410caca7-windows-x64.zip)：24文件、1412887字节；SHA256 `f035711b0c3350d1d5cba9a08cb11bad5c1ba211e72a57748cd481b54f8d3f0e`。包含匹配头/导入库/DLL/宿主、公共调用方源码和探针，不替换冻结SDK。CRC、独立解压逐文件SHA、仅包内公共材料构建调用方及公共分发OS0通过；归档及解包原文件未回填。[证明](../../build/menu-tooltip-20261019-5a840d43/raw/unpack-validation.json)。

本轮回退工具 `build/menu-tooltip-20261019-5a840d43/rollback.py` 默认仅核对；只在13个文件仍匹配本轮最终哈希时允许 `--apply`，恢复入口11文件并撤回本轮2个新源码。旧8项文档修改及既有未跟踪材料不在撤回范围。

未验证：硬件鼠标、外部捕获抢占、物理高/混合DPI、多屏、真实IME、真实英文Windows、桌面合成专项、Session0、严格无登录及长期人工条件。程序DPI、SendInput和桌面采集不替代这些条件。

## 收尾

已释放 0.762 GiB，保留424张必要的原始失败/前后连续画面。清理结果见本轮 `cleanup-result.json`：清理新建的构建树、临时观察器/副本、重复截图、运行缓存、打包暂存及解压目录；保留源码、简明文本、正式ZIP、明确要求的原始失败/前后连续画面和未定位Alt+F4最小诊断。历史目录不动。归档封存后说明和索引写在外部，不补写ZIP。

原raw/run.json保持原文；被清理的重复/错误观察器截图在frame-retention-plan.json逐文件记录原SHA及原因，不将缺图伪称完整视觉证据。正式原始失败和被引用的连续序列保留。入口5307个既有未跟踪路径全部仍存在，Git索引为空；源码及ZIP最终哈希复核匹配。
