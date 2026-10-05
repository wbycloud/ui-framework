# API7 交付收敛、CI可复验性与业务试点准备

日期：2026-10-05至06。起点为干净`68c4e9cafe4ba7e89267d19d0035474c45bad8a7`，分支`codex/menus-offscreen`。读取当前合同及API5/6/7、Runtime、菜单和Session0记录，核对公共头文件/CMake/工作流后实施。API5/6/7及菜单不重复开发。本轮没有推送、远程工作流触发、新标签、代理或新会话。

## 1. 本地修改及身份

| 单元 | 实际commit与范围 |
| --- | --- |
| CI证据与结果校验 | `ae4dc7fdff37eac382d354cef74c95ddc2f791e1`：记录精确CTest清单/二进制/原包来源，拒绝空测试、遗漏、重复、失败或异常跳过；补原API5/6 Runtime过滤 |
| 无Runtime构建修复 | `2fde15efca100cebb57e1d8b0ae65450a1592858`：滚动测试按可选后端编译，关闭WebView2不引用其函数，显式请求该模式退出1；不改变产品行为 |
| GL环境隔离 | `e4a33bf5142550d52e85b728670813ee6cec7390`：WGL/provider/Runtime分阶段，保持资源门槛；记录/核验CTest至少3.26 |
| 文档与证据 | 包含本文的后续本地提交，`git log`核对。仅说明/归档，不改变上述已验执行代码 |

当前入口统一SDK0.7/API7/标准7：修正handoff及开发标准/能力表仍称标签停靠未实现、枚举循环/没有颜色拾取器和接手提示仍为API6的残留。README、构建、迁移、接口、升级提示和CHANGELOG同步；历史版本、失败与结果保留。增加[业务试点准备](../business-pilot.md)，当前没有本轮试点应用指定和修改授权，历史KLayout只读观察不授权迁移。

`include/`、`src/`相对起点没有修改；公共C ABI/API7、Windows x64/C11、完整size字段/偏移/枚举、线程/所有权/卸载合同保持，不升级ABI。不修改请求方应用、PERF-001、SDK1–6或原包，不提高预算或删除断言。

## 2. 先复现、再修复

以下原日志、XML、CSV和运行器保留在[原始证据归档](api7-delivery-evidence.zip)，内部`evidence-index.json`列每份原始字节SHA256。归档190份文件，ZIP SHA256为`63613cd825288f8845dfa656c7b20d5ce633dd04d239cce6b875482aaf7e10a4`。历史失败继续在[API7记录](api7-validation.md)和[Runtime记录](runtime-stability-validation.md)，未覆盖或改写。

| 原失败 | 复验/修复 |
| --- | --- |
| 旧过滤器把`ui_original_api5_package`及`ui_legacy_api6_webview2`排入软件WGL阶段；空目录CTest默认退出0 | `convergence/ci-baseline-red.log`及空CTest日志；当前`--no-tests=error`退出8。独立脚本用例验证版本、三阶段、空/缺/重复/失败/跳过、必跑Runtime及原包身份 |
| 首次新轻量配置链接失败，`component_scroll.c`无条件引用WebView2 create/destroy，`LNK2019` | `light-build-red.log`/configure及首次矩阵进度绑定ae4dc7f；2fde15e加编译条件，light/osmesa新构建通过，关闭后端时显式请求Runtime退出1 |
| 2fde15e OSMesa行真实资源失败：200周期用例在182周期提前停，2000命令未满；341→346句柄、USER99→103、HWND0 | `osmesa-red-*`/`osmesa-red-control/`保留完整断言、CSV及失败退出8，CI脚本退出1。没有提高Session0对照原句柄+2/USER/GDI不增长或128MiB门槛 |
| 同二进制环境对照 | runner SHA256 `6dc81142f4922cf52ff7200611d5d2294fc9299c9e0c9104f55700d91fc90c4b`不变，仅隔离本行部署的四个WGL DLL；三个独立200周期均PASS，句柄237→238、GDI0、USER2→2、HWND0、2000命令；private峰值112107520/105959424/111779840，均原128MiB内 |

