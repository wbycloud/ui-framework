# P1 Mesa24.3.4：进程终止生命周期修补及 A/B/C 对照

2026-10-09。记录者：Codex 本地诊断。状态：候选 provider 已构建并验证；未提交、推送、发布或执行新的托管工作流。

本轮在不加载框架或业务代码的调用方中复现了原固定 DLL 的退出等待，同版本未修改源码的重建也复现。候选修补保留默认多线程，只在 Windows 整个进程终止时停止不安全的全局 screen 清理。候选的正常上下文销毁、loader lock 外 screen 清理、真正 DLL 卸载及自然 OS 退出分别取得证据。默认配置在本机 A/B/C 有限次数均未复现停滞；不能据此宣称目标托管默认配置已修复。

准确的等待对象已经取得，但转储中旧查询值259与后续同对象查询0之间的时序尚未捕获。候选针对已确认的进程终止边界，不把这段未知时序解释成已经证明的检查—等待竞态。原语言用例首轮有一次144 DPI状态断言失败，单独复核通过，原失败保留且仍未定位。

## 1. 本轮基线与保护范围

实际 HEAD：`65f678c98dad601833035761f009479018d23079`，分支 `codex/p1-process-boundaries`。开始时 tracked diff 为空，已有10个未跟踪路径／文件，见 [baseline.json](p1-mesa-lifecycle-evidence-20261009/raw/raw/baseline.json)。本轮仅修改诊断观察脚本及已有未跟踪 OSMesa 调用方，新增 Mesa 两文件补丁和本报告／证据。框架产品源、公共头、应用源、CMake及实际工作流未修改。SDK0.9.0-dev／API9／开发标准9、ABI1／包格式1不变。

新目录为 `build/p1-mesa-lifecycle-20261009/` 和 `docs/validation/p1-mesa-lifecycle-evidence-20261009/`。供应方固定依赖不覆盖，业务应用、PERF-001和冻结 SDK未写入。MSVC链接器不能正确处理 LLVM响应文件中的中文绝对路径，另建 `D:\ui-p1-mesa-lifecycle-20261009` junction 指向本轮新目录；没有移动或删除历史文件。

本机重新核对为 Windows10.0.26300、Session1、24逻辑CPU、2560×1440桌面、系统 DPI96、用户／系统UI语言0x804、父环境 `LP_NUM_THREADS` 未设置，见 [final-environment.json](p1-mesa-lifecycle-evidence-20261009/raw/raw/final-environment.json)。应用输入在解锁 Default 桌面串行执行；未进行硬件鼠标人工验收。

两个旧 P1 build 的1783份文件与本轮开始快照逐字节一致，4个封存基线文件也一致，见 [prior-build-preservation-final.json](p1-mesa-lifecycle-evidence-20261009/raw/raw/prior-build-preservation-final.json)。另核验语言143份／18 PNG、旧托管证据376份、旧线程对照740份和最终 SDK，共1260个索引项，全部匹配，见 [sealed-history-audit.json](p1-mesa-lifecycle-evidence-20261009/raw/raw/sealed-history-audit.json)。两组存在重叠，不能相加为独立文件总数。本轮未重新声称上一轮11637份保护集合的全部文件均完成一次新扫描。

最终44c039f SDK仍为117891276字节，SHA256 `28ca0b8409b321477f1aa3adce2680fda8cf2cd8a9a35dd4308a5f08ef1f79a2`。旧构建、原CI材料、原失败／转储／BMP、证据索引均未重收集或重封装。

## 2. 历史线索与本次责任边界

[上一轮线程对照报告](p1-mesa-thread-control-validation-20261009.md) 已核对 Bug76252、PyVista action和 Mesa24.3.4实际线程配置，本轮沿用其封存原始资料，不把历史测试计为本轮测试。

