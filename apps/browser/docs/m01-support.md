# M0 产品与兼容基线（0.2.0 / M0–M1）

应用维护入口是本工程；框架 apps/browser 是集成镜像。当前无指定的真实目标公网站点，以受控固定集建立承诺。表内“已实现”描述代码能力；本批通过/失败证据独立记在 m01-validation.md，不挪用首版通过数字。

| 样本ID / 来源 | 输入与规模 | 用户操作及预期结果 | 简化或拒绝 | 检查入口 / 状态 |
| --- | --- | --- | --- | --- |
| B01 包内 start/help | 两份UTF8包资源，无网络 | 菜单打开、首页、帮助、关闭；内容可读，标题更新 | 无远程资源 | workspace_public / host_process；已实现 |
| R01 内部 /page | 中英25段、12列表格、1700字节代码长行、列表、表单、PNG | 阅读、横纵滚动、选择复制、查找、链接与返回 | 脚本不执行、坏图替代、iframe/Canvas标记 | workspace_public / desktop_public；已实现 |
| R02 /next 与 /page#末尾 | 相对链接、中文片段 | 点击定位、Home/End、前进后退，恢复适用滚动位置 | 单实例单宿主标签 | desktop_public / workspace_public；已实现 |
| R03 /form | text/hidden/submit、中文空格&+ | 编辑并GET提交，百分号编码可验证 | 拒绝POST/文件/密码/复杂控件 | desktop_public / url；已实现 |
| R04 /formats | PNG/JPEG/GIF/BMP各32×24 | 查看四图；图片释放不残留 | GIF只取第一帧并计简化 | workspace_public；已实现 |
| R05 /gbk /utf16 /gzip | GB18030、UTF16 BOM、压缩UTF8 | 打开后标题和中文正确 | 非法或超额转换明确失败 | workspace_public；已实现 |
| R06 /long-title /next?long=… | 标题约4090字节、查询3000字节 | 地址栏/宿主标题保留可达；诊断与快照隐藏查询 | 不默认搜索；不将放行当整站兼容 | workspace_public；已实现 |
| A01 可信设置 | 每行精确origin，16项/4095字节 | 设→编辑→保存，无需重打包；两实例500ms内更新并回首页 | 非法保存不覆盖旧文件；空列表拒绝全部 | m01_public / settings；本批实现 |
| A02 本地配置 | 缺失/无效/不可读/空文件 | 缺失用包默认；无效/不可读拒绝全部，明确恢复入口 | 不因配置错误全部放行 | settings / m01_public；本批实现 |
| A03 地址或策略拒绝 | 非法URL、file/javascript、凭据、相似域名/端口 | 显示url/protocol/credentials/origin具体原因 | 无网络请求，不自动放行 | url / m01_public；本批实现 |
| A04 /redirect /cross | 同源及跨源302 | 每跳检查；未允许目标明确redirect-origin，手动设置后可继续 | 不自动放宽规则 | m01_public；本批实现 |
| A05 /protocol /credentials-redirect /redirect-missing /loop | 特殊协议、凭据、无Location、循环 | 显示不同redirect原因，展示来源关系、步数 | 最多8次；无凭据或查询泄漏 | m01_public；本批实现 |
| A06 /redirect-secret | 参数带测试敏感值 | 跳转成功；诊断只显示来源，快照无查询或片段 | 不输出完整Location/表单值 | m01_public；本批实现 |
| A07 /diag-resources | 可读主文档+CSS500/来源拒绝+坏图/超图 | 保留正文，明确“部分呈现”，诊断分资源类别与原因 | 有限诊断显示明确标记省略 | m01_public；本批实现 |
| E01 /404 /500 /wrong-type /attachment | 状态码、错误MIME、附件 | http/content-type/attachment准确分类和操作说明 | 不自动下载或执行 | m01_public；本批实现 |
| E02 /disconnect /slow-headers | 断开连接、5秒才发头 | transport阶段和系统码；可取消的3秒操作等待、15秒总期限 | 不延长超时制造通过 | m01_public；本批实现 |
| E03 HTTPS localhost:8766/page | 新自签证书，固定PAGE | 未信任TLS拒绝；本批授权临时信任后可读，撤除后恢复拒绝 | 系统证书验证保持，记录TLS标志 | m01_public / workspace_public；本批复验 |
| E04 /big /longtext /deep /nodes /resource-limit /image-limit | HTML>256KiB、文本4096、深度70、节点1100、图42个、尺寸6000² | 明确预算/文档预算/资源失败；不静默截断 | 现有门槛不提高 | workspace_public / m01_public；已实现并细化分类 |
| E05 /invalid-encoding /empty | 坏UTF8、纯脚本SPA | encoding/document-empty原因可区分 | 不把空白当呈现成功 | m01_public；本批实现 |
| S01 /policy-attack /page | 远程脚本/内联事件/ui.invoke/ui.postMessage | policy_count不变，script_runtime=0；可信加载及桥拒绝 | 不支持远程脚本，无进程沙箱 | document_public / m01_public；持续必过 |
| L01 /slow /decode-page | 慢旧结果、约8MiB内WIC解码图 | 连续导航、停止、规则修改、关闭；旧结果不覆盖新页 | WAIT直至工作线程/回调结束 | workspace_public / m01_public；持续必过 |
| U01 可信界面 | 明暗主题、320逻辑宽、96/144/192程序DPI | 设置、诊断、Tab、复制、按钮均可达；工具栏不受页面样式影响 | 物理条件不由程序DPI替代 | workspace_public --desktop / desktop_public；本批复验 |
| P01 example.com | 公网补充，不是整站样本 | 记录默认/自动代理同参数对照与真实包结果 | 失败独立记录，不承诺公网可达 | proxy_probe / workspace_public --https；未定位项 |
| F01 真实目标站点 | 用户尚未提供站点与关键任务 | 待明确合法样本和关键操作后再建立承诺 | 不宣称任意现代站点兼容 | 待决策；M2以后 |
| F02 IME/物理DPI/多屏/长期人工 | 当前不具备完整条件 | 取得环境后独立验收 | 本批不以合成输入或短程循环代替 | 未验证；M3–M5 |

