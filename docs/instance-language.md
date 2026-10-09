# 应用拥有的实例语言（API9）

SDK0.9.0-dev / API9 / 开发标准9，Windows x64/C11；应用ABI1、包格式1。语言不是进程或浏览器设置。框架仅保存每个 `ui_host_t` 的运行时状态；偏好、用户身份、档案和持久化由应用负责。实现见[language.h](../include/ui_framework/language.h)，[当前源码复核](validation/instance-language-current-validation.md)和[首次实现验收](validation/instance-language-validation.md)按各自提交、产物及环境分别报告。

## 初始化、来源和通知

应用在 create 阶段、注册与首次可见呈现前读取自己的偏好，然后提交精确的 `zh-CN` 或 `en-US`。未提交的 host 使用创建时识别的 Windows 显示语言，不自动写偏好。应用可以决定自己的缺省值；完整示例选择英文，并明确提交。

```c
#include "ui_framework/language.h"
static void language_changed(ui_host_t *host, const ui_language_info_t *info, void *app)
{
    /* 使用应用文本资源，按稳定ID更新名称、标签和帮助；不重注册组件。 */
    update_application_text(app, host, info->language);
}
/* 应用先读自己的文件。验证失败时由应用决定报错或使用自己的缺省值。 */
ui_host_set_language(host, saved_language);
ui_host_set_language_callback(host, language_changed, app);
/* 注册初始文本时同样使用saved_language；注册回调本身不会发送初始通知。 */
```

| 接口 | 合同 |
| --- | --- |
| `ui_host_set_language` | 显式设置；精确支持两个标签。NULL、空串、畸形标签返回INVALID_ARGUMENT；其他形状合法但不支持的标签返回UNSUPPORTED。失败不修改原状态，也不通知 |
| `ui_host_reset_language` | 撤销运行时显式语言并重新读取Windows默认；不删除应用文件 |
| `ui_host_get_language` | 输出有效语言、来源、单调递增generation、实际系统首选名称、识别提供者及是否回退 |
| `ui_host_get_system_language` | 独立查询当前Windows显示语言；不修改host，查询结果generation为0 |
| `ui_host_set_language_callback` | 每host一个应用回调；NULL移除。变更时同步通知，重复同一状态不通知。有效语言、来源或系统诊断变化均算变更 |

所有接口在所属UI线程调用。关闭保护期间禁止设置语言和新增回调；NULL清除回调仍允许，便于unmount解除DLL函数指针。回调参数为调用期借用；需要保存时复制结构。在回调中允许查询与文本更新，递归设置/重置语言返回CANCELLED。框架进入现有dispatch保护，禁止在回调栈内销毁host或卸载应用DLL；按原延迟关闭合同处理。另向现有workspace观察器发送 `ui.host.language_changed`，generation用十进制字符串。

输出结构必须清零并设置完整 `size`。Windows x64的 `ui_language_info_t` 为128字节，generation偏移8、system_language偏移32、fallback偏移120、system_provider偏移124。枚举值见头文件，不依赖私有结构。所有字符串是复制的UTF-8；查询不返回需要应用释放的指针。

## Windows显示语言及独立宿主

识别顺序是 `GetUserPreferredUILanguages(MUI_LANGUAGE_NAME)` 的第一个名称，再退回 `GetUserDefaultUILanguage`；没有平台UI语言时使用英文。`zh` 家族（包括简体、繁体及地区变体）映射到首轮支持的简体中文；`en` 家族映射英文；其他语言回退英文并设置fallback。此处不查询地区格式、时区或输入法。[Windows首选UI语言文档](https://learn.microsoft.com/en-us/windows/win32/api/winnls/nf-winnls-getuserpreferreduilanguages)、[缺省UI语言文档](https://learn.microsoft.com/windows/win32/api/winnls/nf-winnls-getuserdefaultuilanguage)说明这些系统接口。

应用host在创建和显式reset时识别系统来源。独立宿主在启动、状态刷新和系统设置通知后，没有活动应用时重新查询系统；有应用时读取活动host最新状态。切换、关闭活动实例以及关闭最后一个实例均刷新宿主文案。后台实例修改语言不改当前宿主，也不改其他实例的设置。宿主不提供全局语言开关；入口属于应用示例的Language菜单。

宿主chrome使用独立单调版本，不能直接把两个实例各自的generation互相比较。查询的APPLICATION来源表示应用已提交；WINDOWS_DISPLAY及提供者表示系统来源。Windows文件选择器等系统界面仍使用系统自己的语言，框架不能保证它们跟随应用语言。

## 原位文本更新

框架文案集中在 `src/language_catalog.inc`：宿主工具提示、最近/空状态、固定错误、助手、菜单、组件选择器与校验、工作区及浮动面板操作。应用提供菜单、按钮、表头、属性及业务帮助/错误资源。用户草稿、路径、数据、原始参数和已有结果不自动翻译。

| 接口 | 保留内容 |
| --- | --- |
| `ui_host_set_title(host, target, stable_id, text)` | 命令、菜单项、工具栏/工具项、面板、菜单组；菜单组使用稳定path，不把译文当path |
| `ui_component_set_title` | 组件及已打开对话框的标题，保留对象与原HWND |
| `ui_component_set_column_title` | 按稳定列ID更新表头，不改应用初始宽度或用户宽度 |
| `ui_component_set_field_text` | 标签、单位、分组、帮助及枚举显示标签；不改值、草稿及语义选项 |

单个文本上限4095字节；复制所有传入文本，在返回后应用可释放。元数据占原组件描述预算，预算不足明确失败，不提高预算。`ui_field_text_t` x64大小64、option_labels偏移48；完整size读取。NULL文本表示保留，空串表示清空；NULL标签/count0保留，否则必须与注册选项数完全一致，先验证及分配再原子替换。示例显示“低/中/高”或“Low/Medium/High”，提交的值仍为 `low/medium/high`。

原生菜单、工具和面板标题刷新不重新挂载内容。组件重绘文字及重新测量受支持的布局，不重建工作区、DLL、内容HWND或GL context；表头宽度不自动适配。组件数据generation、请求和缓存不因语言改变而失效。迟到的有效数据继续接收；文案呈现按host及语言版本检查，旧tooltip响应不能覆盖新语言。列宽、完整范围滚动、稳定ID和BigInt合同保持。

## 菜单、选择器和模态

变更关闭所属实例菜单及暂态文字/列工具/选择器，恢复选择器入口焦点，保留草稿与选择。活动实例的语言可在已有应用模态中变更；其标题和内置按钮原位刷新，不重新打开模态。宿主普通暂态入口在活动实例/语言变化时关闭。

助手权限确认使用打开时目标实例的语言快照；后台语言变化不会改写已打开的确认内容。既有权限、目标、事务和命令次数保持。新结果使用产生时的语言；已经显示的业务结果不自动翻译。

## 完整接入示例

[stateful_components](../examples/stateful_components/README.md) 在真实宿主加载DLL，包含树/十万行64列表格、属性、异步图像、实际OSMesa帧、布局与助手。复用A/B稳定档案及独占锁，每档案另存UIL1语言偏好。菜单操作显式保存；启动拒绝损坏文件时不会覆盖它。语言文件和UST1状态文件相互独立，已有UCW1/ULYT格式不迁移。

当前只维护API9。请用匹配头文件、导入库、DLL、宿主和清单重建；[迁移指南](migration-v0.8-to-v0.9.md)列出必须改变和保持的部分。真实鼠标、Runtime、系统环境与可控映射测试分别记录，自动截图不代表真实IME或物理跨屏验收。
