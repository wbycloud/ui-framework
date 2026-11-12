# UH-A三项限定整改开发验收记录

2026-11-03。状态：三项限定整改开发、自审及离线自动验证完成，等待再次限定独立复审。本记录不是独立复审、冻结、真实服务或人工验收。

## 1. 实际基线与归属

仓库HEAD e0eca03db25930be55bdb45477027d1793a61904，分支codex/p1-process-boundaries，已有大量修改及未跟踪材料。本轮只以新根build/xuanshu-uha-closeout-20261103-a6f2d8的baseline/source-before为增量基线，不把HEAD至工作区全部差异算作整改。

开始编辑前复制并哈希596份当前源码/文档，Git全量untracked状态另存git-before.txt；这不是全部Git文件内容哈希。原UH-A delivery-identity.json SHA256为6FCD868AC88EDED61E2E866371DE346419C35DCC0402648AF0DEB33DFFBD5861，原413份源码与本轮before逐项相同，原93项程序哈希匹配。原17534顺序捕获与实施交错、四文件不能称为修改前自动捕获等限制保持，没有以本轮新快照回填。参考README可读，参考工程不写入、不扩权，也不扩大历史复审。

承接上轮独立复审“UH-A两项阻断、暂不通过”，本轮新代码不继承原通过结果。RPCB既有冻结仅作前置身份，不重新全面审查P2—P6。交付源码、匹配程序、原始日志/XML、增量diff、导出、依赖、保全及清理均见新根对应清单；源清单与文档清单分别计算，不伪造单一原子Git快照。

## 2. 三项源码闭环

| 整改 | 修前直接证据 | 最小修复和实际断言 |
| --- | --- | --- |
| 同摘要投影失败使旧确认失效 | red-controlled：post用例两项有界断言失败；methods在途令解绑PENDING时旧确认返回OK | 独立只收紧display binding：run/hash/展示代次；接纳前及最后一次目标/目录回调后读实际展示资格；新投影、失败、关闭立即失效。PENDING解绑不解除检查。恢复完整显示正常确认，重复拒绝，零确认/发送负例 |
| 合法长历史超过单包 | 两条各50000字节ASCII、原模型预览合法，单个宿主包320920字节；三项有界断言失败 | 原Light256KiB、文本4096、模型128KiB、arena2MiB/Service8MiB均保持。去掉宿主run重复context_preview；状态SHA256/代次/总量/严格序号、每片8192字节UTF-8完整边界、单个有界组装。全部实际预览文本读回后才绑定。ASCII及汉字/emoji长历史与实际请求逐值相等并完成；历史选择标签标明摘要，原消息历史可分页查看全文 |
| 历史任务复制task:null | red-stored：真实65任务填充64槽，首任务回收后已显示异步快照，复制仍null，一项断言失败 | 内存优先，否则验证异步快照来源/run_id；memory/persisted/pending/error/unavailable明确。worker只读查询追加唯一终态真实error及error_recorded；UI无新SQL。选择生成唯一查询ID，同时匹配run；迟到成功/错误不能覆盖当前任务。复制保留此前错误、不授权、不联网 |

完整增量规格见[xuanshu-uha-projection-closeout-contract.md](../xuanshu-uha-projection-closeout-contract.md)。新ui_ai_model_display_binding_t独立size/version=1，原绑定结构不改。限制回调不能生成可信人工资格，普通逐任务确认不改；RPCB批次授权/ref不推广。Service methods及run引用覆盖检查回调、目标回调和外层退栈，无锁跨宿主回调；destroy/unbind在途PENDING。原UNKNOWN、cleanup-only和安全卸载保持。

长历史执行补强还暴露固定arena内临时解析池/写缓冲重叠导致request_limit。保留allocation-diagnostic及scratch-lifetime失败：先观察body缺失，释放已复制的解析池后body成功但stream不足；最终按实际长度收缩保留序列化容量后完整执行通过。未增预算、缩历史或换依赖；响应/请求内容不变。临时诊断在最终代码移除。

## 3. 新用例与证据边界

