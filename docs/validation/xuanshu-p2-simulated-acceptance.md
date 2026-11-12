# P2 开发阶段模拟及自动验收

日期：2026-10-24。结论：**P2开发阶段模拟及自动验收通过**，限定为下列A/B合同、业务及模拟UI范围；不代表真实设备验收、全部历史CTest绿色或无条件P2完整验收。C类按照本会话明确指令延期到软件整体开发完成后、另获授权的集中人工验收。

## 1. 当前基线和证据身份

实际Git：`codex/p1-process-boundaries`，HEAD `e0eca03db25930be55bdb45477027d1793a61904`；未提交。独立证据根：`build/xuanshu-p2-sim-20261024-d36f82`。入口17406文件baseline保留原修改、未跟踪与根规划；不把HEAD至当前全部差异归本轮。没有重复申请参考工程访问；它保持只读，历史67eacf5身份没有重新核对。

前轮`build/xuanshu-p2-remaining-20261023-b84e6f`的delivery60项、source280项、binary689项在接手时均匹配，见`prior-evidence-check.json`。本轮改5个测试源文件和必要文档/规划；**生产源码、公共头、CMake、版本和固定依赖没有改变**。SDK0.10-dev/API10/标准10、ABI1/包1、旧公开布局/v1及冻结C2合同、JCS/SHA-256/原限额保持。新增别名/参数验证调用应用真正已注册的binding与collector，没有增加产品执行器。

## 2. 验证分类与证据键

A类是接口、模型、合同或专用夹具断言；B类是程序UI、真实控件/窗口和隔离私有接纳后的事件路由。C类是无法由这些测试证明的真实来源/人工判断。每个原项目可拆成A/B和C，而不是删除整行人工要求。

| 键 | 实际证据 | 可证明范围 |
| --- | --- | --- |
| E1 | `aliases-first-light-test.log`、`light/aliases-first.xml`：generic204/0、features389/0、EDA142/0；该阶段总体3/4，apps失败另见E2 | 真实应用binding/collector/业务run与具体值；七菜单/13工具栏别名各自执行，非硬件来源正例 |
| E2 | `reopen-contract-fixed-light-test.log`、同名JUnit：实际五DLL及两个features配置364/0，7.30秒 | 双实例标签固定read调用，关闭/重开、旧instance和单独旧open epoch拒绝，真实安全卸载；门槛仍15秒 |
| E3 | `form-collector-first-light-test.log`、同名JUnit：stateful223/0 | 原201项保留；增加真实FORM/DIALOG收参、shared name、独立草稿、冲突keep/提交和实际模型回读；含真实固定路径I/O UNKNOWN |
| E4 | `origin-range-light-test.log`、同名JUnit：4/4；HMENU29/0、headless inputs596/0、Light inputs741/0、真实workspace650/0 | 实际菜单句柄/匹配链逻辑，来源枚举/混合链、公有模拟拒绝、真实窗口/控件及固定workspace；不是OS硬件分类正例 |
| E5 | `reused-evidence.json`：逐项引用已匹配前轮Native/ASAN joint-regression及Light scoped JUnit的成功case | 未受本轮测试修改影响的JSON/Schema/JCS、目录/C1桥接、预算、v2/FORM合同、生命周期、公共头和组件；含counter37/0。不是本轮重跑 |
| E6 | `production-exports.json`：前轮验收DLL与本轮重建DLL各299导出，私有授权/fixture/route导出为0 | 私有接纳只在静态专用测试中使用，不导出到生产DLL；静态源码/构建宏也已核对 |
| E7 | `origin-component-branch-light-test.log`、同名JUnit：1/1、CI-1 64/0 | 直接包含同一inputs测试源的组件编译分支也重建并验证，不用另一分支通过替代 |

最终通过集合为10个新/受影响测试case在各自最终阶段的并集，并非一次10/10命令。E1通过的三个case没有在E2中修改或机械重跑；E4/E7只修改来源测试真值表，不改变应用测试/产品。源和二进制以各阶段`*-sources.json`、原始日志、JUnit、失败快照和最终清单定位，不能把当前整个树误当早期链接身份。

