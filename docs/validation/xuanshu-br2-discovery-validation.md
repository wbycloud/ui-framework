> 2026-11-10追记：上述匹配交付已有限定独立复审通过，冻结记录见[独立复审记录](xuanshu-br2-discovery-independent-review.md)。本次追记不是新测试或重新完整复审，原交付副本保持。真实重验独立准备且尚未批准，旧真实结果不改。

# BR-2目录分类契约与直接相关缩容整改验收

2026-11-10。开发、自审及离线验证完成，等待限定独立复审；未独立通过、冻结或执行新的真实请求。[实施前规格](../xuanshu-br2-discovery-contract.md)及其空目录缩容补充明确本轮边界。

## 基线、增量和新身份

前置为计量冻结快照`build/xuanshu-br2-meter-20261106-1f8148`，identity SHA256：`C5A430D7DA479E7706043741FE11D66EE95B976C51B849ADA92A26732BF62596`。原快照、原脚本、历史失败、数据库及旁文件保持；当前工作区原有修改未归为本轮。

新候选：[build/xuanshu-br2-discovery-20261110-7e4a96](../../build/xuanshu-br2-discovery-20261110-7e4a96/)。delivery-identity SHA256：

`3DFFCFC515A9958A69396AE703307D6B9E84575D63DD921322D1882E5086F545`

入口531份范围副本不是全部Git内容快照。最终526份代码/配置源码、6项代码增量、158项程序/链接/调试及匹配包资源材料见同根清单。122项归档程序上下文不全是实际失败程序，角色由`failure-roles.json`区分。后续文档状态独立于核心源码、程序、日志/JUnit身份，不自动继承旧验收。

| 文件 | 本轮必要改动 |
| --- | --- |
| src/ai_model.inc | 分类参数说明；非空分类无允许项的明确结果；传递当前有界分类，保持旧命名分类和原过滤 |
| src/ai_responses.c | 私有池缩容使用原owner分配、复制新长度内的数据、释放旧块 |
| tests/business_discovery.c | 正式宿主页、真实browser模型、隔离传输与可信事件的5例直接回归 |
| tests/ai_model_memory.c | 缩容哨兵、相邻活块、原owner、压力失败保留、扩容和普通堆回归 |
| tests/br2_meter.c | 新工具Schema的实测首轮538精确基准及续轮边界，取代原496 |
| CMakeLists.txt | 新增专用目标/注册，窗口用例RUN_SERIAL，原20/30秒门槛保持 |

## 两项责任路径及最小修复

原[真实执行记录](xuanshu-br2-meter-real-20261110-validation.md)仍为业务暂不通过：模型传category=browser，browser.snapshot注册category=browser.read，目录按精确分类过滤为空；没有实际snapshot。本轮没有将其改写为真实成功，也没有追加请求。

分类修复集中在模型包装。参数说明明确使用空字符串枚举当前显式目标的允许read目录；命名分类只能使用发现描述中的精确值，不根据应用名/能力前缀猜测。公共目录、分页/输出限制和原read过滤不变。非空分类无允许项时保留items=[]/next=0，并附category_status=no_permitted_match、requested_category、suggested_category=""及固定说明；它只说明本分类未命中，不声称整个目标没有能力，不保证其他分类一定有能力。提示不自动查询、重试或签发权限。

空白名单对照另发现序列化后的响应内存被覆盖。ASAN/map/PDB对应am_delete→am_drop→am_finish→am_tick_impl；隔离边界记录显示m.batch在am_add_results的am_build_write内变化。冻结yyjson的pool_realloc仅支持old_size<size，而私有am_resize曾把缩容转给它，回退按旧块容量复制到较小新块，覆盖相邻活响应。独立Native哨兵在同一条件直接失败，形成与模型说明/时序无关的因果反例。

只修改私有am_resize的池缩容分支：在原owner内分配较小块，复制header和新长度内的数据后释放旧块；失败返回NULL且旧块不变。普通堆和扩容保持，原2MiB池内完成，不回退到额外无界内存。第三方依赖不改、没有新增线程/回调或改变引用合同。空白名单完整模型流程及独立哨兵修后均通过。临时追踪全部移出最终源码，生产DLL无追踪字符串或新增私有导出。

这不是原真实运行中已发生崩溃的证明，也不归因或解决BR1-KI-001、历史ASAN首败及其他未知问题。原symbol辅助输出的文件路径编码不可靠，函数/map及同期源码用于定位；不把软件边界日志冒充任意地址硬件监视或原历史现场。

## 修前、夹具及辅助问题分账

| 阶段 | 实际结果与限制 |
| --- | --- |
| red-first build | 未指定本地OSMesa，宿主测试目标未注册；补既有本地路径，无下载，不作产品红例 |
| red-controlled | 4例夹具前提失败：SSE缺少函数流事件且response id不一致，3例后继空访问、1例失败；不是生产崩溃证据 |
| red-valid | 混合阶段：分类缺失、模拟计数期待错误及空白名单实际异常；标签不等于全为有效产品红例 |
| red-contract | 3例、6项有效分类契约断言失败：empty/named各1项说明缺失，miss4项说明/提示缺失；合法发现、实际业务read及保存的其他断言通过 |
| red-no-scope / red-symbols | 普通空白名单工具后invalid_batch及ASAN异常；原程序与增加符号的程序分开保留 |
| trace-owner / trace-append | 有界隔离日志将变化缩至追加结果的写入/缩容；仅对应新复现，不冒充原真实捕获 |
| red-memory | 原私有分配器相邻活块哨兵失败；不依赖Provider、Service、模型说明或界面时序 |
| fixed-direct | Light4例通过，内存单元1项因数组构建参数错误Not Run；未写成5/5，通过显式数组完成后续构建 |

