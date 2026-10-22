/* Host integration fixture. Separate public-only document/workspace callers
 * remain the ABI gate; this fixture owns the host's actual chrome observer. */
#define UI_HOST_TEST 1
#include "main.c"
static int failures;
#define T(x) do{if(!(x)){fprintf(stderr,"host entry line%d: %s\n",__LINE__,#x);++failures;}}while(0)
static void pump_for(DWORD ms){ULONGLONG end=GetTickCount64()+ms;do{MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){if(m.message!=WM_QUIT){TranslateMessage(&m);DispatchMessageW(&m);}}Sleep(1);}while(GetTickCount64()<end);}
static void click_native(ui_web_view_t *view,const char *id,uint32_t dpi)
{ui_element_presentation_t p={0};POINT point;INPUT in[2]={{0}};p.size=sizeof(p);T(ui_web_view_get_presentation(view,id,&p)==UI_STATUS_OK&&p.visible&&p.enabled);point=(POINT){MulDiv(p.clip.x+p.clip.width/2,(int)dpi,96),MulDiv(p.clip.y+p.clip.height/2,(int)dpi,96)};ClientToScreen((HWND)ui_web_view_native_handle(view),&point);T(SetCursorPos(point.x,point.y));pump_for(20);T(WindowFromPoint(point)==(HWND)ui_web_view_native_handle(view));in[0].type=in[1].type=INPUT_MOUSE;in[0].mi.dwFlags=MOUSEEVENTF_LEFTDOWN;in[1].mi.dwFlags=MOUSEEVENTF_LEFTUP;T(SendInput(2,in,sizeof(INPUT))==2);pump_for(100);}
int main(void)
{
    host_window_t h={0};HWND root,previous=GetForegroundWindow();POINT cursor;wchar_t directory[32768];DWORD length;GetCursorPos(&cursor);setvbuf(stdout,NULL,_IONBF,0);
    length=GetTempPathW(32768,directory);T(length&&length<32000);swprintf(h.recent_file,4096,L"%sBrowserHostEntry-%lu.bin",directory,GetCurrentProcessId());
    T(ui_framework_initialize()==UI_STATUS_OK);root=create_host(&h,GetModuleHandleW(NULL));T(root!=NULL);if(!root)return 1;ShowWindow(root,SW_SHOWNORMAL);MoveWindow(root,40,40,1100,700,TRUE);SetForegroundWindow(root);pump_for(100);T(GetForegroundWindow()==root);T(ui_workspace_count(h.workspace)==0);T(GetModuleHandleW(L"browser_app.dll")==NULL);
    click_native(h.view,"menu",h.dpi);T(h.popup_view!=NULL);if(h.popup_view){ui_element_presentation_t p={0};p.size=sizeof(p);T(ui_web_view_get_presentation(h.popup_view,"popup-label-0",&p)==UI_STATUS_OK);T(!strcmp(p.text_utf8,"浏览网页"));click_native(h.popup_view,"popup-item-0",h.dpi);}
    T(ui_workspace_count(h.workspace)==1);T(GetModuleHandleW(L"browser_app.dll")!=NULL);
    click_native(h.view,"menu",h.dpi);if(h.popup_view)click_native(h.popup_view,"popup-item-0",h.dpi);T(ui_workspace_count(h.workspace)==2);
    T(ui_workspace_close_all(h.workspace)==UI_STATUS_OK);{ULONGLONG end=GetTickCount64()+10000;while(ui_workspace_count(h.workspace)&&GetTickCount64()<end)pump_for(10);}T(ui_workspace_count(h.workspace)==0);T(GetModuleHandleW(L"browser_app.dll")==NULL);
    SendMessageW(root,WM_CLOSE,0,0);pump_for(30);T(!IsWindow(root));if(GetFileAttributesW(h.recent_file)!=INVALID_FILE_ATTRIBUTES)T(DeleteFileW(h.recent_file));if(previous&&IsWindow(previous))SetForegroundWindow(previous);SetCursorPos(cursor.x,cursor.y);
    printf("built-in menu actual clicks, two instances, actual unload: failures=%d; desktop restored\n",failures);return failures?1:0;
}
