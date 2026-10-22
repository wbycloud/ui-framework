# browser.uapp

C11 受限文档阅读应用，当前0.3.0完成M2阅读兼容性，并保持M0/M1访问与诊断，作为 ui-framework 内置应用交付。每个实例是一个宿主标签；启动宿主不会打开浏览器或发出网页请求。宿主选项菜单的“浏览网页”通过普通 workspace/uapp 生命周期打开包。

本工程实际工作区为 `D:\应用软件框架\uapp浏览器`，接手时为空且没有 Git 仓库。框架实际基准 HEAD 为 `689fa03b34f8c4321a6c838f52ca9dcd790a9319`，分支 `codex/p1-process-boundaries`，包含接手前修改。本轮不创建提交；身份以完整源码 SHA 清单与交付文件 SHA 确认。

在本机运行：

```powershell
& .\tools\build.ps1 -Targets browser_package,framework_host
& .\build\release\framework\framework_host.exe
```

干净的框架构建不需要测试、Mesa 或 WebView2：

```powershell
cmake -S D:/应用软件框架/应用层序框架 -B build/production -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_BUILD_TESTS=OFF -DBROWSER_BUILD_CHECKS=OFF -DUI_FRAMEWORK_ENABLE_WEBVIEW2=OFF -DUI_OSMESA_LIBRARY=
cmake --build build/production --target framework_host browser_package uapp_pack
```

从 deliverables/m2-20261021/browser-sdk-m2.zip 解压匹配SDK（下例位于 D:/sdk/browser-sdk-m2），应用支持仅使用该 SDK 构建，无需框架源码或 QuickJS 源码：

```powershell
cmake -S . -B build/sdk-only -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_BROWSER_SDK_DIR=D:/sdk/browser-sdk-m2 -DBROWSER_BUILD_CHECKS=OFF
cmake --build build/sdk-only --target browser_package
```

需要 MSVC x64 Developer PowerShell。固定依赖不自动下载或升级；SDK 中的 Lexbor 静态库、头文件和许可证来自同一固定版本。包内 `origins.txt` 提供默认精确origin；可信工具栏“设”可保存用户规则，无需重新编译或重打包。用户文件为 `%LOCALAPPDATA%\ui-framework\browser\origins-v1.txt`，非法配置拒绝全部网络，点击“默认”可恢复。可信“诊”面板区分访问阶段、重定向及子资源失败，隐藏凭据、路径和查询。

详细范围见 [支持表与公共合同](docs/browser.md)，实际结果与交付身份见 [验证记录](docs/validation.md)。`tests/document_public.c`、`tests/workspace_public.c`、`tests/failure_public.c` 仅使用公共框架 C 接口；宿主入口夹具单独负责宿主内部观察，不替代它们。

产品目标及M0—M5见[整体路线图](docs/development-roadmap.md)。本批固定样本及源码维护规则见[M0支持矩阵](docs/m01-support.md)，M0/M1构建、真实结果、新包和已知问题见[本批验收](docs/m01-validation.md)。首版docs/validation.md及旧deliverables/INDEX.md保留历史身份；该段保留M0/M1历史入口，当前M2交付见下文；M3—M5后续。

M2新增：start/reversed/li value编号、合并表格与caption、仅可见文本查找复制、基础CSS/外部样式顺序、JPEG EXIF八方向及GET提交语义。具体可测试范围见[本批矩阵](docs/m2-support.md)，实际命令、失败及新交付见[本批验收](docs/m2-validation.md)。M0/M1记录保留历史，M3—M5后续。
