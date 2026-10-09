import ast,json,os,pathlib,subprocess,sys,threading,yaml
root=pathlib.Path(__file__).resolve().parents[3];base=root/'build/p1-mesa-hosted-control-20261009'
for relative in ['tests/osmesa_stall_objects.py','tests/process_stall_capture.py','tests/run_osmesa_lifecycle_control.py','tests/observe_processes.py']:
 ast.parse((root/relative).read_text(encoding='utf-8'))
workflow=yaml.safe_load((root/'.github/workflows/windows-regression.yml').read_text())
assert workflow['jobs']['regression']['timeout-minutes']==25
script=next(step['run'] for step in workflow['jobs']['regression']['steps'] if step.get('name')=='Necessary regressions')
checker=root/'build/p1-mesa-lifecycle-20261009/tools/parser-check.ps1'
pwsh='C:/Users/wbycl/.cache/codex-runtimes/codex-primary-runtime/dependencies/native/powershell/pwsh.exe'
parser=[]
for configuration in ['osmesa','webview2']:
 for diagnostic in ['true','false']:
  path=base/f'raw/rendered-{configuration}-{diagnostic}.ps1'
  path.write_text(script.replace('${{ matrix.configuration }}',configuration).replace('${{ inputs.p1_process_diagnostic }}',diagnostic),encoding='utf-8')
  result=subprocess.run([pwsh,'-NoProfile','-File',str(checker),str(path)],capture_output=True)
  parser.append({'configuration':configuration,'diagnostic':diagnostic,'exit':result.returncode,'output':(result.stdout+result.stderr).decode('utf-8','replace')})
assert all(row['exit']==0 for row in parser),parser
old=root/'build/p1-mesa-lifecycle-20261009';providers={'A':root/'.deps/mesa-24.3.4/x64/osmesa.dll','C':old/'candidate-unpacked/osmesa.dll'}
rows=[];parent=os.environ.get('LP_NUM_THREADS')
for label,provider in providers.items():
 directory=base/f'local-object-{label}';directory.mkdir()
 env=os.environ.copy();env.pop('LP_NUM_THREADS',None);env['UI_MESA_PROBE_GATE']='1';env['GALLIUM_DRIVER']='llvmpipe'
 env['UI_PROCESS_FULL_DUMP']='1';env['UI_MESA_OBJECTS']='1'
 command=[str(old/'osmesa_lifecycle_probe.exe'),str(provider),'exit']
 child=subprocess.Popen(command,env=env,stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
 gate=threading.Event();lines=[]
 def read():
  for line in iter(child.stdout.readline,b''):
   lines.append(line)
   if line.startswith(b'GATE rendered'):gate.set()
 reader=threading.Thread(target=read);reader.start()
 try:
  assert gate.wait(20),'Missing diagnostic gate'
  capture=subprocess.run([sys.executable,str(root/'tests/process_stall_capture.py'),str(directory/'full'),str(child.pid)],env=env,capture_output=True,timeout=20)
  (directory/'capture.log').write_bytes(capture.stdout+capture.stderr)
  records=json.loads((directory/'full/capture.json').read_text());objects=records[0]['mesa_objects']
  assert records[0]['dump_ok'] and records[0]['flags']=='0x1826'
  assert objects['num_threads']==24 and objects['exit_flag']==0 and len(objects['tasks'])==24,objects
  assert all(t['duplicate_ok'] and t['query_ok'] and t['exit_code']==259 and t['zero_wait']==258 for t in objects['tasks'])
  child.stdin.write(b'\n');child.stdin.flush();child.wait(timeout=45);reader.join(timeout=5)
  assert child.returncode==0 and not reader.is_alive()
  rows.append({'label':label,'command':command,'pid':child.pid,'natural_os_exit':child.returncode,'eof':not reader.is_alive(),'gated':True,
   'default_rasterizer_tasks':objects['num_threads'],'saved_handle_duplicates':len(objects['tasks'])})
 finally:
  if child.poll() is None:child.kill();child.wait()
  reader.join(timeout=5);(directory/'stdout.log').write_bytes(b''.join(lines))
assert os.environ.get('LP_NUM_THREADS')==parent
result=subprocess.run([pwsh,'-NoProfile','-File',str(root/'tools/test-windows-ci.ps1')],capture_output=True)
(base/'raw/existing-ci-policy.log').write_bytes(result.stdout+result.stderr);assert result.returncode==0
(base/'raw/local-preflight.json').write_text(json.dumps({'python_ast':'pass','powershell_parse':parser,'local_objects':rows,'original_job_budget':25,'parent_lp':parent,'policy':'pass'},indent=2),encoding='utf-8')
print(json.dumps({'local_objects':rows,'powershell_parse':'pass','policy':'pass'}),flush=True)
