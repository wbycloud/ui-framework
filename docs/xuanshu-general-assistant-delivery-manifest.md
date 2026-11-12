# 当前冻结交付：通用助手Light整改匹配快照

2026-11-12。本轮限定独立复审通过，必须修正项为空；按用户授权将匹配快照冻结记录落盘。快照`build/xuanshu-general-layout-20261112-6b37c1`，delivery-identity SHA256 `2C447E95741824E2C2A94E3EF91EC1782CF75C5870E262D7831C44989C7029CA`。原开发identity及源码/程序/日志清单保持原字节，历史待复审状态不回写。详[独立复审](validation/xuanshu-general-layout-independent-review.md)、[冻结记录](xuanshu-general-layout-freeze.md)。

独立核对447份测试范围源码、154项程序及3313项证据，逐次测试前匹配关系保持。Light100/100；ASAN100项分次覆盖（业务19.7347秒、另次98项、glyph最终1/1）；Native47/47按依赖复用，不是本轮重跑。生产371项导出与前置一致。首败、环覆盖687条、原DB/WAL及历史限制保留。BR1-KI-001仍未解决且用户接受非阻断；取消的临时页面控件测试不恢复。

真实验证准备为[GENERAL-REAL-20261112-a74e20](../build/xuanshu-general-layout-review-20261112-a74e20/authorization-package.md)，仍Proposed。公开官方模型/协议/价格及计量源码当日核对一致；根最多8次、10分钟、USD0.10逐轮实际预留，预算不足即停止，不保证8轮全部完成。包内浏览器读取加自有维护单元格74及本地撤销42，不能称为纯只读。初始请求、有限计划及撤销各自适用本人可信确认；旧真实批准不复用。本轮无构建/测试/生产启动/计量执行/数据库打开/凭据访问/Provider请求。

新准备命令只静态审阅，未动态执行；会话尚未创建，策略未加载。下一步仅等待用户对具体包另行明确批准，不能把本轮冻结或准备授权解释为发送批准。详[准备事实与费用边界](validation/xuanshu-general-layout-real-preparation.md)。

下文当前快照生产文件表的路径与哈希继续适用；更早候选表仅为历史。本轮没有复制、搬移或重新打包程序。

## 此前时点原文保留

# 当前候选：通用助手Light字形测量整改

2026-11-12。已按追加授权完成一次分段基线及一次单条件对照，并据因果结果完成最小产品修复。基线原30秒Timeout；重复按钮字形GDI测量的单条件复用使完整对照20.52秒通过。工作区只改`src/backends/light_web.c`及`tests/light_glyph_padding.c`，不采用前轮无效DC复用，不跳过布局、投影或确认。

最终无本轮探针：Light100/100；ASAN原完整业务19.7347秒通过、另次98项通过、新增字形夹具修正后1/1通过，共100项逐项覆盖，不能写成首次单次100/100。Light完整业务7.64294秒。两次新增夹具失败与原日志/JUnit/匹配程序保留。Native47/47仅按未涉及Light源码及实际编译依赖复用，未重跑。

Light和ASAN关后各3任务、6消息、74事件、1图、2编辑回执；实际74/doc-2、撤销42/doc-3和记录恢复经原完整用例断言，进程正常退出。关后immutable观察无WAL/SHM且前后哈希相同。图/编辑事实不冒充通用tools表记录；六轮属于隔离模型替身，不是真实HTTP计数。371项生产导出、固定依赖、公开合同及预算保持。

匹配候选`build/xuanshu-general-layout-20261112-6b37c1`，identity与清单独立形成，等待限定独立复审；不自行冻结或声明真实服务通过。对照环最早687条覆盖、历史超时和失败均保留，不宣称解决所有未知期限根因。BR1-KI-001仍未解决且用户接受非阻断；临时页面控件测试保持取消。真实质量包Proposed、未核价锁停，无真实凭据、Provider或人工验收。

