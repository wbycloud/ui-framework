#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ui_framework/application.h"
#include "ui_framework/app_data.h"
#include "ui_framework/capability.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <wchar.h>
#include "capability_fixture_control.h"
typedef struct job {ui_workspace_t *workspace;uint64_t instance,call;HANDLE gates[CAP_FIXTURE_GATE_COUNT];} job_t;
typedef struct state {ui_app_context_t context;uint64_t pending;job_t job;HANDLE worker,callback;int ready;} state_t;
static volatile LONG live_workers,live_callbacks;
static int lifecycle_errors;
#define REQUIRE(x) do{if(!(x)){fprintf(stderr,"DLL line %d: %s\n",__LINE__,#x);++lifecycle_errors;}}while(0)
static int stopped(HANDLE thread){return !thread||WaitForSingleObject(thread,0)==WAIT_OBJECT_0;}
static DWORD WINAPI worker(void *data)
{
    job_t *j=data;unsigned char result[8192]={0};ui_status_t status;
    InterlockedIncrement(&live_workers);SetEvent(j->gates[CAP_FIXTURE_WORKER_STARTED]);
    (void)WaitForSingleObject(j->gates[CAP_FIXTURE_POST],INFINITE);memcpy(result,&j->call,sizeof(j->call));
    status=ui_workspace_post_app_data(j->workspace,j->instance,CAP_FIXTURE_RESULT,result,sizeof(result));
    if(status==UI_STATUS_OK)SetEvent(j->gates[CAP_FIXTURE_POST_OK]);
    else if(status==UI_STATUS_LIMIT_EXCEEDED)SetEvent(j->gates[CAP_FIXTURE_POST_LIMIT]);
    /* A copied completion is deliberately posted before actual thread exit. */
    (void)WaitForSingleObject(j->gates[CAP_FIXTURE_WORKER_EXIT],INFINITE);
    InterlockedDecrement(&live_workers);return 0;
}
static void external_callback(job_t *j)
{SetEvent(j->gates[CAP_FIXTURE_CALLBACK_STARTED]);(void)WaitForSingleObject(j->gates[CAP_FIXTURE_CALLBACK_EXIT],INFINITE);}
static DWORD WINAPI callback_thread(void *data)
{InterlockedIncrement(&live_callbacks);external_callback(data);InterlockedDecrement(&live_callbacks);return 0;}
static void close_job(state_t *s)
{
    int i;REQUIRE(stopped(s->worker)&&stopped(s->callback));
    if(s->worker){CloseHandle(s->worker);s->worker=NULL;}if(s->callback){CloseHandle(s->callback);s->callback=NULL;}
    for(i=0;i<CAP_FIXTURE_GATE_COUNT;++i)if(s->job.gates[i]){CloseHandle(s->job.gates[i]);s->job.gates[i]=NULL;}
}
static void finish_ready(state_t *s)
{
    ui_cap_result_t result={0};char json[96];int a=stopped(s->worker),b=stopped(s->callback);
    (void)snprintf(json,sizeof(json),"{\"worker_done\":%d,\"callback_done\":%d}",a,b);
    (void)ui_host_emit_event(s->context.host,"cap.drain",json);
    if(!s->ready||!a||!b)return;
    close_job(s);result.size=sizeof(result);result.version=1;result.outcome=UI_CAP_OK;result.output_json="{\"value\":42}";
    REQUIRE(ui_capability_complete(s->context.host,s->pending,&result)==UI_STATUS_OK);s->pending=0;s->ready=0;
    (void)ui_host_emit_event(s->context.host,"cap.complete","{}");
}
static ui_status_t revision(char *b,size_t n,void *data)
{(void)data;if(n<2)return UI_STATUS_LIMIT_EXCEEDED;b[0]='1';b[1]=0;return UI_STATUS_OK;}
static void execute(uint64_t id,const ui_cap_target_t *t,const char *params,void *data)
{state_t *s=data;(void)t;(void)params;s->pending=id;}
static void execute_worker(uint64_t id,const ui_cap_target_t *t,const char *params,void *data)
{
    state_t *s=data;int i;wchar_t name[128];(void)t;s->pending=id;
    s->job.workspace=s->context.workspace;s->job.instance=s->context.instance_id;s->job.call=id;
    for(i=0;i<CAP_FIXTURE_GATE_COUNT;++i){cap_fixture_gate_name(name,s->job.instance,i);s->job.gates[i]=CreateEventW(NULL,TRUE,FALSE,name);REQUIRE(s->job.gates[i]!=NULL);if(!s->job.gates[i])return;}
    s->callback=CreateThread(NULL,0,callback_thread,&s->job,0,NULL);s->worker=CreateThread(NULL,0,worker,&s->job,0,NULL);REQUIRE(s->worker&&s->callback);
    if(strstr(params,"true")){
        unsigned char filler[4096]={0};ui_status_t status;unsigned count=0;
        do{status=ui_workspace_post_app_data(s->context.workspace,s->context.instance_id,CAP_FIXTURE_FILLER,filler,sizeof(filler));if(status==UI_STATUS_OK)++count;}while(status==UI_STATUS_OK);
        REQUIRE(status==UI_STATUS_LIMIT_EXCEEDED&&count>0);SetEvent(s->job.gates[CAP_FIXTURE_POST]);
    }
}
static void execute_sync(uint64_t id,const ui_cap_target_t *t,const char *params,void *data)
{
    state_t *s=data;ui_cap_result_t r={0};r.size=sizeof(r);r.version=1;r.outcome=UI_CAP_OK;r.output_json="{\"value\":42}";
    REQUIRE(ui_capability_complete(s->context.host,id,&r)==UI_STATUS_OK);
    REQUIRE(ui_capability_forget(s->context.host,id)==UI_STATUS_PENDING);
    REQUIRE(ui_capability_begin_unregistration(s->context.host,"fixture.sync")==UI_STATUS_OK);
    REQUIRE(ui_capability_finish_unregistration(s->context.host,"fixture.sync")==UI_STATUS_PENDING);
    REQUIRE(ui_capability_begin_resource_unregistration(s->context.host,"fixture.document")==UI_STATUS_OK);
    REQUIRE(ui_capability_finish_resource_unregistration(s->context.host,"fixture.document")==UI_STATUS_PENDING);
    REQUIRE(ui_workspace_close(s->context.workspace,s->context.instance_id,UI_APP_CLOSE_TAB)==UI_STATUS_OK);
    ui_workspace_poll(s->context.workspace);
    REQUIRE(ui_workspace_count(s->context.workspace)==1&&GetModuleHandleW(L"ui_capability_fixture.dll")!=NULL);
    REQUIRE(!strcmp(t->resource_id,"fixture.document")&&!strcmp(params,"{}"));
    REQUIRE(ui_capability_get_result(s->context.host,id,&r)==UI_STATUS_OK&&r.state==UI_CAP_FINISHED&&r.outcome==UI_CAP_OK&&!strcmp(r.output_json,"{\"value\":42}"));
    (void)ui_host_emit_event(s->context.host,"cap.sync","{}");
}
static void received(uint64_t kind,const void *data,size_t bytes,void *user)
{
    state_t *s=user;uint64_t id;ui_cap_result_t r={0};ui_status_t status;
    if((kind!=CAP_FIXTURE_RESULT&&kind!=CAP_FIXTURE_POLL&&kind!=CAP_FIXTURE_RETRY)||(bytes!=sizeof(id)&&bytes!=8192))return;memcpy(&id,data,sizeof(id));
    if(kind==CAP_FIXTURE_RETRY){
        REQUIRE(id==s->pending&&stopped(s->worker)&&stopped(s->callback));
        REQUIRE(ui_workspace_post_app_data(s->job.workspace,s->job.instance,CAP_FIXTURE_RESULT,&s->job.call,sizeof(s->job.call))==UI_STATUS_OK);return;
    }
    if(s->worker||s->callback){REQUIRE(id==s->pending);if(id!=s->pending)return;if(kind==CAP_FIXTURE_RESULT)s->ready=1;finish_ready(s);return;}
    if(kind!=CAP_FIXTURE_RESULT)return;
    r.size=sizeof(r);r.version=1;r.outcome=UI_CAP_OK;r.output_json="{\"value\":42}";
    status=ui_capability_complete(s->context.host,id,&r);
    if(status==UI_STATUS_OK){s->pending=0;(void)ui_host_emit_event(s->context.host,"cap.complete","{}");}
}
static ui_status_t UI_APP_CALL create(const ui_app_context_t *c,void **out,ui_assistant_config_t *assistant)
{
    state_t *s=calloc(1,sizeof(*s));ui_cap_resource_desc_t r={0};ui_capability_desc_t d={0};ui_status_t status;
    (void)assistant;if(!s)return UI_STATUS_OUT_OF_MEMORY;*out=s;s->context=*c;
    r.size=sizeof(r);r.version=1;r.kind=UI_CAP_DOCUMENT;r.id="fixture.document";r.get_revision=revision;
    status=ui_capability_register_resource(c->host,&r);if(status)return status;
    d.size=sizeof(d);d.version=d.capability_version=1;d.id="fixture.read";d.name_utf8="Fixture read";d.description_utf8="Stable ID asynchronous fixture";d.category="fixture";
    d.input_schema_json="{\"type\":\"object\",\"additionalProperties\":false}";
    d.output_schema_json="{\"type\":\"object\",\"properties\":{\"value\":{\"type\":\"integer\"}},\"required\":[\"value\"],\"additionalProperties\":false}";
    d.target_kind=UI_CAP_DOCUMENT;d.effect="read";d.execute=execute;d.user_data=s;
    status=ui_capability_register(c->host,&d);if(status)return status;
    d.id="fixture.sync";d.execute=execute_sync;status=ui_capability_register(c->host,&d);if(status)return status;
    d.id="fixture.worker";d.execute=execute_worker;d.input_schema_json="{\"type\":\"object\",\"properties\":{\"fill\":{\"type\":\"boolean\"}},\"required\":[\"fill\"],\"additionalProperties\":false}";
    return ui_capability_register(c->host,&d);
}
static ui_status_t UI_APP_CALL mount(void *data,const ui_app_context_t *c)
{state_t *s=data;s->context=*c;return ui_app_set_data_callback(c,received,s);}
static ui_app_close_decision_t UI_APP_CALL close_app(void *data,ui_app_close_reason_t reason)
{char flag[2];(void)data;(void)reason;return GetEnvironmentVariableA("UI_CAP_FIXTURE_CLOSE_WAIT",flag,sizeof(flag))?UI_APP_CLOSE_WAIT:UI_APP_CLOSE_ALLOW;}
static void UI_APP_CALL unmount(void *data){state_t *s=data;REQUIRE(stopped(s->worker)&&stopped(s->callback));}
static void UI_APP_CALL destroy(void *data){state_t *s=data;if(!stopped(s->worker)||!stopped(s->callback)){REQUIRE(0);return;}close_job(s);free(s);}
static ui_status_t UI_APP_CALL shutdown_module(void){return live_workers||live_callbacks||lifecycle_errors?UI_STATUS_PLATFORM_ERROR:UI_STATUS_OK;}
UI_APP_EXPORT const ui_app_descriptor_t *UI_APP_CALL ui_app_query_v1(void)
{static const ui_app_descriptor_t d={sizeof(d),UI_APPLICATION_ABI_VERSION,UI_FRAMEWORK_API_VERSION,create,mount,NULL,close_app,unmount,destroy,shutdown_module};return &d;}
