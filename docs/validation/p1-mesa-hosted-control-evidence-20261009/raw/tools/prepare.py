import hashlib,json,pathlib,shutil,subprocess
root=pathlib.Path(__file__).resolve().parents[3];base=root/'build/p1-mesa-hosted-control-20261009'
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
baseline={'head':subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),
 'branch':subprocess.check_output(['git','branch','--show-current'],text=True).strip(),
 'status':subprocess.check_output(['git','status','--porcelain'],text=True).strip(),
 'old_evidence_index_sha256':sha(root/'docs/validation/p1-mesa-lifecycle-evidence-20261009/sha256.json'),
 'candidate_sha256':sha(root/'build/p1-mesa-lifecycle-20261009/mesa24.3.4-osmesa-lifecycle-candidate-65f678c-99d8142-x64.zip')}
(base/'raw/baseline.json').write_text(json.dumps(baseline,indent=2),encoding='utf-8')
assert baseline['candidate_sha256']=='afb11d7bf599e991f8e2b7df6971813f8bb6bca66f91f85c8f20b7b5cf59c8ae'
shutil.copyfile(root/'build/p1-hosted-diagnostic-20261009/github_api.py',base/'tools/github_api.py')
shutil.copyfile(root/'docs/validation/p1-mesa-lifecycle-evidence-20261009/ci-proposal/run_osmesa_lifecycle_control.py',root/'tests/run_osmesa_lifecycle_control.py')
subprocess.run(['git','apply', '--exclude=tests/run_osmesa_lifecycle_control.py',str(root/'docs/validation/p1-mesa-lifecycle-evidence-20261009/ci-proposal/proposal.patch')],check=True)
cmd='.reload /f osmesa.dll; lmv m osmesa; dt osmesa!pipe_frontend_screen; dt osmesa!llvmpipe_screen; dt osmesa!lp_rasterizer; dt osmesa!lp_rasterizer_task; dt osmesa!util_semaphore; dt osmesa!thrd_t; ? osmesa!global_fscreen-osmesa; q'
old=root/'build/p1-mesa-lifecycle-20261009'
rows=[]
for label,pid in [('A',32364),('C',25092)]:
 dump=old/f'runs/provider-{label}-exit-default-99/rendered-full/{pid}.dmp'
 symbols=root/'build/p1-hosted-diagnostic-20261009/mesa-debug/x64' if label=='A' else old/'providers/C/build-ascii-llvm/src/gallium/targets/osmesa'
 images=root/'.deps/mesa-24.3.4/x64' if label=='A' else symbols
 command=['C:/Program Files (x86)/Windows Kits/10/Debuggers/x86/cdb.exe','-z',str(dump),'-y',str(symbols),'-i',str(images),'-c',cmd]
 result=subprocess.run(command,capture_output=True)
 (base/f'analysis/provider-{label}-layouts.log').write_bytes(result.stdout+result.stderr)
 rows.append({'label':label,'command':command,'exit':result.returncode})
(base/'raw/layout-commands.json').write_text(json.dumps(rows,indent=2),encoding='utf-8')
assert all(row['exit']==0 for row in rows)
print(json.dumps(baseline))
