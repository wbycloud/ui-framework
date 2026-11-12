# RP-C验证准备开发及离线验收记录

2026-10-31。本轮授权为准备开发、隔离离线测试和范围内整改，不含真实API/密钥读取/生产窗口启动/真实人工验收。不是独立复审或新冻结结论。

## 1. 基线与归属

实际分支codex/p1-process-boundaries，HEAD e0eca03db25930be55bdb45477027d1793a61904未提交。入口baseline.json保存497项当前范围文件（包含ignored根规划），不是HEAD到工作区差异或全历史树；入口副本在baseline-source。原冻结390项源码入口无漂移，原114项程序保留，以frozen-identity.json及最终preservation.json逐哈希核对。原491入口＋3事后补充仍是上一轮历史，不混为本轮基线。

新独立根`build/xuanshu-rpc-prep-20261031-c913e8`，旧根`build/xuanshu-rpab-closeout-20261031-a19c42`只读。参考工程D:/XuanShu仍Permission denied，未扩权、不修改。既有改动/未跟踪文件保留，不把P2—P6/原RP全部归入本轮。

## 2. 冻结合同及实际实现

[RPCP-1](../xuanshu-rpc-preparation-contract.md)先冻结后实现。新增独立ai_validation.h四个限制API及size/version=1结构，旧公开头/布局/错误/依赖/额度不变；不签发外发授权、Key或AI写入权限。enable只接受显式storage_path.model.json；后台保存rpc_open之后ready；propose只生成原计划三项合成任务、空历史、固定投影/目标，仍须原可信原生确认。旧real_propose不能绕过启用后的任务限制。

生产模块rpc_memory.uapp链接ui_framework.dll，唯一memory.read/document资源memory.document，真实内存回读value=10/revision=r1；原Schema、名称、说明一致，无写/事务/私有人工接纳。xuanshu_rpc_probe只创建新目录、显式DB/config、原生Key/Confirm按钮及同一workspace/Service，默认不执行任务。本轮仅编译，不启动该程序。窗口/控件在启动Service之前创建，初始化失败走安全关闭路径。

发送门位于唯一生产WinHttpSendRequest之前，旧WinHTTP路径不替换。manifest提交、配额reserved提交及send_intent后分别复查存储/期限/取消；全局5、分任务1/3/1、重复ID拒绝、失败不返还。COMMIT已成功而checkpoint失败仍消费已保存额度，但不发送。真实S在API返回后登记，调用在途计数未知，ack保存失败另报未确认。离线模拟S/C/U另列，实际S/C/U=0。采用原events/幂等op_id，无DDL迁移。

首次确认起10分钟停止新派发并取消在途；USD0.10本地阈值，以保守固定参考费率计算预留/已知，缺失usage不记0并停止；不是实时账单或服务端硬限制。用过的库重开恢复消费/费用/模拟计数后封闭，不重置剩余额度或恢复授权。策略固定arena计入原8MiB，无新线程/执行器，cleanup-only和原UNKNOWN门不改。

## 3. 新增离线覆盖

| 用例/路径 | 直接断言 |
|---|---|
| vc-quota | 同Service三任务共5次模拟发送、任务1/3/1，重复client幂等/新client拒绝；生产S=0；真实业务10/r1、预留被有效usage替换 |
| vc-limit/duplicate | T2第四manifest不可发送（R=3/M=4）；重复provider request ID只消费一次，不重复发送 |
| vc-unknown/fee | 缺usage标unknown并封闭；保守预留超费用阈值R=0/S=0，无余额查询 |
| vc-expiry/cancel | 受控时钟到10分钟0新发送；取消后destroy在网络在途PENDING，真实测试回调释放后终结，消费不退 |
| vc-store/checkpoint | 保存失败R=0；第32次提交配额COMMIT成功/checkpoint失败R=1；两者均0发送，关库后SQL确认保留事实 |
| vc-restart | 同库重开R=1、task_used=[1,0,0]、模拟S=1、已知42000纳美元；prior封闭，无授权/自动网络恢复 |
| vc-revision/parameters | 变更revision后外发确认拒绝；非法结构、缺目标及无NUL目标字段拒绝，0发送 |
| vc-after-send-store | 模拟发送后存储失败保持已消费R=1/费用未知，0费用确认记录，不续轮，可安全关闭 |
| ui_rpc_memory_production | 生产DLL公开发现/bind/prepare/dispatch/result/forget，实际10/r1及框架revision=r1；未配置/未确认0发送；同库重开封闭 |
| ui_rpc_policy_static | 静态运行时同一限制API/显式配置/0发送/同库重开封闭，没有第二执行器 |

隔离测试编译才有凭据/网络替身和私有确认；内存秘密哨兵不是真实Key。生产导出只增加4个限制API，没有fixture/barrier/probe授权导出。旧模型用例继续验证真实双实例workspace切换/重开、业务值、异步DLL worker/外部回调、背压、存储失败cleanup-only、网络回调及安全退出；不以计数替代业务结果。

## 4. 实际运行与首次失败

原RP Native新增2/2及相称61/61为原两次运行、Light42/42/ASAN52/52均为历史。下表为本轮新证据，不能覆盖历史首败：

