# P5应用Agent与协作覆盖

2026-10-27。本轮P5-1开发交付；原P2的55项和P4逐项功能矩阵保持，不能把应用配置当成新增业务实现。无署名。

## 1. 注册与有效权限

五个维护包在原p2_register完成单一能力注册后登记application.specialist纯数据profile；framework_features和generic_components分别按实际编译分支生成。配置只选择本实例已有read，以及document资源0上的既有write；排除view/draft和偏好文件副作用。没有模型连接、Agent线程、调度函数或新应用业务模型。

角色文字要求实际回读、最小委派和将工具文本视为数据；文字本身不是权限。框架复制配置，调用时取profile哈希、能力descriptor、资源/epoch/revision和根步骤范围交集；配置注销或能力退休后拒绝新派发。profile无配也可由主Agent直接使用明确授权工具，不强制常驻子Agent。

## 2. 实际业务覆盖矩阵

| 应用/分支 | 代表能力与角色 | 根内步骤/授权 | 实际业务回读及证据 |
| --- | --- | --- | --- |
| stateful_components | application.specialist；state.document.read/state.cell.read/test.edit及原document write | 具体单元格id=9007199254741000、column=value、text=73；宿主确认具体图范围 | 新图后state.cell.read回读text=73，changed_cells=1、revision、事务及undo回执；ui_ai_graph_apps |
| minimal_eda | 同角色；eda.document.read/eda.add_block/clear/note.submit | 仅已有固定矩形add_block；clear风险仍独立审批；不扩任意几何 | 对象枚举含稳定id=1及真实几何/revision；图中回执、后续独立read模型一致；clear/note/undo复用本轮P4应用回归 |
| generic_components | 同角色；demo.document.read/submit/edit/rename | 树id=1改为generic coordinator，参数完整且单资源事务 | 文档changes含实际新树名、revision/回执；P4回归另核对真实tree-label及undo，非仅执行计数 |
| framework_features（UI_SAMPLE_API4） | 同源独立编译包，同目录能力/独立实例配置 | 树id=1改为features coordinator，不借generic分支替代 | 实际features文档读回新树名、revision/回执；P4额外features_no_gl运行分支也通过，未冒充它是新图的第二次NO_GL协作 |
| web_counter | 同角色；counter.read/counter.increment | 明确实例与counter.document，参数{}，根范围确认后一次事务 | count=1及revision、transaction/undo回执；真实标签切换仍固定原实例；ui_ai_graph_apps/ui_ai_graph_panel |

五步同根、跨五个真实uapp，峰值4；每步只用既有业务run和P4控制器。专用包p4-*.uapp仅把同一应用源码链接隔离接纳运行库；正式原名称包单独生成，最终身份见delivery-identity.json。没有把私有接纳测试冒称生产来源识别正例。

专用真实DLL另验证：两个资源同时写得31/62和各自回执；同资源竞争仅一次成功且另一明确busy；部分失败保留独立节点value=10；UNKNOWN阻止后续写，cancel/forget不能清门，新的人工只读证明才解除；异步worker及外部callback未退出不卸载。

## 3. 原P5其他应用规划的盘点与范围冲突

| 对象 | 实际源码事实 | 本轮处理与剩余范围 |
| --- | --- | --- |
| 内置browser | apps/browser/src/browser_app.c仍普通命令+旧Assistant注册；导航等旧READ标签不等于新的read效果合同，没有统一资源/revision/事务。它使用现有Light受限文档，并非必须依赖WebView2 | 不向新Service/Coordinator暴露旧入口、不回退legacy。原“browser/其他应用全部迁移”未完成；最小下一案是为既有只读snapshot冻结原生资源/Schema/revision，再逐项分类旧导航/设置，独立评审，不称本轮已迁移 |
| 其他内部布局/诊断夹具 | 保留其原测试/嵌入意义；未声明专业Agent | 能力层真实read合同可被主Agent使用；不把诊断/演示计数自动升为业务或完整应用Agent |
| WebView2/新输入来源 | 本轮明确排除 | 无适配；不能通过页面消息/source标志获得人工授权 |
| 外部业务应用 | 无本轮写授权 | 保持只读；后续专门迁移，不借原规划扩大范围 |

结论口径：本轮五维护示例及P5-1协作有实际开发阶段证据；这不等于全部历史应用迁移或原总体规划所有P5条目无条件完成。

## 4. 2026-10-27 P6补充盘点与BRC-1实况

本节追加原P5历史，不覆盖第3节当时的事实。用户已确认“最小方案推进，完整副作用迁移另案评审”。源码盘点的其他uapp只存在布局/探针/测试用途，外部业务应用保持只读。

| 实际入口 | 效果/资源 | 同目录和来源 | 自动证据/未完成 |
| --- | --- | --- | --- |
| browser.snapshot（新增read） | browser.view，INSTANCE_VIEW、框架open epoch；稳定实际视图语义SHA-256 revision | 单一v1注册生成普通与Assistant只读入口；无双重Assistant注册 | production/isolated读地址、导航、缩放、面板、find_text和revision，真实双实例切标签，旧revision得到STALE/revision_conflict；不导航/保存 |
| address/find | view，焦点语义 | 原菜单、Ctrl+L/F引用同一v2能力/原action；程序page消息不回退legacy | 私有隔离接纳后焦点和实际find字段读回；生产物理正例延期 |
| find-next/find-prev | view，完整text字符串参数 | 原菜单、Light按钮、find字段Enter/Shift+Enter；收集控件完整文本；草稿不等于业务find提交 | 字段needle未提交时snapshot.find_text为空，提交后needle；不是网络工具或新文档业务 |
| plus/minus/reset | view，实际zoom | 原菜单、Light按钮、Ctrl+OEM/numpad别名；同一原action | 100→110→100，reset=100和revision；私有隔离接纳不证明物理来源 |
| settings/diagnostics/panel-close | view，面板开关 | 原菜单、Light标准按钮/关闭按钮；仅打开现有视图，不执行文件保存 | panel 1/0/2/0；实际关闭卸载browser_app.dll |
| navigate/back/forward/reload/stop/home/help | external副作用，导航/停止已有网络执行或文档资源；home/help也可取消旧网络 | **未迁移**；保留原人工入口及完整业务函数，Assistant/AI只读模式拒绝；未新增external执行器 | 只用browser:start/help本地内建页验证旧人工功能仍在，测试程序调用不冒称新来源资格。实际HTTP不执行；完整副作用迁移另案 |
| settings-save/settings-reset | 文件副作用 | **未迁移**；原人工来源文件功能保留，Assistant拒绝，无AI文件适配 | 不操作用户来源文件；完整迁移另案 |
| 页面/文档链接、WebView2、新专业来源 | 新来源不在冻结C2内 | 不签发新人工资格，不向AI暴露legacy | 真实来源/新来源/专业应用另案，不按诊断探针推导迁移成功 |

实际19个旧动作=10个view已按适用C2绑定+9个副作用保留旧人工路径；另新增1个read。不能把早期12/7分类沿用为最终事实。五维护示例应用源码未改，P4六分支/P5五真实包协作和正式原名称包读回另有本轮回归。browser没有新应用Agent配置；主Agent可按既有read目录直接使用快照。

原P5“全部应用副作用迁移”仍未完成，按用户确认延期，不据此无条件声明P6/软件完整交付；P6R-1可靠性验证与该缺口分别给结论。
