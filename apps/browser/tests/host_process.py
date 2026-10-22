"""Run only the specified delivery host, observe its DLL, then close its own HWND.
No foreground input, process termination or unrelated window operations.
"""
import ctypes as c
from ctypes import wintypes as w
import json, os, subprocess, sys, tempfile, time
from contextlib import contextmanager
from pathlib import Path

kernel=c.WinDLL('kernel32',use_last_error=True)
user=c.WinDLL('user32',use_last_error=True)
class Module(c.Structure):
    _fields_=[('size',w.DWORD),('id',w.DWORD),('pid',w.DWORD),('global_count',w.DWORD),('count',w.DWORD),('base',c.c_void_p),('bytes',w.DWORD),('handle',w.HMODULE),('name',w.WCHAR*256),('path',w.WCHAR*260)]
kernel.CreateToolhelp32Snapshot.argtypes=[w.DWORD,w.DWORD]
kernel.CreateToolhelp32Snapshot.restype=w.HANDLE
kernel.Module32FirstW.argtypes=[w.HANDLE,c.POINTER(Module)]
kernel.Module32NextW.argtypes=[w.HANDLE,c.POINTER(Module)]
kernel.CloseHandle.argtypes=[w.HANDLE]
callback=c.WINFUNCTYPE(w.BOOL,w.HWND,w.LPARAM)
user.EnumWindows.argtypes=[callback,w.LPARAM]
user.GetWindowThreadProcessId.argtypes=[w.HWND,c.POINTER(w.DWORD)]
user.PostMessageW.argtypes=[w.HWND,w.UINT,w.WPARAM,w.LPARAM]
def modules(pid):
    snap=kernel.CreateToolhelp32Snapshot(0x8|0x10,pid)
    if snap==c.c_void_p(-1).value:
        if c.get_last_error() in (24,299):return [] # Process loader not ready yet.
        raise c.WinError(c.get_last_error())
    names=[];m=Module();m.size=c.sizeof(m)
    try:
        ok=kernel.Module32FirstW(snap,c.byref(m))
        while ok:
            names.append(m.name.lower());ok=kernel.Module32NextW(snap,c.byref(m))
    finally:kernel.CloseHandle(snap)
    return names
def windows(pid):
    found=[]
    @callback
    def visit(hwnd,_):
        owner=w.DWORD();user.GetWindowThreadProcessId(hwnd,c.byref(owner))
        if owner.value==pid:found.append(hwnd)
        return True
    user.EnumWindows(visit,0)
    return found

@contextmanager
def protect_recent_file():
    # The production host uses SHGetFolderPath, not LOCALAPPDATA. Hold its own
    # history read-only without delete sharing: the host can read, but its
    # atomic replace must fail. No backup/restore race with another host writer.
    shell=c.WinDLL('shell32');folder=c.create_unicode_buffer(4096)
    assert shell.SHGetFolderPathW(None,0x1c,None,0,folder)==0
    directory=Path(folder.value)/'UiFramework';created=not directory.exists()
    directory.mkdir(exist_ok=True);file=directory/'host-recent-v1.bin'
    before=file.read_bytes() if file.exists() else None
    kernel.CreateFileW.argtypes=[w.LPCWSTR,w.DWORD,w.DWORD,c.c_void_p,w.DWORD,w.DWORD,w.HANDLE]
    kernel.CreateFileW.restype=w.HANDLE
    handle=kernel.CreateFileW(str(file),0x80000000,1,None,3,0,None)
    if handle==c.c_void_p(-1).value and c.get_last_error()==2:
        handle=kernel.CreateFileW(str(file),0x80010000,1,None,1,0x04000000,None)
    if handle==c.c_void_p(-1).value:raise c.WinError(c.get_last_error())
    try:yield
    finally:
        kernel.CloseHandle(handle)
        after=file.read_bytes() if file.exists() else None
        assert after==before,'Host history changed during protected process check'
        if created:directory.rmdir()
        print(json.dumps({'host_history_unchanged':True,'initially_present':before is not None}))

folder=Path(sys.argv[1]).resolve()
with protect_recent_file(), tempfile.TemporaryDirectory(prefix='browser-host-m2-') as profile:
 env=dict(os.environ);env['LOCALAPPDATA']=profile
 for with_browser in (False,True):
     startup=subprocess.STARTUPINFO();startup.dwFlags|=subprocess.STARTF_USESHOWWINDOW;startup.wShowWindow=0
     args=[str(folder/'framework_host.exe')]+([str(folder/'browser.uapp')] if with_browser else [])
     process=subprocess.Popen(args,cwd=folder,startupinfo=startup,env=env)
     observed=False
     try:
         end=time.monotonic()+8
         while time.monotonic()<end and process.poll() is None:
             loaded='browser_app.dll' in modules(process.pid)
             handles=windows(process.pid)
             if handles and (loaded if with_browser else not loaded):
                 time.sleep(.25)
                 loaded='browser_app.dll' in modules(process.pid)
                 if loaded==with_browser:observed=True;break
             time.sleep(.02)
         assert observed, 'Host window and expected browser module state not observed'
     finally:
         for hwnd in windows(process.pid):user.PostMessageW(hwnd,0x10,0,0)
         code=process.wait(timeout=10)
     assert code==0,code
     print(json.dumps({'executable':str(folder/'framework_host.exe'),'with_browser':with_browser,'pid':process.pid,'expected_module_observed':observed,'exit_code':code,'process_exited':True},ensure_ascii=False))
