# 应用项目升级到当前 API9 的提示词

目标SDK0.9.0开发版、API9、标准修订9，ABI和包格式仍为1。当前API9开发源码入口为[GitHub main](https://github.com/wbycloud/ui-framework/tree/main)。采用开发版必须记录取得的确切commit；不要用稳定标签v0.1.0替代本轮接口，也不要把离屏测试通过写成真实输入法/跨屏通过。

复制以下内容给负责应用项目的开发者或代码助手。所有入口均来自GitHub；提示词不包含个人本地目录。

```text
仅在请求方明确指定本应用并授权接入修改后执行以下适配。未授权时按docs/business-pilot.md准备检查项，不修改应用。授权后实际完成适配，不只修改版本号。

仓库：https://github.com/wbycloud/ui-framework
从main取得当前开发源码并锁定确切commit；开发源码同步不等同稳定SDK发布。目标SDK0.9.0开发版、框架API9、开发标准修订9。
应用ABI、导出入口ui_app_query_v1、包格式均保持1。

1. 先核对维护方提供的目标commit是否可获取，再从上述GitHub仓库获取该commit，记录实际commit，核对CMake版本0.9.0、UI_FRAMEWORK_API_VERSION=9和标准修订9。读取README.md、CHANGELOG.md、docs/version-policy.md、docs/application-development-standard.md、docs/generic-web-ui.md、docs/framework-menu-offscreen.md、docs/workspace-layout.md、docs/migration-v0.8-to-v0.9.md、docs/validation/instance-language-validation.md、docs/validation/api7-delivery-validation.md、docs/business-pilot.md、include/ui_framework公共头文件及tests/api7_fixture.c。历史应用可参考对应迁移指南，但旧SDK/API/调用方及原包已退出持续兼容保证与CI门槛；本任务应适配并验证当前API9，不能仅改清单。若取得源码不符合目标，报告差异，不猜测接口。

语言接入：先读docs/instance-language.md与完整stateful_components示例。应用自有稳定档案保存语言，在create/首次呈现前提交本host；监听实例通知，用稳定ID更新菜单、面板、组件、表头与字段文本。枚举显示标签不改变语义options。宿主自动跟随活动实例，空宿主使用Windows显示语言；没有全局语言开关。保留草稿、宽度、布局、HWND和GL，不重载DLL。非法/不支持语言明确失败。同步清单/API9与匹配SDK，按语言合同处理回调、线程、复制所有权和卸载。

2. 盘点应用SDK/API/ABI、包清单、菜单与toolbar分组、组件/内容槽、命令、图片、线程和卸载；区分已符合、需要修改、可选离屏适配及框架限制。保留业务模型、算法、文件格式和应用渲染器，只修改应用接入与界面层，不修改框架内部代码。

3. 菜单、工具栏、面板内部控件、状态栏及业务对话框使用公共C接口注册、提供数据和绑定语义命令。复用components.h、images.h、menus.h、现有面板与内容槽；通用HTML/CSS/JS由框架维护。不用TreeView/ListView/Button/Edit/MessageBox或隐藏EDIT代理实现新业务界面。系统文件/目录选择器可保留，OpenGL由应用按需使用。

4. 菜单路径用/表达嵌套，可用menu_group描述给根组明确顺序和名称。工具保留所属toolbar，可选COMPACT加图标；宽度不足使用框架溢出入口。工具/画布菜单用HOST或CONTENT_SLOT锚点，树行用ROW或兼容show_menu，不借用无关业务行来显示工具菜单，不复制内部node-menu。业务target_id独立于位置；核对params.id/target十进制字符串和menu源。

5. 应用Ctrl+O与宿主Ctrl+Shift+O区分。菜单、快捷键、组件和助手复用同一语义命令、业务校验和撤销；助手保持权限确认路径。局部更新使用稳定ID并保留焦点、选择、展开、滚动和草稿。一次操作不得重复执行，数据回填不得再次触发提交，文本编辑快捷键留在编辑区。

6. 树/表格按需查询和虚拟化，不生成全量DOM或提高预算掩盖问题。后台通过workspace复制投递，保留实例、组件代次、项目ID、内容版本、请求ID、DPI；旧/迟到结果安全丢弃。JSON内64位ID使用十进制字符串。

    菜单助记键使用access_key并验证Alt/嵌套/重复键/焦点；需要真正无窗口GL时显式选OSMesa绝对提供方路径、所有GL函数由surface解析并验证NO_WINDOW，离屏MSAA验证实际精确samples。WebView2启用framework_components，把后端借给component_desc.web_backend并保持到host销毁；正确处理PENDING、C图片、异步失效和关闭后的STA消息清理。Session0/无登录运行另行实际验收，不继承Session1通过结论。

    API6保留能力：面板用稳定UTF-8 ID（至多63字节）。应用负责保存不透明布局字节（API6写格式1，API9写格式2并读1）、所属用户/实例和加载策略；注册及mount后恢复，异常输入检查返回值并提供reset。业务文档仍由应用管理。左右栈/浮动/折叠、约束分隔条及拖拽复用原内容和GL context，不重建组件；Windows无窗口只有逻辑矩形，无原生捕获手势。

    API9：共同树/表格/列表纵条映射完整total_count，宽表横条按全部列宽，uint64索引/ID传十进制字符串；共同数据组件无上一页/下一页或列箭头按钮，以完整范围滚动访问；保留按需批次和预算，不以缓存长度冒充总量。应用继续提供真实total_count，并按first/count返回数据；已经符合API9合同无需改动，不自行添加重复分页栏。CANCEL/Esc/捕获失效按框架合同，换源/排序/resize/模态/双实例/迟到结果保持代次及有效草稿选择。底部区域9、旧COUNT8保持；同区tab_group_id属于shell，活动成员切换不重建内容。应用保存不透明格式2，恢复读取格式1，异常/未来版本检查并提供reset；不扩大任意分割树。连续RGBA四通道与透明度预览只改草稿，保留颜色不提交业务，提交一次、取消恢复，核对只读/禁用/非法文本。

    列宽／布局完整文件接入参考examples/stateful_components/README.md及docs/validation/api8-stability-state-validation.md。沿现有save_columns/restore_columns和shell布局接口，稳定组件、列、面板及档案ID跨进程保存；明确存储位置、独占／隔离、加载与显式保存时机、缺失/损坏/未来格式及重置，不用进程内instance_id作为档案身份。业务旧固定坐标受框架几何变化影响时，保留功能断言并迁移定位：有合法所属view时用公共presentation的clip和view-local逻辑坐标转换；独立外部宿主测试没有公开的chrome view获取接口，需经授权采用框架维护的观察器／测试适配，不能臆造getter、直接抄内部句柄或以SendInput返回成功代替最终隐藏/显示状态。

    枚举options使用真实下拉，颜色选取/文本回填校验RGBA。TABLE设置sort_command或selection_flags启用键盘/编辑，Enter提交一次、Esc取消。排序由source对完整数据集按query.sort_column/sort_direction提供窗口，不能只排缓存。多选用非零稳定64位ID，最多512且计入原缓存预算；UI_QUERY_SELECTION返回精确范围ID，post_component_selection复制投递。异步范围完成前旧集合仍有效，select命令表示手势意图，不代表范围结果已完成。排序/换源/更新/双实例/关闭按代次合同处理，不提高预算。

7. RGBA/包内PNG通过C接口发布；像素不经JSON/Base64。遵守顶向下RGBA8、非预乘输入、正stride、缓冲长度、复制和资源预算；缩略图由应用自己的渲染器生成，框架负责显示。借用图片、字符串和回调参数按公共所有权合同使用，跨模块内存由分配方释放。

8. 使用API9新函数时，以匹配头文件和ui_framework_runtime.lib重建DLL，DLL descriptor和严格八字段.uapp清单一致声明API9；ABI/包格式仍1。只共享宿主ui_framework.dll。SDK commit及标准记录在应用文档或锁定文件，不往包清单添加字段。历史API包不再保证兼容或承担专项回归；当前运行库暂存的接受行为不是维护承诺，不能只改清单假装已迁移；结构先清零再设size，新字段只按完整字段末端读取，保留旧偏移与枚举。

9. 默认任务是正常宿主适配。离屏接入可选：查询run_mode，避免自有HWND/timer/显示/系统选择器依赖；无窗口workspace用原内容槽、组件及复制队列，UI线程显式flush/poll。Web捕获使用显式尺寸和调用方RGBA缓冲，先查询能力。绘图需要GPU时显式使用隐藏WGL离屏入口、原frame和input回调；至少3.3 compatibility，不静默降级。隐藏HWND/DC、真正OSMesa无窗口、整机无登录和桌面SwapBuffers的区别写清楚。历史78c24cb框架测试DLL已在实际CI LocalSystem Session0通过，当前API9必须另行复验，但托管runner仍有登录会话1，整机无登录待验；读取docs/validation/session0-osmesa-validation.md，不将框架测试成功当作本应用业务或无登录通过，不宣称普通旧应用自动无头兼容。

10. 实例独立；部分初始化失败与关闭沿同一清理路径。停止并join worker，取消订阅/投递，unmount释放GPU和自有窗口，所有回调退出后才可卸载模块。REFUSE/WAIT保持正常含义，flush空闲不等于worker停止，关闭重开后的旧结果不得进入新实例。

11. 构建DLL和.uapp，在匹配真实宿主验证七类菜单分组、嵌套/最后一项/窄窗工具溢出、动态状态、一次快捷键、双实例及固定目标、草稿/焦点、图片更新和过期结果、大数据DOM、对话框及失败/卸载/重开。覆盖四种窗口尺寸、96/144/192程序DPI、保存/异常恢复/重置、分隔条与拖拽提交/取消、完整排序/跨页多选、实际GL内容槽。离屏适配若做，额外运行真实模板输入、像素、队列及GL读回测试；不以直接调handler代替交互。WebView2须实际Runtime至少138.0.3351.48并具备ControllerOptions4；SDK存在或轻量通过均不能代替。

12. 对照验收记录的UV项补验实际中文输入法、物理跨屏/边缘/桌面合成、长时人工压力、复杂编辑布局、Session0和应用自己的完整业务接入。没有条件明确标未验证并记录原因/复验步骤，不继承框架不存在的全功能通过结论。WebView2按实际Runtime和原API5异步合同验收，不能用轻量后端通过替代。框架API9独立DLL/.uapp仅是框架集成证据，真实业务应用需另有指定和修改授权；没有OSMesa实测瓶颈和明确硬件需求，不自动扩大到硬件无窗口方案。

    使用包含API4补验修复的共享运行库，检查浮动表单在同实例模态期间不能被原生字符输入修改，关闭对话框及redock后实际键盘焦点保留；反复换页、回填STYLE字段后检查图片数量及GDI资源回收。框架自动预览由框架回收，应用自己的图片ID保持原所有权。自动压力和程序DPI结果不替代UV人工验收。

13. 更新应用README、SDK/API/标准/commit记录、迁移与验收记录，交付可构建源码及应用包，分别列已完成、部分支持、未实现、未验证、执行命令与真实结果，以及剩余阻塞。
```

宿主现采用浏览器式标签标题栏与第二行活动应用菜单，见[宿主说明](standalone-host.md)。这属于内部变化，已符合当前API9合同的应用无需为外观重编译或升级API；不要访问宿主DOM/窗口类或存储其最近记录。仍按原内容槽、菜单、助手及关闭回调接入，业务文档和未保存确认仍归应用。框架自有模板采用[统一视觉规范](visual-design.md)，独立宿主同步共同组件主题，应用自有HTML/原生/GL主题仍自行管理；AI启动默认收起，高级区保留原参数/事务/日志访问路径。此前分页入口清理保持当时API7，此前列宽/help升级API8，本轮新增实例语言及文本更新统一升级API9，不改变当前应用数据源合同；历史包兼容承诺以[当前版本政策](version-policy.md)为准，历史验收原样保留。


保留API7轮原生滚动修复的当前API9中，核验完整SDK的源码commit、头文件、ui_framework_runtime.lib、ui_framework.dll、framework_host.exe及产物SHA256，再重建当前应用。应用继续按真实total_count及first/count提供稳定非零ID的复制批次；无需增加缓存或业务滚动条。验收须包括真实鼠标首末项/末列、可见文字或样式像素、resize、焦点和关闭；离屏输入通过不替代原生鼠标。[本轮框架记录](validation/native-component-scroll-validation.md)。

当前框架内部视觉调整无需应用复制模板：空标题、属性自动同行/堆叠、图片按比例显示和列宽对齐由框架处理。请部署同commit的宿主/共享DLL/导入库/头文件；保留真实total、稳定ID、按需查询及原预算。实测参考docs/validation/visual-polish-validation.md，当前维护政策不变，新增公开能力须使用匹配SDK9，原生真实鼠标首末行/末列不得用离屏结果替代。

此前API8引入列宽交互与UCW1应用侧保存/恢复、可选help字段；读取docs/migration-v0.7-to-v0.8.md。列宽初值继续由应用声明，不自动适配；保持真实total_count、按需批次、预算及无分页组件。使用匹配SDK9重建并同步清单/DLL声明；不要修改业务默认列宽或历史固定坐标断言以冒充框架修复。只有明确授权才修改请求方应用。

当前源码已实现实例语言，接手重复API8参考提示词时先核对Git和API9公共头，不重复创建语言系统或无故升级API。最新完整应用、语言快照及匹配SDK复验见[2026-10-09记录](validation/instance-language-current-validation.md)；应用继续拥有偏好/档案存储并在首呈现前提交。框架自有示例通过不代表请求方业务应用验收，未经明确授权不修改该应用。
