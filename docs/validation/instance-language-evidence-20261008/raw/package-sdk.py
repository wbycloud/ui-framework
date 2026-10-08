from pathlib import Path
import datetime,hashlib,json,subprocess,uuid,zipfile
r=Path.cwd().resolve();w=r/'build/language-20261008';sdk=w/'sdk-api9';sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
assert not subprocess.check_output(['git','status','--porcelain'],cwd=r)
for p in (sdk/'include/ui_framework').glob('*.h'):assert sha(p)==sha(r/'include/ui_framework'/p.name)
identity=json.loads((sdk/'identity.json').read_text(encoding='utf-8'));frozen=json.loads((w/'product-artifacts.json').read_text(encoding='utf-8'))
for item in frozen['artifacts']:assert sha(r/item['path'])==item['sha256'],item['path']
for name in ('ui_framework.dll','framework_host.exe','stateful_components.uapp','ui_instance_language_test.exe'):assert sha(sdk/'bin'/name)==sha(w/'after'/name)
files=[]
for p in sorted(sdk.rglob('*')):
 assert not p.is_symlink(),p
 if p.is_file():
  assert '.git' not in p.relative_to(sdk).parts
  files.append({'path':p.relative_to(sdk).as_posix(),'bytes':p.stat().st_size,'sha256':sha(p)})
(sdk/'sha256.json').write_text(json.dumps({'algorithm':'SHA256','excluded_self':'sha256.json','files':files},ensure_ascii=False,indent=2),encoding='utf-8')
archive=w/'ui-framework-sdk0.9.0-dev-api9-6d88770-windows-x64.zip'
with zipfile.ZipFile(archive,'x',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 for p in sorted(sdk.rglob('*')):
  if p.is_file():z.write(p,'sdk/'+p.relative_to(sdk).as_posix())
verify=w/('sdk-verify-'+uuid.uuid4().hex[:12]);verify.mkdir()
with zipfile.ZipFile(archive) as z:
 assert z.testzip() is None
 for item in z.infolist():assert (verify/item.filename).resolve().is_relative_to(verify),item.filename
 z.extractall(verify)
e=verify/'sdk'
for f in files:
 p=e/f['path'];assert p.stat().st_size==f['bytes'] and sha(p)==f['sha256'],f['path']
assert sha(e/'sha256.json')==sha(sdk/'sha256.json')
ret=subprocess.run([str(e/'bin/sdk-consumer.exe')],cwd=e/'bin',stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
(w/'sdk-extracted-consumer.log').write_bytes(ret.stdout);assert ret.returncode==0,ret.stdout
proof={'created_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'archive':str(archive),'bytes':archive.stat().st_size,'sha256':sha(archive),'source_document_snapshot':identity['source_document_snapshot'],'product_commit':identity['product_commit'],'files_including_manifest':len(files)+1,'sha256_manifest':sha(e/'sha256.json'),'crc':'PASS','per_file_verification':'PASS','extracted_directory':str(e),'extracted_public_c_consumer':'PASS','extracted_complete_language_host':'PENDING serial real mouse verification','offline_source_build':'PENDING'}
(w/'sdk-archive.json').write_text(json.dumps(proof,ensure_ascii=False,indent=2),encoding='utf-8')
(w/(archive.name+'.sha256')).write_text(sha(archive)+'  '+archive.name+'\n',encoding='ascii')
print(json.dumps(proof,ensure_ascii=False,indent=2))
