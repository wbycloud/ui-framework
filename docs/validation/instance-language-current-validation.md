# 2026-10-09 当前源码实例语言复核与SDK交付

## 实际基线与修改范围

接手main/876588246a72d261c88b9e1906372d1b5b91d1ca，工作区干净。参考提示词的SDK0.8/API8/cf0e83a已过时：当前为SDK0.9.0-dev/API9/标准9，应用ABI1、包格式1。ee6a2a8/6d88770已有完整实例语言实现，后续f00bbeb语言断言、2fa6f84脚本诊断及af36e02/e0dcf78/07a66a8界面修复保留。本轮没有确认新的语言产品缺陷，不重复新增接口、不升级API、不修改产品或应用示例实现。

源码核对确认应用拥有偏好与UIL1存储，create在首次呈现前提交；每host保存独立语言，宿主跟随活动实例，空宿主查询Windows显示语言。未提交host使用系统默认；示例无有效文件时由应用明确选英文。语言变更同步通知并原位更新稳定ID文本，不重挂载工作区/DLL/HWND/GL，不改变用户列宽、业务值及数据generation。设置/查询/回调/文本size、线程、复制和关闭合同继续[API9接口说明](../instance-language.md)。

字典166条，所检查的宿主、组件、菜单及工作区字面量调用没有缺失键；这项静态检查不能单独代替呈现验收。修正开发标准当前清单仍声明8的残留为9，并把菜单说明的API8“当前验收”归回历史。相关入口补充本记录，原验收、原失败和SDK归档不覆盖。

## 本轮独立构建与回归

新目录`build/language-current-20261009`，没有执行旧测试程序或改写旧产物。匹配EXE、共享DLL、导入库、包及提供方哈希见[构建前身份](instance-language-current-evidence-20261009/raw/artifacts-before.json)。[准确构建命令](instance-language-current-evidence-20261009/raw/build.cmd)、[构建日志](instance-language-current-evidence-20261009/raw/build.log)及[串行分阶段运行器](instance-language-current-evidence-20261009/raw/run-current.ps1)保留。

| 当前配置 | 总计 | 通过 | 失败 | 物理跳过 | 原始证据 |
| --- | ---: | ---: | ---: | ---: | --- |
| 独立无Web原生 | 22 | 21 | 0 | 1 | [JUnit](instance-language-current-evidence-20261009/raw/matrix-native/ctest.xml) |
| Light、OSMesa/provider与真实WebView2完整配置 | 64 | 63 | 0 | 1 | [JUnit](instance-language-current-evidence-20261009/raw/matrix-webview2/ctest.xml) |

原生10.09秒；完整配置436.60秒，WGL前置46项、provider7项、Runtime11项。合计84通过、0失败、两次同一ui_monitor_transition跳过，不是84个互不重复场景。采用原精确清单/失败/唯一跳过检查；原Light存储180/语言240、Runtime存储240/语言300秒、资源及预算断言保持。[结果汇总](instance-language-current-evidence-20261009/raw/matrix-summary.json)。

本次实际Windows10.0.26300/Session1、MSVC19.50、WebView2 Runtime154.0.4258.62、SDK1.0.4129.50；Lexbor7fb22cf5664a331d7c24b113489e566767c9c25a、QuickJS-NG2f0aa72a6b09cf69ff06399bcf9f4c083ac1a278、Mesa24.3.4。软件WGL四DLL/llvmpipe与OSMesa/provider、真实Runtime阶段沿用既有隔离。实际Windows首选UI名称zh-CN/provider1/fallback0；其他语言映射为受控单元输入，不能说已在真实英文Windows验证空宿主。环境见[记录](instance-language-current-evidence-20261009/raw/environment-final.json)。

两后端完整stateful_components DLL/.uapp包含树、十万行64列表格、属性、异步图片、实际OSMesa frame、布局和助手。语言套件分别验证10种UIL1文件以及96/144/192程序DPI的write/restore独立进程，保持真实SendInput命中、最终状态与实际退出。覆盖A中文/B英文、反复标签切换、后台隔离、关闭非当前/最后实例、系统来源恢复、非法语言/文件、菜单/属性/枚举、模态原位更新、草稿/选择/数据generation/用户宽度/HWND保留、GL和AI空间回收。

