# P1 托管 Light 超时：授权执行与退出清理定位

2026-10-09。记录者：Codex 自动诊断。历史原始观察者未记录；不从 Git 作者推断测试者。

本次授权的提交、分支推送及一次目标 CI 诊断均已完成。两个配置构建成功，原存储和语言四项均超时失败。四份原生转储一致定位到 **Mesa24.3.4 在进程退出的 DLL 清理中等待 rasterizer 工作线程的退出信号**。这是本次目标失败的具体阻塞操作；尚未完成安全修复或修复后的目标复验。没有把日志末尾 `failures0` 当成成功，也没有将这四份新栈追溯为全部历史超时的已证实根因。

本报告接续 [P1–P6 本地根因调查](root-cause-investigation-validation.md)，原报告和授权方案保留原字节及当时的“未执行”状态。本次新事实以本报告及 [新证据目录](p1-hosted-diagnostic-evidence-20261009/README.md) 为准。

## 1. 授权动作和源码基线

基底为本地 `main/82a774aaec41d0f6a0adc0f7c25107724ca94bc3`，保留上轮提示修复、API9 校准和公共探针修改。严格核对 [审查的 16 个路径](p1-hosted-diagnostic-evidence-20261009/raw/approved-commit-files.json) 后，提交到 `codex/p1-process-boundaries`：

