/* Actual standalone host resource / bridge / HWND; no product test transport. */
#define UI_HOST_TEST 1
#include "../examples/framework_host/main.c"
#include "ui_framework/package.h"
static int failures;
static host_window_t *key_host;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);++failures;}}while(0)
static void pump(void){MSG m;unsigned n=0;while(n++<5000&&PeekMessageW(&m,NULL,0,0,PM_REMOVE))if(m.message!=WM_QUIT&&(!key_host||!host_menu_key(key_host,&m))){TranslateMessage(&m);DispatchMessageW(&m);}}
static void settle(DWORD ms){ULONGLONG end=GetTickCount64()+ms;do{pump();Sleep(1);}while(GetTickCount64()<end);}
static int click(ui_web_view_t *v,const char *id){ui_element_presentation_t p={0};ui_input_event_t e={0};p.size=sizeof(p);if(ui_web_view_get_presentation(v,id,&p)!=UI_STATUS_OK||!p.visible||!p.enabled)return 0;e.size=sizeof(e);e.x=p.clip.x+p.clip.width/2;e.y=p.clip.y+p.clip.height/2;e.pointer_button=1;e.kind=UI_INPUT_POINTER_DOWN;if(ui_web_view_dispatch_input(v,&e)!=UI_STATUS_OK)return 0;e.kind=UI_INPUT_POINTER_UP;return ui_web_view_dispatch_input(v,&e)==UI_STATUS_OK;}
static void screenshot(HWND window,const wchar_t *prefix,const wchar_t *suffix){
 RECT r;HDC dc;HBITMAP bitmap;HGDIOBJ old;BITMAPINFO info={0};BITMAPFILEHEADER header={0};void *bits;FILE *file=NULL;wchar_t path[1024];
 GetWindowRect(window,&r);r.right-=r.left;r.bottom-=r.top;dc=CreateCompatibleDC(NULL);
 info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=r.right;info.bmiHeader.biHeight=-r.bottom;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
 bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&bits,NULL,0);CHECK(bitmap!=NULL);if(!bitmap){DeleteDC(dc);return;}
 old=SelectObject(dc,bitmap);CHECK(PrintWindow(window,dc,0));GdiFlush();
 header.bfType=0x4d42;header.bfOffBits=sizeof(header)+sizeof(info.bmiHeader);header.bfSize=header.bfOffBits+(DWORD)r.right*(DWORD)r.bottom*4;
 swprintf(path,1024,L"%s-%s.bmp",prefix,suffix);CHECK(!_wfopen_s(&file,path,L"wb"));if(file){fwrite(&header,sizeof(header),1,file);fwrite(&info.bmiHeader,sizeof(info.bmiHeader),1,file);fwrite(bits,4,(size_t)r.right*r.bottom,file);fclose(file);}
 SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);
}

