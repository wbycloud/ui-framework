# UH-A构建失败回调窗口限定独立复审与冻结追记

2026-11-03。本文追记本会话上一轮独立只读复审结论。本轮仅核对身份并记录冻结，没有重新构建、测试或开展完整独立复审。

## 结论与范围

承接结论：剩余冻结阻断窗口已闭环，UH-A限定验收通过，匹配交付快照可以冻结；必须修正项为空，具备冻结记录落盘及进入集中人工第一批零Key、零联网授权确认的条件。

限定范围为JSON构建失败后、撤销旧展示资格前应用回调可能重入确认的窗口，以及直接涉及的任务/方法引用、关闭和兼容路径。不重新全面复审P2—P6、原RP或RPCB。长历史/arena及持久化历史诊断/查询关联承接此前未发现阻断项的结论，不重新命名为本轮实现。

冻结是匹配快照的记录，不是commit、tag、release，也不是集中人工、真实Provider或整个系统完整验收通过。

## 匹配身份

匹配交付根build/xuanshu-uha-buildgate-20261103-b27e91；delivery-identity.json SHA256：B3351D1DB62743F5E5EFC67739976CB2A470B7D32C5855375B25B02A8C68007A。

本轮文档修改前核对19项身份关联、13项增量、416项源码及102项最终产物，无漂移。入口600份是编辑前范围快照，不是全部Git文件内容哈希。HEAD e0eca03db25930be55bdb45477027d1793a61904；已有大量未提交修改与未跟踪材料，不把HEAD差异全部归为UH-A。25项失败阶段产物不能全部称为失败源码新链接的程序。

| 正式程序 | SHA256 |
| --- | --- |
| light/framework_host.exe | C282C6428735C65B35612AA24AF74D4079B0B1E52015ADC68330781E56E47434 |
| light/ui_framework.dll | 888A80D5847214615AB48DB1DFF89DABF270D7092253BEC886256E2AB5EC3FD8 |
| light/rpc_memory.uapp | CA51E958D909AF7C6AB4C4C8890FEC0159014418A3C2DE9A93ADB4626D06EBD7 |

完整源码/程序及原始日志身份仍以原根source-after、binary-files、raw-evidence-files和delivery-identity为准，原根不更新。文档修改前副本、preflight核对及本轮新状态保存于build/xuanshu-uha-manual-first-20261103-5d8a41；更新后的文档哈希不冒充原离线测试时身份。

## 闭环依据及已有证据

send_state在外层JSON已failed时撤销权威展示资格、跳过AI投影；ai_panel_json在自身前缀failed时释放run副本、撤销资格并返回，均早于完整预览/revision回调。失败点后的菜单遍历只调用宿主序列化回调，dispatch idle仅投递唤醒；命令状态/标题/布局不进入应用业务回调。确认门仍绑定run、完整摘要、展示代次和实际文本；PENDING解绑不恢复旧资格，末次应用回调后的条件复查保持。

修前三例共12项有界断言失败，隔离可信事件夹具接纳旧确认；不外推为真实用户越界外发、真实发送、崩溃或死锁。修后直接回归验证失败投影不进入revision、后续只读查询中PENDING解绑仍拒绝、model_calls/ordinal/request_saved/隔离传输均0；不虚构普通宿主没有配置的RPC策略M/R。恢复完整展示后仅确认一次，关闭回调在CLOSED/worker退出后destroy仍因外层方法在途PENDING，退栈后引用归还并安全关闭。

已有Light closure26/26、复制另次1/1；ASAN-Light closure26/26、复制另次1/1，是不同运行，不合并为首次单次通过。旧Native40/40仅在源码、实际程序及依赖匹配范围复用；Ninja STALE来自obj清理，不是本轮新编译/测试。运行时/公开头/错误/DDL/来源/依赖/预算/限额未改，生产无私有探针导出。自审、离线测试、独立只读复审和真实人工观察分别成立，不互相替代。

## 非阻断限制与后续

必须修正项为空。保留原ASAN缓存30.08秒首超时：未完全定位，不默认归因环境，不由后续通过覆盖。ninja首次路径错误有会话/文档说明，未见单独原始stderr文件；后续只读依赖记录及匹配程序存在，这项取证限制保持。const夹具编译错误、产品修前红例、fixed-direct过早检查cleanup引用和随后有界状态屏障分别保留。

真实设备来源、Windows动态正例、IME、DPI、视觉、辅助技术、人工长时尚未验；物理断电仅专用环境后续方案；browser九副作用另案。已有真实T1/T2、T3本地取消/安全退出与远端/费用unknown、483 Tokens差额和重建非抓包边界不变，不追加请求。

本轮另获集中人工第一批零Key、零联网授权；操作过程及结果另记[xuanshu-uha-manual-first-validation.md](xuanshu-uha-manual-first-validation.md)。冻结追记不预先判定人工通过。