[Mesa10.3.4发布说明](https://docs.mesa3d.org/relnotes/10.3.4.html) 的 Bug76252修复对应 `706ad3b649e6a75fdac9dc9acc3caa9e6067b853`：正常DLL卸载期间，loader lock下等待线程完整退出可能死锁，改用工作线程在退出前发出的 `exited` semaphore。24.3.4已经有此逻辑；本轮没有重新引入无限 join。原Bug站点完整讨论未取得，不能声称核对了所有历史情形。

[PyVista action c103a2f](https://github.com/pyvista/setup-headless-display-action/commit/c103a2ff45650d38cb71684b5dc6cdfeb9442c79) 将 Windows offscreen配置的 `LP_NUM_THREADS=0` 写入 job环境。它改变并发与CRT模块引用，不能等同原多线程路径修补。本轮没有执行0配置，也没有把它写入框架、宿主或CI正式配置。

24.3.4 `lp_screen.c` 默认以逻辑CPU数创建 rasterizer／compute工作者，限制32，`LP_NUM_THREADS` 覆盖该值；1配置仍有1+1工作者。普通 `OSMesaDestroyContext` 不释放全局 manager／screen。独立调用方仅加载指定的 OSMesa DLL和系统依赖即可复现，所以等待责任位于 Mesa／其静态CRT退出生命周期，业务和框架代码不是最小复现的必要条件。

[Windows ExitProcess合同](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-exitprocess) 规定其他线程终止并成为signaled后进入进程级DLL detach；[DllMain合同](https://learn.microsoft.com/en-us/windows/win32/dlls/dllmain) 用非空 `lpvReserved` 区分进程终止。此时不能依赖已终止工作者继续执行用户代码发出清理信号。[DLL最佳实践](https://learn.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-best-practices) 也限制loader lock下的同步操作。本轮证明了本候选MSVC入口顺序，没有认定Windows或CRT实现存在已确认缺陷。

## 3. 首次失败的等待对象与实际退出

首次搜索在运行前固定1配置上限24、默认上限8，各配置遇首次失败停止；独立诊断期限45秒，约10秒停滞时先抓全内存／句柄／线程信息，之后才处理停滞子进程。环境只传给子进程，初始化前生效，父环境前后相同。

这组仍使用上一轮审阅后的同一调用方 PE（SHA256 `f5d96ea10a26a006180f5559c2c6fabda2ec99faec02236dbe9353569d9e46ce`），与后续增加私有cleanup入口的新调用方分账：1配置7进程中6自然0、1停滞；默认8/8自然0。[全部命令、PID、期限及 OS 结果](p1-mesa-lifecycle-evidence-20261009/raw/raw/fixed-full-search.json) 完整保留。

首次失败 PID46204，无预先断点、stdin gate或线程句柄观察。`CONTEXT_DESTROY_END` 和 `MAIN_RETURN failures=0` 后没有自然退出；独立45秒期限终止后OS退出1，不能计为通过。

| 对象／状态 | 实测值 |
| --- | --- |
| rast | `0x1f28be22da0`，num_threads1，exit_flag1 |
| task0 | `0x1f28be22db8`，task_index0 |
| Mesa保存的原线程HANDLE | `0x19c` |
| work_ready | `0x1f28be22e58`，counter1 |
| work_done | `0x1f28be22e90`，counter120 |
| exited／实际等待mutex | `0x1f28be22ec8`，counter0 |
| DuplicateHandle原保存HANDLE | 成功，error0；没有按日志TID重新OpenThread |
| 对复制的同一线程对象GetThreadId | 47020，error0；匹配创建日志与DMP句柄对象 |
| 停滞后GetExitCodeThread | 成功，code0，error0 |
| 同对象零时长等待 | WAIT_OBJECT_0=0，error0 |
| 主线程 | TID22560，区别于上述工作线程 |
| DLL detach | reason0，reserved `0x1`，整个进程终止 |

见 [首次capture.json](p1-mesa-lifecycle-evidence-20261009/raw/runs/fixed-full-1-06/first-stall-full/capture.json)、[匹配栈](p1-mesa-lifecycle-evidence-20261009/raw/runs/fixed-full-1-06/first-stall-full/matched-stack.log) 和 [正确frame0x12解码](p1-mesa-lifecycle-evidence-20261009/raw/debug/full-detach-frame18.log)。完整堆DMP为92357722字节，仍在新build，路径／hash见 [local-artifacts.json](p1-mesa-lifecycle-evidence-20261009/local-artifacts.json)。原第一次抓取不覆盖。

匹配私有PDB及实际机器码表明，`lp_rast_destroy+0xdc` 的等待分支先要求 `GetExitCodeThread` 成功并返回259，frame中保存 `exit_code=0x103`、i0。这个旧保存值与停滞后外部查询不是同时观察，不能由其差异直接证明TOCTOU，也不能把259解释为现场仍存在可继续执行的工作线程。[GetExitCodeThread合同](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getexitcodethread) 与真实线程对象等待必须一起解释。

## 4. 六项假设、前置清理和实际二进制

| 假设 | 本轮证据及剩余边界 |
| --- | --- |
| 正确对象已signaled，但查询／保存值为259 | 停滞后同一对象已signaled且code0；frame旧值259。原查询瞬间的对象状态与转变时刻尚未捕获。 |
| 句柄错误、关闭或复用 | 首次失败的原保存HANDLE对应正确rasterizer工作者。独立事件轨迹记录创建和compute join/CloseHandle，关闭的是不同compute句柄；尚无同一个失败进程从创建到失败查询的完整原子轨迹。 |
| 工作者终止，没有执行exited信号代码 | 失败为reserved非空的进程终止，DMP仅主线程，exited0、work_ready1、exit_flag1，符合已终止工作者无法响应后来清理唤醒；是本轮确认的不安全清理边界。 |
| 信号被消耗或重复销毁 | 自然退出事件轨迹manager一次，源码atexit一次注册；首次失败处为i0、counter0。未见消费／重复证据，但不把未记录的信号操作声明为严格排除。 |
| 等待与记录不是同一task/semaphore | PDB类型offset、rast/task地址、真实wait寄存器和exited mutex一致，此次现场已排除错误对象记录。 |
| 正常卸载中的loader lock收尾 | 三个生命周期分别测试。失败现场为整个进程终止，不能混为reserved空的正常卸载；loader lock外清理后真正卸载通过，不证明所有未测试加载方式均安全。 |

`llvmpipe_destroy_screen` 的实际顺序是compute pool先于rasterizer。事件观察器记录 `thrd_create` 的保存HANDLE、CRT模块增引用、compute `thrd_join`／CloseHandle、rast destroy及DllMain／CRT／manager入口。主搜索未预置断点；这些后续断点观察全部标为 `timing_changed=true`，不能用观察后的通过推翻无预先干预失败。见 [lifecycle-traces.json](p1-mesa-lifecycle-evidence-20261009/raw/raw/lifecycle-traces.json) 及 [mapped-provider-traces.json](p1-mesa-lifecycle-evidence-20261009/raw/raw/mapped-provider-traces.json)。

原A事件观察1配置最多6次、遇观察错误停止，实际4次；另默认1次，目标均自然OS0。第4次观察器退出3，记录SuspendThread error5，目标退出0。B/C正常cleanup断点轨迹也有线程结束时观察错误，目标均自然0；观察器失败不能隐藏或算目标产品失败。无预置断点的B/C私有cleanup另外各一进程正常0。

补取仅在查询返回／等待入口设断点的A1配置6次，全部OS0，查询成功code0，未捕获259等待。见 [late-query-runs.json](p1-mesa-lifecycle-evidence-20261009/raw/raw/late-query-runs.json)。该结果受调度影响，原查询疑点保持未确认，没有无限重跑。

固定A和B/C都用匹配RSDS GUID／age1私有PDB检查真实机器码。A为 `a53e732b-2253-4fee-8fee-53f47771d8ea`；B为 `8df6ab05-f57a-4b7e-85c0-151c70c7b1ad`；C为 `2f352030-3f29-4fe7-a7c1-b1ed70c195f3`。A原等待偏移+0xdc，B+0xcc，不能跨产物套固定RVA。

PE TLS callback数组在A/B/C均为空，`__dyn_tls_init_callback` 为NULL；源码中的 `__threads_win32_tls_callback` 没有链接进本批DLL，不能仅凭源码推断其执行。CRT创建的模块普通增引用与正常 `FreeLibraryAndExitThread` 仍存在。实际 `dllmain_dispatch` 在process detach先调用用户DllMain再执行CRT onexit；`_pRawDllMain` 的detach调用位置更晚，未作为标志入口。见 [PE元数据](p1-mesa-lifecycle-evidence-20261009/raw/raw/provider-pe-metadata.json)、[解码命令](p1-mesa-lifecycle-evidence-20261009/raw/raw/provider-decodes.json) 和 `raw/debug/` 原反汇编。

## 5. 最小补丁及适用范围

[mesa-osmesa-process-detach.patch](../../tools/mesa-osmesa-process-detach.patch) SHA256为 `5a7fed431c5444c7de9100523026dc91642db9c9603a450289ee141a5ac8bba9`，仅改 Mesa 两个源文件：

1. `src/gallium/targets/osmesa/osmesa_target.c`：Windows用户DllMain在 `DLL_PROCESS_DETACH && reserved != NULL` 时写入内部进程终止标志；不分配、不加锁、不等待工作者。
2. `src/gallium/frontends/osmesa/osmesa.c`：注册的 `destroy_st_manager` 仅在该标志为真时返回，由OS回收整个进程资源；正常运行和reserved空的动态卸载保留原清理。

候选C真实轨迹为：process detach reason0/reserved1 → 用户DllMain → 内部termination notice → CRT detach terminating1 → manager flag1 → 无compute/rast清理入口 → 自然OS退出0及EOF。不是仅把最后semaphore等待改成join，也没有把阻塞推到compute前置清理。见 [mapped-C-exit/stdout.log](p1-mesa-lifecycle-evidence-20261009/raw/runs/mapped-C-exit/stdout.log)。

适用范围是本轮已验证入口顺序的 Windows x64／MSVC／静态CRT OSMesa DLL。没有验证MinGW、静态OSMesa链接、WGL、硬件GL或其他LLVM使用方，不外推到所有Mesa构建。不新增框架/provider公共生命周期接口，内部notification不在DLL导出表中。

诊断调用方新增 `cleanup manager-RVA global-RVA`：仅用匹配PDB定位的私有函数，在loader lock外调用manager清理并把singleton置NULL，防止注册atexit对已释放对象再次销毁。该入口不是应用公共能力，也没有加入默认产品或CTest。正常私有cleanup轨迹manager函数入口会出现两次，第二次为NULL对象的atexit空清理；screen本身只释放一次。

## 6. A/B/C源码、工具链与准确产物

官方 `mesa-24.3.4.tar.xz` SHA256 `e641ae27191d387599219694560d221b7feaa91c900bcec46bf444218ed66025` 与 [官方24.3.4发布说明](https://docs.mesa3d.org/relnotes/24.3.4.html) 一致。新LLVM为 llvmorg-19.1.7、commit `cd708029e0b2869e80abe31ddb175f7c35361f90`，本地新建static／MT／RTTI／X86 Release。B/C同为MSVC19.50.35727、Meson1.6.1、Mako1.3.8、Ninja1.12.1、Bison3.8.2／Flex2.6.4，Python3.11新venv，OSMesa+llvmpipe+softpipe，LLVM/static glapi，`b_vscrt=mt`、Release／Zi／DEBUG。完整实际命令和初期失败均在 `raw/recipe/`、`raw/tools/`。

供应方固定配方 commit为 `d82e34ed4ffbe6f07998ea1f27dd9315f7b098a7`，VS17.12.4／MSVC19.42、LLVM19.1.7。新构建复用版本／CRT／相关选项，但使用当前MSVC，未加供应方S3TC补丁，关闭zlib/zstd及不需要的驱动，MarkupSafe3.0.4而非3.0.2；不能称为供应方二进制逐字节重建。B/C同条件，只差最小补丁，B确实复现，因此不是仅更换构建条件后C偶然通过。

10640份源文件逐项hash比对，B/C恰有上述2个差异；排除了Meson运行产生的16份 `__pycache__` 字节码。初次把生成文件算作源码的校验失败已纠正，原构建失败日志未删。[identity-results.json](p1-mesa-lifecycle-evidence-20261009/raw/raw/identity-results.json) 含每个差异与产品身份。

| Provider | DLL字节数 | DLL SHA256 |
| --- | --- | --- |
| A 固定供应方，.deps原文件 | 52327936 | `c633b820bba8ec0dcb05466505e3c246e4c4028eaec7caf1054e707c8eb3ca1f` |
| B 同版本未修改源码重建 | 51686912 | `3bd1280a2d72052c3ef673428dde75c8d3a07ab0ca905c048ea0c054d6169159` |
| C 同条件最小修补 | 51686912 | `99d8142434c3abfacce969aa3731220314d99c53ee0afc546896c0afad92f5f5` |

PDB：A67080192字节，SHA256 `42659470e93b8305d514a956e830d1455318b326b2d2829e61426ca265967ee7`；B68710400字节，`41abb9787bf910960f06978b208d065949b4021b055033389747f84e3c646517`；C68718592字节，`c5c15c5cc6673b795d21fa721d87a6abb382005417129685903937e744f73bbd`。

A有353导出，B/C各370且逐项一致，保留全部A导出；新增17个既有内部 `_glapi_*` 导出属于构建配置差异，不是框架新公共能力。DLL入口及TLS比较见上述PE元数据。

后续A/B/C共用一个新调用方 `osmesa_lifecycle_probe.exe`：686592字节，SHA256 `0892a7bacb44c736efa91a637e7192627ac79a4063e9ba1ffc7d51d10f9da55b`。原绘制／像素检查未改变，只增加上述私有cleanup能力。源码SHA256 `5ce8d040f9beda0e79dd184194def1eeab8304c7200a6719d55e5a1a9b321f88`。

框架为本轮新C11／Ninja／Release／WebView2 enabled构建，仅构建两项相关测试目标，不重跑全部历史矩阵。A/C用例共用以下二进制，没有切provider时重建或重新打包：

| 文件 | SHA256 |
| --- | --- |
| ui_framework.dll | `9a918da44bae1f4127b9cd02bcac25cb8584cb97cb6d25aaf44086a334ab15c0` |
| ui_stateful_components_test.exe | `813cd8aacd0344604c11c50b6a807f0251747227969190f05846d03cfedbd6a8` |
| ui_instance_language_test.exe | `3ea346d7b65fb54d0ed462c6d2e6f023c52cf6ac16625388f3568656412c1735` |
| stateful_components_app.dll | `daee2542f79642de300b62c0098ac15f840ee22ec3c9a363fd024541d78ba7d0` |
| stateful_components.uapp | `f5bb7367259f06c074640cb31b82e868788fe64b5717140cb11281f7a92ad715` |

## 7. 三种生命周期、绘制、线程及资源结果

共用新调用方的有界主矩阵：[provider-lifecycle-runs.json](p1-mesa-lifecycle-evidence-20261009/raw/raw/provider-lifecycle-runs.json)。1配置最多12，A/B遇首次失败停止；默认各8，无预置断点／gate的退出验收与后续活线程gate分开。

| Provider | 1配置独立退出 | 默认独立退出 | 默认8次上下文循环 | 释放调用方引用 | 活线程gate诊断 |
| --- | --- | --- | --- | --- | --- |
| A | 3次：2自然0，1停滞 | 8/8自然0 | 1进程自然0 | 1进程自然0，模块保留 | 1进程自然0，24+24工作者 |
| B | 3次：2自然0，1停滞 | 8/8自然0 | 1进程自然0 | 1进程自然0，模块保留 | 1进程自然0，24+24工作者 |
| C | 12/12自然0 | 8/8自然0 | 1进程自然0 | 1进程自然0，模块保留 | 1进程自然0，24+24工作者 |

该矩阵51进程中49自然OS0，2停滞后诊断期限终止OS1。A失败PID44324，B失败PID41072，各自首个全内存DMP及匹配PDB解码保存。B也在 `lp_rast_destroy` 的同一Windows semaphore等待分支，保存i0／259；不是只复现一般超时。每项 `run.json` 记录实际命令、PID、provider/caller SHA、期限、EOF和自然／强制边界。

默认活线程gate的PDB匹配栈实测每组24个 `thread_function`、24个 `lp_cs_tpool_worker`；另有主线程和Windows工作者，未统算为Mesa工作者。gate用于计数，改变调度，不归入无干预退出样本。

512×512 RGBA、120帧×64重叠三角形，中心／背景／alpha／GL error原断言通过，pass0全缓冲FNV64均 `38b57cea08bb9bf9`；8循环每轮累计hash序列跨A/B/C完全一致。每轮context-destroyed为handles166、GDI0、USER1，暖态计数无增长。不是以接口成功或单个像素代替整个缓冲比较。

普通上下文销毁后全局screen与工作者仍存活；A/B/C调用方自有引用2→1→0后模块仍映射，原因是工作者的CRT普通模块增引用。此处只记“自有引用释放”，不称完整卸载。

另用匹配私有RVA在loader lock外清理screen后再释放最后引用，A、B、C各一无预置断点进程自然OS0；B/C额外入口轨迹目标也自然0。compute24和rast24按原清理退出，最后DllMain reason0/reserved0、CRT terminating0、manager flag0／NULL对象空清理、模块UNLOAD且GetModuleHandle为NULL，证明真正卸载。见 [private-A-default](p1-mesa-lifecycle-evidence-20261009/raw/runs/private-A-default/run.json)、[mapped-C-cleanup](p1-mesa-lifecycle-evidence-20261009/raw/runs/mapped-C-cleanup/stdout.log)。

真正卸载后的handles99，不恢复历史0配置冷态75；Windows `lp_rast_destroy` 对原rasterizer句柄未CloseHandle的既有差额仍存在。本次未扩大为普通卸载句柄整改，不能声明所有冷态句柄泄漏已消除。普通运行的原资源门槛均保留。

## 8. 性能与实际框架负载

5轮交错A/B/C顺序、同一绘制负载，共15自然OS0进程，测量时无构建、UI输入、gate或dump并发。见 [performance.json](p1-mesa-lifecycle-evidence-20261009/raw/raw/performance.json)。

| Provider | 绘制区间中位数 |
| --- | --- |
| A | 212.159ms |
| B | 209.239ms |
| C | 205.137ms |

C/B为0.9804，C/A为0.9669，未观察到本负载的明显补丁成本，差异在有限本机采样范围内，不宣称加速或所有应用性能无风险。默认线程数未减少，没有利用0配置改变并发换取通过。

原Light两项仅替换已有 `-Provider` 参数，另建CTest计划，不改原脚本／断言／循环／180与240秒超时。真实输入串行，原资源预算和完整状态检查保留，逐项检查Dispatch、CASE_END、原生OS退出、CTest退出、JUnit和EOF。

| Provider／轮次 | 原用例 | 超时 | 总耗时 | 原生退出／完整结果 |
| --- | --- | --- | --- | --- |
| A | ui_stateful_components_light | 180s | 42.803s | 26/26自然0，26 CASE_END，通过 |
| A | ui_instance_language_light | 240s | 71.397s | 7/7自然0，7 CASE_END，通过 |
| C | ui_stateful_components_light | 180s | 40.383s | 26/26自然0，26 CASE_END，通过 |
| C首轮 | ui_instance_language_light | 240s | 50.419s | 5进程，4自然0、1自然1；CTest失败 |
| C单独复核 | ui_instance_language_light | 240s | 71.921s | 7/7自然0，7 CASE_END，通过 |

结果／命令见 [original-A/results](p1-mesa-lifecycle-evidence-20261009/raw/runs/original-A/results.json)、[original-C/results](p1-mesa-lifecycle-evidence-20261009/raw/runs/original-C/results.json) 和 [单独复核](p1-mesa-lifecycle-evidence-20261009/raw/runs/original-C-language-recheck/results.json)。共71个原生进程，70自然0、1自然1；不能把失败轮删掉只报“全部通过”。

首轮C的restore144有4项语言／宿主状态断言失败，focused tab2但仍中文状态，是真实功能失败，不是进程退出等待；自然返回1，没有卡在Mesa清理。该轮仅UI真实输入串行，但有无窗口独立render探针并行。单独复核未改源、时序、超时或断言且无并行render，全部通过。根因未定位，不能归因于render竞争或宣称已修复。

四张新PNG来自本轮真实HWND BMP，转码后逐像素一致，原BMP保留。来源／尺寸／SHA见 [frame-origins.json](p1-mesa-lifecycle-evidence-20261009/frame-origins.json)：[storage96](p1-mesa-lifecycle-evidence-20261009/frames/candidate-storage-96.png)、[language96](p1-mesa-lifecycle-evidence-20261009/frames/candidate-language-96.png)、[language192](p1-mesa-lifecycle-evidence-20261009/frames/candidate-language-192.png)、[首次失败144](p1-mesa-lifecycle-evidence-20261009/frames/candidate-first-failure-144.png)。程序192 DPI不作为物理DPI验收，静态窗口画面不冒称硬件鼠标连续人工验收。

## 9. 实际命令、诊断修改及本地候选交付

下列为本轮已执行命令摘录，封存目录不应原地重跑。完整参数／环境／PID位于对应ledger；后续需另建目录。

```powershell
cmd /d /c build/p1-mesa-lifecycle-20261009/tools/build-providers.cmd
cmd /d /c build/p1-mesa-lifecycle-20261009/tools/build-product.cmd
python build/p1-mesa-lifecycle-20261009/tools/provider_validation.py
python build/p1-mesa-lifecycle-20261009/tools/run-original-case.py A .deps/mesa-24.3.4/x64/osmesa.dll
python build/p1-mesa-lifecycle-20261009/tools/run-original-case.py C build/p1-mesa-lifecycle-20261009/providers/C/build-ascii-llvm/src/gallium/targets/osmesa/osmesa.dll
python build/p1-mesa-lifecycle-20261009/tools/run-original-case.py C-language-recheck build/p1-mesa-lifecycle-20261009/providers/C/build-ascii-llvm/src/gallium/targets/osmesa/osmesa.dll --test ui_instance_language_light
```

运行时仍由Python仅在诊断子进程环境清除默认LP变量或写入诊断1，并显式选择llvmpipe；不会改变当前shell、GITHUB_ENV或产品配置。失败子进程的诊断器终止仅处理独立deadline，补丁没有TerminateProcess、额外sleep、join或预算提升。

仓库修改：`tests/observe_processes.py` 给进程句柄打开／退出查询增加具体错误码；`tests/process_stall_capture.py` 在显式 `UI_PROCESS_FULL_DUMP=1` 时使用0x1826，原默认0x1004不变；`tests/osmesa_thread_probe.c` 增加私有cleanup诊断；`tools/mesa-osmesa-process-detach.patch` 为两文件provider补丁。两个Python AST检查通过，更新的进程观察链实际目标／父进程自然0、EOF、open/read error0，见 [updated-observer-processes.jsonl](p1-mesa-lifecycle-evidence-20261009/raw/raw/updated-observer-processes.jsonl)。未来CI调用方的 `/W4 /WX /utf-8 /std:c11 /O2 /Z7 /MT` 构建也实际通过。

本地候选归档是 `build/p1-mesa-lifecycle-20261009/mesa24.3.4-osmesa-lifecycle-candidate-65f678c-99d8142-x64.zip`，38073334字节，SHA256 `afb11d7bf599e991f8e2b7df6971813f8bb6bca66f91f85c8f20b7b5cf59c8ae`。包含精确C DLL/PDB、manifest、补丁、构建配方及许可证，标记local diagnostic candidate，不是新SDK／稳定发布。CRC、DLL SHA和全新解包后默认退出、私有cleanup真正卸载实际OS0均验证，见 [candidate-archive.json](p1-mesa-lifecycle-evidence-20261009/raw/raw/candidate-archive.json) 和 [解包验证](p1-mesa-lifecycle-evidence-20261009/raw/raw/candidate-unpacked-results.json)。原固定DLL仍为A，候选只能通过已有显式provider路径选择。

## 10. 新目标CI提案与待补证据

[新CI提案README](p1-mesa-lifecycle-evidence-20261009/ci-proposal/README.md)、[未应用proposal.patch](p1-mesa-lifecycle-evidence-20261009/ci-proposal/proposal.patch) 已准备。实际 `.github/workflows/windows-regression.yml` 未修改，旧封存提案未改。新提案固定A/C，默认线程，不执行0配置。保留两个原托管job、25分钟预算、原Light180/240秒和资源／功能门槛，A失败不阻断C取证但总结果仍失败。

每job只运行原A／C两项Light各一次，另有默认独立退出与单独活线程gate各一进程／provider。同框架、应用包和调用方二进制；已有Provider路径切换，不在目标重建Mesa。保存完整case／进程退出／EOF、首个full dump、正确64位system images和A/C匹配私有PDB。实际CI的WGL副本仅在临时build移入证据目录隔离，不删除。

提案的Python／YAML及4种Actions表达式替换后的PowerShell解析通过，`git apply --check`通过，现有CI策略检查通过；没有执行目标workflow，下载／转储／托管输入仍需目标验证。候选archive的获准传递URL尚未建立，不创建发布或标签绕过授权。

下一最小目标是再次明确授权后，由代理传递精确sealed candidate，在原失败托管环境执行该A/C方案。目标需要补取：首个默认配置真实停滞的rast/task／原HANDLE／GetExitCodeThread成功与错误码／同对象零等待／detach reserved、创建到关闭的顺序及原查询值变化；正常进程完整CASE_END、自然OS退出和EOF。静态DMP能确定停滞对象，不能自动补齐早期原查询的原子时序；若原A目标未复现，报告未复现，不把候选通过当作因果证明。

历史原Light存储180秒、语言240秒托管超时尚不能宣称解决。本轮在独立1配置重现同一Mesa等待支持生命周期责任，仍需要目标默认配置新对照。144 DPI功能瞬态、已知正常rast句柄余量和其他历史显示／Runtime问题不借本补丁宣称根治。

## 11. 独立待验条件

| 独立项目 | 本轮边界／待验条件 |
| --- | --- |
| 当前源码／候选Session0 | 未使用服务runner复验；历史Session0结果不移作当前候选结果。 |
| 严格整机无登录 | 当前Session1，不具备严格无登录条件；不注销用户、修改服务或降低门槛。 |
| 真实英文Windows | 实测用户／系统语言0x804；中文系统的受控映射不能替代。 |
| 中文IME | 本轮没有真实中文IME输入专项；普通自动键盘输入不能替代。 |
| 物理混合DPI | 系统DPI96；程序96/144/192矩阵不能替代实际跨屏物理验收。 |
| 桌面合成 | 本轮未进行专门合成环境复验，不以窗口截图宣称历史显示问题根治。 |
| 长期人工验收 | 有限自动输入／退出样本，不含长期运行或硬件鼠标人工验收。 |

未扩大到硬件GL；本轮程序DPI、语言映射和自动SendInput不代替上述条件。

有限次数C通过、已验证MSVC入口的termination guard、正常清理保留，是本轮可审查的本地候选结论；259旧值形成时序和目标默认配置闭环仍需独立证据。
