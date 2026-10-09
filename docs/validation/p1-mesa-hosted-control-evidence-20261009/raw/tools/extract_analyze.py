import argparse,hashlib,json,pathlib,re,xml.etree.ElementTree as ET,zipfile
root=pathlib.Path(__file__).resolve().parents[3];base=root/'build/p1-mesa-hosted-control-20261009'
parser=argparse.ArgumentParser();parser.add_argument('configuration',choices=['osmesa','webview2']);parser.add_argument('archive',type=pathlib.Path);parser.add_argument('--inventory',type=pathlib.Path,default=base/'raw/api-artifacts-final.json');args=parser.parse_args()
inventory=json.loads(args.inventory.read_text());meta=next(row for row in inventory['artifacts'] if row['name'].startswith('windows-'+args.configuration+'-'))
archive=args.archive;value=hashlib.sha256(archive.read_bytes()).hexdigest()
assert archive.stat().st_size==meta['size_in_bytes'] and 'sha256:'+value==meta['digest'],meta
out=base/'artifacts'/args.configuration;out.mkdir(parents=True,exist_ok=False);files=[]
with zipfile.ZipFile(archive) as package:
 for member in package.infolist():
  target=(out/member.filename).resolve();assert target.is_relative_to(out.resolve())
  if member.is_dir():target.mkdir(parents=True,exist_ok=True);continue
  target.parent.mkdir(parents=True,exist_ok=True);data=package.read(member)
  with target.open('xb') as stream:stream.write(data)
  files.append({'path':member.filename,'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest()})
(base/f'raw/artifact-{args.configuration}-files.json').write_text(json.dumps({'artifact':meta,'archive_sha256':value,'files':files},indent=2),encoding='utf-8')
controls=list(out.rglob('lifecycle-control/raw/results.json'));assert len(controls)==1,controls
result=json.loads(controls[0].read_text());control=controls[0].parents[1]
assert result['head']=='5715a0539b94158e3f18afc32bffbc9a5d6a0168'
summary={'configuration':args.configuration,'head':result['head'],'providers':result['providers'],'caller_sha256':result['caller_sha256'],'binaries':result['binaries'],'runs':[],'dumps':[]}
for row in result['runs']:
 if row['kind']=='standalone':
  directory=control/'runs'/f"probe-{row['label']}-gate{int(row['gated'])}"
  log=(directory/'stdout.log').read_text(encoding='utf-8',errors='replace')
  summary['runs'].append({**row,'renders':re.findall(r'RENDER[^\r\n]+',log),'main_return':re.findall(r'MAIN_RETURN[^\r\n]+',log)})
 else:
  directory=control/'runs'/('original-'+row['label'])/row['test'];events=[json.loads(line) for line in (directory/'processes.jsonl').read_text().splitlines()]
  native={event['pid'] for event in events if event['kind']=='child_observed' and event['exe'].endswith('_test.exe')}
  exits=[event for event in events if event['kind']=='child_exit' and event['pid'] in native]
  text=(directory/'processes.log').read_text(encoding='utf-8',errors='replace');cases=ET.parse(directory/'ctest.xml').getroot().findall('testcase');assert len(cases)==1
  phases=[event for event in events if event['kind']=='stdout_line' and 'TEST_PHASE' in event['line']]
  item={**row,'actual_native_exits':exits,'entered_cases':len(re.findall(r'CASE_BEGIN[^\r\n]+',text)),
    'completed_cases':len(re.findall(r'CASE_END[^\r\n]+',text)),'ctest':cases[0].attrib,
    'ctest_failure':None if cases[0].find('failure') is None else cases[0].find('failure').attrib,
    'parent_exit':[e for e in events if e['kind']=='parent_exit'],'stdout_eof':[e for e in events if e['kind']=='stdout_eof'],
    'last_phase':phases[-1] if phases else None,'stall_events':[e for e in events if e['kind'].startswith('stall_capture')],
    'dispatch_max':max([int(n) for n in re.findall(r'dispatched(\d+)',text)]+[0])}
  expected=26 if 'stateful' in row['test'] else 7
  if row['complete_boundaries_pass']:
   assert row['observer_exit']==0 and len(native)==len(exits)==item['completed_cases']==expected
   assert all(e['read_ok'] and e['code']==0 for e in exits) and item['stdout_eof'] and item['parent_exit'][0]['code']==0 and item['dispatch_max']>0
  summary['runs'].append(item)
for path in control.rglob('capture.json'):
 data=json.loads(path.read_text());summary['dumps'].append({'capture':str(path.relative_to(out)),'records':data})
(base/f'analysis/{args.configuration}-summary.json').write_text(json.dumps(summary,indent=2),encoding='utf-8')
print(json.dumps({'configuration':args.configuration,'files':len(files),'dumps':sum(p.suffix=='.dmp' for p in out.rglob('*')),
 'runs':[{'label':r['label'],'kind':r['kind'],'test':r.get('test'),'os_exit':r.get('os_exit'),'forced':r.get('forced'),'gated':r.get('gated'),
 'complete_boundaries_pass':r.get('complete_boundaries_pass'),'native_count':len(r.get('actual_native_exits',[]))} for r in summary['runs']]}),flush=True)
