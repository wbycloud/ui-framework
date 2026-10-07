from pathlib import Path

root = Path.cwd().resolve()
def replace(file, changes):
    path = root / file
    text = path.read_text(encoding='utf-8')
    for old, new in changes:
        assert old in text, (file, old)
        text = text.replace(old, new)
    path.write_text(text, encoding='utf-8')

replace('README.md', [
 ('[本轮完整窗口真实截图/回归](docs/validation/visual-polish-validation.md)', '[本轮完整窗口真实截图/回归](docs/validation/component-experience-validation.md)'),
 ('本轮[交付收敛与CI审计]', '此前API7[交付收敛与CI审计]'),
 ('[0.6 → 0.7 迁移](docs/migration-v0.6-to-v0.7.md)：当前API7；', '[0.7 → 0.8 迁移](docs/migration-v0.7-to-v0.8.md)：当前API8；[0.6 → 0.7](docs/migration-v0.6-to-v0.7.md)：此前滚动/停靠/颜色；'),
 ('当前SDK/API7', '当前SDK/API8'), ('匹配的API7头文件', '匹配的API8头文件'),
 ('当前见[API7记录](docs/validation/api7-validation.md)', '当前见[API8记录](docs/validation/component-experience-validation.md)'),
 ('当前源码及限制见[API7验收](docs/validation/api7-validation.md)', '当前源码及限制见[API8验收](docs/validation/component-experience-validation.md)'),
])
replace('docs/generic-web-ui.md', [
 ('当前见[API7验收](validation/api7-validation.md)', '当前见[API8验收](validation/component-experience-validation.md)'),
 ('属性名称保留独立、外观平面的可聚焦入口，Tab可到名称/值/有帮助的字段，F1查看名称或完整值；选择器按钮F1查看属性名，选项F1查看完整选项。', '属性名称是展示标签，可单击查看完整名称，不增加Tab停靠点；原输入Ctrl+F1查看名称、F1查看值，选择器按钮F1查看属性名，选项F1查看完整选项。'),
])
replace('docs/workspace-layout.md', [
 ('只覆盖接手确认的API7', '只覆盖接手确认的API8'),
 ('当前交付来源、CI与环境限制见[收敛验收](validation/api7-delivery-validation.md)', '当前交付来源、CI与环境限制见[体验验收](validation/component-experience-validation.md)；此前API7收敛记录保持原版本和结果'),
])
replace('docs/framework-menu-offscreen.md', [
 ('当前API7源码仍需单独复验', '当前API8源码仍需单独服务复验'),
 ('当前GL/Session0与整机无登录分开记于[API7验收](validation/api7-validation.md)', '当前GL/Session0与整机无登录分开记于[API8验收](validation/component-experience-validation.md)'),
])
replace('docs/application-development-standard.md', [
 ('当前版本为API7/SDK0.7/标准7', '当前版本为API8/SDK0.8/标准8'),
 ('当前独立DLL证据见[API7验收](validation/api7-validation.md)', '当前独立DLL证据见[API8验收](validation/component-experience-validation.md)'),
])
replace('docs/build-and-validation.md', [
 ('当前SDK/API7', '当前SDK/API8'), ('## 当前 API7 Windows 回归矩阵', '## 当前 API8 Windows 回归矩阵'),
 ('maintained_api=7', 'maintained_api=8'), ('当前SDK、清单及DLL API7', '当前SDK、清单及DLL API8'),
 ('当前Session0对照包也声明API7', '当前Session0对照包也声明API8'),
 ('本轮当前版本政策及[分页清理实测]', '此前当前版本政策及[分页清理实测]'),
])
replace('docs/handoff.md', [
 ('当前API7原生嵌入式接口', '当前API8原生嵌入式接口'),
 ('本轮API7收敛没有新的推送或远程执行授权', '本轮API8交付没有新的推送或远程执行授权'),
 ('当前已重建API7，最终配置及身份见第10节', '此前重建为API7的身份见第10节；当前API8使用独立build/experience-20261007/after，身份见第17节'),
])