该对照支持将显式OSMesa与软件WGL部署分开；未使用WGL的程序加载应用本地WGL依赖也会影响资源基线（部署差异与资源差异来自对照，内部每个资源的创建栈未全部定位）。e4a33bf先执行WGL用例，再移除本行四个DLL，执行显式provider；WebView2最后执行真实Runtime。没有修改系统OpenGL、清理用户数据、强杀Runtime、延时掩盖或把此隔离解释为历史所有WebView2波动的根治。

诊断设置错误也保留：第一次隔离运行漏建输出目录，退出2且没有执行有效GL用例；首次64周期启动器批处理编码/换行错误，尚未启动用例。补齐目录及ASCII/CRLF启动器后的实际结果另列，不把这些设置错误算为产品失败或通过。

## 3. 当前四行脚本实际本机运行

全部绑定干净`e4a33bf`，固定现有依赖，新`build/ci-*`配置，逐行执行原`tools/windows-ci.ps1 -Stage build/test`；本机复用并核验已存在依赖，没有运行远程prepare/安装Runtime。每行只有`ui_monitor_transition`因单物理显示器跳过，其余实际运行、无失败。

| 配置 | PASS / SKIP / FAIL | CTest实测时间（分阶段相加） |
| --- | --- | --- |
| native | 21 / 1 / 0 | 6.46秒 |
| light | 41 / 1 / 0 | 33.27秒 |
| osmesa | 45 / 1 / 0 | 80.25秒 |
| webview2 | 54 / 1 / 0 | 240.65秒 |

合计165次配置内测试执行，161通过/4同一物理条件跳过，包含跨配置重复，不称165个独立用例。每行`matrix/<配置>/`保存manifest、test-plan.json、JUnit、完整LastTest、编译日志和帧。所有sourceDirty=false，无GitHub run ID，本机结果**不属于托管CI证据**。原API5四项、非MSAA、菜单、布局、滚动/颜色、初始化失败/异步/图片/双实例/模态/卸载及公共C/C++调用方均在相应配置中执行。

原包没有下载源，也不在Git内。四行默认API1使用冻结SDK重新生成的fixture，其他原包没有提供；`original-packages.json`按缺项/重编分别记录，不能因为矩阵通过就算原包通过。六原包随后单独提供并执行第5节。

## 4. 当前关闭稳定性与证据边界

同一e4a33bf实际产物两个独立宿主进程，各64连续重开：`runtime-fast-64.log`和`runtime-ready-64.log`均句柄406→406、GDI12→12、USER17→17、最终pending cleanup=0、逐轮DLL卸载、0失败/退出0。呈现模式必须查到实际Runtime字段后关闭；最后暂存pending64、句柄峰值564，随后正常STA事件推进至0并满足原+12/+4断言，没有将中途pending冒充释放完成。日志reopen0..63确定64周期，汇总旧标签reopen8不作为次数。

四行Runtime的API5内置32周期句柄384→380、GDI9/USER15不增长、pending0；API7 Runtime8周期388→388，Light234→234。`ui_api5_native_host`实际SendInput、owned/焦点/单次命令回归本次6.19秒通过；历史VK_PACKET/前台争抢失败及13次独立通过继续归原源码，不能改写为已永久消除。

框架pending、宿主句柄/GDI/USER与Runtime进程资源分别理解：pending归零与原资源断言支持所列关闭合同，不保证用户数据目录中所有其他view/Runtime进程退出。进程类型/PSS诊断、异步Close/BrowserProcessExited依据及历史未完全归因差值见[稳定性记录](runtime-stability-validation.md)。本轮没有修改关闭产品语句，也不以多次通过宣称全部历史失败同一根因或长期人工已验收。

## 5. 兼容及依赖

