"""External, sequential DbgHelp capture for an opted-in P1 observer stall.

Full memory is opt-in through UI_PROCESS_FULL_DUMP=1. No termination,
target-code injection or gate change.
Dumping may affect timing; this is diagnostic evidence, never normal acceptance.
"""
import ctypes as c
from ctypes import wintypes as w
import json, os, pathlib, sys
k=c.WinDLL('kernel32',use_last_error=True)
k.OpenProcess.argtypes=[w.DWORD,w.BOOL,w.DWORD];k.OpenProcess.restype=w.HANDLE
k.CreateFileW.argtypes=[w.LPCWSTR,w.DWORD,w.DWORD,c.c_void_p,w.DWORD,w.DWORD,w.HANDLE];k.CreateFileW.restype=w.HANDLE
k.CloseHandle.argtypes=[w.HANDLE]
d=c.WinDLL('dbghelp',use_last_error=True)
d.MiniDumpWriteDump.argtypes=[w.HANDLE,w.DWORD,w.HANDLE,w.DWORD,c.c_void_p,c.c_void_p,c.c_void_p]
directory=pathlib.Path(sys.argv[1]);directory.mkdir(exist_ok=False)
records=[]
flags=0x1826 if os.environ.get('UI_PROCESS_FULL_DUMP')=='1' else 0x1004
for argument in sys.argv[2:]:
    pid=int(argument);p=k.OpenProcess(0x450,False,pid);f=None
    row={'pid':pid,'open_process':bool(p),'error':c.get_last_error() if not p else 0}
    if p:
        target=directory/(str(pid)+'.dmp')
        f=k.CreateFileW(str(target.resolve()),0x40000000,0,None,1,0x80,None)
        if f and f!=c.c_void_p(-1).value:
            row['flags']=hex(flags)
            row['dump_ok']=bool(d.MiniDumpWriteDump(p,pid,f,flags,None,None,None))
            row['error']=0 if row['dump_ok'] else c.get_last_error()
            k.CloseHandle(f);row['bytes']=target.stat().st_size
        else:row['file_error']=c.get_last_error()
        if os.environ.get('UI_MESA_OBJECTS')=='1':
            from osmesa_stall_objects import observe
            row['mesa_objects']=observe(pid,directory)
        k.CloseHandle(p)
    records.append(row)
(directory/'capture.json').write_text(json.dumps(records,indent=2),encoding='utf-8')
print(json.dumps(records),flush=True)
