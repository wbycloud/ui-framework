> 2026-11-10使用记录：本清单匹配的四个生产文件及冻结identity保持不变。一次明确批准的真实运行正常关闭，但未完成业务snapshot读取，真实验证及其交付收口暂不通过，见[本次记录](validation/xuanshu-br2-meter-real-20261110-validation.md)。本地材料和原冻结结论不扩大为真实业务通过；本日单次命令和包已经消费，不是可重复验证入口。

# BR-2逐轮计量冻结快照与本地交付清单

2026-11-09。基于现有匹配产物整理，未重新构建、搬移、覆盖或打包程序。[限定独立复审及冻结记录](validation/xuanshu-br2-meter-independent-review.md)已经落盘。

## 冻结身份与生产文件

仓库为D:\应用软件框架\应用层序框架，运行根为其build\xuanshu-br2-meter-20261106-1f8148。delivery-identity.json SHA256为C5A430D7DA479E7706043741FE11D66EE95B976C51B849ADA92A26732BF62596。

| 相对运行根的实际文件 | 用途 | SHA256 |
| --- | --- | --- |
| light/framework_host.exe | DH-1正式宿主 | B5E7FA2FF300297433E8379E83ECB80F34D4D41E9B4CDE5B7490A78D899B148F |
| light/ui_framework.dll | 同一Service、业务read及BR-2生产门 | 7FEFFBD5F03EC8116918950C9C7FFDBBECDA3B423C19592B171F7459688B7375 |
| light/br1-production.uapp | 实际apps/browser及包内合成样本 | 6C23ED0F1AAF3FBC8AA191804C57BABFF131B07F2D7AEEE15933524E0F08BD3B |
| light/xuanshu_br2_meter.exe | 同目录固定身份的完整请求计量组件 | 7E2C2FF2508A83F05E1DB5B891635DABFA843812B4078BC0D7D4FEC3CB6C0EBB |

文件不能混用其他build目录的DLL或同名组件。组件所需VCRUNTIME140/UCRT及宿主所需Windows系统库以同根三个dependents文本为依据；不需要Rust工具链，不在本轮安装或替换系统依赖。固定编码及tokenizer嵌入组件，component目录保存同身份副本、许可证及独立证据，不是第二套执行引擎。

应用读取链为browser.view → browser.snapshot，字段state、revision。state是完整顶层对象，包含标题、地址及布局/资源/错误/诊断/性能等状态，不能称为仅title。包内正文12/9/3未被该接口汇总。包属于合成数据的实际业务链，不代表任意用户业务覆盖。

## 当前锁停入口

下列命令仅供核对，本轮没有执行。

~~~powershell
Set-Location -LiteralPath 'D:\应用软件框架\应用层序框架'
& '.\build\xuanshu-br2-meter-20261106-1f8148\start-readonly.ps1'
~~~

启动器将工作目录设为light，按既有监督机制新建唯一sessions目录并核对launch-files。UI_AI_DB_PATH指定新库；模型配置是该完整库路径追加.model.json；UI_BROWSER_SETTINGS_PATH指定本会话空来源配置；UI_AI_BR2_POLICY指定与库绑定的派生策略。LOCALAPPDATA不改变，不迁移旧库、不复制或复用凭据。

原policy-proposed.json继续verified=false、费率0、ceiling=0。现有监督函数明确拒绝verified策略，因此这个命令是锁停本地入口，**不是已经可执行的真实验证入口**。不能改原模板布尔值后继续，也不应绕开其检查。未来批准的一次受信会话方案见[准备记录和授权包](validation/xuanshu-br2-meter-real-preparation-validation.md)，本轮仅写说明。

原start-readonly.ps1 SHA256为A9D70493374F208E94ECF15790EA1820317DBBD010A19D51980F4E33051E0DC4，supervise.ps1为C68D7DEB827DDD94E77009B54AAC3F2D569D7FCF86E75F0779BC40E3F2AC809D。启动监督8项及辅助2项只按原文件身份和适用范围复用，不称为本轮重新验证或新批准会话的测试。

## 监督、停止与关后证据

保留prestart、PID、stdout/stderr、阶段异常及实际退出码。启动失败无进程时不编造PID或退出码；启动后辅助记录失败仍须监督实际进程。三项临时环境变量均应恢复；不自动重启、不强杀。主证据目录不可写时如实报告，不能声称已经保存。

正常流程通过“保存任务并查看范围”本地接纳，完整范围与详情核对后，实际外发仍需独立的逐任务可信确认。配置ready不是凭据可用或服务端验证成功，display_valid不是确认资格。取消请求不等于cancelled或远端停止；计量中取消等可能保存failed。未知、存储失败及cleanup-only必须保持原事实。

用户正常关闭后先确认进程真实退出，再检查本次自有库旁文件。只有没有未合并WAL时才采用不会创建WAL/SHM的只读核对；有WAL则保留材料和证据限制，不checkpoint或忽略它。诊断仅使用脱敏输出，不读取密码框或完整内存。

## 生产、验证与文档分别保留

118项清单包含运行、链接和调试材料，不全部是日常必需程序。ui_framework_edit_test.dll、br1-isolated.uapp、fixture计量组件及私有可信事件均只用于隔离验证，不能替换生产组合。其余测试程序、JUnit、失败源码及自有库属于证据，不是业务启动文件。

source-final518项与原交付源码副本、binary-files、launch-files及identity对应。入口444份副本不是全部Git快照；后续文档更新不能改变原冻结程序身份。原开发文档字节保存在delivery-final，当前工作区文档另按本轮准备清单核对。

已有开发、限定独立复审、历史自动/人工观察和真实Provider有限结果适用范围分别保持；没有新增真实或人工验收。ASAN首败、release组件边界、全部旁文件、BR1-KI-001和已取消控件测试状态见[冻结记录](validation/xuanshu-br2-meter-independent-review.md)。
