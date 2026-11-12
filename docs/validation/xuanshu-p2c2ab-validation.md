# XuanShu P2 C2-A＋C2-B 独立实施与验证

状态：**实现和本地自动验证完成，停在联合代码评审；物理输入未验，不宣称完整验收通过**。日期：2026-10-23。不署名。整体三份架构设计继续 Proposed；C2A-1 冻结正文不变，C2B-1 在其产品实现前落盘。本记录不回写历史测试结果。

## 1. 基线、授权和交付身份

实际分支 `codex/p1-process-boundaries`，HEAD `e0eca03db25930be55bdb45477027d1793a61904`。本轮开始已存在 A+B、C1 及此前整改和文档变更，不能把 HEAD 至工作区差异当成本批。进入时 17358 个 Git 可见文件的逐文件长度/SHA-256 见 `build/xuanshu-p2c2ab-20261023-91f6c8/baseline.json`，分支/HEAD 见 identity.json、git-entry.txt；本批实际修改见 delivery-files.json，保全见 preservation.json。

证据根目录为 `build/xuanshu-p2c2ab-20261023-91f6c8`（本地材料，不保证 clone 自带）。本轮未提交、未推送、未触发远程流程。参考工程本轮访问被拒绝；历史 main/67eacf5 身份不作为新核对结论，没有修改参考工程。

授权仅涵盖 C2-A 和标准表单/对话框 C2-B。没有应用业务模型、五示例迁移、C2-C、专业画布/WebView2、跨实例共享文档写入、AI Service/Provider/Coordinator、审批/事务/撤销/存储/恢复。SDK0.10-dev/API10/标准10、ABI1/包1、原公开布局、固定依赖、JCS/SHA-256和既有限额保持。

## 2. 接口与实现

新增 [capability_v2.h](../../include/ui_framework/capability_v2.h)、[capability_ui.h](../../include/ui_framework/capability_ui.h) 共11函数，严格采用 [C2A-1](../xuanshu-p2c2a-interface-and-acceptance.md) 的结构和错误合同。新增 [capability_forms.h](../../include/ui_framework/capability_forms.h) 四函数，遵循先落盘的 [C2B-1](../xuanshu-p2c2b-form-contract.md)。公共 C/C++ 断言保留全部旧布局验证，增加新结构布局及可链接调用。Light生产DLL导出核对为这15个增量，无私有human/组件测试钩子导出，原始表见 dll-exports.txt。

同一 capability.c registry、资源、ID、代次、目录2MiB、调用1024/8MiB及执行回调管理 v1/read 和 v2/view/draft/write。capability_v2.inc 是该模块私有实现的续篇，不是第二目录。v2元数据可在共同发现视图查询；Assistant及v1发现仅read，旧invoke及v1执行v2不授权。生成普通命令没有第二业务handler。绑定保存目标副本、open epoch和能力注册incarnation，防止同ID重新注册命中旧绑定。

人工接纳只经Windows私有来源识别及Light标准语义链。按下/释放匹配同节点UID，公开模拟输入使资格失效，DOM代次/disabled/所属modal在回调后再检查。Windows IMO_HARDWARE不等于物理设备证明，不能排除所有UIAccess注入或恶意DLL。私有夹具调用不是来源证明；没有生产token/mint/prepare_nonread/dispatch_nonread入口。

关联和资源门先于ADMITTED、execute建立。后台工作只能准备；UI提交和稳定ID完成，取消不释放门。未知保留原call/资源epoch及阻止状态，forget返回PENDING；显式人工只读核对复用预留工作区，满额不另占call。核对不重写原UNKNOWN。关闭的逻辑未知引用与活跃回调租约分开；可撤回的workspace关闭等待不提前拆未知门，最终卸载才封闭登记引用。

C2-B只绑定框架拥有的Light FORM/DIALOG，完整类型化字段、base revision、提交代次/指纹、私有草稿、显式保留/重载、modal所属及真实模型回读。只读read_model和字段投影不会提交业务。成功后实际值和after revision匹配，且字段generation未变，才接纳相应草稿；新草稿保留并冲突。打开dialog刷新业务基线且保留脏草稿，回调中新开其他modal不会被覆盖。已绑定表单不再调用残留legacy组件业务命令；未绑定组件原行为保留。

UI线程、借用缓冲、回调和解绑租约沿用C1；模型回读也持有C2租约。finish_unbind等待正在借用的回调；已接纳call可在解绑后独立完成并查询。未知或取消不冒充回滚。固定scratch、表单存储和临时回读均在原预算中预留，不增加资源限额。

## 3. 本轮新验证

全新 Native、Light、ASAN 构建目录均在本轮证据根下。Windows x64，VS18/MSVC19.50.35727、C11，Release；ASAN为 `/fsanitize=address /Z7`。固定依赖离线检查通过，未下载/升级。详细配置与命令见 run.ps1 和各 configure/build 日志。已有测试断言和超时未放宽；新C2测试明确15秒，原workspace_offscreen仍20秒。

