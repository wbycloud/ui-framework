# 当前交接：2026-10-22

本页优先于旧交接中的“当前状态”。作者：Codex。此次只做持久化、源码完整性核验和已授权的提交推送，不重新开展界面或 Mesa 专项。

## 基线与仓库内容

接手 HEAD 为 `689fa03b34f8c4321a6c838f52ca9dcd790a9319`，分支 `codex/p1-process-boundaries`。本页所在的后续提交保存菜单/提示修复及此前已完成的受限浏览器集成；实际恢复时先核对 Git，不把旧工作树哈希当成当前提交身份。

SDK0.9.0-dev/API9/开发标准9、应用ABI1、包格式1、Windows x64/C11。新开发能力需要同一快照的头、导入库、DLL、宿主和应用包，冻结SDK不变。公共合同见[版本政策](version-policy.md)、[实例标题和主题](instance-title-and-theme.md)、[受限文档](restricted-browser.md)。

Git保留源码、资源、构建/测试程序和必要说明；本轮不新增日志、截图、转储、SDK或证据目录。以前已入Git的历史材料不删除。`build/`、`.deps/`及应用外部交付不是clone内容；下面的本地证据不存在时如实报告，不重新生成冒充原件。

## 已完成与证据边界

| 项目 | 当前结论 |
| --- | --- |
| 正式宿主菜单留白 | 实际使用字符数估宽，按钮间距为0，留白来自内部宽度；改为同字体/DPI的实际测量加点击留白。48项明暗/程序96、144、192几何检查通过，溢出、Alt/Enter和真实关闭按钮验证通过。 |
| 无关状态更新关闭提示 | 公共C最小复现确认 `replace_caption` 触发host级隐藏。改为组件归属失效；保留公开host级隐藏和必要生命周期清理。 |
| Light父子命中提前离开 | 进入子span误报父按钮leave；改为共同祖先边界事件。公共分发、真实Light及Runtime对照验证。 |
| 所属错误文字更新后提示陈旧 | `set_error` 缺失必要失效，已补齐。重新悬停显示新文字。 |
| 动态标题、主题、文本替换和稳定几何 | 早于本次持久化已完成，不重复开发；标题/主题合同及测试源码已保存。Runtime真实同值粘贴的历史失败仍单列，不外推所有编辑路径已根治。 |
| 受限浏览器 | 既有独立批次已完成M0/M1/M2，采用公共C、Light、Lexbor、WinHTTP/WIC；远程页面无JS/应用桥。框架默认构建必须包含 `apps/browser`、`light_document.h`、`app_data.h` 和受限实现，不能只推共用文件而漏掉它们。详见[应用README](../apps/browser/README.md)、[M2支持表](../apps/browser/docs/m2-support.md)和[M2历史验证](../apps/browser/docs/m2-validation.md)。M3—M5未完成。 |

菜单/提示专项是2026-10-19历史测试，不是本轮重新测试：相关CTest首轮23/24通过，旧溢出夹具补足菜单数据后该项1/1通过；ASAN7/7通过。Light/Runtime×明暗四组最终生命周期父进程OS0，所观察的12个Runtime子进程OS0。248导出前后相等，无新增公共接口。100次布局GDI119→119、HANDLE191→191；40次菜单重开HANDLE289→289、GDI857→857、USER50→50。

该专项源码/资源SHA256为 `9ecb410caca793dcea787f20193f62fb899dfddba4f4393f588963de3bc75328`；随后浏览器改动已改变4个共用文件，不能把该SHA称为当前全部工作树身份。原报告在本地 `docs/validation/menu-tooltip-20261019-validation.md`；原始材料在 `build/menu-tooltip-20261019-5a840d43`，含首次失败、匹配PDB/转储、424张必要真实画面及哈希清单。历史24文件开发ZIP的SHA256为 `f035711b0c3350d1d5cba9a08cb11bad5c1ba211e72a57748cd481b54f8d3f0e`，不是当前浏览器集成SDK，不能补写。