replace('docs/validation/component-experience-validation.md', [
 ('均为本地提交。没有代理', '测试消息泵/宿主就绪收敛0fe08640c3008874449dce77fd7445ef91f6a578；产品二进制仍对应8078281，最终文档提交不改变运行代码。均为本地提交。没有代理'),
 ('FINAL_RESULTS_PENDING', '''| 当前实际执行 | 结果与原始证据 |
| --- | --- |
| 完整框架，Light + 真实Runtime + WGL/OSMesa/MSAA/旧非MSAA，串行 | **55通过、1物理条件跳过、0失败**，202.67秒；[最终日志](component-experience-evidence-20261007/raw/framework-delivery-final.log)、[JUnit](component-experience-evidence-20261007/raw/framework-delivery-final.xml)。包含十万行/64列、列宽/UCW1/列定义变更、完整范围滚动、树/表单、双实例、模态、GL、REFUSE/WAIT、异步关闭与原资源断言 |
| 单独无Web后端的原生配置 | **20通过、1物理跳过**；[日志](component-experience-evidence-20261007/raw/native-8078281.log) |
| 当前业务abc8源码副本完整原测试 | **29通过、1失败 kc_host**，285.96秒；[原日志](component-experience-evidence-20261007/raw/business-8078281.log)、[JUnit](component-experience-evidence-20261007/raw/business-8078281.xml)。旧固定坐标没有命中新折叠入口，断言不改，不能报告30/30或完整业务验收 |
| 框架自有业务鼠标/布局观察器 | **102检查通过**；[日志](component-experience-evidence-20261007/raw/business-real-scroll-pixels.log)。真实SendInput拖末项、真实滚轮至first3后拖回首、64列未缓存末列查询并显示、重复拖动、resize、切焦点、公共presentation折叠/恢复、GL保留和独立进程布局档案；不冒充原业务测试或修改授权 |
| 实际WebView2重开 | **2个独立进程，各32个Runtime呈现就绪周期，0失败**；[运行1](component-experience-evidence-20261007/raw/runtime32-1.log)、[运行2](component-experience-evidence-20261007/raw/runtime32-2.log)。各句柄391→393，GDI12→12、USER17→17、pending cleanup0，保留原门槛；日志末尾历史固定文字“reopen8”不代表实际周期数，逐行reopen记录为32 |
| 匹配SDK消费及离线重建 | 公共C头文件/导入库/DLL列宽160与UCW1 120字节通过；[消费日志](component-experience-evidence-20261007/raw/sdk-consumer.log)。SDK自带依赖离线重建C11公共头/列宽/属性用例，**3/3通过**；[构建](component-experience-evidence-20261007/raw/sdk-offline-build.log)、[测试](component-experience-evidence-20261007/raw/sdk-offline-tests.log) |
| CI本地可复验性检查 | [现有四行分类/测试选择检查通过](component-experience-evidence-20261007/raw/ci-contract-final.log)；未推送、未触发远程工作流。上述本机结果不是托管CI四行运行证据 |

最终冻结产物身份见[artifact-identity.json](component-experience-evidence-20261007/artifact-identity.json)及[delivery-frozen-hashes.json](component-experience-evidence-20261007/raw/delivery-frozen-hashes.json)。8078281的两次中间完整运行均54通过/1宿主超时/1物理跳过，保留原日志。诊断一次取出3001条消息、1983条WM_PAINT、16.2秒，队列不必排空；旧消息泵在PeekMessage取出后才检查条数，且仅限制数量。0fe0864将条数/100ms实际耗时检查移到取消息前，确保取出的消息均Dispatch，不增加睡眠、不延长30秒或放宽断言。活动实例切换后显式layout及公共flush确认就绪；原一次命令断言保留。独立3次通过后再完整矩阵通过。持续重绘的来源尚未定位，不能把消息泵修正写成产品重绘根治；[失败诊断](component-experience-evidence-20261007/raw/features-pump-red.log)、[中间一次命令失败](component-experience-evidence-20261007/raw/features-pump-green.log)、[3次通过](component-experience-evidence-20261007/raw/features-pump-ready-green.log)均保留。

末列63为STYLE，不应伪造文字。观察器验证稳定行ID241、view-local非空clip(8,76,36,32)、实际STYLE内容，并捕获实际显示像素225×323，内部非均匀像素620；日志记录末列请求。原生框架用例另核对文本列稳定ID、文本及非空可见区域，程序DPI96/144/192在完整矩阵内通过。'''),
 ('SCREENSHOTS_PENDING', '''[截图索引](component-experience-evidence-20261007/README.md)及[截图SHA256/尺寸](component-experience-evidence-20261007/screenshots.json)包含105张无损PNG：40张成功修改前基线、57张当前实际截图、8张受限Runtime失败过程图。失败图明确标为failed-restricted-runtime-capture，不能用于成功对照。当前业务只读副本为本轮新增观察，未伪造其本轮修改前截图；真正同应用/数据/1200×800/96DPI/主题的前后对照使用完整框架独立DLL/.uapp（100k/64列、树、属性、异步图片与实际OSMesa GL）。

| 同一真实完整窗口 | 修改前 | 修改后 |
| --- | --- | --- |
| Light浅色 | [之前](component-experience-evidence-20261007/screenshots/before-light-host-light.png) | [之后](component-experience-evidence-20261007/screenshots/after-light-host-light.png) |
| Light深色 | [之前](component-experience-evidence-20261007/screenshots/before-light-host-dark.png) | [之后](component-experience-evidence-20261007/screenshots/after-light-host-dark.png) |
| 实际Runtime浅色 | [之前](component-experience-evidence-20261007/screenshots/before-runtime-desktop-host-light.png) | [之后](component-experience-evidence-20261007/screenshots/after-webview2-host-light.png) |
| 实际Runtime深色 | [之前](component-experience-evidence-20261007/screenshots/before-runtime-desktop-host-dark.png) | [之后](component-experience-evidence-20261007/screenshots/after-webview2-host-dark.png) |

显式列宽[拖动](component-experience-evidence-20261007/screenshots/after-light-columns-resized.png)、[适配/重置入口](component-experience-evidence-20261007/screenshots/after-light-columns-tools.png)、[缓存适配](component-experience-evidence-20261007/screenshots/after-light-columns-fit.png)，以及[完整文本](component-experience-evidence-20261007/screenshots/after-light-complete-text.png)、[长枚举](component-experience-evidence-20261007/screenshots/after-light-enum.png)、[RGBA](component-experience-evidence-20261007/screenshots/after-light-color-picker.png)、[工作区](component-experience-evidence-20261007/screenshots/after-light-workspace-details.png)、[窄窗](component-experience-evidence-20261007/screenshots/after-light-narrow.png)、[当前业务完整GL](component-experience-evidence-20261007/screenshots/after-current-business-light.png)另列实图；Runtime对应图和96/144/192、AI展开/收起在索引内。默认初始列宽未变，所以普通窗口对照不会自动显示适配后的宽度。

捕获使用真实宿主/组件/应用DLL的PrintWindow/桌面BitBlt，PNG仅无损RGB转换，不生成图片或静态HTML效果图。观察源/构建cmd/执行日志均交付；自动输入与截图只覆盖对应程序条件。'''),
 ('当前源码服务Session0权限、严格整机无登录服务runner、托管CI执行授权、真实中文IME、不同缩放物理屏幕、屏幕边缘桌面合成/Snap及长期人工分别检查。', '环境审计为Windows交互Session1、非管理员，申请SC_MANAGER_CREATE_SERVICE失败，准确Win32错误5（拒绝访问）；本轮没有创建/修改服务或注销用户。当前源码服务Session0待授权，严格整机无登录服务runner缺失，托管CI无执行授权；真实中文IME、不同缩放物理屏幕、屏幕边缘桌面合成/Snap及长期人工缺相应条件。见[审计](component-experience-evidence-20261007/raw/environment-final.json)。'),
])

