# PB-3公开网页阅读限定独立复审与快照冻结记录

2026-11-11。PB-3限定验收通过，匹配快照可以冻结。必须修正项为空，冻结记录和本地交付整理具备条件。本次完成独立源码及既有证据核对，既有开发自审、离线运行及本次复审分别记录。当前真实执行仍等待新的具体包集中批准。

## 范围和匹配身份

匹配根为build/xuanshu-page-reading-20261111-a18d74，delivery-identity.json SHA256为4A892228066D04EB09972594509892AF3B960CF32569DE33A125F11BEF611A42。入口实际Git HEAD为e0eca03db25930be55bdb45477027d1793a61904。546份最终源码和195项程序、链接及调试产物逐文件匹配，17项代码/样本增量通过入口快照区分。产品增量只有四份私有场景、恢复和任务路由实现。原始清单、identity、source-after及delivery-final副本保持原状，本次文档状态另外记录。

| 生产文件，相对匹配根 | SHA256 |
| --- | --- |
| light/framework_host.exe | 2584B5D690DB3F6C248862AECDAB6E4EB81E2F8D17D5E3A3E3AAC0BBE128C2BE |
| light/ui_framework.dll | 1CBBD2F7E3635F96970B45B81CD774963A5C85D2E2ED2ABF66CEE6F9215C6FC0 |
| light/reading-production.uapp | 273EE3B364CCB02307AB2D633058CAF1D84058BEBA6EFCDEA80B24B8CDC250A0 |
| light/xuanshu_br2_meter.exe | 7E2C2FF2508A83F05E1DB5B891635DABFA843812B4078BC0D7D4FEC3CB6C0EBB |

参考工程D:/XuanShu目录可读取，本次产品判断来自本仓库。没有修改参考工程、产品、测试、CMake、依赖或SDK，没有构建、运行测试、启动生产程序或打开历史数据库及WAL/SHM。新准备目录中只执行获准的无凭据纯本地计量组件。

## 发现，按严重度

| 级别与性质 | 准确位置 | 触发条件与影响 | 最小处理及阻断判断 |
| --- | --- | --- | --- |
| P2，待核实期限和显示恢复风险 | tests/uha_summary.c第72、116、141、153行；匹配根asan-light/closure-final.xml第193、217、252、289、411行 | ASAN首轮五项摘要失败，包含50ms脚本中断后的绑定/实际文本前提失败、故障尚未触发，以及30.0669/30.0793秒超时。触发因素未定位，首轮两个超时没有安全关闭成功证据 | 保留原程序、源码、原始日志、库及旁文件，并保留风险。最终相同数据和原30秒门槛的17项补项通过，新阅读42项首轮全部通过。失败路径撤销展示资格，补项逐值检查实际文本及旧确认拒绝。当前没有确认或费用保护失效证据，本轮非阻断 |
| P3，首轮调试材料限制 | 匹配根red-binary-files.json第77行 | 首轮摘要测试EXE的PDB未单独归档，回溯该EXE的完整符号诊断受到限制 | 原EXE、运行时DLL、UApp、同期源码和日志均有匹配身份，保留此限制。修前及最终判断由这些实际证据支持，本轮非阻断 |
| P3，关后材料范围限制 | 匹配根retained-sidecars.json第54、55行及closed-reading-light.json、closed-reading-asan-light.json | 96份明确观测主样本均无旁文件，另10份旁文件中6份非空。其他库的合并完整性没有统一结论 | 主样本结论保持原明确范围，其他旁文件保全。本次没有打开数据库、checkpoint或改写旁文件，本轮非阻断 |

没有新增必须修正项。BR1-KI-001仍未解决并按用户已接受状态保持非阻断。计量组件任意卡死时有界结束尚未证明，沿原合同保留PENDING、进程及引用至真实退出。原初始化20.03秒、30.08/30.06/30.11秒、复制error5和其他历史首败继续独立保留。

## 源码闭环依据

src/ai_validation.inc第18至28行集中映射snapshot-v1、dom-body-v1和public-sqlite-zeroconf-v1，私有原整数字段和布局保持。src/ai_business_validation.inc第5至6、26至42、45至60行将新场景关联唯一固定输入、空历史、browser.view、browser.body及body/revision，未知场景和不匹配范围锁停。生产启用完整计量，隔离legacy旁路不能进入生产正文场景。

src/ai_model.inc第124行写入对应validation_scene，src/ai_validation.inc第53行写入开启business_scene。src/ai_business_restore.inc第32至55、59至87行交叉核对场景、固定输入、模型、协议、history、目标和白名单。原全账本审计在src/ai_service.c第295至301行、READY之前执行。完整业务库恢复原计数、known/estimated/unknown后在src/ai_business_restore.inc第213至231行锁停，旧策略仍走原显式恢复合同。恢复记录和页面消息均不能建立新的人工资格。

正式宿主页及其确认实现保持原身份。examples/framework_host/ai_model_panel.inc第83至100行继续核对run/hash、展示代次、真实控件全文、布局和宿主状态。examples/framework_host/ai_projection.inc第20至33行在投影开始及失败时撤销资格，解绑PENDING期间仍独立拒绝。摘要和display_valid单一字段均不能建立授权。新运行时场景路由沿用av_reserve/av_before_send，包含初始和全部续轮的3次额度、逐轮完整计量、10分钟、USD0.10和失败不返额度保持。

