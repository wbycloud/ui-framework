# Web Counter能力接入示例

当前示例使用SDK 0.10开发版、API10、ABI1及包1。每实例保存自己的count和revision，Light普通按钮与统一能力目录复用`counter_run`。应用通过`p2_support.h`复用框架维护示例的注册、UI绑定和P4事务适配，外部应用以公共头为接口边界。[应用源码](app.c)、[示例适配](../p2_support.h)、[接入指南](../../docs/xuanshu-general-assistant-integration.md)

| 资源/能力 | 参数与结果 | 业务及权限 |
|---|---|---|
| counter.document | revision为counter-N | 稳定实例资源，由应用提供实际版本 |
| counter.read | 输入{}，输出count/revision | v1 read及C1生成只读入口 |
| counter.increment | 输入{}，输出count/revision | 明确写操作，普通按钮沿C2可信输入绑定，P4捕获/提交/回读/撤销 |

通用专业角色application.specialist由同一注册适配按实际能力生成。模型范围与角色能力取交集，按钮动作、Profile文本和source字符串分别遵循其来源合同。它展示实际计数业务，不代替任意业务应用验收。

## 构建与运行

在仓库根准备固定Light依赖，并使用匹配头、导入库、运行库和宿主。

```powershell
cmake -S . -B build/counter -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_BUILD_TESTS=OFF
cmake --build build/counter --target framework_host web_counter_package
./build/counter/framework_host.exe ./build/counter/web_counter.uapp
```

同包允许多实例，重复打开产生独立count。正式助手可按目录选择目标；本地保存和模型外发遵循独立确认。目录查询示例无需模型配置。[公共目录示例](../general_assistant/README.md)

## 生命周期与验证范围

create分配模型并注册能力，mount取得内容槽、加载页面和绑定按钮，unmount销毁view/backend，destroy在安全卸载边界释放能力适配和私有状态。关闭沿原框架回调/租约门，未完成调用保留其记录。

本轮静态核对清单、目标名及公共API。既有P4/P5计数器读回、事务及撤销属于对应开发证据，当前设备来源和应用特定验证须按实际环境记录。[P4/P5既有覆盖](../../docs/xuanshu-p5-application-coverage.md)、[构建要求](../../docs/build-and-validation.md)
