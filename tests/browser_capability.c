/* BRC-1. No real input/HTTP. Private acceptance only in isolated runtime. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ui_framework/application.h"
#include "ui_framework/capability_entry.h"
#include "ui_framework/capability_v2.h"
#include "ui_internal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef UI_BROWSER_ISOLATED
__declspec(dllimport) ui_status_t ui_ai_edit_fixture_manual(ui_host_t *,const char *,void *);
#endif
static unsigned checks,failures,entries;static ui_workspace_t *w;static ui_app_instance_info_t a,b;static char state[80000];
#define CHECK(x) do{++checks;if(!(x)){++failures;fprintf(stderr,"BRC line%d: %s\n",__LINE__,#x);}}while(0)
static void pump(void)
{MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}ui_workspace_poll(w);}
static int read_state(ui_app_instance_info_t *app,const char *expected)
{DWORD begin=GetTickCount();do{ui_cap_target_t t={0};ui_cap_result_t r={0};uint64_t call=0;pump();t.size=sizeof(t);t.version=1;ui_status_t status=ui_capability_bind(app->host,"browser.view",&t);if(!status)status=ui_capability_prepare(app->host,"browser.snapshot",&t,"{}",&call);if(!status)status=ui_capability_dispatch(app->host,call);r.size=sizeof(r);r.version=1;if(!status)status=ui_capability_get_result(app->host,call,&r);if(!status&&r.state==UI_CAP_FINISHED&&r.outcome==UI_CAP_OK&&r.output_json){strcpy_s(state,sizeof(state),r.output_json);ui_capability_forget(app->host,call);if(!expected||strstr(state,expected))return 1;}else if(call)ui_capability_forget(app->host,call);(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}while(GetTickCount()-begin<5000);fprintf(stderr,"last browser %.800s\n",state);return 0;}
static void visit(const ui_assistant_command_desc_t *d,void *data)
{(void)data;++entries;CHECK(!strcmp(d->id,"browser.snapshot")||!strcmp(d->id,"browser.body"));CHECK(d->permission==UI_ASSISTANT_PERMISSION_READ);}
#ifdef UI_BROWSER_ISOLATED
static void menu(unsigned index)
{char id[64];snprintf(id,sizeof(id),"browser.menu.%u",index);CHECK(ui_ai_edit_fixture_manual(a.host,id,NULL)==UI_STATUS_OK);CHECK(read_state(&a,NULL));}
static ui_web_view_t *chrome(void)
{for(ui_web_view_t *v=a.host->web_views;v;v=v->host_next){ui_element_presentation_t p={0};p.size=sizeof(p);if(ui_web_view_get_presentation(v,"find",&p)==UI_STATUS_OK)return v;}return NULL;}
static void find_draft(void)
{ui_web_view_t *v=chrome();ui_element_presentation_t p={0};ui_input_event_t e={0};p.size=sizeof(p);CHECK(v!=NULL);if(!v)return;CHECK(ui_web_view_get_presentation(v,"find",&p)==UI_STATUS_OK);CHECK(p.focused&&p.enabled);e.size=sizeof(e);e.kind=UI_INPUT_TEXT;e.text_utf8="needle";ui_status_t status=ui_web_view_dispatch_input(v,&e);if(status)fprintf(stderr,"find text status=%d focused=%d visible=%d rect=%d,%d\n",status,p.focused,p.visible,p.rect.width,p.rect.height);CHECK(status==UI_STATUS_OK);CHECK(ui_web_view_get_presentation(v,"find",&p)==UI_STATUS_OK);CHECK(strstr(p.text_utf8,"needle"));CHECK(read_state(&a,"\"find_text\":\"\""));menu(9);CHECK(strstr(state,"\"find_text\":\"needle\""));}

#endif
int wmain(int argc,wchar_t **argv)
{
 if(argc!=3)return 2;wchar_t old[32768],settings[4096];DWORD old_len=GetEnvironmentVariableW(L"UI_BROWSER_SETTINGS_PATH",old,32768);CHECK(CreateDirectoryW(argv[2],NULL)||GetLastError()==ERROR_ALREADY_EXISTS);_snwprintf_s(settings,4096,_TRUNCATE,L"%s/origins-%lu-%llu.txt",argv[2],GetCurrentProcessId(),(unsigned long long)GetTickCount64());HANDLE settings_file=CreateFileW(settings,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);CHECK(settings_file!=INVALID_HANDLE_VALUE);if(settings_file==INVALID_HANDLE_VALUE)return 1;CloseHandle(settings_file);CHECK(SetEnvironmentVariableW(L"UI_BROWSER_SETTINGS_PATH",settings));CHECK(ui_framework_initialize()==UI_STATUS_OK);HWND parent=CreateWindowExW(0,L"STATIC",L"BRC isolated",WS_OVERLAPPEDWINDOW,-16000,-16000,1000,800,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(parent!=NULL);ui_workspace_config_t c={0};c.size=sizeof(c);c.run_mode=UI_RUN_WINDOWED;c.native_parent=parent;c.max_permission=UI_ASSISTANT_PERMISSION_DESTRUCTIVE;w=ui_workspace_create(&c);CHECK(w!=NULL);char path[4096];CHECK(WideCharToMultiByte(CP_UTF8,0,argv[1],-1,path,sizeof(path),NULL,NULL)>0);uint64_t first=0,second=0;ui_status_t status=ui_workspace_open(w,path,&first);CHECK(status==UI_STATUS_OK);if(status){fprintf(stderr,"browser load: %s\n",ui_workspace_last_error(w));return 1;}CHECK(ui_workspace_open(w,path,&second)==UI_STATUS_OK);a.size=sizeof(a);b.size=sizeof(b);CHECK(ui_workspace_get_instance(w,first,&a)==UI_STATUS_OK);CHECK(ui_workspace_get_instance(w,second,&b)==UI_STATUS_OK);CHECK(read_state(&a,"browser:start"));CHECK(read_state(&b,"browser:start"));CHECK(ui_assistant_visit_commands(a.assistant,visit,NULL)==UI_STATUS_OK&&entries==2);
 const char *blocked[]={"browser.navigate","browser.reload","browser.stop","browser.back","browser.forward","browser.settings-save","browser.settings-reset","browser.plus"};for(unsigned i=0;i<8;++i){uint64_t request=0;status=ui_assistant_invoke(a.assistant,blocked[i],"{}",&request);CHECK(status==UI_STATUS_NOT_FOUND||status==UI_STATUS_PERMISSION_DENIED);}
 uint64_t gen=0;char *json=NULL;CHECK(ui_capability_discover_v2(a.host,"",0,32,&gen,&json)==UI_STATUS_OK);CHECK(json&&strstr(json,"browser.snapshot")&&strstr(json,"\"effect\":\"view\"")&&!strstr(json,"browser.navigate"));ui_capability_release(json);
 /* Retained legacy local UI behavior only; not new human-source evidence. */
 CHECK(ui_host_invoke(a.host,"browser.navigate","{\"url\":\"browser:help\"}","legacy-diagnostic-simulation")!=0);CHECK(read_state(&a,"browser:help"));ui_cap_target_t t={0};uint64_t call=0;t.size=sizeof(t);t.version=1;CHECK(ui_capability_bind(a.host,"browser.view",&t)==UI_STATUS_OK);CHECK(ui_capability_prepare(a.host,"browser.snapshot",&t,"{}",&call)==UI_STATUS_OK);CHECK(ui_workspace_activate(w,second)==UI_STATUS_OK);status=ui_capability_dispatch(a.host,call);CHECK(status==UI_STATUS_OK||status==UI_STATUS_VALIDATION_FAILED);ui_cap_result_t r={0};r.size=sizeof(r);r.version=1;CHECK(ui_capability_get_result(a.host,call,&r)==UI_STATUS_OK);if(status==UI_STATUS_OK&&r.outcome==UI_CAP_OK)CHECK(r.output_json&&strstr(r.output_json,"browser:help"));CHECK(ui_capability_forget(a.host,call)==UI_STATUS_OK);CHECK(read_state(&b,"browser:start"));
 t.size=sizeof(t);t.version=1;CHECK(ui_capability_bind(a.host,"browser.view",&t)==UI_STATUS_OK);CHECK(ui_capability_prepare(a.host,"browser.snapshot",&t,"{}",&call)==UI_STATUS_OK);CHECK(ui_host_invoke(a.host,"browser.navigate","{\"url\":\"browser:start\"}","legacy-diagnostic-simulation")!=0);ui_workspace_poll(w);status=ui_capability_dispatch(a.host,call);CHECK(status==UI_STATUS_OK);r.size=sizeof(r);r.version=1;CHECK(ui_capability_get_result(a.host,call,&r)==UI_STATUS_OK);CHECK(r.state==UI_CAP_FINISHED&&r.outcome==UI_CAP_STALE&&!strcmp(r.code,"revision_conflict"));CHECK(ui_capability_forget(a.host,call)==UI_STATUS_OK);CHECK(read_state(&a,"browser:start"));
