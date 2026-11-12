# PB-2限定独立复审及匹配快照冻结记录

2026-11-11。本记录追记上一轮已完成的限定独立只读复审，并依据本轮必要身份核对归档冻结；不是重新测试或再次完整复审。结论：PB-2正文验证保护限定验收通过，必须修正项为空，匹配快照可以冻结。本轮将该匹配快照冻结记录落盘，不执行commit、tag或release。

匹配根：build/xuanshu-page-body-policy-20261110-d42b8e。delivery-identity.json SHA256：46813CE1DAB1C2DD5620D58A5FFC90306076AFCF26AEDED2BEC620F5BA1D7496。537份源码、241项程序及链接/调试产物、关联清单及四项生产文件匹配。551份入口范围不是全部Git内容快照；产品增量为4份私有运行时实现及CMake/专用回归，共6项。PB-1冻结身份CEFE1130580DE40480F5D5963F4DFD172A45CD7E59FB0EB10C1175E19AD764B0保持，不自动覆盖新代码。

| 生产文件 | SHA256 |
| --- | --- |
| light/framework_host.exe | D9870808CB9A22C86BD1D424448FAFF1BFDFFE8D09559C436F4BF8111211EDE7 |
| light/ui_framework.dll | 124E7BB6F8B3B0B470DC242CA1B6E5A7F76BA370FBA0915BDA9227F5F397154A |
| light/body-production.uapp | 5CB06AEE16E08611E621E1A24871349EEE2DCD30C8482A32EDB0F3FF327F2F48 |
| light/xuanshu_br2_meter.exe | 7E2C2FF2508A83F05E1DB5B891635DABFA843812B4078BC0D7D4FEC3CB6C0EBB |

dom-body-v1严格限定固定正文输入、history=[]、单一browser.view只读目标、browser.body及body/revision；旧snapshot分支保持，未知或矛盾范围锁停。开启business_scene和任务validation_scene在READY前交叉审计；破损账本拒绝打开，完整恢复保留计数、费用及unknown并锁停，不恢复旧授权、请求或工具执行。初始及全部续轮复用同一计量、持久化占用和发送门，共享3次额度、可信确认起10分钟及USD0.10，失败不返还或自动重试。

147字节正文来自实际browser解析和read链，类型、bytes/complete/semantics、revision、续轮与工具保存逐值对应。run/hash、展示代次、实际全文/布局、末次回调及隐藏/PENDING独立拒绝保持。任务、草稿和当前配置分离；回调重入、引用退栈、cleanup-only和安全关闭有直接证据。生产358项导出与PB-1一致，无私有入口；公开头、DDL、错误值、版本、来源、依赖、预算和原时限保持。

| 已有运行 | 结果与适用范围 |
| --- | --- |
| Light closure-final | 201/201；其中36例后来加强，其余165例保持匹配 |
| Light strengthened-final | 另次36/36，替代对应早期36例 |
| ASAN-Light direct-final | 36/36 |
| ASAN-Light compatibility-final | 另次165/165 |
| Native closure-final | 7/7 |

上述五次运行分别记录，不合并为首次单次全部通过。源码、实际程序、测试前清单、原始日志/JUnit和清理前VALID依赖已在原复审核对；本轮不重跑。修前正文提案拒绝、目录与恢复状态期待的夹具错误、/WX变量遮蔽编译首败分别保留。72份最终主库无旁文件及immutable前后哈希一致仅适用于对应范围，其他旁文件和历史材料独立保全。原交付文档副本和identity不回写；本文是后续归档状态。

非阻断风险：计量辅助进程若任意卡死，等待真实退出的路径不能证明有界结束，取消/关闭可能持续PENDING；不绕过计量向Provider发送，不强杀或提前释放。新具体真实命令及普通控件驱动只经静态审阅，尚不具备动态执行正例。隔离可信事件/模型替身与生产公开read分别证明其路径，不能拼成真实人工授权或真实摘要质量。PB-1初始化20.03秒首超时及其他历史期限/复制风险未被新通过解释。BR1-KI-001仍未解决、用户接受非阻断；已取消的临时页面控件测试不恢复。

具备冻结记录落盘、本地交付整理和具体真实授权包集中确认的准备条件。PB2-REAL-20261110-d42b8e已跨日，仅作历史准备材料；BR2-REAL-20261110-111dd1已消费。任何新真实执行仍需当日官方依据/身份核实、全新具体包独立批准、必要原生凭据录入及本人逐任务可信确认。冻结不等于真实服务、生产人工或整个系统完整验收。


## 后续执行记录

2026-11-11，经独立具体包批准、原生凭据交接及本人逐任务可信确认，唯一生产任务已完成并正常关闭。实际结果、费用、关后持久化及驱动首败见[一次真实正文验证](xuanshu-page-body-policy-real-20261111-validation.md)。上文记录原复审时点的证据和限制，原快照身份保持。
