# 在应用中使用和接入代操作

## 使用统一助手

1. 启动匹配的正式宿主，打开样例应用，再从标题栏打开「AI」。在输入框旁选择「代操作」，写明对象与目标。
2. 「发送消息」先保存本地任务。查看完整发送内容并确认后，模型才能根据应用声明提出操作方案。此时未发送实际业务值。
3. 宿主读取固定对象后，在「核对代操作范围」中显示本次业务事实、允许动作、参数来源和限额。核对后选择「授权本次代操作」。执行中可用「接管并停止」。结果按业务提交及独立读回显示。

普通提问沿用聊天入口。一次代操作只使用当前消息及所选应用范围。需要模型配置和有效计价才会进入发送路径。

## SDK 接入

注册现有业务资源、读取能力及可撤销 P4 能力。然后在同一个 `ui_host_t` 上包含 `ui_framework/ai_operation.h`，调用 `ui_ai_operation_register`。应用仍通过原能力实现实际读取、事务和回读。声明按资源组织，可供同一资源上的多个自然语言任务复用。没有宿主名单或中央配置项。

```c
ui_status_t status = ui_ai_operation_register(host,
    "{\"version\":1,\"id\":\"drawing.blocks\",\"title\":\"图块\","
    "\"resource\":\"eda.document\","
    "\"observe\":{\"capability\":\"eda.document.read\","
    "\"fields\":{\"objects\":\"count\"}},"
    "\"actions\":[{\"capability\":\"eda.add_block\",\"identity\":[],\"sources\":{}}],"
    "\"max_actions\":8}");
```

`observe.fields` 中，`value` 允许披露这个标量，`count` 只披露数组长度。动作具有定位参数时，`identity` 要列出完整参数并与读取对象一致。`sources` 按目标参数列出允许引用的观察字段。表格用固定行列定位单元格，允许 `text` 参数引用已核实源格的 `text`。数值、字符串和参数形状沿用应用本身的 Schema，不做隐式转换。[增量合同](xuanshu-operation-tasks-contract.md)

现有 App 可继续使用旧公开接口。接入新声明的 App 需要使用包含这两个新增入口的匹配宿主、DLL、头和导入库。
