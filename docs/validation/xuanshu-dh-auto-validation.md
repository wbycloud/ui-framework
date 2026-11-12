# DH-1零Key、零联网自动界面验收

## 结论与范围

2026-11-04，DH-1零Key、零联网自动界面验收通过。外部脚本操作匹配生产宿主的普通控件，结合目标窗口截图、控件全文读回、脱敏状态和关闭后的隔离库核对，完成一条最小日常流程。没有修改产品、重新构建或运行离线测试；没有用户逐步操作，没有使用computer use、子代理、测试DLL、私有可信入口或Service内部调用。

本结论不是独立源码复审、真实人工来源、真实Provider或整个系统验收，不自行冻结。原DH-1开发自审及离线验证保持各自证据边界；未发现可承接的DH-1独立复审通过记录，下一步可开展限定独立复审。已取消的临时页面控件测试不恢复、不列为待办。

## 身份、隔离与监督

匹配交付：`build/xuanshu-daily-host-20261104-c82f19`。交付identity SHA256：`65823A92BAAF49CE2623F5865F2C47B678215DD813D7E8EDDC8725980107A532`。启动前419项源码、74项产物及身份关联清单匹配，关后、文档更新前再次匹配。此次文档状态不冒充原离线交付身份。

| 程序 | SHA256 |
|---|---|
| light/framework_host.exe | 0B028BE68B45617A3F84A03F891546498EB9AC5723EB3041F4117F861662B22F |
| light/ui_framework.dll | D7169B2BCBCB25141EC71ADA05C4D52125A15A148C36922FE0437F879F61A4C2 |
| light/rpc_memory.uapp | EBF374A177B37135B212150DDF371207F92A8E610B38071D800B14858969629E |

新证据根：`build/xuanshu-dh-auto-20261104-07e6b3`。`UI_AI_DB_PATH`显式为该根`session/conversation.sqlite`；配置为`conversation.sqlite.model.json`。未改变LOCALAPPDATA，未复用旧库或凭据。参考工程只读可访问，未修改。

生产进程仅启动一次，PID12204，监督PID29268；stdout/stderr均为空。监督从Popen持续等待实际退出，取消保存后以WM_CLOSE正常关闭，实际退出码0。没有重启、强杀或卸载绕行。最初驱动pywin32导入失败发生在Popen之前，不是生产进程早退或第二次生产启动。

## 流程与实际证据

下列路径均相对于本轮证据根。每次操作依据当前截图或状态继续；异步状态未出现时重新观察，不以点击返回或固定延时宣称完成。

| 项目 | 实际结果 | 主要证据 |
|---|---|---|
| 日常首页 | 会话、当前任务、草稿分区存在；设置、开发诊断默认折叠；启动无当前任务 | screens/06-home.png |
| 合成配置 | 生产设置保存offline-fixture，代次1、配置读回；凭据not_configured、未提交、未保存、未本地读取 | screens/09-config-state.png；accepted-diagnostic.json |
| 结构化范围 | 历史未选；实例1的memory.document；只选memory.read、revision/value | screens/15-selected.png；request-readback.json |
| 本地保存 | “保存任务并查看范围”仅点击一次；已持久化接纳、等待外发确认；没有再次保存草稿 | screens/16-proposal.png、27-task-top.png；accepted-diagnostic.json |
| 输入及固定范围 | 实际两块详情经普通键盘复制全文并成功解析；输入、空历史、目标、read白名单、offline-fixture及固定instructions/两个tools一致 | detail-0.txt、detail-1.txt、request-readback.json；screens/18-fixed.png、19-details.png |
| 详情隐藏 | 文本显示详情收起不授予确认资格；确认按钮灰色，未点击 | screens/21-hidden.png |
| 草稿与任务分离 | 写入指定未提交草稿，不保存；已接纳任务仍为原输入与目标 | screens/24-draft-entered.png、27-task-top.png、31-before-cancel.png；draft-readback.txt |
| 范围隐藏/恢复 | 草稿存在时收起范围，再通过“查看本任务范围”恢复；详情仍为2块，两块全文与原始读取逐字一致 | screens/26-scope-hidden.png、28-restored-scope.png、29-restored-details.png；restored-0.txt、restored-1.txt |
| 草稿保持 | 恢复后实际控件全文精确等于“DH-1未提交草稿：查看任务范围后应保留。” | screens/30-draft-restored.png；draft-readback.txt |
| 本地取消 | 普通“请求取消当前任务”一次；随后显示cancelled及终态保存是，确认/请求保存否、远端未发送 | screens/33-terminal-saved.png；terminal-diagnostic.json |
| 关闭及落库 | 实际exit0；1任务、2消息、3事件、0工具，唯一cancelled；无重复任务或请求/工具重放 | exit.json；closed-db-observation.json |

