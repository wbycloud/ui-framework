# RP-C验证准备限定独立复审与快照冻结记录

2026-11-02。状态：限定验收通过，匹配交付快照可以冻结。本文件依用户授权追记上一轮只读独立复审结论；本轮只核对身份，没有重新开展完整复审、构建或运行离线测试。冻结仅是交付快照记录，没有commit、tag或release。

## 1. 对应身份

实际分支`codex/p1-process-boundaries`，HEAD `e0eca03db25930be55bdb45477027d1793a61904`，工作区未提交。P2—P6、RP-A/B和原准备修改保持，不把HEAD至工作区全部差异归为整改或本轮。

冻结根`build/xuanshu-rpc-prep-closeout-20261031-f27a64`：

| 身份 | SHA-256 |
|---|---|
| delivery-identity.json | D10E28A6AB8DE63FC854518F75C67B8803E41708701084083691D4A0B32459B7 |
| source-after.json（399项） | A19AB8168D9AC8DD73A6987F8BFD4E9FC7A03FF143B7DF5FE93FEE813FDD1C74 |
| binary-files.json（122项） | 90A253ACF45A54E7A739995F197A69E0A29EC164A2B2346D16DB4DB6FBDD12FA |
| light/xuanshu_rpc_probe.exe | 77B272DD1F008C3B5CA265997387E40391DF62E405D1773CD0DC0EBB2F8838FB |
| light/ui_framework.dll | C672F126E693E69F1959B45418141C5314EB83154F34D46D7DDF545FD9E4F35C |
| light/rpc_memory.uapp | 4EE210EAA0E7651067713F30B6D6A2D5FACB99EF12B88EBC4E5E81586B6DF61A |

本轮文档修改前核对399项源码、13项原增量、122项最终产物、27项red归档、8项固定依赖及identity关联的18份清单和43份证据，均无哈希差异。入口为508项当时范围，不是全工作树；13项增量含5项代码/测试/构建及8项文档/规划。新增复审记录及获准更新文档是本轮文档增量，不自动并入原产品测试身份。核对记录见`build/xuanshu-rpc-real-20261102-b5e731/preflight.json`，旧证据根不更新。

## 2. 上一轮限定复审结论

没有发现新的阻断问题，两项整改闭环。RP-C验证准备限定验收通过，匹配快照可以冻结；没有必须修正项，具备进入首次真实服务验证授权确认的条件。

`src/ai_model.inc`的am_propose在所有client字符串比较前调用ai_id；NULL、空串、超长和非法字符返回INVALID_ARGUMENT，合法不匹配ID仍按原准入合同拒绝。外层received关联及方法/任务租约保持，正常同步只读、重复提交和关闭不受该条件修复影响。

`tests/rpc_memory_contract.c`使用现有JSON类型校验及完整规范化结果比较，严格确认整数10、字符串r1、框架成功状态和结果revision；拒绝100、字符串10和错误revision。生产链接与静态分支均核对相应合同，未把子串匹配当成业务证明。

公共布局、错误数值、v1只读语义、来源政策、依赖、预算和超时不变；私有回归入口未导出到生产DLL。SDK0.10-dev/API10/标准10、ABI1/包1及冻结C2/P4/P5保持。

## 3. 已有证据及限制

已有修后结果是Native新增受控5/5及相称44/44两次运行、Light51/51、ASAN49/49，不写成Native首次单次49全过。本轮不重跑。修前两个NULL用例在独立测试子进程异常终止，非法ID有6项错码断言；同期源码、JUnit、日志及匹配程序实际保留，不外推为用户进程事故、永久死锁或数据损坏。夹具C4389编译错误与产品运行缺陷分开。

原准备阶段最早脚本未扫描tools、首次启动器编译失败完整同期源码未自动归档且当次无成功链接启动器的限制保持。整改证据脚本首次自引用文件名错误发生在身份输出阶段；初次只有工具输出，后补原样复现日志明确标注为补充复现，不冒充原自动捕获。详情仍见[整改验收第7节](xuanshu-rpc-preparation-closeout-validation.md)。

历史期限、frontend、输入及browser首败继续独立未定位，不默认归因环境，不以后续通过覆盖。browser九项副作用另案；真实设备、Windows来源动态正例、IME、DPI、视觉、辅助技术、物理断电和人工长时保持延期。可选补测及范围外能力不阻断本次限定验收。

## 4. 冻结与真实验证的边界

准备验收不等于真实DeepSeek、TLS/代理、原生凭据界面或真实人工验收通过。用户2026-11-02已另行明确授权首次真实验证，但须使用上述匹配程序、新隔离库/config/ref，并由用户原生录入Key和逐任务可信外发确认后才能发送。真实结果另记[首次真实服务记录](xuanshu-rpc-real-service-validation.md)，不倒填为本次独立复审依据。
