# P1 Mesa24.3.4：默认多线程生命周期修补的托管 A/C 复验

2026-10-09。记录者：Codex。状态：授权的一次双 job 诊断已完成；候选通过本次目标退出链验证，尚未替换默认依赖、纳入 SDK 或合入 main。

原 A 在两个托管 job 中重现三次退出停滞，全部匹配 `lp_rast_destroy+0xdc → util_semaphore_wait`。候选 C 保留默认 rasterizer／compute 并发，两 job 的四个原 Light 用例组全部通过，66 个原生测试进程自然 OS exit0、完整 CASE_END 和 stdout EOF。工作流总体为 failure，因为原 A 的三个超时仍按原门槛记失败；没有隐藏失败来取得绿色结果。

本次建立了“目标默认 A 失败、默认 C 正常退出”的对照。它支持修补已确认的 Mesa 进程终止清理边界，不证明偶发问题永久消失，也没有解释原查询／保存值259到后来同对象查询0的精确时序。

## 1. 授权、基线及产物身份

用户授权开始上一轮提出的下一步：必要诊断提交和推送、传递封存候选，以及一次手动双 job A/C 托管验证。接手 HEAD 为 `65f678c98dad601833035761f009479018d23079`，分支 `codex/p1-process-boundaries`。已有两项 tracked 诊断修改和未跟踪报告／调用方／补丁见 [baseline.json](p1-mesa-hosted-control-evidence-20261009/raw/raw/baseline.json)，没有覆盖或混入其他未跟踪工作。

