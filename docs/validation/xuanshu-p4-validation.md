# P4剩余交付：开发阶段模拟及自动验收

2026-10-26。本轮仅剩余P4及P3/P4-A范围内复审整改；结论限定为**P4开发阶段模拟及自动验收通过**，停止联合代码评审。不表示真实设备、视觉/长时或无条件完整验收，不进入P5。文档不署名。

## 1. 基线与归属

实际HEAD `e0eca03db25930be55bdb45477027d1793a61904`，分支`codex/p1-process-boundaries`，未提交。独立证据根`build/xuanshu-p4-final-20261026-c84f12`，17442项入口baseline含三根规划。原P4-A delivery30/source318/binary131入口均匹配；不把HEAD到当前全部差异归本轮。此前P2/P3/P4-A与用户.gitattributes及其他修改保留。

本轮交付文件/前后SHA由delivery-files.json枚举，完整编译输入由source-after.json及final-source快照保存，binary-files.json区分Native/Light/ASAN/ASAN-Light及红运行；delivery-identity.json给出正式宿主/生产运行库/五包的准确身份。清单和源文件比断言数量优先。参考工程此前不可读取，本轮未扩大权限、反复申请或修改，不能补称完成新一轮XuanShu源码审计。

## 2. 独立复审与合同

[P4独立复审](xuanshu-p4-independent-review.md)按P3外层方法/应用回调→编辑授权→持久化→应用阶段→回读→确认→释放核对。新增continue目标回调重入接受edit后，外层仍接受read的窗口，red129/2证明两个断言失败，未声称崩溃。最后target callback后再次检查原编辑互斥；review-fixed核心30/30后才接P4-B/C。

先冻结[P4BC-1](../xuanshu-p4-controlled-edit-contract.md)再实现，独立公共入口仅：

- ui_capability_register_edit_extension：同目录的风险/side_effect增量；可撤销destructive必须新逐项批准，policy-only的AI仍拒绝执行。
- ui_ai_edit_bind_target：UI线程目标解析，方法租约覆盖get_revision，重入关闭后CANCELLED，不输出新目标。
- ui_ai_edit_bind_light：复制控件ID/hash，借用同workspace可信chrome，沿原human guard一次消费；与原生按钮共用16条/8MiB及原解绑合同。

SDK0.10-dev/API10/标准10、ABI1/包1、旧公开布局/v1只读/冻结C2/P4A-1及错误值不变；固定yyjson/SQLite/JCS/SHA-256、2MiB目录/1024调用/8MiB调用与Service原额度均未放宽。生产Native/ASAN314、Light328导出；相对P4-A仅增加上述3个，无fixture/来源/直接批准导出。隔离运行库另有5个私有fixture导出，不能用于生产或冒充物理来源。

## 3. 真实应用与正式面板

[完整55项矩阵及P4逐能力结果](../xuanshu-p2-application-coverage.md)第7节为权威映射：13项内存write及3项既有文件副作用全部有明确接入/拒绝边界。stateful三项write、EDA三项、generic/features各三项、counter一项，由同一注册/唯一run复用实际模型。没有stateful树rename注册，因此不新增业务；实际树名由两个sample分支覆盖。

应用自有before/staged/undo快照，EXECUTE计算后恢复committed，COMMIT才发布及递增revision；READBACK核对真实值、事务/undo回执，最终存储确认后才解门。取消在COMMIT前实际rollback并回读不变；commit后保留实际修改。EDA ID高水位不回退，undo不复原旧revision/视图。草稿不入业务快照，保留独立FORM/DIALOG和原生笔记草稿并标冲突。原UST1/UIL1格式不变，不是业务文档或通用undo系统。

stateful文件三项AI审批/消费后明确拒绝，不删人工原后台入口、不新增文件/网络执行器。新POLICY_ONLY若无事务适配，阶段明确UNSUPPORTED；应用可只读证明worker退出及未执行，UNKNOWN记录确认后才解除，不生成虚假txn/undo。真实偏好只读核对测试也覆盖最终存储失败。

