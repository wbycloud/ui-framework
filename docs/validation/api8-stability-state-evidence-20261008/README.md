# API8 收敛证据索引

本目录只保存2026-10-08本轮真实执行证据，不改历史记录。产品f0b161c、示例／测试d24cdf3、基线660734f；详见[验收](../api8-stability-state-validation.md)。raw中失败、临时诊断和最终运行分开命名。原业务用户diff仅在本地只读审计，不纳入SDK或本目录。

| 证据 | 入口 |
| --- | --- |
| 完整当前矩阵／全部原输出 | [JUnit](raw/matrix.xml)、[WGL](raw/matrix-wgl-raw.log)、[provider](raw/matrix-provider-raw.log)、[Runtime](raw/matrix-runtime-raw.log) |
| 两个产品缺陷的红例 | [稳定列对齐](raw/reordered-alignment-red.log)、[真实连续拖动](raw/repeat-header-isolation-trace.log)、[首次套件失败](raw/state-isolation-final.log) |
| 完整状态实际用例 | [Light](raw/state-light-restore-96.log)、[Runtime](raw/state-webview2-restore-96.log)；同前缀含初始化、隔离、异常和重排列case |
| 依赖／产物／图片身份 | [产物](artifact-identity.json)、[固定依赖与当前Runtime](raw/dependencies-final.json)、[图片SHA256与尺寸](screenshots.json) |
| 只读业务原失败 | [29/30原日志](raw/business-current-full.log)、[JUnit](raw/business-current.xml)；最终重建与定位观察另列final文件 |

## 同一完整应用的真实保存与重启恢复

截图均来自实际宿主／DLL，窗口1200×800逻辑像素，96／144／192程序DPI，100k行／64列，浅主题；不作为物理DPI验收。before为默认Name200／树展开；saved和restored为真实鼠标Name280／树折叠。三个阶段数据与窗口条件相同，恢复截图来自新的进程。

| 后端／程序DPI | 初始 | 保存后 | 独立重启恢复 |
| --- | --- | --- | --- |
| Light96 | [初始](images/state-light-96-before.png) | [保存](images/state-light-96-saved.png) | [恢复](images/state-light-96-restored.png) |
| Light144 | [初始](images/state-light-144-before.png) | [保存](images/state-light-144-saved.png) | [恢复](images/state-light-144-restored.png) |
| Light192 | [初始](images/state-light-192-before.png) | [保存](images/state-light-192-saved.png) | [恢复](images/state-light-192-restored.png) |
| Runtime96 | [初始](images/state-webview2-96-before.png) | [保存](images/state-webview2-96-saved.png) | [恢复](images/state-webview2-96-restored.png) |
| Runtime144 | [初始](images/state-webview2-144-before.png) | [保存](images/state-webview2-144-saved.png) | [恢复](images/state-webview2-144-restored.png) |
| Runtime192 | [初始](images/state-webview2-192-before.png) | [保存](images/state-webview2-192-saved.png) | [恢复](images/state-webview2-192-restored.png) |

正常restore按case标签保存，reordered-columns另存，不能覆盖上表截图。原重排列错位图作为失败证据单列。当前业务浅／深／AI展开图是新只读副本观察，不冒充本轮原业务修改前截图。

同一Light／96 DPI／1200×800／浅主题／Name280的列定义重排对照：[修复前错位](images/reordered-light-96-before-fix.png)、[修复后按ID对齐](images/state-light-96-reordered-fixed.png)。前图原先误标为正常restore，现明确为reorder失败证据；不改变像素。真实Runtime对应[修复后重排](images/state-webview2-96-reordered-fixed.png)另列。

当前只读业务完整GL观察：[浅色](images/business-observation-final-business-light.png)、[深色](images/business-observation-final-business-dark.png)、[AI展开](images/business-observation-final-business-ai.png)。

交付SDK重新链接后的同一只读业务副本：[浅色](images/business-observation-sdk-business-light.png)、[深色](images/business-observation-sdk-business-dark.png)、[AI展开](images/business-observation-sdk-business-ai.png)，与源码配置观察分开保存。

## 新SDK解包后的实际调用

[归档／逐文件验证](raw/sdk-archive.json)和[8个独立进程桌面结果](raw/sdk-extracted-state-manifest.json)绑定源码文档快照ace533a、产品f0b161c、示例测试d24cdf3。以下补图来自该归档的新解包目录，96程序DPI／1200×800／浅主题／同100k数据；真实鼠标调宽与保存、独立重启恢复均来自实际完整DLL，不重复借用打包前截图。

| 解包后端 | 注册默认 | 实际调宽／折叠并保存 | 新进程恢复 |
| --- | --- | --- | --- |
| Light | [初始](images/sdk-extracted-light-96-before.png) | [保存](images/sdk-extracted-light-96-saved.png) | [恢复](images/sdk-extracted-light-96-restored.png) |
| Runtime | [初始](images/sdk-extracted-webview2-96-before.png) | [保存](images/sdk-extracted-webview2-96-saved.png) | [恢复](images/sdk-extracted-webview2-96-restored.png) |

本目录现有36张实际过程图，失败错位图仍单列；图片身份在[screenshots.json](screenshots.json)。外部归档证明不放回SDK源码快照，以免哈希自引用。
