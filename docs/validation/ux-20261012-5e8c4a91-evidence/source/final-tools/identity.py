import pathlib,json,subprocess,hashlib
root=pathlib.Path(__file__).resolve().parents[3];base=root/'build/ux-20261012-5e8c4a91';out=root/'docs/validation/ux-20261012-5e8c4a91-evidence'
def git(*args):return subprocess.check_output(['git',*args],cwd=root)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
diff=git('diff','--binary','HEAD');(base/'raw/final-working-tree.patch').write_bytes(diff)
files=git('diff','--name-only','HEAD').decode('utf-8').splitlines();records=[]
for name in files:
    p=root/name;target=out/'source/current'/name;target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(p.read_bytes());records.append({'path':name,'sha256':sha(p),'bytes':p.stat().st_size})
products=[]
for build in ['baseline','pre-perf','optimized','final2-snapshot','final']:
    for name in ['ui_framework.dll','framework_host.exe','ui_framework_runtime.lib','stateful_components.uapp','ux_probe.exe']:
        p=base/build/name
        if p.exists():products.append({'path':str(p.relative_to(root)),'sha256':sha(p),'bytes':p.stat().st_size})
current={p['path'].split('\\')[-1]:p['sha256'] for p in products if '\\final\\' in p['path']}
v4=json.loads((base/'runs/perf-after-v4-runtime-96/run.json').read_text())
for name in ['ui_framework.dll','stateful_components.uapp']:
    assert current[name]==next(v for k,v in v4['identities'].items() if k.endswith(name))
doc=root/'docs/validation/ux-validation-20261012.md'
result={'head':git('rev-parse','HEAD').decode().strip(),'branch':git('branch','--show-current').decode().strip(),'uncommitted':True,
 'diffSha256':hashlib.sha256(diff).hexdigest(),'sourceFiles':records,'products':products,'publicHeadersChanged':any(n.startswith('include/') for n in files),
 'final4FrameworkAndPackageEqualFinal5':True,'reportSha256':sha(doc),'note':'final5 only removes obsolete private kind5 show-mode branch; exact probe identities remain per run'}
(out/'final-identity.json').write_text(json.dumps(result,ensure_ascii=False,indent=2),encoding='utf-8');print('identity',result['diffSha256'],'headersChanged',result['publicHeadersChanged'])
