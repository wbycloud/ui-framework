# SDK 0.3 → 0.4 迁移

目标SDK0.4.0开发版/API4/标准4，应用ABI与包格式仍为1。采用开发版本记录确切commit；没有新稳定标签。实际能力见[菜单与离屏接口](framework-menu-offscreen.md)，证据及待补验见[API4验收](validation/api4-validation.md)。

## 兼容应用

运行库接受API1/2/3/4。已有二进制应用可保留原清单与DLL声明；API3原包已经单独加载复验。旧原生内容继续兼容。使用API4新函数时，换用匹配头文件和ui_framework_runtime.lib，重建DLL，并把DLL descriptor与包清单一并声明为4；不要只改清单或在包内携带替换运行库。

旧结构及枚举原值保持。toolbar.display只有size包含该字段时读取；workspace.run_mode位于保留API3尾部填充之后，旧size不会被误读为离屏模式。所有新描述清零并填size。冻结头文件位于tests/sdk_v1、sdk_v2、sdk_v3。

## 正常宿主

1. 继续通过ui_host_register_menu_item注册菜单。路径用 `/` 表达子组；可用menus.h的group注册给根组明确顺序和显示名称。不要安装HMENU或复制框架菜单HTML。
2. 工具按所属toolbar分组。可选COMPACT，需发布图标；窄窗溢出由框架提供。不要通过删除业务工具掩盖空间不足。
3. 工具/画布菜单用ui_host_show_menu的HOST或CONTENT_SLOT锚点；树行用ROW或兼容ui_component_show_menu。业务目标身份与锚点独立，64位ID仍为十进制字符串。
4. 宿主打开应用包改为Ctrl+Shift+O，应用可注册Ctrl+O。菜单源为menu；参数保留action="menu"及id，并追加target。需要严格参数校验的应用核对新字段；助手仍走原权限路径。
5. 不依赖旧组件内部node-menu DOM；它属于框架模板，已改为独立公共弹窗。标签切换、行删除或DPI/尺寸改变后旧菜单关闭。

## 可选离屏适配

正常加载不要求离屏迁移。要提供离屏测试，先查询run_mode，再避免自有窗口、timer、IME、显示通知和系统选择器依赖；文件路径可由语义命令传入。Web组件正常mount到无窗口槽，GPU画布显式使用offscreen创建及render/readback。隐藏WGL不等于完全无窗口或Session0。

使用原workspace复制投递，UI线程flush/poll；REFUSE/WAIT不变。所有worker停止并join、回调退出、GPU资源释放后才能卸载。测双实例、部分初始化失败、迟到结果和重开。详见可构建framework_features样例和三个离屏测试，不访问框架私有类型或任意eval入口。

中文输入法、物理跨屏、长时压力等未验证项仍按UV记录补验，不能继承“全部通过”的结论。

API4补验修复浮动组件原生输入绕过模态门控、自动STYLE预览换页/回填后滞留及模态后redock丢失实际焦点的问题。公共ABI及应用清单不变，旧API3组件包也使用修复后的共享运行库；应用无需新增接口或管理框架自动预览。自行发布的图片ID仍由应用按原合同管理。复现与回归见[补验记录](validation/api4-validation.md#5-2026-10-04-接手补验与修复)。
