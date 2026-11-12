# 通用助手ASAN单项驱动修正与唯一复测记录

2026-11-12。本轮只修外部驱动并运行一次`ui_general_host_business`，没有修改产品、测试、构建配置或重新构建。证据目录为`build/xuanshu-general-asan-retest-20261112-0f7a36`。

本次初始化前提成立，业务流程实际进入，但在原30秒门槛下再次超时。两项边界修复的直接回归结论保持；完整ASAN业务验证仍未收口，不能写成补项通过或完整独立复审准备完成。

## 匹配身份和范围

候选为`build/xuanshu-general-boundary-20261112-9d6c53`，delivery-identity SHA256为`C2379C435E0684A1EFC025A85A782397AF5B0D1CE4B3A49E44ABA7E84862C6ED`。

执行前后447份源码、179项程序/链接/调试产物、原ASAN完整运行的57项测试前程序清单均匹配，差异为0。候选identity及被核对的原清单未改写；前次1秒失败目录的13份文件也保持原哈希。本轮新增文档不属于原候选交付副本。

| 实际文件（相对候选根） | SHA256 |
|---|---|
| `asan-light/ui_general_host_test.exe` | `0459B8CA36B31889E30E861EEC1FC7F3BB0AD752FB19F624C9ABC180ECA40DCC` |
| `asan-light/ui_framework_edit_test.dll` | `84C3F25607D30283C0FB423A4004AD1AC4C9AAB239C1F579D0924C545D45AE14` |
| `asan-light/ui_framework.dll` | `00B523B9848F3CF08CE892B004AE7F729B47C9F8E090461F4DB932CC255300D6` |
| `asan-light/br1-isolated.uapp` | `1C1249C3ED7333E35289F47A0B1F277ACE967DCD994E6329541A2D43EF7DA089` |
| `asan-light/p4-stateful_components.uapp` | `76F99C61B2C42BB0FC9A464E33CAB5FCBE2AE0F598ADCE1C8062A6CF541B0DD2` |
| `asan-light/xuanshu_br2_meter_fixture.exe` | `1AB2626CF2DC609452F30DF463B4AB2E77AEB31DB87E4E71001FF470068184A9` |

固定OSMesa仍为`.deps/mesa-24.3.4/x64/osmesa.dll`，SHA256为`C633B820BBA8EC0DCB05466505E3C246E4C4028EAEC7CAF1054E707C8EB3CA1F`。前置公开布局、来源、版本、依赖、资源预算及超时均未变。参考工程仅检查目录可读性，没有修改。

## 外部驱动修正及准备

前次`build/xuanshu-general-asan-once-20261112-954c5b`在PowerShell7.6.9/.NET10.0.14下用`Environment.SetEnvironmentVariable(name,$null,'Process')`删除变量，实际留下空字符串。`ai_assistant_enabled`明确区分“缺失”和“存在但为空”，导致通用助手前提失败；非致命CHECK后夹具继续访问空指针。该轮1.00672秒、CTest退出8及ASAN空写记录保留，不能作为业务期限复测。

本轮`run-once.ps1`改用`ProcessStartInfo.Environment.Remove`，只删除子进程中的`UI_AI_BR2_POLICY`、`UI_AI_PROFILE_PATH`、`UI_AI_PROFILE_ID`和`UI_AI_ASSISTANT_POLICY`。父进程环境未修改，无需恢复；没有读取凭据变量或转储环境。

正式测试前运行无秘密Python/Win32探针。四变量均返回不存在、Win32错误203；独立缺失对照也是203，空值对照则存在且为空、错误0。探针和CTest由同一子进程映射函数构造，CTest没有再设置测试专属环境覆盖。

原CTest参数仅将样本根替换为本轮全新目录。工作目录显式保持原候选`asan-light`，固定计量替身及依赖保持。CTest元数据核对仅一项、`RUN_SERIAL=TRUE`及`TIMEOUT=30`，随后独占创建`attempt-started.json`并启动一次。没有重新编译、改参数补跑或提升期限。

## 本次实际结果

| 项目 | 结果 |
|---|---|
| CTest PID | 46804 |
| 测试进程PID | 36012 |
| JUnit耗时 | 30.0711秒 |
| 结果 | 0/1，Timeout |
| CTest退出码 | 8 |
| 持有进程句柄取得的测试退出码 | 255，发生于CTest期限终止；不是正常应用退出 |
| 后续业务启动 | 0次 |

