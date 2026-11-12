# 当前阶段：通用助手限定复审及冻结完成

2026-11-12追加文档入口。APP开发参考、迁移/构建及示例已完成同步，当前阅读顺序见[项目README](../README.md)，静态核对与文档增量见[同步记录](validation/xuanshu-application-docs-sync-validation.md)。匹配产品冻结与Proposed真实包各保留原身份。

2026-11-12。本轮限定独立复审通过，必须修正项为空；按用户授权将匹配快照冻结记录落盘。快照`build/xuanshu-general-layout-20261112-6b37c1`，delivery-identity SHA256 `2C447E95741824E2C2A94E3EF91EC1782CF75C5870E262D7831C44989C7029CA`。原开发identity及源码/程序/日志清单保持原字节，历史待复审状态不回写。详[独立复审](validation/xuanshu-general-layout-independent-review.md)、[冻结记录](xuanshu-general-layout-freeze.md)。

独立核对447份测试范围源码、154项程序及3313项证据，逐次测试前匹配关系保持。Light100/100；ASAN100项分次覆盖（业务19.7347秒、另次98项、glyph最终1/1）；Native47/47按依赖复用，不是本轮重跑。生产371项导出与前置一致。首败、环覆盖687条、原DB/WAL及历史限制保留。BR1-KI-001仍未解决且用户接受非阻断；取消的临时页面控件测试不恢复。

真实验证准备为[GENERAL-REAL-20261112-a74e20](../build/xuanshu-general-layout-review-20261112-a74e20/authorization-package.md)，仍Proposed。公开官方模型/协议/价格及计量源码当日核对一致；根最多8次、10分钟、USD0.10逐轮实际预留，预算不足即停止，不保证8轮全部完成。包内浏览器读取加自有维护单元格74及本地撤销42，不能称为纯只读。初始请求、有限计划及撤销各自适用本人可信确认；旧真实批准不复用。本轮无构建/测试/生产启动/计量执行/数据库打开/凭据访问/Provider请求。

新准备命令只静态审阅，未动态执行；会话尚未创建，策略未加载。下一步仅等待用户对具体包另行明确批准，不能把本轮冻结或准备授权解释为发送批准。详[准备事实与费用边界](validation/xuanshu-general-layout-real-preparation.md)。

## 此前时点原文保留

# 当前阶段：Light局部成本整改完成，等待限定独立复审

2026-11-12。已按追加授权完成一次分段基线及一次单条件对照，并据因果结果完成最小产品修复。基线原30秒Timeout；重复按钮字形GDI测量的单条件复用使完整对照20.52秒通过。工作区只改`src/backends/light_web.c`及`tests/light_glyph_padding.c`，不采用前轮无效DC复用，不跳过布局、投影或确认。

最终无本轮探针：Light100/100；ASAN原完整业务19.7347秒通过、另次98项通过、新增字形夹具修正后1/1通过，共100项逐项覆盖，不能写成首次单次100/100。Light完整业务7.64294秒。两次新增夹具失败与原日志/JUnit/匹配程序保留。Native47/47仅按未涉及Light源码及实际编译依赖复用，未重跑。

Light和ASAN关后各3任务、6消息、74事件、1图、2编辑回执；实际74/doc-2、撤销42/doc-3和记录恢复经原完整用例断言，进程正常退出。关后immutable观察无WAL/SHM且前后哈希相同。图/编辑事实不冒充通用tools表记录；六轮属于隔离模型替身，不是真实HTTP计数。371项生产导出、固定依赖、公开合同及预算保持。

匹配候选`build/xuanshu-general-layout-20261112-6b37c1`，identity与清单独立形成，等待限定独立复审；不自行冻结或声明真实服务通过。对照环最早687条覆盖、历史超时和失败均保留，不宣称解决所有未知期限根因。BR1-KI-001仍未解决且用户接受非阻断；临时页面控件测试保持取消。真实质量包Proposed、未核价锁停，无真实凭据、Provider或人工验收。

详[因果证据及最终验收记录](validation/xuanshu-general-layout-causal-validation.md)、[最小合同](xuanshu-general-layout-contract.md)与[候选交付清单](xuanshu-general-assistant-delivery-manifest.md)。仅清理本轮983项对象/资源中间件和派生只读观察副本，共451365506字节。保留所有本轮原DB及旁文件、必要失败源码/程序、原始日志/JUnit与trace，未清理历史材料。编译依赖VALID为清理前捕获，清理后对象缺失另行说明。

## 此前时点记录原文保留

# 当前增量：ASAN布局成本已定位到子系统，尚未形成有效修复

2026-11-12。已按集中授权执行一次ASAN完整基线及一次单条件对照：30.0689/30.0615秒Timeout，CTest均8、测试实际均255由CTest期限终止。固定512KiB trace完整保留。Light布局699次，CPU17.750/17.719秒，UI线程累计26.172/26.453秒；本次主要消耗在宿主投影与Light布局，仍有业务推进，不是稳定停滞的证据。同次递归复用测量DC没有解决期限，隔离对照未应用到工作区。

未取得更细责任函数及有效修复，完整ASAN业务仍阻断。已用完本轮两次诊断额度，不追加运行或延长30秒。下一最小范围仅Proposed：在Light布局内区分规则/自然尺寸与几何/文本区测量，并据证据选择单处整改；新诊断运行需另行授权。原候选C2379C435E0684A1EFC025A85A782397AF5B0D1CE4B3A49E44ABA7E84862C6ED及源码/程序保持。

详[因果实验记录](validation/xuanshu-general-asan-causal-validation.md)。没有新的产品验收、冻结或真实验证结论。BR1-KI-001及已取消临时控件测试状态保持。

## 此前记录保持

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

# PB-3本次真实验证及交付收口状态

2026-11-11。PB3-REAL-20261111-6c4f82唯一真实SQLite隔离副本阅读已完成并正常关闭，限定业务闭环通过。唯一PID8976实际exit0；完整847字节正文和revision、固定范围及工具保存逐值对应。关后1任务、2消息、31事件、2工具，唯一completed；3次占用、非模拟传输入口和完整响应，unknown0，本地保守费用USD0.0016113，未结预留0。该具体包已消费，当前没有运行宿主、待Key或待确认操作。详[本次真实验证及限制](validation/xuanshu-page-reading-real-20261111-validation.md)和[执行证据](../build/xuanshu-page-reading-real-run-20261111-6c4f82)。

冻结快照identity 4A892228066D04EB09972594509892AF3B960CF32569DE33A125F11BEF611A42及546源码/195产物保持原身份，本轮完成具体批准、本人原生Key及确认、一次生产验证、正常关闭、immutable关后核对和文档归档。真实回复的核心特性与无待办结论符合原文，其中“嵌入式/进程内”属于未标注推断，来源声明存在一项质量瑕疵，单独保留。账户扣费、全程抓包和原站实时网页等边界保持。