本次构建及测试源码为 [5715a0539b94158e3f18afc32bffbc9a5d6a0168](https://github.com/wbycloud/ui-framework/commit/5715a0539b94158e3f18afc32bffbc9a5d6a0168)。八个提交路径见 [diagnostic-commit.json](p1-mesa-hosted-control-evidence-20261009/raw/raw/diagnostic-commit.json)。框架产品源、公共头、业务应用、CMake和原用例断言未改。SDK0.9.0-dev／API9／开发标准9，ABI1／包格式1不变。后续纯报告提交不能替代此测试身份。

候选 provider 使用上一轮已经封存的 C，没有为本次重新构建 Mesa。A/C SHA 分别为：

| Provider | DLL SHA256 | 匹配 PDB GUID／age |
| --- | --- | --- |
| A 固定供应方24.3.4 | `c633b820bba8ec0dcb05466505e3c246e4c4028eaec7caf1054e707c8eb3ca1f` | `a53e732b-2253-4fee-8fee-53f47771d8ea`／1 |
| C 两文件生命周期修补 | `99d8142434c3abfacce969aa3731220314d99c53ee0afc546896c0afad92f5f5` | `2f352030-3f29-4fe7-a7c1-b1ed70c195f3`／1 |

C 归档为38,073,334字节，SHA256 `afb11d7bf599e991f8e2b7df6971813f8bb6bca66f91f85c8f20b7b5cf59c8ae`。经授权在独立 orphan 诊断分支 `codex/p1-lifecycle-candidate-20261009` 传递，commit为 `136682258b37c0a11647f80be352fd8756deafea`。远端 ZIP 分段重组后校验完整 SHA；目标也核验该 SHA及 A/C PDB SHA。见 [传递身份](p1-mesa-hosted-control-evidence-20261009/raw/raw/candidate-transfer.json)、[归档 manifest](p1-mesa-hosted-control-evidence-20261009/raw/raw/transfer-manifest.json) 和 [远端完整读取证明](p1-mesa-hosted-control-evidence-20261009/raw/raw/candidate-remote-verification.json)。未创建 release、标签或 PR，未改变 main。

上一轮 B 为同条件未修改源码重建，SHA `3bd1280a2d72052c3ef673428dde75c8d3a07ab0ca905c048ea0c054d6169159`，本机也复现同一等待。B/C仅两份源码不同；C为Mesa24.3.4、LLVM19.1.7、MSVC19.50／MT／x64。A供应方工具链不同，本次目标 A/C 不是同工具链的 B/C 实验；B未在本次目标 job 重跑。只读历史身份、配方差异及本地结果见 [封存报告原字节副本](p1-mesa-hosted-control-evidence-20261009/raw/prior-local/report.md) 和 [identity-results.json](p1-mesa-hosted-control-evidence-20261009/raw/prior-local/identity-results.json)，不能将它们计为本次新测试。

## 2. 一次工作流及原用例边界

[Run37917825238](https://github.com/wbycloud/ui-framework/actions/runs/37917825238)，`workflow_dispatch`，UTC10:28:35创建，10:40:38完成；head为5715a053。只有一次 dispatch，没有 rerun。OSMesa job `113778229284`、WebView2 job `113778228867`，prepare和build成功，A/C diagnostic总结果failure，artifact上传成功。最终 [run元数据](p1-mesa-hosted-control-evidence-20261009/raw/raw/api-run-006.json)、[job元数据](p1-mesa-hosted-control-evidence-20261009/raw/raw/api-jobs-006.json) 和 [dispatch记录](p1-mesa-hosted-control-evidence-20261009/raw/raw/dispatch-at.json) 保留。

两 job 均为 Windows Server20348、4逻辑CPU、MSVC19.44.35229构建框架与调用方。桌面由1024×768调整至原托管测试要求的1920×1080，系统DPI96。运行器检查可打开且为Default输入桌面；没有取得新的严格无登录或Session0验收。WebView2 job prepare安装Runtime155.0.4283.45，但本次选取的用例全部为 Light，不能把 job 名称计为真实Runtime交互覆盖。

原 CTest 测试计划、命令、脚本、循环、功能及资源断言保留，仅用已有 `-Provider` 参数选择 DLL。每 job 内A/C共用同一框架、测试EXE、应用DLL／包及独立调用方，切换后身份再次核验。两 job 分别构建，彼此 PE／包不要求字节相同。原计划与实际参数逐项比较通过，存储TIMEOUT180秒、语言240秒、job25分钟未变。输入在各 job 的解锁桌面串行执行；独立 gate探针在应用输入之前结束。

| Job | Provider／用例 | 实际秒数 | 原生进程／CASE_END | CTest／结论 |
| --- | --- | ---: | --- | --- |
| osmesa | A storage | 40.6666 | 26自然0／26完整 | 0，通过；该组本次未复现 |
| osmesa | A language | 240.031 | 6自然0，1超时终止255／6完整 | 8，Timeout |
| osmesa | C storage | 39.6686 | 26自然0／26完整 | 0，通过 |
| osmesa | C language | 72.2777 | 7自然0／7完整 | 0，通过 |
| webview2 | A storage | 180.036 | 1自然0，1超时终止255／1完整 | 8，Timeout |
| webview2 | A language | 240.022 | 1超时终止255／0完整 | 8，Timeout |
| webview2 | C storage | 36.6729 | 26自然0／26完整 | 0，通过 |
| webview2 | C language | 68.0208 | 7自然0／7完整 | 0，通过 |

原A的失败位置：webview2 storage `restore-96` PID4312、language `preferences-96` PID2676；osmesa language `restore-192` PID5480。三者都已记录process-return及零功能失败，却缺少相应CASE_END和自然OS退出，最后由原CTest期限处理。255是超时后的OS结果，不是自然返回255。

C四组实际Dispatch最大记录分别为90／736／90／739；完整读取每个原生进程的OS句柄退出、完整CASE_END、JUnit、父CTest退出和stdout EOF。没有以SendInput返回、process-return或日志尾部failures0替代验收。独立后处理摘要见 [osmesa-summary](p1-mesa-hosted-control-evidence-20261009/raw/analysis/osmesa-summary.json)、[webview2-summary](p1-mesa-hosted-control-evidence-20261009/raw/analysis/webview2-summary.json)；原始运行结果和逐PID事件保存在 `raw/artifacts/{job}/lifecycle-control/`。

每 provider／job另有一个默认独立退出进程及一个标明改变时序的活线程gate进程，共8个，全部自然OS0及EOF。没有把4个gate样本作为无干预退出样本。独立探针期限45秒仅用于诊断，不改原用例超时。本次未运行LP0或1应用矩阵。

## 3. 三个匹配失败现场及剩余时序疑点

全内存dump首先保存，随后才读取活对象并DuplicateHandle；标记 `timing_changed=true`。dump／现场观察在已静默60秒的目标上发生，不是运行前植入断点。使用Mesa实际保存的原HANDLE，未按日志TID重新OpenThread。

| Job／PID | rast | task0／实际exited等待对象 | 原HANDLE／工作线程TID | 主线程TID |
| --- | --- | --- | --- | --- |
| osmesa／5480 | `0x16b83c6d6c0` | `0x16b83c6d6d8`／`0x16b83c6d7e8` | `0x22c`／3560 | 7136 |
| webview2／2676 | `0x2081fa79270` | `0x2081fa79288`／`0x2081fa79398` | `0x308`／5156 | 732 |
| webview2／4312 | `0x128c28c7cf0` | `0x128c28c7d08`／`0x128c28c7e18` | `0x280`／4700 | 2112 |

三份现场共同实测：num_threads4、exit_flag1，四个task_index均对应自身索引，四个exited counter均0、work_ready均1。12个原HANDLE复制／GetThreadId／GetExitCodeThread均成功，错误码0；退出码均0、同对象零等待均WAIT_OBJECT_0=0。dump句柄信息的TID与活对象查询逐项一致，原HANDLE没有指向仍在等待的主线程。

匹配PDB frame4的 `sema` 正是上述task0 exited地址，frame5为 `lp_rast_destroy+0xdc`，等待循环i0，保存exit_code0x103；frame0x12为 `dllmain_dispatch` reason0／reserved1。只剩主线程，完整链为：

```text
ExitProcess / RtlExitUserProcess / LdrShutdownProcess
  Mesa dllmain_dispatch / dllmain_crt_process_detach / CRT onexit
    destroy_st_manager / llvmpipe_destroy_screen
      lp_rast_destroy+0xdc / util_semaphore_wait
        SleepConditionVariableCS
```

三份匹配日志：[5480](p1-mesa-hosted-control-evidence-20261009/raw/analysis/osmesa-A-5480-matched-v2.log)、[2676](p1-mesa-hosted-control-evidence-20261009/raw/analysis/webview2-A-2676-matched-v2.log)、[4312](p1-mesa-hosted-control-evidence-20261009/raw/analysis/webview2-A-4312-matched-v2.log)。[独立对象／模块复核](p1-mesa-hosted-control-evidence-20261009/raw/analysis/hosted-object-checks.json) 验证7份Mesa目标dump及39个PE映像的size／timestamp／checksum、Mesa RSDS GUID／age和真实栈线程分类。Mesa及独立调用方私有PDB匹配；Windows内部帧部分只有导出符号，不能按最近导出名扩大归因。框架／应用测试PE没有随目标artifact单独留存，仅记录SHA，不能声称其全部映像也完成同级匹配。

六项假设的当前判断：

| 假设 | 已确认或仍缺的证据 |
| --- | --- |
| 同对象已signaled但旧查询值259 | 旧frame259与停滞后真实查询0均实测；原查询返回瞬间到后续状态的时序仍未捕获。 |
| 句柄错误／指向主线程／复用 | 三个现场均对应正确rasterizer对象；没有完整创建至原查询的无干预HANDLE轨迹，不能严格排除更早的瞬态。 |
| 工作者已终止，不能发exited | reserved非空的整个进程终止、仅余主线程、12对象均退出、exited0及同一wait支持该不安全清理边界。 |
| 信号消耗或重复销毁 | 此次未取得信号操作完整历史；没有新增支持证据，不能凭counter0严格排除。 |
| 实际wait与记录task不一致 | 三次PDB变量、寄存器、地址、counter和HANDLE对应，现场已排除。 |
| 普通DLL卸载loader lock收尾 | 三次为reserved1进程终止；不能混为reserved0普通卸载。普通清理沿用分别验证的历史本地证据。 |

责任界限是Mesa／其静态CRT注册的全局screen退出清理进入了不安全的进程终止阶段。此前不加载框架或业务代码的同DLL复现仍是独立依据，不能据此认定框架language/storage销毁API存在新缺陷，也没有确认Windows或CRT自身实现错误。

## 4. 最小生命周期修补及正常清理

[补丁](../../tools/mesa-osmesa-process-detach.patch) SHA256 `5a7fed431c5444c7de9100523026dc91642db9c9603a450289ee141a5ac8bba9`，仅改Mesa两文件：Windows OSMesa用户DllMain在process detach且reserved非空时设置内部终止标志；`destroy_st_manager` 只在该标志成立时停止全局screen清理，由OS回收整个进程资源。DllMain不分配、不锁定、不等待，没有TerminateProcess、额外sleep、无限join或反复FreeLibrary。

本候选实际MSVC入口机器码／上一轮轨迹已核对用户DllMain先于CRT onexit，C目标仍为同一DLL字节；因此标志在manager之前生效。不能假定其他编译器／构建也具有同样顺序。[此前C退出轨迹](p1-mesa-hosted-control-evidence-20261009/raw/prior-local/mapped-C-exit-stdout.log) 为reserved1→标志→CRT→manager flag1→无compute/rast等待→自然OS0；该轨迹明确标为历史、调试器改变时序，本次目标自然退出与其分账。

普通上下文销毁、reserved空的正常FreeLibrary和运行时清理不设置该标志，保留原逻辑。上一轮在loader lock外用匹配私有RVA清理screen后，A/B/C均真正模块UNLOAD、GetModuleHandle为NULL、自然OS0，C正常清理轨迹见 [mapped-C-cleanup](p1-mesa-hosted-control-evidence-20261009/raw/prior-local/mapped-C-cleanup-stdout.log)。此私有驱动仅为诊断，不是新增公共能力。本次目标没有重做普通DLL完全卸载专项，不能冒称新托管卸载结果。

历史Bug76252的正常卸载join修复已经在24.3.4中；本补丁不恢复join。PyVista的LP_NUM_THREADS=0改变并发，不是本次正式方案，也未写入框架或GITHUB_ENV。适用范围为已验证入口顺序的Windows x64／MSVC／静态CRT OSMesa DLL。未覆盖MinGW、静态OSMesa链接、WGL、硬件GL或其他LLVM使用方，不扩大为所有Mesa构建的通用修复。

正常卸载后历史24个rasterizer原句柄余量仍未整改；本次没有宣称所有冷态资源泄漏消失。上一轮C首次restore144的功能失败仍保留，后续单独复核及本次目标通过不能解释或根治该瞬态。

## 5. 默认线程、绘制、资源及性能边界

四份活线程gate dump均实际4个 `thread_function`、4个 `lp_cs_tpool_worker`、主线程1和其他Windows线程3，未将后者算作Mesa工作者；本负载utility工作者0。A/C原HANDLE在gate时均查询259／零等待258，exit_flag0。没有由环境清除成功推断线程存在，也没有通过LP0减少线程。

8个独立调用方均完成512×512 RGBA、120帧×64三角形，中心／背景／alpha／GL error原断言和完整缓冲FNV64一致：`38b57cea08bb9bf9`。单次上下文销毁前后返回正常；C日志handles121、GDI0、USER1。该目标单次计数不当作长期泄漏验收，暖态循环资源及正常卸载沿用独立历史本地记录。

| Job | A默认无gate绘制ms | C默认无gate绘制ms | 样本边界 |
| --- | ---: | ---: | --- |
| osmesa | 695.122 | 691.958 | 各1，无统计性能结论 |
| webview2 | 502.778 | 496.142 | 各1，无统计性能结论 |

四个gate测量另记，不混入无干预性能样本。此前本机5轮交错A/B/C中位数212.159／209.239／205.137ms，见 [封存performance原字节副本](p1-mesa-hosted-control-evidence-20261009/raw/prior-local/performance.json)。有限采样未见明显绘制成本；不能宣称加速或所有负载零成本。本次C实际框架storage约37–40秒、language约68–72秒，保持原预算和功能／资源断言；原A超时总耗时不作为绘制性能比较。

六张完整真实窗口PNG见 [frame-origins](p1-mesa-hosted-control-evidence-20261009/frame-origins.json)，从本次目标C原BMP无损转换且逐像素一致，全部检查过实际画面，保留可见GL、表格、字段及英文标签。原135份BMP保持原字节。这是程序96／192 DPI静态HWND画面及原自动交互结果，不作为物理混合DPI、连续硬件鼠标、历史192叠层或窄表头残影根治验收。

## 6. 命令、二进制及证据保存

目标实际命令入口如下，两job配置分别为osmesa／webview2，完整绝对参数见各 `raw/results.json` 和原CTest计划：

```powershell
./tools/windows-ci.ps1 -Configuration osmesa -Stage prepare
./tools/windows-ci.ps1 -Configuration osmesa -Stage build
python tests/run_osmesa_lifecycle_control.py --build build/ci-osmesa --original .deps/mesa-24.3.4/x64/osmesa.dll --candidate build/ci-evidence-osmesa/lifecycle-candidate/unpacked/osmesa.dll --evidence build/ci-evidence-osmesa/lifecycle-control
# 每项原用例经 observe_processes.py 包裹CTest；仅Provider路径变化
ctest --test-dir build/ci-evidence-osmesa/lifecycle-control/runs/original-C -R '^ui_instance_language_light$' --no-tests=error -V --output-junit build/ci-evidence-osmesa/lifecycle-control/runs/original-C/ui_instance_language_light/ctest.xml
```

本机前置AST／PowerShell四种表达式解析／原CI策略通过；活对象观察器分别用A/C实际24个工作线程的gate验证原HANDLE复制、259／258和自然OS0。前置结果见 [local-preflight](p1-mesa-hosted-control-evidence-20261009/raw/raw/local-preflight.json)。目标独立C11调用方以 `/W4 /WX /utf-8 /std:c11 /O2 /Z7 /MT` 构建通过，源自5715a053，目标完整CRLF源文件SHA `bf5ecbb18ba8902e511d3b55261d6ff0b9f096598b3abb008a321b527c15a8e3`。Git LF对象SHA `c7e269584ece47af0eb2a042d87b4d9619e65ccb7295bcd8ccf24d71fa0670cc`；本地133行CRLF／24行LF的副本SHA `5ce8d040f9beda0e79dd184194def1eeab8304c7200a6719d55e5a1a9b321f88`。规范化后内容完全一致，见 [source-newlines.json](p1-mesa-hosted-control-evidence-20261009/raw/analysis/source-newlines.json)，没有把换行身份差异当代码变化。

| 二进制 | osmesa job SHA256 | webview2 job SHA256 |
| --- | --- | --- |
| 独立调用方 | `8f1bea06efa99d2e89e92dfb4b199e281ac850af1f3bfcaaddf9c96e71c5cbb2` | `9ddde29046b3c131c60e8f40330c407c0e6e8f1ab5f446b08e28e07ac3e4bc82` |
| ui_framework.dll | `805b014899af4690b8a47e0f765d3365eb012ebcc4fb0010f483957c3e1ab096` | `9560717767b36ef8af59c3f12d9d4c1372e4281f955aeb1d5788d0d11c33b2b3` |
| storage test EXE | `a61f107171c738f4650ccd4ab1c4279fbc42c5240d5dc4ea8d7c8cd7445c4aed` | `1e81498f91ef3c109369456d190763054141737376c848c215c2b5abd2e45e5e` |
| language test EXE | `075fce28a5500ed259a4ffa592473d9f8246fd296829db781aaf0ceccafafe57` | `49b369f6cccba07c90c817aad9513871bf9a4169ec0a251f7a41440f62594ced` |
| stateful_components.uapp | `cc8dbcca2853d08fb525afd3df1860b9691817c9c0428ce1d368b43fa0307d88` | `57ad5d712e627af264ab549ac87d736e6a4cd1e6f0b7738cdb9508b3bb2a5dd4` |

固定Lexbor／QuickJS commit和Mesa版本未变。新证据目录为 `docs/validation/p1-mesa-hosted-control-evidence-20261009/`，新build为 `build/p1-mesa-hosted-control-20261009/`。取回了776份artifact文件，包括13份原DMP、135份BMP、匹配Mesa PDB、独立调用方和目标64位system images。两个完整ZIP已匹配API digest，提取逐文件SHA并验CRC；首次传输失败、分段元数据、初次deferred符号汇总误判日志均保留，不覆写原失败。签名下载地址／凭据不纳入公开证据。

| Artifact | 字节数 | SHA256 |
| --- | ---: | --- |
| osmesa／11610564176 | 469580301 | `6327c1ef443e2ad62b0ed693ad819bbb1d82b58026c0bf08059a6a2999a2c6bb` |
| webview2／11610653776 | 637913104 | `1b8e54b2c6f4040e2ee2b3378ee112cbe7aacd20a3f277178b42ebeaf7f8b323` |

Git证据保存日志、元数据、逐PID事件、JUnit、活对象和匹配分析及6 PNG；大ZIP／DMP／BMP／PDB／PE仍在本机新build，[local-artifacts.json](p1-mesa-hosted-control-evidence-20261009/local-artifacts.json) 列184项身份。clone不会携带这些build产物；GitHub artifact到2026-11-08到期，不能把届时缺失材料重新生成冒充原交付。全量目标提取清单见 `raw/raw/artifact-{job}-files.json`。

诊断源码提交的实际新增范围为：已有进程观察器补错误码、opt-in全内存／准确Mesa对象观察、独立OSMesa公共C调用方及私有cleanup诊断、A/C原用例运行器、两文件provider补丁、手动诊断工作流路径和原字节Git属性。目标测试没有增加sleep、超时或资源预算，没有删失败断言。桌面设置补回原Stage test的前置条件；只隔离本次临时CI build的WGL副本并随artifact保存。

只读保护检查见 [final-history-preservation-normalized.json](p1-mesa-hosted-control-evidence-20261009/raw/raw/final-history-preservation-normalized.json)：语言143／18PNG、原托管376、线程对照740、上一轮生命周期733、两个旧P1 build快照1783及137本地产物等，去重3,914文件全部匹配。首次按路径拼写计数3915的记录也保留，规范路径后更正为3914。最终44c039f SDK仍为117,891,276字节、SHA `28ca0b8409b321477f1aa3adce2680fda8cf2cd8a9a35dd4308a5f08ef1f79a2`。未写业务应用、冻结SDK、历史包／证据或PERF-001。

## 7. 当前结论、独立待验与下一最小验证

本次目标原默认配置复现了此前存储180秒／语言240秒链的Mesa退出等待；C在同目标两job、同框架与应用二进制条件下完成全部相关原用例及自然OS退出。可以将这次已确认的进程终止清理缺陷记为“候选生命周期修补经目标对照通过”，不能宣称原固定A已经修复，也不能将有限C样本写成所有托管问题永久解决。

仍缺原GetExitCodeThread瞬间、259保存到后续0的原子时序，以及同一无干预失败进程从工作者创建／模块增引用到终止查询的完整事件史。现有对象和终止边界已足以支持当前两文件修补，没有为猜测中的HANDLE／信号管理问题追加代码。历史持续重绘、瞬态Runtime、144功能失败、泛化192叠层／窄表头残影保持原结论。

若要纳入默认provider／后续SDK，下一最小验证应先评审本补丁及实际MSVC构建范围；目标补做同构建条件B/C和正常screen清理→真正DLL卸载的分账对照，并用新目录对原180／240用例做预先限定次数的默认C复验。需继续保存首次失败、原资源／功能门槛、自然OS退出和EOF；不得通过LP0、增加预算或隐藏原A失败收敛。此为后续方案，本次没有执行第二次托管运行；新的推送／工作流或默认交付变更须另行明确授权。

| 独立条件 | 当前边界 |
| --- | --- |
| 当前源码Session0 | 本次为logged-in hosted desktop，没有服务runner复验。 |
| 严格整机无登录 | 未取得符合条件的runner；未注销用户、修改服务或降低门槛。 |
| 真实英文Windows | 不将托管默认环境或受控语言映射自动计为要求的空宿主人工验收。 |
| 中文IME | 本次没有真实中文IME专项输入。 |
| 物理混合DPI | 系统96及程序96／144／192不替代物理跨屏条件。 |
| 桌面合成 | 本次窗口截图与退出链验证不证明历史合成显示问题根治。 |
| 长期人工／硬件鼠标 | 有限自动SendInput及退出样本，未进行长期人工或连续硬件鼠标验收。 |

新增公共API为零；应用继续使用现有provider路径。候选、报告、目标证据和上述剩余边界应一起审查。
