import pathlib,json,hashlib,shutil,re,xml.etree.ElementTree as ET
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
bmp=[{'path':str(p.relative_to(root)),'bytes':p.stat().st_size,'sha256':digest(p)} for p in (base/'runs').rglob('*.bmp')]
(out/'raw/original-bmp-index.json').write_text(json.dumps({'gitIgnored':True,'files':bmp},ensure_ascii=False,indent=2),encoding='utf-8')
(out/'verification-summary.json').write_text(json.dumps({'junit':cases,'final4Matrix':matrix,'v4Samples':1080,'final5WindowOps':2,'originalBmpFiles':len(bmp),'independentConditions':'See report; no physical or target-CI substitution'},ensure_ascii=False,indent=2),encoding='utf-8')
report=root/'docs/validation/ux-validation-20261012.md'
for link in re.findall(r'\]\(([^)]+)\)',report.read_text(encoding='utf-8')):assert (report.parent/link).exists(),link
index=out/'sha256.json';assert not index.exists()
rows=[{'path':p.relative_to(out).as_posix(),'bytes':p.stat().st_size,'sha256':digest(p)} for p in sorted(out.rglob('*')) if p.is_file() and p!=index]
index.write_text(json.dumps({'files':rows},ensure_ascii=False,indent=2),encoding='utf-8')
assert all(digest(out/row['path'])==row['sha256'] for row in rows)
print('sealed files',len(rows),'PNG',len(list((out/'screenshots').rglob('*.png'))),'BMP',len(bmp))
