from pathlib import Path
import importlib.util,json,tempfile
spec=importlib.util.spec_from_file_location('sync',Path(__file__).resolve().parents[1]/'tools'/'export-framework-source.py');sync=importlib.util.module_from_spec(spec);spec.loader.exec_module(sync)
temporary=Path(__file__).resolve().parents[1]/'.work'/'m01';temporary.mkdir(parents=True,exist_ok=True)
with tempfile.TemporaryDirectory(prefix='browser-sync-',dir=temporary) as folder:
    root=Path(folder);a=root/'source';b=root/'mirror';a.mkdir();b.mkdir()
    for name in ('CMakeLists.txt','README.md'):(a/name).write_text('baseline\n');(b/name).write_text('baseline\n')
    state=root/'state.json';state.write_text(json.dumps({name:sync.sha(p) for name,p in sync.files(b).items()}))
    (a/'README.md').write_text('canonical change\n');assert sync.run(a,b,state)==['README.md']
    (b/'README.md').write_text('unique target edit\n')
    try:sync.run(a,b,state,True);raise AssertionError('Drift overwritten')
    except ValueError:pass
    assert (b/'README.md').read_text()=='unique target edit\n'
    (b/'README.md').write_text('baseline\n');sync.run(a,b,state,True);assert not sync.run(a,b,state)
print('canonical/mirror drift refusal and reviewed sync: PASS')
