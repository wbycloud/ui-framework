# 应用项目升级到当前 API4 的提示词

目标SDK0.4.0开发版、API4、标准修订4，ABI和包格式仍为1。源码入口为[GitHub main](https://github.com/wbycloud/ui-framework/tree/main)。采用开发版必须记录取得的确切commit；不要用稳定标签v0.1.0替代本轮接口，也不要把离屏测试通过写成真实输入法/跨屏通过。

复制以下内容给负责应用项目的开发者或代码助手。所有入口均来自GitHub；提示词不包含个人本地目录。

```text
请实际完成当前应用项目向以下框架版本的适配，不要只修改版本号或提供建议。

仓库：https://github.com/wbycloud/ui-framework
分支：main。目标SDK0.4.0开发版、框架API4、开发标准修订4。
应用ABI、导出入口ui_app_query_v1、包格式均保持1。

1. 从上述GitHub仓库获取main，记录实际commit，核对CMake版本0.4.0、UI_FRAMEWORK_API_VERSION=4和标准修订4。读取README.md、CHANGELOG.md、docs/application-development-standard.md、docs/generic-web-ui.md、docs/framework-menu-offscreen.md、docs/migration-v0.3-to-v0.4.md、docs/validation/api4-validation.md、include/ui_framework公共头文件及examples/framework_features/README.md。API1/2应用同时读取此前迁移指南。若取得源码不符合目标，报告差异，不猜测接口。

2. 盘点应用SDK/API/ABI、包清单、菜单与toolbar分组、组件/内容槽、命令、图片、线程和卸载；区分已符合、需要修改、可选离屏适配及框架限制。保留业务模型、算法、文件格式和应用渲染器，只修改应用接入与界面层，不修改框架内部代码。

3. 菜单、工具栏、面板内部控件、状态栏及业务对话框使用公共C接口注册、提供数据和绑定语义命令。复用components.h、images.h、menus.h、现有面板与内容槽；通用HTML/CSS/JS由框架维护。不用TreeView/ListView/Button/Edit/MessageBox或隐藏EDIT代理实现新业务界面。系统文件/目录选择器可保留，OpenGL由应用按需使用。

4. 菜单路径用/表达嵌套，可用menu_group描述给根组明确顺序和名称。工具保留所属toolbar，可选COMPACT加图标；宽度不足使用框架溢出入口。工具/画布菜单用HOST或CONTENT_SLOT锚点，树行用ROW或兼容show_menu，不借用无关业务行来显示工具菜单，不复制内部node-menu。业务target_id独立于位置；核对params.id/target十进制字符串和menu源。

5. 应用Ctrl+O与宿主Ctrl+Shift+O区分。菜单、快捷键、组件和助手复用同一语义命令、业务校验和撤销；助手保持权限确认路径。局部更新使用稳定ID并保留焦点、选择、展开、滚动和草稿。一次操作不得重复执行，数据回填不得再次触发提交，文本编辑快捷键留在编辑区。

6. 树/表格按需查询和虚拟化，不生成全量DOM或提高预算掩盖问题。后台通过workspace复制投递，保留实例、组件代次、项目ID、内容版本、请求ID、DPI；旧/迟到结果安全丢弃。JSON内64位ID使用十进制字符串。

7. RGBA/包内PNG通过C接口发布；像素不经JSON/Base64。遵守顶向下RGBA8、非预乘输入、正stride、缓冲长度、复制和资源预算；缩略图由应用自己的渲染器生成，框架负责显示。借用图片、字符串和回调参数按公共所有权合同使用，跨模块内存由分配方释放。

8. 使用API4新函数时，以匹配头文件和ui_framework_runtime.lib重建DLL，DLL descriptor和严格八字段.uapp清单一致声明API4；ABI/包格式仍1。只共享宿主ui_framework.dll。SDK commit及标准记录在应用文档或锁定文件，不往包清单添加字段。旧API1/2/3包可保留原声明兼容加载，不能只改清单假装已迁移。

9. 默认任务是正常宿主适配。离屏接入可选：查询run_mode，避免自有HWND/timer/显示/系统选择器依赖；无窗口workspace用原内容槽、组件及复制队列，UI线程显式flush/poll。Web捕获使用显式尺寸和调用方RGBA缓冲，先查询能力。绘图需要GPU时显式使用隐藏WGL离屏入口、原frame和input回调；至少3.3 compatibility，不静默降级。隐藏HWND/DC、Session0未验证和桌面SwapBuffers的区别写清楚，不宣称普通旧应用自动无头兼容。

10. 实例独立；部分初始化失败与关闭沿同一清理路径。停止并join worker，取消订阅/投递，unmount释放GPU和自有窗口，所有回调退出后才可卸载模块。REFUSE/WAIT保持正常含义，flush空闲不等于worker停止，关闭重开后的旧结果不得进入新实例。

11. 构建DLL和.uapp，在匹配真实宿主验证七类菜单分组、嵌套/最后一项/窄窗工具溢出、动态状态、一次快捷键、双实例及固定目标、草稿/焦点、图片更新和过期结果、大数据DOM、对话框及失败/卸载/重开。覆盖四种窗口尺寸、96/144/192程序DPI和实际绘图内容槽。离屏适配若做，额外运行真实模板输入、像素、队列及GL读回测试；不以直接调handler代替交互。

12. 对照验收记录的UV项补验实际中文输入法、物理跨屏、长时人工压力、复杂编辑布局、Session0和应用自己的完整业务接入。没有条件明确标未验证并记录原因/复验步骤，不继承框架不存在的全功能通过结论。WebView2新增诊断和组件能力没有等价支持，缺失接口使用能力查询/UNSUPPORTED处理。

    使用包含API4补验修复的共享运行库，检查浮动表单在同实例模态期间不能被原生字符输入修改，关闭对话框及redock后实际键盘焦点保留；反复换页、回填STYLE字段后检查图片数量及GDI资源回收。框架自动预览由框架回收，应用自己的图片ID保持原所有权。自动压力和程序DPI结果不替代UV人工验收。

13. 更新应用README、SDK/API/标准/commit记录、迁移与验收记录，交付可构建源码及应用包，分别列已完成、部分支持、未实现、未验证、执行命令与真实结果，以及剩余阻塞。
```
