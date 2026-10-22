# 当前版本维护与验收政策

本轮接手确认的当前版本为 **SDK0.9.0-dev / API9 / 标准9**。从共同组件分页入口清理这一轮起，后续每轮只维护和验收接手时经源码、HEAD及公共头文件确认的当前版本。本政策取代入口文档和历史迁移指南中持续保证旧版兼容的承诺。

## 维护范围

- 当前公共接口、完整size字段读取、字段偏移、枚举、线程、所有权、数据源和DLL卸载合同继续受维护。基础功能最早在API1–6引入，不影响它们在当前API9中的维护。
- 历史SDK/API、旧调用方及旧二进制应用包不再属于兼容保证、专项回归或交付门槛。当前CI不编译冻结SDK调用方，不注册旧API集成/原包专项，不下载或用新包替代原包。
- 历史冻结SDK、测试源码、原包及验收记录保留原样供追溯；历史成功和失败只描述当时commit与环境，不作当前结论。
- 当前运行库仍接受API1–9声明及部分短size描述，这是现存行为；本轮未清理这些分支，也不承诺今后持续可加载。升级到当前维护范围应使用匹配的头文件、导入库、运行库及清单/DLL声明，实际验证API9应用。

## 当前组件与版本

共同TREE/TABLE/LIST只使用完整范围横纵滚动条导航，移除上一页/下一页及列箭头按钮，回收操作栏空间。FORM/DIALOG提交和取消仍保留；菜单、最近应用和标签溢出列表的分页入口不在清理范围。

底层虚拟窗口和按需批次仍存在：应用返回真实total_count，按first/count交付，不预加载全数据。预算、BigInt/uint64映射、稳定ID、输入、草稿及生命周期不变。当时已符合API7数据源合同的应用无需因分页清理修改或重编译；此前API8公开能力需要匹配SDK重建，数据源仍遵守原合同。

分页清理阶段没有公共头文件、ABI或包格式变化，原SDK/API/标准为0.7/7/7。此前列宽公开能力及help字段使SDK/API/标准升为0.8/8/8；ABI和包格式仍1。将来公共合同如需变化，按仓库版本规则处理；内部控件删除不成为ABI升级理由。

## 验收与交付

四行native/light/webview2/osmesa继续验证当前功能，真实Runtime及实际GL必须运行；物理条件跳过、Session0、严格整机无登录、人工与业务证据分别报告。CI的compatibility-policy.json记录current_only，历史测试若误入必跑清单会被拒绝。没有远程授权只交付本地commit及本地证据。

历史分页清理结果见[原验收](validation/component-scroll-only-validation.md)；当前实现、失败、截图和实际矩阵见[API9语言验收](validation/instance-language-validation.md)；新环境命令见[构建与CI](build-and-validation.md)。


## API9 版本增补

语言设置、来源查询、通知及原位文本接口使SDK/API/标准集中升级为0.9/9/9，应用ABI及包格式仍1。现存低版本加载分支暂存，不代表重新维护历史包；当前接入使用匹配API9产物。[迁移](migration-v0.8-to-v0.9.md)。

## 2026-10-09 开发分支布局增补

本轮在SDK0.9.0-dev/API9/标准9追加独立的对话框/多行布局描述及三个导出，不改既有结构、枚举、ABI1或包格式1，不覆盖冻结SDK。冻结API9头文件、导入库和DLL不含这些新导出；新增能力必须使用同一开发提交的匹配产物，API数字相同不代表不同开发快照可互换。当前完整size和偏移检查、新C调用方链接、导出及Light/Runtime回归单独验证；这是未发布开发分支的增补，不宣布新的稳定SDK或历史二进制兼容承诺。[公共合同](dialog-layout.md)、[本轮证据](validation/ui-repair-validation.md)。

## 2026-10-17 开发分支标题与主题增补

本轮以独立 `theme.h` 结构和六个新导出追加实例标题 set/get、每 host 主题 set/get/callback/refresh，保持既有结构偏移、SDK/API/标准0.9-dev/9/9及ABI/包1。新能力使用同一工作树源码快照和匹配产物；用户禁止提交，因此交付标识采用基准HEAD加源码清单SHA，不虚构新commit。冻结SDK不修改。本轮按用户明确要求追加冻结API9调用方定向验证，不恢复API1–8历史专项或改变current_only CI。见[公共契约](instance-title-and-theme.md)及[本轮验证](validation/interaction-20261017-validation.md)。


## 2026-10-20 受限文档与内置浏览器开发增补

本轮追加独立 light_document.h 与 app_data.h 及对应导出，不改变既有公开结构、SDK/API/标准0.9-dev/9/9、ABI1或包格式1，不覆盖冻结SDK。新受限文档后端永久关闭脚本与应用桥；原可信Light和可选WebView2路径保留。应用复制数据投递复用既有每实例8MiB预算。

browser.uapp 以普通应用机制作为框架自带包；宿主启动不打开它、不请求网页。网络、阅读历史和工具栏在 apps/browser；宿主只定位同目录包并调用 workspace。交付必须匹配同一源码快照的头文件、导入库、DLL和包。HEAD仍为接手基准，当前工作树包含既有与本轮未提交修改；不虚构提交。详见[公共合同与支持表](restricted-browser.md)及 apps/browser/docs/validation.md。

## 2026-10-21 M2受限阅读语义增补

列表编号、合并表格、可见文本索引、基础CSS及结构化GET在现有light_document公共函数内补齐；没有新增导出或修改任何既有公开结构布局。SDK/API/标准继续0.9-dev/9/9、ABI及包格式1，冻结SDK保持原样；本批以实际工作树源码SHA与匹配头文件/库/DLL交付，不虚构commit。受限后端仍永久无脚本和应用桥，沿用原资源门槛。实际矩阵、失败及独立交付见apps/browser/docs/m2-support.md与m2-validation.md。
