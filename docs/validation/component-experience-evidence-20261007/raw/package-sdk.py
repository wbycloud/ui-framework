from pathlib import Path
import hashlib, json, subprocess, zipfile, uuid, datetime

root=Path.cwd().resolve(); work=root/'build/experience-20261007'; sdk=work/'sdk'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
assert sdk.is_relative_to(work) and (sdk/'source/CMakeLists.txt').is_file()
product='807828147bdfb6959abb49b4668dfa90c34eda56'
tests='0fe08640c3008874449dce77fd7445ef91f6a578'
head=subprocess.check_output(['git','rev-parse','HEAD']).decode().strip()
subprocess.run([r'C:/Users/wbycl/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe',str(work/'refresh-sdk.py')],check=True)
readme=sdk/'SDK-README.md'
text=readme.read_text(encoding='utf-8')
text=text.replace('源码含本地CI身份与列定义测试及阶段耗时诊断。', '测试收敛0fe08640c3008874449dce77fd7445ef91f6a578；源码/文档快照'+head+'。源码含本地CI身份、列定义与有界桌面消息泵测试。')
readme.write_text(text,encoding='utf-8')
frozen=json.loads((work/'delivery-frozen-hashes.json').read_text(encoding='utf-8'))
for item in frozen['artifacts']:assert sha(root/item['path'])==item['sha256'],item['path']
for source in (root/'include/ui_framework').glob('*.h'):
    assert sha(source)==sha(sdk/'include/ui_framework'/source.name)
for source in (sdk/'bin').iterdir():
    current=work/'after'/source.name
    if current.is_file():assert sha(source)==sha(current),source.name
for name in ['ui_framework_runtime.lib','ui_framework.lib','ui_framework_headless.lib']:
    assert sha(sdk/'lib'/name)==sha(work/'after'/name)
assert not (sdk/'bin/opengl32.dll').exists()
identity={'sdk':'0.8.0-dev','framework_api':8,'standard':8,'abi':1,'package':1,'product_commit':product,'test_commit':tests,'source_document_snapshot':head,'kind':'local_unpublished','dependencies':json.loads((work/'dependencies-final.json').read_text(encoding='utf-8'))}
(sdk/'identity.json').write_text(json.dumps(identity,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
files=[]
for path in sorted(sdk.rglob('*')):
    assert not path.is_symlink(),str(path)
    if path.is_file() and path!=sdk/'sha256.json':
        assert path.resolve().is_relative_to(sdk)
        assert '.git' not in path.relative_to(sdk).parts
        files.append({'path':path.relative_to(sdk).as_posix(),'bytes':path.stat().st_size,'sha256':sha(path)})
(sdk/'sha256.json').write_text(json.dumps({'algorithm':'SHA256','excluded_self':'sha256.json','files':files},ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
archive=work/'ui-framework-sdk0.8.0-dev-api8-8078281-windows-x64.zip'
assert not archive.exists(),'Existing archive preserved; choose a fresh delivery filename'
with zipfile.ZipFile(archive,'x',compression=zipfile.ZIP_DEFLATED,compresslevel=6) as z:
    for item in files:z.write(sdk/item['path'],item['path'])
    z.write(sdk/'sha256.json','sha256.json')
verify=work/('sdk-verify-'+uuid.uuid4().hex[:12])
assert verify.resolve().is_relative_to(work) and not verify.exists()
verify.mkdir()
with zipfile.ZipFile(archive) as z:
    assert z.testzip() is None,'ZIP CRC failure'
    for name in z.namelist():
        target=verify/name
        assert target.resolve().is_relative_to(verify),name
    z.extractall(verify)
for item in files:
    target=verify/item['path']
    assert target.stat().st_size==item['bytes'] and sha(target)==item['sha256'],item['path']
assert sha(verify/'sha256.json')==sha(sdk/'sha256.json')
consumer=subprocess.run([str(verify/'bin/sdk-consumer.exe')],cwd=verify/'bin',capture_output=True,text=True,timeout=30)
(work/'sdk-extracted-consumer.log').write_text(consumer.stdout+consumer.stderr,encoding='utf-8')
assert consumer.returncode==0,(consumer.returncode,consumer.stdout,consumer.stderr)
metadata={**identity,'archive':archive.relative_to(root).as_posix(),'bytes':archive.stat().st_size,'sha256':sha(archive),'files':len(files)+1,'file_hashes_verified':len(files),'zip_crc':'PASS','extracted_consumer_exit_code':consumer.returncode,'verification_directory':verify.relative_to(root).as_posix(),'verified_at':datetime.datetime.now(datetime.timezone.utc).isoformat(),'original_framework_artifacts_still_frozen':True}
(work/'sdk-archive.json').write_text(json.dumps(metadata,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(json.dumps(metadata,ensure_ascii=False),flush=True)