完整矩阵另执行排序/展开折叠、颜色/校验、布局/列宽持久化、迟到结果、原生鼠标到底/滚轮后回首/未缓存末列、resize/焦点/捕获、REFUSE/WAIT、初始化失败、实际清理和卸载。核心语言测试验证迟到有效数据仍接收且不额外查询；没有预加载全数据、提高预算或恢复共同组件分页按钮。

## 权限确认语言快照补验

[独立观察器源码](instance-language-current-evidence-20261009/raw/confirmation-observer.c)加载同一完整DLL的A/B实例，在B活动时给A打开宿主权限确认，分别验证中文和英文目标。确认打开后改变目标与活动实例语言，检查完整标题/允许/取消仍为打开时快照，主宿主保持活动实例语言。真实鼠标命中本PID的取消按钮，确认返回拒绝、主窗口恢复启用，AI目标/活动实例、原内容HWND保持，关闭后DLL卸载和捕获释放。

Light与真实Runtime各2个快照场景均0失败：[Light](instance-language-current-evidence-20261009/raw/confirmation-light.log)、[Runtime](instance-language-current-evidence-20261009/raw/confirmation-webview2.log)。此观察器直接测试宿主确认适配器，不执行业务命令；正常权限批准/拒绝和一次命令另由完整矩阵ui_web_host_frontend覆盖，不把适配器测试冒充业务应用验收。

辅助程序初版漏链生成的宿主HTML资源，root创建断言失败；补链接时曾使用错误res路径，原失败和链接日志保留，正确链接当前generated/host.rc.res后原断言通过。脚本首轮使用PowerShell只读HOME变量导致用例尚未执行就退出，改为任务变量；初始构建启动也有路径引号/换行问题。[工具/夹具失败](instance-language-current-evidence-20261009/raw/tool-incidents.md)、[资源失败](instance-language-current-evidence-20261009/raw/confirmation-resource-failure.log)、[链接失败](instance-language-current-evidence-20261009/raw/confirmation-link-failure.log)分别记录，不归为语言产品缺陷。

## 真实截图

截图来自本轮新产物的完整真实宿主/应用DLL，只有无损BMP→PNG转换，没有生成、拼造或编辑像素。[原BMP/PNG哈希与尺寸](instance-language-current-evidence-20261009/raw/screenshots.json)。主对照使用同一A实例、数据、用户草稿/选择/列宽、1200×800、程序DPI96、深色主题；英文与中文的界面文本不同，业务数据/用户文字不翻译。双实例及空宿主图另列，不能把不同实例当作相同状态前后对照。

| 场景 | 中文 | 英文 |
| --- | --- | --- |
| Light完整窗口同状态 | [中文](instance-language-current-evidence-20261009/screenshots/light-chinese-dark.png) | [English](instance-language-current-evidence-20261009/screenshots/light-english-dark.png) |
| Runtime完整窗口同状态 | [中文](instance-language-current-evidence-20261009/screenshots/webview2-chinese-dark.png) | [English](instance-language-current-evidence-20261009/screenshots/webview2-english-dark.png) |
| 同示例双实例 | [A中文](instance-language-current-evidence-20261009/screenshots/light-chinese-dual.png) | [B英文](instance-language-current-evidence-20261009/screenshots/light-english-dual.png) |
| 目标语言的权限确认 | [中文快照](instance-language-current-evidence-20261009/screenshots/webview2-owned-chinese-confirm.png) | [English快照](instance-language-current-evidence-20261009/screenshots/webview2-owned-english-confirm.png) |

[空宿主系统默认](instance-language-current-evidence-20261009/screenshots/light-empty-system.png)、[关闭最后实例后](instance-language-current-evidence-20261009/screenshots/light-empty-after-close.png)、[AI展开](instance-language-current-evidence-20261009/screenshots/light-chinese-assistant.png)。浅色、窄窗、其他程序DPI原BMP保存在本轮完整脚本证据目录，不能冒充物理高DPI、真实IME或长期人工。

## SDK与证据保存

本轮使用新的完整开发快照交付匹配公共头、静态/导入库、共享DLL、宿主、完整应用包/测试、源码、固定依赖和许可证，不重写已封存6d88770/07a66a8归档。[原包哈希核对](instance-language-current-evidence-20261009/raw/preserved-archives.json)。归档精确源码/说明快照、SHA256和最终解包证明在归档后另行保存，避免源码快照自引用归档哈希。

