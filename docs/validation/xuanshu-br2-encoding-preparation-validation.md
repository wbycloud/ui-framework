# BR-2官方编码链续查与发送前计量缺口

2026-11-06。本轮承接真实验证准备授权，继续查找能够成立的计费依据。原冻结产品、程序、依赖和历史准备材料保持原身份。当前仍停在真实执行及集中授权包提交之前。

## 本轮新增依据

DeepSeek官方DeepInfer说明deepseek-recipe的处理逻辑与其官方API一致。Recipe提供Responses请求到Conversation的转换、V4.1提示词渲染及带tokenizer的token ID编码。由此补齐了上一轮仅检查tokenizer演示ZIP时缺少的生产编码规范来源。[DeepInfer官方说明](https://github.com/deepseek-ai/DeepInfer/blob/main/doc/inference_deployment_qa.md) [Recipe官方组件](https://github.com/deepseek-ai/deepseek-recipe)

必要摘录为“Its processing logic is fully aligned with the official DeepSeek API”。

Recipe的tokenizer说明要求正文按移除added vocabulary后的普通文本编码，显式控制符按special_ranges编码；用户文本中的同名拼写继续作为普通内容。每轮计量需要完整的请求转换、渲染和token化，输出4096与完整input一起检查上下文。[官方tokenizer说明](https://github.com/deepseek-ai/deepseek-recipe/blob/main/docs/tokenizer.md)

## 来源及模型对应

本轮材料位于build/xuanshu-br2-real-preparation-20261106-d83d0e/encoding-f696f7。只读获取官方公开源码和模型参考编码，保留许可证及来源哈希。源码按数据审查，模型推理和HTTP传输部分均未启动。

| 材料 | SHA256 |
|---|---|
| 官方Recipe源码归档 | `BF40407077C893BA51DCFE2D015C457198CFF8C9F01923FCBAD4B11ACD01E9CC` |
| Flash-1017模型参考encoding.py | `13231C592E958FB39E2AF6001DAE69D90E7A36B265702B9B390236FB387599E2` |
| Recipe V4.1 tokenizer.json | `81F64D1248A68CE3663E07AB3EE48B851E5DF0E32D27CB98E4C9A268151E8D99` |
| Flash-1017 tokenizer.json观察 | `C90DFA01249DB1BE4245780A052EDE752E1361C612AC6D08E2BDADA7D599476B` |

两份tokenizer的model、normalizer、pre_tokenizer、post_processor、decoder、truncation、padding和version逐项相同。added tokens仅在ID129264的图像标记不同，文本场景没有图像输入。此对照将本轮文本计量材料对应到当前Flash-1017模型。[模型对应记录](../../build/xuanshu-br2-real-preparation-20261106-d83d0e/encoding-f696f7/model-tokenizer-match.json)

本机原先没有Recipe或tokenizers Python包。Recipe公开0.2.0发布文件没有Windows预编译包，未安装或构建该组件。本轮在独立辅助目录中加载tokenizers 0.23.3 Windows预编译绑定，文件按公开发布SHA256核对，原系统及项目依赖保持原身份。只调用本地Tokenizer.from_str，未调用from_pretrained或凭据接口。[辅助资产](../../build/xuanshu-br2-real-preparation-20261106-d83d0e/encoding-f696f7/helper-assets.json)

## 固定初始内容的离线参考计数

隔离脚本从上一轮固定组件取得完整instructions、两条function工具、空历史、结构化read范围及固定输入，按官方Responses源码梳理的初始文本子集映射到模型参考消息，再调用官方模型encoding.py渲染及本地tokenizer。对state/revision两种字段顺序均计得496个参考prompt token，按原峰时保守费率和4096输出预留为5064000 nano USD，即USD0.005064。[脚本及结果](../../build/xuanshu-br2-real-preparation-20261106-d83d0e/encoding-f696f7/initial-reference-count.json)

该检查尚未执行完整Recipe Responses转换器，未启动生产Service，未计量动态续轮。结果文件明确记录official_responses_converter_executed=false、dynamic_continuations_counted=false、authorization_input_ceiling_established=false。496是所测初始参考内容的计数，不能直接填作全任务input_ceiling。

首次脚本错误地假设post_processor为空，实际配置为ByteLevel，检查在计数前失败。修正为核对并保留实际完整ByteLevel配置，原脚本、stderr及后续成功日志分别保留。源模型和tokenizer保持原字节，产品断言、预算及超时没有调整。[首次辅助错误](../../build/xuanshu-br2-real-preparation-20261106-d83d0e/encoding-f696f7/initial-reference-first.stderr.txt)

## 剩余具体边界

本轮已经取得可进一步实现计量的官方规范，剩余问题收窄到完整转换对齐及动态请求发送前的执行门。

冻结发送链由am_body构造body，av_manifest保存候选请求身份，再经UI目标/配置核对、av_reserve和av_before_send进入WinHttpSendRequest。am_append会将实际模型工具调用及工具结果加入下一轮items。当前av_reserve使用静态策略input_ceiling计费，av_price在收到usage后检查超界。公开完整预览读取使用初始preview，当前没有把外部准备脚本对每轮实际body的计量结果接入发送门。[请求序列化及续轮](../../src/ai_model.inc#L145) [费用门](../../src/ai_validation.inc#L83) [传输边界](../../src/ai_model_net.inc#L64)

未来的工具调用名称/参数、函数结果及模型输出在初始准备时尚未全部确定。本轮没有取得覆盖所有合法续轮的静态上界证明，初始样本计数不能替代该证明。三个请求额度和USD0.10、10分钟规则继续保持，策略verified=false、ceiling=0，执行会话未创建。

## 最小后续实现范围

若继续推进，最直接的闭环是新增仅限BR-2的发送前计量门。在同一worker中，对本轮实际body按已固定身份的完整Responses转换及V4.1编码/tokenizer计算input token，核对计量有效、策略上界和剩余预算，然后沿原持久化占用及最终发送前检查前进。每次初始和续轮均经过同一门，计量失败、未知版本或超界时锁停，额度和逐任务确认语义保持。

这一路径涉及冻结产品的发送逻辑、匹配程序重新构建，以及计量实现或辅助组件的接入边界。原准备授权要求冻结源码、依赖及程序保持原身份，因此相关开发需另行明确授权，并执行修前对照、逐值计量/续轮/费用及关闭回归和限定独立复审。当前仅形成明确的最小开发范围，未修改产品。

完成计量门及相应独立复审后，再准备绑定唯一新库的真实策略和监督入口，提交完整集中授权包。真实凭据及真实请求继续等待该包获批和逐任务可信确认。BR1-KI-001、原冻结结论、全部历史旁文件和已取消控件测试状态保持。
