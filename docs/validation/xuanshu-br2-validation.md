# BR-2 验证专用发送保护开发验收

本轮是开发自审和离线验证，不是独立复审、冻结、真实Provider或人工验收。新根build/xuanshu-br2-20261105-9b7f14。BR1-KI-001仍未解决，按用户决定保留为非阻断遗留；本轮未重复定位。

## 基线与实际增量

原BR-1 identity=7C1FB0F647B8CB9F9270A00DF914B03427C8D9071FA7E12E93AC87892ADA5595；启动监督候选identity=276E55FC8ADFA39875E8F342300C04DBA8D7888BA0585411D8F75E7C463C4023。prior-identity.json核对原文件。入口源码范围399份，baseline-files/source-before不是全部Git内容快照。工作区原有大量修改，不归为本轮。参考工程只读可列目录，未改动。

新增ai_business_validation.h及ai_business_validation.inc，复用ai_validation账本/worker/网络门；宿主只读取显式UI_AI_BR2_POLICY并显示保护状态。改动Service打开时识别历史BR-2记录、real_propose收紧、原确认末次回调后BR-2状态检查及10分钟期限。旧结构、错误值、DDL、可信来源、C2/P4/P5和各预算未改变。原357导出保留，仅新增限制接口，私有探针未进生产。新DLL身份与旧DLL不同，不继承旧测试。

## 已有能力—实际缺口—证据

|已有能力|本轮缺口与改动|直接证据|
|---|---|---|
|唯一worker、初始/续轮、validation事件账本|普通业务任务没有3次验证预算；独立BR-2策略绑定一个任务和库，所有发送共用reserve/before_send|br2 quota/duplicate/read及持久化检查|
|逐任务可信确认与完整预览|不能借启用策略取得确认；保持原门，追加策略ready/stopped及固定run检查|未确认、隐藏/PENDING、回调重入/关闭，原buildgate/summary门|
|原10分钟验证时钟/费用状态|BR-2确认起10分钟；绑定价格及可靠input上界，unknown停止|deadline/inflight-expiry/fee/fee-edge/usage-over/unknown|
|原保存/取消/安全退出|预算失败不退；缺省启动参数不能漏掉旧库保护|storage/cancel/disconnect/http/ledger-corrupt/read恢复|
|真实浏览器read与保存|同策略下固定业务结果、两标签、重复提交、草稿隔离|read/host-policy，关后state.title/address/revision/policy_count逐值检查|
|启动监督8项|增加Proposed库绑定及第三环境变量恢复|startup-reuse；新增2个辅助进程检查独立记录|

## 硬门与费用语义

所有初始和后继请求在原worker内经过av_reserve，再经av_before_send；先占用并保存，才允许传输。业务最多3次；取消/失败/断流不返还，unknown停止后继。第四份候选manifest可能已保存，但第4次reservation/传输入口被拒绝，不能把model_calls/ordinal/manifest计为HTTP次数。存储故障例内存reserved=1、持久化reserved=0、传输0，记录失败不伪称已保存；Service不可用且重开锁停。

库绑定防止相同批准策略复制到其他路径；已存在BR-2事件时在READY前安装停止门，省略启动参数也不退回普通模式。打开错误/不完整记录不授权。独占数据库锁及单worker序列化发送；非UI线程启用拒绝，回调重复确认/重复请求ID不能增加额度，方法与任务引用覆盖退栈。

费用known来自绑定价格和完整usage，不是账户账单；estimated为下一轮保守上界预留；usage缺失/超界或远端未知为unknown而非0。USD0.10上界检查包含known与未结清estimated。费用预估恰好达到阈值允许第一轮，结算后剩余额度不足则拒绝下一轮。未核实价格、模型/协议或计费上界时当前Proposed入口锁停，不能直接真实调用。策略仅可信宿主初始化复制，页面/模型/工具不能扩大。

## 首次失败与自审整改

- first-Light：9/10。存储错误后的第一次关闭按宿主合同只展示错误并暂停退出；原夹具没有验证错误确认阶段。补实际状态轮询、保留store_failed后再次正常收尾，未清错/恢复派发。
- direct2-Light：9/10。新增关后SQL误用runs.run_id（真实列id）；修正连接字段，不改数据库或业务断言。
- gate-final-Light：17/20。重复提案夹具改变字段数组顺序，形成不同投影，保护按冲突拒绝并锁停；改为读取原任务plan的精确projection，再比较幂等任务身份。原失败没有额外发送。
- final-complete-Light：56/73。本轮新增目标边界检查误将browser.view判为UI_CAP_DOCUMENT；实际browser_capabilities.inc注册UI_CAP_INSTANCE_VIEW，导致17例合法提案被拒绝，0发送。按真实注册种类最小修复；字符串终止符、scope及库路径限制保留。
- 启动扩展夹具首次因PowerShell script作用域未触发预期异常，随后辅助Windows PowerShell默认ANSI读取UTF-8中文路径导致exit9。分别修正隔离驱动作用域与显式UTF8，不归因产品或未知环境；原结果、程序输出及辅助会话保留。未启动真实宿主。

