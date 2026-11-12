# RP-C验证准备限定评审整改记录

> 2026-11-02追记：上一轮限定独立复审已确认整改闭环、准备限定验收通过、匹配快照可以冻结。本轮仅核对身份并落盘[复审记录](xuanshu-rpc-preparation-independent-review.md)，没有重测或重新完整复审；原下文“等待复审”保留历史。真实验证获本轮独立授权，结果另记[真实服务记录](xuanshu-rpc-real-service-validation.md)，不改变原离线证据。

> 本轮真实尝试的实际停止点：T1在SendRequest前credential_unavailable失败，M1/R1/S0/C0/U0；正常关闭及失败持久化已核对，T2/T3未执行，不重试。准备限定冻结与真实服务未通过分别记录；不将本次失败冒充厂商拒绝或据此覆盖历史离线结果。

2026-10-31。范围仅为client ID校验顺序及10/r1精确断言、必要离线回归。不是新功能、独立复审通过或快照冻结；不授权真实API、真实Key、生产验证启动器或人工验收。

## 1. 入口与原交付

实际分支codex/p1-process-boundaries，HEAD e0eca03db25930be55bdb45477027d1793a61904未提交。本轮新根`build/xuanshu-rpc-prep-closeout-20261031-f27a64`；baseline.json保存508项当前范围源码/文档/ignored规划及tools，baseline-source保留入口副本，不是HEAD到工作区全差异或全部历史文件。

原准备根`build/xuanshu-rpc-prep-20261031-c913e8`入口397源码、24交付、135最终产物及85失败归档逐哈希匹配，原delivery identity为`588D0C31595EDD1F981116F89F100867BA1FCF89AE8B1C7294EB1864A4606F4F`。本轮不修改旧根/脚本/清单/日志，original-identity.json记录入口身份。原开发阶段离线通过、随后独立复审暂不通过分别保留；新修复不自动继承冻结或独立验收。

参考工程README可读，未扩大权限或修改参考工程；Git目录访问限制独立记录。保留框架所有原修改/未跟踪文件及冻结SDK。规划只追加，不覆盖历史主线。

## 2. 源码核实与最小修复

外层ui_ai_validation_propose在调用旧real_propose期间保持admitting；ai_submit_impl同步通知received observer，外层尚未清零。原am_propose先进入validation准入strcmp，再由下层ai_submit_impl检查client ID，NULL可先解引用，其他非法ID则返回错误的准入拒绝码。

产品实现只修改src/ai_model.inc初始校验的一处条件，加入`!ai_id(q->client_request_id)`，置于所有client比较前。NULL、空串、65字符及非法字符明确INVALID_ARGUMENT；合法不匹配ID仍PERMISSION_DENIED。不禁止回调重入、不关闭旧接口、不更改任何结构或错误数值。未启用validation的旧入口也先执行相同既有ID规则。

tests/ai_validation_closeout.inc新增三组vr用例，注册到原ui_ai_model_test，CTest每项独立进程且原20秒门槛保持。received callback分别重入propose/continue；另覆盖三类非法ID的两个旧接口与合法不匹配ID。校验发生于admitting及callback/method租约仍在途期间。测试短临界区固定引用计数快照，不跨网络/应用业务回调等待worker，不引入生产锁或授权入口。

tests/rpc_memory_contract.c复用公开ui_cap_json_validate及RFC8785规范化，校验整数/字符串类型并完整匹配`{"revision":"r1","value":10}`；同时拒绝100、字符串“10”和r10。实际生产read的FINISHED/OK及框架revision=r1断言保留，随后forget。没有改生产夹具、业务值、Schema、依赖或限额；静态分支也验证精确比较谓词。

## 3. 修前受控证据

在产品修复前先构建red/native。SetErrorMode仅在专用测试进程内避免系统错误对话框；不终止或操作用户进程，不启用真实传输。red-controlled的3项首次运行：

| 用例 | 修前事实 | 边界 |
|---|---|---|
| vr-null-propose | 独立CTest子进程SEGFAULT，日志最后为received内NULL比较前标记 | 当前已动态核实该隔离路径，不扩大为用户宿主曾崩溃或数据损坏 |
| vr-null-continue | 独立CTest子进程SEGFAULT，同样保留前置标记 | 不冒称永久死锁或其他历史阶段故障 |
| vr-invalid | 6项status==expected断言失败；合法不匹配的两个调用正确拒绝，外层最终可关闭 | 错误码合同问题与NULL异常分别记录 |

本轮此前唯一编译失败red-native-build为新增夹具methods有符号/无符号比较C4389/WX，修正测试局部类型与私有结构一致后构建成功，不弱化告警。这不是产品运行缺陷。红测源码/原始日志/JUnit、完整red构建及匹配程序保留；此后不再重建red/native，不以最终DLL覆盖修前程序。

## 4. 修后直接断言与相称回归

首批fixed-controlled Native5/5通过（8.30秒）：三组重入＋生产夹具＋静态夹具。两个NULL及六个其他非法ID明确返回INVALID_ARGUMENT；合法不匹配ID仍拒绝；callback内方法/任务引用恢复原值，在途destroy=PENDING，外层返回后methods=0。received关联先于返回且只一次，未确认requests=0/R=0；重复提交返回同run，不重复通知或发送。随后原文本和同步只读发现/读取正常完成，共4次模拟模型请求，实际值10/r1；库中2个run/2个唯一终态/1次memory.read，正常关闭完成。外部真实发送始终0。

