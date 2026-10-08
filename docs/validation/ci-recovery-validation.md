# API9 托管 CI 失败的本地收敛（2026-10-08）

接手干净 `main`、HEAD `1756b06725c44078960b6f9c9b1a354adfcaee51`。只读 `git ls-remote` 确认远端 main 与 codex/menus-offscreen 仍为 `b6a36fee3f4c1a73818cdb01ea7271ad5abd30db`。本轮只在框架本地工作和提交，没有推送、远程工作流、业务仓库写入、新会话或子代理；没有修改产品源码、公共头、冻结 SDK、历史证据或 PERF-001。SDK0.9.0-dev/API9/标准9、ABI1/包格式1保持。

## 原失败与可复验定位

原 main 托管结果仍是 native 21/0/1、light 42/2/1、osmesa 47/4/1、webview2 56/5/1（通过/失败/物理跳过），绑定 b6a36fee、push、attempt1，见[原记录](github-sync-validation.md)。原证据 `build/github-sync-20261008` 的 437 份文件逐项核验长度和 SHA256，全部匹配；[核验记录](ci-recovery-evidence-20261008/raw/original-evidence-verification.json)保存本轮核对时间和原索引哈希。原目录没有写入。clone 不包含该 build 目录，不能以新日志冒充原 CI。

新目录 `build/ci-recovery-20261008` 的接手源码基线，五项原失败用例全部本机通过，121.17 秒；其中完整 Light 存储 36.64 秒、语言 64.68 秒。此中文 Windows 结果不能排除英文 runner 的语言问题，也不能定位原超时。

随后只让实际应用实例通过公共 `ui_host_set_language` 提交语言，保留旧中文断言，得到[可重复红例](ci-recovery-evidence-20261008/raw/locale-red.log)。[诊断差异](ci-recovery-evidence-20261008/raw/diagnostic-before-fix.patch)保存当时的输入和断言，未改变系统显示语言、全局 locale 或环境语言。

| 原用例 | 根因及证据 | 本轮变更 |
| --- | --- | --- |
| ui_browser_host | 空宿主按 Windows UI 来源显示英文，旧测试却固定比较中文最近列表空提示；原 runner 失败已保留 | 公共系统/宿主查询核验有效语言、来源、实际系统名称、提供者及 fallback；精确比较中/英文完整文本，保留可见与禁用断言、最后实例关闭后的系统来源 |
| ui_web_host_frontend | 活动实例提交 en-US 后，原“是”及两个“高级”断言稳定失败；实际提示为完整英文文案 | 两实例分别提交中英文并交换语言；按公共 chrome 查询比较完整 Yes/是及高级提示。原参数、未知大整数、非法草稿、UTF-8预算及一次执行断言全部保留 |
| ui_host_repaint | 红例准确指出失败是中文“操作完成”匹配。实际结果为 `Application #1: Operation completed. See Advanced for details.`；失效区已清空、实际 Dispatch1、所有空闲 paint0、关闭 window0 | 每条失败打印行号和表达式，保留 paint≤100 门槛；中英文完整结果、命令 request/instance 身份、实际 GL 图片 presentation 和正常关闭均验证 |
| 完整存储/语言超时 | 接手基线未复现；原存储最后输出 profile-b 成功，原语言最后输出 dpi96 restore 成功，随后分别180/240秒整项 Timeout。不能据此确定是否卡在进程退出或下一启动 | 只增加诊断和归档；未宣称超时已修复 |

重绘红例定位了本用例的语言期望，不构成历史持续重绘来源的根治证据。原固定中文的每条语义检查仍有对应中英文精确断言，没有任意非空匹配或删断言。

## 超时诊断与证据保存

两个完整脚本在每个子用例启动前写 manifest 的 `running` 状态并打印 CASE_BEGIN；原生调用及 Tee 管道返回后记录 elapsedMs、实际 exitCode 和 CASE_END。C 测试记录 PID、单调 tick、实际 Dispatch 计数及创建、打开、截图、关闭请求、COM退出和返回阶段。末条 C `failures0` 与脚本已获得进程退出码因此可以区分，不能再以子用例成功代替整项完成。

