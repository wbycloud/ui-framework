# API4 菜单与离屏纯 C 示例

本示例以编译开关UI_SAMPLE_API4复用generic_components/app.c，通过公共接口展示七个菜单分组、嵌套菜单、三个toolbar、13个新增工具（加原有2项）、紧凑图标、按需树/表格/表单/对话框、后台缩略图和可选OpenGL内容槽。框架不包含示例数据的业务模型。

在准备固定依赖的x64开发终端运行：

```powershell
cmake -S . -B build/web-shell -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/web-shell
& .\build\web-shell\framework_host.exe .\build\web-shell\framework_features.uapp .\build\web-shell\framework_features.uapp
ctest --test-dir build/web-shell -R 'ui_(framework_features_host|menu_offscreen|workspace_offscreen|offscreen_gl)' --output-on-failure
```

正常示例请求3.3 compatibility，不满足驱动要求时明确加载失败。离屏workspace测试在自身进程设置UI_FEATURES_NO_GL，只测无HWND的Web路径；离屏GL另由ui_offscreen_gl_test验证。UI_FEATURES_FAIL_CREATE/FAIL_MOUNT用于部分初始化清理测试，不是框架配置。

ui_workspace_offscreen_test是公共API控制台驱动样例；ui_framework_features_host_test运行真实宿主和同一应用包，输出自己的客户区截图。功能与限制见[接口说明](../../docs/framework-menu-offscreen.md)及[验收](../../docs/validation/api4-validation.md)。这些测试不替代真实中文输入法、物理跨屏或请求方应用的完整业务验收。