## 源码集成规则

`python tools/export-framework-source.py` 默认只检查；`--sync` 才同步。工具先核对目标与 tools/integration-state.json 的最后审阅哈希。目标独有文件、目标独立修改或意外删除都会拒绝写入，保留文件供人工审查；不得用重新生成状态绕过冲突。状态文件只由成功审阅的同步更新。tests/integration_sync.py 实际验证目标独有修改不会被覆盖。

本批接手差异只有新工程README的路线入口和新增两份交接/路线文档，镜像没有独有修改；均先备份并记录摘要后才同步。不要在镜像独立开发或运行同步写入。源码快照标识实际文件，不以未改变的HEAD冒充已提交版本。

## 性能样本与观察终点

本批继续固定 server.py 的 PAGE，不减少首版规模。network_ms实际是主请求管线耗时（含文本转换和句柄收尾），decode_ms是WIC或后续输入校验；解析、末次布局、WM_PAINT和资源就绪分别记录。现有字段不能完整剥离网络EOF与编码转换，作为M4指标缺口，不把管线值冒充纯网络时间。隐藏窗口paint=0不计可见成功。真实前台测试使用同一PAGE及新设置/诊断面板，等到实际输入状态变化与绘制完成，再验证关键操作。短程内存门槛仍为总量≤128MiB、16次往返增长≤32MiB，网络/文档/图片/队列门槛沿用 browser.md。

M2阅读扩展、M3日常体验、M4长期可靠性与性能、M5稳定交付深验均保留为后续路线，本批只完成其持续必需的回归和交付检查。
