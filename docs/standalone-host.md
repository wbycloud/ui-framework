# 独立宿主使用说明

当前为SDK0.7/API7。浏览器式外壳属于宿主内部实现，没有新增公共接口；旧应用仍按原菜单、内容槽、助手和生命周期合同运行。[实际验收](validation/browser-host-validation.md)。

## 1. 打开、标签和菜单

第一行依次是最近应用向下箭头、实例标签及各自关闭按钮、加号、空白拖动区、AI和三个窗口按钮。再次打开同一包创建独立实例，标签包含实例ID。第二行显示活动应用注册的菜单，右端“⋯”提供打开、关闭当前标签、前后切换、主题、重置当前应用布局和退出。

加号或空宿主的“打开应用包”调用Windows文件选择器。Ctrl+Shift+O打开包，Ctrl+W关闭活动标签，Ctrl+Tab / Ctrl+Shift+Tab切换；Ctrl+O留给应用。关闭最后一个标签回到空宿主，窗口退出仍先请求应用关闭。

标签正常宽88–180逻辑像素。不足时显示活动标签所在的一组，“…”按每页8项列出所有当前实例；极窄时全部从该入口访问。它与左侧最近历史分开。关闭中的标签和列表项禁用。交互缩放最小尺寸320×240逻辑像素；程序设置更小尺寸时尽力裁剪，不承诺任意小窗所有控件同时完整显示。

应用菜单继续复用分组、侧向嵌套、悬停切换、窄窗“更多”、分页、Alt/助记键/方向键/Enter/Esc及一次语义命令。切换、关闭、模态和失效取消旧菜单。独立菜单窗口覆盖原生/GL内容，注册ID和命令参数不变。

## 2. 窗口与助手

标题空白区由Windows处理拖动、双击最大化/还原，边缘和角落由系统缩放。交互控件按实际DOM矩形命中；最大化按钮返回HTMAXBUTTON，轻量子窗口对非客户区命中透明。保留窗口缩放/系统菜单/最小化/最大化样式，通过NCCALCSIZE移除传统标题区域。最大化客户区使用所在显示器work area，DPI变化重新布局并关闭旧弹窗。标题空白右键及Alt+Space打开Windows系统菜单。

Windows11 Snap布局依赖系统版本、桌面和设置；实现遵循[Microsoft自绘最大化命中要求](https://learn.microsoft.com/windows/apps/desktop/modernize/apply-snap-layout-menu)，HTMAXBUTTON断言不能当成实际Snap浮层验收。[自绘框架规则](https://learn.microsoft.com/windows/win32/dwm/customframe)说明传统标题区域处理。物理边缘和桌面合成另验。

AI显示/收起既有助手并重算内容尺寸。每次启动默认收起，点击AI后展开；手动选择在当前宿主有效，调整窗口大小不会自动展开，窄窗侧栏可能占据大部分内容宽度。固定目标不随活动标签改变；目标失效沿原逻辑选择有效实例。权限、确认、事务和结果合同保持。主题控制框架外壳和菜单，应用内容主题由应用管理。

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
