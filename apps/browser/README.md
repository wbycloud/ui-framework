# browser.uapp开发与助手接入

当前仓库内应用位于apps/browser，使用实际受限文档解析、状态模型及统一read链。应用版本0.3.0承接M0—M2阅读功能，框架接入使用当前SDK 0.10开发版、API10、ABI1和包1。普通浏览操作、助手读取及原副作用分别遵循各自来源和范围。[应用接入指南](../../docs/xuanshu-general-assistant-integration.md)、[浏览器阅读合同](docs/browser.md)

## 资源、字段和专业角色

| 资源与能力 | 输入/输出 | 当前合同 |
|---|---|---|
| browser.view / browser.snapshot | 输入{}，顶层state、revision、find_text | state含title、address、加载/布局/错误等完整状态，选择state即选择整个对象 |
| browser.view / browser.body | 输入{}，顶层body、revision | body含text/bytes/complete/semantics，DOM规范化正文最多8192 UTF-8字节，语义dom-body-v1，排除脚本和表单控件，超限或不可用失败 |
| browser.reader | browser.snapshot、browser.body | 专业角色是复制的说明和允许列表，与根任务及目标取交集 |

源码在browser_capabilities.inc注册资源、真实SHA-256 revision、两项v1 read、C1发布/退役及专业Profile。状态与正文均来自实际当前文档，旧revision或重开epoch保持原固定目标拒绝语义。[能力注册与执行](src/browser_capabilities.inc)、[正文提取](src/browser_body.c)

现有十项view动作沿C2人工UI绑定。导航/back/forward/reload/stop/home/help及settings-save/settings-reset九项副作用保留原人工行为，完整AI副作用迁移另案。状态读取和正文读取均不导航或修改用户来源规则。[原P6分类及边界](../../docs/xuanshu-p5-application-coverage.md#4-2026-10-27-p6补充盘点与brc-1实况)

## 从框架仓库构建

在仓库根、MSVC x64开发环境准备固定Lexbor/QuickJS依赖。下列命令仅列使用入口，本轮静态核对了选项、目标和包位置。

```powershell
cmake -S . -B build/browser-dev -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_BUILD_TESTS=OFF -DBROWSER_BUILD_CHECKS=OFF -DUI_FRAMEWORK_ENABLE_WEBVIEW2=OFF
cmake --build build/browser-dev --target framework_host browser_package uapp_pack
./build/browser-dev/framework_host.exe ./build/browser-dev/browser.uapp
```

生产浏览器不要求OSMesa。单独以apps/browser为源目录时可通过UI_BROWSER_SDK_DIR指定匹配的已解包SDK，需包含bin/ui_framework.dll、bin/uapp_pack.exe、lib/ui_framework_runtime.lib、include及third_party/lexbor的匹配库/头。具体导入关系由CMake定义，历史M2 SDK仅保留其原接口和身份。[构建定义](CMakeLists.txt)、[框架匹配材料](../../docs/build-and-validation.md#23-当前开发材料和代表目标)

同app_id的打开遵循workspace模块复用规则。验证不同包内资源时先使用明确的隔离实例和包身份，已有同ID模块不能被另一包悄悄替换。

## 来源配置与隔离样本

默认用户来源文件为`%LOCALAPPDATA%/ui-framework/browser/origins-v1.txt`。`UI_BROWSER_SETTINGS_PATH`显式选择配置位置，非法/空值/超长显式路径拒绝并保持失败状态，未设置时保留默认行为。环境变量隔离文件位置，资源授权和来源规则仍由原合同约束。[配置实现](src/browser_settings.c)

本地验证使用全新自有目录、空来源规则及包内合成样本。匹配交付br1-production.uapp使用同一生产browser实现和samples/browser-read资源，能够验证包内合成交接单。它与普通browser.uapp及隔离测试链接包分别有文件身份。[合成业务样本](../../samples/browser-read/start.html)、[匹配交付清单](../../docs/xuanshu-general-assistant-delivery-manifest.md)

## 历史阅读能力和证据

M0/M1访问、诊断与M2列表/表格/CSS/图片/查找等阅读能力按原合同和验证记录保留。M3—M5路线图按实际接入推进，当前快照的通用助手结果另有身份。[路线图](docs/development-roadmap.md)、[M0/M1支持](docs/m01-support.md)、[M2支持](docs/m2-support.md)、[M2验证](docs/m2-validation.md)

当前完整助手联调的模型及可信事件为隔离替身。历史真实Provider结果有各自程序、日期和数据范围，新的真实验证包保持Proposed，需另行批准。浏览器本文及命令在本轮只做静态核对。[当前限定复审](../../docs/validation/xuanshu-general-layout-independent-review.md)
