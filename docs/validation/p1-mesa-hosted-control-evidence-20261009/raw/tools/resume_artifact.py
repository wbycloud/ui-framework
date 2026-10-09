import argparse, concurrent.futures, hashlib, json, pathlib, re, subprocess, threading, time

base = pathlib.Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('configuration')
parser.add_argument('--epoch',default='resume')
parser.add_argument('--workers',type=int,default=16)
args = parser.parse_args()
meta = next(row for row in json.loads((base/'raw/api-artifacts-final.json').read_text())['artifacts']
            if row['name'].startswith('windows-'+args.configuration+'-'))
old = base/'downloads'/args.configuration
out = base/'downloads'/(args.configuration+'-'+args.epoch)
out.mkdir(exist_ok=False)
credential = subprocess.run(['git','credential','fill'], input='protocol=https\nhost=github.com\npath=wbycloud/ui-framework.git\n\n', text=True, capture_output=True, check=True)
token = dict(line.split('=',1) for line in credential.stdout.splitlines() if '=' in line)['password']
lock = threading.Lock()
state = {'url':None,'time':0,'index':0,'refresh':0,'completed':0}

def quote(value):
    return '"'+value.replace('\\','\\\\').replace('"','\\"')+'"'

def get_url(force=False):
    with lock:
        if force or not state['url'] or time.monotonic()-state['time']>360:
            state['refresh'] += 1
            stem = out/f"private-redirect-{state['refresh']:03d}"
            config = '\n'.join(['url = '+quote(meta['archive_download_url']), 'output = '+quote(str(stem)+'.body'),
                'dump-header = '+quote(str(stem)+'.headers'), 'header = '+quote('Authorization: Bearer '+token),
                'header = "Accept: application/vnd.github+json"', 'max-time = 45','silent','show-error'])
            result = subprocess.run(['curl.exe','--config','-'],input=config.encode(),capture_output=True)
            assert result.returncode==0
            urls = re.findall(r'(?im)^Location:\s*([^\r\n]+)',pathlib.Path(str(stem)+'.headers').read_text())
            assert len(urls)==1 and urls[0].startswith('https://')
            state['url'],state['time']=urls[0],time.monotonic()
        return state['url']

segments = []
records=[]
for directory in (base/'downloads').glob(args.configuration+'*'):
    if directory!=out:records.extend(directory.glob('*.json'))
for record in records:
    if record.name=='zip-entry-metadata.json': continue
    row = json.loads(record.read_text())
    body = record.with_suffix('.bin')
    if not body.exists(): continue
    if 'start' in row and row.get('content_range')==[f"bytes {row['start']}-{row['end']}/{meta['size_in_bytes']}"] and 0<body.stat().st_size<=row['end']-row['start']+1:
        segments.append((row['start'],row['start']+body.stat().st_size,body))
segments.sort(key=lambda r:(r[0],-r[1]))
coverage=[]; cursor=0; gaps=[]
for start,end,path in segments:
    if end<=cursor: continue
    if start>cursor: gaps.append((cursor,start))
    take=max(start,cursor)
    coverage.append((take,end,path,take-start));cursor=end
if cursor<meta['size_in_bytes']:gaps.append((cursor,meta['size_in_bytes']))
tasks=[]
for start,end in gaps:
    for position in range(start,end,512*1024): tasks.append((position,min(end,position+512*1024)))
print(json.dumps({'configuration':args.configuration,'reused_bytes':sum(end-start for start,end,_,_ in coverage),'missing_bytes':sum(end-start for start,end in gaps),'requests':len(tasks)}),flush=True)

def fetch(task):
    start,end=task; pieces=[]; position=start
    for attempt in range(8):
        url=get_url()
        with lock:
            state['index']+=1; index=state['index']
        stem=out/f'part-{index:05d}';body=stem.with_suffix('.bin');headers=stem.with_suffix('.headers')
        config='\n'.join(['url = '+quote(url),'output = '+quote(str(body)),'dump-header = '+quote(str(headers)),
            'header = '+quote(f'Range: bytes={position}-{end-1}'),'max-time = 45','connect-timeout = 15','silent','show-error'])
        result=subprocess.run(['curl.exe','--config','-'],input=config.encode(),capture_output=True)
        text=headers.read_text() if headers.exists() else ''
        size=body.stat().st_size if body.exists() else 0
        ranges=re.findall(r'(?im)^Content-Range:\s*([^\r\n]+)',text)
        valid=ranges==[f"bytes {position}-{end-1}/{meta['size_in_bytes']}"] and 0<size<=end-position
        row={'start':position,'end':end-1,'attempt':attempt,'bytes':size,'valid_prefix':valid,'content_range':ranges,'curl_exit':result.returncode,'error':result.stderr.decode('utf-8','replace').replace(url,'<signed-url>')}
        stem.with_suffix('.json').write_text(json.dumps(row,indent=2),encoding='utf-8')
        if valid:
            pieces.append((position,position+size,body,0));position+=size
            if position==end:
                with lock:
                    state['completed']+=1
                    if state['completed']%100==0:print(json.dumps({'configuration':args.configuration,'completed':state['completed'],'total':len(tasks)}),flush=True)
                return pieces
        elif re.search(r'HTTP/\S+ 40[13]',text):get_url(force=True)
    raise RuntimeError({'configuration':args.configuration,'start':start,'end':end,'remaining':end-position})

with concurrent.futures.ThreadPoolExecutor(max_workers=args.workers) as pool:
    for pieces in pool.map(fetch,tasks): coverage.extend(pieces)
coverage.sort(); cursor=0; digest=hashlib.sha256()
archive=base/f'raw/windows-{args.configuration}-verified.zip'
with archive.open('xb') as stream:
    for start,end,path,offset in coverage:
        assert start==cursor,(start,cursor)
        with path.open('rb') as source:
            source.seek(offset);data=source.read(end-start)
        assert len(data)==end-start
        stream.write(data);digest.update(data);cursor=end
assert cursor==meta['size_in_bytes'] and 'sha256:'+digest.hexdigest()==meta['digest']
(base/f'raw/{args.configuration}-download-verification.json').write_text(json.dumps({'artifact':meta,'archive_sha256':digest.hexdigest(),'size':cursor,'reused_segments':len(segments),'new_requests':state['index']},indent=2),encoding='utf-8')
print(json.dumps({'configuration':args.configuration,'archive_verified':True,'bytes':cursor,'sha256':digest.hexdigest()}),flush=True)
