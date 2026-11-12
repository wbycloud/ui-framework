# UHS-1简短零Key、零联网人工复核

状态：本轮必要人工检查及关后核对完成，限定通过。

冻结依据见[独立复审记录](xuanshu-uhs-cache-independent-review.md)。本轮新证据根build/xuanshu-uhs-cache-manual-20261104-b19f62；数据库session/conversation.sqlite，模型配置为该路径加.model.json。保留所有旧库及证据，不改变LOCALAPPDATA或复用旧凭据。

仅一次初始启动，监督保存PID、stdout/stderr、心跳和实际退出码；提前退出停止、不重启。仅生产宿主/DLL/rpc_memory，不用测试入口。

人工按实际控件逐步进行：AI展开→offline-fixture配置→memory.document/read/value/revision（不选历史）→一次本地未确认提案→中文任务状态及范围定位→固定目标→详情折叠/恢复→未提交草稿保持→取消→正常关闭。每次只给一个当前步骤，不生成stub历史，不点击Key或外发确认。

任务输入：UHS-1范围摘要人工复核：仅查看合成内存只读范围，不发送到模型。
草稿：UHS-1未提交草稿：查看摘要与详情后应保留。

用户观察、当前软件状态和关后持久化分别记录；关键诊断原则上一次。关闭后先确认实际退出，再以immutable只读方式核对本轮库；若有未合并WAL则停止完整性判定，不checkpoint、不重开Service。应只有预期任务及取消终态，无确认/模型请求保存/工具执行，不虚构RPC计数或抓包证明。

## 实际结果

各步用户反馈及关后结果见下节；不自动进入真实Provider或其他批次。

## 最终结果（2026-11-04）

UHS-1简短零Key、零联网人工复核通过。用户实际观察：中文任务状态可辨识；“查看本任务范围”可定位输入及未选历史；固定目标memory.document/r1/read及memory.read、revision/value清楚；完整详情共2块，收起后隐藏且确认不可用，重新展开恢复2块。未提交草稿完整并可定位光标，原已接纳任务输入未被草稿覆盖；复制脱敏诊断可用。截图保存于本轮证据根。没有人工故障注入，不替代离线中断恢复证明。

软件诊断（用户粘贴）确认run_id `52c0d25dc7f42c12ae202e1083e0f9b3926344f2dac38443d1385a75419aa45e`、持久化接纳、phase3等待确认、display_valid=true、代次59；凭据stage=not_configured、submitted/local_readable=false。配置ready仅为本地合成配置，不是凭据可用或服务端验证。诊断关键字段另以事后摘录保存，不冒充原始自动抓取。

用户取消截图显示cancelled、终态保存是、外发/确认保存/请求保存否；用户正常关闭后，监督记录PID19548实际exit0，1899.094秒，无强杀、重启。stdout/stderr保留。

关后本轮库以mode=ro&immutable=1核对，读取前后SHA256不变，无WAL/SHM。仅1任务、2消息、3顺序事件（accepted、awaiting_disclosure、finished）、0工具，唯一cancelled终态committed=true。历史[]、目标instance1及host/open epoch、memory.document/r1/read和白名单保持；未提交草稿未成为额外任务或消息。终态确认/确认保存/模型请求保存均false，model_calls/tool_calls=0、provider_request_id空、remote_delivery=not_sent；没有重复提交、模型工具派发或重放证据。

DB SHA256：`7A11F53888A05C9092CA7B5E022A9FDC861948163ADB25F1C0560AA56C8E619C`。产品418源码/53产物关闭后仍匹配冻结快照。身份清单、入口文档副本、新文档、用户观察/截图及关闭证据分开保存。没有全程网络遥测，因此结论是用户零Key操作及未进入模型请求路径的限定证据，不是抓包证明或精确HTTP计数，不使用RPC M/R/S/C/U。

本轮无阻断异常；真实设备仅覆盖实际鼠标/焦点/阅读操作，不等于完整来源分类。IME、DPI、辅助技术、人工长时、专用断电及browser另案继续保留；历史失败和期限风险不覆盖。原交付冻结身份不改，本轮文档新状态独立。未修改产品、未重建/离线测试、未读取凭据/Provider或旧库。无无用途材料需清理，保留本轮配置、库及必要截图/日志；停止，不启动其他批次。
