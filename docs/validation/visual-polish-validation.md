# 完整窗口视觉打磨验收

2026-10-07，本地分支 codex/menus-offscreen，接手干净 HEAD 43587c3e8e0cb4549da314e6c7e15a4654a10100，SDK0.7.0-dev/API7/标准7、ABI1。前轮原生修复9df202f/2e87e02保留；当前版本维护政策不变。没有代理、graph-engineering、新会话、推送、远程CI、发布或标签。

## 范围与具体改善

| 界面 | 问题与实现 |
| --- | --- |
| 公共规则 | 中性浅深表面继续使用，正文Segoe UI 13、标签/表头12，Windows提供中文字体回退；重复值仍在src/visual.css，没有CSS变量或新主题系统 |
| 标签与工具 | 显式零内边距图标，名称内距10、可截断；活动AI强调；窗口按钮命中和关闭合同不改 |
| 工作区 | 面板标签位于工具前，分组标题不再重复占位，按有限页成员分配宽度；20像素浮动工具置于28像素标题内；状态栏4/8内距 |
| 属性 | 无标题不占24像素；足够宽的面板标签80与值同行，窄窗/多行/长标签/图片堆叠；单位16像素；颜色箭头与文本同一行，展开保留RGBA、透明度、预览、撤销与提交 |
| 表格/树/列表 | 单元格与树行32、纵向居中；表头与行同一列宽约束，避免Runtime默认flex收缩；图片24、STYLE按列宽保持比例、勾选24；不恢复分页 |
| 图片与只读 | C图片尺寸作为私有模板元数据，IMAGE按可用区域保持比例显示；无可编辑字段和提交/取消命令的只读表单隐藏无效操作栏；对话框取消始终可访问 |
| 反馈 | 空/加载、异步图片、错误边框/文字、选中背景与焦点、禁用、pressed分别表达；页脚显示可见范围、完整total与完整列数 |
| 菜单/助手 | 弹出标记、文字、箭头20像素对齐；装饰子节点复用所属行hover，disabled不能执行；AI初始收起与高级折叠不改语义 |

行高32、轨道12、最小滑块24及BigInt映射未更改。TREE的empty-state仍为rows内绝对覆盖层；原生GetCapture相同HWND时不重复SetCapture。节点1024、缓存2MiB、JS8MiB/50ms、图片/投递和句柄/GDI/USER门槛不增加。每字段增加一个值容器，仍以原节点预算验收。公共12个头文件、枚举、完整size读取、输入坐标、线程/所有权和生命周期不变。当前API7应用提供真实total与按first/count查询，无需复制内部模板或改业务；不保证历史SDK/包兼容。

## 环境与来源

Windows x64 26300、Session1真实输入桌面；MSVC19.50、Windows SDK10.0.26100.0、C11，CMake/CTest4.2.3-msvc3。Lexbor 7fb22cf5664a331d7c24b113489e566767c9c25a、QuickJS-NG 2f0aa72a6b09cf69ff06399bcf9f4c083ac1a278、WebView2 SDK1.0.4129.50。最终配对Runtime使用官方Loader实测154.0.4258.62（HRESULT0）；之前的独立周期日志保留原环境信息，不由注册目录推断具体加载版本。OSMesa与系统WGL字符串见identity及原日志。显式OSMesa24.3.4；新目录没有部署Mesa WGL DLL。真实Runtime及SendInput/桌面BitBlt需正常交互桌面权限。

新构建目录 build/visual-polish-20261007。before产品源锁43587c3，新增红断言和显示检查以独立测试修改记录，不把其标为纯净源码。after按最终源哈希编译；准确提交、运行计划、二进制和截图身份另存。本机没有GitHub run ID，不算托管CI。

框架api7_fixture.uapp未修改：两个实例、100000行/64列、超过2^53稳定ID、属性/颜色、异步缩略图、实际OSMesa frame和关闭卸载。framework_features.uapp补充实际WGL浮动视口/工具/样式/勾选观察；其应用自有历史标题和“API3”状态文字不是公共版本声明。

