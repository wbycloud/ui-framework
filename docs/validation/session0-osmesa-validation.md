# Session0／无登录 Windows CI：实际应用 DLL 与 OSMesa

日期：2026-10-04。接续 [API5 验收](api5-validation.md)，仅补验实际应用 DLL 的 OSMesa 路径。当前本地准备性回归通过，目标 CI 结果待追加；不据此宣称 Session0 或无登录通过。

## 1. 用例与成功标准

[独立应用 DLL](../../tests/session0_gl_app.c) 只导出 `ui_app_query_v1`，ABI1/API5，包清单保持八字段。运行器通过公共 offscreen workspace 加载 `.uapp`、语义命令及快照驱动应用。GL frame 位于 DLL 内，所有 GL 函数经 surface 解析，包内资源沿原 read/release 接口。未修改业务应用、冻结 SDK、PERF-001、原包、公共头文件或产品实现；不新增公共 ABI。

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

[脚本](../../tools/run-session0-validation.ps1)创建本次独有GUID名、手动启动的临时LocalSystem服务，只停止并删除自己创建的服务。不设自动启动、不保存密码、不改既有服务、不注销用户。超过60秒、缺少完成记录、环境不符、崩溃或断言失败均失败。产物包括精确源码/二进制/依赖哈希及环境manifest、run.log、resources.csv、8张真实PPM帧、result.txt和服务删除记录。

[工作流](../../.github/workflows/session0-osmesa.yml)使用windows-2022，校验依赖归档，记录实际image及编译器，不从镜像名称推断运行环境。main相关文件push或workflow_dispatch触发；成功与失败均保留30天artifact。公开check annotations保留关键原始记录、完整CSV/manifest和PPM原字节的SHA256/Base64，便于不提取本地凭据的复取；Base64仅为证据传输，不进入框架公共图片接口。

## 3. 本地失败及准备性结果

起点为干净 `128d03275ec702c373e7ef861a9e4e537dd961f0`，已登录Windows/Session1，未关闭用户应用。

- S4U任务注册被Windows拒绝，`0x80070005`，未创建任务；允许终端仍为非提升Session1。
- 默认运行器拒绝Session1/LogonType=2，退出1、0次frame，环境反例通过。
- 初版测试未在mount注册助手命令，29处失败、0次命令；补齐测试DLL的注册后进入原路由，不是产品缺陷。
- 初始“预热后仅允许增加8 MiB”在第15/17轮失败。200轮诊断观察私有量约13→106→13 MiB交替，句柄180–181、每轮DLL卸载；HeapCompact未降低末次私有量。保留两条失败曲线和诊断，不丢弃峰值；正式门槛改为声明的128 MiB进程预算，不声称缓存已释放。
- 最终sandbox Session1对照200轮0失败、2000命令、创建/残留HWND=0；句柄180→181，GDI0→0，USER2→2，私有量预热14,917,632／最终103,854,080／峰值106,045,440字节。不能替代目标环境结果。
- 原生21项为20PASS/1物理跨屏SKIP。后续环境证据细分不改产品代码，目标构建必须另记录commit和结果。

本地原始日志在忽略目录 `build/session0-evidence/`；目标证据另行追加并公开，不覆盖失败事实。
