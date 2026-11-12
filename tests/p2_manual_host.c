/* Public-SDK manual acceptance driver. Deliberately no private fixture header,
 * authority hook or simulated input. This executable is NOT run by CTest. */
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <objbase.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui_framework/application.h"
#include "ui_framework/capability.h"
#include "ui_framework/capability_v2.h"
static ui_workspace_t *workspace;
static int closing;
static uint64_t last_instance,last_call;
static void source(const char *where)
{INPUT_MESSAGE_SOURCE input={0};BOOL ok=GetCurrentInputMessageSource(&input);printf("SOURCE %s ok=%d origin=%d device=%d (classification, not physical proof)\n",where,(int)ok,(int)input.originId,(int)input.deviceType);}
static void result(uint64_t instance,const ui_result_t *r,void *data)
{(void)data;printf("RESULT instance=%llu request=%llu success=%d %s %s\n",(unsigned long long)instance,(unsigned long long)r->request_id,r->success,r->command_id,r->result_json);}
static void event(uint64_t instance,const char *name,const char *json,void *data)
{(void)data;if(!strcmp(name,"p2.capability")){const char *call=strstr(json,"\"call\":\"");if(call&&strstr(json,"\"stage\":1,")){last_instance=instance;last_call=_strtoui64(call+8,NULL,10);}source(name);printf("UI instance=%llu %s\n",(unsigned long long)instance,json);}}
static int active(ui_app_instance_info_t *info)
{memset(info,0,sizeof(*info));info->size=sizeof(*info);return workspace&&ui_workspace_get_instance(workspace,ui_workspace_active(workspace),info)==UI_STATUS_OK;}
static void open_package(const wchar_t *wide)
{char *path;uint64_t id=0;ui_status_t status;int bytes=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,wide,-1,NULL,0,NULL,NULL);if(!bytes)return;path=malloc((size_t)bytes);if(!path)return;WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,wide,-1,path,bytes,NULL,NULL);status=ui_workspace_open(workspace,path,&id);printf("OPEN instance=%llu status=%d %s\n",(unsigned long long)id,(int)status,path);free(path);}
static void open_dialog(HWND window)
{OPENFILENAMEW dialog={0};wchar_t path[32768]={0};dialog.lStructSize=sizeof(dialog);dialog.hwndOwner=window;dialog.lpstrFilter=L"Application package\0*.uapp\0\0";dialog.lpstrFile=path;dialog.nMaxFile=32768;dialog.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;if(GetOpenFileNameW(&dialog))open_package(path);}
static void read_business(ui_host_t *host,const char *resource,const char *id,const char *params)
{
 ui_cap_target_t target={0};ui_cap_result_t r={0};uint64_t call=0;ui_status_t s;target.size=sizeof(target);target.version=1;s=ui_capability_bind(host,resource,&target);
 if(!s)s=ui_capability_prepare(host,id,&target,params,&call);if(!s)s=ui_capability_dispatch(host,call);r.size=sizeof(r);r.version=1;if(!s)s=ui_capability_get_result(host,call,&r);
 printf("READ %s status=%d outcome=%d code=%s %s\n",id,(int)s,(int)r.outcome,r.code,r.output_json?r.output_json:"");if(call)(void)ui_capability_forget(host,call);
}
static void read_active(void)
{
 ui_app_instance_info_t i;source("F6 read");if(!active(&i))return;printf("ACTIVE %llu %s\n",(unsigned long long)i.instance_id,i.app_id);
 if(strstr(i.app_id,"stateful")){read_business(i.host,"state.document","state.document.read","{}");read_business(i.host,"state.document","state.cell.read","{\"id\":\"9007199254741000\",\"column\":\"name\"}");read_business(i.host,"state.view","state.view.read","{}");read_business(i.host,"state.preferences","state.preferences.read","{}");}
 else if(strstr(i.app_id,"minimal-eda")){read_business(i.host,"eda.document","eda.document.read","{}");read_business(i.host,"eda.view","eda.view.read","{}");}
 else if(strstr(i.app_id,"web-counter"))read_business(i.host,"counter.document","counter.read","{}");
 else{read_business(i.host,"demo.document","demo.document.read","{}");read_business(i.host,"demo.view","demo.view.read","{}");}
}
static void next_instance(void)
{size_t count=ui_workspace_count(workspace),at;if(!count)return;for(at=0;at<count&&ui_workspace_instance_at(workspace,at)!=ui_workspace_active(workspace);++at){}(void)ui_workspace_activate(workspace,ui_workspace_instance_at(workspace,(at+1)%count));printf("SWITCH active=%llu\n",(unsigned long long)ui_workspace_active(workspace));}
static void layout(HWND window)
{RECT r;UINT dpi=GetDpiForWindow(window);if(!dpi)dpi=96;GetClientRect(window,&r);if(workspace)(void)ui_workspace_set_rect(workspace,&(ui_rect_t){0,0,MulDiv(r.right,96,(int)dpi),MulDiv(r.bottom,96,(int)dpi)},dpi);}
static LRESULT CALLBACK window_proc(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
 ui_app_instance_info_t info;
 if(message==WM_TIMER){if(workspace){ui_workspace_poll(workspace);if(closing&&!ui_workspace_count(workspace)){KillTimer(window,1);if(ui_workspace_destroy(workspace)==UI_STATUS_OK){workspace=NULL;DestroyWindow(window);}}}return 0;}
 if(message==WM_SIZE&&workspace){layout(window);return 0;}
 if(message==WM_DPICHANGED){RECT *r=(RECT *)lp;SetWindowPos(window,NULL,r->left,r->top,r->right-r->left,r->bottom-r->top,SWP_NOZORDER|SWP_NOACTIVATE);layout(window);return 0;}
 if(message==WM_CLOSE){closing=1;if(workspace)(void)ui_workspace_close_all(workspace);else DestroyWindow(window);return 0;}
 if(message==WM_DESTROY){PostQuitMessage(0);return 0;}
 if(workspace&&active(&info)){intptr_t result_value=0;if(ui_native_shell_handle_message(info.shell,window,message,(uintptr_t)wp,(intptr_t)lp,&result_value)==UI_STATUS_OK)return (LRESULT)result_value;}
 return DefWindowProcW(window,message,wp,lp);
}
int wmain(int argc,wchar_t **argv)
{
 WNDCLASSW cls={0};HWND window;ui_workspace_config_t config={0};MSG message;
 if(argc<2){fprintf(stderr,"usage: ui_p2_manual_host.exe package.uapp [...]\nSet UI_API7_OSMESA_DLL and isolated UI_STATE_EXAMPLE_HOME for stateful.\n");return 2;}
 SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);setvbuf(stdout,NULL,_IONBF,0);(void)CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);cls.lpfnWndProc=window_proc;cls.hInstance=GetModuleHandleW(NULL);cls.lpszClassName=L"UiFramework.P2.Manual";cls.hCursor=LoadCursorW(NULL,MAKEINTRESOURCEW(32512));if(!RegisterClassW(&cls))return 2;
 window=CreateWindowExW(0,cls.lpszClassName,L"P2 manual: F6 read | Ctrl+Tab | Ctrl+F4 close | F9 open | F7/F8 negative | F11/F12 original call",WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,80,60,1280,880,NULL,NULL,cls.hInstance,NULL);if(!window)return 2;
 config.size=sizeof(config);config.native_parent=window;config.result=result;config.event=event;workspace=ui_workspace_create(&config);if(!workspace){DestroyWindow(window);return 2;}
 for(int at=1;at<argc;++at)open_package(argv[at]);
 layout(window);SetTimer(window,1,20,NULL);ShowWindow(window,SW_SHOW);UpdateWindow(window);
 while(GetMessageW(&message,NULL,0,0)>0){
   if(message.message==WM_KEYDOWN&&message.wParam==VK_F6){read_active();continue;}
   if(message.message==WM_KEYDOWN&&message.wParam==VK_TAB&&(GetKeyState(VK_CONTROL)&0x8000)){next_instance();continue;}
   if(message.message==WM_KEYDOWN&&message.wParam==VK_F4&&(GetKeyState(VK_CONTROL)&0x8000)){uint64_t id=ui_workspace_active(workspace);printf("CLOSE instance=%llu status=%d\n",(unsigned long long)id,(int)ui_workspace_close(workspace,id,UI_APP_CLOSE_TAB));continue;}
   if(message.message==WM_KEYDOWN&&message.wParam==VK_F9){open_dialog(window);continue;}
   if(message.message==WM_KEYDOWN&&(message.wParam==VK_F11||message.wParam==VK_F12)){ui_app_instance_info_t info={0};info.size=sizeof(info);if(ui_workspace_get_instance(workspace,last_instance,&info)==UI_STATUS_OK&&last_call){ui_status_t s=message.wParam==VK_F11?ui_capability_cancel_v2(info.host,last_call):ui_capability_forget_v2(info.host,last_call);printf("CONTROL original_instance=%llu call=%llu %s status=%d\n",(unsigned long long)last_instance,(unsigned long long)last_call,message.wParam==VK_F11?"cancel":"forget",(int)s);}else puts("CONTROL no original live instance/call; not redirected");continue;}
   if(message.message==WM_KEYDOWN&&(message.wParam==VK_F7||message.wParam==VK_F8)){ui_app_instance_info_t info;source("programmatic negative");if(active(&info)){
     if(message.wParam==VK_F7){const char *command=strstr(info.app_id,"stateful")?"test.render":strstr(info.app_id,"minimal-eda")?"eda.add_block":strstr(info.app_id,"web-counter")?"counter.increment":"demo.refresh";(void)ui_host_invoke(info.host,command,"{}","human");}
     else{HMENU menu=(HMENU)ui_native_shell_menu_handle(info.shell),child=menu?GetSubMenu(menu,0):NULL;UINT id=child?GetMenuItemID(child,0):0;if(id&&id!=UINT_MAX)SendMessageW(window,WM_COMMAND,id,0);}}continue;}
   TranslateMessage(&message);DispatchMessageW(&message);
 }CoUninitialize();return 0;
}
