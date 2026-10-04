# 0.5 → 0.6 迁移

SDK0.6.0开发版、API6、标准修订6。应用ABI1、导出`ui_app_query_v1`、包格式1及旧枚举数值保持。运行库接受API1–6；新应用清单和DLL descriptor一致声明6，API6应用不能加载到仅支持API5的运行库。仍未创建新稳定标签，取得[main](https://github.com/wbycloud/ui-framework/tree/main)后记录确切commit。

## 兼容策略

保留原应用包后，用新运行库运行原包；另外用[冻结SDK5](../tests/sdk_v5/README.md)验证旧调用方。不得覆盖原包后将重编译结果写成二进制兼容。SDK1–4保持原样，[API5历史断言](../tests/api5_baseline/README.md)说明为何版本拒绝和新结构尺寸断言更新，旧偏移和失败预算断言保留。

| Windows x64结构 | API5尺寸/尾部 | API6追加字段 |
| --- | --- | --- |
| ui_component_desc_t | 144字节；web_backend偏移136 | sort_command偏移144，selection_flags偏移152，总尺寸160 |
| ui_component_query_t | 96字节完整前缀 | sort_column偏移96，sort_direction偏移104，总尺寸112 |
| ui_component_state_t | 120字节完整前缀 | selected_count偏移120，sort_column偏移128，sort_direction偏移136，总尺寸144 |

描述、输出结构先清零，再设置size。框架只读/写完整字段，部分追加字段忽略；旧尾padding不作为新字段。query参数和sort_column在source回调期间借用，异步保留需复制。`UI_QUERY_SELECTION=3`、`UI_SELECTION_MULTIPLE=1`为新增值，既有值不变。新ui_panel_layout_t要求完整size，不把任意结构都当作可截断描述。

旧组件未配置API6体验时继续点击编辑；现有列、字段、图片、队列和虚拟化预算不变。枚举下拉和颜色选择完善了共同模板；应用旧提交命令与字段ID仍适用。

## 布局接入

在mount完成注册与挂载后，调用save/restore/reset布局接口。应用负责存储字节和布局所属用户/实例，不把业务文档交给框架。稳定面板ID不超过63字节UTF-8，标题可以改变。格式1、异常数据、DPI/work area、左右栈/浮动及逻辑离屏行为见[布局合同](workspace-layout.md)。

侧栏宽度用resize_splitter的NULL panel参数，栈高度指定panel；独立宿主提供手势和reset入口，嵌入应用可调用begin/update/end。提交重排现有容器，取消保留原布局；不用销毁/重建组件或GL surface。原ui_native_shell_refresh仍是重建语义，持久布局更新使用ui_shell_refresh及新接口。

## 排序、多选与编辑

TABLE配置sort_command绑定已注册语义命令，source按query的列/方向对**完整数据集**排序并提供窗口；框架不排序缓存页。方向0/1/-1表示无/升/降，排序重新请求并使旧异步结果失效。应用收到edit命令后更新模型并回填，命令只发一次不代表业务已保存。

selection_flags=UI_SELECTION_MULTIPLE开启Ctrl/Shift与稳定ID集合。程序选择不发select命令；用户select是手势意图，异步范围完成前集合保持上次结果。source处理UI_QUERY_SELECTION，返回至多512个精确范围ID；后台使用post_component_selection复制投递，消费后再查询集合。排序保留ID但重置范围锚点，换源清空，删除/结构更新按[组件合同](generic-web-ui.md)处理。

颜色文本接受#RRGGBB/#RRGGBBAA，非法值提交显示错误；下拉选项来自原options。Enter/F2编辑、Enter提交/Esc取消、方向/Home/End/Tab导航由共同模板维护，两后端语义相同，Runtime异步结果仍需正常消息循环。

## 验收与部署

[构建说明](build-and-validation.md)列出固定依赖和四种CI配置。[独立API6测试DLL](../tests/api6_fixture.c)用于框架集成，不能冒充应用自己的业务试点。旧API5 Alt、OSMesa、MSAA和实际WebView2桥接继续回归；普通非MSAA路径保留。

Session0实际应用GL已有通过证据，整机无登录仍需服务模式Windows x64 runner和严格工作流。中文IME、物理不同缩放屏幕/边缘/桌面合成、长期人工与业务应用修改授权分别检查；条件缺失保持未验收。未发现经实测证明的OSMesa性能瓶颈且没有明确硬件GL需求时，不自动扩展到硬件无窗口方案。