任务run_id：`cc5288f4f5ac9716ac6d1771be23f77126c20a666e5a5b3671b59c04de1b75d0`；client_request_id：`panel-1a1a6763841-2`。

固定目标：instance_id=1，host_epoch=`31864d32-a355-4cf0-971b-6ae2b45dcdc8`，open_epoch=`1e33fb2e-7f3a-4003-bc94-c4eda3e1da1a`，resource=memory.document，revision=r1，scope={"access":"read"}。任务输入为“DH-1自动界面验收：仅查看合成内存只读范围，不发送到模型。”，history=[]，capabilities=[memory.read]，fields=[revision,value]。接受、隐藏、恢复、终态诊断及落库对应同一身份和固定范围。

## 判定边界

`hidden-diagnostic.json`中的display_valid=true是宿主状态投影完成标志，不能直接解释为确认资格有效或无效。源码`ai_model_panel.inc`的确认检查还独立要求preview_ready、实际文本/布局可见、run/hash及展示代次匹配；`ai_projection.inc`在状态投影完成后可置display_valid=true。本轮确认失效的动态证据是详情隐藏后明确提示与灰色确认按钮；不触发确认、不使用私有探针，因此没有动态执行旧确认拒绝路径，也不将该单一诊断字段包装成权限测试。既有离线确认门证据与本次界面观察分别保留。

截图直接检查了真实生产窗口的文字和布局；OCR仅辅助定位，存在中文误识别，不作为唯一判据。窄面板的输入框可能水平滚动而看不到开头；全文以实际控件复制结果核对，未将截图的可视片段误认为截断数据。两个完整详情块经标准Ctrl+A/C取出并解析，恢复后逐字相等；不是页面注入、网络抓包或数据库反向伪造预览。

PostMessage与标准注入键盘属于合成输入，不证明真实鼠标键盘、完整Windows来源分类。没有点击密钥或确认外发，未访问凭据API或环境Key。软件状态及持久化记录显示model_calls/tool_calls=0、disclosure_confirmed/disclosure_saved/model_request_saved=false、remote_delivery=not_sent；real_provider=true只表示任务类型，不表示已请求Provider。普通宿主无RPC配额策略，不提供M/R/S/C/U或精确HTTP次数。没有全程网络遥测，结论为“现有证据显示未进入模型请求路径”，不冒充抓包证明。

关后确认PID退出、WAL/SHM不存在，采用SQLite URI mode=ro&immutable=1，只读取本轮数据库；integrity_check=ok。事件仅accepted、awaiting_disclosure、finished，最后事件terminal=1；与终态消息和任务唯一对应。读取前后数据库SHA256均为`2969759861A5FD84C7D7DA8F329F705DC05C77F49374F635421C9FC60EF147A6`，没有创建WAL/SHM或checkpoint。

## 驱动失败、限制与保全

以下是驱动或取证问题，未证实为产品缺陷：启动前pywin32模块装载失败，改用独立ctypes驱动；受限上下文WinRT读取拒绝及初次Language类型准备缺失；Python默认GBK读写错误；Windows PowerShell无BOM中文脚本解析错误。均在本轮外部脚本内调整，没有改生产程序。相关可得原错误保留在ocr-first-error.txt、ocr-capability-log.txt及对话工具输出；未自动落盘的工具错误只作事后说明，不伪称同期原始日志。

一次初始诊断复制未在有界窗口内读到本轮诊断；当时取景变化，不能判定产品复制失败。重新依据当前截图定位后，接纳、隐藏、恢复及终态诊断均成功；没有再次提交任务。窗口截图早期一次被其他窗口覆盖，不作为证据，已删除该无用输出；之后改用匹配PID窗口PrintWindow。原始有效截图、状态、独立脚本、动作记录、退出记录和自有库保留。可再生成的2倍OCR图片等过程产物清理见cleanup.json，不清理历史材料或交付文件。

未发现本轮必要流程的产品阻断项。此次没有进行故障注入、真实来源、IME/DPI、辅助技术、人工长时、断电或真实服务验证；取消的临时页面控件测试不列为未完成项。既有历史失败、期限未知根因、Provider费用unknown等不由此次通过覆盖。

## 后续位置

本轮自动界面验收证据已准备，可进入DH-1限定独立复审及交付收口判断；不自动冻结、不启动新程序、不进入有Key或其他阶段。后续若需要改善窄面板全文阅读，可单独评估换行与摘要布局，不以该可选易用性建议阻断本轮已完成流程。