后续新任务、网页和功能仅列Proposed，需独立具体授权，原默认入口仍未核价锁停。历史ASAN首败、BR1-KI-001、计量任意卡死及其他未验边界保留，临时控件测试维持取消。

## 以下各时点记录保留原文

# PB-3当前冻结及真实验证准备状态

2026-11-11。PB-3限定独立复审通过，匹配快照冻结记录与本地交付材料已齐备。identity 4A892228066D04EB09972594509892AF3B960CF32569DE33A125F11BEF611A42，546源码和195项程序/链接产物匹配，343条最终case对应既有Light168、ASAN168、Native7及分别记录的补项。新复审及[本日具体包](../build/xuanshu-page-reading-real-preparation-20261111-6c4f82/authorization-package.md)已归档，PB3-REAL-20261111-6c4f82仍Proposed。12份当日官方依据相同，三次本地计量exit0，三次总保守预留USD0.0327456。等待一次具体批准，当前没有执行目录、生产进程、Key或真实请求。默认入口仍锁停。

首轮ASAN五项摘要失败、两次期限根因未定位、首轮PDB缺失、96主样本和其他旁文件的范围限制、BR1-KI-001及计量任意卡死边界保持。临时控件测试维持取消。详[限定复审与冻结](validation/xuanshu-page-reading-independent-review.md)、[入口](xuanshu-page-reading-entry.md)和[清单](xuanshu-page-reading-delivery-manifest.md)。

## 复审之前各时点记录，原文保留

> 2026-11-11。PB-3公开SQLite Zero-Configuration隔离副本阅读开发、自审和离线验证完成，等待限定独立复审。候选根build/xuanshu-page-reading-20261111-a18d74，identity SHA256为4A892228066D04EB09972594509892AF3B960CF32569DE33A125F11BEF611A42。原始HTML4209字节、实际DOM正文847字节；同一正式宿主/Service/browser/read/存储链保持。最终逐项Light168、ASAN168、Native7及清理后生产公开read各另次1/1，运行与首败分账。96份阅读主样本无旁文件、immutable前后哈希一致；另10旁文件（6非空）保留。真实验证包Proposed，默认未核价锁停，本轮没有真实模型请求、凭据读取或人工验收。前置PB-2批准已消费。 见[PB-3入口](xuanshu-page-reading-entry.md)、[验收与风险](validation/xuanshu-page-reading-validation.md)及[交付清单](xuanshu-page-reading-delivery-manifest.md)。

## PB-3之前的历史状态

> 2026-11-11。PB-2匹配快照冻结记录已归档，PB2-REAL-20261111-8f3c2a唯一真实正文任务已完成并正常关闭。生产PID28300实际exit0；147字节正文、revision、12/9/3、林青与周明及复核三台设备逐值对应。关后1任务、2消息、31事件、2工具，唯一completed；3次占用、非模拟传输入口和完整响应，unknown0；本地保守费用USD0.0010926，未结预留0。该包已消费，当前没有本轮运行进程、待Key、待确认或继续执行操作。后续新真实任务需独立具体授权，原默认模板维持Proposed锁停。 详见[本次完整结果](validation/xuanshu-page-body-policy-real-20261111-validation.md)和[证据目录](../build/xuanshu-page-body-real-run-20261111-8f3c2a)。

## 以下历史记录按各自时点保留

> 2026-11-11当前状态：PB-2匹配快照已限定独立通过，冻结记录已归档。见[冻结记录](validation/xuanshu-page-body-policy-independent-review.md)及[本日准备](validation/xuanshu-page-body-policy-real-preparation-20261111-validation.md)。PB2-REAL-20261111-8f3c2a仍Proposed，等待一次具体批准；没有生产执行或真实请求。11月10日正文包已失效，以下原文按当时状态保留，不构成当前操作或授权。

# PB-2正文验证保护开发收尾 checkpoint

2026-11-10。PB-1原快照限定复审/冻结已匿名归档；PB-2新候选完成最小场景路由、开启/任务双标记及READY前恢复保护。Light逐项201、ASAN逐项201、Native7，不合并为首次单次全过。新程序/源码/证据身份见build/xuanshu-page-body-policy-20261110-d42b8e及[验收记录](validation/xuanshu-page-body-policy-validation.md)。

具体正文真实包Proposed已准备，但新候选独立复审及集中真实批准未取得，入口未激活、不读取Key或发送。当前停在开发交付与限定复审准备，旧包消费/遗留/取消控件测试及延期边界保持。

## 历史阶段记录（保留）

# 当前阶段索引：PB-1开发收尾

2026-11-10：[当前网页DOM正文只读摘要](xuanshu-page-body-entry.md)开发及离线逐项验证完成，候选build/xuanshu-page-body-20261110-a64c28，等待限定独立复审。见[合同](xuanshu-page-body-contract.md)和[验证、首败与证据](validation/xuanshu-page-body-validation.md)。未冻结、未真实Provider或人工验收；新范围不继承旧授权。

BR-2阶段继续保持完成，旧真实包已消费。BR1-KI-001未解决非阻断，临时控件测试保持取消。下一步只建议独立复审；真实正文保护策略、官方依据与集中授权另行准备，不自动进入。

## 先前阶段关闭记录（原文保留）

# 当前阶段关闭索引

2026-11-10：**BR-2本阶段已完成，最终限定收口复核通过，必须修正项为空。** 已冻结基线为`build/xuanshu-br2-discovery-20261110-7e4a96`，delivery-identity SHA256为`3DFFCFC515A9958A69396AE703307D6B9E84575D63DD921322D1882E5086F545`。

- 唯一当前程序和使用入口：[BR-2只读交付入口](xuanshu-br2-discovery-entry.md)。
- 匹配快照：[限定独立复审及冻结记录](validation/xuanshu-br2-discovery-independent-review.md)。
- 最终结论：[收口复核归档及阶段关闭](validation/xuanshu-br2-discovery-final-closeout-review.md)。
- 已完成真实业务及保存/关闭证据：[BR2-REAL-20261110-111dd1](validation/xuanshu-br2-discovery-real-validation.md)。

没有进行中的本阶段宿主/Service或待用户Key/确认步骤，已消费具体包不续跑；原模板仍Proposed锁停。当前结论不等于整个系统、任意业务应用或正式发布通过。BR1-KI-001和历史首败保留，临时页面控件测试保持取消。

下一方向仅Proposed“当前网页正文只读摘要”，需要独立数据/能力/保护规格及开发、真实调用授权；本轮不进入。此次仅归档复核和整理文档，没有新测试、启动、数据库重开或请求。

