# 通用 Web UI：当前 API8 接入约定

本文描述 **SDK 0.8.0 开发版 / API8 / 标准修订8** 当前组件合同，应用 ABI、导出入口和包格式保持1。菜单及离屏增补见[接口约定](framework-menu-offscreen.md)；当前结果见[API8验收](validation/component-experience-validation.md)，[API5记录](validation/api5-validation.md)保留历史结果，历史证据保留在[API3验收](validation/api3-validation.md)与[API4验收](validation/api4-validation.md)。

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

表格按视口查询，上下各两行、左右各一列缓冲；起始边界减少前置缓冲。缓冲节点隐藏/裁剪，数据量不决定 DOM 数。滚轮按行移动，Shift+滚轮及完整列范围横条移动列。共同树/列表/表格移除上一页/下一页及列箭头按钮，操作栏隐藏并回收高度；表单/对话框提交取消保留。视口重新测量后按需请求可见行列。保留1024节点预算，根据列数限制虚拟窗口，超大视口仍用完整范围滚动条访问预算窗口外数据，不预加载全部数据。显式 query 则使用调用者范围，不自动加缓冲。

树保存展开分支计数和位置元数据，按扁平窗口定位，不预生成后代。展开按需请求、折叠保留状态、F2 重命名；右键命令可调用 show_menu，复用指定 menu_path 的现有注册项显示 Web 节点菜单，API4转发到独立公共Web弹窗，单层最多128项并分页；命令携带项目ID。详见[菜单约定](framework-menu-offscreen.md)。LIST 复用行窗口，可显示图标和单元格。

局部更新保留稳定节点、草稿、焦点和选择，更新图片不重建行。缓存之外的行由应用更新数据源后查询；直接 update/remove 按 ID 操作当前缓存。get_state 返回实际 DOM 数、累计行节点创建数、缓存行/字节、呈现错误，注册成功不代表页面呈现成功。

## 4. 属性、纯 Web 编辑与对话框

字段支持文本、数字、布尔、枚举、颜色、图片、样式和分组，带单位、只读、禁用、混合值、修改标记和错误。枚举使用API6共同下拉；颜色使用API7共同连续RGBA/透明度选择、基础色及六/八位文本回填，交互只改草稿，表单提交才发业务命令。具体提交/取消合同见末尾API6/API7增补。

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
| 按需树、表格/列表、增量更新 | 已实现固定行高/虚拟窗口及完整范围滚动；十万数据测试通过 |
| 属性、草稿、校验、实例模态、Web 编辑 | 已实现上述子集；输入传输与剪贴板测试通过 |
| RGBA/包 PNG/样式、异步缩略图、缓存/过期结果 | 已实现；WIC、合成、资源压力及宿主测试通过 |
| API7双实例、关闭重开、原生/OpenGL | 当前样例与回归；历史API/原包不再列入兼容维护门槛 |
| 实际中文 IME、物理跨显示器、长时间手动滚动压力 | 未验证，需相应操作和硬件条件 |
| WebView2 的 C 图片 ID/框架组件呈现 | API5可选实际Runtime后端；异步呈现/捕获合同见本文增补 |
| 枚举下拉、颜色回填、表格键盘/排序/多选 | API6共同模板；排序和范围由完整数据源提供 |
| 布局持久化、约束分隔条、左右/底部栈、同区标签、浮动 | API6/7 Windows workspace；[范围与格式](workspace-layout.md) |
| 完整范围横纵滚动条、连续RGBA/透明度 | API7共同模板；轻量overflow条由框架绘制，无分页按钮，保持原预算 |
| 完整浏览器、Canvas/SVG、富文本、可变行高、任意嵌套分割树 | 未实现 |

轻量后端报告 IMAGES/WEB_TEXT_EDIT/COMPONENTS；API5 WebView2也提供图片和编辑桥接，显式framework_components=1才声明COMPONENTS。NULL组件后端默认仍为轻量；可借用WebView2后端，详见本页API5增补。

## API5 WebView2 与共同组件

启用可选 WebView2 后端及实际 Runtime 后，新增 PRESENTATION_QUERY、OFFSCREEN_CAPTURE、IMAGES、WEB_TEXT_EDIT 和 ASYNC_RENDER 能力。此处捕获是窗口承载的 Runtime 图像，不意味着浏览器无 HWND 或 Session0。旧配置尺寸默认不提供 native_handle/COMPONENTS；显式 `framework_components=1` 创建框架承载容器，并声明 COMPONENTS/NATIVE_WINDOW。`ui_component_desc_t.web_backend` 借用这个后端，NULL 沿用轻量后端；应用保留它直到 host 销毁后再在 destroy 回调中用匹配的 destroy 函数释放。

