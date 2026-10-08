from pathlib import Path
import re
def edit(path, fn):
 p=Path(path); s=p.read_text(encoding='utf-8'); p.write_text(fn(s),encoding='utf-8')
def localized(s):
 a,b=s.split('<script>',1); b,c=b.split('</script>',1)
 b=re.sub(r"'([^'\\\r\n]|\\.)*'|\"([^\"\\\r\n]|\\.)*\"",lambda m:'t('+m[0]+')' if re.search('[\u4e00-\u9fff]',m[0]) else m[0],b)
 return a+'<script>'+b+'</script>'+c
def components(s):
 s=localized(s).replace('function send(x){ui.postMessage(x)}',"function t(key){return state&&state.texts&&state.texts[key]||key}\nfunction send(x){if(state)x.languageGeneration=state.languageGeneration;ui.postMessage(x)}")
 s=s.replace("ui.onmessage=function(s){", "ui.onmessage=function(s){if(state&&BigInt(s.languageGeneration)<BigInt(state.languageGeneration))return;var languageChanged=state&&s.languageGeneration!==state.languageGeneration;if(languageChanged){closeText(true);closeColumnTools();cancelColorDrag(false);closeChoices()}" )
 # Close transient text views, preserving data/edit and capture generations.
 s=s.replace("state=s;detailSize();", "state=s;var submitNode=document.getElementById('submit'),cancelNode=document.getElementById('cancel');if(submitNode)setText(submitNode,t('提交'));if(cancelNode)setText(cancelNode,t('取消'));detailSize();")
 s=s.replace('var value=values[index],b=button(value,',"var value=values[index],label=n.field.kind===4?(n.field.optionLabels[index]||value):value,b=button(label,")
 s=s.replace("n.input.value:n.input.value", "n.input.value:n.input.value")
 s=s.replace("f.value.text==='true'?t('☑ 是'):t('☐ 否'):f.value.text", "f.value.text==='true'?t('☑ 是'):t('☐ 否'):f.kind===4?(f.optionLabels[f.options.indexOf(f.value.text)]||f.value.text):f.value.text")
 s=s.replace('return f.options[Number(p.option)]', 'return f.optionLabels[Number(p.option)]')
 s=s.replace('v.error,row.imageState]', 'v.error,row.imageState,state.languageGeneration]')
 return s
def menu(s):
 s=s.replace('ui.onmessage=function(s){model=s;',"ui.onmessage=function(s){if(model&&BigInt(s.languageGeneration)<BigInt(model.languageGeneration))return;model=s;")
 return s
def host(s):
 s=localized(s)
 start=s.index('<script>')+len('<script>')
 # Only capture original static chrome leaves, before app-owned DOM exists.
 s=s[:start]+"\nvar languageTexts={},staticLanguage=[];\nfunction t(key){return languageTexts[key]||key}\nfunction rememberText(n){var key=n.getAttribute('title');if(key)staticLanguage.push([n,key,1]);if(!n.children.length&&n.textContent)staticLanguage.push([n,n.textContent,0]);for(var i=0;i<n.children.length;i++)rememberText(n.children[i])}\nrememberText(document.body);\nfunction applyLanguage(s){languageTexts=s.texts||{};for(var i=0;i<staticLanguage.length;i++){var a=staticLanguage[i],v=t(a[1]);if(a[2]){if(a[0].getAttribute('title')!==v)a[0].setAttribute('title',v)}else if(a[0].textContent!==v)a[0].textContent=v}}\n"+s[start:]
 s=s.replace('ui.onmessage=function(s){', 'ui.onmessage=function(s){if(s.texts)applyLanguage(s);')
 # Actual handler has an alternate whitespace spelling; fail if not connected.
 if 'if(s.texts)applyLanguage(s)' not in s:
  s=s.replace('ui.onmessage=function(state) {','ui.onmessage=function(state) {if(state.texts)applyLanguage(state);')
 assert 'applyLanguage(state)' in s or 'if(s.texts)applyLanguage(s)' in s
 return s
edit('examples/framework_host/host.html',host)
edit('src/menus.c',lambda s:s.replace('ui_status_t ui_menus_set_title', 'const char *ui_menus_group_title(ui_host_t *host,const char *path)\n{menu_group_t *g=find_group(host,path);const char *tail=strrchr(path,\'/\');return g?g->title:tail?tail+1:path;}\nui_status_t ui_menus_set_title').replace('free(g->title);g->title=copy;return', 'free(g->title);g->title=copy;if(!host->language_changing)ui_shell_refresh_titles(host);return'))
edit('src/platform/headless_shell.c',lambda s:s+'\nvoid ui_shell_refresh_titles(ui_host_t *host){(void)host;}\n')
edit('CMakeLists.txt',lambda s:s.replace('    enable_testing()','    enable_testing()\n    add_executable(ui_language_core_test tests/language_core.c)\n    target_link_libraries(ui_language_core_test PRIVATE ui_framework)\n    if(MSVC)\n        target_compile_options(ui_language_core_test PRIVATE /W4 /WX /utf-8)\n    endif()\n    add_test(NAME ui_language_core COMMAND ui_language_core_test)'))
