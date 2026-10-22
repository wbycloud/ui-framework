# M0/M1 0.2.0 本批验收

本批日期2026-10-20。旧 docs/validation.md 与旧正式归档保持首版身份。

## 接手与责任

应用无Git；框架HEAD 689fa03b34f8c4321a6c838f52ca9dcd790a9319、分支codex/p1-process-boundaries、已有未提交修改。API9/标准9/SDK0.9.0-dev、ABI1/包1政策不变。读取实际交接、路线、标准、版本/构建/宿主/主题与公共C头文件；未发现实际AGENTS.md，执行用户提供约束。

应用是维护入口，apps/browser是集成镜像；接手只差README和两份新路线/交接文档，无镜像独有修改。同步由 tools/integration-state.json 审阅摘要保护，默认只检查，目标独有修改拒绝覆盖。框架本批预计仅集成应用源码，不新增公共接口；最终以源码差异清单确认。

## 已建立的失败检查

- 旧URL解析对非法编辑会清空旧规则：失败1；整份校验后替换的新实现通过。
- 新纯公共C调用方对旧正式包缺少设置及policy状态：失败2且正常卸载；新包集成检查通过。
- 可解析通配符来源与端口0均先复现被旧实现接受；新实现拒绝。端口0被WinHTTP解释为默认端口，不能当作精确来源端口使用。
- 固定慢响应5秒时原实现等到响应才报文档空，已增加可取消的3秒操作等待、15秒总期限。应用等待1460与WinHTTP12002分开；实际SendRequest可能仍未完成，不假设一定进入Headers。
- 新设置原子写、非法/不可读拒绝、空列表、恢复默认、多实例500ms重读、在途取消、诊断脱敏、重定向/资源分类、脚本隔离均由测试源码覆盖。

## 网络问题证据

公开WinHTTP查询报告machine_access=1、机器代理0、当前用户代理/PAC/自动检测均0。只读代理探针用相同3秒参数先在旧默认和自动模式均得到12002，随后两种模式均HTTP200，DNS/TCP建立、TLS标志0；实际新包也已读取Example Domain且真正卸载。未改系统代理或TLS设置、未增加超时。外部瞬态超时的根因仍未定位，不把自动代理切换当作已证明的超时根因修复。

