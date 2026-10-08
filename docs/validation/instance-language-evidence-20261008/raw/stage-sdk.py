from pathlib import Path
import hashlib,json,shutil,subprocess
root=Path.cwd().resolve();work=root/'build/language-20261008';sdk=work/'sdk-api9';after=work/'after'
assert not sdk.exists(), 'Fresh delivery directory required'
assert not subprocess.check_output(['git','status','--porcelain'],cwd=root)
snapshot=subprocess.check_output(['git','rev-parse','HEAD'],cwd=root,text=True).strip()
frozen=json.loads((work/'product-artifacts.json').read_text(encoding='utf-8'))
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
for item in frozen['artifacts']:assert sha(root/item['path'])==item['sha256'],item['path']
for name in ('source','include/ui_framework','bin','lib','licenses','examples','providers/source'):(sdk/name).mkdir(parents=True,exist_ok=True)
for name in subprocess.check_output(['git','ls-files','-z'],cwd=root).decode('utf-8').split('\0'):
 if name:
  p=root/name;dest=sdk/'source'/name
  assert p.is_file() and not p.is_symlink(),name
  dest.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,dest)
for name,commit in json.loads((work/'dependencies-final.json').read_text(encoding='utf-8'))['source_commits'].items():
 dep=root/'.deps'/name
 assert subprocess.check_output(['git','rev-parse','HEAD'],cwd=dep,text=True).strip()==commit
 for filename in subprocess.check_output(['git','ls-files','-z'],cwd=dep).decode('utf-8').split('\0'):
  if filename and (dep/filename).is_file():
   dest=sdk/'source/.deps'/name/filename;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(dep/filename,dest)
web=root/'.deps/Microsoft.Web.WebView2.1.0.4129.50';shutil.copytree(web,sdk/'source/.deps'/web.name)
for p in (root/'include/ui_framework').glob('*.h'):shutil.copy2(p,sdk/'include/ui_framework'/p.name)
for name in ('ui_framework.dll','framework_host.exe','uapp_pack.exe','ui_headless_example.exe','stateful_components.uapp','api7_fixture.uapp','framework_features.uapp','generic_components.uapp','web_counter.uapp','minimal_eda.uapp','ui_stateful_components_test.exe','ui_host_repaint_test.exe','ui_instance_language_test.exe','ui_language_core_test.exe'):
 shutil.copy2(after/name,sdk/'bin'/name);assert sha(after/name)==sha(sdk/'bin'/name)
for name in ('ui_framework_runtime.lib','ui_framework.lib','ui_framework_headless.lib'):shutil.copy2(after/name,sdk/'lib'/name)
for p in after.rglob('*.lib'):
 if p.name in ('lexbor_static.lib','qjs.lib'):shutil.copy2(p,sdk/'lib'/p.name)
assert (sdk/'lib/lexbor_static.lib').exists() and (sdk/'lib/qjs.lib').exists()
for name in ('osmesa.dll','libglapi.dll'):shutil.copy2(root/'.deps/mesa-24.3.4/x64'/name,sdk/'bin'/name)
shutil.copy2(web/'build/native/x64/WebView2Loader.dll',sdk/'bin/WebView2Loader.dll')
for p in root.glob('LICENSE*'):shutil.copy2(p,sdk/'licenses'/p.name)
for name in ('lexbor','quickjs'):
 for pattern in ('LICENSE*','COPYING*'):
  for p in (root/'.deps'/name).glob(pattern):shutil.copy2(p,sdk/'licenses'/(name+'-'+p.name))
