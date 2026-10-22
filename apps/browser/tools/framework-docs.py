from pathlib import Path
import shutil
APP=Path(__file__).resolve().parents[1]
FRAME=Path(r'D:\应用软件框架\应用层序框架')
BASE=APP/'.work'/'framework-baseline'
sections={
'docs/version-policy.md':'''## 2026-10-20 受限文档与内置浏览器开发增补

本轮追加独立 light_document.h 与 app_data.h 及对应导出，不改变既有公开结构、SDK/API/标准0.9-dev/9/9、ABI1或包格式1，不覆盖冻结SDK。新受限文档后端永久关闭脚本与应用桥；原可信Light和可选WebView2路径保留。应用复制数据投递复用既有每实例8MiB预算。

browser.uapp 以普通应用机制作为框架自带包；宿主启动不打开它、不请求网页。网络、阅读历史和工具栏在 apps/browser；宿主只定位同目录包并调用 workspace。交付必须匹配同一源码快照的头文件、导入库、DLL和包。HEAD仍为接手基准，当前工作树包含既有与本轮未提交修改；不虚构提交。详见[公共合同与支持表](restricted-browser.md)及 apps/browser/docs/validation.md。''',
'docs/build-and-validation.md':'''## 2026-10-20 内置 browser.uapp 构建

默认独立宿主包含 apps/browser 源码并生成同目录 browser.uapp。浏览器包不依赖测试目标、Mesa或WebView2：

```powershell
cmake -S . -B build/browser-release -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_BUILD_TESTS=OFF -DBROWSER_BUILD_CHECKS=OFF -DUI_FRAMEWORK_ENABLE_WEBVIEW2=OFF -DUI_OSMESA_LIBRARY=
cmake --build build/browser-release --target framework_host browser_package uapp_pack
```

开发时 UI_BROWSER_SOURCE_DIR 可指向独立应用工作区。匹配SDK支持应用独立构建：在应用源码使用 UI_BROWSER_SDK_DIR，读取SDK的公共头文件、import lib、DLL、打包器及固定Lexbor静态库，不读取框架源码或QuickJS源码。见[应用构建命令](../apps/browser/README.md)。

应用验证开关 BROWSER_BUILD_CHECKS 默认OFF；开启后提供纯公共C的 document/workspace/failure/url 调用方。宿主内部入口夹具另列，不替代纯公共调用方。真实输入仅在协调空闲桌面后串行执行；物理DPI、多屏、IME和长期人工单列。已有测试/固定依赖/冻结SDK保留，未恢复历史兼容专项。''',
'docs/standalone-host.md':'''## 2026-10-20 内置浏览网页入口

宿主选项菜单首项“浏览网页”定位 framework_host.exe 同目录的 browser.uapp，通过既有 open_path/workspace 打开。用户无需选择应用包；每次打开一个普通多实例标签。应用默认不随宿主启动、不恢复网页会话、不产生后台网络请求。原加号的打开包行为保留。

浏览器不创建第二层标签栏。标题、主题、切换、关闭等待、卸载继续使用公共应用机制；页面和来源限制见[支持表](restricted-browser.md)。包缺失仍走正常宿主加载错误，不回退到其他浏览器内核。''',
'docs/application-development-standard.md':'''## 2026-10-20 受限文档公共 C 增补

可信应用HTML继续使用 ui_light_web_backend_create/load_html；下载页面必须使用独立的 ui_light_document_backend_create 与 light_document.h。该后端不创建JS runtime，拒绝可信加载与JSON应用桥，只交付结构化导航/GET字段和资源事件，不能从页面升级成可信模式。窗口、呈现、输入和图片仍复用公共C接口。

后台结果通过 app_data.h 的复制投递交给应用自己的UI接收函数，沿用workspace稳定ID、线程、所有权、预算和WAIT关闭合同。应用及调用示例见[受限浏览器合同](restricted-browser.md)与[公共验证源码](../apps/browser/tests/document_public.c)。新增独立结构读取要求完整size；冻结SDK不变，使用匹配开发快照。'''
}
for name,block in sections.items():
    p=FRAME/name;old=p.read_bytes();marker=block.splitlines()[0].encode('utf-8')
    if marker in old:continue
    saved=BASE/name;saved.parent.mkdir(parents=True,exist_ok=True)
    if not saved.exists():shutil.copyfile(p,saved)
    newline=b'\r\n' if b'\r\n' in old else b'\n'
    p.write_bytes(old+newline+newline+block.replace('\n','\r\n' if newline==b'\r\n' else '\n').encode('utf-8')+newline)
shutil.copyfile(APP/'docs'/'browser.md',FRAME/'docs'/'restricted-browser.md')
print('Appended only browser-related policy/build/host/public-contract notes.')
