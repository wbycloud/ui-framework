#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "capability_v2_dll_control.h"
#include <stdio.h>
#include <stdlib.h>
typedef struct fixture {
    ui_host_t *host;c2_dll_api_t api;HANDLE worker,callback,worker_gate,callback_gate;
    uint64_t call;int sync,value,revision,stack_checks,completed;
} fixture_t;
static DWORD WINAPI worker(void *data)
{fixture_t *f=data;WaitForSingleObject(f->worker_gate,INFINITE);return 0;}
static DWORD WINAPI external_callback(void *data)
{fixture_t *f=data;WaitForSingleObject(f->callback_gate,INFINITE);return 0;}
static ui_status_t revision(char *out,size_t capacity,void *data)
{fixture_t *f=data;snprintf(out,capacity,"r%d",f->revision);return UI_STATUS_OK;}
static ui_status_t complete(fixture_t *f)
{
    ui_cap_completion_v2_t out={0};f->value=7;++f->revision;
    out.size=sizeof(out);out.version=2;out.outcome=UI_CAP2_OK;out.effect=UI_CAP2_EFFECT_APPLIED;out.output_json="{\"value\":7}";
    f->completed=1;return f->api.complete(f->host,f->call,&out);
}
static void execute(uint64_t call,const ui_cap_target_v2_t *target,const char *params,void *data)
{
    fixture_t *f=data;(void)target;(void)params;f->call=call;
    if(f->sync){
      if(complete(f)==UI_STATUS_OK)++f->stack_checks;
      if(f->api.forget(f->host,call)==UI_STATUS_PENDING)++f->stack_checks;
      if(f->api.begin(f->host,"dll-write")==UI_STATUS_OK)++f->stack_checks;
      if(f->api.finish(f->host,"dll-write")==UI_STATUS_PENDING)++f->stack_checks;
      if(f->api.quiesce(f->host)==UI_STATUS_PENDING)++f->stack_checks;
      f->api.destroy(f->host);++f->stack_checks; /* Must remain alive through this DLL stack. */
    }else{f->worker=CreateThread(NULL,0,worker,f,0,NULL);f->callback=CreateThread(NULL,0,external_callback,f,0,NULL);}
}
static ui_status_t reconcile(uint64_t call,const ui_cap_target_v2_t *t,const char *params,ui_cap_reconciliation_v2_t *r,
    char *json,size_t capacity,size_t *length,void *data)
{fixture_t *f=data;(void)call;(void)t;(void)params;r->resolution=UI_CAP2_RESOLVED_APPLIED;snprintf(r->observed_revision,128,"r%d",f->revision);*length=(size_t)snprintf(json,capacity,"{\"value\":%d}",f->value);return UI_STATUS_OK;}
__declspec(dllexport) void *c2_dll_create(ui_host_t *host,const c2_dll_api_t *api,int sync)
{
    fixture_t *f=calloc(1,sizeof(*f));ui_cap_resource_desc_t resource={0};ui_capability_desc_v2_t cap={0};
    if(!f)return NULL;f->host=host;f->api=*api;f->sync=sync;
    f->worker_gate=CreateEventW(NULL,TRUE,FALSE,NULL);f->callback_gate=CreateEventW(NULL,TRUE,FALSE,NULL);
    resource.size=sizeof(resource);resource.version=1;resource.id="dll-business";resource.kind=UI_CAP_DOCUMENT;resource.get_revision=revision;resource.user_data=f;
    cap.size=sizeof(cap);cap.version=2;cap.capability_version=1;cap.id="dll-write";cap.name_utf8="DLL write";cap.description_utf8="";cap.category="test";
    cap.input_schema_json="{\"type\":\"object\",\"properties\":{},\"additionalProperties\":false}";
    cap.output_schema_json=cap.reconcile_schema_json="{\"type\":\"object\",\"properties\":{\"value\":{\"type\":\"integer\"}},\"required\":[\"value\"],\"additionalProperties\":false}";
    cap.target_kind=UI_CAP_DOCUMENT;cap.effect="write";cap.execute=execute;cap.reconcile=reconcile;cap.user_data=f;
    if(!f->worker_gate||!f->callback_gate||api->resource(host,&resource)!=UI_STATUS_OK||api->capability(host,&cap)!=UI_STATUS_OK)return NULL;
    return f;
}
__declspec(dllexport) ui_status_t c2_dll_pump(void *data)
{
    fixture_t *f=data;if(f->completed)return UI_STATUS_ALREADY_EXISTS;
    if(!f->worker||!f->callback||WaitForSingleObject(f->worker,0)!=WAIT_OBJECT_0||WaitForSingleObject(f->callback,0)!=WAIT_OBJECT_0)return UI_STATUS_PENDING;
    return complete(f);
}
__declspec(dllexport) void c2_dll_signal(void *data,int callback)
{fixture_t *f=data;SetEvent(callback?f->callback_gate:f->worker_gate);}
__declspec(dllexport) int c2_dll_state(void *data,int which)
{fixture_t *f=data;if(which==0)return f->value;if(which==1)return f->stack_checks;
 if(which==2)return f->worker&&WaitForSingleObject(f->worker,0)==WAIT_OBJECT_0;return f->callback&&WaitForSingleObject(f->callback,0)==WAIT_OBJECT_0;}
__declspec(dllexport) void c2_dll_destroy(void *data)
{fixture_t *f=data;if(f->worker)CloseHandle(f->worker);if(f->callback)CloseHandle(f->callback);CloseHandle(f->worker_gate);CloseHandle(f->callback_gate);free(f);}
