"""Assemble this batch separately; no old delivery or SDK is replaced."""
from pathlib import Path
import argparse, difflib, hashlib, json, shutil, zipfile

APP=Path(__file__).resolve().parents[1]
FRAME=Path(r'D:\应用软件框架\应用层序框架')
WORK=APP/'.work'/'m01'
CANDIDATE=WORK/'sdk-candidate'
RELEASE=WORK/'release'
FINAL=APP/'deliverables'/'m01-20261020'

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def copy(a,b):b.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(a,b)
def manifest(root):
    (root/'SHA256SUMS.txt').write_text(''.join(f'{sha(p)}  {p.relative_to(root).as_posix()}\n' for p in sorted(root.rglob('*')) if p.is_file() and p.name!='SHA256SUMS.txt'),encoding='utf-8')
def archive(root,out):
    assert not out.exists(),out
    with zipfile.ZipFile(out,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as z:
        for p in sorted(root.rglob('*')):
            if p.is_file():z.write(p,root.name+'/'+p.relative_to(root).as_posix())
def candidate():
    assert not CANDIDATE.exists(),CANDIDATE
    production=APP/'build'/'m01-production'
    for name in ('framework_host.exe','ui_framework.dll','uapp_pack.exe','browser.uapp'):copy(production/name,CANDIDATE/'bin'/name)
    copy(production/'ui_framework_runtime.lib',CANDIDATE/'lib'/'ui_framework_runtime.lib')
    for p in (FRAME/'include').rglob('*.h'):copy(p,CANDIDATE/'include'/p.relative_to(FRAME/'include'))
    for p in (FRAME/'.deps'/'lexbor'/'source'/'lexbor').rglob('*.h'):copy(p,CANDIDATE/'third_party'/'lexbor'/'include'/'lexbor'/p.relative_to(FRAME/'.deps'/'lexbor'/'source'/'lexbor'))
    copy(production/'light_web'/'lexbor'/'lexbor_static.lib',CANDIDATE/'third_party'/'lexbor'/'lib'/'lexbor_static.lib')
    for dep,title in (('lexbor','Lexbor'),('quickjs','QuickJS-NG')):copy(FRAME/'.deps'/dep/'LICENSE',CANDIDATE/'licenses'/f'{title}-LICENSE.txt')
    copy(APP/'deliverables'/'browser-sdk-20261020'/'bin'/'vcruntime140.dll',CANDIDATE/'bin'/'vcruntime140.dll')
    print(CANDIDATE)
def stage():
    assert not RELEASE.exists(),RELEASE
    RELEASE.mkdir(parents=True)
    sdk=RELEASE/'browser-sdk-m01';runtime=RELEASE/'browser-runtime-m01'
    shutil.copytree(CANDIDATE,sdk)
    copy(APP/'build'/'m01-sdk-only'/'browser.uapp',sdk/'bin'/'browser.uapp')
    for name in ('document','url','settings','failure','workspace','m01'):
        copy(APP/'build'/'m01-checks'/f'browser_{name}_check.exe',sdk/'verify'/f'browser_{name}_check.exe')
    for name in ('browser.md','m01-support.md','m01-validation.md'):copy(APP/'docs'/name,sdk/'docs'/name)
    copy(APP/'README.md',sdk/'README.md')
    shutil.copytree(sdk/'bin',runtime);shutil.copytree(sdk/'licenses',runtime/'licenses');shutil.copytree(sdk/'docs',runtime/'docs')
    copy(APP/'README.md',runtime/'README.md');copy(sdk/'bin'/'browser.uapp',RELEASE/'browser.uapp')
    source=WORK/'browser-source-m01';assert not source.exists();source.mkdir()
    for name in ('CMakeLists.txt','README.md','.gitignore'):copy(APP/name,source/name)
    for folder in ('src','resources','tests','tools','docs'):
        for p in (APP/folder).rglob('*'):
            if p.is_file() and '__pycache__' not in p.parts:copy(p,source/p.relative_to(APP))
    manifest(source);archive(source,RELEASE/'browser-source-m01.zip')
    fs=WORK/'ui-framework-source-m01';assert not fs.exists();fs.mkdir()
    for name in ('CMakeLists.txt','README.md','CHANGELOG.md','.gitignore','.gitattributes'):
        if (FRAME/name).exists():copy(FRAME/name,fs/name)
    for folder in ('src','include','examples','cmake','tools','tests','apps','.github'):
        for p in (FRAME/folder).rglob('*'):
            if p.is_file() and not any(x in ('.git','.deps','build','__pycache__') for x in p.relative_to(FRAME).parts):copy(p,fs/p.relative_to(FRAME))
    for p in (FRAME/'docs').glob('*.md'):copy(p,fs/'docs'/p.name)
    manifest(fs);archive(fs,RELEASE/'ui-framework-source-m01.zip')
    baseline=json.loads((WORK/'entry'/'baseline.json').read_text(encoding='utf-8'));changes=[]
    before=WORK/'entry'/'mirror';after=FRAME/'apps'/'browser'
    paths={p.relative_to(before).as_posix() for p in before.rglob('*') if p.is_file()}|{p.relative_to(after).as_posix() for p in after.rglob('*') if p.is_file() and '__pycache__' not in p.parts}
    for name in sorted(paths):
        a=before/name;b=after/name
        if a.exists() and b.exists() and sha(a)==sha(b):continue
        changes.extend(difflib.unified_diff(a.read_text(encoding='utf-8-sig').splitlines(True) if a.exists() else [],b.read_text(encoding='utf-8-sig').splitlines(True) if b.exists() else [],fromfile='a/apps/browser/'+name if a.exists() else '/dev/null',tofile='b/apps/browser/'+name if b.exists() else '/dev/null'))
    (RELEASE/'framework-m01.patch').write_text(''.join(changes),encoding='utf-8')
    identity={'product':'browser.uapp','version':'0.2.0','milestones':['M0','M1'],'application_git':'none','framework_head':'689fa03b34f8c4321a6c838f52ca9dcd790a9319','framework_branch':'codex/p1-process-boundaries','framework_dirty':True,'sdk':'0.9.0-dev','api':9,'abi':1,'package':1,'lexbor':'7fb22cf5664a331d7c24b113489e566767c9c25a','quickjs_ng':'2f0aa72a6b09cf69ff06399bcf9f4c083ac1a278','application_manifest_sha256':sha(source/'SHA256SUMS.txt'),'framework_manifest_sha256':sha(fs/'SHA256SUMS.txt'),'baseline_warning':'Already-dirty entry; M01 patch covers apps/browser only. Historical baseline is not clean HEAD.','entry_mirror_sha256':baseline['mirror']}
    for root in (RELEASE,sdk,runtime):
        (root/'SOURCE-IDENTITY.json').write_text(json.dumps(identity,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    for root in (sdk,runtime):manifest(root);archive(root,RELEASE/(root.name+'.zip'))
    print(RELEASE)
def finalize():
    assert not FINAL.exists(),FINAL
    evidence=json.loads((RELEASE/'verification.json').read_text(encoding='utf-8'))
    assert evidence['verified'] and evidence['source_rebuild_exit']==0
    FINAL.mkdir(parents=True)
    for p in RELEASE.iterdir():
        if p.is_file():copy(p,FINAL/p.name)
    index=['# browser.uapp 0.2.0 · M0/M1\n','本批独立交付；既有正式包保持原样。运行包解压后启动framework_host.exe，通过＋/应用菜单“浏览网页”打开。来源设置在工具栏“设”，诊断在“诊”；默认不随宿主启动。\n','源码维护入口为应用工程；框架 apps/browser 为审阅摘要保护的集成镜像。本批无新增公共ABI；SDK与DLL来自本批测试OFF、WebView2 OFF、Mesa空配置构建。\n','M2—M5后续；公网瞬态12002与早期OLE恢复崩溃底层原因未定位。物理DPI、多屏、真实IME、长期人工及任意恶意网站安全性未验证。细节见归档docs/m01-validation.md及verification.json。\n','| 文件 | SHA256 |\n| --- | --- |']
    index.extend(f'| [{p.name}]({p.name}) | `{sha(p)}` |' for p in sorted(FINAL.iterdir()) if p.is_file())
    (FINAL/'INDEX.md').write_text('\n'.join(index)+'\n',encoding='utf-8');manifest(FINAL)
    print(FINAL)
if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('mode',choices=['candidate','stage','final']);a=parser.parse_args()
    {'candidate':candidate,'stage':stage,'final':finalize}[a.mode]()
