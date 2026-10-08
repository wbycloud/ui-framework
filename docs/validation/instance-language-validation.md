# API9 应用实例语言验收（2026-10-08）

本轮只修改框架和框架自有示例。接手干净 `cf0e83a3b3d478cfeba27fb841b185e4e80cf631`；未访问或修改请求方业务仓库、冻结SDK、原包、历史证据或PERF-001。SDK0.9.0-dev / API9 / 标准9；应用ABI1、包格式1；没有推送、远程工作流、发布或稳定标签。

## 实现与身份

| 本地提交 | 内容 |
| --- | --- |
| `ee6a2a83a37ef968968f473be25a556bac284a80` | 每host语言、来源查询、通知、框架统一字典、宿主活动跟随、原位标题/字段/枚举标签；完整DLL应用自有UIL1与当前API9测试/CI要求 |
| `8db942f00f7fb4c8f083c90b1efbff307871a543` | 错误节点准确定位，实际中英文校验文案、非法草稿及业务提交次数验证 |
| `6d887703ca1e5bde0cc1e19ea2129d9eb0645a27` | 关闭/unmount期间允许清除回调，继续拒绝新增回调；失败复现与修复 |

最终产物绑定第三个提交，逐文件身份见 [product-artifacts.json](instance-language-evidence-20261008/raw/product-artifacts.json)。原生捕获9df202f、异步轨道2e87e02、列序/捕获f0b161c和消息泵0fe0864保留。没有提高节点、缓存、图片、投递或资源阈值，没有恢复分页按钮。

接口、来源、线程、完整size、复制所有权、回调重入、模态和存储合同见 [实例语言](../instance-language.md)、[迁移](../migration-v0.8-to-v0.9.md)和 [完整示例](../../examples/stateful_components/README.md)。宿主确认用打开时目标语言快照；系统文件选择器/系统菜单仍遵循Windows语言。业务值、路径、原参数和已有结果不自动翻译。

## 实际运行条件

Windows x64已登录交互桌面、MSVC19.50.35727.0、C11 Release/Ninja。固定 Lexbor `7fb22cf5664a331d7c24b113489e566767c9c25a`、QuickJS `2f0aa72a6b09cf69ff06399bcf9f4c083ac1a278`、WebView2 SDK1.0.4129.50，Loader实际查询Runtime `154.0.4258.62`；显式Mesa24.3.4 OSMesa x64提供方SHA256见 [依赖身份](instance-language-evidence-20261008/raw/dependencies-final.json)。无新依赖，不把轻量后端结果替代Runtime。

真实Windows查询得到首选 `zh-CN`、有效 `zh-CN`、USER_PREFERRED_UI提供者、fallback0。zh-Hant-TW、ZH-hk、en-GB、fr-FR只是可控映射单元输入；没有修改机器显示语言，不能声称运行了英文或法文Windows环境。

所有截图都来自实际窗口与加载的完整示例DLL，包含树、十万行64列表格、属性、异步图片和实际OSMesa MSAA帧。维护测试程序复用独立宿主的实际HTML/C、窗口过程和生命周期；测试隔离宿主最近记录，不向请求方应用写入文件。这是框架集成证据，未开展真实业务试点。

## 用例与结果

