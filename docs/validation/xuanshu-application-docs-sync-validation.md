# APP开发参考文档同步记录

2026-11-12。本轮按文档执行授权核对实际源码、公共头、CMake、匹配清单和既有证据，并同步开发者参考。静态核对只处理文档及文件身份。命令、示例和片段的编译/运行结果沿其原验证记录查询。

## 基线与文档增量

匹配冻结为`build/xuanshu-general-layout-20261112-6b37c1`，delivery-identity SHA256为`2C447E95741824E2C2A94E3EF91EC1782CF75C5870E262D7831C44989C7029CA`。限定复审及冻结记录已在前轮完成，本轮承接该事实。参考工程D:\XuanShu可读，仅作只读可用性核对。

独立文档记录位于`build/xuanshu-app-docs-20261112-3e8a61`。docs-before及before保留入口文档字节，current-claims逐项关联主张、公共头/源码、修改和历史范围。product-identity-before/final及documents-final区分产品快照与本轮工作区文档状态。新文档状态单独记账，原产品交付的文档副本和清单继续保持原身份。

## 开发者当前阅读路径

| 顺序 | 文档及用途 |
|---|---|
| 项目入口 | [README](../../README.md)说明当前API10、Service、P4/P5及通用助手，并链接匹配交付 |
| 开发基础 | [开发标准](../application-development-standard.md)说明APP职责、清单、线程、所有权和生命周期 |
| 当前接口与能力 | [统一能力/API索引](../application-capability-contract.md#当前公共接口索引)及[通用助手接入](../xuanshu-general-assistant-integration.md)给出注册/使用/注销、事务、专业角色和宿主责任 |
| 升级 | [迁移](../migration-v0.9-to-v0.10.md)、[可分享提示词](../application-upgrade-prompt.md)、[版本政策](../version-policy.md)区分API9迁移、同API10独立扩展与具体文件身份 |
| 最小示例 | [公共目录](../../examples/general_assistant/README.md)、[计数器](../../examples/web_counter/README.md)对应实际C源码、CMake目标及参数 |
| 业务示例 | [维护组件](../../examples/stateful_components/README.md)、[浏览器](../../apps/browser/README.md)列真实读取、结果字段、P4及撤销。另同步[通用组件](../../examples/generic_components/README.md)和[功能示例](../../examples/framework_features/README.md) |
| 构建与交付 | [构建文档](../build-and-validation.md)、[匹配交付清单](../xuanshu-general-assistant-delivery-manifest.md)、[冻结记录](../xuanshu-general-layout-freeze.md)关联头、导入库、DLL、宿主、包及依赖 |

## 纠正的当前主张

README的“无AI Service”“AI继续只读”和当前API9/0.9入口已按源码更新。开发标准主文现在分别说明v1 read、C2人工UI、P4事务、P5及通用助手。原API10首批read层和其头文件阶段注释有明确适用范围。

能力合同先给当前支持矩阵和公共头索引，再保留早期Proposed设计及2026-10-22至27逐阶段记录。迁移指南说明同API10后续扩展需要成套头、库及运行时。构建、包清单和示例目标与当前API10一致，API9及P2首批验证保留历史身份和原锚点。

维护组件、通用组件和framework_features已按真实模型、统一目录、UI绑定和事务说明替换早期“尚未迁移”表述。浏览器README以仓库内当前构建入口取代原外部工作目录入口，保留M0—M2合同/验证链接。升级提示词提供当前完整可分享块，使用仓库地址及相对路径，旧API9块保留历史日期范围。

支持矩阵明确browser.snapshot的state是完整顶层对象，browser.body为最多8192字节的DOM正文。维护组件的实际读/单元格编辑/P4回执与撤销均有真实源码对应。浏览器九项副作用、完整EDA、PNG入模及其他未接入业务按各自支持范围保留。

## 公共接口与匹配材料

静态核对覆盖资源/Schema注册、prepare/dispatch/complete/result/forget、C2绑定、P4事务、专业Profile、Service及通用助手接口。结构版本、复制/借用期、release、UI线程、取消、PENDING、UNKNOWN及关闭顺序按实际公共头说明。文档片段优先链接现有源码。

当前公共头由447份source-final范围逐项匹配。生产导入库为`light-final/ui_framework_runtime.lib`，SHA256 `607B39737F886246AF2F3CFA09813C93C6634241CA6A9FCC1EA86316DA1B2F40`，同快照DLL/宿主/包依binary-files及launch-files核对。生产371项导出按PE字节目录读取核对，私有可信事件及探针仍为隔离验证用途。

SDK0.10开发版/API10/标准10、ABI1/包1保持。相同API数值不能选择早期缺扩展的头/库与当前运行时混用，外部项目须取得维护方提供的完整匹配材料。当前源码含未提交工作区增量，GitHub main和HEAD只描述其中一部分身份。

## 静态检查与保全

static-initial保留首轮检查。早期符号扫描把四个当前CTest名称及一个历史测试名称误归为公共函数，后续分别以CMake和原日期段落核对。实例标题set/get缩写已改完整函数名。static-final核对23份文档、663处链接/锚点、公共声明及导出，6个构建目标和8个选项均可在CMake找到。state-scan按当前/历史及v1范围分类原状态表述，代码块以静态方式检查。

root三规划仅追加checkpoint。当前交接和必要阶段/准备索引有简明入口。原文档字节存于本轮before目录，原日期明确的产品验证、冻结副本、失败材料及DB/WAL均保持其历史身份。本轮产物为必要文档、差异和静态结果，清理情况见cleanup.json。

BR1-KI-001保持未解决、用户接受非阻断。已取消临时页面控件测试保持取消。历史期限、复制错误、原始日志限制和非空WAL按原记录保留。GENERAL-REAL继续Proposed，本次文档授权未转为真实执行批准。

APP开发参考文档已同步到当前匹配交付。本结论覆盖当前开发者入口、API/能力说明、迁移/构建和代表示例的文档一致性。产品测试、源码独立复审及真实服务结果继续引用各自已有记录。
