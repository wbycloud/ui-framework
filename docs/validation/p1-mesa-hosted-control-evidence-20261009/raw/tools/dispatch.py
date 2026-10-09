import datetime,json,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[3];base=root/'build/p1-mesa-hosted-control-20261009'
verified=json.loads((base/'raw/candidate-remote-verification.json').read_text())
commit=json.loads((base/'raw/diagnostic-commit.json').read_text())['head']
assert verified['sha256']=='afb11d7bf599e991f8e2b7df6971813f8bb6bca66f91f85c8f20b7b5cf59c8ae'
remote=subprocess.check_output(['git','ls-remote','--heads','origin','codex/p1-process-boundaries'],text=True).split()[0]
assert remote==commit
body={'ref':'codex/p1-process-boundaries','inputs':{'p1_process_diagnostic':'true','p1_candidate_url':verified['url']}}
bodyfile=base/'raw/dispatch-body.json';bodyfile.write_text(json.dumps(body,indent=2),encoding='utf-8')
start=datetime.datetime.now(datetime.timezone.utc).isoformat()
(base/'raw/dispatch-at.json').write_text(json.dumps({'started_utc':start,'head':commit,'single_dispatch':True},indent=2),encoding='utf-8')
result=subprocess.run([sys.executable,str(base/'tools/github_api.py'),'POST','/repos/wbycloud/ui-framework/actions/workflows/windows-regression.yml/dispatches',str(base/'raw/api-dispatch.json'),'--body',str(bodyfile)],capture_output=True)
(base/'raw/dispatch.log').write_bytes(result.stdout+result.stderr)
assert result.returncode==0,result.stdout.decode('utf-8','replace')
print(json.dumps({'head':commit,'started_utc':start,'response':json.loads((base/'raw/api-dispatch.json').read_text())}),flush=True)
