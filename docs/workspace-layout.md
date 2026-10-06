# 工作区布局：API7

SDK0.7.0开发版、API7、标准修订7；Windows x64/C11/C ABI。接口见[shell.h](../include/ui_framework/shell.h)，实现见[Win32 shell](../src/platform/win32_shell.c)与[布局实现](../src/platform/shell_layout.inc)。应用ABI和包格式仍为1，旧API1–6描述及默认布局继续兼容。

设计顺序是保存/恢复、分隔条、拖拽停靠。当前模型支持左右侧栏的有序面板栈、面板高度、单面板折叠/关闭、浮动窗口与侧栏宽度，复用既有内容槽、组件和surface生命周期。API7新增底部和同区标签组；不支持任意嵌套分割树。

## 保存与恢复

应用负责存储布局字节、选择用户/文档/实例的存储位置，并在注册面板和挂载内容之后恢复。框架不会代写注册表或业务文档，也不恢复应用的业务数据。实例重新打开时，由应用重新提供注册描述，再加载对应布局。

面板ID是稳定、非空的UTF-8注册ID，布局格式最多63字节；显示标题和数组索引不是身份。改名显示标题不影响恢复，修改ID视为移除旧面板并增加新面板。保存和手势至多支持512个面板，字节预算64KiB。

```c
size_t bytes = 0;
ui_shell_t *shell = ui_host_get_shell(context->host);
if (ui_shell_save_layout(shell, NULL, 0, &bytes) == UI_STATUS_OK) {
    void *data = malloc(bytes);
    if (data && ui_shell_save_layout(shell, data, bytes, &bytes) == UI_STATUS_OK) {
        /* 应用把 data[0..bytes) 写入自己的存储。 */
    }
    free(data);
}
/* 加载后检查返回值；异常文件可选择 ui_shell_reset_layout(shell)。 */
```

`ui_shell_restore_layout`先验证完整输入，未知版本返回UNSUPPORTED，长度、枚举、标志、尺寸、DPI、ID或重复ID非法返回INVALID_ARGUMENT，验证失败不修改布局。缺失面板记录忽略，新注册面板保留当前默认状态。平台重排错误另行返回，不能以成功解析文件推断全部平台调用完成。`ui_shell_reset_layout`恢复shell创建时的侧栏布局和面板注册状态，不重建内容或清除编辑草稿。

保留的格式1是小端二进制，与C结构packing无关：80字节头包含`ULYT`、版本1、记录数、总长度及16个原host布局整数；每个104字节记录包含64字节NUL结尾ID、停靠区、floating/collapsed/closed三个标志位、顺序、高度、浮动x/y/width/height、DPI及必须为0的保留字段。应用应把它作为不透明字节保存，升级格式时以新版本明确拒绝旧读取器。

尺寸为逻辑像素，浮动原点是按所记DPI换算的屏幕坐标。恢复到显示器减少或work area变化的环境时，窗口限制到最近显示器可用区域；处理WM_DISPLAYCHANGE和work area变化，布局/DPI调整时也夹紧浮动窗口。没有把程序DPI注入当作真实跨屏验收。

## 分隔条与拖拽

`ui_shell_resize_splitter(shell, region, NULL, width)`调整左右侧栏宽度，遵守原布局min/max与主内容宽度策略。指定panel ID时，调整它和下一停靠面板的高度，双方至少72逻辑像素；空间不足返回CANCELLED。窄窗继续使用原侧栏折叠/浮动策略和入口。高度是栈内权重，窗口resize按权重分配剩余空间，折叠只保留标题。

有窗口下，`ui_shell_begin_splitter_drag`捕获指针，移动时更新尺寸，松开提交；Esc或捕获丢失恢复手势开始时的布局。开始、修改或提交时检查活动实例、模态和关闭门控。

面板拖拽由begin/update/end三个入口组成。update输入host局部逻辑坐标并返回预览和目标区：左右外侧四分之一命中相应栈，中央底部四分之一命中底部、已有标题命中标签组，其余（含宿主外）为浮动；停靠按指针相对面板中线插入并重新编排顺序，重复手势不会累计溢出。窗口路径显示预览并捕获指针，松开提交，Esc/捕获丢失/实例失活取消。只有提交才改变停靠位置。

独立宿主提供标题拖拽、折叠按钮、标题下缘高度分隔条、侧栏内缘宽度分隔条与重置按钮；浮动Web标题可拖回侧栏。原生嵌入路径启用API6布局后可从面板标题拖拽，折叠标签可展开。嵌入应用自行提供存储入口，也可调用公共API控制手势。

