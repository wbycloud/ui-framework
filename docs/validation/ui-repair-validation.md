# API9 界面修复、公共布局与 CI 遗留验收

2026-10-09接手干净 `main` / `359085b58d5c69792533e3b781fd002cd481314b`。先阅读CI恢复提示、第21/22节交接、同步及恢复验收；未发现仓库内AGENTS.md，遵守用户提供的AGENTS规则。SDK0.9.0-dev/API9/标准9、ABI1/包格式1保持。本轮仅框架本地修改及提交，没有推送、远程工作流、业务应用、冻结SDK、历史证据或PERF-001修改，没有graph-engineering、子代理或新会话。

原CI目录437个文件的长度/SHA256在接手及交付时全部匹配，冻结SDK哈希仍为 `7c1dbffce1211b2c0654b41d2942b05adbd4c54d111e983d836c27550d8802e1`。[接手](ui-repair-evidence-20261009/raw/takeover.json)、[保留核验](ui-repair-evidence-20261009/raw/preservation-delivery.json)。新原始证据在 `build/ui-repair-20261009`；该目录和SDK ZIP不会随clone下载。可随Git取得的日志、JUnit、阶段manifest和无损截图裁剪另存[证据目录](ui-repair-evidence-20261009/README.md)，不覆盖历史目录。名为baseline的构建树已逐次重建，当前是修复后的代码；旧DLL红例有独立副本及哈希，不能把当前baseline树当作旧二进制。

## 结果与责任边界

| 项目 | 定位与本轮结果 | 状态 |
|---|---|---|
| 宿主中文固定断言 | 已有f00bbeb修复，核验系统来源及实例中英文完整合同，当前相关用例通过；未重复修改 | 已修复并本地验证；真实英文Windows空宿主仍待环境 |
| ui_host_repaint文案失败 | 既有英文红例准确失败于固定中文完成提示；当前中英文、实际GL结果、失效区、空闲paint及关闭检查通过 | 文案已定位并验证；历史持续重绘仍未定位 |
| 完整存储/语言托管超时 | 当前完整六组脚本实际退出，阶段/PID/调度/elapsed/退出码可查；本机没有复现原180/240秒阻塞 | 未定位、未宣称修复托管超时 |
| 浮动反馈、释放偏移/尺寸、边角命中 | 旧DLL实际输入红例；修复窗口层级、保存几何及实际可命中边缘，GL/草稿两组逐帧验证 | 已修复并通过原生输入/可见画面；硬件鼠标人工验收未执行 |
| 只读多行 | 旧DLL28px及不可达末行；模板高度、Light文字滚动范围及持久偏移修复；两后端41行、更新、窄窗、内外滚动实际末行可读 | 已修复并本地验证；物理DPI另列 |
| 提示尺寸与生命周期 | 短提示原320×180、离开残留；内容测量、区域限制和拥有者生命周期修复，公开隐藏覆盖内部标题提示 | 已修复并本地验证 |
| 公共对话框布局 | 新增独立首选/最小/最大客户区描述、字段行数/高度；真实约束、模态/键盘、说明/错误可读；补修Light长说明自身滚动 | 已实现并本地验证；匹配开发SDK，未发布稳定版本 |
| 长可编辑文字End | 独立公共C旧DLL四项失败；Uniscribe尾部CP计算错误已定位，实际尾部/光标通过 | 已修复并验证；Home原已正常，保留回归 |
| 泛化程序192叠层、窄表头残影 | 公共C宽/窄及96/192程序DPI没有独立复现；排除预期自身提示后留桌面图，缩窄/新建表头像素相等 | 尚未定位，不宣称消失或修复 |
| 后续工作区功能 | [范围、接口影响、验收及预算前置需求](../workspace-feature-candidates.md) | 方案交付，未自动实现 |

源码线索、实际失败、根因定位和修复通过分别按下文证据认定；自动状态成功不能代替画面可读性。首轮Light长说明和Runtime滚动条操作均出现“探针0失败但画面不合格”，原记录保留，最终结论使用后续独立画面。

## 浮动、停靠与调整尺寸

根因：预览原为宿主子窗口，GL和浮窗处于不同窗口层级；释放固定320×240并把鼠标释放点当左上角。内容窗口占据浮窗边缘，只改父窗口NCHITTEST无法保证收到命中；标题mousedown先抢焦点，保存的成为标题焦点。[原拖动日志](ui-repair-evidence-20261009/raw/red-float.log)实际390→320且抓取偏移失败，[失败画面](ui-repair-evidence-20261009/screenshots/red-float/002-held-gl.png)保留遮挡及残线。旧共享DLL副本SHA256为 `0fff7943a2b6dd767614634c5c7b6c0d06fc8f0f0e8f7048e045127119f222cc`，原历史文件未动。