正式framework_host仍一个Service/主对话；增加结构化提案、范围/风险/期限、逐项批准/拒绝、实际结果/revision/回执、新undo、UNKNOWN证明、迁移及历史。确认动作没有页面消息授权路径，程序点击和自称source都拒绝。范围/结果/历史/目录按原4095字节控件容量明确展示分页，完整权威记录不截断。解绑PENDING不缓存就绪，退栈后重试，额度不足可观察。

## 4. 本轮验证（分阶段并集，不相加冒称无重叠）

所有新配置在本轮独立目录。日志LABEL-CONFIG-STAGE.log及同名sources.json，JUnit位于配置目录。1ms MsgWait仅让出执行机会，不以固定等待判完成；20/30秒及原15秒门槛不变。原公共头断言/数据范围未弱化。

| 新证据 | 实际结果/覆盖 |
|---|---|
| native/acceptance.xml | 50/50；P3核心/静态嵌入/工具、22个P4核心case、C1/A+B/JSON/JCS/v2/FORM、Assistant、公共C/C++、组件及真实DLL两编译分支 |
| light/acceptance.xml | 72/72；包含上述Light适用项、组件/来源、五P2逻辑分支、真实生产包及6种真实应用配置/正式面板；后续受改动项按下列记录更新 |
| asan/acceptance.xml | 50/50；相同核心链路无AddressSanitizer内存错误报告 |
| native/handoff.xml、asan/handoff.xml | 各8/8：最终阶段continue/tools、basic/UNKNOWN/真实DLL生命周期、背压、重入互斥和目标关闭；其余未受影响项复用上列，不重复全跑 |
| light/isolated-final.xml | 7/7：五真实应用＋features_no_gl＋正式面板，GUID临时库；后续sample投影/宿主重试使用下列受影响回归 |
| light/review-final.xml | 6/6：sample两个编译分支及no_gl、原P2 generic/features、真实生产包ui_p2_apps；实际树label及undo回读另断言 |
| light/host-retry.xml | 2/2：原P3正式聊天及P4正式面板；在途解绑PENDING→退栈后换绑定、共享16槽、伪造批准拒绝、固定目标、UNKNOWN核对、历史分页 |
| asan-light/isolated-final.xml | 7/7真实应用/面板；有下节脚本中断诊断，不能写成无UI诊断 |
| asan-light/projection-review.xml、host-final.xml | 3/3受影响应用配置＋1/1面板最终回归，实际树投影、回执/回滚与租约重试；无ASAN内存错误报告 |
| asan-light/manual-comparison.xml | 2/2原非AI人工generic204/features389对照；同样出现脚本中断，限定其与共享投影路径的关系，不是完整旧框架二进制对照 |

验收以具体断言为准：人工cell55/AI66/undo55、共享name独立草稿/冲突/Keep、实际bool/tree name及投影、counter1→2→1、EDA ID1→2→undo1/取消占用3后新4、实际笔记与保留EDIT草稿；COMMIT前回读仍before、取消后effect NONE和rolled_back；最终SQLite没有非终态run且每run终态事件唯一。断言执行次数随受控poll变化，不当功能数。

原P4-A 44/49/44是入口已有证据，不冒称本轮运行；原P3/更早P2证据只在未变公共合同处复用，受影响路径有本轮测试。专用真实应用包直接编译维护源，两sample分支分开验证，生产包未被测试DLL覆盖。

## 5. 首次失败、修复和证据限制

