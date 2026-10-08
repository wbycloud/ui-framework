from pathlib import Path
import re
for name in ['src/components.html','examples/framework_host/host.html']:
 p=Path(name);s=p.read_text(encoding='utf-8');s=re.sub(r'\bt\(', 'frameworkText(',s);p.write_text(s,encoding='utf-8')
p=Path('tests/stateful_components.c');s=p.read_text(encoding='utf-8').replace('strstr(h.result_summary,"操作完成")','strstr(h.result_summary,ui_language_text(info.host,"操作完成"))');p.write_text(s,encoding='utf-8')
p=Path('examples/framework_host/main.c');s=p.read_text(encoding='utf-8').replace('case HOST_REFRESH_MESSAGE:', 'case WM_SETTINGCHANGE:if(s->workspace)schedule_refresh(s);break;\n    case HOST_REFRESH_MESSAGE:');p.write_text(s,encoding='utf-8')
p=Path('tests/instance_language.c');s=p.read_text(encoding='utf-8').replace('static HWND language_popup;','static HWND language_popup,language_dialog;').replace('GetClassNameW(window,name,80);if(IsWindowVisible(window)', 'GetClassNameW(window,name,80);if(IsWindowVisible(window)&&!wcscmp(name,L"UIFrameworkWebDialog3"))language_dialog=window;if(IsWindowVisible(window)')
s=s.replace('invoke(a.host,"test.modal.close");', 'CHECK(ui_host_invoke(a.host,"test.modal.close","{}","state-test")==0);language_dialog=NULL;EnumThreadWindows(GetCurrentThreadId(),locate_language_popup,0);CHECK(language_dialog!=NULL);if(language_dialog)click_rect(language_dialog,&submit.clip,dpi);')
p.write_text(s,encoding='utf-8')
