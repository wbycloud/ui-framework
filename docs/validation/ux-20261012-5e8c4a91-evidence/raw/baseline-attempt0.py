import ctypes as c,hashlib,json,pathlib,subprocess
from ctypes import wintypes as w
root=pathlib.Path(__file__).resolve().parents[3]
base=root/'build/ux-20261012-5e8c4a91'
raw=base/'raw';raw.mkdir(parents=True,exist_ok=False)
(base/'baseline').mkdir();(base/'after').mkdir()
def git(*args):return subprocess.check_output(['git',*args],cwd=root).decode('utf-8','replace').strip()
def digest(path):
    value=hashlib.sha256()
    with path.open('rb') as stream:
        for data in iter(lambda:stream.read(8*1024*1024),b''):value.update(data)
    return value.hexdigest()
assert git('rev-parse','HEAD')=='7848a946fcf417eefe87bd63e42f83c8a3f875f3'
assert not git('diff','--name-only') and not git('diff','--cached','--name-only')
groups=[];errors=[];files=[]
for name in ['instance-language-current','ui-repair','p1-mesa-lifecycle','p1-mesa-hosted-control']:
    directory=root/'docs/validation'/(name+'-evidence-20261009')
    index=directory/'sha256.json'
    records=json.loads(index.read_text())['files']
    for row in records:
        path=directory/row['path']
        if not path.exists() or digest(path)!=row['sha256']:errors.append(str(path))
    groups.append({'directory':str(directory.relative_to(root)),'entries':len(records),'index_sha256':digest(index)})
for relative in ['.deps/mesa-24.3.4/x64/osmesa.dll','build/language-current-20261009/ui-framework-sdk0.9.0-dev-api9-44c039f-windows-x64.zip',
                 'src/backends/light_web.c','src/visual.css','examples/framework_host/host.html','examples/framework_host/main.c','src/components.html']:
    path=root/relative
    files.append({'path':relative,'bytes':path.stat().st_size,'sha256':digest(path)})
assert files[0]['sha256']=='c633b820bba8ec0dcb05466505e3c246e4c4028eaec7caf1054e707c8eb3ca1f'
assert files[1]['sha256']=='28ca0b8409b321477f1aa3adce2680fda8cf2cd8a9a35dd4308a5f08ef1f79a2'
assert not errors,errors
kernel=c.WinDLL('kernel32');user=c.WinDLL('user32',use_last_error=True);session=w.DWORD()
kernel.ProcessIdToSessionId(kernel.GetCurrentProcessId(),c.byref(session))
user.OpenInputDesktop.restype=w.HANDLE;user.GetUserObjectInformationW.argtypes=[w.HANDLE,c.c_int,c.c_void_p,w.DWORD,c.POINTER(w.DWORD)];user.CloseDesktop.argtypes=[w.HANDLE]
desktop=user.OpenInputDesktop(0,False,0x100);name=c.create_unicode_buffer(256);needed=w.DWORD()
ok=bool(desktop and user.GetUserObjectInformationW(desktop,2,name,c.sizeof(name),c.byref(needed)))
if desktop:user.CloseDesktop(desktop)
env={'session':session.value,'desktop':name.value if ok else None,'desktop_error':0 if ok else c.get_last_error(),
     'screen':[user.GetSystemMetrics(0),user.GetSystemMetrics(1)],'system_dpi':user.GetDpiForSystem(),
     'user_ui_language':hex(kernel.GetUserDefaultUILanguage()),'system_ui_language':hex(kernel.GetSystemDefaultUILanguage())}
data={'head':git('rev-parse','HEAD'),'branch':git('branch','--show-current'),'status':git('status','--short'),'environment':env,
      'dependencies':{name:git('-C','.deps/'+name,'rev-parse','HEAD') for name in ['lexbor','quickjs']},
      'protected_groups':groups,'errors':errors,'files':files,'new_directory':'ux-20261012-5e8c4a91'}
(raw/'baseline.json').write_text(json.dumps(data,indent=2),encoding='utf-8')
print(json.dumps({'head':data['head'],'environment':env,'protected_entries':sum(g['entries'] for g in groups),'errors':errors}),flush=True)
