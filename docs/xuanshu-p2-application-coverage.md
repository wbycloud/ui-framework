# P2 维护示例：功能、入口和业务覆盖

2026-10-24增补：本页功能和55项能力范围不变。已完成[开发阶段A/B模拟与自动验收](validation/xuanshu-p2-simulated-acceptance.md)，新增实际别名/collector、两共享分支、stateful表单/对话框及真实workspace重开/epoch验证；生产代码未变。下列“物理待验”统一指[整体软件完成后的延期集中验收](xuanshu-p2-deferred-manual-acceptance.md)，没有删除或提前记通过。

状态：源码接入及自动合同验证；真实人工入口尚未验收。整体架构仍为Proposed。本文盘点本地实际源码，不将规划中的任意编辑、AI写入或跨文档事务列为现成功能。

统一来源为各应用的 `capabilities.inc`（counter在app.c）中的p2_spec数组。共享 `examples/p2_support.h` 只注册同一框架目录、转发同一个应用run函数、完成及安全回收；不是第二执行器。普通命令由目录投影，Assistant兼容视图只有read。表中同名菜单/工具栏是引用，不重新注册业务handler/权限。

## 1. stateful_components

实际数据源保持100000行、64列，树20个根及各一个子节点；未把api7原编辑计数当业务模型。应用增加64项稀疏单元格状态与共享name/mode/color、document revision。source直接从模型读取，state.cell.read返回实际ID/column/text。树缩略图和OSMesa仍为原设施。

| 功能 | UI入口 | 能力ID／效果 | 唯一业务实现及回读 | 自动证据／状态 |
| --- | --- | --- | --- | --- |
| 属性与单元格读取 | 生成普通read/Assistant、验收驱动F6 | state.document.read、state.cell.read／read | state_run → state_describe/state_cell_text；不读控件作业务证明 | p2_stateful、p2_apps；已接入 |
| 全表单属性提交 | FORM提交/Enter | test.submit／write | name/mode/color实际模型，值变化才升revision | p2_stateful；两私有草稿与冲突回读 |
| 对话框name提交 | DIALOG提交/Enter | test.modal.close／write | 同一name，不复制第二属性；选择性接纳后才关闭 | p2_stateful；已接入，物理待验 |
| 单元格文本/数值编辑 | CELL_EDIT，F2/Enter；Esc丢局部草稿 | test.edit／write | 校验完整64位ID/列/文本；有限数值；source与read实际回读 | p2_stateful、component_inputs；已接入 |
| 选择 | 标准表格点击、Ctrl/Shift、箭头/Tab/Home/End | test.select／view | 模型选择ID及组件selection共用；不升document revision | p2_stateful，导航收参代码核对；物理组合待验 |
| 排序 | 标准表头 | test.sort／view | 沿原source的direction反向枚举；不是新增按任意单元格内容排序 | p2_stateful实际sort/selection回读；不重复invoke旧命令 |
| 界面状态读取 | read/Assistant/F6 | state.view.read／read | 实际列/布局快照、选中ID、sort、theme、tab title、image、modal | p2_apps及源代码；已接入 |
| 打开属性 | State/Edit properties菜单 | test.modal／view | 投影同一模型，然后show_dialog | C2B、p2_stateful；物理菜单待验 |
| 取消与冲突解决 | FORM/DIALOG Cancel、Reload/Keep | 所属C2B绑定的draft控制 | 保留未提交草稿；显式reload/keep重设base，不写document | C2B原合同与应用冲突回归 |
| GL刷新 | Diagnostics菜单 | test.render／view | 原render；真实OSMesa帧/图像，不以frames计数证明文档修改 | p2_apps真实DLL mount必须render成功；再次点击待人工 |
| 布局重置 | Diagnostics菜单 | test.reset／view | 原ui_shell_reset_layout | 已接入；人工布局回读待验 |
| 图片释放/迟到诊断 | Diagnostics菜单 | test.image.release、test.late／view | 原图像释放和旧generation拒绝语义；不是文档编辑 | 已接入；独立原图像/组件回归，人工诊断待验 |
| UST1保存/恢复 | State两个菜单 | state.save／write（偏好）、state.load／view | UI snapshot/apply；固定路径/原格式；I/O后台，UI确认线程退出后完成 | p2_stateful真实写入、改变宽度、重载回读；已接入 |
| 单列/全部列布局重置 | State菜单 | state.reset-column、state.reset／view | 原列/布局API，实际快照回读 | 已接入；物理拖列后重置待验 |
| 标签标题 | State/Use draft as tab title | state.title／view | 收集完整title；实际workspace实例title，未提交name仍是草稿 | 显式固定实例；物理双标签验证待验 |
| 系统主题 | State菜单 | state.theme-system／view | 原theme API和原语义图标回调 | 已接入，theme原回归；物理待验 |
| 语言和偏好读取 | Language菜单、read/Assistant/F6 | language.zh、language.en／write；state.preferences.read／read | 原UIL1的固定16字节；业务key不随标签翻译变化 | p2_stateful语言/文件回读；独立worker退出 |
| 启动恢复、A/B锁及schema变体 | 挂载与原环境配置 | 应用生命周期，不伪装为新AI命令 | 原profile锁/路径/UST1、UIL1；reorder/evolve保留 | 实际双实例/关闭通过；变体物理专项未重跑 |

