# XuanShu P2 C2 集中收口：联合评审、整改与验证

日期：2026-10-23。不署名。状态：**A/B范围内缺陷已整改，C2-C六类绑定和workspace chrome适配已实施；C2整体暂不具备完整验收条件**。原生HMENU非read仍是本轮范围内未完成项，真实鼠标/键盘及Windows来源动态正例未验。不得把限定自动通过写成C2全部完成。

## 1. 基线及增量归属

实际分支`codex/p1-process-boundaries`，HEAD `e0eca03db25930be55bdb45477027d1793a61904`，没有提交、推送或远程运行。初始工作区已有原A+B、C1、C2-A/B及用户历史改动。本轮证据根为`build/xuanshu-p2c2-closeout-20261023-a93d5e`，17373项baseline包含Git可见文件及被Git忽略的三份根规划。以baseline/delivery-files/preservation区分本轮，不能用全部HEAD差异代替交付清单。

进入时核对原C2-A/B目录`build/xuanshu-p2c2ab-20261023-91f6c8`：delivery33项、source232项、binary203项均与当时磁盘一致，三配置原JUnit身份保留于prior-evidence-check.json。原C1限定验收仅作为前置边界，没有重做全面复审。原验收文档和历史目录不覆盖。

本轮访问`D:\XuanShu`被拒绝；没有修改该参考工程，也不能把历史main/67eacf5冒充新核对。SDK0.10-dev/API10/标准10、ABI1/包1、旧公开布局、v1只读语义/错误、固定yyjson/JCS/SHA-256和限额保持。未适配入口不借source、参数、页面消息或legacy恢复非read。

## 2. A/B联合评审及整改

以下为本轮源码核对结论，范围内直接最小修复；不是将原C1/A+B全部差异重新归为C2。

| 编号/程度 | 触发、影响及源码 | 修正与证据 |
| --- | --- | --- |
| R01 高，已动态证实 | ADMITTED/check/revision回调重入更换私有人工事件后，原gate仍可能执行。`src/capability_v2.inc:183`末次门原未固定来源序列和guard；失败后同一事件也可能被再次尝试 | 最后应用回调后复查捕获的event/guard、目录/目标/modal/关闭门；所有退出消费资格（现197、274、284行）。核对路径同样在最后回调后检查。ab-red-native-test为5924检查/2失败，原源码和匹配二进制保留；修复后原断言通过 |
| R02 中，源码已证实，新增回归通过 | 本地cancel/conflict处理调用read_model，回调可打开其他modal；旧代码仍重置/接纳草稿 | `src/component_forms.inc:202`在回读后复查来源及form可用条件；保留草稿和原冻结错误，不禁止回调重入 |
| R03 中，源码已证实，物理正例未验 | Light释放捕获早于语义提交，使真实私有来源失效；混合链、DOM重建、重复key及回调销毁owner还需完整失效/租约 | `src/backends/light_web.c`在语义动作后释放捕获，固定UID/DOM代次、拒公开/混合输入；`src/ui.c`及shell对回调使用中owner销毁延后，避免backend销毁循环。夹具验证来源失效及借用寿命，不能冒充真实输入 |

冻结C2A-1、C2B-1正文及既有结构/错误不改变。普通人工未迁移功能保持原行为；已绑定动作不再调用第二个legacy业务handler。C2B整体FORM/DIALOG绑定与C2C组件入口不能同时成为权威绑定，双向ALREADY_EXISTS，退休仍占用至finish。

## 3. C2-C冻结接口和实现

新增[C2C-1](../xuanshu-p2c2c-input-contract.md)在相应产品实现前落盘，其第5、6节在字段/真实宿主桥接实现前补充。新增[capability_inputs.h](../../include/ui_framework/capability_inputs.h)三函数：

- `ui_capability_input_bind`：独立version1描述/上下文，SHORTCUT、TOOLBAR、LIGHT_CONTROL、COMPONENT、SURFACE、NATIVE_BUTTON六类元数据引用。
- `ui_capability_workspace_input_bind` / `unbind`：框架Light chrome纯路由元数据，不是签发token或公开执行函数。

