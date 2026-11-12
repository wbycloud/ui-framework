# P3独立复审与租约收口

2026-10-26。评审基线HEAD e0eca03db25930be55bdb45477027d1793a61904及原P3未提交交付。无署名。

## 1. 原发现与证据限制

ui_ai_continue经ai_targets_check→ui_capability_bind→get_revision，Service destroy原门只检查worker、poll、事件callbacks及工具，未覆盖外层continue。源码确认存在重入关闭/poll/销毁窗口；复审当时未运行测试，未动态复现崩溃。原P3 Native6/6、Light7/7、ASAN6/6不能覆盖带目标continue的该窗口。

frontend63次失败包含旧Assistant写EDA、双注册及程序输入修改期待；同当前DLL下入口host/page/test对照集合相同，不是完整旧框架对照，参数/草稿连带失败不能全部判为旧测试错误。Light surface_input、browser20、workspace20.05秒/20秒门槛、Native输入首次继续独立。

## 2. 本轮授权与验证计划

先受控回归核实：revision内请求关闭；外层和回调在途destroy PENDING；回调后检查关闭；外层退出再安全销毁；正常continue完成。仅增加必要私有方法租约，不禁止全部重入，不改公共合同。独立证据build/xuanshu-p4a-20261026-a61d4e；实际红绿结果待执行后追加，不预填通过。


## 3. 已核实窗口、最小修复及新证据

原路径为src/ai_service.c的ui_ai_continue→ai_submit→ai_targets_check→ui_capability_bind→应用get_revision。tests/ai_continue.c由隔离子进程先写入未完成记录，父进程对真实workspace资源继续；revision回调请求关闭并持续有界poll到CLOSED，然后尝试destroy。

p3-red-native-test.log实际48检查/2失败、freed=1：旧Service被允许销毁；回归当场返回CANCELLED而不再使用已释放Service，未制造或宣称复现崩溃。red/native以及failures/p3-red-native-test保留匹配EXE/DLL/uapp、源码及p3-red输入身份；不能以编译失败后留下的旧EXE充当红测试。

修复在Service私有结构加methods计数，ai_submit外层进入至返回一直持租约；destroy把methods纳入PENDING门。fresh target应用回调返回后重查READY，关闭时不继续接纳。不禁止回调重入，poll/关闭/查询仍合法。新continue正常分支及重入分支50检查/0失败、freed=0；回调及外层仍在栈时不能销毁，外层返回后能安全销毁。

p3-fixed阶段Native9/9先通过，再开展P4。最终联合配置结果见P4-A验收：包括Service/continue/native静态embedded/control/storage/limits/tools、Light面板，以及原A+B和C1两条真实DLL生命周期分支。新methods不更改任何公开布局或错误值。原P3 32项交付/308项源身份/101项二进制在本轮入口全匹配；本轮修改归属按新baseline而不是HEAD差异。

## 4. 结论及限制

本轮确认并收口该P3阻断窗口，具备承接已授权P4-A专用夹具的条件。P3单workspace/主对话、只读工具、取消、存储与面板由原证据及受影响回归共同支持，没有重新开展全套P2或真实人工验收。

旧frontend63断言及其对照只说明当前DLL下入口host/page/test同集合失败，不能证明全属旧测试错误，也不是完整旧版本对照。与新P4专用内存闭环没有已证实因果；后续正式面板接入前应迁移其合同预期并核查诊断显示。其余Light surface_input、browser20、workspace首次20.05秒/20秒、Native输入首次继续未定位，原始材料未改；不能由本轮限定通过销账。真实设备及长时风险仍延期，未当作无限延长此次租约收口的理由。
