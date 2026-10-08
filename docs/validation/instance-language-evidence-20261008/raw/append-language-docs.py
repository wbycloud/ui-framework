from pathlib import Path
sections={
 'README.md':('API9 应用实例语言','应用决定并持久化语言，每个host独立支持zh-CN/en-US。宿主跟随活动实例，无应用时跟随Windows显示语言。新增设置、来源查询、通知及原位文本更新能力，使用匹配SDK0.9/API9；不重建DLL、工作区或GL。完整示例通过A/B档案演示语言与原列宽/布局状态的独立持久化。[语言合同](docs/instance-language.md)、[当前验收](docs/validation/instance-language-validation.md)、[完整示例](examples/stateful_components/README.md)。'),
 'docs/application-development-standard.md':('API9 应用语言与文本','标准9要求应用拥有语言选择及存储，在create、首次呈现前向本host提交。不得用setlocale、全局环境变量或浏览器语言代替实例状态。宿主活动跟随、空宿主Windows UI默认、通知重入、完整size/复制所有权和原位更新见[语言合同](instance-language.md)。命令、菜单path、列和面板ID稳定；枚举标签与提交值分开，翻译不自动适配用户列宽。应用unmount移除回调，保持原异步关闭和卸载。新增公开能力集中升级API9，ABI1/包1不变；API8列宽/help继续维护。'),
 'docs/standalone-host.md':('应用语言跟随（API9）','语言入口在应用，不在宿主全局设置。有应用时读取活动host，后台变更隔离；切换/关闭活动标签更新文案，最后一个关闭后恢复Windows显示语言。最近、空状态、工具提示、固定错误和助手使用框架统一资源。用户路径、参数和已有结果不翻译；助手确认按目标实例打开时快照。Windows文件选择器系统按钮依旧使用系统语言，其应用包过滤标签属于框架。实际识别时机和模态策略见[语言合同](instance-language.md)，[本轮证据](validation/instance-language-validation.md)不继承历史通过。'),
 'docs/generic-web-ui.md':('API9 原位文本与语言','共同模板接收所属host的复制文本字典及精确字符串languageGeneration；两后端共同语义。新接口只更新标题/标签/帮助/选项显示标签，不改草稿、业务值、组件数据generation、缓存、完整范围滚动或列宽。异步tooltip检查语言版本，有效数据批次不因语言切换被抛弃；不重复查询来翻译。选择器暂态关闭并返回入口焦点，应用模态原位更新。详见[语言合同](instance-language.md)和[完整示例](../examples/stateful_components/README.md)。'),
 'docs/workspace-layout.md':('API9 面板文本更新','ui_host_set_title按稳定panel ID更新标题；停靠标签、浮动标题和宿主摘要原位刷新。标题不是持久化身份，语言变化不改变布局格式、分隔条状态、原HWND或GL context。实例自己的弹窗沿用本host语言，浮动工具复用框架字典。语言设置仍由应用存储，见[语言合同](instance-language.md)。'),
 'docs/framework-menu-offscreen.md':('API9 菜单文本更新','菜单组通过稳定path更新显示title；菜单项、命令和工具通过稳定ID更新。翻译不重建另一套命令系统或改助记键。语言切换关闭旧菜单，后续打开读取所属实例最新语言；原Alt、方向键、Esc、禁用项和一次命令合同保持。新公开语言/文本接口使当前版本升级API9，GL/MSAA及无窗口合同不变，见[语言合同](instance-language.md)。'),
 'docs/build-and-validation.md':('API9 语言回归与SDK','当前构建产出API9头文件、导入库、共享DLL、宿主及清单匹配的应用包。ui_language_core覆盖来源、通知、线程、完整size和旧语言异步失效；ui_instance_language_light/webview2使用实际完整DLL、十万行/64列、GL和真实鼠标，覆盖96/144/192及跨进程档案。run-instance-language.ps1先验证10种偏好文件，再write/restore；Runtime必须实际可用。CI沿用四行矩阵并强制这些当前用例，未触发远程工作流。锁屏/共享输入、真实IME、物理跨屏、Session0及严格无登录与本机结果分开报告。[实际命令、失败和SDK校验](validation/instance-language-validation.md)。'),
 'docs/visual-design.md':('API9 中英文呈现','本轮新增实例语言及原位文本公开接口，列宽/help是此前API8能力。框架统一字典配合应用资源；文本改变不覆盖用户宽度或自动适配，长文使用既有截断/完整内容路径。浅深主题、角色样式、紧凑几何和预算保持；实际同尺寸完整应用截图见[API9验收](validation/instance-language-validation.md)。'),
 'docs/version-policy.md':('API9 版本增补','语言设置、来源查询、通知及原位文本接口使SDK/API/标准集中升级为0.9/9/9，应用ABI及包格式仍1。现存低版本加载分支暂存，不代表重新维护历史包；当前接入使用匹配API9产物。[迁移](migration-v0.8-to-v0.9.md)。'),
}
for name,(title,text) in sections.items():
 p=Path(name);s=p.read_text(encoding='utf-8');p.write_text(s+'\n\n## '+title+'\n\n'+text+'\n',encoding='utf-8')
