# 剩余P2实施合同与分批验收

状态：实施规格，2026-10-23；先设计后实现。不署名。整体服务架构仍Proposed；本轮授权至代码/自动验证和集中人工验收准备，不执行物理验收、不进入P3。

## 1. 基线和交付顺序

实际分支codex/p1-process-boundaries，HEAD e0eca03db25930be55bdb45477027d1793a61904。新根build/xuanshu-p2-remaining-20261023-b84e6f记录17381文件初始身份，包含前轮交付和忽略的根规划。前轮delivery35/source242/binary392全部匹配。参考工程访问仍被拒绝，不重复申请或阻止独立实现。

顺序：C2复审及HMENU → stateful业务适配 → EDA实际对象/笔记 → generic/framework_features两个分支及counter → 联合回归/整改 → 匹配二进制与集中人工清单。每阶段先记录必要增量再实施，原主线规划只追加。旧证据和首次失败独立保留。

## 2. HMENU-1（对应实现前冻结）

问题：公开shell_process_message/WM_COMMAND既可程序调用又无匹配物理动作证明，不能补签资格。保留现有C2A菜单binding，不增加公开mint/dispatch或第二目录。

选择：框架为自己拥有的原生HMENU设置MNS_NOTIFYBYPOS，并安装菜单owner私有subclass、仅本UI线程的WH_MSGFILTER。过滤器只处理本框架当前菜单循环的MSGF_MENU；按当前Windows来源分类记录鼠标按下/释放、Enter/Space或唯一助记字符的匹配leaf候选。owner的WM_MENUCOMMAND必须匹配实际HMENU/position/native ID、循环和候选，先消费再进入原ui_action。WM_COMMAND、直接调用公开process_message、伪造WM_MENUCOMMAND、系统/未知/普通注入均不能自行产生候选。

点击开启后释放于菜单项的拖选，只允许由私有原生HTMENU按下的合格激活链延续；中间任何不合格输入清链。键盘匹配当前选择或当前菜单唯一助记项，不将普通导航键本身当提交。退出、菜单重建、切换owner/active实例、失焦及取消清除候选；重入新输入使旧call最后门失效。WM_MENUSELECT只提供菜单选择身份，不授予资格。源码采取fail closed；真实Windows消息顺序/来源是否符合预期留集中物理验收，不用逻辑钩子冒充已验证。

菜单循环/处理和应用回调借用host/shell，refresh/destroy在借用中拒绝或延后，UI不等待worker。同一菜单binding仍复制目标、epoch/revision，元数据/能力/call预算不增。菜单是框架拥有的固定结构，应用不能通过任意HMENU句柄注册权限。

自动验证：真实菜单句柄及MNS_NOTIFYBYPOS、无候选/错菜单/错项/重复/退出/混合来源/公开过程拒绝；专用私有逻辑夹具验证匹配候选可执行及最后门。不会把该夹具记为physical正例。人工清单覆盖鼠标、拖选、Alt/方向/Enter/助记符及未知来源拒绝。

