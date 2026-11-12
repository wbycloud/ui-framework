# P1 Mesa24.3.4 退出等待：LP_NUM_THREADS 因果对照

2026-10-09。记录者：Codex 自动诊断；不从 Git 作者推断历史测试者。

本轮用不加载框架或业务 DLL 的独立 OSMesa C 调用方，在 **1 线程配置复现了与此前托管 P1 相同的进程终止等待**，保存四份匹配转储。默认配置本机 92 个诊断进程均自然退出 0；0 配置 20 个均自然退出 0。因此尚未建立“本机默认失败、0 配置消除同一失败”的严格对照，目标托管问题也未宣布解决。

0 配置的作用已由实际线程栈和模块引用追踪确认：本负载的 rasterizer、compute 工作线程从默认的 24+24 降为 0+0，对应 CRT 的 Mesa 模块增引用从 48 降为 0；调用方释放最后自身引用后，0 配置确实卸载了 Mesa。它是有实测依据的候选绕行，改变了并发路径。本负载绘制耗时约为默认的 4.99 倍，尚未固化为正式设置。完整原始记录、失败及未应用的 CI 提案见 [新证据目录](p1-mesa-thread-control-evidence-20261009/README.md)。

## 1. 基线、实际产物和保护范围

实际 HEAD 为 `65f678c98dad601833035761f009479018d23079`，分支 `codex/p1-process-boundaries`。接手时 tracked diff 为空，已有七个未跟踪历史报告／证据路径，见 [baseline.json](p1-mesa-thread-control-evidence-20261009/raw/raw/baseline.json)。这些材料未覆盖。新增 `tests/osmesa_thread_probe.c` 为独立手工诊断调用方，没有加入 CTest 或默认构建；产品、公共头、CMake、CI 工作流和原测试脚本均未修改。没有提交、推送、查询本仓库实时远端或触发工作流，没有新增公共 API、SDK、发布或标签。

本机 Windows10.0.26300、Session1、24 个逻辑 CPU、MSVC19.50.35727、Python64，桌面 2560×1440、系统 DPI96，父进程 LP_NUM_THREADS 未设置，见 [local-environment](p1-mesa-thread-control-evidence-20261009/raw/raw/local-environment.json)。这是程序及自动输入验证，不是物理混合 DPI 或硬件鼠标验收。此前失败的 Windows20348／Session2／4 CPU／MSVC19.44 托管环境属于历史证据，见 [此前 P1 报告](p1-hosted-diagnostic-validation-20261009.md)。

所有新构建在 `build/p1-mesa-thread-control-20261009/`。固定 `.deps/mesa-24.3.4/x64/osmesa.dll` 原字节未改变：52,327,936 字节，SHA256 `c633b820bba8ec0dcb05466505e3c246e4c4028eaec7caf1054e707c8eb3ca1f`。使用同一固定 DLL，而非按配置更换 provider。Lexbor、QuickJS、WebView2 SDK 未 prepare、升级或覆盖。SDK0.9.0-dev／API9／开发标准9，应用 ABI1／包格式1 保持原身份。

| 实际文件 | SHA256 |
| --- | --- |
| 初始全部主对照的 osmesa_thread_probe.exe | 655ba57d84699b7e2242653b9170272cc5551d578a49a227d2e471b7b5cc652e |
| 新 framework DLL | f64f4fc32620e682760ac9f14b1b36f1ea7c95bd95f973047be1d17db067e01f |
| 新 storage test EXE | df811125ab5651d572e8b908b72e7685068bdef8a94428eed710f03ded85d168 |
| 新 language test EXE | eb43520f1fd5ecb60560c38a89a14a008feae80e08b1642d096574b4b2ec3141 |
| 新 stateful_components.uapp | 78ac8b267aa8d005fbe881db4ae0e92cb4ba7af7864252d5a3a3992b5784fdf9 |

[final-identities](p1-mesa-thread-control-evidence-20261009/raw/raw/final-identities.json) 还包含 import/static lib、探针 PDB、外部引用调试器及依赖源文件身份。框架产品源自上述 commit 的未改 tracked 源码；新增诊断文件尚未提交，不能把诊断调用方冒称该 commit 已有产品。