修复为独立、非激活、透鼠标、镂空的顶层预览；蓝色自由移动、绿色停靠，坐标从实际客户区转换。保存实际尺寸和鼠标抓取偏移，浮窗留8逻辑像素边缘，标题消息前保存内容焦点。保留原内容HWND、视图和GL上下文。

最终产品07a66a8的公共C探针只加载框架，不加载业务DLL。SendInput经过Windows鼠标/键盘输入路径，桌面BitBlt记录实际窗口；每帧包含单调tick、按键状态、捕获、WindowFromPoint得到的HWND/类名/PID及焦点。输入串行、单一探针进程，GL为实际窗口WGL。它不是调用拖动API或发送窗口消息的验收，也不是物理鼠标人工验收。捕获丢失在真实鼠标按下后由SetCapture/ReleaseCapture受控注入，未验证外部软件抢占。

两模式各29帧，包括10帧经过GL、左右/底部目标切换、5帧从停靠转自由浮动、Esc及捕获丢失。逐帧审阅蓝/绿轮廓持续可见；没有将最终位置代替按住过程。[GL日志](ui-repair-evidence-20261009/raw/accepted-07-float.log)、[草稿日志](ui-repair-evidence-20261009/raw/accepted-07-float-form.log)、[GL按住连续画面](ui-repair-evidence-20261009/accepted-07-float-held-contact.png)、[草稿按住连续画面](ui-repair-evidence-20261009/accepted-07-float-form-held-contact.png)。接触图仅用于浏览，逐帧无损PNG及原BMP身份在screenshots.json。

释放保留390×240及86/20抓取偏移；7px角点实际命中浮窗HWND并出现NWSE光标，原生鼠标调整到440×290。底部停靠后再次浮动保留调整后的尺寸和抓取点；Esc/捕获丢失回滚并销毁预览。实际键入的Z草稿、内容HWND、焦点、HGLRC均保留。短标题内部提示由公开拥有者隐藏及开始拖动关闭。物理高DPI下边角、外部捕获抢占、长期人工组合未执行。

## 只读 FORM 与文字滚动

旧DLL带READONLY|MULTILINE的[红例](ui-repair-evidence-20261009/raw/red-readonly2.log)字段仅28px；不以最初漏只读flags的red-form当只读证据。Light既缺少textarea自身文字范围，也在hover/layout后自动按光标重置读者滚动位置。模板默认改为98px四行、最低42px；Light按实际行高/宽度计算范围，绘制裁剪与滚动条分开，读取滚动偏移持续保留，显式光标导航/编辑才重新保证光标可见。多字段容器允许完整范围滚动。

最终Light与实际WebView2输入验证：41行LF，第20行含长行，完整末行为 `FINAL LINE 41: COMPLETE END MARKER 20261009`，滚轮到达末行。更新为三行后完整显示新末行；随后41行CRLF更新，以 `END41: COMPLETE 20261009`为末行，42/65/160px高度及350px窄窗都实际看到完整标记。尝试只读输入后内容不变。六字段外层滚轮实际到达 `OUTER END6: ALL CONTENT`，不以clip非零替代可读性。

[Light最终日志](ui-repair-evidence-20261009/raw/accepted-07-form.log)、[LF末行](ui-repair-evidence-20261009/screenshots/accepted-07-form/001-form-wheel-end.png)、[窄窗65px末行](ui-repair-evidence-20261009/screenshots/accepted-07-form/008-height-65-wheel-end.png)、[内层滚动条末行](ui-repair-evidence-20261009/screenshots/accepted-07-form/013-field-scrollbar-end.png)、[外层最后字段](ui-repair-evidence-20261009/screenshots/accepted-07-form/014-outer-wheel-last-field.png)。Runtime滚轮/42px/窄窗见[原日志](ui-repair-evidence-20261009/raw/accepted-07-form-webview2.log)及同名截图目录。

Runtime滚动条拖动首轮仍在首行，虽进程exit0也不通过该画面项。原输入点距字段右边5px落在边界；实际截图定位滚动块后，改为内侧12px。只修改探针，不修改Runtime产品。最终[原生输入日志](ui-repair-evidence-20261009/raw/runtime-bar-verified.log)、[按住末行](ui-repair-evidence-20261009/screenshots/runtime-bar-verified/011-bar-held-end.png)、[释放并移开后的完整末行](ui-repair-evidence-20261009/screenshots/runtime-bar-verified/013-field-scrollbar-end.png)独立通过。旧首行图、校准探针源码及二进制身份保留；实际命中不可由API状态或无失败退出代替。