文档修正前的8765882基准SDK已核验3501文件、117108072字节、CRC及逐文件身份，解包C语言/查询/通知/文本调用通过；实际解包DLL/.uapp的两后端完整语言脚本均通过（10种偏好文件/后端及12个独立write/restore进程）。[基准归档证明](instance-language-current-evidence-20261009/raw/sdk-baseline-archive.json)、[实际解包套件](instance-language-current-evidence-20261009/raw/sdk-actual-results.json)。这份基准归档保留，不用其结果替代最终包验证。

最终交付`ui-framework-sdk0.9.0-dev-api9-44c039f-windows-x64.zip`，源码/说明快照为本轮首提交44c039fce36c191dba03631a7bcc1c5756e2dd90；产品、头文件、测试与8765882没有差异。3602文件、117891276字节，SHA256 `28ca0b8409b321477f1aa3adce2680fda8cf2cd8a9a35dd4308a5f08ef1f79a2`。CRC、逐文件校验、解包共享C调用通过；解包目录实际Light/Runtime完整语言脚本各7个子进程全部exited/exitCode0，含每后端10种偏好文件及96/144/192的12个独立write/restore进程。[最终归档证明](instance-language-current-evidence-20261009/raw/sdk-final-archive.json)、[实际调用结果](instance-language-current-evidence-20261009/raw/sdk-final-actual-results.json)、[Light清单](instance-language-current-evidence-20261009/raw/final-sdk-language-light/manifest.json)、[Runtime清单](instance-language-current-evidence-20261009/raw/final-sdk-language-webview2/manifest.json)。

仅使用最终解包源码中的固定Lexbor/QuickJS/WebView2依赖离线构建宿主、完整应用及相关调用方，实际ui_language_core/ui_public_headers_cpp/ui_windowless_gl三项通过（0.69秒）。[构建命令](instance-language-current-evidence-20261009/raw/sdk-offline-build.cmd)、[复验命令](instance-language-current-evidence-20261009/raw/sdk-offline-tests-command.ps1)、[JUnit](instance-language-current-evidence-20261009/raw/sdk-offline-tests.xml)。第一次选错Ninja目标的失败独立保留；改为既有ui_offscreen_msaa_test目标，没有改产品或用例。[目标选择失败](instance-language-current-evidence-20261009/raw/sdk-final-tool-incident.md)。离线构建不等于无外部构建工具或无需安装Runtime。

最终完整套件和离线构建之后，3602个交付文件（包括清单自身）及原6d88770/07a66a8/本轮基准归档大小与哈希保持。[保存核对](instance-language-current-evidence-20261009/raw/sdk-final-preservation.json)。最终归档封存后不再补写源码/证据；本文与外部证明在后续本地文档提交保存，避免归档自引用。实际产物、归档及运行身份分别报告，不将文档提交当作新产品测试身份。

最终文档检查核对455个相对链接、保留此前98个raw/PNG原字节及产品/头/测试/示例代码无差异；示例README属于授权说明更新。检查器首次将这份README算入代码差异的工具失败原输出独立保留，明确范围后通过。[检查结果](instance-language-current-evidence-20261009/raw/final-document-checks.json)、[原范围错误](instance-language-current-evidence-20261009/raw/document-check-scope-failure.log)。

完整本地日志/源BMP/SDK在`build/language-current-20261009`；关键日志、身份、失败与真实PNG纳入本记录的证据目录。SDK包本身只本地交付，不推送、发布或创建稳定标签；用户业务仓库、冻结SDK、历史验收/原始CI证据及PERF-001没有改动。文档及证据另作本地提交，实际SHA见git log，不把文档提交当成新产品测试身份。

## 仍待验与未根治事项

没有本轮推送/托管CI执行授权，原b6a36fee远程失败属于[原CI记录](github-sync-validation.md)。本轮本机全部退出不能证明原180/240秒托管超时或历史持续重绘已根治；原结果不覆盖。当前会话非管理员，没有执行8765882的服务Session0复验；原b6a Session0实际GL通过、严格整机无登录logged_sessions1失败分别保留，不继承为当前源码通过。用户没有严格无登录服务runner，门槛不放宽。

真实其他Windows显示语言、中文IME、不同DPI物理显示器/屏幕边缘桌面合成及长期人工仍待条件。没有修改机器显示语言、注销用户或修改既有服务，没有业务试点或硬件GL扩围。独立源码核对、完整应用回归与本地SDK交付分别报告，全部目标环境验收尚不能宣称完成。
