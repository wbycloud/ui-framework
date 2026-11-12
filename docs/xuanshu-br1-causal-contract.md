# BR-1 模型名因果定位增量

2026-11-05。证据根 build/xuanshu-br1-causal-20261105-6e20b4。两个前置 identity 匹配；原少字符阻断保留。启动监督8项已有结果只核对复用。本轮不把历史现场缺失设为必须重新捕获的条件，也不由通过次数推断根因。

## 写入路径与区分实验

| 候选 | 源码路径 | 最早观察与排除条件 |
|---|---|---|
| 整串输入或选择替换丢字 | ai_edit_panel.text → dispatch_input → lw_text_insert → lw_text_replace | 输入返回立即全文比较；空值、同值替换均应完整；这里差异早于页面或存储 |
| 原生队列与合成焦点混合 | click调用SetFocus；pump不筛选线程消息，TranslateMessage/DispatchMessage；WM_CHAR/IME进入文本替换 | 在自有窗口用普通字符消息按前缀逐值读取，与整串输入对照；不注入Backspace/Delete，不人为删字符 |
| 焦点或刷新改写 | lw_text_pointer、lw_sync_value；paintModel仅在空值时从配置初始化 | 输入后改变焦点、发送状态投影、同值刷新，每次实际全文读取；不等即停止，不重填掩盖 |
| 页面桥接截断 | value getter → JSON.stringify → postMessage → parse_action/read_string → configure | 原有page-wire、answer、host-configure及配置读回关联；保存前后全文一致方可排除此受控条件 |
| 保存/绑定/持久化 | pending_model整串复制 → worker配置文件 → model → task plan →数据库 | 前置探针及persisted直接比较，不能仅比较摘要；未改产品路径 |

第一组新最小实验每配置一次，Light结束后才开始ASAN。每次固定：整串替换、同值替换、焦点转移、三次状态投影、原生逐字符队列（每字符以实际文本为屏障）、立即保存。所有值为合成offline-fixture；每阶段记录线程、序号、长度与实际值。失败时立即停止后续保存并正常关闭，保留自有库、日志和匹配程序。每例原30秒外部门槛，不改变脚本预算。若全部一致，仅排除这些受控序列，不排除未观测的外部编辑或其他写入。

第二组依据第一组结果决定，不能自动重复完整persisted追求通过。界面测试始终串行。没有责任证据时不修改产品，不以保护性断言冒充根因修复。临时控件人工测试已取消，不恢复。
