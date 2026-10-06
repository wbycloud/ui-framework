# 0.6 → 0.7 迁移

SDK0.7开发版、API7、标准7；ABI1/包格式1/ui_app_query_v1不变。从本轮分页清理起，[当前维护政策](version-policy.md)替代旧包免重编兼容承诺；历史结果不改。适配当前API7须匹配头文件/导入库和DLL，DLL descriptor及包清单同步声明7。未推送的本地提交不是GitHub main当前版本。

## C ABI 与输入

| 项目 | 原值/尺寸 | API7 |
| --- | --- | --- |
| ui_layout_desc | 原尺寸/全部偏移 | 保持 |
| ui_panel_layout | 48字节 | 64字节；group偏移48、active偏移56 |
| size完整字段 | 原48前缀 | size≥56访问group，size≥60访问active；55/59部分字段不访问 |
| ui_input_event | 原尺寸/偏移 | 保持；CANCEL=8追加 |
| 区域 | COUNT=8哨兵 | 原枚举不改，8保留，BOTTOM=9 |
| 应用/包 | ABI1/格式1 | 保持 |

先清零，再设置size。旧get只写旧48字节，旧同区set保留标签组，移区/浮动脱组；新字段0表示脱组。不用COUNT遍历所有新区域，以公开区域值查询。窗口/无窗口捕获丢失可提交CANCEL，旧1..7输入语义保留；Esc/CANCEL恢复手势起点，失活/模态/resize/代次变化解除捕获并保留最新有效状态。

## 布局与组件

底部在两侧栏间，默认220，set_bottom_height控制逻辑高度，窄高给主区留120或分配一半。dock_panel_tab/activate_panel复用已有槽和GL，不重mount。稳定group_id属于shell，0无组，UINT64_MAX保留；活动关闭换可用成员，activate重开，reset恢复注册默认。

布局2用96字节头/112字节记录，仍512记录/64KiB；读格式1（80/104），新组默认0，未来版本UNSUPPORTED，异常输入完整验证后拒绝。旧API1–6且未启用新能力仍保存1，新API7保存2。应用负责存储、版本备份和加载时机，旧运行库不能读2，业务文档仍由应用管理。详见[布局](workspace-layout.md)。

共同虚拟数据纵条基于完整total_count、横条基于全部列宽。无需应用HTML或新组件描述字段；排序/范围仍由完整source负责。共同数据组件移除分页和列箭头按钮，视口回收操作栏；按需批次及原节点/缓存/图片/投递预算不变，已符合API7数据源合同无需修改。索引和ID用字符串，JS BigInt处理uint64，避免Number精度丢失。

source总量缩小时可返回row_count=0的空批报告新total_count，原请求first允许超出新范围；非空批的first/count仍必须有效。框架夹紧并按需重查，不预加载。已知展开分支可在移出缓存后折叠。

连续RGBA/透明度/文本沿用颜色字段及commands.submit，不新增业务提交合同。拖动只写草稿；保留颜色关闭选择器，表单提交一次；取消/Esc与非法输入语义见[标准](application-development-standard.md)。

## 验证

冻结SDK、原包和历史验收保持原样，但不再编译或验收历史调用方/原包专项。当前版本的原生、轻量、真实Runtime、OSMesa/MSAA/普通附件分别执行。框架独立DLL集成不等于业务应用验收。实测、失败、commit及环境限制见[API7验收](validation/api7-validation.md)，重开问题见[资源记录](validation/runtime-stability-validation.md)。

API7面板注册的dock_region可直接声明BOTTOM，NONE保留原RIGHT默认；初始有/无窗口布局为其分配底部空间，reset回到注册默认底部。直接声明与事后移动同样保留内容生命周期，见[注册复现和修复](validation/api7-validation.md)。

交付来源与验证再核对[CI收敛记录](validation/api7-delivery-validation.md)。新clone可能仍为旧API，先取得维护方指定且实际可获取的API7 commit。业务接入按[试点清单](business-pilot.md)核对应用和授权；本轮CI/测试内部修正不新增公共接口或升级ABI。
