# 受控轻量 Web 后端与验证程序

轻量后端以 C11 实现，静态链接 Lexbor 和 QuickJS-NG，通过公共 `ui_web_backend_t` 接入 host。当前 Windows 独立宿主默认使用它，不需要浏览器 Runtime。布局与绘制由框架的 C/GDI 实现，文本输入可使用 Unicode Win32 EDIT。它面向随应用分发的受控工具页，不提供完整浏览器的 DOM/CSS/Web API。

## 构建和运行

先按[依赖准备说明](../../docs/build-and-validation.md#3-启用两种-web-后端)获取固定版本 Lexbor/QuickJS-NG。依赖不随仓库提交，CMake 不自动下载。在 x64 Visual Studio Developer PowerShell 中构建主项目：

```powershell
cmake -S . -B build/web-shell -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/web-shell
ctest --test-dir build/web-shell --output-on-failure -R 'ui_light_web_(parity|dynamic)'
```

这个独立子项目自动关闭根项目的宿主和轻量后端目标，再调用 `ui_enable_light_web(ui_framework)` 启用 probe 的静态后端。下面命令显式列出这些选项，并关闭可选 WebView2：

```powershell
cmake -S examples/light_web_probe -B build/light_web_probe -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_BUILD_STANDALONE_HOST=OFF -DUI_FRAMEWORK_ENABLE_LIGHT_WEB=OFF -DUI_FRAMEWORK_ENABLE_WEBVIEW2=OFF
cmake --build build/light_web_probe
ctest --test-dir build/light_web_probe --output-on-failure
```

自建 CMake 项目可在创建框架 target 后 include `cmake/light_web.cmake`，调用 `ui_enable_light_web(ui_framework)`。依赖默认位于 `.deps/lexbor`、`.deps/quickjs`，也可通过 `UI_LEXBOR_SOURCE_DIR`、`UI_QUICKJS_SOURCE_DIR` 指定。

[`probe.c`](probe.c) 读取两种后端共用的 [`assistant.html`](../web_common/assistant.html)，验证布局/命令、UTF-8、滚动、资源限制和错误返回。原 probe 保留“class 属性应被拒绝”和“193 节点应被拒绝”两条修订 1 断言；当前实现新增 class 支持并扩展至 1024 内部节点，与这两条断言冲突，当前不能把它报告为 `0 failure(s)`。新增动态行为由 [`tests/light_web_dynamic.c`](../../tests/light_web_dynamic.c) 验证，最终实际结果见[迁移验收记录](../../docs/build-and-validation.md#51-020-开发版本迁移验收)。

## HTML 和内容结构

Lexbor 解析 HTML 文档，body 是参与布局的根节点，body 的 class/style 和样式表可影响它。内部节点是框架维护的受控表示；修改公共脚本对象会增量更新该表示，不暴露 Lexbor 或 QuickJS 私有类型。

| 内容元素 | 范围 |
| --- | --- |
| `div`、`header`、`footer`、`nav`、`aside`、`section`、`main`、`p`、`span` | 矩形容器；叶子节点绘制文字 |
| `button` | 按钮文字、禁用状态和受控事件 |
| `input` | 默认或 `type="text"` 的单行输入 |
| `textarea` | 多行文本输入 |
| `script` | 同步内联 JavaScript，拒绝外部 `src`/模块 |
| `style` | 下述受控 CSS 规则 |

内容属性支持 `id`、`class`、`style`、`onclick`、`onmousedown`、`oninput`、`value`、文本 `type`、`disabled`、`readonly`。未知属性/元素返回 `UI_STATUS_UNSUPPORTED`，重复非空 ID 返回 `UI_STATUS_ALREADY_EXISTS`。容器中的混合文字不是浏览器式排版，使用叶子 `span` 或 `p` 表达文字。资源 URL 不自动加载为图片、脚本或页面。

## CSS 和布局子集

样式表支持标签、class、ID、后代选择器和 `:hover`、`:focus`、`:disabled`，支持逗号分组及有限的规则优先级/行内覆盖。`@media` 接受 width 的 `min-width` 或 `max-width` 条件。它没有完整 CSS cascade、属性选择器、通用组合器、任意媒体条件或变量系统。

| 属性类别 | 支持范围 |
| --- | --- |
| 显示与 flex | `display:block/flex/none`、`flex-direction:row/column`、`flex`/`flex-grow` 的 0–1000 整数权重 |
| 尺寸 | width/height 的整数 px、0–100 整数百分比或 auto；min/max width/height 的整数 px |
| 位置 | `position:static/relative/absolute`；left/top/right/bottom 的非负整数 px |
| 盒模型 | padding/margin 的 1–4 个非负整数 px、单边值、gap、`box-sizing:border-box` |
| 对齐 | 基础 align-items/justify-content/text-align；不提供通用 Flexbox 规范行为 |
| 颜色与边框 | `#RGB`/`#RRGGBB`，background 可 transparent；solid 边框、边框颜色、圆角 |
| 文字 | font-size、font-weight、Segoe UI / Microsoft YaHei UI / sans-serif；有限 white-space 和文本对齐 |
| 裁剪/滚动 | overflow/overflow-x/overflow-y 的 auto/scroll/hidden/visible，受控滚动与裁剪 |
| cursor 声明 | pointer/default/text 语法；不等于完整浏览器光标行为 |

内容测量、文本布局、flex 空间分配和 absolute 定位由固定算法完成；同名 CSS 属性不意味着支持浏览器的所有边界行为。没有 Grid、flex-wrap/shrink、transform/animation、图片、Canvas 或 SVG。对目标页面单独验证尺寸，超出子集的样式应修改或选择 WebView2。

`ui_web_view_get_element_rect()` 返回 view 内的逻辑矩形，隐藏元素返回零矩形，缺失 ID 返回 `UI_STATUS_NOT_FOUND`。参考受控断点：

```html
<style>
@media (max-width: 600px) { #assistant { display: none; } }
</style>
```

`ui_light_web_config_t.narrow_width` 是保留字段，不产生隐含 CSS 规则。

## 动态 DOM、事件和 JSON 消息

脚本支持 `document.body`、`document.getElementById()`、`document.createElement()`。受控元素对象提供 `id/textContent/className/value/disabled/style`、`firstChild/parentNode/children`，以及 `appendChild/removeChild/remove`、受限 `setAttribute/getAttribute/removeAttribute`、`focus()`。style 对象提供子集属性和 `cssText`；没有完整 DOM、querySelector、innerHTML、DOM parser 或一般浏览器文档 API。

元素 `addEventListener()` 接受 click、input、keydown、focus、blur、mouseenter、mouseleave、mousedown；document 监听限 keydown。事件提供受控 target、按键/修饰键和 preventDefault 语义，不能推断完整冒泡/捕获/浏览器键盘导航。支持范围以 [`light_web.c`](../../src/backends/light_web.c) 为准。

`ui.invoke(commandId, params)` 向创建 view 的 host 提交语义命令，source 为 `light-web`，同步返回请求 ID；参数可为 JSON 文本或对象。业务结果仍进入 C result callback。`ui.value(inputId)` 返回当前文本。

API 2 提供双向 JSON 数据消息。以下页面模式来自[可构建 Web Counter](../web_counter/app.c)：

```js
ui.postMessage({action: 'increment'});
ui.onmessage = function (data) {
    document.getElementById('count').textContent = String(data.count);
};
```

C 侧用 `ui_web_view_set_message_callback()` 接收，`ui_web_view_post_json()` 发送。页面也可用 `window.addEventListener('message', callback)` 读取 `event.data`。消息按 JSON 数据解析，不拼接成脚本；C 参数只在本次调用/回调内借用，异步保留时复制。消息 callback、业务 command 和脚本执行期间不得销毁当前 view/backend/host，也不得重载当前文档；将关闭或重载延迟到回调返回后。

增量修改保留未删除节点的输入值、原生 EDIT、焦点和滚动状态。`load_html()` 会重新创建文档和 JS 全局状态。只执行同步脚本，没有网络 fetch、定时器、外部 script/module 或 Promise job 调度。助手和模型输出仍经过应用的语义命令、校验、权限和确认路径。

## 输入、DPI 和能力查询

公共输入 API 使用 view 内的逻辑坐标，支持主按钮命中、move/down/up、滚轮、文本和退格等受控输入；可选 Win32 EDIT 提供单行/多行原生编辑。布局、命中和元素查询使用逻辑像素，GDI/字体/child 位置按当前 DPI 转换。未实现的输入返回 `UI_STATUS_UNSUPPORTED`，不能把接口可调用当成完整浏览器输入协议。

`ui_web_view_set_rect(view, &rect, dpi)` 设置相对 parent 的逻辑位置和尺寸，`get_rect()` 查询；后续 resize 保留 x/y。元素矩形和输入坐标仍相对 view 原点。像素矩形采用两边缩放后求差，避免非整数 DPI 的边缘空隙。`ui_web_view_set_layout_region()` 或[内容槽绑定](../../include/ui_framework/shell.h)使 view 跟随 host 布局/DPI；手动 view 的尺寸由应用维护。

`ui_web_view_get_capabilities()` 提供 JSON_MESSAGES、DYNAMIC_DOM、RESPONSIVE_LAYOUT、TEXT_INPUT；存在呈现 HWND 时还提供 NATIVE_WINDOW。能力位不扩大本页子集。`ui_web_view_native_handle()` 返回借用窗口，应用不得销毁它；`ui_light_web_backend_capabilities()` 的字符串用于诊断。

## 资源上限和错误

| 限制 | 当前上限 |
| --- | --- |
| HTML / JSON 消息文本 | 256 KiB，合法 UTF-8 |
| 内部节点 | 1024，包含 body 根节点 |
| CSS 规则 | 256 |
| 文本 / input 或 textarea value | 4095 UTF-8 字节 |
| ID / class 字符串 | 95 / 511 字节 |
| 内联 style/handler 属性、单条规则声明 | 2047 字节 |
| 单个 script/style 文本 | 64 KiB |
| QuickJS runtime | 8 MiB 内存、256 KiB 栈 |
| 单次同步脚本执行 | 50 ms 中断预算，由解释器安全点检查 |
| viewport | 宽高 0–32767、x/y -32767–32767 逻辑像素，DPI 1–768 |

文本上限按 UTF-8 字节计，不是中文字符数；原生 EDIT 另外设置保守的字符输入上限。50 ms 预算不会抢占应用 C handler，耗时业务仍需异步实现。

超限、非法 UTF-8、脚本异常/中断返回 `UI_STATUS_VALIDATION_FAILED`，不支持的元素/属性/CSS 返回 `UI_STATUS_UNSUPPORTED`。整体 HTML 超限或非法 UTF-8 在替换文档前被拒绝；载入过程开始后失败会清理当前文档，可随后重新载入。引擎面向可信应用 UI，不能把受控脚本资源上限当成不可信网页的浏览器安全边界。
