import csv,json,re,pathlib,collections,math
base=pathlib.Path(__file__).resolve().parents[1]
def stats(values):
    values=sorted(values)
    def q(p):
        x=(len(values)-1)*p;i=int(x);return values[i]+(values[min(i+1,len(values)-1)]-values[i])*(x-i)
    return {'n':len(values),'p95':q(.95),'p99':q(.99),'worst':max(values)} if values else {'n':0}
result={}
for run in sorted((base/'runs').glob('perf-*-v4-*-96')):
    if not (run/'run.json').exists():continue
    meta=json.loads((run/'run.json').read_text());groups=collections.defaultdict(list)
    for row in csv.DictReader((run/'measurements.csv').open()):groups[row['category']].append(row)
    categories={}
    for name,rows in groups.items():
        categories[name]={'samples':len(rows),'invalid':sum(r['valid']!='1' for r in rows),
            'missingDesktopChange':sum(float(r['firstDesktopChangeMs'])<0 for r in rows),
            'desktop':stats([float(r['firstDesktopChangeMs']) for r in rows if float(r['firstDesktopChangeMs'])>=0]),
            'state':stats([float(r['validatedStateMs']) for r in rows]),
            'samplingGap':stats([float(r['maxSampleGapMs']) for r in rows])}
    text=(run/'stdout.log').read_text(encoding='utf-8',errors='replace');phases=collections.defaultdict(lambda:collections.Counter())
    for phase,layouts,lms,paints,pms,visits,area in re.findall(r'UX_LIGHT hwnd\S+ phase=(\S+) layouts=(\d+) layoutMs=([\d.]+) paints=(\d+) paintMs=([\d.]+) nodeVisits=(\d+) paintArea=(\d+)',text):
        for k,v in zip(['layouts','layoutMs','paints','paintMs','nodeVisits','paintArea'],[layouts,lms,paints,pms,visits,area]):phases[phase][k]+=float(v)
    cpu=collections.defaultdict(list)
    for row in csv.DictReader((run/'process-cpu.csv').open()):cpu[row['pid']].append(row)
    cpu_summary=[{'pid':pid,'name':rows[-1]['name'],'cpuMs':(float(rows[-1]['cpuSeconds'])-float(rows[0]['cpuSeconds']))*1000,'peakRssBytes':max(int(r['rssBytes']) for r in rows)} for pid,rows in cpu.items()]
    samples=collections.defaultdict(list)
    for row in csv.DictReader((run/'desktop-samples.csv').open()):samples[(row['category'],row['index'])].append((float(row['qpcMs']),row['pixelHash']))
    for name in ['scrollbar-drag','float-drag-gl','dock']:
        intervals=[]
        for row in groups[name]:
            last_hash=None;last_change=None
            for ms,value in samples[(name,row['index'])]:
                if ms<float(row['inputQpcMs']):last_hash=value;continue
                if value!=last_hash:
                    if last_change is not None:intervals.append(ms-last_change)
                    last_change=ms;last_hash=value
        categories[name]['observedChangedPixelIntervalMs']=stats(intervals)
        categories[name]['gapsOverAssumed60Hz']=sum(v>1000/60 for v in intervals)
        categories[name]['unseen60HzSlotsEstimate']=sum(max(0,math.ceil(v/(1000/60))-1) for v in intervals)
    result[run.name]={'exit':meta['os_exit'],'forced':meta['forced'],'elapsedMs':meta['elapsed_ms'],'categories':categories,'lightPhases':dict(phases),'cpu':cpu_summary,'assertions':re.findall(r'state line[^\n]+',text),'coldSetup':re.findall(r'READY[^\n]+',text),'idle':re.findall(r'UX_IDLE[^\n]+',text)}
(base/'raw/performance-summary-v4.json').write_text(json.dumps(result,ensure_ascii=False,indent=2),encoding='utf-8')
for label,run in result.items():
    print(label,'exit',run['exit'],'assertions',run['assertions'])
    for name in ['host-hover','host-click','menu-open','dialog-open','long-input','instance-switch']:
        print(name,run['categories'][name])

