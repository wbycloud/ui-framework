# 通用 Web UI：API 3/4 接入约定

本文的 API3 组件合同在 **SDK 0.4.0 开发版 / API4 / 标准修订4** 继续适用，应用 ABI、导出入口和包格式保持1。API4菜单与离屏增补见[接口约定](framework-menu-offscreen.md)；历史及当前测试、未实测条件分别见[API3验收](validation/api3-validation.md)与[API4验收](validation/api4-validation.md)。

## 1. 架构、复用与边界

| 已有机制 | API 3 的使用方式 |
| --- | --- |
| host、workspace、生命周期 | 每实例拥有组件和图片；沿用关闭、投递和卸载流程 |
| 语义命令、assistant | 菜单、工具、快捷键、组件绑定同一命令 |
| 菜单和工具栏注册 | 追加图片、快捷键、状态与增量刷新 |
| 面板、停靠/浮动、内容槽 | 挂载组件；HWND 只作平台承载 |
| Web view 和 JSON | 框架模板只传当前窗口的数据，像素走 C 接口 |
| OpenGL surface | 继续挂入内容槽，应用负责绘图/GPU资源 |

补充的组件数据源、图片资源、共用模板和文本编辑器没有建立第二套业务命令或生命周期。新组件和宿主业务界面不创建 TreeView、ListView、Button、EDIT、MessageBox 或隐藏编辑代理；旧应用自有原生内容、原生嵌入式及系统文件/目录选择器保留。

公共定义见 [components.h](../include/ui_framework/components.h)、[images.h](../include/ui_framework/images.h)。共用页面在 [components.html](../src/components.html)，状态和数据在 [components.c](../src/components.c)。应用不编写通用组件 HTML，不引用后端私有类型。

## 2. 注册、挂载与命令

在 create 注册命令、工具和面板，再注册组件；mount 获取借用 shell/content slot 并挂载。完整可构建调用顺序见[通用纯 C 示例](../examples/generic_components/app.c)。核心接口：

| 入口 | 用途 |
| --- | --- |
| register / mount / unregister | 复制描述、挂载内容槽、注销并使请求失效 |
| query / submit / post_component_batch | 按需请求、同步或后台复制交付 |
| update_rows / insert_rows / remove_rows | 按稳定项目 ID 局部更新 |
| expand / select / get_state | 展开、选择及实际呈现/缓存统计 |
| set_field / get_field / set_error / accept_fields | 回填、借用草稿、校验错误、接受提交 |
| show_dialog / close_dialog / set_text | 实例模态及状态文本 |

组件 ID 最多 63 UTF-8 字节，列/字段 ID 最多 32 字节；项目 ID 非零且在组件内唯一，不能随滚动改变。内容版本由应用递增。默认固定行高 32 逻辑像素，最小 24。

select/edit/rename/context_menu/submit/cancel 绑定已有语义命令。编辑 payload 含 action、字符串 id、column、text；重命名含 id/text；表单提交为 `{"fields":{"字段ID":"值"}}`，字段值用文本表达，布尔为 true/false 文本。选择、展开、焦点和草稿由框架保存；回填不重复提交。应用仍负责业务校验和事务。

`ui_host_set_command_state()`、`ui_host_set_item_state()` 更新 visible/enabled/checked/busy。命令与界面项均启用时才可操作；助手仍走原有权限和 validator。工具分组用 toolbar ID/order，菜单用 menu_path/order。图片用 image_id 或 set_item_image，旧 icon_url 不自动解析。快捷键用 virtual-key 和公共 modifier 位；文本焦点保留编辑键，OpenGL 槽也分发已注册快捷键。

## 3. 数据请求、虚拟窗口和增量更新

source 在 UI 线程执行。UI_QUERY_ROWS 包含 component_generation、request_id、parent_id、first/count、所需列、DPI。根 parent 为 0。结果必须原样返回请求身份、父 ID、起点，附 total_count 和行数组；同步 submit 或后台 post_component_batch。请求参数只借用到回调返回，后台自行复制。