#ifdef UI_BROWSER_ISOLATED
 CHECK(ui_workspace_activate(w,first)==UI_STATUS_OK);menu(11);CHECK(strstr(state,"\"zoom\":110"));menu(12);CHECK(strstr(state,"\"zoom\":100"));menu(13);CHECK(strstr(state,"\"zoom\":100"));menu(14);CHECK(strstr(state,"\"panel\":1"));menu(18);CHECK(strstr(state,"\"panel\":0"));menu(15);CHECK(strstr(state,"\"panel\":2"));menu(18);menu(7);menu(8);find_draft();menu(10);CHECK(strstr(state,"\"find_text\":\"needle\""));
#endif
 CHECK(ui_workspace_close_all(w)==UI_STATUS_OK);DWORD begin=GetTickCount();while(ui_workspace_count(w)&&GetTickCount()-begin<5000){pump();(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}CHECK(ui_workspace_count(w)==0);CHECK(ui_workspace_destroy(w)==UI_STATUS_OK);DestroyWindow(parent);CHECK(GetModuleHandleW(L"browser_app.dll")==NULL);CHECK(SetEnvironmentVariableW(L"UI_BROWSER_SETTINGS_PATH",old_len?old:NULL));printf("BRC checks=%u failures=%u; local builtins/read/isolated UI only, no physical or HTTP\n",checks,failures);return failures?1:0;
}
