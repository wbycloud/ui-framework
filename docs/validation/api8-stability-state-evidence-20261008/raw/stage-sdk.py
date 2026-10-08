from pathlib import Path
import hashlib,json,shutil,subprocess
root=Path.cwd().resolve();work=root/'build/stability-state-20261008';sdk=work/'sdk';after=work/'after'
sdk.mkdir(exist_ok=True)
for name in ('source','include/ui_framework','bin','lib','licenses'):(sdk/name).mkdir(parents=True,exist_ok=True)
paths=subprocess.check_output(['git','ls-files','-z'],cwd=root).decode('utf-8').split('\0')
paths += [p.relative_to(root).as_posix() for p in (root/'docs/validation/api8-stability-state-evidence-20261008').rglob('*') if p.is_file()]
paths += ['docs/validation/api8-stability-state-validation.md']
for name in set(paths):
 if name and (root/name).is_file():
  dest=sdk/'source'/name;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(root/name,dest)
for name in ('lexbor','quickjs'):
 dependency=root/'.deps'/name
 for filename in subprocess.check_output(['git','ls-files','-z'],cwd=dependency).decode('utf-8').split('\0'):
  if filename and (dependency/filename).is_file():
   dest=sdk/'source/.deps'/name/filename;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(dependency/filename,dest)
web=root/'.deps/Microsoft.Web.WebView2.1.0.4129.50'
shutil.copytree(web,sdk/'source/.deps'/web.name,dirs_exist_ok=True)
for source in (root/'include/ui_framework').glob('*.h'):shutil.copy2(source,sdk/'include/ui_framework'/source.name)
for name in ('ui_framework.dll','framework_host.exe','uapp_pack.exe','ui_headless_example.exe','stateful_components.uapp','api7_fixture.uapp','framework_features.uapp','generic_components.uapp','web_counter.uapp','minimal_eda.uapp','ui_stateful_components_test.exe','ui_host_repaint_test.exe'):
 shutil.copy2(after/name,sdk/'bin'/name)
for name in ('ui_framework_runtime.lib','ui_framework.lib','ui_framework_headless.lib'):shutil.copy2(after/name,sdk/'lib'/name)
for p in after.rglob('*.lib'):
 if p.name in ('lexbor_static.lib','qjs.lib'):shutil.copy2(p,sdk/'lib'/p.name)
for name in ('osmesa.dll','libglapi.dll'):shutil.copy2(root/'.deps/mesa-24.3.4/x64'/name,sdk/'bin'/name)
shutil.copy2(web/'build/native/x64/WebView2Loader.dll',sdk/'bin/WebView2Loader.dll')
for p in root.glob('LICENSE*'):shutil.copy2(p,sdk/'licenses'/p.name)
for name in ('lexbor','quickjs'):
 for pattern in ('LICENSE*','COPYING*'):
  for p in (root/'.deps'/name).glob(pattern):shutil.copy2(p,sdk/'licenses'/(name+'-'+p.name))
for p in web.glob('*LICENSE*'):shutil.copy2(p,sdk/'licenses'/('WebView2-'+p.name))
for p in (root/'.deps/mesa-24.3.4').rglob('*'):
 if p.is_file() and p.name.lower() in ('copyright','license','license.txt','copying'):shutil.copy2(p,sdk/'licenses'/('Mesa-'+p.name))
assert not (sdk/'bin/opengl32.dll').exists()
print('Fresh matched SDK staged',sdk)
