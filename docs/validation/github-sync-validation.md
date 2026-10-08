# API9源码同步与托管CI结果

记录日期：2026-10-08（Asia/Hong_Kong）。本记录保存已完成运行的结果，不代表后续提交或重跑结果。

## 1. 提交与交付范围

同步提交为[b6a36fee3f4c1a73818cdb01ea7271ad5abd30db](https://github.com/wbycloud/ui-framework/commit/b6a36fee3f4c1a73818cdb01ea7271ad5abd30db)。接手a93967f工作区干净，远端main ea5b108为本地祖先、无远端独有提交；74个本地开发提交与1个同步文档提交普通快进推送至main及codex/menus-offscreen。推送后两分支远端和本地SHA一致；main未受保护，无需PR。没有强推、reset、删除分支、稳定标签、Release或workflow_dispatch。

README、CMake、公共API/语言头文件、标准、完整示例清单、语言测试和API9验收共8份关键远端Git blob与本地提交匹配，主页实际显示SDK0.9.0-dev/API9/标准9，应用ABI1/包格式1。历史v0.1.0标签不变。1225份交付差异检查未包含build/.deps/档案/凭据；本次只修改5份当前入口文档，179个相对链接及diff检查通过，未修改产品或测试。

此前SDK归档保持不变：产品6d88770、文档快照312bce4，SHA256为`7c1dbffce1211b2c0654b41d2942b05adbd4c54d111e983d836c27550d8802e1`。本次公开源码没有重新打包或发布SDK；原本机矩阵61通过/1物理跳过仍属于[API9本机验收](instance-language-validation.md)，不能替代以下CI。

## 2. 实际自动运行

以下三次运行均为上述head_sha、push事件、attempt1，全部completed/failure。

| 运行 | 分支 | 结论 |
| --- | --- | --- |
| [37753988354](https://github.com/wbycloud/ui-framework/actions/runs/37753988354) Windows矩阵 | main | failure |
| [37753988621](https://github.com/wbycloud/ui-framework/actions/runs/37753988621) Windows矩阵 | codex/menus-offscreen | failure |
| [37753988202](https://github.com/wbycloud/ui-framework/actions/runs/37753988202) 严格Session0 | main | failure |

main原始JUnit合计如下；各配置重复用例不代表多个独立根因。

| 配置 | 总计 | 通过 | 失败 | 物理跳过 | 失败用例 |
| --- | ---: | ---: | ---: | ---: | --- |
| native | 22 | 21 | 0 | 1 | 无 |
| light | 45 | 42 | 2 | 1 | ui_browser_host、ui_web_host_frontend |
| osmesa | 52 | 47 | 4 | 1 | 上述两项、ui_host_repaint、ui_instance_language_light（Timeout） |
| webview2 | 62 | 56 | 5 | 1 | 上述四项、ui_stateful_components_light（Timeout） |

跳过项均为ui_monitor_transition。main真实Runtime专属阶段10/10通过、165.97秒，工作分支该阶段10/10通过；整体配置仍失败。工作分支四行中native成功，其余失败；main各行计数不能套用于工作分支。

实际环境为Windows Server 2022/10.0.20348、runner image 20260927.320.1、x64、MSVC19.44、Session2，矩阵桌面1920×1080、DPI96。固定Lexbor `7fb22cf5664a331d7c24b113489e566767c9c25a`、QuickJS-NG `2f0aa72a6b09cf69ff06399bcf9f4c083ac1a278`、WebView2 SDK1.0.4129.50，实际更新后Runtime154.0.4258.62。Mesa24.3.4的软件WGL与显式OSMesa测试分阶段隔离，真实Runtime阶段移除应用局部软件WGL部署；不能把软件GL环境当作物理桌面验收。

## 3. 已确认与未定位的失败

| 用例 | 证据与定位边界 | 后续验证 |
| --- | --- | --- |
| ui_browser_host | [源码](../../tests/browser_host.c)第90行固定期望中文最近列表空提示；英文系统按API9空宿主语言合同呈现英文 | 空宿主按实际系统来源核验；实例语言明确指定，覆盖中英文并保持原可见/禁用断言 |
| ui_web_host_frontend | [源码](../../tests/web_host_frontend.c)第79/86/88行固定期望“是/高级”；原日志显示英文Advanced提示，3个断言失败 | 以明确的所属实例语言检查完整文本及原参数/草稿/一次执行语义，不能改为任意非空文本 |
| ui_host_repaint | [源码](../../tests/host_repaint.c)累计failed，原日志idle-0/1/2及idle-after-GL均paint0；总计1失败。第66行固定中文“操作完成”，是线索，尚未确认具体失败断言 | 先增加可追踪断言诊断，再核验失效区、GL结果、关闭；不能仅因测试名称断定重绘回归或根治 |
| ui_stateful_components_light | webview2配置下整项180秒Timeout | 分阶段及子进程耗时/退出/关闭诊断，不能延长TIMEOUT或sleep遮掩 |
| ui_instance_language_light | osmesa/webview2配置下整项240秒Timeout；部分子用例输出failures0仍不改变整项超时 | 核验脚本所有阶段、消息Dispatch、最终状态及退出，保持实际SendInput和跨进程验证 |

本次同步未修复这些失败，也未确认超时与重绘用例的产品根因。日志打印的Windows error可能是残留GetLastError值，不能单独当成根因。可复验入口为[CI工作流](../../.github/workflows/windows-regression.yml)、[矩阵脚本](../../tools/windows-ci.ps1)、[语言脚本](../../tests/run-instance-language.ps1)、[状态脚本](../../tests/run-stateful-components.ps1)及CMake中原TIMEOUT。保持原资源/预算/关闭断言。

## 4. 当前Session0与严格无登录

当前提交实际LocalSystem/ServiceMain、SessionId0，窗口站Service-0x0-3e7$、Desktop Default；实际应用DLL的OSMesa frame/MSAA与非MSAA通过。

```text
SESSION0 GL PASS: failures=0 commands=2000
NO_LOGIN FAIL: logged_sessions=1 inventory_valid=1
SESSION0 FAIL: failures=1 commands=2000 created_HWNDs=0 process_HWNDs=0
```

资源原日志为句柄147→147，GDI0→0、USER1→1，应用module_shutdown时live_instances0/live_surfaces0。runneradmin存在已登录Session2，故[严格工作流](../../.github/workflows/session0-osmesa.yml)整体失败。它支持当前Session0 GL证据，不能支持整机无登录通过；用户没有严格无登录服务runner，禁止注销用户、修改既有服务或降低门槛。

## 5. 原始证据与交接边界

同机原始证据目录：`D:\应用软件框架\应用层序框架\build\github-sync-20261008`。

- `report.md`、`result.json`：交付与结果摘要。
- `evidence-sha256.json`：437份原证据文件的哈希。
- `main-native-artifact`、`main-light-artifact`、`main-osmesa-artifact`、`main-webview2-artifact`：原JUnit、环境/产物manifest及日志；对应原ZIP保留。
- `main-session0-artifact/target`：原run.log、resources.csv、manifest、实际GL帧。
- `branch-webview2-artifact`与三个`run-*.json`：工作分支原结果和运行身份。

build目录被忽略，不会随clone下载。新环境需从第2节链接下载原Artifacts/日志；过期或缺失时准确记录，不生成数据冒充。凭据与签名下载URL不得写入文档、终端输出或Git。

本次交接文档只本地提交，不新增产品、脚本、测试修改或远程运行。下一轮先复现原失败并保留证据，按[提示词](../ci-recovery-prompt.md)完成本地修复与回归；远程推送/执行需新的明确授权。当前真实其他Windows显示语言、IME、物理不同DPI屏幕/桌面合成、长期人工及严格无登录仍待环境；请求方业务应用修改未授权。
