import hashlib,json,pathlib,re,struct,uuid

root=pathlib.Path(__file__).resolve().parents[3];base=root/'build/p1-mesa-hosted-control-20261009'
def digest(path):
    value=hashlib.sha256()
    with path.open('rb') as stream:
        for data in iter(lambda:stream.read(8*1024*1024),b''):value.update(data)
    return value.hexdigest()
def pe(path):
    data=path.read_bytes();at=struct.unpack_from('<I',data,0x3c)[0]
    assert data[at:at+4]==b'PE\0\0'
    machine,sections,timestamp=struct.unpack_from('<HHI',data,at+4)
    optional=at+24;size,checksum=struct.unpack_from('<II',data,optional+56)
    checksum=struct.unpack_from('<I',data,optional+64)[0]
    return {'path':str(path.relative_to(root)),'sha256':hashlib.sha256(data).hexdigest(),'image_size':size,'timestamp':timestamp,'checksum':checksum,'machine':hex(machine)}
rows=[]
for configuration in ['osmesa','webview2']:
    artifact=base/'artifacts'/configuration;control=artifact/'lifecycle-control'
    summary=json.loads((base/f'analysis/{configuration}-summary.json').read_text())
    decodes=json.loads((base/f'analysis/{configuration}-matched-decodes.json').read_text())
    plan=json.loads((control/'raw/original-test-plan.json').read_text())['tests']
    for label in ['A','C']:
        text=(control/'runs'/('original-'+label)/'CTestTestfile.cmake').read_text()
        for test in plan:
            line=next(line for line in text.splitlines() if line.startswith('add_test('+test['name']+' '))
            actual=re.findall(r'\[=\[(.*?)\]=\]',line)
            expected=list(test['command']);expected[expected.index('-Provider')+1]=summary['providers'][label]['path']
            assert actual==expected
            assert 'TIMEOUT '+str(180 if 'stateful' in test['name'] else 240)+' RUN_SERIAL TRUE' in text
    for decode in decodes:
        dump=root/decode['dump'];data=dump.read_bytes();assert data[:4]==b'MDMP'
        count,directory=struct.unpack_from('<II',data,8);modules=[]
        for index in range(count):
            kind,size,rva=struct.unpack_from('<III',data,directory+index*12)
            if kind!=4:continue
            for i in range(struct.unpack_from('<I',data,rva)[0]):
                at=rva+4+i*108
                address,image_size,checksum,timestamp,name_rva=struct.unpack_from('<QIIII',data,at)
                length=struct.unpack_from('<I',data,name_rva)[0];name=data[name_rva+4:name_rva+4+length].decode('utf-16le')
                cv_size,cv_rva=struct.unpack_from('<II',data,at+76);cv=data[cv_rva:cv_rva+cv_size]
                module={'name':name,'base':hex(address),'image_size':image_size,'timestamp':timestamp,'checksum':checksum}
                if cv[:4]==b'RSDS':module.update(pdb_guid=str(uuid.UUID(bytes_le=cv[4:20])),pdb_age=struct.unpack_from('<I',cv,20)[0])
                modules.append(module)
        mesa=next(m for m in modules if pathlib.Path(m['name']).name.lower()=='osmesa.dll')
        assert mesa['pdb_guid']==decode['pdb_guid'] and mesa['pdb_age']==1
        provider=root/'.deps/mesa-24.3.4/x64/osmesa.dll' if decode['label']=='A' else artifact/'lifecycle-candidate/unpacked/osmesa.dll'
        chosen=[provider]+list((control/'system-images').glob('*.dll'))
        if decode['gated']:chosen.append(control/'caller/osmesa_thread_probe.exe')
        matches=[]
        for image in chosen:
            identity=pe(image);module=next(m for m in modules if pathlib.Path(m['name']).name.lower()==image.name.lower())
            assert all(identity[k]==module[k] for k in ['image_size','timestamp','checksum'])
            matches.append({'pe':identity,'dump_module':module})
        log=(root/decode['log']).read_text(encoding='utf-8',errors='replace')
        threads=[]
        for text in re.split(r'\n[. ]*\d+\s+Id:',log)[1:]:
            kind='other'
            if 'osmesa!thread_function' in text:kind='rasterizer'
            elif 'osmesa!lp_cs_tpool_worker' in text:kind='compute'
            elif 'osmesa!util_queue_thread_func' in text:kind='utility'
            elif 'osmesa_thread_probe!' in text:kind='probe-main'
            threads.append({'id':text.splitlines()[0].strip(),'kind':kind})
        counts={kind:sum(t['kind']==kind for t in threads) for kind in ['rasterizer','compute','utility','probe-main','other']}
        capture=json.loads((root/decode['capture']).read_text());obj=next(r['mesa_objects'] for r in capture if r['pid']==decode['pid'])
        if decode['gated']:assert counts['rasterizer']==counts['compute']==obj['num_threads']==4
        else:
            assert len(threads)==1 and 'lp_rast_destroy+0xdc' in log and 'unsigned long exit_code = 0x103' in log
            assert 'unsigned long reason = 0' in log and 'reserved = 0x00000000`00000001' in log
            sema=re.search(r'struct util_semaphore \* sema = (0x[0-9a-f`]+)',log).group(1)
            assert int(sema.replace('`',''),16)==int(obj['tasks'][0]['semaphores']['exited']['address'],16)
            assert all(t['duplicate_ok'] and t['query_ok'] and t['exit_code']==0 and t['query_error']==0 and t['zero_wait']==0 and t['wait_error']==0 for t in obj['tasks'])
            assert all(t['semaphores']['exited']['counter']==0 for t in obj['tasks'])
        rows.append({'configuration':configuration,'pid':decode['pid'],'label':decode['label'],'gated':decode['gated'],
            'matches':matches,'threads':threads,'thread_counts':counts,'dump_sha256':digest(dump),'same_wait_task0':not decode['gated'],
            'limitation':'Framework/test PE images not retained in the target artifact; recorded SHA only. Windows symbols are exports; Mesa/caller private symbols matched.'})
(base/'analysis/hosted-object-checks.json').write_text(json.dumps({'checks':rows,'original_commands_and_timeouts':'matched','pass':True},indent=2),encoding='utf-8')
print(json.dumps({'matched_dumps':len(rows),'matched_images':sum(len(r['matches']) for r in rows),'counts':[{'configuration':r['configuration'],'pid':r['pid'],'counts':r['thread_counts']} for r in rows],'pass':True}),flush=True)
