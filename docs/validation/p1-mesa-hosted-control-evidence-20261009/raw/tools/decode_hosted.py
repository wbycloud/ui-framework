import argparse,hashlib,json,pathlib,subprocess

root=pathlib.Path(__file__).resolve().parents[3]
base=root/'build/p1-mesa-hosted-control-20261009'
cdb='C:/Program Files (x86)/Windows Kits/10/Debuggers/x86/cdb.exe'
providers={'A':root/'.deps/mesa-24.3.4/x64/osmesa.dll',
           'C':root/'build/p1-mesa-lifecycle-20261009/candidate-unpacked/osmesa.dll'}
symbols={'A':root/'build/p1-hosted-diagnostic-20261009/mesa-debug/x64',
         'C':providers['C'].parent}
parser=argparse.ArgumentParser();parser.add_argument('configuration',choices=['osmesa','webview2']);args=parser.parse_args()
ledger=[]
for configuration in [args.configuration]:
    artifact=base/'artifacts'/configuration
    control=artifact/'lifecycle-control'
    for capture in sorted(control.rglob('capture.json')):
        for row in json.loads(capture.read_text()):
            obj=row.get('mesa_objects',{})
            if not obj.get('mesa_present'):continue
            label=obj['matched_layout']['label'];pid=row['pid']
            dump=capture.parent/f'{pid}.dmp';assert dump.exists()
            sha=hashlib.sha256(providers[label].read_bytes()).hexdigest();assert sha==obj['sha256']
            command_text='vertarget; .reload /f osmesa.dll; lmv m osmesa; lmv m ntdll; lmv m kernelbase; lmv m osmesa_thread_probe; ~* kb; kn; '
            if obj['exit_flag']:
                command_text+='.frame /r 4; dv /t; .frame /r 5; dv /t; .frame /r 0x12; dv /t; '
            command_text+='dt osmesa!lp_rasterizer '+obj['rast_address']+'; '
            for task in obj['tasks']:
                command_text+='dt osmesa!lp_rasterizer_task '+task['task']+'; !handle '+task['saved_handle']+' f; '
                command_text+='dt osmesa!util_semaphore '+task['semaphores']['exited']['address']+'; '
            command_text+='uf osmesa!lp_rast_destroy; uf osmesa!dllmain_dispatch; uf osmesa!DllMain; uf osmesa!destroy_st_manager; dq poi(osmesa!_tls_used+0x18) L1; q'
            command=[cdb,'-z',str(dump),'-y',str(symbols[label])+';'+str(control/'caller'),
                     '-i',str(providers[label].parent)+';'+str(control/'system-images')+';'+str(control/'caller'),'-c',command_text]
            result=subprocess.run(command,capture_output=True,timeout=55)
            log=base/'analysis'/f'{configuration}-{label}-{pid}-matched-v2.log'
            with log.open('xb') as stream:stream.write(result.stdout+result.stderr)
            text=log.read_text(encoding='utf-8',errors='replace')
            assert 'private pdb symbols' in text and 'lp_rast_destroy' in text
            ledger.append({'configuration':configuration,'label':label,'pid':pid,'gated':not bool(obj['exit_flag']),
                'dump':str(dump.relative_to(root)),'capture':str(capture.relative_to(root)),
                'log':str(log.relative_to(root)),'command':command,'debugger_exit':result.returncode,
                'rasterizer_frames':text.count('osmesa!thread_function+'),'compute_frames':text.count('osmesa!lp_cs_tpool_worker+'),
                'pdb_guid':obj['matched_layout']['pdb_guid']})
            print(json.dumps({k:v for k,v in ledger[-1].items() if k not in ['command','capture']}),flush=True)
(base/f'analysis/{args.configuration}-matched-decodes.json').write_text(json.dumps(ledger,indent=2),encoding='utf-8')
