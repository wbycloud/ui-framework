# API8 稳定性与完整状态接入收敛

2026-10-08 本地执行，SDK0.8.0-dev／API8／标准8，应用ABI1／包格式1。接手实际为干净 `660734fa66a868478416f226696e4feb990e0b49`，保留原生捕获9df202f、异步轨道2e87e02、消息泵／活动实例就绪0fe0864。历史结果与失败未改写。本轮未推送、发布、触发远程工作流或创建稳定标签。

## 实现与身份

| 本地提交 | 改动 |
| --- | --- |
| f0b161cae20b405ab5217368f17b466e757d0655 | 两项已实际复现的共同模板修复：顺序正确的节点不重挂载，TABLE单元格按注册稳定列ID排序 |
| d24cdf386889337fc68d3aabb5184c39a5591a54 | 完整状态DLL／包、跨进程实际鼠标用例、重绘诊断，复用原四行CI并归档新日志／截图 |

公共头文件与原生输入产品源码相对接手未改，API8不升级。只维护当前版本，不增加旧SDK／原包专项。节点1024、缓存2MiB、图片／投递／批次、句柄及GDI／USER资源断言保持。具体文件身份见[产物清单](api8-stability-state-evidence-20261008/artifact-identity.json)，依赖与命令见[证据索引](api8-stability-state-evidence-20261008/README.md)。重新configure会改变生成头和PE时间信息，不能用二进制SHA变化猜测产品修复。

## 重绘触发链与未定位范围

新 `ui_host_repaint` 在真实独立宿主加载完整DLL，记录实际Dispatch的队列WM_PAINT、发送消息hook、绘制前后失效区、WM_WINDOWPOSCHANGED、宿主refresh、workspace wake、layout事件、进程CPU和句柄／GDI／USER。泵在取消息前检查3000条／100ms；已取出的普通消息全部Translate／Dispatch。最初只用发送消息hook漏掉队列WM_PAINT，主动失效正对照失败；[原失败](api8-stability-state-evidence-20261008/raw/state-light-and-repaint.log)保留，修正的是观察器。

源码链路：宿主changed／event回调合并为HOST_REFRESH_MESSAGE→layout/send_state；Light布局／invalidate调用InvalidateRect→WM_PAINT→BeginPaint／实际GDI绘制／EndPaint；窗口GL surface的invalidate→WM_PAINT，先验证失效区，再执行应用frame；OSMesa显式render执行实际应用frame／resolve／RGBA读回，再更新C图片和组件。没有删除必要GL刷新、丢弃消息、加sleep、放宽超时或阈值。

