# 通用交互与公共契约整改验证

2026-10-17，Codex；记录标识 `interaction-20261017-a7d913e4`。面向框架维护者及公共 C 调用方。结论来自本轮本地新构建；旧交接结果仅作溯源。

## 结论与身份

修复了 Light 相同内容替换不折叠选区、嵌套滚动中焦点未显露，以及 Runtime 144 程序 DPI 下捕获边界不一致三条已复现路径。新增按稳定实例 ID 设置/查询标题的公共 C 能力，以及每 host 的主题查询、设置、通知和系统主题刷新契约。没有据此宣称历史泛化叠层、持续重绘或托管超时已根治。

| 项目 | 实际身份 |
| --- | --- |
| 入口与最终 HEAD | `1af1f5a9f2efd6627ffea300ef90c41171cb917b`，相同，未提交 |
| 分支 | `codex/p1-process-boundaries` |
| 入口工作区 | tracked clean；11 个历史未跟踪路径保留 |
| 开发版本 | SDK0.9.0-dev / API9 / 标准9 / 应用 ABI1 / 包格式1 |
| 最终代码/资源身份 | `599800c363e101a6179dcc1c709a8d721cd74eafb47b21b22f986664324c2b94` |
| 新构建根 | `build/interaction-20261017-a7d913e4` |
| 新证据根 | `docs/validation/interaction-20261017-a7d913e4-evidence` |
| 最终 Release DLL SHA256 | `20693498923b2c272d772161990680fc87732e3929da96e93c8ca3fc944bee88` |
| 最终宿主 SHA256 | `9e2375974f15c71f635cdf6cc7c4c7c457b39859f79b2548818c249593bd533e` |

代码身份是有序的非 Markdown 源码/资源路径、长度、SHA256 的紧凑 UTF-8 JSON 的 SHA256，完整清单见 [final-code-identity.json](interaction-20261017-a7d913e4-evidence/raw/final-code-identity.json)。它不是一个新 Git commit。入口 280 文件快照位于构建根 `snapshots/entry-source-resources.zip`，入口状态与每文件哈希见 [entry-identity.json](interaction-20261017-a7d913e4-evidence/raw/entry-identity.json)。最终完整源码/说明快照、二进制差异和回退清单另列在证据 `raw/final-worktree2.json`，不回写 SDK。

实际环境是 Windows、Session1、Default 解锁桌面，2560×1440、单屏、系统/窗口/物理显示器 DPI96、ACP936、Windows UI 语言 zh-CN；程序另设96/144/192。MSVC19.50.35727.0，WebView2 Runtime155.0.4283.45，固定 SDK1.0.4129.50。Lexbor、QuickJS 固定提交不变；Mesa24.3.4 OSMesa SHA256 为 `c633b820bba8ec0dcb05466505e3c246e4c4028eaec7caf1054e707c8eb3ca1f`。实际前台信息见证据中的 desktop JSON。未修改系统主题、既有服务或登录状态。

## 1. 文本替换：确认框架责任并修复

入口 `src/backends/light_text.inc` 的共享 `lw_text_replace` 在新旧完整字符串相等时提前返回，跳过正常替换的选区折叠、光标和编辑历史。原值0、Ctrl+A、逐字输入0.002：首个0保留了全选，后续句点覆盖0，最终成为 `.002`。新基线 Light 的真实输入在96/144/192均复现；Runtime 原生编辑路径没有该缺陷。

移除内容相等的提前返回，只保留“无选区且无新增字符”的真正空操作。正常同值替换仍走既有撤销、redo 清理、光标/选区和输入事件路径。标准浏览器对选区同值输入也产生 input 事件；没有把 DOM 相同值赋值与用户替换混为一谈。共享实现覆盖公共 Web input/textarea、FORM 单行/多行及数字字段；没有在某个业务入口补丁绕过。

`tests/text_replacement.c` 通过公开分发覆盖全选、部分同值、完全同值、插入、空/中文/多行、删除/退格、连续输入、撤销重做、同值 DOM setter 和焦点切换。额外公共数字调用方核对 `UI_VALUE_NUMBER` 的文本表示：旧 DLL 为 `.002`，新 Light/Runtime 为0.002。原错误猜测数值成员的编译失败和修正后的源码都保留。

