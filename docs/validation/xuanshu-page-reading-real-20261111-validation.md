# PB-3一次真实公开网页阅读验证及交付收口

2026-11-11。PB3-REAL-20261111-6c4f82唯一真实业务只读任务已完成并正常退出，本次限定业务闭环通过，交付记录已收口。答复核心配置特性及无待办结论符合原文，另有一项未标注推断的质量瑕疵，单独记录。该具体包已消费，当前没有运行宿主、待Key或待确认操作。

## 身份、授权及实际程序

匹配冻结根为build/xuanshu-page-reading-20261111-a18d74，delivery-identity.json SHA256为4A892228066D04EB09972594509892AF3B960CF32569DE33A125F11BEF611A42。授权包位于build/xuanshu-page-reading-real-preparation-20261111-6c4f82/authorization-package.md，SHA256为DE37BAC245BB0734161DD00583A118D96DFE3AE2C43630D6A366E3B803D0E8B6。用户明确批准“明确批准本日包，仅执行这一次”，另通过原生入口保存Key并本人执行一次逐任务可信确认。

唯一实际启动PID8976，监督exec session5138。生产宿主、DLL、reading-production.uapp及固定计量组件与冻结身份一致。启动前546份源码、195项程序/链接产物和12份当日公开官方依据再次匹配，确认前四份生产文件、包和12份依据继续一致，关后源码及程序身份仍相同。实际正常退出码0，监督记录errors为空、exit_observed=true、environment_restored=true。没有重新构建或产品改动。

实际执行根为build/xuanshu-page-reading-real-run-20261111-6c4f82，显式独立数据库为session/conversation.sqlite，模型配置为该路径追加.model.json，来源配置为session/origins.txt，策略为session/business-policy.json。原历史库、批准、额度及凭据引用保持原消费状态。

## 实际输入、范围及确认

固定输入如下。

> PB-3：阅读SQLite《Zero-Configuration》公开网页隔离副本，概括配置特性、关键事实及待办；原文没有待办时明确说明，不导航或修改。

唯一任务为1864c87d3957d0b6bfd71dd50e28fbd8ccd21846d11ed6a5f6eb8418e41aaa34，模型deepseek-flash，配置代次2，history=[]，场景public-sqlite-zeroconf-v1。固定目标为browser.view实例1，host_epoch=b50ba334-04ba-4a6a-84fc-ad24b0811f97，open_epoch=dd335699-e75e-4643-ae12-43fd215575ef，revision=77fbdb9e18262e8aa0c807ae275fbfdab249f027c46f7068e4831509755f70af，scope为只读。能力仅browser.body，字段仅body/revision。

普通驱动通过现有控件配置模型、结构化选择和点击一次“保存任务并查看范围”。模型和任务输入均先复制实际全文核对，确认持久化接纳后读取两块实际完整预览，块长3000/1156 UTF-8字节。初始1608字节请求与批准准备逐值及哈希一致。接纳阶段confirmed、request_saved、占用和传输入口均为0。

用户原生Key录入期间全部截图、剪贴板和输入驱动暂停，反馈“已保存”后仅枚举PID可见窗口及类确认原生窗口结束，再恢复普通驱动。助手没有读取Key或密码框。用户反馈“已确认”后，软件及关后账本共同证明外发确认、确认保存、模型请求保存和终态保存均已完成。普通脚本输入按合成来源记录，可信确认由用户本人执行。

## 实际正文和结果