布局操作保留内容槽、Web view、surface和GL context，改变父容器/矩形并使用原resize路径。有效焦点恢复到活动实例中仍可接收输入的原对象；隐藏或折叠后不强制把焦点送到不可见对象。组件草稿、稳定ID选择和有效单元格编辑仍属于原组件，不因重排重新mount。

## 无窗口边界与验收

Windows完整运行库的`UI_RUN_OFFSCREEN`保存/恢复同一布局和逻辑停靠、浮动、折叠状态，resize与逻辑拖拽不创建HWND。浮动是逻辑矩形，没有实际显示器、窗口预览或原生捕获；原生分隔条手势返回UNSUPPORTED，调用方直接提交逻辑尺寸。OSMesa继续绑定内存，隐藏WGL路径保持原窗口依赖声明。

`ui_framework_headless`是核心状态测试适配器，不包含这个Windows布局管理器；不能用其UNSUPPORTED返回冒充Windows无窗口功能的完成证据。

[workspace_layout测试](../tests/workspace_layout.c)覆盖格式、原子验证、缺失/新增ID、512记录、部分ABI字段、重复拖拽、分隔条取消、程序DPI与屏幕夹紧。[独立应用集成](../tests/api6_integration.c)在轻量离屏及真实WebView2中检查草稿、焦点、选择、GL、双实例、模态和卸载。最终结果与物理条件边界见[API6验收](validation/api6-validation.md)。

## API7 底部与同区标签

BOTTOM=9，原COUNT=8不改（8保留）。底部位于两侧栏之间，用主区底部空间；set_bottom_height默认220、范围28..32767，实际给主区保留120，空间不足分配一半。左右/底部均是平面有序栈，面板高度仍为权重，底部上缘为高度分隔条。无窗口调用逻辑尺寸，捕获分隔条仅有窗口。

dock_panel_tab(moving,anchor)把已有面板移入anchor同区组并激活，浮动anchor返回UNSUPPORTED。非零uint64组ID属于shell，0为独立面板，UINT64_MAX保留。tab_group_id偏移48、tab_active偏移56，只按完整字段访问，旧48字节get/set有效；旧尺寸同区set保留组，移区/浮动脱组，新字段明确0可脱组。活动页可见，非活动页槽/view/草稿/选择/GL存活；关闭活动成员选其他未关闭成员，activate可重开，reset清组恢复注册状态。

标题28像素带拖拽命中组，预览整框；提交入组、取消不改。中央下四分之一命中底部，左右外四分之一栈插入，其余浮动，标题优先。窗口显示预览并捕获，无窗口只返回逻辑预览。独立宿主每次最多4标签及前后入口；原生复用标签按钮，每页4个，点击活动标签循环下一成员（含关闭页）。任意成员也可activate，reset是全部入口恢复路径。

格式2：96字节头保留原80，80为底部高度u32、84必须0、88为next_group_id u64；112字节记录保留原104，flags位3为tab_active、104为group_id u64。小端、512记录/64KiB不变。组必须同区且非浮动，最多一个活动标志，next非0且大于所有组ID。先完整验证后应用；缺失ID跳过、新增ID保留，丢失活动成员自动选可用页。格式1仍按80/104读取，组0、底部220；未来UNSUPPORTED，异常输入不部分应用。API1–6 host未启用新能力保存格式1，API7或使用新能力保存格式2；旧运行库不能读格式2，应保留迁移备份并检查返回。

[workspace7](../tests/workspace7.c)覆盖原生/Web shell、Windows无窗口、底部/标签/关闭重开/同槽、提交取消、格式2/未来/异常/部分字段。原[workspace_layout](../tests/workspace_layout.c)用冻结SDK6保留全部格式1断言。集成与边界见[API7验收](validation/api7-validation.md)。

API7面板注册的dock_region可直接声明BOTTOM，NONE保留原RIGHT默认；初始有/无窗口布局为其分配底部空间，reset回到注册默认底部。直接声明与事后移动同样保留内容生命周期，见[注册复现和修复](validation/api7-validation.md)。

当前交付来源、CI与环境限制见[收敛验收](validation/api7-delivery-validation.md)。真实业务布局存储/恢复试点步骤见[准备清单](business-pilot.md)，框架测试DLL不替代业务文档恢复验收。

## 自有外框视觉

独立宿主及Web浮动标题使用[统一视觉规范](visual-design.md)：28逻辑像素紧凑标题、20像素标题工具、平面标签及中性分隔条，悬停/拖动有强调状态。主题切换只更新外框/共同模板，不重建内容槽、GL context或组件；格式2读取1、面板ID、恢复/草稿/选择/焦点及布局手势合同不变。原生嵌入标题、屏幕停靠预览仍按实际Win32实现呈现；物理跨屏/桌面合成另验。[本轮回归](validation/visual-ui-validation.md)。
