import subprocess,pathlib,json
base=pathlib.Path(__file__).resolve().parents[1];runner=base/'tools/run-probe.py'
cases=[]
for backend in ['light','webview2']:
    for dpi in ['96','144','192']:
        for theme in ['light','dark']:
            if False:continue
            cases.append((backend,dpi,theme,'buttons'))
for backend in ['light','webview2']:
    for dpi,theme in [('96','light'),('192','dark')]:cases.append((backend,dpi,theme,'narrow'))
for backend,dpi,theme,mode in cases:
    label=f'final3-path-{backend}-{dpi}-{theme}-{mode}'
    subprocess.run(['python',str(runner),label,'--backend',backend,'--dpi',dpi,'--theme',theme,'--build','final','--mode',mode],check=True,stdout=subprocess.DEVNULL)
    result=json.loads((base/'runs'/label/'run.json').read_text());print(label,'exit',result['os_exit'],'forced',result['forced'],flush=True)
    if result['forced']:break