| 用例 | 直接覆盖 |
| --- | --- |
| ui_uha_closeout_post | 同摘要后续post失败、methods在途/PENDING解绑、旧代次、缺片/重复/乱序/迟到、稳定刷新、恢复一次接纳及重复拒绝 |
| ui_uha_closeout_long | 两条50000字节ASCII从真实Service任务持久化、明确选取sequence；每包限额、完整4项input、实际请求逐值一致、重建和完成 |
| ui_uha_closeout_utf8 | 同规模UTF-8汉字/emoji分片、读回、实际请求逐值及安全关闭 |
| ui_uha_closeout_stored | 原64槽不缩减，65次真实提交令首任务回收；内存不存在时持久化来源、终态/保存确认；任务切换、迟到/错误、身份不符 |
| ui_uha_closeout_restart | 安全关闭后同隔离库新Service仅恢复记录；异步诊断、无联网；A→B→A旧查询拒绝 |
| ui_uha_closeout_failed | 真正stub_disconnect失败终态重开；读取真实错误、保存确认，非虚构空error |
| ui_uha_closeout_reentry | 最后一次revision回调中投影失败、旧解绑PENDING；回调请求关闭/destroy=PENDING；零发送及退栈后安全关闭 |
| ui_uha_closeout_faults | 隔离翻译单元的生成/投影准备/传输/实际读回失败分支；旧资格拒绝、恢复和正常确认 |

故障探针宏仅隔离测试翻译单元启用；生产宏编译为0，生产DLL无fixture/probe导出。模拟点击证明拒绝；接纳正例使用既有隔离私有入口，不冒充设备/来源正例。生产链接rpc_memory和静态合同分别验证整数10、字符串r1、framework revision、只读及精确JSON；真实workspace双实例、统一read和保存证据继续在相称用例中验证，不能仅以计数/控件作为业务成功。

## 4. 最终运行分账

| 编译分支/运行 | 结果与原始证据 |
| --- | --- |
| Native相称 | final.xml 40/40：公共C/C++、Service/控制/存储/限额、continue/静态嵌入、Provider文本/工具/错误/取消/重启/回调、批次相关；源码及二进制匹配 |
| Light初版联合 | final-joint.xml 57/57，随后query-final.xml 17/17及诊断各1/1为不同运行，不合并首次通过 |
| 最终Light closure | closure.xml 59/59；closure-clipboard.xml另1/1。包含最终重链接DLL、8新增用例、生产memory和静态合同及原面板/真实DLL相关分支 |
| ASAN初版联合 | final.xml 57/57；query-final.xml 16通过/1超时，首败保留 |
| 最终ASAN closure | closure.xml 59/59；closure-clipboard.xml另1/1。缓存填充阶段6500ms，完整用例仍原30秒门槛，无ASAN报告 |

每次命令执行前自动sources及test-binaries清单；源码未与构建/测试并行编辑。最终源码副本、PE实际导出及最终测试使用的程序逐项哈希核对。ASAN仅证明本轮编译/执行路径，不外推历史全覆盖或无限稳定。

## 5. 首败、自审及证据限制

- 原实现三场景的2＋3＋1断言失败如第2节；不是实际越界外发、用户进程崩溃、永久死锁或数据损坏。
- 初始准备脚本默认GBK读取UTF-8失败；证据采集辅助脚本首稿嵌套三引号SyntaxError，未形成执行文件/程序，原工具输出在会话，补充说明不冒充自动原日志。收尾采集脚本读取含中文路径的UTF-8 cleanup也曾误用默认GBK，原错误日志保留，明确编码后再收口；与产品缺陷分账。
- 新am_accept参数遗漏批次调用点造成编译失败；首次替换未命中，第二次按实际调用修正。移除临时诊断时遗留endif造成编译失败，保留源码/构建日志；普通编译不改变告警或合同。
- strengthened分片失败后的恢复测试未安排新刷新；faults探针误走合法稳定repaint而不是新的投影准备，均修夹具屏障/注入位置，不改变生产断言、来源、上限或超时。
- 自审A→B→A旧A异步结果仍可能结束新查询：red-query-guard一个直接断言失败，随后唯一query操作ID修复并验证。不是仅凭推测增加限制。
- ASAN query-final缓存用例首次30.08秒超时，没有异常栈或具体阶段输出，不能定位为崩溃/永久死锁，也不默认归因环境。之前同场景23.88秒通过。后续夹具在65任务填充时仅推进Service而不重复渲染无关UI，填充后再完整投影；保留真实64槽/回收/任务状态/迟到/关闭断言与30秒门槛，增加阶段耗时输出。此编排改进和同门槛通过不抹去首次期限风险，不冒称完全定位其根因。
- build失败归档可能含前一成功链接的输出，不能称为当次未编译成功源码对应程序；实际运行失败则保留命令源清单、日志/JUnit和使用的DLL/exe。pre-query-guard-passing明确是已通过程序，不称作失败材料。
- 原UH-Atools捕获/基线交错、受限剪贴板error5、历史ASAN启动依赖等原限制仍保留；新正常上下文复制只读自身成功写入的数据，不读取用户原剪贴板或凭据。