框架复制描述、字符串和数组。一次最多 512 行、每行 64 单元格、字符串 4095 UTF-8 字节；单页和保留数据缓存限制 2 MiB。换数据源增加代次，旧结果 CANCELLED。

表格按视口查询，上下各两行、左右各一列缓冲；起始边界减少前置缓冲。缓冲节点隐藏/裁剪，数据量不决定 DOM 数。滚轮按行移动，Shift+滚轮及列按钮移动列，支持分页。保留 1024 节点预算，根据列数限制窗口大小，极大视口继续分页。显式 query 则使用调用者范围，不自动加缓冲。

树保存展开分支计数和位置元数据，按扁平窗口定位，不预生成后代。展开按需请求、折叠保留状态、F2 重命名；右键命令可调用 show_menu，复用指定 menu_path 的现有注册项显示 Web 节点菜单，API4转发到独立公共Web弹窗，单层最多128项并分页；命令携带项目ID。详见[菜单约定](framework-menu-offscreen.md)。LIST 复用行窗口，可显示图标和单元格。

局部更新保留稳定节点、草稿、焦点和选择，更新图片不重建行。缓存之外的行由应用更新数据源后查询；直接 update/remove 按 ID 操作当前缓存。get_state 返回实际 DOM 数、累计行节点创建数、缓存行/字节、呈现错误，注册成功不代表页面呈现成功。

## 4. 属性、纯 Web 编辑与对话框

字段支持文本、数字、布尔、枚举、颜色、图片、样式和分组，带单位、只读、禁用、混合值、修改标记和错误。枚举第一版采用按钮循环选择；颜色可通过文本/数据回填表示，没有专门颜色拾取器。

Uniscribe 处理整形、光标和命中；Web view 自己保存/绘制编辑状态，支持选择/拖选、剪贴板、撤销/重做、单行/多行、只读/禁用。enable_native_input 保留 ABI 布局但废弃且忽略。IMM32 接入组合、候选定位、提交/取消；实际中文 IME 尚未实测。最多 4095 UTF-8 字节，多行按显式换行显示，不提供富文本或自动折行编辑。

失焦不提交。表格 Enter 提交、Esc 取消；表单显式提交/取消，单行 Enter 提交、多行 Enter 换行/Ctrl+Enter 提交。组合期间 Enter/Esc 优先交给输入法。set_field 不覆盖已修改草稿，但更新状态；get_field 借用当前草稿，业务成功后 accept_fields，set_error 定位错误。框架检查必填和有限数字，示例额外验证尺寸大于零。

DIALOG 内容使用同一 Web 模板，独立窗口仅承载。show_dialog 只禁用所属实例，切换标签隐藏并保留，返回恢复；close_dialog 恢复此前焦点。成功提交由应用关闭，校验失败保留。框架危险操作确认沿用已有 Web 路径，阻塞整个宿主并保护回调/模块寿命。

浮动组件的 Win32 鼠标、按键、字符和 IME 输入与公共组件输入遵守相同的实例激活、关闭和模态门控；模态对话框自身仍可编辑，回到停靠后保留原面板内有效的键盘焦点。框架生成的 STYLE 预览只保留当前呈现所需的图片，换页、回填及组件注销后回收，元数据计入图片预算。应用提供的 cell.image_id 或 style.image_id 继续借用，不由预览回收路径释放。

## 5. 图片、PNG、预览和预算

公共 RGBA 为顶向下、8 位通道、非预乘透明度。stride 为正且至少 width×4，bytes 覆盖最后一行；验证溢出后复制，返回后应用可释放输入。内部使用预乘 BGRA 和 AlphaBlend。单图边长最多 4096、像素数据最多 8 MiB。

load_png 使用 Windows WIC C 接口，输入最多 8 MiB，并平衡 COM 初始化/释放。共享模式的 load_resource 通过包接口读取 PNG；静态库没有包 workspace，返回 UNSUPPORTED。其他图片解码格式未实现。像素不经过 JSON/Base64，资源 ID 统一用于菜单、工具、节点和单元格。

