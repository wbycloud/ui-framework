import hashlib,json,pathlib,subprocess
root=pathlib.Path(__file__).resolve().parents[3];base=root/'build/p1-mesa-hosted-control-20261009'
paths=['.gitattributes','.github/workflows/windows-regression.yml','tests/observe_processes.py','tests/process_stall_capture.py',
       'tests/osmesa_thread_probe.c','tests/osmesa_stall_objects.py','tests/run_osmesa_lifecycle_control.py','tools/mesa-osmesa-process-detach.patch']
assert subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip()=='65f678c98dad601833035761f009479018d23079'
existing=subprocess.check_output(['git','diff','--cached','--name-only'],text=True).splitlines()
assert not existing or sorted(existing)==sorted(paths),existing
subprocess.run(['git','add','--',*paths],check=True)
staged=subprocess.check_output(['git','diff','--cached','--name-only'],text=True).splitlines()
assert sorted(staged)==sorted(paths),staged
subprocess.run(['git','diff','--cached','--check'],check=True)
(base/'raw/diagnostic-staged.patch').write_bytes(subprocess.check_output(['git','diff','--cached','--binary']))
(base/'raw/staged-identities.json').write_text(json.dumps([{'path':p,'sha256':hashlib.sha256((root/p).read_bytes()).hexdigest()} for p in paths],indent=2),encoding='utf-8')
message=base/'raw/diagnostic-message.txt'
message.write_text('Add Mesa lifecycle patch and hosted A/C diagnostics\n\nKeep default workers and normal teardown; select the sealed termination-safe provider explicitly. Capture matching live rasterizer HANDLE/semaphore state after full dumps and check complete native exits, CASE_END and EOF at the original 180/240-second Light gates and 25-minute job budget.\n\nLocal A/C live-object capture, Python AST, four rendered PowerShell parsers and existing CI policy checks passed. Remote default-thread A/C outcome remains to be measured.\n',encoding='utf-8')
with (base/'raw/diagnostic-commit.log').open('xb') as stream:
 result=subprocess.run(['git','commit','-F',str(message)],stdout=stream,stderr=subprocess.STDOUT)
assert result.returncode==0
commit=subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip()
assert sorted(subprocess.check_output(['git','diff-tree','--no-commit-id','--name-only','-r',commit],text=True).splitlines())==sorted(paths)
(base/'raw/diagnostic-commit.json').write_text(json.dumps({'head':commit,'parent':'65f678c98dad601833035761f009479018d23079','paths':paths},indent=2),encoding='utf-8')
with (base/'raw/diagnostic-push.log').open('xb') as stream:
 result=subprocess.run(['git','push','origin','HEAD:refs/heads/codex/p1-process-boundaries'],stdout=stream,stderr=subprocess.STDOUT)
assert result.returncode==0
print(json.dumps({'commit':commit,'files':paths}),flush=True)
