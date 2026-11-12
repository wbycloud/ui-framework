# RPCB启动准备整改记录

2026-11-02。用户另授权在批次接纳/额度消费前持续修复准备问题；接纳或消费后仍失败即停止，不退款/重试/追加轮次。旧PID5916早退、旧程序和全部证据不改。

本轮仅tools/xuanshu_rpc_batch.c增量：早期参数、路径/窗口错误的脱敏阶段及错误码；--startup-check只创建窗口并初始化框架，不创建Service、不读凭据或联网。独立构建根build/xuanshu-rpc-startup-20261102-2bd7e4，以独立CMake链接原已验证Light生产import库及JSON库，生产DLL/uapp哈希完全匹配旧交付。版本/合同/4及0-3-1门和实际执行器未改。

构建首轮默认Debug编译探测出现C1902，保留configure-first.log；改为既有Release探测。首次误链静态ui_framework.lib出现134未解析符号，保留link-first.log，纠正为生产ui_framework_runtime.lib。均为构建编排问题，不冒充产品缺陷。

startup.xml三个子进程用例通过：中文含空格路径成功，缺失父目录错误3、已有目录错误183，均零Service/凭据/发送。原程序受控缺失父目录返回2但无错误，不能据此断定旧PID5916根因。新监督脚本保持进程句柄并保存退出码。

准备重启PID13052已稳定并生成后台credential-check：指定ref本地读取失败，read_error=5。尚未请求可信批次开始，无请求；不将访问被拒绝归因为Key或服务端。避免让用户重复录Key，需要安全停止当前准备窗口，再核对启动权限上下文。尚未真实T2/T3，不能报告真实补测通过。后续只追加。

## 准备整改及真实批次最终结果

PID13052由用户正常停止，自动导出M/R/S/C/U均0、batch_confirm为0、service_destroyed/workspace_destroyed=true，未接纳/未消费，退出码2。按本轮新增授权继续准备，没有退款或改写该库。

同一新程序、相同指定ref及同一profile，以正常用户上下文在受限沙箱之外受监督启动PID34224；凭据read_error从5变为0，local_readable=true且need_key=false，未再输入/保存/复制Key。受控对照支持先前访问拒绝与受限启动上下文有关；不把它扩写为旧PID5916早退的根因。此前首次早退具体错误仍无原始捕获。系统访问上下文修正不改变生产来源、批次授权或配额。

生产框架记录了一次可信原生batch_confirm（batch a79dbc66-8f58-43e5-b109-2ed8b64713ba）；助手没有发送程序授权消息、使用私有确认或操作UI，也未独立观察用户点击动作。确认后自动完成全部执行与取证，用户无需逐任务提交/复制JSON/关窗。

T2共3次POST：发现page0、实际memory.read、模型汇总；完整read描述及输入输出Schema已保存，实际结果为整数10、字符串r1，独立framework_revision=r1的proof事件已保存。唯一completed终态。T3共1次POST，生产在途状态触发取消，cancel_observed_inflight=true、cancel_window_missed=false；唯一cancelled终态，远端处理未知。T1为0，没有任何重试/额外轮次。

总M/R/S/C/U=4/4/4/3/1，模拟S=0；四个request_id唯一，任务分配[2,2,2,3]，send_count_saved/known均true。T2 usage输入2135、输出190、合计2325；本地保守参考868500纳美元=USD0.0008685。T3未取得有效usage，estimated=5595600纳美元=USD0.0055956、fee_unknown=true；不按0计费，不声称服务端停止、免除费用或账户账单已核对。取消后validation_network_or_send_unknown为按合同停止原因，不为追求成功再请求。

自动导出后进程正常结束，退出码2（保留unknown诊断），stdout确认Service/workspace均destroyed。关后无WAL/SHM，以mode=ro&immutable=1核对新库：2任务、4消息、43顺序事件、2工具、2个唯一终态；数据库SHA256 C778B840A1396E1428C1D94E51474B44AC0F6FED5C920A0A9F25F10230EDE647，读前后相同。UTF-8数据正确，控制台代码页乱码不等于库内容损坏。没有恢复Service或操作用户其他数据库。

结论：启动准备整改与离线检查通过；T2真实只读闭环通过；T3真实在途取消及本地安全收尾已覆盖，远端结果和费用仍未核定。整轮按授权收口，不作无条件RP-C或整个系统完整验收通过结论。实际TLS成功仅覆盖本轮连接，代理/真实设备全面来源/IME/DPI/视觉/辅助技术/物理断电/长时仍按原延期。无独立复审或正式新快照冻结结论。

证据位于新根ready/launch.json、exit.json、session/conversation.sqlite.batch.json、closed-db-observation.json；旧根和本轮首次构建/准备失败日志保持。运行时DLL和rpc_memory.uapp与原已验证Light交付哈希一致；只重编启动器，不机械重跑未受影响Service/P2/P4/P5回归。新启动子进程检查3/3为当前新增结果，不与原Native/Light/ASAN合并成一次全过。
