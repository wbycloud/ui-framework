#define UI_HOST_TEST 1
#include "../examples/framework_host/main.c"
static int failures;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);++failures;}}while(0)
static void pump(host_window_t *host,DWORD ms)
{ULONGLONG start=GetTickCount64();do{MSG m;unsigned n=0;while(n++<1000&&PeekMessageW(&m,NULL,0,0,PM_REMOVE)){int handled=host_menu_key(host,&m);if(m.message>=WM_KEYFIRST&&m.message<=WM_KEYLAST)fprintf(stderr,"native key msg=%u key=%llu handled=%d ctrl=%d shift=%d alt=%d\n",m.message,(unsigned long long)m.wParam,handled,GetKeyState(VK_CONTROL),GetKeyState(VK_SHIFT),GetKeyState(VK_MENU));if(m.message!=WM_QUIT&&!handled){TranslateMessage(&m);DispatchMessageW(&m);}}ui_workspace_poll(host->workspace);Sleep(1);}while(GetTickCount64()-start<ms);}
static void send_key(host_window_t *host,WORD key,int up)
{INPUT input={0};HWND foreground=GetForegroundWindow();int owned=foreground&&GetAncestor(foreground,GA_ROOTOWNER)==host->hwnd;CHECK(owned);if(!owned)return;input.type=INPUT_KEYBOARD;input.ki.wVk=key;input.ki.dwFlags=up?KEYEVENTF_KEYUP:0;CHECK(SendInput(1,&input,sizeof(input))==1);pump(host,100);}
int wmain(int argc,wchar_t **argv)
{
 host_window_t host={0};ui_app_instance_info_t info={0};ui_web_view_t *view;ui_element_presentation_t p={0};ui_cell_t field={0};uint64_t before;HWND focus;
 if(argc!=3)return 2;CHECK(SUCCEEDED(CoInitializeEx(NULL,COINIT_APARTMENTTHREADED)));CHECK(ui_framework_initialize()==UI_STATUS_OK);SetEnvironmentVariableW(L"UI_API5_OSMESA_DLL",argv[2]);
 host.hwnd=create_host(&host,GetModuleHandleW(NULL));CHECK(host.hwnd!=NULL);if(!host.hwnd)return 1;ShowWindow(host.hwnd,SW_SHOW);open_path(&host,argv[1]);CHECK(get_instance(&host,ui_workspace_active(host.workspace),&info));if(!info.host)return 1;
 pump(&host,2000);view=info.host->web_views;while(view&&(!ui_web_view_native_handle(view)||GetParent((HWND)ui_web_view_native_handle(view))!=(HWND)info.host->native_parent))view=view->host_next;CHECK(view!=NULL);if(!view)return 1;
 SetForegroundWindow(host.hwnd);SetFocus((HWND)ui_web_view_native_handle(view));pump(&host,200);focus=GetFocus();CHECK(focus&&IsChild(host.hwnd,focus));before=info.host->next_request_id;
 send_key(&host,VK_MENU,0);send_key(&host,'F',0);send_key(&host,'F',1);send_key(&host,VK_MENU,1);
 p.size=sizeof(p);CHECK(ui_host_menu_get_item_presentation(info.host,"submit",&p)==UI_STATUS_OK&&p.visible);
 send_key(&host,'S',0);send_key(&host,'S',0);send_key(&host,'S',1);CHECK(!strcmp(ui_host_status_text(info.host),"submitted"));CHECK(info.host->next_request_id==before+1);CHECK(GetFocus()==focus);
 send_key(&host,VK_MENU,0);send_key(&host,VK_MENU,1);CHECK(ui_host_menu_get_presentation(info.host,"close",&p)==UI_STATUS_OK);send_key(&host,VK_ESCAPE,0);send_key(&host,VK_ESCAPE,1);CHECK(GetFocus()==focus);
 before=info.host->next_request_id;send_key(&host,VK_CONTROL,0);send_key(&host,'O',0);send_key(&host,'O',1);send_key(&host,VK_CONTROL,1);field.size=sizeof(field);CHECK(ui_component_get_field(ui_component_find(info.host,"form"),"name",&field)==UI_STATUS_OK&&!strcmp(field.text,"DLL updated"));CHECK(info.host->next_request_id==before+1);
 CHECK(ui_workspace_close_all(host.workspace)==UI_STATUS_OK);pump(&host,2000);DestroyWindow(host.hwnd);SetEnvironmentVariableW(L"UI_API5_OSMESA_DLL",NULL);CoUninitialize();printf("Actual native WebView focus / host Alt / single command / focus restore: %d failures\n",failures);return failures?1:0;
}