普通数据分页、树展开、滚动、列宽/显隐及完整文本/图像预览属于框架已有显示/布局设施，不宣称这些显示变化提交了业务文档；对应source/组件回归保留。新document状态只在内存，UST1/UIL1不保存业务name/单元格，更不是完整文档格式。启动恢复未结束时修改明确失败，不排队覆盖新提交。

## 2. minimal_eda

| 功能 | UI入口 | 能力ID／效果 | 业务与回读 | 自动证据／状态 |
| --- | --- | --- | --- | --- |
| 文档/对象枚举 | read/Assistant/F6 | eda.document.read／read | note及最多64个对象的稳定字符串ID、x/y/width/height | p2_eda、p2_apps；已接入 |
| 视图/选择读取 | read/Assistant/F6 | eda.view.read／read | zoom、现存对象selection、draft标志及独立revision | p2_eda；已接入 |
| 加固定矩形 | 原菜单、工具栏、画布A键 | eda.add_block／write | 保留原网格坐标/固定尺寸/64上限；ID递增且clear后不重用 | p2_eda实际几何和ID回读；物理待验 |
| 放大 | 原菜单、工具栏、画布Z键 | eda.zoom_in／view | 原×1.1、上限4；不升document revision | p2_eda；物理待验 |
| 清空及复位zoom | 原菜单、工具栏、画布C键 | eda.clear／write | 原清对象及重置zoom=1的UI效果，清选择；无跨资源原子事务承诺 | p2_eda；物理待验 |
| 现有对象选择 | 通用原生画布点击 | eda.select／view | 按实际视口/zoom命中已有矩形，完整ids参数；仅单选 | p2_eda ID验证/失效ID拒绝；真实坐标命中待人工 |
| 笔记编辑 | 原生多行EDIT | 私有draft；不调用write | 原readme作初始已提交值；EN_CHANGE只记录base/generation/dirty | p2_eda原生编辑通知；非physical |
| 提交笔记 | 新Submit原生按钮 | eda.note.submit／write | 完整text/base_revision/generation；旧base零修改失败 | p2_eda actual committed note及冲突；物理待验 |
| 取消/保留冲突草稿 | 新Cancel/reload、Keep draft按钮 | eda.note.cancel、eda.note.keep／draft | 重新加载模型或明确保留草稿后再提交；不假报文档修改 | p2_eda；物理待验 |
| 激活、WGL、resize/DPI、关闭 | 原canvas/属性窗口 | 原生命周期/显示设施 | 原GL内容槽；无额外worker或业务执行器 | p2_apps隐藏真实窗口WGL双实例；真实DPI待人工 |

没有新增任意几何编辑、任意文件/网络、EDA业务对象持久格式或专业应用执行器。原生EDIT为应用收参，使用框架已有NATIVE_BUTTON来源；不假称C2B已自动适配任意原生编辑框。

## 3. generic_components与framework_features

两包分别编译同一源的默认/`UI_SAMPLE_API4`分支。保留100000×16表格、100根各1000子节点、原64项修改表、图像/样式预览及后台缩略图。新增有效属性模型name/size/enabled/mode/notes，初值Untitled/1/true/自动/空。与新模型有关的旧空初值测试改为全选后替换，原完整值断言不放宽。