| 相对新候选根路径 | 用途 | SHA256 |
|---|---|---|
| `light-final/framework_host.exe` | 正式宿主 | `E2513D98F9C56B7419B7241A5374A2FA05A1930A2FE9D4E584E4AE6AEED762FB` |
| `light-final/ui_framework.dll` | 生产Light运行时 | `15DECCEFFF6A752ABBBB85D647FCD68A3103D1726BEDDAAD0FD24B6088A53D57` |
| `light-final/br1-production.uapp` | 实际浏览器及包内合成样本 | `A19982DFB36D3064982ED4CD80C6D9E2320646E8276855693576D12C84BE4182` |
| `light-final/stateful_components.uapp` | 维护组件与P4事务 | `EEDC6A0049A6F416E5BFD3219EF7E8DDBD64E2C8DABDE05E6A2DF6F4F0A7725B` |
| `light-final/xuanshu_br2_meter.exe` | 固定本地计量组件 | `7E2C2FF2508A83F05E1DB5B891635DABFA843812B4078BC0D7D4FEC3CB6C0EBB` |
| `supervise.ps1` | 原监督器 | `C1AD5354FC615609BF8602174F4A6F8303D2E9231E647BBD057C22E964F14373` |
| `start-general.ps1` | 仅更新匹配路径的启动入口 | `0E410286B987ED9B26A2F43521FDF74BAD8A3F426CBF77AE54FCE2EFE4845BB2` |
| `../../.deps/mesa-24.3.4/x64/osmesa.dll` | 固定OSMesa依赖 | `C633B820BBA8EC0DCB05466505E3C246E4C4028EAEC7CAF1054E707C8EB3CA1F` |

新identity不继承前置验收；source-final、build-source-final、binary-files、final-test-match、diagnostic-program-match分别关联最终生产、测试及诊断变体。入口557项源码范围不是全部Git内容哈希；最终447项测试范围源码与每次测试前清单核对。ASAN前两次仅字形夹具源码及对应exe随后变化，其他98项及完整业务程序逐字匹配，字形最终补项使用最终源码。

诊断`asan-light`、隔离测试DLL及可信事件不属于生产入口。基线归档程序是同阶段上下文，不全称为失败程序。生产`framework_host.exe`本轮未启动，实际正式页面由隔离测试宿主运行。仅清理本轮983项对象/资源中间件和派生只读观察副本，共451365506字节。保留所有本轮原DB及旁文件、必要失败源码/程序、原始日志/JUnit与trace，未清理历史材料。编译依赖VALID为清理前捕获，清理后对象缺失另行说明。

详[使用入口](xuanshu-general-assistant-entry.md)和[本轮验收记录](validation/xuanshu-general-layout-causal-validation.md)。

## 历史候选原文保留

# 通用应用助手首版候选交付清单

## 2026-11-12两项边界整改候选，完整验证待收口

候选根`build/xuanshu-general-boundary-20261112-9d6c53`。delivery-identity记录实际源码、程序及未完成验证状态。两项直接回归通过，Light95/95、ASAN94/95和Native47/47分别记录。ASAN业务30秒期限未解决，当前没有完整完成或冻结结论。

| 相对候选根路径 | 用途 | SHA256 |
|---|---|---|
| `light/framework_host.exe` | 正式宿主 | `C51446239E7D5D01C1D351E89FC6627DED62A36FF3AC53D56A83891F65A8EF5A` |
| `light/ui_framework.dll` | 生产运行时 | `1BC23CEA6906864589C95A5E75FE58ABF84509129BCDEE8A16507BC0B75F2E0A` |
| `light/br1-production.uapp` | 实际浏览器和包内合成样本 | `4839893059FE2CDF93B85DFD97F19FD2B1BA3988334429C725A85DD11EBD2DC2` |
| `light/stateful_components.uapp` | 维护组件与P4事务 | `6B1459CC9DD6E9EA924B7CBB4E464DAEA2A77F8ABC1C2A4F0CD4EF38CDB3D132` |
| `light/xuanshu_br2_meter.exe` | 固定本地计量 | `7E2C2FF2508A83F05E1DB5B891635DABFA843812B4078BC0D7D4FEC3CB6C0EBB` |