## 3. 原人工清单逐项A/B映射

下表顺序和项目名与[原清单第2节](../xuanshu-p2-manual-acceptance.md)的20行一一对应。C编号指向[延期清单](../xuanshu-p2-deferred-manual-acceptance.md)，原操作步骤未删除。

| ID／原项目 | A接口/模型方法与具体断言 | B模拟UI方法与边界 | 证据与C延期 |
| --- | --- | --- | --- |
| M01 原生HMENU | 正确binding只执行一次；wrong menu、重复候选、modal零效果；Forget在菜单loop租约内PENDING | 真实HMENU/MNS_NOTIFYBYPOS句柄＋隔离候选；WM_COMMAND/无资格WM_MENUCOMMAND拒绝；未经过真实MSGF_MENU硬件来源 | E4；D01/D02/D03 |
| M02 Light菜单/按钮/工具栏 | generic/features所有原菜单、节点菜单、两原工具项及features七菜单/13工具项各自引用同一capability；属性/格值/树名实际文本与revision，view不改document | 使用实际binding、收参器和ui_inputs_route，私有接纳而非OS输入；布尔实际Light节点及树展开/右键逻辑复用E5 | E1/E5；D01/D06 |
| M03 程序旁路负例 | 普通invoke/source=human不改变真实模型；C1只读模式直接事务及修改入口明确拒绝 | counter公有dispatch不能增加count，私有一次接纳count=1/revision=2；普通invoke不能二次写入 | E1/E2/E5；真实宿主F7/F8体验D01 |
| M04 物理与注入区分 | origin读取失败、非HARDWARE及非法device拒绝；已知device1/2/4/8/16按原策略；混合down/up/按钮/修饰键/owner拒绝 | Light公开模拟不授权；程序消息实测origin=0/device=0。私有合格输入只测后半链；未执行SendInput动态矩阵或物理正例 | E4/E5/E6；D01/D02/D03 |
| M05 固定实例 | 原workspace v2异步调用在切标签后只使原模型value/revision=1，另一模型=0；五包read prepared于A、激活B后仍读A | 真实workspace和两个实例，不用固定指针冒充标签；chrome源/活动实例桥由E4覆盖 | E2/E4；真实切标签D01 |
| M06 关闭/重开 | 新instance及open epoch不同、同进程host_epoch相同；旧全target和仅替换旧open epoch均NOT_FOUND/call=0；新模型按明确初态读取 | 五实际DLL关闭、卸载、重开；迟到post CANCELLED，旧binding不能命中新实例 | E2/E4/E5；D01/D07 |
| M07 stateful单元格 | text实际“Actual business cell”、source一致，rev1→2；非法数值不改“42”和revision；NUL/超字节容量拒绝 | CI编辑器真实Light值/代次与last readonly门；Esc取消/再开、完整64位ID；私有接纳不等于设备编辑 | E3/E7；D02/D04 |
| M08 stateful选择/排序 | Ctrl增减anchor、Shift/CtrlShift范围及降序ID实际回读；document revision不变，旧sort handler结果为0 | collector完整ids/anchor参数＋实际组件selection/sort；导航的框架部分复用CI测试，不声称所有物理组合已执行 | E3/E5；D01/D02 |
| M09 共享name冲突 | 原form/dialog各自草稿保留；实际FORM提交name=actual form collected name；DIALOG仍私有且conflict=1；旧base提交VALIDATION_FAILED，keep后提交共享name=actual dialog private name | 真正ui_components_form_action及应用read_model/observer，不是把自造fields传给另一业务函数；私有入口隔离 | E3；D01/D02 |
| M10 对话框modal | 所属提交可用，外部modal门拒绝；末次revision/check/read_model回调重入改变modal后execute为0 | 实际stateful对话框完成后modal=NULL；冻结A/B最后门和同步栈租约用原专用夹具 | E3/E4/E5；D01/D06 |
| M11 stateful偏好 | 实际UST1保存列宽280、改为360后load回280；语言zh-CN及原UIL1；新业务name/格子不伪称持久文档 | 程序设置真实列宽/实际后台I/O，以结果/worker退出条件轮询；布局、theme/语言基础合同复用原组件证据 | E3/E5；物理拖列、DPI、主题体验D01/D05/D06 |
| M12 EDA实际操作 | 固定矩形width=.22/height=.12、ID不复用、clear后zoom=1；实际toolbar add、A/Z键binding更新真实对象/zoom，view不改doc revision | 真实surface和应用collector计算命中坐标，选中原对象ID；没有任意几何编辑 | E1；D01/D02/D05 |
| M13 EDA笔记 | 程序编辑原生EDIT不改committed note/revision；实际Submit收完整text/base/generation后note=actual native collector note；disabled按钮无业务修改 | 原生EN_CHANGE、真实按钮对象，私有路由提交；公开无资格route拒绝，不经过物理设备 | E1；D01/D04 |
| M14 EDA冲突 | 草稿期间加对象产生冲突；旧Submit不改已提交note；Keep后Submit得到preserve conflicted native note且revision+1；Cancel丢草稿不写模型 | 使用应用实际submit/keep/cancel绑定及原生EDIT值，非替代执行器 | E1；D01/D04 |
| M15 generic和features | 原真实格文本、Boolean、树名、size=2.5/name/notes、非法size/ID/NUL/字节容量与revision；两分支均保留 | 实际Light checkbox、树展开/context、导航＋所有现有menu/tool collector；属性模型和独立草稿原回归保留 | E1/E5；D01/D02/D04/D06 |
| M16 快捷键/画布 | Ctrl+D实开modal，features Ctrl+O image_version+1且doc revision不变；EDA A/Z实际对象/zoom，公开快捷键不授权 | 私有接纳后实际SHORTCUT/SURFACE metadata；焦点编辑与混合链的通用拒绝测试，不声称设备焦点组合已验 | E1/E4；D02 |
| M17 关闭诊断 | features refuse/allow真实close决定；真实DLL worker未退出时实例/module仍在，退出确认后才能count=0且模块卸载 | 专用受控worker/外部回调与实际workspace，不强杀、不UI join；同步complete外层栈仍持租约 | E1/E2/E5；D07/D08 |
| M18 异步/busy/取消 | 同资源在途第二写busy拒绝，value/revision不重复；取消不代表回滚；取消及队列满不丢原call/关联 | 有界状态条件轮询和受控DLL事件，不用固定睡眠猜已完成；实例切换后异步结果只归原call | E4/E5；D08 |
| M19 UNKNOWN | stateful实际临时路径创建失败得到UNKNOWN/EFFECT_UNKNOWN；cancel后forget仍PENDING；重复Save无第二worker；显式核对无证据仍阻止。invalid_output、预算/背压由框架夹具验证 | 独立test profile中创建并删除本次空.tmp目录；真实业务结果/ID及HWND寿命断言，不是控件计数 | E3/E5；未来故障交互D08 |
| M20 resize/DPI/IME及退出 | 原逻辑DPI/尺寸、字符、组件回读和真实装卸合同复用；没有把它们提升为完整IME/DPI结论 | 程序设置尺寸、隐藏窗口/GL，短时循环关闭/重开；视觉/跨屏/长时不可靠部分保留C | E2/E5；D04/D05/D06/D07 |