请求方业务仓库只读源码实际HEAD为abc8d870efc6401dc4fa55e50170a5449c11e36b（与先前a727不同），原仓库已跟踪文件无改动；已有未跟踪历史证据原样保留。Git archive独立副本分别与435基线及新框架构建，读取副本precision-official.gds，经原权限确认接口打开；不保存业务文档，不改原仓库/冻结SDK/原包/证据。这是只读视觉观察，不冒充完整业务试点验收。

## 原失败与修复

| 运行 | 实际结果及解释 |
| --- | --- |
| Light新布局红例 | 10失败：空标题、字段对齐、只读操作栏和GL预览不足；完整窗口截图保留 |
| Runtime沙箱 | 完整窗口67失败，简单render超时；ready0/nav0/HRESULT0，无创建完成回调，不算产品成功 |
| 同程序正常桌面 | ready1/nav1/HRESULT0、pending cleanup0；原Runtime新布局红例12失败，真实before另存 |
| 像素用例校正 | 未设视口、单body子节点填满、焦点框/ClearType统计的早期诊断保留；有效同用例原版箭头extent8..10、5亮度像素失败，新版4..15、22通过 |
| 首轮Light after | 2失败：缺省命令复制成拥有的空字符串，不能以非NULL决定操作栏；改为首字节判定后通过 |
| Runtime列对齐 | 原before及首轮after表头145/内容142；列宽min/max约束后两后端对齐 |
| 首轮完整矩阵 | 51通过、2失败、1物理跳过；menu_desktop与menu_cascade_offscreen失败原日志保留 |
| 菜单定位/修复 | 恢复旧子节点几何可通过；对齐后指针命中装饰子节点，light mouseenter不冒泡。子节点复用所属行hover后4专项通过，128项118分页步、级联13层止于原预算、命令一次 |
| 桌面捕获 | PrintWindow省略业务GL、沙箱BitBlt失败均不算GL证据；正常桌面BitBlt实际GL像素before/after另存 |

## 最终结果与准确身份

产品提交95f5d7813f61195bd52f7946091ac6384f3fba59、6a0170a4f263bc08a004134eb958bf299219d318。最终完整矩阵运行前保存的12个产品/测试/CI源文件SHA256，与提交后文件逐项相同；文档提交不改变运行源码。公共12头文件与接手基线无差异。

| 当前源码运行 | 结果与证据文件（原字节ZIP） |
| --- | --- |
| 完整框架矩阵 | 54项：53 PASS、1物理跨屏SKIP、0 FAIL，159.86秒；current-matrix-final.log/.xml、current-plan.json、after/LastTest.log |
| 最终完整视觉DLL | Light和真实Runtime分别0失败；两主题、96/144/192程序DPI、双实例、窄窗、RGBA/枚举、AI尺寸/目标保持；screens-light-final.log、runtime-screens-final.log |
| 原生真实鼠标 | TREE/LIST末项5000、真实滚轮first3回首ID1；64列TABLE末列63实际查询/文字/非空clip；重复/resize/焦点/取消/异步轨道/关闭、96/144/192均0失败；native-final.log |
| Runtime独立进程 | A、B各32次实际ready重开，均0失败；句柄398→400、GDI12→12、USER17→17、最终pending cleanup0；runtime-independent-32-a/b.log。日志旧汇总标签reopen8不是次数，以0–31逐周期和运行环境UI_RUNTIME_CYCLES=32为准 |
| 业务副本完整回归 | abc8源码与当前框架，29 PASS/1 FAIL，274.33秒；business-current.log/.xml。kc_host的原固定坐标(88,114)未命中新右侧折叠按钮；独立重跑20.18秒仍同一失败，business-host-independent.log/.xml |
| 公共坐标业务观察 | 新编译框架自有观察器，当前应用DLL，真实鼠标(260,156)折叠/恢复保留同HWND，实际2Cell/4shape GDS、GL截图、AI和关闭0失败；business-fold-desktop-final.log |
| 业务滚动/布局观察 | 4201稳定Cell/300层、末列style，95检查先通过；追加真实滚轮first3后鼠标回首，新匹配SDK观察器100检查0失败；business-native-scroll-wheel3-final.log。末项ID8589938792文字PAGE_4200、首项ID8589934592文字PAGE_0000及非空clip实际可见 |
| GL及资源 | 完整矩阵实际OSMesa llvmpipe LLVM19.1.7、Mesa24.3.4、samples4/edge_pixels123；系统WGL Intel UHD770/32.0.101.7079，MSAA与原samples0均通过，资源断言未提高 |
| CI契约 | test-windows-ci.ps1本地PASS：当前版本、准确清单、provider/Runtime分区、失败码及仅物理跳过；ci-contract-final.log。不等于托管CI运行 |
| Session0条件 | 非管理员/无服务管理权限；现有脚本在权限门控抛错，未创建/修改服务；session0-rights.log。Session1控制通过不替代当前Session0 |