Windows当前建议自动代理模式，旧DEFAULT_PROXY不会继承用户配置；本批采用AUTOMATIC_PROXY以遵循系统/用户配置。来源及逐跳检查、禁Cookie/自动认证/下载、证书验证仍保留。参考[WinHttpOpen](https://learn.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpopen)、[响应超时选项](https://learn.microsoft.com/en-us/windows/win32/winhttp/option-flags)。

## 通过及失败记录

| 检查 | 本批实际结果 |
| --- | --- |
| 新SDK纯公共C文档/URL/设置/失败注入/M01/工作区 | 六个程序exit0；script_runtime=0，下载中/已观察WIC解码中关闭，DLL均真正卸载 |
| 设置 | 原子持久化、非法及不可读拒绝、空列表、默认恢复、三实例重读、旧请求取消、远程写入隔离通过 |
| 诊断 | URL/端口/协议/凭据/origin、逐跳拒绝及循环、404/500、断开/超时、MIME/附件/编码/空文档/预算、图片WIC及样式分类、查询脱敏通过 |
| 真实输入 | 最后整轮failures=0：点击地址/设置/诊断，Ctrl+L/F/C、缩放、Alt左右、刷新及Esc，设置保存/默认、诊断复制、Tab退出；明暗、320逻辑宽与96/144/192程序DPI通过，桌面恢复释放 |
| 资源短程 | 固定PAGE/next往返16次：private 22,831,104→18,821,120字节；总量≤128MiB、增长≤32MiB门槛通过；不代替长期压力 |
| 本地HTTPS | 最终匹配包信任新测试根后正文及图片通过；完整公共M01程序exit0并卸载；删除后新进程12175/TLS标志0x8、exit0并卸载 |
| 临时根 | SHA1 8529E4E87442F35FC7DEAB957D723C8C3B0304A6，CurrentUser/Root初始0、撤除后独立certutil查询0；旧测试根48EF9DEA7A462F77E2FD54B634E60113044A488F也不存在 |
| 框架相关回归 | package_format/theme_core/public_headers_c/light_rect_stability/light_scroll/menu_cascade_offscreen/workspace_titles/text_replacement/light_web_dynamic共9/9；完整矩阵未运行 |
| 生产构建 | MSVC19.50.35727、C11、Release，UI_BUILD_TESTS=OFF、BROWSER_BUILD_CHECKS=OFF、WEBVIEW2=OFF、UI_OSMESA_LIBRARY为空；新SDK仅头文件/库独立构建检查OFF通过 |
| 源码保护 | 模拟镜像独有修改被拒绝且原文件保留；实际镜像摘要保护同步通过；旧正式10个文件SHA、公开头文件及政策与接手一致；固定依赖无修改 |

初轮桌面夹具用Unicode换行代替Enter造成保存断言失败；第一及第四轮在OLE剪贴板恢复/flush阶段发生ntdll.dll 0xc0000005（首例偏移0x12c513）。底层崩溃原因未定位，不能归因到应用；已改为输入前物化可读取的剪贴板句柄并恢复，之后无该崩溃。曾声明但不可读取的DIB/DIBV5原数据无法恢复，未冒称完整原剪贴板保真。第二轮未取得前台仍输入是测试保护缺陷；后续前台、鼠标目标保护均已加入。中止/失败轮次均不计通过。

实际Alt前进失败已定位：应用截获箭头按下，菜单仍记着此前Alt按下；Alt释放打开UIFrameworkRegisteredMenu4，拦截后续按键。应用使用公共ui_host_close_menu在处理Alt导航时清除待打开菜单状态，真实后退/前进复验通过；未修改框架私有结构。

HTTPS复验曾遇旧证书重复导入未实际持久化：导入返回成功但独立存储查询无记录，系统UntrustedRoot，应用12175；该轮撤回正向通过。最后重新生成显式CA约束localhost夹具，按PID/完整SHA1确认真正的“是”按钮，独立原生存储查询后正向通过、随后立即删除。旧夹具未持久化的底层原因未完全定位，不视为浏览器TLS故障已修复。

框架首次回归8/9：workspace_titles缺少未构建的web_counter.uapp；补建原有web_counter_package后相同9项全过，没有改测试或缩减样本。

## 构建及复验入口

在MSVC可用的PowerShell中，本批实际执行：

```powershell
./tools/build-m01-production.ps1
python tools/package-m01.py candidate
./tools/build-sdk.ps1 -SdkDir "$PWD/.work/m01/sdk-candidate" -BuildDir build/m01-sdk-only
./tools/build-m01.ps1 -SdkDir "$PWD/.work/m01/sdk-candidate" -BuildDir build/m01-checks
# bin加入PATH；测试服务：python tests/server.py --log .work/m01/requests-final.json --tls .work/m01/tls2
./build/m01-checks/browser_document_check.exe
./build/m01-checks/browser_url_check.exe
./build/m01-checks/browser_settings_check.exe
./build/m01-checks/browser_failure_check.exe ./build/m01-sdk-only/browser.uapp
./build/m01-checks/browser_m01_check.exe ./build/m01-sdk-only/browser.uapp
./build/m01-checks/browser_workspace_check.exe ./build/m01-sdk-only/browser.uapp
# --desktop必须另行协调前台；TLS脚本必须取得当轮临时根授权。
python tools/package-m01.py stage
python tests/delivery_m01.py
python tools/package-m01.py final
```

生产框架输入为实际已脏源码，不将HEAD当完整源码身份。新交付在deliverables/m01-20261020：browser.uapp、browser-runtime-m01.zip、browser-sdk-m01.zip、应用与框架源码ZIP、仅apps/browser的本批补丁和SOURCE-IDENTITY.json。全部SHA与新交付INDEX/SHA256SUMS配套；应用无Git，源码身份采用归档内逐文件SHA清单。旧正式交付及首版验收保持历史身份。

独立归档复验以同目录verification.json为机器证据：逐文件SHA、公共C解包重包一致、新包及重包实际工作区加载/卸载、运行包宿主进程关闭退出、从解压应用源码仅对解压SDK重建并实际运行。没有用测试日志failures=0代替独立模块及进程观察。

本批框架改动只在apps/browser集成镜像；没有新增公共ABI、改既有结构布局或覆盖冻结SDK。仅公共C示例见tests/m01_public.c、document_public.c、workspace_public.c、failure_public.c；来源持久化和网络诊断属于应用。

## 尚未解决或未验证

公网12002瞬态根因、旧证书重复导入不持久化、早期OLE恢复崩溃底层原因各自保留独立结论。没有指定目标公网站点，未承诺整站兼容。物理DPI、多屏、真实IME、长期人工、完整框架矩阵及任意恶意网页安全性未验证。性能network_ms包含转换与句柄收尾，隐藏窗口paint=0不计可见延迟；拆分EOF/转换、长期压力属于M4。

M2阅读扩展、M3日常体验、M4可靠性性能与M5稳定交付深化保留后续。清理的精确结果写入交付cleanup.json：只清本批中间物、截图、私钥/临时证书、解压目录和已核验的专用测试进程；保留旧材料及最小未定位证据。
