"""Verify newly generated archives, unpack/repack via public ABI, and run them.
Requires the fixed loopback fixture server. No foreground input is sent.
"""
from pathlib import Path
import ctypes as c
import datetime, hashlib, json, os, subprocess, sys, zipfile

APP=Path(__file__).resolve().parents[1]
OUT=APP/'deliverables'
STAGE=APP/'.work'/'delivery-check'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def manifest(root):
    file=root/'SHA256SUMS.txt';listed=set()
    for line in file.read_text(encoding='utf-8').splitlines():
        digest,name=line.split('  ',1);path=(root/name).resolve()
        assert path.is_relative_to(root.resolve()) and path.is_file(),name
        assert sha(path)==digest,name
        listed.add(name)
    actual={p.relative_to(root).as_posix() for p in root.rglob('*') if p.is_file() and p!=file}
    assert listed==actual,(listed^actual)
    return len(listed)

STAGE.mkdir(parents=True,exist_ok=False)
result={'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'archives':{},'runs':[]}
for name in ('browser-sdk-20261020','browser-runtime-20261020','browser-source-20261020','ui-framework-source-20261020'):
    archive=OUT/(name+'.zip')
    with zipfile.ZipFile(archive) as z:
        for entry in z.infolist():
            assert (STAGE/entry.filename).resolve().is_relative_to(STAGE.resolve())
        z.extractall(STAGE)
    root=STAGE/name
    result['archives'][name]={'sha256':sha(archive),'verified_files':manifest(root)}

sdk=STAGE/'browser-sdk-20261020';runtime=STAGE/'browser-runtime-20261020'
assert sha(OUT/'browser.uapp')==sha(sdk/'bin'/'browser.uapp')==sha(runtime/'browser.uapp')
env=dict(os.environ);env['PATH']=str(sdk/'bin')+os.pathsep+env.get('PATH','')
logs=STAGE/'logs';logs.mkdir()
def run(label,args):
    completed=subprocess.run([str(a) for a in args],env=env,cwd=STAGE,capture_output=True,timeout=60)
    text=completed.stdout.decode('utf-8','replace');error=completed.stderr.decode('utf-8','replace')
    (logs/(label+'.txt')).write_text(text+error,encoding='utf-8')
    evidence=[line for line in text.splitlines() if any(word in line for word in ('failures=','memory private_','DLL unloaded=','process_exited','TLS '))]
    result['runs'].append({'label':label,'exit_code':completed.returncode,'evidence':evidence})
    assert completed.returncode==0,(label,text[-3000:],error)
    print(label+': exit=0',flush=True)

for name in ('document','url','failure','workspace'):
    args=[sdk/'verify'/f'browser_{name}_check.exe']
    if name in ('failure','workspace'):args.append(sdk/'bin'/'browser.uapp')
    run(name,args)

# Public package functions, no internal format parsing or private loader.
dll=c.CDLL(str(sdk/'bin'/'ui_framework.dll'))
class Entry(c.Structure):_fields_=[('name',c.c_char_p),('length',c.c_uint64)]
dll.ui_package_open.argtypes=[c.c_char_p,c.POINTER(c.c_void_p),c.c_char_p,c.c_size_t]
dll.ui_package_entry_count.argtypes=[c.c_void_p];dll.ui_package_entry_count.restype=c.c_size_t
dll.ui_package_get_entry.argtypes=[c.c_void_p,c.c_size_t,c.POINTER(Entry)]
dll.ui_package_read.argtypes=[c.c_void_p,c.c_char_p,c.POINTER(c.c_void_p),c.POINTER(c.c_size_t)]
dll.ui_package_release.argtypes=[c.c_void_p];dll.ui_package_destroy.argtypes=[c.c_void_p]
dll.ui_package_pack.argtypes=[c.c_char_p,c.c_char_p,c.c_char_p,c.c_char_p,c.c_size_t]
package=c.c_void_p();error=c.create_string_buffer(1024)
assert dll.ui_package_open(str(sdk/'bin'/'browser.uapp').encode(),c.byref(package),error,len(error))==0,error.value
unpacked=STAGE/'unpacked-browser';unpacked.mkdir();entries=[]
try:
    for i in range(dll.ui_package_entry_count(package)):
        entry=Entry();assert dll.ui_package_get_entry(package,i,c.byref(entry))==0
        name=entry.name.decode();target=(unpacked/name).resolve();assert target.is_relative_to(unpacked.resolve())
        data=c.c_void_p();length=c.c_size_t()
        assert dll.ui_package_read(package,entry.name,c.byref(data),c.byref(length))==0
        try:
            target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(c.string_at(data,length.value))
            entries.append({'name':name,'bytes':length.value,'sha256':sha(target)})
        finally:dll.ui_package_release(data)
finally:dll.ui_package_destroy(package)
roundtrip=STAGE/'roundtrip.uapp'
assert dll.ui_package_pack(str(unpacked/'app.ini').encode(),str(unpacked).encode(),str(roundtrip).encode(),error,len(error))==0,error.value
assert sha(roundtrip)==sha(OUT/'browser.uapp'),'Package round trip differs'
result['package_roundtrip']={'identical':True,'sha256':sha(roundtrip),'entries':entries}
run('repacked-workspace',[sdk/'verify'/'browser_workspace_check.exe',roundtrip])
run('runtime-host',[sys.executable,APP/'tests'/'host_process.py',runtime])
result['tls_trust']=json.loads((APP/'.work'/'tls-trust-result.json').read_text(encoding='utf-8-sig'))
assert result['tls_trust']['remaining_count']==0 and result['tls_trust']['test_exit_code']==0
result['earlier_validation']={}
for name in ('trusted-https','after-trust-removed','official-desktop-final','desktop-run3','host-entry2','regression-final'):
    text=(APP/'.work'/(name+'.txt')).read_text(encoding='utf-8-sig',errors='replace')
    result['earlier_validation'][name]=[line for line in text.splitlines() if any(word in line for word in ('failures=','100% tests passed','TLS ','visible endpoint=','desktop restored','DLL unloaded='))]
destination=OUT/'verification.json'
assert not destination.exists(),'Refusing to replace a formal verification result'
destination.write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print('DELIVERY_VERIFIED: archive hashes, public ABI, actual package unload, repack, host process exit',flush=True)
