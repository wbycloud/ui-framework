# API9 Windows x64 当前开发快照

SDK0.9.0-dev/API9/标准9，应用ABI1/包格式1。此SDK是本地开发交付，未发布稳定版本。精确源码/文档commit在identity.json，include、导入库、共享DLL、宿主、完整示例和测试匹配该快照；不能只凭API数字混用另一份开发DLL。逐文件清单sha256.json不包含自身；归档哈希与解包验证另附外部证明。

## 完整应用与应用拥有的中英文

```powershell
$env:UI_API7_OSMESA_DLL=(Resolve-Path ./bin/osmesa.dll).Path
$env:UI_API7_BACKEND='light' # webview2需要已安装的真实Runtime
$env:UI_STATE_EXAMPLE_HOME="$env:LOCALAPPDATA/UiFrameworkStateExample"
$env:UI_STATE_EXAMPLE_PROFILE='A'
./bin/framework_host.exe ./bin/stateful_components.uapp
```

Language/语言菜单提交并保存本实例语言。create先读取稳定A/B档案UIL1，在首次呈现前提交；无有效偏好由示例明确选择英文，非法文件拒绝且不覆盖。第二实例可用B档案保持不同语言；框架不写语言文件，不提供全局覆盖开关。标签切换时宿主跟随活动host，无实例时查询Windows显示语言。State菜单保存/恢复UST1（UCW1列宽+ULYT布局），状态文件与语言文件独立。不重载DLL、内容HWND或GL，业务值/草稿/用户列宽不因翻译变化。接口/通知/线程/复制/关闭合同见source/docs/instance-language.md和source/examples/stateful_components/README.md。

## 固定依赖与离线源码构建

Lexbor、QuickJS固定源码和WebView2 SDK1.0.4129.50在source/.deps；OSMesa24.3.4在bin，Mesa源码归档和许可证另附。WebView2 Runtime是外部系统依赖，SDK不等于Runtime；本轮实际版本以外部验收报告为准。有窗口GL继续使用系统驱动，本SDK不部署替换opengl32.dll。

```powershell
# MSVC x64开发终端，CMake/Ninja和Windows SDK为外部构建环境
cmake -S ./source -B ./build -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_FRAMEWORK_ENABLE_WEBVIEW2=ON "-DUI_OSMESA_LIBRARY=$((Resolve-Path ./bin/osmesa.dll).Path)"
cmake --build ./build
ctest --test-dir ./build --output-on-failure
```

公共共享调用链接lib/ui_framework_runtime.lib并载入bin/ui_framework.dll。examples/sdk-consumer.c验证当前C ABI、语言/查询/通知/文本和列宽保留；以/std:c11 /W4 /WX /utf-8 /MD /DUI_FRAMEWORK_BUILD_SHARED、/Iinclude和匹配导入库编译，EXE放bin运行。

解包后实际语言套件：

```powershell
pwsh -NoProfile -File ./source/tests/run-instance-language.ps1 -BuildDirectory ./bin -Provider ./bin/osmesa.dll -Backend light
# Backend改webview2验证真实Runtime
```

真实宿主/完整DLL截图和本轮解包结果由归档外部证明提供，包内历史验收不能冒充本轮通过。原托管超时尚未根治，当前源码服务Session0及整机无登录、其他真实Windows显示语言、IME/物理跨屏/桌面合成/长期人工仍须分别验收。不修改系统显示语言或注销用户来测试。
