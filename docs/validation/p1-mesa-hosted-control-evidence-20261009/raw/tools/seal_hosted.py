import ast,hashlib,json,pathlib,re,shutil,subprocess
root=pathlib.Path(__file__).resolve().parents[3]
base=root/'build/p1-mesa-hosted-control-20261009'
out=root/'docs/validation/p1-mesa-hosted-control-evidence-20261009'
report=root/'docs/validation/p1-mesa-hosted-control-validation-20261009.md'
for name in ['persist_prior_context.py','seal_hosted.py']:
    path=base/'tools'/name;target=out/'raw/tools'/name
    with path.open('rb') as src,target.open('xb') as dst:shutil.copyfileobj(src,dst)
for path in (out/'raw/tools').glob('*.py'):ast.parse(path.read_text(encoding='utf-8'))
json_files=0
for path in out.rglob('*.json'):
    json.loads(path.read_text(encoding='utf-8-sig'));json_files+=1
suspect=[]
patterns=[r'gh[pousr]_[A-Za-z0-9]{30,}',r'github_pat_[A-Za-z0-9_]{30,}',r'https://[^\s"<>]+[?&]sig=']
for path in out.rglob('*'):
    if path.is_file() and path.suffix not in ['.png','.bin']:
        value=path.read_text(encoding='utf-8',errors='replace')
        if any(re.search(pattern,value) for pattern in patterns):suspect.append(str(path.relative_to(out)))
assert not suspect,{'secret_or_signed_url_files':suspect}
links=0
for path in [report,out/'README.md']:
    value=path.read_text(encoding='utf-8')
    for link in re.findall(r'\]\(([^)]+)\)',value):
        if link.startswith(('http:','https:','#')):continue
        target=path.parent/link
        if target==out/'sha256.json':continue
        assert target.exists(),{'document':str(path),'link':link}
        links+=1
assert not subprocess.check_output(['git','diff','--name-only']).strip()
receipt={'test_head':'5715a0539b94158e3f18afc32bffbc9a5d6a0168','json_parsed':json_files,
    'local_report_links_checked':links,'helper_ast':'pass','secret_or_signed_url_scan':'pass',
    'protected_history_result':'raw/raw/final-history-preservation-normalized.json','tracked_diff':'clean',
    'report_sha256':hashlib.sha256(report.read_bytes()).hexdigest()}
(out/'raw/analysis/delivery-checks.json').write_text(json.dumps(receipt,indent=2),encoding='utf-8')
files=[]
for path in sorted(out.rglob('*')):
    if path.is_file():files.append({'path':str(path.relative_to(out)).replace('\\','/'),'bytes':path.stat().st_size,
        'sha256':hashlib.sha256(path.read_bytes()).hexdigest()})
index=out/'sha256.json'
with index.open('x',encoding='utf-8') as stream:json.dump({'files':files},stream,indent=2);stream.write('\n')
for row in files:
    path=out/row['path'];assert path.stat().st_size==row['bytes'] and hashlib.sha256(path.read_bytes()).hexdigest()==row['sha256']
print(json.dumps({'files':len(files),'png':sum(r['path'].endswith('.png') for r in files),'sha256_index':hashlib.sha256(index.read_bytes()).hexdigest(),'report_sha256':receipt['report_sha256'],'links':links}),flush=True)
