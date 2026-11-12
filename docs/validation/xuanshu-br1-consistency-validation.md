# BR-1 模型名一致性诊断与启动取证记录

## 结论：部分完成，冻结阻断保留

2026-11-05。原交付7C1FB0F647B8CB9F9270A00DF914B03427C8D9071FA7E12E93AC87892ADA5595匹配。新证据根build/xuanshu-br1-consistency-20261105-f4c92a。638份入口源码/文档副本是编辑前范围，不是全部Git快照。参考工程只读可访问；未修改。

启动异常取证已整改并通过隔离辅助进程验证；模型名少字符仍未定位，不能宣称整改完成或具备冻结条件。原ASAN失败库plan.model=offline-fixtur和恢复前后三项断言失败继续有效。同程序原补项及本轮通过均不能覆盖该首败。

## 逐边界证据及缺口

tests/ai_edit_panel.c增加只记录合成模型名的requested/input-return/after-pump/before-save记录及实际全文断言。UI_HOST_TEST且定义UI_HOST_MODEL_TRACE时，宿主额外记录页面消息原文、解析answer及进入configure的值；生产编译无这些调用、无新增导出。tests/uha_summary.c直接比较配置读回、权威预览request.model、异步恢复快照stub_plan.model，日志携带操作序号、页面token、run及字节长度。没有修改输入内容、补字符或跳过保存。

初始Light诊断1/1（211检查）时尚未包含全部后续边界；补齐后最终Light相称运行包含persisted（215检查）。ASAN预先限定三次，各1/1，215检查，每次原30秒门槛；三次均完整15字节offline-fixture。改当前配置new-config-not-task后，历史任务仍保留原绑定。关后测试自有库immutable核对无WAL/SHM、哈希前后不变，5个测试任务均保存完整模型名，详closed-name-observation.json。未触碰原用户库/人工库。

原失败没有输入返回、消息泵后、保存前、页面wire及configure入参的同期记录，因此当前不能确定原差异首次出现在哪个边界。检查Light文本替换、DOM value、JSON桥接、宿主parse_action和配置复制/写入调用关系，未找到可以由现有证据确认的末字节删除分支。不将缺少复现解释为环境问题、夹具问题或产品已修复；不作无依据生产逻辑修改。

下一步最小诊断范围是保留这些边界，在有依据的触发条件下记录首次不等，而不是自动追加轮次直到通过。若输入后已不等，再定位输入/同步事件；若wire前后不等定位桥接；若configure入参正确而读回错误定位复制/存储。责任尚不能归属，完成条件1/2未满足。此处不请求用户执行临时控件测试，不启动真实调用。

## 已完成的启动取证

新目录supervise.ps1和start-readonly.ps1只服务本轮入口，不改原脚本。阶段包括session-create、identity、local-config、prestart-record、process-start、process-record、process-wait、final-record。错误区分异常类型、HResult和可取得的Win32 NativeErrorCode；不可取得为null。无进程时pid/exit_code均null。启动后先保留进程句柄，记录失败也继续WaitForExit；不强杀/重启。finally恢复原UI_AI_DB_PATH及UI_BROWSER_SETTINGS_PATH，不改变LOCALAPPDATA。目录不可写时明确stderr报告未落盘，不虚构证据。

test-supervision.ps1仅使用新自有目录与powershell辅助进程，覆盖身份哈希不匹配、目录创建失败、预启动记录失败、Start-Process失败、启动后记录失败、正常0、非零7、最终记录失败。最终8/8；启动后记录失败仍观察到实际退出，环境恢复断言通过。测试写入替身仅在测试脚本覆盖Write-BREvidence，生产入口无故障开关。未启动正式宿主人工验收。

## 相称验证与首败

- Light affected.xml：14/14，含最终模型名持久化、gate/interrupted、buildgate三项、业务read/cancel/revision/reopen/production、静态嵌入及公共C/C++头。
- ASAN affected.xml：13/13，不再重复前三次已限定的persisted；原业务结果、revision及工具保存、隐藏/PENDING、关闭重入、恢复、公开read通过。
- ASAN boundary1/2/3分别通过；这些不合并为首次单次全过。affected两配置运行存在时间重叠，不能作为严格串行桌面竞争排除证据；三个ASAN持久化诊断逐次串行执行。
- 新诊断块局部变量d与原外层d遮蔽导致/WX编译失败，第一处修正后发现第二处，均保留日志/源码/上下文程序；改为trace_doc后编译通过，未降低编译选项。
- 只读rg通配路径/不存在文件和PowerShell未引用花括号路径错误仅存在工具输出；不得把本文事后说明冒充原日志。

运行时、公开头、依赖、来源政策和预算未改。原业务长历史/缓存及存储cleanup-only证据仅按不变路径复用，不宣称本轮全部重跑；本轮没有新增确认旁路或真实发送。原30.08/30.06/30.11秒、复制error5等历史风险未解释。已取消临时控件测试不恢复。

## 保全与停止

新程序只用于诊断候选，不继承原冻结结论。清单、匹配程序、原始日志/JUnit、首次编译失败、测试自有库及逐边界记录保留；只清理本轮可再生obj，详cleanup.json。原交付与原失败材料未修改。新身份是部分完成的诊断/监督交付，不是少字符修复证明。

必须修正项未清零；当前不具备宣称“模型名一致性整改完成”的条件。真实验证请求硬门仍为另行授权事项，本轮未实施。停在已完成取证整改和未定位阻断的证据交付，不自行独立通过、冻结或发布。