for p in web.glob('*LICENSE*'):shutil.copy2(p,sdk/'licenses'/('WebView2-'+p.name))
shutil.copy2(root/'.deps/mesa-24.3.4-source/mesa-24.3.4/docs/license.rst',sdk/'licenses/Mesa-license.rst')
shutil.copy2(root/'.deps/mesa-24.3.4-source.tar.xz',sdk/'providers/source/mesa-24.3.4-source.tar.xz')
shutil.copy2(work/'sdk-consumer.c',sdk/'examples/sdk-consumer.c')
assert not (sdk/'bin/opengl32.dll').exists()
(sdk/'README.md').write_text('''# API9 Windows x64 本地开发 SDK

SDK0.9.0-dev / API9 / 开发标准9 / 应用ABI1 / 包格式1。未推送、发布或创建标签。
产品6d887703ca1e5bde0cc1e19ea2129d9eb0645a27；源代码和文档快照见identity.json。
include、lib、bin匹配。sha256.json逐文件身份不包含自身；归档外部证明另附，避免自引用。
只维护当前API9，原捕获/轨道、预算、数据和卸载合同保持。

## 完整应用与应用拥有的语言

PowerShell从SDK根目录运行：

```powershell
$env:UI_API7_OSMESA_DLL=(Resolve-Path ./bin/osmesa.dll).Path
$env:UI_API7_BACKEND='light' # webview2须实际Runtime
$env:UI_STATE_EXAMPLE_HOME="$env:LOCALAPPDATA/UiFrameworkStateExample"
$env:UI_STATE_EXAMPLE_PROFILE='A'
./bin/framework_host.exe ./bin/stateful_components.uapp
```

Language菜单切换Chinese/English，示例立即保存自己的UIL1偏好；首次呈现前读取并提交本host。
换成档案B再开同一包可保持不同语言，宿主随活动标签；无实例时查询Windows显示语言。
A/B稳定身份和独占锁避免覆盖；不写包/业务文档。缺失偏好由示例明确选择英文并提交，损坏文件拒绝且保持。
State菜单保存/加载UST1（UCW1列宽+ULYT布局）、单列/全表与布局重置。
不重载DLL/HWND/GL，实际OSMesa MSAA帧和异步图片不由语言切换重建。
合同、时机、线程、复制所有权和异常见source/docs/instance-language.md及source/examples/stateful_components/README.md。

## 固定依赖与离线源码构建

Lexbor、QuickJS固定源码及WebView2 SDK1.0.4129.50在source/.deps；OSMesa24.3.4 x64在bin，Mesa源归档在providers/source。
许可证在licenses，提供方不需要网络下载。MSVC x64开发终端：

```powershell
cmake -S ./source -B ./build -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_FRAMEWORK_ENABLE_WEBVIEW2=ON "-DUI_OSMESA_LIBRARY=$((Resolve-Path ./bin/osmesa.dll).Path)"
cmake --build ./build
ctest --test-dir ./build --output-on-failure
```

MSVC19.50、CMake/Ninja及Windows SDK是外部构建环境。WebView2 Runtime需另行安装，实测154.0.4258.62。
有窗口GL使用Windows驱动，不部署替换的opengl32.dll。
共享调用链接lib/ui_framework_runtime.lib并使用bin/ui_framework.dll；静态调用使用匹配Lexbor/QuickJS及Win32库。

## 解包实际调用与验收边界

examples/sdk-consumer.c在x64开发终端以/std:c11 /utf-8 /MD /DUI_FRAMEWORK_BUILD_SHARED、/Iinclude和lib/ui_framework_runtime.lib编译，输出放bin后运行。
ui_instance_language_test.exe加载交付完整DLL包，真实SendInput验证两个后端、双实例、跨进程偏好和关闭。
本地矩阵61通过/1物理跳过；无Web21通过/1物理跳过；两个独立真实Runtime各32次重开pending0。
源验证及保留失败在source/docs/validation/instance-language-validation.md。归档后CRC、逐文件哈希、实际C调用、完整宿主与离线源码构建另附外部证明。
当前源码Session0缺服务权限，严格整机无登录runner缺失，托管CI未授权；真实IME、物理跨屏/桌面合成及长期人工待验。
真实系统仅zh-CN；其他映射只是单元测试，不修改机器语言。Runtime瞬态历史及持续重绘问题不宣称根治。没有业务应用试点。
''',encoding='utf-8')
(sdk/'identity.json').write_text(json.dumps({'sdk':'0.9.0-dev','api':9,'standard':9,'application_abi':1,'package_format':1,'product_commit':frozen['product_commit'],'source_document_snapshot':snapshot,'local_unpublished':True,'dependencies':json.loads((work/'dependencies-final.json').read_text(encoding='utf-8')),'external_archive_proof':'Created after source snapshot; archive hash is not self-referential.'},ensure_ascii=False,indent=2),encoding='utf-8')
print('Fresh SDK',sdk,'source snapshot',snapshot)
