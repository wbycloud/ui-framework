# 2026-10-09 本地界面修复证据

逐项根因、实际提交、回归及限制见[验收](../ui-repair-validation.md)。本目录是新证据的交付副本；原build/github-sync-20261008及历史验收不改写。原始桌面BMP、EXE/DLL、SDK ZIP仍在本机build/ui-repair-20261009，不随clone取得。本文不将本机结果当成托管CI、硬件鼠标人工操作、物理192DPI或Session0。

## 阅读顺序

1. [接手](raw/takeover.json)、[历史证据/SDK只读核验](raw/preservation-delivery.json)。
2. [原浮动失败](raw/red-float.log)、[明确只读/提示失败](raw/red-readonly2.log)、[End强断言失败](raw/display-red-assertion.log)、[长说明独立旧DLL失败](raw/help-red.log)。对应失败截图保留，不把早期fixture错误当产品根因。
3. [最终矩阵](raw/matrix-final-summary.json)、[原生矩阵](raw/matrix-native/ctest.xml)、[脚本完成汇总](raw/script-runs-final-summary.json)、[六组脚本可移植索引](raw/script-runs-index.json)。索引将本机绝对路径映射到raw/script-runs内原样复制的manifest及逐子进程日志。
4. [07a输入身份](raw/accepted-07-provenance.json)、[五模式实际退出](raw/accepted-07-exits.jsonl)、[校准Runtime探针身份](raw/runtime-bar-verified-provenance.json)和[真实输入日志](raw/runtime-bar-verified.log)。最初Runtime滚动条操作exit0但截图仍是首行，不能作为该项通过证据；校准后按住/释放/移开均完整显示末行。
5. [GL连续按住](accepted-07-float-held-contact.png)、[草稿连续按住](accepted-07-float-form-held-contact.png)及各screenshots目录。两模式各29帧，接触图包含10帧经过GL、3停靠目标、5从停靠浮动及Esc/捕获丢失按住阶段。接触图缩放用于浏览，判断细节使用逐帧PNG。
6. [SDK归档身份](raw/sdk-07a66a8-archive.json)、[包内文件身份](raw/sdk-07a66a8-manifest.json)、[独立解包C调用](raw/sdk-consumer-07a66a8.log)、[实际导出](raw/exports-07a66a8.log)、[示例导入](raw/example-imports-07a66a8.log)。

screenshots.json记录332张PNG的原BMP路径/SHA256、2560×1440尺寸及裁剪坐标、派生PNG/SHA256；只有无损矩形裁剪和RGB转换，没有内容修补。浮动图取1200×820，普通窗口735×565，右下边缘400×492。原BMP保留桌面其他窗口；公共复现前景、焦点及实际命中另在日志确认。截图目录名含accepted/visible不能单独表示通过，判定以验收和画面为准；失败长说明与边界命中截图同样保留。

## 重建与运行

在仓库根目录的x64 VS开发终端操作。固定依赖存在时核验提交/工作区，不运行prepare覆盖依赖；本轮准确配置/编译命令原样保存于[Runtime](raw/build.cmd)、[Light](raw/light-build.cmd)、[OSMesa](raw/osmesa-build.cmd)、[native](raw/native-build.cmd)。这些命令内含本机路径及原证据目录，复验须改用独立新目录并替换本机工具路径，不能覆盖本轮或历史材料。

```powershell
$provider=(Resolve-Path .deps/mesa-24.3.4/x64/osmesa.dll).Path
cmake -S . -B build/ui-repair-replay-webview2 -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_FRAMEWORK_ENABLE_WEBVIEW2=ON "-DUI_OSMESA_LIBRARY=$provider"
cmake --build build/ui-repair-replay-webview2
./tools/test-windows-ci.ps1
& ./docs/validation/ci-recovery-evidence-20261008/raw/run-regression.ps1 -BuildDirectory build/ui-repair-replay-webview2 -EvidenceDirectory build/ui-repair-replay-webview2-evidence
```

该既有分阶段运行器读取当前tools/windows-ci-checks.ps1和构建的JSON清单；本轮执行的四配置/最终三配置准确版本是[matrix.ps1](raw/matrix.ps1)及[matrix-final.ps1](raw/matrix-final.ps1)。其他三配置cmake选项见对应cmd。CTest≥3.26，串行执行；WGL前复制Mesa四DLL，显式provider及Runtime前移除**该新树内本轮复制的四文件**。不要对任意历史目录清理DLL。完整Light存储180/语言240、Runtime存储240/语言300和repaint45秒保持，JUnit保留失败及唯一物理skip。

真实可见性不由CTest替代。各模式在解锁桌面串行执行，证据目录必须全新；禁止并行输入探针。示例：

```powershell
New-Item -ItemType Directory build/ui-repair-replay-input-form
& ./build/ui-repair-replay-webview2/ui_interaction_probe.exe (Resolve-Path build/ui-repair-replay-input-form).Path form
New-Item -ItemType Directory build/ui-repair-replay-input-runtime
& ./build/ui-repair-replay-webview2/ui_interaction_probe.exe (Resolve-Path build/ui-repair-replay-input-runtime).Path form-webview2
New-Item -ItemType Directory build/ui-repair-replay-input-float
& ./build/ui-repair-replay-webview2/ui_interaction_probe.exe (Resolve-Path build/ui-repair-replay-input-float).Path float
New-Item -ItemType Directory build/ui-repair-replay-input-draft
& ./build/ui-repair-replay-webview2/ui_interaction_probe.exe (Resolve-Path build/ui-repair-replay-input-draft).Path float-form
New-Item -ItemType Directory build/ui-repair-replay-input-display
& ./build/ui-repair-replay-webview2/ui_display_probe.exe (Resolve-Path build/ui-repair-replay-input-display).Path
```

必须重定向并保存各次标准输出/实际exitCode、审阅末行/连续画面，而不只判断exit0。SendInput是OS原生输入路径；人工硬件鼠标、外部捕获抢占、物理混合DPI、IME及长期操作尚待条件。程序192/窗口96/monitor effective96、raw108分开记录。泛化192叠层、窄表头残影未独立定位；托管超时、历史重绘未定位且未远程复验。历史Session0 GL通过与严格无登录失败分别沿用，普通登录桌面不替代。

## 身份核验

evidence-sha256.json列出本目录交付文件大小/SHA256，排除索引自身及audit-result.json；局部.gitattributes禁止EOL转换，以保留原始日志字节。原始文件的本机→交付映射在raw/copied-file-identities.json。不同的早期快照不覆盖，同名后续日志另用-delivery后缀。

```powershell
python docs/validation/ui-repair-evidence-20261009/audit-delivery.py
```

默认核验随Git文件、截图派生身份、最终185项JUnit及六份完整脚本退出；不要求ignored的build目录。加 `--with-local-originals`另核验332张原BMP及复制源，缺失如实报错而不生成。SDK归档已CRC及逐文件SHA256核验，包内C调用编译命令是[原记录](raw/sdk-consumer-07a66a8.cmd)；归档本身未纳入Git。

当前产品07a66a8与手动探针4fdfcc2的源码/二进制分开绑定；最终文档提交不改变产品。未来授权托管运行仍需在原英文runner按原预算留最后running/PID、线程栈/调度等待、关闭/卸载及真正进程退出，不能把本目录本机成功写成托管修复。