| 功能 | UI入口 | 能力ID／效果 | 业务及参数 | 自动证据／状态 |
| --- | --- | --- | --- | --- |
| 完整业务读取 | 生成read/Assistant/F6 | demo.document.read／read | 实际属性与64项修改的id/column/text；不以operations证明 | p2_generic、p2_features、p2_apps |
| 视图读取 | read/Assistant/F6 | demo.view.read／read | 实际table/tree selection、image_version、modal、close policy | 两分支回读 |
| 对话框 | 原菜单、两工具条入口、Ctrl+D、节点菜单 | demo.dialog／view | 同一属性模型投影，独立C2B草稿 | 两分支绑定及韧性回归；物理待验 |
| 图片刷新 | 原菜单/工具栏/节点菜单；API4 Ctrl+O | demo.refresh／view | 原图片cache版本和source重查；不声称像素已全部异步到齐 | 两分支合法view执行回读；程序Assistant明确拒绝+版本未变 |
| 属性提交 | 标准FORM/DIALOG、原Submit菜单/工具栏别名 | demo.submit／write | 完整typed fields；size必须有限且>0；菜单同样检查draft base/conflict | 两分支模型、非法值、保留私有草稿；C2B回归 |
| 文本/布尔格编辑 | CELL_EDIT、CELL_TOGGLE；原Edit菜单/工具栏 | demo.edit／write | 完整64位ID/column/text；菜单从已选表格行及name草稿收参，缺选择失败 | 两分支真实bool/text source回读、字节/NUL拒绝 |
| 选择 | 表格/树标准入口、导航及原Select菜单别名 | demo.select／view | 完整component/id；保留原单选，键盘行方向按完整数据源计算 | 两分支选择、私有导航和树展开回归 |
| 树重命名 | F2/Enter、原Rename菜单/工具栏 | demo.rename／write | 原修改表中真实title，菜单取当前树选择/name草稿 | 两分支实际树名回读及非法ID拒绝 |
| 节点菜单 | 右击、原Context菜单/工具栏 | demo.context／view | 完整component/id，复用通用popup和同一菜单绑定 | 两分支真实Light右键链/打开popup回归（非physical） |
| 取消/冲突 | 标准Cancel、Reload/Keep；原Cancel菜单别名 | demo.cancel／view及所属C2B draft控制 | 不写模型；modal内仅所属组件入口可用 | C2B与韧性回归 |
| 七组菜单别名 | API4 File/Edit/View/Tools/Cell/Layer/Help | feature.menu.0…6引用上述前7项 | 无第二注册/权限/handler | features单独编译；物理逐项待验 |
| 13个工具栏别名 | API4 feature.tool.00…12 | 引用commands[i%8] | 每个别名独立binding，复用同一collector及capability | features单独编译/绑定；溢出物理待验 |
| 拒绝/允许关闭诊断 | API4新增可见Close test菜单（原命令功能保留） | demo.close_refuse、demo.close_allow／view | 原refuse_close，普通Assistant不能调用修改 | p2_features真实close决策验证 |
| GL、浮动/停靠、分页/展开/预览、resize | 原标准框架显示设施 | 显示操作，不伪装为文档write | 原表格/树容量、图片版本/布局；没有专业样式编辑模型 | 原组件/菜单/韧性自动回归；两真实DLL正常GL+API4原NO_GL分支 |

“刷新完成”表示已有视图source/cache版本生效，不承诺后台所有缩略图像素到齐。缩略图worker是独立显示设施，晚结果仍走原generation/request检查；关闭由应用确认worker真正退出后ack，UI不join。原Close test命令不因只读政策消失，转为可见人工诊断入口。

## 4. web_counter

| 功能 | UI入口 | 能力ID／效果 | 业务/证据 | 状态 |
| --- | --- | --- | --- | --- |
| 读取 | read/Assistant/F6 | counter.read／read | 实际count及revision；此count本身即本示例业务模型 | p2_counter、p2_apps |
| 增加 | 原菜单、Light add按钮 | counter.increment／write | 单一counter_run；真实模型++并投影；UINT_MAX明确失败不回绕 | 私有逻辑正例、公有模拟/普通invoke拒绝；物理待验 |
| 装载、激活、关闭 | 原生命周期 | 不新增业务能力 | 原Light页面，无worker或文件格式 | 双实例真实DLL、关闭卸载通过 |

## 5. 完成边界及待验

