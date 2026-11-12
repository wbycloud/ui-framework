# PB-1已冻结正文只读交付清单

2026-11-10。范围为已通过限定独立复审的PB-1离线快照，不是新策略或真实摘要验证。见[冻结归档](validation/xuanshu-page-body-independent-review.md)、[原开发证据](validation/xuanshu-page-body-validation.md)及[使用入口](xuanshu-page-body-entry.md)。

根为build/xuanshu-page-body-20261110-a64c28，identity SHA256：CEFE1130580DE40480F5D5963F4DFD172A45CD7E59FB0EB10C1175E19AD764B0。

| 文件 | SHA256及用途 |
| --- | --- |
| light/framework_host.exe | 067E79F2D6C8B1B200DD52622616884C58920DDA640CD6ABBE0F1F3845331793；正式宿主 |
| light/ui_framework.dll | 85215C6543318C6A7A1CBBE120BD64E017AA29C938EBB002A3F24605FA2A1300；生产DLL |
| light/body-production.uapp | 5C818BF820BF67C6822C4FA08DC5272A4278B0FB1EA60CD2349294E5AB657DDD；真实browser及合成业务样本 |
| light/xuanshu_br2_meter.exe | 7E2C2FF2508A83F05E1DB5B891635DABFA843812B4078BC0D7D4FEC3CB6C0EBB；固定纯计量组件 |

完整运行文件以该根launch-files/binary-files为准；ASAN、测试DLL、私有事件、红例和旁文件仅供验证诊断，不替换生产文件。start-readonly.ps1仅在另有启动授权时使用，明确唯一新隔离库、来源配置和DB+.model.json，保存PID、stdout/stderr、退出码与异常；不改变LOCALAPPDATA、不迁移库、不复制凭据、不重启或强杀。

原启动模板仍Proposed且不支持正文真实验证。下面的新策略开发形成独立身份，不继承本快照冻结结论，不改写或重新打包本目录。真实使用必须满足新策略独立复审及具体授权门槛。