## 历史记录（保留原文）

以下内容按各自记录时点保留。旧的“待批准”“待确认”“执行中”和“下一步”均为历史，不构成当前操作或授权；当前状态以上方阶段关闭记录为准。

---

> 2026-11-10本次已完成：[BR2-REAL-20261110-111dd1真实业务只读限定验证通过](validation/xuanshu-br2-discovery-real-validation.md)，目录分类匹配快照冻结记录和本地交付收尾已完成。唯一PID35088正常exit0；实际空分类发现后执行browser.snapshot，标题/browser:start/revision逐值一致。关后1任务/2消息/31事件/2工具、唯一completed，3次占用/非模拟传输入口/完整响应，unknown0；本地保守绑定价USD0.0014961，不是账户账单。该包已消费，不再启动/提交/确认/读Key；后续真实调用需新具体授权。原模板仍Proposed锁停，旧真实失败/遗留保持。以下历史状态各按原时点保留。

> 2026-11-10最新准备：目录分类快照冻结记录已落盘；[BR2-REAL-20261110-111dd1具体授权包](../build/xuanshu-br2-discovery-real-preparation-20261110-111dd1/authorization-package.md)已准备，尚未批准。执行目录/可执行监督命令未创建，不启动、读凭据或发送。用户仅负责本包批准、必要原生Key及一次完整范围可信确认，其余普通操作和收尾由助手处理。[准备记录](validation/xuanshu-br2-discovery-real-preparation-validation.md)。旧真实失败及旧包消费保持。以下历史状态不自动继承。

> 2026-11-10阶段索引：[目录分类整改限定复审及冻结](validation/xuanshu-br2-discovery-independent-review.md)已记录，匹配快照可以冻结。真实重验与本地冻结分开；新准备BR2-REAL-20261110-111dd1待具体包批准，不自动进入真实调用或其他阶段。

> 2026-11-10分类契约限定整改候选已完成开发、自审及离线验证。新根build/xuanshu-br2-discovery-20261110-7e4a96，identity 3DFFCFC515A9958A69396AE703307D6B9E84575D63DD921322D1882E5086F545。分类说明明确空分类只枚举允许read，非空无匹配返回明确提示；直接相关的私有池缩容越界已用独立哨兵和空白名单ASAN因果定位后最小修复。最终Light118/118、ASAN118/118、Native49/49分次通过。
> [本轮验收与首败](validation/xuanshu-br2-discovery-validation.md)。原SSE夹具/空访问、有效分类红例、独立池哨兵红例及ASAN内存异常分别保存，最终通过不覆盖首败。新代码不继承旧冻结，真实业务重验仍需后续独立授权；不自动进入下一阶段。

> 2026-11-10本次真实业务验证已结束：包BR2-REAL-20261110-4da059已明确批准并仅执行一次，PID23608实际exit0。唯一任务运行时completed并保存，但只执行category=browser的目录查询，browser.snapshot注册于browser.read，实际返回空目录且未调用snapshot，标题/来源/业务返回revision未取得。本次真实业务验收及交付收口暂不通过；原限定冻结和离线结论保持原适用范围，不追加请求、不重启、不修改产品。
> [本次阶段收尾](validation/xuanshu-br2-meter-real-20261110-validation.md)。无越界数据/重复任务或额外请求的当前持久化迹象；没有全程网络遥测，不声称精确HTTP次数或服务端账单。BR1-KI-001、ASAN首败和其他历史限制保持。

> 2026-11-10阶段索引：BR-2冻结对象不变，当天准备和唯一集中包已整理，停止于明确批准之前；11月9日包仅作过期参考。
> [当天身份/官方资料差异、费用和执行命令](validation/xuanshu-br2-meter-real-preparation-20261110-validation.md)。未恢复已取消控件测试，历史首败及非阻断遗留不改写。

> 2026-11-09阶段索引：BR-2逐轮完整计量匹配快照已限定冻结，本地交付材料齐备；一次真实验证前置核实与具体集中包已准备，停止于授权确认之前。旧阶段和首败不被覆盖。
> [计量冻结记录](validation/xuanshu-br2-meter-independent-review.md)；[交付清单](xuanshu-br2-meter-delivery-manifest.md)；[本日公开资料和待授权包](validation/xuanshu-br2-meter-real-preparation-validation.md)。

> 2026-11-06 BR-2逐轮完整计量开发完成。候选build/xuanshu-br2-meter-20261106-1f8148接入官方完整Responses转换及每轮实际body发送前门，新增组件独立128MiB限制，原确认、Service及预算合同保持。Light主轮127/127，ASAN主轮121/127并另次原程序6/6对照，最终计量取证补项各20/20，Native38/38，组件15/15。首轮frontend中断/超时根因未定位，旧计量日志的中文路径截断已修为UTF-8，相关旧源码/程序保留。新身份等待限定独立复审，真实策略仍Proposed锁停。详[计量开发记录](validation/xuanshu-br2-meter-validation.md)及[候选入口](xuanshu-br2-meter-entry.md)。原冻结及历史结论保持其原适用范围。

> 2026-11-06 BR-2编码规范续查。已确认官方Recipe与API处理逻辑对应及模型文本tokenizer字段一致，初始参考内容496 token。当前止于动态续轮发送前计量接入，原准备授权内没有修改冻结产品。新增计量开发、构建及限定独立复审需独立授权，真实执行和凭据仍未授权。详[续查记录](validation/xuanshu-br2-encoding-preparation-validation.md)。

> 2026-11-06 BR-2真实验证准备停止于前置条件。官方模型/协议/价格及生产发送链已核对，可靠计费input上界尚未成立；当前没有真实业务结果或服务验收结论。匹配快照冻结与本地材料结论保留，新准备材料及12项辅助自检见[本轮前置记录](validation/xuanshu-br2-real-preparation-validation.md)。后续权威计费依据、必要实现、集中授权及真实执行分别待授权。

> 2026-11-06 BR-2费用恢复匹配快照冻结与本地交付收尾。费用恢复兼容限定验收通过，匹配build/xuanshu-br2-fee-20261106-54d9a8的冻结记录已落盘，本地交付材料齐备。必须修正项为空，详[独立复审及冻结记录](validation/xuanshu-br2-fee-restore-independent-review.md)和[当前交付清单](xuanshu-br2-delivery-manifest.md)。本阶段止于匹配快照及本地交付准备，真实验证保持Proposed、未核价锁停。下文保留各旧阶段原始状态。