`65f678c98dad601833035761f009479018d23079`，见 [GitHub 提交](https://github.com/wbycloud/ui-framework/commit/65f678c98dad601833035761f009479018d23079)。完整差异为 [commit.patch](p1-hosted-diagnostic-evidence-20261009/raw/commit.patch)。推送及远端分支响应分别在 [push.log](p1-hosted-diagnostic-evidence-20261009/raw/push.log)、[api-ref.json](p1-hosted-diagnostic-evidence-20261009/raw/api-ref.json)。本地 main 未改变，也没有合并、PR、发布或标签。

16 个路径：`.gitattributes`、`.github/workflows/windows-regression.yml`、`CMakeLists.txt`、`docs/application-development-standard.md`、`docs/build-and-validation.md`、`src/components.html`、`src/menus.c`、`tests/display_probe.c`、`tests/framework_features_host.c`、`tests/interaction_probe.c`、`tests/observe_processes.py`、`tests/process_stall_capture.py`、`tests/presentation_transition_probe.c`、`tests/readonly_navigation_probe.c`、`tests/runtime_boundary_probe.c`、`tools/windows-ci.ps1`。产品改动仍为前轮已经验证的提示约束/穿透和末端页脚计数；本次没有追加 Mesa 或框架退出补丁，也没有新增公共 API。未跟踪的本地报告与证据不混入此次提交。

应用 CI v2 提案后，`windows-ci.ps1` 一次出现工作区混合行尾的字节差异；规范化文本相同，随后恢复为审查文件的精确原字节。初次检查与最终匹配分别保留 [applied-hashes](p1-hosted-diagnostic-evidence-20261009/raw/applied-hashes.json)、[applied-exact-hashes](p1-hosted-diagnostic-evidence-20261009/raw/applied-exact-hashes.json)。提交中的 Git LF blob 与工作区 CRLF 文件分别取 hash，没有将二者混同。补充 [公共头及销毁路径源码](p1-hosted-diagnostic-evidence-20261009/raw/supplemental-identities.json) 绑定同一 commit；首次快照中误列不存在的 `include/ui_framework/framework.h` 已保留为 missing，正确公共头是 `include/ui_framework/ui.h`。

## 2. 一次工作流、精确用例和实际环境

仅对现有 `windows-regression.yml` 发出一次成功的 `workflow_dispatch`，HTTP204，[请求正文](p1-hosted-diagnostic-evidence-20261009/raw/dispatch-body.json) 为分支 `codex/p1-process-boundaries` 和 `p1_process_diagnostic=true`。最终只读列表显示该分支一次 dispatch、attempt1：[api-runs-final.json](p1-hosted-diagnostic-evidence-20261009/raw/api-runs-final.json)。

[Run 37889566976](https://github.com/wbycloud/ui-framework/actions/runs/37889566976)，UTC05:38:48 创建、05:48:20 完成，结论 failure。`regression (osmesa)` job113687183225 和 `regression (webview2)` job113687183445 的 prepare/build 成功、Necessary regressions 失败、artifact 上传成功，见 [最终 run](p1-hosted-diagnostic-evidence-20261009/raw/api-run-011.json) 和 [jobs](p1-hosted-diagnostic-evidence-20261009/raw/api-jobs-011.json)。没有触发第二次运行。

每个配置精确选择原来的 `ui_stateful_components_light`、`ui_instance_language_light`。存储 TIMEOUT180、语言 TIMEOUT240，资源、循环、失败断言、原脚本/Tee 管道及 provider 隔离保留。诊断按用例独立观察进程和抓取转储；普通完整矩阵入口仍保留。本地 PowerShell 解析、`git diff --check`、`tools/test-windows-ci.ps1` 通过，见 [parser.log](p1-hosted-diagnostic-evidence-20261009/raw/parser.log)、[ci-checks.log](p1-hosted-diagnostic-evidence-20261009/raw/ci-checks.log)。这次运行属于定位，不是完整 CI 验收。

实际入口如下，两配置分别使用对应的 `build/ci-webview2` 或 `build/ci-osmesa`，语言项使用自己的精确名称和独立目录：

```powershell
./tools/windows-ci.ps1 -Configuration webview2 -Stage prepare
./tools/windows-ci.ps1 -Configuration webview2 -Stage build
./tools/test-windows-ci.ps1
./tools/windows-ci.ps1 -Configuration webview2 -Stage test -P1ProcessDiagnostic

# 原 provider 阶段中的单项观察命令；日志记录解析后的绝对 JUnit 路径。
$env:UI_PROCESS_STALL_SECONDS = '60'
python tests/observe_processes.py build/ci-evidence-webview2/p1/ui_stateful_components_light/processes.jsonl -- ctest --test-dir build/ci-webview2 -R '^ui_stateful_components_light$' --no-tests=error -V --output-junit build/ci-evidence-webview2/p1/ui_stateful_components_light/ctest.xml
```

两目标均为 Windows10.0.20348.0、Session2、AMD64、hosted image20261004.326.1、MSVC19.44.35229、CTest3.31.6；桌面从1024×768调整为1920×1080，有效DPI96。webview2 配置的安装 Runtime 为155.0.4283.45，但选中的两项实际 backend 均为 Light，不能将它们计为真实 Runtime UI 验收。osmesa 配置 Runtime disabled。固定 Lexbor、QuickJS-NG、WebView2 SDK 和 Mesa24.3.4 未改版。实际环境及完整 PE 清单在两份原 [osmesa manifest](p1-hosted-diagnostic-evidence-20261009/raw/artifacts/osmesa/manifest.json)、[webview2 manifest](p1-hosted-diagnostic-evidence-20261009/raw/artifacts/webview2/manifest.json)。

## 3. 四项真实失败和进程边界

| 配置 | 原 CTest | 时间 | 已进入／完成原生序列 | 最后阶段、原生 PID |
| --- | --- | ---: | ---: | --- |
| osmesa | ui_stateful_components_light | 180.025s Timeout | 2／1 | restore-96，6920 |
| osmesa | ui_instance_language_light | 240.025s Timeout | 1／0 | preferences-96，3532 |
| webview2 | ui_stateful_components_light | 180.025s Timeout | 4／3 | restore-144，2108 |
| webview2 | ui_instance_language_light | 240.010s Timeout | 1／0 | preferences-96，5844 |

完整事实在 [boundaries-summary.json](p1-hosted-diagnostic-evidence-20261009/raw/analysis/boundaries-summary.json)，原始 `processes.jsonl`、`processes.log`、JUnit 位于 `raw/artifacts/配置/p1/用例/`。实际计划和结果分别验证精确两项，无 skipped。原存储脚本其余序列、语言后续 restore 等没有执行完，不能宣布功能矩阵通过。

四个最后原生进程分别在观察器 elapsed4407、8234、7813、8375ms 已打印 `process-return`，之前有关闭、状态断言、COM 清理和零失败打印。此后外部查询仍为 alive。停止输出满60秒后，分别在 elapsed64407、68234、67829、68391ms 开始抓取 CTest、pwsh、native 共12份转储，全部 dump_ok，捕获耗时235–609ms。抓取没有主动终止产品。

到原 CTest 门槛才出现最后 native 和 pwsh 的实际 OS exit255、CTest exit8、stdout EOF。已经完成的前序原生进程退出码真实为0：osmesa 的 write96；webview2 的 write96、restore96、write144。`CASE_END` 数量与这些正常边界一致。四个停滞进程均有实际 Dispatch 记录，语言分别373／369，存储32／33。零失败输出没有替代这些边界。

由此排除“最后子进程已正常退出，仅日志 EOF 卡住”以及“仅下一子进程启动卡住”作为这四项的首要原因。父脚本仍等待未退出的 native 属于下游等待；原 COM 返回及 `process-return` 打印均早于真正 OS 退出。选中 Light 进程没有 Chromium 子进程，不用本次结果解释历史 Runtime 异常。

## 4. 匹配线程栈与责任判断

按每份 dump 模块中的 image timestamp/size 和 CodeView GUID/age，使用目标匹配的系统映像、ntdll/kernel32 PDB，以及该固定 Mesa24.3.4 的私有 PDB 解码。20项映像对照（4个 native ×5模块）全部匹配，见 [supplemental-identities](p1-hosted-diagnostic-evidence-20261009/raw/supplemental-identities.json)。全部12份 debugger 实际退出0，见 [dump-stack-summary](p1-hosted-diagnostic-evidence-20261009/raw/analysis/dump-stack-summary.json)。没有开启忽略符号不匹配选项。

四个 native 一致的调用链：

```text
ucrtbase!exit
kernel32!ExitProcessImplementation
ntdll!RtlExitUserProcess → LdrShutdownProcess → LdrpCallInitRoutine
osmesa!dllmain_dispatch → dllmain_crt_process_detach
osmesa!__scrt_dllmain_uninitialize_c → common_exit → _execute_onexit_table
osmesa!destroy_st_manager → llvmpipe_destroy_screen → lp_rast_destroy+0xdc
osmesa!util_semaphore_wait → u_cnd_monotonic_wait
KERNELBASE!SleepConditionVariableCS
ntdll!RtlSleepConditionVariableCS → NtWaitForAlertByThreadId
```

四份完整匹配栈为 [osmesa 存储](p1-hosted-diagnostic-evidence-20261009/raw/analysis/osmesa-ui_stateful_components_light-6920-matched-stack.log)、[osmesa 语言](p1-hosted-diagnostic-evidence-20261009/raw/analysis/osmesa-ui_instance_language_light-3532-matched-stack.log)、[webview2 存储](p1-hosted-diagnostic-evidence-20261009/raw/analysis/webview2-ui_stateful_components_light-2108-matched-stack.log)、[webview2 语言](p1-hosted-diagnostic-evidence-20261009/raw/analysis/webview2-ui_instance_language_light-5844-matched-stack.log)。`lp_rast_destroy` 帧中的等待循环 i=0、`exit_code=0x103`（STILL_ACTIVE259）一致。转储只列主线程，不能仅据此推断具体线程句柄已经 signaled。

只读 Mesa 源码显示：`osmesa.c` 的全局 manager 在 `atexit` 注册 destroy，manager 释放 screen；`lp_screen.c` 释放 rasterizer；`lp_rast.c` 设置 exit_flag、唤醒工作线程，然后 Windows 分支在 `GetExitCodeThread == STILL_ACTIVE` 时等待每个 task 的 `exited` 信号。原字节副本在 `raw/dependency-source/`，哈希见 supplemental-identities。Windows 的进程退出涉及终止其他线程及 DLL_PROCESS_DETACH，返回 main 也会进入该路径；因此此处不能依赖线程继续运行用户退出代码。这与已取得的实际栈一致，见 [Microsoft ExitProcess 合同](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-exitprocess)。

还追回模块保留路径：Mesa 的 `src/c11/impl/threads_win32.c` 用 `_beginthreadex` 启动线程；匹配二进制中的 `create_thread_parameter` 调用 `GetModuleHandleExW`，flags=4（FROM_ADDRESS），不含 UNCHANGED_REFCOUNT；`common_end_thread` 正常释放路径调用 `FreeLibraryAndExitThread`。见 [创建反汇编](p1-hosted-diagnostic-evidence-20261009/raw/analysis/osmesa-crt-module-pin-disassembly.log)、[释放反汇编](p1-hosted-diagnostic-evidence-20261009/raw/analysis/osmesa-crt-module-release-disassembly.log)、[源身份](p1-hosted-diagnostic-evidence-20261009/raw/thread-retention-identities.json)。该调用会增加模块引用，见 [Microsoft GetModuleHandleExW 合同](https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulehandleexw)。这里是普通持有引用，不是使用 PIN flag 永久固定 DLL。

框架现有 `ui_surface_destroy → ui_platform_surface_destroy` 已有 OSMesaDestroyContext 和对应 FreeLibrary；完整示例 fixture 的 unmount/destroy 也走此路径，已保存同 commit 源码。因此“框架有 FreeLibrary，Mesa 应必然完全卸载”这一推断不成立，Mesa 工作线程的 CRT 模块引用可以保留 DLL。当前 dump 未保存完整活跃引用/任务内存，不能声称已经逐个量化所有引用或排除其他持有者。直接责任定位在固定 Mesa 的进程终止清理等待；尚未证实新的语言/存储/公开销毁接口缺陷，不能通过多调 FreeLibrary 盲目修补。

## 5. 修复假设的反证和下一最小实验

在本次新 build 目录建立独立 Win32 DLL，DLL 内工作线程运行，main 返回，DllMain 仅记录线程退出查询和零超时等待，不执行任何框架/Mesa 调用、不阻塞等待。MSVC19.50 的本地 Windows10.0.26300 实际 OS exit0，输出：

```text
MAIN_RETURN pid3272
DETACH code=00000103 wait=00000102 ok=1
```

`wait=0x102` 为 WAIT_TIMEOUT。[源码、构建、结果](p1-hosted-diagnostic-evidence-20261009/raw/exit-guard/result.json) 和 result.log 均保留。这是进程终止时线程查询的机制反例，不能当 Mesa 或 Windows20348 目标验收；它说明仅将 `GetExitCodeThread` 换成 `WaitForSingleObject(thread,0)` 并据其等待结果继续等 task 信号，仍不能被宣布为修复。没有提交这个猜测补丁。

下一步最小实验应在独立 provider 调用方中建立正常 context 创建/销毁及 DLL 引用账本，区分普通 FreeLibrary 卸载与进程终止 detach；若调整 provider 退出策略，需要同时验证普通清理、重复 context、资源和正常 CRT/OS 退出。进程终止阶段不等待已终止工作线程是待审方向，尚未实现/构建为实际 Mesa 修复。固定依赖、本地历史 provider DLL 和冻结 SDK 未替换，也没有在框架层调用 TerminateProcess、重复 FreeLibrary、关闭功能或提高预算。

本次授权只包含一次目标诊断。新的 provider/框架修复必须先取得上述本地红绿因果证据和可审查补丁；再次推送或目标 CI 复验须另行明确授权。已有四次异常与本地机制反例支持定位，不能代替未插桩的修复验收。

## 6. 构建、下载、符号与画面身份

两构建源码为上述干净 commit、sourceDirty=false。本地之前的 MSVC19.50 DLL 不冒充托管19.44产物；目标 DLL/宿主/应用包和调用方在原 manifest 中分别列 SHA256。核心 DLL 为 osmesa `145624f293e7083219107f70283b9a52812a0769459ec1a9c11a55344515ce16`、webview2 `b50f1d033c50b20940205c8636887ca6e05057fe70a4a78a2252ba2ab9851eb2`。目标公共头以 commit blob 绑定，原 artifact 没有上传全部 PE/导入库；未封存或冒称新配套 SDK。缺少 native EXE 映像会限制最后 CRT 启动帧的深入解码，已匹配的系统/Mesa 阻塞链完整保留。

| 原 artifact | GitHub ID | zip 字节 | SHA256 | 解包文件数 |
| --- | ---: | ---: | --- | ---: |
| windows-osmesa-65f678c98dad601833035761f009479018d23079-1 | 11597702201 | 934406 | e4959e3b30fe67a56d9047829bf9a3a7411f7452cdbc05535805a88a432b1e1c | 97 |
| windows-webview2-65f678c98dad601833035761f009479018d23079-1 | 11598461286 | 1166363 | c7ec9f6a636ad966f074731e81264e4a522a5722f396edd58b05f4b5ddabb219 | 105 |

下载 zip 与 GitHub API 的 digest、字节数一致，见 [artifact 元数据](p1-hosted-diagnostic-evidence-20261009/raw/api-artifacts-001.json)；两份 raw zip 和202份解包材料逐字节复制并记录 copy-ledger，不改写日志、manifest、dump 或 BMP。

Mesa DLL 本机原固定文件与目标均 SHA256 `c633b820bba8ec0dcb05466505e3c246e4c4028eaec7caf1054e707c8eb3ca1f`。私有符号来自原固定分发的 [Mesa24.3.4 debug-info release](https://github.com/pal1000/mesa-dist-win/releases/tag/24.3.4)，使用81段 HTTP206/Content-Range/长度逐项核验后组装84354035字节7z，SHA256 `60b2ccf9d81646aab53ce50bb1117c292e314049799b58f620d6838d134b48b1`。这是本机计算的 hash，release API 没有发布 digest。只解出 x64/osmesa.pdb，67080192字节、SHA256 `42659470e93b8305d514a956e830d1455318b326b2d2829e61426ca265967ee7`；目标 PDB key `A53E732B22534FEE8FEE53F47771D8EA1` 正常加载私有符号。

部分初次网络读取、系统 PDB 和大归档下载失败/部分文件仍留在新 build，不当作有效符号或归档使用。KERNELBASE/ucrtbase 的 PDB 不完整，最终只采用匹配系统映像及导出/展开信息；ntdll、kernel32、Mesa PDB 完整。初期无匹配映像的猜测符号日志不作为最终函数归因。有效大型归档、PDB、映像和失败下载均保留在 ignored `build/p1-hosted-diagnostic-20261009`，哈希/来源/解码日志进入证据；clone 不会携带这些 build 文件，也没有将其安装为新依赖。一次附加 debugger 读取退出0后，终端尝试显示原编码文本失败；原日志已先写入，后续只读提取使用原文件，没有改变调试结果。

原 artifact 的15张 BMP 全部保留，另派生15张原尺寸 PNG，逐张解码RGB一致，见 [images.json](p1-hosted-diagnostic-evidence-20261009/raw/images.json)。这些是原用例阶段窗口捕获，不是连续鼠标输入视频或新的完整视觉矩阵；本次没有重复 P2–P6 桌面输入。之前本地连续画面及失败帧保持原记录，不将托管截图替代硬件鼠标、实际英文Windows、IME或物理DPI验收。

## 7. 其余问题和停止边界

P2 程序192泛化叠层、P3历史窄表头残影：前轮独立公共序列及同尺寸像素对照未复现，原失败图/具体操作仍不足；页脚计数红绿修复独立成立，不解释两项原症状。P4历史持续重绘：有原超时/1983paint直接日志，但原失效闭环仍未追回；当前原操作链有界不能称根治。P5历史Runtime：创建、导航、资源、pending关闭分别追踪，前轮合法边界与fast64通过仅支持对应路径，剩余历史异常未归因。P6只读Home/End：前轮同Runtime原生textarea38组及无监听22组对照解释了默认语义/错误起点期待，没有新增产品键盘补丁。详细证据沿用原本地报告，本次没有重新执行或抬高这些结论。

当前源码Session0、严格整机无登录、真实英文Windows、中文IME、物理混合DPI、桌面合成专项、长期人工、硬件鼠标/键盘及外部捕获抢占继续分别待验；本次托管Session2和程序DPI不替代它们。不注销用户、不修改服务、不降低门槛、不扩大硬件GL。业务应用、PERF-001、历史SDK和证据不写入。本次授权动作完成；P1为“当前目标故障已定位，安全修复及修复复验未完成”。

## 8. 历史只读保全和最终工作区

授权执行后的新只读核验完成，见 [preservation-after-remote.json](p1-hosted-diagnostic-evidence-20261009/raw/preservation-after-remote.json)。各组可有相同文件，unique实际读取11259份；下表不将组计数相加冒称独立总数。

| 已封存组 | 校验份数 | 字节 | 缺失／变化 |
| --- | ---: | ---: | ---: |
| 原历史 SDK、437 CI、既有证据等 preservation 清单 | 7830 | 28017059122 | 0 |
| 前轮根因调查 evidence 索引 | 2313 | 745307719 | 0 |
| 前轮原始 BMP | 1003 | 14476829874 | 0 |
| 前轮最终构建二进制 | 95 | 73124565 | 0 |
| 前轮根因报告及其索引自身 | 2 | 490269 | 0 |
| 本次授权16路径工作区原字节 | 16 | 399966 | 0 |

这包括封存44c039f SDK、143份语言证据/18张PNG、原CI、前轮失败和BMP；没有重跑旧收集/打包程序或改写封存报告。新增证据索引自身排除自身哈希，报告独立列hash；`raw/copy-ledger.json`保留来源和精确复制关系。

最终分支/HEAD仍为codex/p1-process-boundaries/65f678c98dad601833035761f009479018d23079，tracked工作区无改动，main仍82a774a。未跟踪的原五个本地报告/目录仍保留，新增本报告和一个证据目录；没有自动提交或推送这些材料，也未重写已推送提交。本次助手启动的观察、下载、debugger和哈希任务均结束，不终止用户其他进程。
