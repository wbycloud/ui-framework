# UH-A显隐、空视图、输入与标签限定整改验收

2026-11-03。开发、自审及离线自动验证记录；不是独立复审、正式冻结或真实人工验收。整改已完成，准备好在新授权后进行简短零Key、零联网人工复核；本轮没有启动该复核或生产宿主。

## 基线与身份

实际分支codex/p1-process-boundaries，HEAD e0eca03db25930be55bdb45477027d1793a61904，未提交。入口baseline.json先于本轮编辑复制6346份源码/文档范围文件；不是全部Git内容哈希，也不把HEAD到工作区全部差异归本轮。新根`build/xuanshu-uha-visibility-20261103-e57b29`。

原冻结buildgate identity B3351D1DB62743F5E5EFC67739976CB2A470B7D32C5855375B25B02A8C68007A，对应416源码、102程序入口核对匹配；原人工根xuanshu-uha-manual-first-20261103-5d8a41的105文件仅做字节哈希保全，包括库文件，没有打开/操作原库。新代码不继承旧独立复审或冻结结论。参考工程README可读，未修改或扩权。

最终完整源码、程序、增量、原始日志/JUnit及哈希以新根delivery-files/source-after/binary-files/delivery-identity为准；source-before是编辑前范围副本，source-final/delivery-final是本轮最终副本。最终生产候选（本轮未启动）：

- `framework_host.exe`：`D386E229872295CE604196AF456CD3B4FB3100F39687377D7012B011FA5F6E17`
- `ui_framework.dll`：`E0ECEF4362DA77690F849F8B9415BDF82C1640B860656DDEE531C18DFF6ADBE5`
- `rpc_memory.uapp`：`235F8CBEC2ED287F52AA05B5EEC92F3667D3344A4DF0A11A98BB262FA9F8769F`

## 现象、核实和最小修复

| 项目 | 直接证据与判断 | 修复及边界 |
| --- | --- | --- |
| 主区黑色 | 原人工截图黑区成立，但没有收起前完整窗口对照。默认workspace容器无背景刷/擦背景/绘制处理；无视图内存应用的WM_ERASEBKGND返回0、黑DIB不变，收起/尺寸改变后同样发生 | 默认容器按主题填充确定背景，在没有框架视图/表面/组件及原生子窗口时显示明确空状态；尺寸变化后正确失效，不周期全屏刷新。不能倒推原截图黑色开始时刻或唯一因果 |
| 助手残留 | 程序对照显示页面已隐藏、原生chrome区域已排除主区；有内容视图的背景、文字和hit标记保留。无视图底层不绘制是旧像素残留的可达缺口 | 修复底层绘制边界，保留既有显隐/工作区布局/区域更新。没有证据声称业务视图丢失，也不只藏右栏便宣称全部解决 |
| 输入命中 | 隐藏焦点仍接收TEXT并改变草稿；隐藏按压/捕获仍占用原生capture | Light样式计算后，仅对已经隐藏/删除的交互目标清理焦点、组合态、按压、滚动、捕获及相关输入序列；先清内部状态再ReleaseCapture，仍在原dispatch保护内。可见控件的稳定捕获不动 |
| 展示资格 | 旧文本在隐藏后仍可回读，原host ready为true；页面重建默认折叠，展开后无宿主刷新通知 | 顶部AI切换立即撤销权威资格，确认和绑定要求助手展开、每块实际布局非零及原文本/hash/代次一致。配置折叠/展开仅通知失效并重新读回，不自报成功或授权；解绑PENDING仍不准旧确认 |
| 输入辨识 | 四类框无常驻用途标签，原人工多次找错 | 增加任务输入、关联问题回复、stub可选资源ID、stub诊断脚本中文标签；按钮明确“提交stub任务”，说明本地保存模型提案和确认外发区别。原ID、草稿、编辑/图/命令诊断保留 |

产品仅改main.c、ai_model_panel.inc、host.html、workspace.c、light_web.c；CMake仅注册隔离用例。运行时改动有必要，补了真实DLL/静态嵌入/输入/生命周期/cleanup-only回归；Service、Provider、业务执行器、公开头及存储DDL未改。