解绑使用原UI begin/finish，统一binding ID/目录代次/资源/调用与注销。`src/capability_inputs.inc`直接编入原capability.c，复用ui_action/gate/execute及稳定ID，不是第二目录或执行器。bind不定义权限/业务handler。collect只表达完整参数，Schema/JCS后固定能力、资源、epoch、revision和指纹；后台仅准备，提交/complete回UI线程。cancel、forget、重登记不清UNKNOWN，核对只能新的人工只读动作。

DOM ID复制上限255字节；Light当前值最多4096字节，复制到原8MiB预留工作区，借用至collect返回，不是业务已提交证据。原2MiB目录、1024调用、8MiB调用及原JSON限额不增加。input event文本指针不跨回调。owner/节点/实例重建不自动改绑；回调/通知期间不得提前释放、解绑或卸载。

Windows私有WndProc/subclass分类只接纳已知hardware来源/有效device，匹配动作链；程序消息及可识别注入/系统/未知不授权。Windows分类不等于物理设备证明，不能排除所有UIAccess注入，不隔离恶意DLL。private fixture仅编入静态测试库，生产shared DLL不导出human/fixture钩子。

## 4. 实际入口覆盖矩阵

“自动通过”只表示下面的专用夹具/已有回归通过；不是物理事件验收。所有已适配非read均走同一人工接纳、UI action和业务执行器；read继续C1/v1。

| 实际入口 | 效果、来源和收参 | 固定目标/路径 | 本轮证据与限制 |
| --- | --- | --- | --- |
| Light菜单叶项 | view/draft/单资源write；复用A私有节点链及collect | 复制ref/epoch/incarnation，ui_action→gate | C2-A5896/0；真实鼠标键盘未验 |
| 标准FORM/DIALOG | 同上；私有字段草稿、提交快照、代次/base、显式冲突、modal所属和实际回读 | 复用B同一binding/call；非提交不改业务 | B214/0、Light266/0；业务模型仅内存夹具 |
| host快捷键 | 私有Light或native surface KEY_DOWN，key/modifier；编辑器保留自身手势，自动重复不重复接纳 | 显式SHORTCUT binding；公开dispatch不能授权 | inputs Native228/0，Light373/0；物理键盘未验 |
| Light工具栏popup | 实际item ID/节点匹配，完整参数collect | TOOLBAR binding及最后enabled/busy/modal门 | 源码路径及Light宿主/组件回归；该popup物理正例未验 |
| 真实宿主chrome工具栏/快捷键 | 独立chrome host私有事件，元数据桥接；页面app-command不能授权 | 成功同步投影记录input_instance；匹配显示实例仍active，不能追随新标签重定向 | 真实workspace Light282/0含独立chrome固定目标；host frontend通过；夹具直接设置投影身份，不冒充完整真实人工投影验收 |
| Win32 toolbar | 私有子类，TB_HITTEST/GETBUTTON真实item，左键或Enter/Space | 同一TOOLBAR metadata | 实际shell/toolbar句柄及拒公开路径夹具；物理正例未验 |
| 标准Light按钮、输入框 | 私有精确节点/UID/DOM代次；当前值复制，显式语义提交 | LIGHT_CONTROL metadata；绑定前提不由JS自报 | 实际模板click/key/当前值/公开及混合链负例；物理正例未验 |
| TREE/TABLE/LIST | 明确component和行/单元格DOM ID，collect语义参数 | COMPONENT metadata，复用同一cap；与FORM权威互斥 | 三种真实Light标准组件分别测试；STATUS仅显示，不伪造提交 |
| Win32 BUTTON | 同UI线程、host父链内真实BUTTON，私有子类；BM_CLICK拒绝 | NATIVE_BUTTON，collect→原gate | 真实隐藏控件/程序消息负例和私有链夹具；恢复legacy未绑定行为；物理正例未验 |
| 原生canvas surface | 原生按下/释放、key及逻辑坐标快照；专业业务参数由collect描述 | SURFACE metadata及固定资源；不提供EDA几何执行器 | Light真实HWND surface，Native/ASAN headless注册链，原surface_input回归；OpenGL渲染/物理输入未验 |
| 原生HMENU | **非read未适配，范围内未完成**。公开WM_COMMAND不证明私有匹配来源 | 保持拒绝非read，无legacy降级 | 源码确认没有安全签发适配；缺实现及正例，阻止“C2入口全覆盖”结论 |
| 面板/标签/内部布局 | 保留原框架view动作，无业务提交的显示不当write | 原框架行为，不宣称已迁移应用业务 | 组件/宿主回归；真实标签切换由workspace API驱动，非物理点击 |
| WebView2、自定义非Light网页 | 本轮明确排除，不能以页面消息授权非read | 不降级 | 未实现/未验，不把其当本轮应交付项 |

