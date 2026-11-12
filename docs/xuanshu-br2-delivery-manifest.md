> 2026-11-06 真实验证准备补充。冻结产品及本地材料身份保持；官方模型、协议和价格已完成本轮公开资料核对，可靠input计费上界仍不足。当前本地入口保持锁停，真实执行尚未开始，集中授权包待前置条件成立后提交。当前事实以[前置核实记录](validation/xuanshu-br2-real-preparation-validation.md)为准，下文未核价表述保留历史时点。

# BR-2冻结快照与本地交付清单

2026-11-06。BR-2旧策略费用恢复兼容匹配快照限定验收通过，冻结记录已落盘，本地交付材料齐备。当前入口使用未核价的Proposed策略，真实验证保持锁停。[独立复审与冻结记录](validation/xuanshu-br2-fee-restore-independent-review.md)

## 匹配身份

仓库与命令工作目录为`D:\应用软件框架\应用层序框架`。匹配交付根为`D:\应用软件框架\应用层序框架\build\xuanshu-br2-fee-20261106-54d9a8`。

delivery-identity.json的SHA256为`1EF3D1643AE1DF94ED6B6CC163F061F1376D82E3A5CC540823F1E197687105FB`。本轮编辑前433项源码、113项程序及链接/调试产物、5项launch-files、16项交付增量和30份identity关联清单均匹配。原交付中的程序和文档副本保持原字节，本轮文档记录封装后的冻结与交付状态。[原身份](../build/xuanshu-br2-fee-20261106-54d9a8/delivery-identity.json)

以下表内路径相对于上述绝对交付根。

| 文件 | 用途 | SHA256 |
|---|---|---|
| `light/framework_host.exe` | DH-1正式宿主 | `FA7DB4741BD3F7DBA95ED67142A4FA5F4F4CF9F238BB8AF888D9911ACB988C7F` |
| `light/ui_framework.dll` | 生产运行时及原Service/Provider/read链 | `40F74E512FC3D45130D41F4536DEA8D2C7276240759CA5CDB26D6A4FD71492E6` |
| `light/br1-production.uapp` | 实际浏览器应用与包内合成样本 | `C4BA8A621136EA0F1D40F791FA81588CB6C5C129C4C66784A137F489405E0AC9` |
| `start-readonly.ps1` | 当前本地监督启动入口 | `A9D70493374F208E94ECF15790EA1820317DBBD010A19D51980F4E33051E0DC4` |
| `supervise.ps1` | 身份检查、会话隔离、PID和退出监督 | `C68D7DEB827DDD94E77009B54AAC3F2D569D7FCF86E75F0779BC40E3F2AC809D` |
| `policy-proposed.json` | 未核实价格及计费上界的锁停模板 | `760F0909F853C743EA183C1F83ECCC27F8DF4475DCD33FD0506AA83ED8472286` |
| `launch-files.json` | 上述3项生产文件、策略模板及监督脚本的身份检查表 | `48FA6E287C3FD7661531C1C3905DC33510F6ACFF8E45B8A8603BC4774F544296` |

原source-final及binary-files继续作为完整源码和程序身份索引。本清单按现有路径交付，程序目录保持现有组织。[源码清单](../build/xuanshu-br2-fee-20261106-54d9a8/source-final.json) [程序清单](../build/xuanshu-br2-fee-20261106-54d9a8/binary-files.json)

## 浏览器业务及依赖

`br1-production.uapp`使用实际`apps/browser`，app_id为`org.ui-framework.browser`，API10、ABI1，多实例开启。包内含`browser_app.dll`、`app.ini`、`chrome.html`、`help.html`、`start.html`及`origins.txt`。内含模块SHA256为`E6F654D0E319CFD6E225C92FB48741A4E35DABE8376064B68ECAF8314EA41F77`，合成start.html为`153CC22F3327E5C3ECAFFA100E898EC0B7642448F54FE50D7FD019425A3A2A95`。两者已通过只读包索引核对，资源随包提供。

合成标题为“BR-1 合成交接单：设备巡检”，来源为`browser:start`。场景读取标题、来源与版本，history=[]，目标为接纳时固定的`browser.view`，能力仅`browser.snapshot`，字段为`state`和`revision`。state是现有完整顶层对象，包括标题、地址、阅读/布局状态及有界诊断。样本正文12/9/3数字保留在文档中，正文汇总覆盖沿原证据边界处理。[业务范围](xuanshu-business-read-contract.md)

只读PE导入核对显示正式宿主和包内模块依赖生产`ui_framework.dll`，另依赖Windows x64系统组件及现有Microsoft C/C++运行库。系统组件包括USER32、GDI32、COMCTL32、OPENGL32、IMM32、USP10、MSIMG32、ole32、bcrypt、WINHTTP及相应系统API集。网络库的导入仅说明链接关系，具体请求行为由任务保护及运行证据判断。

| 本机运行库 | 2026-11-06只读观察SHA256 |
|---|---|
| `C:\Windows\System32\vcruntime140.dll` | `184146852727A9DB4EEA06178716BEC3CDBB1015C911F6B0F915B184AD7775B2` |
| `C:\Windows\System32\ucrtbase.dll` | `DEDB7611E1ABC1FF3852DCAC3D4C93A048D729E25B061973B9969F22698F14CA` |

运行库表属于本机依赖观察层，冻结交付文件身份按原identity和程序清单记录。迁往其他机器前须核对其Windows x64及VC运行库可用条件；安装器或新打包方案继续作为独立Proposed工作。

## 当前唯一入口

以下命令仅作使用说明，执行需相应启动授权。本轮只完成文档和身份核对。

```powershell
Set-Location -LiteralPath 'D:\应用软件框架\应用层序框架'
& '.\build\xuanshu-br2-fee-20261106-54d9a8\start-readonly.ps1'
```

