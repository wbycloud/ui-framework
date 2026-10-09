# 独立宿主使用说明

当前为SDK0.9/API9。浏览器式外壳属于宿主内部实现，没有新增公共接口；当前API9应用按现行菜单、内容槽、助手和生命周期合同运行；历史兼容维护已按[版本政策](version-policy.md)撤销。[浏览器式宿主验收](validation/browser-host-validation.md)与[本轮视觉验收](validation/visual-ui-validation.md)。统一数值及后端边界见[视觉规范](visual-design.md)。

## 1. 打开、标签和菜单

第一行依次是最近应用向下箭头、实例标签及各自关闭按钮、加号、空白拖动区、AI和三个窗口按钮。再次打开同一包创建独立实例，标签包含实例ID。第二行显示活动应用注册的菜单，右端“⋯”提供打开、关闭当前标签、前后切换、主题、重置当前应用布局和退出。

加号或空宿主的“打开应用包”调用Windows文件选择器。Ctrl+Shift+O打开包，Ctrl+W关闭活动标签，Ctrl+Tab / Ctrl+Shift+Tab切换；Ctrl+O留给应用。关闭最后一个标签回到空宿主，窗口退出仍先请求应用关闭。

标签正常宽88–180逻辑像素。不足时显示活动标签所在的一组，“…”按每页8项列出所有当前实例；极窄时全部从该入口访问。它与左侧最近历史分开。关闭中的标签和列表项禁用。交互缩放最小尺寸320×240逻辑像素；程序设置更小尺寸时尽力裁剪，不承诺任意小窗所有控件同时完整显示。

应用菜单继续复用分组、侧向嵌套、悬停切换、窄窗“更多”、分页、Alt/助记键/方向键/Enter/Esc及一次语义命令。切换、关闭、模态和失效取消旧菜单。独立菜单窗口覆盖原生/GL内容，注册ID和命令参数不变。

## 2. 窗口与助手

标题空白区由Windows处理拖动、双击最大化/还原，边缘和角落由系统缩放。交互控件按实际DOM矩形命中；最大化按钮返回HTMAXBUTTON，轻量子窗口对非客户区命中透明。保留窗口缩放/系统菜单/最小化/最大化样式，通过NCCALCSIZE只移除传统标题区域，普通窗口保留系统左/右/底边缩放框，避免原生内容子窗口遮住外缘命中。最大化客户区使用所在显示器work area，DPI变化重新布局并关闭旧弹窗。标题空白右键及Alt+Space打开Windows系统菜单。

