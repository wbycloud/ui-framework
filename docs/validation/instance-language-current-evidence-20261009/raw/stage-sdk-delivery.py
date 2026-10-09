from pathlib import Path
import hashlib,json,shutil,subprocess

root=Path.cwd().resolve()
work=root/'build/language-current-20261009'
sdk=work/'sdk-delivery'
current=work/'current'
assert not sdk.exists()
assert not subprocess.check_output(['git','status','--porcelain'],cwd=root)
commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=root,text=True).strip()
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
before=json.loads((work/'artifacts-before.json').read_text(encoding='utf-8-sig'))
for item in before['artifacts']:
    assert sha(Path(item['path']))==item['sha256'],item['path']
for directory in ['source','include/ui_framework','bin','lib','licenses','examples','providers/source']:
    (sdk/directory).mkdir(parents=True,exist_ok=True)
for name in subprocess.check_output(['git','ls-files','-z'],cwd=root).decode('utf-8').split('\0'):
    if name:
        source=root/name
        assert source.is_file() and not source.is_symlink(),name
        dest=sdk/'source'/name;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(source,dest)
dependencies={'lexbor':'7fb22cf5664a331d7c24b113489e566767c9c25a','quickjs':'2f0aa72a6b09cf69ff06399bcf9f4c083ac1a278'}
for name,expected in dependencies.items():
    dep=root/'.deps'/name
    assert subprocess.check_output(['git','rev-parse','HEAD'],cwd=dep,text=True).strip()==expected
    assert not subprocess.check_output(['git','diff','--name-only'],cwd=dep)
    for filename in subprocess.check_output(['git','ls-files','-z'],cwd=dep).decode('utf-8').split('\0'):
        if filename and (dep/filename).is_file():
            dest=sdk/'source/.deps'/name/filename;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(dep/filename,dest)
web=root/'.deps/Microsoft.Web.WebView2.1.0.4129.50'
shutil.copytree(web,sdk/'source/.deps'/web.name)
for p in (root/'include/ui_framework').glob('*.h'):shutil.copy2(p,sdk/'include/ui_framework'/p.name)
for name in ['ui_framework.dll','framework_host.exe','uapp_pack.exe','ui_headless_example.exe','stateful_components.uapp','api7_fixture.uapp','framework_features.uapp','generic_components.uapp','web_counter.uapp','minimal_eda.uapp','ui_stateful_components_test.exe','ui_host_repaint_test.exe','ui_instance_language_test.exe','ui_language_core_test.exe']:
    shutil.copy2(current/name,sdk/'bin'/name)
for name in ['ui_framework_runtime.lib','ui_framework.lib','ui_framework_headless.lib']:shutil.copy2(current/name,sdk/'lib'/name)
for p in current.rglob('*.lib'):
    if p.name in ['lexbor_static.lib','qjs.lib']:shutil.copy2(p,sdk/'lib'/p.name)
assert (sdk/'lib/lexbor_static.lib').is_file() and (sdk/'lib/qjs.lib').is_file()
for name in ['osmesa.dll','libglapi.dll']:shutil.copy2(root/'.deps/mesa-24.3.4/x64'/name,sdk/'bin'/name)
shutil.copy2(web/'build/native/x64/WebView2Loader.dll',sdk/'bin/WebView2Loader.dll')
for p in root.glob('LICENSE*'):shutil.copy2(p,sdk/'licenses'/p.name)
for name in dependencies:
    for pattern in ['LICENSE*','COPYING*']:
        for p in (root/'.deps'/name).glob(pattern):shutil.copy2(p,sdk/'licenses'/(name+'-'+p.name))
