# RPCP-1：首次真实验证准备增量合同

> 2026-11-02增量：原准备快照已限定复审通过并记录冻结，但旧真实T1在发送前失败且用户未见Key输入窗。现按独立授权实施[RPC-K1凭据/体验整改](xuanshu-rpc-credential-closeout-contract.md)，只添加私有阶段和get_state诊断对象、可靠原生录入/复制，不改变RPCP-1配额、外发、错误数值或公开布局。新代码不自动继承旧冻结结论，旧R=1不退还。

2026-10-31，实施前冻结。本轮仅准备实现与离线验证，不授权联网、真实Key或真实人工验收。继承RPAB-1，SDK0.10-dev/API10/标准10、ABI1/包1、旧公开布局/错误、v1/C2/P4/P5、依赖及上限不变。

## 1. 独立限制接口

新增ai_validation.h，独立size/version=1描述及任务枚举；enable/get_state/propose/stop为UI线程接口。enable只限制现有Service，不能确认外发或输入Key；无自报人工source。配置固定DeepSeek/flash及任务白名单。model配置必须显式为storage_path加.model.json，与验证启动器新目录绑定。后台检查新数据库并保存rpc_open事件后才ready；使用过的库重开只恢复计数、保持封闭，不能重置预算或继续授权。旧接口布局和错误数值不改，不启用策略的RP保持原合同。

propose只能选择文本/发现读取/取消三任务，固定原计划输入、空history、0或1固定目标及投影；每类绑定一个client/run，同ID同内容幂等，不同内容明确拒绝。旧real_propose不能绕过启用后的任务关联。仍须原生/Light可信人工确认，策略API不签发授权。

## 2. 发送与计费合同

同Service全局R≤5，各任务1/3/1，不挪用、不返还、不自动重试。原provider_request提交后重新UI检查，发送前原子保存rpc_reserved记录，绑定session/run/request/ordinal/body hash；COMMIT成功但checkpoint失败仍消费R且禁止发送。每个request最多一个发送意图。最后一次取消/期限/存储检查后才进入WinHttpSendRequest，API返回后登记已实际进入的S，与manifest M及完整响应C、发送未知U分开；调用在途send_count_known=false，不能将调用前意图称精确进入次数。发送后保存ack失败send_count_saved=false并封闭后续，不能伪装未发送。存储使用原events及幂等op_id，无新DDL。

首次原生确认起10分钟停新派发并请求取消；仍等待真实回调/连接/worker退出，不强杀或提前卸载。费用阈值USD0.10；费率采用已确认Flash峰时输入每token300纳美元、输出1200纳美元（可由描述提供更保守的费率，不允许降低），不是实时账单。发送前预留估算=（编码body字节数+1024的保守输入估算）×输入费率＋4096×输出费率；该字节代理只标估算，绝不称精确token/服务端费用上限。有效usage保存后才以known替换本次预留；缺失/异常usage、网络错误/未知或费用不可核对，保留估算并封闭新发送，unknown不记0。预算不足不发送。真实调用前还须复核当天费率和费用不确定性。

后台持稳定ID/复制数据，所有SQLite与网络仍原串行worker。策略状态有界分配计入原8MiB；UI只查询复制投影，不做I/O或等待worker。scan/net引用、存储失败cleanup-only、UNKNOWN及注销门不变。隔离替身的模拟进入/响应/未知另计，实际S/C/U保持0，不冒充真实WinHTTP执行。

## 3. 交付与验收

生产链接只读uapp唯一memory.read/resource memory.document，公开API返回真实内存10/r1及原计划Schema，无写入/事务/外发/私有接纳。验证启动器原生窗口显式创建新目录内库/config，使用生产Service、原生Key与确认绑定，未点确认不发请求；默认启动不执行任务，目录已存在拒绝，不能覆盖默认用户数据。不用旧隔离DLL替代生产运行时。

新增离线夹具验证5及1/3/1、续轮/共享/重复、费用估算/已知/unknown、期限/取消/关闭、持久化前失败与checkpoint、重开封闭、固定目标与业务10/r1、脱敏/回调/背压及静态/Light/公共头/导出。所有私有接纳/故障/传输替身仅测试构建；生产DLL仅新限制API，无fixture导出。新独立构建根保留首败/源码/JUnit/匹配程序，不修改历史证据。不把准备通过称真实服务或独立复审通过。

## 4. 实际接口与所有权补充

ui_ai_validation_enable复制validation_id/参考费率，首次enable返回OK仅表示已接纳后台初始化，get_state.ready才表示rpc_open保存确认；重复enable返回ALREADY_EXISTS，配置路径不隔离返回PERMISSION_DENIED，关闭或未READY返回PENDING，分配失败沿用OUT_OF_MEMORY/LIMIT_EXCEEDED。这些均沿用现有错误数值，不改变旧接口。

propose复制client/固定目标并立即建立原Service run关联；同任务同client同目标返回原run，不重复发送，不同client/内容ALREADY_EXISTS；非法size/version/任务参数INVALID_ARGUMENT，未启用/失效/目标不符PERMISSION_DENIED，已有在途模型调用PENDING。目标副本含instance、host/open epoch、resource、revision和scope。返回OK不是已保存/外发批准，仍走原外发预览确认。get_state返回复制JSON，调用者用ui_ai_release释放；借用描述和目标仅在函数期间有效，不留外部指针。

stop只请求停止和取消，不代表已退出/退款/可销毁；destroy仍按原方法、worker、网络及应用回调租约返回PENDING。策略5个固定ticket以及文本/身份存储计入原8MiB，扫描和网络引用独立；无公共凭据参数/confirm/传输注册接口。rpc_open/reserved/send_intent/send_entered/network_end/cost为既有schema3事件的版本化载荷，无新DDL，后续重开只能对账封闭。费用known为本地有效usage计价部分，unknown单次记录null，估算保留，不声称服务端结算准确。

## 5. 限定评审整改状态（2026-10-31）

原准备开发通过后，独立只读复审指出同步received observer期间，旧real_propose/continue的client比较先于ID校验，限定验收暂不通过。后续专项整改已在am_propose初始校验先执行ai_id，非法client一律INVALID_ARGUMENT；合法不匹配仍PERMISSION_DENIED，不改变准入政策或正常重入。产品差异仅该条件，三任务、配额、费用、期限、线程、来源、公开结构及错误数值均不变。

新增三组隔离回归和生产/静态10/r1完整JSON断言，修前受控子进程异常、错码与夹具编译错误分别保留；详[限定整改记录](validation/xuanshu-rpc-preparation-closeout-validation.md)。新候选身份为build/xuanshu-rpc-prep-closeout-20261031-f27a64，不能自动继承原准备或RP-A/B验收。停止整改代码评审，不自行宣布独立复审/冻结，也不授权真实调用。
