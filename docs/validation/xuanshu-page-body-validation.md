# PB-1当前网页正文只读摘要开发验收

2026-11-10。开发及离线验证记录，不是独立复审、冻结、真实Provider或人工验收。基线身份3DFFCFC515A9958A69396AE703307D6B9E84575D63DD921322D1882E5086F545已核对，原BR-2阶段完成、旧包已消费。新根build/xuanshu-page-body-20261110-a64c28；增量以source-before/baseline-files对照，不把HEAD全部差异归为本轮。参考工程只读目录可访问，未改动。未使用子代理或computer use。

## 规格和实现

先冻结[正文合同](../xuanshu-page-body-contract.md)。实际apps/browser在成功加载同一份业务HTML后，以固定Lexbor解析器提取DOM正文并持有有界副本；不走剪贴板/私有脚本，不新建执行器或扩展原snapshot.state。新browser.body通过原browser.view和统一read链返回body/revision；body.text、bytes、complete及semantics明确。

合成样本精确归一全文：

```text
设备巡检交接单
设备总数：12台。
已完成：9台；待处理：3台。
负责人：林青 & 周明。
下一步
复核三台设备。
```

预期UTF-8为147字节。DOM实体解码、多段及br规范化，脚本/模板/表单内容排除；CSS隐藏作者文本仍可能包含，不称为屏幕可见文本。8192字节上限和UTF-8边界直接检查，超限不返回部分文本。框架revision与工具结果和固定目标逐值比较。

## 首败和修正

- red-controlled：生产公开read，页面真实加载而browser.body未注册，1例1项有效缺口失败。同期程序、DLL、包在red-program，测试增量在red-test-source，原始日志及JUnit保留；不称为真实服务故障。
- red构建最初遗漏既有本地OSMesa配置，宿主测试目标未生成；恢复匹配固定依赖配置后继续，未改依赖。
- body-first：6例中2通过、4项宿主例各1断言失败；实际正文/摘要保存已取得，失败位于ai-view-scope顶部裁剪。受控诊断rect.y=74、clip.y=84，原夹具硬编码70导致错误滚动方向。改为比较实际裁剪位置，保留完整可见/可用断言；fixed-scroll独立6/6通过。首败和诊断源码/程序分别保留，不归因模型名遗留。
- 一次误用全目标构建触及既有ui_browser_host_test缺yyjson包含目录的无关编译失败；保留日志，随后只构建本轮必要目标，不整改该无关历史入口。

最终结果按下节的各次独立运行记录；较早6/6、10/10或11/11均不替代最终匹配验证。

## 边界和停止

生产DLL公开read证明真实浏览器解析与新能力；实际正式宿主页在隔离DLL/可信事件/凭据及网络替身中证明提案、确认、工具保存和恢复，不是生产真实人工授权。原发送计量/3次额度/期限/费用保护不改，新正文范围不获旧BR-2真实策略授权，启动模板保持Proposed锁停。无真实凭据读取或请求，无人工验收。

BR1-KI-001未解决、用户接受非阻断；本轮模型名实际全文及读回不一致将停止受影响任务，不重复定位。历史首败、旁文件、未知期限根因和已取消控件测试状态保留。后续真实摘要质量、实际来源完整分类、IME/DPI/辅助技术/长时/断电不由本轮证明。交付仅等待限定独立复审，详[本地入口](../xuanshu-page-body-entry.md)。

## 最终验证、费用路径及首败判断

| 配置及运行 | 实际结果 | 适用范围 |
| --- | --- | --- |
| Light delivery-final | 131/131 | 正文、正式宿主、缓存/112及48页、发送门、存储/关闭及兼容 |
| Light isolation-final | 另次5/5 | 浏览器生产/隔离兼容及正文公开read三例，使用最终显式来源路径夹具 |
| ASAN-Light delivery-final | 130/131，1项20.03秒超时 | 正文11项和UI缓存/业务/发送门均通过；保留初始化首败 |
| ASAN-Light readiness-supplement | 同程序另次1/1，3.02秒 | 原20秒门槛，未修改程序、断言、数据或时限 |
| ASAN-Light isolation-final | 另次5/5 | 最终显式路径夹具，程序身份与最终对应 |
| Native closure-final | 7/7 | 公共C/C++、静态嵌入、响应解析/有界池及真实DLL生命周期 |

