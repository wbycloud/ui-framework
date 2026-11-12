# RP-A＋RP-B限定独立复审与交付快照冻结记录

日期：2026-10-31。状态：限定验收通过；匹配交付快照可以冻结。本页追记本会话上一轮独立只读复审结论，本轮仅核对身份并记录，不是重新开展完整复审、重新测试或真实服务验收。

## 1. 结论与边界

上一轮结论原意：两项整改闭环，RP-A＋RP-B限定验收通过，匹配交付快照可以冻结；无必须修正项。该结论针对发现结果适配、cleanup扫描引用及相称回归，不扩大为P2—P6全面复审、真实DeepSeek或真实人工验收通过。

冻结是交付快照记录，没有执行commit、tag、release、推送或文件只读锁定。代码、产物或配置如改变，应逐项区分新变化，不自动继承原验收。RP-C规格确认可以开展，实际联网必须另行授权。

## 2. 对应基线与身份

分支`codex/p1-process-boundaries`；HEAD `e0eca03db25930be55bdb45477027d1793a61904`，未提交工作区。整改根`build/xuanshu-rpab-closeout-20261031-a19c42`；原RP根`build/xuanshu-rpab-20261029-6f38a1`保持只读，不将HEAD至工作区全部差异归为整改或本轮文档。

| 身份对象 | SHA-256 |
|---|---|
| delivery-identity.json | B2F9941B437964071D775345F3390C847A5A6ADC3190618CC500EAB93197DAA0 |
| source-after.json | DE9DD6C31991971676C58995F319AC337130D61315D2205F78B213B608C718C8 |
| binary-files.json | 7416C6FA7E5F27C222CAD4FDB0C315941929EC9B6D1D883E188E8E583843855B |
| red-binary-files.json | 747048ED6B7535601F114862EA45747964C020BB9114A018D3BDBD31A4773E8B |
| Light framework_host.exe | A55617390C4FB2854881DB838B4D08B2BAE96BB880E36AA584EEE6B7D7BB550A |
| Light ui_framework.dll | E04987A3AED11C3CE44AF584DBACEFAF4308E469AF486CD7CE47A3E179A56C53 |

写文档前，本轮只读核对390项源码、14项整改交付、114项最终产物及36项修前归档产物均匹配实际文件哈希，delivery-identity引用的清单亦匹配。没有发现新的产品身份漂移。参考工程`D:/XuanShu`仍不可读取，未扩大权限，以上结论以框架实际源码为准。

入口baseline捕获491项；另3份原RP新增文档为事后依据原交付长度及SHA-256补核，不得写成494项全部入口自动捕获。baseline-supplement保留其来源界限。当前8份文档修改属于冻结追记/RP-C规格增量；其中6份原有文档会相对原delivery-files产生预期文档差异，不修改旧清单，也不将新文字冒充旧快照已测试内容。390项产品/测试源码与114项产物保持原身份。

## 3. 上一轮复审依据追记

发现结果：原P3/v1根数组摘要保持，RP适配从同一目录describe取得完整read描述；投影按数组读取并应用白名单，非法结构失败，不伪装空目录。新增测试直接解析function_call_output.output，逐项核对ID、能力版本、说明及输入输出Schema。依据src/ai_model.inc的am_discovery_append/am_result、src/ai_service.c的发现分支及tests/ai_model_closeout.inc。

生命周期：扫描在Service锁内选择候选、复制稳定run ID并取得独立引用，解锁处理网络cleanup，结束核对身份再归还；网络引用独立，缓存仅回收无引用任务。真实64槽压力下候选保持，归还后可正常回收与关闭。cleanup-only没有重新派发或清除UNKNOWN。测试屏障/探针未导出至生产DLL。原32份公共头、固定依赖和生产导出匹配，SDK0.10-dev/API10/标准10、ABI1/包1、既有错误、v1/C2/P4/P5、预算/来源政策及超时均未放宽。

这些是上一轮限定源码判断的追记，不由本轮哈希匹配代替源码复审。

## 4. 已有测试与首败保留

| 整改轮已有证据 | 结果 | 必须保留的表述 |
|---|---|---|
| native/fixed-controlled.xml | 2/2 | 新增受控回归 |
| native/matching-regression.xml | 61/61 | 与2/2是两次运行，非首次单次63全过 |
| light/light-matching.xml | 42/42 | 已有离线结果，本轮未重跑 |
| asan/asan-matching.xml | 52/52 | 已有离线结果，本轮未重跑 |

修前red-controlled发现用例31项断言失败；真实64槽扫描4项断言失败，refs_during=0、retained=0。归档保留同期源码、原始日志/JUnit和匹配程序。修后日志为refs_during=1、retained=1，消费后引用归还、候选可正常回收。屏障在身份异常时跳过危险解引用，仅证明有界失败，不宣称已复现崩溃、永久死锁或数据损坏。

首次夹具回调签名、received关联变量、修复编译局部名及Light空Mesa配置/目标编排错误分别记录，不混同产品缺陷。原RP Native61/61、Light40/40、ASAN50/50仍为更早证据。详[xuanshu-rpab-closeout-validation](xuanshu-rpab-closeout-validation.md)及原始清单；本轮没有执行run.ps1、finalize-closeout.py或任何历史证据更新脚本。

## 5. 保留事项与后续入口

四组期限风险（并行graph_boundaries>20秒、ASAN控制5秒、P2五包15.04秒、面板180.44秒）未定位，后续串行通过不是根因证明。旧frontend63、ASAN-Light中断、Light surface_input、browser20、workspace首次20.05秒/20秒门槛、Native输入首次失败保持原结论，不默认归因环境，不以新通过覆盖。

真实DeepSeek、TLS/代理、Windows凭据界面、真实设备/Windows来源动态正例、IME、DPI、视觉、辅助技术、物理断电及人工长时未完成。browser九项副作用另案，OpenAI/OAuth/AI写入不在冻结补交范围。延期项不自动阻断该限定快照，但不得写成已验。

后续按[RP-C规格](../xuanshu-rpc-validation-plan.md)先取得验证准备实现授权，再在准备验收且用户完成原生凭据/外发确认后取得真实调用授权。冻结记录不授权任何真实API调用或读取密钥。本轮不清理历史文件、不操作用户数据库、不修改产品/测试/构建/版本或依赖。
