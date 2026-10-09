import hashlib,json,pathlib,shutil
from PIL import Image

root=pathlib.Path(__file__).resolve().parents[3];base=root/'build/p1-mesa-hosted-control-20261009'
out=root/'docs/validation/p1-mesa-hosted-control-evidence-20261009'
assert out.exists() and not list(out.iterdir())
def copy(source,target):
    target.parent.mkdir(parents=True,exist_ok=True)
    with source.open('rb') as src,target.open('xb') as dst:shutil.copyfileobj(src,dst)
def digest(path):
    sha=hashlib.sha256()
    with path.open('rb') as stream:
        for data in iter(lambda:stream.read(8*1024*1024),b''):sha.update(data)
    return sha.hexdigest()
for folder in ['raw','analysis','tools']:
    for path in (base/folder).iterdir():
        if path.suffix.lower() in {'.json','.jsonl','.log','.txt','.patch','.ps1','.py'}:
            copy(path,out/'raw'/folder/path.name)
for configuration in ['osmesa','webview2']:
    artifact=base/'artifacts'/configuration
    for path in artifact.rglob('*'):
        if path.is_file() and path.suffix.lower() in {'.log','.json','.jsonl','.txt','.xml','.cmake','.cmd','.bin'}:
            copy(path,out/'raw/artifacts'/configuration/path.relative_to(artifact))
    # Preserve failed range attempts, excluding signed URL response headers.
    for directory in (base/'downloads').glob(configuration+'*'):
        for path in directory.glob('*.json'):
            copy(path,out/'raw/transfers'/directory.name/path.name)
frames=[]
for configuration in ['osmesa','webview2']:
    product=base/'artifacts'/configuration/'lifecycle-control/product-evidence'
    manifests=[(p,json.loads(p.read_text(encoding='utf-8-sig'))) for p in product.glob('*/manifest.json')]
    language=next(p.parent for p,d in manifests if p.parent.name.startswith('language') and 'lifecycle-candidate' in d['provider'])
    storage=next(p.parent for p,d in manifests if p.parent.name.startswith('state') and 'lifecycle-candidate' in d['provider'])
    for name,source in [('storage-96',storage/'dpi-96/saved.bmp'),('language-96',language/'dpi-96/english-full.bmp'),('language-192',language/'dpi-192/english-full.bmp')]:
        target=out/'frames'/f'{configuration}-candidate-{name}.png';target.parent.mkdir(exist_ok=True)
        with Image.open(source) as original:
            original.load();original.save(target)
            with Image.open(target) as converted:
                assert original.mode==converted.mode and original.size==converted.size and original.tobytes()==converted.tobytes()
            frames.append({'frame':str(target.relative_to(out)),'source_bmp':str(source.relative_to(root)),
                'source_sha256':digest(source),'png_sha256':digest(target),'size':original.size,'mode':original.mode,
                'pixel_sha256':hashlib.sha256(original.tobytes()).hexdigest(),'conversion':'Lossless PNG, verified every decoded pixel; original BMP unchanged.'})
(out/'frame-origins.json').write_text(json.dumps({'frames':frames},indent=2),encoding='utf-8')
local=[]
for directory in [base/'artifacts',base/'local-object-A',base/'local-object-C']:
    for path in directory.rglob('*'):
        if path.is_file() and path.suffix.lower() in {'.dll','.exe','.pdb','.lib','.obj','.bmp','.dmp','.zip','.7z'}:
            local.append({'path':str(path.relative_to(root)).replace('\\','/'),'bytes':path.stat().st_size,'sha256':digest(path)})
for configuration in ['osmesa','webview2']:
    path=base/f'raw/windows-{configuration}-verified.zip'
    local.append({'path':str(path.relative_to(root)).replace('\\','/'),'bytes':path.stat().st_size,'sha256':digest(path)})
(out/'local-artifacts.json').write_text(json.dumps({'count':len(local),'files':local},indent=2),encoding='utf-8')
print(json.dumps({'new_evidence_files':len([p for p in out.rglob('*') if p.is_file()]),'png':len(frames),'local_artifacts':len(local)}),flush=True)
