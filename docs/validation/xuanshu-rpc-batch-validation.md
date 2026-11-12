> 最终状态：RPCB-1离线自动测试通过；本轮真实尝试在启动阶段早退，未进入批次确认、凭据检查或T2/T3执行，未通过真实补测。没有追加启动或请求。后面的“尚未启动”均为带阶段含义的开发时记录，最终事实以下方收尾为准。

# RPCB-1批次验证交付与限定真实补测

2026-11-02。记录本轮开发、离线验证、自审与一轮真实结果，不作为独立复审或整个系统验收。真实批次尚未启动，以下仅为当前开发证据；后续结果在同一记录追记。

## 基线与归属

仓库HEAD e0eca03db25930be55bdb45477027d1793a61904，分支codex/p1-process-boundaries。新根build/xuanshu-rpc-batch-20261102-8ca4d2，入口414项范围基线（403源码及11文档/规划/上下文），不是全工作区或HEAD差异清单。保留所有既有修改、未跟踪文件、冻结SDK和历史材料。参考工程只读，不申请扩大权限；既有不可读范围不阻止独立工作。

旧RPC-K1离线identity 0CBE4D1CA7A43E6557F7783402E033D10B3FA8FB6B978324326D809FEC1D3317，运行D66845A10EE93E383E992639DCA3AAD855D68E280F39721743FF5FF81BAA1150；原T2/T3补测identity 3CE7ADF4C7D4CF284E7BA1DF18C5D295EC5E1E7F2ECBD245B0FFD9DC40C68F37。历史T2 page1失败与M1/R1/S1/C1/U0、工具0及USD0.0002562保持。新程序形成新身份，不继承旧独立验收或冻结。

## 修前与首次失败

- red-page：实际序列化工具定义未约束page0、说明缺失，两项有界断言失败，228项检查；同期开测清单、源码及red/native匹配程序保留。原page1真实失败没有重试或退款。
- batch-first：四项有界失败，其中诊断导出借用了SQLite字段名，语句finalize后字段名寿命结束；修复为池内复制。其余是测试received关联误用手工run变量及取消竞态假设，分别修正夹具。未宣称崩溃或数据损坏。
- review：夹具误用不存在的关闭枚举，编译失败；review-fixed：13项中12项仅因窄字符fopen无法打开中文路径的断言失败，改为UTF-8转宽字符_wfopen。产品导出当时已存在，不混成导出故障。
- strengthened：新增夹具局部变量遮蔽及不完整私有类型访问，编译失败；strengthened-fixed：夹具误用未导出私有hash函数，链接失败；改用原公开JCS指纹。
- bounded：15/16通过；bc-limit可存在两次合法read，新增证明比对错误地只记最后一个ID，1项断言失败。已改为逐个tool_call_id关联原工具结果及摘要，不放宽业务值或次数断言。

全部失败及匹配材料保留在新根原始log/JUnit和failures对应目录；编译失败不伪装成曾链接成功。

## 已有受控结果与待完成

association-fixed Native16/16通过，包含实际请求page enum0/非法page1拒绝、未确认零发送、伪造/重复确认、0/3/1及4门、后台凭据替身、配置/目标/期限变化、实际工具JSON和framework_revision关联、存储及checkpoint门、填满64项通知后排空、冷重开封闭、自动导出及安全关闭。内存替身与私有确认不导出生产DLL，不冒充真实设备或真实服务。跨构建回归、交付身份及真实结果后续据实补充。

历史期限、frontend、输入与browser首败不在本轮重新定位，不归因环境；browser九副作用及所有延期人工项目保持。

### 最后一次回调重入

新增bc-reentry在真实DLL目标revision回调中同步触发第二次批次接纳，修前6项有界断言失败（reentry-red），没有崩溃。最小修复在回调前取得accepting租约，重复接纳直接拒绝且不污染外层状态；回调后重新检查关闭/存储/策略，再单次持久接纳。修后acceptance 1/1通过，方法租约内destroy仍PENDING，外层关联和实际read证明保持。

Light首次closure构建因未指定既有OSMesa路径而缺少ui_ai_model_panel_test目标，属于本轮配置编排错误；保留日志和当时源码/产物，使用原固定.deps/mesa-24.3.4/x64/osmesa.dll重新配置，不替换或下载依赖，也不移除面板覆盖。

## 相称回归与最后自审边界

- Native closure96/96：原Service/存储/关闭、P2入口、Assistant、公共C/C++头、静态嵌入、真实DLL相关分支及RP路径；之后accepting修改仅影响新批次，另以最终批次17项复核，不写成首次单次113项通过。
- Light closure-final99/99；ASAN closure-final97/97：包含最终accepting核心、新增17项以及相称旧路径，原阈值/断言保持。
- 两项最终回归后，仅启动器“停止并安全退出”在批次open之前补同WM_CLOSE的request_close回退，三个启动器重新编译；该一行不链接进上述测试可执行文件或运行时。测试源码身份对照会如实显示此启动器差异，不伪装为完整GUI自动验收；实际UI正例由本轮用户一次开始覆盖，紧急按钮早期分支为源码自审及编译证据。

自审覆盖：回调前accepting及方法门、回调后关闭/配置/目录校验；网络、扫描和结果待消费引用沿旧合同；后台初始凭据检查与本轮固定ref；导出池计入8MiB、512KiB有界、复制SQLite字段名、CREATE_NEW不覆盖；停止后cleanup-only、连接回调真实退出、Service与workspace安全销毁。未改普通来源/授权、v1布局/错误、DDL、版本、依赖或额度。