补充诊断也保留：一次观察器传入不存在的GDS路径，打开返回OK但实际为空模型；已增添真实路径、4shape及modified=false断言，错误截图只列diagnostic。一次业务观察从仓库根而非规定构建目录运行，文件选择取消导致两项格式1恢复断言失败；在规定目录复跑100/100，未改产品/业务或断言。业务完整套件原生成的锁身份仍打印43587c3，实际CMake源码override及二进制哈希另证当前6a0170a；没有篡改原应用锁文件。框架自有观察器明确打印实际override。

业务固定坐标用例没有修改，不能记为全套通过；当前公共presentation真实命中复验支持实际折叠功能。业务副本和框架自有观察器仅作本轮集成/只读观察证据，完整业务试点与长期操作另验。Runtime多个进程通过支持当前关闭合同，不把历史资源波动宣称根治。

交付检查发现依赖DLL索引曾误按UTF8文本读取；已改用原字节、保留诊断索引并逐项复核，不是DLL或依赖发生变化。最终747个本地文档链接有效，12个运行源码、12个公共头文件、31个产物/依赖/观察器、8个只读文件及331个ZIP原文件与48张PNG哈希复核通过。Git archive的LF与Windows CRLF按Git内容比较，同时保留当前产物原字节SHA256。

产物、12头文件、24组48张无损配对PNG和ZIP每文件SHA256见[证据索引](visual-polish-evidence-20261007/README.md)。本轮框架共享DLL SHA256为43c80d5de399f7d3753730942bc95295e6213fbd38ff39ddeed53fcfba3a0948；应用副本包8757c1cafb4058e15f64b89e526b8c055b8b54a2c51005f4e4b1249dd28df3fd。

## 真实截图

同应用/数据、1200×800外窗、96程序DPI、同主题配对。Light与Runtime字体和图片插值允许不同。高DPI截图和输入不等于物理跨屏；窄窗、AI、RGBA、枚举等附图见证据目录。GL来自实际DLL frame，未生成或替换业务图片。

[轻量before](visual-polish-evidence-20261007/screenshots/before-light-host-light.png) · [轻量after](visual-polish-evidence-20261007/screenshots/after-light-host-light.png) · [Runtime before](visual-polish-evidence-20261007/screenshots/before-runtime-host-light.png) · [Runtime after](visual-polish-evidence-20261007/screenshots/after-runtime-host-light.png)。

[业务真实GL before](visual-polish-evidence-20261007/screenshots/before-business-light.png) · [业务真实GL after](visual-polish-evidence-20261007/screenshots/after-business-light.png)。业务GL画面及配色由应用负责；完整框架WGL及深色/窄窗/助手附图另存。

## 复现与限制

新目录使用固定依赖、真实Runtime与显式OSMesa，当前CTest清单不使用--baseline。视觉程序参数见[构建说明](../build-and-validation.md#框架视觉复验)。Runtime沙箱未完成创建时记录ready/navigation/HRESULT及环境，再以同二进制正常桌面对照；不提高timeout、强杀Runtime或清理用户数据掩盖问题。SendInput串行验证可见滑块、窗口/PID/捕获、稳定ID/文字/非空clip与末列查询，不能用Home/End/离屏输入替代。

严格无登录runner仍无；当前源码Session0服务、真实中文IME、不同DPI物理显示器、屏幕边缘合成/实际Snap、长期人工和完整业务试点待验。历史Session0及当前Session1控制不替代当前Session0。没有OSMesa实测瓶颈或明确硬件需求，不扩展硬件无窗口GL。无远程授权，CI配置与本机结果不等于托管CI成功。
