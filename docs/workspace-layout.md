# 工作区布局：API6

SDK0.6.0开发版、API6、标准修订6；Windows x64/C11/C ABI。接口见[shell.h](../include/ui_framework/shell.h)，实现见[Win32 shell](../src/platform/win32_shell.c)与[布局实现](../src/platform/shell_layout.inc)。应用ABI和包格式仍为1，旧API1–5描述及默认布局继续兼容。

设计顺序是保存/恢复、分隔条、拖拽停靠。当前模型支持左右侧栏的有序面板栈、面板高度、单面板折叠/关闭、浮动窗口与侧栏宽度，复用既有内容槽、组件和surface生命周期。没有新增底部停靠、标签式停靠或任意嵌套分割树。

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

格式1是小端二进制，与C结构packing无关：80字节头包含`ULYT`、版本1、记录数、总长度及16个原host布局整数；每个104字节记录包含64字节NUL结尾ID、停靠区、floating/collapsed/closed三个标志位、顺序、高度、浮动x/y/width/height、DPI及必须为0的保留字段。应用应把它作为不透明字节保存，升级格式时以新版本明确拒绝旧读取器。

尺寸为逻辑像素，浮动原点是按所记DPI换算的屏幕坐标。恢复到显示器减少或work area变化的环境时，窗口限制到最近显示器可用区域；处理WM_DISPLAYCHANGE和work area变化，布局/DPI调整时也夹紧浮动窗口。没有把程序DPI注入当作真实跨屏验收。

## 分隔条与拖拽

`ui_shell_resize_splitter(shell, region, NULL, width)`调整左右侧栏宽度，遵守原布局min/max与主内容宽度策略。指定panel ID时，调整它和下一停靠面板的高度，双方至少72逻辑像素；空间不足返回CANCELLED。窄窗继续使用原侧栏折叠/浮动策略和入口。高度是栈内权重，窗口resize按权重分配剩余空间，折叠只保留标题。

有窗口下，`ui_shell_begin_splitter_drag`捕获指针，移动时更新尺寸，松开提交；Esc或捕获丢失恢复手势开始时的布局。开始、修改或提交时检查活动实例、模态和关闭门控。

面板拖拽由begin/update/end三个入口组成。update输入host局部逻辑坐标并返回预览和目标区：左右外侧四分之一命中相应栈，其他位置为浮动；停靠按指针相对面板中线插入并重新编排顺序，重复手势不会累计溢出。窗口路径显示预览并捕获指针，松开提交，Esc/捕获丢失/实例失活取消。只有提交才改变停靠位置。

独立宿主提供标题拖拽、折叠按钮、标题下缘高度分隔条、侧栏内缘宽度分隔条与重置按钮；浮动Web标题可拖回侧栏。原生嵌入路径启用API6布局后可从面板标题拖拽，折叠标签可展开。嵌入应用自行提供存储入口，也可调用公共API控制手势。

布局操作保留内容槽、Web view、surface和GL context，改变父容器/矩形并使用原resize路径。有效焦点恢复到活动实例中仍可接收输入的原对象；隐藏或折叠后不强制把焦点送到不可见对象。组件草稿、稳定ID选择和有效单元格编辑仍属于原组件，不因重排重新mount。

## 无窗口边界与验收

Windows完整运行库的`UI_RUN_OFFSCREEN`保存/恢复同一布局和逻辑停靠、浮动、折叠状态，resize与逻辑拖拽不创建HWND。浮动是逻辑矩形，没有实际显示器、窗口预览或原生捕获；原生分隔条手势返回UNSUPPORTED，调用方直接提交逻辑尺寸。OSMesa继续绑定内存，隐藏WGL路径保持原窗口依赖声明。

`ui_framework_headless`是核心状态测试适配器，不包含这个Windows布局管理器；不能用其UNSUPPORTED返回冒充Windows无窗口功能的完成证据。

[workspace_layout测试](../tests/workspace_layout.c)覆盖格式、原子验证、缺失/新增ID、512记录、部分ABI字段、重复拖拽、分隔条取消、程序DPI与屏幕夹紧。[独立应用集成](../tests/api6_integration.c)在轻量离屏及真实WebView2中检查草稿、焦点、选择、GL、双实例、模态和卸载。最终结果与物理条件边界见[API6验收](validation/api6-validation.md)。