SDK1–6只读核对未变；SDK6全部12头文件仍等于658b497原接口（仅归一Git换行）；六个原包SHA256仍与[原清单](api7-validation.md#5-兼容依赖和复现)一致。另配置同一build/ci-webview2的六个UI_LEGACY路径，运行`ui_application_versions`、原API2/3/4/5及原API6 Light/Runtime，共**7/7 PASS，111.41秒**。记录在`original-tests.log/xml`、`original-output.log`、`original-packages.json`；所有六原包状态passed。这是本机原包证据，不能继承给未提供原包的托管CI。

当前API7调用方与冻结SDK调用方在四行构建/运行，公共接口/产品源码完全未变；不是任意第三方包兼容保证。64周期开始前、原包重新配置后的二进制身份分别在followup-progress及final-environment保存，不用后来hash覆盖之前身份。独立测试fixture仅是框架集成，不是业务应用验收。

| 环境/依赖 | 实际条件 |
| --- | --- |
| OS/编译 | Windows x64 NT10.0.26300、MSVC19.50、Windows SDK10.0.26100.0、Release/C11；CTest4.2.3-msvc3 |
| 轻量 | Lexbor7fb22cf5664a331d7c24b113489e566767c9c25a；QuickJS-NG2f0aa72a6b09cf69ff06399bcf9f4c083ac1a278 |
| Runtime | SDK1.0.4129.50；实际Evergreen154.0.4258.53，最低接口条件138.0.3351.48；不能用SDK或Light代替实际运行 |
| GL | Mesa24.3.4 x64实际OSMesa DLL SHA256 c633b820bba8ec0dcb05466505e3c246e4c4028eaec7caf1054e707c8eb3ca1f；真实llvmpipe frame/MSAA0和4。WGL为显式应用本地软件提供方，provider/Runtime阶段移除该部署 |

依赖固定源/hash检查、命令及阶段说明见[构建文档](../build-and-validation.md)。归档保留实际frame PPM/RGBA结果，不用生成或替代图片。用户界面/截图/注入输入不当作物理或真实IME证据。

## 6. 最后环境检查与远程具体下一步

实际最后检查2026-10-06：Session1、非管理员、1920×1080单显示器/DPI96、有DWM进程；DWM存在不是屏幕边缘合成验收。final-environment记录dirty=true是独立fixture输出到根目录的`api7-runtime-frame.ppm`；随后移入证据目录，post-artifact-status记录工作区干净，产品/头文件无变化。当前服务脚本第11行权限守卫退出1；前后UiFrameworkSession0-*服务为空，未创建服务、注销用户或修改既有服务。**当前源码Session0未执行，无登录也未验**；历史78c24cb通过/无登录失败保留在[Session0历史记录](session0-osmesa-validation.md)。

| 待验 | 明确下一步 |
| --- | --- |
| 当前Session0 | 在已有管理员/服务管理权限的Windows x64环境，用e4a33bf及匹配hash产物执行现有run-session0-validation.ps1，先分别记录Session0 GL与整机登录会话；本机权限不足不绕过 |
| 严格整机无登录 | 用户已确认没有目标runner；有现成、启动后无人登录的服务runner及其标签后，运行现有session0-osmesa.yml，RequireNoLogin保持，记录WTS清单/Session0/实际GL/窗口/资源/哈希 |
| 托管四行CI | 先明确推送/执行授权；将包含三个修正及文档交付的确切本地HEAD，以普通fast-forward发布到codex/menus-offscreen，再观察已有windows-regression.yml四行自动触发，记录run URL、GITHUB_SHA、run_attempt及下载artifact。不重复dispatch同一push，不强推或自动更新main |
| 手动Session0工作流 | 同样先取得远程执行授权和实际已发布的API7 ref；workflow_dispatch选择session0-osmesa.yml、准确ref与runner_label。默认windows-2022有登录会话，严格job失败不等于独立Session0 GL失败；无登录通过需专用runner |
| 物理/人工 | 真实中文IME、不同DPI物理屏/边缘/DWM、长期人工操作条件未具备；按原UV清单记录操作和结果，程序DPI/输入/短重复不能替代 |
| 真实业务 | 先指定本轮应用/commit、数据和接入修改范围，按[试点准备](../business-pilot.md)最小接入；当前未执行业务验收 |

只读远端查询：main=`ea5b10816681c43b662adf351dcfe57e8ae00e7a`，codex/menus-offscreen=`9705565bea9b74afcad7f27aa92df9e1050c4e97`；本轮local e4a33bf不在这些远端头上。没有新的推送/远程执行授权，未执行相关动作。若另授权推main，CMake变更会匹配严格Session0工作流的push路径，需预先理解其无登录门槛，不能放宽断言求绿。

本轮可独立执行的代码/脚本修正、必要回归与准备文档完成；托管CI、当前Session0/整机无登录和物理/业务条件仍待验，不宣称全部验收完成。没有实测OSMesa性能瓶颈及硬件需求，不扩大硬件无窗口GL范围。
