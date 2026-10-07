from pathlib import Path
import shutil,subprocess,json,hashlib
root=Path.cwd().resolve();p=root/'build/experience-20261007';sdk=p/'sdk'
assert sdk.is_relative_to(root) and (sdk/'source').is_dir()
paths=subprocess.check_output(['git','ls-files','-z']).decode('utf-8').split('\0')
paths += ['docs/migration-v0.7-to-v0.8.md','docs/validation/component-experience-validation.md']
paths += [q.relative_to(root).as_posix() for q in (root/'docs/validation/component-experience-evidence-20261007').rglob('*') if q.is_file()]
for name in paths:
 if name and (root/name).is_file():
  out=sdk/'source'/name;out.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(root/name,out)
for q in (root/'include/ui_framework').glob('*.h'):shutil.copy2(q,sdk/'include/ui_framework'/q.name)
for name in ['ui_framework.dll','framework_host.exe','uapp_pack.exe','ui_headless_example.exe','api7_fixture.uapp','framework_features.uapp','generic_components.uapp','web_counter.uapp','minimal_eda.uapp']:
 q=p/'after'/name
 if q.is_file():shutil.copy2(q,sdk/'bin'/name)
for name in ['ui_framework_runtime.lib','ui_framework.lib','ui_framework_headless.lib']:
 q=p/'after'/name
 if q.is_file():shutil.copy2(q,sdk/'lib'/name)
for q in (p/'after').rglob('*.lib'):
 if q.name in ['lexbor_static.lib','qjs.lib']:shutil.copy2(q,sdk/'lib'/q.name)
print('SDK refreshed',len(paths),'source/evidence paths')
