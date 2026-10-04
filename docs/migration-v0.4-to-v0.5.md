# 0.4 → 0.5 迁移

SDK0.5.0开发版、API5、标准修订5。应用ABI1、`ui_app_query_v1`、包格式1及旧枚举不变，运行库接受API1/2/3/4/5；没有新稳定标签。先保存旧包与SDK commit，再取得 [main](https://github.com/wbycloud/ui-framework/tree/main) 并记录实际 HEAD。

## 1. 兼容与版本声明

旧 API1/2/3/4 包保留原声明，可直接验证新运行库；不能覆盖原包后冒充二进制兼容。仅采用新函数/字段的应用才以匹配头文件与 import library 重建，DLL descriptor 与严格包清单同时声明 API5。SDK4被冻结为独立测试调用方，保留完整结构前缀和旧 padding；所有描述清零、设置 size，新字段不从旧内存读取。

API4 对照样例 `framework_features.uapp` 继续用冻结 SDK4 构建。新实际 Runtime/DLL 联合入口是 `ui_api5_fixture` 与 `ui_api5_integration_test`，不修改请求方应用或其 PERF-001。

## 2. 菜单

在 menu group/item 填 `access_key`（字母/数字或0）。裸 Alt、助记键、方向键、Enter/Esc、重复键循环与单次命令路由见[菜单合同](framework-menu-offscreen.md#6-api5-菜单无窗口-gl-与-msaa-合同)。现有命令ID、状态、menu_path、图像、锚点及shortcut不变。原生宿主在 TranslateMessage 前先调用菜单入口，并在组合输入时跳过；框架宿主已处理。

## 3. GL 与 MSAA

旧隐藏WGL入口继续报告 HIDDEN_WINDOW。需要真正无窗口时显式用 `ui_opengl_windowless_surface_create`、绝对 OSMesa DLL 路径和至少3.3 compatibility请求。报告 NO_WINDOW；所有 GL 调用都从当前 surface 取得函数指针，保留应用 frame/input 与业务渲染器。软件 GL 是真实 context 输出，不是 GPU 承诺。

离屏 `samples` 是精确采样数，驱动不满足返回失败；用 get_info 查询实际值。RGBA缓冲归调用方，NULL pixels 只查尺寸。新路径的32MiB预算计入附件与临时读回；旧隐藏WGL单采样保留API4的附件32MiB、临时读回另计合同，2048×2048仍可创建。具体计费见[菜单/离屏合同](framework-menu-offscreen.md#6-api5-菜单无窗口-gl-与-msaa-合同)。窗口依赖、Session0和无登录CI分别验收；部署固定可用提供方和许可，OSMesa进程级缓存的边界见接口与验收记录。

## 4. WebView2

准备固定 SDK 和实际 Runtime，配置 `framework_components=1`，再把后端借给 `component_desc.web_backend`；host销毁后才释放后端。NULL 仍用轻量组件。图片ID、数据源、语义命令、模态、实例门控与模板相同。

呈现和捕获新增正状态 PENDING。UI线程在整个消息循环期间保持STA；外层正常泵消息并重试，不能在Runtime回调中嵌套泵消息，不能把接受输入或postJSON的OK当成结果已经完成。捕获顶向下RGBA8，alpha255，物理尺寸由逻辑尺寸与DPI确定，调用方缓冲不会被异步保留。导航/resize/输入/数据更新使旧结果失效。后台实例和同实例模态外输入取消。销毁后异步清理仍要求UI线程及框架DLL存活，应用DLL可按原合同卸载。接口与所有权见[通用Web API5](generic-web-ui.md#api5-webview2-与共同组件)。

## 5. 验收

按 [构建说明](build-and-validation.md) 与[API5验收](validation/api5-validation.md) 重跑当前源码。原 SDK 调用方、原包、实际 GL、实际 Runtime/DLL 各自提供证据；缺失物理 IME、跨屏、人工长时压力、Session0及CI环境时分别记录待验。当前尚未验收或失败的项目不得对应用宣称完成。
