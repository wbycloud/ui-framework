# M2 阅读兼容性验收（0.3.0，2026-10-21）

本批范围以M2用户提示词为准；旧handoff/M01记录只作历史。每项以实际检查为准；最终机器证据位于deliverables/m2-20261021/verification.json与INDEX.md。

## 实际接手身份

应用无Git，框架HEAD仍689fa03b34f8c4321a6c838f52ca9dcd790a9319，分支codex/p1-process-boundaries，存在既有修改。未发现额外AGENTS.md；使用用户给定约束。SDK0.9.0-dev/API9/标准9、ABI及包1政策保持。固定Lexbor7fb22cf5664a331d7c24b113489e566767c9c25a、QuickJS-NG2f0aa72a6b09cf69ff06399bcf9f4c083ac1a278无改动。309个既有正式文件记录SHA，未覆盖旧包或冻结SDK。

先读取当前README/计划/路线、支持与M01验收及INDEX/身份/验证/诊断/清理证据，框架政策/标准/构建/宿主/标题主题、application/ui/light_web/light_document/app_data/images公共头文件与相应实现。镜像初检无差异，保留目标独有修改的摘要保护不重置。

## 可失败检查及责任

- 上一批SDK上的公共阅读检查先失败14项；扩展CSS样本后失败19项，分别是编号、合并格/caption、隐藏查找、混合声明、disabled/提交者、间距/空白、外部表来源顺序。既有公共文档隔离检查为0失败。
- 编码优先级样本复现meta UTF-16错误（两项结果断言）；修改遵循固定Lexbor转换及HTML meta规则，BOM和HTTP优先级保持。GET单元先失败CRLF及action片段两项。
- 旧WIC实际解码EXIF6为12×8，预期8×12；新实现八方向逐像素与Pillow确定样本一致（最大差0），PNG/BMP/GIF首帧、损坏及预算检查通过。
- 首个应用M2夹具误用仅字符串JSON助手读数字，整轮撤回；改为读取可信快照既知数字字段后，旧0.2包稳定失败9项并真正卸载。新应用一度只剩资源晚到导致片段偏离：content_height2213/scroll_y1491，最终未到视口。保留完整资源后重新定位，再检查通过。
- 临时旧WIC探针首次漏/MD导致CRT链接错误；纠正夹具编译选项后复现成功，没有修改固定库或SDK。

框架改动是受限布局/文本/CSS/GET实现及合同注释；无新增公开结构或导出。应用改动是严格编码分类、WIC有界方向处理、GET编码/片段及资源完成后定位，网络/策略/工具栏继续归应用。C11、不执行远程脚本和桥，旧WebView2源码测试保留。

公开调用方tests/m2_document.c仅使用公共C框架接口，验证布局、查找、捕获、输入与结构化事件；tests/m2_public.c通过workspace实际加载应用。图片/编码单元是应用内部验证，不替代公共调用方。

## 已运行的当前检查

当前开发构建：阅读、公共文档隔离、编码、WIC、设置、M2实际包、M01实际包、初始化/挂载失败及工作区程序均exit0；下载及已观察的WIC解码中关闭，DLL真正卸载。PAGE/next原16次往返private 22,913,024→18,833,408字节，原128MiB/增量32MiB门槛通过，不能替代长期压力。

本轮重新获准后，串行真实点击、选择复制、滚轮、Home/End、Tab GET、Alt前进后退、缩放及明暗/窄窗口96/144/192程序DPI为0失败；失去前台即永久停止的保护未触发。桌面/剪贴板/鼠标恢复并释放。合并表和192DPI原生客户区图像人工检查可读，图像过程件收尾删除。可见M2样本paint_count=9、network=6.413ms、decode=2.435ms、parse=2.321ms、layout=5.646ms、paint=3.534ms；这是单次完成查找后的观测，不是首帧时延或统计性能承诺。