Windows11 Snap布局依赖系统版本、桌面和设置；实现遵循[Microsoft自绘最大化命中要求](https://learn.microsoft.com/windows/apps/desktop/modernize/apply-snap-layout-menu)，HTMAXBUTTON断言不能当成实际Snap浮层验收。[自绘框架规则](https://learn.microsoft.com/windows/win32/dwm/customframe)说明传统标题区域处理。物理边缘和桌面合成另验。

AI显示/收起既有助手并重算内容尺寸。每次启动默认收起，点击AI后展开；手动选择在当前宿主有效，调整窗口大小不会自动展开，窄窗侧栏可能占据大部分内容宽度。固定目标不随活动标签改变；目标失效沿原逻辑选择有效实例。权限、确认、事务和结果合同保持。普通助手视图显示可读操作、受支持基本参数、状态及结果；事务/快照、schema、原始JSON和详细日志位于初始折叠的“高级”区，复杂参数与无法无损回填的数值JSON有明确的高级编辑入口；无效基本草稿不丢失，修正前禁止执行旧JSON，合并参数保持原4095 UTF-8字节预算，原权限/校验/事务路径保持。主题同步框架外壳、共同组件、菜单和Web浮动标题；应用自有HTML、原生控件及GL内容主题由应用管理。系统控件仍按Windows实际外观显示。

窗口关闭继续遵守REFUSE/WAIT和异步完成：拒绝保留窗口和实例；等待显示关闭中并继续泵消息，完成后才unmount/destroy、module_shutdown和卸载。未保存内容确认归应用原回调，外壳不猜测业务文档状态。

## 3. 最近成功打开的应用

宿主在用户本地状态目录跨重启保存记录：

```text
%LOCALAPPDATA%\UiFramework\host-recent-v1.bin
```

身份为GetFullPathName规范化绝对路径，以Windows序数不区分大小写比较，折叠相对/点路径；不按app_id合并，也不把硬链接、符号链接或8.3别名视为同一文件。workspace_open实际成功后置顶，同路径去重，最多12项，新项淘汰最旧项。同包多实例只一条记录，不保存标签会话或业务文档。

格式1：UHR1魔数、little-endian uint32条数，然后每条uint32 UTF-8长度和非NUL路径字节。每路径1–4095字节，总文件最大49196字节。条数、长度、UTF-8、绝对路径、重复和尾随字节完整校验；未来/异常格式整体忽略并记日志，不删除原文件。下次成功打开才重写。同目录进程专属临时文件写完后替换；并行宿主最后保存生效，不合并各进程内存记录。目录/写入失败保留当前打开和进程记录并提示日志；有效包路径超过历史上限仍可打开，但不记录。

空列表显示“尚无成功打开的应用”，每页8条，以包文件名显示。点击重新验证原路径，不寻找移动文件或改包。移动/删除、无效包及初始化失败均显示加载器错误，不置顶、不新增；已有失效记录保留，失败不伪装成成功。

## 4. 复验

按[构建说明](build-and-validation.md)准备依赖，在可交互Windows桌面执行：

```powershell
cmake --build build/web-shell
ctest --test-dir build/web-shell --output-on-failure -R '^(ui_browser_host|ui_web_host_frontend|ui_menu_desktop|ui_framework_features_host)$'
& .\build\web-shell\framework_host.exe .\build\web-shell\web_counter.uapp .\build\web-shell\web_counter.uapp
```

ui_browser_host使用框架测试包和私有临时历史目录，保留GetCursorPos/SendInput及窗口归属断言；测试不改用户最近记录或请求方应用。真实IME、不同DPI物理屏幕、边缘Snap/DWM及长期人工仍待验。自动截图、程序DPI和独立DLL不替代这些证据。

共同树/列表/表格移除分页和列箭头按钮，完整范围滚动及按需数据查询保留。宿主菜单、最近应用和标签溢出列表是独立入口，其分页仍保留；不将两种列表混同。[本轮真实前后图](validation/component-scroll-only-validation.md)。

## 完整窗口视觉打磨

当前宿主继续单行标签标题栏。零内距MDL2图标与名称截断、20像素面板标题工具、位于工具前的有界停靠标签、助手展开高亮和状态栏内距统一；AI仍只由用户切换。共同属性区自动同行/堆叠，实际C图片按比例显示；空标题和只读无操作表单回收空间。主题只作用于框架界面，不改变应用GL/业务配色、内容槽或生命周期。[本轮实际Light、Runtime与只读业务GL对照](validation/visual-polish-validation.md)。

## 当前体验与长名称

面板标题、停靠标签悬停或F1显示完整名称，Esc关闭；溢出标签按面板宽度分页，所有面板仍可激活。窗口最小尺寸/标题工具/空心停靠预览见[布局](workspace-layout.md)。AI仍每次默认收起；展开状态只随用户操作改变，resize或实例切换不会自动打开，原生及GL内容使用重排后的槽尺寸。SDK8的新列宽能力不更改宿主或应用生命周期合同。

## 完整状态示例与测试定位

打开框架提供的stateful_components.uapp，可在State菜单保存／恢复列宽和布局，关闭后重新启动验证；AI仍默认收起。状态归示例应用档案管理，不与宿主最近应用记录或业务文档混用。详见[示例](../examples/stateful_components/README.md)。

宿主按钮 geometry 可能随宽度、DPI和界面调整变化。框架维护的宿主测试持有自己的Web view，使用公共presentation可见clip中心、view-local→physical→screen换算和实际鼠标状态检查。应用没有公共“取得宿主chrome view”接口，不能照抄私有h.view或假造API；正式业务测试需由框架宿主观察器提供定位支持。旧固定坐标失败及原“隐藏而不销毁HWND”断言保留，迁移只替换定位方法，见[本轮验收](validation/api8-stability-state-validation.md)。


## 应用语言跟随（API9）

语言入口在应用，不在宿主全局设置。有应用时读取活动host，后台变更隔离；切换/关闭活动标签更新文案，最后一个关闭后恢复Windows显示语言。最近、空状态、工具提示、固定错误和助手使用框架统一资源。用户路径、参数和已有结果不翻译；助手确认按目标实例打开时快照。Windows文件选择器系统按钮依旧使用系统语言，其应用包过滤标签属于框架。实际识别时机和模态策略见[语言合同](instance-language.md)，[本轮证据](validation/instance-language-validation.md)不继承历史通过。

[2026-10-09当前源码复核](validation/instance-language-current-validation.md)补充完整示例双实例、实际Runtime及同状态中英文截图。权限确认已打开时保持目标实例的语言快照，后来活动标签或后台实例改语言不会改写该确认；无实例时按实际Windows UI查询，不以输入法/地区格式代替。