| 范围 | 实际结果与证据 |
| --- | --- |
| API9核心 | 设置/查询/回调次数、重入、两host隔离、非法/不支持/短size、错误UI线程；标题/枚举标签不改语义值；迟到数据接受且不重复查询；关闭清除回调的红/绿复现 |
| 完整语言示例 | Light及真实Runtime各10种偏好文件校验，再96/144/192的write/restore，共12个独立进程。真实SendInput命中本PID的公共presentation可见区域，检查最终状态；A中文/B英文、8轮标签往返、后台隔离、跨进程保存恢复、模态、草稿/选择、内容HWND、GL、AI回收、关闭和卸载均通过 |
| 当前完整矩阵 | 最终产品6d88770：62项中61通过/0失败/1物理跳过，398.63秒；[日志](instance-language-evidence-20261008/raw/full-closure.log)及[JUnit](instance-language-evidence-20261008/raw/full-closure.xml)绑定本轮产物 |
| 原生无Web | 最终产品6d88770重新构建并运行22项：21通过/0失败/1物理跳过，6.52秒；[日志](instance-language-evidence-20261008/raw/native-closure.log)，不把有Web构建替代无Web配置 |
| 实际校验错误 | 两后端直接查询error-color真实节点，英文/中文文案随语言更新，非法草稿和submits==1保持；[修复复验](instance-language-evidence-20261008/raw/language-error-leaf.log) |
| 关闭稳定性 | 最终产品两个独立Runtime进程各32个真实呈现就绪周期，末端pending0；句柄390→391及391→392，两次GDI12→12、USER17→17。日志runtime-closure32-1/2保留原资源断言；前一提交结果另存，不混用 |
| 原生滚动及GL | 最终矩阵保留真实拖到底、滚轮后拖回首、未缓存末列查询/显示、resize/焦点；OSMesa、MSAA及非MSAA、有窗口GL、实际DLL和Session1对照均执行。Session1对照不代表服务Session0 |

偏好文件用例覆盖缺失、空、截断、错误magic、未来版本、非法语言、档案不符、保留位非零及有效中/英文。拒绝文件逐字节保持，默认宽度200不被翻译/适配覆盖；A/B锁与UST1状态独立，首次可见语言正确。公共输入semantic诊断日志明确不作为真实鼠标通过。

## 保留的失败及修复

原始日志保留在 [raw](instance-language-evidence-20261008/raw)。首次API8编译找不到language.h构成接口红例。随后Light诊断确认不支持title属性getter及宿主局部变量t遮蔽翻译函数，已改用现有能力和明确函数名；AI展开=-1的旧合同及消息刷新就绪也有独立失败/修复记录。

测试脚本首次误用只读HOME变量、补充编译首次目标名错误，均保留日志。错误目标后运行的旧二进制结果不计新用例通过。Light父容器呈现查询读取自身文本，不聚合子节点；改为准确查询错误叶节点，原失败不删除。API升级后包格式、workspace及宿主生命周期测试清单仍写8/未来版本仍写9，修正为当前9/未来10，未放宽原断言。

期间桌面锁定，真实输入命中LockScreenBackstopFrame/其他PID、SendInput0；保留runtime-first失败，用户正常解锁后用真实鼠标重跑。一次Runtime首次导航超时、一次异步清理超时及一次剪贴板断言失败仍保留；后续独立及完整矩阵通过不构成根治结论。没有强杀Runtime、清用户目录、额外延时或提高阈值来遮盖。

关闭保护期间原callback移除返回CANCELLED的2条失败已定位：现仅允许NULL清除回调并清空user_data，新增回调仍CANCELLED；[红例](instance-language-evidence-20261008/raw/cleanup-red.log)、[绿例](instance-language-evidence-20261008/raw/cleanup-green.log)及完整应用卸载复验分别保留。

## 真实截图

普通浅色基准均1200×800、程序DPI96，同一示例/数据源/初始列宽；中文full图是在加入保留草稿和选择后切换，差异明确属于测试操作。深色中/英文对照保留相同草稿、选择、列宽和数据。截图仅BMP无损转PNG，未生成或拼造；原BMP与PNG哈希、DPI和运行路径见 [screenshots.json](instance-language-evidence-20261008/screenshots.json)。

| 场景 | 截图 |
| --- | --- |
| 接手API8完整窗口 | [修改前](instance-language-evidence-20261008/before-api8.png) |
| Light同尺寸中英文 | [English](instance-language-evidence-20261008/light-english-full.png)、[中文](instance-language-evidence-20261008/light-chinese-full.png) |
| Light同状态深色 | [English](instance-language-evidence-20261008/light-english-dark.png)、[中文](instance-language-evidence-20261008/light-chinese-dark.png) |
| Runtime完整中英文 | [English](instance-language-evidence-20261008/webview2-english-full.png)、[中文](instance-language-evidence-20261008/webview2-chinese-full.png) |
| 双实例/助手/空宿主 | [A中文](instance-language-evidence-20261008/light-chinese-dual.png)、[B英文](instance-language-evidence-20261008/light-english-dual.png)、[助手](instance-language-evidence-20261008/light-chinese-assistant.png)、[关闭最后实例](instance-language-evidence-20261008/light-empty-after-close.png) |