早期closure Light72/72和ASAN72/72是库绑定收紧前程序，不替代最终版本。pre-path-binding归档明确为早期通过程序；其运行时源文件是去除后来新增行后重建、逐哈希匹配的副本，不冒充同期自动捕获。失败目录含阶段上下文程序，不将全部称为失败源码新链接程序。

## 证据边界与收尾

测试使用真实正式宿主页面、隔离测试DLL、模型/凭据替身及隔离可信事件；生产DLL+生产浏览器包公开read另测。二者不拼接成生产人工授权或真实设备来源。没有真实Provider、凭据读取或人工启动。关后本轮自有库只读，WAL存在时不忽略；历史用户库和人工库未操作。

旧30.08/30.06/30.11秒、复制error5、历史启动/frontend/输入/browser首败保留，不声称解释未知根因。真实费用unknown、483 Tokens差额及请求重建非抓包不扩审。已取消的临时控件测试不恢复。当前Proposed策略尚缺真实官方协议/价格与可靠计费上界证明，是禁止真实启动的条件，不用提示词或人工计数替代硬门。

最终运行、清单、身份及清理结果由本记录末尾与verification-summary/final-test-match/final-readback联合索引；final-readback不是新测试或独立验收。

## 最终相称验证与状态

最终源码对应release-check运行：Light 73/73（132.16秒），ASAN-Light 73/73（291.11秒），Native 23/23（27.33秒）；三次不同运行，不合并成首次单次通过。Light/ASAN各含21个BR-2直接用例；旧完整112/48页、两条50000字节ASCII/UTF-8、缓存中断/持久化、buildgate、RPC/RPCB受影响分支及真实浏览器生产read均通过。Native补公共C/C++静态/动态头、Service嵌入、在途回调、DLL生命周期和原保护分支。时间/断言/合法数据范围未放宽。

启动原8项只按身份和设计复用；新增库绑定/环境恢复辅助程序2/2通过（policy-launch-complete.log），不启动生产宿主。首次scope与编码夹具错误分别归档。最终生产358导出仅新增ui_ai_business_validation_enable，无移除/私有入口。

关后选取每个BR-2模式最新自有库，共42库，无WAL/SHM，immutable读前后哈希一致。Light和ASAN的read各1任务/2消息/22事件/1工具，工具title=BR-1 合成交接单：设备巡检、address=browser:start、policy_count为整数0、revision与固定目标逐值相同。quota各1任务/2消息/36事件/3工具，4份provider_request候选记录、3次rpc_reserved和3次模拟传输入口；没有第4次传输。此为隔离模型路径，不能称零模型路径或真实HTTP次数。storage的失败占用保持、0传输入口，关后没有伪造成功占用记录。

399份入口范围与tests/public_headers.cpp补充前态共同界定增量。该.cpp入口未复制内容，使用首轮失败归档副本并核对入口hash，见baseline-supplement；不冒充原入口自动快照。源码/程序/测试前清单匹配见final-test-match；原依赖未改，新编译依赖在清理前为VALID。只清理本轮CMake编译中间件，保留失败、匹配程序、自有库、日志/JUnit及必要诊断；清理数量以cleanup.json为准。

本轮必须修正项已清零；BR1-KI-001未解决、用户接受非阻断。真实价格/协议与计费上界尚未核实，因此当前真实调用仍被锁停；这不是已获真实使用授权。独立复审待进行，未冻结或发布。开发交付及集中授权材料已准备。

封装读回补充：最终dumpbin/Ninja只读取证首次受Python默认GBK解码影响报错，改为保留原始字节并显式UTF-8读配置后完成；不是编译或产品失败。原工具错误仅在会话输出，未事后冒充原始日志。辅助启动汇总的session字段有控制台编码损失，路径关联以sessions下原始result.json、PID和目录身份为准；原始result.json路径完整。辅助stderr含PowerShell模块准备进度CLIXML，不将其解释为产品异常。


## 2026-11-06 后续恢复保护整改索引

原独立复审为暂不通过，冻结阻断为开启标记缺失/误分类后的恢复保护降级。后续整改使用独立根build/xuanshu-br2-restore-20261106-3c8d42，见[恢复整改记录](xuanshu-br2-restore-validation.md)。本记录的73/73、73/73、23/23与原程序身份继续对应原交付，新结果另行索引。