## 修前证据和失败分账

| 阶段 | 结果和性质 | 材料 |
| --- | --- | --- |
| red首次编译 | 夹具shell参数类型不符，C4133/WX；修为公开ui_host_get_shell，不降告警 | red-light-build.log、failures/red-light-build |
| red-controlled | 5个隔离进程5项用例失败，分别5/1/4/2/4项，共16项断言：空背景、隐藏展示资格、隐藏输入/草稿、capture、标签 | light/red-controlled.xml、red-controlled-light-test.log、匹配源码和exe/DLL/uapp |
| fixed-direct | 基础5/5通过，中间程序不是最终六例证明 | light/fixed-direct.xml |
| selfreview构建/编排 | 加入窗口子类夹具后漏链comctl32；其后错误启动未链接程序造成6项Not Run，与产品缺陷分开 | selfreview-light-build.log、selfreview-light-test.log及归档 |
| enhanced | 5项失败：4项配置区重建默认折叠的旧期待、1项未驱动refresh便期待捕获回调；修正受控时序，保留关闭保护断言 | enhanced-light-test.log/XML/同期程序 |
| enhanced2/rebuild-check | 展开配置后未安排宿主重新读回，ready仍false；这是自审发现的恢复缺口，增加仅请求失效/刷新通知 | 相应日志/XML/程序，不作为用户已遭遇的事实 |
| closure Light | 50/53通过，旧rebuild/long/utf8夹具在重建后隐藏配置区便直接期待授权；现先断言隐藏拒绝且零发送，再实际展开完整预览后验证原正常确认 | closure-light-test.log/XML；未弱化展示或业务断言 |
| 首次ASAN closure-final | 48/51通过，3个新夹具stack-use-after-scope：host_action.id指向已经退出局部块的instance数组。延长测试数据寿命，不改产品以掩盖夹具错误 | closure-final-asan-light-test.log/XML、同期源码/程序/ASAN栈 |
| ASAN settled期限 | cr-scan为50通过/1超时（20.02秒）；已打印64槽/refs1保持、归还回收、1578项检查0失败及最终workspace关闭销毁断言完成，仍未在CTest期限内退出。原同程序另次scan-check在原20秒门槛通过；不能据此定位首超时或归因环境，独立保留期限风险，不修改产品/断言/额度/超时 | settled-asan-light-test.log/XML、同期源/程序、scan-check日志/XML |
| 辅助取证 | 非VS会话直接找ctest失败；一次打印中文diff因控制台GBK编码失败；CIM只读进程查询权限拒绝后改用Get-Process；不涉及产品数据损坏或测试通过。对应完整原始输出仅存在工具会话，后补说明不冒充原自动归档 | selfreview-notes.txt |

首个red的BMP未在runner中独立归档，被同名后续输出覆盖。首败同期源码、日志/JUnit及匹配程序保留。red-visual-reproduction明确是用原匹配exe/DLL/uapp的后补复现（3项预期失败），不是原首轮自动图像；后续runner才增加失败BMP归档。原人工截图完全保留。上述受控断言不外推真实用户越界发送、崩溃、永久死锁或数据损坏。

## 最终自动验证

| 最终运行 | 结果 | 说明 |
| --- | --- | --- |
| Native final | 23/23 | Service/control/storage/limits、静态嵌入、公共C/C++、真实workspace两分支、真实DLL worker/外部回调、RP只读/取消/回调、生产rpc_memory及静态10/r1 |
| Light settled | 53/53 | 新六例、原buildgate/完整预览/结构化选择/真实固定目标/长历史UTF-8/64槽查询诊断/重建、存储失败cleanup-only、输入/滚动、真实DLL及面板 |
| ASAN-Light settled＋另次scan-check | 50通过/1超时，补项1/1，逐项覆盖51/51 | 直接及相称路径重新插桩，包括本轮Light/workspace运行时和含生产宿主的隔离测试程序；不冒充未执行的全部运行时全面验收 |
| Light production | 2/2 | 生产DLL链接rpc_memory实际整数10、字符串r1、框架revision及静态分支 |
| Light copy-final | 1/1 | 另次正常用户上下文，人工未参与；仅自有脱敏诊断及哨兵检查 |
| ASAN-Light copy-final | 1/1 | 另次带匹配ASAN运行库PATH的正常用户上下文 |