真实宿主探针逐字符输入及 Ctrl+V 同值粘贴后继续输入，两个后端三档 DPI 均通过；例如粘贴相同0.002后 X/Y 的后续输入得到预期 `0.002xY`。保存按键、Dispatch、光标/焦点、实际值和实际 OS 退出，不能只看 SendInput 返回值。结束 GDI/USER 计数 Light13/4、Runtime13/15，与对应旧探针一致；这些是整个探针的结束计数，不是“所有系统资源为零”的证明。

证据：`raw/local-records/runs/real-before2-display-light`、`final-real-display-light`、`final-real-display-runtime`、`real-identical-paste-*`、`number-entry-light`、`number-fixed-*`。每个新进程 run.json 记录 EXE/DLL/参数产物 SHA、PID、超时、实际退出码和 forced=false。

## 2. 程序 DPI、捕获边界与混合表面

使用公共 C 示例及实际 native HWND、公共 Web、现有软件 GL 表面，分别记录 view-local 逻辑坐标、host-relative、设备像素与 screen 坐标。没有加载业务应用。

Runtime 的独立24组合捕获复现包含三档 DPI、奇数起点和尺寸：原 DLL 有6项断言失败，其中3项实际捕获错误、3项元信息与 native controller 尺寸不同。根因有两部分：宽高独立取整与“左右边缘相减”不一致；144 DPI 时 CapturePreview 可能先将实际像素视口取整到 DIP，再栅格化，多出边缘像素。

`src/backends/webview2.c` 让尺寸元信息按已用的 controller 边缘取整；WIC 仅接受实际视口尺寸或严格计算出的整 DIP 栅格尺寸，按真实视口裁取像素。不缩放、不填充、不重建，不放宽其他尺寸错误；保留32MiB上限。成功裁取详情只在子进程诊断环境启用时记录。相同 EXE、新 DLL 的24组合尺寸及全部像素断言通过。

真实 Runtime144首轮3个显示探针失败由此修复；最终三档的窄表头“调整后”和“新建同尺寸”逐像素一致，Home/End 长文本末端可见。实际浮动穿过软件 GL、持有反馈、切换目标、Esc、受控捕获丢失、调宽/缩放、关闭及重新挂载在三档通过，保留内容 HWND/上下文、草稿等既有断言。144/192初次探针失败是测试窗口过程把设备坐标当逻辑坐标，校准转换后通过；没有因此修改产品拖动语义。

只读41行中的另一个长文字区域专门做了分离对照：它是悬停完整值提示。公开 `ui_host_hide_tooltip` 后公开 detail 查询 NOT_FOUND，基础字段画面正常。Light、Runtime 两组均实际 OS0，完整帧位于 `frames-final/readonly-overlay-*`。没有把正常提示归为残影。

这些结果确认捕获路径及所测混合表面操作，**不证明**历史泛化192叠层、所有原生表面合成、物理混合 DPI、多屏或其他软件抢占捕获已解决。未独立复现的持续残影仍未定位；控制式捕获丢失不等于外部软件抢占。

## 3. 窄布局、长文本及焦点显露

入口 Light 在嵌套容器内 focus/Tab 时没有逐层显露。基线三档均焦点已设置，但目标可见高度0；新 `lw_reveal` 从内向外调整滚动，每层后重算几何，供公开 focus、scrollIntoView 和 Tab 共用。三档目标可见高度32；Runtime 原路径为21，未重复开发。未改变数据范围、列宽、generation、uint64/BigInt 或按需查询预算。

已有对话框 preferred/min/max 客户区和 `ui_component_set_field_layout` 可表达多行可视高度，继续复用。公共 FORM、检查器、选项/菜单的原有边界、滚动、宽度和键盘回归纳入最终矩阵。公开文档补齐调用方的外层滚动、内层文字滚动、长说明和多行高度用法，没有新增平行布局 API。

