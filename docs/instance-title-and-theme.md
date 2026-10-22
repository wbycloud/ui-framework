# 实例标题与主题协作（API9 开发增补）

2026-10-17，Codex。面向公共 C 调用方。本文件对应本轮工作树快照；使用同包头文件、导入库、DLL、宿主及应用包。SDK0.9.0-dev/API9/标准9、应用 ABI1、包格式1；既有已发布结构没有插入字段，冻结 SDK 没有回写。相同 API 数字不保证不同开发快照拥有新导出。

## 实例标签标题

`application.h` 的 `ui_workspace_set_instance_title(workspace, instance_id, utf8)` 按稳定实例 ID 设置显示标题。应用从 `ui_app_context_t` 取得 workspace 和 ID，mount 及之后调用；create 阶段实例尚未进入工作区。名称和保存状态标记都由调用方提供，框架不推断业务状态。

```c
/* UI 线程；字符串被复制，调用返回后可释放。 */
ui_status_t status = ui_workspace_set_instance_title(ctx->workspace,
    ctx->instance_id, "材料参数 — 修改中");
/* NULL 恢复包清单名称。 */
status = ui_workspace_set_instance_title(ctx->workspace,
    ctx->instance_id, NULL);
```

非 NULL 标题必须是非空、有效 UTF-8，最多 `UI_WORKSPACE_TITLE_MAX_BYTES`（4095）字节，不包含终止 NUL；超长、空串、非法编码或错误线程返回 INVALID_ARGUMENT。不存在或已销毁的实例 ID 返回 NOT_FOUND；关闭中或标题通知内递归设置返回 CANCELLED。workspace 指针必须仍有效；没有支持悬空指针的承诺。

`ui_workspace_get_instance_title` 复制标题。`required` 包含 NUL；`buffer=NULL, capacity=0` 查询长度；短缓冲返回 LIMIT_EXCEEDED，缓冲原字节不变。包元数据 `ui_app_instance_info_t.name_utf8` 不变。实际名称改变才触发已有 workspace changed 通知；不激活实例，不重新挂载或改变路由、内容 HWND、焦点、草稿和布局。同名标题允许，独立宿主继续附带实例 ID，关闭中的既有标记保持。

独立宿主标签、溢出列表和实例选择器采用该标题。标签保持原有宽度限制；悬停查看完整提示，聚焦标签按 F1 打开可滚动的完整标题弹层，Esc 关闭。标题改变或实例失效关闭旧提示与标题弹层。长标题不通过扩大标签挤掉关闭和溢出入口。

## 每个 host 的主题

公共 `theme.h` 提供 LIGHT、DARK、SYSTEM 三种模式。默认 LIGHT，兼容现有零初始化调用方。host 相互独立；SYSTEM 是显式选择，无持久化存储或全局开关。

```c
#include "ui_framework/theme.h"
static void theme_changed(ui_host_t *host, const ui_theme_info_t *t, void *user) {
    /* 将 0xRRGGBBAA 转成调用方像素，更新自己的同 ID 图标或绘图资源。 */
    (void)host; (void)t; (void)user;
}
/* 在 UI 线程注册；先查询初始值，回调只通知后续变化。 */
ui_theme_info_t t = {0};
t.size = sizeof(t);
ui_host_get_theme(host, &t);
ui_host_set_theme_callback(host, theme_changed, user);
ui_host_set_theme(host, UI_THEME_SYSTEM);
/* unmount 先移除回调，再释放应用自己的图片/画刷等资源。 */
ui_host_set_theme_callback(host, NULL, NULL);
```

`ui_theme_info_t` 是独立的 88 字节结构，读取要求完整 size；原结构未变。信息含请求模式、解析后的 LIGHT/DARK、generation、system_theme_available 及 16 个不透明语义色：

| 用途 | 成员 |
| --- | --- |
| 页面、内容表面 | background_rgba、surface_rgba |
| 正文、辅助文字、边框 | text_rgba、muted_text_rgba、border_rgba |
| 键盘焦点、选择 | focus_rgba、selection_rgba、selection_text_rgba |
| 悬停、按下 | hover_rgba、pressed_rgba |
| 禁用 | disabled_background_rgba、disabled_text_rgba |
| 进行中、图标、强调、错误 | progress_rgba、icon_rgba、accent_rgba、error_rgba |

