from pathlib import Path
import hashlib,json,subprocess,datetime,ctypes
r=Path.cwd();w=r/'build/language-20261008';build=w/'after'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
files=[]
for p in sorted(build.iterdir()):
 if p.is_file() and p.suffix in ('.exe','.dll','.lib','.uapp'):files.append({'path':p.relative_to(r).as_posix(),'sha256':sha(p)})
record={'product_commit':subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),'captured_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'artifacts':files}
(w/'product-artifacts.json').write_text(json.dumps(record,ensure_ascii=False,indent=2),encoding='utf-8')
loader=ctypes.WinDLL(str(r/'.deps/Microsoft.Web.WebView2.1.0.4129.50/build/native/x64/WebView2Loader.dll'));version=ctypes.c_void_p();fn=loader.GetAvailableCoreWebView2BrowserVersionString;fn.argtypes=[ctypes.c_wchar_p,ctypes.POINTER(ctypes.c_void_p)];fn.restype=ctypes.c_int32;hr=fn(None,ctypes.byref(version));runtime=ctypes.wstring_at(version) if hr>=0 and version.value else None
if version.value:ctypes.windll.ole32.CoTaskMemFree(ctypes.c_void_p(version.value))
deps={'WebView2Sdk':'1.0.4129.50','runtime':runtime,'runtime_lookup_hresult':hr,'osmesa':'24.3.4','osmesa_sha256':sha(r/'.deps/mesa-24.3.4/x64/osmesa.dll'),'source_commits':{name:subprocess.check_output(['git','-C',str(r/'.deps'/name),'rev-parse','HEAD'],text=True).strip() for name in ['lexbor','quickjs']}}
(w/'dependencies-final.json').write_text(json.dumps(deps,indent=2),encoding='utf-8');print('Product identity',record['product_commit'],'artifacts',len(files),'actual Runtime',runtime)
advapi=ctypes.WinDLL('advapi32',use_last_error=True);advapi.OpenSCManagerW.argtypes=[ctypes.c_wchar_p,ctypes.c_wchar_p,ctypes.c_uint32];advapi.OpenSCManagerW.restype=ctypes.c_void_p
handle=advapi.OpenSCManagerW(None,None,2);error=ctypes.get_last_error();env={'session0_service_create_access':bool(handle),'error':0 if handle else error,'remote_authorization':False,'strict_no_login_runner':'user confirmed absent','physical_ime_and_monitors':'not verified; program DPI does not replace physical conditions'}
if handle:advapi.CloseServiceHandle.argtypes=[ctypes.c_void_p];advapi.CloseServiceHandle(handle)
(w/'environment-final.json').write_text(json.dumps(env,indent=2),encoding='utf-8');print('Read-only service access audit',env)
