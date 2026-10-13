import pathlib,json,hashlib,shutil
root=pathlib.Path(__file__).resolve().parents[3]
base=root/'build/ux-20261012-5e8c4a91'
evidence=root/'docs/validation/ux-20261012-5e8c4a91-evidence'
destination=pathlib.Path('C:/Users/wbycl/.codex/artifacts/ui-framework-ux-20261012-5e8c4a91-recovery')
assert not destination.exists(), 'Never overwrite an existing recovery copy'
assert (evidence/'sha256.json').exists(), 'Copy only after evidence seal'
destination.mkdir(parents=True)
def digest(path):
    h=hashlib.sha256()
    with path.open('rb') as stream:
        for data in iter(lambda:stream.read(4*1024*1024),b''):h.update(data)
    return h.hexdigest()
files=[p for p in evidence.rglob('*') if p.is_file()]
files.append(root/'docs/validation/ux-validation-20261012.md')
identity=json.loads((evidence/'final-identity.json').read_text(encoding='utf-8'))
files.extend(root/r['path'] for r in identity['sourceFiles'])
files.extend(root/r['path'] for r in identity['products'])
files.extend(p for p in (base/'final').glob('*.dll') if p not in files)
files.append(base/'raw/final-working-tree.patch')
rows=[]
for p in sorted(set(files)):
    relative=p.relative_to(root)
    target=destination/relative
    target.parent.mkdir(parents=True,exist_ok=True)
    expected=digest(p)
    shutil.copyfile(p,target)
    assert digest(target)==expected, relative
    rows.append({'path':relative.as_posix(),'bytes':p.stat().st_size,'sha256':expected})
    if len(rows)%500==0:print('COPY_VERIFIED',len(rows),flush=True)
index=json.loads((evidence/'sha256.json').read_text(encoding='utf-8'))
assert all(digest(destination/evidence.relative_to(root)/r['path'])==r['sha256'] for r in index['files'])
result={'status':'passed','root':str(destination),'files':rows,'fileCount':len(rows),
    'bytes':sum(r['bytes'] for r in rows),'evidenceIndexVerified':True,
    'excludes':'Original ignored BMPs, all historical SDKs and business application; two CRC BMPs remain unchanged on D',
    'physicalMediaIndependenceEstablished':False}
(destination/'recovery-manifest.json').write_text(json.dumps(result,ensure_ascii=False,indent=2),encoding='utf-8')
print('RECOVERY_PASSED',len(rows),'files',result['bytes'],'bytes',flush=True)
