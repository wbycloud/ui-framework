# M2 阅读兼容矩阵（0.3.0）

维护入口为本工程，apps/browser是摘要保护的集成镜像。M0/M1既有能力单独回归；下表的新能力必须以m2-validation.md和本批verification.json的实际结果为准。没有指定真实站点，继续使用固定本地集，不承诺任意整站兼容。

| 样本 / 输入 | 操作与预期 | 最小实现、简化或拒绝 | 检查入口 |
| --- | --- | --- | --- |
| E1 BOM/HTTP/meta冲突、中英、多字节 | BOM优先，HTTP其次，前1024字节meta再次，默认UTF-8；中文完整 | meta的UTF-16标签按HTML规则用UTF-8；HTTP/BOM UTF-16严格解码 | encoding.c、/m2-bom-conflict、/m2-http-conflict、/m2-meta-utf16 |
| E2 未知声明、坏UTF8/奇数UTF16、NUL、GB18030转换后270KiB | 未知/损坏/转换预算原因可区分，明确失败 | 原256KiB输入及输出预算，不回退猜测编码或截断 | encoding.c、m2_public.c |
| T1 实体、组合字符、嵌套强调、pre/code及长行 | 查找、选择复制、换行、横向到达；跨节点顺序正确 | Uniscribe字簇不拆开；pre保留空白、Tab按4空格位置显示；不承诺完整Unicode排版 | m2_document.c、workspace_public --desktop、/m2-reading |
| L1 嵌套ul/ol，start/reversed/li value及负数 | 7、11、12和倒序3/-2/-3编号正确 | 十进制；整数声明限制32位，计数安全扩展；自定义列表标记不保证 | m2_document.c、/m2-reading |
| G1 caption、表头、空格、rowspan/colspan、rowspan=0、12列宽表 | 合并格不重叠，后续格不占已覆盖列；行高包含跨行正文；宽表横滚 | 最多64列；跨行限制在所属行组，0延伸至组末；复制为DOM阅读顺序文本，不还原电子表格合并元数据 | m2_document.c、/m2-reading |
| G2 colspan65、257条CSS及节点/文本/深度旧超限 | 整份明确拒绝，避免静默截断或丢格 | 原节点1024/深度64/规则256等门槛不提高 | m2_public.c、workspace_public.c |
| C1 作者颜色、字体继承、正常/预格式化换行、px行高和非负间距 | 保留有效声明，忽略不支持声明时计入受限；隐藏正文不进入查找复制 | 只实现基础阅读流，不扩展现代布局/完整CSS；固定阅读边距仍存在 | m2_document.c、/m2-style.css |
| C2 先外部link后style，外部表异步晚到 | 按文档来源顺序及既有选择器优先级应用，后置作者样式保留 | link样式受origin/大小/数量检查；CSS url/@import不自动请求 | m2_document.c、m01_public.c |
| I1 PNG/JPEG/BMP、GIF两帧、EXIF1–8 | WIC严格解码、八方向尺寸与像素正确、保持比例 | 动画只取首帧并在诊断明确标记；EXIF限JPEG IFD方向字段，不承诺所有元数据/色彩管理 | image.c、m2_images.py、/m2-image/* |
| I2 损坏、无来源、超尺寸和超像素 | 替代文字参与阅读流、查找与复制；失败和等待有可见标记 | WIC分配前检查4096边长；RGBA及方向位图共同计入原8MiB，累计RGBA32MiB | image.c、m2_document.c、workspace_public.c |
| N1 相对链接、中文片段、资源晚到 | 普通导航；全部资源完成后定位最终目标，失败片段明确提示 | 不执行javascript/file/data/应用协议 | m2_public.c、workspace_public --desktop |
| F1 空值、重复字段、disabled、提交按钮、CRLF和百分号 | 仅成功控件按顺序交付；显式按钮及Enter默认提交者；保留action片段、替换query | 简单GET/text/search/hidden/submit；字段上限64、URL4096字节 | m2_document.c、url.c、m2_public.c、真实Tab提交 |
| F2 POST、密码/上传/复杂控件及提交覆盖 | 可见拒绝，不发送缺少关键控件的“成功”GET | 继续不支持复杂控件/外部form关联/submit overrides | m2_document.c、/m2-reading |
| S1 远程脚本/事件/桥、作者样式、双实例 | 脚本运行时0、桥不可达；工具栏及其他实例不受样式影响 | 无进程沙箱，仅受控来源；保持M1策略和脱敏 | document_public.c、m01_public.c、m2_public.c |
| R1 PAGE/next循环、取消、下载/解码关闭 | WAIT直到线程/回调结束，真实DLL卸载和宿主退出 | 沿用原16次往返、128MiB总量与32MiB增量门槛 | workspace_public.c、failure_public.c、host_process.py |

性能样本保留旧PAGE（25段、12列、1700字节代码）并增加/m2-reading（25段、合并表及12列、2000字节代码、方向图/GIF/坏图）。请求管线network_ms仍含转换与句柄收尾；解析、布局、WM_PAINT分开记录。前台以完成绘制后能查找复制、滚轮/Tab及GET返回为可见终点；隐藏窗口paint=0只作逻辑检查。M4再深化EOF、转换和长期指标，不以API成功替代可见反馈。

物理DPI、多屏、真实IME、长期人工及完整框架矩阵条件不足时单列未验证。M3—M5继续留在路线图后续。

M2边界补充：`visibility:hidden/collapse` 折叠其子树并增加简化计数，不保留CSS可见性占位或允许子元素恢复visible；极小像素行高约束到测得的字体高度，避免正文重叠并增加简化计数。均由公共阅读检查先复现后验证。

编码meta标签中的UTF-16声明按[HTML解析规则](https://html.spec.whatwg.org/multipage/parsing.html)使用UTF-8；BOM和HTTP头保持较高优先级。WIC旋转实现采用一次RGBA分配后的有界原位变换，避免[WIC未缓冲旋转器](https://learn.microsoft.com/en-us/windows/desktop/api/wincodec/nn-wincodec-iwicbitmapfliprotator)反复请求像素的代价，方向辅助位图同样计入原队列门槛。
