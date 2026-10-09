import hashlib,json,pathlib
root=pathlib.Path(__file__).resolve().parents[3]
base=root/'build/p1-mesa-hosted-control-20261009'
checked={};groups=[];errors=[]
def check(path,size,sha):
    path=pathlib.Path(path).as_posix()
    if path in checked:
        assert checked[path]==(size,sha)
        return
    full=root/path
    if not full.exists():errors.append({'path':path,'error':'missing'});return
    value=hashlib.sha256()
    with full.open('rb') as stream:
        for chunk in iter(lambda:stream.read(8*1024*1024),b''):value.update(chunk)
    if full.stat().st_size!=size or value.hexdigest()!=sha:errors.append({'path':path,'error':'identity mismatch'})
    checked[path]=(size,sha)
for name in ['instance-language-current','p1-hosted-diagnostic','p1-mesa-thread-control','p1-mesa-lifecycle']:
    directory=root/'docs/validation'/(name+'-evidence-20261009')
    index=json.loads((directory/'sha256.json').read_text(encoding='utf-8'))
    for row in index['files']:check(str((directory/row['path']).relative_to(root)),row['bytes'],row['sha256'])
    groups.append({'directory':str(directory.relative_to(root)),'entries':len(index['files'])})
snapshot=json.loads((root/'build/p1-mesa-lifecycle-20261009/raw/prior-build-preservation-start.json').read_text())
for row in snapshot['files']:check(row['path'],row['size'],row['sha256'])
groups.append({'kind':'prior two P1 builds snapshot','entries':len(snapshot['files'])})
local=json.loads((root/'docs/validation/p1-mesa-lifecycle-evidence-20261009/local-artifacts.json').read_text())
for row in local['files']:check(row['path'],row['bytes'],row['sha256'])
groups.append({'kind':'prior candidate/lifecycle artifacts','entries':len(local['files'])})
check('docs/validation/p1-mesa-lifecycle-evidence-20261009/sha256.json',
      (root/'docs/validation/p1-mesa-lifecycle-evidence-20261009/sha256.json').stat().st_size,
      '6d06e8866863a7276c77ffe091090de401dad80da1cc0153bd09cb50f4bb0362')
check('docs/validation/p1-mesa-lifecycle-validation-20261009.md',
      (root/'docs/validation/p1-mesa-lifecycle-validation-20261009.md').stat().st_size,
      'ab30db8bd3c6ee20544cb1acd043a526133bc64342ac7cd3341ba43e3ee36fdd')
check('build/language-current-20261009/ui-framework-sdk0.9.0-dev-api9-44c039f-windows-x64.zip',117891276,
      '28ca0b8409b321477f1aa3adce2680fda8cf2cd8a9a35dd4308a5f08ef1f79a2')
result={'groups':groups,'unique_files_checked':len(checked),'errors':errors,'pass':not errors}
(base/'raw/final-history-preservation-normalized.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
print(json.dumps(result),flush=True)
assert not errors
