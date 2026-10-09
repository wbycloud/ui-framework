from pathlib import Path
import hashlib,json,subprocess,zipfile
root=Path.cwd().resolve();work=root/'build/language-current-20261009';sdk=work/'sdk-delivery'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
assert not subprocess.check_output(['git','status','--porcelain'],cwd=root)
identity=json.loads((sdk/'identity.json').read_text(encoding='utf-8'))
assert identity['source_document_snapshot']==subprocess.check_output(['git','rev-parse','HEAD'],cwd=root,text=True).strip()
for p in (sdk/'include/ui_framework').glob('*.h'):assert sha(p)==sha(root/'include/ui_framework'/p.name)
for name in ['ui_framework.dll','framework_host.exe','stateful_components.uapp','ui_instance_language_test.exe']:
    assert sha(sdk/'bin'/name)==sha(work/'current'/name)
files=[]
for p in sorted(sdk.rglob('*')):
    assert not p.is_symlink(),p
    if p.is_file():files.append({'path':p.relative_to(sdk).as_posix(),'bytes':p.stat().st_size,'sha256':sha(p)})
(sdk/'sha256.json').write_text(json.dumps({'algorithm':'SHA256','excluded_self':'sha256.json','files':files},ensure_ascii=False,indent=2),encoding='utf-8')
archive=work/'ui-framework-sdk0.9.0-dev-api9-44c039f-windows-x64.zip'
with zipfile.ZipFile(archive,'x',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
    for p in sorted(sdk.rglob('*')):
        if p.is_file():z.write(p,'sdk/'+p.relative_to(sdk).as_posix())
unpacked=work/'sdk-delivery-extracted';assert not unpacked.exists();unpacked.mkdir()
with zipfile.ZipFile(archive) as z:
    assert z.testzip() is None
    for item in z.infolist():assert (unpacked/item.filename).resolve().is_relative_to(unpacked)
    z.extractall(unpacked)
e=unpacked/'sdk'
for item in files:
    p=e/item['path'];assert p.stat().st_size==item['bytes'] and sha(p)==item['sha256'],item['path']
assert sha(e/'sha256.json')==sha(sdk/'sha256.json')
result=subprocess.run([str(e/'bin/sdk-consumer.exe')],cwd=e/'bin',stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
(work/'sdk-final-extracted-consumer.log').write_bytes(result.stdout);assert result.returncode==0
proof={'archive':str(archive),'bytes':archive.stat().st_size,'sha256':sha(archive),'source_document_snapshot':identity['source_document_snapshot'],'files_including_manifest':len(files)+1,'crc':'PASS','per_file_verification':'PASS','extracted_public_c_consumer':'PASS','extracted_directory':str(e),'actual_extracted_complete_language_application':'PENDING serial tests','offline_source_build':'PENDING'}
(work/'sdk-final-archive.json').write_text(json.dumps(proof,ensure_ascii=False,indent=2),encoding='utf-8')
(work/(archive.name+'.sha256')).write_text(proof['sha256']+'  '+archive.name+'\n',encoding='ascii')
print('SDK files',len(files)+1,'bytes',proof['bytes'],'sha256',proof['sha256'])