业务数据为SQLite官方Zero-Configuration完整HTML的包内browser:start隔离副本，原始HTML4209字节，SHA256为D8ECCE18365870C07E60802A76585B9FDE5CCD53714F579A3C8C5BE4E80387EA。实际browser.body返回847 UTF-8字节，完整正文SHA256为A73C755A8328BD65D7085506C145EACD12760DAA3634A5DEAF42E750D24B773D，与独立全文基准逐字节相同，bytes为整数847、complete=true、semantics=dom-body-v1，结果revision与固定目标一致。[原网页](https://www.sqlite.org/zeroconf.html)。

模型先以空category发现授权目录，再通过xs_read_call实际执行browser.body。实际工具保存为read-directory和browser.body各一条。原目录记录保留应用的两个read描述，给模型的续轮只包含本任务允许的browser.body描述，正文续轮只包含body/revision。三份续轮重建与持久化body hash完全一致，没有其他能力、导航、写入或工具重放记录。

模型回复概括了无需安装/setup、独立server进程、管理员创建实例或配置文件等特性，并明确“原文没有任何待办事项”。实际结果控件两页900/165 UTF-16单元与完整回复加outcome/error全文字节一致，任务状态控件全文显示终态和最终保存为是。截图、实际控件读回、软件诊断和关后记录分别保留。

答复把“嵌入式、进程内”作为推断写入关键事实，原847字节正文没有直接使用这些表述，答复末尾“未做超出正文的推断”因此不够准确。这是本次真实回复的来源标注瑕疵，不影响已证实的正文读取、范围约束、保存及关闭闭环。后续来源定位或推断标注改进保持Proposed，本轮保持原程序和唯一调用次数。

## 实际请求、计量及费用

| ordinal | 实际重建请求字节 | 输入token | 缓存token | 输出token | 本地保守费用nano USD |
| --- | --- | --- | --- | --- | --- |
| 1 | 1608 | 556 | 384 | 91 | 276000 |
| 2 | 3011 | 899 | 512 | 78 | 363300 |
| 3 | 4437 | 1236 | 896 | 501 | 972000 |

每轮实际计量receipt与响应input_tokens逐值一致，reasoning_tokens均为0。预备请求字节1608/2927/4245与实际续轮3011/4437按各自身份记录，实际服务返回的调用ID及状态字段进入真实续轮，完整计量和body hash分别对应实际请求。

3次占用、3个非模拟传输入口和3份完整响应均已保存，模拟计数及远端unknown为0。每次发送前预留10915200 nano USD，总保守预留32745600 nano USD。最终本地known=1611300 nano USD，即USD0.0016113，未结estimated=0，fee_unknown=false。所有输入按峰时cache-miss价计入本地保守费用，实际账户扣费保持未核实。[当日价格依据](../../build/xuanshu-page-reading-real-preparation-20261111-6c4f82/price-basis.json)。

总上限3次、本人可信确认起10分钟和USD0.10停止阈值处于本次绑定策略中。完成后保护以business_task_finished停止。本次实际运行没有触发第4次、期限或unknown停止反例，这些反例继续引用匹配离线证据。占用、传输入口、完整响应和模型轮次分别记录，重建请求属于账本关联证据，账户账单及全程网络抓包未取得。

## 关后只读及安全关闭

确认真实退出后检查本次新库无WAL/SHM/journal，以mode=ro&immutable=1及query_only读取。读前后数据库SHA256均为0321EECA060D86C6FEED71E7011684D94006B020E9721B0087E9242FFBEA5013。

关后1任务、2消息、31事件、2工具，唯一completed终态。固定输入、空历史、场景、模型、配置代次和目标与确认前记录一致。每个请求ID、ordinal、绑定hash、body hash、计量receipt、usage及费用逐值对应。没有重复任务、额外提案、重启执行或授权恢复。唯一WM_CLOSE正常关闭，真实退出后没有重新启动Service。

closed-db-observation.json、closed-db-schema-observation.json、execution-assessment.json、postflight.json及reconstructed-requests/提供本次明确证据范围。原始数据库、日志及截图保留，模型配置文件内容未由助手打开，原生凭据及环境Key保持不可访问。

## 驱动首败、保全及清理

OCR首次把“任务”识别错误，定位返回空集合，未派发点击，随后按实际截图采用普通坐标。终态控件首次全文比较中，本轮生成的expected文件被Python默认Windows换行写成CRLF，实际控件为源字符串LF。保留首个expected、实际全文及首次比较记录，仅修正预期文件的newline=LF序列化后全文字节相等。以上驱动问题与产品及真实请求分别记录，未自动落盘的工具错误在driver-errors.md明确作为事后追记。

清理仅本轮screens/中30份可再生成的双倍OCR图，13040331字节。全部原图、OCR JSON、失败读回、数据库、原始日志、计量/请求关联、授权和匹配程序保留。原PB-3快照、冻结SDK及历史失败和旁文件没有改动。

BR1-KI-001、计量组件任意卡死、ASAN首轮五项摘要失败与期限未知因素、首轮PDB缺失及历史其他风险保持原记录。真实来源完整分类、IME/DPI、辅助技术、人工长时、断电和browser九项副作用保持既有未验边界。临时控件测试维持取消。

## 结论与停止位置

本次PB-3真实业务只读任务的本地接纳、本人确认、生产真实请求、精确正文、范围保护、结果展示、持久化及安全关闭形成闭环，限定业务验证通过。回复质量瑕疵和账户/抓包等证据边界已明确记录。

PB3-REAL-20261111-6c4f82已消费，匹配冻结快照与本次真实结果分别归档。当前没有运行宿主、待Key、待确认或自动后继操作。后续新的网页、功能或真实请求均保持Proposed，等待对应独立授权。

## 此前进行中记录，原文按时点保留

# PB-3本日一次真实阅读验证记录

2026-11-11，进行中。用户明确批准PB3-REAL-20261111-6c4f82，匹配快照4A892228066D04EB09972594509892AF3B960CF32569DE33A125F11BEF611A42。12份官方资料和546源码/195产物启动前一致，唯一PID8976，监督session5138。

本次新执行根build/xuanshu-page-reading-real-run-20261111-6c4f82，显式独立DB/来源/策略和原生凭据引用派生位置。deepseek-flash控件全文经实际复制核对，再通过普通保存入口读回配置代次1。尚未提案或可信确认，当前等待用户原生Key；截图、剪贴板及输入驱动暂停。程序实际退出和最终结果待后续取证，当前不作通过结论。

原PB-3限定复审及冻结、原始ASAN失败、首轮PDB缺失、其他旁文件和BR1-KI-001保持。普通驱动属于合成输入，逐任务可信确认仍由用户本人执行。

## 本地接纳及确认前实际证据

用户原生保存后仅枚举PID窗口/类确认原生窗口结束，再解除驱动暂停。配置诊断为deepseek-flash、profile2、本地可读取，验证保护ready，计数0。唯一提案按钮已派发一次，持久化接纳已由状态及诊断确认。完整详情两块实际全文解析，与包中1608字节请求逐值和字节SHA一致。固定run、目标及epoch/revision、空历史和body/revision均在accepted-diagnostic.json及full-preview-readback.json中。

当前等待本人可信确认，尚无本轮模型请求保存或外发。新一次12份官方依据及生产文件确认前核对均匹配。OCR“任务”定位失配未派发按钮，随后审阅截图后使用普通坐标，已单独保留driver-errors.md。
