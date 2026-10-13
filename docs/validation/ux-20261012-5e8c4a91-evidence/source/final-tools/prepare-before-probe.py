import pathlib,subprocess
root=pathlib.Path(__file__).resolve().parents[3];base=root/'build/ux-20261012-5e8c4a91';tools=base/'tools'
main=subprocess.check_output(['git','show','7848a946fcf417eefe87bd63e42f83c8a3f875f3:examples/framework_host/main.c'],cwd=root).decode('utf-8')
main=main.replace('../../src/ui_internal.h',(root/'src/ui_internal.h').as_posix())
main=main.replace('"browser_shell.h"','"'+(root/'examples/framework_host/browser_shell.h').as_posix()+'"')
(tools/'ux-host-before.c').write_text(main,encoding='utf-8')
test=(root/'tests/stateful_components.c').read_text(encoding='utf-8').replace('../examples/framework_host/main.c','ux-host-before.c')
(tools/'ux-stateful-before.c').write_text(test,encoding='utf-8')
cmd=(tools/'build-probe-pre-perf.cmd').read_text(encoding='utf-8').replace('/DUI_FRAMEWORK_BUILD_SHARED','/DUI_FRAMEWORK_BUILD_SHARED /DUX_PRE_PERF')
(tools/'build-probe-pre-perf3.cmd').write_text(cmd.replace('probe-pre-perf-build0','probe-pre-perf-build3'),encoding='utf-8')