## 6. 边界、交付和下一步

SDK0.10-dev/API10/标准10、ABI1/包1，旧公开头/布局/错误值、v1、C2/P4/P5、JCS/SHA256、依赖、限额、超时不放宽。无DDL、真实凭据/Provider、用户原库操作或真实人工验收。新根保留原始首败、同期源码、程序/PDB、测试自有隔离库及原始XML；仅删除新根无用.obj，详cleanup.json。历史源/程序/证据及根三规划前缀保全。

原历史期限/frontend/输入/browser/ASAN-Light首次失败独立，不用本轮通过解释或覆盖未知根因；browser九项副作用另案。旧真实T1/T2、T3本地取消/远端费用unknown、483 Tokens差额和请求重建非抓包保持。真实来源、Windows动态正例、IME/DPI/视觉/辅助技术/人工长时继续未验，物理断电仅专用环境方案。

本轮停在整改后代码评审。匹配候选程序和后续人工步骤见[人工入口](../xuanshu-uha-manual-acceptance.md)，须先再次限定独立复审及用户授权，不自动启动、录Key、联网或冻结。

## 7. 最终结论及交付索引

三项限定整改已完成，自动验证通过，已具备再次限定独立复审的条件。自审必须修正项清零；ASAN首次30.08秒有界超时仍单列为未完全定位的测试期限风险，不因夹具优化后的通过抹去。没有改变生产时间/额度/历史范围或断言；最终程序均有匹配源码与独立原始运行证据。Native40/40、最终Light59/59＋复制1/1、最终ASAN59/59＋复制1/1为分别运行，不能写为首次单次全部通过。

交付根为build/xuanshu-uha-closeout-20261103-a6f2d8。主要清单delivery-identity.json、delivery-files.json、source-after.json、binary-files.json、red-binary-files.json；修前在failures/red-controlled-light-test、failures/red-stored-light-test，自审查询首败在failures/red-query-guard-light-test，ASAN首超时在failures/query-final-asan-light-test。完整所有阶段以verification-summary/raw-evidence-files为准。

候选正式程序light/framework_host.exe、light/ui_framework.dll、light/rpc_memory.uapp，禁止用fixture DLL/uapp冒充生产来源验证。新增仅一个生产限制性导出ui_ai_model_bind_light_display，旧公开布局/导出保持，测试宏/私有入口无生产导出。原413源码/93程序身份及冻结SDK/固定依赖保全。新文档哈希不冒充原UH-A测试身份。

后续只建议对本轮三项整改及必要回归开展限定独立只读复审；本轮不自行冻结、不启动集中人工验收，不申请实际Key或真实请求。

最终清单核对：本轮23项增量（13项源码/构建/测试，10项文档/规划）、415项最终源码输入、150项最终程序/链接/PDB，191项失败阶段归档产物；后者含早先已成功链接输出，不全部称为失败程序。另17项pre-query-guard-passing明确为通过程序。保全596项入口范围无丢失或授权外改变、根三规划原前缀完整；固定依赖和旧公开头保持。清理仅本轮715个无用.obj，共251486708字节，必要程序、失败、原始日志/XML及1258项测试数据/配置材料保留；两种秘密哨兵运行数据扫描零命中。

主要候选程序SHA256（精确清单以binary-files.json为准）：

- framework_host.exe：4C64143A3FE1CE7E2132B7224EBF78426BE4C998A5E0DAA9AEDB3B6794C9BE12
- ui_framework.dll：D06642AD10BC1A1F0E208268C1A6C0976EC0047331C7D195C462B41524A9418B
- rpc_memory.uapp：0F1A3AE86A120A3697BB81EF5310D1D944D8FE093653BBE6AAB19CD727A7D871


## 2026-11-03后续限定复审与剩余窗口整改

承接后续限定复审：长历史/arena、持久化诊断/查询关联未发现阻断；JSON已失败但后续应用回调仍可重入旧确认，冻结结论仍暂不通过。原三项交付身份与原首败保持，不回写为独立通过。

该剩余窗口在新根build/xuanshu-uha-buildgate-20261103-b27e91完成最小宿主修复及离线验证，详[xuanshu-uha-buildgate-validation.md](xuanshu-uha-buildgate-validation.md)。新源/程序另立身份，等待再次限定复审；本追加不是重新测试本旧快照或冻结记录。
