# browser.uapp 实际验证与交付记录

日期：2026-10-20。结果针对本轮完整源码快照及匹配 SDK，不代表任意网站兼容或任意恶意网页安全性。

## 身份与改动账

应用源码：`D:\应用软件框架\uapp浏览器`，接手时为空、无 Git 仓库，未创建提交。框架：`D:\应用软件框架\应用层序框架`，HEAD `689fa03b34f8c4321a6c838f52ca9dcd790a9319`，分支 `codex/p1-process-boundaries`；接手时已脏，保留原修改及未跟踪材料。没有实际 AGENTS.md 文件；执行用户提供的 AGENTS 约束。

应用改动为 C11 应用、WinHTTP/WIC/编码/URL 封装、工具栏与包内页、支持表、构建脚本和固定测试。框架增量为 `light_document.h`、`app_data.h`、Light 受限文档实现、workspace 复制数据投递、宿主菜单入口、构建接线和对应文档。框架 `apps/browser` 保存相同应用源码；宿主不承担网络、历史或阅读逻辑。`framework-changes.patch` 以接手时的脏文件为基线，不能直接假设适用于干净 HEAD。

SDK0.9-dev/API9/标准9、ABI1、包格式1。无既有公开结构布局变更，无冻结 SDK 覆盖，无历史兼容专项恢复。固定 Lexbor `7fb22cf5664a331d7c24b113489e566767c9c25a`，QuickJS-NG `2f0aa72a6b09cf69ff06399bcf9f4c083ac1a278`；依赖源码未修改。源码 ZIP 的 SHA256SUMS 以及 SDK 的 SOURCE-IDENTITY.json 标识实际未提交文件。

## 构建

Windows x64、MSVC19.50.35727、VS18 BuildTools、CMake4.2.3-msvc3、Ninja。先进入 x64 Developer PowerShell，或由 `tools/msvc-env.cmd` 导入环境。

```powershell
cmake -S D:/应用软件框架/应用层序框架 -B build/production -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_BUILD_TESTS=OFF -DBROWSER_BUILD_CHECKS=OFF -DBROWSER_DIAGNOSTIC_SYMBOLS=OFF -DUI_FRAMEWORK_ENABLE_WEBVIEW2=OFF -DUI_OSMESA_LIBRARY=
cmake --build build/production --target framework_host browser_package uapp_pack
./tools/build-sdk.ps1 -SdkDir D:/应用软件框架/uapp浏览器/deliverables/browser-sdk-20261020
```

以上两种构建实际成功；第二种仅使用 SDK 公共头文件、导入库、运行库、打包器和固定 Lexbor 静态库，不需要框架或 QuickJS 源码。正式 browser.uapp 取 SDK 独立构建结果。构建无 Node、硬件 GL、Mesa、WebView2 依赖；框架原有 OPENGL32 导入仍存在，浏览器不创建 GL 上下文。DLL 导出只有 ui_app_query_v1，WinHTTP/WIC 均为系统能力。WebView2 源码及原测试完整保留。

## 固定测试与结果

服务源码为 `tests/server.py`，HTTP `127.0.0.1:8765`、HTTPS `localhost:8766`。测试依赖仅 Python/Pillow/cryptography；应用没有 Python 运行依赖。

```powershell
python tests/make_tls.py .work/tls
python tests/server.py --log .work/requests.json --tls .work/tls
cmake -S . -B build/checks -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_BROWSER_SDK_DIR=<匹配SDK> -DBROWSER_BUILD_CHECKS=ON
cmake --build build/checks
$env:PATH="<匹配SDK>/bin;$env:PATH"
./build/checks/browser_document_check.exe
./build/checks/browser_url_check.exe
./build/checks/browser_failure_check.exe <browser.uapp>
./build/checks/browser_workspace_check.exe <browser.uapp>
python tests/host_process.py <解压后的运行目录>
```