tests/uha_summary.c第72至80、114至132行最多三次同步回放只完成显示，不泵Service、不重新提案。每次未完成均检查展示无效、旧确认拒绝及零请求。实际控件全文、页码、详情块、任务身份、草稿、一次正常确认和重复拒绝断言保持。回放旧A或取消快照后先恢复新的权威B投影，旧任务资格不能借显示回放恢复。原两条各50000字节和完整112/48页检查仍在第38、50、153至154行及匹配日志中。

tests/page_body_policy.c第69至79、119至132行覆盖完整/破损恢复、零新增执行、两标签及关闭重开。同一进程host_epoch保持，重开生成新实例与open_epoch，旧确认被拒绝。首轮错误期待进程UUID改变属于夹具问题，修正保留原目标、零请求及安全关闭断言。引用、回调重入、cleanup-only、真实DLL、静态嵌入及公共C/C++由本轮实际运行支持，原线程和关闭合同保持。

## 业务真实性与证据范围

使用2026-11-11取得的SQLite官方Zero-Configuration完整原始HTML隔离副本，原始HTML4209字节，SHA256为D8ECCE18365870C07E60802A76585B9FDE5CCD53714F579A3C8C5BE4E80387EA。实际browser/Lexbor解析出的作者DOM全文847 UTF-8字节，SHA256为A73C755A8328BD65D7085506C145EACD12760DAA3634A5DEAF42E750D24B773D。官方copyright声明文档属于公共领域。运行时为browser:start，原站实时导航、样式和图片具有各自未验边界。[原文](https://www.sqlite.org/zeroconf.html)和[许可依据](https://www.sqlite.org/copyright.html)。

独立文字常量与实际全文、bytes、complete、dom-body-v1和框架revision逐值比较。正文包含作者导航文本，脚本和表单值按原规则排除，CSS隐藏作者文字可能保留。revision继续表示不透明框架状态版本，原文内容另用哈希关联。事实为无需安装/setup、独立server进程、管理员创建实例或配置文件，原文没有具体待办。

正式宿主页完整流程使用隔离DLL、模型替身和私有可信事件，生产DLL及生产链接包另经公开read验证。生产公开read先以snapshot等待文档就绪，再实际读取body，模型任务白名单始终仅body/revision。两条证据分别支持业务读取和隔离端到端流程。摘要由替身提供，真实模型质量和真实设备来源留待其对应证据。

## 已有运行与最终匹配

| 独立运行 | 实际结果及最终覆盖角色 |
| --- | --- |
| Light red-controlled | 2例，1个新场景前提失败，生产公开read对照通过 |
| Light fixed-direct | 早期37/37，独立保留 |
| Light closure-final | 167/168，最终取其中72项 |
| Light direct-final | 另次79/79，替代共享正文/阅读测试项 |
| Light summary-final | 另次17/17，替代摘要测试项 |
| ASAN closure-final | 163/168，全部42项新阅读通过，最终取其中151项 |
| ASAN summary-final | 另次17/17，原30秒门槛，完整112/48页保持 |
| Native closure | 7/7 |
| 清理后生产公开read | Light及ASAN各另次1/1 |

最终逐项Light168、ASAN168和Native7，343条case的JUnit通过、实际EXE/UApp/保留DLL、产品源码及变化的共享测试源均已核对。各运行保持独立，首轮失败没有被后续结果覆盖。ASAN最终long22.8555秒、UTF-8 23.0772秒、cache-page18.8988秒支持对应最终功能覆盖，首次期限触发因素仍待定位。

96份明确关后主样本有无旁文件、immutable/query_only及前后同哈希记录，其中48条成功正文工具结果和16组破损恢复零新增对照逐值一致。代表性discovery库1任务、2消息、31事件、2工具，唯一completed，SHA256为D22B4E629DF5436AFB9EF0EB661C0829CD72C93D3C675CFAC58D90CF00219B5F。种子用过隔离确认及模型替身，零新增结论适用于恢复尝试。原数据库和全部旁文件本次保持未打开。

生产358项导出与前置名称/顺序一致，私有入口仅在隔离构建。9/9/1组实际编译依赖VALID记录对应中间obj清理前时点。版本、公开头/布局、错误值、DDL、来源政策、C2/P4/P5、固定依赖、50ms脚本、8MiB JS堆、128KiB模型请求、2MiB宿主arena、8MiB Service及256KiB单包保持。

本次复审核对脚本首次把清理前包装暂存DLL与清理后清单比较，修正为核对保留DLL及实际UApp身份。该取证脚本范围错误独立保存在新准备根review-control-first-error.json，产品身份没有变化。

## 冻结及下一阶段

PB-3限定复审通过，匹配快照冻结记录与本地交付材料已齐备。冻结仅记录上述匹配快照。真实来源完整分类、IME/DPI、辅助技术、人工长时、断电及browser九项副作用保持原未验范围。已取消临时页面控件测试维持取消。

复审核对和新准备证据保存于build/xuanshu-page-reading-real-preparation-20261111-6c4f82，review-evidence.json、review-integrity.json及baseline.json对应本次核对。新具体真实包保持Proposed，执行前仍须当日官方依据及费用条件一致、独立集中批准、本人原生Key和逐任务可信确认。原PB-2及BR-2批准、库和额度保留各自已消费状态。本轮没有启动真实执行。