两主题、窄窗和144/192另有实际截图；程序DPI和自动输入不是物理跨屏或真实IME验收。

## SDK与复验命令

匹配SDK已完成：`ui-framework-sdk0.9.0-dev-api9-6d88770-windows-x64.zip`，2648文件、96,297,031字节，SHA256 `7c1dbffce1211b2c0654b41d2942b05adbd4c54d111e983d836c27550d8802e1`。产品6d88770、源码/文档快照312bce4；归档后外部证明另行提交，包内不自包含归档哈希。见[完整身份](instance-language-evidence-20261008/raw/sdk-archive.json)。本地归档位于build/language-20261008，不假定GitHub已提供此未发布文件。

含匹配include、静态/导入库、共享DLL、宿主、完整.uapp、完整源代码、固定Lexbor/QuickJS/WebView2 SDK、OSMesa DLL及Mesa源归档、许可证和逐文件清单。CRC、全部2648文件身份和解包后的实际C11共享调用通过；调用验证设置/查询/通知/文本、拒绝不支持语言、宽度保持。

实际执行解包的ui_instance_language_test.exe、ui_framework.dll、stateful_components.uapp及OSMesa DLL；两后端各10种偏好输入和96/144/192写入/恢复（12个独立进程）真实鼠标通过。[运行身份](instance-language-evidence-20261008/raw/sdk-language-manifest.json)含准确路径/哈希/退出码；不是仓库重编译程序替代交付产物。[SDK Light英文](instance-language-evidence-20261008/sdk-light-english-dark.png)/[中文](instance-language-evidence-20261008/sdk-light-chinese-dark.png)和[Runtime英文](instance-language-evidence-20261008/sdk-webview2-english-dark.png)/[中文](instance-language-evidence-20261008/sdk-webview2-chinese-dark.png)来自解包后的完整窗口，原图/PNG身份另记sdk-screenshots.json。

独立新构建目录只用解包source/.deps，构建实际宿主、完整包、语言测试及GL；语言核心/C++公共头/实际OSMesa GL 3/3通过（0.72秒），[日志](instance-language-evidence-20261008/raw/sdk-offline-tests.log)。MSVC19.50.35727.0、Windows SDK10.0.26100.0；没有网络下载。全部交付文件在实际运行后重新校验仍一致，没有修改SDK内容。

```powershell
# x64开发终端；提供方是绝对路径
cmake -S . -B build/current -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_FRAMEWORK_ENABLE_WEBVIEW2=ON -DUI_OSMESA_LIBRARY=C:/providers/osmesa.dll
cmake --build build/current
ctest --test-dir build/current --output-on-failure --output-junit results.xml
powershell -NoProfile -File tests/run-instance-language.ps1 -BuildDirectory build/current -Provider C:/providers/osmesa.dll -Backend light
# 同命令改Backend为webview2，要求真实Runtime
```

实际本轮目录、构建/复验脚本、日志与产物哈希记录在raw；不以编译成功、接口返回值、离屏或旧结果替代当前真实呈现。

## 待验边界

服务创建访问返回Windows错误5，未创建或修改服务：最终源码Session0仍待权限。用户已确认无严格整机无登录服务runner，门槛不放宽。没有托管CI执行授权，工作流配置不是远程执行证据。真实中文IME、不同DPI物理屏幕、桌面边缘/合成及长期人工仍待条件。仅真实zh-CN系统环境，其他系统映射是单元证据。Runtime早期瞬态失败及历史持续重绘来源未根治；本轮未扩大硬件无窗口GL。全部验收尚不能宣称完成。