> 2026-11-05 BR-2最新候选：验证专用3次总额度、10分钟及费用/unknown硬停止开发和离线验证完成；Light73/73、ASAN73/73、Native23/23分次通过。新身份与旧BR-1分开，未独立通过/冻结。当前入口价格及可靠计费上界未核实，真实发送锁停，待集中授权。BR1-KI-001仍未解决、用户接受非阻断。见[开发验收](validation/xuanshu-br2-validation.md)、[入口](xuanshu-br2-entry.md)、[Proposed授权包](xuanshu-br2-real-proposal.md)。下文为历史记录，不改写旧首败或旧结论；已取消临时控件测试不恢复。

> 2026-11-05 最新用户决定：模型名少字符改列BR1-KI-001“未解决、用户接受的非阻断遗留问题”，后续再现时定位；不再主动重复排查或阻断本地交付推进。不是问题已修复，也不追改历史复审为通过。当前本地入口采用consistency候选已验证的启动监督，详[遗留与推进记录](validation/xuanshu-br1-known-issue-disposition.md)。真实请求硬门及真实调用仍需分别授权，本轮不执行。

> 2026-11-05 BR-1一致性整改：启动异常取证已完成并以隔离辅助进程8/8验证；模型名少字符仍未定位，冻结阻断保留。新增逐边界探针及相称回归通过不能替代原根因；新候选build/xuanshu-br1-consistency-20261105-f4c92a仅诊断/监督交付。详[记录](validation/xuanshu-br1-consistency-validation.md)。不进入真实调用或人工验收。

> 2026-11-05 BR-1业务接入候选：实际受限浏览器读取新合成交接单标题/地址/revision，正式宿主隔离模型闭环及生产公开read验证完成。Light43/43及最终业务6/6；ASAN首轮41/43、补项及最终业务通过，持久化摘要少字符首次原因未定位，独立保留。新交付build/xuanshu-business-read-20261105-8c6e21等待限定独立复审，不继承DH-1冻结。见[开发验收](validation/xuanshu-business-read-validation.md)、[业务入口](xuanshu-business-read-entry.md)。未读取真实凭据、调用Provider或启动人工验收。真实验证请求总数硬门仍需独立授权；临时控件测试取消状态保持。

> 2026-11-05 DH-1冻结与只读交付收尾：承接此前限定独立复审通过、必须修正项为空的结论，419项源码/74项产物及清单身份核对匹配，快照65823A92BAAF49CE2623F5865F2C47B678215DD813D7E8EDDC8725980107A532的冻结记录已落盘。详[复审记录](validation/xuanshu-daily-host-independent-review.md)、[交付清单](xuanshu-readonly-delivery-manifest.md)、[使用说明](xuanshu-readonly-user-guide.md)。仅文档收尾与身份核对，无新测试、启动、凭据或请求；未重新读取数据库。当前只读交付材料齐备，不等于正式发布或整个系统完整验收。后续有Key验证、产品改进仅Proposed，需独立授权。
> 下文保留各阶段原始记录；其中“等待复审”“未冻结”“方案待确认”及旧快照入口属于历史状态，当前交付入口以上述清单为准。临时页面控件测试已取消，不恢复、不列为待办或阻断；历史失败和独立未验边界保留。

> 2026-11-04跨标签补测最终通过：同冻结生产web_counter只作被动合成第二标签；用户截图证明RP-C→Web Counter #2→RP-C，两个实时诊断task/model逐值一致，run/client/instance/host-open epoch/resource/r1/read scope及字段不变，仅展示代次21→23。返回后取消，PID13776实际exit0；新库1任务/2消息/3事件/0工具，唯一cancelled/committed终态，无确认/请求保存/工具派发；DB SHA25697886179…8B3BC0D，immutable前后不变、无WAL/SHM。承接此前第二批两次exit0及历史选择/取消/恢复通过证据，集中人工第二批零Key、零联网限定验收通过。不是全程抓包、真实Provider或系统完整验收。 详[跨标签最终记录](validation/xuanshu-uha-cross-tab-validation.md)。新证据根build/xuanshu-uha-cross-tab-20261104-d3c057；未读旧库/凭据，产品417源码/153程序/21身份链接匹配，已停止，不自动进入新阶段。

> 2026-11-04跨标签补测准备：已核对同冻结快照的生产web_counter.uapp（导入ui_framework.dll），仅查看合成页面及切标签，不点击+1或调用其能力；排除测试DLL包。417源码/153程序/21关联清单匹配，新隔离根build/xuanshu-uha-cross-tab-20261104-d3c057。此前第二批两次exit0及已通过分项保持，本轮只补固定目标动态切换，不重新做历史/恢复；实际人工与退出尚待执行，不预填整批通过。详[跨标签补测](validation/xuanshu-uha-cross-tab-validation.md)。

> 2026-11-04第二批收口：本地合成历史/仅A选择、未确认模型提案/预览、主动取消、同库恢复及persisted诊断分项通过；两次PID61528/64548均实际exit0。关后两次均3任务/6消息/9事件/0工具，整库/配置哈希及全部记录不变，无确认/模型请求/工具重放。指定rpc_memory单实例，跨标签固定目标未覆盖，故整批为部分完成，不宣称全部通过；预览定位困难及首块取证限制保留。详[第二批最终记录](validation/xuanshu-uha-manual-second-validation.md)。已停止，不再启动或进入有Key/真实调用，原冻结和历史风险保持。

> 2026-11-04第二批已获执行授权：新根build/xuanshu-uha-manual-second-20261104-89bc95，匹配FF84…64AB0源码417/程序153未变；只补合成历史选择、本地未确认提案/完整预览、主动取消及同库记录恢复。最多初始＋证据核对后同库恢复各一次；零Key/Provider，不确认外发。rpc_memory单实例，跨标签暂列无入口未覆盖。准备完成、实际步骤及退出码待用户操作，不预判通过，见[第二批记录](validation/xuanshu-uha-manual-second-validation.md)。以下为各自历史状态。

> 2026-11-04最终：简短零Key、零联网人工复核通过。匹配FF84…64AB0正式宿主本次PID80044由用户完成展开→草稿→收起→主区→再展开→编辑→正常关闭；截图主区浅色且空状态明确、无助手残留，草稿完整保留并可编辑。实际exit0，关后隔离库任务/消息/事件/工具均0、无模型配置；没有Key/任务提交/确认/Provider路径，非抓包证明。首次PID45096退出码未知及原人工失败/首败不覆盖，另授权原库唯一重启不重置；IME等后续项目未执行，不自动进入下一批。详[短复核最终记录](validation/xuanshu-uha-visibility-manual-validation.md)。以下保留各阶段历史状态。