55项描述（22+9+10+12+2）由对应编译分支独立注册；11项read投影与44项非read人工能力，五个应用均启用C1只读兼容政策。既有窗口/控件显示API仍允许应用投影UI；这不赋予Agent调用修改能力的资格，也不把显示更新当业务写入。

已实现与自动证据详见[本轮独立记录](validation/xuanshu-p2-remaining-validation.md)。原物理/SendInput及直接Assistant修改的旧脚本保留；不能当新来源正例。其未运行项目和迁移后的测试差异逐项记录，不宣称全套CTest绿色。

真实菜单/工具栏/快捷键/表单/对话框/画布、Windows来源动态分类和实际DPI/IME留[集中人工清单](xuanshu-p2-manual-acceptance.md)。代码/自动准备不等于P2完整验收；WebView2、外部业务应用、浏览器业务及P3以后仍不在本轮实现内。


## 6. P4BC-1实现前覆盖冻结（2026-10-26）

本节为原55项功能的P4增量列，先冻结后实现；2026-10-26已完成下列适用能力接入，实际值与证据见第7节，不把P2人工接入当AI事务。UI别名/收参/唯一run沿前述每行，不另设权威目录。

| 实际功能及能力ID | 效果/风险/业务实现 | 事务/撤销及授权 | UI入口/验收计划 |
|---|---|---|---|
| stateful test.edit | write/普通，state_run稀疏单元格 | 应用before/staged/undo，宿主范围确认 | 原单元格/菜单，同值与非数值/非法ID/AI值回读 |
| stateful test.submit、test.modal.close | write/普通，共享name/mode/color | 同文档事务，私有草稿不入快照，revision冲突显式解决 | FORM/DIALOG及AI提案，独立草稿/回读/undo |
| stateful树 | 原source只读显示，无rename handler | 不新增业务，不假标已迁移树写 | 原树显示/展开；实际树名编辑见下方两分支 |
| state.save、language.zh、language.en | 既有固定路径UST1/UIL1，文件及偏好副作用/不可撤销 | POLICY_ONLY：AI逐项审批/单次消费后拒绝；人工原C2路径后台执行不删改 | 原菜单，临时profile文件回读；不能称普通内存write |
| eda.add_block | write/普通，原固定矩形与递增ID | 单文档事务，undo不回退ID高水位 | 菜单/工具栏/A键与AI，几何和对象ID回读 |
| eda.clear | write/destructive，原清对象及复位视图 | APPROVED_DESTRUCTIVE；任务范围＋5分钟逐项批准，应用事务/undo | 原菜单/工具栏/C键，拒绝0执行、批准后对象空/undo还原 |
| eda.note.submit | write/普通，原committed_note | 同文档事务及undo，保留本地草稿并标冲突 | 原Submit/Cancel/Keep与AI；base/generation/实际笔记 |
| generic demo.submit、demo.edit、demo.rename | write/普通，sample_run属性/修改表/树名 | 应用具体快照与回执，字段类型/完整ID/容量不放宽 | FORM/DIALOG/单元格/树/菜单/工具栏与AI实际回读/undo |
| features同三项（UI_SAMPLE_API4） | 同源独立编译/额外别名保留 | 单独验证，不用generic成功替代 | 七菜单/十三工具栏既有alias及真实包 |
| counter.increment | write/普通，counter_run真实count | 应用count快照/revision/undo，UINT_MAX不回绕 | 原菜单/Light按钮与AI，count具体值及revision |
| 其余read/view/draft | 原第1—4节全部入口，不降级/删除功能 | 不开放AI view/draft，Assistant/read视图仍只读 | 原P2/C2回归及真正控制面板；不把显示当修改 |

13项内存write＋3项文件副作用均须有明确实现/拒绝及证据后才更新为完成。状态/矩阵不可因实现困难缩减；文件AI拒绝是本轮明确执行边界而非伪装已实现可撤销文件编辑。未适配物理输入按原清单保留，WebView2不扩展。

## 7. P4真实接入结果（原55项不减项）

原UI别名、完整收参和唯一run见第1—4节；以下是新增受控编辑/真实事务列。同源隔离运行库只替换运行库链接，不替换应用run/模型/收参源码；原生产包的人工与read回归另有ui_p2_apps及五逻辑分支。