真实两后端 `real-narrow-form-*` 验证41行和长行的末行标记、选择复制、实际滚轮/滚动条/键盘、多行只读/可编辑对话框、Enter/Ctrl+Enter/Tab/Esc、窗口约束、错误/帮助及窄窗口。公共输入组件的实际 Home/End、主题切换后后续输入也通过。范围以各 run 完整断言为准，不把单个末行截图外推为所有下拉选项内容的人工可读性验收。

新标题弹层第一候选采用普通 div，实际长无断点标题被裁切，已撤回。最终仅该新弹层改为有固定可视高度的只读 textarea：实际 F1、Ctrl+End、全选复制验证300多字符及 `TITLE_END` 完整可达；Esc 和标题失效关闭保持。普通菜单行为未改。

## 4. 动态实例标题公共 C 契约

原先只可查询包名，没有等价的动态实例标题公共设置能力。新增：

```c
ui_workspace_set_instance_title(workspace, instance_id, "参数 — 修改中");
ui_workspace_set_instance_title(workspace, instance_id, NULL); /* 恢复包名 */
ui_workspace_get_instance_title(workspace, instance_id, buffer, capacity, &required);
```

UI线程、mount及之后调用，复制字符串；非空有效UTF-8最多4095字节。required包括NUL，长度查询和短缓冲原子拒绝；销毁/未知ID NOT_FOUND，关闭/递归设置 CANCELLED，workspace本身必须活着。包元数据、稳定ID、路由、选择、关闭和挂载不变。同名允许；宿主附加原实例ID，限制标签宽度，悬停/F1可访问完整标题。

`tests/workspace_titles.c` 验证多实例、后台20次改名、4095字节、错误编码/长度/线程、复制/重置、短缓冲、回调重入、关闭和销毁ID，记录 HWND 和活动实例不变。实际独立宿主装载两个只使用公共 C 的诊断应用，通过公开按钮设置后台/当前标题、F1完整标题、主题和切换实例、正常关闭。`tests/title_theme_app.c` 与 `tests/title_theme_probe.c` 可复用，操作没有私有 JS。

## 5. 主题契约、所有权及限制

新增 `theme.h` 独立88字节完整 size 结构，原已发布结构不插字段；四个导出：`ui_host_get_theme`、set_theme、set_theme_callback、refresh_theme。每host独立，默认LIGHT，显式DARK/SYSTEM；16个语义色覆盖背景、正文、边框、焦点、选择、悬停、按下、禁用、进行中、图标、强调及错误。它们是调用方协作依据，不是所有控件像素统一成同一颜色。

变化顺序为提交模式/generation、关闭所属提示/菜单、更新框架表面、同步应用回调、workspace主题事件。Runtime呈现仍异步。回调可查询、更新资源，递归set/refresh拒绝，卸载先取消回调再释放应用资源。应用自绘 native/Web/GL/图片须应用调色板或根据effective选明暗图标；框架不替换调用方资源。维护示例更新同ID菜单图标，并新增从草稿设标题、跟随系统的命令。

SYSTEM采用公开 Windows UISettings.Foreground 亮度规则；查询不写系统配置，平衡自己的WinRT初始化，保留调用方已有MTA。拥有者转发WM_SETTINGCHANGE/WM_THEMECHANGED或显式refresh；未订阅跨线程ColorValuesChanged。当前系统真实查询可用，但本轮未实际切换Windows系统明暗。高对比度、系统强调色、自绘内容自动适配不在已实现承诺内。

活动实例控制宿主装饰明暗。现有宿主主题菜单显式设置当前所有实例；新host默认LIGHT。关闭最后实例后根宿主保留当时解析出的显式装饰明暗，不自动切换为SYSTEM。应用自己的模式仍独立。详细错误返回、生命周期、边界及 C 示例见[公共契约](../instance-title-and-theme.md)。

主题测试覆盖两host隔离、完整结构、初始值、16色、通知顺序、不重复通知、重入/错误线程、语言/草稿/数据保留、未初始化COM及已有MTA。解包公共调用方连续20次主题更新保持图像ID1、count1、bytes1112不增长，卸载前显式释放后count0。不能由此宣称所有调用方自绘表面已自动适配。

## 6. 实际构建与完整回归