这些是应用协作的基础色，不是每个组件像素的截图规范：关闭按钮仍保留危险悬停红色，控件子类型仍有既定强调/选择规则。进行中颜色不创建动画或任务。图标可以用 icon_rgba 绘制，也可根据 effective 选择明暗资源；不根据宿主背景猜测。

实际变化顺序：提交模式、解析结果和 generation → 关闭本 host 的旧提示/菜单 → 更新框架外壳与公共组件 → 同步应用回调 → `ui.host.theme_changed` workspace 事件。Runtime 后续布局/绘制仍异步，使用公开 flush/presentation 合同等待。相同模式、解析结果及可用性不重复通知。初始 generation 为1；通知中的数字 generation 以 JSON 字符串传递。回调信息只在调用期间借用；回调可查询主题并更新应用资源，递归 set/refresh 返回 CANCELLED。非 UI 线程、非法模式/参数/短 size 返回 INVALID_ARGUMENT；关闭阻断期间设置返回 CANCELLED，移除回调仍可用。已销毁 host 指针无效。

SYSTEM 使用 Windows 公开 UISettings 的 Foreground 亮度解析明暗；查询不可用时 LIGHT、available=0。查询不修改系统设置，并平衡自身 COM/WinRT 初始化，保留调用方已有 apartment。亮度规则来自 [Microsoft 主题指南](https://learn.microsoft.com/en-us/windows/apps/desktop/modernize/ui/apply-windows-themes)，初始化配对遵循 [RoInitialize 合同](https://learn.microsoft.com/windows/win32/api/roapi/nf-roapi-roinitialize)。这不是系统完整调色板、强调色或高对比度支持。

SYSTEM 变更需拥有者把 WM_SETTINGCHANGE / WM_THEMECHANGED 转发给 `ui_host_handle_message`，或在 UI 线程调用 `ui_host_refresh_theme`。独立宿主转发到各实例；隐藏/嵌入 host 的调用方负责转发或刷新。本增补没有保留跨线程 WinRT ColorValuesChanged 订阅；只发该事件而无窗口通知的环境需要显式 refresh。LIGHT/DARK 的 refresh 无操作。

宿主装饰、框架菜单、公共组件和浮动标题由框架管理。活动实例决定宿主装饰的有效明暗；宿主现有主题菜单显式设置当时打开的全部实例，新 host 仍从 LIGHT 开始。应用自有 HTML/CSS、原生系统控件、GL、图片和自绘表面由应用管理；框架不注入私有脚本替换资源、不重建这些内容。维护示例新增“Use draft as tab title”和“Follow system theme”，并以应用回调更新自己的同 ID 菜单图标；GL 示例画布保留自己的颜色。

## 窄布局与焦点

现有 `ui_component_set_dialog_layout` / get、`ui_component_set_field_layout` 提供客户区约束和多行可视行数，无需新增第二套布局接口。公共 FORM/检查器应保留字段及操作，使用既有外层滚动；长编辑值由内层文字滚动、Home/End、选择和复制访问，多行用 visible_rows 控制合理高度。调用方静态长说明采用 help/多行说明而不是依靠短标签展示全部说明。下拉/菜单仍走既有边界约束、滚动及键盘定位，不能把截断的标签当作完整值。

本轮 Light 将 `.focus()`、`scrollIntoView()` 和 Tab 共用逐层显露逻辑：内层滚动后重算布局，再处理外层；不改用户列宽、数据 generation、数值精度和查询预算。程序 DPI 的 presentation 是 view-local 逻辑坐标，原生 HWND/捕获像素是设备像素；screen 坐标需按公开槽及 ClientToScreen 转换。96/144/192 程序测试不替代物理混合 DPI、多屏、真实 IME或人工体验。

本轮实测、失败、资源和交付身份见[验证记录](validation/interaction-20261017-validation.md)。
