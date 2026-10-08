from pathlib import Path
p=Path('src/platform/win32_shell.c');s=p.read_text(encoding='utf-8').replace('json.text','json.data');p.write_text(s,encoding='utf-8')
p=Path('examples/framework_host/main.c');s=p.read_text(encoding='utf-8').replace('const char *notice=tr(s,"\\n…（显示内容已省略）\\n");','const char *notice="\\n… (display truncated)\\n";');p.write_text(s,encoding='utf-8')
# All maintained packages are rebuilt for the current SDK; historic archives unchanged.
for folder in ['examples','tests']:
 for p in Path(folder).rglob('*.ini'):
  s=p.read_text(encoding='utf-8');n=s.replace('framework_api_version=8','framework_api_version=9')
  if n!=s:p.write_text(n,encoding='utf-8')
p=Path('CMakeLists.txt');s=p.read_text(encoding='utf-8').replace('framework_api_version=8','framework_api_version=9');p.write_text(s,encoding='utf-8')
p=Path('build/language-20261008/build-after.cmd');s=Path('build/language-20261008/build-baseline.cmd').read_text().replace('/baseline','/after').replace('baseline-','after-').replace('ui_stateful_components_test --parallel','ui_stateful_components_test ui_language_core_test --parallel');p.write_text(s)