最终独立 Release 新构建按当前政策分别配置，软件WGL仅部署到独立测试复制目录，不往SDK bin塞WGL覆盖DLL。固定OSMesa/provider及真实WebView2分别运行。精确命令、环境、PID、EXE/DLL/包SHA、逐用例JUnit和原预算见证据 `raw/local-records/regression/*-final`。

| 配置 | 实际用例 | 通过 | 失败 | 物理条件跳过 | CTest进程退出 |
| --- | ---: | ---: | ---: | ---: | --- |
| native Release | 23 | 22 | 0 | 1 | 0 |
| Light Release | 49 | 48 | 0 | 1 | 0 |
| OSMesa Release | 56 | 55 | 0 | 1 | 0 / 0 |
| WebView2 Release | 69 | 68 | 0 | 1 | 0 / 0 / 0 |
| Release优化 + MSVC AddressSanitizer 定向检查 | 11 | 11 | 0 | 0 | 0 |
| 新SDK解包固定依赖离线构建 | 3 | 3 | 0 | 0 | 0 |

四配置重复覆盖合计193通过、0失败、4次同一物理条件跳过，不是193个不同功能。每行唯一跳过为 `ui_monitor_transition`，没有服务无登录或硬件GL替代。当前原CI结果校验器核对精确inventory/required/skip/fail通过，见 `raw/local-records/raw/final-regression-audit.json`。ASAN为11项定向检查，不是所有矩阵的ASAN或LeakSanitizer；包含公共C/C++头、主题、标题、两后端文本、布局、捕获及初始化/挂载失败清理。resilience26.51秒在原30秒限内，未提高门槛。

复现/重建入口命令（历史同名输出禁止覆盖，重新执行应先更换唯一ID）：

```powershell
# 从仓库根，在 VS x64 环境；完整配置参数见对应 cmd。
cmd /c build/interaction-20261017-a7d913e4/tools/build-final-webview2.cmd
python build/interaction-20261017-a7d913e4/tools/regression.py webview2 final
# 其余 native/light/osmesa 为对应 build-final-*.cmd 与 regression.py 参数。
# 实际输入由公开 C 探针/项目 SendInput 路径执行；完整参数在每个 run.json。
python build/interaction-20261017-a7d913e4/tools/run.py <新标签> <探针EXE> <公开参数>
```

这些是本轮执行命令的索引，不建议原样重跑写入旧路径。所有构建cmd、输入驱动源码、runner、完整日志已复制到证据 `raw/local-records/tools` 和各子目录。基线与修复DLL对照使用相同EXE；运行时环境变量只作用于对应诊断子进程。

实际真实输入串行在解锁前台进行，结束已释放桌面。保存按下、移动、松开、键盘导航、命中/捕获/Dispatch/最终状态；Light/Runtime正常OS退出。Runtime显示探针还等待6个实际子进程signaled并核对exitCode0。解包实际宿主亦等待父进程退出0，未用main返回打印替代。

两后端原存储180秒、语言240秒用例在本轮完整矩阵内按原限制通过，未调预算。此为本机结果，不证明原托管超时修复。Mesa专项、259→0时序、同条件B/C及真正卸载仍沿历史独立调查，本轮没有重开或改默认provider。

## 7. 截图及资源/响应比较

完整真实帧位于 `frames`、`frames-more`、`frames-final`，每组 conversion.json 保留原BMP路径/哈希及PNG哈希，转换逐像素验证，不裁切、不重绘。原BMP保留在新构建 runs。覆盖旧 `.002`、修复后逐字/粘贴、长文Home/End、主题焦点、标题第一次失败及最终通过、三档混合GL连续拖动、41行末尾及正常提示分离。画面属于自动真实SendInput验证，硬件鼠标人工验收未完成。

专项捕获成本对照采用同一个 `/O2` 公共C EXE、同Runtime、96 DPI的8组原先即可成功场景，before/after交替各3个独立进程；无并行构建或其他测试。最终数据为 `raw/local-records/capture-cost-final/summary.json`，每侧24次捕获、3个父进程资源样本，全6进程实际OS0：

