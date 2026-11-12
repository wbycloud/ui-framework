# 通用应用助手接入指南

2026-11-12。当前开发接口为SDK 0.10.0开发版、API10、标准10、ABI1、包格式1。应用提供业务模型、资源和能力，正式宿主管理一个workspace、Service、Provider、确认、存储及模型轮次。开发总入口与基础UI合同见应用开发标准。[应用开发标准](application-development-standard.md)

## 匹配的开发材料

本地匹配冻结交付为`build/xuanshu-general-layout-20261112-6b37c1`，delivery-identity SHA256为`2C447E95741824E2C2A94E3EF91EC1782CF75C5870E262D7831C44989C7029CA`。本轮逐项核对当前公共头所在447份源码范围、生产DLL及导入库身份。开发应成套使用这些头、`light-final/ui_framework_runtime.lib`、`ui_framework.dll`和宿主/包。实际程序路径及哈希见清单，冻结范围见独立记录。[匹配清单](xuanshu-general-assistant-delivery-manifest.md)、[冻结记录](xuanshu-general-layout-freeze.md)

API10内的Service、P4、P5及通用助手采用独立头和size/version结构追加。同为API10的早期冻结SDK可能缺这些声明和导出。对外接入应取得维护方提供的匹配源码/SDK及清单，记录commit、工作区增量和文件哈希。当前未提交快照的完整身份由交付清单关联，GitHub main的名称本身不足以选定它。SDK获取与构建条件见构建文档。[构建与匹配交付](build-and-validation.md)

## 当前支持及选择粒度

| 应用或层 | 资源、能力与性质 | 结果、事务及当前边界 | 入口 |
|---|---|---|---|
| 普通C/Web应用 | 公共application、components、shell及能力注册 | 应用拥有真实数据与业务函数，UI、菜单、工具、快捷键和AI复用该函数 | [Web Counter](../examples/web_counter/README.md) |
| 浏览器快照 | browser.view，browser.snapshot，read | 选择state会包含完整顶层状态对象，含title、address及加载/布局诊断。revision单独可选，find_text也是目录定义的独立顶层字段 | [浏览器源码](../apps/browser/src/browser_capabilities.inc) |
| 浏览器正文 | browser.view，browser.body，read | body含text、bytes、complete和semantics，另有revision。DOM规范化正文最多8192 UTF-8字节，超限/不可用明确失败，语义为dom-body-v1 | [浏览器接入](../apps/browser/README.md) |
| 维护组件 | state.document，state.document.read/state.cell.read，read，test.edit，write | 文档和实际单元格结果来自应用内存。选定顶层字段如revision、changed_cells、text。编辑沿P4事务及独立撤销，示例对象ID用十进制字符串 | [维护组件](../examples/stateful_components/README.md) |
| 最小计数器 | counter.document，counter.read/read，counter.increment/write | 返回count/revision。按钮与事务复用counter_run，P4回读及撤销 | [计数器](../examples/web_counter/README.md) |
| 通用组件/功能示例 | demo.document及各自目录中的read/编辑能力 | generic_components和framework_features分别按源码分支注册，维护示例覆盖其已有编辑功能 | [维护应用矩阵](xuanshu-p2-application-coverage.md) |
| 最小EDA | eda.document及已有read/固定矩形、清空、笔记能力 | 沿既有事务、风险和撤销。完整EDA文档/几何能力仍待应用接入 | [原P4/P5覆盖](xuanshu-p5-application-coverage.md) |

浏览器导航、back/forward/reload/stop/home/help及settings-save/reset九项副作用保持各自原人工入口，完整AI副作用迁移另案。播放器、游戏及其他未注册业务能力按应用逐项接入。PNG附件当前仅本地预览，文本附件有独立有界快照合同。完整业务闭环的离线证据使用实际browser与维护组件，模型和可信事件为隔离替身。[通用助手合同](xuanshu-general-assistant-contract.md)、[开发验收](validation/xuanshu-general-assistant-validation.md)

## APP初始化与注册

