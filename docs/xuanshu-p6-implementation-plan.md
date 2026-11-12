# P6本轮实施计划

2026-10-27。P6R-1先冻结，源码状态由验收记录更新，无署名。

1. P5限定审计与身份保全：原P5公共路径/最终ownership证据，内部uapp与诊断入口盘点；browser副作用迁移与冻结external冲突集中确认，独立可靠性继续。
2. 同一Service存储可靠性：src/ai_store.inc及必要私有模块补启动逻辑一致性、worker检查点与明确失败；无新schema/公开布局/依赖。
3. 专用tests/ai_reliability.c复用原内存真实DLL夹具和独立进程；故障VFS在测试EXE，生产不导出；受控各阶段、SQL核对、重复恢复与保护。CMake只注册新增测试/构建，不动旧断言和超时。
4. 核心稳定后执行60轮装卸、256+任务、600秒持续采样，Native及ASAN相称回归；原P4/P5/Light面板/五维护包与静态嵌入复核，持续测试与短测分账。
5. 更新四份P6文档、必要整体设计/覆盖/交接、根三规划只追加；final source/delivery/preservation/binary/log/JUnit/fault/resource清单完整，旧失败/匹配程序保留，仅清理本轮过程产物。

全部阶段遵守P6R-1的目标/ID/revision、UI线程/深复制/租约、后台SQLite/无join、取消/UNKNOWN/安全卸载。未适配副作用不回退legacy AI；原人工功能不删除。无真实Provider/网络执行器/跨实例共享写入/通用事务及用户库操作。最终停在开发交付与联合评审，真实人工另授权。

## 2026-10-27 实际交付checkpoint

P6R-1实施完成：恢复前逻辑审计、明确PASSIVE错误、子编辑job唤醒、20隔离进程中断点/VFS错误/坏副本/UNKNOWN冷重开/60轮及600秒资源验证；无新公开接口/schema/固定依赖/额度。browser新增同目录snapshot与10个旧view，9项副作用按用户确认另案，五维护示例原业务保留。真实输入等延期。

最终Native/ASAN故障37/37各一组；资源2/2各一组（1755/1824任务）；Light生产14/14、投影/60轮/browser5/5。兼容性同源码逐项覆盖49/49各配置，包含失败后串行重核，不伪称一次49全通过。并行期限风险、P2首次15.04秒和历史首次失败全部保留；不作无条件P6/原P5全应用迁移完成宣称。源码/二进制、失败回收来源及清理由独立P6验收和最终清单核对。停止联合代码评审，不启动真实Provider/人工/外部迁移。

## 2026-10-28 P6R-1三项整改完成checkpoint（等待独立复审）

本轮实际e0eca03db25930be55bdb45477027d1793a61904 / codex/p1-process-boundaries未变，无提交；证据build/xuanshu-p6r1-closeout-20261028-91c4e7，入口17470，增量单独核对。先有受控红证据，再最小修复：Coordinator引用移交至done待消费并核对稳定ID；存储失败独立cleanup-only，不依赖active/c.failed；ai_begin及Provider/Coordinator/子编辑提交后门拒绝不可用存储推进。自审补齐关闭read关联排空及UI错误路径不等待通知队列。UNKNOWN、实际提交事实、原线程/回调和执行门保持，没有新执行器。

最终Native96/96、ASAN96/96；Light原批55项通过/1项未构建未运行，随后五应用目标构建并单项1/1（416/0），同源码逐项覆盖56/56，不冒称一次56全绿。9个新增受控case、原Service/P4/P5/37存储故障、两真实DLL编译分支、公共C/C++头/Assistant/静态嵌入及Light正式投影相称覆盖。首版夹具超时/屏障不足、原实现有界失败、自审read排空red、ASAN故障未接纳及Light编排未运行分别保留；不是已复现崩溃/永久死锁，也不以重跑覆盖首次。

SDK0.10-dev/API10/标准10、ABI1/包1、公开布局/v1/C2/P4/P5/固定依赖/JCS及额度不变。原并行boundaries20秒、ASAN控制5秒、P2五包15.04秒、面板180.44秒仍期限风险，未据本轮串行通过定位或解决；旧frontend63/ASAN-Light/输入/browser20/workspace20.05秒等仍独立。browser9副作用另案，真实设备/Windows正例/IME/DPI/视觉/辅助技术/物理断电/人工长时延期。未重复600秒/60轮旧样本，不外推无限稳定。

具备再次开展P6R-1限定复审的开发交付条件；本轮仅自审，不自行宣布独立验收通过或代码正式冻结。详docs/validation/xuanshu-p6r1-closeout-validation.md及新根delivery/preservation/source/binary/exports/日志/JUnit/cleanup。没有参考工程/用户库/外部应用/冻结SDK/固定依赖写入、子代理、computer use、真实Provider或Git/远程操作。原主线及历史仅追加保留。


保全清单补充：入口baseline.json自动捕获Git可见17470项，遗漏被Git忽略的三根规划。原baseline不改；baseline-supplement.json通过剥离本轮两段仅追加checkpoint恢复原始前缀，并与原P6归档逐字节匹配，另核对3项。合计17473项保全核对，不把这3项称为入口自动捕获。首次清单KeyError及补充来源见metadata-reconciliation.txt。

本轮最终保全/身份/清理：15项交付/374源码/102程序及PDB，所有最终/失败身份一致，旧P6 373/120归档不变。仅清理本轮522个对象/链接缓存，225298091字节；失败及隔离DB/匹配程序保留。详细SHA及补充baseline说明见独立P6R-1整改验收，不宣称独立复审通过或正式冻结。
