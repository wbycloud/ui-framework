# UHS-1显示缓存一致性限定整改验收

2026-11-04。本轮为开发、自审及离线验证；两项显示缓存限定整改已完成，自动验证通过，已具备再次限定独立复审的条件。证据根为`build/xuanshu-uhs-cache-20261104-4a9e72`，先冻结的增量设计见[缓存整改合同](../xuanshu-uhs-cache-closeout-contract.md)。自审不等于独立复审、冻结或人工验收。

## 实际基线和改动

入口HEAD为`e0eca03db25930be55bdb45477027d1793a61904`、分支`codex/p1-process-boundaries`。实际工作区已有大量修改及未跟踪文件，仅以本轮编辑前范围副本识别增量，不把HEAD全部差异归本轮。用户AGENTS适用，范围内没有另发现AGENTS；参考工程README可读取，未修改或扩大权限。

原交付`build/xuanshu-uha-summary-20261104-d7c518`的delivery-identity SHA256为`0AC73BF4F83938573AB6BA2BEF17E67176F48FA8CF2CF005D9553A5D8B19DFC7`。入口418份源码、53项产物、15项交付文件核对匹配；原结论为限定复审暂不通过，不能用旧通过测试代替新快照验收。新根source-before保存558份编辑前范围源码/文档，不是全部Git文件内容哈希。

产品仅改`examples/framework_host/host.html`四处局部缓存语句。测试只改`tests/uha_summary.c`并扩展CMake用例注册，另有合同、记录、交接、人工入口、阶段索引和根三规划追加。运行时、公开头、DDL、版本、错误值、来源政策、固定依赖均未修改；生产DLL本轮重新链接形成独立字节身份。

## 两项窗口与最小修复

详情：原A签名可在清除A的DOM、替换B期间仍有效，中断后返回A会跳过重建。现在先将realChunks和realPreview失效，再解析候选、清除及构建；全部实际块和总块数说明成功后才提交候选关联及签名。失败无需依赖catch清理。稳定成功内容仍保持节点，不重建焦点、选区、滚动、捕获或草稿。

摘要分页：原pageText先改待显示text，同内容后续会跳过尚未完成的控件写入。现在单一当前分页对象保留text/page及painted：文本变化、翻页和绘制开始前失效，实际文本与页码全部成功后才完成。同内容但未完成仍重绘。父级静态摘要先撤销key/preview，全部子字段完成后提交；缓存命中还检查四个子分页完成状态。动态任务、保存、错误字段始终独立更新，不被静态缓存跳过。没有按任务无限累积缓存。

缓存从不签发授权。原宿主run/hash/展示代次、全部实际文本、布局、最后一次回调、可信逐任务确认及PENDING解绑的独立拒绝全部保持。未提高50ms脚本、8MiB JS堆、128KiB模型、2MiB arena、8MiB Service、256KiB单包及原30秒用例门槛。

## 直接回归及证据性质

新增11例覆盖清除旧详情后/追加部分节点后中断并返回A或重试B；待显示赋值后/实际文本后中断；父级A→B部分摘要→A；动态取消保存状态；UTF-8翻页中断同内容恢复。直接读取实际控件文本、页码、每个详情块、块数及任务身份，未以缓存标志或控件数代替内容。

Service串行接纳等待任务，夹具顺序取得真实A/B提案投影；旧投影仅用于隔离显示重放，不恢复A授权。当前B的旧绑定在方法租约保持、解绑PENDING时仍拒绝；失败阶段检查所有模型run的confirmed、model_calls、tool_calls、ordinal、request_saved及隔离传输requests均零。普通宿主没有RPC额度策略，未虚构M/R/S/C/U。恢复权威当前投影后验证一次接纳、重复拒绝、草稿和选择保留、隔离请求及正常关闭。

翻页直接例保留两条各50000字节UTF-8历史，仅检查同任务翻页中断，不重复无关A/B任务替换；实际页文本、页码、完整详情和确认/关闭均保留。原long/utf8另逐页检查完整112/48页、模型请求逐值一致与原数据范围。故障点和可信事件只存在于隔离测试页及测试DLL，不能证明真实设备来源。

## 首败、夹具纠正和自审

| 运行 | 事实与处理 |
|---|---|
| red-cache | 九例前提失败：A等待确认时B排队，未取得B预览；不是产品缓存证据。改为顺序获取实际投影。 |
| red-sequential、red-scoped | 隔离处理器最初在IIFE外，element不可见；辅助Python引号SyntaxError后驱动仍跑了一次未改变程序。两轮日志/源码/程序保留。SyntaxError仅工具输出，此处追记不冒充原自动日志。 |
| red-injected | 有效修前10例5过5失败：详情返回A、待显示同内容、动态保存状态及翻页直接失败。均有界，不证明真实发送、用户崩溃或死锁。 |
| fixed-direct、expanded | 修复后10/10，扩充B两页及父级缓存后11/11；属于中间运行，不代替最终源码验证。 |
| final-closure Light | 早期45/45通过；其后仅隔离测试修订，最终结果另列。 |
| final-closure ASAN | 原34项通过、新11项异常终止。新增隔离onmessage包装叠加调用层，后续同程序诊断及fixture-diagnostic捕获RangeError: Maximum call stack size exceeded；原脚本栈限额未变。夹具初始投影失败后仍访问空目录结果造成空指针异常。增加前提失败的有界退出，将隔离分支插入原处理器而不额外包装；生产页未增加测试分支。 |
| fixture-stack | 上述修正后10/11通过；UTF-8翻页复合流程30.11秒超时。保留首超时，收敛掉与翻页无关的第二任务及多次大投影重放，保留原数据、断言和门槛。paging-isolated同任务直接例19.56秒通过；不据此声称已解释所有历史期限根因。 |

