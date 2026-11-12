# UH-A投影构建失败与回调重入限定整改验收

2026-11-03。开发、自审及相称离线验证完成，停止在再次限定独立复审之前。本记录不宣称独立复审通过、冻结或真实人工验收。

## 1. 实际基线和归属

HEAD e0eca03db25930be55bdb45477027d1793a61904，分支codex/p1-process-boundaries。已有修改与未跟踪文件保留，不把HEAD差异全部归为本轮。新根为build/xuanshu-uha-buildgate-20261103-b27e91；入口先复制并哈希600份当前源码/文档，另保存Git状态，不冒称全部Git文件内容快照。

原交付build/xuanshu-uha-closeout-20261103-a6f2d8的delivery-identity.json SHA256为154B21D4271AFBEB8D98261A3FF540868771B0C8F4686FAEC3B1A271DD517AD4。入口逐项核对原415份源码及150项程序，无漂移；原根不改。承接最新限定复审：长历史/arena、持久化诊断/查询关联未发现阻断，投影构建失败后的回调窗口仍阻断冻结，UH-A暂不通过。新代码不自动继承旧证据或验收。参考README可读，未修改参考工程、扩大权限或重做历史全面审查。

## 2. 调用关系、修前证据和修复

原send_state在JSON构建失败后仍调用ai_panel_json；后者写入AI前缀后未检查failed，继续ai_model_panel_projection→ui_ai_model_get_disclosure→目标get_revision。旧展示资格直到send_state尾部才撤销。revision可重入原确认路径；末次回调检查只能看到尚未撤销的旧资格。

tests/uha_buildgate.c仅在隔离测试翻译单元包装宿主realloc，真实返回一次NULL，不改变夹具目录、运行时或公共授权。先完成同摘要完整展示，再分别在外层JSON首分配及AI前缀首分配注入失败，revision中使用既有隔离可信事件夹具尝试确认。该夹具不证明真实设备来源正例。

修前light/red-controlled.xml三用例均失败，每例4项断言，共12项；callbacks=1、accepted=1，已确认旧确认可在该隔离时序被接纳。额外“close”用例修前也在接纳断言处失败，尚未进入修后专用的关闭证明，不外推为已复现用户环境越界外发、真实发送、崩溃或死锁。当时源码、开测程序哈希、JUnit/原始日志及程序/PDB均在failures/red-controlled-light-test保留。

最小产品修复仅两处：

- main.c：外层JSON已failed时，撤销宿主权威展示资格，跳过本次AI投影。
- ai_panel.inc：AI前缀构建后、进入完整预览查询前检查failed；失败立即释放复制的run JSON、撤销资格并返回。

这是两个不同构建层次的回调边界检查，不在每次JSON写入重复加条件。外层余下菜单/布局序列化读取框架缓存，visit_menu调用宿主自身序列化回调，不调用应用revision/业务实现。失败后的第一次可能应用回调已被阻止；没有等到send_state尾部才失效。正常路径、末次回调后的运行时检查、run/hash/展示代次、逐任务可信人工来源均保留。

src、include及固定依赖字节不变，无新公开结构、导出、错误值、DDL、线程或来源语义。UNKNOWN/cleanup-only保持。SDK0.10-dev/API10/标准10、ABI1/包1、256KiB单包、128KiB模型、2MiB arena、8MiB Service及原超时不改。

## 3. 直接断言和相称回归

| 证据 | 结果与范围 |
| --- | --- |
| light/red-controlled.xml | 修前3/3失败、12项断言失败，隔离旧确认接纳；不是生产事故 |
| light/fixed-direct.xml | 初次修后1通过/2失败：终态出现即检查refs=0，未等待正常网络cleanup；关闭用例通过 |
| light/fixed-lease.xml | 保持refs=0断言，改为5秒内按实际引用有界poll，3/3通过 |
| light/closure.xml | 最终26/26通过，包含最终加强草稿断言的新直接3例 |
| light/copy.xml | 同程序正常桌面上下文复制诊断另次1/1通过 |
| asan-light/closure.xml | 最终26/26通过 |
| asan-light/copy.xml | 同程序/匹配运行库PATH，正常上下文复制另次1/1通过 |

以上为不同运行，不合并为首次单次全部通过。没有在测试期间修改源码，最终closure/copy的源码与全部开测程序清单逐项匹配最终文件。

直接用例entry、prefix、close证明：构建失败后callbacks=0、display_valid/preview_ready均撤销、确认接纳0、模型ordinal/model_calls/request_saved为0、隔离传输requests=0，run/hash/完整预览及含输入/历史/能力/字段/资源的选择草稿保留。普通宿主没有RPC验证配额策略，不虚构测得M/R；未确认、ordinal=0且无模型请求保存，证明未进入请求消费/发送路径，未引入策略或额度旁路。

随后独立调用允许的只读预览查询，在应用回调中检查宿主资格仍失效、Service方法及run引用在途、unbind=PENDING而旧确认仍拒绝。close用例在回调内请求关闭，worker确实退出且Service CLOSED后destroy仍PENDING；回调和外层退栈后引用归零、安全关闭。正常用例恢复完整实际展示，确认恰好一次、重复拒绝、离线回复完成；传输替身仅收到恢复后的一个请求。

