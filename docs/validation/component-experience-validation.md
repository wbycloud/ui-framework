# API8 共同组件、工作区与辅助体验验收

2026-10-07，接手干净 c62753d55b5f97d97a0a9acba0b0ec93e350b9c7，原SDK0.7/API7/标准7。当前SDK0.8.0-dev/API8/标准8，ABI1/包1。产品7051596、焦点8986b5d、CI身份5c02bad、列定义测试e0eff1e、最终输入/完整文本807828147bdfb6959abb49b4668dfa90c34eda56，测试消息泵/宿主就绪收敛0fe08640c3008874449dce77fd7445ef91f6a578；产品二进制仍对应8078281，最终文档提交不改变运行代码。均为本地提交。没有代理、新会话、graph-engineering、推送、远程CI、发布或稳定标签。

## 五项实现与边界

| 项目 | 已实现与成功检查 |
| --- | --- |
| 列宽 | 6逻辑像素表头拖动区域，24–4096显式调宽；排序区域独立；捕获/释放/Esc/丢失/源/排序/模态/失活/关闭取消。右键/F10/Alt+Down适配当前缓存与表头，单列/全表重置。UCW1由应用存储，组件/列稳定ID、实例隔离、原子拒绝异常、未知/新增列处理。初始应用宽度不自动改变；表头、内容、滚动条与按需范围同步，草稿/选择/焦点保留 |
| 数据可读性 | 特殊列3像素内距、24像素勾选、按宽度显示色块/图标；表头排序留位，与内容严格相同列宽。选择背景、焦点边框与琥珀编辑状态区分。悬停预览、F1/右键完整只读查看，可滚动复制、Esc恢复焦点。旧分页按钮未恢复 |
| 属性/选择器 | 宽区标签80逻辑像素与值同行，长标签/窄区/多行堆叠；单位、可选help、错误各自呈现。128项有界枚举滚动及键盘首末项；RGBA响应式轨道、预览草稿、原提交一次/取消原色。名称是展示标签，保留旧Tab顺序；原输入Ctrl+F1查看完整名称 |
| 工作区 | 长标题与停靠标签悬停/F1查看，按实际宽度分配有界1–4标签及既有溢出路径；恢复可见有效焦点。浮动最小160×120、对话框240×180逻辑像素，标题工具对齐，停靠预览空心。布局格式2及原GL context复用 |
| 弹窗/AI | 模态DPI初始化、正确本地→屏幕→宿主锚点；悬停提示及子窗口不激活/不截获输入、不恢复旧焦点。完整查看层位于原组件内，不新建业务命令或重建GL；Enter不提交底层表单。AI保持每次启动收起、用户切换、目标固定与原生/GL尺寸回收 |

无窗口：列宽逻辑操作、状态与共同轻量输入/捕获/完整查看均适用；有窗口鼠标捕获、桌面弹窗与拖拽需要交互桌面。WebView2需要真实Evergreen Runtime。基础overflow和虚拟数据条不同，后者仍按完整total/列宽与uint64/BigInt工作；未提高1024节点、2MiB缓存、图片、投递、JS或资源门槛，未预载全量数据。轻量后端旧同HWND捕获、加载覆盖层保持，src/backends/light_web.c本轮无改动。

## 原失败与修复证据

首轮完整矩阵46通过、9失败、1物理跳过，日志保留；Light getBoundingClientRect没有right/bottom以及浮动静态onkeydown/onmouseenter/tabindex不受支持，改用x+width/y+height及已支持的动态监听/属性。受影响9项通过。Runtime首轮呈现超时保留，后续通过不视为永久根治Runtime启动波动。

8078281之前的业务副本完整结果26/30、24/30及两次高级输入180秒超时均保留，不由后续通过覆盖。框架自有只读诊断记录模态字段rect96,78,320,28，却出现提示位于视图左上并激活/截获点击：模态c->dpi未初始化，新锚点换算成零；提示子窗口仍接收鼠标。修复初始化DPI、模态坐标换算、提示穿透/禁止激活及编辑键盘路由后，真实SendInput字段输入通过。诊断原cleanup断言不等待WAIT而失败，仅作定位，不作为关闭验收；正式原生命周期用例负责最终回收。

属性名称按钮引入额外Tab站，原业务Shift+Tab枚举导航失效。恢复展示标签，完整名称改由原控件Ctrl+F1及鼠标访问；业务原断言保持。最终专项业务workflow/usability/advanced 3/3通过，原生新模态真实鼠标/输入/F1/Esc与96/144/192 DPI、Light/Runtime列宽/枚举5/5通过。完整查看和表头状态用公共presentation验证实际文本、非空clip及焦点，不能用仅编译或离屏结果替代原生输入。

## 最终结果

