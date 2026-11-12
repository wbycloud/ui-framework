> 2026-11-05 DH-1冻结与只读交付收尾：承接此前限定独立复审通过、必须修正项为空的结论，419项源码/74项产物及清单身份核对匹配，快照65823A92BAAF49CE2623F5865F2C47B678215DD813D7E8EDDC8725980107A532的冻结记录已落盘。详[复审记录](xuanshu-daily-host-independent-review.md)、[交付清单](../xuanshu-readonly-delivery-manifest.md)、[使用说明](../xuanshu-readonly-user-guide.md)。仅文档收尾与身份核对，无新测试、启动、凭据或请求；未重新读取数据库。当前只读交付材料齐备，不等于正式发布或整个系统完整验收。后续有Key验证、产品改进仅Proposed，需独立授权。
> 下文保留各阶段原始记录；其中“等待复审”“未冻结”“方案待确认”及旧快照入口属于历史状态，当前交付入口以上述清单为准。临时页面控件测试已取消，不恢复、不列为待办或阻断；历史失败和独立未验边界保留。

# DH-1正式只读助手界面开发验收记录

## 范围、基线与结论

2026-11-04。用户已授权“会话与当前任务＋新任务草稿；任务范围独立查看；设置和开发诊断按需展开”。正式只读助手界面开发及自动验证完成，已具备限定独立复审条件。本记录是开发自审与离线结果，不是独立复审、正式冻结、真实人工或真实服务通过。没有启动集中人工验收。

Git HEAD为e0eca03db25930be55bdb45477027d1793a61904。前置build/xuanshu-uhs-cache-20261104-4a9e72的delivery-identity.json SHA256为B5FD964B3348C7CE0EC32A265EC667EDDC70EFEF32C574E34670004568ABDC23，入口418源码/53产物匹配；只核对身份，没有重做历史全面复审。当前工作区有大量既有修改与未跟踪文件，本轮只取新根source-before对照增量，不用HEAD全部差异冒充本轮。

新交付根：`build/xuanshu-daily-host-20261104-c82f19`。入口2354份范围源码/文档在本轮编辑前保全，并非所有Git文件内容快照。新合同先于实现；baseline.json是在封装时补充的入口说明，不冒充同期自动报告。参考工程D:/XuanShu可读、保持只读。原人工库/历史证据未操作。

## 实现与使用流程

产品仅改examples/framework_host/host.html；没有修改运行时、宿主C投影include、公共头或Provider。新增tests/uha_daily.c及CMake注册；五个已有UH-A测试只调整实际可见容器导航，原业务/缓存/授权/关闭断言保持。

| 区域/动作 | 最终行为 |
|---|---|
| 日常首页 | 当前任务与状态、任务内容、结果、新任务草稿；会话/任务列表可折叠。问题回复仅在有question_id时显示 |
| 新任务草稿 | 自然语言输入；“选择新任务范围”独立展开历史、资源、read能力/字段；默认不选历史 |
| 保存任务并查看范围 | 原ai-model-propose，本地提案不等于发送；匹配accepted/existing才确认接纳，当前run/committed匹配才自动展开范围；失败保留输入/选择，不重试 |
| 查看本任务范围 | 当前任务固定输入/历史/目标/配置/白名单/完整请求，独立于新草稿；查看旧任务不覆盖草稿 |
| 完整请求详情 | 原分块全文与实际读回；范围或详情隐藏即失效，恢复由宿主验证run/hash/代次/实际文本/布局/末次回调条件 |
| 保存与处理详情 | 完整接纳、确认、请求保存、工具/模型轮次、终态保存、错误与远端状态；首页状态摘要不虚构HTTP次数 |
| 请求取消当前任务 | 先显示请求已发起，终态/保存以实际投影为准；取消不代表远端停止 |
| 设置 | 当前模型配置、路径和分阶段凭据状态；任务绑定模型配置另列，不自动读真实Key |
| 开发诊断 | 默认折叠，保留stub、编辑、图和命令诊断全部原动作/权限；真实模型不开放写入 |
| 复制脱敏诊断 | 日常可见，复用原只读复制；不授权、不提交、不读密码框、不联网 |

单个pending防重复保存，若保存期间查看另一任务，匹配回执仍解除等待，不把另一任务当成本次结果。用户已改动的草稿不清除。稳定任务列表按签名复用节点，替换前撤销签名；原详情/摘要缓存失效—构建—成功提交顺序未变。默认技术事实折叠，以实际状态显示简洁中文；超过180字符的当前内容明确标注摘要，全文仍可查看。

## 直接修前证据与自审

