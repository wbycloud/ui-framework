from pathlib import Path
import datetime, hashlib, json, shutil, subprocess, uuid, zipfile

root = Path.cwd().resolve()
work = root / 'build/stability-state-20261008'
sdk = work / 'sdk'
snapshot = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=root, text=True).strip()
assert not subprocess.check_output(['git', 'status', '--porcelain'], cwd=root)
def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()
def save(path, value):
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')

frozen = json.loads((root / 'docs/validation/api8-stability-state-evidence-20261008/artifact-identity.json').read_text(encoding='utf-8'))
for item in frozen['artifacts']:
    assert sha(root / item['path']) == item['sha256'], item['path']
for p in (sdk / 'include/ui_framework').glob('*.h'):
    assert sha(p) == sha(root / 'include/ui_framework' / p.name)
for name in ('ui_framework.dll', 'framework_host.exe', 'uapp_pack.exe', 'stateful_components.uapp', 'ui_stateful_components_test.exe', 'ui_host_repaint_test.exe'):
    assert sha(sdk / 'bin' / name) == sha(work / 'after' / name)
assert sha(sdk / 'lib/ui_framework_runtime.lib') == sha(work / 'after/ui_framework_runtime.lib')
assert not (sdk / 'bin/opengl32.dll').exists()
(sdk / 'examples').mkdir(exist_ok=True)
shutil.copy2(work / 'sdk-consumer.c', sdk / 'examples/sdk-consumer.c')
(sdk / 'README.md').write_text('''# API8 Windows x64 本地开发 SDK

SDK0.8.0-dev / API8 / 开发标准8 / 应用ABI1 / 包格式1。未发布，未推送。
产品提交 f0b161c，示例和测试提交 d24cdf3；完整源码与文档快照见 identity.json。
include、lib、bin 来自同一当前构建；sha256.json 记录每个文件的身份，不包含自身。
原生捕获及异步轨道修复保留。只维护当前版本，不承诺历史 SDK 或旧包兼容。

## 实际完整应用

在本目录运行 PowerShell：

```powershell
$env:UI_API7_OSMESA_DLL=(Resolve-Path ./bin/osmesa.dll).Path
$env:UI_API7_BACKEND='light' # webview2 需要已安装的真实 Runtime
$env:UI_STATE_EXAMPLE_HOME="$env:LOCALAPPDATA/UiFrameworkStateExample"
$env:UI_STATE_EXAMPLE_PROFILE='A'
./bin/framework_host.exe ./bin/stateful_components.uapp
```

State 菜单提供显式保存、加载、单列重置、全表及布局重置。
应用拥有 A/B 稳定档案和独占锁，UST1 包含已有 UCW1 与 ULYT；不写应用包或业务文档。
关闭不自动覆盖文件。完整时机、异常与实例合同见 source/examples/stateful_components/README.md。
示例实际生成 OSMesa MSAA frame 并读回，不是模拟图片。

## 源码与依赖

固定 Lexbor、QuickJS 源码及 WebView2 SDK 位于 source/.deps，OSMesa24.3.4 x64 位于 bin。
MSVC x64 C11 开发终端可离线构建：

```powershell
cmake -S ./source -B ./build -G Ninja -DCMAKE_BUILD_TYPE=Release -DUI_FRAMEWORK_ENABLE_WEBVIEW2=ON "-DUI_OSMESA_LIBRARY=$((Resolve-Path ./bin/osmesa.dll).Path)"
cmake --build ./build
```

MSVC19.50、Windows SDK10.0.26100、CMake/Ninja 是构建环境，不随包安装。
WebView2 SDK1.0.4129.50 的 Loader 随包；真实 Runtime 必须另行安装，本轮实际验证154.0.4258.62。
Windows 有窗口 GL 使用本机 OpenGL；本包不部署软件 opengl32.dll 到宿主或 Runtime。
许可证位于 licenses。静态库使用 lib 内匹配的 Lexbor/QuickJS；共享调用使用 ui_framework_runtime.lib 和 bin/ui_framework.dll。

## 校验与消费

外部交付记录保存 ZIP SHA256、CRC、逐文件校验、解包后的实际 C 调用和真实鼠标保存／重启恢复。
examples/sdk-consumer.c 可在 x64 开发终端以 /std:c11 /utf-8 /MD /DUI_FRAMEWORK_BUILD_SHARED、/Iinclude 和 lib/ui_framework_runtime.lib 编译，输出放 bin 后运行。
源码验证记录在 source/docs/validation/api8-stability-state-validation.md；归档后的证明单独记录，避免归档哈希自引用。
连续重绘历史来源未定位，业务原套件29/30的固定坐标失败保留。
当前源码 Session0、严格整机无登录 runner、托管CI、真实IME、物理跨屏／桌面边缘合成、长期人工及正式业务修改仍待环境或授权。
程序 DPI、自动截图和独立框架观察不能替代这些验收。
''', encoding='utf-8')
save(sdk / 'identity.json', {
    'sdk':'0.8.0-dev', 'api':8, 'standard':8, 'application_abi':1, 'package_format':1,
    'product_commit':frozen['product_commit'], 'example_test_commit':frozen['example_test_commit'],
    'source_document_snapshot':snapshot, 'local_unpublished':True,
    'dependencies':json.loads((work / 'dependencies-final.json').read_text(encoding='utf-8-sig')),
    'external_archive_proof':'Generated after this source snapshot; no self-referential archive hash.'
})
files = []
for p in sorted(sdk.rglob('*')):
    assert not p.is_symlink(), p
    if p.is_file() and p != sdk / 'sha256.json':
        assert '.git' not in p.relative_to(sdk).parts, p
        files.append({'path':p.relative_to(sdk).as_posix(), 'bytes':p.stat().st_size, 'sha256':sha(p)})