| 最终运行 | 结果 | 原始日志/JUnit |
| --- | --- | --- |
| Native联合 | 17/17，5.28秒 | joint-final-native-test.log；native/joint-final.xml |
| Light联合 | 23/23，12.99秒 | joint-final-light-test.log；light/joint-final.xml |
| Native ASAN联合 | 17/17，8.28秒 | joint-final-asan-test.log；asan/joint-final.xml |

各配置C2-A专用5876检查/0、C2-B headless212/0、新C2 DLL53/0。Light真实模板的C2-B分支264/0。原C1入口20923/0、closeout5349/0、JSON79/0、core3747/0、JCS差分268向量及Assistant、公共C/C++头、组件和语言回归通过。原capability_workspace.c的普通/ENTRY两个编译分支均分别运行；其worker时序会使检查次数不同，不能用数量代替结论。

新增DLL夹具由真实加载DLL拥有worker和独立外部回调线程，UI零等待poll检查：关闭/注销在途PENDING；worker退但外部回调未退仍PENDING；二者退出后UI提交并完成。同步complete的外层execute栈上，forget/finish_unregister/quiesce/host_destroy仍被租约阻止。它使用函数表接入同一静态框架，并非新的C2 workspace加载器；真实workspace加载/复制队列/安全poll由原两个DLL配置回归补充，不能夸大为C2全链路物理应用验收。

## 4. 验收矩阵及证据限制

“自动通过”只指专用夹具及列出的回归；涉及实际人工来源、真实业务应用或长时间运行的部分不在本轮自动结果内。

| 冻结项 | 本轮证据和状态 |
| --- | --- |
| A01 | capability_v2 basic/incarnation与原entry/core：同一目录、重复ID/退休、生成视图、代次及ABA；自动通过 |
| A02 | basic与Light表单：旧invoke/Assistant/v1/公开模拟输入不授权；Windows来源分类/注入拒绝代码核对，普通注入及系统/未知的OS动态矩阵未验 |
| A03 | basic/rejection及notification：一次消费、重入背压、重复完成；DOM代次/匹配链源码核对；真实原生嵌套输入组合未验 |
| A04 | incarnation/rejection和原C1固定目标：epoch/注册重开不命中；v2真实workspace标签切换的独立正例未验，不以固定指针夹具冒充标签操作 |
| A05 | rejection、Light form：check/get_revision/ADMITTED modal变化、所属阻挡；disabled/busy/关闭门复用core/entry回归及源码核对 |
| A06 | basic/isolated_effect：write实际值与revision回读，独立view/draft资源变化不改业务值/revision；NONE不增版本；不代表应用模型迁移 |
| A07 | basic、notification、DLL同步：先关联/同步完成、重复跨版本不执行、延后通知及观察回调迭代不丢失；自动通过 |
| A08/A09 | async_unknown与DLL：异步、取消、同资源busy拒绝、排空再调用；没有UI阻塞等待worker；自动通过 |
| A10/A11 | async_unknown/budget：非法输出UNKNOWN、取消/forget/重复不能绕过，原call新人工核对与证据hash；DLL确认worker/外部回调退出；恶意谎报退出不在合作边界内 |
| A12 | budget两个分支保持1024/8MiB耗尽，未释放满额call/output仍核对；原closeout固定失败槽连续借用、多Assistant共享/重入；自动通过 |
| A13 | DLL、incarnation、notification及原workspace两分支：栈上/在途门、队列背压、重开迟到、UNKNOWN排空可关闭；不承诺任意应用线程自动跟踪 |
| A14 | 本节Native/Light/ASAN、公共头及原两DLL分支；自动通过 |
| A15 | 真实鼠标/键盘正例未验，不能由私有夹具或模拟输入替代 |
| B01/B02 | typed_shared/invalid_readback：同目录绑定、类型化text/number/bool/enum/color和参数校验；动态缺失/每个非法枚举组合不是穷举，Schema沿用原JSON回归 |
| B03/B04 | typed_shared/conflict_snapshot：共享业务name、独立草稿、真实回读，异步期间更新generation不被接纳；自动通过 |
| B05/B06 | conflict_snapshot/completion_edge：外部已投影/未投影revision变化、保留草稿/冲突，显式保留/重载只读且新事件再提交；自动通过 |
| B07 | Light分支：所属modal提交、父表单被阻挡、check新开modal、打开dialog回读新开其他modal不覆盖；程序消息/模拟输入拒绝；真实物理操作未验 |
| B08 | invalid_readback/completion_edge：读回失败、值错、版本错、UNKNOWN不假清草稿，写门保持；自动通过 |
| B09 | unbind_pending及C2-A/DLL/C1回归：在途解绑仍可完成/查结果，新绑定重读业务；取消/队列/预算复用共同合同，未做长期压力或故障注入 |
| B10 | 两组件编译分支、原布局/宽度/体验/modal/原C1/A+B/headers与ASAN回归；物理输入未验 |

