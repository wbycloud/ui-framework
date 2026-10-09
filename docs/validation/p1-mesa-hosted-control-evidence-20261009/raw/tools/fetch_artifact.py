import argparse,concurrent.futures,hashlib,json,pathlib,re,subprocess,threading,zipfile
root=pathlib.Path(__file__).resolve().parents[3];base=root/'build/p1-mesa-hosted-control-20261009'
parser=argparse.ArgumentParser();parser.add_argument('configuration');parser.add_argument('inventory',type=pathlib.Path);args=parser.parse_args()
meta=next(row for row in json.loads(args.inventory.read_text())['artifacts'] if row['name'].startswith('windows-'+args.configuration+'-'))
directory=base/'downloads'/args.configuration;directory.mkdir(parents=True,exist_ok=False)
credential=subprocess.run(['git','credential','fill'],input='protocol=https\nhost=github.com\npath=wbycloud/ui-framework.git\n\n',text=True,capture_output=True,check=True)
fields=dict(line.split('=',1) for line in credential.stdout.splitlines() if '=' in line);token=fields['password']
def quote(value):return '"'+value.replace('\\','\\\\').replace('"','\\"')+'"'
route=meta['archive_download_url']
headers=directory/'private-redirect.headers';body=directory/'redirect-body'
config='\n'.join(['url = '+quote(route),'output = '+quote(str(body)), 'dump-header = '+quote(str(headers)),
 'header = '+quote('Authorization: Bearer '+token),'header = "Accept: application/vnd.github+json"','max-time = 60','silent','show-error'])
result=subprocess.run(['curl.exe','--config','-'],input=config.encode(),capture_output=True);assert result.returncode==0
locations=re.findall(r'(?im)^Location:\s*([^\r\n]+)',headers.read_text());assert len(locations)==1 and locations[0].startswith('https://')
url=locations[0];counter=0;lock=threading.Lock()
def fetch(start,end,phase):
 global counter
 for attempt in range(2):
  with lock:counter+=1;index=counter
  output=directory/f'{phase}-{index:04d}.bin';response=directory/f'{phase}-{index:04d}.headers'
  config='\n'.join(['url = '+quote(url),'output = '+quote(str(output)), 'dump-header = '+quote(str(response)),
    'header = '+quote(f'Range: bytes={start}-{end}'),'max-time = 45','connect-timeout = 15','silent','show-error'])
  result=subprocess.run(['curl.exe','--config','-'],input=config.encode(),capture_output=True)
  text=response.read_text() if response.exists() else ''
  row={'index':index,'start':start,'end':end,'attempt':attempt,'curl_exit':result.returncode,
   'error':result.stderr.decode('utf-8','replace').replace(url,'<signed-url>'),'content_range':re.findall(r'(?im)^Content-Range:\s*([^\r\n]+)',text),
   'bytes':output.stat().st_size if output.exists() else 0}
  (directory/f'{phase}-{index:04d}.json').write_text(json.dumps(row,indent=2),encoding='utf-8')
  if result.returncode==0 and row['content_range']==[f"bytes {start}-{end}/{meta['size_in_bytes']}"] and row['bytes']==end-start+1:return output
 raise RuntimeError(json.dumps(row))
class Remote:
 def __init__(self):self.position=0
 def seek(self,offset,whence=0):
  self.position=offset if whence==0 else self.position+offset if whence==1 else meta['size_in_bytes']+offset
  return self.position
 def tell(self):return self.position
 def seekable(self):return True
 def read(self,size=-1):
  if size<0:size=meta['size_in_bytes']-self.position
  size=min(size,meta['size_in_bytes']-self.position)
  if size<=0:return b''
  start=self.position;self.position+=size
  return fetch(start,start+size-1,'preview').read_bytes()
preview=base/'artifact-preview'/args.configuration;preview.mkdir(parents=True,exist_ok=False)
with zipfile.ZipFile(Remote()) as package:
 entries=package.infolist();(directory/'zip-entry-metadata.json').write_text(json.dumps([{'path':e.filename,'bytes':e.file_size,'compressed':e.compress_size,'offset':e.header_offset} for e in entries],indent=2))
 selected=[e for e in entries if e.filename.endswith(('lifecycle-control/raw/results.json','lifecycle-desktop.json','/capture.json'))]
 for entry in selected:
  target=(preview/entry.filename).resolve();assert target.is_relative_to(preview.resolve());target.parent.mkdir(parents=True,exist_ok=True)
  target.write_bytes(package.read(entry))
 results=list(preview.rglob('lifecycle-control/raw/results.json'))
 if results:
  rows=json.loads(results[0].read_text())['runs']
  print(json.dumps({'configuration':args.configuration,'preview_runs':[{'label':r['label'],'kind':r['kind'],'test':r.get('test'),'os_exit':r.get('os_exit'),'forced':r.get('forced'),
    'gated':r.get('gated'),'pass':r.get('complete_boundaries_pass'),'native_count':len(r.get('native_exits',[]))} for r in rows]}),flush=True)
size=meta['size_in_bytes'];chunk=2*1048576
def part(index):return index,fetch(index*chunk,min(size,(index+1)*chunk)-1,'full')
with concurrent.futures.ThreadPoolExecutor(max_workers=8) as pool:parts=list(pool.map(part,range((size+chunk-1)//chunk)))
archive=base/f'raw/windows-{args.configuration}-verified.zip';value=hashlib.sha256()
with archive.open('xb') as stream:
 for index,path in sorted(parts):
  data=path.read_bytes();stream.write(data);value.update(data)
assert 'sha256:'+value.hexdigest()==meta['digest']
(base/f'raw/{args.configuration}-download-verification.json').write_text(json.dumps({'artifact':meta,'archive_sha256':value.hexdigest(),'size':size,'http_ranges':len(parts)},indent=2),encoding='utf-8')
print(json.dumps({'configuration':args.configuration,'archive_verified':True,'bytes':size,'sha256':value.hexdigest()}),flush=True)
