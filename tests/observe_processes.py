"""Read-only process-boundary observer. Original child scripts/pipes are intact.

Usage: python observe_processes.py OUTPUT.jsonl -- COMMAND ARG ...
Uses only stdlib/Win32; retaining SYNCHRONIZE handles is diagnostic overhead,
not part of measured product handle budgets. Does not change any time gate.
"""
import ctypes as c
from ctypes import wintypes as w
import hashlib, json, os, pathlib, shutil, subprocess, sys, threading, time

class Entry(c.Structure):
    _fields_=[('size',w.DWORD),('usage',w.DWORD),('pid',w.DWORD),('heap',c.c_size_t),('module',w.DWORD),('threads',w.DWORD),('parent',w.DWORD),('priority',w.LONG),('flags',w.DWORD),('exe',w.WCHAR*260)]
k=c.WinDLL('kernel32',use_last_error=True)
k.CreateToolhelp32Snapshot.restype=w.HANDLE;k.OpenProcess.restype=w.HANDLE
k.Process32FirstW.argtypes=[w.HANDLE,c.POINTER(Entry)];k.Process32NextW.argtypes=[w.HANDLE,c.POINTER(Entry)]
k.WaitForSingleObject.argtypes=[w.HANDLE,w.DWORD];k.GetExitCodeProcess.argtypes=[w.HANDLE,c.POINTER(w.DWORD)];k.CloseHandle.argtypes=[w.HANDLE]
path=pathlib.Path(sys.argv[1]);command=sys.argv[3:]
if any(p.exists() for p in (path,path.with_suffix('.log'),path.with_suffix('.patch'),path.with_suffix('.identities.json'))):
    raise SystemExit('Evidence output already exists; select a new run path.')
patch=subprocess.check_output(['git','diff','--binary'])
path.with_suffix('.patch').write_bytes(patch)
files=[pathlib.Path('CMakeLists.txt'),*pathlib.Path('include').rglob('*.h'),*pathlib.Path('tests').glob('*probe.c'),pathlib.Path('tests/observe_processes.py'),pathlib.Path('tests/process_stall_capture.py'),pathlib.Path('tests/framework_features_host.c'),pathlib.Path('src/menus.c'),pathlib.Path('src/components.html')]
build=pathlib.Path('build/root-cause-investigation-20261009/current')
for option in ('--test-dir','-BuildDirectory'):
    if option in command:build=pathlib.Path(command[command.index(option)+1]);break
files+=list(build.glob('*.exe'))+list(build.glob('*.dll'))+list(build.glob('*.lib'))+list(build.glob('*.uapp'))
command_exe=pathlib.Path(shutil.which(command[0]) or command[0])
command_files=[command_exe,*command_exe.parent.glob('*.dll'),*command_exe.parent.glob('*.lib')]
def identify(paths):
    return [{'path':str(f),'bytes':f.stat().st_size,'sha256':hashlib.sha256(f.read_bytes()).hexdigest()} for f in paths if f.is_file()]
identity={'head':subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),'patch_sha256':hashlib.sha256(patch).hexdigest(),'files':identify(files),'actual_command_files':identify(command_files)}
path.with_suffix('.identities.json').write_text(json.dumps(identity,ensure_ascii=False,indent=2),encoding='utf-8')
for f in files:
    if f.is_file() and f.suffix in ('.c','.h','.py','.txt','.html'):
        copy=path.parent/(path.stem+'-source-snapshot')/f;copy.parent.mkdir(parents=True,exist_ok=True);copy.write_bytes(f.read_bytes())
lock=threading.Lock();start=time.monotonic();last_output=start;trace=path.open('x',encoding='utf-8');log=path.with_suffix('.log').open('x',encoding='utf-8')
def emit(kind,**args):
    with lock:
        trace.write(json.dumps({'kind':kind,'elapsed_ms':round((time.monotonic()-start)*1000,3),**args},ensure_ascii=False)+'\n');trace.flush()
emit('observer_start',command=command,cwd=str(pathlib.Path.cwd()),time_ns=time.time_ns(),diagnostic_environment={name:os.environ.get(name) for name in ('UI_RUNTIME_CYCLES','UI_RUNTIME_READY_REOPENS','UI_RUNTIME_RESOURCES','UI_FEATURE_TRACE','UI_FEATURE_PAINT_TRACE','UI_PROCESS_STALL_SECONDS')})
p=subprocess.Popen(command,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,encoding='utf-8',errors='replace')
emit('parent_started',pid=p.pid)
def read():
    global last_output
    for line in p.stdout:
        last_output=time.monotonic()
        log.write(line);log.flush()
        if 'CASE_' in line or 'PHASE' in line or 'failures' in line or 'FINAL ' in line:emit('stdout_line',line=line.rstrip())
    emit('stdout_eof')
reader=threading.Thread(target=read);reader.start()
known={p.pid};handles={};exited=set();parent_recorded=False
stall_seconds=float(os.environ.get('UI_PROCESS_STALL_SECONDS','0'));capture_thread=None
def capture(ids):
    emit('stall_capture_started',pids=ids,idle_seconds=round(time.monotonic()-last_output,3))
    result=subprocess.run([sys.executable,str(pathlib.Path(__file__).with_name('process_stall_capture.py')),str(path.parent/'stall-dumps'),*[str(pid) for pid in ids]],stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
    emit('stall_capture_completed',exit=result.returncode,result=result.stdout.rstrip())
while True:
    parent_code=p.poll()
    if parent_code is not None and not parent_recorded:
        emit('parent_exit',pid=p.pid,code=parent_code);parent_recorded=True
    snapshot=k.CreateToolhelp32Snapshot(2,0);entry=Entry();entry.size=c.sizeof(entry);items=[]
    if k.Process32FirstW(snapshot,c.byref(entry)):
        while True:
            items.append((entry.pid,entry.parent,entry.exe))
            if not k.Process32NextW(snapshot,c.byref(entry)):break
    k.CloseHandle(snapshot)
    for _ in range(3):
        for pid,parent,name in items:
            if pid not in known and parent in known:
                known.add(pid);handle=k.OpenProcess(0x100000|0x1000,False,pid)
                if handle:handles[pid]=handle
                emit('child_observed',pid=pid,parent=parent,exe=name,handle_open=bool(handle),open_error=0 if handle else c.get_last_error())
    for pid,handle in handles.items():
        if pid not in exited and k.WaitForSingleObject(handle,0)==0:
            code=w.DWORD();ok=k.GetExitCodeProcess(handle,c.byref(code));error=0 if ok else c.get_last_error();exited.add(pid);emit('child_exit',pid=pid,code=code.value,read_ok=bool(ok),read_error=error)
    if stall_seconds>0 and capture_thread is None and time.monotonic()-last_output>=stall_seconds:
        ids=([p.pid] if parent_code is None else [])+[pid for pid in handles if pid not in exited]
        capture_thread=threading.Thread(target=capture,args=(ids,));capture_thread.start()
    if parent_code is not None and not reader.is_alive():break
    time.sleep(0.025) # observer sampling only; never delays the child/test pump
reader.join();p.wait()
if capture_thread:capture_thread.join()
for handle in handles.values():k.CloseHandle(handle)
trace.close();log.close();print(json.dumps({'pid':p.pid,'exit':p.returncode,'seconds':round(time.monotonic()-start,3),'trace':str(path)}));sys.exit(p.returncode)