没有用handler执行次数、草稿或屏幕变化单独证明业务成功。生命周期事件次数仅用于验证“不重复/未提前卸载”等生命周期合同，并辅以call状态、module存在性、实际业务值/revision或对象身份。

## 4. 新测试、首次失败及证据复用

新增覆盖位于`tests/p2_generic.c`（默认/API4分别编译）、`tests/p2_eda.c`、`tests/p2_stateful.c`、`tests/p2_apps.c`、`tests/capability_inputs.c`。未增加公共API或生产测试入口。

首次`aliases-first`为3/4：apps358检查/6失败。新断言误要求重开后进程级host_epoch也改变；实际源码从A+B起即为process_epoch，真正应改变的是实例/open epoch。修正新测试以明确相等/不等合同，并另用“新instance+旧open epoch”独立验证NOT_FOUND，最终364/0。**不是修复了生产epoch缺陷**，没有改既有合同或将失败简单放宽为任意错误。失败源码、EXE/DLL/uapp、日志/JUnit保留在`failures/aliases-first-light-test`；此前已执行的原断言均保留。

入口baseline helper第一次将二进制清单的证据根相对路径误当仓库相对路径，造成assert。修私有核对脚本的base后689项全匹配；该错误不是产品崩溃、二进制丢失或动态red。见`baseline-harness-note.json`。