失效区应在实际绘制后核对；仅取出WM_PAINT不是Dispatch，SendInput返回成功也不证明状态已改变。依据[Microsoft InvalidateRect](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-invalidaterect)与[WM_PAINT合同](https://learn.microsoft.com/en-us/windows/win32/gdi/wm-paint)。

本轮完整场景正对照为1次paint、返回后失效区空；三个2秒闲置段均0次paint／refresh／layout，CPU0–15.625ms，句柄282、GDI414、USER31不变。resize每段8次paint，显式GL命令7次paint；随后2秒闲置再次0次paint，资源平稳。resize字体／布局缓存变化单列，不归因于历史GDI问题；原集成资源门槛另验。见[当前完整原输出](api8-stability-state-evidence-20261008/raw/matrix-provider-raw.log)。

历史1983条WM_PAINT／16.2秒现象**尚未在本轮场景复现，来源仍未定位**。两个独立进程追加观察另列日志；低闲置计数及已确认的DOM捕获缺陷不能解释所有历史重绘。不宣称根治，保留原诊断与取消息前边界修正。

## 完整状态示例与两项产品缺陷

[stateful_components](../../examples/stateful_components/README.md)复用框架维护的完整场景：十万行／64列、树、属性、枚举／RGBA、异步图片、模态和实际OSMesa MSAA／GL资源。应用自己存UCW1＋ULYT，稳定组件／列／面板和A/B档案跨进程存在；独占锁避免实例覆盖。文件不写入包／业务文档，mount后加载，用户显式保存，关闭不自动覆盖。UST1只是示例容器，不是新公共存储API。

真实SendInput按公共presentation的可见clip定位，view-local逻辑坐标经内容槽偏移／DPI／ClientToScreen变为screen；核对实际PID／命中，Light核对同HWND捕获，最终列宽必须改变。Runtime用DOM捕获，不能把UI线程GetCapture()==NULL误写为失败。新测试早期漏加main槽偏移、误断言Runtime HWND捕获及PowerShell保留变量冲突均保留原失败与修正，不算产品根因。

实际表头200→280后保存、关闭、独立进程恢复；B档案通过两次实际拖动200→280→360，A仍280。单列／全表重置、锁冲突、create／mount失败释放锁、缺失／新增／重排列、缺失／空／截断／超长／非法／未来文件、UCW1与布局异常均验证最终状态；布局负载失败时列宽回滚，期间不泵消息。c63保存210再删除，新added保持180，不预加载全数据。重载保留修改草稿、精确大于2^53的选择ID、内容HWND、实际GL显示列表及非空图像；WAIT清理后DLL卸载。

两个经测试先失败的产品问题：

1. [重排列红例](api8-stability-state-evidence-20261008/raw/reordered-alignment-red.log)：Name表头x128、同ID单元格x8，值仍editable。模板只按数据源数组顺序挂载；修复为按注册列ID排列原单元格，不改源／列宽／编辑／预算。
2. [连续调宽原事件链](api8-stability-state-evidence-20261008/raw/repeat-header-isolation-trace.log)：第二次真实pointerdown命中resize-name、column-width360已到框架，但可见列数改变后重挂载表头，触发lostpointercapture→column-cancel，宽度回280。修复只跳过顺序已正确的DOM挂载，真正捕获丢失仍取消；没有延时或取消断言。

临时trace已从产品移除；[诊断补丁](api8-stability-state-evidence-20261008/raw/temporary-header-trace.patch)可重建追踪，不冒充最终SDK。该临时DLL未独立留存完整哈希，诊断日志与补丁绑定，最终测试／SDK则完整冻结。截图文件现在按case标签区分，重排列不会覆盖正常restore图；原误标图留作失败说明。

## 当前运行结果

| 当前源码实际运行 | 结果与边界 |
| --- | --- |
| 本地完整矩阵 | **58通过／1物理跳过**，59项精确清单；WGL43＋跳过1、provider6、真实Runtime9；[汇总](api8-stability-state-evidence-20261008/raw/matrix.xml)、[分类检查](api8-stability-state-evidence-20261008/raw/ci-checks-final.log) |
| 完整存储两后端 | 最终矩阵内Light与真实Runtime各自完整跨进程套件通过；三个程序DPI、不同档案宽度、异常最终状态及重排列文字／对齐均验；首次[连续拖动失败](api8-stability-state-evidence-20261008/raw/state-isolation-final.log)保留 |
| 原生滚动及相关回归 | 完整矩阵原ui_component_scroll_native真实鼠标三项及重复／resize／焦点、菜单、输入／模态、布局／GL／关闭合同保持；原生独立配置及业务观察结果下节另报 |
| 原生独立配置 | **20通过／1物理跳过**，21项；[日志](api8-stability-state-evidence-20261008/raw/native-current.log)、[JUnit](api8-stability-state-evidence-20261008/raw/native-current.xml)，6.08秒 |
| 两个独立Runtime进程 | 各32个真实呈现就绪重开周期，句柄389→391、GDI12→12、USER17→17、末次pending0／逐轮DLL卸载；[运行1](api8-stability-state-evidence-20261008/raw/runtime32-1.log)、[运行2](api8-stability-state-evidence-20261008/raw/runtime32-2.log)。原+12／+4门槛保持，日志固定文字reopen8不是实际周期数 |
| 独立重绘进程 | [进程1](api8-stability-state-evidence-20261008/raw/repaint-independent-1.log)、[进程2](api8-stability-state-evidence-20261008/raw/repaint-independent-2.log)：实际失效正对照通过、每个闲置段0paint，GL后再次闲置0paint；不覆盖历史失败 |
| GL／OSMesa | 当前实际应用DLL、samples4／MSAA边缘及RGBA/C图片、旧非MSAA路径通过；Session1控制不能替代Session0服务 |
| 实际CI | 只完成本地四行选择／失败退出／精确清单检查；没有托管运行授权或托管运行证据 |

## 业务折叠定位与原失败

原业务仓库只读，实际HEAD为ff44a0fff0ba198f6bfc45d7fcdabb347f6fdeda且存在用户改动，不用历史干净状态代替。按已登记的失败基线另建abc8d870efc6401dc4fa55e50170a5449c11e36b源码／构建目录，仅修改清单API与SDK锁元数据；原业务代码、默认列宽、原固定坐标及断言不改。

当前源码副本首次完整套件仍**29通过／1失败kc_host**，[原输出](api8-stability-state-evidence-20261008/raw/business-current-full.log)、[JUnit](api8-stability-state-evidence-20261008/raw/business-current.xml)。最终产品再次匹配重建后的结果另列最终日志。旧位置(88,114)未命中调整后的折叠按钮，框架侧提供真实定位和迁移，不把失败简单归责业务。

最终产品源码重建完整套件仍**29/30**，292.60秒，[日志](api8-stability-state-evidence-20261008/raw/business-product-final.log)、[JUnit](api8-stability-state-evidence-20261008/raw/business-product-final.xml)。另以交付SDK逐字节相同的头文件和实际导入库重新链接应用DLL，复制同一共享DLL及宿主、重新打包后验证；[实际链接](api8-stability-state-evidence-20261008/raw/business-sdk-relink.cmd)、[匹配前产物身份](api8-stability-state-evidence-20261008/raw/business-source-build-hashes-before-sdk.json)区分配置，后续SDK完整套件结果不得沿用源码构建的292.60秒。

**实际交付SDK完整业务副本套件也为29/30，292.88秒**，[日志](api8-stability-state-evidence-20261008/raw/business-sdk-full.log)、[JUnit](api8-stability-state-evidence-20261008/raw/business-sdk-full.xml)。唯一失败仍为原kc_host，断言未改；重建源码模式、交付SDK模式和框架自有观察结果分开报告，不把29/30改成通过。

框架自有观察器持有自己的宿主view，通过公共presentation定位按钮→真实鼠标→实际状态检查，在96／144／192程序DPI及resize下折叠／恢复。必须同一HWND隐藏／显示，不销毁，原断言仍成立。独立真实滚动／业务GL及布局观察不覆盖原29/30。

[当前折叠日志](api8-stability-state-evidence-20261008/raw/business-fold-final.log)的12次真实操作全部通过，同一内容HWND与实际WGL context保留，旧logical(88,114)不在按钮clip(202,106,20,20)中，焦点有效。另[业务鼠标／布局观察102/102](api8-stability-state-evidence-20261008/raw/business-scroll-fixture-fixed.log)验证4201个实际Cell末项稳定ID8589938792／PAGE_4200、真实轮至first3后返回稳定首项8589934592／PAGE_0000；业务四列Layer表末列style按需请求到first_column3，实际非文字STYLE像素非空。框架原生夹具另验证64列末列63文字／稳定ID，不把业务STYLE伪造为文字。

观察器首轮102项中两项布局失败是工作目录遗漏只读layout-format1文件，[原失败](api8-stability-state-evidence-20261008/raw/business-scroll-missing-fixture-red.log)保留。复制源码中同一输入文件后所有原断言通过，[输入／原二进制哈希](api8-stability-state-evidence-20261008/raw/business-fixture-hashes.txt)明确，不修改业务测试或通过放宽断言取绿。

实际交付SDK重新链接的同一业务DLL也完成[102/102鼠标／布局观察](api8-stability-state-evidence-20261008/raw/business-scroll-sdk.log)与[12次折叠／恢复](api8-stability-state-evidence-20261008/raw/business-fold-sdk.log)，同HWND及WGL context保持。该观察器结果仍不覆盖SDK完整套件的kc_host失败。

迁移仅替换定位方法：取得合法所属view后，重查按钮可见clip中心，按公共view-local逻辑像素转换，核对命中PID，真实鼠标后检查collapsed、IsWindow／IsWindowVisible、同HWND与GL／焦点。外部启动kc_host没有公开的宿主chrome view getter；正式修改需获授权后接入框架维护的宿主观察器／测试适配，不能假造接口、恢复旧界面、加重复入口或放宽断言。[观察器源](api8-stability-state-evidence-20261008/raw/business-observe.c)与[迁移准备](../business-pilot.md)。

本轮独立观察首次在软件WGL默认Mesa D3D12路径发生C000041D，[异常与调用栈](api8-stability-state-evidence-20261008/raw/business-open-exception-trace.log)。同一观察二进制显式GALLIUM_DRIVER=llvmpipe后通过，[二进制哈希](api8-stability-state-evidence-20261008/raw/business-observer-trace-hash.txt)、[对照](api8-stability-state-evidence-20261008/raw/business-fold-explicit-driver.log)。这是部署条件对照，不宣称修复框架GL／重绘根因，不强杀Runtime或改系统GL。

## 复验与实际截图

MSVC19.50／C11／Windows SDK10.0.26100，Lexbor7fb22cf、QuickJS2f0aa72、WebView2 SDK1.0.4129.50／**实际Runtime154.0.4258.62**、Mesa24.3.4 x64。WGL测试使用应用目录软件llvmpipe；provider和Runtime阶段移除四个WGL DLL，OSMesa使用明确绝对路径。安装版本由Loader当前查询，不以目录猜测。

```powershell
# 在x64 VS开发终端，以当前仓库为工作目录
cmake -S . -B build/current -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_FRAMEWORK_ENABLE_WEBVIEW2=ON -DUI_OSMESA_LIBRARY=<绝对路径>/osmesa.dll
cmake --build build/current
ctest --test-dir build/current -R '^ui_stateful_components_(light|webview2)$' --output-on-failure
ctest --test-dir build/current -R '^ui_host_repaint$' --output-on-failure
./tools/test-windows-ci.ps1
```

完整矩阵按WGL／provider／Runtime隔离串行运行，准确参数见[run-matrix.ps1](api8-stability-state-evidence-20261008/raw/run-matrix.ps1)；测试结果非零、遗漏／重复／意外跳过必须失败。源观察器位于本轮build子目录，其相对include需保持对应源码路径；交付raw是只读证据，不当作旧程序执行。

同一完整应用、100k／64列、1200×800逻辑窗口、同DPI／浅主题的真实保存／独立重启恢复图见[索引](api8-stability-state-evidence-20261008/README.md)。PNG只做实际BMP的无损RGB转换；GL是应用实际OSMesa输出，不是模拟图。业务图是当前只读副本的新观察，不伪称原应用本轮前后对照。截图及程序输入不能替代真实IME、物理跨屏／桌面合成／长期人工。

## SDK与待验

匹配SDK包含include／导入及静态lib／共享DLL／宿主／完整stateful包／源码／固定依赖／许可／逐文件SHA256。归档`ui-framework-sdk0.8.0-dev-api8-f0b161c-windows-x64.zip`位于本轮本地build目录，62724776字节、2457文件；SHA256为`8e962a14f1fb9e77f5aaa8d4b4067688bef757f674bb34a1686c0969fa067dc2`。[归档身份与验证](api8-stability-state-evidence-20261008/raw/sdk-archive.json)、[逐文件清单](api8-stability-state-evidence-20261008/raw/sdk-sha256.json)、[SDK使用说明](api8-stability-state-evidence-20261008/raw/sdk-README.md)绑定产品f0b161c／示例测试d24cdf3及源码文档快照**ace533acde16171568dfa38f699387a6e79f8587**。本段及外部归档证明在该快照之后提交，仅增加交付证据，不重写归档以制造哈希自引用。

SDK随包固定依赖离线重建通过：[构建](api8-stability-state-evidence-20261008/raw/sdk-offline-build.log)，当前C头文件／列宽[2项](api8-stability-state-evidence-20261008/raw/sdk-offline-tests.log)及属性／选择体验[1项](api8-stability-state-evidence-20261008/raw/sdk-offline-experience.log)共3/3。第三项初始筛选名ui_properties不在清单中，没有算成3项，追加实际存在的ui_component_experience；清单绑定[当前测试计划](api8-stability-state-evidence-20261008/raw/sdk-offline-plan.json)。匹配头文件／导入库／DLL的C消费实际得到160列宽、120字节UCW1，[打包前调用](api8-stability-state-evidence-20261008/raw/sdk-consumer.log)和[解包后独立调用](api8-stability-state-evidence-20261008/raw/sdk-extracted-consumer.log)均通过。

归档CRC与解包逐文件SHA256全部通过。解包目录内的实际宿主／stateful DLL在Light及真实Runtime分别执行write、restore、isolation、profile-b，共**8个独立进程全部通过**：[完整日志](api8-stability-state-evidence-20261008/raw/sdk-extracted-state.log)、[产物／独立进程清单](api8-stability-state-evidence-20261008/raw/sdk-extracted-state-manifest.json)。真实鼠标200→280后持久化，另一进程恢复280及折叠；B连续拖动到360另存／重启恢复，A保持280。运行后2457个交付文件哈希再次一致。业务副本使用的共享DLL和宿主与该SDK逐字节相同，实际导入库重链及29/30结果如上。归档与解包实际命令保存在[打包脚本](api8-stability-state-evidence-20261008/raw/package-current-sdk.py)及[桌面复验脚本](api8-stability-state-evidence-20261008/raw/verify-extracted-state.ps1)，包中不含软件opengl32.dll，不修改Runtime用户目录。

历史持续重绘来源、当前源码服务Session0、严格整机无登录runner、托管CI授权、真实中文IME、不同DPI物理屏幕／屏幕边缘桌面合成及长期人工均分别核对。原业务正式接入与原测试修改无授权，原套件结果不得被观察器覆盖。没有实测OSMesa性能瓶颈或明确硬件需求，不扩大到新硬件无窗口后端。当前独立功能完成不等于全部验收通过。

最终[环境审计](api8-stability-state-evidence-20261008/raw/environment-final.json)：WinNT10.0.26300／交互Session1／2560×1440／系统DPI96／检测到1屏，DWM已启用不等于屏幕边缘／物理混合DPI验收。服务创建访问探测Win32错误5，未创建或修改服务；初版探测的字符串null／错误123已纠正为原生NULL并在P/Invoke内立即读取错误，123不作为权限结论。CIM读取拒绝，OS身份使用只读注册表／进程版本。原业务HEAD ff44a0f及23项用户tracked diff与接手一致。没有CI执行／原业务正式修改授权，也没有严格无登录runner；IME／物理／长期人工待验。授权后才将指定本地提交推至codex/menus-offscreen并运行windows-regression.yml；Session0需有权限的独立服务环境运行session0-osmesa.yml，不能继承历史成功。