模型输入、输入返回、消息泵后、保存前、页面消息和宿主配置入参均为完整`deepseek-flash`。没有再次出现模型名少字符，也没有据此更改BR1-KI-001的遗留状态。

日志有四次`InternalError: interrupted`：组件`updateRow`一次、`paintScrollbars`两次、宿主`paintParams/paintState/projectionPart`一次。没有打印业务CHECK失败或ASAN内存错误，但期限终止不能证明后续全部断言、安全关闭或内存路径通过。原50ms脚本预算保持。

## 关后持久化证据

测试进程退出后，原主库139264字节及非空WAL733392字节保留。原主库SHA256为`9822B2D61CCA901262970D8B49D56B2FB6DBEB0A647363A8C2ECCE9DB753046A`；WAL为`608D2BADA0B5B5A337CD693172F58A783095F4A280C13C7756FE17058028824F`。

没有把独立主库作为完整状态打开。将本轮主库和WAL逐字复制至新的观察目录，对副本使用`mode=ro`及`query_only`，未checkpoint、未重开Service或恢复执行。SQLite仅在派生目录生成SHM。原5份样本文件前后哈希、大小及集合完全一致，派生主库和WAL也未改变。观测记录见`closed-observation.json`。

WAL感知观察得到2任务、3消息、48事件、0条原tools表记录、1个图和1条编辑记录。图节点结果单独存于`ai_graphs`，因此不能把tools表为0解释为没有执行图业务。

- 根任务`0f3c0db48c9930fb00324e6ceca266720a87b2a25daa34071beabaa6075cd38f`为phase2、terminal0，未保存最终根结果。
- 浏览器节点已保存实际标题“BR-1 合成交接单：设备巡检”、来源`browser:start`及框架revision，节点completed。
- 维护节点保存了参数text=`74`、changed_cells=1、transaction_committed=true、storage_confirmed=true及`doc-2`回执；子任务completed。参数和回执不代替之后独立的单元格实际全文读回。
- 根账本有4次隔离占用、请求保存及传输入口，而完整用例预期6轮。图中两节点已收集，undo_finished=false。
- `cell_expect(74,doc-2)`、显式撤销及`cell_expect(42,doc-3)`、根终态保存、正常关闭和记录恢复没有完成，不能借用其他运行将本次改记通过。

种子和业务过程使用既有隔离确认、模型、凭据及传输替身。上述计数不是实际HTTP次数，也不是真实设备来源或真实服务验收。

## 历史分账及收口判断

Light95/95、ASAN94/95及Native47/47仍为原三次完整运行。本次是同身份下另一次0/1超时，没有形成ASAN补项1/1，更不能写成首次单次95/95。

原候选30.0569秒超时、前置原程序约30.08秒超时、1.00672秒驱动前提失败和本次30.0711秒有效期限失败分别保留。本轮已修正并验证驱动变量删除问题，但没有定位共享ASAN期限根因。电脑同时负载仅为用户报告，未作因果验证；距离自然完成仍需多少时间未知。

两项边界修复及其直接Light/ASAN8/8、Native7/7证据保持。完整相称验证仍未收口，当前不宣称完整复审准备、独立验收通过或冻结。后续若获独立授权，最小定位范围是两节点收集、第四轮保存之后的专业/根结果续轮推进，以及正式宿主和组件投影、消息泵与原50ms预算的关系。应记录这一段真实阶段与耗时边界，不继续无区分地重复全例。本轮不执行该定位或产品整改。

## 记录与保全

本轮记录包括`preflight.json`、外部驱动、环境探针及结果、CTest元数据、唯一启动记录、原始日志/JUnit、`execution.json`、`closed-observation.json`、`result.json`、`postflight.json`及证据清单。日志写入期间一次旁观读取遇到共享锁，退出后完整读取成功；该收集提示与业务失败分开，不影响测试参数或输出。

清理仅限本轮观察结束后可由原主库/WAL重建的3份派生复制文件（含派生SHM）。原失败库、WAL、必要脚本、原始日志、状态和身份材料全部保留。候选源码、程序、原identity及历史证据没有改写。根三规划只追加checkpoint，最新交接和验收入口另行关联本次记录。

本轮未访问真实Key、环境Key或已有凭据，未调用真实Provider，未启动人工验收。真实质量包保持Proposed，已取消的临时控件测试保持取消。后续复审、冻结及真实执行仍属于独立阶段。