| 检查 | 实际结果及观察终点 |
| --- | --- |
| 纯公共 C 隔离 | failures=0；无 JS runtime，远程脚本/内联事件无应用调用，可信加载、JSON桥及消息回调拒绝；结构化链接、作者颜色、空SPA拒绝、超大字号拒绝 |
| URL/来源 | failures=0；相对路径、带端口数字主机、IDNA、片段、中文、GET编码、特殊协议/凭据/来源拒绝 |
| 初始化与挂载失败 | failures=0；无效来源表、缺工具栏、缺起始页、无效工具栏后均无实例残留且 DLL 已卸载；随后正常包成功；最新句柄151→157，在既定容差内 |
| 实际标准包 | failures=0；双实例、标题、长地址/标题、四种图片、表格/代码/列表、UTF8/GB18030/UTF16、gzip、404/500、断开连接12152、损坏资源、编码/深度/节点/资源/字号超限 |
| 导航与关闭 | 慢旧结果未覆盖新页，停止完成；下载中关闭与确实观察到 WIC decoding=1 时关闭均等到线程/回调结束，GetModuleHandle(browser_app.dll)==NULL；进程正常退出 |
| 真实文档输入 | failures=0；原生点击、表单编辑与GET提交、滚轮、Tab/反向边界、选择复制、查找、Home/End；程序DPI96/144/192、窄窗口、明暗工具栏；截图经目视核对 |
| 实际包快捷键 | failures=0；Ctrl+L/F/R、F5、Alt+Left/Right、Esc、Ctrl+Plus/Minus/0；真实地址栏点击与查找后Ctrl+C，原剪贴板/前台/鼠标位置恢复，桌面释放 |
| 宿主入口 | failures=0；空启动无浏览器 DLL；原生菜单“浏览网页”打开两个普通 workspace 实例；关闭后真实卸载。入口夹具不计入纯公共 C 证明 |
| 相关框架回归 | 9/9，通过2.80秒：package_format、theme_core、public_headers_c、light_rect_stability、light_scroll、menu_cascade_offscreen、workspace_titles、text_replacement、light_web_dynamic |

HTTPS 先验证未信任自签证书被系统拒绝（12175）。经用户明确授权，临时加入当前用户 Root 的唯一测试证书 SHA1 `7A91FD79DA98FBCD64DFA975793C9B687DD66BE7`，固定 `/page` 正向加载标题和 PNG 成功，脚本 runtime=0，完整检查 failures=0。随后立即通过 finally 移除，重新打开证书库核对数量0；新应用进程再次得到12175且完整检查 failures=0。没有使用忽略证书错误标志。系统加入/移除确认均只作用于已核实测试进程、localhost及完整指纹；测试私钥随后清理。

## 性能观察

样本预先固定为 server.py 的 PAGE：中文/英文25段、12列表格、1700字节代码长行、链接/表单、图片及拒绝脚本。网络、文本转换、WIC解码、Lexbor解析、布局、WM_PAINT 分别记录；后台隐藏 workspace 的 paint_count=0 不能当作可见页面。

一次冷启动真实窗口样本，观察终点为首个已完成 WM_PAINT 后再泵100ms：总153.228ms，解析2.077ms、布局52.641ms、绘制7.552ms，paint_count=1。该直接文档样本未交付图片，保留替代标记；不据此推断网络或完整页面可用时间。可信本地 HTTPS 样本主请求结束34.655ms、解码0.107ms、解析2.660ms、末次布局1.604ms；其隐藏窗口无可见延迟结论。各分段为最近操作计时，不可机械相加。

16次 page/next 往返，最后一次普通公共检查私有内存20,451,328→16,654,336字节；原门槛是增长≤32MiB且总量≤128MiB，未抬高。它只证明短程趋势，不能代表长期稳定性。

## 失败、撤回与未验证

- 公网 `https://example.com/` 补充测试失败，WinHTTP12002超时；没有提高超时或计为通过。受控本地HTTPS正反向已独立通过。
- 早期节点扩容后旧指针导致崩溃，已修复并回归；错误中文 showIncludes 前缀导致增量漏编，已修复并重建。旧二进制/失败截图不作为通过证据。
- 固定 Lexbor2.5 数字主机相对URL复制遗漏 host.type，在本任务两个包装层恢复已解析元数据，先失败后通过，未改固定依赖。
- 输入夹具初次未等到原生事件送达、宿主观察过早读取模块列表、误写 CMake 目标名及字号用例超出原解析器范围，均修正后重跑。失败轮次撤回通过计数。测试启动的遗留隐藏宿主已正常 WM_CLOSE 退出。
- 未运行完整框架矩阵、WebView2回归或C++头文件专项（当前配置未发现C++编译器）；不恢复历史兼容检查。固定依赖已有编译告警未顺手修改。
- 物理DPI、多屏、真实IME、长期人工阅读、任意现代网站/恶意网页安全性、完整无障碍树及强制系统内存耗尽未验证。首版限制详见 browser.md。

## 交付、解压复验与清理

正式文件位于工作区 `deliverables`：browser.uapp、运行 ZIP、匹配 SDK ZIP、应用和框架源码 ZIP、本轮框架补丁、接手基线哈希、SHA256SUMS。SDK 的 verify 目录带仅用公共C接口的验证程序；源码 ZIP 不含构建、缓存、日志、截图或依赖副本。框架源码快照不含 .deps，重建框架仍需上述固定依赖；应用可直接用匹配SDK重建。

归档生成后的独立解压、全文件SHA复核、公共程序、实际 framework_host.exe 的空启动/带包运行及进程退出结果另存 `deliverables/verification.json`，以该机器结果确认实际执行，不以API成功代替运行。收尾删除本轮 build、.work、测试服务、临时证书私钥、截图、日志及解压目录；保留源码、简明记录和正式交付物，不删除既有框架材料。
