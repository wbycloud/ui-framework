# BR-1 实际业务只读接入开发验收

## 状态与身份

2026-11-05开发自审及离线验证完成，候选交付 `build/xuanshu-business-read-20261105-8c6e21`，等待限定独立复审；未冻结、发布或执行真实服务/人工验收。最终身份以该目录delivery-identity.json及source-final、binary-files为准。前置DH-1身份65823A92BAAF49CE2623F5865F2C47B678215DD813D7E8EDDC8725980107A532经419项源码及74项产物匹配核对，旧结论仅适用旧快照。

入口519份范围源码/文档在编辑前保存，不是全部Git内容快照。本轮只修改浏览器配置文件定位、测试注册并新增样本/隔离测试/文档；框架运行时与公开头未变。参考工程目录可读，未修改。原用户库、旧人工库、冻结SDK及历史证据未操作。

## 业务闭环及证据

实际应用为仓库受限浏览器apps/browser，包内合成交接文档由真实文档解析器读取。browser.view/browser.snapshot读取state/revision；标题精确为“BR-1 合成交接单：设备巡检”，地址browser:start，框架revision与任务固定revision、持久化工具结果revision一致。本轮观测revision为77fbdb9e18262e8aa0c807ae275fbfdab249f027c46f7068e4831509755f70af。不是rpc_memory或固定返回值替身；样本文本中的12/9/3不属于本次已验证的结构化读取结果。

正式宿主页在隔离测试中完成草稿、结构化范围、本地提案、完整预览、隔离可信确认、模型传输替身驱动实际浏览器read、结果展示、最终保存、关闭和同库记录恢复。实际请求对model、input前缀、instructions、tools、reasoning、stream和max_output_tokens与预览比较。history=[]、能力browser.snapshot、字段state/revision；state包含现有阅读/诊断状态，不宣称仅标题字段。两实例标签切换后固定目标JSON和run保持；revision变化和重开旧epoch拒绝确认。隐藏范围且解绑PENDING时旧确认拒绝，恢复后确认一次、重复拒绝。恢复只读取持久化记录，不恢复授权或请求。

独立生产链接测试使用生产DLL及br1-production.uapp，通过公开目录/read接口读取同一样本，核对标题、地址、空来源策略和两实例身份。它不创建模型请求。生产公开读取与隔离宿主全流程是两条证据，不能写成外部脚本完成生产宿主全链人工授权。未进行新的生产界面截图或真实设备验收。

closed-business-db.json记录最终Light业务用例关后immutable只读核对：1任务、2消息、11事件、1工具、唯一completed；无WAL/SHM，数据库核对前后SHA256均C397FC778AB7CE9933D8BB6300DCBF3DE188D01F85D32DDFFB313B83046E0065。工具结果实际保存标题、地址、revision；该库属于隔离模型测试，有测试确认及请求记录，不得描述成零模型路径。生产真实Provider请求未执行。

## 相称验证（不同运行）

| 配置/证据 | 结果 | 适用范围 |
| --- | --- | --- |
| Light closure.xml | 43/43 | 正式页面、缓存/长历史112/48页、确认门、查询、控制/存储、静态及公共头、新业务 |
| Light final-complete.xml | 6/6 | 最终业务测试及生产读取、配置隔离 |
| ASAN closure-final.xml | 41/43 | 首次两失败保留，不能写成单次全过 |
| ASAN persisted-investigation.xml | 1/1 | 原程序原30秒门槛持久化摘要补项，203检查，6.27秒 |
| ASAN final-complete.xml | 6/6 | 最终业务补项，17.42秒 |

逐项最终覆盖43/43，不合并成首次单次通过。verification-summary.json和各测试前sources/binaries清单关联原始日志/JUnit。业务测试最终增加有界5秒轮询实际终态保存投影，未降低原断言、扩大原30秒测试期限。无需机械重跑未改动运行时的全部历史阶段。

## 首次失败、自审及风险

- business-first实际读取成功，但新测试误期待RPCB特有framework_revision字段；改为公开框架revision、固定目标及工具revision逐值比较，首败保留。
- settings-recovery夹具缺wchar.h导致编译失败；补头后通过。
- recovery中SSE替身缺delta导致invalid_final_response；补符合原协议的替身，不修改解析器。
- final-observation把同app_id不同包当成不同模块；workspace按app_id复用模块/包，前提不成立。改为同生产包双实例身份/读取检查。入口要求新进程只打开本轮包，不能混用同ID的其他包。
- ASAN closure-final业务输出先于最终保存投影；保持检查并改受控状态轮询，最终通过。
- ASAN closure-final持久化摘要出现模型名offline-fixtur少末字符，3断言失败，无ASAN内存错误。后续同程序补项通过，但原原因未定位。该路径本轮宿主/运行时字节未改；作为独立非阻断风险保留，不能归因环境或声称已修复。若复审认为影响交付，应围绕该首次日志最小定位；不以通过覆盖首败。
- Python编码、错误路径、只读SQL列名及工具错误见driver-errors.md，事后说明不是原始日志。失败目录含上下文程序，不能把全部归档称为失败源码新链接的程序；以每轮清单为准。

历史30.08/30.06/30.11秒、候选绑定、复制error5、取证traceback以及启动/frontend/输入/browser首次失败继续独立分账。新通过不解释未知历史根因。

## 兼容、自审及交付

生产357导出与DH-1一致，无私有入口；runtime-preservation与fixed-dependencies核对保持。SDK0.10-dev/API10/标准10、ABI1/包1、公开布局、错误值、DDL、来源政策C2/P4/P5、50ms脚本、8MiB JS、128KiB模型、2MiB arena、8MiB Service、256KiB单包和原测试超时未放宽。配置隔离增加显式UI_BROWSER_SETTINGS_PATH；非法/过长拒绝，不回退旧用户路径，未设置时保留旧默认行为。没有第二执行引擎或AI写入。

[业务入口](../xuanshu-business-read-entry.md)、[合同](../xuanshu-business-read-contract.md)、[真实验证待授权包](../xuanshu-business-read-real-proposal.md)。start-readonly.ps1仅语法及身份检查，未启动人工验收。已确认范围缺陷修正，自动验证最终逐项通过；自审不等于独立复审。真实调用前仍需单独授权实现本场景请求总数硬门、核对当天协议/价格并绑定最终程序；当前不能直接凭人工计数发送。

清理仅本轮已无用途obj，依赖记录先保存；保留源码、匹配程序、日志、JUnit、必要失败及测试自有库，详cleanup.json/preservation.json。真实设备、IME/DPI/辅助技术/人工长时/专用断电未验，已取消临时页面控件测试不恢复，browser九项副作用另案。