浏览器M2历史验证包含生产tests OFF/WebView2 OFF/Mesa空配置、匹配SDK独立构建、公共C阅读/隔离/应用调用方和关闭真正卸载。不是本轮新的完整框架回归；其外部deliverables路径不会随框架clone提供。应用复制中的独立工作区打包/同步脚本可能引用原维护目录，框架clone构建以根目录CMake和下节命令为准。

## 构建恢复

在MSVC x64开发终端，按[构建说明](build-and-validation.md)准备固定依赖后：

```powershell
cmake -S . -B build/current -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/current
```

固定Lexbor `7fb22cf5664a331d7c24b113489e566767c9c25a`、QuickJS-NG `2f0aa72a6b09cf69ff06399bcf9f4c083ac1a278`。WebView2 SDK `1.0.4129.50` 和Mesa `24.3.4`只在相应配置需要；默认不替换provider、不自动下载升级。完全不使用Web的构建需同时关闭 `UI_BUILD_STANDALONE_HOST`、`UI_FRAMEWORK_ENABLE_LIGHT_WEB`、`UI_FRAMEWORK_ENABLE_WEBVIEW2`。

本次源码完整性检查及结果见[2026-10-22推送核验](validation/handoff-20261022-validation.md)。它只证明列出的构建和检查，不替代真实桌面/物理条件或完整历史矩阵。

## 遗留问题分账

- 额外正式宿主Alt+F4脚本：入口及候选在10秒内均未关闭，前台PID正确，但缺完整按键Dispatch链；责任尚未定位。诊断器只终止自有目标为254。后续真实关闭按钮OS0不能解释该失败，亦不能据此认定已进入Mesa退出。
- Mesa默认A的 `lp_rast_destroy → util_semaphore_wait` 退出等待历史已复现，候选C一次目标A/C自然退出；不能写成所有退出均未定位或所有托管超时已解决。B/C目标对照、原子259→0、多线程正常screen清理及真正UNLOAD仍待闭环。LP_NUM_THREADS=0是因果对照且有性能代价，不是原多线程修复；C未纳入默认provider或SDK。
- Runtime144冷write曾主OS0但两子进程原10秒仍259，未及时取得失败转储；后续限定对照通过不解释首次失败。Runtime96真实同值粘贴后Y丢失也仍未定位。产品Runtime、首次restore144和诊断mirror pending分开追踪。
- 泛化192叠层、窄表头残影未独立复现；已修的Runtime144捕获边界与诊断截图时序不能冒称显示根因。持续重绘/瞬态Runtime保留各自边界，不能凭单个idle paint0判根治。
- 受限浏览器M01公网12002、早期OLE恢复崩溃、旧测试证书导入不持久化的底层原因仍未定位；本次不修改网络或信任配置。
- 当前源码Session0、严格整机无登录、真实英文Windows/中文IME、物理高/混合DPI、多屏、桌面合成专项、硬件鼠标/外部捕获抢占和长期人工仍独立待验。自动输入和程序DPI不能替代。

## 后续授权与保护

本次授权仅持久化及推送；不自动继续开发、运行远程CI、合并main、发布或建标签。下一轮由用户指定。先恢复实际Git和源码身份，保留所有既有修改与未跟踪文件。新工作使用日期加唯一标识目录，回退只能撤回该轮新增改动。

业务 `D:\应用软件框架\klayoutC`、PERF-001、固定依赖、冻结SDK和历史证据保持只读。不使用子代理、新会话、graph-engineering、computer use、私有调用方接口/JS；不停止未知进程、改服务或注销用户。不放宽断言、超时、资源和数据范围。真实前台输入须重新协调空闲解锁、串行运行，分别核对Dispatch、最终状态和OS退出。收尾仅清理本轮无用途过程件，保留明确要求及未解决问题的最小证据。