没有改变原消息泵、SendInput、等待、异步关闭、DLL卸载或全部存储/偏好输入。CTest原180/240/300秒及其他资源门槛保持；没有 sleep、强杀 Runtime、清用户目录、预加载或提高预算。CI脚本新增 `language-evidence-*` 归档，原 `state-evidence-*` 归档保持，超时留下的最后 running 阶段也能随后续授权运行保存。

完整存储每后端26个独立进程，含96/144/192写入/恢复、A/B隔离和锁、重排列/新旧列、10种异常文件、初始化失败、重置及跨进程默认。语言每后端7个独立进程，含10种偏好文件及三DPI写入/恢复；保留双实例、模态、草稿/选择、内容HWND、实际GL、AI尺寸回收、关闭和卸载。实际结果与剩余限制以下述最终记录为准。

## 最终回归与产物身份

| 本地提交 | 内容 |
| --- | --- |
| `f00bbeb790c64ad40c10b3ec1622a65331d3495e` | 三项宿主语言断言修正、精确中英文及GL/关闭检查 |
| `2fa6f84cb1ca202d54ff822971ca908c2e6dc36f` | 完整应用阶段/退出诊断与语言证据归档；没有超时产品修复 |

| 新构建配置 | 实际结果 | 原始证据 |
| --- | --- | --- |
| WebView2完整配置 | 62项：61通过/0失败/1物理跳过；WGL45项78.25秒、显式OSMesa/provider7项162.53秒、真实Runtime10项187.84秒 | [精确JUnit](ci-recovery-evidence-20261008/raw/webview2-results.xml)、[配置清单](ci-recovery-evidence-20261008/raw/webview2-test-plan.json)、[WGL](ci-recovery-evidence-20261008/raw/webview2-wgl.log)、[provider](ci-recovery-evidence-20261008/raw/webview2-provider.log)、[Runtime](ci-recovery-evidence-20261008/raw/webview2-runtime.log) |
| 不启用Runtime的OSMesa配置 | 52项：51通过/0失败/1物理跳过；WGL79.40秒、provider163.28秒 | [JUnit](ci-recovery-evidence-20261008/raw/osmesa-results.xml)、[WGL](ci-recovery-evidence-20261008/raw/osmesa-wgl.log)、[provider](ci-recovery-evidence-20261008/raw/osmesa-provider.log) |
| 无Web原生配置 | 22项：21通过/0失败/1物理跳过，6.18秒 | [JUnit](ci-recovery-evidence-20261008/raw/native.xml)、[原生日志](ci-recovery-evidence-20261008/raw/native.log) |
| CI校验器 | 精确清单、分阶段分类、非零失败及唯一物理跳过规则通过；三配置实际JUnit均经原校验器验证 | [原输出](ci-recovery-evidence-20261008/raw/ci-checks.log) |

三配置均仅跳过ui_monitor_transition，没有其他跳过。WebView2构建的完整Light存储/语言分别36.65/65.59秒，真实Runtime存储/语言41.28/72.06秒；独立OSMesa构建的Light存储/语言36.76/64.60秒。六份完整脚本manifest中每个子进程均为exited/exitCode0，见[汇总及manifest索引](ci-recovery-evidence-20261008/raw/result.json)。这些结果没有复现原180/240秒Timeout，不能宣称已修复原托管超时。

原生鼠标回归保留TREE/LIST到底、真实滚轮后回首、64列未缓存末列按需查询、同HWND捕获/取消、TREE异步轨道几何与96/144/192程序DPI。完整集成保留REFUSE/WAIT、两实例、模态、布局/列宽、草稿/选择/焦点、uint64/BigInt、预算、实际GL及DLL卸载。Runtime原重开资源断言实测句柄396→396、GDI12→12、USER17→17、pending cleanup0；没有以新阈值换取通过。

182份最终EXE/DLL/包的SHA256及八个改动源码的已提交Git blob在[产物绑定](ci-recovery-evidence-20261008/raw/final-artifacts.json)。二进制在本地提交前构建，随后逐项确认源码与2fa6f84树相同；产品目录、公共头、示例、CMake和工作流相对1756b06无差异。接手基线另有[独立产物哈希](ci-recovery-evidence-20261008/raw/baseline-artifacts.json)。红例保存实际日志与输入补丁，未单独封存诊断EXE，不以之后重建的哈希冒称当时的EXE身份。160份本轮原始交付文件的[SHA256索引](ci-recovery-evidence-20261008/raw/evidence-sha256.json)排除索引自身，局部.gitattributes保留原始字节。

