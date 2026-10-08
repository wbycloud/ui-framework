from pathlib import Path
import re,subprocess,json
r=Path.cwd(); names=subprocess.check_output(['git','diff','--name-only'],text=True).splitlines()+['docs/instance-language.md','docs/migration-v0.8-to-v0.9.md','docs/validation/instance-language-validation.md']
errors=[];count=0
for name in names:
 p=r/name
 if p.suffix!='.md':continue
 for target in re.findall(r'\]\(([^)]+)\)',p.read_text(encoding='utf-8-sig')):
  target=target.strip().split('#',1)[0]
  if not target or '://' in target or target.startswith('mailto:'):continue
  target=target.strip('<>')
  if not (p.parent/target).exists():errors.append((name,target))
  count+=1
print('Current changed Markdown local link checks',count,'missing',len(errors))
for e in errors:print(e)
assert not errors