> 2026-11-03最新：UH-A显隐/空视图/输入/标签限定独立复审“通过、匹配快照可以冻结、必须修正项为空”已[追记落盘](validation/xuanshu-uha-visibility-independent-review.md)，冻结identity FF84C151A97784832D932AAE72F55BF9107DDE7E81DFF97F006FF41887664AB0；本轮仅身份核对和文档，不重跑测试。另获简短零Key/零联网复核授权，新根build/xuanshu-uha-visibility-manual-20261103-83e2a7；唯一监督启动及逐步用户观察待实际结果，见[本轮记录](validation/xuanshu-uha-visibility-manual-validation.md)。不提交任务、不设置Key/确认外发，不把冻结写成人工通过。原人工暂不通过、首图限制、ASAN20.02秒/30.08秒及其他旧风险均保留。下方是各自历史状态，不执行旧启动模板。

> 2026-11-03最新：人工第一批发现的黑区、助手残留、隐藏输入及输入辨识已完成限定开发、自审与离线验证，原人工第一批暂不通过结论保留。新根build/xuanshu-uha-visibility-20261103-e57b29；旧B335…8007A冻结仍仅对应旧快照，新代码等待限定独立复审，未自行冻结/启动人工。详[本轮记录](validation/xuanshu-uha-visibility-validation.md)、[增量合同](xuanshu-uha-visibility-contract.md)。底层无视图确定背景/空提示、隐藏交互收尾、实际布局预览门及常驻中文标签已实现；原人工唯一因果/时序限制仍保留。下面启动模板和阶段状态按各自历史理解。

> 2026-11-03第一批最终：唯一监督重试PID8708由用户Alt+F4关闭，实际exit0；关后只读库1任务/2消息/3事件/0工具，唯一cancelled，未确认/无模型请求或发送记录，零Key/Provider边界保持。配置、结构化read选择、本地保存及诊断已覆盖；AI收起后面板残留且用户报告控件无响应，输入框无清晰标签导致两次定位困难，第一批暂不通过，未再启动。 详[第一批最终记录](validation/xuanshu-uha-manual-first-validation.md)。原UH-A限定冻结记录保持快照边界；下一轮仅建议收起可见性/布局/重绘/命中同步及输入标签限定整改，尚未授权实施。首次启动/WAL取证限制与全部旧风险保留。

> 2026-11-03第一批启动异常：冻结追记已完成；匹配正式宿主PID43480创建后提前退出，未捕获退出码，stdout/stderr空，新库/WAL保留、模型配置未创建。未请求用户操作/Key/外发确认、未提交任务；不推定根因或正常关闭，不自动重启。人工第一批未完成，停止相关步骤，详docs/validation/xuanshu-uha-manual-first-validation.md。 原限定通过/快照冻结与这次人工启动结果分开，不宣称集中人工通过。

> 2026-11-03 UH-A剩余构建失败窗口已承接上一轮限定独立复审“闭环、限定通过、匹配快照可以冻结、必须修正项为空”并追记落盘。冻结identity B3351D1DB62743F5E5EFC67739976CB2A470B7D32C5855375B25B02A8C68007A；本轮只核对身份，不重跑测试或完整复审。另获集中人工第一批零Key/零联网授权，新根build/xuanshu-uha-manual-first-20261103-5d8a41；启动及逐步人工观察仍待实际结果，不预判通过。详[冻结追记](validation/xuanshu-uha-buildgate-independent-review.md)、[第一批实际记录](validation/xuanshu-uha-manual-first-validation.md)。本轮新文档状态不冒充原离线交付文档哈希。

> 2026-11-03构建失败回调窗口整改已完成开发、自审及离线验证；Light相称26/26＋另次复制1/1，ASAN-Light相称26/26＋另次复制1/1。新根build/xuanshu-uha-buildgate-20261103-b27e91；产品仅宿主两个回调边界检查，运行时/公开头/来源/预算不变。修前3例12项断言失败、夹具const编译错误及cleanup屏障不足分别保留；原ASAN30.08秒及其他首败不覆盖。当前等待再次限定独立复审，不自行冻结，不启动集中人工/Key/Provider。详[本轮整改记录](validation/xuanshu-uha-buildgate-validation.md)。现有真实服务、browser另案及延期人工索引保持。

> 2026-11-03最新整改状态：原UH-A限定复审两项阻断/暂不通过之后，投影失效、合法长历史和持久化诊断三项已按新合同完成开发、自审和相称离线验证。新根build/xuanshu-uha-closeout-20261103-a6f2d8；Native40/40、最终Light59/59＋复制1/1、最终ASAN59/59＋复制1/1分别运行。详[整改记录](validation/xuanshu-uha-closeout-validation.md)。当前等待再次限定独立只读复审，不自行冻结或启动人工验收；下面原开发/冻结/真实服务及Proposed历史按各自当时边界保留。

> 2026-11-03更新：UH-A已在独立新根完成正式宿主结构化只读范围、完整初始请求预览显示门、配置隔离/凭据状态、草稿及诊断；代码与离线验证完成，停在集中人工验收之前，尚未执行真实设备/Key/Provider。详[本轮记录](validation/xuanshu-uha-validation.md)、[规格](xuanshu-uha-host-contract.md)和[人工入口](xuanshu-uha-manual-acceptance.md)。RPCB-1原冻结identity/真实服务事实及下文历史Proposed状态保留；UH-A现已由2026-11-03授权实施，不再只是建议。下一步需要集中人工授权，未获新真实请求授权。

# XuanShu原生AI子系统阶段收尾索引

2026-11-02。本文件用于索引已交付范围、证据边界、延期项目和下一批建议，不重新全面评审P2—P6，也不宣称整个系统完整验收通过。本轮仅文档修改和只读身份核对，没有产品修改、构建/测试、程序启动、密钥读取或Provider调用。

## 1. 当前冻结锚点

RPCB-1最新预览展示与启动诊断两项整改已由上一轮限定独立复审判定闭环、验收通过、匹配快照可以冻结。当前以[独立复审追记](validation/xuanshu-rpcb-display-independent-review.md)记录冻结：build/xuanshu-rpcb-display-20261102-64c8a2/delivery-identity.json，SHA-256为41D15CFF8A2E0F639D02B6748C0F90AB95E9D6B4CAB7368221CF357BA4E8FF7C。冻结不等于commit/tag/release，也不自动批准下一批。

本轮新文档状态与原离线文档身份分开；原build清单、source-final、失败材料和二进制保持。最终UI17/17、ASAN UI17/17、生产启动3/3及另次核心六项证据均不重跑，不拼成首次单次全部通过。ASAN仅对应UI测试/所含启动器代码，CMake入口基线为事后补充，构建依赖、signed、捕获自审失败均保留。

## 2. 已完成且有阶段证据的范围

