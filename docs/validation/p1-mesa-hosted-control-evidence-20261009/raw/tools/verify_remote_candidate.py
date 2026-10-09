import concurrent.futures,hashlib,json,pathlib,re,subprocess
root=pathlib.Path(__file__).resolve().parents[3];base=root/'build/p1-mesa-hosted-control-20261009'
metadata=json.loads((base/'raw/api-candidate-file.json').read_text())
assert metadata['sha']=='ac75374284f4ec97b6ceade6e413753e2c55f6db' and metadata['size']==38073334
out=base/'remote-candidate-ranges';out.mkdir();size=metadata['size'];chunk=1048576
def fetch(index):
 start=index*chunk;end=min(size,start+chunk)-1
 body=out/f'{index:03d}.bin';headers=out/f'{index:03d}.headers'
 command=['curl.exe','--silent','--show-error','--max-time','45','-H',f'Range: bytes={start}-{end}',
    '--dump-header',str(headers),'--output',str(body),metadata['download_url']]
 result=subprocess.run(command,capture_output=True)
 text=headers.read_text(encoding='ascii') if headers.exists() else ''
 row={'index':index,'start':start,'end':end,'exit':result.returncode,'error':result.stderr.decode('utf-8','replace'),
    'content_range':re.findall(r'(?im)^Content-Range:\s*([^\r\n]+)',text),'bytes':body.stat().st_size if body.exists() else 0}
 (out/f'{index:03d}.json').write_text(json.dumps(row,indent=2),encoding='utf-8')
 assert result.returncode==0 and row['content_range']==[f'bytes {start}-{end}/{size}'] and row['bytes']==end-start+1,row
 return row
with concurrent.futures.ThreadPoolExecutor(max_workers=6) as pool:rows=list(pool.map(fetch,range((size+chunk-1)//chunk)))
target=base/'raw/candidate-remote-verified.zip'
with target.open('xb') as stream:
 for row in rows:stream.write((out/f"{row['index']:03d}.bin").read_bytes())
sha=hashlib.sha256(target.read_bytes()).hexdigest()
assert sha=='afb11d7bf599e991f8e2b7df6971813f8bb6bca66f91f85c8f20b7b5cf59c8ae'
(base/'raw/candidate-remote-verification.json').write_text(json.dumps({'url':metadata['download_url'],'size':size,'sha256':sha,'git_blob':metadata['sha'],
 'ranges':rows,'initial_full_download':{'exit':28,'incomplete_bytes':1833536,'preserved':'raw/candidate-from-remote.zip'}},indent=2),encoding='utf-8')
print(json.dumps({'remote_verified':True,'size':size,'sha256':sha}),flush=True)