专业画布仅通用事件来源和绑定；stateful/EDA内存业务模型、五维护示例能力迁移、跨实例共享文档均未实现，不属于本轮夹具交付。

## 5. 新验证及相称回归

全新Native/Light/Native-ASAN配置，VS18/MSVC19.50.35727，Release，ASAN `/fsanitize=address /Z7`。Light额外开启真实framework_host，WebView2 OFF，Mesa空路径；固定依赖离线复用。原断言/超时未放宽，workspace_offscreen仍20秒。命令见run.ps1；最终构建surface-fixed日志，最终测试accepted日志/JUnit。

| 配置 | 最终结果 | 原始日志/JUnit |
| --- | --- | --- |
| Native | 22/22，6.95秒 | accepted-native-test.log；native/accepted.xml |
| Light | 30/30，19.28秒 | accepted-light-test.log；light/accepted.xml |
| Native ASAN | 22/22，10.05秒 | accepted-asan-test.log；asan/accepted.xml |

C2-A5896/0、B214/0（Light266/0）、inputs228/0（Light373/0）、真实workspace Native257/0（Light282/0）、C2 DLL53/0。原C1 entry20923/0、closeout5349/0、JSON79/core3747、JCS268向量及Assistant protocol/lifecycle、公共C/C++头、组件/布局/语言/surface、workspace及Light frontend通过。原capability_workspace.c普通和ENTRY两个真实DLL编译分支分别运行；新C2 DLL自有worker/外部回调、同步完成外层栈上租约/复制队列的原测试亦在列，没有用固定指针取代真实workspace验证。

伪造/重复接纳、混合链、公开调用、epoch/revision、草稿冲突、非法参数、disabled/busy/modal及末次回调、同步/异步/重复/取消/UNKNOWN、满额队列、迟到及在途解绑注销关闭由A/B、inputs及真实DLL/旧入口回归组合覆盖。并非每个输入profile逐项重复全部生命周期矩阵，未将组合覆盖夸为全部物理路径验证。

实际程序消息读取Windows来源：ok=1、origin=0、device=0，拒绝；来源判定穷举和私有混合链是逻辑夹具，不能代替硬件/普通注入/UIAccess/系统来源动态矩阵。真实鼠标、键盘及这些OS动态正例仍未验。ASAN是Native配置，未验Light内存检测、长运行及完整release矩阵。

## 6. 首次失败、修复及匹配产物

各次build/test前记录sources.json；实际测试失败对应failures子目录保留当时源码、EXE/DLL/lib/PDB/uapp和原日志/JUnit，后续构建不覆盖。编译失败没有该新失败目标的可运行匹配二进制，不将目录中旧可执行产物冒充该次red。

| 首次失败标签 | 核实与修正 |
| --- | --- |
| ab-red-native-test | 5924/2，R01有界合同断言失败；先留red，再修来源门/失败消费。不是已复现崩溃 |
| c-input-fixed-native-test | 新夹具175/1，headless不能创建真实native HWND；明确分开headless注册链与Light真实HWND分支，保留必须创建断言 |
| c-workspace-first-native-test | 新workspace夹具243/34，漏必需unmount回调导致setup失败级联；补合法完整生命周期并加setup诊断，不放宽业务结果 |
| c-workspace-valid-native-test | 254/1，新断言误写迟到post为NOT_FOUND，源码已有关闭队列明确CANCELLED；修为单一精确错误，不用宽泛OR |
| c-joint-light-test / c-frontend-fixed-light-test | 先28/29通过、frontend4失败；来源桥先错误拦截无关框架按钮，修为先分类入口。剩余1失败是旧测试选择器popup-item-4实际为“上一个标签”；改为真实第5项主题，保留dark结果断言及60秒。菜单顺序本轮未改，frontend-selector-evidence.txt保存HEAD/初始磁盘规范换行后身份。最终frontend通过不解释旧browser20失败 |
| final-native-test / surface-red-asan-test | 原surface_input测试移除旧raw回调后发送WM_CHAR，本轮放开语义输入门引入空回调调用；Native22中1 SegFault，独立ASAN1/1失败且pc=0。`src/platform/win32_surface.c:174,284`补raw字符回调检查/清surrogate，不阻止新语义路径。保留匹配源码二进制；修复后最终三配置通过，原测试无改动 |

