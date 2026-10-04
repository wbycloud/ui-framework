# Session0／无登录 Windows CI：实际应用 DLL 与 OSMesa

日期：2026-10-04。接续 [API5 验收](api5-validation.md)，仅补验实际测试应用 DLL 的 OSMesa 路径。**真实 Session0 GL通过；整机无登录未通过**。托管runner有一个已登录会话，用户确认当前没有无登录runner；不宣称整个目标或全阶段已完成。

## 1. 用例与成功标准

[独立应用 DLL](../../tests/session0_gl_app.c) 只导出 `ui_app_query_v1`，ABI1/API5，包清单保持八字段。运行器通过公共 offscreen workspace 加载 `.uapp`、语义命令及快照驱动应用。GL frame 位于 DLL 内，所有 GL 函数经 surface 解析，包内资源沿原 read/release 接口。未修改业务应用、冻结 SDK、PERF-001、原包或公共头文件；产品仅修复等价的编译期位宽检查，不新增公共 ABI。

[运行器](../../tests/session0_gl.c) 默认拒绝非 Session0，记录实际进程 SessionId、令牌登录类型、SYSTEM SID、SCM ServiceMain、window station、desktop。WTS 枚举全部会话用户名，包括断开会话；`--require-no-login` 另要求枚举成功且整机已登录会话数为0。SYSTEM 固定认证会话若报告 Undefined(0)，仅在 SYSTEM SID、Session0、实际 SCM ServiceMain 同时成立时接受。

两项分别报告。有登录用户时继续独立 Session0 GL 用例，无登录组合仍失败；不注销用户。`--allow-interactive` 仅产生 `INTERACTIVE_CONTROL` 对照结果。

200轮共400实例、1200次真实 frame、2000次命令，交替精确 samples=0/4。frame 查询 GL_SAMPLES、绘制三角形并同步读取顶向下 RGBA8；NULL缓冲不执行frame。4 samples必须存在抗锯齿中间值，0 samples没有。验证输入改变输出、非活动实例拒绝输入、双实例隔离、80×50 resize、预算失败保持旧尺寸、9999 samples明确失败。

每轮关闭后要求应用 DLL卸载、当前 OSMesa context为NULL、module_shutdown实例/surface计数为0。顶层、子窗口、message-only窗口及UI线程CBT捕获的瞬时HWND创建均为0。CSV保留每轮句柄、GDI、USER、私有提交量及卸载状态。

资源门槛：四轮预热后句柄增量≤2，GDI/USER保持预热值，正式采样私有提交量≤128 MiB。Mesa/LLVM及系统分配器保留进程缓存，不要求模块或提交量回到加载前。这些有限轮次不能证明所有业务资源无泄漏或替代长期人工压力。

## 2. 依赖与复验