save(sdk / 'sha256.json', {'algorithm':'SHA256','excluded_self':'sha256.json','files':files})
archive = work / 'ui-framework-sdk0.8.0-dev-api8-f0b161c-windows-x64.zip'
with zipfile.ZipFile(archive, 'x', zipfile.ZIP_DEFLATED, compresslevel=6) as z:
    for p in sorted(sdk.rglob('*')):
        if p.is_file(): z.write(p, 'sdk/' + p.relative_to(sdk).as_posix())
verify = work / ('sdk-verify-' + uuid.uuid4().hex[:12])
verify.mkdir()
with zipfile.ZipFile(archive) as z:
    assert z.testzip() is None
    for item in z.infolist():
        target = (verify / item.filename).resolve()
        assert target.is_relative_to(verify.resolve()), item.filename
    z.extractall(verify)
extracted = verify / 'sdk'
manifest = json.loads((extracted / 'sha256.json').read_text(encoding='utf-8'))
assert len(manifest['files']) == len(files)
for item in manifest['files']:
    p = extracted / item['path']
    assert p.stat().st_size == item['bytes'] and sha(p) == item['sha256'], item['path']
assert sha(extracted / 'sha256.json') == sha(sdk / 'sha256.json')
result = subprocess.run([str(extracted / 'bin/sdk-consumer.exe')], cwd=extracted / 'bin', stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
(work / 'sdk-extracted-consumer.log').write_bytes(result.stdout)
assert result.returncode == 0, result.stdout
matched_business = {}
for name in ('ui_framework.dll', 'framework_host.exe'):
    business = work / 'business-current/framework' / name
    assert sha(business) == sha(sdk / 'bin' / name)
    matched_business[name] = sha(business)
save(work / 'sdk-archive.json', {
    'created_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),
    'archive':str(archive), 'bytes':archive.stat().st_size, 'sha256':sha(archive),
    'source_document_snapshot':snapshot, 'product_commit':frozen['product_commit'],
    'example_test_commit':frozen['example_test_commit'], 'files_including_manifest':len(files)+1,
    'sha256_manifest':sha(sdk / 'sha256.json'), 'crc':'PASS', 'extracted_directory':str(extracted),
    'per_file_verification':'PASS', 'extracted_public_c_consumer':'PASS',
    'business_framework_bin_matches_delivery':matched_business,
    'extracted_real_host_state':'PENDING next serial desktop verification'
})
(work / (archive.name + '.sha256')).write_text(sha(archive) + '  ' + archive.name + '\n', encoding='ascii')
print(json.dumps({'files':len(files)+1, 'bytes':archive.stat().st_size, 'sha256':sha(archive), 'source_snapshot':snapshot, 'extracted':str(extracted)}, ensure_ascii=False))
