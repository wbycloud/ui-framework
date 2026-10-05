#define UI_HOST_TEST 1
#include "../examples/framework_host/main.c"
static int failures;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);++failures;}}while(0)
static void pump(host_window_t *host,DWORD ms)
{ULONGLONG start=GetTickCount64();do{MSG m;unsigned n=0;while(n++<1000&&PeekMessageW(&m,NULL,0,0,PM_REMOVE)){int handled=host_menu_key(host,&m);if(m.message>=WM_KEYFIRST&&m.message<=WM_KEYLAST)fprintf(stderr,"native key msg=%u key=%llu handled=%d ctrl=%d shift=%d alt=%d\n",m.message,(unsigned long long)m.wParam,handled,GetKeyState(VK_CONTROL),GetKeyState(VK_SHIFT),GetKeyState(VK_MENU));if(m.message!=WM_QUIT&&!handled){TranslateMessage(&m);DispatchMessageW(&m);}}ui_workspace_poll(host->workspace);Sleep(1);}while(GetTickCount64()-start<ms);}
static void activate_test_host(host_window_t *host)
{
 RECT r;POINT previous,target;INPUT events[3]={0};int width=GetSystemMetrics(SM_CXVIRTUALSCREEN),height=GetSystemMetrics(SM_CYVIRTUALSCREEN);
 if(SetForegroundWindow(host->hwnd))return;
 if(!GetWindowRect(host->hwnd,&r)||width<2||height<2)return;GetCursorPos(&previous);target=(POINT){r.left+100,r.top+12};
 SetWindowPos(host->hwnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
 /* Only our exposed title bar is eligible; no input into other applications. */
 if(GetAncestor(WindowFromPoint(target),GA_ROOTOWNER)==host->hwnd){
  events[0].type=INPUT_MOUSE;events[0].mi.dx=MulDiv(target.x-GetSystemMetrics(SM_XVIRTUALSCREEN),65535,width-1);events[0].mi.dy=MulDiv(target.y-GetSystemMetrics(SM_YVIRTUALSCREEN),65535,height-1);events[0].mi.dwFlags=MOUSEEVENTF_MOVE|MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_VIRTUALDESK;
  events[1].type=events[2].type=INPUT_MOUSE;events[1].mi.dwFlags=MOUSEEVENTF_LEFTDOWN;events[2].mi.dwFlags=MOUSEEVENTF_LEFTUP;CHECK(SendInput(3,events,sizeof(events[0]))==3);pump(host,100);
 }SetWindowPos(host->hwnd,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);SetCursorPos(previous.x,previous.y);
}
static void send_key(host_window_t *host,WORD key,int up)
{INPUT input={0};HWND foreground=GetForegroundWindow();int owned=foreground&&GetAncestor(foreground,GA_ROOTOWNER)==host->hwnd;CHECK(owned);if(!owned)return;input.type=INPUT_KEYBOARD;input.ki.wVk=key;input.ki.dwFlags=up?KEYEVENTF_KEYUP:0;CHECK(SendInput(1,&input,sizeof(input))==1);pump(host,100);}
int wmain(int argc,wchar_t **argv)
{
 host_window_t host={0};ui_app_instance_info_t info={0};ui_web_view_t *view;ui_element_presentation_t p={0};ui_cell_t field={0};uint64_t before;HWND focus;
 if(argc!=3)return 2;CHECK(SUCCEEDED(CoInitializeEx(NULL,COINIT_APARTMENTTHREADED)));CHECK(ui_framework_initialize()==UI_STATUS_OK);SetEnvironmentVariableW(L"UI_API5_OSMESA_DLL",argv[2]);
 host.hwnd=create_host(&host,GetModuleHandleW(NULL));CHECK(host.hwnd!=NULL);if(!host.hwnd)return 1;ShowWindow(host.hwnd,SW_SHOW);open_path(&host,argv[1]);CHECK(get_instance(&host,ui_workspace_active(host.workspace),&info));if(!info.host)return 1;
 pump(&host,2000);view=info.host->web_views;while(view&&(!ui_web_view_native_handle(view)||GetParent((HWND)ui_web_view_native_handle(view))!=(HWND)info.host->native_parent))view=view->host_next;CHECK(view!=NULL);if(!view)return 1;
 {wchar_t thread_desktop[128]={0},input_desktop[128]={0};DWORD bytes=0;HDESK desktop=OpenInputDesktop(0,FALSE,DESKTOP_READOBJECTS);
 GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,thread_desktop,sizeof(thread_desktop),&bytes);if(desktop){GetUserObjectInformationW(desktop,UOI_NAME,input_desktop,sizeof(input_desktop),&bytes);CloseDesktop(desktop);}
 fprintf(stderr,"Native input desktop thread=%ls input=%ls available=%d foreground=%p expected=%p\n",thread_desktop,input_desktop,desktop!=NULL,GetForegroundWindow(),host.hwnd);}
 activate_test_host(&host);SetFocus((HWND)ui_web_view_native_handle(view));pump(&host,200);focus=GetFocus();CHECK(focus&&IsChild(host.hwnd,focus));before=info.host->next_request_id;
 send_key(&host,VK_MENU,0);send_key(&host,'F',0);send_key(&host,'F',1);send_key(&host,VK_MENU,1);
 p.size=sizeof(p);CHECK(ui_host_menu_get_item_presentation(info.host,"submit",&p)==UI_STATUS_OK&&p.visible);
 send_key(&host,'S',0);send_key(&host,'S',0);send_key(&host,'S',1);CHECK(!strcmp(ui_host_status_text(info.host),"submitted"));CHECK(info.host->next_request_id==before+1);CHECK(GetFocus()==focus);
 send_key(&host,VK_MENU,0);send_key(&host,VK_MENU,1);CHECK(ui_host_menu_get_presentation(info.host,"close",&p)==UI_STATUS_OK);send_key(&host,VK_ESCAPE,0);send_key(&host,VK_ESCAPE,1);CHECK(GetFocus()==focus);
 before=info.host->next_request_id;send_key(&host,VK_CONTROL,0);send_key(&host,'O',0);send_key(&host,'O',1);send_key(&host,VK_CONTROL,1);field.size=sizeof(field);CHECK(ui_component_get_field(ui_component_find(info.host,"form"),"name",&field)==UI_STATUS_OK&&!strcmp(field.text,"DLL updated"));CHECK(info.host->next_request_id==before+1);
 CHECK(ui_workspace_close_all(host.workspace)==UI_STATUS_OK);pump(&host,2000);DestroyWindow(host.hwnd);SetEnvironmentVariableW(L"UI_API5_OSMESA_DLL",NULL);CoUninitialize();printf("Actual native WebView focus / host Alt / single command / focus restore: %d failures\n",failures);return failures?1:0;
}