for p in web.glob('*LICENSE*'):shutil.copy2(p,sdk/'licenses'/('WebView2-'+p.name))
shutil.copy2(root/'.deps/mesa-24.3.4-source/mesa-24.3.4/docs/license.rst',sdk/'licenses/Mesa-license.rst')
shutil.copy2(root/'.deps/mesa-24.3.4-source.tar.xz',sdk/'providers/source/mesa-24.3.4-source.tar.xz')
shutil.copy2(root/'build/language-20261008/sdk-consumer.c',sdk/'examples/sdk-consumer.c')
(sdk/'identity.json').write_text(json.dumps({'sdk':'0.9.0-dev','api':9,'standard':9,'application_abi':1,'package_format':1,'source_document_snapshot':commit,'last_product_commit':'07a66a82370db93c49a66c90db5e217a8812e60e','source_dirty':False,'build':'fresh current-source Release MSVC x64 C11','dependencies':dependencies,'webview2_sdk':'1.0.4129.50','osmesa':'24.3.4','published':False,'proof':'Archive and actual extracted validation recorded externally after packaging.'},ensure_ascii=False,indent=2),encoding='utf-8')
(sdk/'README.md').write_text('''# API9 Windows x64 当前开发快照

SDK0.9.0-dev/API9/标准9，应用ABI1/包格式1。此SDK是本地开发交付，未发布稳定版本。精确源码/文档commit在identity.json，include、导入库、共享DLL、宿主、完整示例和测试匹配该快照；不能只凭API数字混用另一份开发DLL。逐文件清单sha256.json不包含自身；归档哈希与解包验证另附外部证明。

## 完整应用与应用拥有的中英文

```powershell
$env:UI_API7_OSMESA_DLL=(Resolve-Path ./bin/osmesa.dll).Path
$env:UI_API7_BACKEND='light' # webview2需要已安装的真实Runtime
$env:UI_STATE_EXAMPLE_HOME="$env:LOCALAPPDATA/UiFrameworkStateExample"
$env:UI_STATE_EXAMPLE_PROFILE='A'
./bin/framework_host.exe ./bin/stateful_components.uapp
```

Language/语言菜单提交并保存本实例语言。create先读取稳定A/B档案UIL1，在首次呈现前提交；无有效偏好由示例明确选择英文，非法文件拒绝且不覆盖。第二实例可用B档案保持不同语言；框架不写语言文件，不提供全局覆盖开关。标签切换时宿主跟随活动host，无实例时查询Windows显示语言。State菜单保存/恢复UST1（UCW1列宽+ULYT布局），状态文件与语言文件独立。不重载DLL、内容HWND或GL，业务值/草稿/用户列宽不因翻译变化。接口/通知/线程/复制/关闭合同见source/docs/instance-language.md和source/examples/stateful_components/README.md。

## 固定依赖与离线源码构建

Lexbor、QuickJS固定源码和WebView2 SDK1.0.4129.50在source/.deps；OSMesa24.3.4在bin，Mesa源码归档和许可证另附。WebView2 Runtime是外部系统依赖，SDK不等于Runtime；本轮实际版本以外部验收报告为准。有窗口GL继续使用系统驱动，本SDK不部署替换opengl32.dll。

```powershell
# MSVC x64开发终端，CMake/Ninja和Windows SDK为外部构建环境
cmake -S ./source -B ./build -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_FRAMEWORK_ENABLE_WEBVIEW2=ON "-DUI_OSMESA_LIBRARY=$((Resolve-Path ./bin/osmesa.dll).Path)"
cmake --build ./build
ctest --test-dir ./build --output-on-failure
```

公共共享调用链接lib/ui_framework_runtime.lib并载入bin/ui_framework.dll。examples/sdk-consumer.c验证当前C ABI、语言/查询/通知/文本和列宽保留；以/std:c11 /W4 /WX /utf-8 /MD /DUI_FRAMEWORK_BUILD_SHARED、/Iinclude和匹配导入库编译，EXE放bin运行。

解包后实际语言套件：

```powershell
pwsh -NoProfile -File ./source/tests/run-instance-language.ps1 -BuildDirectory ./bin -Provider ./bin/osmesa.dll -Backend light
# Backend改webview2验证真实Runtime
```

真实宿主/完整DLL截图和本轮解包结果由归档外部证明提供，包内历史验收不能冒充本轮通过。原托管超时尚未根治，当前源码服务Session0及整机无登录、其他真实Windows显示语言、IME/物理跨屏/桌面合成/长期人工仍须分别验收。不修改系统显示语言或注销用户来测试。
''',encoding='utf-8')
assert not (sdk/'bin/opengl32.dll').exists()
print(json.dumps({'staging':str(sdk),'snapshot':commit,'files':sum(1 for p in sdk.rglob('*') if p.is_file())},ensure_ascii=False))