依据：[Microsoft WM_MENUCOMMAND](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-menucommand)、[菜单通知](https://learn.microsoft.com/en-us/windows/win32/menurc/about-menus)、[线程消息过滤](https://learn.microsoft.com/en-us/windows/win32/winmsg/about-hooks)。它们说明菜单句柄/position与模态过滤机制，不提供物理来源证明；继续沿用Windows分类与同进程合作边界。

## 3. 后续批次约束

保留SDK0.10-dev/API10/标准10、ABI1/包1、v1/C2A-1结构语义和错误、固定依赖/yyjson/JCS/SHA-256及原限额。非read仅人工；一个权威目录/业务执行器，界面收参不复制业务实现。view/draft不升业务revision，write必须回读；UNKNOWN不被取消/forget/重登记消除。

stateful的单元格最小修改表及共享name/mode/color在应用内，原api7测试夹具不改成业务系统。保留UST1/UIL1，不新增文档格式。EDA仅原固定矩形/缩放/清空，加稳定ID和笔记提交/取消，不开放任意几何。已有局部状态文件功能逐项盘点，不因迁移消失；涉及超出冻结效果/生命周期的真实冲突单独记录方案。

五维护示例逐项建立功能/UI/能力/业务回读/测试矩阵；共享源码两个分支分别验证。WebView2仍排除，未适配入口拒非read。后续必要的新独立接口在本文件追加明确声明和验收后才实现。

## 4. CI-1 动态标准组件绑定（对应实现前冻结）

实际十万行表格/树不能给每个DOM行预注册独立绑定。新增独立capability_component_inputs.h；ui_capability_component_input_bind函数，描述为size/version=1、component、selector、capability_id、resource、collect、observe、user_data。selector只允许框架模板的CELL_EDIT、ROW_RENAME、ROW_SELECT、COLUMN_SORT、ROW_CONTEXT；不接受任意正则/页面自报动作。返回同一UI binding ID，解绑沿用C2A；没有业务handler或权限字段，不修改既有input结构。第二个只读函数ui_capability_component_editor_state(host,binding_id,node_id,out)返回独立size/version=1的generation/conflict/base_revision[128]，不提供清门或重置权限。

实际私有事件命中edit-ID-column、rename-ID、cell-ID-column、sort-column、tree-label-ID等模板节点后，框架提供原input context，control_id为本次实际节点，value为复制快照，ID以十进制字符串保持64位精度。collect需表达完整业务参数，目标仍是绑定的实例/资源/epoch，不能从活动标签重选。Enter提交编辑/重命名，Escape明确取消该局部草稿；选择/排序/菜单保留各自原鼠标/键盘语义。已匹配动作不回退legacy业务命令；页面edit/rename消息自身不能执行能力。

编辑器开启时由框架标准模板记录该editor身份及当前业务revision；该草稿记录不授予调用权限。每binding至多64个草稿基线，存储计入原2MiB；同节点已有草稿不得被重复open覆盖。业务更新保留编辑值和原base，后续提交将base用于原revision门，冲突不重试、不清草稿；显式取消/重开或明确解决后才新提交。同步/异步完成只选择性接纳未再变动且业务读回一致的提交，不把显示/计数作为修改证明。FORM/DIALOG继续C2B，不能与此增量双重占有同一入口。

最后回调、modal/busy/disabled、来源/DOM、回调/队列/注销门均复用C2；借用数据到collect返回，callback/user_data到finish成功。跨实例共享资源、WebView2、AI写入仍不支持。验收必须覆盖动态64位ID、非法节点/参数、公开消息、revision与草稿冲突、保留/显式解决、同步/异步和在途解绑；专用逻辑/Light模板夹具不替代物理输入。

## 5. AP-1应用安全poll和维护示例适配（对应实现前冻结）

必要性：C2 observer退栈前不能释放call；应用自己的worker必须由应用确认退出后才能通知关闭。现有示例在worker返回前post close_complete并在unmount无限join，不能把它继续包装成新P2无阻塞生命周期。采用opt-in独立app_poll.h：`ui_app_set_poll_callback(const ui_app_context_t *, void (*)(const ui_app_context_t *,void *),void *)`。不改变任何v1结构/既有接口语义，不创建执行器、目录或线程，也不替应用追踪任意worker。

仅UI线程注册，按workspace/instance/host完整身份核对；在现有workspace安全poll每实例至多调用一次，包括closing但尚未unmount的实例。callback/user_data借用至返回，回调内卸载/递归poll不能提前回收；清空注册阻止后续调用，仍须保留本次栈上数据。卸载前断开注册，不能调用已卸载模块。驱动者按既有合同定期poll，不提供自动排队业务操作或阻塞worker等待。未注册的旧应用行为不变。

五维护示例使用共享的应用内注册辅助代码，仅把能力描述/参数/效果/目标注册到现有目录，普通命令和Assistant读视图从该目录投影。每能力只有一个应用业务函数；辅助代码负责complete、通知复制、终态call安全释放和应用提供的只读核对，不复制业务执行器。内存业务模型与资源revision属于应用，不是AI Service/运行持久化。

应用poll仅做有界终态回收、结果确认和WaitForSingleObject(handle,0)检查；closing时应用确认自己的worker/外部回调退出，才post close_complete。worker在所有可能失败的mount步骤完成后最后启动，避免失败挂载依赖UI无限join。原api7测试夹具默认分支保持原义，stateful嵌入分支采用明确宏钩子，不能以改计数夹具断言冒充业务状态。

已有UST1/UIL1本地偏好功能保留固定路径/格式；它们是应用既有用户界面状态，不是AI存储恢复。只读资源身份/校验在UI，耗时文件准备/写入后台，稳定call在UI确认后完成；失败若已发生部分效果按UNKNOWN保留，不自动重试。不开放任意路径/网络/外发。首轮仅内存的C2A试点边界不变，本次新增的是已明确授权的维护应用既有功能接入。

自动覆盖AP-1：UI线程/错误实例/旧epoch、回调只一次/递归poll推迟、callback中的关闭/清空注册、不在unmount后回调、真实DLL保持到worker退出。旧使用SendInput和普通invoke执行修改的物理脚本原源码/断言保留，不用它们冒充新人工正例；新专用自动语义用例和最终物理清单分别记账。

AP-1私有context租约与能力业务回调租约分开：仍保护host/Assistant/context及模块退栈寿命，但允许安全poll回收终态call和已退役目录元数据；不把整个poll误计为每一项能力正在执行的回调。首次enable仍由现有dispatch_depth门返回PENDING，v1/C2能力自身的租约及错误不改。

## 6. 五示例共享注册辅助代码（应用接入前细化）

examples/p2_support.h只服务本仓库五示例，最多64项描述、4类应用自有资源，能力仍由原框架目录管理。描述引用一个应用业务函数及应用资源revision/只读证据函数；没有手工普通命令/Assistant第二描述和第二执行业务实现。v1 read和v2非read分别complete，关联和生命周期复用框架。业务状态和UI/view/settings资源分开，view变化不得升document revision。

维护示例的typed JSON读写复用同一固定yyjson源码的私有静态目标，不升级、改源或导出新的JSON/AI SDK接口。编译严格配置和源身份与现有固定版本一致；已有许可证保留。避免继续用只能读flat对象的旧demo JSON helper解析C2B嵌套typed fields。框架仍是Schema/资源/权限/来源及JCS权威。

终态UI调用由应用observer复制必要结果，安全poll显式forget；UNKNOWN不被回收或自动解除。调用方拥有的v1/read结果不由应用任意提前释放。若complete发生预算背压，至多每资源保留一份有界输出副本，poll只重投同一完成结果，不重执行业务。新调用明确busy/背压，不伪造完成。

非read结束后，同一固定资源的只读兼容投影若revision过期，按原C1合同退役/注销该read能力，再从同一描述重新注册并显式publish新target；不得原地改target或追随活动标签。在途read保留旧身份，旧结果不丢；刷新等待其安全收尾，不自动重放read请求。

UNKNOWN的应用证据仅来自原调用已返回/应用确认相关worker和外部回调退出后的真实状态与revision；应用可为原能力登记显式人工核对菜单，引用原capability和call ID，不新增核对业务执行器。不能证明时UNRESOLVED。状态文件I/O沿原固定路径/格式在后台完成，结果回UI且确认线程退出后才complete；后台结果重投不等于业务重试。原始失败和匹配二进制单独保留。

## 7. stateful应用接入细化（实现前）

内存document：共享name/mode/color、稀疏单元格修改（64项，原十万行/64列不缩减）、递增revision；table source直接回读修改表。form/dialog各自使用C2B绑定，dialog只提交name，模型投影保留另一处草稿及冲突。单元格用CI-1完整row ID/column/text，选择和排序属于独立view资源。原GL、图片释放/迟到诊断、布局重置、主题、标题、语言和UST1保存恢复均单独列能力，不继续注册旧业务handler。

嵌入api7夹具只增加opt-in行投影钩子及P2生命周期分支，默认测试分支语义和断言不变。P2分支mount不启动worker；应用完成所有可失败绑定/GL初始化后最后启动；close只发stop，poll确认worker退出才ack，unmount不join。原固定UST1/UIL1用户偏好I/O在应用worker准备，UI snapshot/apply/readback；启动恢复也经同一固定路径后台任务，尚在恢复时相关人工操作busy，不覆盖新提交。文件持久结果不能由UI计数证明；无法判定部分效果保留UNKNOWN。用户文件格式、profile锁及schema变体不改。

## 8. minimal_eda接入细化（实现前）

原固定加矩形、放大和清空保留原公式/64上限；每矩形新增实例内不重用的十进制稳定ID，读取枚举包含完整几何。选择只在现有画布坐标命中对象，属于view，不扩展任意几何编辑。document revision仅对象/已提交笔记改变；zoom/selection独立view revision。clear保留原同时复位zoom的UI效果，仍只有该文档的同步写入，没有跨应用原子事务。

保留原生多行EDIT与原readme初值；增加原生提交、取消及明确保留草稿/重新加载按钮。编辑区拥有私有draft base，输入不写document；外部模型变化保留dirty并显示冲突。提交collector完整表达文本及draft base（文本上限由64KiB输入预算约束，不改原控件既有默认限制）；业务执行前核对base，否则明确业务前提失败，零修改。reload/keep只是显式draft操作，新的人工事件重新接纳；不借页面/source字符串授权。该原生EDIT适配为应用参数收集，继续使用已有C2C native button，不伪称其为C2B标准组件自动投影。

menu/toolbar/A-Z-C画布键及新增按钮都引用同一能力。native EDIT窗口消息只更新draft元数据，不直接执行；程序设置EDIT不能签发凭据。若出现无法证明的晚到修改仍沿UNKNOWN核对门，应用无worker的同步模型可提供真实回读证据。

## 9. CI-1勾选及共享示例迁移（实现前）

generic的布尔单元格实际使用check-ID-column标准按钮；在同一独立selector枚举追加CELL_TOGGLE=6，不修改已有数值/结构。仅匹配标准布尔cached cell、原disabled/readonly及私有鼠标/Space/Enter来源，collector依据真实模型生成完整id/column/boolean文本；与CELL_EDIT共同引用同一demo.edit，不新增executor。旧edit页面消息在任一edit/toggle selector拥有时不得回退。格式/只读图像、样式预览及分页/展开/列布局仍是框架已有通用显示操作，不伪称为文档写入。

generic/framework_features共享源分别编译核对；原64项业务修改表保留，属性name/size/enabled/mode/notes新增可回读模型，form/dialog独立C2B草稿。菜单/工具栏中原无参数edit/rename/select/context不再传空对象：collector明确从当前选择/当前字段收参，缺少选择明确前提失败；可先用原表格/树选择建立目标。八个原功能、API4七菜单别名/13工具栏及close_refuse/allow均引用同一描述。Ctrl+D及API4 Ctrl+O显式绑定同一能力；未适配的公开invoke/Assistant修改均拒绝。

后台缩略图worker最后启动；poll确认退出后才close_complete，unmount不等待。模型修改不由operations/缩略图版本计数证明；单元格/树名实际回读、属性模型及revision是验收依据。图片refresh仅已有view/图片缓存版本，不冒充文档编辑。无GL的原API4分支保留且独立验证，正常渲染另以真实DLL验收。

CI-1选择入口须保留原树展开及表格键盘导航：ROW_SELECT接受标准cell的箭头/Tab/Home/End，collector依原完整数据源顺序表达下一行ID，UI只继续本地焦点/分页/展开，旧select消息不再重复执行。stateful考虑排序方向及原Ctrl/Shift选择；generic保留原单选。树标签选择后允许原local展开继续，不把抑制legacy业务误写成禁用展开。输入来源消费仍发生在该能力接纳前，JS后续不带可复用授权。

## 10. web_counter接入细化（实现前）

counter自身计数即该示例真实业务模型（与仅记handler次数的夹具不同）；read返回count及独立revision，increment是唯一业务写函数。菜单和Light add按钮都引用这一项，移除旧message转普通invoke与Assistant修改注册。仍保留原页面和快照；公开消息、模拟/程序输入不得改变count。UINT_MAX时明确拒绝，不溢出回绕。AP-1负责终态UI call回收及read投影更新，无worker/新存储。

## 11. 集中人工验收准备（不是物理结果）

保留现有framework_host作为Web chrome主验收入口；另提供只使用公开SDK的p2_manual_host测试驱动，真实native workspace/HMENU，F6显式只读回读、Ctrl+Tab换实例、F7普通invoke负例、F8公开WM_COMMAND负例，普通Win32消息泵/定时poll关闭，不包含任何来源签发钩子或SendInput。源码与匹配二进制一起交付，本轮不启动其交互验收。

示例共享observer向原event接口发出有界p2.capability通知（能力ID/稳定call/stage/status/code），便于该驱动在当前回调栈记录GetCurrentInputMessageSource分类与业务结果；通知不授予权限或改变业务成功。没有通知/只看到HARDWARE均不等于物理来源证明。框架公开结构、目录/预算及冻结来源合同不变。人工记录必须同时有真实操作、readback和分类；普通注入/系统未知拒绝及UIAccess限制沿已确认边界。

驱动F11/F12只对最后ADMITTED的原instance/call调用既有cancel/forget，切标签不改绑，已释放/实例已关明确NOT_FOUND/不转发。用于人工观察取消和UNKNOWN保持门，不是新的业务入口。异步过短未捕获在途须列未验，不能用最终成功冒充取消验证。可在独立验收profile临时创建固定.tmp同名目录，令原生保存明确失败并保守进入UNKNOWN；不改ACL、不破坏原文件，测试目录由用户人工阶段创建。

保留C2B对模型投影的校验：required name必须是有效非空业务值，示例初值为Untitled。不能为适配旧“空对话框”测试而放宽冻结合同。旧韧性测试改为SelectAll再输入，保持原完整文本断言；其程序Assistant刷新改为明确拒绝加真实view回读，合法刷新在两分支专用夹具验证。原100000/缓存/时限/迭代范围不放宽。

共享适配器动态登记UNKNOWN核对菜单时只用现有ui_shell_refresh保留内容槽；不得使用会refresh_panels的ui_native_shell_refresh销毁应用子窗口。新增真实native HWND子控件+严格invalid_output UNKNOWN回归，验证菜单生成、取消/forget不清门、写入拒绝及内容寿命，仍是private admission逻辑测试，不是physical。没有新增公共接口。

示例模型用C字符串存业务文本，按业务前提拒绝嵌入U+0000（Schema仍可描述标准JSON字符串）；UTF-8字段同时按既有存储字节容量检查，不能将code point maxLength误当字节。框架JSON/JCS对合法U+0000的行为保持，不改全局Schema、错误或限额。回归证明拒绝发生在任何模型修改之前，不截断后假报成功。

stateful保留原Ctrl更新anchor（包括移除被点击行）、Shift区间及Ctrl+Shift并集、降序枚举顺序；collector把完整ids/anchor写入参数，只有业务接纳后才更新anchor，不能在collect阶段修改模型。排序变化清原anchor而保留selection。512选择上限保持，范围过大明确失败，不截断或重试。