| 指标 | 原DLL | 新DLL |
| --- | --- | --- |
| CapturePreview→WIC→公共RGBA复制中位耗时 | 114.197ms | 107.981ms |
| 最差捕获耗时 | 299.657ms | 346.399ms |
| 父进程CPU时间，三个样本 | 46.875 / 31.250 / 109.375ms | 93.750 / 93.750 / 109.375ms |
| 采样private峰值范围 | 4,018,176–4,227,072字节 | 4,222,976–4,239,360字节 |
| 采样RSS峰值范围 | 26,972,160–27,328,512字节 | 27,131,904–27,320,320字节 |
| 采样handle / thread峰值 | 269 / 11 | 269 / 11 |

中位略低，最差和CPU不一致，样本少且只观察父进程，不能宣称速度提升或统计无退化。没有把该捕获耗时当作输入到屏幕呈现延迟、60Hz帧率或50ms体验目标。首轮数据与一次2.5秒编译重叠，单独保留于 capture-cost，并明确不作为主对照。原144失败场景不能伪装成可比较的成功吞吐样本。

裁取方案复用输出视口缓冲，不增加完整栅格RGBA副本；334×235与335×236的示例输出少2280字节，原32MiB上限保持。嵌套焦点滚动仅按祖先层级重布局；没有查询全量数据或重建控件。资源断言与实际退出分别通过，但长期资源趋势及完整输入到屏幕呈现P95/P99未测得，不将paint或处理返回冒充呈现完成。

## 8. 失败、校准、撤回和证据边界

以下失败全部保留，没有删除原失败断言或靠延时/放宽预算收敛：

1. 受限MSVC自检C1902，隔离目录提升执行后正常；最初公共探针误用FORM backend/离屏条件、native_parent成员和函数指针转换编译失败，按实际头文件修正测试。
2. 初始显示9失败含3个真正 `.002` 与6个测试把dirty draft当成模型已提交的种子错误；先用公开accept_fields校准并断言尾标记，再保留3项缺陷。数字字段首次误猜不存在的number成员编译失败也保留。
3. 标题测试窄argv中文路径打开失败后空指针访问退出 `0xc0000409`；同EXE纯ASCII路径通过，改UTF-8 wmain和前置条件验证。主题测试移除回调后的计数预期校准，通知顺序断言保留。
4. 首轮69用例为65通过、3失败、1跳过：软件WGL隔离目录漏复制web_parity.html；旧视觉测试直接写内部dark字段、期待已过时popup_kind5及任意捕获尺寸。基线也有旧断言及Runtime pending-capture失败。校准为公共主题设置、精确提示内容、真实挂载槽和DPI；原色值/行数/资源断言保留。
5. Light提示首次受前一阶段队列中的capture-cancel影响，pump(0)排空已有消息；没有增加sleep或超时。144/192浮动探针首次使用错误坐标，修正探针，保留产品原拖动语义。
6. 新公共宿主驱动第一次OS0却未真正点中主题，作为不完整证据；第二次按强化断言失败2项，第三次按实际按钮几何和命令/事件/复制核对通过。最终长标题textarea取代不可达的div候选，保留前后图。
7. 第一次诊断应用C4996日志被本轮重试覆盖，工具回显恢复存为 title-theme-app-first-build-tool-recovery.txt，明确不是原始raw。没有覆盖历史证据。一次视觉构建选错未配置target；随后两个run实际用了旧EXE，不计为新源码验收；另一次自身尚未退出的探针导致LNK1104，等待正常退出后再构建。
8. SDK前两个部分暂存目录因猜错exe名、QuickJS可选gitlink test262目录而失败；保留目录，第三个新目录成功，未把test262当成ACL问题。回归审核脚本首次XML根节点取法错误，audit2修正后通过。
9. 早期registry主题候选保留在本轮raw，正式改公开UISettings；没有修改系统注册表。初次捕获成本对照受编译影响，最终静态条件数据独立保存。
10. 本轮补收5个provider用例完整stdout时复用了**本轮新建** final-webview2/session0-control工作目录；其现存内容属于补充运行，不是原最终矩阵截图。原矩阵CTest/JUnit、阶段PID/耗时/退出和storage/language各自证据仍保存；早期summary把failed也列入skip的错误原记录保留，最终audit精确区分。没有复用历史证据目录。

## 9. 新SDK、解包复验及兼容

最终归档：