准确构建命令保留在[WebView2](ci-recovery-evidence-20261008/raw/current-build.cmd)、[OSMesa](ci-recovery-evidence-20261008/raw/osmesa-build.cmd)、[原生](ci-recovery-evidence-20261008/raw/native-build.cmd)，分阶段运行器保留在[WebView2运行脚本](ci-recovery-evidence-20261008/raw/run-regression.ps1)与[OSMesa运行脚本](ci-recovery-evidence-20261008/raw/run-osmesa.ps1)。以下从仓库根目录、x64 VS开发终端重建至新目录；不覆盖本轮或历史证据。

```powershell
$provider=(Resolve-Path .deps/mesa-24.3.4/x64/osmesa.dll).Path
cmake -S . -B build/ci-recovery-replay-webview2 -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_FRAMEWORK_ENABLE_WEBVIEW2=ON "-DUI_OSMESA_LIBRARY=$provider"
cmake --build build/ci-recovery-replay-webview2
& ./docs/validation/ci-recovery-evidence-20261008/raw/run-regression.ps1 -BuildDirectory build/ci-recovery-replay-webview2 -EvidenceDirectory build/ci-recovery-replay-webview2-evidence
cmake -S . -B build/ci-recovery-replay-osmesa -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_FRAMEWORK_ENABLE_WEBVIEW2=OFF "-DUI_OSMESA_LIBRARY=$provider"
cmake --build build/ci-recovery-replay-osmesa
& ./docs/validation/ci-recovery-evidence-20261008/raw/run-osmesa.ps1 -BuildDirectory build/ci-recovery-replay-osmesa -EvidenceDirectory build/ci-recovery-replay-osmesa-evidence
./tools/test-windows-ci.ps1
```

原生独立配置使用UI_BUILD_STANDALONE_HOST=OFF、UI_FRAMEWORK_ENABLE_LIGHT_WEB=OFF、UI_FRAMEWORK_ENABLE_WEBVIEW2=OFF；本轮软件WGL四DLL局部部署、GALLIUM_DRIVER=llvmpipe与provider/Runtime移除隔离均与既有CI一致。运行器固定本机CTest路径，其他机器须替换为其经验证的CTest≥3.26路径；不依赖原build目录、临时网络脚本或远程执行。

## 独立环境限制

本轮 Windows 10.0.26300、Session1已登录桌面、MSVC19.50.35727、C11 Release/Ninja；实际 Runtime154.0.4258.62。Lexbor `7fb22cf5664a331d7c24b113489e566767c9c25a`、QuickJS-NG `2f0aa72a6b09cf69ff06399bcf9f4c083ac1a278` 干净且固定；WebView2 SDK1.0.4129.50、Mesa24.3.4。[实际环境及依赖哈希](ci-recovery-evidence-20261008/raw/environment.json)不套用旧 runner/MSVC/image 数据。软件 WGL、显式 OSMesa和真实 Runtime阶段沿用现有隔离，桌面输入串行执行。

真实系统查询只有 zh-CN；实例 en-US 是应用提交，其他 Windows语言映射是核心单元证据。尚未在真实英文 Windows 对空宿主重跑。本机通过不替代托管 CI；本轮无推送/远程执行授权，原超时与历史瞬态导航/清理问题仍待带诊断的目标环境运行。

原 b6a36fee 的 LocalSystem/ServiceMain、Session0 实际应用DLL/OSMesa GL 已通过：failures0、commands2000、created_HWNDs0/process_HWNDs0。严格整机无登录仍失败：logged_sessions1、inventory_valid1；已登录runneradmin的Session2是独立环境条件。没有新的服务/无登录复验，也没有注销用户、改服务或降低RequireNoLogin。本轮桌面Session1对照另计。

真实IME、物理不同DPI跨屏、桌面合成/边缘、长期人工及严格无登录服务环境继续待验。冻结SDK产品6d88770/文档312bce4和原归档SHA256 `7c1dbffce1211b2c0654b41d2942b05adbd4c54d111e983d836c27550d8802e1`再次匹配，见[只读核验](ci-recovery-evidence-20261008/raw/frozen-sdk-verification.json)；不以本轮测试/工具修改重写旧包，当前不需要新的产品SDK交付。
