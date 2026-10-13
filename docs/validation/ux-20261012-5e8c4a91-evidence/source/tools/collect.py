import pathlib,hashlib,json,shutil,re
from PIL import Image
root=pathlib.Path(__file__).resolve().parents[3];base=root/'build/ux-20261012-5e8c4a91';out=root/'docs/validation/ux-20261012-5e8c4a91-evidence'
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def copy(p,target):
    target.parent.mkdir(parents=True,exist_ok=True)
    if target.exists():
        if digest(p)==digest(target):return
        # Current generated summaries can be refreshed until the index is sealed.
        if target.name not in ['performance-summary.json','performance-summary.txt']:raise ValueError(str(target))
    shutil.copyfile(p,target)
inventory=[]
for directory in sorted((base/'runs').iterdir()):
    if not (directory/'run.json').exists():continue
    result=json.loads((directory/'run.json').read_text());result['bmp_count']=len(list(directory.glob('*.bmp')));inventory.append(result)
    for file in directory.iterdir():
        if file.suffix in ['.json','.csv','.log']:copy(file,out/'raw/runs'/directory.name/file.name)
    for file in directory.glob('*.bmp'):
        target=out/'screenshots'/directory.name/(file.stem+'.png');target.parent.mkdir(parents=True,exist_ok=True)
        if not target.exists():
            with Image.open(file) as original:
                rgb=original.convert('RGB');rgb.save(target)
                with Image.open(target) as check:assert check.convert('RGB').tobytes()==rgb.tobytes()
for file in (base/'raw').iterdir():
    if file.is_file() and file.suffix in ['.log','.json','.txt','.xml','.c','.inc','.py']:copy(file,out/'raw'/file.name)
for file in (base/'tools').iterdir():
    if file.is_file():copy(file,out/'source/tools'/file.name)
(out/'run-inventory.json').write_text(json.dumps(inventory,ensure_ascii=False,indent=2),encoding='utf-8')
print('runs',len(inventory),'all_natural_zero',sum(r['os_exit']==0 and not r['forced'] for r in inventory),'screenshots',len(list((out/'screenshots').rglob('*.png'))))