- red-default：旧UI在新日常要求下1例4断言失败：主保存入口默认隐藏，stub/编辑/图默认占用主流。同期源码、测试程序、JUnit及日志保留；这是需求缺口，不外推越界发送或崩溃。
- 首次目标构建因未指定原OSMesa而未注册对应测试目标；补充同一固定库后构建成功，没有替换依赖。
- daily-initial 2/2、related-initial 43/43属于早期布局；后续自审把技术保存事实折叠、避免稳定任务按钮重建、分开取消请求与终态、修正保存期间选择另一任务的回执处理。pass-archive的ASAN仅构建上下文，不称该阶段已测试通过。
- daily-selfreview：4例中store仅close_host失败。源码核实存储错误第一次关闭显示错误并撤销退出，第二次才关闭。夹具补有界等待真实error_ack/错误提示后再正常关闭，未清除错误、取消refs断言、放宽超时或修改运行时；最终store通过。
- closure第一次用了CTest不支持的正则，0例，不计通过；改显式枚举后执行实际48例。日志保留。
- Light复制首次受限上下文error5，同程序正常用户上下文通过；不把对照结果归因为所有历史复制错误的根因。
- Ninja取证首次PATH/target名称错误见driver-errors.md；未自动落盘的stderr只有事后说明，不伪装原日志。最终实际RC对象依赖证明生产host包含新HTML。

## 最终验证（各次分别计数）

| 构建/运行 | 结果 | 原始证据（新根下） |
|---|---|---|
| Light closure-complete | 48/48，116.50秒 | closure-complete-light-test.log；light/closure-complete.xml |
| ASAN-Light closure-complete | 48/48，248.05秒 | closure-complete-asan-light-test.log；asan-light/closure-complete.xml |
| Light控制补项 | 6/6，8.02秒 | control-supplement-light-test.log；light/control-supplement.xml |
| ASAN控制补项 | 6/6，8.34秒 | control-supplement-asan-light-test.log；asan-light/control-supplement.xml |
| Light诊断复制正常用户对照 | 另次1/1 | final-copy-user-light-test.log；light/final-copy-user.xml |
| ASAN诊断复制正常用户对照 | 另次1/1 | final-copy-user-asan-light-test.log；asan-light/final-copy-user.xml |

不合并成首次单次全部通过；主回归包括4项新增日常、17项摘要/缓存、3项构建失败门、6项显隐、8项closeout、5项宿主、2项公共C/C++头及3项静态/存储/取消。补项覆盖实际read值与revision、排队、重复、重启只恢复记录、末次revision回调和关闭。普通宿主没有RPC配额策略，不虚构M/R/S/C/U。

日常直接用例通过正式页面控件与生产宿主代码（隔离Service/传输/凭据）完成选择、本地保存、权威范围、PENDING解绑拒绝、恢复一次确认/重复拒绝、序列化request逐值比较、草稿保持、开发入口、取消与storefailed。既有用例保留两条50000字节历史、ASCII112页/UTF-8 48页全文、缓存11项中断恢复、隐藏/重建/失败/旧代次、双实例固定目标、A→B→A查询、64槽回收、复制和回调退栈关闭。各断言/超时未减弱。

自动渲染截图task-state/scope-summary经查看，确认任务与草稿、当前固定范围与选择草稿的中文分区；仅证明隔离渲染路径，不是用户实际视觉或真实设备来源正例。截图在visual-review，原BMP保留于相应用例自有目录。

## 兼容和证据复用

103项运行时/公共头/固定依赖文件与前置源码逐项相同；生产DLL357导出无新增/移除，无fixture/probe/test入口。生产host与rpc_memory导入生产ui_framework.dll。新DLL重新链接形成新身份，不用旧二进制身份代替；compile-dependencies与resource-object-dependency记录实际编译包含关系。公共C/C++头及静态嵌入已在本轮执行；未机械重跑Native全部Service/P2/P4/P5，未改路径只承接匹配的历史范围证据，不称本轮重测。

SDK0.10-dev/API10/标准10、ABI1/包1、旧公开布局、错误值、DDL、来源/C2/P4/P5、50ms/8MiB JS、128KiB模型、2MiB arena、8MiB Service、256KiB单包、原数据范围与测试超时保持。私有驱动仅隔离测试，不进入生产导出。

## 交付、保全与停止位置

source-before、source-after/source-final、delivery-files、incremental-source-diff、binary-files、red-binary-files、pass-archive-files、verification-summary、exports、runtime-preservation、fixed-dependencies、preservation、cleanup、delivery-identity和final-readback为核对入口。最终源码/程序与最终每次测试前清单匹配；final-readback仅封装核对，不是新测试或独立验收。

仅清理本轮最终构建的可重建obj中间文件，先保留实际依赖，保留源码、匹配程序、库、JUnit/日志、自有测试数据与全部首败/上下文档案。历史材料不删不覆盖。具体删除清单、数量和路径约束见cleanup.json。

原ASAN30.08/30.06/30.11秒等期限首败、候选绑定失败、复制error5、取证限制及启动/frontend/输入/browser首败独立保留；本轮通过不解释其未知根因。既有Provider费用unknown、483 Tokens差额和请求重建非抓包限制保持。无真实Key/账户/Provider请求，无人工程序启动，无用户数据库操作。

已取消的临时页面控件测试不再安排、不是待办或阻断，也不补写通过。新正式界面阅读体验、真实来源、IME/DPI/辅助技术及其他独立历史延期项目未验；物理断电仅专用环境方案，browser九项副作用另案。本轮必须修正项已清零，下一步是限定独立复审；之后如需真实人工另行授权。没有新的方向或普通实现选择需要用户决定。
