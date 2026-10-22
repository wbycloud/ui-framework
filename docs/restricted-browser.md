# 受限浏览器范围与公共合同

当前对应 SDK0.9-dev/API9/标准9、ABI1、包格式1。新增两个独立头文件 `light_document.h` 与 `app_data.h`，不修改既有公开结构布局，不回写冻结 SDK。使用同一源码快照的头文件、导入库、DLL、宿主和包。

## 支持表

| 内容或行为 | 首版范围 | 可复验检查 |
| --- | --- | --- |
| 起始页、帮助 | 包内资源，无网络；默认不启动浏览器 | workspace 双实例初始快照、宿主空启动无 DLL |
| HTTP/HTTPS | WinHTTP 异步；完整 origin 允许表；逐跳检查、最多8次重定向；保留系统证书检查 | `/page`、`/redirect`、`/cross`、`/protocol`、`/loop`；本地自签 TLS 被拒绝 |
| URL | 固定 Lexbor WHATWG 解析、IDNA、相对路径、端口、片段、UTF-8 百分号编码 | URL 公共测试、中文片段、GET 表单、3000字节查询地址 |
| 文本编码 | BOM优先，其次HTTP charset、前1024字节HTML meta，再默认UTF-8；固定Lexbor解码器严格转换，转换结果也计限制 | UTF-8中文/英文、GB18030、UTF-16 BOM、非法/超额文本拒绝 |
| 文本 | p、h1–h6、br、strong/b、em/i、span、区块及列表；ol序号为连续默认序号 | 固定中文/英文页面；真实选择、复制和键盘输入 |
| 链接、片段 | 本实例导航；不执行事件或特殊协议；不存在的元素返回 NOT_FOUND | 公共结构化事件、真实点击、双轴定位 |
| 图片 | WIC PNG/JPEG/GIF第一帧/BMP；框架 RGBA 图片生命周期；错误显示 alt 或 image 标记 | `/formats`四图、动画简化计数、损坏/超尺寸图片 |
| 表格 | 矩形行列，最多64列；按内容计算列宽；宽表格使用文档横向滚动 | 12列固定表、真实 Home/End 和滚动 |
| pre/code | 保留换行、等宽字；长行可横向访问 | 1700字节代码行、Ctrl+End / Ctrl+Home |
| GET表单 | text/search、hidden、submit；最多64个成功字段；原生事件交付字段数组；UTF-8 form 编码 | 真实编辑与提交，中文、空格、&、+ 编码 |
| 选择与复制 | 跨文本节点的 UTF-16 选择，Uniscribe 绘制与命中；系统 Unicode 剪贴板 | 实际鼠标、查找后 Ctrl+C；测试恢复原剪贴板 |
| 查找与键盘 | 大小写不敏感、前后、回绕、显露；Tab焦点显露并可离开文档；方向/Page/Home/End | 公共检查、真实文档窗口和实际包快捷键 |
| 缩放与DPI | 文档50–200%，独立于窗口DPI；程序96/144/192；主题只改变工具栏 | 窄320逻辑宽、三档DPI真实点击；作者颜色像素检查 |
| 导航与多实例 | 后退/前进/刷新/停止/首页；64条会话历史；适用滚动恢复；实例独立 | 双实例、慢旧结果、停止、长标题/地址 |
| 关闭 | WAIT直到线程真正退出、句柄最终通知结束；模块 shutdown 检查活跃实例/线程/回调 | 下载中关闭、观察到 WIC 解码时关闭、失败清理、最后 DLL 不再存在 |

CSS 使用现有 Light 的受控颜色、字体、类/ID/后代选择器与媒体规则。文档流使用阅读布局，CSS绝对定位、尺寸、复杂flex、复杂选择器及不支持声明会被简化或拒绝并计入受限状态。页面样式只存在于独立文档视图，不进入工具栏、宿主或其他实例。作者的 body 背景和文字颜色可保留。

无脚本的SPA、空文档或样式使全部内容不可读时明确失败。iframe、Canvas、SVG、音视频、对象等显示不支持标记；POST、密码、文件、复杂输入控件拒绝。表格合并单元格、ol start/reversed、自定义列表标记、字体下载、动画、图片EXIF方向和完整可访问性树不在首版保证中。没有视频/音频/PDF/WebGL、上传、自动下载执行、持久Cookie/存储、复杂登录、扩展或开发者工具。