## 提示的尺寸、范围与拥有者

原提示按最小320×180布置，离开后缺少完整锚点生命周期；内部浮窗标题提示属于shell内部host，公开主host隐藏未覆盖。现在按字体/内容测量，最大384×240逻辑像素，并与可见拥有者及监视器工作区取交集；长词测量后再次夹宽，物理转逻辑向下取整，避免程序192下大于可见宽度。短标题/短值不再被长内容预算撑大。

鼠标离开已经进入的锚点、锚点/拥有者移动或隐藏、字段/标题更新、开始拖动/停靠、关闭或销毁均结束旧提示。程序/键盘在指针尚未进入锚点时调用公开显示仍可查看，不立即关闭。ui_host_hide_tooltip覆盖自身和shell拥有的内部面板标题host；独立应用host及正常菜单保持各自范围。所属范围及兼容性在[公共声明](../../include/ui_framework/menus.h)及合同中明确，没有禁用全部提示或隐藏业务内容。

短提示实际detail35×21，短值“7”小于100×60；离开、内容更新、拥有者移动、组件销毁、host销毁、内部标题公开隐藏和开始拖动通过。独立host提示在另一host公开隐藏后仍存在。[输入/状态日志](ui-repair-evidence-20261009/raw/accepted-07-form.log)记录边缘程序96与192下工作区0,0..2560,1392；程序192窗口报告96，提示334×380物理像素，未超过拥有者334×381。对应[短值](ui-repair-evidence-20261009/screenshots/accepted-07-form/023-short-value-tip.png)、[192边缘](ui-repair-evidence-20261009/screenshots/accepted-07-form/025-edge-tip-dpi-192.png)。长内容受限制但保留内容/滚动入口；不把预期自身提示层当成泛化DPI叠层根因。

## 公共对话框能力与 ABI

新增完整size的独立ui_dialog_layout_t、ui_field_layout_t，以及ui_component_set_dialog_layout、ui_component_get_dialog_layout、ui_component_set_field_layout。前两者用于DIALOG，字段布局用于FORM/DIALOG多行TEXT；都在UI线程调用，id仅调用期间读取。首选/最小/最大均为96DPI逻辑客户区尺寸，零首选高度按内容计算，height优先于行数。系统边框按窗口报告DPI计算，内容按程序DPI计算。完整范围/默认/原子拒绝/重开保留规则见[公共合同](../dialog-layout.md)，没有改变原结构偏移或枚举。

Light及实际Runtime的ui_component_layout各覆盖12组合：96/144/192程序DPI、单/三字段、260/600宽、长说明/错误、size/范围拒绝、行高与只读、close/reopen视图/草稿、按钮clip。96DPI原生输入另验证单字段初始内容高度、实际角点约束240×120及600×230客户区、拥有者模态禁用、Tab、Enter恰好一次、Esc恢复拥有者、重开后真实取消按钮。窄对话框可到达末字段和完整错误；按钮一直可见/可操作。

首轮探针0失败但Light长说明图仍未显示末行，Runtime已显示；保存失败后用公共C ui_light_scroll_test链接e0dcf78 DLL，两个场景均失败。根因为通用滚动文字叶节点只累计子节点高度，忽略自身换行高度，绘制也未应用独立文字偏移。07a66a8修复范围计算及绘制裁剪，增加窗口/离屏的滚轮前后真实像素回归。[旧DLL失败](ui-repair-evidence-20261009/raw/help-red.log)、[失败退出](ui-repair-evidence-20261009/raw/help-red-exit.log)、[聚焦回归](ui-repair-evidence-20261009/raw/help-focused.log)。最终原生滚轮图完整显示 `HELP END: COMPLETE`：[Light](ui-repair-evidence-20261009/screenshots/accepted-07-form/022-dialog-help-end.png)、[Runtime](ui-repair-evidence-20261009/screenshots/runtime-bar-verified/022-dialog-help-end.png)。原visible-form/022-dialog-help-end失败图保留。