Windows x64/C11、MSVC、匹配运行库。固定 [Mesa 24.3.4 MSVC归档](https://github.com/pal1000/mesa-dist-win/releases/tag/24.3.4)，SHA256 `7ebc711ad1896ac88ab21e142f1017f8ff035f0f342bdb72fbb5e2eb881ba363`。运行实际软件桌面GL，不声称GPU加速；不用WGL、替代图片或Web后端测试代替GL。依赖和许可遵守[构建说明](../build-and-validation.md)。

在x64 VS开发终端准备归档的x64 DLL后：

```powershell
$provider = (Resolve-Path .deps/mesa-24.3.4/x64/osmesa.dll).Path
cmake -S . -B build/session0 -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_BUILD_STANDALONE_HOST=OFF -DUI_FRAMEWORK_ENABLE_LIGHT_WEB=OFF -DUI_FRAMEWORK_ENABLE_WEBVIEW2=OFF "-DUI_OSMESA_LIBRARY=$provider"
cmake --build build/session0 --target ui_session0_gl_test
```

在提升权限的PowerShell或具有服务管理权限的Windows CI，使用全新的证据目录：

```powershell
./tools/run-session0-validation.ps1 -BuildDirectory build/session0 -OSMesaLibrary $provider -OutputDirectory build/session0-evidence/target -RequireNoLogin
```

[脚本](../../tools/run-session0-validation.ps1)创建本次独有GUID名、手动启动的临时LocalSystem服务，只停止并删除自己创建的服务。不设自动启动、不保存密码、不改既有服务、不注销用户。超过120秒、缺少完成记录、环境不符、崩溃或断言失败均失败。产物包括精确源码/二进制/依赖哈希及环境manifest、run.log、errors.log、resources.csv、8张真实PPM帧、result.txt和服务删除记录。原60秒门槛调整为120秒，是实测200轮服务阶段耗时56秒后为CI调度波动预留的时间；资源预算和轮次未放宽。

[工作流](../../.github/workflows/session0-osmesa.yml)默认windows-2022，手动执行可填已有Windows x64 `runner_label`，不自动创建runner。目标需以服务运行且整机无登录；托管默认项能测Session0，当前整机门槛会明确失败。工作流校验归档、记录实际image及编译器，不从镜像名称推断环境。main相关文件push或workflow_dispatch触发；成功与失败均保留30天artifact。公开check annotations保留关键原始记录、完整CSV/manifest和PPM原字节的SHA256/GZip/Base64；文本按3000字符分块，避免GitHub实测4096字符截断。证据压缩可逆，不改变GL像素；不进入框架公共图片接口。

## 3. 本地失败及准备性结果

起点为干净 `128d03275ec702c373e7ef861a9e4e537dd961f0`，已登录Windows/Session1，未关闭用户应用。本节保留初始产品未改时的准备性记录；随后CI暴露包加载器的编译兼容问题，修复见第4节。

- S4U任务注册被Windows拒绝，`0x80070005`，未创建任务；允许终端仍为非提升Session1。
- 默认运行器拒绝Session1/LogonType=2，退出1、0次frame，环境反例通过。
- 初版测试未在mount注册助手命令，29处失败、0次命令；补齐测试DLL的注册后进入原路由，不是产品缺陷。
- 初始“预热后仅允许增加8 MiB”在第15/17轮失败。200轮诊断观察私有量约13→106→13 MiB交替，句柄180–181、每轮DLL卸载；HeapCompact未降低末次私有量。保留两条失败曲线和诊断，不丢弃峰值；正式门槛改为声明的128 MiB进程预算，不声称缓存已释放。
- 最终sandbox Session1对照200轮0失败、2000命令、创建/残留HWND=0；句柄180→181，GDI0→0，USER2→2，私有量预热14,917,632／最终103,854,080／峰值106,045,440字节。不能替代目标环境结果。
- 原生21项为20PASS/1物理跨屏SKIP。后续环境证据细分不改产品代码，目标构建必须另记录commit和结果。

本地完整日志在忽略目录 `build/session0-evidence/`，公开[失败及修复摘要](session0-osmesa-failures.log)保留上述事实。

## 4. 实际 Windows CI 结果与修复

实际运行源码 [3e1fca59daa189d15302c52f22394e9f5d37fd0a](https://github.com/wbycloud/ui-framework/commit/3e1fca59daa189d15302c52f22394e9f5d37fd0a)，框架和测试DLL使用同一commit。记录 [Actions run 37210914032](https://github.com/wbycloud/ui-framework/actions/runs/37210914032)，2026-10-04 14:53:45–14:54:41 UTC。整项job为FAIL，因为无登录门槛失败；不把job状态改称PASS。

| 核验 | 实测结果 |
| --- | --- |
| 系统/工具链 | Windows Server2022 build20348，image `win22/20260927.320.1`；MSVC19.44.35229 x64/C11/W4/WX，CMake3.31.6 |
| 真正服务执行 | PID1012，SessionId0，LocalSystem=1，SCM ServiceMain=1，LogonType0，`Service-0x0-3e7$` / Default；符合第1节SYSTEM固定认证会话规则 |
| 实际 GL | Mesa / llvmpipe LLVM19.1.7 256bits / 4.5 Compatibility Mesa24.3.4 git1950a8b78c，软件GL；实际samples0与4，NO_WINDOW且native_handle=NULL |
| frame / 输入 / 生命周期 | 200轮、400实例、1200真实frame、2000命令，独立GL用例0失败，完整卸载/重开、输入、双实例、resize、预算及不支持配置通过 |
| 窗口 | 201次采样均0 process HWND，UI线程累计创建HWND0；未借用已登录桌面做渲染 |
| 资源 | 句柄预热/最终/峰值148/148/148，GDI0/0、USER1/1；私有量9,023,488 / 12,316,672 / 100,663,296字节，符合128 MiB预算；provider进程缓存驻留 |
| 整机无登录 | WTS枚举有效，已登录会话1，NO_LOGIN FAIL；这是缺少目标机器的失败，不是成功运行证据 |
| 清理 | 本次GUID服务停止并删除，`DeleteService SUCCESS`；未修改既有服务或注销用户 |

永久证据：[关键原始日志](session0-osmesa-result.log)、[完整CSV](session0-osmesa-resources.csv)、[精确二进制/依赖manifest](session0-osmesa-manifest.json)、[原始公开check annotations](session0-osmesa-checks.json)、[服务删除记录](session0-osmesa-service-cleanup.log)。完整run.log/构建日志/PPM在上述Actions的 artifact `session0-osmesa-3e1fca59daa189d15302c52f22394e9f5d37fd0a-1`（ID11306188229，30天保留，下载需正常GitHub登录）。提供方归档哈希见第2节。

8张PPM从公开annotation的GZip/Base64复取，每张都校验原SHA256。下列PNG仅作无损格式转换，没有重绘、生成或替代应用frame：64×64单采样中间值0、4 samples中间值119；真实输入后118，resize80×50后119。

![Session0单采样实际输出](session0-osmesa-plain.png)
![Session0四采样实际输出](session0-osmesa-msaa.png)
![Session0实际输入后输出](session0-osmesa-input.png)
![Session0实际resize后输出](session0-osmesa-resize.png)

两次编译失败保留：[run37209678540](https://github.com/wbycloud/ui-framework/actions/runs/37209678540)和[run37209999053](https://github.com/wbycloud/ui-framework/actions/runs/37209999053)。MSVC19.44/WX在包加载器 `sizeof(void*) != 8` 报C4127，改为等价 `UINTPTR_MAX != UINT64_MAX` 编译期分支；64位限制、错误返回及C ABI不变，未压制警告。[c93d083运行](https://github.com/wbycloud/ui-framework/actions/runs/37210160101)构建通过但服务日志初始化失败。无控制台的本地DETACHED_PROCESS复现退出2、空run.log；stderr `_fileno` 为-2，原dup2不能使用。分别重开stdout/stderr文件后本地无控制台200轮通过，并得到本节实际Session0结果。

修复后[必要回归日志](session0-osmesa-regression.log)：完整43项42PASS/1物理跨屏SKIP，原生21项20PASS/1同项SKIP。原API1/2/3二进制混合宿主及原API4包分别0失败，四个原包SHA256均保持[API5历史值](api5-validation.md#3-可复验步骤与最终检查)。原SDK1/2/3/4未修改，版本仍SDK0.5/API5/ABI1/包1，只有测试和等价编译兼容修复。

待补：取得整机无用户登录的Windows x64服务模式runner，再手动选择其真实标签执行同一严格工作流；要求WTS已登录会话0及GL/回收断言全部通过。用户目前无此runner，不能记录无登录通过。此证据也不代替轻量Web/隐藏WGL/WebView2的目标环境验收、真实IME、物理跨屏、长期人工压力或请求方应用完整业务接入。