static void action(host_window_t *h,const char *name,uint64_t id){host_action_t a={0};char number[32];snprintf(number,32,"%llu",(unsigned long long)id);a.action=(char *)name;a.id=number;process_action(h,&a);pump();}
static LPARAM at(host_window_t *h,const char *id,POINT *out){ui_rect_t r={0};POINT p;CHECK(ui_web_view_get_element_rect(h->view,id,&r)==UI_STATUS_OK);p.x=MulDiv(r.x+r.width/2,(int)h->dpi,96);p.y=MulDiv(r.y+r.height/2,(int)h->dpi,96);ClientToScreen(h->hwnd,&p);if(out)*out=p;return MAKELPARAM(p.x,p.y);}
static INPUT pointer_event(POINT p,DWORD flags){INPUT e={0};e.type=INPUT_MOUSE;e.mi.dx=MulDiv(p.x-GetSystemMetrics(SM_XVIRTUALSCREEN),65535,GetSystemMetrics(SM_CXVIRTUALSCREEN)-1);e.mi.dy=MulDiv(p.y-GetSystemMetrics(SM_YVIRTUALSCREEN),65535,GetSystemMetrics(SM_CYVIRTUALSCREEN)-1);e.mi.dwFlags=flags|MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_VIRTUALDESK;return e;}
static void CALLBACK cancel_system_menu(HWND hwnd,UINT message,UINT_PTR timer,DWORD time){GUITHREADINFO info={sizeof(info)};(void)message;(void)time;KillTimer(hwnd,timer);CHECK(GetGUIThreadInfo(GetCurrentThreadId(),&info));CHECK(info.hwndMenuOwner==hwnd&&(info.flags&GUI_INMENUMODE));PostMessageW(hwnd,WM_CANCELMODE,0,0);}
static void test_frame(host_window_t *h){
 POINT cursor,p,end;RECT before,after;INPUT e[4];MONITORINFO mi={sizeof(mi)};
 CHECK(GetCursorPos(&cursor));SetWindowPos(h->hwnd,HWND_TOPMOST,40,40,1200,800,0);SetForegroundWindow(h->hwnd);settle(100);
 at(h,"title-space",&p);CHECK(SendMessageW(h->hwnd,WM_NCHITTEST,0,MAKELPARAM(p.x,p.y))==HTCAPTION);
 CHECK(SendMessageW(h->web_hwnd,WM_NCHITTEST,0,MAKELPARAM(p.x,p.y))==HTTRANSPARENT);
 CHECK(GetAncestor(WindowFromPoint(p),GA_ROOT)==h->hwnd);GetWindowRect(h->hwnd,&before);end=(POINT){p.x+40,p.y+30};
 e[0]=pointer_event(p,MOUSEEVENTF_MOVE);e[1]=pointer_event(p,MOUSEEVENTF_LEFTDOWN);e[2]=pointer_event(end,MOUSEEVENTF_MOVE);e[3]=pointer_event(end,MOUSEEVENTF_LEFTUP);
 CHECK(SendInput(4,e,sizeof(e[0]))==4);settle(300);GetWindowRect(h->hwnd,&after);printf("Actual drag %ld,%ld -> %ld,%ld\n",before.left,before.top,after.left,after.top);CHECK(after.left!=before.left||after.top!=before.top);
 at(h,"title-space",&p);e[0]=pointer_event(p,MOUSEEVENTF_MOVE);e[1]=pointer_event(p,MOUSEEVENTF_LEFTDOWN);e[2]=pointer_event(p,MOUSEEVENTF_LEFTUP);CHECK(SendInput(3,e,sizeof(e[0]))==3);pump();e[0]=pointer_event(p,MOUSEEVENTF_LEFTDOWN);e[1]=pointer_event(p,MOUSEEVENTF_LEFTUP);CHECK(SendInput(2,e,sizeof(e[0]))==2);settle(200);CHECK(IsZoomed(h->hwnd));
 GetMonitorInfoW(MonitorFromWindow(h->hwnd,MONITOR_DEFAULTTONEAREST),&mi);GetClientRect(h->hwnd,&after);p=(POINT){0,0};ClientToScreen(h->hwnd,&p);CHECK(p.x==mi.rcWork.left&&p.y==mi.rcWork.top&&after.right==mi.rcWork.right-mi.rcWork.left&&after.bottom==mi.rcWork.bottom-mi.rcWork.top);
 CHECK(click(h->view,"window-max"));pump();CHECK(!IsZoomed(h->hwnd));
 {const char *ids[]={"recent","open","assistant-toggle","window-min","window-close"};size_t i;for(i=0;i<5;++i)CHECK(SendMessageW(h->hwnd,WM_NCHITTEST,0,at(h,ids[i],NULL))==HTCLIENT);CHECK(SendMessageW(h->hwnd,WM_NCHITTEST,0,at(h,"window-max",NULL))==HTMAXBUTTON);}
 at(h,"window-max",&p);e[0]=pointer_event(p,MOUSEEVENTF_MOVE);e[1]=pointer_event(p,MOUSEEVENTF_LEFTDOWN);e[2]=pointer_event(p,MOUSEEVENTF_LEFTUP);CHECK(GetAncestor(WindowFromPoint(p),GA_ROOT)==h->hwnd);CHECK(SendInput(3,e,sizeof(e[0]))==3);settle(200);CHECK(IsZoomed(h->hwnd));CHECK(click(h->view,"window-max"));pump();
 GetWindowRect(h->hwnd,&before);p=(POINT){before.right-2,before.top+(before.bottom-before.top)/2};end=(POINT){p.x+30,p.y};CHECK(SendMessageW(h->hwnd,WM_NCHITTEST,0,MAKELPARAM(p.x,p.y))==HTRIGHT);
 printf("Resize actual target %p host %p chrome %p capture %p\n",(void *)WindowFromPoint(p),(void *)h->hwnd,(void *)h->web_hwnd,(void *)GetCapture());e[0]=pointer_event(p,MOUSEEVENTF_MOVE);e[1]=pointer_event(p,MOUSEEVENTF_LEFTDOWN);e[2]=pointer_event(end,MOUSEEVENTF_MOVE);e[3]=pointer_event(end,MOUSEEVENTF_LEFTUP);CHECK(SendInput(4,e,sizeof(e[0]))==4);settle(200);GetWindowRect(h->hwnd,&after);printf("Resize actual right %ld -> %ld\n",before.right,after.right);CHECK(after.right-before.right>=20);
 CHECK(GetSystemMenu(h->hwnd,FALSE)!=NULL);CHECK(GetMenuState(GetSystemMenu(h->hwnd,FALSE),SC_CLOSE,MF_BYCOMMAND)!=0xffffffff);
 at(h,"title-space",&p);CHECK(SetTimer(h->hwnd,992,100,cancel_system_menu)!=0);e[0]=pointer_event(p,MOUSEEVENTF_MOVE);e[1]=pointer_event(p,MOUSEEVENTF_RIGHTDOWN);e[2]=pointer_event(p,MOUSEEVENTF_RIGHTUP);CHECK(SendInput(3,e,sizeof(e[0]))==3);settle(200);
 {INPUT keys[4]={0};CHECK(SetTimer(h->hwnd,993,100,cancel_system_menu)!=0);for(int i=0;i<4;++i){keys[i].type=INPUT_KEYBOARD;keys[i].ki.wVk=(i==0||i==3)?VK_MENU:VK_SPACE;keys[i].ki.dwFlags=i>=2?KEYEVENTF_KEYUP:0;}CHECK(SendInput(4,keys,sizeof(keys[0]))==4);settle(200);}
 /* Release outside, Esc/capture loss must not toggle a custom max button. */
 at(h,"window-max",&p);SendMessageW(h->hwnd,WM_NCLBUTTONDOWN,HTMAXBUTTON,MAKELPARAM(p.x,p.y));CHECK(GetCapture()==h->hwnd);SendMessageW(h->hwnd,WM_CANCELMODE,0,0);if(GetCapture()==h->hwnd)ReleaseCapture();CHECK(!h->max_pressed&&!IsZoomed(h->hwnd));
 CHECK(click(h->view,"window-min"));pump();CHECK(IsIconic(h->hwnd));ShowWindow(h->hwnd,SW_RESTORE);pump();CHECK(!IsIconic(h->hwnd));
 SetWindowPos(h->hwnd,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);CHECK(SetCursorPos(cursor.x,cursor.y));
}
static void test_history(host_window_t *h,const wchar_t *directory,const wchar_t *package){
 wchar_t path[1024];size_t count=ui_workspace_count(h->workspace);char newest[HOST_RECENT_PATH];ui_element_presentation_t p={0};
 CHECK(h->recent_count==1);
 {wchar_t full[32768];GetFullPathNameW(package,32768,full,NULL);for(wchar_t *q=full;*q;++q)if(*q>=L'a'&&*q<=L'z')*q-=L'a'-L'A';open_path(h,full);pump();CHECK(h->recent_count==1);action(h,"close",ui_workspace_active(h->workspace));CHECK(ui_workspace_count(h->workspace)==count);}
 CHECK(click(h->view,"recent"));pump();CHECK(h->popup_kind==1);CHECK(click(h->popup_view,"popup-item-0"));pump();CHECK(ui_workspace_count(h->workspace)==count+1&&h->recent_count==1);
 strcpy_s(newest,sizeof(newest),h->recent[0].path);swprintf(path,1024,L"%s\\missing.uapp",directory);open_path(h,path);pump();CHECK(h->recent_count==1&&!strcmp(newest,h->recent[0].path));CHECK(h->popup_kind==4);close_popup(h);
 {FILE *f;swprintf(path,1024,L"%s\\invalid.uapp",directory);CHECK(!_wfopen_s(&f,path,L"wb"));if(f){fputs("invalid",f);fclose(f);}open_path(h,path);pump();CHECK(h->recent_count==1&&ui_workspace_count(h->workspace)==count+1);close_popup(h);CHECK(DeleteFileW(path));}
 for(unsigned i=0;i<13;++i){swprintf(path,1024,L"%s\\app%u.uapp",directory,i);CHECK(CopyFileW(package,path,TRUE));open_path(h,path);pump();CHECK(h->recent_count==(i+2<12?i+2:12));CHECK(ui_workspace_count(h->workspace)==count+2+i);CHECK(DeleteFileW(path));}
 CHECK(click(h->view,"recent"));pump();p.size=sizeof(p);CHECK(ui_web_view_get_presentation(h->popup_view,"popup-item-0",&p)==UI_STATUS_OK&&p.visible);printf("Recent row text=[%s] newest=[%s]\n",p.text_utf8,h->recent[0].path);CHECK(ui_web_view_get_presentation(h->popup_view,"popup-label-0",&p)==UI_STATUS_OK&&!strcmp(p.text_utf8,"app12.uapp"));
 CHECK(click(h->popup_view,"popup-item-0"));pump();CHECK(h->popup_kind==4&&h->recent_count==12);close_popup(h);
 {host_window_t *saved=(host_window_t *)calloc(1,sizeof(*saved));CHECK(saved!=NULL);if(saved){wcscpy_s(saved->recent_file,4096,h->recent_file);recent_load(saved);CHECK(saved->recent_count==12&&!strcmp(saved->recent[0].path,h->recent[0].path));free(saved);}}
 {wchar_t broken[1024];FILE *f;host_window_t *invalid=(host_window_t *)calloc(1,sizeof(*invalid));swprintf(broken,1024,L"%s\\future.bin",directory);CHECK(!_wfopen_s(&f,broken,L"wb"));if(f){fputs("UHR9",f);fclose(f);}if(invalid){wcscpy_s(invalid->recent_file,4096,broken);recent_load(invalid);CHECK(!invalid->recent_count&&invalid->log!=NULL);free(invalid->log);free(invalid);}CHECK(DeleteFileW(broken));}
}
static void test_tabs(host_window_t *h,uint64_t first,uint64_t second){
 ui_rect_t r={0};ui_app_instance_info_t info={0};char id[80];uint32_t dpi;
 for(dpi=96;dpi<=192;dpi+=48){h->dpi=dpi;MoveWindow(h->hwnd,40,40,480,480,TRUE);layout(h);pump();const char *ids[]={"recent","tab-more","open","assistant-toggle","window-min","window-max","window-close"};for(size_t i=0;i<7;++i){ui_element_presentation_t p={0};p.size=sizeof(p);CHECK(ui_web_view_get_presentation(h->view,ids[i],&p)==UI_STATUS_OK&&p.visible&&p.clip.width==p.rect.width&&p.clip.height==p.rect.height);}CHECK(click(h->view,"tab-more"));pump();CHECK(h->popup_kind==1);CHECK(click(h->popup_view,"popup-item-0"));pump();CHECK(ui_workspace_active(h->workspace)==first);}
 h->dpi=GetDpiForWindow(h->hwnd);MoveWindow(h->hwnd,40,40,1200,800,TRUE);layout(h);pump();snprintf(id,80,"tab-%llu",(unsigned long long)second);CHECK(click(h->view,id));pump();CHECK(ui_workspace_active(h->workspace)==second);
 snprintf(id,80,"close-%llu",(unsigned long long)first);CHECK(click(h->view,id));pump();CHECK(!get_instance(h,first,&info)&&ui_workspace_active(h->workspace)==second);
 CHECK(!assistant_visible(h));CHECK(ui_web_view_get_element_rect(h->view,"workspace",&r)==UI_STATUS_OK);int width=r.width;uint64_t target=h->target;CHECK(click(h->view,"assistant-toggle"));pump();CHECK(ui_web_view_get_element_rect(h->view,"workspace",&r)==UI_STATUS_OK&&r.width<width&&h->target==target);CHECK(click(h->view,"assistant-toggle"));pump();
}
static void test_close_contract(host_window_t *h,const wchar_t *directory,const wchar_t *dll){
 wchar_t stage[1024],file[1024],manifest[1024],package[1024],module[1024];char *m,*d,*p,error[256];FILE *f;ui_app_instance_info_t info={0};uint64_t id;
 swprintf(stage,1024,L"%s\\stage",directory);CHECK(CreateDirectoryW(stage,NULL));swprintf(module,1024,L"%s\\fixture.dll",stage);CHECK(CopyFileW(dll,module,TRUE));
 swprintf(file,1024,L"%s\\fixture.txt",stage);CHECK(!_wfopen_s(&f,file,L"wb"));if(f){fwrite("fixture resource",1,16,f);fclose(f);}
 swprintf(manifest,1024,L"%s\\manifest.ini",directory);CHECK(!_wfopen_s(&f,manifest,L"wb"));if(f){fputs("[application]\napp_id=org.ui.browser.fixture\nname=Browser lifecycle\nversion=1.0.0\narchitecture=x64\nabi_version=1\nframework_api_version=8\nmodule=fixture.dll\nmultiple_instances=true\n",f);fclose(f);}
 swprintf(package,1024,L"%s\\fixture.uapp",directory);m=to_utf8(manifest);d=to_utf8(stage);p=to_utf8(package);CHECK(ui_package_pack(m,d,p,error,sizeof(error))==UI_STATUS_OK);free(m);free(d);free(p);
 {size_t before=h->recent_count;SetEnvironmentVariableW(L"UI_FIXTURE_FAIL_MOUNT",L"1");open_path(h,package);pump();CHECK(!ui_workspace_count(h->workspace)&&h->recent_count==before);close_popup(h);SetEnvironmentVariableW(L"UI_FIXTURE_FAIL_MOUNT",NULL);}
 open_path(h,package);pump();id=ui_workspace_active(h->workspace);CHECK(id&&get_instance(h,id,&info));
 CHECK(ui_host_invoke(info.host,"fixture.close_refuse","{}","web.user")!=0);CHECK(click(h->view,"window-close"));pump();CHECK(IsWindow(h->hwnd)&&!h->exiting&&ui_workspace_count(h->workspace)==1);close_popup(h);
 CHECK(ui_host_invoke(info.host,"fixture.close_wait","{}","web.user")!=0);action(h,"close",id);CHECK(IsWindow(h->hwnd)&&get_instance(h,id,&info)&&info.closing);
 {char tab[80];ui_element_presentation_t state={0};snprintf(tab,80,"close-%llu",(unsigned long long)id);state.size=sizeof(state);CHECK(ui_web_view_get_presentation(h->view,tab,&state)==UI_STATUS_OK&&!state.enabled);}
 CHECK(ui_workspace_post_close_complete(h->workspace,id,UI_APP_CLOSE_REFUSE)==UI_STATUS_OK);pump();CHECK(get_instance(h,id,&info)&&!info.closing);
 CHECK(click(h->view,"window-close"));pump();CHECK(IsWindow(h->hwnd)&&h->exiting&&get_instance(h,id,&info)&&info.closing);CHECK(ui_workspace_post_close_complete(h->workspace,id,UI_APP_CLOSE_ALLOW)==UI_STATUS_OK);pump();CHECK(!IsWindow(h->hwnd));
 CHECK(DeleteFileW(package)&&DeleteFileW(manifest)&&DeleteFileW(module)&&DeleteFileW(file)&&RemoveDirectoryW(stage));
}