尚未做实际菜单的真实鼠标/键盘正例、OS来源分类动态穷举、v2真实workspace标签切换、实际五应用业务集成、长时间运行和allocator故障注入。前述未验不阻止独立代码交付，但不能据此宣称全部A01—A15和产品验收完成。当前交付位置是两批联合代码评审。

## 5. 首次失败、修复与源码匹配

每次build/test前记录 `*-sources.json`；失败运行保留 `failures/<label>-<config>-<stage>/` 中当时源码与已存在匹配exe/DLL/lib/PDB/包，最终构建不覆盖这些路径。binary-files.json及source-snapshots.json核对实际保存身份，源码不等于HEAD身份。

| 本轮首次失败 | 核实及处理 |
| --- | --- |
| a-v2-first-native-build | 新夹具strcpy触发/WX C4996；改用有界strcpy_s，没有放宽编译警告。编译失败不存在该新目标匹配可运行二进制，不冒充red运行 |
| a-v2-first-native-test | 新C2观察通知在execute栈内立即回调，原C1嵌套read被拒；测试继续strcmp空值引发SegFault。修复私有延后通知，不修改C1 gate；测试增加NULL保护而保留必须read成功的断言。保留当时源码和二进制 |
| a-notice-native-test | 219检查/1失败：busy通知code错误；修为busy，原断言不变 |
| b-fixture-native-build | 新表单测试漏yyjson内部include路径；仅补该目标include目录，未改固定依赖 |
| ab-scope-light-test | 22通过/1失败：新Light表单165检查/1，对话框打开未更新基线导致预期有效提交被拒；补真实回读及脏草稿保留/冲突，不放宽revision门 |
| b-legacy-red-light-test | 263检查/1失败：已绑定FORM的rename消息仍可进旧handler；封闭该组件legacy业务分支，保留未绑定组件原行为；最终Light264/0 |

自审还修复：同ID重注册绑定ABA、观察回调完成其他call时通知丢失、可撤销关闭阶段过早拆UNKNOWN引用、模型回读租约、表单投影/草稿重置原子性和modal回读后重检。均保留冻结公共合同。

PowerShell5.1缺Path.GetRelativePath、一次Python编辑脚本语法错误、一次命令行多target传参错误也保留最小工具日志；它们不作为产品崩溃。历史旧red48失败缺失匹配二进制的限制不变，本轮新失败材料不能倒补旧运行。

## 6. 历史问题独立分账

原browser20失败：`build/xuanshu-p2ab-20261022-6e7d9c/scoped-light.log`；workspace首次20.05秒超时（门槛20秒）：同目录final-scoped-light.log；Native输入首次失败：`build/xuanshu-p2ab-closeout-20261022-4f9c2a/final-native-native-test.log`。均仍未定位，不默认归因环境；本轮没有修复或完整重跑这些问题。新的workspace限定通过不解释首次失败。

本轮涉及公共UI路径，历史输入/浏览器仍需独立定位回归关联；没有证据将三项全部归因本轮，也没有证据排除关联。现阶段不以它们冒充新C2失败，不用本轮通过覆盖原始失败。

## 7. 保全、清理与停止位置

delivery-files.json只列本轮相对baseline的文件，preservation.json核对原文件、固定依赖/冻结SDK/历史材料和追加文档前缀；原A+B/C1证据只读。binary-files.json保留最终及首次失败匹配产物。清理仅本轮证据根内无用对象文件和一次性编辑脚本；实际数量以cleanup.json为准，必要失败源码、原始日志/JUnit、配置及匹配二进制不删。

不修改版本/包/旧公开结构，不引入第二能力执行器或人工来源自报。后续首先联合代码评审，并补真实物理来源及列明未验；C2-C、应用模型/五示例和P3—P6必须另授权。此处没有授予后续自动实施权限。

### 7.1 最终文件核对补充

Git可见baseline17358个：17340未变、18项授权修改、12项新增，无缺失/范围外修改；另三份根规划被Git忽略，delivery单独列出它们，合计33项交付文件。三份规划只追加checkpoint，但进入时的Git可见baseline未包含其独立哈希，不能声称这三份有完整密码学前后核对；当前完整副本和身份已单列。其余五份原设计/交接文档的历史内容以及C2A-1冻结正文核对保留。

清理422个本轮无用对象/一次性编辑文件，64167544字节；没有清理历史材料、原始日志/JUnit、失败源码、匹配二进制或固定依赖。交付静态脚本首次因Windows默认GBK读UTF-8文档失败，明确指定UTF-8后复核通过；该工具失败不属于产品运行失败。

保全哈希的范围严格以baseline文件列表为准；Git忽略的依赖缓存/SDK若不在该列表，不能宣称已有独立逐文件前后哈希证明。本轮没有对其执行写入操作，固定yyjson源身份仍由既有离线构建检查验证。