| 当前实际执行 | 结果与原始证据 |
| --- | --- |
| 完整框架，Light + 真实Runtime + WGL/OSMesa/MSAA/旧非MSAA，串行 | **55通过、1物理条件跳过、0失败**，202.67秒；[最终日志](component-experience-evidence-20261007/raw/framework-delivery-final.log)、[JUnit](component-experience-evidence-20261007/raw/framework-delivery-final.xml)。包含十万行/64列、列宽/UCW1/列定义变更、完整范围滚动、树/表单、双实例、模态、GL、REFUSE/WAIT、异步关闭与原资源断言 |
| 单独无Web后端的原生配置 | **20通过、1物理跳过**；[日志](component-experience-evidence-20261007/raw/native-8078281.log) |
| 当前业务abc8源码副本完整原测试 | **29通过、1失败 kc_host**，285.96秒；[原日志](component-experience-evidence-20261007/raw/business-8078281.log)、[JUnit](component-experience-evidence-20261007/raw/business-8078281.xml)。旧固定坐标没有命中新折叠入口，断言不改，不能报告30/30或完整业务验收 |
| 框架自有业务鼠标/布局观察器 | **102检查通过**；[日志](component-experience-evidence-20261007/raw/business-real-scroll-pixels.log)。真实SendInput拖末项、真实滚轮至first3后拖回首、64列未缓存末列查询并显示、重复拖动、resize、切焦点、公共presentation折叠/恢复、GL保留和独立进程布局档案；不冒充原业务测试或修改授权 |
| 实际WebView2重开 | **2个独立进程，各32个Runtime呈现就绪周期，0失败**；[运行1](component-experience-evidence-20261007/raw/runtime32-1.log)、[运行2](component-experience-evidence-20261007/raw/runtime32-2.log)。各句柄391→393，GDI12→12、USER17→17、pending cleanup0，保留原门槛；日志末尾历史固定文字“reopen8”不代表实际周期数，逐行reopen记录为32 |
| 匹配SDK消费及离线重建 | 公共C头文件/导入库/DLL列宽160与UCW1 120字节通过；[消费日志](component-experience-evidence-20261007/raw/sdk-consumer.log)。SDK自带依赖离线重建C11公共头/列宽/属性用例，**3/3通过**；[构建](component-experience-evidence-20261007/raw/sdk-offline-build.log)、[测试](component-experience-evidence-20261007/raw/sdk-offline-tests.log) |
| CI本地可复验性检查 | [现有四行分类/测试选择检查通过](component-experience-evidence-20261007/raw/ci-contract-final.log)；未推送、未触发远程工作流。上述本机结果不是托管CI四行运行证据 |

最终冻结产物身份见[artifact-identity.json](component-experience-evidence-20261007/artifact-identity.json)及[delivery-frozen-hashes.json](component-experience-evidence-20261007/raw/delivery-frozen-hashes.json)。8078281的两次中间完整运行均54通过/1宿主超时/1物理跳过，保留原日志。诊断一次取出3001条消息、1983条WM_PAINT、16.2秒，队列不必排空；旧消息泵在PeekMessage取出后才检查条数，且仅限制数量。0fe0864将条数/100ms实际耗时检查移到取消息前，确保取出的消息均Dispatch，不增加睡眠、不延长30秒或放宽断言。活动实例切换后显式layout及公共flush确认就绪；原一次命令断言保留。独立3次通过后再完整矩阵通过。持续重绘的来源尚未定位，不能把消息泵修正写成产品重绘根治；[失败诊断](component-experience-evidence-20261007/raw/features-pump-red.log)、[中间一次命令失败](component-experience-evidence-20261007/raw/features-pump-green.log)、[3次通过](component-experience-evidence-20261007/raw/features-pump-ready-green.log)均保留。

末列63为STYLE，不应伪造文字。观察器验证稳定行ID241、view-local非空clip(8,76,36,32)、实际STYLE内容，并捕获实际显示像素225×323，内部非均匀像素620；日志记录末列请求。原生框架用例另核对文本列稳定ID、文本及非空可见区域，程序DPI96/144/192在完整矩阵内通过。

原业务固定坐标折叠断言及历史29/30记录保留，未更新应用默认列宽或原测试。公共presentation定位真实折叠、原生鼠标末项/真实滚轮first3后回首/未缓存末列查询和显示、重复/resize/切焦点由框架维护观察器复验。业务副本仅改变依赖锁和清单元数据，不修改原仓库、冻结SDK/原包/历史证据/PERF。接手原业务abc8d870efc6401dc4fa55e50170a5449c11e36b；期间外部提交769477b为历史过程产物清理文档，本文观察仍锁定原abc8归档，不替换失败断言。

## 复验命令与依赖

Windows x64 26300交互Session1，MSVC19.50、C11、Windows SDK10.0.26100.0，CMake/CTest4.2.3-msvc3。Lexbor7fb22cf5664a331d7c24b113489e566767c9c25a、QuickJS2f0aa72a6b09cf69ff06399bcf9f4c083ac1a278、WebView2 SDK1.0.4129.50、OSMesa24.3.4 x64。依赖/真实Runtime身份及产品SHA256另见证据清单。OSMesa部署osmesa.dll和libglapi.dll，未部署软件opengl32.dll；WebView2使用实际Runtime而非轻量结果替代。

