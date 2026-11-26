# 当前交接与APP开发参考

## 2026-11-26 代操作方案拒绝与重新生成

`ce632004` 后的本地整改已通过 Light 34/34、ASAN Light 6/6。首轮方案错误有可读原因及「重新生成方案」入口。复制为新草稿后重新绑定当前对象与计价，两次可信确认后才允许操作。画板 16 图元缺括号的合成形态继续零写入。DeepSeek Responses 支持结构化输出，现有计量器对该请求形状仍锁停。 [使用入口](xuanshu-operation-entry.md) · [验证与计量边界](validation/xuanshu-operation-tasks-validation.md#2026-11-26-首轮方案拒绝与重新生成)

## 2026-11-25 共用助手代操作

本地交付 `3be274` 支持已注册 App 在一次授权内按核实版本逐笔代操作。表格覆盖固定值、同任务两种真实源值。EDA 仅新增自身注册，实测从 1 或 2 个补到 3 个及已有 3 个时零写入。Light 和 ASAN 各 12/12，旧入口相称回归 34/34，隔离启动正式宿主加载两款生产包后退出 0。真实模型验证待另行授权。 [SDK 接入](xuanshu-operation-entry.md) · [验证](validation/xuanshu-operation-tasks-validation.md) · [本地入口](../build/xuanshu-app-operations-20261125-3be274/dist/start.cmd)

2026-11-12。面向APP开发者的参考入口已同步当前匹配交付。README、开发标准、统一能力/API索引、API10迁移、构建与升级提示词，以及目录/计数器/维护组件/browser示例形成连贯路径。静态核对与前后清单见[开发参考同步记录](validation/xuanshu-application-docs-sync-validation.md)。

通用助手限定独立复审及冻结已完成，产品快照`build/xuanshu-general-layout-20261112-6b37c1`、identity `2C447E95741824E2C2A94E3EF91EC1782CF75C5870E262D7831C44989C7029CA`保持。具体记录见[冻结](xuanshu-general-layout-freeze.md)及[交付清单](xuanshu-general-assistant-delivery-manifest.md)。本轮为文档静态核对，真实验证包GENERAL-REAL-20261112-a74e20保持Proposed。

BR1-KI-001仍未解决、用户接受非阻断。临时页面控件测试保持取消。历史首败、非空WAL及真实服务证据边界沿原记录保留。

## 此前冻结及准备checkpoint

# 当前：限定独立复审通过，冻结落盘，真实验证包Proposed

2026-11-12。本轮限定独立复审通过，必须修正项为空；按用户授权将匹配快照冻结记录落盘。快照`build/xuanshu-general-layout-20261112-6b37c1`，delivery-identity SHA256 `2C447E95741824E2C2A94E3EF91EC1782CF75C5870E262D7831C44989C7029CA`。原开发identity及源码/程序/日志清单保持原字节，历史待复审状态不回写。详[独立复审](validation/xuanshu-general-layout-independent-review.md)、[冻结记录](xuanshu-general-layout-freeze.md)。

独立核对447份测试范围源码、154项程序及3313项证据，逐次测试前匹配关系保持。Light100/100；ASAN100项分次覆盖（业务19.7347秒、另次98项、glyph最终1/1）；Native47/47按依赖复用，不是本轮重跑。生产371项导出与前置一致。首败、环覆盖687条、原DB/WAL及历史限制保留。BR1-KI-001仍未解决且用户接受非阻断；取消的临时页面控件测试不恢复。

真实验证准备为[GENERAL-REAL-20261112-a74e20](../build/xuanshu-general-layout-review-20261112-a74e20/authorization-package.md)，仍Proposed。公开官方模型/协议/价格及计量源码当日核对一致；根最多8次、10分钟、USD0.10逐轮实际预留，预算不足即停止，不保证8轮全部完成。包内浏览器读取加自有维护单元格74及本地撤销42，不能称为纯只读。初始请求、有限计划及撤销各自适用本人可信确认；旧真实批准不复用。本轮无构建/测试/生产启动/计量执行/数据库打开/凭据访问/Provider请求。

新准备命令只静态审阅，未动态执行；会话尚未创建，策略未加载。下一步仅等待用户对具体包另行明确批准，不能把本轮冻结或准备授权解释为发送批准。详[准备事实与费用边界](validation/xuanshu-general-layout-real-preparation.md)。

## 此前时点原文保留

# 当前：Light字形测量最小整改和自动验证完成，待独立复审

2026-11-12。已按追加授权完成一次分段基线及一次单条件对照，并据因果结果完成最小产品修复。基线原30秒Timeout；重复按钮字形GDI测量的单条件复用使完整对照20.52秒通过。工作区只改`src/backends/light_web.c`及`tests/light_glyph_padding.c`，不采用前轮无效DC复用，不跳过布局、投影或确认。

最终无本轮探针：Light100/100；ASAN原完整业务19.7347秒通过、另次98项通过、新增字形夹具修正后1/1通过，共100项逐项覆盖，不能写成首次单次100/100。Light完整业务7.64294秒。两次新增夹具失败与原日志/JUnit/匹配程序保留。Native47/47仅按未涉及Light源码及实际编译依赖复用，未重跑。

Light和ASAN关后各3任务、6消息、74事件、1图、2编辑回执；实际74/doc-2、撤销42/doc-3和记录恢复经原完整用例断言，进程正常退出。关后immutable观察无WAL/SHM且前后哈希相同。图/编辑事实不冒充通用tools表记录；六轮属于隔离模型替身，不是真实HTTP计数。371项生产导出、固定依赖、公开合同及预算保持。

匹配候选`build/xuanshu-general-layout-20261112-6b37c1`，identity与清单独立形成，等待限定独立复审；不自行冻结或声明真实服务通过。对照环最早687条覆盖、历史超时和失败均保留，不宣称解决所有未知期限根因。BR1-KI-001仍未解决且用户接受非阻断；临时页面控件测试保持取消。真实质量包Proposed、未核价锁停，无真实凭据、Provider或人工验收。

详[因果证据及最终验收记录](validation/xuanshu-general-layout-causal-validation.md)、[最小合同](xuanshu-general-layout-contract.md)与[候选交付清单](xuanshu-general-assistant-delivery-manifest.md)。仅清理本轮983项对象/资源中间件和派生只读观察副本，共451365506字节。保留所有本轮原DB及旁文件、必要失败源码/程序、原始日志/JUnit与trace，未清理历史材料。编译依赖VALID为清理前捕获，清理后对象缺失另行说明。

## 此前时点记录原文保留

# ASAN两次因果实验完成，布局CPU占主导，期限仍未解决

2026-11-12。已按集中授权执行一次ASAN完整基线及一次单条件对照：30.0689/30.0615秒Timeout，CTest均8、测试实际均255由CTest期限终止。固定512KiB trace完整保留。Light布局699次，CPU17.750/17.719秒，UI线程累计26.172/26.453秒；本次主要消耗在宿主投影与Light布局，仍有业务推进，不是稳定停滞的证据。同次递归复用测量DC没有解决期限，隔离对照未应用到工作区。

未取得更细责任函数及有效修复，完整ASAN业务仍阻断。已用完本轮两次诊断额度，不追加运行或延长30秒。下一最小范围仅Proposed：在Light布局内区分规则/自然尺寸与几何/文本区测量，并据证据选择单处整改；新诊断运行需另行授权。原候选C2379C435E0684A1EFC025A85A782397AF5B0D1CE4B3A49E44ABA7E84862C6ED及源码/程序保持。

证据根build/xuanshu-general-causal-20261112-68b2e4；详[本轮因果实验及限制](validation/xuanshu-general-asan-causal-validation.md)。两组原DB及非空WAL保留，关后仅观察派生只读副本；图和编辑保存browser结果及74/doc-2提交回执，根终态/独立74读回/42撤销/正常关闭恢复未完成。基线周期摘要缓存读取问题、控制前置映射和编译错误单列。无真实凭据、Provider或人工，不冻结，不宣称整改完成。

## 此前时点原文保留

# ASAN期限只读区分完成，观测构建与因果对照待集中授权

2026-11-12。当前不是待Key或本人确认状态。候选identity C2379C435E0684A1EFC025A85A782397AF5B0D1CE4B3A49E44ABA7E84862C6ED、447源码和179产物匹配。已核实归档诊断变体在29.109秒仍推进到root-complete及实际74/doc-2；不能把所有超时统一写成第4轮后卡住。最新有效30.0711秒超时的中间耗时仍未知。

本次仅只读审计与文档，不构建、不运行、不改产品或测试、不打开数据库。已形成九类候选的源码边界及两次以内的区分方案，详[只读审计与最小实验](validation/xuanshu-general-asan-causal-plan.md)。目前优先区分UI投影/布局/pump累计成本与worker/队列/计量/存储等待，尚无责任根因或新修复。原图JSON版本差不是独立保存阻塞证据，100000逻辑行也不是DOM行数。

上一轮明确仅一次且保持源码/程序身份，额度已经消费；新的隔离观测构建、最多一次基线及一次单条件对照需要集中扩展授权。原30秒、断言和预算保持，原两项修复保留。完整ASAN业务仍未收口；不得自行冻结、独立通过或启动真实调用。无新诊断目录，无需清理的过程产物，历史证据和全部旁文件保持。

## 此前状态原文保留

# ASAN唯一有效补测再次超时，两项边界直接修复结论保持

2026-11-12。外部驱动以ProcessStartInfo子进程映射真正移除四个非秘密策略/Profile变量，Win32缺失/空值对照通过，父进程环境未修改。原候选`build/xuanshu-general-boundary-20261112-9d6c53`的447份源码、179项程序产物及原ASAN测试前57项程序保持匹配，identity仍为`C2379C435E0684A1EFC025A85A782397AF5B0D1CE4B3A49E44ABA7E84862C6ED`。

只运行一次ui_general_host_business：PID36012，30.0711秒Timeout，CTest退出8，进程句柄取得退出255。原30秒门槛、断言、预算、工作目录及计量替身保持。本次初始化和业务进入有效，日志有四次脚本中断。关后WAL副本只读观察确认浏览器结果及维护doc-2提交回执已保存；根任务terminal0、隔离请求4轮。独立74读回、42/doc-3撤销、根终态保存、正常关闭和记录恢复未完成。未追加运行。

最新证据根`build/xuanshu-general-asan-retest-20261112-0f7a36`；详[唯一复测记录](validation/xuanshu-general-asan-retest-validation.md)。原30.0569秒、原程序30.08秒、前次1.00672秒驱动前提失败分别保留。电脑负载影响与具体期限根因仍未确定，不用后补记录覆盖首败。

两项边界直接Light/ASAN各8/8、Native7/7保持；完整Light95/95、ASAN94/95、Native47/47分别记录，本次补项0/1，完整相称验证仍未收口。当前没有待用户Key、确认或人工操作，也没有运行中的测试进程。后续最小范围为两节点完成及第四轮保存后的专业/根结果推进与投影/消息泵期限关系，需后续独立授权；本轮不自动扩大。真实质量包Proposed，BR1-KI-001及取消的临时控件测试状态保持。

## 此前状态原文保留

# 两项边界代码修复及直接回归完成，完整ASAN业务期限未收口

2026-11-12。候选`build/xuanshu-general-boundary-20261112-9d6c53`。所属图保存和引用完成前DRAIN、初始预览委托回调后状态重检均已最小修复。有效修前8例5失败、3对照通过，修后两项直接Light和ASAN各8/8、Native7/7。

最终匹配完整运行分别为Light95/95、ASAN94/95和Native47/47。ASAN正式宿主业务用例30.0569秒超时，原程序/原DLL新目录对照也30.08秒超时。具体期限原因仍未定位。无效视口、消息泵和临时探针试改已撤回，预算与原30秒门槛保持。完整自动验证尚未完成，当前不作冻结或独立验收结论。

当前源码447项及选定程序与三次完整运行测试前清单逐项匹配，修前及所有诊断变体、原始日志/JUnit和旁文件保全。66份关联主样本中65份immutable核对，ASAN超时主样本含非空WAL，保留并单独解释。生产371项导出和原358项保持，无私有入口。真实质量包仍Proposed、未核价锁停。

详[边界合同](xuanshu-general-boundary-contract.md)、[直接证据及剩余阻断](validation/xuanshu-general-boundary-validation.md)、[候选清单](xuanshu-general-assistant-delivery-manifest.md)和[入口](xuanshu-general-assistant-entry.md)。下一步最小定位范围是共享ASAN正式宿主业务的页面投影、50ms脚本中断与30秒期限。BR1-KI-001仍未解决、用户接受非阻断，临时控件测试保持取消。无真实凭据、Provider请求或人工验收。

## 此前状态原文保留

# 通用应用助手三项阻断整改完成，等待再次限定独立复审

2026-11-12。新候选`build/xuanshu-general-closeout-20261112-4f8a21`已完成最小修复、自审及最终匹配验证。closure-final为Light87/87、ASAN-Light87/87和Native40/40三次独立运行。原候选identity `06AC88C2CF5B1CC40C5EB6C9174D1F8C3317DFEDBF938B7DBF7CD92F26EA96C4`及限定复审暂不通过继续保留。

有限计划在末次应用回调后重新核对实际完整展示和运行时条件。手动接管覆盖准备read的未派发取消、实际在途排空和保存，宿主按所属run观察交接完成。公共有限预览查询保持方法及任务租约至外层返回。修前直接15例有13失败，补充宿主投影2项断言失败。取消状态夹具的实际显示完成等待、C4459及其他前提/观察错误分别保留。

43份最终关后主样本采用immutable观察，主库及原旁文件哈希前后一致。39份带空WAL和SHM，4份无旁文件。实际browser/P4维护链读回74/doc-2，显式撤销42/doc-3，隔离六轮共用根账本。原371项导出和旧358项保持，私有入口为0，预算及30秒门槛保持。

详见[整改合同](xuanshu-general-closeout-contract.md)、[验收与限制](validation/xuanshu-general-closeout-validation.md)、[匹配交付清单](xuanshu-general-assistant-delivery-manifest.md)和[单一入口](xuanshu-general-assistant-entry.md)。新identity由候选根delivery-identity.json索引，未冻结。启动器及监督器逐字复用，本轮未执行生产启动、真实凭据或Provider请求。真实质量包保持Proposed，默认未核价锁停。

BR1-KI-001继续未解决、用户接受非阻断，临时控件测试保持取消。原候选、历史首败和全部旁文件保留。仅清理本轮749项对象/资源中间件，源码、程序、日志/JUnit、必要图像、数据库及旁文件保全。后续仅进入再次限定独立复审和新的具体真实授权准备。

## 此前状态原文保留

# 通用应用助手首版候选已完成开发及自动验证

2026-11-11。候选根`build/xuanshu-general-assistant-20261111-7a2f90`已完成实现、自审整改及相称验证，等待限定独立复审。最终Light71/71、ASAN-Light71/71和Native31/31分别通过，生产普通入口PID22372实际exit0。左助手、稳定Profile、有限模型计划、应用专业上下文、根计量、手动接管、附件及依据展示齐备。真实browser读取及维护组件74/doc-2、撤销42/doc-3由隔离正式宿主链验证，六轮共用根账本。

普通生产只保存一个未确认提案，实际五块详情恢复后逐字一致，跨标签固定目标和草稿保持，取消保存并正常关闭。关后1任务/2消息/3事件/0工具，未进入模型请求路径。状态覆盖瑕疵经2项修前失败直接修复，首轮ASAN预算中断及启动身份模块缺失分别保留。最终代码和程序关系见[候选清单](xuanshu-general-assistant-delivery-manifest.md)，详细证据见[开发验收](validation/xuanshu-general-assistant-validation.md)。

日常唯一入口start-general.ps1已用新隔离Profile实际运行。默认未核价保持发送锁停，真实质量包Proposed。BR1-KI-001继续未解决、用户接受非阻断，临时控件测试保持取消。后续仅进行限定独立复审和新的具体真实授权准备。

## 此前记录保留

# 当前交接，通用应用助手首版连续开发进行中

2026-11-11。经用户授权的侧会话完整实施指令已核实并保存，当前任务是通用应用助手首版开发。PB-3具体真实包已经消费，当前没有待Key或待本人确认操作。候选根`build/xuanshu-general-assistant-20261111-7a2f90`，入口649文件已保全，前置546源码及四程序匹配。

已实现左助手、稳定Profile、同一Service模型与Coordinator桥接、专业参数准备和问题路由、共用根计量、真实browser及维护组件P4编辑/撤销、手动交接、文本/本地PNG附件及依据展示。直接生产页隔离业务用例已经读回74/doc-2和撤销42/doc-3，六轮均为替身。当前正在执行串行受影响回归、ASAN及交付核对，尚未形成最终交付identity或完成结论。无真实凭据访问、Provider调用或人工验收。

首轮运行及修正分别保留。初始详情脚本67574字节超过原64KiB单脚本限制，现分两段在同一视图全局运行。已修正计划阶段投影触发、新旧hash关联、撤销范围定位及手动状态下的新任务初始确认门。旧显隐夹具的滚动X仍按右侧定位，改用实际矩形，断言及30秒门槛保持。

后续继续固定完成矩阵，完成最终源码/程序/JUnit匹配、普通生产驱动、关后只读核对、启动入口、开发标准、使用说明、Proposed真实质量包及必要清理。临时控件测试保持取消。BR1-KI-001保持未解决、用户接受非阻断，实际不一致时停止受影响任务。不得使用子代理或computer use，不启动真实请求。

## 此前时点交接原文保留

# 当前交接，PB-3一次真实验证已完成并关闭

2026-11-11。PB3-REAL-20261111-6c4f82唯一真实SQLite隔离副本阅读已完成并正常关闭，限定业务闭环通过。唯一PID8976实际exit0；完整847字节正文和revision、固定范围及工具保存逐值对应。关后1任务、2消息、31事件、2工具，唯一completed；3次占用、非模拟传输入口和完整响应，unknown0，本地保守费用USD0.0016113，未结预留0。该具体包已消费，当前没有运行宿主、待Key或待确认操作。详[本次真实验证及限制](validation/xuanshu-page-reading-real-20261111-validation.md)和[执行证据](../build/xuanshu-page-reading-real-run-20261111-6c4f82)。

冻结快照identity 4A892228066D04EB09972594509892AF3B960CF32569DE33A125F11BEF611A42及546源码/195产物保持原身份，本轮完成具体批准、本人原生Key及确认、一次生产验证、正常关闭、immutable关后核对和文档归档。真实回复的核心特性与无待办结论符合原文，其中“嵌入式/进程内”属于未标注推断，来源声明存在一项质量瑕疵，单独保留。账户扣费、全程抓包和原站实时网页等边界保持。

后续新任务、网页和功能仅列Proposed，需独立具体授权，原默认入口仍未核价锁停。历史ASAN首败、BR1-KI-001、计量任意卡死及其他未验边界保留，临时控件测试维持取消。

## 以下各时点记录保留原文

# 当前交接，PB-3唯一任务等待本人可信确认

2026-11-11。PB3-REAL-20261111-6c4f82已明确批准，唯一PID8976，监督exec session5138。用户已原生保存Key，普通诊断显示本地可读取、profile2。唯一任务1864c87d3957d0b6bfd71dd50e28fbd8ccd21846d11ed6a5f6eb8418e41aaa34已持久化接纳phase3，尚未确认、请求未保存、model/tool_calls及占用/传输入口0。执行根build/xuanshu-page-reading-real-run-20261111-6c4f82。

history=[]，public-sqlite-zeroconf-v1，固定browser.view实例1、host b50ba334-04ba-4a6a-84fc-ad24b0811f97、open dd335699-e75e-4643-ae12-43fd215575ef、revision 77fbdb9e18262e8aa0c807ae275fbfdab249f027c46f7068e4831509755f70af、read scope，能力仅browser.body，字段body/revision。实际两块只读控件全文3000/1156字节已复制解析，初始request逐值和1608字节SHA 92D3A6DD5973BAA02BE219B7E13B85E5658E25E24A6C28239CB189DA503CCC1A与具体包一致。

确认前本日日期、四生产文件、授权包和12份官方依据再次一致。截图24-confirm-ready.png显示完整详情已展开，按钮位于右侧两块详情下面。下一步仅用户核对范围后本人点击一次“确认此任务外发范围（一次）”，回复“已确认”。助手不点此按钮，不重新提案/配置/启动，不读取Key或关前数据库。收到反馈后立即核对软件实际接纳，持续监督至终态保存，按授权普通关闭及关后取证。原监督session保持。

## 此前各时点交接，原文保留

# 当前交接，PB-3唯一真实会话等待用户原生Key

2026-11-11。用户已明确批准PB3-REAL-20261111-6c4f82，授权包SHA256 DE37BAC245BB0734161DD00583A118D96DFE3AE2C43630D6A366E3B803D0E8B6。启动前546源码/195产物及12份官方资料再匹配，唯一生产PID8976，监督exec session5138。执行根build/xuanshu-page-reading-real-run-20261111-6c4f82。普通界面模型全文deepseek-flash已经实际复制读回一致并保存，配置读回代次1，当前显示凭据未确认可用。

credential-pending.json已经建立，全部截图、剪贴板及自动输入暂停。下一步仅用户通过“原生窗口设置密钥”录入并保存Key，反馈“已保存”。收到后先只枚举本PID可见窗口/类确认原生窗口结束，再解除暂停，保持密码框和Key不可访问。尚未提案、可信确认或发送，关闭前不读DB。不重复启动或换库补额度，继承同一包、PID和监督session。

## 原交接按时点保留

# 当前交接，PB-3已冻结，本日具体包等待集中批准

2026-11-11。PB-3限定独立复审通过，匹配快照冻结记录与本地交付材料已齐备。identity 4A892228066D04EB09972594509892AF3B960CF32569DE33A125F11BEF611A42，546源码和195项程序/链接产物匹配，343条最终case对应既有Light168、ASAN168、Native7及分别记录的补项。新复审及[本日具体包](../build/xuanshu-page-reading-real-preparation-20261111-6c4f82/authorization-package.md)已归档，PB3-REAL-20261111-6c4f82仍Proposed。12份当日官方依据相同，三次本地计量exit0，三次总保守预留USD0.0327456。等待一次具体批准，当前没有执行目录、生产进程、Key或真实请求。默认入口仍锁停。

首轮ASAN五项摘要失败、两次期限根因未定位、首轮PDB缺失、96主样本和其他旁文件的范围限制、BR1-KI-001及计量任意卡死边界保持。临时控件测试维持取消。详[限定复审与冻结](validation/xuanshu-page-reading-independent-review.md)、[入口](xuanshu-page-reading-entry.md)和[清单](xuanshu-page-reading-delivery-manifest.md)。

## 本轮复审前交接，原文保留

# 当前交接：PB-3开发及离线验证完成，等待限定独立复审

2026-11-11。PB-3公开SQLite Zero-Configuration隔离副本阅读开发、自审和离线验证完成，等待限定独立复审。候选根build/xuanshu-page-reading-20261111-a18d74，identity SHA256为4A892228066D04EB09972594509892AF3B960CF32569DE33A125F11BEF611A42。原始HTML4209字节、实际DOM正文847字节；同一正式宿主/Service/browser/read/存储链保持。最终逐项Light168、ASAN168、Native7及清理后生产公开read各另次1/1，运行与首败分账。96份阅读主样本无旁文件、immutable前后哈希一致；另10旁文件（6非空）保留。真实验证包Proposed，默认未核价锁停，本轮没有真实模型请求、凭据读取或人工验收。前置PB-2批准已消费。

源码546份，代码/样本增量17项，其中产品仅4份私有场景/恢复/任务路由，生产358项导出保持。资料/任务/预览/保护范围见[合同](xuanshu-page-reading-contract.md)、[验收](validation/xuanshu-page-reading-validation.md)、[入口](xuanshu-page-reading-entry.md)及[待授权准备](xuanshu-page-reading-real-proposal.md)。新identity不继承PB-2冻结，没有本轮生产待Key/待确认步骤。

Light首轮167/168，reopen新增夹具期待错误修正后79/79替代；摘要完成屏障补项17/17。ASAN首轮163/168，全部42项阅读通过，摘要补项17/17。两份50000字节及112/48页保留。预算/期限首次触发因素仍未定位，原始日志、程序/源码/自有库和旁文件保留。BR1-KI-001与计量任意卡死边界保持，临时控件测试维持取消。清理仅本轮760项编译/包装暂存，程序/依赖记录及证据清单匹配。

## PB-3开始前的历史交接

# 当前交接：PB-2一次真实正文验证已完成

> 2026-11-11。PB-2匹配快照冻结记录已归档，PB2-REAL-20261111-8f3c2a唯一真实正文任务已完成并正常关闭。生产PID28300实际exit0；147字节正文、revision、12/9/3、林青与周明及复核三台设备逐值对应。关后1任务、2消息、31事件、2工具，唯一completed；3次占用、非模拟传输入口和完整响应，unknown0；本地保守费用USD0.0010926，未结预留0。该包已消费，当前没有本轮运行进程、待Key、待确认或继续执行操作。后续新真实任务需独立具体授权，原默认模板维持Proposed锁停。 详见[本次完整结果](validation/xuanshu-page-body-policy-real-20261111-validation.md)和[证据目录](../build/xuanshu-page-body-real-run-20261111-8f3c2a)。

## 以下历史记录按各自时点保留

# 当前交接：PB-2唯一任务等待本人可信确认

2026-11-11。PB2-REAL-20261111-8f3c2a已批准，唯一PID28300，监督exec session11718。用户已原生保存Key，软件本地可读取/profile2。唯一任务360f30e612e4e93903911ced6bfe5ffd16b15339a24eb6a719f1a053a362a9f5持久化接纳phase3，history=[]、dom-body-v1、固定browser.view/epoch/revision/read scope和browser.body/body+revision匹配，未确认、请求未保存、占用及传输入口0。两块实际只读控件全文3000/1094字节已复制解析，初始request逐值及统一序列化1560字节SHA FB8F7A8A7703B6280C194A1D451C449997DEA9F1C3C14FC4EB3486CBA173DD69与包一致。

确认前日期、四生产文件及12官方依据再次一致。完整详情已展开，按钮当前在右侧两块详情下面。下一步仅用户核对实际范围后本人点击一次“确认此任务外发范围（一次）”；助手不点、不再提案/配置/读Key/重启/换库，关前不开DB。用户“已确认”后立即核对软件实际接纳及运行，连续监督、普通关闭和关后取证。原文历史状态保留。

# 当前交接：本日PB-2唯一会话等待用户原生Key

2026-11-11。PB2-REAL-20261111-8f3c2a已明确批准，唯一PID28300，监督exec session11718。模型实际全文deepseek-flash已复制核对并通过普通界面保存，界面配置读回代次1。当前执行根build/xuanshu-page-body-real-run-20261111-8f3c2a，credential-pending.json已建立，全部截图/剪贴板/输入驱动暂停。下一步仅用户点击正式“原生窗口设置密钥”并录入保存。用户反馈后先只枚举本PID可见窗口/类确认原生窗口结束，再解除暂停，不读密码框或Key。尚未提案/可信确认/发送，关前不读DB，不重启或重复申请包批准。

## 先前状态按原时点保留

# 当前交接：PB2-REAL-20261111-8f3c2a已批准并唯一启动

2026-11-11。用户已明确批准本日具体包，授权包SHA256 2F81761A899F853D31D14AD109BC1DF4812944EF15DBB074697457B2EC9C1096。唯一生产PID28300，监督exec session11718，执行根build/xuanshu-page-body-real-run-20261111-8f3c2a。启动前537源码、四生产文件及12份官方资料均匹配。当前由助手普通控件配置模型及提案，尚未原生Key或逐任务可信确认，关前不开DB。不要重复启动/换库/补额度或再次请求同一批准。

## 先前状态按原时点保留

# 当前交接：PB-2已冻结，本日具体包等待集中批准

> 2026-11-11当前状态：PB-2匹配快照已限定独立通过，冻结记录已归档。见[冻结记录](validation/xuanshu-page-body-policy-independent-review.md)及[本日准备](validation/xuanshu-page-body-policy-real-preparation-20261111-validation.md)。PB2-REAL-20261111-8f3c2a仍Proposed，等待一次具体批准；没有生产执行或真实请求。11月10日正文包已失效，以下原文按当时状态保留，不构成当前操作或授权。

# 当前交接：PB-2正文验证保护候选等待限定独立复审

2026-11-10。PB-1原匹配快照冻结记录已落盘；新策略开发/离线验证完成，候选根build/xuanshu-page-body-policy-20261110-d42b8e。见[新入口](xuanshu-page-body-policy-entry.md)、[验证与首败](validation/xuanshu-page-body-policy-validation.md)、[交付清单](xuanshu-page-body-policy-delivery-manifest.md)。新identity以候选根delivery-identity.json为准，未冻结或独立通过。

Light closure-final201/201＋加强后的36/36替代对应早期用例；ASAN direct-final36/36＋compatibility-final165/165；Native7/7分别记录。产品仅4份私有运行时场景路由/双标记恢复，公有头/DDL/来源/预算及358导出保持。72份最终新主样本及旁文件、首败、匹配源码/程序/日志和清理在新清单。

[PB2-REAL-20261110-d42b8e](../build/xuanshu-page-body-real-preparation-20261110-d42b8e/authorization-package.md)Proposed，公开依据和新输入完整计量已核实，三次新保守预留USD0.0327456；新策略独立复审和本包集中批准仍未完成。没有生产人工验收或真实执行进程，没有待用户录Key/确认步骤，不创建执行目录或续跑旧包。原BR2-REAL-20261110-111dd1已消费。BR1-KI-001、PB-1首超时和其他历史风险保留，取消的临时控件测试不恢复。

## 本轮开始时状态（保留）

# 当前交接：PB-2正文验证策略开发中

2026-11-10。PB-1原快照限定复审通过及冻结归档已落盘，原identity保持。新根build/xuanshu-page-body-policy-20261110-d42b8e独立开发，实施前合同docs/xuanshu-page-body-policy-contract.md。当前仅隔离离线开发；未批准任何新真实包，不读取Key或发送。旧BR2-REAL-20261110-111dd1已消费，不续跑。

## 先前记录（原文保留）

# 当前交接：PB-1正文摘要候选等待限定独立复审

2026-11-10。当前网页DOM正文只读摘要开发及离线逐项验证完成。新根build/xuanshu-page-body-20261110-a64c28；[当前候选入口](xuanshu-page-body-entry.md)、[增量合同](xuanshu-page-body-contract.md)、[验证/首败/未验](validation/xuanshu-page-body-validation.md)。最终身份以新根delivery-identity.json为准，不自动冻结。

正文来自实际browser HTML解析，独立browser.body/body+revision，不扩大snapshot.state。Light131/131、隔离另次5/5；ASAN130/131、READY初始化20.03秒首超时同程序原门槛补项1/1、隔离另次5/5，最终逐项131/131；Native7/7。首超时根因未定位、非阻断风险保留。正式页面选项缓存先失效后提交、能力/字段for循环减少JS栈，临时异常探针已移除。关后10主样本逐值/哈希核对；普通正文任务没有RPC配额，不作HTTP或费用账单声称。

没有待用户操作或本轮生产/人工验收。真实正文验证策略不支持当前新范围，仍Proposed锁停；须独立复审、必要新策略接入授权、核价/计量及独立集中授权和逐任务可信确认。不得复用旧包/额度/库/凭据。BR-2已完成状态保持；BR1-KI-001未解决，临时控件测试维持取消。

## 先前阶段记录（原文保留）

# 当前交接：BR-2阶段已完成

2026-11-10：**最终限定收口复核结论已归档，BR-2本阶段已完成。没有进行中的本阶段宿主/Service，没有待用户录入Key、待确认或待继续执行的操作。不要续跑下方历史流程。**

唯一当前交付入口：[BR-2只读交付入口](xuanshu-br2-discovery-entry.md)。已冻结根为`build/xuanshu-br2-discovery-20261110-7e4a96`，delivery-identity.json SHA256：

`3DFFCFC515A9958A69396AE703307D6B9E84575D63DD921322D1882E5086F545`

[最终限定收口复核归档](validation/xuanshu-br2-discovery-final-closeout-review.md)承接上一轮只读复核：限定收口通过，本地材料齐备，必须修正项为空。[匹配快照冻结记录](validation/xuanshu-br2-discovery-independent-review.md)和[真实业务限定验证](validation/xuanshu-br2-discovery-real-validation.md)分别保留原身份及范围。本轮仅必要身份核对和文档更新，不是重新测试或完整复审。

具体包BR2-REAL-20261110-111dd1已消费并关闭，唯一PID35088实际exit0。真实空分类发现后执行browser.snapshot；标题、browser:start和框架revision与工具及保存记录一致。关后1任务/2消息/31事件/2工具，唯一completed；3次占用/非模拟传输入口/完整响应，本地保守费用USD0.0014961不是账户账单。已有新库immutable核对结论保持，本轮未重开数据库。

不复用旧包、命令、库、额度或凭据引用，不追加请求；默认启动模板仍Proposed锁停。当前成功不证明网页正文摘要、业务版本或全部系统通过。BR1-KI-001未解决、用户接受非阻断；历史首败和旁文件保全，临时页面控件测试维持取消。

后续仅Proposed“当前网页正文只读摘要”。需另立能力、数据范围和保护规格；开发与真实调用分别等待独立授权。本轮不盘点开发、不核价、不准备新包、不进入其他阶段。

## 历史记录（保留原文）

以下内容按各自记录时点保留。旧的“待批准”“待确认”“执行中”和“下一步”均为历史，不构成当前操作或授权；当前状态以上方阶段关闭记录为准。

---

> 2026-11-10本次已完成：[BR2-REAL-20261110-111dd1真实业务只读限定验证通过](validation/xuanshu-br2-discovery-real-validation.md)，目录分类匹配快照冻结记录和本地交付收尾已完成。唯一PID35088正常exit0；实际空分类发现后执行browser.snapshot，标题/browser:start/revision逐值一致。关后1任务/2消息/31事件/2工具、唯一completed，3次占用/非模拟传输入口/完整响应，unknown0；本地保守绑定价USD0.0014961，不是账户账单。该包已消费，不再启动/提交/确认/读Key；后续真实调用需新具体授权。原模板仍Proposed锁停，旧真实失败/遗留保持。以下历史状态各按原时点保留。

> 2026-11-10本次已运行至终态：BR2-REAL-20261110-111dd1，PID35088，监督session99700；用户报告“已确认”，软件确认/确认保存/模型请求保存为是。任务1e17e5a8892b3e906ac06f91b9e64a4ea164e21f8f49d78df8d7f76de893cd07completed，界面终态保存是，3占用/3非模拟传输入口/3完整响应、unknown0，保护business_task_finished锁停。回复含正确标题/browser:start/revision，仍须关后工具逐值验证才下业务结论。下一步仅助手WM_CLOSE并等实际退出，随后本轮库immutable只读；不追加、重启或换库。

> 2026-11-10当前交接：BR2-REAL-20261110-111dd1已批准，唯一PID35088，监督session99700；用户已原生保存Key，界面本地可读取/profile2，未证明服务端认证。唯一任务1e17e5a8892b3e906ac06f91b9e64a4ea164e21f8f49d78df8d7f76de893cd07已持久化接纳phase3，history=[]、固定browser.view/epoch/revision/read scope、browser.snapshot及revision/state均匹配。accepted-diagnostic额度/传输入口0、未确认/请求未保存。两块实际控件全文已复制解析，初始request SHA 3B2229C22AF89CF0D5948573EC7FD6873E7B5629E7F578342B7BDAE7E0B9BB0B与本包1523字节正文一致。确认前12官方资料和四程序再次匹配。当前右侧完整详情及确认按钮已定位，下一步仅用户核对后本人点击一次；助手不点击，不再提案/配置/录Key/重启/换库。关前不读DB。

> 2026-11-10当前交接：BR2-REAL-20261110-111dd1已批准，唯一PID35088，监督session99700。模型实际全文deepseek-flash已复制读回，保存后界面配置读回代次1。尚未提案/可信确认/发送。当前交用户原生Key；执行根credential-pending.json已建立，暂停全部截图/剪贴板/输入驱动。用户反馈完成后先仅枚举本PID窗口确认原生窗已关闭，再解除暂停，勿读取密码框/Key或重复录入。外部OCR小字定位和PowerShell UTF8兼容问题已独立保留，未改产品/重启。

> 2026-11-10执行中：用户已明确批准BR2-REAL-20261110-111dd1，仅本日一次。唯一生产PID35088，监督exec session99700；执行根build/xuanshu-br2-discovery-real-run-20261110-111dd1。日期、526源码/四生产文件及12份当日官方依据均匹配；批准记录和唯一命令已保存。当前先由外部普通控件驱动配置模型，再交用户原生Key，尚未提案或可信确认。不要重启/换库/追加额度，不读取Key；原包失败保留。

> 2026-11-10最新准备：目录分类快照冻结记录已落盘；[BR2-REAL-20261110-111dd1具体授权包](../build/xuanshu-br2-discovery-real-preparation-20261110-111dd1/authorization-package.md)已准备，尚未批准。执行目录/可执行监督命令未创建，不启动、读凭据或发送。用户仅负责本包批准、必要原生Key及一次完整范围可信确认，其余普通操作和收尾由助手处理。[准备记录](validation/xuanshu-br2-discovery-real-preparation-validation.md)。旧真实失败及旧包消费保持。以下历史状态不自动继承。

> 2026-11-10最新：目录分类与池缩容限定独立复审通过，冻结记录已落盘；identity 3DFFCFC515A9958A69396AE703307D6B9E84575D63DD921322D1882E5086F545匹配。详[冻结记录](validation/xuanshu-br2-discovery-independent-review.md)。新真实重验准备BR2-REAL-20261110-111dd1尚未批准，执行目录未创建、不启动/读凭据/发送。用户仅负责集中批准、必要原生录Key及一次可信确认；普通配置/任务/范围、监督和关闭由助手处理。旧包消费且业务失败的事实保持。以下旧状态按原范围保留。

> 2026-11-10分类契约限定整改候选已完成开发、自审及离线验证。新根build/xuanshu-br2-discovery-20261110-7e4a96，identity 3DFFCFC515A9958A69396AE703307D6B9E84575D63DD921322D1882E5086F545。分类说明明确空分类只枚举允许read，非空无匹配返回明确提示；直接相关的私有池缩容越界已用独立哨兵和空白名单ASAN因果定位后最小修复。最终Light118/118、ASAN118/118、Native49/49分次通过。
> [开发验收](validation/xuanshu-br2-discovery-validation.md)、[候选入口](xuanshu-br2-discovery-entry.md)、[规格](xuanshu-br2-discovery-contract.md)。6项代码增量、526源码、158程序/链接/调试资源匹配；2份full-meter关后库各1任务/2消息/31事件/2工具，实际标题/来源/revision及3轮538/1129/1471计量与body hash保持。新候选未独立通过/冻结/真实调用；旧真实包已消费、原业务真实验收仍失败，不重开旧库。策略Proposed锁停，下一步仅限定独立复审，再考虑新的真实授权。47旁文件含5份非空WAL保留；330个obj已按安全字面清单删除。BR1-KI-001和历史首败保持，临时控件测试继续取消。以下旧状态保留历史范围。

> 2026-11-10本次真实业务验证已结束：包BR2-REAL-20261110-4da059已明确批准并仅执行一次，PID23608实际exit0。唯一任务运行时completed并保存，但只执行category=browser的目录查询，browser.snapshot注册于browser.read，实际返回空目录且未调用snapshot，标题/来源/业务返回revision未取得。本次真实业务验收及交付收口暂不通过；原限定冻结和离线结论保持原适用范围，不追加请求、不重启、不修改产品。
> [本次真实验证及关后证据](validation/xuanshu-br2-meter-real-20261110-validation.md)。两轮计量496/592与usage相同，2次占用/2个非模拟传输入口/2个完整响应，本地绑定价627600 nano USD，未核对账户账单；关后1任务/2消息/21事件/1目录工具，无WAL/SHM，immutable前后哈希一致。唯一包已消费，不再使用剩余额度；后续分类契约修正/离线回归及任何真实补测均须独立授权。下文执行中/未批准等为历史状态。

> 2026-11-10真实验证执行中：唯一PID23608/监督26095，包BR2-REAL-20261110-4da059已明确批准。已接纳唯一任务f07c794e22ca5bfaadfcece1e013c7f81cb6eca84e38a55025d9d8f450448089，生产脱敏诊断核对phase3/committed、deepseek-flash/profile2、history=[]、browser.view及固定epoch/revision/read scope、browser.snapshot和revision/state均匹配；BR-2 ready、逐轮计量启用、占用及传输入口0，尚未可信确认。证据build/xuanshu-br2-meter-real-run-20261110-4da059。当前下一步只让用户查看完整请求详情，不因display_valid代次73签发资格；之后复核当天依据才引导用户本人一次确认。不要再次保存/录Key/重启/换库，关前不开数据库。下文等待批准或未提案记录保留为历史。

> 2026-11-10执行中：用户已明确批准BR2-REAL-20261110-4da059，唯一宿主PID23608、监督session26095仍运行。证据位于build/xuanshu-br2-meter-real-run-20261110-4da059；原冻结程序/脚本不改。用户已报告deepseek-flash配置读回、原生凭据本地可读取及代次2，尚不代表服务端验证。当前已展开新任务范围并看到browser.view，下一步选择该目标；唯一提案与逐任务可信确认尚未进行本轮引导，软件/数据库计数仍待正式脱敏诊断及关后核对。不要重启、换库、再次索要集中批准或读取Key。下文等待批准记录保留为历史。

> 2026-11-10当前交接：BR2-REAL-20261110-4da059为唯一当日Proposed包，尚未明确批准。本计划转发不是授权，11月9日包已过期。冻结identity C5A430D7DA479E7706043741FE11D66EE95B976C51B849ADA92A26732BF62596匹配，12份当天官方正文/编码和价格无差异，三次保守预留USD0.0294912。
> [当天准备、完整固定参数及监督命令](validation/xuanshu-br2-meter-real-preparation-20261110-validation.md)。命令仅静态审阅、未动态测试；原脚本/Proposed模板锁停，新执行目录/策略未创建，不启动、不读凭据、不发送。ASAN首败、failed终态、release/PENDING边界及BR1-KI-001保持，取消测试不恢复。下文历史记录保留。

> 2026-11-09当前交接：BR-2逐轮计量限定验收通过、匹配快照冻结记录落盘，本地交付材料齐备。identity C5A430D7DA479E7706043741FE11D66EE95B976C51B849ADA92A26732BF62596；官方模型/协议/价格/编码已按本日核对，BR2-REAL-20261109-4ffee5包等待明确批准，不启动、不激活、不读凭据、不发送。
> [冻结记录](validation/xuanshu-br2-meter-independent-review.md)、[交付清单](xuanshu-br2-meter-delivery-manifest.md)、[准备与具体集中包](validation/xuanshu-br2-meter-real-preparation-validation.md)。ASAN首败、计量中failed、release子进程/PENDING边界及BR1-KI-001保持；取消的临时控件测试不恢复。以下原文保留历史状态。

> 2026-11-06 BR-2逐轮完整计量开发完成。候选build/xuanshu-br2-meter-20261106-1f8148接入官方完整Responses转换及每轮实际body发送前门，新增组件独立128MiB限制，原确认、Service及预算合同保持。Light主轮127/127，ASAN主轮121/127并另次原程序6/6对照，最终计量取证补项各20/20，Native38/38，组件15/15。首轮frontend中断/超时根因未定位，旧计量日志的中文路径截断已修为UTF-8，相关旧源码/程序保留。新身份等待限定独立复审，真实策略仍Proposed锁停。详[计量开发记录](validation/xuanshu-br2-meter-validation.md)及[候选入口](xuanshu-br2-meter-entry.md)。原冻结及历史结论保持其原适用范围。

> 2026-11-06 BR-2官方编码链续查。已找到DeepSeek官方Recipe，其说明与官方API处理逻辑对齐，并核对Flash-1017与Recipe普通文本tokenizer字段一致。新阶段为build/xuanshu-br2-real-preparation-20261106-d83d0e/encoding-f696f7，固定初始内容两种字段顺序的模型参考计数均496，对应完整4096输出预留USD0.005064；完整Recipe转换器和动态续轮未执行，不能据此设置全任务ceiling。剩余明确范围是完整Responses计量对齐及动态每轮发送前门，修改冻结发送链需独立开发授权、相称离线验证和再次限定独立复审。原策略verified=false、ceiling=0，真实会话未创建，未启动宿主、读凭据或请求。详[编码链记录](validation/xuanshu-br2-encoding-preparation-validation.md)。原准备阶段身份和失败材料保留，原本地冻结结论保持，临时控件测试保持取消。以下历史原文保留。

> 2026-11-06 BR-2真实验证前置核实已完成一次明确对照，停止于可靠input计费上界。原冻结identity 1EF3D1643AE1DF94ED6B6CC163F061F1376D82E3A5CC540823F1E197687105FB、433源码/113产物/5启动清单/30关联清单匹配。官方确认deepseek-flash映射Flash-1017、POST /responses及峰时输入300/输出1200 nano USD每token。USD0.10的单轮input预算容量为316949，官方1M上下文及tokenizer演示未提供覆盖Responses封装/工具/续轮的更小可靠上界，verified保持false、ceiling=0。新准备目录build/xuanshu-br2-real-preparation-20261106-d83d0e，辅助12/12仅验证准备算术和锁停。执行会话尚未创建，无PID/退出码，未读取凭据、启动宿主或发送请求。详[前置核实及最小下一步](validation/xuanshu-br2-real-preparation-validation.md)。原本地冻结结论保留，真实验证及其交付收口尚未满足，集中授权包尚未达到提交条件。原失败、全部旁文件、BR1-KI-001及取消的临时控件测试状态保持。以下历史原文保留。

> 2026-11-06 BR-2费用恢复匹配快照冻结与本地交付收尾。承接上一轮限定独立复审通过、必须修正项为空的结论，已核对匹配identity 1EF3D1643AE1DF94ED6B6CC163F061F1376D82E3A5CC540823F1E197687105FB及433项源码、113项程序产物、5项launch-files和30份关联清单。冻结记录与本地交付材料已落盘，当前入口为build/xuanshu-br2-fee-20261106-54d9a8/start-readonly.ps1，见[独立复审及冻结](validation/xuanshu-br2-fee-restore-independent-review.md)、[交付清单](xuanshu-br2-delivery-manifest.md)、[待授权包](xuanshu-br2-real-proposal.md)。

> 本轮仅文档及只读身份核对，沿用已核对的原离线及关后证据，原identity、清单、delivery-final、程序、失败和全部旁文件保留原字节。原identity.status中的待复审属于封装时状态，当前文档记录后续冻结。BR1-KI-001仍未解决且用户接受非阻断；44份其他旁文件含6份非空WAL、历史首败和取证限制独立保留。真实验证需官方协议/价格/计费规则及可靠上界核实、唯一新隔离库策略和监督配置、独立集中授权及逐任务可信确认；授权包Proposed且入口锁停。临时页面控件测试保持取消。以下历史原文保留。

> 2026-11-06 BR-2旧策略费用恢复兼容限定整改完成，等待再次限定独立复审。新根build/xuanshu-br2-fee-20261106-54d9a8。产品仅费用恢复及局部安全算术，旧RPCP/RPCB合法known可高于预留或达/超阈值，保留事实并显式恢复锁停；BR-2可靠上界保持，阈值后矛盾占用拒绝。有效修前13例中5失败、8对照通过，最终Light107/107、ASAN107/107、Native36/36分次通过；27组合法恢复逐值相同，99份主样本关后immutable哈希不变。初始编译/包前提/夹具异常及44份其他旁文件独立保留，320份编译中间件清理。详[费用恢复验收](validation/xuanshu-br2-fee-restore-validation.md)、[规格](xuanshu-br2-fee-restore-contract.md)、[本地入口](xuanshu-br2-entry.md)。前置恢复快照后续复审暂不通过事实及历史限制保持。BR1-KI-001未解决且用户接受非阻断，启动8+2按身份复用，Proposed入口未核价锁停；未真实调用/人工验收，未自行独立通过或冻结。以下历史原文保留。

> 2026-11-06 BR-2恢复保护整改完成，等待再次限定独立复审。新根build/xuanshu-br2-restore-20261106-3c8d42。READY前流式核对开启/任务/占用/发送/usage/费用关联，不完整或误分类打开失败，完整BR-2恢复原计数并锁停。最终Light94/94、ASAN94/94、Native23/23分次通过，48份关后自有库immutable核对不变。修前独立对照两例均证明旧运行时READY且接纳新提案；早期夹具目录复用导致首轮红库初始字节身份缺失的限制保留，新增自审两类矛盾首败同样保留。启动监督8+2只按身份复用。详[整改验收](validation/xuanshu-br2-restore-validation.md)、[恢复合同](xuanshu-br2-restore-contract.md)、[入口](xuanshu-br2-entry.md)。BR1-KI-001未解决且用户接受非阻断；未真实调用/人工验收，价格及计费上界未核实，Proposed入口保持锁停。没有自行独立通过或冻结。以下历史原文保留。

> 2026-11-05 BR-2最新候选：验证专用3次总额度、10分钟及费用/unknown硬停止开发和离线验证完成；Light73/73、ASAN73/73、Native23/23分次通过。新身份与旧BR-1分开，未独立通过/冻结。当前入口价格及可靠计费上界未核实，真实发送锁停，待集中授权。BR1-KI-001仍未解决、用户接受非阻断。见[开发验收](validation/xuanshu-br2-validation.md)、[入口](xuanshu-br2-entry.md)、[Proposed授权包](xuanshu-br2-real-proposal.md)。下文为历史记录，不改写旧首败或旧结论；已取消临时控件测试不恢复。

> 2026-11-05 BR-1一致性整改：启动异常取证已完成并以隔离辅助进程8/8验证；模型名少字符仍未定位，冻结阻断保留。新增逐边界探针及相称回归通过不能替代原根因；新候选build/xuanshu-br1-consistency-20261105-f4c92a仅诊断/监督交付。详[记录](validation/xuanshu-br1-consistency-validation.md)。不进入真实调用或人工验收。

> 2026-11-05 BR-1业务接入候选：实际受限浏览器读取新合成交接单标题/地址/revision，正式宿主隔离模型闭环及生产公开read验证完成。Light43/43及最终业务6/6；ASAN首轮41/43、补项及最终业务通过，持久化摘要少字符首次原因未定位，独立保留。新交付build/xuanshu-business-read-20261105-8c6e21等待限定独立复审，不继承DH-1冻结。见[开发验收](validation/xuanshu-business-read-validation.md)、[业务入口](xuanshu-business-read-entry.md)。未读取真实凭据、调用Provider或启动人工验收。真实验证请求总数硬门仍需独立授权；临时控件测试取消状态保持。

> 2026-11-05 DH-1冻结与只读交付收尾：承接此前限定独立复审通过、必须修正项为空的结论，419项源码/74项产物及清单身份核对匹配，快照65823A92BAAF49CE2623F5865F2C47B678215DD813D7E8EDDC8725980107A532的冻结记录已落盘。详[复审记录](validation/xuanshu-daily-host-independent-review.md)、[交付清单](xuanshu-readonly-delivery-manifest.md)、[使用说明](xuanshu-readonly-user-guide.md)。仅文档收尾与身份核对，无新测试、启动、凭据或请求；未重新读取数据库。当前只读交付材料齐备，不等于正式发布或整个系统完整验收。后续有Key验证、产品改进仅Proposed，需独立授权。
> 下文保留各阶段原始记录；其中“等待复审”“未冻结”“方案待确认”及旧快照入口属于历史状态，当前交付入口以上述清单为准。临时页面控件测试已取消，不恢复、不列为待办或阻断；历史失败和独立未验边界保留。

> 2026-11-04 DH-1自动界面验收通过：匹配65823A92…107A532生产宿主，仅外部合成输入；一任务本地保存、范围/详情显隐恢复、草稿分离、取消保存及exit0完成。关后1任务/2消息/3事件/0工具，未进入模型请求路径证据齐全；无全程网络遥测，不是真实人工、Provider或独立复审，不自行冻结。新根build/xuanshu-dh-auto-20261104-07e6b3，详[自动界面验收](validation/xuanshu-dh-auto-validation.md)。未改产品/构建，取消的临时控件测试不恢复；以下历史保持。

> 2026-11-04 DH-1：正式只读助手界面开发及自动验证完成，已具备限定独立复审条件。产品仅host.html；日常当前任务/草稿、独立范围、设置/开发诊断折叠，原Service/可信确认/预算不改。Light与ASAN各closure48/48、控制补项各6/6、复制各另次1/1；首败与编排/存储关闭夹具/复制error5限制保留。新根build/xuanshu-daily-host-20261104-c82f19，详[开发验收](validation/xuanshu-daily-host-validation.md)、[合同](xuanshu-daily-host-contract.md)。未独立通过、未正式冻结、未启动人工/真实Provider，取消的临时控件测试不再待办。以下历史原文保留。

> 2026-11-04只读版本独立准备已完成：25清单/418源码/53产物匹配原冻结身份；当前待办已梳理，正式页面信息结构、静态草图及实现/验证矩阵已形成Proposed。详[方案](xuanshu-daily-host-proposal.md)、[准备核对](validation/xuanshu-readonly-preparation-validation.md)。等待交互方向确认，未改产品/运行测试/启动程序/读取Key。临时页面控件测试已取消，历史记录保持。

> 2026-11-04用户纠正：临时页面控件测试已取消，不是暂缓，不再安排或列为后续待办/交付阻断。已完成输入观察与exit0证据保留。以下此前“待补”类表述以本条纠正为准；不自动开展正式页面开发或其他验收。

> 2026-11-04第三批收尾：用户确认IME/编辑/Tab部分路径，决定停止临时页面细粒度控件验收；未宣称整批通过。PID33920实际exit0，关后0任务/消息/事件/工具，无模型配置，库哈希未变。未改产品、未测试、无Key/Provider。详[第三批最终记录](validation/xuanshu-uha-manual-third-validation.md)。停止，不自动启动下一批或实现页面调整。

> 2026-11-04 UH-A第三批：准备身份匹配，新根build/xuanshu-uha-manual-third-20261104-6f2c81；微软输入法/单屏/无屏幕阅读器，长时未约定。产品未改、无测试/Key/Provider。当前待唯一监督启动及逐步人工，详[第三批](validation/xuanshu-uha-manual-third-validation.md)、[交付准备](xuanshu-readonly-delivery-readiness.md)。以下历史保持。

> UHS-1简短零Key、零联网人工复核最终通过：范围/详情/草稿/取消保存已实际观察，PID19548 exit0，关后1任务/2消息/3事件/0工具，唯一cancelled，无确认和模型请求。新根build/xuanshu-uhs-cache-manual-20261104-b19f62；详[最终记录](validation/xuanshu-uhs-cache-manual-validation.md)。冻结B5FD964B…ABDC23保持；未进入其他阶段，历史和人工延期限制保留。

> UHS-1最新：限定独立复审及匹配快照冻结已追记；简短人工复核准备完成，结果待用户操作。新根build/xuanshu-uhs-cache-manual-20261104-b19f62，最多一次启动，不设Key、不确认外发。详[本轮记录](validation/xuanshu-uhs-cache-manual-validation.md)。以下历史保持。

> 2026-11-04 UHS-1缓存最新整改：两项显示缓存限定整改已完成，自动验证通过，已具备再次限定独立复审的条件。产品仅host.html失效/构建/成功提交顺序；详情、分页及父子缓存中断后同内容/任务切回恢复，原授权与预算不变。最终Light45/45、ASAN45/45，复制各另次1/1；有效修前失败、测试包装栈问题/空访问、翻页30.11秒首超时独立保留。新根build/xuanshu-uhs-cache-20261104-4a9e72，详[整改验收](validation/xuanshu-uhs-cache-closeout-validation.md)、[合同](xuanshu-uhs-cache-closeout-contract.md)。旧摘要候选复审暂不通过记录保持；新快照未独立通过、未冻结、未启动人工、无Key/Provider请求。以下历史原文保留。

> 2026-11-04 UHS-1最新：预览与范围摘要易用性开发及相称自动验证完成，新根build/xuanshu-uha-summary-20261104-d7c518。五个常驻中文区、任务范围定位、同源分页摘要、草稿/固定目标分离、持久化缺项“未记录”；分块中断恢复与原完整展示门保持。Light34/34、ASAN34/34，复制各另次1/1；首败、受限复制error5及期限风险独立保留。运行时/公共接口/来源/DDL/依赖/限额未改，新DLL重新链接不继承旧身份。详[开发验收](validation/xuanshu-uha-summary-validation.md)、[增量合同](xuanshu-uha-summary-contract.md)。当前等待限定独立复审及后续简短零Key/零联网人工授权；没有启动正式宿主、读取Key或真实请求，不能写成冻结/人工通过。以下各历史状态保留。

> 2026-11-04跨标签补测最终通过：同冻结生产web_counter只作被动合成第二标签；用户截图证明RP-C→Web Counter #2→RP-C，两个实时诊断task/model逐值一致，run/client/instance/host-open epoch/resource/r1/read scope及字段不变，仅展示代次21→23。返回后取消，PID13776实际exit0；新库1任务/2消息/3事件/0工具，唯一cancelled/committed终态，无确认/请求保存/工具派发；DB SHA25697886179…8B3BC0D，immutable前后不变、无WAL/SHM。承接此前第二批两次exit0及历史选择/取消/恢复通过证据，集中人工第二批零Key、零联网限定验收通过。不是全程抓包、真实Provider或系统完整验收。 详[跨标签最终记录](validation/xuanshu-uha-cross-tab-validation.md)。新证据根build/xuanshu-uha-cross-tab-20261104-d3c057；未读旧库/凭据，产品417源码/153程序/21身份链接匹配，已停止，不自动进入新阶段。

> 2026-11-04跨标签补测已启动：匹配正式宿主PID13776、监督PID20744/会话57720，两生产应用模块身份匹配。新隔离根build/xuanshu-uha-cross-tab-20261104-d3c057，当前仅等待用户确认两个标签并选择rpc_memory；未提交/配置/Key/确认，退出码待实际关闭。不再有自动重启额度。详[补测记录](validation/xuanshu-uha-cross-tab-validation.md)。

> 2026-11-04跨标签补测准备：已核对同冻结快照的生产web_counter.uapp（导入ui_framework.dll），仅查看合成页面及切标签，不点击+1或调用其能力；排除测试DLL包。417源码/153程序/21关联清单匹配，新隔离根build/xuanshu-uha-cross-tab-20261104-d3c057。此前第二批两次exit0及已通过分项保持，本轮只补固定目标动态切换，不重新做历史/恢复；实际人工与退出尚待执行，不预填整批通过。详[跨标签补测](validation/xuanshu-uha-cross-tab-validation.md)。

> 2026-11-04第二批收口：本地合成历史/仅A选择、未确认模型提案/预览、主动取消、同库恢复及persisted诊断分项通过；两次PID61528/64548均实际exit0。关后两次均3任务/6消息/9事件/0工具，整库/配置哈希及全部记录不变，无确认/模型请求/工具重放。指定rpc_memory单实例，跨标签固定目标未覆盖，故整批为部分完成，不宣称全部通过；预览定位困难及首块取证限制保留。详[第二批最终记录](validation/xuanshu-uha-manual-second-validation.md)。已停止，不再启动或进入有Key/真实调用，原冻结和历史风险保持。

> 2026-11-04第二批恢复检查已启动：初始PID61528实际exit0，关后3任务/6消息/9事件/0工具，A/B completed、模型唯一cancelled；确认/模型请求均未发生，目标与history=[1]一致。本轮唯一同库恢复PID64548、监督会话90761，生产DLL匹配，当前等待用户打开AI查看记录；不再有额外启动额度。根build/xuanshu-uha-manual-second-20261104-89bc95，详[第二批记录](validation/xuanshu-uha-manual-second-validation.md)。

> 2026-11-04第二批初始宿主已启动：PID61528，监督会话12024，窗口及实际生产DLL匹配；新库build/xuanshu-uha-manual-second-20261104-89bc95/session/conversation.sqlite。当前只待用户第1步AI展开，未提交/配置/Key/确认；本轮后续只允许第一次正常关闭核对后的同库恢复一次，不能自动循环。详[第二批记录](validation/xuanshu-uha-manual-second-validation.md)。

> 2026-11-04第二批已获执行授权：新根build/xuanshu-uha-manual-second-20261104-89bc95，匹配FF84…64AB0源码417/程序153未变；只补合成历史选择、本地未确认提案/完整预览、主动取消及同库记录恢复。最多初始＋证据核对后同库恢复各一次；零Key/Provider，不确认外发。rpc_memory单实例，跨标签暂列无入口未覆盖。准备完成、实际步骤及退出码待用户操作，不预判通过，见[第二批记录](validation/xuanshu-uha-manual-second-validation.md)。以下为各自历史状态。

> 2026-11-04最终：简短零Key、零联网人工复核通过。匹配FF84…64AB0正式宿主本次PID80044由用户完成展开→草稿→收起→主区→再展开→编辑→正常关闭；截图主区浅色且空状态明确、无助手残留，草稿完整保留并可编辑。实际exit0，关后隔离库任务/消息/事件/工具均0、无模型配置；没有Key/任务提交/确认/Provider路径，非抓包证明。首次PID45096退出码未知及原人工失败/首败不覆盖，另授权原库唯一重启不重置；IME等后续项目未执行，不自动进入下一批。详[短复核最终记录](validation/xuanshu-uha-visibility-manual-validation.md)。以下保留各阶段历史状态。

> 2026-11-04用户明确要求重新运行：原进程/监督已不存在且未留下实际退出码，未知退出事实保留；已保全原DB/WAL，在同一隔离库唯一重启PID80044（监督会话14690），匹配生产程序/实际DLL、窗口稳定。当前仍待人工第1步AI展开反馈，不读Key/提交/确认/联网，不重置或自动再试。详[短复核记录](validation/xuanshu-uha-visibility-manual-validation.md)。

> 2026-11-03简短复核唯一监督启动已完成：匹配生产宿主PID45096、监督PID53568/会话74599，正常用户上下文窗口稳定、实际生产DLL哈希一致；当前等待用户第1步点击顶部AI。新库build/xuanshu-uha-visibility-manual-20261103-83e2a7/session/conversation.sqlite，未设置Key/提交/确认。首次受限进程观察不足保留，正常用户对照不代表额外启动。实际退出码待关闭；不重启、不强杀，详[本轮记录](validation/xuanshu-uha-visibility-manual-validation.md)。

> 2026-11-03最新：UH-A显隐/空视图/输入/标签限定独立复审“通过、匹配快照可以冻结、必须修正项为空”已[追记落盘](validation/xuanshu-uha-visibility-independent-review.md)，冻结identity FF84C151A97784832D932AAE72F55BF9107DDE7E81DFF97F006FF41887664AB0；本轮仅身份核对和文档，不重跑测试。另获简短零Key/零联网复核授权，新根build/xuanshu-uha-visibility-manual-20261103-83e2a7；唯一监督启动及逐步用户观察待实际结果，见[本轮记录](validation/xuanshu-uha-visibility-manual-validation.md)。不提交任务、不设置Key/确认外发，不把冻结写成人工通过。原人工暂不通过、首图限制、ASAN20.02秒/30.08秒及其他旧风险均保留。下方是各自历史状态，不执行旧启动模板。

> 2026-11-03最新：人工第一批发现的黑区、助手残留、隐藏输入及输入辨识已完成限定开发、自审与离线验证，原人工第一批暂不通过结论保留。新根build/xuanshu-uha-visibility-20261103-e57b29；旧B335…8007A冻结仍仅对应旧快照，新代码等待限定独立复审，未自行冻结/启动人工。详[本轮记录](validation/xuanshu-uha-visibility-validation.md)、[增量合同](xuanshu-uha-visibility-contract.md)。底层无视图确定背景/空提示、隐藏交互收尾、实际布局预览门及常驻中文标签已实现；原人工唯一因果/时序限制仍保留。下面启动模板和阶段状态按各自历史理解。

> 2026-11-03第一批最终：唯一监督重试PID8708由用户Alt+F4关闭，实际exit0；关后只读库1任务/2消息/3事件/0工具，唯一cancelled，未确认/无模型请求或发送记录，零Key/Provider边界保持。配置、结构化read选择、本地保存及诊断已覆盖；AI收起后面板残留且用户报告控件无响应，输入框无清晰标签导致两次定位困难，第一批暂不通过，未再启动。 详[第一批最终记录](validation/xuanshu-uha-manual-first-validation.md)。原UH-A限定冻结记录保持快照边界；下一轮仅建议收起可见性/布局/重绘/命中同步及输入标签限定整改，尚未授权实施。首次启动/WAL取证限制与全部旧风险保留。

> 2026-11-03追加授权的唯一监督启动已进行：原库保留且只读核对任务/消息/事件/工具各0，无重置；当前PID8708及正式窗口存在、生产DLL匹配，监督会话51648持续等待退出码，人工第一步待用户点击AI。原PID43480取证不足及原WAL只留哈希未独立复制的限制保留，不能倒推其退出时间。详[第一批记录](validation/xuanshu-uha-manual-first-validation.md)，不读Key、不联网、不提交任务。

> 2026-11-03第一批启动异常：冻结追记已完成；匹配正式宿主PID43480创建后提前退出，未捕获退出码，stdout/stderr空，新库/WAL保留、模型配置未创建。未请求用户操作/Key/外发确认、未提交任务；不推定根因或正常关闭，不自动重启。人工第一批未完成，停止相关步骤，详docs/validation/xuanshu-uha-manual-first-validation.md。 原冻结identity B335…8007A及全部首败保持；后续仅建议启动监督取证，不自行修产品或进入真实服务。

> 2026-11-03 UH-A剩余构建失败窗口已承接上一轮限定独立复审“闭环、限定通过、匹配快照可以冻结、必须修正项为空”并追记落盘。冻结identity B3351D1DB62743F5E5EFC67739976CB2A470B7D32C5855375B25B02A8C68007A；本轮只核对身份，不重跑测试或完整复审。另获集中人工第一批零Key/零联网授权，新根build/xuanshu-uha-manual-first-20261103-5d8a41；启动及逐步人工观察仍待实际结果，不预判通过。详[冻结追记](validation/xuanshu-uha-buildgate-independent-review.md)、[人工第一批记录](validation/xuanshu-uha-manual-first-validation.md)。旧交付及首败/30.08秒/ninja首次取证限制保持，不自动进入有Key/真实请求。

> 2026-11-03构建失败回调窗口整改已完成开发、自审及离线验证；Light相称26/26＋另次复制1/1，ASAN-Light相称26/26＋另次复制1/1。新根build/xuanshu-uha-buildgate-20261103-b27e91；产品仅宿主两个回调边界检查，运行时/公开头/来源/预算不变。修前3例12项断言失败、夹具const编译错误及cleanup屏障不足分别保留；原ASAN30.08秒及其他首败不覆盖。当前等待再次限定独立复审，不自行冻结，不启动集中人工/Key/Provider。详[本轮整改记录](validation/xuanshu-uha-buildgate-validation.md)。下方三项整改完成/旧人工候选均保留历史，不能将新源自动纳入旧通过结论。

> 2026-11-03当前停止：UH-A三项限定整改已完成，自动验证通过，已具备再次限定独立复审的条件；不是独立通过、正式冻结或真实人工验收。新根build/xuanshu-uha-closeout-20261103-a6f2d8，详[整改验收](validation/xuanshu-uha-closeout-validation.md)及[增量合同](xuanshu-uha-projection-closeout-contract.md)。接纳前/末次回调后的限制性展示检查防止PENDING解绑保留旧确认；有界分片与原2MiB arena缓冲收缩使两条50000字节历史完整执行；内存/异步持久化诊断、终态错误及唯一查询ID防错投。Native40/40、最终Light59/59＋复制1/1、ASAN59/59＋复制1/1分别记录；ASAN30.08秒首次超时、编译/夹具/自审首败及原捕获限制不覆盖。596份before先于编辑、原413源码/93程序匹配，新增单个限制性导出，无来源/额度/依赖/DDL/旧ABI改变。无真实Key/Provider/用户原库/集中人工；下方旧开发准备结论保留历史，不自动继承到本候选。

> 2026-11-03当前停止：UH-A正式宿主开发及离线自动验证完成，已准备好开始集中人工验收；本轮没有启动正式验收、读取真实Key或Provider请求。新根build/xuanshu-uha-20261103-71d0c9，匹配身份见delivery-identity/source-after/binary-files。结构化历史/read/字段、逐块完整预览读回后绑定、宿主内存草稿、原生诊断复制、显式库派生配置及超长拒绝已落实；原逐任务人工确认不变，RPCB固定ref/批次确认未推广。Native29/29；Light联合95/95后最后宿主局部修正9/9、控制3/3、诊断1/1分别记录；ASAN33/33后局部6/6、诊断1/1。首次失败、基线捕获交错/历史对照、复制error5与正常上下文对照、ASAN启动依赖30秒超时均单列，不掩盖。自审不是独立复审。详[验收](validation/xuanshu-uha-validation.md)、[合同](xuanshu-uha-host-contract.md)、[人工入口](xuanshu-uha-manual-acceptance.md)。下一步只需授权集中人工第一批零Key/零联网操作，当前不自动启动。历史失败/browser另案/T3 unknown/483 Tokens差额及人工延期全部保留。

> 2026-11-02当前停止位置：RPCB-1预览展示与启动诊断两项整改已承接上一轮独立复审“通过/匹配快照可以冻结”并[追记落盘](validation/xuanshu-rpcb-display-independent-review.md)。identity41D15CFF8A2E0F639D02B6748C0F90AB95E9D6B4CAB7368221CF357BA4E8FF7C，源码/程序/清单核对无漂移；本轮只改8份授权文档，未重新测试或完整复审。最终UI17/17、ASAN UI17/17、生产启动3/3与另次core六项分别记录，日志引用已统一，中间运行/失败保留。原文档副本与新文档状态区分，build证据不改，无commit/tag/release。[阶段收尾](xuanshu-native-stage-closeout.md)已索引已有T1/T2真实结果、T3本地取消/安全退出及费用unknown、483 Tokens差额、历史风险、browser另案和延期人工；不追加请求。推荐UH-A正式宿主只读范围选择/诊断易用性仍属Proposed，等待下一阶段授权，不自动实现。

> 2026-11-02当前：RPCB预览接纳门及启动错误域限定整改已完成开发/离线验证，待再次独立复审，未正式冻结。产品只改启动器，runtime/DLL/uapp保持；当前hash完整预览成功显示后才绑定/启用开始，失败、旧hash和关闭均禁止接纳。最终UI17/17、ASAN UI17/17、生产零凭据启动检查3/3；原批次6项相称回归另计。修前8项/19断言及构建依赖/自审符号问题材料保留。新根build/xuanshu-rpcb-display-20261102-64c8a2；详[整改记录](validation/xuanshu-rpcb-display-closeout-validation.md)。本轮无真实调用/Key读取/用户库操作；旧T2成功、T3取消及费用unknown、483 Tokens差额和历史风险均不覆盖。

> 2026-11-02最新收尾：启动准备整改3/3通过；正常用户上下文同ref本地读取成功，无需重新录Key。一次可信批次确认后自动T2三POST取得真实发现及10/r1/framework revision保存，T3一POST真实在途取消；M/R/S/C/U=4/4/4/3/1，T2参考USD0.0008685、T3费用unknown，未追加。自动导出、安全销毁及进程退出已核对，详[最终记录](validation/xuanshu-rpc-startup-validation.md)。新根build/xuanshu-rpc-startup-20261102-2bd7e4；旧失败/额度不改，非独立复审或系统完整验收。

> 2026-11-02 RPCB-1最终：一次批次确认/自动T2-T3/自动取证关闭核心已实现并自审整改，Native相称96/96后最终批次17/17、Light99/99、ASAN97/97通过。新根build/xuanshu-rpc-batch-20261102-8ca4d2，离线identity341F240F…6EC58（408源码/23增量/136最终/146失败归档）。唯一真实启动PID5916在创建session/配置前早退，stdout/stderr空，根因未定位；尚未请求用户确认/录Key，实际POST0，T2/T3未执行，无库内M/R/S/C/U或关闭标记。按约不重启、不改实跑程序续发、不额外请求；真实补测未通过。详[最终记录](validation/xuanshu-rpc-batch-validation.md)。匹配程序、最终源码、失败和全部旧轮/凭据/额度保留；仅清548个新根obj。后续仅建议最小启动阶段取证与离线零凭据/零联网子进程验证，另授权后再推进，不自动执行。

> 2026-11-02本轮RPCB-1：用户已授权一次可信批次确认、验证专用固定凭据ref、新总4及0/3/1门和自动T2/T3/导出/关闭。设计先冻结并完成新独立入口/生产启动器，page对外Schema现与原adapter一致只接受0，1仍拒绝。新根build/xuanshu-rpc-batch-20261102-8ca4d2；Light99/99、ASAN97/97及Native相称/受影响路径分批证据见[本轮验收记录](validation/xuanshu-rpc-batch-validation.md)。本轮新程序仅自审/离线结论，未继承旧独立验收；真实批次尚未启动。旧T1成功、旧page1失败及所有R额度/库/凭据/历史证据保持，既有冻结SDK与来源政策不改。下面保留历史。

> 2026-11-02当前收尾：RPC-K1限定独立复审通过及匹配快照冻结已[落盘追记](validation/xuanshu-rpc-credential-independent-review.md)，产品身份不变、非阻断文案未改。本次独立T2/T3新根build/xuanshu-rpc-t2t3-20261102-53b8a6，原生Key确认成功、直接T2一次真实POST返回xs_read_discover/page=1，被仅接受0的适配器拒绝；发给模型的Schema只声明integer，说明缺口已定位但未修。M1/R1/S1/C1/U0、工具0、usage494/90、本地参考USD0.0002562，唯一failed终态已保存；T3按规则停止、T1未重复。用户正常关闭，PID75568退出，关后只读库/摘要/程序身份匹配。详[补测最终记录](validation/xuanshu-rpc-real-service-validation-t2t3.md)。旧T1成功、两轮额度/失败/期限与全部证据保持。无源码修改/构建/测试/重试/额外API，不自动重启；后续仅建议另授权page0声明与真实工具定义回归的最小整改，再评审补测。以下各阶段为历史，不扩写为全部RP-C通过。

> 2026-11-02上一轮收尾：RPC-K1限定整改及离线验收完成（当时尚非独立复审）：最终closure Native66/66、Light68/68、ASAN66/66。实际用户原生API Key窗提交，后台凭据保存/配置保存/同ref读回均确认；“复制诊断”已可用。T1准确回复“玄枢流式验证成功”，关后只读库确认唯一completed终态、M1/R1/S1/C1/U0、tools0、usage467/6及参考费用USD0.0001473。T2准备时Policy validation_deadline，T2/T3未执行，整轮RP-C未完成；用户正常关窗、PID76012退出、DB哈希相同。详[整改记录](validation/xuanshu-rpc-credential-validation.md)、[第二轮最终记录](validation/xuanshu-rpc-real-service-validation-round2.md)。旧根及原R1失败、隔离库/凭据均保留，只有559无用obj被清理；旧CredUI返回未知不补造，来源/布局/错误/限额/依赖及延期边界保持。

> 2026-11-02最终：RP-C首次真实服务限定验证未通过；T1已确认但发送前credential_unavailable失败，M=1/R=1/S=0/C=0/U=0、tools=0，额度不退还。T2/T3停止、无重试/新目录续跑。用户正常关窗，PID3056已退出；关后只读库确认唯一T1及唯一failed终态、8条顺序事件，无发送意图/进入/usage，库哈希前后相同。估算USD0.0055827不作账单，凭据不可用根因未定位，助手未读Key；不是DeepSeek拒绝或HTTP401。准备冻结记录已落盘，399源码/122程序/27red/8依赖/原证据身份保持；本轮仅9授权文档及新证据，无产品修改/构建。详[真实记录](validation/xuanshu-rpc-real-service-validation.md)，证据build/xuanshu-rpc-real-20261102-b5e731。最小下一步是另授权凭据录入/保存/读取的脱敏诊断及窗口复制问题排查，本轮不修、不重启、不追加请求。历史与延期边界不变，下方保留各阶段记录。

> 2026-11-02最新：上一轮RP-C准备限定独立复审通过，匹配快照可以冻结，现已[落盘追记](validation/xuanshu-rpc-preparation-independent-review.md)。identity=D10E28A6AB8DE63FC854518F75C67B8803E41708701084083691D4A0B32459B7，文档修改前399源码/13原交付/122最终及27red产物/8依赖匹配，没有重跑测试或重做全面复审。用户已独立授权本轮首次真实验证，仅新隔离库、deepseek-flash/Responses、三合成任务/空历史、最多5 POST按1/3/1、USD0.10停止/首次确认10分钟/原网络超时；Key和逐任务确认由用户原生操作。该段写入时尚未发送，后续过程见[真实记录](validation/xuanshu-rpc-real-service-validation.md)，证据build/xuanshu-rpc-real-20261102-b5e731。没有产品修改/构建/真实验收通过宣称。下方均保留历史。

> 2026-10-31最新：RP-C准备限定评审整改及离线回归完成，等待限定独立复审，不自行冻结或进入真实RP-C。详[整改记录](validation/xuanshu-rpc-preparation-closeout-validation.md)，新根build/xuanshu-rpc-prep-closeout-20261031-f27a64；下方原开发通过及独立复审前状态保持历史。

实际codex/p1-process-boundaries / e0eca03db25930be55bdb45477027d1793a61904未提交；入口508项当前范围（含tools及ignored规划），原准备397源码/24交付/135程序/85失败归档入口匹配，不归并HEAD至工作区全部差异。唯一产品修复为am_propose初始ai_id(client_request_id)校验，先于任何client比较；不改公开结构/错误数值、来源、5次及1/3/1、费用/期限或固定依赖。精确10/r1测试复用公共JSON校验/JCS比较，拒绝100/字符串10/错误版本，保留状态及框架revision断言。

修前两个独立CTest子进程在同步received内NULL重入propose/continue发生SEGFAULT；其他非法ID出现6个错码断言，合法不匹配仍正确拒绝。原先“仅源码窗口”现已在该隔离路径动态核实，不扩大为用户进程事故、死锁或数据损坏。首个新夹具C4389/WX类型错误单列，未放宽告警。red构建/源码/程序/日志/JUnit全部保留，未回填原最早tools缺失的同期源码。

修后Native受控5/5＋相称44/44（两次运行）、Light51/51、ASAN49/49；外层关联/方法及任务引用、未确认零发送、正常文本及同步read、重复提交不重发和安全关闭均有断言。原Native78/78/Light57/57/ASAN66通过1Not Run＋补1仍仅历史。新候选程序/启动模板见真实清单第6节，但本轮未运行生产启动器/读真实Key/联网/人工验收。

新根清单关联源码/程序/开测身份，旧公开头/导出/固定依赖保持；仅清新根无用obj，保留失败和必要诊断，不动旧证据/用户数据/冻结SDK。参考README可读、Git目录仍Permission denied，未扩权。历史期限/frontend/输入/browser首败未定位不覆盖，延期人工及browser九副作用另案不变。停止整改代码评审，真实调用仍需另授权。

> 2026-10-31最新：RP-C验证准备开发及离线验收通过，停止准备代码评审，未获真实调用许可。新增RPCP-1限制策略、生产链接memory.read=10/r1夹具及隔离原生启动器，未启动生产窗口/读取真实Key/调用API。Native78/78、Light57/57；ASAN首轮66通过/1 Not Run，补构建后该项1/1，逐项67/67。详[准备验收](validation/xuanshu-rpc-preparation-validation.md)、[冻结增量合同](xuanshu-rpc-preparation-contract.md)及[将来真实清单](xuanshu-rpc-real-service-checklist.md)。证据build/xuanshu-rpc-prep-20261031-c913e8。下方只写8文档、待准备/待复审等均为各轮历史，不自动授权RP-C。

实际codex/p1-process-boundaries / e0eca03db25930be55bdb45477027d1793a61904未提交；入口497项当前范围（含ignored根规划）区别于原491＋3；旧390源码入口匹配、114原冻结程序/清单保留，非HEAD差异全归本轮。准备增量24项（16项代码/构建/测试，8项文档/规划），397源码/135最终程序及PDB清单见新根，32旧公开头/固定依赖不变，仅新独立限制API4项，未改DDL/版本/错误/来源/额度。D:/XuanShu拒绝仍只记录、不扩权。

新启动器仅接受新目录，显式conversation.sqlite和conversation.sqlite.model.json，ref由后者派生，不改LOCALAPPDATA；一个Service/主对话，原生Key/确认仍原可信人工链。策略只限制不授权；三合成任务/空历史/固定10/r1目标和投影；全局5、分任务1/3/1，所有续轮均须配额/保存确认，失败取消未知不退款不重试。M/R/真实S/C/U与模拟计数分开，API在途/保存不确定如实显示。USD0.10非服务端硬保证，usage未知封闭；首次确认10分钟停新派发，回调实际退出才关闭；同库重开恢复计数但封闭授权，不自动联网。

首轮12项8通过/4失败及20.01秒超时、启动器窄/宽字串和yyjson夹具签名编译错误、ASAN未构建目标Not Run均保留，不弱化断言。首个构建脚本未扫描tools，启动器首次编译失败完整同期源码未自动归档（日志仍在，无当次成功链接程序）；运行失败源码/日志/JUnit及匹配程序保留，不能用最终源伪装原捕获。新根仅清510个obj，失败85产物/PDB及必要诊断保留，不动历史或冻结SDK。

后续先独立评审准备快照，再另授权具体5次真实调用及用户原生Key录入/逐任务外发确认；不可直接运行旧默认模型配置的宿主冒充隔离。真实TLS/代理/Key界面/设备/IME/DPI/视觉/辅助技术/长时未验。历史期限/frontend/输入/browser首败不定位不覆盖，browser九副作用仍另案，不进入OpenAI/OAuth/AI写入或其他功能。

> 2026-10-31上一轮：RP-A/B限定独立验收通过，匹配快照可以冻结，记录见[独立复审追记](validation/xuanshu-rpab-independent-review.md)。RP-C规格已沟通确认，当时仍须另授权准备；原轮只写8份文档，不构建/启动/联网/读Key。详[RP-C计划](xuanshu-rpc-validation-plan.md)。

> 2026-10-31最新：RP-A/B两项限定整改及离线回归完成，待重新独立复审，不是正式冻结或RP-C许可。Native新增2/2＋相称61/61、Light42/42、ASAN52/52；原RP/P6结论和历史首败保留。见[整改验收](validation/xuanshu-rpab-closeout-validation.md)，新证据build/xuanshu-rpab-closeout-20261031-a19c42。

> 2026-10-29最新：RP-A/B代码与离线验证交付，停止联合代码评审及RP-C之前。下文P6冻结及旧Provider提案状态均为历史。

## 2026-10-29 RP-A＋RP-B开发交付checkpoint（停止联合评审／RP-C之前）

实际分支codex/p1-process-boundaries，HEAD e0eca03db25930be55bdb45477027d1793a61904，未提交；入口476项当前源码/直接文档，保留原改动/未跟踪项，不归并P2—P6全部差异。P6R-1入口374源码/102程序匹配，原冻结结论仍仅针对原快照。D:/XuanShu读取仍拒绝，未扩权。

RPAB-1先冻结后实现：DeepSeek官方Responses固定HTTPS、非思考模式、用户配置/Windows凭据引用、原生Key窗口及框架私有一次外发确认；当前输入＋明确历史实际预览＋所选目标read投影。一个Service/主对话，复用原统一目录及read工具，不开放P4/P5写入/审批/图。新独立ai_model.h/ai_model_ui.h保持旧布局/API10/ABI1/包1和固定依赖/额度。按原8MiB计入2MiB arena；生产无测试授权导出，不自动POST或工具重试。保存后再UI核对才发送，错误/取消/到期仍等连接及回调退出；冷开不恢复确认或自动联网。

自审修正并保留首败：整批预检不遗忘未执行call、PREPARED绑定确认后dispatch、排队取消可收尾、七字段消息封装、在途到期不提前释放、SSE精确满额余量、方法/run/回调租约及预览arena互斥。修前有界失败不冒充崩溃；初版时序屏障不足与夹具编排/源代码缺陷分开记。

最终匹配Native61/61、Light40/40、ASAN50/50；真实双workspace实例值10回读、异步DLL worker/外部回调、受控网络回调、存储checkpoint、隔离子进程冷开、正式Light及静态嵌入均有离线证据。结论为“RP-A＋RP-B开发阶段模拟及自动验收通过”；非独立复审、真实服务或真实人工验收通过。完整合同/失败/未验见docs/validation/xuanshu-rpab-validation.md、docs/xuanshu-rpab-contract.md及新根build/xuanshu-rpab-20261029-6f38a1清单/JUnit/日志/匹配程序。

RP-C需另授权具体模型、凭据入口、数据/费用/时长及停止条件；本轮未用实际Key、未请求真实API，也未运行生产framework_host。真实TLS/代理/系统凭据窗口和物理来源正例仍未验；软件整体人工清单保持延期。四组期限风险及旧frontend63/ASAN-Light/输入/browser20/workspace20.05秒历史结论保留，未被新通过覆盖；browser九副作用仍另案。未进入新功能、未改参考/外部应用/冻结SDK/其他依赖或历史证据，无子代理/computer use/Git/远程/benchmark/recovery。只清理本轮独立根的无用编译obj，失败源/日志/JUnit/匹配程序及隔离核对数据保留。

> 2026-10-28最新：P6R-1三项整改已由上一轮限定独立复审通过，匹配交付快照可以冻结；本轮仅追记该结论并核对身份，没有新测试或产品改动。详[独立复审与冻结记录](validation/xuanshu-p6r1-independent-review.md)。下方“等待独立复审”均保留为历史。

## 2026-10-28冻结记录及真实Provider规格评审交接

实际codex/p1-process-boundaries / e0eca03db25930be55bdb45477027d1793a61904未变。文档修改前原374源码/102最终程序/9 red程序/15交付匹配；冻结以build/xuanshu-p6r1-closeout-20261028-91c4e7清单为准，不执行commit/tag/release。17470入口自动Git项＋3事后补充ignored根规划分别记录，原Native96/96、ASAN96/96；Light首次55通过/1 Not Run，后补1/1，逐项56/56。

本轮仅7份授权文档：新增独立复审记录、真实Provider Proposed提案，更新原整改验收、本交接和根三规划；原产品源码/测试/构建/版本/固定依赖/证据目录不动。文档变动不作为新产品测试身份。D:/XuanShu读取仍Permission denied，不扩权。

用户已确认修订首批为DeepSeek官方Responses，OpenAI后补；逐任务确认外发当前输入、明确选取历史和必要只读能力/结果，不自动上传整条主对话或其他应用。推荐下一轮RP-A离线核心，RP-B用户配置/Light，RP-C真实服务另授权。接口/超时/协议子限额及分批细节仍Proposed。当前没有真实HTTP Provider、用户配置/凭据实现或新的AI写授权；stub不是自然语言理解。详[接入提案](xuanshu-real-provider-integration-proposal.md)。

四组未定位期限风险、final/proof提交后checkpoint非阻断补测、原历史失败均保留；browser九副作用另案及全部人工延期不改。未连接真实模型、读取Key、发送业务数据、构建/测试/启动程序或清理历史。停在规格评审，不自动进入实现。



保全清单补充：入口baseline.json自动捕获Git可见17470项，遗漏被Git忽略的三根规划。原baseline不改；baseline-supplement.json通过剥离本轮两段仅追加checkpoint恢复原始前缀，并与原P6归档逐字节匹配，另核对3项。合计17473项保全核对，不把这3项称为入口自动捕获。首次清单KeyError及补充来源见metadata-reconciliation.txt。
> 2026-10-28最新：P6R-1三项限定整改已完成，等待独立复审，不自行冻结。下文旧交付/验收结论保留为历史。

## 2026-10-28 P6R-1三项整改完成checkpoint（等待独立复审）

本轮实际e0eca03db25930be55bdb45477027d1793a61904 / codex/p1-process-boundaries未变，无提交；证据build/xuanshu-p6r1-closeout-20261028-91c4e7，入口17470，增量单独核对。先有受控红证据，再最小修复：Coordinator引用移交至done待消费并核对稳定ID；存储失败独立cleanup-only，不依赖active/c.failed；ai_begin及Provider/Coordinator/子编辑提交后门拒绝不可用存储推进。自审补齐关闭read关联排空及UI错误路径不等待通知队列。UNKNOWN、实际提交事实、原线程/回调和执行门保持，没有新执行器。

最终Native96/96、ASAN96/96；Light原批55项通过/1项未构建未运行，随后五应用目标构建并单项1/1（416/0），同源码逐项覆盖56/56，不冒称一次56全绿。9个新增受控case、原Service/P4/P5/37存储故障、两真实DLL编译分支、公共C/C++头/Assistant/静态嵌入及Light正式投影相称覆盖。首版夹具超时/屏障不足、原实现有界失败、自审read排空red、ASAN故障未接纳及Light编排未运行分别保留；不是已复现崩溃/永久死锁，也不以重跑覆盖首次。

SDK0.10-dev/API10/标准10、ABI1/包1、公开布局/v1/C2/P4/P5/固定依赖/JCS及额度不变。原并行boundaries20秒、ASAN控制5秒、P2五包15.04秒、面板180.44秒仍期限风险，未据本轮串行通过定位或解决；旧frontend63/ASAN-Light/输入/browser20/workspace20.05秒等仍独立。browser9副作用另案，真实设备/Windows正例/IME/DPI/视觉/辅助技术/物理断电/人工长时延期。未重复600秒/60轮旧样本，不外推无限稳定。

具备再次开展P6R-1限定复审的开发交付条件；本轮仅自审，不自行宣布独立验收通过或代码正式冻结。详docs/validation/xuanshu-p6r1-closeout-validation.md及新根delivery/preservation/source/binary/exports/日志/JUnit/cleanup。没有参考工程/用户库/外部应用/冻结SDK/固定依赖写入、子代理、computer use、真实Provider或Git/远程操作。原主线及历史仅追加保留。

> 2026-10-27最新：P6R-1限定可靠性开发交付已完成，停止P6联合代码评审。**不作无条件P6/原P5全应用迁移验收通过宣称**：browser9项副作用已由用户确认另案，新的并行短测期限风险保留；真实设备/物理断电/人工项目延期。下文“不进入P6”等均为历史。

## 2026-10-27 P6R-1交接checkpoint

实际codex/p1-process-boundaries / e0eca03db25930be55bdb45477027d1793a61904，无提交。新证据build/xuanshu-p6-20261027-4a92d1，入口baseline17463；本轮增量以delivery-files为准，不把HEAD全差异归本轮。原P5 37/331/114及修前362项归档匹配。D:\XuanShu元信息可见，Git读取Permission denied，无扩权/写入；未伪称新审计其HEAD。

P5限定复审发现子编辑存储job漏唤醒，连续正式面板第15轮停滞；UI poll锁内遍历子job后受控真实线程退出回归值42，原UI不join、回调/UNKNOWN/门保持。P6增加恢复写入前只读逻辑审计及周期PASSIVE checkpoint。自审发现checkpoint错误被忽略，red1205/3保留，最小修复后错误显式UNAVAILABLE且保留已提交事实；无新schema、公开接口/错误合同、额度或固定依赖。

最终Native/ASAN故障专项各37/37（20进程窗口、8坏副本、9专项），60轮真实DLL＋600秒资源各2/2，1755/1824任务，唯一终态及冷重开不执行。Light生产五包/六分支/公共头/Assistant/真实DLL/P3面板14/14；P4/P5面板、60轮面板和browser5/5。兼容同源码逐项覆盖各49/49，包含只对失败项的串行重核，不是一次49全通过；原联合负载超时/控制断言保留。资源最大Service5475077<8MiB，Native private约12MiB，ASAN约402MiB quarantine单列；有限十分钟不代表无限稳定。

browser现有Light内部应用新增browser.snapshot/原生资源/open epoch/revision和10个既有view的菜单/标准控件/适用快捷键绑定，复用原action和统一目录。Assistant只读及旧修改旁路拒绝。navigate/back/forward/reload/stop/home/help（7）和settings-save/reset（2）仍保留原人工，完整副作用迁移另案；不是WebView2排除、没有新网络/文件执行器、没有新业务模型。五维护应用源码不改，不冒称全部历史应用副作用完成迁移。

首次audit中文路径、面板停滞/180.44秒超时、browser描述/夹具、P4旧投影夹具、未构建8项、P2 15.04秒、联合负载boundaries/control及自审checkpoint red均分账。串行通过不能解释首次根因；panel-wakeup两个错时源码已从旧失败档恢复到匹配开测哈希的独立副本，原复制件保留；实际模块名正反断言由最终60轮证明。旧ASAN-Light/frontend63/Light surface_input/browser20/workspace20.05秒(20门槛)/Native首次仍独立未定位。

SDK0.10-dev/API10/std10 ABI1/package1、旧布局、v1/C2/P4/P5、固定yyjson/SQLite/JCS不变。生产导出Native/ASAN326、Light340，均无fixture；隔离348仍原8个私有测试入口。无真实Provider/自然语言质量证明、外发/跨实例共享写入、用户库操作、真实物理验收、Git/远程工作流或下一阶段。

详见[独立P6验收及全部失败](validation/xuanshu-p6-validation.md)、[P6R-1合同](xuanshu-p6-reliability-contract.md)、[应用矩阵追加](xuanshu-p5-application-coverage.md)、[未来人工/匹配正式包和启动](xuanshu-p6-deferred-manual-acceptance.md)。delivery/preservation/source/binary/failure/resource/exports/identity及cleanup均在独立证据根。仅清理本轮可重建过程产物；旧交接原文保持下方完整。

> 2026-10-27最终所有权收口已通过：ownership Native22/22、Light32/32、ASAN22/22，Native SDK/两DLL分支9/9。仅私有队列窗口修正，受控回归通过；下方21/31/21为修前历史。最终身份及清理由本轮清单核对，停在P5联合代码评审，不进入P6。

最终37交付/331源码/114运行链接产物均有清单；三配置源/程序无最终身份差异，原P4 352项及修前362项保留。最后正式五包1/1（364/0），清理仅本轮过程产物累计1199删除事件/420803398字节，见独立验收第10节与cleanup汇总。

> 2026-10-27最新状态：P4限定收口及P5-1已交付，**P5开发阶段模拟及自动验收通过**（本轮明确授权范围）。停止联合代码评审，不进入P6；下方P4“P5待授权”均为历史。

## 2026-10-27 P5交接checkpoint

实际HEAD e0eca03db25930be55bdb45477027d1793a61904 / codex/p1-process-boundaries未变，无提交。独立根build/xuanshu-p5-20261026-e63b8a，17450入口baseline；原P4 43/323/352入口匹配。增量、源码/二进制和保全/清理以本轮清单为准，不把HEAD全差异归本轮。

先冻结P5-1，再实现单Service图调度/共享预算/私有上下文、纯数据profile、有限多步骤授权及原P4编辑控制器复用、schema3一致备份确认迁移、五真实维护应用协作和同Light正式面板。Assistant/AI不回退legacy，普通read仍可主Agent直接执行；子Agent不拥有Provider/权限/完整主对话。固定目标/epoch/revision，取消/UNKNOWN/回调/资源门和应用自有事务/undo保持。

最终verified Native21/21、Light31/31、ASAN21/21；Native静态/头/Assistant/两真实DLL分支sdk-closure9/9单列，前轮/前批次不相加。首图/编译/夹具/面板/审批等待失败与匹配材料保留；最后取消排队任务committed混淆导致三配置超时，经admitted独立事实及受控时钟回归收口，未放宽20秒/资源额度。真实值包括31/62、stateful73、EDA稳定ID1、两分支树名及counter1；不以计数或草稿变化冒充修改。

P4前置限定复核另封闭新控制绑定早于范围投影的源码窗口，未声称已动态复现真实误写；未投影先解绑，Light同步post成功才重新绑定。重启只恢复记录，旧epoch先异步查身份，明确继续新绑定且拒绝旧charged operation重放；完整私有context不落普通事件，只留manifest/hash和必要关联。

SDK0.10-dev/API10/std10 ABI1/package1、v1/C2/P4布局/错误/限额、固定yyjson/SQLite/JCS保持。参考工程仍未新核验/修改，不扩权。browser为现有Light legacy，原其他应用全迁移未完成，必须独立合同评审，不以WebView2排除理由掩盖缺口。历史ASAN-Light interrupted/frontend63/Light surface_input/browser20/workspace20.05秒(20门槛)/Native首次仍独立未定位。

详见[P5验收/首次失败](validation/xuanshu-p5-validation.md)、[冻结合同](xuanshu-p5-coordinator-contract.md)、[应用矩阵](xuanshu-p5-application-coverage.md)、[延期人工/匹配正式包和未来启动](xuanshu-p5-deferred-manual-acceptance.md)。真实来源/IME/DPI/视觉/辅助技术/长时未验，无真实Provider、用户库迁移、外部应用/冻结SDK写入、Git/远程操作或P6。

> 2026-10-26剩余P4最新状态：**P4开发阶段模拟及自动验收通过**，停在联合代码评审，不进入P5。已独立复审/修正P3/P4-A，完成五维护应用13项内存write、3项文件政策边界及正式Light授权/批准/撤销/UNKNOWN/历史面板。真实人工仍延期，ASAN-Light共享投影脚本中断诊断保留。下文P4-A/更早状态均为历史。

## 2026-10-26 剩余P4交付checkpoint

HEAD e0eca03db25930be55bdb45477027d1793a61904 / codex/p1-process-boundaries未变，无提交。17442项独立入口baseline；P4-A 30/318/131入口身份匹配。增量、源码及二进制、保全/清理以build/xuanshu-p4-final-20261026-c84f12内delivery-files/source-after/binary-files/preservation/audit/cleanup为准，不把HEAD全部差异归本轮。

复审red129/2核实continue目标回调可重入接纳edit后外层仍接纳read，最后回调后重查互斥，未制造崩溃。新增带方法租约的目标解析、同目录风险扩展及可信Light绑定，仅3个独立公开入口；生产Native/ASAN314、Light328导出，无私有fixture/直接批准/来源旁路。SDK0.10-dev/API10/std10、ABI1/package1、v1/C2/P4A-1、额度及固定依赖不改。

stateful单元格/共享属性、EDA对象/笔记/clear审批、generic/features各分支的属性/单元格/树名、counter全部复用原业务run及应用快照/commit/rollback/READBACK/undo。草稿保留、revision单调、ID高水位、UNKNOWN及持久化门保持。三项现有UST1/UIL1副作用AI审批消费后明确拒绝，原人工后台入口保留；不是新增文件执行器或文档格式。正式framework_host一个Service/主对话，页消息/程序点击不能批准，Light与原生共享16绑定，解绑PENDING不缓存就绪。

本轮联合Native50/50、Light72/72、Native-ASAN50/50；受后续应用投影/宿主租约整改影响者另有Light6/6与2/2、ASAN-Light真实应用/面板7/7及3/3和1/1，原非AI人工对照2/2。最终Native/ASAN核心各8/8。分阶段复用不相加为独立用例数，具体源码身份见清单。GUID隔离修正新测试PID复用库引起的counter20.01秒超时，首库和红材料保留；先前WGL/通知/异步谓词/Schema/分页/声明等首失败逐项分账。

ASAN-Light generic/features的50ms脚本中断诊断，非AI人工对照亦有；业务/回执/实际树投影和ASAN内存检查通过，生产Light相应路径无该诊断，但不能当视觉验收或默认环境原因。frontend63及不完整旧版本对照、Light surface_input首次、browser20、workspace20.05秒/20秒门槛、Native输入首次继续独立未定位，不被新通过覆盖。

详见[P4独立复审](validation/xuanshu-p4-independent-review.md)、[P4最终验收](validation/xuanshu-p4-validation.md)、[55项及逐能力矩阵](xuanshu-p2-application-coverage.md)、[延期人工操作/正式匹配包](xuanshu-p4-deferred-manual-acceptance.md)。没有用户库迁移、XuanShu/外部应用/冻结SDK/固定依赖写入、真实Provider、P5、通用事务或跨实例共享写入。整体其余架构仍Proposed，物理/Windows动态正例/IME/DPI/视觉/辅助技术/长时未验，等待后续明确授权。

> 2026-10-26最新状态：本轮先冻结[P4A-1](xuanshu-p4-controlled-edit-contract.md)，已完成P3租约收口及P4-A单资源夹具实现；Native44/44、Light49/49、ASAN44/44，限定为**P4-A开发阶段模拟及自动验收通过**。见[P3独立复审](validation/xuanshu-p3-independent-review.md)、[本轮验收/身份/首次失败](validation/xuanshu-p4a-validation.md)。整体架构仍Proposed，P4-B/C、正式面板、应用接入和真实人工均未完成；下文历史状态保留。

# 当前交接：2026-10-22

## 2026-10-26 最新交接：P3收口及P4-A开发阶段模拟及自动验收通过

本节优先于原P3“等待P4授权”历史状态。HEAD e0eca03db25930be55bdb45477027d1793a61904 / codex/p1-process-boundaries未变，无提交。证据build/xuanshu-p4a-20261026-a61d4e；17427项入口baseline，原P3 32/308/101身份入口全匹配。本轮30项增量（15修改/15新增），17412项入口文件未变，无删除或越权改动，不把HEAD全差异归本轮。

P3 continue的get_revision可重入关闭/poll/destroy，red48/2确认旧门实际可销毁；测试立即停用旧对象，未制造或宣称崩溃。私有methods租约及回调后READY重查修复，正常continue及外层退栈后安全销毁通过；P3独立记录保留。P4先冻结P4A-1，再实现同目录编辑阶段、独立宿主授权/审批、共用资源门、后台原子意图/单次消费、真实DLL事务/回读/撤销回执及UNKNOWN核对。schema2仅隔离库迁移/备份验证，重启只恢复记录，不恢复授权或重放业务。

最终review-final Native44/44、Light49/49、ASAN44/44；每配置20个P4专用case，另有P3核心/静态嵌入/Light面板及原A+B/C1/v2真实DLL生命周期、Assistant、公共C/C++头、组件/来源相称回归。生产Native/ASAN311、Light325导出，仅新增10接口，无私有fixture/授权导出；旧公开布局/v1/C2合同、版本、依赖及限额不变。

首构建/Not Run、夹具unmount级联超时、describe_v2合法描述误OOM、过早scope、核对RUNNING、回读旧txn污染、UNDO漏门及新测试父ID关联错误均逐项保留/修正，不放宽原断言。UNKNOWN/确认丢失不自动重试，原唯一终态及审批消费不重复。精确源码/匹配二进制、阶段输入、JUnit和DB证据及清理见[本轮验收](validation/xuanshu-p4a-validation.md)。

frontend63及其不完整旧版本对照、Light surface_input首次、browser20、workspace首次20.05秒(20秒门槛)、Native输入首次仍独立未定位，不被新通过销账或归因环境。真实设备、Windows来源动态正例、IME/DPI、视觉/长时继续延期到整体完成后另获授权。本批没有用户库/参考工程写入、真实Provider、P4-B/C、stateful/EDA业务接入或正式聊天授权审批面板。停止P3整改及P4-A联合代码评审，不自动进入下一批。

## 2026-10-24 最新交接：P3本轮开发交付，停止代码评审，不进入P4

本节优先于下方历史P2停止状态。本会话后续已明确授权P3，按[P3-1](xuanshu-p3-service-contract.md)先冻结设计再实现；结论是**P3开发阶段模拟及自动验收通过**，不是设备/完整人工验收或全部历史CTest绿色。实际HEAD e0eca03db25930be55bdb45477027d1793a61904、codex/p1-process-boundaries未变，无提交。独立证据根build/xuanshu-p3-20261024-f19c6a，17408入口baseline；原P2 delivery17/source280/binary77入口匹配，原用户修改与历史保留。

新增workspace唯一原生C Service、一个可恢复主对话、后台SQLite3.54.1离线固定供应、16排队/1活动/独立控制、稳定ID/幂等/有序事件和唯一终态，stub流式/显式脚本、统一目录read工具及现有Light宿主全局面板。收面板/切标签不改任务，恢复未完成仅NEEDS_REVIEW，显式继续才新建关联run并重新绑定。UI只poll，不join；应用REFUSE后恢复接纳但不复活旧取消，存储未开完不提前READY。Assistant/AI仍只读，不导出Provider/审批/写入/Coordinator接口。

P3最终专项Native6/6、Light7/7、ASAN6/6；相称旧回归Native13项、Light14项、ASAN6项另列，真实DLL原入口和C1生成入口两分支均核对。真实counter count/revision、完整工具JSON、非法输出/重复tool ID、取消停滞、事件背压、隔离子进程重启、锁/损坏/未来schema、worker/外部回调退栈、同步complete外层execute及REFUSE都有实际证据。公共头旧布局断言保留；Light生产315导出/Native301，均无私有fixture/授权导出。SDK0.10-dev/API10/std10 ABI1/package1、P2合同/限额及其他依赖不变。

本轮首control超时、构建/夹具错误及ASAN夹具级联UAF原证据保留，不写成产品Service崩溃修复。旧完整frontend63断言仍失败，入口宿主/页面/测试与当前相同失败集合，主要含过时Assistant写入假设；它不是完整旧框架二进制对照。旧JS null节点诊断及Light surface_input首次/browser20/workspace20.05秒(20秒门槛)/Native输入首次仍独立，不默认归因环境或被新通过覆盖。详见[本轮验收](validation/xuanshu-p3-validation.md)。

正式物理鼠标/键盘、Windows来源动态正例、IME/DPI、视觉和长时统一延期到整体软件完成后另获授权，见[P3未来集中步骤/隔离启动](xuanshu-p3-deferred-manual-acceptance.md)和原P2延期表。当前没有真实Provider、用户库操作、参考工程修改或SDK覆盖。保留源码/二进制清单、首失败DB/journal和匹配产物；清理只限本轮无用中间件。停止P3交付/评审位置，P4等待新授权。

## 2026-10-24 最新交接：P2开发阶段模拟及自动验收通过，人工统一延期

本节优先于下方历史“等待当前人工验收”。用户明确仅授权当前P2验收收口，真实设备与必须人工判断的项目延期到软件整体开发完成后、另获授权集中验收；不进入P3。本轮实际分支codex/p1-process-boundaries、HEAD e0eca03db25930be55bdb45477027d1793a61904，17406文件独立baseline；上轮delivery60/source280/binary689入口身份全部匹配，无提交。

本轮仅改5个测试源和必要文档/规划，产品源码/公共头/CMake/版本/固定依赖未改。应用实际菜单/工具栏所有别名/快捷键、EDA按钮/画布collector、stateful真正FORM/DIALOG收参，以及五真实DLL/两features配置的重开和旧open epoch拒绝已补测。新10个case在最终分阶段并集全部通过：generic204、features389、EDA142、stateful223、真实包364，HMENU29、inputs596/741、真实workspace650及组件64检查均0失败；未变核心/C1/A+B/预算/真实DLL生命周期/公共头与counter证据复用，不能冒称本轮全套重跑。

首轮apps358/6是新fixture误以为进程host_epoch应在重开后改变；按既有process_epoch合同修正新断言，并独立验证旧open epoch，最终364/0。不是产品epoch修复。red源码/匹配二进制与日志/JUnit保留。生产DLL299导出中无私有授权/fixture/route入口；来源政策未放宽。原Light surface_input首次、browser20、workspace20.05秒(20秒门槛)、Native输入首次继续独立未定位，不被本轮通过覆盖。

证据根build/xuanshu-p2-sim-20261024-d36f82，见[本轮独立验收和20行A/B映射](validation/xuanshu-p2-simulated-acceptance.md)、[C延期原因/替代证据/风险/将来步骤](xuanshu-p2-deferred-manual-acceptance.md)。原人工清单/前轮证据保留，未启动人工宿主、未操作设备或SendInput。SDK0.10-dev/API10/std10 ABI1/package1、冻结合同/旧布局/限额不变；参考工程没有再申请读取或写入。停止当前P2开发阶段结论，不宣称真实人工或无条件P2完整验收通过。

## 2026-10-23 最新交接：剩余P2代码与自动准备交付，人工验收尚未开始

本节优先于下方历史阶段。实际HEAD e0eca03db25930be55bdb45477027d1793a61904、codex/p1-process-boundaries不变，未提交；入口17381文件独立baseline。原A+B/C1/C2及用户改动保留，不能将HEAD全差异归本轮。参考工程一次读取被拒，没有重复扩大权限或修改。

完成C2范围复审及HMENU-1、标准组件CI-1、应用安全poll AP-1，修复目录长metadata、选择local continuation、动态UNKNOWN菜单保留原生content等范围内问题。五个维护示例55项（11 read/44人工非read）迁移到同一目录和业务执行器；stateful共享name/独立草稿及稀疏单元格，EDA稳定ID/固定矩形/明确笔记提交，其余两编译分支和counter均实际模型回读。UST1/UIL1不变，不新增业务文档格式。SDK0.10-dev/API10/std10 ABI1/package1、旧公开布局/v1/冻结C2合同、固定依赖/限额保持。

新证据根build/xuanshu-p2-remaining-20261023-b84e6f：Native25/25、ASAN25/25，后续公共组件各10/10；Light联合41/43，resilience策略测试已迁移并限定回归通过；surface_input首次thread modifiers失败仍未定位（旧A+B也有同断言），不能以随后通过覆盖。最新stateful真实I/O UNKNOWN201/0，其他业务及真实五DLL分别通过，详见[独立记录](validation/xuanshu-p2-remaining-validation.md)。原browser20、workspace首次20.05秒/20秒门槛、Native输入首次仍独立分账。

[功能矩阵](xuanshu-p2-application-coverage.md)和[集中人工清单](xuanshu-p2-manual-acceptance.md)已准备。匹配二进制/SDK、启动及负例脚本在本轮证据根，未启动人工宿主、未执行物理输入或SendInput。真实来源动态矩阵、IME/DPI、长时及其他人工项目待用户集中操作；不宣称P2完整验收通过。没有AI Service/Provider/Coordinator/存储恢复/通用事务撤销。停止人工验收开始前，不自动进入P3。

## 2026-10-23 最新状态：C2集中收口已交付增量，完整验收暂不通过

承接C1限定验收，本轮连续完成A/B联合源码审查、范围内整改，冻结C2C-1并实现六类输入元数据及真实宿主chrome桥。实际分支codex/p1-process-boundaries、HEAD e0eca03db25930be55bdb45477027d1793a61904；17373项独立baseline含三根规划。下方旧“C2-C未授权”是历史状态，不代表本次授权。

最终Native22/22、Light30/30、Native-ASAN22/22；原C1/A+B/Assistant/headers/组件/两真实DLL分支及新真实workspace固定目标分别验证。末次回调来源门、失败资格消费、form回读后modal门已修。本轮surface raw回调移除后WM_CHAR空调用在Native/ASAN独立复现并修复；不是历史Native首次失败。

**未完成/未验**：原生HMENU非read仍是范围内缺口，公开WM_COMMAND不能签发可信人工资格；真实鼠标/键盘及Windows完整动态来源正例未验。因此C2暂不通过完整验收，不把不支持入口移出范围后宣称全完成。WebView2明确排除；应用模型/五示例迁移/AI和P3—P6均未实施。

证据根build/xuanshu-p2c2-closeout-20261023-a93d5e保留baseline/delivery/preservation、源码/二进制清单、首失败及匹配产物、日志/JUnit；见[独立集中报告](validation/xuanshu-p2c2-closeout-validation.md)和[C2C-1](xuanshu-p2c2c-input-contract.md)。SDK0.10-dev/API10/std10 ABI1/package1、旧布局/错误/限额/固定依赖不变。参考工程访问被拒，未改动；旧browser20/workspace首次20.05秒(门槛20秒)/Native首次继续独立未定位。没有commit/push/远程操作。停止C2联合代码评审，不自动进入应用迁移或P3。


## 2026-10-23 最新状态：C2-A＋C2-B已实施，待联合代码评审

本会话明确授权并已完成C2A-1及C2B-1标准表单/对话框实现、范围内自审修正和本地验证。C1限定验收承接；下方“C2-A仅设计/未授权”均是历史阶段。实际分支codex/p1-process-boundaries、HEAD e0eca03db25930be55bdb45477027d1793a61904；交付增量相对本轮17358文件baseline，而非全部HEAD差异。

同一目录的非read只经框架内部一次性人工接纳，v1/Assistant继续只读；11个C2-A加4个C2-B独立接口，固定目标/revision、busy门、UNKNOWN只读核对、同步/异步租约与预算、Light表单完整参数/快照/草稿冲突/modal/业务回读已接通。专用内存和真实DLL夹具不改变stateful/EDA业务或五示例；没有C2-C/AI Service/Provider/事务/存储实现。

最终新Native17/17、Light23/23、ASAN17/17通过，原失败及修复前后源码/匹配二进制保留。详见[联合记录](validation/xuanshu-p2c2ab-validation.md)、[C2A-1](xuanshu-p2c2a-interface-and-acceptance.md)和[C2B-1](xuanshu-p2c2b-form-contract.md)。真实鼠标/键盘、OS来源动态矩阵及v2真实workspace标签切换仍未验，不宣称完整验收。Windows分类不证明物理来源，不隔离恶意DLL。

SDK0.10-dev/API10/标准10、ABI1/包1、原布局/限额/依赖保持；参考工程只读且本轮访问被拒绝。原browser20失败、workspace首次20.05秒（门槛20秒）、Native输入首次失败仍独立未定位。停止在两批联合代码评审，不自动进入下一批。

本页优先于旧交接中的“当前状态”。作者：Codex。此次只做持久化、源码完整性核验和已授权的提交推送，不重新开展界面或 Mesa 专项。

## 2026-10-23 当前状态：C1限定验收通过，C2-A仅设计冻结

本会话已接受C1最终整改的限定验收，原实现/整改及历史证据保留。最新状态为[C2A-1接口与验收规格](xuanshu-p2c2a-interface-and-acceptance.md)已获文档落盘授权：11个独立增量接口、内部一次性人工凭据、Windows/Light一个菜单叶项、资源busy拒绝、未知结果人工只读核对及A01—A15已冻结为设计，尚未实现/运行。三份整体设计仍为Proposed。

本轮仅新增该规范并更新三份设计、本页和三份根规划，共8份文档；未创建公共头，未改产品/测试/构建/版本，未构建或运行测试。保持SDK0.10-dev/API10/标准10、ABI1/包1、既有公开布局、固定依赖/JCS/限额。实现须另行明确授权，不自动进入C2-B、应用模型、五示例迁移或P3—P6。

人工来源只由框架私有匹配事件链确认；识别到的注入、系统/未知来源、程序调用不授权。Windows来源分类不等于物理证明，不承诺排除全部UIAccess注入或隔离恶意DLL，真实鼠标/键盘验收保留。AI/Assistant继续只读，旧人工未迁移入口保持原行为但不宣称已迁移。参考工程本次访问被拒绝，历史main/67eacf5身份不是当前重核结论。原浏览器20失败、workspace首次20.05秒/20秒门槛及Native输入首次失败仍独立未定位。

## P2 A+B未提交增量（2026-10-22）

最新C1 observer独立整改（2026-10-23）：仅修复legacy异步结果observer初次enable窗口，ui.c在摘除pending前持有独立host旧回调租约，直到observer及pending清理结束；不占能力dispatch嵌套标记。成功/失败异步reply修复前5331检查/6失败；修复后扩展回归5349/0，固定失败槽在8MiB仍满时连续3次安全借用；正常read可从已启用模式的legacy observer同步执行。新Native17/17、Light17/17、ASAN11/11，详情见[C1第8节](validation/xuanshu-p2c1-validation.md#8-2026-10-23-legacy异步结果observer独立整改)。新目录独立保留修复前后源码身份、日志/JUnit及匹配二进制，没有倒补旧48失败red的缺失二进制。公共合同、版本、限额和依赖不变；历史浏览器20失败、workspace首次20.05秒超时、Native输入首次失败仍独立未定位。当前停在C1复审，未进入C2。

最新C1评审收口：在实际HEAD `e0eca03db25930be55bdb45477027d1793a61904`及原A+B/C1工作区上，仅修复旧Assistant回调激活窗口、事务辅助API修改旁路及失败桥接预算/通知重入三项。独立旧回调租约阻挡初次enable和关闭，派发前重查策略；启用后begin/commit/rollback/undo拒绝，普通人工命令和未迁移legacy保持。pending/source实际计账贯穿失败observer，原8MiB内固定失败槽及接纳前背压封闭递归通知。没有新增公共接口/错误码或改变版本布局。新证据见[C1第7节](validation/xuanshu-p2c1-validation.md#7-2026-10-22-c1评审收口独立证据)：Native17/17、Light17/17、ASAN11/11；新增回归5259检查/0失败，原C1入口20923/0。修复前有界回归5214检查/48失败及一次ASAN目标选择错误保留，不宣称复现崩溃。旧浏览器20失败、workspace首次20.05秒超时（20秒门槛）、Native输入首次失败独立未定位；限定通过不覆盖首次。当前停在C1整改后评审，不进入C2。

最新C1独立增量：同一HEAD及已有A+B工作区上，新增`capability_entry.h`的enable/publish/request_call和单一read目录投影；普通命令与动态Assistant视图共享ID/Schema/目标/注销生命周期，旧请求关联先于execute。启用只读模式的宿主拒绝legacy Assistant修改，保留普通人工入口；未迁移宿主保持legacy，本批没有迁移五示例或browser。结果租约、稳定ID、宿主共享1024请求和原预算、workspace下一安全poll回收均已验证。新证据见[C1独立验收](validation/xuanshu-p2c1-validation.md)：Native限定16/16、Light限定16/16、ASAN10/10，C1单测20923检查；首次测试断言/工具命令失败保留。原浏览器20失败、workspace首次20.05秒超时、Native输入首次失败继续独立未定位。版本及v1布局不变；当前停在C1评审，C2及后续未实施/未授权。

以下原交接保留其历史基线；当前工作区另获XuanShu P2 A批+B批最小只读闭环实现授权，基准HEAD仍为 `e0eca03db25930be55bdb45477027d1793a61904`。源码现为SDK0.10-dev/API10/标准10，ABI1/包1和旧公开结构保持。新增独立capability.h、固定yyjson0.13.0、严格Schema/JCS与只读目录/资源/调用/关闭门；只做本地构建测试，无提交、推送或远程工作流。

本轮范围、运行结果和失败见[A+B独立报告](validation/xuanshu-p2ab-validation.md)；三份整体设计仍为Proposed，确认决策和实现子集已分别标明。UI绑定、草稿、业务写入、五示例能力迁移、AI Service及P3—P6均未完成或授权。只同步当前包声明，不重做browser能力。

保留原.gitattributes修改、所有历史未跟踪材料、参考工程、固定依赖和冻结SDK。既有ui_browser_host的输入/最近文件失败及本轮一次workspace_offscreen超时单列，不用后续限定通过宣称根治；其他历史遗留继续按下文分账。下一步停在A+B代码评审。

后续评审收口已修复最后一次应用回调后的modal执行门，补真实DLL自有worker/独立受控回调、同步完成栈上租约、8MiB复制队列背压与重开实例迟到消息验证；公共合同和版本不变。新证据独立见[A+B报告第7节](validation/xuanshu-p2ab-validation.md#7-2026-10-22评审收口独立新证据)：Light限定15/15、ASAN含真实DLL4/4；Native完整25通过/1输入断言失败/1跳过，输入单项后续通过但首次根因未定位。原workspace超时门槛文字纠正为20秒，历史失败不删除。仍停在A+B整改后的代码评审，不进入下一批。

## 基线与仓库内容

接手 HEAD 为 `689fa03b34f8c4321a6c838f52ca9dcd790a9319`，分支 `codex/p1-process-boundaries`。本页所在的后续提交保存菜单/提示修复及此前已完成的受限浏览器集成；实际恢复时先核对 Git，不把旧工作树哈希当成当前提交身份。

SDK0.9.0-dev/API9/开发标准9、应用ABI1、包格式1、Windows x64/C11。新开发能力需要同一快照的头、导入库、DLL、宿主和应用包，冻结SDK不变。公共合同见[版本政策](version-policy.md)、[实例标题和主题](instance-title-and-theme.md)、[受限文档](restricted-browser.md)。

Git保留源码、资源、构建/测试程序和必要说明；本轮不新增日志、截图、转储、SDK或证据目录。以前已入Git的历史材料不删除。`build/`、`.deps/`及应用外部交付不是clone内容；下面的本地证据不存在时如实报告，不重新生成冒充原件。

## 已完成与证据边界

| 项目 | 当前结论 |
| --- | --- |
| 正式宿主菜单留白 | 实际使用字符数估宽，按钮间距为0，留白来自内部宽度；改为同字体/DPI的实际测量加点击留白。48项明暗/程序96、144、192几何检查通过，溢出、Alt/Enter和真实关闭按钮验证通过。 |
| 无关状态更新关闭提示 | 公共C最小复现确认 `replace_caption` 触发host级隐藏。改为组件归属失效；保留公开host级隐藏和必要生命周期清理。 |
| Light父子命中提前离开 | 进入子span误报父按钮leave；改为共同祖先边界事件。公共分发、真实Light及Runtime对照验证。 |
| 所属错误文字更新后提示陈旧 | `set_error` 缺失必要失效，已补齐。重新悬停显示新文字。 |
| 动态标题、主题、文本替换和稳定几何 | 早于本次持久化已完成，不重复开发；标题/主题合同及测试源码已保存。Runtime真实同值粘贴的历史失败仍单列，不外推所有编辑路径已根治。 |
| 受限浏览器 | 既有独立批次已完成M0/M1/M2，采用公共C、Light、Lexbor、WinHTTP/WIC；远程页面无JS/应用桥。框架默认构建必须包含 `apps/browser`、`light_document.h`、`app_data.h` 和受限实现，不能只推共用文件而漏掉它们。详见[应用README](../apps/browser/README.md)、[M2支持表](../apps/browser/docs/m2-support.md)和[M2历史验证](../apps/browser/docs/m2-validation.md)。M3—M5未完成。 |

菜单/提示专项是2026-10-19历史测试，不是本轮重新测试：相关CTest首轮23/24通过，旧溢出夹具补足菜单数据后该项1/1通过；ASAN7/7通过。Light/Runtime×明暗四组最终生命周期父进程OS0，所观察的12个Runtime子进程OS0。248导出前后相等，无新增公共接口。100次布局GDI119→119、HANDLE191→191；40次菜单重开HANDLE289→289、GDI857→857、USER50→50。

该专项源码/资源SHA256为 `9ecb410caca793dcea787f20193f62fb899dfddba4f4393f588963de3bc75328`；随后浏览器改动已改变4个共用文件，不能把该SHA称为当前全部工作树身份。原报告在本地 `docs/validation/menu-tooltip-20261019-validation.md`；原始材料在 `build/menu-tooltip-20261019-5a840d43`，含首次失败、匹配PDB/转储、424张必要真实画面及哈希清单。历史24文件开发ZIP的SHA256为 `f035711b0c3350d1d5cba9a08cb11bad5c1ba211e72a57748cd481b54f8d3f0e`，不是当前浏览器集成SDK，不能补写。

浏览器M2历史验证包含生产tests OFF/WebView2 OFF/Mesa空配置、匹配SDK独立构建、公共C阅读/隔离/应用调用方和关闭真正卸载。不是本轮新的完整框架回归；其外部deliverables路径不会随框架clone提供。应用复制中的独立工作区打包/同步脚本可能引用原维护目录，框架clone构建以根目录CMake和下节命令为准。

## 构建恢复

在MSVC x64开发终端，按[构建说明](build-and-validation.md)准备固定依赖后：

```powershell
cmake -S . -B build/current -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/current
```

固定Lexbor `7fb22cf5664a331d7c24b113489e566767c9c25a`、QuickJS-NG `2f0aa72a6b09cf69ff06399bcf9f4c083ac1a278`。WebView2 SDK `1.0.4129.50` 和Mesa `24.3.4`只在相应配置需要；默认不替换provider、不自动下载升级。完全不使用Web的构建需同时关闭 `UI_BUILD_STANDALONE_HOST`、`UI_FRAMEWORK_ENABLE_LIGHT_WEB`、`UI_FRAMEWORK_ENABLE_WEBVIEW2`。

本次源码完整性检查及结果见[2026-10-22推送核验](validation/handoff-20261022-validation.md)。它只证明列出的构建和检查，不替代真实桌面/物理条件或完整历史矩阵。

## 遗留问题分账

- 额外正式宿主Alt+F4脚本：入口及候选在10秒内均未关闭，前台PID正确，但缺完整按键Dispatch链；责任尚未定位。诊断器只终止自有目标为254。后续真实关闭按钮OS0不能解释该失败，亦不能据此认定已进入Mesa退出。
- Mesa默认A的 `lp_rast_destroy → util_semaphore_wait` 退出等待历史已复现，候选C一次目标A/C自然退出；不能写成所有退出均未定位或所有托管超时已解决。B/C目标对照、原子259→0、多线程正常screen清理及真正UNLOAD仍待闭环。LP_NUM_THREADS=0是因果对照且有性能代价，不是原多线程修复；C未纳入默认provider或SDK。
- Runtime144冷write曾主OS0但两子进程原10秒仍259，未及时取得失败转储；后续限定对照通过不解释首次失败。Runtime96真实同值粘贴后Y丢失也仍未定位。产品Runtime、首次restore144和诊断mirror pending分开追踪。
- 泛化192叠层、窄表头残影未独立复现；已修的Runtime144捕获边界与诊断截图时序不能冒称显示根因。持续重绘/瞬态Runtime保留各自边界，不能凭单个idle paint0判根治。
- 受限浏览器M01公网12002、早期OLE恢复崩溃、旧测试证书导入不持久化的底层原因仍未定位；本次不修改网络或信任配置。
- 当前源码Session0、严格整机无登录、真实英文Windows/中文IME、物理高/混合DPI、多屏、桌面合成专项、硬件鼠标/外部捕获抢占和长期人工仍独立待验。自动输入和程序DPI不能替代。

## 后续授权与保护

本次授权仅持久化及推送；不自动继续开发、运行远程CI、合并main、发布或建标签。下一轮由用户指定。先恢复实际Git和源码身份，保留所有既有修改与未跟踪文件。新工作使用日期加唯一标识目录，回退只能撤回该轮新增改动。

业务 `D:\应用软件框架\klayoutC`、PERF-001、固定依赖、冻结SDK和历史证据保持只读。不使用子代理、新会话、graph-engineering、computer use、私有调用方接口/JS；不停止未知进程、改服务或注销用户。不放宽断言、超时、资源和数据范围。真实前台输入须重新协调空闲解锁、串行运行，分别核对Dispatch、最终状态和OS退出。收尾仅清理本轮无用途过程件，保留明确要求及未解决问题的最小证据。



## 2026-11-05 BR-1因果定位checkpoint

新目录build/xuanshu-br1-causal-20261105-6e20b4；改为最小输入/焦点/投影/原生字符队列对照，Light与ASAN串行各115检查通过。未复现错误、未确定责任，未修改产品，冻结阻断保留。明确排除原夹具逐字符漏泵假设（原调用为整串输入）；保留未归属原生编辑与未捕获缓冲区改写候选，不归因环境。详docs/validation/xuanshu-br1-causal-validation.md。不能宣称整改完成；启动取证8项仅按身份复用。



## 2026-11-05 BR-1首次写入归属checkpoint

新证据build/xuanshu-br1-writewatch-20261105-92d6b1。已核对前置identity。x64 CDB启动受拒，SDK x86 CDB目标启动error50；未提权。隔离写点内存追踪关联Light节点、配置worker和任务绑定，消息泵保留第一差异；修复了新观察器跨视图误判并保留首败。最终Light5/5、ASAN2/2仅是诊断路径结果；未捕获原15→14异常写入，未确定根因，冻结阻断保留。不得宣称整改完成。见docs/validation/xuanshu-br1-writewatch-validation.md。启动8项只复用，不恢复已取消人工测试。



## 2026-11-05 BR-1原条件区分checkpoint

原closure-final CTest内43例86条Start/结果无重叠，RUN_SERIAL与原程序哈希匹配；外部事件未知。本轮修正观察器生命周期token、实际派发及直接宿主提交保护，并验证完整合法字符串变化的第一差异锁存；不是原少字符根因。直接保存/实际滚动投影/同view重建及任务恢复链均未取得15→14异常。条件对照最终Light与ASAN各4/4，原30秒不变。首次滚动坐标夹具失败保留。根因仍未确定，不具备整改完成独立复审条件。详docs/validation/xuanshu-br1-conditions-validation.md；证据build/xuanshu-br1-conditions-20261105-3f8a60。启动8项按身份复用，不恢复取消的人工控件测试。


## 2026-11-05 BR-1保存所有权checkpoint

本轮build/xuanshu-br1-ownership-20261105-5b7d20。原配置文件已被原测试后段改为revision2，不能还原第一次保存。使用实际HOST_ACTION_MESSAGE屏障和worker锁屏障，完成plain/queued-edit/worker-edit/queued-rebuild因果对照；逐字节及NUL显示排队action、pending、读回、任务绑定独立且完整。最终Light4/4、ASAN4/4及本轮库核对，仅排除所测借用/源变动假设，未找到原少字符根因、未改产品，冻结阻断保留。见docs/validation/xuanshu-br1-ownership-validation.md。启动8项仅身份复用。


## 2026-11-05 BR-1整串输入矩阵checkpoint

证据build/xuanshu-br1-input-matrix-20261105-7c4a18。输入raw/转换/临时缓冲/写入/回调/布局六边界，七组Light与ASAN最终各7/7，包含实际节点地址迁移及1/15/17/127字节、UTF-8、非零选区。合法选区与完整15字节输入下替换长度不可能小于15是条件性源码证明，不是原现场捕获。未捕获原异常、未定位根因、没有产品修复，冻结阻断保留。生产357导出无私有探针；本轮28个自有库无任务。详docs/validation/xuanshu-br1-input-matrix-validation.md。自审限定矩阵目标只在探针配置注册；原失败及启动8项成果保留，取消的人工控件测试不恢复。


### 同轮完整链补充（不覆盖上述阶段记录）

七组矩阵后已执行一次persisted-chain/配置，将输入内部六边界连接页面提交、宿主、worker、任务及恢复。最终chain Light8/8、ASAN8/8；每配置persisted-chain215检查零失败。此前矩阵成功程序单独保存在matrix-programs-before-chain。关后共44自有库：42无任务；2个完整链库各2stub completed及1模型cancelled，模型均完整，0工具。汇总脚本首个任务数量假设错误已区分记录。仍未捕获异常或确定根因，不能报告整改完成。


## 2026-11-05 BR-1用户接受遗留并继续推进

用户明确将模型名少字符作为遗留，后续再现时解决。BR1-KI-001状态为未解决、用户接受的非阻断问题；停止主动重复定位，不再单独阻断本地交付。历史首败及原复审结论不改写，不称修复或独立通过。当前生产入口使用build/xuanshu-br1-consistency-20261105-f4c92a/start-readonly.ps1（本轮仅哈希核对、未运行），正式宿主/生产DLL/业务包及启动8项关联身份匹配。下一项Proposed为真实验证请求总数硬门，仍需独立实现授权；真实调用另行集中确认。详docs/validation/xuanshu-br1-known-issue-disposition.md。只改文档，不测试、不启动、不读凭据/数据库、不清理历史材料。