| 范围 | 当前阶段事实 | 原记录/限制 |
| --- | --- | --- |
| P2统一目录、read与人工入口及五维护示例 | 单一注册来源、固定目标、绑定、实际模型/revision/草稿及生命周期有开发阶段证据 | [P2模拟验收](validation/xuanshu-p2-simulated-acceptance.md)、[应用覆盖](xuanshu-p2-application-coverage.md)；真实设备完整矩阵未验 |
| P3服务、主对话和存储 | workspace级Service、stub、异步SQLite、恢复仅记录及关闭合同有分阶段实现/整改记录 | [P3合同](xuanshu-p3-service-contract.md)、[P3复审](validation/xuanshu-p3-independent-review.md)及后续P4整改；stub不代表自然语言理解 |
| P4/P5受控编辑及协作 | 应用自有事务/回执、授权、审批、预算与UNKNOWN有夹具/应用阶段证据 | [P4记录](validation/xuanshu-p4-validation.md)、[P5记录](validation/xuanshu-p5-validation.md)、[P5覆盖](xuanshu-p5-application-coverage.md)；真实模型只读，不能因接通就启用修改/图调度 |
| P6可靠性收口 | P6R-1三项整改已限定复审并记录冻结 | [P6R-1复审](validation/xuanshu-p6r1-independent-review.md)；有限样本不外推无限稳定或物理断电 |
| RP-A/B、RPC-K1 | DeepSeek生产Responses/read链、正式Light绑定和凭据/诊断已有实现；相应限定复审记录保持 | [RP-A/B复审](validation/xuanshu-rpab-independent-review.md)、[RPC-K1复审](validation/xuanshu-rpc-credential-independent-review.md) |
| RPCB验证自动化及最新门整改 | 一次固定批次确认、总4/0-3-1硬门、固定ref、自动T2/T3/取证/安全关闭；最新UI失败门限定通过并冻结 | [批次合同](xuanshu-rpc-batch-contract.md)、[最新复审](validation/xuanshu-rpcb-display-independent-review.md)；验证专用模式不推广成普通自动授权 |

上述索引承接历史各自的实际口径，不给未复审项目补造独立验收结论，不把全部历史应用副作用迁移或未来功能计为已完成。

## 3. 已有真实服务覆盖与独立账目

| 轮次 | 已有事实 | 不覆盖的边界 |
| --- | --- | --- |
| 最早credential_unavailable | 发送前失败，M/R/S/C/U=1/1/0/0/0，唯一failed保存 | 未见可输入Key窗口，不能归因用户未配置/Key无效或DeepSeek拒绝；额度不退 |
| RPC-K1文本T1 | 回复“玄枢流式验证成功”；1/1/1/1/0，usage467+6=473，参考USD0.0001473 | 随后validation_deadline使当轮T2/T3未执行，旧期限不延长 |
| 旧T2 page1 | 一次完整响应，1/1/1/1/0，tools0；usage494+90=584，参考USD0.0002562；失败保存 | 原Schema只声明integer而adapter只接受0；旧失败不覆盖、不退款 |
| PID5916准备早退 | 无会话/自动计数记录，无确认/请求；原输出为空 | 未捕获具体错误/退出码，后续结果不倒推其根因 |
| PID13052准备 | 指定ref读取error5，正常停止，五项计数0、无batch_confirm，安全销毁 | 不等于Key失效或服务端认证失败 |
| PID34224真实固定批次 | 同ref读取成功、无需新录Key；一次可信batch_confirm，T2三POST、T3一POST，T1=0 | 只覆盖其匹配程序和明确合成范围；新启动器UI不自动继承真实实跑 |

最终批次T2真实发现完整read描述/Schema、memory.read和汇总完成；实际整数10、字符串r1、framework_revision=r1和工具结果已保存。T3在真实在途状态取消，唯一cancelled；M/R/S/C/U=4/4/4/3/1，模拟S=0，T2唯一completed。自动导出后Service/workspace均destroyed，进程退出码2保留unknown诊断；关后库2任务/4消息/43事件/2工具/2唯一终态，SHA-256 C778B840A1396E1428C1D94E51474B44AC0F6FED5C920A0A9F25F10230EDE647。

T2 usage2135+190=2325，本地保守参考USD0.0008685；T3缺少有效usage，保留估算USD0.0055956、fee_unknown=true、远端处理未知。取消不证明服务端停止或免除费用，参考计价不是账户账单。用户提供的日汇总6次/3865 Tokens/低于¥0.01可作数量级佐证；已知473+584+2325=3382，相差483仍待逐笔核对，不归给T3，不补造usage或精确账单。重建请求与摘要一致只证明重建对账，不是原始网络抓包；原始HTTP头及逐片视觉观察等限制保持。

原记录：[旧首败](validation/xuanshu-rpc-real-service-validation.md)、[T1](validation/xuanshu-rpc-real-service-validation-round2.md)、[page1失败](validation/xuanshu-rpc-real-service-validation-t2t3.md)、[启动整改和最终真实批次](validation/xuanshu-rpc-startup-validation.md)。本轮不重开Service、不操作数据库或凭据、不追加真实请求来消除这些非阻断限制。

## 4. 集中人工验收索引：继续延期未验

软件整体完成并明确授权后，先核对当时匹配产物/隔离数据范围，再执行原清单；以下是索引而非删减原验收项目。原清单的详细步骤仍有效，不能直接拿历史启动命令当本轮执行授权。

