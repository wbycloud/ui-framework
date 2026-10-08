# API8 完整应用的列宽与布局存储示例

`stateful_components.uapp` 是框架维护的独立集成应用，包含十万行宽表、异步缩略图树、属性编辑、颜色／枚举、模态和实际 OSMesa MSAA 输出。它复用 [完整场景](../../tests/api7_fixture.c) 的数据和生命周期；文件名中的 API7 是原用例名称，当前头文件、DLL 和清单声明均为 API8。结果只算框架集成证据。

## 构建与运行

在仓库根目录的 x64 开发命令行中，按 [构建说明](../../docs/build-and-validation.md)准备固定依赖和真实 Runtime，然后执行：

```powershell
cmake -S . -B build/current -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_FRAMEWORK_ENABLE_WEBVIEW2=ON -DUI_OSMESA_LIBRARY=C:/providers/osmesa.dll
cmake --build build/current --target stateful_components_package framework_host
$env:UI_API7_OSMESA_DLL='C:/providers/osmesa.dll'
$env:UI_API7_BACKEND='light' # 或 webview2；需真实 Runtime
$env:UI_STATE_EXAMPLE_PROFILE='A'
./build/current/framework_host.exe ./build/current/stateful_components.uapp
```

提供方使用绝对路径；实际 GL 函数由 OSMesa surface 解析，图片是应用实际 frame 的 RGBA 读回。示例不部署软件 WGL DLL 到 Runtime 目录。当前构建保持 `UI_BUILD_TESTS=ON`，因为示例复用了完整测试场景。

## 身份、存储与操作

默认目录是 `%LOCALAPPDATA%\UiFrameworkStateExample`；可用 `UI_STATE_EXAMPLE_HOME` 指定已可访问的独立目录，推荐绝对路径。应用只创建该目录、`profile-A/B.lock`、`profile-A/B.ust` 和保存时的同目录临时文件，不写应用包、业务文档或框架配置。

组件 `table`、列 `name/value/c2…c63`、面板 `tree/form/viewport` 是稳定 ID。档案 A／B 跨进程保留，不使用本次 `instance_id` 作为文件身份。明确指定档案便于用户／测试跨重启选择；未指定时使用第一个未占用档案。两实例分别占 A、B，锁由 `CreateFile` 的零共享模式持有到 destroy。两个档案都被占用、或明确指定已占用档案时，打开失败并走原初始化清理。该示例只提供两个档案，不改变框架实例预算；实际应用自行决定用户和档案规则。

菜单 **State** 复用现有语义命令：

| 操作 | 行为 |
| --- | --- |
| Save interface state | 同时保存当前 UCW1 列宽与工作区布局；先写临时文件、刷新、同目录替换 |
| Reload interface state | 从同一档案读取；不清空业务文档、属性草稿或选择 |
| Reset Name width | 只恢复 name 的注册宽度 200，不自动保存 |
| Reset widths and layout | 重置全部列宽及布局，不自动保存；需要再点 Save 持久化 |

注册宽度为 name 200、value 120、其余 180 **逻辑像素**。恢复前没有有效状态时保持这些宽度，不自动按内容适配。表头拖动、右键／F10 适配与单列／全表宽度工具仍由共同组件提供，24–4096 逻辑像素合同不变。

加载发生在全部注册和 mount、GL／worker 初始化成功之后。保存仅由用户明确的 Save 命令触发，关闭不会悄悄覆盖文件，拒绝的文件也不会被启动过程改写。状态／错误经现有 status 组件呈现。

## 文件格式与异常处理

UST1 是**示例应用自己的容器**，不是新公共框架存储接口。32 字节小端头后依次放不透明 UCW1 和 ULYT 字节；整个文件上限 70000 字节，框架原列宽／布局及 UI 预算不变。

| 偏移 | 内容 |
| --- | --- |
| 0 | `UST1`，4 字节 |
| 4 | 容器版本 uint32，1 |
| 8 / 12 | UCW1／ULYT 字节数 uint32 |
| 16 | 档案 ASCII A 或 B |
| 17–19 / 24–31 | 必须为零的保留字节 |
| 20 | uint32 FNV-1a 校验：初值 2166136261，逐字节异或并乘 16777619，跳过偏移 20–23 |

校验检测意外损坏，不是签名或安全边界。缺失文件使用初始状态；空、截断、非法长度／ID／保留位、校验不符和未来版本明确拒绝。公共 restore 自己验证其完整负载；示例在 UI 线程先保存原列宽与布局，同步恢复两部分，任一失败立即恢复原快照，期间不泵消息。异常用例验证最终两部分都保持原值；平台级回滚错误仍返回错误，不视为成功。

按稳定 ID 恢复：已删除列被忽略、新列保留注册宽度、重排列匹配 ID。`UI_STATE_EXAMPLE_SCHEMA=reorder` 交换前两列；`evolve` 将 c63 替换为 added，仅用于复验定义变化。加载成功前不扫描全量数据，内容适配仍只使用预算内缓存。

布局／列宽操作复用原对象，不重新 mount 或销毁 GL。worker 停止与 WAIT、失败 create/mount、destroy 和模块 shutdown 均委托完整场景；存储锁在 destroy 释放。原资源、线程、所有权和 DLL 卸载合同保留。

## 复验

```powershell
ctest --test-dir build/current -R '^ui_stateful_components_(light|webview2)$' --output-on-failure
```

[串行脚本](../../tests/run-stateful-components.ps1) 每次创建独立证据目录，调用新进程和真实宿主，使用公共 presentation 的可见 grip 中心换算 screen 坐标，实际 SendInput 调宽后保存、关闭、再启动恢复。主槽额外加上 host-relative 槽偏移，不能把 view-local 当成 host 坐标。Light 检查同 HWND 原生捕获；Runtime 使用浏览器 DOM pointer capture，记录 hit 窗口／线程及最终宽度，不能要求其等于框架线程的 GetCapture。

覆盖 96/144/192 程序 DPI、两实例和锁冲突、重排列／删除新增列、单列／全表重置、草稿与实际 GL 再渲染、缺失／空／截断／非法／未来文件、失败 create/mount 和锁回收。脚本保存当前 EXE、DLL、包哈希、原日志和真实 BMP；不会删除已有证据或 Runtime 用户目录。

程序输入与截图不替代真实 IME、物理显示器、桌面合成或长期人工验收。当前结果与剩余条件见 [本轮验收](../../docs/validation/api8-stability-state-validation.md)。
