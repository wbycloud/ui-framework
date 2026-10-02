# 从 v0.1.0 迁移到 0.2.0 开发版本

本文面向 Windows C/C++ 应用开发者，说明从稳定 SDK `v0.1.0`、开发标准修订 1 升级到当前 `0.2.0` 开发源码、修订 2 的变化。当前尚未发布 `v0.2.0` 标签；使用开发源码时记录确切 commit，不使用不存在的稳定标签作为构建基准。完整约定见[应用开发标准](application-development-standard.md)，版本记录见 [CHANGELOG](../CHANGELOG.md)。

## 1. 先判断应用是否需要改造

独立宿主改为 Web 外壳；应用内容仍可使用原生控件、OpenGL 或 Web。旧包的兼容目标和需要迁移的依赖按下表区分：

| 应用现状 | 升级方式 |
| --- | --- |
| API 1/ABI 1 包，通过框架注册菜单/工具栏/面板，在约定内容容器挂载，遵守线程与卸载约定 | 保存原 `.uapp`，直接在新宿主验证，兼容目标无需重新编译或改包 |
| 希望使用 shell/content-slot、增量刷新或统一 JSON 消息 | 使用当前 SDK 构建 API 2 应用，同步清单和 DLL 的版本声明 |
| 直接 `SetMenu`、修改借用 HMENU、枚举 Common Controls toolbar，或依赖宿主外壳窗口类 | 必须改为公共 UI 注册/命令接口；不要依赖独立宿主的 Win32 外壳内部结构 |
| 应用自行创建顶层窗口并使用原生嵌入式框架 | 原生 API 保留；native-only 构建同时关闭独立宿主和轻量 Web |

独立 `framework_host.exe` 只提供 Web 外壳，没有切回原生外壳的运行参数。原生嵌入式是另一种接入方式，仍由应用拥有顶层窗口与消息循环。先检查应用对外壳的依赖，再决定是否需要重建。

## 2. 版本和二进制边界

| 项目 | v0.1.0 | 当前 0.2.0 开发源码 |
| --- | --- | --- |
| SDK / 开发标准修订 | 0.1.0 / 1 | 0.2.0 / 2 |
| 公共头文件 API 宏 | 1 | 2 |
| 运行库接受的框架 API | 1 | 1、2 |
| 应用 ABI | 1 | 1 |
| DLL 导出入口 | `ui_app_query_v1` | `ui_app_query_v1` |
| `ui_app_context_t` 与生命周期函数表 | ABI 1 | 保留 ABI 1 布局 |
| `.uapp` 格式与清单八字段 | 1 | 保留 1 |

清单 `framework_api_version` 必须与 DLL 的 `ui_app_descriptor_t.framework_api_version` 一致。原 API 1 包保持 1；使用当前头文件的 `UI_FRAMEWORK_API_VERSION` 构建新 DLL 时值为 2，应把清单同步为 2。不要只编辑旧包的清单“升级版本”，也不要把新 DLL 改写成 API 1 来绕过旧宿主检查。

API 2 包在 API 1 宿主上被拒绝。新运行库通过 `ui_framework_supports_api()` 接受 1/2，拒绝不支持的版本；这不是未来版本范围协商，也没有自动迁移工具。包格式仍严格拒绝未知字段，应用的 SDK/commit/标准修订记录保存在应用项目文档中。

## 3. 构建配置发生了什么变化