最终隔离页保留初始投影异常诊断及前提检查，避免初始失败被后续空访问遮蔽。没有扩大JS栈或预算、缩小历史、减少完整分页断言、放宽确认或超时。失败阶段目录包含同时存在的上下文程序，不能全部称为该失败源码新链接的程序。实际执行程序以原日志命令、test-binaries清单和归档对应。中间通过记录保留日志和源码清单；未另行留存的中间程序不冒称已归档。

## 最终验证

| 最终独立运行 | 结果 | 原始日志/JUnit |
|---|---|---|
| Light closure-complete | 45/45，90.73秒 | closure-complete-light-test.log、light/closure-complete.xml |
| ASAN-Light closure-complete | 45/45，228.06秒 | closure-complete-asan-light-test.log、asan-light/closure-complete.xml |
| Light正常用户上下文诊断复制 | 1/1 | final-copy-user-light-test.log、light/final-copy-user.xml |
| ASAN-Light正常用户上下文诊断复制 | 1/1 | final-copy-user-asan-light-test.log、asan-light/final-copy-user.xml |

上述四次运行的源码和实际程序均与最终清单核对，不用修改前通过替代。复制用例仅写入并读回测试自有脱敏快照，不读取原剪贴板或密码框。独立运行分别记录，不合并为首次单次全部通过。受影响范围为原34项加11项缓存直接例：完整预览、结构化范围/实际请求、双实例固定目标、真实read10/r1及框架revision/工具保存、存储cleanup-only、控制/限额、生命周期重入、显示门、重建、诊断历史及真实64槽、静态嵌入和公共C/C++头。复制诊断另次运行。

没有修改运行时，不机械重跑全部Native/P2—P6。既有Native等仅在未改源码和实际依赖所支持的范围引用，不计为本轮执行。ASAN仅证明本轮相应隔离程序及其包含路径，不冒充生产人工或全系统重新验收。

## 兼容、身份与保全

最终source-after/source-final、delivery-files/delivery-final、binary-files、每次源码和程序清单、原始日志/JUnit、失败对照、实际Ninja依赖、固定依赖、导出及脱敏扫描在独立新根。final-readback是封装后核对，不是测试或独立验收。交付identity不含自身，精确值在final-readback；工作区新文档与原离线交付文档副本区分。

生产宿主仅构建封装，未启动。未读取真实/环境Key、其他凭据或旧人工数据库，未调用Provider或运行人工。历史原目录只读核对清单及哈希；测试自有数据库保留。仅清理本轮成功构建目录的无用obj，保留失败材料、必要图像、PDB、匹配程序和测试数据；清理实际列表见cleanup.json。

## 停止位置与未验

范围内必须修正项已清零；两项显示缓存限定整改已完成，自动验证通过，已具备再次限定独立复审的条件。仍不自行宣称限定独立复审通过、冻结或真实人工验收。真实来源、Windows完整分类、IME、DPI、视觉、辅助技术、长时及专用环境断电均未执行，browser九副作用另案。

原ASAN30.08/30.06秒、候选绑定失败、复制error5、取证traceback未自动落盘，以及显隐20.02秒、历史启动/期限/frontend/输入/browser首败继续独立保留。没有用新通过覆盖首败或宣称全部未知根因已解决。已有Provider T3远端/费用unknown、483 Tokens差额和重建非抓包的边界不扩审、不追加请求。

## 匹配程序身份

下列为新构建身份，不继承旧生产DLL字节身份。

| 文件（相对新证据根） | SHA256 |
|---|---|
| light/framework_host.exe | F53CEA79FF0B42CAC6AF8990A86B8886DA0CABCD41DE9F864CC3CFA1FE3E0BD6 |
| light/ui_framework.dll | 1F4210520D19BE162710EFEF11CAA06BD5D17AFD36F42FD51BE62D8554C51978 |
| light/rpc_memory.uapp | 93FE8EB64AE8F0599BB0FF5BA7B35EFDAFB4D9A98E29B61E6250D4AB2D07E4D8 |
| light/ui_uha_summary_test.exe | 5F2A3EE7AF5E5F93D543551CDBD2270DAF0F00900FF50FA317235D9F7C1C9511 |
| asan-light/ui_uha_summary_test.exe | F4D7CF903F583A3458A3AA25860D81F161FA89009FEA2BEAF91BA094E329FA34 |

## 2026-11-04 UHS-1冻结与简短人工准备checkpoint

上一轮限定独立复审通过，匹配B5FD964B…ABDC23快照冻结记录已落盘，非本轮重测。详docs/validation/xuanshu-uhs-cache-independent-review.md。本轮新隔离根build/xuanshu-uhs-cache-manual-20261104-b19f62，仅一次正式宿主启动、offline-fixture本地未确认提案及摘要/详情/草稿/取消观察；零Key，不确认外发。人工结果待反馈，不能预填通过；首败及所有期限限制保留。

## 2026-11-04 UHS-1简短人工复核完成checkpoint

UHS-1简短零Key、零联网人工复核通过。中文范围定位、固定目标/白名单、详情2块折叠恢复、草稿完整可编辑且不覆盖原任务、脱敏复制及取消保存已获用户观察/截图；PID19548实际exit0。关后新库1任务/2消息/3事件/0工具，唯一cancelled/committed，无外发确认/模型请求保存，immutable哈希不变且无WAL/SHM。产品418源码/53产物匹配B5FD964B…ABDC23，冻结记录已落盘；人工根build/xuanshu-uhs-cache-manual-20261104-b19f62。非抓包、真实Provider或全部人工通过；历史限制及延期保持，停止不进入下一阶段。详docs/validation/xuanshu-uhs-cache-manual-validation.md。