1. 在应用`create`分配私有状态，保留context的合法生命周期，按稳定业务身份准备资源。`ui_capability_register_resource`登记资源类型、稳定id、open_epoch和`get_revision`。未提供open_epoch时框架生成新打开UUID。revision由应用真实模型取得，注册回调可以同步重入，业务大ID使用字符串。
2. 用`ui_capability_register`登记v1 read，声明精确input/output Schema和check/execute/cancel。C1只读兼容入口使用`ui_capability_enable_read_entries`、`ui_capability_publish_read_entry`。v1的read scope保持原语义，其他效果使用相应独立扩展。
3. 需要受控写时，用`ui_capability_register_edit`注册唯一业务执行器及transaction回调。菜单绑定使用`ui_capability_ui_bind`，Light控件/快捷键使用`ui_capability_input_bind`，表单和组件语义分别使用forms/component_inputs头。绑定收集明确参数，业务check仍负责对象、只读、模态、busy和实际revision。
4. 在资源及能力注册后用`ui_ai_agent_profile_register`登记专业角色的说明和允许能力数组。字段被复制，角色与能力/目标/根范围取权限交集。示例角色为browser.reader和maintenance.editor。角色文本和模型输出只作为候选数据。
5. `mount`取得实际内容槽、创建应用内容和普通UI绑定。需要包资源时通过`ui_app_resource_read`在合法已挂载实例读取，并由`ui_app_resource_release`归还。应用自绘工具可按`ui_shell_get_chrome_suppressed`协作收起，保留业务内容及状态HUD。

完整调用位于真实应用源码及公共头。最小读链、UI绑定、P4和专业角色分别对应明确接口，按所需层接入。[capability.h](../include/ui_framework/capability.h)、[capability_ui.h](../include/ui_framework/capability_ui.h)、[capability_inputs.h](../include/ui_framework/capability_inputs.h)、[capability_forms.h](../include/ui_framework/capability_forms.h)、[capability_component_inputs.h](../include/ui_framework/capability_component_inputs.h)、[capability_edit.h](../include/ui_framework/capability_edit.h)、[ai_coordinator.h](../include/ui_framework/ai_coordinator.h)

## read执行、异步及注销顺序

调用方先用`ui_capability_bind`取得固定目标，再`ui_capability_prepare`复制输入并取得稳定call_id。`dispatch`重新核对资源和业务状态，其返回OK表示已接纳执行，业务结果由`ui_capability_get_result`读取。目标包含实例、host/open epoch、资源、expected_revision及scope。切换活动标签只改变当前目录选择，已接纳任务保持原绑定。

应用execute可以同步`ui_capability_complete`，或启动协作异步工作。后台通过app_data复制数据和稳定ID至UI接收器，实际worker和外部回调退出后，在UI线程complete。取消请求只改变协作状态，仍需真实完成。get_result的output_json借用至forget或host结束，复制需要保留的结果后再`ui_capability_forget`。PENDING期间继续由UI poll推进。[app_data.h](../include/ui_framework/app_data.h)、[app_poll.h](../include/ui_framework/app_poll.h)

