# API3 通用组件纯 C 示例

应用只注册 C 描述、提供数据、绑定已有语义命令，不提供通用组件 HTML。它是测试数据示例，框架没有加入应用专用文档模型。

## 构建与启动

准备固定 Lexbor/QuickJS-NG 后，在 x64 Visual Studio Developer PowerShell 执行：

```powershell
cmake -S . -B build/web-shell -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/web-shell
ctest --test-dir build/web-shell --output-on-failure
& .\build\web-shell\framework_host.exe .\build\web-shell\generic_components.uapp .\build\web-shell\generic_components.uapp
```

生成应用 DLL、含 icon.png 的包和独立宿主。宿主/DLL 在同目录，应用清单 API3/ABI1/包格式1。默认允许多实例。详细依赖与限制见[构建说明](../../docs/build-and-validation.md)。

## 内容与操作

- 注册菜单/工具图标、Ctrl+D 对话框快捷键和状态组件。
- 左侧按需树：100个父节点，每个1000子节点；展开/折叠、选择、F2重命名、右键菜单命令。
- 主区100000行×16列：布尔复选框、可编辑文本、RGBA缩略图、样式和颜色单元格。滚轮移动行，Shift+滚轮或按钮移动列，Enter提交、Esc取消。
- 右侧属性：文本/数字/单位、布尔、枚举、混合值、多行输入；提交前校验必填/数字，应用额外要求尺寸大于零。
- Web参数对话框只阻塞当前实例，切换标签隐藏并保留；多行Enter换行/Ctrl+Enter提交。
- 浮动绘图槽使用应用 OpenGL 三角形，仅验证内容槽共存。示例使用显式旧式 WGL；框架没有强制应用选择 OpenGL。
- “刷新图片”增加内容版本并重新查询，旧异步缩略图不会覆盖新版本。

编辑记录只保留64条，每条文本最多255字节；这些是示例数据模型限制，不是组件公共限制。包图片通过WIC读取，后台线程生成 RGBA，调用公共复制投递后立即释放缓冲。

## 生命周期和接入位置

[app.c](app.c) 的 create 注册命令/布局/组件，mount 获取已有内容槽并启动图片线程，source 提交临时行数组。run 处理所有语义入口，snapshot 返回实例状态及实际 DOM/缓存指标。

关闭返回 WAIT，停止线程后投递关闭完成；unmount 仍 join 线程后释放 GPU 内容，防止模块卸载时线程尚在 DLL 代码中。框架统一释放组件/图片，destroy 释放应用私有队列和句柄。错误路径与正常卸载使用同一路径。

测试为 [generic_web_host.c](../../tests/generic_web_host.c)；完整[合同](../../docs/generic-web-ui.md)与[实际验收](../../docs/validation/api3-validation.md)区分自动消息、真实页面及未验证的实际中文 IME/跨屏条件。