`capability_inputs.c`只扩展已有分类真值表到device0…31，覆盖产品原本接受的device16及更多非法值；不是改变可接纳来源或扩大产品资源限额。E4程序消息来源分类是负例，未运行物理正例/SendInput。

复用的case按配置、JUnit、实际源码与二进制身份登记；并列Native/ASAN记录不冒称独立功能数量。当前生产与上轮源码完全相同，仅上述测试源有增量，因此没有机械重跑全部C1、A+B、公共头、worker/队列合同或历史benchmark/recovery。新构建Light非ASAN，不能称新增应用测试经过ASAN；原Native ASAN对应未变核心证据保留。

## 5. 未定位失败及阻断判断

| 独立账目 | 本轮依据、优先级和判断 |
| --- | --- |
| 上轮Light surface_input首次thread modifiers失败 | 原日志保留，同断言在A+B Native首次也存在；生产win32_surface本轮未变。本轮没有重跑此测试或宣称根因解决。需输入稳定性独立核对；没有已证实由当前P2能力/模型新增路径引入的缺陷，不作为本轮A/B模拟功能收口阻断，保留集成风险 |
| 历史browser20失败 | 未定位、未重跑、未修；此前触及共享UI，不能排除关联。独立公共UI回归项，WebView2/browser业务不借本轮通过销账 |
| workspace首次20.05秒，门槛20秒 | 首次超时仍未定位；新P2实际workspace测试门槛15秒保持，7.30秒通过不是旧超时已解决。后续时序/负载诊断 |
| Native输入首次失败 | 与本轮首次fixture host_epoch错误分开，也与此前明确修复的WM_CHAR空回调缺陷分开；后续通过不覆盖首次 |

本轮没有发现需改变生产来源政策/冻结合同的阻断项，也没有因这些未定位旧失败放宽断言/超时/限额。开发阶段结论限于已列A/B范围；C延期不是“已验”，未定位账目也不是“无风险”。

## 6. 交付、保全和停止位置

独立目录保存baseline/identity、前轮身份核对、每阶段source/log/JUnit、red及最终匹配二进制、production-exports、reuse、delivery/preservation和cleanup。前轮匹配人工包保持原样；没有向冻结SDK、固定依赖、外部业务应用或参考工程写入。入口baseline不包含全部被忽略依赖/冻结SDK目录，不能夸称对未纳入路径作过全量入口出口散列。

最终`delivery-files.json`列本轮精确增量，`preservation.json`列基线保全和缺失项，`binary-files.json`及`failure-files.json`锁定产物。只清理本轮无用对象/编译中间产物，保留首次失败和必要匹配产物。没有commit/push/release/tag、远程流程、真实Provider、子代理或computer use。

真实设备和必须人工判断的D01—D08保留在[软件整体完成后的集中验收清单](../xuanshu-p2-deferred-manual-acceptance.md)。本轮停止当前P2开发阶段模拟/自动验收结论，不要求现在人工操作，不启动P3或AI Service/Provider/Coordinator/审批/存储恢复/通用事务撤销/跨实例共享写入。