在VS x64 Developer终端，以仓库根目录运行：

```powershell
cmake -S . -B build/experience-20261007/after -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_FRAMEWORK_ENABLE_WEBVIEW2=ON -DUI_OSMESA_LIBRARY=<绝对目录>/.deps/mesa-24.3.4/x64/osmesa.dll
cmake --build build/experience-20261007/after
ctest --test-dir build/experience-20261007/after -N
ctest --test-dir build/experience-20261007/after --output-on-failure -j 1
powershell -NoProfile -File tools/test-windows-ci.ps1
```

真实桌面测试串行，匹配头文件/导入库/DLL/宿主/.uapp并冻结哈希；使用完整路径CTest，不把No tests found/命令未找到的退出0算通过。独立业务构建、观察及实际执行参数见本轮证据中的cmd/日志/计划。SDK匹配include/lib/bin/source固定依赖与许可，UCW1和help字段完整size/线程/所有权见[迁移](../migration-v0.7-to-v0.8.md)。字段help x64偏移64/完整size72、枚举和既有偏移不变；源码未无故升级ABI。只维护本轮当前SDK8，不加入历史调用方/原包专项。

## 真实截图

[截图索引](component-experience-evidence-20261007/README.md)及[截图SHA256/尺寸](component-experience-evidence-20261007/screenshots.json)包含105张无损PNG：40张成功修改前基线、57张当前实际截图、8张受限Runtime失败过程图。失败图明确标为failed-restricted-runtime-capture，不能用于成功对照。当前业务只读副本为本轮新增观察，未伪造其本轮修改前截图；真正同应用/数据/1200×800/96DPI/主题的前后对照使用完整框架独立DLL/.uapp（100k/64列、树、属性、异步图片与实际OSMesa GL）。

| 同一真实完整窗口 | 修改前 | 修改后 |
| --- | --- | --- |
| Light浅色 | [之前](component-experience-evidence-20261007/screenshots/before-light-host-light.png) | [之后](component-experience-evidence-20261007/screenshots/after-light-host-light.png) |
| Light深色 | [之前](component-experience-evidence-20261007/screenshots/before-light-host-dark.png) | [之后](component-experience-evidence-20261007/screenshots/after-light-host-dark.png) |
| 实际Runtime浅色 | [之前](component-experience-evidence-20261007/screenshots/before-runtime-desktop-host-light.png) | [之后](component-experience-evidence-20261007/screenshots/after-webview2-host-light.png) |
| 实际Runtime深色 | [之前](component-experience-evidence-20261007/screenshots/before-runtime-desktop-host-dark.png) | [之后](component-experience-evidence-20261007/screenshots/after-webview2-host-dark.png) |

显式列宽[调宽后](component-experience-evidence-20261007/screenshots/after-light-columns-resized.png)、[适配/重置入口](component-experience-evidence-20261007/screenshots/after-light-columns-tools.png)、[缓存适配](component-experience-evidence-20261007/screenshots/after-light-columns-fit.png)，以及[完整文本](component-experience-evidence-20261007/screenshots/after-light-complete-text.png)、[长枚举](component-experience-evidence-20261007/screenshots/after-light-enum.png)、[RGBA](component-experience-evidence-20261007/screenshots/after-light-color-picker.png)、[工作区](component-experience-evidence-20261007/screenshots/after-light-workspace-details.png)、[窄窗](component-experience-evidence-20261007/screenshots/after-light-narrow.png)、[当前业务完整GL](component-experience-evidence-20261007/screenshots/after-current-business-light.png)另列实图；Runtime对应图和96/144/192、AI展开/收起在索引内。截图观察器用公开set_column_width展示调宽状态，真实表头鼠标拖动由原生SendInput用例另验，不能互相替代。默认初始列宽未变，所以普通窗口对照不会自动显示适配后的宽度。

捕获使用真实宿主/组件/应用DLL的PrintWindow/桌面BitBlt，PNG仅无损RGB转换，不生成图片或静态HTML效果图。观察源/构建cmd/执行日志均交付；自动输入与截图只覆盖对应程序条件。

## 待验条件

环境审计为Windows交互Session1、非管理员，申请SC_MANAGER_CREATE_SERVICE失败，准确Win32错误5（拒绝访问）；本轮没有创建/修改服务或注销用户。当前源码服务Session0待授权，严格整机无登录服务runner缺失，托管CI无执行授权；真实中文IME、不同缩放物理屏幕、屏幕边缘桌面合成/Snap及长期人工缺相应条件。见[审计](component-experience-evidence-20261007/raw/environment-final.json)。程序DPI/自动输入/真实Runtime和截图不替代物理与人工验收；历史Session0不能冒充本轮服务复验。用户已确认没有严格无登录runner。没有实测OSMesa瓶颈或明确硬件需求，本轮没有扩大硬件无窗口GL。缺失条件明确保留待验，不宣称全部验收通过。