启动及监督脚本逐字复用，launch-files关联固定OSMesa和匹配程序。测试DLL、可信事件、保存屏障及诊断变体保持隔离验证用途。原交付、修前失败、超时WAL和符号材料保留。详[验收及限制](validation/xuanshu-general-boundary-validation.md)和[入口](xuanshu-general-assistant-entry.md)。后文旧快照保持各自身份。

## 2026-11-12三项限定整改候选

当前交付根为`build/xuanshu-general-closeout-20261112-4f8a21`。最终身份由delivery-identity.json及关联清单确定，等待再次限定独立复审。原首版候选、其开发证据及限定复审暂不通过分别保留。新代码和重新链接程序独立形成身份。

| 相对新候选根的路径 | 用途 | SHA256 |
|---|---|---|
| `light/framework_host.exe` | 正式宿主及所属任务交接投影 | `C6BCCD6EA8A0C4BFB3795E1DD404DD63EE44A8C01E0C9127AFFBD78D9F228691` |
| `light/ui_framework.dll` | 生产运行时，371项导出保持 | `43181C1AB42A99863A0AF95A583ED0907C970B9B2443587628375959C69C1BCB` |
| `light/br1-production.uapp` | 实际浏览器及包内合成交接单 | `0FE1BFC4ECA0929AF6335AFF241D1A0FCD85A33BAF23E942985E14140FAFFBED` |
| `light/stateful_components.uapp` | 实际维护组件及P4事务 | `1A520B5257B3D9ED736DB9CA29B1F8E633E4E2597487D153FC56C39857731844` |
| `light/xuanshu_br2_meter.exe` | 原纯本地计量组件 | `7E2C2FF2508A83F05E1DB5B891635DABFA843812B4078BC0D7D4FEC3CB6C0EBB` |

必要OSMesa依赖继续使用`../../.deps/mesa-24.3.4/x64/osmesa.dll`，SHA256为`C633B820BBA8EC0DCB05466505E3C246E4C4028EAEC7CAF1054E707C8EB3CA1F`。start-general.ps1和supervise.ps1与原候选逐字一致，新增launch-files仅关联本候选实际程序。历史启动监督8项及辅助2项按原身份和适用范围复用。本轮没有执行启动器或生产framework_host.exe，实际页面由隔离测试宿主验证。

最终closure-final分别为Light87/87、ASAN87/87及Native40/40。43份最终关后主样本的主库及现有旁文件哈希前后相同。39份带空WAL和SHM，4份无旁文件。当前候选669项交付文件、447项测试源码、179项程序及链接/调试产物由独立清单关联。保留修前13项直接失败、宿主投影2项断言失败及单独记账的夹具/观察前提问题。

新候选的source-before、增量diff、source-after、源码及二进制清单、修前程序/日志/JUnit、最终测试匹配、编译依赖、导出、关后自有库、旁文件保全和清理材料均位于新根。测试DLL、屏障和隔离可信事件属于验证材料。自动验证结果与限制见[限定整改验收](validation/xuanshu-general-closeout-validation.md)。

具体启动方式见[使用入口](xuanshu-general-assistant-entry.md)。真实质量验证包位于新候选authorization-package-proposed.md，保持Proposed、未核价锁停。后续需独立复审和新的具体授权。

## 原首版快照及历史证据保留

2026-11-11。候选根为`build/xuanshu-general-assistant-20261111-7a2f90`。开发自审及离线验证完成，等待限定独立复审。匹配身份由同目录delivery-identity.json索引，历史冻结快照保持原身份。

## 运行文件