入口按launch-files核对身份，实际子进程工作目录为交付根下`light`。监督脚本创建`sessions\`下唯一32位GUID子目录，已有路径冲突时停止。默认启动打开生产浏览器样本，任务提交与外发继续受原界面和保护条件控制。

| 本轮派生位置 | 用途 |
|---|---|
| `sessions\GUID\conversation.sqlite` | `UI_AI_DB_PATH`，本次独立任务库 |
| `sessions\GUID\conversation.sqlite.model.json` | 数据库路径追加`.model.json`，本次模型配置位置 |
| `sessions\GUID\origins.txt` | `UI_BROWSER_SETTINGS_PATH`，空的本地网络来源配置 |
| `sessions\GUID\business-policy.json` | `UI_AI_BR2_POLICY`，复制未核实模板并绑定本次数据库路径 |

表中的GUID由脚本生成。三个环境变量仅作用于该次启动，监督结束后恢复先前值，LOCALAPPDATA保持原设置。用户原有库、旧人工库及凭据引用保持隔离，模型配置路径按数据库路径派生。

Proposed模板中价格、价格证据及可靠input计费上界尚未核实。当前入口拒绝verified模板，BR-2任务接纳及真实发送受锁停约束。后续真实验证需要独立的一次库绑定策略与监督配置，以及集中授权和逐任务可信确认。[待核实及待授权包](xuanshu-br2-real-proposal.md)

## 监督、诊断与安全停止

每个会话按实际进度保存以下材料。

| 材料 | 内容及适用边界 |
|---|---|
| `prestart.json` | 预启动阶段及隔离路径 |
| `process.json` | 实际启动后的PID和记录时间 |
| `stdout.txt`、`stderr.txt` | 该次子进程标准输出及错误输出 |
| `result.json` | 监督结果、实际退出码、分阶段异常及证据保存状态 |
| `conversation.sqlite`及旁文件 | 本次任务、事件、消息和工具事实，关后按实际旁文件状态判断完整性 |

未创建进程时PID和退出码保持空值；辅助记录失败仍继续监督已启动进程。PowerShell异常、阶段和可取得的系统错误码分别记录，证据目录不可写时按实际失败报告。监督持续等待真实退出，入口保持一次启动语义。

任务查看、草稿及设置使用正式DH-1界面。“保存任务并查看范围”表示本地提案，收到和持久化接纳分别展示。固定范围包括任务输入、所选历史、目标、read白名单、模型配置和完整请求。摘要与display_valid单字段只说明对应投影状态，确认资格仍依赖run/hash、代次、完整实际文本、布局及末次回调后检查。隐藏范围或详情、投影失败及条件变化立即撤销旧资格。[操作说明](xuanshu-readonly-user-guide.md)

1. 如需终止当前未完成任务，使用“请求取消”，等待实际终态和最终保存事实。
2. 使用宿主标题栏关闭按钮正常关闭，以监督记录的实际退出码及安全收尾事实确认结果。
3. 发生目标、模型名、确认资格、存储、费用、期限或显示异常时，停止对应提交、确认和后继发送，保留当前状态及诊断，按既有生命周期等待安全关闭。

取消请求与取消终态、最终保存、远端停止和费用状态分别判断。UNKNOWN继续保留。诊断优先使用“复制脱敏诊断”，只保存对应合成范围、任务关联、阶段及错误，排除密码框、Key、Authorization、凭据Blob和完整内存。存在未合并WAL时保留DB/WAL/SHM共同判断，关后取证继续限于该次自有会话。

## 验证材料与历史保全

生产运行使用上列3项生产文件及入口支持文件。`br1-isolated.uapp`、`ui_framework_edit_test.dll`、测试EXE、私有可信事件、合成凭据和传输替身属于隔离验证。PDB、LIB、EXP、原始日志/JUnit、source-before、delivery-final、packaging-first和失败阶段归档供诊断及溯源使用。实际浏览器业务与包内合成数据的覆盖范围按业务合同记录。

开发自审及离线验证见原验收记录，限定独立复审及冻结决定见新增记录。本轮按对应身份引用已有结果，运行分次保留。有效red-valid为13例中5失败、8对照通过；最终Light107/107、ASAN-Light107/107、Native36/36；较早fixed-direct13/13另列。27组合法恢复逐值保持，恢复尝试的新确认、占用和传输入口增量均为0。[开发证据](validation/xuanshu-br2-fee-restore-validation.md) [独立复审及冻结](validation/xuanshu-br2-fee-restore-independent-review.md)

99份主样本的无WAL/SHM及immutable前后哈希一致仅适用于51份费用和48份BR-2明确观测主样本。44份其他旁文件含6份非空WAL继续独立保留。种子库原有隔离模型路径与恢复后零增量分别解释。C2037/C2198、缺包夹具异常、前置红库字节身份限制、封装别名计数及其他历史首败保留。packaging-first仅为首次封装元数据和文档上下文。

BR1-KI-001保持未解决、用户接受非阻断，再现实际模型名不一致时停止对应任务。临时页面控件测试保持取消。真实来源完整分类、IME、跨屏DPI、辅助技术、人工长时、专用环境断电及browser九项副作用沿各自范围记录。

## 下一阶段的Proposed工作

先核对官方模型、协议、价格版本、计费规则和可靠计费上界，资料不足时继续锁停。随后准备唯一新隔离库绑定策略及监督配置，集中确认合成数据、固定输入、目标与白名单、初始及全部续轮最多3次、可信确认起10分钟、USD0.10、失败停止、原生凭据录入和证据要求。真实执行继续等待独立集中授权及逐任务可信确认。后续核实、实现和执行分别按新增授权处理。[集中授权准备](xuanshu-br2-real-proposal.md)