| 应用/能力ID | 实际业务证明、事务和撤销 | 授权/UI/证据 |
|---|---|---|
| stateful/test.edit | 人工55→取消99仍55→AI66→undo55；稀疏cell完整ID、revision及应用回执 | 原单元格/菜单参数与Service同run；ui_ai_apps_stateful_components |
| stateful/test.submit | name/mode/color实际模型；FORM/DIALOG各保留私有草稿并冲突，undo回Untitled后显式Keep/Submit | 原FORM与宿主范围确认，非控件计数判成功 |
| stateful/test.modal.close | 通过同一name业务属性提交shared dialog name，再undo原name | 原DIALOG收参保留；真正modal打开仍受执行门拒绝，不伪装read调用 |
| stateful/state.save | fixed-path-local-filesystem/irreversible，AI批准消费后明确effect_not_enabled、NONE | 既有人工后台文件入口保留；旧stateful临时profile I/O回归 |
| stateful/language.zh | 同上，不把UIL1语言文件称文档事务 | 原人工语言菜单保留；AI政策拒绝 |
| stateful/language.en | 同上，最终存储失败UNKNOWN后新只读证明/退出核对解除，无虚假txn | 政策回读原preferences资源，不执行文件handler |
| EDA/eda.add_block | 原固定矩形，ID1→2→undo1；取消的ID3不复用，下次ID4；commit前回读仍旧模型 | 原菜单/工具栏/A与AI同eda_run；ui_ai_apps_minimal_eda |
| EDA/eda.clear | 拒绝审批不改对象；批准后空、undo恢复ID1；空对象仍按原行为复位zoom | destructive逐项审批，undo也新审批；不扩写几何编辑 |
| EDA/eda.note.submit | committed AI note、revision和undo；原生EDIT草稿文字不被AI清掉，dirty/conflict真实回读 | 模拟SetWindowText+EN_CHANGE仅草稿，不签发人工资格；Submit/Cancel/Keep原绑定保留 |
| generic/demo.submit | 类型化name/size/enabled/mode/notes实际值和undo；独立草稿冲突/Keep | 原FORM/DIALOG；ui_ai_apps_generic_components |
| generic/demo.edit | checked实际true，undo恢复changes=[]，revision递增 | 原稀疏表业务与AI同run |
| generic/demo.rename | 取消保留原树名，AI Real tree name，undo恢复；另断言tree-label-1投影 | 原tree完整ID/菜单/工具栏，非仅显示判成功 |
| features/demo.submit | 同源UI_SAMPLE_API4独立编译；实际字段/undo/草稿 | ui_ai_apps_framework_features及features_no_gl分别验证 |
| features/demo.edit | 独立分支checked模型true/undo、原菜单/工具栏别名 | 原P2 features389检查另回归 |
| features/demo.rename | 独立树名取消/commit/undo及实际tree-label-1投影 | 原别名未删减，不借generic通过替代 |
| counter/counter.increment | 人工1→取消仍1→AI2→undo1，实际count/revision/回执 | 原Light按钮/菜单与同counter_run；ui_ai_apps_web_counter |
| 其余39项read/view/draft及生命周期 | 按原第1—5节实际效果，不新增业务write/AI权限 | Assistant/旧工具视图仍read-only，C1/P2回归保留 |

应用before/staged/undo是各自内存状态，不是新文档格式或框架通用撤销栈。每次undo重新查当前revision与回执；应用重开不复用旧epoch/内存undo。EDA clear的undo恢复文档对象/笔记，不恢复历史zoom/selection。stateful实际树无rename注册，因此其树显示不适用AI write；实际树名编辑由两个sample分支覆盖，未删功能或新增业务。

本轮是P4开发阶段模拟/自动接入，真实物理入口及体验仍按[P4延期表](xuanshu-p4-deferred-manual-acceptance.md)。精确阶段、失败及ASAN展示诊断见[独立验收](validation/xuanshu-p4-validation.md)。


## 2026-10-27 P5覆盖承接

原55能力/P4逐能力矩阵不删除。本轮五维护包只新增可选application.specialist纯数据配置，选择原read和document write；业务run/事务/撤销及UI入口没有再迁移或增设第二目录。Coordinator真实五包协作与具体值、对象/树名/revision/回执、编译分支和browser原规划缺口见[xuanshu-p5-application-coverage.md](xuanshu-p5-application-coverage.md)。开发阶段自动通过不等于真实物理或全部历史应用迁移。