显式资源保留到 release。默认每实例图片、资源记录及缩略图索引合计 32 MiB，可再生成缩略图/样式采用 LRU；set_limit/get_stats 配置与查询。显式图片占满时返回 LIMIT_EXCEEDED，不强制淘汰。每组件最多 512 条缩略图身份元数据，超出淘汰旧身份并拒绝迟到结果。

可见范围产生 UI_QUERY_THUMBNAIL，含项目、内容版本、请求、目标像素尺寸、DPI。应用自己的渲染器生成 RGBA，thumbnail 或 post_thumbnail 交付。框架等比居中、裁剪、显示空/加载/失败状态及禁用效果。旧代次、删除项目、旧内容版本或 DPI 不能覆盖新结果。

样式颜色为 0xRRGGBBAA，支持透明度、边框、实线/虚线/点线及重复点阵，不解释业务含义。style.image_id 非零时借用应用图片；create_preview 返回该借用 ID，否则创建新资源，应用按实际所有权释放。

## 6. 所有权、后台投递与卸载

| 对象/数据 | 约定 |
| --- | --- |
| 描述、字段/选项、批次、RGBA | 返回前复制，应用立即可释放输入 |
| component | host 所有，unregister/host 销毁后指针失效 |
| source 参数、命令 params | 回调期间借用，异步保存需复制 |
| get_field/status_text 字符串 | 框架借用，更新或销毁后失效 |
| shell/slot/平台句柄 | 借用，禁止销毁承载对象 |
| 后台 post | 队列复制，消费/丢弃时释放，无应用函数指针 |

UI 对象接口只在 UI 线程调用。后台只通过 workspace 投递复制数据，携带实例、组件、代次、项目/内容版本和请求身份，不传 host、HWND 或回调函数。JSON 中所有 64 位对象/图片/请求 ID 均用十进制字符串。

新数据/图片队列每实例默认 8 MiB，可配置；超限立即返回 LIMIT_EXCEEDED，由应用决定重试或降级。workspace 存活到所有工作线程停止。关闭期间停止新任务，迟到/过期结果丢弃；回调中注销返回 CANCELLED；呈现/数据源回调内只能提交当前请求结果，换源或修改/删除行返回 CANCELLED，应延迟到退出后处理。

unmount 停止并 join 线程、撤销外部回调、释放 GPU 内容；宿主再清理组件/请求/图片，destroy 释放应用状态，最后 module_shutdown，所有调用退出后卸载。部分初始化失败沿同一路径清理。投递关闭完成不能替代工作线程真正离开 DLL 的证明，通用示例采用 WAIT 加 join。

## 7. 能力清单

| 能力 | 当前状态 |
| --- | --- |
| Web 菜单/工具/面板外框/状态/确认、命令状态/图标 | 已实现；真实宿主自动测试通过 |
| 按需树、表格/列表、增量更新 | 已实现固定行高/分页子集；十万数据测试通过 |
| 属性、草稿、校验、实例模态、Web 编辑 | 已实现上述子集；输入传输与剪贴板测试通过 |
| RGBA/包 PNG/样式、异步缩略图、缓存/过期结果 | 已实现；WIC、合成、资源压力及宿主测试通过 |
| API1/2/3、双实例、关闭重开、原生/OpenGL | 所列样例与回归已验证，非任意第三方包保证 |
| 实际中文 IME、物理跨显示器、长时间手动滚动压力 | 未验证，需相应操作和硬件条件 |
| WebView2 的新增 C 图片 ID/框架组件呈现 | 未实现；原 HTML/消息能力保留 |
| 完整浏览器、Canvas/SVG、富文本、可变行高、拖拽停靠、布局持久化 | 未实现 |

轻量后端报告 IMAGES/WEB_TEXT_EDIT/COMPONENTS；WebView2 不报告这些新增位。组件 mount 固定使用轻量后端，没有 WebView2 组件呈现入口；未编入轻量后端时 mount 返回 UNSUPPORTED。WebView2 的浏览器图片/输入不等于本版 C 资源桥接。
