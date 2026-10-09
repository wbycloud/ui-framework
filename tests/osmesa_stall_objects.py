"""Opt-in, read-only live Mesa objects after full dump (or a labeled gate).

Private layouts are from matching A/C PDBs, not a public provider interface.
Only exact DLL SHA matches are read. Duplicate the saved HANDLE, never OpenThread.
Observations occur after the dump and can affect timing; query history is unknown.
"""
import ctypes as c
from ctypes import wintypes as w
import hashlib,pathlib,struct,time

LAYOUTS={
 'c633b820bba8ec0dcb05466505e3c246e4c4028eaec7caf1054e707c8eb3ca1f':
    {'label':'A','global_rva':0x30702c0,'pdb_guid':'a53e732b-2253-4fee-8fee-53f47771d8ea'},
 '99d8142434c3abfacce969aa3731220314d99c53ee0afc546896c0afad92f5f5':
    {'label':'C','global_rva':0x2fdd840,'pdb_guid':'2f352030-3f29-4fe7-a7c1-b1ed70c195f3'}}

def observe(pid,directory):
    k=c.WinDLL('kernel32',use_last_error=True);p=c.WinDLL('psapi',use_last_error=True)
    k.OpenProcess.argtypes=[w.DWORD,w.BOOL,w.DWORD];k.OpenProcess.restype=w.HANDLE
    k.ReadProcessMemory.argtypes=[w.HANDLE,c.c_void_p,c.c_void_p,c.c_size_t,c.POINTER(c.c_size_t)]
    k.DuplicateHandle.argtypes=[w.HANDLE,w.HANDLE,w.HANDLE,c.POINTER(w.HANDLE),w.DWORD,w.BOOL,w.DWORD]
    k.GetCurrentProcess.restype=w.HANDLE;k.GetThreadId.argtypes=[w.HANDLE];k.GetThreadId.restype=w.DWORD
    k.GetExitCodeThread.argtypes=[w.HANDLE,c.POINTER(w.DWORD)]
    k.WaitForSingleObject.argtypes=[w.HANDLE,w.DWORD];k.WaitForSingleObject.restype=w.DWORD
    k.CloseHandle.argtypes=[w.HANDLE]
    p.EnumProcessModulesEx.argtypes=[w.HANDLE,c.POINTER(w.HMODULE),w.DWORD,c.POINTER(w.DWORD),w.DWORD]
    p.GetModuleFileNameExW.argtypes=[w.HANDLE,w.HMODULE,w.LPWSTR,w.DWORD]
    row={'pid':pid,'observed_ns':time.time_ns(),'timing_changed':True,'reads':[]}
    process=k.OpenProcess(0x450,False,pid)
    row['open_ok']=bool(process);row['open_error']=0 if process else c.get_last_error()
    if not process:return row
    try:
        modules=(w.HMODULE*256)();needed=w.DWORD()
        ok=bool(p.EnumProcessModulesEx(process,modules,c.sizeof(modules),c.byref(needed),2))
        row['modules_ok']=ok;row['modules_error']=0 if ok else c.get_last_error()
        if not ok or needed.value>c.sizeof(modules):return row
        module=None
        for address in modules[:needed.value//c.sizeof(w.HMODULE)]:
            name=c.create_unicode_buffer(32768)
            if p.GetModuleFileNameExW(process,address,name,len(name)) and pathlib.Path(name.value).name.lower()=='osmesa.dll':
                module=(address,pathlib.Path(name.value));break
        if module is None:row['mesa_present']=False;return row
        address,path=module;sha=hashlib.sha256(path.read_bytes()).hexdigest();layout=LAYOUTS.get(sha)
        row.update(mesa_present=True,module_base=hex(address),path=str(path),sha256=sha,matched_layout=layout)
        if layout is None:return row
        def read(address,size):
            data=c.create_string_buffer(size);count=c.c_size_t()
            ok=bool(k.ReadProcessMemory(process,address,data,size,c.byref(count)))
            row['reads'].append({'address':hex(address),'bytes':count.value,'ok':ok,'error':0 if ok else c.get_last_error()})
            if not ok or count.value!=size:raise RuntimeError('ReadProcessMemory failed or partial')
            return data.raw
        def pointer(address):return struct.unpack('<Q',read(address,8))[0]
        frontend=pointer(address+layout['global_rva']);row['global_fscreen']=hex(frontend)
        if not frontend:return row
        screen=pointer(frontend);row['screen']=hex(screen)
        if not screen:return row
        rast=pointer(screen+0x290);row['rast_address']=hex(rast)
        row['cs_tpool_pointer']=hex(pointer(screen+0x2c0)) # may already be freed during screen destruction
        if not rast:return row
        data=read(rast,0x2a68);(pathlib.Path(directory)/f'{pid}-rast-live.bin').write_bytes(data)
        number=struct.unpack_from('<I',data,0x2918)[0]
        row.update(num_threads=number,exit_flag=data[0],tasks=[])
        if number>32:raise RuntimeError('Invalid num_threads for matched layout')
        for index in range(number):
            task_offset=0x18+index*0x148;task=rast+task_offset
            handle=struct.unpack_from('<Q',data,0x2920+index*8)[0]
            entry={'index':index,'task':hex(task),'saved_handle':hex(handle),'task_index':struct.unpack_from('<I',data,task_offset+0x78)[0],
                'semaphores':{name:{'address':hex(task+offset),'counter':struct.unpack_from('<i',data,task_offset+offset+0x30)[0]}
                    for name,offset in [('work_ready',0xa0),('work_done',0xd8),('exited',0x110)]}}
            duplicate=w.HANDLE();copied=bool(k.DuplicateHandle(process,handle,k.GetCurrentProcess(),c.byref(duplicate),0,False,2))
            entry.update(duplicate_ok=copied,duplicate_error=0 if copied else c.get_last_error())
            if copied:
                try:
                    tid=k.GetThreadId(duplicate);entry.update(thread_id=tid,thread_id_error=0 if tid else c.get_last_error())
                    code=w.DWORD();queried=bool(k.GetExitCodeThread(duplicate,c.byref(code)))
                    entry.update(query_ok=queried,exit_code=code.value,query_error=0 if queried else c.get_last_error())
                    wait=k.WaitForSingleObject(duplicate,0);entry.update(zero_wait=wait,wait_error=c.get_last_error() if wait==0xffffffff else 0)
                finally:
                    entry['observer_close_ok']=bool(k.CloseHandle(duplicate))
            row['tasks'].append(entry)
    except (OSError,RuntimeError) as error:row['observation_error']=str(error)
    finally:k.CloseHandle(process)
    return row