| 延期项目 | 当前替代证据 | 剩余边界 | 将来操作步骤及原记录 |
| --- | --- | --- | --- |
| 真实鼠标/键盘和生产来源全链 | 绑定/收参、注入与程序消息负例、业务结果自动回读；旧真实原生Key/批次路径有局部记录 | 未系统覆盖菜单/HMENU、按钮/工具栏、快捷键、表单/对话框、画布的全部设备消息/捕获/焦点 | 真实设备逐入口一次操作，核对实际模型/目标/revision；拖出/取消/禁用/按键重复不得提交。[P2 D01/D02](xuanshu-p2-deferred-manual-acceptance.md)、[原步骤](xuanshu-p2-manual-acceptance.md) |
| Windows来源动态正例及限制 | 分类真值表、混合事件/程序调用拒绝、可信私有事件链 | HARDWARE不等于物理设备证明，不保证排除UIAccess或隔离恶意同进程DLL | 分别真实设备、可识别注入、程序消息、来源读取失败/未知，记录origin/device/采样位置及拒绝结果。[P2 D03](xuanshu-p2-deferred-manual-acceptance.md)、[RP-A/B延期](xuanshu-rpab-deferred-validation.md) |
| IME | UTF-8/容量/草稿及提交模型测试 | composition、候选窗、确认/取消与快捷键冲突未整体验证 | 中日文真实组合输入，提交前模型未改，确认后文本完整，取消不提交。[P2 D04](xuanshu-p2-deferred-manual-acceptance.md)、[P3](xuanshu-p3-deferred-manual-acceptance.md) |
| 跨屏DPI | 尺寸/布局与有界组件回归 | 实际跨屏缩放、坐标/焦点、文字清晰度未验 | 不同DPI屏幕间移动/缩放，重复各入口及画布选择，记录DPI和业务结果。[P2 D05](xuanshu-p2-deferred-manual-acceptance.md)、[P6](xuanshu-p6-deferred-manual-acceptance.md) |
| 视觉及交互体验 | UI投影、分页、文本长度、捕获和输入保留断言 | 人的可理解性、遮挡/主题/长结果展示、错误可见性未判 | 检查明暗主题、溢出/长范围、审批/问题、取消/busy/modal/REFUSE和面板重建；以业务回读补充视觉判断。[P2 D06/D08](xuanshu-p2-deferred-manual-acceptance.md)、[P4](xuanshu-p4-deferred-manual-acceptance.md)、[P5](xuanshu-p5-deferred-manual-acceptance.md) |
| 辅助技术 | 程序焦点/投影和组件状态 | 屏幕阅读器、可访问名称、焦点顺序和错误播报未完整验证 | 实际辅助工具逐项朗读范围/错误并键盘完成操作，检查长结果分页。[P4](xuanshu-p4-deferred-manual-acceptance.md)、[P5](xuanshu-p5-deferred-manual-acceptance.md)、[P6](xuanshu-p6-deferred-manual-acceptance.md) |
| 物理断电 | SQLite故障注入、隔离子进程中断/冷开、损坏副本保护 | 真实设备缓存/断电窗口未证明 | 另授权专机/可恢复镜像和设施，在无用户库环境断电后核对记录/意图/实际业务/唯一终态；不在日常机器强断电。[P6](xuanshu-p6-deferred-manual-acceptance.md)、[RP-A/B](xuanshu-rpab-deferred-validation.md) |
| 人工长时 | 有界ASAN、600秒/60轮及生命周期样本 | 无限稳定、慢设备/长时泄漏/系统资源趋势未证明 | 另定持续时长、负载与资源阈值，重复多实例装卸/取消/REFUSE并留资源与业务证据；不得以短测替代。[P2 D07](xuanshu-p2-deferred-manual-acceptance.md)、[P6](xuanshu-p6-deferred-manual-acceptance.md) |

## 5. 历史风险和另案

并行graph_boundaries超过20秒、ASAN控制任务原5秒期限失败、P2五包15.04秒首超时、面板180.44秒首超时继续独立保留；后续串行同阈值通过不等于首败已定位，不默认归因环境。frontend63、ASAN-Light中断、Light surface_input、browser20、workspace20.05秒/20秒门槛及Native输入首败不覆盖。参见[P6记录](validation/xuanshu-p6-validation.md)、[P6R-1整改](validation/xuanshu-p6r1-closeout-validation.md)和原阶段记录；本轮不重新分配根因或扩大复审。

browser九项副作用navigate/back/forward/reload/stop/home/help/settings-save/settings-reset已确认另案。保留原人工业务入口、统一只读snapshot及适用view接入，不把九项伪装read/view或本轮漏实现。OpenAI/OAuth、新Provider/网络工具、真实模型AI写入及跨实例共享写入均不借本记录获得授权。

## 6. 正式宿主日常闭环：当前状态与下一批建议（Proposed）

只读核对当前framework_host代码：ai_model_panel.inc已有模型设置、原生Key/逐任务确认绑定、提案/显式继续；host.html已有历史sequence输入、projection JSON、范围分页及任务状态；ai_panel.inc在显式UI_AI_DB_PATH时派生.model.json，否则走用户默认配置。服务/执行器不缺第二套，不能把RPCB固定凭据复用和一次批次确认直接搬到普通任务。

| 分类 | 剩余事项或已有基础 |
| --- | --- |
| 已完成且有证据 | 单Service/主对话、DeepSeek Responses只读工具、目标固定、配置/凭据引用、确认后发送、持久化、取消/关闭和正式Light接口；验证专用一次批次及展示失败门已限定冻结 |
| 仍需规格确认 | 正式宿主将历史sequence/projection JSON转为可理解的历史/能力/字段选择；默认不外发未选内容；模型配置/凭据状态、固定目标、发送未知和保存状态的日常展示及诊断取得方式 |
| 仍需规格确认 | 普通用户配置与显式隔离库路径的使用说明/是否需独立设置入口，必须明确普通origin/引用归属；不得默认复用RPCB指定ref或自动迁移已有凭据/用户数据库 |
| 延期人工验收 | 上表真实设备/IME/DPI/视觉/辅助技术/断电/长时；宿主日常体验尚不能由验证窗口证明 |
| 已确认另案 | browser九副作用及其他Provider/OAuth/真实模型写入等，保持原边界 |

**推荐UH-A：正式宿主只读任务范围选择与诊断易用性收口。仅为建议，尚未授权。**

- 目标：基于现有接口把技术性历史/字段输入整理为结构化选择、明确固定目标及完整外发预览；显示配置可用与本地可读/服务端未验证、保存确认/发送/远端未知、取消/结果状态。普通任务继续逐任务可信确认，不做通用自动批准。
- 依赖及先行规格：沿用冻结RP-A/B、C2/P4/P5和RPCB的证据边界；先盘点现有Light投影/绑定复用。确认用户级配置入口与隔离调试路径的边界、历史/字段选择的默认值和错误展示，不重新开放已确认架构或更改额度。
- 预计文件：examples/framework_host/host.html、ai_model_panel.inc、ai_panel.inc及必要main.c局部；tests/ai_model_panel.c及相应专用夹具/CMake注册；新增宿主交互合同/验收记录及当前交接。具体接口增量先设计再评审；当前不修改这些文件。
- 验收：只选内容与实际序列化请求一致，未显示/未确认零发送；状态变化/旧摘要/标签切换/重开/版本冲突、输入保留、伪造消息拒绝、一次确认/重复拒绝、取消未知/保存失败及关闭；先离线替身/真实workspace，私有入口不导出。真实服务/密钥读取和集中人工项目仍须单独授权。
- 停止条件：离线与自审完成后停代码评审；若必须改变冻结结构/错误/来源/限额/生命周期，暂停相关部分提交具体最小方案。禁止实际Provider调用、用户库迁移、通用自动授权、AI写入、browser迁移和新网络执行器。

所需下一轮授权是UH-A规格细化及明确范围的实现/离线验证/文档；本轮只提出建议，不自动实施、不要求现在提供Key或人工操作。

## 7. 阶段停止位置

RPCB-1冻结记录已落盘、阶段收尾已整理，等待下一阶段授权。没有本轮新测试或真实请求，也没有凭据、库、历史文件清理。上述已完成、Proposed、延期及另案相互区分，不以阶段整理宣布系统完整验收。


