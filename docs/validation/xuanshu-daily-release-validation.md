# 2026-11-24 扫雷日常聊天开发验收

本轮基于工作区 `1ec6b67`，当前交付为 `73b9c1`。与 `f6b2e1` 比较的 20 项输入差异中，15 项属于本轮源码与验证变动，5 项已存在。原 SDK ZIP 的 SHA256 保持 `0E62E47CE6EAEC8B52BB516F124DD7AE0E60F79C9DD8514D40BF9FB62986476C`。新清单覆盖 628 项源码输入和发行目录 273 个文件。 [交付身份](../../build/xuanshu-daily-release-20261124-73b9c1/delivery-identity.json) · [源码对照](../../build/xuanshu-daily-release-20261124-73b9c1/source-comparison.json)

## 日常流程

- 零目标调用仍进入原助手 Service。明确的空权限、空能力与字段、无附件进入纯文字路径，预览与实际序列化请求一致。工具列表为空，意外函数请求被拒绝。合成答复保存为唯一终态，使用原额度、计量和费用账本。
- 可撤销的「允许读取当前棋局摘要」由发行清单匹配实际扫雷应用。选择时固定实例、epoch 与版本。提交仍用原公开读链。继续阶段和最终模型请求只投影 `state`、`revision`。本地完整业务结果按原合同保留。
- 模型和计价各自显示状态。发行快照使用高峰未命中缓存的 300/1200 nano USD 单价，价格期限截止 2026-12-01 00:00 UTC。期限在原预留和发送前路径检查。绑定配套计量器及原 8 轮、10 分钟、USD 0.10 本地门槛。 [官方价格](https://api-docs.deepseek.com/quick_start/pricing/) · [官方协议](https://api-docs.deepseek.com/api/create-response/)

## 直接证据

- Light 定向 30/30。后续最终验证驱动在 C 盘自有状态目录通过 8/8，其中 6 项为本轮页面验收，2 项复验同一批中的原业务和取消用例。零目标确认与结果、过期锁停、完整预览隐藏、固定实例、保存前版本变化、确认前版本变化和独立的关闭重开分别验证。保持原 30 秒门槛。 [Light JUnit](../../build/xuanshu-daily-release-20261124-73b9c1/scoped-final.junit.xml) · [最终宿主 JUnit](../../build/xuanshu-daily-release-20261124-73b9c1/acceptance.junit.xml)
- 两局持久化工具结果分别为 9×9、10 雷、`board-1` 和 16×16、40 雷、`board-2`。任务、工具的实例与 host/open epoch 及 revision 匹配。关后 immutable 核对主库哈希保持。旁文件独立登记，观测到的 WAL 均为空。过期和旧范围路径均无模拟传输入口。
- 生产 DLL 与生产链接扫雷包的公开 API10 验收 1912 项断言通过。正式 DLL 的 371 项导出与原包名称一致。隔离页面用例使用同一宿主页面、按同一真实应用源码构建并链接隔离运行时的扫雷包，以及模拟网络、凭据和可信事件。 [关后读回](../../build/xuanshu-daily-release-20261124-73b9c1/final-owned-state.json) · [API10 实测](../../build/xuanshu-daily-release-20261124-73b9c1/production-mine-check.log)

## 首败与交付边界

保留首轮过期用例对错误来源的断言失败、Light 对 `align-self` 的不支持，以及 PowerShell 将清空变量传成显式空值所致的夹具拒绝。最终用例从真实缺省策略环境创建 Service。后续单次 D 盘原业务检查记录关后和零断言失败后，进程未在 30 秒内退出。最终 C 盘对照通过，D 盘该次退出延迟的根因未确定。各次日志按原结果保留。

本轮生成新的 [本地入口](../../build/xuanshu-daily-release-20261124-73b9c1/dist/助手使用.md)。正式 `start.cmd` 和配套文件已核对。当前执行环境的 Known Folder 指向日常用户最近应用记录，正式脚本的隔离实启仍待验证。范围为本地开发、离线行为和正式公开读取链。真实 DeepSeek 发送、独立复审与新版本发布未发生。历史 `f6b2e1` 验收和保全记录保持原身份。

## 2026-11-24 启动入口追加验收

追加验证使用一次性 AppContainer，每次先用 `SHGetFolderPathW(CSIDL_LOCAL_APPDATA)` 核对实际路径，再启动正式脚本。正式 `start.cmd` 直接启动时，宿主 PID 32020 加载匹配 DLL 与扫雷模块，退出 0。新增入口实测从仓库外目录调用同字节正式脚本，宿主 PID 19228，加载配套 DLL 和扫雷模块，观察到 1184×724 的可见棋盘原生表面，正常退出 0。测试使用自有会话路径。两轮测试 profile 在关闭后删除。 [直接启动](../../build/xuanshu-daily-release-20261124-73b9c1-entry/profile-smoke.txt) · [新版入口实启](../../build/xuanshu-daily-release-20261124-73b9c1-entry/profile-entry-smoke.txt) · [AppContainer profile API](https://learn.microsoft.com/en-us/windows/win32/api/userenv/nf-userenv-createappcontainerprofile)

聊天输入和棋局摘要沿用同页面的离线 8/8 证据。生产实启保持默认折叠状态，未做桌面输入自动化、录入 Key 或发送模型请求。独立入口和消费端补丁另有 17 项相对路径及缺件验证。扫雷原入口与 README 保留，补丁在副本完成验证。 [启动补证清单](../../build/xuanshu-daily-release-20261124-73b9c1-entry/entry-identity.json) · [离线宿主验证](../../build/xuanshu-daily-release-20261124-73b9c1/acceptance.junit.xml)

## 2026-11-24 棋局读取编号修正

用户截图对应 `73b9c1` 已保存、已确认的一条任务。宿主收到 DeepSeek 完整回复。回复调用了范围内的 `minesweeper.board.read`，给出的目标别名为字符串 `"0"`。原解析器只接受 `target-0` 一类的别名，校验以 `batch_wrapper_invalid` 结束，工具调用数为 0。 [脱敏失败事实](../../build/xuanshu-mine-tool-alias-20261124-8ca6d2/reported-failure.json)

`8ca6d2` 在通用助手读取包装层兼容已固定目标的十进制序号字符串，在规划指令中给出正式 `target-N` 写法。目标仍按已接纳的序号、实例和版本查找，能力及字段仍按原范围校验。已经收到回复时，输入区提示「模型已回复，任务处理未完成」。修前同形回包复现失败，关后库仅有一次替身传输和唯一失败终态。 [修前 JUnit](../../build/xuanshu-mine-tool-alias-20261124-8ca6d2/red/red.junit.xml) · [关后关键信息](../../build/xuanshu-mine-tool-alias-20261124-8ca6d2/closed-fixture-readback.json)

修后 Light 定向 18/18。一局由 `"0"` 读取到 9×9、10 雷、`board-1`，另一局由 `"target-0"` 读取到 16×16、40 雷、`board-2`。两局切标签后仍读取原实例，工具结果和框架版本逐值核对，模型回传投影限 `state`、`revision`。越界序号 `"1"` 保留失败终态、零工具和一次替身传输。实际生产包的公开 API10 验证通过 1912 项断言，生产导出 371 项与前置一致。隔离实启加载匹配的 DLL 和扫雷模块，宿主 PID 41076 正常退出 0。 [最终 JUnit](../../build/xuanshu-mine-tool-alias-20261124-8ca6d2/acceptance/acceptance.junit.xml) · [实启](../../build/xuanshu-mine-tool-alias-20261124-8ca6d2/profile-launch.txt) · [交付身份](../../build/xuanshu-mine-tool-alias-20261124-8ca6d2/delivery-identity.json)

本轮使用隔离响应验证修正，真实任务留存原终态。原 `73b9c1` 和官方 `f6b2e1` 保持各自证据。 [本地入口](../../build/xuanshu-mine-tool-alias-20261124-8ca6d2/dist/start.cmd)

## 2026-11-24 日常聊天侧栏精简

前置 `8ca6d2` 在聊天滚动区域内展开完整模型设置，发送条长期显示模型名和说明，已结束任务仍占用主发送范围入口。修前生产页面配合隔离模型复现这些状态。 [修前 JUnit](../../build/xuanshu-chat-focus-20261124-31d7e4/red/red.junit.xml)

日常助手在左侧栏单独显示模型设置。模型和密钥为第一层，配置代次、位置及脱敏诊断按需展开。返回聊天保留草稿。输入区使用简短的「范围」入口，计价、保存待确认等需要处理的情况才显示相关说明。已结束任务的发送范围收进「详情」。切入设置会收起完整预览，确认须回到聊天重新打开。同一任务的完整内容、可信确认与停止规则沿原宿主实现。

最终生产界面相关 Light 20/20，涵盖设置往返、未确认零传输、结束后隐藏发送范围、历史范围只读、扫雷固定棋局、计价锁停及真实页面撤销入口。中间一次测试在预览自动滚动后用聊天状态行代替输入区的保存提示，状态行已滑出视口。最终保留待确认时可见的保存提示。该次通用宿主用例直接发撤销动作后没有取得绑定，最终走「详情」中的页面入口并验证回执、业务版本和关后记录。开发专用页面做了同 DLL 的前后对照，两版均因 262 个选择器超过 Light 的 256 条上限而返回 `-9`。日常设置的诊断入口由上述生产界面用例直接验证。 [中间运行](../../build/xuanshu-chat-focus-20261124-31d7e4/first-review-status/acceptance.junit.xml) · [开发页对照](../../build/xuanshu-chat-focus-20261124-31d7e4/development-load-comparison.log) · [最终 JUnit](../../build/xuanshu-chat-focus-20261124-31d7e4/acceptance/acceptance.junit.xml)

本地候选 `31d7e4` 重新嵌入正式宿主页，运行时 DLL、扫雷包及其他启动文件沿用 `8ca6d2` 匹配字节。真实启动脚本在一次性 AppContainer 从仓库外工作目录实启，核对隔离 Known Folder，宿主 PID 19796 加载扫雷模块并正常退出 0。自动验证使用隔离模型，未发送真实请求。 [实启](../../build/xuanshu-chat-focus-20261124-31d7e4/profile-launch.txt) · [交付身份](../../build/xuanshu-chat-focus-20261124-31d7e4/delivery-identity.json)

## 2026-11-24 框架通用聊天

前置入口启动时固定打开扫雷样例，宿主的通用设置仍有诊断入口。修前用例对实际设置区域给出两项失败。候选 `a73e50` 使用同一宿主页对所有日常应用提供聊天界面。共用的摘要选择使用通用文案，具体应用能力来自原发布预设。 [修前 JUnit](../../build/xuanshu-framework-assistant-20261124-a73e50/red/red.junit.xml)

日常「设置」只显示模型、密钥和必要状态。「维护」是独立视图，打开后可以按需查看脱敏诊断。完整确认预览显示时消息区退出布局，关闭后恢复对话。进入设置或维护会收起旧预览，返回的草稿保持。逐任务确认、实际发送范围与业务能力使用原接口。

最终定向 Light 20/20。空工作区验证设置和维护往返、草稿保留、请求前零传输和结束后历史查看。联合用例由同一宿主打开实际浏览器和状态组件示例，切换后均检查聊天与诊断的显示条件。扫雷两局、旧范围拒绝和不匹配目标也通过。自动化使用隔离模型。历史开发专用页面的样式条数限制见前置候选的对照。 [最终 JUnit](../../build/xuanshu-framework-assistant-20261124-a73e50/acceptance/acceptance.junit.xml)

新入口无应用参数。在独立 AppContainer 中从仓库外启动，核对已隔离的 Known Folder 和配套 DLL，宿主 PID 19480 正常退出 0。生产运行库、计量器、发布配置和内置浏览器继续使用前置匹配字节。 [框架入口](../../build/xuanshu-framework-assistant-20261124-a73e50/dist/start.cmd) · [实启记录](../../build/xuanshu-framework-assistant-20261124-a73e50/profile-launch.txt) · [交付身份](../../build/xuanshu-framework-assistant-20261124-a73e50/delivery-identity.json)
