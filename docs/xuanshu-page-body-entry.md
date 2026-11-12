> 2026-11-10后续状态：PB-1限定独立复审通过，原匹配快照冻结记录已落盘。见[复审归档](validation/xuanshu-page-body-independent-review.md)和[冻结交付清单](xuanshu-page-body-delivery-manifest.md)。以下正文按原开发交付时点保留；新正文验证策略形成独立候选，不继承本快照冻结，真实调用仍需新的具体批准。

# PB-1当前网页DOM正文摘要开发交付入口

2026-11-10。正文read开发及离线逐项验证完成，等待限定独立复审；仍为候选，不继承BR-2验收或冻结，不执行真实调用或人工验收。BR-2阶段及已消费具体包保持结束。开发及自动验证结果见[验收记录](validation/xuanshu-page-body-validation.md)，正文语义见[增量合同](xuanshu-page-body-contract.md)。

## 匹配程序和本地入口

唯一候选根为`D:\应用软件框架\应用层序框架\build\xuanshu-page-body-20261110-a64c28`；最终identity和文件哈希以该根delivery-identity.json、binary-files.json及launch-files.json为准。

| 文件 | 用途 |
| --- | --- |
| light/framework_host.exe | 原正式DH-1宿主，同一Service和主对话 |
| light/ui_framework.dll | 生产运行时，禁止换成隔离测试DLL |
| light/body-production.uapp | 实际apps/browser及本轮包内中文合成网页，不是固定返回值夹具 |
| light/xuanshu_br2_meter.exe | 匹配本地计量组件；原发送保护保持 |
| samples/browser-body/start.html | 新合成业务数据，含12/9/3及两位负责人；不是用户文档 |
| asan-light、测试exe/DLL、body-isolated和red目录 | 隔离验证及失败材料，不作生产或人工授权入口 |

启动说明仅供后续有相应启动授权时使用，本轮不运行此入口：

```powershell
Set-Location -LiteralPath 'D:\应用软件框架\应用层序框架'
& '.\build\xuanshu-page-body-20261110-a64c28\start-readonly.ps1'
```

沿用已验证的supervise机制和未核价Proposed策略。入口核对生产文件哈希，新建唯一sessions目录，显式UI_AI_DB_PATH、UI_BROWSER_SETTINGS_PATH和UI_AI_BR2_POLICY；模型配置为数据库路径加.model.json，不改变LOCALAPPDATA、不迁移库或复制凭据。记录PID、stdout/stderr、实际退出码、阶段异常和环境恢复，不循环重启或强杀。启动默认无任务、确认、发送和凭据访问。

当前策略只支持原BR-2固定snapshot场景且未核价，不能接纳/授权新正文真实验证；入口保持锁停，不能改布尔值或复用旧包激活。宿主页正文流程由隔离测试证明，生产DLL公开read另外验证，两类证据不拼成生产真实模型正例。启动监督8项及辅助2项按原函数身份复用，不扩大启动器工作。

## 使用范围

任务输入建议：“读取当前合成交接单DOM正文，概括设备总数、已完成、待处理及负责人；不导航或修改。”历史默认不选；选择明确实例browser.view、browser.body及body/revision。body含归一DOM文本、UTF-8字节数、complete及dom-body-v1标记。原snapshot.state不包含正文，不获得新隐式权限。

“保存任务并查看范围”仅本地提案；“查看本任务范围（只查看，不授权）”展示已接纳任务的固定输入、目标和白名单，草稿及当前配置保持分离。完整详情不能以摘要替代；隐藏/投影失败/旧代次/PENDING时旧确认失效。取消请求与终态保存分别查看，异常保留原错误和unknown，再按合同正常关闭；使用“复制脱敏诊断”而不读取密码框。

正文是作者DOM文本，不等同CSS可见文本；不含脚本、模板或表单控制子树。最大8192 UTF-8字节，超限拒绝、无部分全文。纯空页面若被既有渲染器拒绝则按文档失败处理，不伪造空正文成功。可加载但无可提取正文时返回完整空字符串。未读取其他标签、用户旧文件或完整主对话。

## 后续真实摘要验证仅Proposed

先独立复审本候选；另行明确正文授权范围、固定输入/实例/epoch/revision/scope、能力及字段、允许的合成文本和全部续轮。原BR-2策略没有正文场景，需独立授权最小策略接入及其离线验证，不能靠提示词、普通任务确认或人工计数代替发送硬门。

之后才按新匹配程序核对官方模型/协议/价格版本、完整请求计量、可靠计费上界、总请求数/费用/期限及异常停止条件，准备唯一新隔离库的集中授权包，再由用户原生录入凭据并逐任务可信确认。旧BR2-REAL-20261110-111dd1、数据库、额度和凭据引用不可复用。当前不核价、读取凭据、激活策略或发送；隔离替身不证明真实模型摘要质量。

最终验证：Light131/131及显式隔离另次5/5；ASAN130/131、初始化20.03秒首超时同程序原门槛另次1/1（3.02秒）、隔离另次5/5，逐项131/131；Native7/7。各运行分别记录，期限风险不因补项通过消失。完整程序/源码/文档及证据身份见候选根delivery-identity.json；启动命令本轮仅静态审阅，监督函数按原身份复用，未启动生产验收。
