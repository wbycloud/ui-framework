import argparse,hashlib,json,os,pathlib,subprocess,time,threading
import psutil
root=pathlib.Path(__file__).resolve().parents[3];base=root/'build/ux-20261012-5e8c4a91'
parser=argparse.ArgumentParser()
parser.add_argument('label');parser.add_argument('--backend',default='light');parser.add_argument('--chrome',default='light')
parser.add_argument('--dpi',default='96');parser.add_argument('--theme',default='light');parser.add_argument('--build',default='baseline')
parser.add_argument('--mode',default='buttons')
args=parser.parse_args();out=base/'runs'/args.label;out.mkdir(parents=True,exist_ok=False)
build=base/args.build;provider=root/'.deps/mesa-24.3.4/x64/osmesa.dll'
command=[str(build/'ux_probe.exe'),str(build/'stateful_components.uapp'),str(provider),args.backend,str(out),args.dpi,args.theme,args.chrome,args.mode]
identities={str(path.relative_to(root)):hashlib.sha256(path.read_bytes()).hexdigest() for path in [build/'ux_probe.exe',build/'ui_framework.dll',build/'stateful_components.uapp',provider]}
started=time.monotonic()
with (out/'stdout.log').open('xb') as stream:
    env=os.environ.copy()
    if args.mode in ('benchmark','dialogready'):env['UI_UX_TRACE']='1'
    if args.mode=='windowops':env['UI_LIGHT_SCRIPT_DIAGNOSTICS']='1'
    child=subprocess.Popen(command,cwd=build,env=env,stdout=stream,stderr=subprocess.STDOUT)
    def monitor():
        with (out/'process-cpu.csv').open('x',encoding='utf-8') as cpu:
            cpu.write('qpcMs,pid,name,cpuSeconds,rssBytes\n')
            while child.poll() is None:
                try:
                    parent=psutil.Process(child.pid)
                    for process in [parent]+parent.children(recursive=True):
                        try:
                            times=process.cpu_times();cpu.write(f'{time.perf_counter()*1000:.6f},{process.pid},{process.name()},{times.user+times.system:.6f},{process.memory_info().rss}\n')
                        except (psutil.NoSuchProcess,psutil.AccessDenied):pass
                except psutil.NoSuchProcess:break
                time.sleep(.01) # External CPU observer only.
    observer=threading.Thread(target=monitor);observer.start()
    try:code=child.wait(timeout=120);forced=False
    except subprocess.TimeoutExpired:
        # Diagnostic failure retained; never count termination as natural exit.
        child.kill();code=child.wait();forced=True
    observer.join(timeout=3)
result={'label':args.label,'command':command,'pid':child.pid,'os_exit':code,'forced':forced,'elapsed_ms':round((time.monotonic()-started)*1000,3),
        'backend':args.backend,'chrome':args.chrome,'program_dpi':args.dpi,'theme':args.theme,'identities':identities,
        'lp_parent':os.environ.get('LP_NUM_THREADS'),'input':'real SendInput','capture':'desktop BitBlt','eof':'stdout file closed after process wait'}
(out/'run.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
print(json.dumps({k:v for k,v in result.items() if k not in ['command','identities']}),flush=True)
print((out/'stdout.log').read_text(encoding='utf-8',errors='replace')[-1800:],flush=True)
