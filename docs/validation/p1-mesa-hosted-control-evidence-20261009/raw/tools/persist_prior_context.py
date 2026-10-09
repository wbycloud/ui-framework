import pathlib,shutil
root=pathlib.Path(__file__).resolve().parents[3]
out=root/'docs/validation/p1-mesa-hosted-control-evidence-20261009/raw/prior-local'
out.mkdir(exist_ok=False)
sources={'report.md':root/'docs/validation/p1-mesa-lifecycle-validation-20261009.md'}
old=root/'docs/validation/p1-mesa-lifecycle-evidence-20261009'
for name in ['identity-results.json','performance.json','provider-lifecycle-runs.json','provider-pe-metadata.json','candidate-archive.json']:
    sources[name]=old/'raw/raw'/name
for name in ['mapped-C-exit','mapped-C-cleanup','private-A-default','private-B-default','private-C-default']:
    directory=old/'raw/runs'/name
    for path in directory.glob('*.json'):sources[name+'-'+path.name]=path
    log=directory/'stdout.log'
    if log.exists():sources[name+'-stdout.log']=log
for name,path in sources.items():
    assert path.exists(),path
    with path.open('rb') as src,(out/name).open('xb') as dst:shutil.copyfileobj(src,dst)
print('Copied',len(sources),'sealed prior local records without modification')