Light及ASAN按最终匹配依赖逐项覆盖131/131，Native7/7；六次运行不合并成首次单次全过。最终证据索引为verification-summary.json和final-test-match.json；补项覆盖前程序由清单匹配，未执行目标不冒充覆盖。

20.03秒首败位于ui_ai_model_reads的setup：Service尚未READY、edit/schema/coordinator前提检查失败，随后触及既有CTest上限；尚未进入本轮正文工具链。相关运行时和测试源码未被本轮修改，匹配程序及自有库/旁文件保留。同程序原门槛补项通过仅补齐该路径覆盖，不能解释首败。将其作为本轮非阻断初始化期限风险交给独立复审，未宣称根因解决、默认归因环境或扩大超时；正文自身的解析、工具保存及关闭均有独立正例。

ASAN早期46项失败先分账，实际投影catch取得RangeError和能力/字段嵌套forEach栈。最小修复为同序块作用域for循环，保持逐控件闭包的cap/field/index；不改运行时或256KiB栈/50ms/8MiB。同场景ASAN2/2及最终综合通过支持该责任路径。范围/历史选项缓存又以两例受控红例证明，现为破坏DOM前失效、全部选项及说明成功后提交；稳定同内容跳过重建。无效空闲诊断缓存候选已撤销。临时异常/CONTROL探针已删除，保留专用choices测试注入，未进入生产HTML或DLL导出。

两项旧browser_capability夹具早期使用子进程内LOCALAPPDATA临时隔离；发现后改为唯一自有UI_BROWSER_SETTINGS_PATH，显式空文件，不改变LOCALAPPDATA。原运行保留，最终各5项补测验证该最终源码。生产入口从未采用LOCALAPPDATA替换，未读取用户原有业务文件、真实凭据或数据库。

## 业务、持久化和关闭证据

关后closed-body-db.json索引Light/ASAN各5份最终主样本，均先检查无WAL/SHM/journal，再immutable/query_only读取，哈希前后相同。read样本各1任务、2消息、11事件、1工具，唯一completed；body全文147字节、complete=true、dom-body-v1及框架revision逐值正确，发送给隔离模型的工具正文也相同。history=[]、仅browser.body及body/revision，固定实例/epoch/resource/revision/read scope，标签切换不改绑，草稿不覆盖任务。

oversize样本实际经同一Service和业务read后失败，1条失败工具记录、仅1轮隔离模型请求，无成功body或摘要，失败终态与保存可恢复。cancel/revision/reopen分别保留唯一cancelled、0模型请求/0工具；旧epoch或revision拒绝确认。重开仅查询持久化快照，内存任务不存在、请求0、旧授权和工具不恢复。正常关闭由各用例检查并由CTest成功退出支持；超时等非正常首败及旁文件不混作正常关闭。

原BR-2额度、续轮、第4次、期限、费用unknown、存储失败和PENDING/隐藏/旧范围拒绝均保持。普通正文任务未配置RPC验证配额，不虚构M/R/S/C/U或精确HTTP次数；测试请求为隔离替身，并非真实Provider或账户费用证据。

## 最终身份、保全及结论

本轮代码增量为应用正文模块/接入、新样本和专用回归、CMake注册、正式页面选项缓存/调用层级，以及两处夹具适配。运行时/公共头99份与前置源码匹配，生产Light和ASAN358项导出与前置顺序一致，没有私有入口。版本、公开布局、错误值、DDL、来源政策、C2/P4/P5、固定依赖、预算和原时限保持。编译依赖在对象清理前捕获；清理后不把缺失obj误写成仍存在。

最终源文件、程序、文档副本、清单及日志/JUnit见新根delivery-identity.json；文档状态不回写BR-2冻结身份。6399份入口记录只是选定文件范围；86份过滤遗漏经冻结hash核对记录在baseline-normalization，不冒称全Git快照或产品增量。source-after、失败角色、原始日志、库和旁文件保留；仅清理新轮对象及无用途的复制Rust缓存，不动历史材料。

限定范围必须修正项为空，开发/离线逐项验证完成，具备限定独立复审条件。独立复审、冻结、真实模型摘要质量、真实设备来源及人工验收均未在本轮宣称通过。BR1-KI-001和历史期限/复制等首败保持；取消的临时控件测试不恢复。真实正文策略和授权继续Proposed，当前入口锁停真实验证。