两后端使用同一组件 HTML、C 数据源、64位十进制 ID、语义命令、草稿、模态和实例输入门控。DOM 输入通过 Runtime 执行脚本排队，OK 表示接受，不表示 C 回调完成；应用继续正常 Win32 消息循环。原生 Runtime 输入仍由浏览器处理，程序输入覆盖命中、编辑、选择、撤销和模板键盘行为，真实 IME 需另验。C 图片 ID 通过实例限定的私有虚拟资源 URL 编码 PNG；更新刷新版本，释放返回缺失资源，不跨实例借用。

WebView2 呈现查询和捕获使用正状态 `UI_STATUS_PENDING`，调用方在正常消息循环推进后重试同一请求。框架保存结果，不保存调用方输出缓冲地址；完成一次消费后释放内部像素。导航、resize、数据/图片/输入改变使旧代次失效，关闭后不调用应用回调。查询输出 view 本地逻辑 rect/clip、可见/启用/焦点/溢出及复制文本；input/textarea/select 用值，其他元素用文本。找不到元素的完成结果是 NOT_FOUND。

NULL pixels 同步查询捕获物理宽高/stride。提供像素缓冲后，等待模板投递、图片 decode、字体及两个动画帧，再异步 CapturePreview/WIC 解码为顶向下 RGBA8、alpha255；物理尺寸按逻辑尺寸×DPI/96四舍五入。应用拥有最终缓冲，capacity/stride 需满足公共预算。若 CapturePreview 返回尺寸不符，明确失败而非填充假图。flush 只检查本后端已排队工作，未完成返回 PENDING，不嵌套泵系统消息或冒充业务完成；轻量 flush 的历史 job-budget/CANCELLED 合同保留。

