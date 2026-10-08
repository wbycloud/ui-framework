from pathlib import Path
import ast,json,re,sys
sys.stdout.reconfigure(encoding='utf-8')
root=Path.cwd(); found={}
paths=list((root/'src').rglob('*.c'))+list((root/'src').rglob('*.inc'))+list((root/'src').glob('*.html'))+[root/'examples/framework_host/main.c',root/'examples/framework_host/host.html']
for p in paths:
 text=p.read_text(encoding='utf-8-sig')
 pattern=r'''"(?:[^"\\\r\n]|\\.)*"|'(?:[^'\\\r\n]|\\.)*' ''' if p.suffix=='.html' else r'"(?:[^"\\\r\n]|\\.)*"'
 for match in re.finditer(pattern,text,re.X):
  literal=match.group()
  if not re.search('[\u4e00-\u9fff]',literal):continue
  try:value=ast.literal_eval(literal)
  except (SyntaxError,ValueError):continue
  found.setdefault(value,[]).append(str(p.relative_to(root)))
 if p.suffix=='.html':
  for value in re.findall(r'>([^<>]*[\u4e00-\u9fff][^<>]*)<',text.split('<script>')[0]):found.setdefault(value.strip(),[]).append(str(p.relative_to(root)))
items=[{'zh':key,'paths':sorted(set(value))} for key,value in sorted(found.items())]
(root/'build/language-20261008/text-inventory.json').write_text(json.dumps(items,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print('Chinese framework literals',len(items))
for i,item in enumerate(items):print(i,repr(item['zh']))
