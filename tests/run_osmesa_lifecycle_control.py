"""Authorized P1 target diagnostic: default-thread original/candidate provider only."""
import argparse,ctypes as c,hashlib,json,os,pathlib,re,shutil,subprocess,sys,threading,time,xml.etree.ElementTree as ET
from ctypes import wintypes as w

def digest(path):
    value=hashlib.sha256()
    with path.open('rb') as stream:
        for part in iter(lambda:stream.read(8*1024*1024),b''):value.update(part)
    return value.hexdigest()

def main():
    parser=argparse.ArgumentParser()
    for name in ['build','original','candidate','evidence']:parser.add_argument('--'+name,required=True,type=pathlib.Path)
    args=parser.parse_args();root=pathlib.Path.cwd().resolve();build=args.build.resolve();out=args.evidence.resolve()
    providers={'A':args.original.resolve(),'C':args.candidate.resolve()}
    assert out.is_relative_to(root/'build') and build.is_relative_to(root/'build')
    assert digest(providers['A'])=='c633b820bba8ec0dcb05466505e3c246e4c4028eaec7caf1054e707c8eb3ca1f'
    assert digest(providers['C'])=='99d8142434c3abfacce969aa3731220314d99c53ee0afc546896c0afad92f5f5'
    out.mkdir(exist_ok=False);(out/'runs').mkdir();(out/'raw').mkdir()
    parent_lp=os.environ.get('LP_NUM_THREADS');user=c.WinDLL('user32',use_last_error=True)
    user.OpenInputDesktop.argtypes=[w.DWORD,w.BOOL,w.DWORD];user.OpenInputDesktop.restype=w.HANDLE
    user.GetUserObjectInformationW.argtypes=[w.HANDLE,c.c_int,c.c_void_p,w.DWORD,c.POINTER(w.DWORD)];user.CloseDesktop.argtypes=[w.HANDLE]
    desktop=user.OpenInputDesktop(0,False,0x100);name=c.create_unicode_buffer(256);needed=w.DWORD()
    unlocked=bool(desktop and user.GetUserObjectInformationW(desktop,2,name,c.sizeof(name),c.byref(needed)))
    if desktop:user.CloseDesktop(desktop)
    assert unlocked and name.value=='Default' and user.GetSystemMetrics(0)>=1920 and user.GetSystemMetrics(1)>=1080
    # Preserve and isolate only the current ephemeral CI build's WGL copies.
    saved=out/'isolated-wgl';saved.mkdir()
    for file in ['opengl32.dll','libgallium_wgl.dll','libglapi.dll','pipe_swrast.dll']:
        path=build/file
        if path.exists():shutil.move(str(path),str(saved/file))
    binaries={str(p.relative_to(root)):digest(p) for p in build.iterdir() if p.is_file() and p.suffix in ['.exe','.dll','.uapp','.lib']}
    images=out/'system-images';images.mkdir()
    for file in ['ntdll.dll','kernel32.dll','KERNELBASE.dll','ucrtbase.dll']:shutil.copyfile(pathlib.Path(os.environ['SystemRoot'])/'System32'/file,images/file)
    plan=subprocess.check_output(['ctest','--test-dir',str(build),'-R','^(ui_stateful_components_light|ui_instance_language_light)$','--show-only=json-v1'])
    (out/'raw/original-test-plan.json').write_bytes(plan);tests=json.loads(plan)['tests']
    assert len(tests)==2 and {test['name'] for test in tests}=={'ui_stateful_components_light','ui_instance_language_light'}
    for test in tests:
        properties={p['name']:p['value'] for p in test['properties']}
        assert properties['TIMEOUT']==(180 if 'stateful' in test['name'] else 240)
    caller=out/'caller';caller.mkdir()
    vswhere=pathlib.Path(os.environ['ProgramFiles(x86)'])/'Microsoft Visual Studio/Installer/vswhere.exe'
    vs=subprocess.check_output([str(vswhere),'-latest','-products','*','-property','installationPath'],text=True).strip()
    dev=pathlib.Path(vs)/'Common7/Tools/VsDevCmd.bat';source=root/'tests/osmesa_thread_probe.c'
    script=caller/'build.cmd';script.write_text(f'@echo off\ncall "{dev}" -arch=x64 -host_arch=x64 >nul\nif errorlevel 1 exit /b 1\ncl /nologo /W4 /WX /utf-8 /std:c11 /O2 /Z7 /MT "{source}" /link user32.lib /DEBUG /OUT:osmesa_thread_probe.exe /PDB:osmesa_thread_probe.pdb\nexit /b %errorlevel%\n',encoding='utf-8')
    with (out/'raw/caller-build.log').open('xb') as stream:subprocess.run(['cmd','/d','/c',str(script)],cwd=caller,stdout=stream,stderr=subprocess.STDOUT,check=True)
    executable=caller/'osmesa_thread_probe.exe';results=[]
    def environment():
        env=os.environ.copy();env.pop('LP_NUM_THREADS',None);env.pop('UI_MESA_PROBE_GATE',None)
        env['GALLIUM_DRIVER']='llvmpipe';env['UI_PROCESS_STALL_SECONDS']='60';env['UI_PROCESS_FULL_DUMP']='1';env['UI_MESA_OBJECTS']='1';return env
    for label,provider in providers.items():
        # One ungated process and one separately labeled live-worker gate.
        for gated in [False,True]:
            directory=out/'runs'/f'probe-{label}-gate{int(gated)}';directory.mkdir();env=environment()
            if gated:env['UI_MESA_PROBE_GATE']='1'
            command=[str(executable),str(provider),'exit'];started=time.monotonic()
            child=subprocess.Popen(command,env=env,stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            activity=[time.monotonic()];gate=threading.Event()
            with (directory/'stdout.log').open('xb') as log:
                def read():
                    for line in iter(child.stdout.readline,b''):
                        log.write(line);log.flush();activity[0]=time.monotonic()
                        if line.startswith(b'GATE rendered'):gate.set()
                reader=threading.Thread(target=read);reader.start();captured=False;released=False;forced=False
                while child.poll() is None:
                    if not captured and (gate.is_set() or time.monotonic()-activity[0]>=10):
                        target=directory/'first-full-dump'
                        capture=subprocess.run([sys.executable,'tests/process_stall_capture.py',str(target),str(child.pid)],env=env,capture_output=True)
                        (directory/'capture.log').write_bytes(capture.stdout+capture.stderr);captured=True
                        if gated and gate.is_set():child.stdin.write(b'\n');child.stdin.flush();released=True
                    if time.monotonic()-started>=45:
                        child.kill();forced=True;break # standalone diagnostic deadline only
                    time.sleep(0.025) # external observer sampling; no delay added to test pump
                child.wait();reader.join()
            results.append({'label':label,'kind':'standalone','command':command,'pid':child.pid,'os_exit':child.returncode,
                'forced':forced,'gated':gated,'gate_released':released,'eof':not reader.is_alive(),'provider_sha256':digest(provider)})
        directory=out/'runs'/f'original-{label}';directory.mkdir();lines=[]
        for test in tests:
            cmd=list(test['command']);cmd[cmd.index('-Provider')+1]=str(provider)
            lines.append('add_test('+test['name']+' '+ ' '.join('[=['+part+']=]' for part in cmd)+')')
            lines.append('set_tests_properties('+test['name']+' PROPERTIES TIMEOUT '+str(180 if 'stateful' in test['name'] else 240)+' RUN_SERIAL TRUE)')
        (directory/'CTestTestfile.cmake').write_text('\n'.join(lines)+'\n',encoding='utf-8')
        for test in tests:
            case=directory/test['name'];case.mkdir()
            command=[sys.executable,'tests/observe_processes.py',str(case/'processes.jsonl'),'--','ctest','--test-dir',str(directory),'-R','^'+test['name']+'$','--no-tests=error','-V','--output-junit',str(case/'ctest.xml')]
            with (case/'observer.log').open('xb') as log:result=subprocess.run(command,env=environment(),stdout=log,stderr=subprocess.STDOUT)
            events=[json.loads(line) for line in (case/'processes.jsonl').read_text(encoding='utf-8').splitlines()]
            native={e['pid'] for e in events if e['kind']=='child_observed' and e['exe'].endswith('_test.exe')}
            exits=[e for e in events if e['kind']=='child_exit' and e['pid'] in native]
            text=(case/'processes.log').read_text(encoding='utf-8');ends=re.findall(r'CASE_END[^\r\n]+',text)
            expected=26 if 'stateful' in test['name'] else 7;cases=ET.parse(case/'ctest.xml').getroot().findall('testcase')
            passed=(result.returncode==0 and len(cases)==1 and cases[0].get('name')==test['name'] and cases[0].find('failure') is None and
                len(native)==len(exits)==len(ends)==expected and all(e['read_ok'] and e['code']==0 for e in exits) and
                any(e['kind']=='stdout_eof' for e in events) and any(e['kind']=='parent_exit' and e['code']==0 for e in events) and
                'dispatched' in text and all('exit=0' in end for end in ends))
            results.append({'label':label,'kind':'original','test':test['name'],'command':command,'observer_exit':result.returncode,
                'native_exits':exits,'case_ends':ends,'complete_boundaries_pass':passed})
    for path in build.glob('*evidence-*'):
        if path.is_dir():shutil.copytree(path,out/'product-evidence'/path.name)
    assert binaries=={str(p.relative_to(root)):digest(p) for p in build.iterdir() if p.is_file() and p.suffix in ['.exe','.dll','.uapp','.lib']}
    assert os.environ.get('LP_NUM_THREADS')==parent_lp
    (out/'raw/results.json').write_text(json.dumps({'head':subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),
        'providers':{k:{'path':str(p),'sha256':digest(p)} for k,p in providers.items()},'caller_sha256':digest(executable),
        'source_sha256':digest(source),'binaries':binaries,'parent_lp_before':parent_lp,'parent_lp_after':os.environ.get('LP_NUM_THREADS'),'runs':results},indent=2),encoding='utf-8')
    return int(any((r['forced'] or r['os_exit']!=0) if r['kind']=='standalone' else not r['complete_boundaries_pass'] for r in results))

if __name__=='__main__':sys.exit(main())
