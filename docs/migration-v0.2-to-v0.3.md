# 从 SDK 0.2 开发版迁移到 0.3 开发版

目标为 SDK 0.3.0、API3、标准修订3；没有新稳定标签。应用 ABI、`ui_app_query_v1` 和包格式保持1。运行库接受 API1/2/3，不能把 API3 包交给只接受1/2的旧运行库。

## 1. 已有应用是否需要修改

| 情形 | 要求 |
| --- | --- |
| 符合生命周期的 API1/2 二进制包 | 保留原声明，先在新宿主验收；无需仅为 API3 改包 |
| 旧应用自有原生内容、原生嵌入式 | 保留兼容路径，不承诺任意窗口内部依赖 |
| 应用想使用框架树/表格/表单/图片 | 使用新头文件/import library，并同步 DLL 与清单 API 为3 |
| 依赖轻量后端 EDIT 子窗口或 enable_native_input | 必须移除这种依赖；开关现在忽略，不再创建 EDIT |
| 自定义 Web backend | 旧 ops size 继续接受；新增可选钩子缺失时没有对应能力 |
| WebView2 内容页 | 原 HTML/消息能力保留，新增 C 图片 ID/通用组件尚不支持 |

API版本是应用要求的框架合同，SDK commit 是构建/说明基准，两者分别记录。清单只有原八个字段，不添加 SDK commit 或标准修订字段。

## 2. 接入步骤

1. 在应用仓库保存所用 SDK 标签/commit、标准修订、API/ABI，并保留升级前 `.uapp`。
2. 阅读[标准修订3](application-development-standard.md)和[通用 Web UI](generic-web-ui.md)，选需要迁移的面板。业务模型、算法、GPU renderer 留在应用。
3. 仍用原命令 ID、权限、validator、事务和生命周期。create 注册公共组件描述和命令，mount 将组件挂入已有内容槽；不再给每个业务控件建 HWND。
4. 数据源按范围查询，提交时返回代次/请求/父节点/位置；传入数组、字符串在返回后可释放。项目 ID 保持稳定，内容变化递增版本。
5. 图标/缩略图通过 RGBA 或包 PNG 创建资源，JSON 只传十进制字符串 ID。后台只投递复制数据，处理 CANCELLED/LIMIT_EXCEEDED，不在工作线程操作组件或 HWND。
6. 成功命令后回填数据/接受草稿；失败用字段错误或 Web 对话框显示。unmount 停止并 join 工作线程，确保不调用已卸载模块。
7. 采用新接口时设置 `framework_api_version=3`，DLL descriptor 的 framework API 同为3。应用 ABI/包格式仍为1；应用自己的 version 按业务发布规则更新。
8. 验收多实例、过期请求、编辑草稿、两实例模态、32 MiB 图片预算/8 MiB 新队列、关闭重开，以及实际 IME和DPI。

## 3. 可构建参考与检查

[通用示例](../examples/generic_components/README.md)只使用公共框架头文件、Windows线程和应用绘图接口；组件页面由框架维护。[Web Counter](../examples/web_counter/app.c)继续作为冻结 SDK2 调用方，[EDA](../examples/minimal_eda/app.c)保持 SDK1 调用方。

新接口结构清零并设置 size，不改变 packing；旧枚举值及结构前缀保留。所有跨模块输入均复制或明确借用，禁止用应用 CRT 释放框架指针。`get_field()` 返回借用草稿，需要长期保存时自行复制。

当前支持固定行高、按需/分页窗口、普通文本编辑和基础样式预览；没有富文本、完整浏览器、可变行高、布局持久化或 WebView2 新组件后端。实际中文 IME和物理跨屏仍未验证，不能将自动消息测试当作实操验收。完整结果见[验收记录](validation/api3-validation.md)。
