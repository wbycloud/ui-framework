import pathlib,json,hashlib,shutil,re,time,xml.etree.ElementTree as ET
root=pathlib.Path(__file__).resolve().parents[3];base=root/'build/ux-20261012-5e8c4a91';out=root/'docs/validation/ux-20261012-5e8c4a91-evidence'
def digest(p):
    h=hashlib.sha256()
    with p.open('rb') as stream:
        for data in iter(lambda:stream.read(4*1024*1024),b''):h.update(data)
    return h.hexdigest()
for p in (base/'final').glob('*-evidence-*'):
    if not p.is_dir():continue
    for file in p.rglob('*'):
        if file.is_file() and file.suffix in ['.log','.json','.txt']:
            target=out/'raw/regression-cases'/p.name/file.relative_to(p);target.parent.mkdir(parents=True,exist_ok=True)
            if target.exists():assert digest(file)==digest(target)
            else:shutil.copyfile(file,target)
cases=[]
for name in ['regression-junit.xml','focus-regression-junit.xml','final-regression-junit.xml']:
    suite=ET.parse(base/'raw'/name).getroot();assert suite.attrib['failures']=='0'
    cases.append({'file':name,**suite.attrib})
matrix=[]
for path in sorted((base/'runs').glob('final4-path-*')):
    result=json.loads((path/'run.json').read_text());assert result['os_exit']==0 and not result['forced'];assert 'failures0' in (path/'stdout.log').read_text()
    matrix.append(result['label'])
assert len(matrix)==16
for path in (base/'runs').glob('perf-*-v4-*-96'):
    result=json.loads((path/'run.json').read_text());assert result['os_exit']==0 and not result['forced']
    assert 'state line' not in (path/'stdout.log').read_text()
for name in ['release-window-light','release-window-runtime']:
    result=json.loads((base/'runs'/name/'run.json').read_text());assert result['os_exit']==0 and not result['forced']
    text=(base/'runs'/name/'stdout.log').read_text();assert 'CSS declaration rejected' not in text and 'Web update:' not in text
cache_path=base/'raw/bmp-hash-cache.jsonl';cached={}
if cache_path.exists():
    for line in cache_path.read_text(encoding='utf-8').splitlines():
        row=json.loads(line);cached[row['path']]=row
bmp=[];errors=[]
with cache_path.open('a',encoding='utf-8') as cache:
 for p in sorted((base/'runs').rglob('*.bmp')):
    name=str(p.relative_to(root));stat=p.stat();row=cached.get(name)
    if not row or row['bytes']!=stat.st_size or row['mtimeNs']!=stat.st_mtime_ns:
        for attempt in range(3):
            try:value=digest(p);break
            except OSError as error:
                print('BMP_READ_ERROR',name,repr(error),flush=True)
                if attempt==2:errors.append(name);value=None
                else:time.sleep(.05)
        if value is None:continue
        row={'path':name,'bytes':stat.st_size,'mtimeNs':stat.st_mtime_ns,'sha256':value};cache.write(json.dumps(row,ensure_ascii=False)+'\n');cache.flush()
    bmp.append(row)
    if len(bmp)%500==0:print('BMP_HASH_PROGRESS',len(bmp),flush=True)
(out/'raw/bmp-read-errors.json').write_text(json.dumps({'allOriginalBmpHashVerified':not errors,'errors':errors,'status':'failed' if errors else 'passed','note':'Windows Get-FileHash reports CRC; no source BMP regeneration or disk repair'},ensure_ascii=False,indent=2),encoding='utf-8')
(out/'raw/original-bmp-index.json').write_text(json.dumps({'gitIgnored':True,'files':bmp},ensure_ascii=False,indent=2),encoding='utf-8')
(out/'verification-summary.json').write_text(json.dumps({'junit':cases,'final4Matrix':matrix,'v4Samples':1080,'final5WindowOps':2,'originalBmpHashed':len(bmp),'originalBmpReadFailures':errors,'originalBmpAllVerified':not errors,'independentConditions':'See report; no physical or target-CI substitution'},ensure_ascii=False,indent=2),encoding='utf-8')
report=root/'docs/validation/ux-validation-20261012.md'
for link in re.findall(r'\]\(([^)]+)\)',report.read_text(encoding='utf-8')):assert (report.parent/link).exists(),link
index=out/'sha256.json';assert not index.exists()
rows=[{'path':p.relative_to(out).as_posix(),'bytes':p.stat().st_size,'sha256':digest(p)} for p in sorted(out.rglob('*')) if p.is_file() and p!=index]
index.write_text(json.dumps({'files':rows},ensure_ascii=False,indent=2),encoding='utf-8')
assert all(digest(out/row['path'])==row['sha256'] for row in rows)
print('sealed files',len(rows),'PNG',len(list((out/'screenshots').rglob('*.png'))),'BMP',len(bmp))
