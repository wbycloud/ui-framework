# API8 Windows x64 本地开发 SDK

SDK0.8.0-dev / API8 / 开发标准8 / 应用ABI1 / 包格式1。未发布，未推送。
产品提交 f0b161c，示例和测试提交 d24cdf3；完整源码与文档快照见 identity.json。
include、lib、bin 来自同一当前构建；sha256.json 记录每个文件的身份，不包含自身。
原生捕获及异步轨道修复保留。只维护当前版本，不承诺历史 SDK 或旧包兼容。

## 实际完整应用

在本目录运行 PowerShell：

```powershell
$env:UI_API7_OSMESA_DLL=(Resolve-Path ./bin/osmesa.dll).Path
$env:UI_API7_BACKEND='light' # webview2 需要已安装的真实 Runtime
$env:UI_STATE_EXAMPLE_HOME="$env:LOCALAPPDATA/UiFrameworkStateExample"
$env:UI_STATE_EXAMPLE_PROFILE='A'
./bin/framework_host.exe ./bin/stateful_components.uapp
```

State 菜单提供显式保存、加载、单列重置、全表及布局重置。
应用拥有 A/B 稳定档案和独占锁，UST1 包含已有 UCW1 与 ULYT；不写应用包或业务文档。
关闭不自动覆盖文件。完整时机、异常与实例合同见 source/examples/stateful_components/README.md。
示例实际生成 OSMesa MSAA frame 并读回，不是模拟图片。

## 源码与依赖

固定 Lexbor、QuickJS 源码及 WebView2 SDK 位于 source/.deps，OSMesa24.3.4 x64 位于 bin。
MSVC x64 C11 开发终端可离线构建：

```powershell
cmake -S ./source -B ./build -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_FRAMEWORK_ENABLE_WEBVIEW2=ON "-DUI_OSMESA_LIBRARY=$((Resolve-Path ./bin/osmesa.dll).Path)"
cmake --build ./build
```

MSVC19.50、Windows SDK10.0.26100、CMake/Ninja 是构建环境，不随包安装。
WebView2 SDK1.0.4129.50 的 Loader 随包；真实 Runtime 必须另行安装，本轮实际验证154.0.4258.62。
Windows 有窗口 GL 使用本机 OpenGL；本包不部署软件 opengl32.dll 到宿主或 Runtime。
许可证位于 licenses。静态库使用 lib 内匹配的 Lexbor/QuickJS；共享调用使用 ui_framework_runtime.lib 和 bin/ui_framework.dll。

## 校验与消费

外部交付记录保存 ZIP SHA256、CRC、逐文件校验、解包后的实际 C 调用和真实鼠标保存／重启恢复。
examples/sdk-consumer.c 可在 x64 开发终端以 /std:c11 /utf-8 /MD /DUI_FRAMEWORK_BUILD_SHARED、/Iinclude 和 lib/ui_framework_runtime.lib 编译，输出放 bin 后运行。
源码验证记录在 source/docs/validation/api8-stability-state-validation.md；归档后的证明单独记录，避免归档哈希自引用。
连续重绘历史来源未定位，业务原套件29/30的固定坐标失败保留。
当前源码 Session0、严格整机无登录 runner、托管CI、真实IME、物理跨屏／桌面边缘合成、长期人工及正式业务修改仍待环境或授权。
程序 DPI、自动截图和独立框架观察不能替代这些验收。