Windows 默认 `UI_BUILD_STANDALONE_HOST=ON`，`UI_FRAMEWORK_ENABLE_LIGHT_WEB` 随宿主默认开启。默认宿主需要固定 Lexbor/QuickJS-NG，Git clone 不包含 `.deps/`，CMake 不自动下载依赖。先按[依赖准备说明](build-and-validation.md#3-启用两种-web-后端)准备，再构建：

```powershell
cmake -S . -B build/web-shell -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/web-shell
ctest --test-dir build/web-shell --output-on-failure
```

只关闭轻量 Web、保持宿主开启会报配置错误。保留不含 Web 引擎的原生嵌入式构建时，同时关闭两个开关，WebView2 也明确设为 OFF：

```powershell
cmake -S . -B build/native -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_BUILD_STANDALONE_HOST=OFF -DUI_FRAMEWORK_ENABLE_LIGHT_WEB=OFF -DUI_FRAMEWORK_ENABLE_WEBVIEW2=OFF
cmake --build build/native
ctest --test-dir build/native --output-on-failure
& .\build\native\minimal_eda.exe
```

这条路径不生成独立宿主。WebView2 默认 OFF，仅作为可选应用内容后端；开启时另行准备 SDK/Runtime，独立宿主外壳仍由轻量后端绘制。默认 Web 共享运行库使用 `/MD`，分发时检查动态 CRT 与第三方许可，详见[打包与分发](build-and-validation.md#21-打包自己的应用)。

## 4. UI、内容槽和刷新迁移

### 4.1 菜单和工具入口

继续用 `ui_host_register_command()` 注册业务行为，再通过 `ui_host_register_menu_item()`、toolbar/item 描述引用命令 ID。Web 宿主从活动实例的注册项生成 UI，应用无需重写已注册的菜单内容。

`ui_native_shell_menu_handle()` 在 Web 兼容 shell 中保留借用 HMENU 镜像，不安装到宿主窗口。直接安装、修改它，或查找原生 toolbar 的代码必须改为公共注册和命令调用。应用自定义原生 child 和自有快捷键仍可保留，但必须遵守回调 scope 与线程约定。

### 4.2 内容槽和坐标

新接口在 [`shell.h`](../include/ui_framework/shell.h)。mount 时调用 `ui_host_get_shell(context->host)`，随后 `ui_shell_get_content_slot(shell, NULL)` 获取主区，传已注册 panel ID 获取面板槽。返回 shell/slot 都是借用对象；slot 到 shell 销毁才失效，应用不释放它们或销毁容器 HWND。

原生内容可在 `ui_content_slot_native_handle()` 返回的容器中创建自己的 child；原生/OpenGL surface 使用 `ui_content_slot_attach_surface()`，Web view 使用 `ui_content_slot_attach_web_view()`。内容对象必须属于槽的 host，一个槽绑定一种内容，同一对象不能同时绑定到另一槽。绑定不转移所有权，unmount 仍由应用销毁内容；Web view 先于其 backend 销毁。传 NULL 可解除对应内容绑定。

主 Web 内容使用实例 host 的 parent 和 host 坐标；面板 Web backend 使用槽 HWND 为 parent，view 使用局部坐标。槽的逻辑/像素矩形相对实例 host，浮动槽以 `(0,0)` 表示浮窗内容尺寸。`ui_host_get_rect()` 继续返回实例容器内的逻辑坐标，宿主的标签和全局助手空间已经被扣除。OpenGL viewport 使用 surface 像素尺寸。

旧 `ui_native_shell_panel_handle()` 保留真实面板容器。标签切换、停靠/浮动保留容器、原生 child 和 GL context；不要在 `set_active` 中重新 mount 或销毁模型。自有 child 的内部布局仍由应用响应容器尺寸变化。

### 4.3 选择正确的刷新操作

| 操作 | 内容有效性与调用方责任 |
| --- | --- |
| resize/DPI/reflow | 调整位置、尺寸和可见性，保留内容；响应新的 framebuffer/容器尺寸 |
| 新 `ui_shell_refresh()` | 更新注册项、增加新面板，保留已有内容容器和绑定；刷新后可查询新槽 |
| 旧 `ui_native_shell_refresh()` | 重建面板，旧 HWND 及其 child 失效；刷新前释放内容，刷新后查询新 handle 并重建 |

需要增量更新的新应用采用 `ui_shell_refresh()`。旧二进制调用原 refresh 时继续执行原有重建约定，不能因为宿主换成 Web 就假定旧句柄永远有效。

## 5. Web 内容和 JSON 桥接

API 2 在 [`ui.h`](../include/ui_framework/ui.h) 追加消息、能力和 native handle 回调；旧自定义 backend ops 尺寸继续接受，未实现的新操作返回 UNSUPPORTED。能力位是类别提示，不保证完整 DOM/CSS。

页面用 `ui.postMessage(data)` 发送 JSON，应用用 `ui_web_view_set_message_callback()` 接收；C 侧用 `ui_web_view_post_json()` 发送数据，页面通过 `ui.onmessage` 或 window message 接收。消息按 JSON 数据解析，不拼接成可执行脚本；借用参数需要异步保留时复制。回调在 UI 线程执行，在返回前不得销毁当前 view/backend/host。

完整可构建参考是 [`examples/web_counter/app.c`](../examples/web_counter/app.c)及其 [API 2 清单](../examples/web_counter/manifest.ini)。它在 C 中保存业务计数与助手快照，页面按钮发送 increment 消息，C 命令更新状态并回推 count；不创建 OpenGL context。可与 API 1 EDA 混合运行。

轻量后端实现受控动态 DOM、基础 flex/absolute、class/ID/后代选择器、有限媒体规则、按钮和文本输入。完整子集与上限见[后端说明](../examples/light_web_probe/README.md)，不能按完整浏览器或通用前端框架编写页面。TypeScript 在构建阶段离线编译成 JavaScript，框架不执行 `.ts`。WebView2 的浏览器能力由 Runtime 决定，也需要检查其异步加载和接口差异。

助手仍通过 `ui_workspace_invoke()` 固定实例后进入该实例的 assistant 允许列表。Web UI 消息和可信 `ui.invoke()` 不替代助手的参数校验、权限、确认、合作取消、事务或卸载保护；模型输出继续走语义命令路径。

## 6. 验证升级

1. 保存原 v0.1.0 `.uapp` 及其 hash、SDK/标准记录，使用同一个未重建包打开新宿主。不要先重打包再声称原二进制兼容。
2. 同时打开原包的两个实例和 API 2 Web Counter，分别修改状态，验证标签切换、停靠/浮动、增量刷新、助手目标与请求 ID 隔离。
3. 检查加载失败与错误报告：清单/DLL API 不一致、未知 API、错误 ABI/架构，关闭中的实例拒绝新调用。
4. 验证 UI 尺寸/DPI、原生输入、Web JSON/转义与 OpenGL framebuffer；在实际不同 DPI 显示器和目标 GPU 上完成硬件验证。
5. 验证拒绝/等待关闭、迟到消息丢弃、最后实例卸载和重新加载，确认自有 callback scope 与 worker 停止顺序。
6. 更新应用适配记录，注明目标 commit、标准修订 2、应用 API/ABI、实际测试结果及仍待验证的硬件条件。

仓库的 [`tests/sdk_v1`](../tests/sdk_v1/README.md) 头文件快照用于编译真实 API 1 调用方，不能代替原包未重建的证明。若已将原 EDA 包保存在 `build/sdk-v1/minimal_eda.uapp`，可显式指定它运行兼容目标：

```powershell
$legacyPackage = (Resolve-Path .\build\sdk-v1\minimal_eda.uapp).Path
cmake -S . -B build/web-shell -G Ninja -DCMAKE_BUILD_TYPE=Release "-DUI_LEGACY_EDA_PACKAGE=$legacyPackage"
cmake --build build/web-shell
ctest --test-dir build/web-shell --output-on-failure -R ui_application_versions
```

`UI_LEGACY_EDA_PACKAGE` 默认指向当前构建中使用冻结 SDK 1 头文件生成的 EDA 包。要验证其他原应用，还需在宿主里按应用自己的验收项运行；EDA 专用测试不自动覆盖任意 `.uapp`。

## 7. 本次验收状态

本轮已完成干净构建、原 EDA 二进制包混合 API 1/2 运行和实际 Web 外壳视觉验证。新增 6 项集成测试通过；完整可选 CTest 为 21 通过、2 失败、1 跳过，保留的旧版本/子集断言失败和单屏限制见[当前迁移验收记录](build-and-validation.md#51-020-开发版本迁移验收)。这不证明任意第三方包都兼容，也不表示已经发布稳定版本。