| 本轮最终运行 | 结果 | JUnit |
|---|---:|---|
| Native final-native | 78/78通过，160.43秒 | native/final-native.xml |
| Light final-light | 57/57通过，96.67秒 | light/final-light.xml |
| ASAN final-asan | 首轮66通过、1 Not Run，97.29秒 | asan/final-asan.xml |
| ASAN asan-assistant | 补构建遗漏目标后1/1通过，0.13秒；逐项覆盖67/67，并非首次单次67全过 | asan/asan-assistant.xml |

每项测试沿用原20秒等CTest门槛，未提高资源/请求/数据范围或放宽断言。新13项vc、生产及静态夹具，加原Service/RP/C1/P4/P5/公共头/两DLL分支/Light投影的相称集合；非全面重跑历史测试。原Native早期12/12和自审16/16属于中间身份，不冒称最终源码验证。

首轮first-policy Native共12项，8通过/4失败：vc-quota 20.01秒超时；vc-limit、vc-unknown及生产夹具失败。前两项源自测试拼接重复usage，协议严格解析正确拒绝invalid_event_json；unknown样本沿用原usage而未实际缺失；生产夹具缺必需request_close/unmount回调，加载被正确拒绝。修正样本JSON构造和夹具生命周期，不改生产解析/门槛。first-native-build的LoadCursorW窄字串/WX错误、fixture-corrected-native-build的yyjson mutable删除API签名错误均单独保留日志，不伪装产品运行缺陷。ASAN首轮Assistant Not Run为本轮未列入build targets的编排遗漏，保留67项原JUnit/日志和当时源码/程序，补目标后仅跑该项，不覆盖原失败，也不称全部单次通过。

证据限制：最早构建脚本的源码扫描没有tools目录，因此首次启动器编译错误的完整tools同期源文件未自动归档，且该次尚无成功链接的启动器二进制；保留原编译命令/准确诊断，不用最终源码或后补材料冒充原捕获。随后扫描补tools。运行失败的框架/测试/夹具同期源文件、JUnit、原始日志和匹配程序在failures及各次sources/binaries清单核对；本轮未将有界失败称为崩溃、死锁或数据损坏。

自审另收口：失败任务停止后继任务；网络结束与有效usage分别持久化；重开保留计数并标实际发送不确定；实际S在WinHTTP返回后登记；目标字段有界验证；启动器窗口创建先于Service，失败不直接遗留已运行worker。没有改公开旧语义/来源政策/网络期限/生产额度。

## 5. 身份、保全与结论边界

以本轮delivery-files.json/incremental-source-diff.patch逐项归属，source-after.json/final-source、binary-files.json、red-binary-files.json、verification-summary.json、exports.json及delivery-identity.json关联最终源码/产物/测试。preservation核对入口文件、根规划历史前缀、原冻结清单及114程序；fixed-dependencies核对原依赖，不重下/升级。只删除新根无用途obj；程序/PDB、首败源码/程序/日志/JUnit及隔离诊断库保留，未触及用户库/Key/其他凭据或历史目录。

真实DeepSeek/TLS/代理/凭据窗口/物理输入仍未验，生产启动器未运行。后续操作见[真实服务清单](../xuanshu-rpc-real-service-checklist.md)，必须另行授权5次真实调用及用户Key/外发确认；不能将本轮离线替身或生产公开read回读称为真实服务兼容通过。

历史四组期限风险、frontend63、ASAN-Light、Light surface_input、browser20、workspace20.05秒（门槛20秒）及Native输入首败独立未定位，原RP-C本轮首轮20.01秒也保留；后续限定通过不定位历史根因，不归因环境。browser九副作用另案；真实设备/来源正例/IME/DPI/视觉/辅助技术/物理断电/长时延期不删除。

## 6. 最终核对及结论

三配置最终源码/程序与各自开测清单匹配；Native78/78、Light57/57，ASAN首轮66通过/1 Not Run并补该项1/1，逐项67/67。32份既有公开头及原固定依赖哈希不变，三个生产DLL都只增加enable/get_state/propose/stop四项限制导出，无私有测试入口。秘密哨兵对本轮原始日志/JUnit扫描0命中；仅为该范围证据，不外推任意内存绝无秘密。

最终397项源码清单SHA-256 `57CBE425DB9EBD50ADF0828196ADF5EE070F8A3CEA3EE1BC67577392E81F9F2A`；135项最终程序/链接及PDB产物清单`50CAB333701642F42F62C0EE37EA8EB29303E539407412F12A62AFEF34CD8CFC`。Light启动器`F294ABE50E1080AA2627338E7E376AC25F3C33917134B36D4AC8B30430DAEB2B`，生产DLL`68ABF009AFA1110832FD1DCB4377DA7A94C1C62BFF439D13FF3200088EBDC99A`，rpc_memory.uapp `E635C990E99A280819F6E91F9E78854E3D432433CFEAC3A68216075A18F70576`。完整长度/路径及其他配置身份以binary-files清单为准。交付记录只针对本轮准备增量，不转移原RP冻结结论。

原冻结114项产物及四份身份清单保持；本轮85项失败归档程序/链接及PDB保留，510个无用途obj已限定新根清理。隔离诊断数据保留供核对，不打开用户库。首个tools源文件未自动捕获的限制如第4节，不以新材料补冒原记录。源码、运行失败的匹配程序、原始JUnit及失败记录保留。

结论：**RP-C验证准备开发及离线验收通过**。尚非独立复审通过、正式冻结、真实DeepSeek或真实人工验收通过。停止准备交付与代码评审，不启动验证窗口、不录入Key、不执行5次真实请求。