int wmain(int argc,wchar_t **argv){
 host_window_t h={0};HWND root;uint64_t first,second;ui_app_instance_info_t a={0},b={0};ui_rect_t rect={0};POINT origin={0,0};RECT frame;
 wchar_t temporary[MAX_PATH],history[MAX_PATH];if(argc<4)return 2;CHECK(GetTempPathW(MAX_PATH,temporary)!=0);CHECK(GetTempFileNameW(temporary,L"ubh",0,history)!=0);DeleteFileW(history);CHECK(CreateDirectoryW(history,NULL));swprintf(h.recent_file,4096,L"%s\\recent.bin",history);CHECK(ui_framework_initialize()==UI_STATUS_OK);root=create_host(&h,GetModuleHandleW(NULL));CHECK(root!=NULL);if(!root)return 1;
 key_host=&h;ShowWindow(root,SW_SHOWNOACTIVATE);MoveWindow(root,30,30,1200,800,TRUE);settle(150);
 CHECK(ui_workspace_count(h.workspace)==0);CHECK(GetMenu(root)==NULL);CHECK(!assistant_visible(&h));
 CHECK(click(h.view,"recent"));pump();CHECK(h.popup_hwnd!=NULL);{ui_element_presentation_t p={0};p.size=sizeof(p);CHECK(ui_web_view_get_presentation(h.popup_view,"popup-label-0",&p)==UI_STATUS_OK&&!strcmp(p.text_utf8,"尚无成功打开的应用"));CHECK(ui_web_view_get_presentation(h.popup_view,"popup-item-0",&p)==UI_STATUS_OK&&!p.enabled);}close_popup(&h);
 open_path(&h,argv[1]);pump();first=ui_workspace_active(h.workspace);
 open_path(&h,argv[1]);pump();second=ui_workspace_active(h.workspace);
 CHECK(first&&second&&first!=second);CHECK(get_instance(&h,first,&a)&&get_instance(&h,second,&b));
 if(argc>4){settle(150);screenshot(root,argv[4],L"light");h.dark=1;layout(&h);pump();screenshot(root,argv[4],L"dark");h.dark=0;layout(&h);pump();}
 GetWindowRect(root,&frame);ClientToScreen(root,&origin);CHECK(origin.y-frame.top<=GetSystemMetricsForDpi(SM_CYFRAME,h.dpi)+GetSystemMetricsForDpi(SM_CXPADDEDBORDER,h.dpi));
 CHECK(ui_web_view_get_element_rect(h.view,"window-min",&rect)==UI_STATUS_OK&&rect.width>0);
 CHECK(ui_web_view_get_element_rect(h.view,"window-max",&rect)==UI_STATUS_OK&&rect.width>0);
 CHECK(ui_web_view_get_element_rect(h.view,"window-close",&rect)==UI_STATUS_OK&&rect.width>0);
 CHECK(click(h.view,"assistant-toggle"));pump();CHECK(assistant_visible(&h));CHECK(click(h.view,"assistant-toggle"));pump();CHECK(!assistant_visible(&h));
 test_history(&h,history,argv[1]);
 test_frame(&h);
 test_tabs(&h,first,second);
 open_path(&h,argv[2]);pump();CHECK(get_instance(&h,ui_workspace_active(h.workspace),&a)&&a.host->surfaces!=NULL);
 {HWND surface=(HWND)ui_surface_native_handle(a.host->surfaces);CHECK(click(h.view,"assistant-toggle"));pump();CHECK(IsWindow(surface));CHECK(click(h.view,"assistant-toggle"));pump();CHECK((HWND)ui_surface_native_handle(a.host->surfaces)==surface);}
 if(argc>4){settle(150);screenshot(root,argv[4],L"gl");}
 while(ui_workspace_count(h.workspace)>1)action(&h,"close",ui_workspace_instance_at(h.workspace,ui_workspace_count(h.workspace)-1));
 {uint64_t last=ui_workspace_active(h.workspace);char id[80];snprintf(id,80,"close-%llu",(unsigned long long)last);CHECK(click(h.view,id));pump();CHECK(IsWindow(root)&&!ui_workspace_count(h.workspace));}
 {DWORD before=0,after=0,gdi=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS),user=GetGuiResources(GetCurrentProcess(),GR_USEROBJECTS);CHECK(GetProcessHandleCount(GetCurrentProcess(),&before));for(int cycle=0;cycle<24;++cycle){open_path(&h,argv[1]);pump();action(&h,"close",ui_workspace_active(h.workspace));CHECK(!ui_workspace_count(h.workspace));}CHECK(GetProcessHandleCount(GetCurrentProcess(),&after));CHECK(after<=before+12);CHECK(GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS)<=gdi+4);CHECK(GetGuiResources(GetCurrentProcess(),GR_USEROBJECTS)<=user+4);printf("24 actual DLL reopen/close: handles %lu -> %lu GDI %lu -> %lu USER %lu -> %lu\n",before,after,gdi,GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS),user,GetGuiResources(GetCurrentProcess(),GR_USEROBJECTS));}
 test_close_contract(&h,history,argv[3]);
 CHECK(!IsWindow(root));
 {host_window_t *restored=(host_window_t *)calloc(1,sizeof(*restored));CHECK(restored!=NULL);if(restored){HWND restarted;wcscpy_s(restored->recent_file,4096,h.recent_file);restarted=create_host(restored,GetModuleHandleW(NULL));key_host=restored;pump();CHECK(restarted!=NULL&&restored->recent_count==h.recent_count&&recent_equal(restored->recent[0].path,h.recent[0].path));CHECK(!ui_workspace_count(restored->workspace));CHECK(click(restored->view,"recent"));pump();CHECK(click(restored->popup_view,"popup-item-1"));pump();CHECK(ui_workspace_count(restored->workspace)==1);SendMessageW(restarted,WM_CLOSE,0,0);pump();CHECK(!IsWindow(restarted));free(restored);key_host=NULL;}}
 CHECK(DeleteFileW(h.recent_file));CHECK(RemoveDirectoryW(history));
 
 printf("Browser host: %d failures\n",failures);return failures?1:0;
}
