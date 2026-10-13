import pathlib,json,hashlib,subprocess
root=pathlib.Path(__file__).resolve().parents[3];base=root/'build/ux-20261012-5e8c4a91'
def sha(p):
    h=hashlib.sha256()
    with p.open('rb') as stream:
        for data in iter(lambda:stream.read(8*1024*1024),b''):h.update(data)
    return h.hexdigest()
baseline=json.loads((base/'raw/baseline.json').read_text(encoding='utf-8'));errors=[];groups=[]
for group in baseline['protected_groups']:
    directory=root/group['directory'];index=directory/('evidence-sha256.json' if directory.name.startswith('ui-repair') else 'sha256.json')
    if sha(index)!=group['index_sha256']:errors.append(str(index))
    rows=json.loads(index.read_text(encoding='utf-8'))['files']
    for row in rows:
        p=directory/row['path']
        if not p.exists() or sha(p)!=row['sha256']:errors.append(str(p))
    groups.append({'directory':group['directory'],'verified':len(rows)})
for row in baseline['files']:
    if row['path'].startswith(('.deps/','build/language-current-')) and sha(root/row['path'])!=row['sha256']:errors.append(row['path'])
head=subprocess.check_output(['git','rev-parse','HEAD'],cwd=root).decode().strip();assert head==baseline['head']
result={'head':head,'groups':groups,'protectedFiles':sum(g['verified'] for g in groups),'sdkAndMesaUnchanged':not errors,'errors':errors}
(base/'raw/final-protection-audit.json').write_text(json.dumps(result,ensure_ascii=False,indent=2),encoding='utf-8');print(result);assert not errors
