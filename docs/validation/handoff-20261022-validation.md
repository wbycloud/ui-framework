# 2026-10-22 持久化与源码推送核验

接手HEAD `689fa03b34f8c4321a6c838f52ca9dcd790a9319`，分支 `codex/p1-process-boundaries`。本次不改产品实现；保存已完成的菜单/提示修复及浏览器集成源码，补当前交接。排除所有新增原始证据、SDK、截图、转储及过程产物；原已跟踪历史文件保留。

## 本次实际检查

从Git暂存区导出329份构建所需源码/资源到新目录 `build/handoff-20261022-621f94c6/source-clean`，不从工作树补漏文件。逐文件SHA256清单聚合身份为 `abbc8a65187a198b3b70422b108bbe7b83a839b05f2e7da65dfb3a7ba3d46297`（算法为按路径排列的path/sha256对象数组，JSON sort_keys、紧凑分隔符，再取SHA256）。依赖只引用既有固定Lexbor/QuickJS；实际Git HEAD分别核对为合同中的两个固定commit，未下载或覆盖依赖。

MSVC19.50.35727、CMake/Ninja，x64 Release。实际配置与构建命令如下，两个依赖路径是本机只读固定checkout，clone须先按构建说明准备它们：

```powershell
cmake -S source-clean -B compile-clean -G Ninja -DCMAKE_BUILD_TYPE=Release -DBROWSER_BUILD_CHECKS=ON -DUI_FRAMEWORK_ENABLE_WEBVIEW2=OFF -DUI_OSMESA_LIBRARY= -DUI_LEXBOR_SOURCE_DIR=<固定Lexbor目录> -DUI_QUICKJS_SOURCE_DIR=<固定QuickJS目录>
cmake --build compile-clean
ctest --test-dir compile-clean -R '^ui_(hover_boundaries|text_replacement|theme_core|workspace_titles|public_headers_c|public_headers_cpp|light_rect_stability)$' --output-on-failure --output-junit checks.xml
```

- 全构建440步骤完成、OS退出0：包含默认框架/宿主/维护示例和 `browser.uapp`，并额外开启浏览器检查程序。没有Mesa或WebView2依赖，不扩大到硬件GL。
- 上述7项CTest全部通过；不是完整框架回归。
- 浏览器document、reading、url、settings、encoding五个独立检查均OS0。image首次漏传必需样本目录，OS2，未进入图片断言；用仓库 `tests/m2_images.py` 生成确定性样本后，该项OS0、八方向pixel_max_delta=0、格式/预算0失败。保留首次失败，不记为一次六项全绿。
- CI清单/隔离政策自检通过；产品/构建/测试源码的暂存差异格式检查通过。两份既有浏览器Markdown的4处双空格为显式换行，未为消除格式提示改写原记录。
- 入口清单中只有本次获授权改写的 `docs/handoff.md`、`docs/next-session-prompt.md` 发生额外变化；原产品及浏览器文件保持接手字节。暂存93个文件，扫描无新增ZIP、DLL、EXE、PDB、截图、转储、原始日志或证据目录。

## 失败与限制

首次全索引导出遇到旧历史证据的Windows长路径，退出1；改为只导出构建必需的329文件。首次受限工具链配置也退出1；使用完整本机工具链权限的新构建目录完成上述结果，不改安装、依赖或产品代码。原失败日志保留在本轮本地目录。

没有重新运行真实桌面、ASAN、Runtime、Mesa、全矩阵、远程CI或物理条件专项，也不把旧专项结果计为本次新测试。已授权的推送仅当前开发分支，提交带 `[skip ci]`；未合并main或执行workflow_dispatch。交接提交身份以实际Git为准，本报告不回填历史包。

本轮临时源码副本、构建树和生成图片样本在核验后清理，只保留本地文本结果和入口保护；历史材料不动。当前未提交的 `.gitattributes` 历史证据规则与旧未跟踪材料继续留在本地。旧专项结果与下一轮范围见[当前交接](../handoff-current.md)。
