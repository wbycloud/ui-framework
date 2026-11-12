# BR-1 保存消息与配置所有权对照

2026-11-05。证据目录build/xuanshu-br1-ownership-20261105-5b7d20。本轮按已授权方案定位；未找到原少字符根因，没有产品修复，不满足整改完成条件。

## 原材料与源码核对

原失败DB关闭后无WAL/SHM，immutable只读前后哈希一致。原模型任务plan.model为offline-fixtur、cancelled。其配置文件现存值是revision2/new-config-not-task，已经被原测试后段更新，不能用它证明第一次配置保存的值。仅提取合成模型名、版本和任务结果，不输出凭据引用。

Light的lw_dom_get对value调用JS_NewString；固定QuickJS quickjs.h:893传strlen到JS_NewStringLen，quickjs.c:4736对ASCII分配并memcpy完整长度，cutils.h:1077的ASCII分支保持buf_len。未发现ASCII长度主动减一规则。lw_js_post在JS字符串释放前同步调用宿主，main.c:221的read_string将每个普通字节复制至独立malloc并写NUL；on_web_message把已解析action排队，WndProc处理后才action_free。配置锁内复制pending_model，worker保存后复制model，任务再复制独立model及plan。没有为模型名执行trim或截断容错。

## 有因果屏障的四臂

设计先写experiment-plan.md。tests/br1_ownership.c使用正式宿主页、实际公开合成输入及真实HOST_ACTION_MESSAGE，网络和凭据不参与。仅将已真实产生的消息暂时移出队列，控制正常处理顺序，不伪造source、不调用确认。

1. plain：取出保存消息立即交由原WndProc派发，作为对照。
2. queued-edit：消息已经解析、宿主尚未处理，在此间将控件正常改成另一条完整合法字符串并投影，再处理原消息。
3. worker-edit：测试UI持有既有递归锁，正常派发配置动作；action按生产路径释放，worker暂不能取得锁。在此间修改控件，检查pending，然后释放锁才轮询worker。不持锁等待worker，无新增生产锁。
4. queued-rebuild：消息已解析后正常重建页面、重新关联节点并输入另一条完整合法字符串，然后处理原消息。

每臂建立一个未确认的纯文本本地任务，检查配置和任务model，取消并安全关闭。输出测试GUID、消息token、有效期内地址、长度、NUL及合成文本hex。没有人为删除字符或提交短模型名。任何比较失败立即走关闭，不重填追求通过。

## 结果与排除范围

最终Light final 4/4、ASAN final 4/4，串行、每例原30秒门槛；分别记录，不合并为首次单次全过。Light首轮4/4后，自审将dispatch保护从CHECK加fault判断改为guard失败立即退出，最终源码另编译核对；未声称发生产品故障。ASAN首次执行为最终程序。

所有臂queued-answer、配置读回和任务绑定均为15字节，hex为6f66666c696e652d6669787475726500。queued-edit/rebuild后独立action.answer不变；action释放后、worker尚未继续时pending仍完整，控件变更后pending仍完整。关后每个自有库一个模型任务，plan.model完整，cancelled；哈希只读核对不变。

这直接排除所测顺序下三项具体假设：①action借用可变DOM缓冲；②页面重建释放了排队answer；③worker借用已释放action.answer。它不证明原故障已修复，也不排除尚未捕获的其他写入。旧失败仍无法确定发生在页面读取前还是其后；不将正常对照外推为原输入必定已短。

本轮没有动态读取JS内部UTF-16长度；ASCII长度规则依据固定源码，C侧通过逐字节及NUL比较。既有同步MODEL-BOUNDARY日志和屏障会改变时序，不声称无扰动。没有取得异常边界，因而未再尝试无目标的全局数据断点，也未新增运行时探针。

## 状态与保全

只新增隔离所有权测试、CMake注册及旧测试主函数别名，不改变产品或运行时。生产源码/头/宿主与前轮匹配范围核对后复用导出与启动8项成果；新隔离程序身份另记，不冒用前轮二进制身份。未做无关全量业务回归，未启动真实请求或人工验收。

原失败及历轮材料保留；保留本轮程序、源码、日志/JUnit和自有库，清理仅限本轮可再生obj。当前仍有一个冻结阻断：原15→14异常的触发条件与责任写入未确定。没有证据支持产品修复，不具备整改完成后的独立复审条件。
