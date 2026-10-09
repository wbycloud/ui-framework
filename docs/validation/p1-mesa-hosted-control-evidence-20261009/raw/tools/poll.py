import json,pathlib,subprocess,sys
base=pathlib.Path(__file__).resolve().parents[1];run=json.loads((base/'raw/selected-run.json').read_text())['id']
index=max([int(p.stem.rsplit('-',1)[1]) for p in (base/'raw').glob('api-run-*.json')]+[0])+1
data={}
for name,route in [('run',f'/repos/wbycloud/ui-framework/actions/runs/{run}'),('jobs',f'/repos/wbycloud/ui-framework/actions/runs/{run}/jobs?per_page=100')]:
 target=base/f'raw/api-{name}-{index:03d}.json'
 result=subprocess.run([sys.executable,str(base/'tools/github_api.py'),'GET',route,str(target),'--curl'],capture_output=True)
 if result.returncode:
  (base/f'raw/poll-{name}-{index:03d}-error.log').write_bytes(result.stdout+result.stderr);raise SystemExit(result.returncode)
 data[name]=json.loads(target.read_text())
assert data['run']['head_sha']=='5715a0539b94158e3f18afc32bffbc9a5d6a0168'
summary={'snapshot':index,'status':data['run']['status'],'conclusion':data['run']['conclusion'],'jobs':[]}
for job in data['jobs']['jobs']:
 summary['jobs'].append({'id':job['id'],'name':job['name'],'status':job['status'],'conclusion':job['conclusion'],
   'steps':[{k:step[k] for k in ['name','status','conclusion']} for step in job.get('steps',[]) if step['status'] in ['in_progress','completed']]})
print(json.dumps(summary),flush=True)
