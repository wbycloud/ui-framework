/* Isolated launcher projection facade. No Service, credential or transport. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ui_framework/ai_validation_batch.h"
#include "ui_framework/ai_model_ui.h"
#include "yyjson.h"
static int fail_control,fail_init,control_failures,init_calls;static int fault,once,closed,bindings,checks,errors,during;static HWND parent_window;static char state_json[4096];
static void frame(void);
static void *test_alloc(size_t n){if(fault==1)return NULL;return malloc(n);}
static int test_convert(UINT a,DWORD b,LPCCH c,int d,LPWSTR e,int f){if(fault==2||(fault==7&&e))return 0;return MultiByteToWideChar(a,b,c,d,e,f);}
static BOOL test_text(HWND w,LPCWSTR text);
static char *test_write(const yyjson_doc *d,yyjson_write_flag f,size_t *n){return fault==6?NULL:yyjson_write(d,f,n);}
static ui_status_t initialize_fake(void){++init_calls;return fail_init?UI_STATUS_UNSUPPORTED:UI_STATUS_OK;}
static HWND test_create(LPCWSTR c,LPCWSTR t,DWORD st,int x,int y,int w,int h,HWND p,HMENU m,HINSTANCE i,LPVOID v){if((fail_control==1&&!wcscmp(c,L"EDIT"))||(fail_control>=2&&(INT_PTR)m==99+fail_control)){++control_failures;SetLastError(ERROR_NOT_ENOUGH_MEMORY);return NULL;}return CreateWindowExW(0,c,t,st,x,y,w,h,p,m,i,v);}
#undef CreateWindowW
#define CreateWindowW test_create
#define ui_framework_initialize initialize_fake
#define yyjson_write test_write
static ui_status_t poll_fake(ui_ai_service_t *s,uint32_t n){(void)s;(void)n;return UI_STATUS_OK;}
static void workspace_poll_fake(ui_workspace_t *w){(void)w;}
static ui_status_t service_state_fake(ui_ai_service_t *s,ui_ai_service_state_t *v){(void)s;v->phase=closed?UI_AI_CLOSED:UI_AI_READY;return UI_STATUS_OK;}
static ui_status_t batch_state_fake(ui_ai_service_t *s,char **v){(void)s;*v=_strdup(state_json);return UI_STATUS_OK;}
static ui_status_t state_fake(ui_ai_service_t *s,char **v){(void)s;*v=_strdup("{}");return UI_STATUS_OK;}
static ui_status_t bind_fake(ui_ai_service_t *s,const ui_ai_validation_batch_button_t *v,uint64_t *id){(void)s;(void)v;*id=++bindings;return UI_STATUS_OK;}
static ui_status_t unbind_fake(ui_ai_service_t *s,uint64_t id){(void)s;(void)id;return UI_STATUS_OK;}
static ui_status_t stop_fake(ui_ai_service_t *s){(void)s;closed=1;return UI_STATUS_OK;}
static ui_status_t destroy_fake(ui_ai_service_t *s){(void)s;return UI_STATUS_OK;}
static ui_status_t workspace_stop_fake(ui_workspace_t *w){(void)w;return UI_STATUS_OK;}
#define malloc test_alloc
#define MultiByteToWideChar test_convert
#define SetWindowTextW test_text
#define ui_ai_service_poll poll_fake
#define ui_workspace_poll workspace_poll_fake
#define ui_ai_service_get_state service_state_fake
#define ui_ai_validation_batch_get_state batch_state_fake
#define ui_ai_model_get_state state_fake
#define ui_ai_validation_get_state state_fake
#define ui_ai_validation_batch_bind_button bind_fake
#define ui_ai_model_unbind unbind_fake
#define ui_ai_validation_stop stop_fake
#define ui_ai_service_request_close stop_fake
#define ui_ai_service_destroy destroy_fake
#define ui_workspace_close_all workspace_stop_fake
#define ui_workspace_destroy workspace_stop_fake
#define ui_ai_release free
#define wmain launcher_reference_main
#include "../tools/xuanshu_rpc_batch.c"
#undef wmain
#undef malloc
#undef SetWindowTextW
static BOOL test_text(HWND w,LPCWSTR text)
{if(w==display){if(IsWindowEnabled(start_button))during=1;if(fault==3)return FALSE;if(fault==8){SetWindowTextW(w,L"part");return TRUE;}if(fault==4&&!once++){frame();}if(fault==5&&!once++){proc(parent_window,WM_CLOSE,0,0);}}return SetWindowTextW(w,text);}
#define CHECK(x) do{++checks;if(!(x)){++errors;fprintf(stderr,"line %d: %s\n",__LINE__,#x);}}while(0)
static void frame(void){tick(parent_window,"unused-no-credential.json");}
static void state(const char *preview,int mismatch)
{char hash[65];CHECK(ui_cap_json_fingerprint(preview,strlen(preview),hash)==UI_STATUS_OK);if(mismatch)hash[0]=hash[0]=='0'?'1':'0';yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *o=yyjson_mut_obj(d);yyjson_mut_doc_set_root(d,o);yyjson_mut_obj_add_strcpy(d,o,"batch_hash",hash);yyjson_mut_obj_add_strcpy(d,o,"preview",preview);yyjson_mut_obj_add_bool(d,o,"ready",true);char *p=yyjson_mut_write(d,0,NULL);strcpy_s(state_json,sizeof(state_json),p);free(p);yyjson_mut_doc_free(d);}
int main(int argc,char **argv)
{
 if(argc!=2)return 2;
 if(!strncmp(argv[1],"startup-",8)){fail_control=!strcmp(argv[1],"startup-control");if(!strcmp(argv[1],"startup-start"))fail_control=2;if(!strcmp(argv[1],"startup-key"))fail_control=3;if(!strcmp(argv[1],"startup-stop"))fail_control=4;fail_init=!strcmp(argv[1],"startup-framework");wchar_t path[80];swprintf_s(path,80,L"startup-fault-%lu-%llu",GetCurrentProcessId(),(unsigned long long)GetTickCount64());wchar_t *args[]={L"test",path,L"unused.json",L"unused.uapp",L"--startup-check"};int code=launcher_reference_main(5,args);CHECK(code==2);CHECK(service==NULL&&workspace==NULL);CHECK(fail_init?init_calls==1:control_failures==1);printf("checks=%d failures=%d; zero Service/credential/network\n",checks,errors);return errors?1:0;}
 parent_window=CreateWindowW(L"STATIC",L"isolated",WS_OVERLAPPED,0,0,1000,700,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(parent_window!=NULL);start_button=CreateWindowW(L"BUTTON",L"start",WS_CHILD,0,0,80,30,parent_window,NULL,NULL,NULL);key_button=CreateWindowW(L"BUTTON",L"key",WS_CHILD,0,0,80,30,parent_window,NULL,NULL,NULL);display=CreateWindowW(L"EDIT",L"placeholder",WS_CHILD|ES_MULTILINE,0,40,900,600,parent_window,NULL,NULL,NULL);EnableWindow(start_button,FALSE);service=(ui_ai_service_t *)(uintptr_t)1;workspace=(ui_workspace_t *)(uintptr_t)1;opened=1;state("{\"scope\":\"synthetic T2/T3\"}",0);
 if(!strcmp(argv[1],"serialize"))fault=6;if(!strcmp(argv[1],"second-conversion"))fault=7;if(!strcmp(argv[1],"partial"))fault=8;if(!strcmp(argv[1],"allocation"))fault=1;if(!strcmp(argv[1],"conversion"))fault=2;if(!strcmp(argv[1],"display"))fault=3;if(!strcmp(argv[1],"reentry"))fault=4;if(!strcmp(argv[1],"close"))fault=5;
 if(!strcmp(argv[1],"missing-control")){DestroyWindow(display);display=NULL;}
 if(!strcmp(argv[1],"stale")){frame();CHECK(IsWindowEnabled(start_button));state("{\"scope\":\"changed\"}",1);}
 if(!strcmp(argv[1],"recover")){frame();CHECK(IsWindowEnabled(start_button));state("{\"scope\":\"changed\"}",0);fault=3;frame();CHECK(!IsWindowEnabled(start_button));CHECK(shown_hash[0]==0);CHECK(bindings==1);fault=0;frame();CHECK(IsWindowEnabled(start_button));CHECK(bindings==2);service=NULL;workspace=NULL;DestroyWindow(parent_window);free(snapshot);printf("checks=%d failures=%d; no Service/credential/network\n",checks,errors);return errors?1:0;}
 frame();int success=!strcmp(argv[1],"normal")||!strcmp(argv[1],"reentry");CHECK(!!IsWindowEnabled(start_button)==success);if(success){CHECK(bindings==1);SetCapture(start_button);frame();CHECK(GetCapture()==start_button);ReleaseCapture();CHECK(bindings==1);CHECK(!during);}else CHECK(!IsWindowEnabled(start_button));
 /* Facade owns no real Service or worker. A bad frame cannot even bind Start. */
 if(!success&&strcmp(argv[1],"stale"))CHECK(bindings==0);service=NULL;workspace=NULL;DestroyWindow(parent_window);free(snapshot);snapshot=NULL;printf("checks=%d failures=%d; no Service/credential/network\n",checks,errors);return errors?1:0;
}

/* Launcher source is an explicit build dependency. */