`build/interaction-20261017-a7d913e4/delivery/ui-framework-sdk0.9.0-dev-api9-1af1f5a9-worktree-599800c363e1-windows-x64.zip`

大小 **74,305,928字节**，**1620文件**，SHA256：

`c2b5fe17daeb169b5fc4156cfb4f9f3aaf390a46dcfa8256a241445e7067fce8`

见 [sdk-final-archive.json](interaction-20261017-a7d913e4-evidence/raw/sdk-final-archive.json)。封存后不补写，本报告及最终外部证据不塞回SDK。包内source-identity与sha256.json描述封存时源码/说明；代码身份与最终工作树逐文件匹配。封存后维护示例README仅去除一个多余EOF空行，另新增外部验证报告；不宣称这些最终说明字节也在封存包内。

匹配头文件、导入/静态库、共享DLL、宿主、维护示例包、公共C诊断包/驱动、固定依赖源码及库一并交付。未覆盖44c039f冻结SDK，也没有往原有结构无版本保护插字段。API9开发分支以独立结构/新增导出增补，旧开发快照没有新符号，使用者须使用同包产物。

真正从新ZIP解压到 `unpack/final/sdk`，CRC及包内逐文件SHA全部匹配。测试后包内原1620文件仍匹配；运行解包文本测试的默认WebView2用户目录另外生成177个profile文件，仅在新解压目录，未进入归档。保留这些运行产物，不把1797个现存文件误称为包有1797文件。首次post-test元数据的 `noArchiveOrUnpackAmendment` 把“原文件未改”与“没有新增文件”混为一谈，原记录保留，以 delivery-preservation-after-tests2.json 及其额外文件清单澄清。实际复验包括：

- 只用解包头/库的C调用方：后台20次改名、20次主题更新、稳定ID/图像资源、正常关闭，OS0。
- 只用解包头/库的C++头编译运行，OS0。
- 解包两后端文本程序、标题程序分别OS0。
- 解包真实宿主+公开诊断包执行标题/主题/切换/关闭，实际宿主OS0，连续6画面。
- 解包source与固定离线依赖重新构建240步，主题core、C++公共头、实际OSMesa三项通过，未下载或升级依赖。
- 旧44c039f API9头及旧web_counter.uapp，链接新库/新DLL，公开调用/关闭OS0；定向兼容按本轮用户明确要求执行，不改写历史current_only事实，不重新引入API1–8兼容专项。

历史保护核验162项匹配，包括原SDK在内5个受保护产物记录、14个原证据索引、143份原语言证据；18张原PNG未改。两张已知历史BMP存在原CRC问题，没有读取修复、替换或重生成。业务应用、PERF-001未写。封存SDK解包复验后再次哈希见外部 delivery-preservation-after-tests2.json。

## 10. 文件范围、回退及未验证项

产品改动集中在 Light编辑/焦点、Runtime捕获、workspace标题、theme实现及host消息转发；公开头、独立宿主和维护示例同步。测试/构建清单、公共调用方和当前说明同步；逐文件清单及原/新SHA见 raw/final-change-manifest.json，完整git差异见 raw/final-worktree.patch。

仅回退本轮源码/当前说明：使用证据中的 `raw/rollback-current.ps1` 默认只检查。它要求HEAD、入口ZIP及所有目标文件最终SHA匹配，无该范围暂存修改；`-Apply`才从入口ZIP恢复本轮修改的tracked文件、删除清单内本轮新增产品文件。非递归、逐文件、LiteralPath、先核验路径位于仓库；保留原11个未跟踪路径、新构建/交付/全部证据和本报告。没有执行Apply，没有git reset/clean。用户之后改过任一目标会拒绝，避免覆盖后续修改。

继续独立列为未验证或尚未定位：当前源码Session0服务环境、严格整机无登录、真实英文Windows、真实中文IME、物理高/混合DPI及多屏、桌面合成专项、硬件鼠标和长期人工体验、系统主题实际切换及特殊只发WinRT事件环境、历史泛化叠层/窄表头残影、原托管180/240秒根因。未注销用户、改既有服务、降低严格无登录门槛、变更默认provider或提高超时；未提交、推送、发布、打标签或触发远程工作流。
