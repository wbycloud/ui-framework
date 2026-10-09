import hashlib,json,os,pathlib,subprocess
root=pathlib.Path(__file__).resolve().parents[3];base=root/'build/p1-mesa-hosted-control-20261009'
archive=root/'build/p1-mesa-lifecycle-20261009/mesa24.3.4-osmesa-lifecycle-candidate-65f678c-99d8142-x64.zip'
assert hashlib.sha256(archive.read_bytes()).hexdigest()=='afb11d7bf599e991f8e2b7df6971813f8bb6bca66f91f85c8f20b7b5cf59c8ae'
branch='codex/p1-lifecycle-candidate-20261009'
assert not subprocess.check_output(['git','ls-remote','--heads','origin',branch],text=True).strip()
assert not subprocess.check_output(['git','diff','--cached','--name-only'],text=True).strip()
manifest=json.loads((root/'build/p1-mesa-lifecycle-20261009/raw/candidate-archive.json').read_text())
(base/'raw/transfer-manifest.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
(base/'raw/transfer-README.md').write_text('# Sealed P1 OSMesa lifecycle candidate\n\nAuthorized diagnostic transfer only. This branch contains the exact local candidate; it is not a framework SDK or stable release.\n\nArchive SHA256: `afb11d7bf599e991f8e2b7df6971813f8bb6bca66f91f85c8f20b7b5cf59c8ae`. Provider DLL: `99d8142434c3abfacce969aa3731220314d99c53ee0afc546896c0afad92f5f5`. Default worker policy retained. Framework source baseline: `65f678c98dad601833035761f009479018d23079`.\n',encoding='utf-8')
env=os.environ.copy();env['GIT_INDEX_FILE']=str(base/'candidate-transfer.index')
subprocess.run(['git','read-tree','--empty'],env=env,check=True)
entries=[]
for path,name in [(archive,archive.name),(base/'raw/transfer-manifest.json','manifest.json'),(base/'raw/transfer-README.md','README.md')]:
 blob=subprocess.check_output(['git','hash-object','-w','--no-filters',str(path)],text=True).strip()
 subprocess.run(['git','update-index','--add','--cacheinfo','100644',blob,name],env=env,check=True)
 entries.append({'path':name,'blob':blob,'sha256':hashlib.sha256(path.read_bytes()).hexdigest()})
tree=subprocess.check_output(['git','write-tree'],env=env,text=True).strip()
message=base/'raw/transfer-message.txt';message.write_text('Transfer sealed Mesa24.3.4 P1 lifecycle candidate\n\nExact local DLL/PDB archive for explicitly authorized A/C hosted diagnostic. No release, tag or framework SDK.\n',encoding='utf-8')
commit=subprocess.check_output(['git','commit-tree',tree,'-F',str(message)],text=True).strip()
subprocess.run(['git','update-ref','refs/heads/'+branch,commit,'0'*40],check=True)
url=f'https://raw.githubusercontent.com/wbycloud/ui-framework/{commit}/{archive.name}'
payload={'branch':branch,'commit':commit,'tree':tree,'files':entries,'url':url,'framework_head':subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip()}
(base/'raw/candidate-transfer.json').write_text(json.dumps(payload,indent=2),encoding='utf-8')
assert not subprocess.check_output(['git','diff','--cached','--name-only'],text=True).strip()
with (base/'raw/candidate-push.log').open('xb') as stream:
 result=subprocess.run(['git','push','origin',commit+':refs/heads/'+branch],stdout=stream,stderr=subprocess.STDOUT)
assert result.returncode==0
print(json.dumps(payload),flush=True)
