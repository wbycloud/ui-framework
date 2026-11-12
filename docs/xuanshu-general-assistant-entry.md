# 当前入口：匹配快照冻结，日常与真实验证分开

2026-11-12。当前匹配快照已限定复审通过并记录冻结，见[冻结记录](xuanshu-general-layout-freeze.md)。在仓库根可核对以下日常隔离入口；本轮没有执行。

```powershell
powershell.exe -NoProfile -File ./build/xuanshu-general-layout-20261112-6b37c1/start-general.ps1 -IsolatedProfile
```

该入口创建唯一隔离Profile及会话，默认移除通用计价策略，真实发送保持锁停。实际程序与依赖见[清单](xuanshu-general-assistant-delivery-manifest.md)。不复制旧Key、账户引用或用户库。

新的[具体真实验证包](../build/xuanshu-general-layout-review-20261112-a74e20/authorization-package.md)及其launch-after-approval.ps1为单独准备材料，当前Proposed；新命令只静态解析，未动态运行。不能用日常入口继承UI_AI_ASSISTANT_POLICY后宣称真实保护已启用。只在用户另行批准具体包后，才允许一次匹配生产启动及适用可信确认。当前没有待用户录Key或点击的运行窗口。

## 此前时点原文保留

# 当前入口：Light字形测量整改候选

2026-11-12。当前候选为`build/xuanshu-general-layout-20261112-6b37c1`，最小修复及相称自动验证完成，等待限定独立复审。详[本轮因果与验收记录](validation/xuanshu-general-layout-causal-validation.md)及[交付清单](xuanshu-general-assistant-delivery-manifest.md)。原先ASAN完整业务期限的待收口状态由本轮结果更新，历史失败不删除。

工作目录为仓库根，下列命令仅供后续核对，本轮没有执行生产入口：

```powershell
powershell.exe -NoProfile -File .\build\xuanshu-general-layout-20261112-6b37c1\start-general.ps1 -IsolatedProfile
```

入口指向`light-final/framework_host.exe`、生产`ui_framework.dll`、实际浏览器合成样本包`br1-production.uapp`及维护组件`stateful_components.uapp`；计量组件和固定OSMesa按launch-files核对。监督器逐字复用，启动器仅把相对路径light/改为light-final/，本轮只做静态核对，没有运行这条新路径。

`-IsolatedProfile`新建独立Profile、唯一会话和自有应用样本。数据库、浏览器来源及维护样本继续按原启动监督配置，未核价保持真实发送锁停；不继承历史真实调用授权。记录PID、stdout/stderr、实际退出码和异常，不自动重启。具体操作及取消/关闭说明沿后文原入口合同，真实验证须新的具体授权包及适用可信确认。

## 历史入口与结论保留

# 通用应用助手首版使用与交付入口

## 2026-11-12唯一ASAN复测后状态

当前候选程序身份未变。修正外部驱动环境删除方式后，一次有效`ui_general_host_business`复测仍在30.0711秒超时，未形成通过补项。两项直接边界回归保持，完整ASAN业务验证尚未收口；详[唯一复测、关后事实与限制](validation/xuanshu-general-asan-retest-validation.md)。本轮没有执行下列生产入口，没有真实请求或人工验收，真实质量包仍Proposed。

## 2026-11-12边界整改候选及诊断入口

候选`build/xuanshu-general-boundary-20261112-9d6c53`保存两项边界修复及直接回归。完整ASAN业务期限仍有阻断，见[当前验收](validation/xuanshu-general-boundary-validation.md)。单一入口start-general.ps1和supervise.ps1逐字复用，launch-files关联本候选生产程序。命令仅作说明，本轮没有执行生产启动。

```powershell
powershell.exe -NoProfile -File ./build/xuanshu-general-boundary-20261112-9d6c53/start-general.ps1 -IsolatedProfile
```

默认未核价锁停，新建隔离Profile、会话和样本。图read结果保存及所属引用归还前显示交接中，成功后恢复手动操作，保存失败保留未保存事实。真实质量包仍Proposed，候选等待剩余验证收口及后续限定独立复审。

## 2026-11-12整改候选入口

当前整改候选为`build/xuanshu-general-closeout-20261112-4f8a21`，等待再次限定独立复审。匹配程序及使用范围见[交付清单](xuanshu-general-assistant-delivery-manifest.md)，三项整改及自动验证见[新验收记录](validation/xuanshu-general-closeout-validation.md)。原候选入口和历史普通生产观察保留在后文。

从仓库根使用以下匹配候选入口。此处仅提供命令，本轮没有执行生产启动。

```powershell
powershell.exe -NoProfile -File .\build\xuanshu-general-closeout-20261112-4f8a21\start-general.ps1 -IsolatedProfile
```