| 首次/阶段证据 | 核实与处理 |
|---|---|
| review-red Native129/2 | 真实revision重入接受edit窗口，最小互斥重检；源/二进制/原日志保留，无崩溃宣称 |
| app-core/首链接与ui-core | Native无Light专用target、私有声明/依赖耦合、隔离克隆缺OpenGL/comctl32链接；编译阶段失败不称执行红，最小声明和测试链接修正 |
| apps-system-libs、apps-owner | 夹具对非活动实例表单提交、立即销毁尚有关闭事件、用offscreen跑本来需windowed WGL应用；按原合同修正夹具/隐藏真实父窗，不放宽产品 |
| panel-first 30.05秒及后续Schema定位 | 异步迁移检查过早、新默认约束缺additionalProperties:false导致提案拒绝；修正状态谓词及默认Schema，首失败/匹配运行产物保留 |
| panel-origin-negative RangeError | 编辑历史/范围超过Light单控件容量；明确展示分页，不提高限额。初次pager使用不支持的insertBefore，绿色断言中仍有TypeError，不能验收；静态分页控件修复并加投影断言。该中间绿色诊断的日志/输入hash存在，未单独保存其旧EXE，不能冒称该中间版本二进制已保全 |
| boundary-tests/policy-proof及app-rollback EDA | 新夹具寻找原生note/模拟EN_CHANGE、末尾zoom字段断言和测试插入位置的before对象预期错误；修正新测试，模型/ID/回执断言不放宽。staging保留原clear视图复位，避免真实功能丢失 |
| app-final Light counter20.01秒/原20秒门槛 | ai-636.db旧schema2/历史client冲突，源码PID命名在重复运行可复用。改新夹具为GUID隔离；不删除首次库或把它归产品执行/环境根因。隔离后7/7，原失败继续保留 |
| final Native/Light全构建 | workspace不必要包含yyjson私有头，独立旧fixture缺include路径；将仅需的私有声明放现有ui_internal，恢复原capability_internal原字节，不改变公开接口 |
| panel-projection编译后误接测试 | 编译失败后执行的是前一轮旧EXE；该运行不能作为新源码匹配红证据，另记录实际执行源码来自panel-wait，完整日志保留 |

失败目录按实际测试只保留必要EXE/DLL/uapp及源/输入身份；构建失败没有执行新EXE，不把顺带复制的旧/无关二进制作为匹配证据。原始首失败及failures/review-red-native-test与最终配置目录分离。delivery/source/binary/preservation/audit清单记录精确身份及阶段差异；后续仅改测试/投影时不机械重跑已无影响的全部核心。

## 6. 未定位与历史分账

ASAN-Light generic/features共享UI投影有`InternalError: interrupted`，原50ms脚本保护未放宽；减少本轮事务对无关表/树的重刷后仍出现，非AI人工对照亦出现。当前只能关联共享投影，不能宣称根因已定位、环境问题或所有旧版本均如此。实际业务值/树label/undo与ASAN内存检查通过，生产Light对应路径无此诊断；列非阻断仪器构建展示风险，保留诊断源/匹配二进制。后续视觉/性能核对不能省略。

frontend63及其入口宿主/页面/测试＋当前DLL对照不是完整旧框架比较；旧Assistant写入假设等与迁移合同冲突，但不能认定63条全部是旧测试问题。本轮不全跑、不删除旧断言；独立正式面板P3/P4路径通过，不等于该完整前端组通过。优先级P2：后续按条迁移诊断/前端测试合同，若影响必经控制链再提级。

Light surface_input首次、browser20、workspace首次20.05秒（20秒门槛）、Native输入首次仍独立未定位；本轮未将其重跑/销账或归因环境。与P4共用UI/消息基础，存在待核实关联，但无证据证明导致本轮授权/事务/持久化错误。P4实际修改与来源门有独立受控证据，故这些历史组不自动阻断开发阶段收口；生产来源正例仍延期。

## 7. 保全、清理与停止

preservation.json对比17442项入口字节，列明授权新增/改动、缺失及越界检查；原P4-A 131二进制再次核对。固定依赖、冻结SDK和历史材料未写入；忽略的历史/build/SDK不宣称全部穷尽重hash。旧.git/分支/版本未更改，没有参考工程写入、用户库迁移、真实模型/网络或P5。

cleanup.json只操作本轮已核对绝对根下的无用对象/重复打包暂存及未执行旧二进制副本，保留源快照、首失败日志/JUnit、匹配运行产物及必要DB/WAL/SHM/备份。未来集中人工启动、具体步骤、延期原因/替代证据/剩余风险见[P4延期清单](../xuanshu-p4-deferred-manual-acceptance.md)。真实设备、Windows动态正例、IME/DPI、视觉、辅助技术和长时未执行。

停止P4联合代码评审，后续只在新明确授权内推进；不自动启动物理验收或P5。

清理补充：主批次1054项有逐路径记录；后续核对3份app_poll暂存映射并清理链接.exp。补充命令在不存在的red/native处停止，.exp逐文件删除清单未持久化，故不宣称精确补充项数/字节；实际红运行位于failures/review-red-native-test，未丢失。最终二进制和原P4-A清单再核对，不因该清理日志限制改变源码/测试结论。