后续相称回归覆盖原RP目录/文本/read/重复/非法工具、取消与异步DLL、网络回调和cleanup、缓存扫描、目标/revision/epoch、checkpoint/存储、配额费用/到期/重开、Service回调/关闭、公共C/C++头、静态嵌入、两个真实DLL工作区编译分支、生产导出及Light面板。只补受影响路径，不重跑无关P4/P5/历史全量。

| 本轮运行 | 结果 | JUnit |
|---|---|---|
| native/fixed-controlled | 5/5通过 | native/fixed-controlled.xml |
| native/matching | 44/44通过，101.10秒；与5/5是两次运行，不称首次单次49全过 | native/matching.xml |
| light/matching | 51/51通过，76.97秒 | light/matching.xml |
| asan/matching | 49/49通过，74.68秒 | asan/matching.xml |

原Native78/78、Light57/57、ASAN首轮66通过/1 Not Run及后补1/1仅为历史；不将两次ASAN写成首次单次67全过。本轮首次红测与夹具编译失败不被后续通过覆盖。原最早脚本未捕获tools同期源、当次无成功启动器程序的限制仍保留，不用本轮新归档冒充原捕获。

## 5. 兼容、自审与保全

SDK0.10-dev/API10/标准10、ABI1/包1、全部既有公开头/布局、v1/C2/P4/P5、错误数值、固定yyjson/SQLite/JCS、原预算/超时/来源保持。5次及1/3/1、USD0.10非硬费用保证、10分钟和网络10/30/30/120合同完全不变。没有新增公开接口、执行器、来源证明、Provider或业务功能。

新根delivery-files和incremental-source-diff按入口区分；source-after/final-source、binary-files/red-binary-files、verification-summary关联最终源码/开测输入/程序/JUnit；failure-source/binary-comparison复核失败同期副本。生产导出应与原准备完全相同，私有夹具仅测试构建。sentinel-scan仅对本轮原始日志/JUnit及内存替身哨兵，不外推真实Key或任意内存安全。

本轮只允许清理新根无用途obj；red源码/程序、失败日志/JUnit/PDB、匹配最终程序及必要隔离临时库保留，不删除用户文件、历史材料或旧根。原目录清单和程序在preservation逐项核对，规划历史前缀保持。

历史期限/frontend/输入/browser首败仍独立未定位，不默认归因环境，不以本轮通过解释原根因。真实DeepSeek/TLS/代理/凭据界面/物理来源/IME/DPI/视觉/辅助技术/断电/长时仍未验；browser九副作用另案，不属于本轮漏实现。

## 6. 停止位置

两项限定整改及相称离线回归已完成，具备再次开展限定独立复审的条件；最终哈希/导出/保全核对见本轮清单。本轮不自行给出独立复审通过或正式冻结。后续应先限定独立复审，再另行确认真实调用授权。不得启动生产验证程序、录入Key或执行5次真实请求。

## 7. 最终交付身份与证据核对

本轮13项增量：5项代码/测试/构建（产品实现仅am_propose条件一处）、8项文档/规划。399项源码清单在原范围上纳入新测试include及既有tools/uapp_pack.c的盘点，不代表新增两个产品模块；SHA-256 `A19AB8168D9AC8DD73A6987F8BFD4E9FC7A03FF143B7DF5FE93FEE813FDD1C74`。122项最终程序/链接/PDB清单SHA-256 `90A253ACF45A54E7A739995F197A69E0A29EC164A2B2346D16DB4DB6FBDD12FA`。新delivery-identity关联清单和原始日志/JUnit，最终源码与四次开测输入/程序均匹配。

Light候选启动器SHA-256 `77B272DD1F008C3B5CA265997387E40391DF62E405D1773CD0DC0EBB2F8838FB`；生产DLL `C672F126E693E69F1959B45418141C5314EB83154F34D46D7DDF545FD9E4F35C`；rpc_memory.uapp `4EE210EAA0E7651067713F30B6D6A2D5FACB99EF12B88EBC4E5E81586B6DF61A`。这里只记录匹配候选，不启动程序或代替独立验收。

33份既有公开头、8项固定依赖逐哈希保持；三个生产DLL相对原准备无新增/删除导出、无fixture/barrier/probe私有入口。原135最终及85失败归档保持，508项入口中497未改/无缺失，规划3份历史前缀保持。本轮27项修前/失败归档产物保留，9项红测直接程序与开测哈希匹配；两份失败源码清单各399项全部匹配。日志/JUnit哨兵扫描0命中。仅清理新根539个无用途obj；必要隔离诊断数据和全部原材料保留。

收尾脚本首次因自引用残留旧文件名失败，发生于身份输出阶段，测试/哈希断言已完成，不是产品运行失败。初次仅有工具输出；错脚本另存evidence-pipeline-failure，原样受控复现日志明确命名finalize-failure-reproduction.log，不冒充首次自动捕获；修正后生成清单，详evidence-pipeline-failure.json及finalize-corrected.log。未因此重新构建或运行已通过测试。

结论为本轮限定整改及离线回归完成、可提交再次限定独立复审；尚不宣布独立验收通过、正式冻结或真实DeepSeek验证通过。