## 隔离与资源

可信工具栏仅从包资源读取，通过公共 JSON 数据消息更新。远程后端永远不创建 JSRuntime/JSContext；通用 load_html、post_json 和应用消息回调全部拒绝。没有脚本、内联事件、javascript/file/data/应用协议、ui.invoke 或 ui.postMessage 的可达路径。下载内容从不拼成脚本或命令。

远程文档不是进程沙箱。应用 DLL 与宿主同进程，面向受控来源；没有验证任意恶意网页安全性。

| 门槛 | 值 |
| --- | --- |
| HTML输入及转换后UTF-8 | 256 KiB |
| 全DOM/Light节点、深度、绘制片段 | 1024、64、8192 |
| 单文本/属性 | 4095 / 2047 UTF-8字节；ID95字节 |
| 可解析的远程字体逻辑字号 | 1–128；超额拒绝；无效声明另计简化，不分配巨型字体 |
| 子资源、CSS、样式规则 | 40个、每份64KiB、共256条 |
| 单图传输、尺寸、RGBA | 2MiB、每边4096、少于8MiB（还预留投递头开销） |
| 每导航累计传输/解码 | 8MiB / 32MiB |
| 框架每实例图片/复制投递 | 原32MiB / 原8MiB，不抬高 |
| 网络时间 | 系统分阶段3秒；请求含重定向15秒期限；取消唤醒等待 |

超额不截断冒充成功。WIC 在像素分配前读取尺寸；转换后的文本也受上限约束。文档及资源请求带稳定 navigation/resource ID，旧结果被丢弃。WinHTTP最终关闭通知、在途回调计数和真实线程句柄共同参与关闭判断；工作线程只持独立网络状态、workspace与实例ID。

## 公共 C 使用

```c
#include "ui_framework/light_document.h"
ui_light_web_config_t cfg = {0};
cfg.size = sizeof(cfg);
cfg.parent_hwnd = content_parent;
ui_web_backend_t *backend = ui_light_document_backend_create(&cfg);
ui_web_view_t *view = ui_web_view_create(host, backend);
ui_light_document_set_event_callback(view, on_document_event, application);
ui_document_load_t load = {0};
load.size = sizeof(load);
load.navigation_id = next_navigation_id;
load.source_url = final_checked_url;
load.html_utf8 = downloaded_utf8;
ui_status_t result = ui_light_document_load(view, &load);
/* 退出当前输入回调后，在UI线程flush发出资源事件。 */
if (result == UI_STATUS_OK) ui_web_view_flush(view, 64);
```

事件参数借用于回调内，复制后再处理。NAVIGATE、SUBMIT交付URL及GET字段；IMAGE/STYLESHEET交付资源ID；CANCEL_RESOURCE通知已发资源被取消；FOCUS_OUT的resource_id为1表示反向边界。回调内不重载/销毁视图。资源交付、状态、查找、定位、缩放、复制都通过独立公共函数；陈旧资源返回 NOT_FOUND。

`app_data.h` 提供 UI 注册接收函数与线程安全 `ui_workspace_post_app_data`。投递复制二进制数据，复用实例8MiB预算，队列没有应用回调指针。WAIT期间继续交付清理结果；ALLOW后丢弃剩余复制，workspace存活到全部工作停止。

完整可执行公共调用方位于 `tests/document_public.c`、`tests/workspace_public.c`、`tests/failure_public.c`。宿主入口逻辑只定位同目录 browser.uapp 并调用现有 open_path/workspace，不包含解析、网络、历史或应用工具栏逻辑。

## M2阅读兼容性（0.3.0）

本批在既有公共C合同下实现十进制列表起始/倒序/单项覆盖、64列内的合并单元格与caption、仅可见文本查找复制、受控字体/空白/间距和异步样式来源顺序，以及GET成功字段/提交者。应用补齐编码错误分类、JPEG EXIF八方向及最终片段定位。公共结构和导出不变；头文件、库、DLL与应用按同一源码SHA匹配，固定依赖和冻结SDK不覆盖。具体范围、简化、资源门槛及本批实际证据见apps/browser/docs/m2-support.md和m2-validation.md；既有历史记录不替代本批验证。
