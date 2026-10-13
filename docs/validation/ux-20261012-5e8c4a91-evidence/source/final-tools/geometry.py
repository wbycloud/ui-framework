import pathlib,re,json,math
from PIL import Image
base=pathlib.Path(__file__).resolve().parents[1];rows=[]
for label in ['before-light-96','final-path-light-96-light-buttons','final-path-light-144-dark-buttons','final-path-webview2-192-dark-buttons']:
    directory=base/'runs'/label;text=(directory/'stdout.log').read_text(encoding='utf-8');origin=(48,40);bounds=(40,40,1240,840)
    for line in text.splitlines():
        coord=re.search(r'viewOrigin(-?\d+),(-?\d+)',line)
        if coord:origin=tuple(map(int,coord.groups()))
        capture=re.match(r'CAPTURE_RECT (-?\d+),(-?\d+),(-?\d+),(-?\d+)',line)
        if capture:bounds=tuple(map(int,capture.groups()))
        match=re.search(r'FRAME n(\d+).*? id(\S+) stagehover rect(\d+),(\d+),(\d+),(\d+).*? dpi(\d+)',line)
        if not match:continue
        n,id,x,y,w,h,dpi=match.groups();x,y,w,h,dpi=map(int,[x,y,w,h,dpi]);file=directory/f'{int(n):03d}-{id}-hover.bmp'
        if not file.exists() or id not in ['open','window-min','window-max','window-close','panel-form','fold-form']:continue
        def device(v):return math.floor(v*dpi/96+.5)
        box=(origin[0]-bounds[0]+device(x),origin[1]-bounds[1]+device(y),origin[0]-bounds[0]+device(x+w),origin[1]-bounds[1]+device(y+h))
        image=Image.open(file).convert('RGB');points=[];dark='dark' in label;pad=max(2,device(2))
        for yy in range(box[1]+pad,box[3]-pad):
            for xx in range(box[0]+pad,box[2]-pad):
                rgb=image.getpixel((xx,yy));foreground=(min(rgb)>75 and max(rgb)-min(rgb)<35) if dark else min(rgb)>190 if id=='window-close' else max(rgb)<170
                if foreground:points.append((xx,yy))
        glyph=(min(p[0] for p in points),min(p[1] for p in points),max(p[0] for p in points)+1,max(p[1] for p in points)+1) if points else None
        rows.append({'run':label,'id':id,'logicalButton':[x,y,w,h],'deviceButton':box,'foregroundBounds':glyph,
            'centerOffsetDevice':[(glyph[0]+glyph[2]-box[0]-box[2])/2,(glyph[1]+glyph[3]-box[1]-box[3])/2] if glyph else None,
            'method':'foreground threshold inside desktop button ROI; anti-alias fringes excluded',
            'originEvidence':'logged COORD/CAPTURE_RECT' if not label.startswith('before') else 'known probe root plus observed 8px left frame; historical probe did not log native origin'})
(base/'raw/button-geometry.json').write_text(json.dumps(rows,ensure_ascii=False,indent=2),encoding='utf-8');print([(r['run'],r['id'],r['centerOffsetDevice']) for r in rows])