原始日志/JUnit、同期源码和匹配程序按同根`failure-roles.json`保留。driver-errors.md说明的元数据/工具错误与原始文件分别判断，未自动落盘的工具输出不由后补说明冒充。导出首次错误地将Native同Light比较，后按相同配置基线核对；不是导出丢失。

新增说明使固定初始输入由496变为538，`initial-meter-count.json`记录固定纯组件的实测body/receipt。测试继续精确断言538并验证续轮越界，生产16384上界、3次额度、费用和期限未放宽。

## 最终自动验证

三个独立运行，均对应最终源码和程序，不合并为首次单次全过：

| 配置 | 最终结果 | 原始证据 |
| --- | --- | --- |
| Light | 118/118 | light/final-audit.xml、final-audit-light-test.log |
| ASAN-Light | 118/118 | asan-light/final-audit.xml、final-audit-asan-light-test.log |
| Native | 49/49 | native/final-audit.xml、final-audit-native-test.log |

同根verification-summary、final-test-match及测试前源码/程序清单核对一致。所有窗口/输入测试串行；没有固定等待代替完成判断，没有扩大超时或缩小断言。

5例为empty、named、miss、no-scope、meter。前三例及完整计量例完成发现→实际snapshot→最终回答，最多3轮；空白名单仅返回允许项为空，不调用snapshot。直接检查实际序列化结果、控件、草稿、固定目标及SQLite工具内容，包含标签往返、隐藏拒绝、恢复展示、一次确认及重复拒绝、正常关闭和恢复仅展示记录。隔离可信事件不证明真实设备来源。

受影响回归包含原完整预览、PENDING、旧范围/代次、投影失败、缓存中断、两条各50000字节历史及112/48页、异步/回调、取消、存储失败cleanup-only、3次额度/第4次拒绝、期限/费用unknown、完整逐轮计量及恢复保护。Native补充真实DLL回调/卸载、静态嵌入、公共C/C++及旧费用恢复。具体覆盖与仍匹配证据复用见compatibility-coverage.json；没有机械重跑全部历史阶段。

## 精确业务和关后证据

两份唯一新建full-meter正例库分别属于最终Light、ASAN的discovery-meter；各1任务、2消息、31事件、2工具，唯一completed。一次目录工具和一次实际browser.snapshot，原固定目标、host/open epoch、read scope及框架revision保持，重开只恢复记录，确认/请求不恢复。

实际state.title为“BR-1 合成交接单：设备巡检”，state.address为browser:start，policy_count为整数0；业务revision与绑定值逐值相等。持久化原业务结果包含原空find_text，模型上下文只投影revision/state，目录Schema中的字段名不冒充字段数据。state是完整顶层对象，不是title-only，也不覆盖正文12/9/3汇总。

三轮输入计量均为538、1129、1471，每份receipt与manifest/body hash匹配。用同期序列化器、持久化响应及工具结果重建3个body，六份哈希逐字节一致；不是抓包。所有传输入口均simulated=true，不能将隔离model_request记录写成零模型路径或真实HTTP次数。

两库先查无WAL/SHM/journal，再immutable+query_only读取，前后哈希一致，见[closed-discovery-meter-db.json](../../build/xuanshu-br2-discovery-20261110-7e4a96/closed-discovery-meter-db.json)。观察器不重开Service或checkpoint。851项自有数据材料及47份旁文件另列，其中26份WAL、5份非空；不把两库的无旁文件结论扩展到所有材料。早期用例没有显式打印每份DB路径，不仅凭mtime将所有早期GUID库归到具体阶段；文件及旁文件继续保全。

## 兼容、自审、清理和停止

Light/ASAN生产358项导出、Native344项分别与原对应配置一致。公开头、版本、错误值、DDL、来源政策、C2/P4/P5、固定依赖和计量组件保持。50ms脚本、8MiB JS堆、128KiB模型、2MiB宿主、8MiB Service、256KiB包及计量组件独立128MiB限制不变。VALID编译依赖在obj清理前采集，清理后不声称obj仍存在。

自审核对了错误分类提示不签发权限/自动执行，空分类不泄漏未选能力，缩容保留原owner及失败旧块，实际工具值/序列化/hash相等，引用/PENDING和关闭保持。必须修正项在本轮限定范围清零；自审不是独立复审。

最初批量递归清理被自动审核拒绝且未执行；改为逐项明确绝对路径、非递归删除330份obj，327940869字节，全部成功。匹配程序、PDB/map、包内容核对材料、原始日志/JUnit、源副本、因果证据和全部数据库/旁文件保留。历史材料和冻结SDK不清理。

本轮无真实Key/环境Key/已有凭据读取，无真实Provider或账户API、无人工验收；生产framework_host.exe仅构建，未启动。启动监督8项及辅助2项只按原文件和适用机制复用，新入口未执行。BR1-KI-001未解决、用户接受非阻断；历史ASAN121/127与另次6/6、期限、复制、release计量组件非ASAN及任意卡死限时关闭等限制保留，不宣称已解释。已取消临时控件测试不恢复。

匹配入口见[交付说明](../xuanshu-br2-discovery-entry.md)，策略仍Proposed锁停。提示不能保证任意真实模型的行为，多余发现轮次仍可能耗尽3次预算，不能加额度补成通过。新候选需限定独立复审；之后才重新核对官方模型/协议/价格/上界并取得新的具体集中授权及逐任务可信确认。旧真实包已消费，不复用、不自行冻结或发布。