新localhost测试根332734CD9A29171810656595A5D43AC8972FA949临时加入CurrentUser/Root后，同一M2 HTTPS固定集0失败、实际DLL卸载。立即撤除后独立certutil查无条目，同页回到系统12175/TLS标志0x8拒绝。没有绕过证书验证或沿用旧授权；该检查未复现历史旧根导入问题。

生产UI_BUILD_TESTS=OFF、BROWSER_BUILD_CHECKS=OFF、WebView2 OFF、Mesa空配置构建完成。仅本批候选SDK独立构建检查OFF通过，再以同SDK构建十个检查（document/url/settings/encoding/image/reading/failure/workspace/m01/m2）全部exit0；SDK工作区PAGE循环private22,679,552→18,575,360，WIC解码关闭已观察、DLL确实卸载。框架原9项加glyph_padding/menu_offscreen/application_contract共12/12通过；未运行完整框架矩阵及含无保护真实SendInput的旧宿主夹具。

生产宿主无应用启动和显式应用启动分别观察正确模块状态，两个自有进程关闭后exit0；夹具锁保护宿主历史并校验字节不变，应用使用专用临时配置。归档逐文件SHA、公共包接口解包重包、解压应用仅用解压SDK重建与实际运行结果分别记入verification.json，不把隐藏运行的paint0算可见成功。

## 边界及历史问题

具体输入、操作、预期及简化见m2-support.md。网络pipeline指标仍包括转换/收尾，隐藏paint0不计可见反馈。目标站点尚未提供；不承诺任意网站。物理DPI、多屏、真实IME、长期人工、完整框架矩阵、任意恶意网页安全性未验证。M3—M5后续。

M01公网12002、早期OLE恢复崩溃及旧测试证书重复导入不持久化的底层原因维持未定位，原diagnostics.json及SHA保留；本批不无证据归因。本批测试保持物化剪贴板句柄恢复，失去前台永久停止输入，发现外部剪贴板变化则保留用户新内容，不恢复OLE代理/flush旧路径。

## 公共接口及构建入口

API9/ABI1无新增导出或公开结构布局变化；`light_document.h`仅补语义注释。完整公共C示例为SDK的examples/m2_document.c（相同源码位于tests/m2_document.c）：创建`ui_light_document_backend_create`，通过`ui_light_document_load`传source_url/HTML，回调只接收结构化导航/资源/GET字段；调用`ui_light_document_resource`和图片接口交付受策略允许的资源，使用get_state/find/get_element_rect验证。远程视图的load_html/post_json仍明确UNSUPPORTED。

实际生产命令由`tools/build-m2-production.ps1`执行：

```powershell
cmake -S D:/应用软件框架/应用层序框架 -B build/m2-production -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_BUILD_TESTS=OFF -DBROWSER_BUILD_CHECKS=OFF -DUI_FRAMEWORK_ENABLE_WEBVIEW2=OFF -DUI_OSMESA_LIBRARY=
cmake --build build/m2-production --target framework_host browser_package uapp_pack
./tools/build-m2-sdk.ps1 -SdkDir .work/m2/sdk-candidate
./tools/build-m2-sdk.ps1 -SdkDir .work/m2/sdk-candidate -BuildDir build/m2-checks -Checks
python tools/package-m2.py stage
python tests/delivery_m2.py
```

源码归档内的build-m2-sdk.ps1只需MSVC/CMake/Ninja及解压SDK，不需要框架/固定依赖源码、Mesa或Node。框架完整源码快照不内置固定依赖；pins与许可证在SOURCE-IDENTITY/SDK中，复建需提供同版本依赖。

额外失败记录：隐藏CSS与1px行高的新公共样本先失败2项，最小修复后0；受限工具链的首次新目录MSVC探测报C1902，完整本机工具链权限下重试通过，未改系统安装，不把错误未经证明归因于依赖或代理。正式包中的失败/撤回/未定位结论详见diagnostics.json；原M0/M1材料保持原样。