上述为开发离线与自审，不是独立复审，也不证明真实DeepSeek/设备/凭据可用。实时取消可能导致usage或费用unknown，按既有合同停止并如实保留；不以此追加请求。

## 离线交付小结（启动真实批次之前）

最终Native batch-final17/17通过；Native closure96/96为本轮较早相称回归，接受租约修改后的受影响路径由17项复核。Light99/99、ASAN97/97通过，不合并成首次一次全部通过。生产Runtime只新增三个ui_ai_validation_batch_*，没有测试fixture、私有确认或传输导出；既有公开头与8项固定依赖哈希保持。

交付源408项，本轮增量23项，最终136项程序/链接/PDB，修前/失败146项归档产物，实际以delivery-files、source-after、binary-files、red-binary-files及清单哈希为准。清单生成脚本第一次因路径分隔符转义SyntaxError未执行；失败后修改前复制脚本并补录工具stderr，见evidence-errors，不伪装为原自动捕获。修正工具脚本不涉及产品，重新生成成功。

开发阶段模拟及自动验证通过；仅为自审，不等同独立复审。后续唯一真实轮仍以实际结果判断，不继承旧冻结快照验收。

本轮清理仅删除新根内548个已无用途obj，约None字节；日志/JUnit、源码、失败/最终程序和PDB、CMake重现配置均保留。未删除旧轮、凭据或数据库。

## 唯一真实尝试与停止（2026-11-02 16:42后，UTC+8）

离线匹配identity为341F240F5EB1775FEB8A7B539446327DFF71AE91C24EDD7F1251638EBA46EC58（offline-identity.json与原delivery-identity.json相同），源码408、最终程序/链接/PDB136、修前失败归档146，全部离线MD已另存offline-documents，最终源码存source-final。后续本节及交接/规划文档为运行证据增量，不自动算进已测试离线文档身份；产品源码及程序没有再改动。

08:41:37 UTC完成实际源码/程序/公开官方资料及费用核对；08:42:03 UTC只启动一次匹配Light/xuanshu_rpc_batch.exe，PID5916。参数为新real/session、独立validation-profile/model.json及同构建rpc_memory.uapp；不改LOCALAPPDATA、不触及旧库/config。启动函数返回后PID迅速不存在。只读检查确认session目录、数据库、model.json、credential-check和batch导出均不存在，stdout/stderr为空；未请求用户开始批次或输入Key，没有可信批次确认。

实际真实POST为0，T1/T2/T3均未接纳/执行。M/R/S/C/U没有数据库或自动导出计数记录，不能编造一份全0的持久化计数；未确认零发送的生产门及没有会话/任务为0 POST依据。无usage或费用核对过程，不把它写成fee_unknown=false或账户费用为0。指定凭据没有被助手读取/枚举/复制；软件本地读取阶段没有实际报告，也不伪造保存或读取确认。

当前根因未定位。入口在创建session之前有argc/GetFullPathNameW/CreateDirectoryW等提前返回分支，但没有捕获错误码或数字退出码，不能选择某一分支当作已证实原因。只读PE依赖核对未找到明确缺失的普通DLL；api-ms-win-*是API set契约，文件不存在不等于缺少依赖。精确本程序Application Error事件查询被拒绝，未扩大权限；没有据此断言无异常、归因环境、Key、TLS或DeepSeek服务。不得将此次早退宣称为已复现崩溃/数据损坏。

按真实阶段失败即停止的授权约束，不修实跑程序后续发、不重新启动、不换目录/ID或新建轮次追求通过。保留failure-programs下的exe/DLL/uapp匹配副本、同期source-final、launch/preflight、空日志及startup-observation；空输出本身亦保留。无在线数据库可读，无Service/worker关闭标记，因此本轮只能确认进程不存在，不能宣称真实自动安全收尾已验证。离线排空、存储关闭与destroy断言仍是其限定证据。

## 验收结论及最小后续范围

- 代码合同/page0修正及批次核心离线验证通过，自审发现的导出寿命和接纳重入问题已修正并回归；并非独立复审结论。
- 本轮T2/T3真实补测未通过，生产验证启动就绪、指定凭据本地检查、真实发现和10/r1、真实在途取消、生产自动导出/关闭均未覆盖。不能报告RP-C首次真实服务限定验证通过。
- 下一轮最小范围建议：仅验证启动早期分支和错误码、独立监督进程捕获退出码/阶段，再以不读取真实凭据、不联网的隔离子进程验证启动/就绪/关闭；实际原因明确并复核后另行确认真实补测。本轮不实现该建议，也不再次发送。
- 既有API/标准/ABI/包、33份既有公共头、错误数值、来源政策、固定8项依赖和限额未变；三个新限制入口仅验证专用。旧RPC-K1独立冻结与旧page1失败/额度保持，所有历史期限/frontend/输入/browser及延期人工项目不覆盖、不默认归因环境。

详real/preflight.json、launch.json、startup-observation.json、loader-import-observation.json、own-event-observation.json和最终交付/保全清单。原始失败、匹配程序及本轮目录均保留；只有启动前已清理的548个本轮无用途obj被删除，未删凭据、库、固定依赖或历史证据。