组件承载使用 Runtime 的 ControllerOptions4 AllowHostInputProcessing，让聚焦浏览器时的菜单和宿主/应用快捷键进入正常消息循环。[最低Runtime为138.0.3351.48](https://learn.microsoft.com/en-us/microsoft-edge/webview2/release-notes/sdk/1-0-3351-48)，接口仍按实际QueryInterface结果核验。程序键输入在页面未 preventDefault 时走同一语义命令，并保留编辑快捷键。没有该 Runtime 接口时组件承载创建明确失败。公共菜单仍使用框架轻量后端，部署完整宿主需要同时启用轻量与可选 WebView2。

销毁先撤销事件及应用回调，SDK 未完成操作只持有失效的内部 view；在 SDK 回调返回后的 UI 消息中停止导航、关闭controller并释放环境。controller成功创建后、任何Close之前订阅 BrowserProcessExited，环境保留到对应浏览器退出事件后释放。创建过程中取消时，API6仅接受所启动内部空白导航ID的NavigationCompleted，忽略初始about:blank完成，只释放一次事件所有权，并等待该完成文档的renderer脚本确认后再清理；不装载应用文档或调用已失效的应用回调。此前只执行初始文档脚本或接受任意完成事件，在实际CI仍出现关闭句柄增长，失败和修复见API6验收。应用 DLL 可按既有卸载合同释放；UI线程保持STA和正常消息循环，共享框架 DLL 应继续处理 Runtime 的关闭消息。同一用户数据目录的其他 view 仍活动时，浏览器退出和内部环境清理会延后；不要以 destroy 返回或 flush 空闲推断所有浏览器进程已退出。不会清除调用方的用户数据目录。

API5 DLL/Runtime、双实例、图片更新释放、编辑、树数据、模态、异步失效及关闭的历史证据见 [API5验收](validation/api5-validation.md)。API6历史布局、共同组件及综合回归状态见[API6验收](validation/api6-validation.md)，当前见[API8验收](validation/component-experience-validation.md)，不继承历史记录中未覆盖的新功能结论。

## API6 组件交互、完整排序与选择

轻量和显式WebView2组件共享模板及C消息处理。枚举字段展开真实选项列表，每页至多8项，保持原128选项限制；上下/Enter/Esc用于选择/取消，readonly/disabled不编辑。颜色字段提供六种基础RGBA色和任意#RRGGBB/#RRGGBBAA文本输入，选择同步填回文本与color_rgba；非法颜色提交显示字段错误，不发语义提交。颜色预览只使用合法CSS值，避免轻量后端在校验前抛脚本异常。

TABLE在配置sort_command或selection_flags后单击选择并聚焦单元格，方向键/Home/End/Tab导航，跨页/列按原虚拟化窗口重新请求，Enter/F2编辑、Enter只提交一次、Esc取消。语义edit带稳定行ID、列和文本；应用更新真实模型并用update_rows/source回填，框架不把编辑命令接受当作业务保存成功。旧描述保持原点击编辑。布局变化保留仍有效的草稿、焦点和节点，换源/排序换代取消旧编辑和异步身份。

desc.sort_command是复制的已注册命令ID。ui_component_set_sort(table,column,direction)的direction为0/1/-1（无/升/降），保存状态、命令一次、generation换代、清页/缩略图/预览后查询。命令params含column、direction和字符串generation；同状态调用不重复命令，禁用/busy或模态拒绝。source每次query收到借用sort_column和sort_direction，必须按完整数据集排序后的顺序提供数据；默认顺序由source定义。框架不对缓存页作假完整排序，列头按升→降→无循环。sort_column由描述列限定，get_state返回借用字符串。

selection_flags=UI_SELECTION_MULTIPLE启用Ctrl切换、Shift范围，选择身份始终是uint64非零稳定ID，JSON用十进制字符串，不能用缓存下标冒充身份。set/get_selection至多512个，拒绝重复/零ID，内存计入原2MiB缓存；程序调用不发select命令。页切换/排序保留有效ID，换source清空，remove_rows剔除删除项；排序或结构更新重置范围锚点，首次新手势重新建立锚点。两个实例独立存储选择。

范围查询ui_component_select_range(first,count,extend)中的first/count是当前完整数据源排序中的位置，框架发UI_QUERY_SELECTION，请source返回这些位置的稳定ID；无需加载范围内所有行。同步submit_selection或后台post_component_selection都核对generation/request及精确count，下一范围/排序/换源/结构变化/关闭使迟到结果失效。source需要保留请求数据时复制全部完整字段，sort_column只在回调期间借用。超过512范围或合并后超过512明确失败，不提高预算。

用户select语义命令表示手势意图，params保留id/index及ctrl/shift，命令只发一次；异步范围尚未返回时get_selection仍是上次完成的结果，不能在命令回调中把它当作完整新范围。应用在投递消费之后查询选择状态；如果业务需要立即获得范围，source可同步返回ID。程序排序会发sort_command，程序选择不会发select，这是两个接口的明确合同。

选择数据复制投递复用每实例8MiB预算，无应用回调指针。缓存2MiB、DOM1024、单批512、列64、字段64、图片32MiB、JS8MiB和旧队列/过期结果门槛保持。[实际100000行、>2^53 ID及两后端测试](../tests/component_experience.c)与[真实DLL集成](../tests/api6_integration.c)见[验收记录](validation/api6-validation.md)。

## API7 完整数据范围滚动和颜色

共同TREE/TABLE/LIST纵条基于source的total_count（TREE加入展开分支计数）；宽表横条基于最多64列的全部宽度，浏览器自身overflow不替代虚拟数据条。缓存仍是窗口，纵向BigInt映射uint64，横向按列边界停靠，不预加载全数据。轨道12、最小滑块24逻辑像素，无溢出隐藏，点击轨道一视口、交汇留白，滚轮/Shift滚轮/键盘和稳定选择合同保留；数据批次继续按需查询，分页及列箭头按钮不再显示。

数据总量缩小时，source可以为原请求返回空批（row_count=0），报告新的total_count，即使请求first已超出新总量；非空批仍要求first及数量在总量内。框架夹紧位置并按需重查有效窗口，不用旧缓存推算总量。树可折叠已知但当前不在缓存中的展开分支，重新计算完整扁平范围；祖先已移除的分支不计入范围。

轻量overflow:auto/scroll在GDI层绘制两轴条，无系统滚动控件；窗口和NULL-HWND输入均支持。公共UI_INPUT_CANCEL、Esc/捕获丢失恢复手势起点；失活/模态/resize/代次变化解除捕获，保留最新有效状态。数据更换和完整源排序按原代次处理迟到结果。

COLOR增加RGBA滑轨和暗/白底透明度预览，保留#RRGGBB/#RRGGBBAA校验。拖动/键盘只写草稿，保留颜色不等于业务提交，提交复用commands.submit。撤销/选择器Esc恢复打开前值，捕获取消恢复拖动前值；只读/禁用不修改。Runtime有窗口、轻量支持无HWND，各自实际验收。

Runtime创建排入STA并在开始时检查dispatch_blocked；WAIT拒绝后有效操作可恢复创建。已开始的操作继续特定内部文档确认、回调返回后Close和BrowserProcessExited释放环境。失败、资源类型诊断及重复复验见[稳定性记录](validation/runtime-stability-validation.md)，最终结果见[API7验收](validation/api7-validation.md)。

Runtime模态在controller尚未创建时可能先获得容器焦点。文档就绪后仅当该容器仍持有焦点、实例活动且输入门控允许时转交给Runtime；失活、关闭、模态变更或已转移焦点不会由异步就绪强行取回。原输入命令/所有权及PENDING合同保持。

## 框架自有视觉与状态

共同模板默认浅色；独立宿主同步全部实例的浅深主题，应用自有HTML/原生/GL不自动注入。紧凑32行高、12像素完整范围滚动条、表单/颜色/枚举、错误/加载/图片失败与焦点状态见[视觉规范](visual-design.md)。此前0.7视觉轮只改内部模板与轻量绘制，当时公共API7、BigInt总量、缓存/节点/图片预算、排序/选择/草稿及输入合同保持；[两后端真实运行与截图](validation/visual-ui-validation.md)不替代物理IME或业务验收。

当前维护范围见[版本政策](version-policy.md)，此前分页清理轮真实两后端回归及截图见[分页清理验收](validation/component-scroll-only-validation.md)。应用继续返回真实total_count与first/count，数据源语义不变；本轮SDK8需要匹配重建，旧包不纳入维护。菜单、最近应用及标签溢出的分页不在共同数据组件清理范围。


原生共同组件滚动修复（2026-10-07）：避免同一HWND重复SetCapture触发同步取消；TREE加载/空状态改为行视口内覆盖层，异步批次不再改变轨道尺寸。公共API7/ABI1、输入坐标、按需查询和资源预算保持。[真实鼠标与业务复验](validation/native-component-scroll-validation.md)。

## 当前共同模板的视觉几何

空title不占行；非空标题仍保留。字段至少220逻辑像素宽、短标签且非多行/图片/组时采用80像素标签与值同行，窄窗及选择器展开时堆叠；单位16像素。数据单元格/树行32、表头28、缩略图24、勾选24。Runtime表头与数据单元格都约束至声明列宽，轻量使用同一规则；滚动仍按真实total/完整列宽、BigInt与按需批次。

IMAGE字段按C资源尺寸与可用区域保持比例；STYLE预览有界。尺寸元数据是框架私有模板内容，不新增公共结构或转移图片所有权。纯只读且无业务提交/取消命令的表单隐藏无效操作栏，显式命令/可编辑字段及模态对话框保留原提交取消。现有ID、草稿、焦点和选择更新合同不变，无分页按钮。[实际两后端完整窗口与原生输入复验](validation/visual-polish-validation.md)。

## API8 体验合同

公开列宽及UCW1、完整help字段见[迁移](migration-v0.7-to-v0.8.md)。窄勾选/色块/图片列使用紧凑内距，表头与内容同宽；F1/悬停查看完整内容。事件中的可见锚点和组件代次避免Runtime异步几何查询丢失提示。长枚举在128项预算内滚动并支持首末项键盘访问，RGBA取消恢复草稿，业务提交仍一次。现有空/加载/错误/图片状态、BigInt滚动和原预算继续维护；不恢复分页条。

属性名称是展示标签，可单击查看完整名称，不增加Tab停靠点；原输入Ctrl+F1查看名称、F1查看值，选择器按钮F1查看属性名，选项F1查看完整选项。悬停提示不获得窗口焦点，关闭时不恢复历史焦点；菜单和模态仍按各自焦点合同恢复。

完整文本查看保留原表单Tab顺序：名称单击或文本右键打开；原输入F1查看值，Ctrl+F1查看完整名称，枚举/颜色按钮F1查看名称，选项F1查看完整选项。组件内只读文本可滚动/复制，Esc关闭并恢复控件焦点；Enter不提交底层表单。悬停提示为不可交互窗口，不参与键盘菜单路由；其关闭不抢夺后续点击焦点。