在主对照之后，调用方像素日志把三个常量改为实际读到的 G/B/A 字节；此前四通道真实像素断言本已存在。原源码、EXE/PDB 全部保留。修订调用方单独构建 `/W4 /WX` 通过，SHA256 `f5d96ea10a26a006180f5559c2c6fabda2ec99faec02236dbe9353569d9e46ce`，默认／0／1 三项 unload 验证均实际退出 0；此三项与原 126 个对照分开，见 [修订调用方结果](p1-mesa-thread-control-evidence-20261009/raw/raw/reviewed-source-results.json)。

[只读保护核验](p1-mesa-thread-control-evidence-20261009/raw/raw/preservation-final.json) 检查 11,637 个去重文件，全部 hash／字节数匹配：历史基线 7,830、root 证据 2,313、root BMP1,003、root 二进制95、上轮 P1 证据376及封存报告／索引、上轮16个授权源码文件。分组存在路径重叠，不能把分组数相加作为独立文件数。143份语言证据、原 CI 材料和封存 SDK 位于历史保护集合。业务应用、冻结 SDK、PERF-001 未写入；没有执行历史 raw 收集或打包程序。

## 2. Bug76252、PyVista 与当前源码

[Mesa10.3.4 官方发布说明](https://docs.mesa3d.org/relnotes/10.3.4.html) 将 Bug76252 列为动态装载／卸载 opengl32.dll 死锁修复，包含 José Fonseca 的 llvmpipe 清理改动。原提交为 `706ad3b649e6a75fdac9dc9acc3caa9e6067b853`，2014-11-07。其原始提交内容和 patch 从保留 Git SHA 的镜像读取，见 [原提交响应](p1-mesa-thread-control-evidence-20261009/raw/upstream/mesa-original-commit.json)，[原提交镜像](https://github.com/pundiramit/external-mesa3d/commit/706ad3b649e6a75fdac9dc9acc3caa9e6067b853)。Bug 网站及 canonical GitLab 页面访问受阻，未声称读到原 Bug 全部讨论。

该历史修复针对 DllMain 持有 loader lock 时 join 工作线程的问题：改成等待线程即将结束前发出的 semaphore，而非等待完整线程退出。Mesa24.3.4 已有这条修复思路，Windows 使用独立 `task->exited` 信号，还先以 `GetExitCodeThread == STILL_ACTIVE` 检查。不能把 Bug76252 直接列为本次未应用补丁；它讨论的正常 DLL 卸载，与本次 main 返回之后的整个进程终止，是不同边界。

[PyVista action 提交 c103a2f](https://github.com/pyvista/setup-headless-display-action/commit/c103a2ff45650d38cb71684b5dc6cdfeb9442c79)／[PR55](https://github.com/pyvista/setup-headless-display-action/pull/55) 于 2026-09-09 合入，在 Windows 且 `install-mesa3d-offscreen` 开启时写入 `LP_NUM_THREADS=0` 到 GITHUB_ENV。该选项默认 true，固定提交的 action.yml 原字节解码在 [pinned action](p1-mesa-thread-control-evidence-20261009/raw/upstream/pyvista-action-pinned-decoded.yml)，对应 API 元数据在同目录。它是 action job 的环境配置，不是对所有系统／Mesa 版本的已验证修补；其默认 Mesa release 为24.3.0，也不等同本仓库固定24.3.4 DLL。

[PyVista PR9170](https://github.com/pyvista/pyvista/pull/9170) 报告类似 OSMesa 退出等待，外部计数分别为默认37/3000、关闭 shader cache15/2000、LP0 0/3000；这些是上游作者的实践，非本轮 ui-framework 测试。其小图性能观察不能代替本轮较重绘制负载的成本测量。尚未核实可以直接解决本固定 DLL 路径的新上游 Mesa 补丁，未盲目升级依赖。

实际只读 Mesa24.3.4 源码副本在 [dependency source](p1-mesa-thread-control-evidence-20261009/raw/source/dependency/src/gallium/drivers/llvmpipe/lp_screen.c)：`lp_screen.c` 默认采用逻辑 CPU 数，LP_NUM_THREADS 覆盖并限制在32；同一数值同时传给 rasterizer 和 compute pool。`lp_rast.c` 与 `lp_cs_tpool.c` 在0配置直接由调用线程执行相应任务，1配置分别创建1个 rasterizer 和1个 compute 工作者。因此“1线程”不是“完全没有 Mesa 工作线程”。OSMesa 全局 screen 由 manager 的 atexit 清理释放，普通最后上下文销毁并不销毁该全局 screen。环境必须在首次 Mesa 初始化前设置；本轮只由 subprocess 的 env 传给诊断／测试子进程，父环境前后均未改变。

## 3. 最小调用方、真实线程及 DLL 引用

调用方只动态加载 OSMesa DLL，创建 GL3.3 compatibility 512×512 RGBA／depth24 上下文；不加载框架、应用包或业务代码。每个上下文绘制120帧，每帧64个重叠三角形，并 glFinish；检查实际中心红色、背景黑色、alpha255、GL error0 和完整缓冲 hash。三配置 pass0 均为 `38b57cea08bb9bf9`。`exit` 销毁上下文但保留调用方 DLL 引用进入进程终止；`unload` 销毁上下文后释放自身最后引用；`cycle` 做8次上下文创建／销毁后释放引用。记录 load ledger、实际模块是否仍映射、线程 ID／入口、句柄/GDI/USER、销毁前后和 main 返回边界。

首次三配置 live 转储在完成绘制的受控 stdin gate 抓取，不是失败转储。固定 DLL 私有 PDB 正常匹配加载，Mesa PDB key `A53E732B22534FEE8FEE53F47771D8EA1`，SHA256 `42659470e93b8305d514a956e830d1455318b326b2d2829e61426ca265967ee7`。转储模块与正确64位 system images／调用方 PE 的60项检查全部匹配。Windows 内部帧部分仅有导出符号；不能将最近导出名当作完整内部函数归因。Mesa 等待路径和调用方使用匹配私有 PDB。

| 配置 | rasterizer | compute pool | 主线程 | 其他线程 | CRT成功增加Mesa模块引用 |
| --- | ---: | ---: | ---: | ---: | ---: |
| 默认 | 24 | 24 | 1 | 3 | 48 |
| 0 | 0 | 0 | 1 | 3 | 0 |
| 1 | 1 | 1 | 1 | 3 | 2 |

线程计数来自 [匹配栈分类](p1-mesa-thread-control-evidence-20261009/raw/raw/live-thread-classification.json) 和 [三份 matched logs](p1-mesa-thread-control-evidence-20261009/raw/debug/default-live-matched-stacks.log)，不是环境设置成功或 ThreadDescription 名称。此 DLL 的名称接口返回空；`named_raster=0` 无法表示没有 rasterizer。未观察到 utility queue 工作者只限定本负载，不证明其他 shader/cache/LLVM 工作负载均没有后台线程。

模块引用另以新 native x64 外部调试器，在固定 DLL 已匹配的 `_beginthreadex` CRT `GetModuleHandleExW` 返回点及正常 `FreeLibraryAndExitThread` 释放点做断点／单步追踪。日志含实际成功返回、非空 Mesa handle、入口函数和最后 DEBUG_EVENT 的 OS exit0，见 [native-reference-results](p1-mesa-thread-control-evidence-20261009/raw/raw/native-reference-results.json)。默认48、1配置2、0配置0次成功增引用；三个进程终止之前都未观察到正常 CRT_RELEASE。这里测量的是该 CRT 路径，非 Windows 总引用计数；外部断点改变调度，三次引用观察与普通运行分开，不算正式无干预通过。磁盘 DLL 未改。

普通 FreeLibrary 返回后，默认／1的 owned_loads 为0但模块仍映射、工作线程仍在；0配置的模块为NULL。不能把默认／1的调用方引用释放成功报告为 DLL 已完全卸载。增加 FreeLibrary 次数会消耗不属于调用方的线程引用，不能作为修复。

## 4. 本机停滞与实际 OS 退出

[126个进程汇总](p1-mesa-thread-control-evidence-20261009/raw/raw/local-contrast-summary.json) 包括最初6个 gate 边界对照、3个8周期进程、15个交错顺序性能进程、24个默认退出搜索、限定追加搜索以及独立线程状态观察。追加搜索预先限定默认最多60、0最多12、1最多12，并在每配置两次失败后停止该配置；状态观察最多20、首失败停止。次数是诊断搜索计数，不能当作随机样本估计故障率。

| 配置 | 进程数 | 自然 OS exit0 | 停滞后诊断终止 |
| --- | ---: | ---: | ---: |
| 默认 | 92 | 92 | 0 |
| 0 | 20 | 20 | 0 |
| 1 | 14 | 10 | 4 |

其中1配置的4个失败是 PID26588、11564、31988、42900。前3个没有 gate／断点／预先线程句柄观察，后1个为独立外部线程状态诊断。均在 main 返回打印零失败后停滞，静默满10秒保存转储；45秒独立诊断期限才终止目标，实际 OS exit1 来自诊断器 TerminateProcess，**不是正常返回1或正常清理完成**。45秒只属于 standalone diagnostic，没有改变原 CTest180／240。

[四份匹配栈摘要](p1-mesa-thread-control-evidence-20261009/raw/raw/stall-stack-summary.json) 与完整栈、局部变量均已保留：进程终止进入 `LdrShutdownProcess`，Mesa CRT DLL_PROCESS_DETACH／onexit 走 `destroy_st_manager → llvmpipe_destroy_screen → lp_rast_destroy+0xdc → util_semaphore_wait → SleepConditionVariableCS`；四份都只剩主线程，等待循环 i=0，保存的 exit_code 为0x103。源码在 GetExitCodeThread 检查之后等待 rasterizer task 的 exited 信号，已经进入进程退出的工作线程无法保证继续执行发信号代码。这与 [Microsoft ExitProcess 合同](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-exitprocess) 及此前四份托管 dump 一致。

外部线程状态观察提前尝试保留6个线程 handle，成功5个；在停滞时主线程仍 alive，另4个已 signaled／exit0，其中一个为 Mesa CRT 入口。另一个 Mesa 线程未成功保留 handle，未记录其 OpenThread 错误，不能补造状态，见 [thread-state-at-stall](p1-mesa-thread-control-evidence-20261009/raw/runs/thread-state-search-01/thread-state-at-stall.json)。该结果不能把检查时0x103和10秒后状态等同，也尚未逐一对应等待 task 的原 handle。检查到等待之间的具体竞态及完整 semaphore/task 内存仍缺证据。

责任判断：同 DLL、匹配等待点、无框架／业务代码的复现确认了 Mesa 的进程终止清理路径责任，1线程仍可触发。没有由此确认新的框架 language/storage/public destroy 缺陷。默认本机未复现；0配置删除了本负载相应线程、引用和等待循环条件，但不能以成功退出宣称原多线程清理已修复。此前目标 CI 的默认4个失败仍是历史事实；本轮没有目标 CI 默认／0实测。

## 5. 绘制、资源、性能与正常卸载边界

三配置绘制结果相同，像素实际断言及完整 hash 通过。8周期 warm context-destroyed 后，默认 handles166、0配置118、1配置120；每配置8次稳定，GDI0／USER1。0配置实际卸载后 handles75、模块NULL；这不等于所有系统 DLL/cache 回到冷启动基线，也不是所有 compute/shader 工作负载的泄漏证明。原应用资源门槛另外由原用例保留并执行。

5次均衡顺序、无 gate／转储的相同 draw workload：

| 配置 | glFinish在内的120帧绘制中位数 | 相对默认 |
| --- | ---: | ---: |
| 默认 | 170.706ms | 1.00 |
| 0 | 852.111ms | 4.99 |
| 1 | 861.347ms | 5.05 |

1配置中一轮在绘制完成后退出停滞，表中的测量仅为已经完成的 draw 区间，非该进程总耗时。负载是512×512、64重叠三角形×120帧，不外推到所有应用或声称0永远慢5倍。PyVista 小图表现与这里不同。禁用并行 rasterizer/compute 的成本必须按应用负载继续衡量。

| 边界 | 已验证 | 未覆盖／不能声称 |
| --- | --- | --- |
| 普通上下文销毁 | 三配置单次及8周期 destroy 返回，实际像素与warm资源稳定 | 无 global screen 销毁接口，不等于manager/rasterizer已销毁 |
| 调用方正常DLL释放 | 自有2→1→0 ledger；0配置真正卸载且自然exit0 | 默认／1只释放自有引用，仍有CRT线程引用，未验多线程DLL完整卸载 |
| 整个进程终止 | 真实外部OS状态；1配置4个同栈失败，0配置20个自然exit0 | 默认本机92个未复现，不能解除历史目标CI失败 |

## 6. 原存储180／语言240用例：本轮局部验证

新的 Release／Ninja／C11 framework build、WebView2 enabled、固定 OSMesa 共246步骤成功，仅构建原两个测试目标；见 [configure](p1-mesa-thread-control-evidence-20261009/raw/raw/product-configure.log)、[build](p1-mesa-thread-control-evidence-20261009/raw/raw/product-build.log)。在解锁 Default 桌面串行执行，实际 foreground／hit／Dispatch 和原最终状态断言均由原测试检查。两项选择 backend Light，不算真实 Runtime UI 验收。

| 配置 | 原用例 | observer总耗时 | 原生子进程实际exit0／CASE_END | 原TIMEOUT |
| --- | --- | ---: | ---: | ---: |
| 默认 | ui_stateful_components_light | 37.281s | 26／26 | 180s |
| 0 | ui_stateful_components_light | 36.485s | 26／26 | 180s |
| 默认 | ui_instance_language_light | 66.375s | 7／7 | 240s |
| 0 | ui_instance_language_light | 66.265s | 7／7 | 240s |

[完整边界复核](p1-mesa-thread-control-evidence-20261009/raw/raw/original-boundaries-verified.json) 验证精确单项 JUnit、所有native实际 OS exit0、CASE_END、process-return、父CTest退出0与stdout EOF；四项共66个原生进程。存储最大Dispatch96，语言默认821／0配置817。资源、功能、循环与失败断言未删；没有增加sleep、放宽timeout或预算。原脚本、新manifest及日志在 [original-product-evidence](p1-mesa-thread-control-evidence-20261009/raw/original-product-evidence/)。

完整原BMP86份在新build保存原字节，[bmp-originals](p1-mesa-thread-control-evidence-20261009/raw/bmp-originals.json) 列出hash。选出默认／0的saved及restored-chinese各一张，共4张完整1200×800真实窗口PNG，编码前后像素逐项相等并已由Codex读取画面核对软件GL三角形和文字；来源及运行归属见 [frame-origins](p1-mesa-thread-control-evidence-20261009/raw/frame-origins.json)。这4张不是新的连续拖动画面，也不承担之前全部UI问题的验收。

本轮命令原文保留在 diagnostic-sources：`build-product.cmd`、`build-probe-repaired.cmd`、`run_probe.py`、`run_controls_resume.py`、`run_followup.py`、`run_probe_query.py`、`run_original.py`。单项原入口为：

```powershell
# 由 run_original.py 的子进程 env 设置／清除 LP_NUM_THREADS；不改当前shell。
python tests/observe_processes.py build/p1-mesa-thread-control-20261009/runs/original-default-ui_stateful_components_light/processes.jsonl -- "C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/ctest.exe" --test-dir build/p1-mesa-thread-control-20261009/product -R '^ui_stateful_components_light$' --no-tests=error -V --output-junit build/p1-mesa-thread-control-20261009/runs/original-default-ui_stateful_components_light/ctest.xml
```

路径在实际记录中为绝对路径。已存在的证据目录不能原位重跑；重跑须新目录，不覆盖本轮或历史结果。

## 7. 正式方案比较与下一最小目标验证

| 方案 | 证据支持／限制 | 本轮决定 |
| --- | --- | --- |
| Mesa生命周期修补 | 应区分正常销毁／卸载与进程终止；工作线程模块引用、global screen atexit及等待竞态均需处理；完整task/handle状态尚缺 | 不猜测改DLL、跳过正常清理或多次FreeLibrary；先补目标对照／准确任务状态，再在独立新provider构建验证 |
| 已验证上游修复 | 2014 Bug76252修复思路已在24.3.4，不是本次新修复；尚无已核实解决本固定DLL等待的新补丁 | 不盲目升级或把历史修复重新应用当验收 |
| LP_NUM_THREADS=0绕行 | 本负载线程0+0、CRT pin0、真实卸载、自然exit0；有匹配1线程失败及上游实践；重绘制成本4.99倍 | 保留为Windows固定OSMesa诊断／候选绕行，尚未成为框架或宿主正式默认 |
| LP_NUM_THREADS=1 | 有1+1工作线程及两次pin，四个同栈退出停滞 | 不能作为避免该等待的替代绕行 |

下一目标 CI 提案已成文并可审查，见 [proposal-final.patch](p1-mesa-thread-control-evidence-20261009/raw/ci-proposal/proposal-final.patch) 与 [提案说明](p1-mesa-thread-control-evidence-20261009/raw/ci-proposal/README.md)。补丁未应用，新增测试runner/helper仅作为拟议文件保存在新证据中；`tests/osmesa_thread_probe.c` 为待纳入授权提交的当前调用方。普通工作流路径不改，诊断仍只选择原两项，保留25分钟job预算。

授权后最小目标是一次既有 P1 dispatch 的 webview2／osmesa 两配置，每job同一新构建、同一固定DLL和同一caller，在独立进程顺序进行默认／0／1的exit/unload校准，再同一framework二进制默认／0分别运行原两项。环境只传到相关子进程，首次初始化前生效；原TIMEOUT仍180／240。默认失败保留，0通过也不会把总多线程路径宣称修复。

必须补取：目标默认及0的 live worker栈与真实模块是否卸载、完整pid/tick/Dispatch/running/exited/exitCode/EOF；若再次停滞，抓对应线程／模块匹配dump并尽量取得等待task原handle、exited semaphore计数及GetExitCodeThread检查时序。目标provider、system images、PDB和PE必须匹配；主机DLL引用实际计数可用本轮native观察器独立追加，不能混为普通通过。新runner捕获同构系统映像及source/product hash；独立45秒限制只用于standalone，不作用于原CTest。

提案`git apply --check`、Python AST、YAML、替换Actions表达式后的4份PowerShell脚本解析及现有CI政策检查通过。未执行目标runner的远程流程，因此不声称整个提案已目标实测。未经替换的`${{ }}`不能直接作为PowerShell解析；首次失败记录保留。再次提交／推送／远程dispatch需要新的明确授权，本轮没有执行。

## 8. 未解决项、记录局限与交付状态

仍缺目标默认／0因果对照、目标完整task/handle/semaphore状态及能保留并行性能的生命周期修补。1线程失败支持同等待路径责任，但不能解释默认本机92个为什么通过、目标默认四项为什么易复现。四个本机转储有限内存，未补造完整活跃引用总数或任意线程exit状态。

首次编译有GL宏冲突及user32漏链接，均修正并保存失败源码／日志；一次benchmark runner标签括号错误在新进程启动前失败，已保留且续跑未重做完成的cycle；32位CDB无法启动64位目标，实际没有启动probe；首次用System32映像经32位重定向造成系统映像不匹配，其栈日志保留并排除，随后复制正确64位映像、重解并逐个匹配。probe主PE/PDB未被修订版覆盖。辅助初始化、ctest路径及字符编码错误不计产品失败，也未删除记录。

当前源码 Session0、严格整机无登录、真实英文Windows、中文IME、物理混合DPI、桌面合成、长期人工验收仍独立待条件。没有注销用户、修改服务、降低无登录门槛或扩大硬件GL。历史持续重绘和瞬态Runtime不由本次Light退出定位宣布根治。

新增范围仅为独立C调用方、本报告、新证据及未应用目标CI提案。正式环境变量、框架API和产品生命周期未改；没有SDK重打包。新build的caller/PE/PDB/完整BMP仍被Git忽略；证据索引记录其本地位置和hash，clone不会携带这些忽略产物。下一步是在新的明确远程授权下执行已审查目标对照，或先对匹配停滞task补抓内存后再设计Mesa局部修补。
