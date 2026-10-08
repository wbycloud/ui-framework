from pathlib import Path
import datetime, hashlib, json, shutil
from PIL import Image
root=Path.cwd().resolve(); work=root/'build/stability-state-20261008'
e=root/'docs/validation/api8-stability-state-evidence-20261008'; raw=e/'raw'
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
proof=json.loads((work/'sdk-archive.json').read_text(encoding='utf-8-sig'))
proof['created_utc']=datetime.datetime.fromisoformat(proof['created_utc']).astimezone(datetime.timezone.utc).isoformat()
assert sha(Path(proof['archive']))==proof['sha256']
sdk=Path(proof['extracted_directory']); manifest=json.loads((sdk/'sha256.json').read_text(encoding='utf-8'))
for item in manifest['files']:
    p=sdk/item['path']; assert p.stat().st_size==item['bytes'] and sha(p)==item['sha256'], item['path']
assert sha(sdk/'sha256.json')==proof['sha256_manifest']
proof['post_desktop_per_file_verification']='PASS: all 2457 delivered files remain unchanged'
(work/'sdk-archive.json').write_text(json.dumps(proof,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
for name in ('sdk-archive.json','sdk-extracted-consumer.log','sdk-extracted-state.log','package-current-sdk.py','stage-sdk.py','verify-extracted-state.ps1','sdk-consumer.c','sdk-consumer-build.cmd'):
    shutil.copy2(work/name,raw/name)
for name in ('identity.json','sha256.json','README.md'):
    shutil.copy2(sdk/name,raw/('sdk-'+name))
archive=Path(proof['archive']); shutil.copy2(work/(archive.name+'.sha256'),raw/'sdk-archive.sha256')
state=Path(proof['extracted_real_host_evidence'])
for p in state.glob('*.log'):shutil.copy2(p,raw/('sdk-extracted-'+p.name))
shutil.copy2(state/'manifest.json',raw/'sdk-extracted-state-manifest.json')
pictures=json.loads((e/'screenshots.json').read_text(encoding='utf-8'))
for backend in ('light','webview2'):
    for mode in ('before','saved','restored'):
        original=state/backend/(mode+'.bmp'); dest=e/'images'/('sdk-extracted-'+backend+'-96-'+mode+'.png')
        with Image.open(original) as image:
            image.convert('RGB').save(dest)
            size=list(image.size)
        pictures.append({'path':dest.relative_to(e).as_posix(),'size':size,'sha256':sha(dest),'phase':'extracted_sdk_actual_'+mode,'origin':'actual running host/application; lossless BMP RGB conversion','archive_sha256':proof['sha256'],'original_bmp_sha256':sha(original)})
(e/'screenshots.json').write_text(json.dumps(pictures,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print('Post-desktop SDK hashes PASS; evidence added; actual screenshots',len(pictures))
