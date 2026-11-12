# BR-2发送前本地计量组件

该纯计算组件将完整Responses JSON经固定DeepSeek Recipe转换为V4.1 Conversation，渲染并编码全部实际input。它不执行模型、工具或网络请求，不获取凭据。仅识别deepseek-flash对应DeepSeek-V4.1-Flash-1017、非思考、纯文本、4096总输出上限和既有七字段请求格式。

## 构建与固定依据

源码取自同仓库third_party/deepseek_recipe，UPSTREAM.json记录官方归档和文件哈希。Rust源文件保持原字节，工作区只选择四个文本组件，tokenizers固定0.23.2并关闭HTTP/default features。独立Cargo.lock锁定传递依赖，原Lexbor、QuickJS、SQLite和yyjson保持原身份。

在已准备匹配Rust工具链和锁定依赖的环境中构建。这里仅提供命令，未来开发需沿当轮授权执行。

```powershell
cargo build --manifest-path tools/br2_meter/Cargo.toml --release --locked --offline
```

生产产物命名为xuanshu_br2_meter.exe，与包含Service的生产模块同目录。CMake的UI_BR2_METER_PROGRAM显式绑定其文件，配置时复制并把SHA256编入运行时。没有组件时普通框架仍可构建，新生产BR-2策略保持锁停。

fixture feature仅构建独立隔离测试组件，额外识别offline-fixture。对应UI_BR2_METER_FIXTURE_PROGRAM和xuanshu_br2_meter_fixture.exe仅用于UI_AI_FIXTURE运行时。生产组件拒绝该模型，生产DLL不接受fixture_legacy_meter。

## 协议与生命周期

stdin仅接收无认证头的UTF-8请求JSON，最多131072字节。成功输出包含完整body SHA256、profile、profile.json依据哈希、input token数、4096输出上限及进程提交内存峰值。stdout最多1024字节，成功退出0，协议或资源失败退出2。父worker只在真实退出、组件文件哈希及全部receipt关联校验成功后采用结果。

组件启动时自行建立128MiB提交内存限制，禁用tokenizer并行，不继承宿主环境或无关句柄。该新增组件预算单列，Service原8MiB、模型128KiB和其余冻结限额保持。父进程排空输出并等待真实退出，不设置kill-on-close或强制结束。计算在途取消/关闭先保留worker及任务引用，再按原关闭合同退栈。

计量完成后交回UI线程重新核对固定目标和配置。随后每轮仍经过原持久化额度占用及传输前检查，计量成功不会签发确认资格。缺少profile、依据不匹配、组件失败、超出input上界或异常usage均停止后继。

## 证据边界

官方模型映射、协议和价格仍须在未来真实验证前重新核对，授权包保持Proposed。该组件提供已固定编码版本下的程序计量，不能保证远端计费实现永远不变。真实usage超过计量上界时保留unknown并停止后继，不将unknown记为0，也不承诺服务端绝对扣费上限。

官方依据见先前编码准备记录和本轮增量合同，实际交付身份、离线结果及首败按新交付清单分别核对。[编码准备记录](../../docs/validation/xuanshu-br2-encoding-preparation-validation.md) [计量增量合同](../../docs/xuanshu-br2-meter-contract.md)
