# RPCB-1：一次确认的受控T2/T3验证批次

2026-11-02，实施前冻结。仅验证专用增量；用户已明确授权本合同调整，不署名。普通RPAB-1、C2/P4/P5及v1布局/错误数值、版本/依赖/限额不变。自审不代替独立复审。

## 1. 单一来源和明确增量

新增独立ai_validation_batch.h及入口；原Service、目录、Provider、队列、SQLite事件表及read执行器复用，不创建第二执行器。普通model_open的路径派生ref及逐任务确认保持；仅RPCB入口可将新的验证配置绑定到用户指定的固定DeepSeek引用ui-framework/xuanshu/deepseek/d15f7883aeeac194fe15029c233b44bdeb1f20dcd3d2f680373ed5e70803ab38。其他引用、origin、模型、历史和任意任务模板不接纳；不枚举凭据、复制Key到新引用或修改旧库/config。

批次open描述size/version=1，包含验证模型配置路径和一个完整read目标；数据复制，UI线程调用。后台完成配置加载/初次保存和指定引用本地读取检查，readable不虚构本进程保存。不可读取时只提供既有可信原生Key入口，明确遮罩输入与保存/取消；正常读取成功不再要求用户输入。Key/配置I/O在worker，临时明文清零，不经过页面/事件/日志。

批次bind_button为独立size/version=1结构，绑定当前批次摘要及Win32按钮，仍由原输入来源检查和方法租约一次消费。没有公开confirm、自动批准或自报source入口；旧model确认、旧propose/continue不能绕过批次。get_state返回复制脱敏JSON，ui_ai_release释放。旧结构和枚举数值不变。

## 2. 一次确认及持久顺序

确认预览固定批次ID/Service session、两个任务原模板、history=[]、系统指令/两read包装完整定义、DeepSeek origin/model、配置revision/ref、目标instance/host/open epoch/r1/scope、目录指纹、能力/字段白名单、T1=0/T2=3/T3=1/合计4、USD0.10及10分钟/网络期限。JCS/SHA-256绑定完整预览；条件变化拒绝，不自动改绑或重签。

可信“开始受控补测”一次消费仅接纳该批次，立即开始10分钟计时；batch_confirm事件提交确认后才可接纳T2及内部授权。任务关联、任务保存和各任务disclosure继续原持久化顺序。自动内部确认仅允许该已保存批次的固定T2/T3；没有外部调用的自动确认接口。重入/重复点击不再执行，重启只恢复记录、封闭执行，不恢复批准。

所有模型请求及续轮在发送前共同检查已保存批次、配置/目标/目录/期限、固定任务关联、T1禁用、R总4、T2≤3/T3≤1、费用及原存储确认门。失败/取消/发送未知占额，不返还/挪用/重试。原RPCP-1模式5及1/3/1保持，仅RPCB加强为4及0/3/1。

## 3. 工具与业务证据

xs_read_discover的实际序列化Schema明确page为integer、enum=[0]，说明仅支持0；适配器仍拒绝1，不映射或重试。系统指令/工具定义由同一私有常量生成请求与预览，测试直接解析实际请求，不能检查ID子串代替Schema。

T2严格先发现memory.read，再读取。沿原bind/prepare/dispatch/稳定ID/result/forget，记录实际目录描述及read JSON，必须整数10/字符串r1且P2框架结果revision=r1；这些证据保存后才允许T3。仅模型回复、目标r1或执行次数不算成功。增加验证专用proof事件，无DDL或公共结果结构变更。非read、其他能力或没有发现证明的read拒绝。

## 4. 自动执行、取消及收尾

新验证启动器只有完整预览、可信开始按钮、必要时Key入口和紧急停止，T1无入口且API拒绝。用户一次开始后程序自行推进，不要求复制JSON、逐任务确认或手动关窗。

T3在生产SendRequest进入已确认、连接/请求尚在途时通过控制路径取消；不以固定等待猜测。若完成先到，记录取消未覆盖，不追加。费用known/estimated/unknown沿用冻结参考，usage缺失/不确定后停新请求，取消不等于远端停止/免除费用。网络10/30/30/120秒及批次10分钟不提高。

完成/失败先quiesce，继续poll排空网络/应用调用/worker/回调。后台将有限的合成运行/消息/事件/工具/阶段/计数导出到新库路径加.batch.json，不含Key/Authorization/Blob/内存，文件CREATE_NEW。导出与关闭有明确成功/失败状态；SQLite不可用仍cleanup-only及最小错误导出。导出完成后请求关闭，等Service CLOSED及worker/回调租约退出才destroy并结束窗口。UI不等待worker或队列空间，不强杀/提前卸载；不能安全退出时保持可观察。

## 5. 预算、验收及停止

私有批次结构、复制目标/预览、绑定及导出工作缓冲计入原Service8MiB，最多一个批次。导出有界512KiB，超限明确失败，不静默删最终结果。所有新结构size/version=1，借用期仅调用期间，worker不持应用裸指针；结果/工作项及外层方法生命周期沿原门。