| 路径，相对候选根 | 用途 | SHA256 |
|---|---|---|
| `light/framework_host.exe` | 正式宿主 | `531556F2209620F8CEF049BC5DE55E3395C2EE3FAAB6F3EA6412527923F78418` |
| `light/ui_framework.dll` | 生产运行时，371项公共导出 | `E7997F86C8A86990BF10DDF56752F7C8F203AAE77D3F248BDACC97C33171A2E8` |
| `light/br1-production.uapp` | 实际浏览器与包内合成交接单 | `DC60E1DA94FA76D0D24B7DF883CA7B527CCE004862D1D9179FBF2201071E5B8D` |
| `light/stateful_components.uapp` | 实际维护组件示例及P4事务 | `6A3B600B3EC77247DF9EB0832CC22AAE45520B61F83BB8C1735CF9D13A1731F5` |
| `light/xuanshu_br2_meter.exe` | 固定本地纯计量组件 | `7E2C2FF2508A83F05E1DB5B891635DABFA843812B4078BC0D7D4FEC3CB6C0EBB` |
| `supervise.ps1` | 进程监督及异常取证 | `C1AD5354FC615609BF8602174F4A6F8303D2E9231E647BBD057C22E964F14373` |
| `start-general.ps1` | 单一启动入口 | `D3914393E2469087D0D283606CF4132B6CCB5726406A80E8DF0BBEC56740A609` |
| `../../.deps/mesa-24.3.4/x64/osmesa.dll` | 维护组件固定GL依赖 | `C633B820BBA8EC0DCB05466505E3C246E4C4028EAEC7CAF1054E707C8EB3CA1F` |

Windows系统依赖见dependencies-host.txt及dependencies-runtime.txt。测试DLL、测试可信事件、替身计量器及故障探针属于隔离验证材料，部署入口只列生产文件。包内数据是本轮合成样本。

## 使用入口

从仓库根运行以下命令。

```powershell
powershell.exe -NoProfile -File .\build\xuanshu-general-assistant-20261111-7a2f90\start-general.ps1
```

日常入口使用稳定daily-default Profile。离线体验可加`-IsolatedProfile`，创建独立新Profile和账户身份。每次建立唯一会话库、空浏览器来源配置及新的维护样本目录。UI_AI_DB_PATH和UI_BROWSER_SETTINGS_PATH由监督器显式设置，UI_AI_PROFILE_PATH与UI_AI_PROFILE_ID独立于会话数据库。UI_AI_ASSISTANT_POLICY缺省未核价，发送锁停。原RPCP/RPCB/BR-2策略与通用模式互斥。

日志、PID、实际退出码和分阶段错误保存在sessions唯一目录，维护样本和隔离Profile位于application-sessions唯一目录。Profile文件仅含非秘密配置和引用，原生凭据按原合同保存。来源配置和业务数据仅用对应自有路径。

正常取消后等待终态及最终保存，再正常关闭并等待实际退出。异常时保留会话目录和全部旁文件。文档、启动器和状态投影均不签发人工授权。

## 验证与支持

最终Light71/71、ASAN71/71及Native31/31分别记录。普通生产两轮及一次身份阶段失败独立保留。最终生产PID22372、exit0，五块实际详情、草稿、跨标签固定目标、取消和关后核对齐备。实际业务模型桥接使用隔离DLL和模型/可信事件替身，六轮根请求、浏览器读取及维护编辑/撤销得到直接证据。

代表支持范围包括browser.snapshot/browser.body的现有读取链、维护组件文档/单元格读和test.edit可撤销操作、计数器最小公共目录示例。新联合业务用例直接覆盖browser.snapshot及维护单元格74/doc-2和撤销42/doc-3。维护工具原始文档结果与额外精确单元格核对分别记录。播放器、游戏、完整EDA及图片入模继续属于扩展范围。

[操作指南](xuanshu-general-assistant-entry.md)、[开发标准](xuanshu-general-assistant-integration.md)、[开发验收和限制](validation/xuanshu-general-assistant-validation.md)及[真实质量验证准备](xuanshu-general-assistant-real-proposal.md)共同构成交付入口。候选根authorization-package-proposed.md整理一次合成业务质量验证的具体条件，当前保持Proposed。
