# 公共通用助手目录示例

`catalog.c`是公共生产接口调用方，打开一个实际应用包，创建workspace及新Service，打印资源、能力Schema和Profile目录，再安全关闭。程序不配置模型或提交任务。完整接入顺序与匹配SDK要求见指南。[接入指南](../../docs/xuanshu-general-assistant-integration.md)、[源码](catalog.c)

## 构建与参数

在仓库根、已准备固定Light依赖的x64开发环境使用以下命令。目标由CMake在ui_framework_shared和web_counter_package存在时注册。

```powershell
cmake -S . -B build/catalog -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_BUILD_TESTS=OFF
cmake --build build/catalog --target ui_general_catalog_example web_counter_package
New-Item -ItemType Directory -Path build/catalog/example-session
./build/catalog/ui_general_catalog_example.exe ./build/catalog/web_counter.uapp ./build/catalog/example-session/conversation.sqlite
```

两参数依次是包路径和全新数据库路径。父目录预先存在，数据库已存在时程序拒绝。示例使用offscreen workspace并选择编辑许可上限作为目录运行环境，未配置价格/计量，实际流程只查询目录。上述命令在本轮仅经目标、参数和源码静态核对。

## 阅读重点

初始化先`ui_framework_initialize`，再创建workspace并打开包。`ui_ai_assistant_enable`发生在Service open之前，UI线程轮询至READY。`ui_ai_assistant_catalog`返回复制JSON，使用`ui_ai_release`。关闭先request_close并poll至Service允许destroy，再关闭workspace及应用。

APP注册资源、能力和专业Profile的最小业务实现可继续阅读计数器。实际事务和文档解析分别见维护组件与browser。`tests/general_host.c`的可信事件及模型替身仅用于隔离验证。[计数器](../web_counter/README.md)、[维护组件](../stateful_components/README.md)、[浏览器](../../apps/browser/README.md)、[隔离业务流程](../../tests/general_host.c)