相称26项覆盖新直接3例、原UHA预览/显示/read/历史/重建5例、原closeout8例（相同摘要post失败、旧代次/缺片/重复/乱序/迟到、PENDING、ASCII两条各50000字节及UTF-8、真实64槽回收、重启/失败快照、A→B→A、回调关闭等）、Service/模型/编辑/图四面板、静态嵌入与模型静态、C/C++头、生产链接memory及静态配额策略。实际read10/r1和框架revision、工具结果保存及完整请求逐值一致复用既有断言，未以计数或控件变化替代业务结果。

运行时没有修改。旧Native40/40不重跑：原Native程序哈希保持，原Ninja依赖记录显示无本轮宿主文件依赖，src/include/相关测试源码保持；元数据中的STALE来自此前清理obj，不称为新编译或新测试。新根Light/ASAN重编并验证含宿主的各分支。生产DLL可能因新构建重新链接形成不同文件身份，以当前binary-files为准，未把旧DLL哈希当新文件；生产导出集合与原交付相同，私有realloc/fixture/probe没有进入生产DLL。

## 4. 首败、自审和证据边界

- red-light-build：新测试将const资源ID赋给可变宿主action字段触发C4090/WX；改用相同字面量，告警规则未放宽。该次无新成功链接的buildgate程序，归档内其他程序不冒称该失败源码的新程序。
- red-controlled：产品窗口的直接有界断言失败，原始材料保留。
- fixed-direct：产品拒绝旧确认已生效，测试未等待终态后的真实cleanup引用归还；添加有界状态poll而不删除refs断言，不改生产超时。
- 只读依赖取证首次因ninja不在当前PATH产生WinError2；随后使用原CMakeCache记录的绝对ninja路径读取依赖，无构建。不是产品缺陷或测试首败。
- 自审补充包含选择草稿的逐值保留断言，最终Light/ASAN均在该最终测试源码上运行。没有临时诊断留入生产。

原ASAN缓存用例30.08秒首超时仍未完全定位；本轮未改变种子投影、真实64槽、业务/回收/查询/关闭断言或30秒门槛。后续通过不证明原超时根因解决。历史期限、frontend、输入、browser及ASAN-Light首次失败分别保留，不默认归因环境。

真实Key/Provider、原用户库、集中人工均未访问或执行。模拟来源/离线替身/ASAN只证明相应有限条件。真实设备与生产来源、Windows动态正例、IME、DPI、视觉、辅助技术、人工长时未验，物理断电仅专用环境后续方案；browser九副作用继续另案。已有T1/T2真实结果、T3远端/费用unknown、483 Tokens差额和请求重建非抓包不变，不追加请求。

## 5. 最终身份、保全与停止

新根delivery-identity.json关联delivery-files、source-after/source-final、增量diff、binary-files、失败源码/程序对照、verification-summary、compatibility-coverage、exports、fixed-dependencies、sentinel-scan、owned-data-files及cleanup。final-readback仅为封装后核对，不是新测试或独立验收。

关键程序SHA256：

| 程序 | SHA256 |
| --- | --- |
| light/framework_host.exe | C282C6428735C65B35612AA24AF74D4079B0B1E52015ADC68330781E56E47434 |
| light/ui_framework.dll | 888A80D5847214615AB48DB1DFF89DABF270D7092253BEC886256E2AB5EC3FD8 |
| light/ui_uha_buildgate_test.exe | 2D485DA0B5B0E83F355B3E0210B3884ECDBB83B7961CA6859FDA241D2980FE25 |
| asan-light/ui_uha_buildgate_test.exe | FC4F42456D01A9140974DB2BFA94D5490E8D15503301053239EC49A818C2EF44 |

根三规划仅追加；原根/历史文件、原修改、未跟踪文件、冻结SDK及固定依赖保持。只删除经绝对路径确认在新根内的无用编译obj；保留源码副本、首败、日志/JUnit、匹配exe/DLL/uapp/lib/PDB和测试自有库/配置，数量见cleanup.json。必要过程脚本和依赖取证用于复核，继续保留，不清理历史证据。

当前授权范围必须修正项在开发/自审层面已清零：剩余限定整改已完成，自动验证通过，已具备再次限定独立复审的条件。下一步仅建议限定独立只读复审；不自行宣布独立通过、正式冻结或进入集中人工。


## 2026-11-03独立复审结论追记与冻结

上一轮限定独立只读复审确认剩余阻断窗口闭环、UH-A限定验收通过、匹配快照可以冻结、必须修正项为空；现按授权[追记落盘](xuanshu-uha-buildgate-independent-review.md)。本轮核对原identity/416源码/102产物无漂移，不重新测试或开展完整复审，不改原build证据。原ASAN30.08秒与ninja首次stderr限制及全部首败保持。更新后的文档是新状态，不冒充原测试时哈希。另获第一批零Key/零联网人工授权，过程另见[xuanshu-uha-manual-first-validation.md](xuanshu-uha-manual-first-validation.md)，不预判通过、不进入真实Provider。


## 第一批人工观察的后续边界

本次原快照冻结追记不变。唯一监督重试后的人工第一批部分覆盖，但收起残留/控件无响应及输入区难定位已实际暴露，第一批暂不通过；用户正常关闭，exit0及关后唯一取消终态、无模型请求记录已核对。详[xuanshu-uha-manual-first-validation.md](xuanshu-uha-manual-first-validation.md)。未修改产品/重建，不将此前限定独立通过扩大为日常体验完整验收。
