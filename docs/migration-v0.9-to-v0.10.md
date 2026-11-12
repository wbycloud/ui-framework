# 0.9 → 0.10及同API10后续接入

2026-11-12。当前开发源码为SDK 0.10.0开发版、API10、标准10，应用ABI1、包格式1。迁移材料应记录确切commit与工作区/文件清单，配套取得公共头、导入库、运行库、宿主及应用包。当前匹配快照见开发入口。[匹配开发材料](xuanshu-general-assistant-integration.md#匹配的开发材料)

## 从API9应用迁移

1. 核对当前`UI_FRAMEWORK_API_VERSION=10`、CMake 0.10.0和清单/DLL descriptor。使用同一快照的头及ui_framework_runtime.lib重建应用，共享宿主同一ui_framework.dll。
2. 保留既有UI、文档、语言、列宽、DPI及关闭合同。登记真实资源id、open_epoch及revision，按Schema注册能力。v1 read使用capability.h的bind/prepare/dispatch/complete/get_result/forget。
3. 需要菜单、表单、Light输入或共同组件绑定时使用相应C2独立头。受控事务使用capability_edit.h；专业Profile和有限计划使用ai_coordinator.h。这些是API10内的独立扩展。
4. 普通APP在正式宿主中复用既有Service/Provider/确认/存储。实现独立宿主时按接入指南启用Service与通用助手，处理持久化接纳、完整预览、初始/有限计划确认和安全关闭。
5. 按实际接口范围验证应用的真实业务值、revision、事务/撤销、双实例、异步取消和DLL卸载，记录已验及未验条件。

具体公共接口、所有权和最小示例由接入指南统一索引。[应用能力合同](application-capability-contract.md)、[接入指南](xuanshu-general-assistant-integration.md)、[最小目录示例](../examples/general_assistant/README.md)

## 已有API10应用采用后续扩展

早期API10 SDK只含v1 read层。后续增加C1/C2、P4、P5、Service、模型及通用助手等独立结构和导出，公开API主版本保持10。采用新增接口时核对具体头和生产导出，并成套更新匹配导入库、DLL和宿主。旧冻结SDK保留原字节与原覆盖，当前包头版本数值相同不代表二进制可混用。

v1结构仍只接受read效果及固定read scope。C2/v2处理原人工UI来源及UNKNOWN，P4提供受控事务/撤销，P5及通用助手在同一目录和业务执行器上工作。received、持久化接纳、确认、发送、终态和最终保存分别处理，恢复只展示记录。接口及应用矩阵见当前能力合同。

## 首批A+B历史范围（2026-10-22）

当时只新增capability.h及严格JSON/Schema/JCS最小读链，未包含AI Service、写入事务或五维护示例完整迁移。原只读结构、调用ID、引用和错误合同继续适用于v1层。该阶段结果及未完成事项按原日期保留。[首批验收](validation/xuanshu-p2ab-validation.md)

后续C1/C2、P3—P6、browser及通用助手在API10内逐项接入。五维护示例现已有真实资源/能力/UI绑定及相应事务，browser有snapshot/body读取。browser九项副作用和完整EDA等仍按当前支持矩阵安排。历史旧版本加载分支按版本政策解释。[版本政策](version-policy.md)、[当前支持矩阵](xuanshu-general-assistant-integration.md#当前支持及选择粒度)
