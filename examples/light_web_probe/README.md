# 受控轻量 Web 后端与验证程序

该后端以 C11 实现，静态链接 Lexbor 和 QuickJS-NG，通过公共 `ui_web_backend_t` 接口接入 host。默认框架构建不启用它；启用后不需要浏览器运行时。它用于程序随附的固定 UI 页面，当前 Windows 绘制使用 GDI，输入框使用 Unicode Win32 EDIT。

## 构建和运行

在配置了 MSVC、Windows SDK 和 CMake 的开发者终端中运行：

首次克隆后，先按[依赖准备说明](../../docs/build-and-validation.md#3-启用两种-web-后端)获取固定版本的 Lexbor 和 QuickJS-NG；依赖源码不包含在本仓库中。

```powershell
cmake -S examples/light_web_probe -B build/light_web_probe -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/light_web_probe
ctest --test-dir build/light_web_probe --output-on-failure
```

主项目也可以通过 `-DUI_FRAMEWORK_ENABLE_LIGHT_WEB=ON` 启用后端。自建 CMake 项目可在创建框架 target 后 include `cmake/light_web.cmake`，调用 `ui_enable_light_web(ui_framework)`。依赖源码默认来自 `.deps/lexbor` 和 `.deps/quickjs`，可以通过 `UI_LEXBOR_SOURCE_DIR`、`UI_QUICKJS_SOURCE_DIR` 指定源码目录。

probe 读取 `examples/web_common/assistant.html`，该页面也用于 WebView2 行为对比；随后重新加载其他页面验证 UTF-8、JSON 转义、滚动、运行限制和错误返回。通过时输出 `Light Web probe: 0 failure(s)`。

## HTML 与 CSS 子集

Lexbor 解析完整 HTML 文档，外层 `html`、`head`、`body` 作为文档外壳。body 采用无边距 viewport，外壳属性不会参与布局。body 中最多有一个可视根元素；根元素占满调用 `ui_web_view_resize` 指定的逻辑尺寸。根节点以下接受以下元素：

| 元素 | 行为 |
| --- | --- |
| `div`、`aside`、`section`、`main`、`p`、`span` | 矩形容器；没有子元素时绘制单行文本 |
| `button` | 单行文字按钮，必须没有子元素；按下和抬起命中同一按钮时执行 `onclick` |
| `input` | 单行文本；仅默认类型或 `type="text"`；支持初始 `value` 和 `oninput` |
| `script` | 同步内联 JavaScript；拒绝外部 `src`，没有模块加载器 |
| `style` | 仅下述助手断点规则 |

可视元素接受 `id`、`style`、`onclick`、`oninput`、`value`、`type` 属性。其他属性返回 `UI_STATUS_UNSUPPORTED`，重复非空 ID 返回 `UI_STATUS_ALREADY_EXISTS`。直接放在容器中的混合文字与子元素不参与文本绘制；用叶子 `span` 或 `p` 表达文字。

内联 `style` 支持以下固定语法：

| 属性 | 接受的值 |
| --- | --- |
| `display` | `flex`、`block`、`none` |
| `flex-direction` | `row`、`column`，默认 column |
| `flex` | 0–1000 的整数权重；正权重平分固定子项之外的剩余主轴空间 |
| `width`、`height` | 0–32767 的整数或整数 `px`；0–100 的整数百分比 |
| `background`、`background-color` | 六位十六进制颜色，如 `#303840` |
| `overflow-y` | `auto`、`scroll`；column 容器支持垂直滚动 |

非根元素的默认主轴尺寸为 column 32 逻辑像素、row 120 逻辑像素；默认交叉轴尺寸填满父容器。固定尺寸不会自动缩小，超出父区域的内容被裁剪。此算法是固定 UI 布局约定，未实现 CSS 的内容测量、换行、flex-shrink、padding、margin、边框或通用级联规则。

`style` 元素最多接受一个断点；允许空格和末尾分号，但选择器固定为 `#assistant`：

```html
<style>
@media (max-width: 600px) { #assistant { display: none; } }
</style>
```

窗口逻辑宽度不大于断点时，助手节点及后代隐藏。`ui_web_view_get_element_rect` 返回逻辑矩形；隐藏元素返回零矩形，找不到的 ID 返回 `UI_STATUS_NOT_FOUND`。重新加载 HTML 会重置断点和 JavaScript 全局状态。`ui_light_web_config_t.narrow_width` 为保留字段，不提供隐含 CSS 规则。

## JavaScript 与语义命令

`ui.invoke(commandId, params)` 把请求交给 `ui_host_invoke`，source 为 `light-web`，返回请求 ID。`params` 可以是 JSON 文本字符串，也可以是对象；对象使用 QuickJS `JSON.stringify` 转为 UTF-8 JSON。返回值表示请求被提交，业务结果仍由 host 的 result callback 接收。

```js
ui.invoke("eda.add_block", {kind: "resistor"});
```

`ui.value(inputId)` 返回输入框当前的 UTF-8 字符串。执行 `oninput` 或 `onclick` 时提供临时全局 `event.target.id` 和 `event.target.value`；未提供完整浏览器 DOM、`document`、事件冒泡或 `this` 元素绑定。下面是共同验证页面使用的输入处理：

```html
<input id="name" value="EDA"
       oninput='ui.invoke("probe.input",JSON.stringify({value:ui.value("name")}))'>
```

只执行同步脚本；没有定时器、网络、ES 模块或 Promise job 调度。调用 `ui.invoke` 后，由应用注册的命令实现参数验证、权限检查、撤销和结果。注册命令的回调执行期间，不得销毁正在执行 JavaScript 的 view 或 backend。

## 输入、DPI 和限制

公共输入 API 使用逻辑坐标，支持主按钮 pointer down/up、pointer move、wheel、文本输入，以及 `VK_BACK` 删除最后一个 UTF-8 字符。文本输入追加到最近按下的 input。Win32 EDIT 允许用户原生编辑，读取和绘制均使用 W 接口。wheel 命中节点沿父链找到滚动容器，每 120 wheel delta 移动 48 逻辑像素；没有滚动容器时返回 `UI_STATUS_UNSUPPORTED`。未实现的键、按钮或输入类型也返回该状态。

布局、命中测试和元素查询始终使用逻辑像素。绘制、字体、子窗口和编辑控件按当前 DPI 缩放；96、144、192 DPI 分别对应 1、1.5、2 倍像素尺寸。调用方通过公共 `ui_web_view_resize` 更新尺寸和 DPI。

`ui_web_view_set_rect(view, &rect, dpi)` 可将 view 放在 host 客户区内的任意逻辑矩形，`ui_web_view_get_rect` 查询该矩形。像素宽高通过缩放矩形的两条边再求差计算，避免非整数 DPI 下相邻区域产生空隙。后续 `ui_web_view_resize` 保留已经设置的 x/y。元素矩形和输入坐标仍以 view 左上角为原点，不添加 host 中的偏移。

`ui_web_view_set_layout_region(view, UI_LAYOUT_REGION_RIGHT_SIDEBAR)` 可以直接绑定到 host 侧栏，绑定 MAIN 也使用相同接口。host 尺寸、布局或 DPI 变化会自动重新安排绑定 view；应用的语义命令仍发送给创建 view 时的原 host。probe 验证非零位置、144 DPI 的矩形边缘舍入，以及 MAIN/RIGHT 绑定后的重排与命令。

| 限制 | 当前上限 |
| --- | --- |
| HTML | 256 KiB，合法 UTF-8 |
| 可视节点 | 192 |
| 单行文本、input value | 1023 UTF-8 字节 |
| ID | 95 UTF-8 字节 |
| 内联属性、style 规则 | 2047 字节 |
| 单个 script | 64 KiB |
| QuickJS runtime | 8 MiB 内存、256 KiB 栈 |
| 单次脚本执行 | 50 ms 中断预算；由解释器安全点检查 |
| viewport | 宽高 0–32767、x/y -32767–32767 逻辑像素，DPI 1–768 |

超过内容上限、非法 UTF-8、脚本异常或脚本预算中断返回 `UI_STATUS_VALIDATION_FAILED`；不支持的标签、属性或 CSS 返回 `UI_STATUS_UNSUPPORTED`。载入开始后发生错误时，view 清空并可重新载入；超过整体 HTML 上限或非法 UTF-8 在替换前返回错误。引擎没有承诺支持任意网站，也没有面向不可信远程代码的浏览器安全边界。