入口使用新隔离Profile、新会话库及应用样本，默认未核价保持发送锁停。启动器和监督器与前置逐字一致，程序及launch-files由新候选独立索引。手动接管后须等待真实“应用已交还手动操作”状态及在途保存完成。有限计划完整详情在末次应用回调之后仍须有效，重新显示后由宿主重新读回。关闭恢复继续只展示记录。

## 原首版入口与观察保留

2026-11-11。候选目录为`build/xuanshu-general-assistant-20261111-7a2f90`，开发及验证完成，匹配身份封装后进入限定独立复审。默认价格尚待核实，真实发送保持锁停。

## 单一启动入口

在仓库根目录使用以下入口。入口已通过专门新隔离目录的普通生产驱动核对，实际PID22372、exit0。

```powershell
powershell.exe -NoProfile -File .\build\xuanshu-general-assistant-20261111-7a2f90\start-general.ps1
```

入口核对`launch-files.json`中的生产文件和监督器，创建唯一新会话库及应用样本目录。正式宿主加载生产`ui_framework.dll`、`br1-production.uapp`及`stateful_components.uapp`。浏览器打开包内合成交接单，来源配置为空列表。维护组件使用匹配OSMesa固定依赖。

`UI_AI_DB_PATH`及`UI_BROWSER_SETTINGS_PATH`指向新会话，维护组件状态位于新的应用样本目录。普通入口显式采用稳定`daily-default` Profile，独立于会话数据库。`-IsolatedProfile`创建单独Profile及账户身份。首次使用由用户通过原生入口配置凭据。入口保留PID、stdout、stderr、实际退出码及分阶段异常，恢复临时环境，不循环重启。

## 日常操作

顶部“AI”展开左侧助手，右侧保留业务内容及HUD。常规菜单和工具收起，收起助手后恢复。“设置”和“开发诊断”按需展开。

“当前任务与处理状态”展示正在查看的记录与保存事实。用户消息靠右，助手消息靠左，正文左对齐。选择旧任务保持“新任务草稿”。“任务列表”可按需展开。

在“任务输入（填写要本地保存的任务内容）”填写目标。通过“选择新任务范围”和“选择当前应用的目标与能力”选择资源、能力、字段及可选专业角色。Schema参数卡填写明确值，准备模型也能在范围内补充参数。使用“加入／更新此草稿目标”固定草稿目标清单。历史默认不选。

“保存任务并查看范围”接纳本地提案。收到与持久化接纳分别报告。保存失败时保留草稿和选择，核对错误后再决定后续动作。

“查看本任务范围（只查看，不授权）”展示实际输入、历史、固定目标、能力字段、绑定配置及完整请求。初始请求和随后具体有限计划分别经过可信人工确认门。专业补充问题携带所属任务和节点，答案精确路由。具体计划确认后固定执行，参数或目标变化需要新计划。

“查看业务结果与依据”展示节点、revision、事务、撤销回执和保存事实。可用时通过“查看此步骤的撤销范围”核对新的本地撤销操作，再进行单独确认。编辑草稿保持原任务范围。

“手动接管应用”先阻止新冲突派发，待在途完成或取消、引用归还后显示手动可用。“交回AI”重检目标和版本，状态变化使旧计划失效。收起助手保持任务。

“请求取消当前任务”发出取消请求，仍须等待终态与最终保存，远端unknown单独保留。“复制脱敏诊断”保留任务原错误，复制错误单独显示。

通过窗口关闭入口正常关闭并等待真实退出。恢复只展示记录和配置，旧授权及执行保持失效。发生异常时保留本次目录及全部旁文件。

## 保护与支持边界

未核价时可以保存本地提案，真实发送锁停。配置ready、本地凭据可读和服务端验证分别展示。Profile复用保持逐任务确认要求。`display_valid`只表示状态投影完成，完整内容、run/hash、代次、布局、运行时及末次回调后的核对共同决定资格。摘要、缓存或页面消息不能替代完整预览。隐藏、重建或中断立即失效，恢复后由宿主重新读回。

文本附件以快照进入任务，PNG仅本地预览。结果区分模型解读、原文事实/待办和推断。开发接入及支持矩阵见[接入标准](xuanshu-general-assistant-integration.md)，证据及限制见[开发验收记录](validation/xuanshu-general-assistant-validation.md)，真实质量验证仍须[具体授权准备](xuanshu-general-assistant-real-proposal.md)。


## 运行材料

[候选交付清单](xuanshu-general-assistant-delivery-manifest.md)列明实际文件、SHA256及依赖。当前候选的真实模型发送保持未核价锁停。正常保存接纳后，新草稿与原任务输入分开。会话库、应用样本和日志按唯一新目录保存，稳定日常Profile单独复用。

发生错误时保留错误原文、脱敏诊断和本次目录全部旁文件。保存事实及远端状态按实际记录核对。独立复审及新的真实质量验证各有自己的适用范围。