## 2026-11-04 UHS-1索引更新

显隐冻结及第二批/跨标签零Key人工通过仅对应旧快照。新[可读摘要合同](xuanshu-uha-summary-contract.md)与[开发验收](validation/xuanshu-uha-summary-validation.md)已完成宿主限定易用性增量，Light/ASAN各34项相称回归及另次复制通过，产品限宿主四文件，运行时不改。自审修正中途构建缓存提交及术语，原首败与新ASAN期限记录保留；不等于独立复审或人工通过。

停止于新根build/xuanshu-uha-summary-20261104-d7c518代码评审，后续只建议限定独立只读复审，再按[人工入口](xuanshu-uha-manual-acceptance.md)最新节授权简短定位/阅读/复制/关闭复核；本轮没有启动用户程序或任何Provider。原IME/DPI/来源/辅助技术/长时/断电及browser另案不变，不推进其他功能阶段。

## 2026-11-04 UHS-1显示缓存限定整改交付

原摘要候选限定复审暂不通过；两项显示中断缓存窗口已最小修复并自审，产品只改宿主页。最新证据`build/xuanshu-uhs-cache-20261104-4a9e72`：Light45/45、ASAN45/45，复制各另次1/1；修前5项有效产品失败、夹具/编排/ASAN栈及30.11秒首超时各自保留。详[整改验收](validation/xuanshu-uhs-cache-closeout-validation.md)和[增量合同](xuanshu-uhs-cache-closeout-contract.md)。当前具备再次限定独立复审条件，未独立通过、未冻结、未启动人工或真实服务。历史阶段结论、所有未知期限/Provider费用限制、人工延期及browser另案不变。

## 2026-11-04 UHS-1冻结与简短人工准备checkpoint

上一轮限定独立复审通过，匹配B5FD964B…ABDC23快照冻结记录已落盘，非本轮重测。详docs/validation/xuanshu-uhs-cache-independent-review.md。本轮新隔离根build/xuanshu-uhs-cache-manual-20261104-b19f62，仅一次正式宿主启动、offline-fixture本地未确认提案及摘要/详情/草稿/取消观察；零Key，不确认外发。人工结果待反馈，不能预填通过；首败及所有期限限制保留。

## 2026-11-04 UHS-1简短人工复核完成checkpoint

UHS-1简短零Key、零联网人工复核通过。中文范围定位、固定目标/白名单、详情2块折叠恢复、草稿完整可编辑且不覆盖原任务、脱敏复制及取消保存已获用户观察/截图；PID19548实际exit0。关后新库1任务/2消息/3事件/0工具，唯一cancelled/committed，无外发确认/模型请求保存，immutable哈希不变且无WAL/SHM。产品418源码/53产物匹配B5FD964B…ABDC23，冻结记录已落盘；人工根build/xuanshu-uhs-cache-manual-20261104-b19f62。非抓包、真实Provider或全部人工通过；历史限制及延期保持，停止不进入下一阶段。详docs/validation/xuanshu-uhs-cache-manual-validation.md。


## 2026-11-04 第三批人工准备

见[第三批记录](validation/xuanshu-uha-manual-third-validation.md)和[只读交付准备](xuanshu-readonly-delivery-readiness.md)。仅补IME、编辑焦点、滚动和尺寸等；单屏/无屏幕阅读器保留设备未验，长时未定。不重复已通过完整流程，当前未预判通过。


## 2026-11-04 第三批收尾

输入法候选/提交/取消、混排换行、选择复制粘贴、撤销重做及Tab往返由用户确认。用户决定停止临时页面细粒度控件验收，滚动/尺寸/长时等未执行，不算整批通过。PID33920正常exit0，关后0任务/消息/事件/工具，无配置或模型请求记录。产品未改，历史冻结不变。详[第三批最终记录](validation/xuanshu-uha-manual-third-validation.md)。后续正式页面规格和任务流程另授权；设备及历史未验项继续保留。


## 2026-11-04 用户明确取消临时页面控件测试

用户纠正：这个临时页面的控件测试是取消，不是暂缓。此前“暂缓”“待补验”及将该页面未执行滚动、尺寸、焦点等检查列为后续待办的表述撤回。此类测试不再安排、不作为交付待补项或阻断；已完成的观察和正常关闭证据只作历史保留，不补写通过。该取消不自动取消其他独立系统合同或历史风险，也不授权新的正式页面开发或验收。


## DH-1正式只读界面开发增量

用户已授权日常界面方向，实施根build/xuanshu-daily-host-20261104-c82f19。范围、状态、草稿与设置/开发诊断分离；同Service与原逐任务确认。详[开发合同](xuanshu-daily-host-contract.md)、[开发验收](validation/xuanshu-daily-host-validation.md)。本轮仅开发/离线/自审，不继承旧冻结，不宣称独立或人工验收，不开展真实服务。

旧UHS-1冻结、各人工批次、T1/T2/T3及费用unknown/483 Tokens限制独立保留；临时页面控件测试取消，不是暂缓或通过。


## 2026-11-06 BR-2恢复保护整改索引

新增READY前开启/任务/占用/传输/费用一致性审计及隔离恢复回归。新交付build/xuanshu-br2-restore-20261106-3c8d42，详[恢复整改记录](validation/xuanshu-br2-restore-validation.md)。原交付及历史首败保留，BR1-KI-001继续为未解决且用户接受的非阻断遗留。真实验证保持Proposed，等待独立复审及集中授权。


### 2026-11-06 BR-2恢复整改最终证据

最终Light94/94、ASAN-Light94/94、Native23/23分次通过；48份本轮自有关后库immutable核对不变。新候选build/xuanshu-br2-restore-20261106-3c8d42等待再次限定独立复审，尚未冻结。详[整改记录](validation/xuanshu-br2-restore-validation.md)。原独立复审暂不通过、修前窗口、首轮红库字节身份限制及历史首败保持。真实验证包仍Proposed，BR1-KI-001仍未解决且用户接受非阻断。


### 2026-11-06 BR-2旧费用恢复限定增量

新候选build/xuanshu-br2-fee-20261106-54d9a8。按原策略区分字节estimate与BR-2可靠上界，保留实际known及unknown，新增合法达到/超过阈值与矛盾后继占用验证。开发状态及最终程序证据见[本轮验收记录](validation/xuanshu-br2-fee-restore-validation.md)。前置暂不通过和历史首败保持，新候选等待再次限定独立复审，真实调用继续待独立授权。

最终Light107/107、ASAN-Light107/107、Native36/36分次通过，27组费用恢复匹配及99份关后主样本读取不变，必须修正项清零，等待再次限定独立复审。新身份不继承前置验收或冻结，真实验证仍Proposed。