关闭时先停止新业务接纳和新派发，再请求取消/排空，归还terminal调用记录。UI绑定按begin_unbind/finish_unbind两阶段退役，能力按begin_unregistration/finish_unregistration退役，再完成资源注销。期间PENDING保留回调和user_data。专业Profile通过`ui_ai_agent_profile_unregister`撤销，正在使用的元数据与指纹仍按运行时检查。注销成功且全部worker/回调退栈后才释放应用状态，宿主继续处理WAIT至可卸载。[生命周期与所有权](application-development-standard.md#2-应用包和生命周期)

## P4事务与撤销

`ui_cap_edit_desc_t`包含v2能力描述、risk和transaction回调，事务阶段包括BEGIN、EXECUTE、COMMIT、ROLLBACK、READBACK、UNDO和RECONCILE。应用用`ui_capability_edit_complete`复制输出及transaction/undo回执，executor_exited说明真实执行者已经退出。调用方同时核对效果、实际读回、revision及保存事实。UNKNOWN保留未知效果和资源门，人工事实核对沿原reconcile合同执行。[受控事务公共头](../include/ui_framework/capability_edit.h)、[P4合同](xuanshu-p4-controlled-edit-contract.md)

初始模型请求确认、具体有限计划确认、危险动作审批及后续撤销各有适用范围。通用助手的`ui_ai_assistant_bind_node_undo`绑定一次明确本地撤销显示及可信来源。撤销回执与模型回复分开展示，取消请求与远端停止、终态及最终保存各有独立事实。

## 正式宿主的Service顺序

普通APP复用正式宿主现有Service。下列步骤供实现独立宿主的开发者参考。

1. 创建workspace及同一个`ui_ai_service_create`。在open之前调用`ui_ai_assistant_enable`固定根请求、期限、费用及计量策略。参数来自宿主明确策略。
2. 打开Service并持续`ui_ai_service_poll`/workspace poll，等实际READY或明确错误。新通用库使用既有schema3，旧库按原显式迁移合同处理。
3. 沿`ui_ai_edit_enable`与`ui_ai_coordinator_enable`完成P4/Coordinator存储检查，等待各自可用。正式宿主的tick代码给出异步推进顺序。
4. `ui_ai_assistant_catalog`取得当前应用资源、Schema和Profile。每个固定目标的permissions包含capabilities、profiles及可选fields、capability-keyed parameters。`ui_ai_assistant_propose`复制范围和附件。收到与持久化接纳分别处理，保存失败保留草稿与选择。
5. 用`ui_ai_assistant_get_disclosure`展示初始完整请求，按实际run/hash、代次、文字、布局及末次回调后条件建立可信Light绑定。初始请求经本人确认后，模型提出的有限计划再由Coordinator核对并完整显示，另行确认。
6. `ui_ai_assistant_get`投影当前阶段、问题、节点结果、revision、回执、占用和保存事实。问题答复沿`ui_ai_graph_answer`携带root/node/question/operation精确关联。已查看任务、当前草稿及当前配置保持分离。
7. 手动接管调用`ui_ai_assistant_control`。保持交接中，直至网络、能力调用、P4、图保存及所属Coordinator引用排空后才恢复手动操作。交回AI重新核对实际身份、revision和描述指纹。
8. 请求Service关闭后继续poll至可销毁，`ui_ai_service_destroy`返回PENDING时保留对象。Service安全结束后再关闭workspace并等待应用实例及模块卸载。重开只恢复记录，旧授权和工具执行保持失效。

方法和任务租约覆盖同步回调及外层退栈，关闭不能提前销毁。正式宿主实现与公共接口分别可查。[宿主tick](../examples/framework_host/ai_panel.inc)、[ai_service.h](../include/ui_framework/ai_service.h)、[ai_assistant.h](../include/ui_framework/ai_assistant.h)

## 结构、所有权与错误

| 接口层 | 结构版本与所有权 |
|---|---|
| 资源、v1能力/目标/结果 | 完整size，version=1。注册复制元数据，回调和user_data由应用保持至成功注销。complete借用输出至返回并复制 |
| capability_v2 | 目标/能力/完成/结果按version=2使用，原v1数值与布局保持 |
| P4 edit、Profile、Service及通用助手独立结构 | 各自完整size/version=1，内嵌v2结构仍设2。按头文件约定逐字段初始化 |
| capability分配JSON | 使用ui_capability_release |
| Service/助手/图/Profile分配JSON | 使用ui_ai_release |
| 回调参数、context、event payload | 当前回调内借用，异步保留时复制。跨CRT归还给原分配方 |

UI线程接口包括注册、prepare/dispatch/complete、结果读取、绑定及Service poll。嵌套poll返回PENDING。状态码与业务outcome分别检查，LIMIT_EXCEEDED/STALE/CANCELLED/UNKNOWN保留其含义。发现UNKNOWN或存储失败后沿cleanup-only收尾。调用重入和实例注销保持公共合同。[基础线程合同](application-development-standard.md#3-ui-线程utf-8-和-c-abi)

## 显示、授权及根保护

完整摘要由当前任务权威快照生成，完整请求详情仍须成功构建并经宿主实际回读。`display_valid=true`表示一次状态投影完成。确认还要求run/hash、显示代次、实际文本、布局、绑定及最后一次回调之后的运行时条件。详情或范围隐藏、投影失败、中断、条件变化及解绑PENDING使旧资格失效。恢复展示后由宿主重新核对。页面消息、缓存、source字符串、Profile文本及模型输出均不能签发人工资格。[完整投影合同](xuanshu-uha-projection-closeout-contract.md)、[显示缓存合同](xuanshu-uhs-cache-closeout-contract.md)

请求数、期限、费用和可靠计量是特定宿主/根策略。历史BR-2三次和具体Proposed包八次各有适用任务范围。APP声明能力时保持这些保护独立，所有模型及专业续轮经过同一发送前边界。配置ready、本地凭据可读和服务端验证分别展示。

## 示例、数据与证据

从公共目录示例开始，随后阅读计数器的资源/能力及维护组件事务，最后参考browser真实文档链。目录示例参数依次为匹配包和全新数据库路径，父目录由调用方准备。[目录示例](../examples/general_assistant/README.md)、[计数器](../examples/web_counter/README.md)、[维护组件](../examples/stateful_components/README.md)、[浏览器](../apps/browser/README.md)

UTF-8文本附件最多4份、每份16KiB，保存名称、类型、字节数、SHA256及快照。PNG最多256KiB仅本地预览。默认历史不选，跨应用上下文必须进入明确根范围。模型结果区分解读、原文事实/待办和推断。

`tests/general_host.c`是隔离端到端参考，使用正式页面、实际browser/P4应用及模型/可信事件替身。本轮仅静态核对代码、符号和链接。最新既有离线结果及其首败、真实来源和真实服务边界见独立复审。[隔离流程源码](../tests/general_host.c)、[限定复审](validation/xuanshu-general-layout-independent-review.md)
