from pathlib import Path
import json,hashlib,shutil
from PIL import Image
root=Path.cwd();work=root/'build/language-20261008';out=root/'docs/validation/instance-language-evidence-20261008';raw=out/'raw';raw.mkdir(parents=True,exist_ok=True)
# Failures stay distinct from the final run. Complete original logs remain in the new build directory.
for p in work.iterdir():
 if p.is_file() and p.suffix in ('.log','.xml','.json','.cmd','.py') and p.name not in ('package-sdk.py','stage-sdk.py'):
  shutil.copy2(p,raw/p.name)
for provider in ['light','webview2']:
 runs=list((work/'after').glob('language-evidence-'+provider+'-*'));run=max(runs,key=lambda p:p.stat().st_mtime)
 for p in run.glob('*.log'):shutil.copy2(p,raw/(provider+'-'+p.name))
 shutil.copy2(run/'manifest.json',raw/(provider+'-manifest.json'))
manifest=[]
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
base=work/'baseline-profile/before.bmp';image=out/'before-api8.png';Image.open(base).save(image);manifest.append({'image':image.name,'source':str(base),'source_sha256':sha(base),'sha256':sha(image),'baseline_commit':'cf0e83a3b3d478cfeba27fb841b185e4e80cf631','size':[1200,800],'dpi':96,'theme':'light'})
for provider in ['light','webview2']:
 run=max((work/'after').glob('language-evidence-'+provider+'-*'),key=lambda p:p.stat().st_mtime)
 for name in ['english-full','chinese-full','english-dark','chinese-dark','english-dual','chinese-dual','chinese-assistant','chinese-narrow','empty-system','empty-after-close']:
  bmp=run/'dpi-96'/(name+'.bmp');png=out/(provider+'-'+name+'.png');im=Image.open(bmp);im.save(png);manifest.append({'image':png.name,'source':str(bmp),'source_sha256':sha(bmp),'sha256':sha(png),'size':list(im.size),'dpi':96,'theme':'dark' if 'dark' in name else 'light','product_commit':'6d887703ca1e5bde0cc1e19ea2129d9eb0645a27'})
 for dpi in [144,192]:
  bmp=run/('dpi-'+str(dpi))/'chinese-full.bmp';png=out/(provider+'-chinese-dpi'+str(dpi)+'.png');im=Image.open(bmp);im.save(png);manifest.append({'image':png.name,'source':str(bmp),'source_sha256':sha(bmp),'sha256':sha(png),'size':list(im.size),'dpi':dpi,'theme':'light','product_commit':'6d887703ca1e5bde0cc1e19ea2129d9eb0645a27'})
(out/'screenshots.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2),encoding='utf-8');print('Actual window screenshots',len(manifest),'lossless BMP to PNG; no generated images')