这些是不同运行，不合并成首次单次全部通过。开测前source/binary清单、最终程序和最终源码关联见compatibility-coverage；所有首败保留，后续通过不覆盖。新直接六例使用公开视图/slot和实际宿主路径，包含已知背景/内容/命中标记与无视图对照、实际可见性/原生裁剪、滚动/输入焦点、稳定刷新保留capture、隐藏清理、连续展开/收起、尺寸变化、页面重建、草稿/选择/任务保持及捕获释放同步重入关闭。所有传输/凭据都是隔离替身，没有实际Key和Provider调用；程序输入不证明物理设备来源正例。

最终检查生产导出与原357项一致，私有窗口驱动/分配故障/可信事件夹具/探针未进入生产DLL。原SDK0.10-dev/API10/标准10、ABI1/包1、旧布局/错误、v1只读、C2/P4/P5、固定依赖、JCS/SHA-256及256KiB/128KiB/2MiB/8MiB/64槽和超时保持。

## 保全、未验与停止位置

只清本轮新根可验证范围的obj及无用途重复过程图像（若有，按cleanup记账）；保留必要失败材料、匹配程序/PDB、日志/JUnit、自有临时库、源码和必要BMP。原人工105文件、旧源码/程序/SDK和根规划前缀保全见preservation；原库本轮从未打开。final-readback只是封装后的哈希核对，不是新测试或独立验收。

黑区/残留的源代码可达绘制缺口与隐藏输入缺陷已修复；原人工事件的完整时序和全部唯一因果仍无法追认，须短人工复核。真实设备/生产来源链、Windows动态正例、IME、DPI、视觉、辅助技术、人工长时未验，物理断电仅专用环境后续。本轮cr-scan20.02秒退出期限和原ASAN缓存30.08秒、启动/期限/frontend/输入/browser/ASAN-Light等首败继续独立分账，不默认环境、不宣称未知根因已定位；browser九项副作用另案。旧真实服务/T3远端费用unknown/483 Tokens差额保持，不追加请求。

本轮范围内必须修正项已清零，已具备限定独立复审及简短零Key、零联网人工复核条件；不是自行独立复审通过、正式冻结或人工通过。复核只准备：展开→输入草稿→收起→检查主区→再展开→检查草稿→正常关闭；具体待授权入口见人工清单。本轮不启动、不要求用户操作、不进入其他阶段。


## 2026-11-03限定独立复审追记与本次短复核授权

上一轮限定独立只读复审已确认整改闭环、限定验收通过、匹配FF84C151A97784832D932AAE72F55BF9107DDE7E81DFF97F006FF41887664AB0快照可以冻结，必须修正项为空。本轮仅核对源码/程序/清单并[追记冻结](xuanshu-uha-visibility-independent-review.md)，没有重新测试或完整复审；原离线副本、首败、BMP缺失限制及ASAN20.02秒/30.08秒风险不改写。用户另授权本次简短零Key/零联网复核，尚未预判结果，详[逐步记录](xuanshu-uha-visibility-manual-validation.md)。新文档状态与原离线identity分开，未commit/tag/release。


## 2026-11-04简短人工复核收口（不重跑离线验证）

冻结身份保持FF84C151A97784832D932AAE72F55BF9107DDE7E81DFF97F006FF41887664AB0；本次417源码和153程序末次匹配。用户按单步指引完成显隐、草稿、主区空状态、重新展开编辑及正常关闭，PID80044实际exit0；关后本轮库任务/消息/事件/工具均0，无模型配置，SHA256 502CC17D49118C44F7FA441AA035598CF38D1A7478ABAECAFFE7AC1843B04C6B。简短零Key、零联网人工复核通过，详[独立人工记录](xuanshu-uha-visibility-manual-validation.md)。不等于完整人工/真实Provider/系统验收；无全程网络遥测，不虚构HTTP计数。首次45096退出码未知、原BMP和ASAN期限风险保持；不修改原离线副本/清单，也不追加测试、真实请求或阶段。