with (root/'docs/handoff.md').open('a', encoding='utf-8') as f:
    f.write('\n最终产品8078281、测试收敛0fe0864：完整框架55通过/1物理跳过，原生20通过/1物理跳过；只读abc8业务副本29通过/1原固定坐标失败，框架自有真实业务观察102检查通过（末列STYLE实际像素），Runtime两次独立32周期通过。持续重绘来源未定位，测试消息泵修正不代表根治；原失败均保留。交付匹配SDK include/lib/bin/source及固定依赖、105张实际过程截图（8张失败图不作基线）；源码Session0服务权限错误5、无登录runner、远程CI与物理/人工分别待验。SDK归档位于本地build/experience-20261007，解包逐项sha256验证；最终文档提交不改变已冻结运行产物。\n')
with (root/'CHANGELOG.md').open('r', encoding='utf-8') as f:
    text = f.read()
text = text.replace('## 0.7.0 —', '- 原生/Light/真实Runtime/GL回归及匹配SDK交付，保留业务原固定坐标失败。测试宿主消息泵先检查时间/数量再取消息、切实例后确认layout/flush就绪，不放宽原30秒和一次命令断言；持续重绘来源仍待定位。\n\n## 0.7.0 —', 1)
(root/'CHANGELOG.md').write_text(text, encoding='utf-8')
print('Current entry documentation and acceptance finalized')