p=Path('docs/application-upgrade-prompt.md');s=p.read_text(encoding='utf-8');needle='2. 盘点应用SDK/API/ABI';addition='语言接入：先读docs/instance-language.md与完整stateful_components示例。应用自有稳定档案保存语言，在create/首次呈现前提交本host；监听实例通知，用稳定ID更新菜单、面板、组件、表头与字段文本。枚举显示标签不改变语义options。宿主自动跟随活动实例，空宿主使用Windows显示语言；没有全局语言开关。保留草稿、宽度、布局、HWND和GL，不重载DLL。非法/不支持语言明确失败。同步清单/API9与匹配SDK，按语言合同处理回调、线程、复制所有权和卸载。\n\n';assert needle in s;s=s.replace(needle,addition+needle);p.write_text(s,encoding='utf-8')
p=Path('examples/stateful_components/README.md');s=p.read_text(encoding='utf-8');s=s.replace('只创建该目录、`profile-A/B.lock`、`profile-A/B.ust`','只创建该目录、`profile-A/B.lock`、`profile-A/B.ust`、`profile-A/B.ust.language`');s+='''

## 应用拥有的中英文偏好（API9）

菜单 Language / 语言 提供简体中文和English。应用create先按稳定A/B档案读取语言，在注册、mount和首次呈现前提交；没有有效文件时选择本示例的明确英文默认值。框架不为它写偏好。选语言后先提交本host，再由应用保存；保存失败提示、保留运行语言，不谎报持久化成功。

`profile-A/B.ust.language` 是独立应用文件UIL1，不改UST1/UCW1/ULYT。恰好16字节：偏移0为UIL1；4为小端uint32版本1；8为语言1（zh-CN）或2（en-US）；9为档案ASCII A/B；10–15必须零。缺失保持应用英文默认；空、截断、非法、档案不符及未来版本整体拒绝并提示，不覆盖原文件。用户显式选择时，写同目录临时文件、FlushFileBuffers、MoveFileEx(REPLACE_EXISTING|WRITE_THROUGH)。关闭不自动保存语言，已有状态文件不需要迁移。

档案锁与原状态共用，第二实例使用B；用户/根目录也由应用选择。关闭重启选择同一档案可恢复，不能用instance_id给偏好命名。环境变量只选择示例档案和测试目录，不作为框架语言来源。

语言回调更新稳定菜单/命令、面板、组件、列和字段标签/帮助。树行、表格值、用户草稿、路径和已有结果是原业务数据，保持不变。枚举显示Low/Medium/High或低/中/高，提交仍是low/medium/high。无需重新加载DLL、GL或查询全数据；列宽与布局恢复独立进行。unmount移除回调。

真实宿主验证命令：

```powershell
powershell -NoProfile -File tests/run-instance-language.ps1 -BuildDirectory build/current -Provider C:/providers/osmesa.dll -Backend light
# 使用已安装的真实Runtime，将Backend改为webview2。
```

该脚本验证完整DLL的10种偏好文件、真实菜单鼠标、A中文/B英文、反复标签切换、后台隔离、96/144/192、草稿/选择/GL、模态、AI尺寸回收、关闭和跨进程恢复。public input diagnostic模式只供定位，不替代真实鼠标。合同见[实例语言](../../docs/instance-language.md)，原失败及本轮证据见[验收](../../docs/validation/instance-language-validation.md)。
''';p.write_text(s,encoding='utf-8')
p=Path('CHANGELOG.md');s=p.read_text(encoding='utf-8');pos=s.find('\n');s=s[:pos]+'''\n
## 0.9.0-dev / API9 / 标准9（本地未发布）

新增应用拥有的每实例zh-CN/en-US状态、系统显示语言来源查询和同步通知。独立宿主跟随活动实例，空宿主跟随Windows UI语言；后台修改隔离。框架统一文案字典，菜单/工具/面板/组件/列/字段与枚举显示标签可原位更新，不改业务值、列宽、数据generation、HWND或GL。完整示例使用应用自己的UIL1档案偏好，首呈现前提交、A/B隔离、显式保存及异常拒绝。

新增公开能力集中升API9；应用ABI1/包1、C11、原size/所有权/生命周期保持。只维护当前版本，不恢复历史兼容专项；没有新依赖、预算放宽或组件分页按钮。[合同](docs/instance-language.md)、[迁移](docs/migration-v0.8-to-v0.9.md)、[本轮实际测试、失败及待验](docs/validation/instance-language-validation.md)。
''' +s[pos:];p.write_text(s,encoding='utf-8')