C11头文件断言保留现有结构，新增描述x64 sizeof分别28/24、id偏移8、rows16、height20；实际DLL导出及独立示例导入均检查。[公共C示例](../../examples/dialog_layout/main.c)、版本政策及标准的开发快照要求同步。SDK0.9.0-dev/API9/ABI1/包1保持；冻结API9 DLL不含新导出，必须使用同一开发commit的headers/import library/DLL，不能只凭API数字混用或称稳定发布。

独立匹配快照：build/ui-repair-20261009/ui-framework-sdk0.9.0-dev-api9-07a66a8-windows-x64.zip，915943字节，SHA256 `b5296fba8d39447603222705ab85edcf446fa5c98088d86c76a153a4fd9c60f0`。[归档身份](ui-repair-evidence-20261009/raw/sdk-07a66a8-archive.json)、[包内manifest](ui-repair-evidence-20261009/raw/sdk-07a66a8-manifest.json)。解包独立C调用方以/W4 /WX编译、链接解包导入库并实际载入解包共享DLL，三个函数及六行136px配置0失败：[编译](ui-repair-evidence-20261009/raw/sdk-consumer-07a66a8-build.log)、[运行](ui-repair-evidence-20261009/raw/sdk-consumer-07a66a8.log)。这是ABI/加载/状态证明，可见性以上述输入为准。Runtime及OSMesa仍是外部依赖，未覆盖冻结归档或打稳定标签。

## 显示调查与程序 DPI

[公共C最小复现](../../tests/display_probe.c)不加载业务代码，958字符文字、明确START/TAIL标记，四列表头；串行单一进程、实际焦点和命中HWND确认。桌面BitBlt与公共RGBA表头读回分开。记录一台2560×1440显示器，effective96、raw108、窗口96；host显式96/192。程序设置192不是物理192验证。

End旧DLL四次实际右边缘断言失败；早期仅“光标在clip内”用例通过也没有证明尾部可见，因此另保留更强红例。诊断显示最终CP=count958传入Uniscribe返回80070057/x0；改为最后字符CP=count-1的trailing TRUE，空文本保持原空路径。新输入在900/350px、程序96/192均到右边缘并完整暴露TAIL_MARKER_20261009，Home回首正常。[旧失败](ui-repair-evidence-20261009/raw/display-red-assertion.log)、[诊断](ui-repair-evidence-20261009/raw/display-debug.log)、[最终日志](ui-repair-evidence-20261009/raw/accepted-07-display.log)、[窄窗192尾部](ui-repair-evidence-20261009/screenshots/accepted-07-display/008-dpi-192-width-350-End.png)。

泛化叠层调查先用公开隐藏关闭预期自身文字提示，只改变复现场景，不禁用产品提示。窗口内实际指针命中本探针HWND、焦点属于该视图；桌面其他窗口位于探针后方，系统调整尺寸时指针移到窗口外的外部HWND记录不当作内容命中。缩窄表头与同宽新建表头公共RGBA逐字节相等，旧DLL/新DLL均如此；真实桌面截图同时保存。这些场景未复现程序192泛化叠层或窄表头残影，不能归责业务、凭单图改产品或宣称问题消失。需具体公共接口序列、持续画面和真实物理环境补验。

## 完整回归与 CI 遗留

最终产品07a66a8构建后重跑受影响三配置；独立无Web原生配置在e0dcf78执行后，07a只修改其未编译的Light路径，最终native重建报告no work。下面原生结果明确沿用先前执行，其余均最终代码重跑。4fdfcc2只校准手动探针命中点，未改变产品或CTest用例，无需重跑完整矩阵。

| 配置 | 通过/失败/跳过 | 秒 | 最终证据 |
|---|---|---|---|
| native | 21/0/1，22项 | 9.47 | [JUnit](ui-repair-evidence-20261009/raw/matrix-native/ctest.xml)、[未受影响重建](ui-repair-evidence-20261009/raw/native-unchanged-after-07.log) |
| Light | 45/0/1，46项 | 81.55 | [JUnit](ui-repair-evidence-20261009/raw/matrix-final-light/ctest.xml) |
| OSMesa | 52/0/1，53项 | 244.35 | [JUnit](ui-repair-evidence-20261009/raw/matrix-final-osmesa/ctest.xml) |
| WebView2 | 63/0/1，64项 | 410.65 | [JUnit](ui-repair-evidence-20261009/raw/matrix-final-webview2/ctest.xml) |

合计185项，181通过、0失败、4次同一物理ui_monitor_transition跳过；不是181个互不重复场景。各配置经原精确清单/阶段/失败/唯一跳过校验器判定PASS，[最终汇总](ui-repair-evidence-20261009/raw/matrix-final-summary.json)、[校验器](ui-repair-evidence-20261009/raw/ci-plan-final.log)。没有减少旧断言或额外跳过。新公共布局用例单独60秒；原repaint45、完整Light存储180/语言240、Runtime存储240/语言300秒及资源门槛保持。

