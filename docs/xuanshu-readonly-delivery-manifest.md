# DH-1只读版本交付清单

状态：2026-11-05匹配快照冻结记录已落盘，只读版本交付材料齐备。无安装器或新打包结构；使用原产物，不搬移、不覆盖、不替换DLL。不是正式发布或整个系统验收。

## 身份及运行文件

仓库：`D:\应用软件框架\应用层序框架`。
交付根：`D:\应用软件框架\应用层序框架\build\xuanshu-daily-host-20261104-c82f19`。
交付identity：`65823A92BAAF49CE2623F5865F2C47B678215DD813D7E8EDDC8725980107A532`（delivery-identity.json SHA256）。

以下路径相对于上述明确交付根；运行工作目录为该根的light目录。

| 文件 | 字节 | SHA256 | 用途 |
|---|---:|---|---|
| light/framework_host.exe | 465408 | 0B028BE68B45617A3F84A03F891546498EB9AC5723EB3041F4117F861662B22F | 正式宿主，运行必需，宿主页面为编译资源 |
| light/ui_framework.dll | 5682688 | D7169B2BCBCB25141EC71ADA05C4D52125A15A148C36922FE0437F879F61A4C2 | 匹配生产运行时，运行必需，不能替换为测试DLL |
| light/rpc_memory.uapp | 11579 | EBF374A177B37135B212150DDF371207F92A8E610B38071D800B14858969629E | 已验证启动场景的生产链接合成只读夹具，不是真实业务应用 |

source-after.json关联419项源码，binary-files.json关联74项产物，delivery-identity.json关联证据清单。全部产物不等于全部运行依赖；测试程序、测试DLL、ASAN目录、静态库、原始日志/JUnit、驱动与截图属于验证/开发材料，不作为日常安装入口。其他应用包不因存在于目录而纳入本次验收。rpc_memory不代表任意宿主应用已通过。

宿主及DLL的导入依赖包括Windows USER32/GDI32/IMM32/ole32/bcrypt/KERNEL32等系统组件，DLL另有OpenGL、WinHTTP、UCRT及VCRUNTIME140.dll等。需要兼容的Windows环境和Microsoft Visual C++ x64运行库；不得从不明来源复制系统DLL。本轮只读核对导入，没有安装或下载依赖。当前机器已运行通过不等于全新机器安装验证。既有OSMesa离线测试依赖不自动列为本生产宿主的额外部署文件。

## 路径与启动说明

未指定UI_AI_DB_PATH时，默认数据库为 `%LOCALAPPDATA%\ui-framework\xuanshu\conversation.sqlite`，默认模型配置为 `%LOCALAPPDATA%\ui-framework\xuanshu\model-config.json`。

显式指定UI_AI_DB_PATH时，配置精确派生为数据库路径追加 `.model.json`，例如conversation.sqlite.model.json。超长路径明确拒绝，不截断或回退默认配置。不得改变LOCALAPPDATA、迁移旧库、复制凭据或复用RPCB指定引用。

下面命令仅供以后另行授权的隔离启动使用，本轮没有执行；不授权Key录入或外发确认。它创建全新目录，不复用旧人工库，保留PID、stdout/stderr和退出码：

```powershell
$programRoot = 'D:\应用软件框架\应用层序框架\build\xuanshu-daily-host-20261104-c82f19\light'
$sessionRoot = Join-Path 'D:\应用软件框架\应用层序框架\build' ('xuanshu-readonly-session-' + [guid]::NewGuid().ToString('N'))
if (Test-Path -LiteralPath $sessionRoot) { throw '隔离目录已存在，停止' }
New-Item -ItemType Directory -Path $sessionRoot -ErrorAction Stop | Out-Null
$previousDbPath = $env:UI_AI_DB_PATH
try {
    $env:UI_AI_DB_PATH = Join-Path $sessionRoot 'conversation.sqlite'
    $appProcess = Start-Process -FilePath (Join-Path $programRoot 'framework_host.exe') -ArgumentList ('"' + (Join-Path $programRoot 'rpc_memory.uapp') + '"') -WorkingDirectory $programRoot -RedirectStandardOutput (Join-Path $sessionRoot 'stdout.log') -RedirectStandardError (Join-Path $sessionRoot 'stderr.log') -PassThru
    $appProcess.Id | Set-Content -LiteralPath (Join-Path $sessionRoot 'pid.txt')
    $appProcess.WaitForExit()
    $appProcess.Refresh()
    $appProcess.ExitCode | Set-Content -LiteralPath (Join-Path $sessionRoot 'exit-code.txt')
} finally {
    $env:UI_AI_DB_PATH = $previousDbPath
}
```

启动后默认不提交任务；不得把启动成功或配置ready当作凭据/网络验证。提前退出保留材料并停止，不循环重启。运行时正常取消、等待保存及正常关闭，异常时不强杀、不清UNKNOWN；操作入口见[使用说明](xuanshu-readonly-user-guide.md)。保存故障材料前排除Key、Authorization、凭据Blob和完整内存。

## 证据入口与限制

- [开发验收](validation/xuanshu-daily-host-validation.md)：各次离线测试及自审，原始首败保留。
- [独立复审与冻结](validation/xuanshu-daily-host-independent-review.md)：承接此前复审，本轮仅身份核对与记录。
- [自动界面验收](validation/xuanshu-dh-auto-validation.md)：`build/xuanshu-dh-auto-20261104-07e6b3`，唯一PID12204、exit0；关后1任务/2消息/3事件/0工具，唯一cancelled，无外发确认或模型请求保存。
- [阶段索引](xuanshu-native-stage-closeout.md)：历史人工及真实Provider证据各自独立，不自动覆盖新界面全部行为。

既有关后库SHA256为 `2969759861A5FD84C7D7DA8F329F705DC05C77F49374F635421C9FC60EF147A6`；无WAL/SHM且immutable核对前后相同，本轮未重读库。无全程网络遥测，只报告现有证据显示未进入模型请求路径，不虚构精确HTTP或RPC配额计数。

## 交付状态与Proposed后续

必须修正项为空。只读交付文档齐备；源码与生产快照不因后续文档更新改变身份。全部系统、任意真实业务应用、全部真实服务及人工验收仍不能宣称通过。

有Key真实使用验证、可选易用性改进、新安装器/打包及新机器部署检查均为Proposed，需独立明确授权后才实施。取消的临时页面控件测试不恢复；原独立设备未验边界和browser九项副作用另案保持。本轮不启动任何下一阶段。
