# 0.8 → 0.9：应用拥有的实例语言

当前为SDK0.9.0-dev / API9 / 开发标准9，应用ABI1、`ui_app_query_v1`及包格式1。新增公开语言与文本更新能力，因此升级API；不是把UTF-8显示支持当作语言切换。只维护当前版本，历史SDK/旧包没有兼容专项。

本指南描述语言能力首次从API8升级的步骤。当前源码已实现API9；重复接手同一需求时先按[当前源码复核](validation/instance-language-current-validation.md)核对并复验，不重复增加接口或无故升级API。

1. 锁定维护方提供的确切commit，使用同一SDK的include、导入库、共享DLL和宿主。清单 `framework_api_version=9`，DLL描述使用 `UI_FRAMEWORK_API_VERSION`；不要仅改清单而继续使用旧DLL。
2. include `language.h`，在create及首次可见呈现前读取应用自己的偏好，提交 `zh-CN` 或 `en-US`。未提交host使用Windows UI默认；应用可以明确选择自己的缺省语言，不由框架替应用保存。
3. 注册每host回调，以稳定ID调用 `ui_host_set_title` 和组件标题/列标题/字段文本更新接口。不重注册，不改变菜单路径、命令、列或面板身份。初始注册同样使用应用资源；回调注册不自动重放一次。
4. 枚举显示文本使用 `ui_field_text_t.option_labels`，保持原语义options和值。保存文件使用稳定用户/档案身份，不使用本次instance_id。完整示例UIL1与原UST1分开保存，已有布局/列宽文件不改格式。
5. unmount移除回调，遵守UI线程、dispatch、异步关闭及DLL卸载合同。应用结果、业务数据、草稿和路径不由框架翻译。

既有结构字段偏移、枚举和API8列宽/帮助能力保持；新结构必须设置完整size。没有新增系统依赖、进程setlocale或框架公共存储。完整合同、系统变体映射、菜单/模态策略与示例见 [实例语言](instance-language.md)。

宿主跟随活动应用，后台变更隔离，空宿主恢复Windows显示语言。Windows系统对话框仍遵循系统语言。回归、实际失败与待验见 [本轮验收](validation/instance-language-validation.md)，历史 [API8记录](validation/api8-stability-state-validation.md)不作为本轮通过证据。当前API9开发源码入口为[main](https://github.com/wbycloud/ui-framework/tree/main)，接入仍须锁定确切commit；源码同步不等于稳定SDK发布，历史v0.1.0不能代替当前接口。