WGL局部部署Mesa四DLL/GALLIUM_DRIVER=llvmpipe；进入显式OSMesa/provider7项前移除四DLL，Runtime11项使用实际安装Runtime，避免提供方污染。[准确命令](ui-repair-evidence-20261009/README.md#重建与运行)及分阶段日志保留。MSVC19.50.35727、C11 Release/Ninja、Windows10.0.26300、Session1；Lexbor/QuickJS固定干净，SDK1.0.4129.50、实际Runtime154.0.4258.62、Mesa24.3.4；输入不并行运行。[实际环境及只读本地远端跟踪引用](ui-repair-evidence-20261009/raw/environment-delivery.json)、[已提交源码身份](ui-repair-evidence-20261009/raw/committed-source-identities.json)分别保存；本轮没有网络查询或更新远端。

没有重复修正f00bbeb的中文固定断言，也没有改写2fa6f84的阶段/PID/tick/实际Dispatch/elapsed/退出诊断。[既有恢复红例及合同](ci-recovery-validation.md)保持。真实系统来源只有zh-CN，en-US来自公共实例提交及单元受控映射；没有宣称真实英文Windows空宿主已验。

六份最终完整脚本manifest所有子进程exited/exit0：三个存储各26例，耗时35.59/33.65/35.44秒；对应语言各7例，耗时66.05/64.74/65.73秒（baseline Light/Runtime、OSMesa Light）。[身份及汇总](ui-repair-evidence-20261009/raw/script-runs-final-summary.json)指向保存的阶段日志、PID和running/exited manifest。顶层manifest没有虚构exitCode/elapsed，整项退出依据JUnit及运行器phase exit。原180/240秒Timeout未复现；没有阻塞线程栈/调度等待证据，不据本机耗时推断托管根因。历史持续重绘也未独立定位；文字修复、空闲paint0和正常关闭不能替代根治证据。

## 实际本地提交与剩余验证

| 实际提交 | 范围 |
|---|---|
| af36e02f5483dd174d91d32da1868dcbb162dc82 | 浮动几何/预览/实际命中、提示隐藏范围及生命周期、公共C复现 |
| e0dcf788365c0023b57f7f41734f5e488477a70c | 多行可达、文字End、提示边界、公共布局C声明/导出/示例/兼容性与测试 |
| 07a66a82370db93c49a66c90db5e217a8812e60e | 长说明自身滚动及像素回归、实际对话框约束/说明画面 |
| 4fdfcc2d5f679166064355abe08bc8e855f6a5eb | Runtime滚动条真实输入点校准，产品无变化 |

文档/证据在以上提交后另行本地提交，其SHA用 `git log -1 --format=%H -- docs/validation/ui-repair-validation.md`查询，最终回复报告确切值。[产物身份](ui-repair-evidence-20261009/raw/artifacts-07a66a8.json)、[输入探针身份](ui-repair-evidence-20261009/raw/accepted-07-provenance.json)、[校准探针身份](ui-repair-evidence-20261009/raw/runtime-bar-verified-provenance.json)区分各次二进制/源码。

原b6a36fee严格LocalSystem/ServiceMain Session0实际应用DLL/OSMesa GL：failures0、commands2000、created/process HWND0，**通过**。同次严格整机无登录：logged_sessions1、inventory_valid1，runneradmin已登录Session2，**失败**。两项历史结论保持，本轮无当前源码服务复验；Session1普通ui_session0_interactive_control及桌面输入不是Session0或无登录验证。

剩余步骤：获得未来远程复验授权后，在原英文托管配置以原预算运行，保存最后running case/PID/tick/Dispatch/关闭/卸载/实际退出；若卡住采集当时线程栈及调度等待，不能先增加超时或资源。历史持续重绘需独立持续实际Dispatch/paint/失效区域轨迹，不能由本机单次成功结案。当前未执行该远程步骤。

物理鼠标长期按住连续画面、外部捕获抢占、真实IME、物理192及不同DPI跨屏/边缘合成、当前源码严格Session0和真正无登录环境仍未执行或受环境限制。硬件/人工验收须在具备条件的桌面另存完整过程；不能把SendInput、程序192、状态测试或历史服务结果替代。泛化叠层/残影保持尚未定位，后续候选只交付方案。
