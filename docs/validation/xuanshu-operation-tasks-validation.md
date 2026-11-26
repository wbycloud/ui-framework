# 共用应用代操作本地验收

2026-11-25。前置交付 `7a4b2e` 的身份以实际文件核对。新候选为 `3be274`，承接工作区既有修改，记录本轮相关源码和产物。 [最终身份](../../build/xuanshu-app-operations-20261125-3be274/delivery-identity.json) · [最终源码与程序核对](../../build/xuanshu-app-operations-20261125-3be274/final-runtime-lock.json)

## 实际业务结果

| 场景 | 最初实际值 | 已提交并读回 |
| --- | --- | --- |
| 第 1 行名称列改为「已复核」 | `editable`，`doc-1` | `已复核`，`doc-2`，一笔事务 |
| 同一句复制指令，源格甲 | 源格 `源格·样本甲`，`doc-2` | 目标格精确读回 `源格·样本甲`，`doc-3` |
| 同一句复制指令，源格乙 | 源格 `源格·样本乙`，`doc-2` | 目标格精确读回 `源格·样本乙`，`doc-3` |
| EDA 补到三个，初始 1 个 | `[1]`，`doc-2` | 新增 `2`、`3`，两笔事务，读回 `[1,2,3]`，`doc-4` |
| EDA 补到三个，初始 2 个 | `[1,2]`，`doc-3` | 新增 `3`，一笔事务，读回 `[1,2,3]`，`doc-4` |
| EDA 已有三个 | `[1,2,3]`，`doc-4` | 零写入，仍为 `doc-4`。有一轮模型替身参与目标提议 |

首轮占用不含表格实际源值。续轮模型替身根据捕获的真实请求内的已核实观察生成动作参数。宿主核对来源和值，再走 P4，独立读回验证实际单元格。EDA 续轮仅披露图块数量。图块 ID、事务及撤销回执保留在本地。两次关后观察共 24 份自有主库，均无 WAL/SHM，immutable 读取前后 SHA256 一致。 [关后逐笔结果](../../build/xuanshu-app-operations-20261125-3be274/closed-operation-db.json) · [原库与归档对应](../../build/xuanshu-app-operations-20261125-3be274/closed-database-archive.json)

表格先以 7/7 完成并锁定 35 项。接入 EDA 后，锁定项中只有 EDA 注册源码和对应隔离包变化。通用宿主、两种 DLL、生成页、提示和配置保持逐字节一致。后来修复在途关闭后重新锁定最终身份，再用同一套通用代码复核两款应用。 [接入前清单](../../build/xuanshu-app-operations-20261125-3be274/table-locked-before-eda.json) · [接入后回读](../../build/xuanshu-app-operations-20261125-3be274/after-eda-identity.json)

## 首败与整改

- 修前普通聊天能生成「已复核」回复和唯一终态，零工具，实际单元格仍为 `42 / doc-1`。两项业务断言直接失败。 [修前记录](../../build/xuanshu-app-operations-20261125-3be274/red/junit.xml)
- 第一组夹具按旧请求顺序解析范围。两条复制用例首次停在源值播种前提。校准初始请求、按真实应用就绪状态及调用完成等候后，两个源值均从业务入口产生。授权按钮在完整详情下方，启用状态与实际文案单独核对。 [首轮构造和界面记录](../../build/xuanshu-app-operations-20261125-3be274/table-direct/junit.xml)
- 首次可判定的跨进程重开在 READY 前因 `logical_consistency` 被拒。新的 Coordinator 包少计结构预留。修复保留原计数和独立模型账本，新进程可查回已存记录，零新请求且无可用旧确认。 [恢复首败](../../build/xuanshu-app-operations-20261125-3be274/restart-ready/junit.xml)
- 受控首轮到期和续轮接管在修前均出现等待传输时提前完成。修复把停止交给原模型回调收尾。其后首次接管对照确认账本正确保存远端及费用未知，任务终态仍显示为失败，原 CTest 记录超时。最终回归分别验证人工停止、远端及费用未知、零写入、唯一终态和正常关闭。 [两项有效修前失败](../../build/xuanshu-app-operations-20261125-3be274/drain-red/junit.xml) · [中间结果](../../build/xuanshu-app-operations-20261125-3be274/light-v2/junit.xml)
- 两个 EDA 目标并发链接时出现 LNK1104，串行重编通过，通用代码未为接入变化。生产实启驱动首次在复制样本时失败，修正测试目录拼接后正常启动。 [链接首败](../../build/xuanshu-app-operations-20261125-3be274/eda-attach-build.log) · [启动首败](../../build/xuanshu-app-operations-20261125-3be274/production-smoke/supervisor.log)

## 最终验证和入口

| 运行 | 实测结果 |
| --- | --- |
| Release Light | 12/12，含两款 App、重复确认、非法参数、旧版本、两类在途停止、授权保存失败和跨进程恢复 |
| ASAN Light | 12/12，同一用例范围、独立构建和数据库 |
| 受影响前置能力 | 34/34，含固定 P5、模型和存储门、旧助手、正式页面、公开 C/C++ 头和静态嵌入 |

实际生产启动从新 AppContainer 的隔离 Known Folder 和独立工作目录执行 `start.cmd`。宿主 PID 19360 加载匹配生产 DLL 和两款生产应用模块，正常关闭，实际退出码 0。生产导出 373 项，原 371 项保留，新加 `ui_ai_operation_register` 和 `ui_ai_operation_list`，零私有测试入口。 [Light](../../build/xuanshu-app-operations-20261125-3be274/light-release/junit.xml) · [ASAN](../../build/xuanshu-app-operations-20261125-3be274/asan-release/junit.xml) · [兼容](../../build/xuanshu-app-operations-20261125-3be274/legacy-release/junit.xml) · [正式启动与版本](../../build/xuanshu-app-operations-20261125-3be274/production-smoke/host-final.log) · [导出比较](../../build/xuanshu-app-operations-20261125-3be274/exports-release-check.json)

业务写入使用正式页面、实际 App 能力、隔离可信事件、模型与计价替身。生产启动验证覆盖模块加载和退出。真实模型决策效果、真实网络传输和物理人工确认仍待另行验证。先前扫雷整局工单仍停止。本次保留旧交付及首败，完成本地候选及应用接入说明。 [使用说明](../../build/xuanshu-app-operations-20261125-3be274/dist/使用说明.md) · [开发者接入](../xuanshu-operation-entry.md)
