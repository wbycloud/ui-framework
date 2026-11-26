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

## 2026-11-26 首轮方案拒绝与重新生成

前置 Git 为 `ce632004`，验证采用现有工作区源码。缺动作闭合括号和裸 `goal.equals` 修前分别得到 `operation_invalid_response` 和 `operation_proposal_rejected`，各有一次隔离模型响应、零工具及零业务修改。后补的 16 图元合成样本保留正确的完成条件和缺括号边界，使用修前运行库与其原页面资源单独复现。两次修前运行均缺有效示例、可读失败原因和重新生成入口。 [修前两例](../../build/xuanshu-plan-response-20261126/red/junit.xml) · [后补嵌套复现](../../build/xuanshu-plan-response-20261126/red-nested/junit.xml)

首轮及续轮预览的 JSON 示例能由严格解析器读回。畸形方案继续进入原失败终态。新的「重新生成方案」只复制草稿，已有草稿时保留其内容。在另一标签保存得到新的实例与 `doc-2`，切回旧标签仍固定该目标。隐藏确认和重复确认均被拒。用户路径的隔离可信确认后只提交一笔事务，目标读回 `Sketch / doc-3`，原实例为 `editable / doc-1`。关后有两个根任务及一个 P4 子任务，原根保留一次模拟传输、零工具和唯一失败终态。

首轮修后 Light 为 22/23，新任务在给编辑控制器分配内存时停在 `edit_budget`。在该分配点复用现有终态缓存回收，继续执行到了实际提交。次轮 33/34 的末项失败来自账本夹具把 P4 子任务也按根任务计数，核对原始关后记录后修正测试。另保留诊断编译时对前置声明类型取尺寸导致的 C2027。 [编辑预算关后记录](../../build/xuanshu-plan-response-20261126/light/plan-schema-closed.log) · [子任务计数关后记录](../../build/xuanshu-plan-response-20261126/light-final/plan-schema-closed.log)

最终同范围源码的 Light 34/34、ASAN Light 6/6 分别通过，包含表格、EDA 真实图块写入、隐藏与重复确认、旧计划、引用关闭、公开 C/C++ 头。生产 DLL 导出仍为 373 项，名称集合保持一致。验收使用隔离模型和测试自有数据。 [Light](../../build/xuanshu-plan-response-20261126/light-verified/junit.xml) · [ASAN](../../build/xuanshu-plan-response-20261126/asan/junit.xml) · [源码、程序与用例证据](../../build/xuanshu-plan-response-20261126/verification.json)

DeepSeek Responses 官方文档支持 `text.format` 下的 `json_schema`。当前实测发送前计量器仅接纳原七字段请求，添加 `json_object` 或 `json_schema` 均退出 2 并返回 `meter_protocol`。固定编码实现会将 JSON Schema 编入模型输入。启用需有支持完整请求和 Schema 输入计量的匹配身份。本次离线结果适用于方案拒绝、重新生成和再确认路径，真实模型仍待验证。 [官方请求字段](https://api-docs.deepseek.com/api/create-response/#request) · [计量器同程序对照](../../build/xuanshu-plan-response-20261126/structured-output-meter-comparison.json)