其他首次编译/工具错误原样保留简明日志：未等Light配置结束；新测试错误枚举/缺include/局部替换/头顺序、静态workspace漏链接和局部变量遮蔽；Light standalone OFF时请求不存在的frontend target；临时诊断字段误名和编辑换行误写；直接GetDpiForWindow在既有_WIN32_WINNT下不可见，改用已有ui_platform_get_dpi，不提高最低版本或压警告。普通错误均范围内修正，没有交回用户或改变冻结合同。

原C1旧48失败red缺失匹配二进制仍是历史证据限制，本次5924/2、surface ASAN及最终产物不倒补旧运行。

## 7. 历史问题独立分账

| 历史证据 | 当前判断/优先级 |
| --- | --- |
| browser20失败：build/xuanshu-p2ab-20261022-6e7d9c/scoped-light.log | 根因仍未定位。C2触及共享UI/宿主路径，需独立对比，不能排除关联；本次frontend选择器修正不是browser20修复。独立公共UI回归优先项 |
| workspace首次20.05秒，门槛20秒：同目录final-scoped-light.log | 原超时保留，后续限定通过不定位首次；本次无延长超时。后续时序/负载诊断，未证实与C2关联 |
| Native输入首次失败：build/xuanshu-p2ab-closeout-20261022-4f9c2a/final-native-native-test.log | 原断言根因未定位；本次surface空回调是另一项明确的新回归，已独立复现/修复。不能合并账目或以新通过覆盖首次 |

没有默认归因环境，不要求本轮借机修复历史未定位问题；新的测试失败与上述三项分别记录。

## 8. 保全、清理及验收结论

delivery-files.json列本轮新增/修改，preservation.json核对初始文件、根规划追加及冻结正文，source-after.json/最终源码快照、binary-files.json、failure-retention.json和原始日志/JUnit提供身份追溯。旧证据树、用户.gitattributes、冻结SDK、固定依赖和历史材料保留；未声称已逐个散列未纳入baseline的所有忽略缓存。cleanup.json仅列本轮根内已无用途对象/编辑脚本等；必须保留失败源码、诊断、匹配二进制/包/PDB。

**限定结论**：A/B已确认缺陷整改和C2-C已列六类绑定/真实workspace chrome自动闭环可提交联合代码评审。**C2暂不通过完整验收**：原生HMENU非read尚未适配；真实物理输入及Windows动态来源矩阵未验，不能用private fixture冒充。前者是实际未完成，后者是验收缺证，不混为应用迁移或P3缺失。

后续必须先处理/明确决定HMENU来源适配并完成真实输入验收，再宣称C2完整。非阻断补强可包括各profile更细OS动态组合和Light ASAN；不自动运行或扩大范围。本轮停止联合代码评审，不进入应用模型、五示例迁移或P3。没有AI Service/Provider/Coordinator、审批/事务/撤销、存储恢复或跨实例共享文档写入。

### 8.1 最终文件核对

17373项初始文件中17346项不变、27项授权修改，另新增8项；无缺失或范围外变化。三规划、三整体设计及A/B冻结文档原前缀均完整，原交接正文仍完整保留。242项最终编译源码清单/快照与三次最终运行记录一致，392项保留二进制有实际哈希清单；失败测试源码清单全部匹配其独立快照。公共原15个增量加本轮3个增量可链接/导出，生产DLL无私有human/fixture导出。

仅清理本轮根内800个无用途过程文件，共394487863字节；cleanup.json列实际路径和原因。编译失败目录中的旧重复二进制被清理，不声称它们是新失败目标；实际测试失败的匹配产物、源快照、原始日志/JUnit及最终产物保留。收尾统计脚本的字节聚合空值按实际逐文件length补正，不影响删除路径、产品或测试结论。HEAD/分支保持，暂存为空。
