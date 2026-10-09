# 当前API9新会话恢复提示词

复制下面代码块发给新会话。本提示词只授权恢复上下文与只读核对；下一轮开发、远程执行或应用修改等待用户另行指定，不新建会话或后台任务。

```markdown
请接手 https://github.com/wbycloud/ui-framework，先恢复当前交接上下文。本次仅核对本地基线、交付和证据，报告后待命；不要重复开发语言能力，不立即构建、测试、修复、清理文件、fetch、推送或触发工作流。

先读docs/handoff.md（重点第21–25节）、docs/validation/instance-language-current-validation.md、docs/instance-language.md、examples/stateful_components/README.md、docs/application-development-standard.md、docs/build-and-validation.md。根据后续任务再读宿主、组件、布局、菜单/离屏、Runtime和CI收敛记录；docs/ci-recovery-prompt.md是较早快照，不作为当前未实现清单。

参考基线以接手实际Git为准：
- 持久化准备前main为419290bc9ebf6fb8c4056f3139932d89b6dec219，工作区干净；交接文档提交在它之后，不是新产品测试身份。
- SDK0.9.0-dev/API9/标准9，应用ABI1/包格式1，Windows x64/C11。API8/cf0e83a旧提示词已过时。
- 44c039fce36c191dba03631a7bcc1c5756e2dd90保存本轮说明/回归，419290b保存最终SDK外部证明；产品、公共头、测试/示例实现与新构建基线876588246a72d261c88b9e1906372d1b5b91d1ca相同，只有示例README说明更新。
- 最后产品修复07a66a8；af36e02/e0dcf78/07a66a8界面、4fdfcc2Runtime输入点、f00bbeb精确语言断言、2fa6f84阶段诊断均已有实现。

中英文能力已完成且重新验证：应用拥有设置与UIL1存储，稳定A/B档案隔离、首次呈现前提交；每host独立语言，宿主跟随活动实例，后台不污染，最后关闭恢复Windows显示语言。未提交host用系统语言，完整示例无有效偏好时明确选择英文。同步通知和稳定ID原位文本更新不重载DLL/HWND/GL，不翻译用户值、不改变用户列宽/布局/查询generation。宿主权限确认使用打开时目标语言快照。不要新增全局语言开关、另一套存储或重复接口。

本轮结果必须保留准确身份，不能当新会话的新测试：完整64项63通过/0失败/1物理跳过，原生22项21通过/0失败/1跳过；两后端完整应用覆盖偏好异常、96/144/192跨进程、双实例、草稿/选择/模态/GL/AI/关闭。确认快照两后端共4场景通过。最终解包语言14个独立进程正常退出，包内固定依赖离线构建及语言core/公共C++头/实际OSMesa三项通过。真实截图包含完整树、十万行表格、属性、异步图片和实际GL；不是空窗口或静态页面。

先检查git status、HEAD、分支及提交差异，保留用户修改；只读核对docs/validation/instance-language-current-evidence-20261009/sha256.json中143份证据，不重写raw、历史失败或截图。关键日志、manifest和18张PNG已入Git。完整源BMP、SDK和原CI下载目录被build忽略，clone不会带入。

最终SDK：build/language-current-20261009/ui-framework-sdk0.9.0-dev-api9-44c039f-windows-x64.zip；源码/说明快照44c039f，3602文件/117891276字节；SHA256为28ca0b8409b321477f1aa3adce2680fda8cf2cd8a9a35dd4308a5f08ef1f79a2。证明见raw/sdk-final-archive.json、sdk-final-preservation.json、final-sdk-language-*/manifest.json。SDK不补写后续提交；旧6d88770/07a66a8及本轮基准包保持。缺少文件时如实报告，禁止重生成冒充原交付。不要执行raw收集/打包程序覆盖已有记录。

固定依赖：Lexbor7fb22cf5664a331d7c24b113489e566767c9c25a，QuickJS-NG2f0aa72a6b09cf69ff06399bcf9f4c083ac1a278，WebView2 SDK1.0.4129.50，Mesa24.3.4；上轮Windows10.0.26300/Session1/MSVC19.50/Runtime154.0.4258.62，真实系统UI仅zh-CN。当前环境重新核对，不能盲目prepare覆盖依赖或把程序DPI当物理DPI。

未完成事项：
1. 原托管Light存储180秒/语言240秒超时根因未定位；诊断已补，本机通过不代表目标CI修复。历史持续重绘/瞬态Runtime、泛化192叠层与窄表头残影亦未根治。
2. 最后核验远端为历史b6a36fee3f4c1a73818cdb01ea7271ad5abd30db，本轮未查询实时远端；旧同步授权已用完，新推送/托管CI需要明确授权。历史CI与Session0见github-sync-validation.md。
3. 历史b6a Session0实际GL通过，但当前源码无服务复验；上轮非管理员。严格无登录原logged_sessions1失败，用户没有服务runner，不注销用户、不改既有服务、不降门槛。
4. 实际英文Windows空宿主、真实中文IME、物理混合DPI/桌面合成/长期人工待条件；英文实例和受控映射不能代替系统环境。
5. 本轮不访问或修改业务应用。请求方仓库、冻结SDK、原包、历史证据及PERF-001保持只读；正式接入需要另行明确授权。

共同约束：保留9df202f/2e87e02同HWND捕获/异步轨道和f0b161c列序/调宽捕获；完整范围滚动uint64/BigInt与按需查询不变，不恢复分页按钮或旧版本专项，不提高预算或删除失败断言。只维护当前版本，新增公共能力按仓库规则处理。获新开发授权后新建本地构建/证据目录，真实输入串行且解锁，检查实际Dispatch、最终状态与进程退出；保留原超时和资源门槛。不得使用graph-engineering、子代理或新会话，不自动推送、发布、创建标签或扩展硬件GL。

恢复后简短报告实际HEAD/用户改动、已完成能力、证据是否完整、待验和授权边界，再等待用户给出下一轮具体任务。
```
