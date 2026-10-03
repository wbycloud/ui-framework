# 原测试合同快照

保存实施基准 `3375459ca2d85d0627e3072e3f1e83fc4fa84aae` 的五个源文件，便于对照有意变化。此目录不加入当前 CTest，不通过删除历史断言掩盖行为改变。

| 文件 | 当前测试调整原因 |
| --- | --- |
| application_versions.c | 新运行库接受API3，未知版本改用4 |
| package_format.c | API3清单合法，清单/DLL仍须一致 |
| light_web_dynamic.c | EDIT被移除，验证Web Unicode/焦点/文本传输 |
| probe.c | class与1024节点已是公开子集，193节点拒绝不再适用 |
| web_host_frontend.c | 工具使用稳定注册ID，文本直接走Web，禁止原生编辑代理 |

对应的新测试仍验证非法输入、资源预算、UTF-8及兼容性，完整结果见[API3验收](../../docs/validation/api3-validation.md)。API1/2头文件分别在 sdk_v1/sdk_v2；二进制原包保存在本地验证产物中，未作为源码上传。