新独立构建根build/xuanshu-rpc-batch-20261102-8ca4d2；保留修前工具Schema有界失败及首个编译/测试失败、同期源码、JUnit/日志/匹配程序。离线覆盖批次一次确认/伪造/重复/变更/过期、凭据替身/复用、未确认零发送、0/3/1及总4、真实业务/框架revision证明、存储前门/checkpoint/背压、取消/重启封闭/自动导出关闭。替身/私有接纳仅测试构建，生产导出不得包含。

离线稳定及自审后才启动唯一真实轮，预先核对官方资料与当天费率。助手不读取实际Key；程序仅访问已指定引用。真实失败后只读定位，不改实跑程序续发、不新建轮次追求通过。T1及历史失败保持，不自动增加请求；通过结论仅按实际证据，取消/费用未知如实记录。

## 6. 实际交付与观察约定

已实现的三个独立入口为ui_ai_validation_batch_open、ui_ai_validation_batch_bind_button和ui_ai_validation_batch_get_state，原model动作枚举不增加公开值。启动器xuanshu_rpc_batch使用NEW_SESSION_DIRECTORY、FIXED_VALIDATION_MODEL_CONFIG及rpc_memory.uapp三个参数；只创建一个新会话目录，配置由显式独立路径绑定指定引用。普通model_open仍按路径派生引用。

初始同ref检查由后台单独生成conversation.sqlite.credential-check.json，只有脱敏引用、read_error、revision及local_readable/need_key，没有Key。此文件是初始检查而非服务端验证，也不冒充本进程保存；不覆盖原文件。主窗口以可读中文概述加完整格式化预览展示范围，标题“可开始”只代表准备及可信按钮绑定就绪，仍须真实人工点击。不可读取时只显示一次Key入口；取消或失败结束本轮，不反复要求录入。

结束时后台生成conversation.sqlite.batch.json，包含有界runs/messages/events/tools、实际P2结果与framework_revision证明、M/R/S/C/U、模拟数、凭据阶段及费用。文件生成早于最后关闭，不能仅用该文件证明destroy完成；启动器进程正常退出后stdout的service_destroyed/workspace_destroyed及关后只读库共同核对。导出失败明确返回非零，不覆盖旧导出。

公开协议资料于2026-11-02只读核对：[Responses端点/流与工具](https://api-docs.deepseek.com/api/create-response/)、[当前模型及价格](https://api-docs.deepseek.com/quick_start/pricing/)。deepseek-flash及POST /responses存在，采用reasoning.effort=none；本地保守参考以输入300、输出1200纳美元/Token计，不使用缓存/非高峰优惠，不作账户账单。没有调用模型列表、余额或其他API。

## 7. 启动准备补充

用户追加授权：可信批次接纳及额度消费之前，允许最小准备修复与重新启动；此前失败仍保留，接纳或消费后仍失败即停止。新增--startup-check仅校验会话路径/原生窗口/框架初始化，不创建Service、读取凭据、接纳任务或联网，不是授权旁路。监督进程持有子进程句柄到退出并保存退出码；生产程序需在能访问用户已指定凭据的正常用户上下文运行，不从受限沙箱访问失败推断Key失效。仍仅由生产程序读取指定ref，助手不提取或枚举秘密。

## 8. 预览接纳门及诊断限定整改（待独立复审）

上一轮独立复审结论为暂不通过/需先整改：必需预览控件或投影失败时，开始按钮仍可能接纳；另有启动诊断错误域与遗漏。该结论保留。此次仅启动器和隔离测试/注册增量，不改变运行时、公开结构、错误数值、可信人工来源或已冻结批次授权。

启动时逐项确认开始、Key、停止和预览控件创建成功，失败在任何Service/计时器/凭据工作之前诊断并退出。同一摘要和完整文本已确认显示时，稳定刷新保持现有按钮状态和鼠标捕获；需更新文本、摘要变化、失败或关闭时先禁用开始并撤销已显示摘要；解析完整preview对象、校验其JCS指纹等于当前batch_hash、完整序列化/转换成功、SetWindowText成功并核对实际文本长度后，才标记该摘要已显示并绑定/启用开始。准备占位、旧摘要、部分文本或仅有绑定均不算完成；摘要变化解绑旧绑定。相同完整投影可复用，不反复重置选区。投影失败明确一次诊断及禁止开始标题；成功恢复后可重新展示/绑定。刷新重入只跳过启动器自身重复刷新，不禁止原Service或应用回调重入；关闭会立即禁用开始并阻止外层重新启用。

诊断分error_domain=win32/ui_status/launcher，code_known及有符号code；未知为null而非0成功。框架错误不转成Win32数值，已有路径转换、控件/窗口、计时器、框架、workspace和Service初始化分支均记录阶段，仍按原异步关闭门收尾。日志不包含秘密。--startup-check仍无Service/凭据/网络；测试故障和Service外观替身只在独立测试可执行文件中，生产DLL不增加导出。

本轮只做离线验证和自审；已有T2真实10/r1及T3取消/费用unknown不重跑，账户日汇总差483 Tokens仍独立待核对，不推定为T3 usage。新交付等待限定独立复审，不自动冻结。
