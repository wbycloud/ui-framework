#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "ui_framework/application.h"
#include "ui_framework/app_data.h"
#include "ui_framework/app_poll.h"
#include "ui_framework/capability_entry.h"
typedef struct poll_app {ui_app_context_t context;unsigned polls;int in_poll,closing,posted;ui_status_t enable_status,collect_status,retire_status;uint64_t call;HANDLE release,worker;char json[224];} poll_app_t;
static LONG live,workers;
static ui_status_t revision(char *out,size_t capacity,void *data)
{(void)data;return strcpy_s(out,capacity,"r0")?UI_STATUS_INVALID_ARGUMENT:UI_STATUS_OK;}
static void execute(uint64_t id,const ui_cap_target_t *target,const char *params,void *data)
{poll_app_t *s=data;ui_cap_result_t result={0};(void)target;(void)params;result.size=sizeof(result);result.version=1;result.outcome=UI_CAP_OK;result.output_json="{}";(void)ui_capability_complete(s->context.host,id,&result);}
static DWORD WINAPI worker(void *data)
{poll_app_t *s=data;InterlockedIncrement(&workers);WaitForSingleObject(s->release,INFINITE);InterlockedDecrement(&workers);return 0;}
static void poll_app(const ui_app_context_t *context,void *data)
{
    poll_app_t *s=data;++s->polls;++s->in_poll;ui_host_destroy(context->host);ui_assistant_destroy(context->assistant);s->enable_status=ui_capability_enable_read_entries(context->host);
    if(s->polls==1){ui_cap_target_t target={0};ui_status_t status;target.size=sizeof(target);target.version=1;ui_workspace_poll(context->workspace);
      status=ui_capability_bind(context->host,"document",&target);if(!status)status=ui_capability_prepare(context->host,"read",&target,"{}",&s->call);
      if(!status)status=ui_capability_dispatch(context->host,s->call);s->collect_status=status?status:ui_capability_forget(context->host,s->call);
      (void)ui_capability_begin_unregistration(context->host,"read");s->retire_status=ui_capability_finish_unregistration(context->host,"read");}
    if(s->closing&&!s->posted&&(!s->worker||WaitForSingleObject(s->worker,0)==WAIT_OBJECT_0)){
      if(ui_workspace_post_close_complete(context->workspace,context->instance_id,UI_APP_CLOSE_ALLOW)==UI_STATUS_OK)s->posted=1;}
    --s->in_poll;
}
static void data_app(uint64_t kind,const void *data,size_t bytes,void *user)
{
    poll_app_t *s=user;(void)data;(void)bytes;
    if(kind==1&&!s->worker)s->worker=CreateThread(NULL,0,worker,s,0,NULL);
    else if(kind==2)SetEvent(s->release);
    else if(kind==3)(void)ui_app_set_poll_callback(&s->context,NULL,NULL);
    else if(kind==4)(void)ui_app_set_poll_callback(&s->context,poll_app,s);
}
static const char *snapshot(void *data)
{poll_app_t *s=data;snprintf(s->json,sizeof(s->json),"{\"polls\":%u,\"in_poll\":%d,\"enable\":%d,\"collect\":%d,\"retire\":%d,\"worker\":%s}",s->polls,s->in_poll,s->enable_status,s->collect_status,s->retire_status,s->worker?"true":"false");return s->json;}
static ui_status_t UI_APP_CALL create(const ui_app_context_t *context,void **out,ui_assistant_config_t *assistant)
{
    poll_app_t *s=calloc(1,sizeof(*s));ui_cap_resource_desc_t resource={0};ui_capability_desc_t cap={0};ui_status_t status;
    if(!s)return UI_STATUS_OUT_OF_MEMORY;s->context=*context;s->release=CreateEventW(NULL,TRUE,FALSE,NULL);*out=s;
    InterlockedIncrement(&live);assistant->snapshot=snapshot;assistant->user_data=s;if(!s->release)return UI_STATUS_PLATFORM_ERROR;
    resource.size=sizeof(resource);resource.version=1;resource.id="document";resource.kind=UI_CAP_DOCUMENT;resource.get_revision=revision;resource.user_data=s;
    status=ui_capability_register_resource(context->host,&resource);if(status)return status;cap.size=sizeof(cap);cap.version=1;cap.capability_version=1;cap.id="read";cap.name_utf8="Read";cap.description_utf8="";cap.category="fixture";
    cap.input_schema_json=cap.output_schema_json="{\"type\":\"object\",\"properties\":{},\"additionalProperties\":false}";cap.effect="read";cap.target_kind=UI_CAP_DOCUMENT;cap.execute=execute;cap.user_data=s;return ui_capability_register(context->host,&cap);
}
static ui_status_t UI_APP_CALL mount(void *data,const ui_app_context_t *context)
{poll_app_t *s=data;ui_status_t status;s->context=*context;status=ui_app_set_data_callback(context,data_app,s);return status?status:ui_app_set_poll_callback(context,poll_app,s);}
static ui_app_close_decision_t UI_APP_CALL close_app(void *data,ui_app_close_reason_t reason)
{poll_app_t *s=data;(void)reason;s->closing=1;return s->worker&&WaitForSingleObject(s->worker,0)!=WAIT_OBJECT_0?UI_APP_CLOSE_WAIT:UI_APP_CLOSE_ALLOW;}
static void UI_APP_CALL unmount(void *data)
{poll_app_t *s=data;if(s->in_poll)abort();if(s->worker){if(WaitForSingleObject(s->worker,0)!=WAIT_OBJECT_0)abort();CloseHandle(s->worker);s->worker=NULL;}}
static void UI_APP_CALL destroy(void *data)
{poll_app_t *s=data;if(s->release)CloseHandle(s->release);free(s);InterlockedDecrement(&live);}
static ui_status_t UI_APP_CALL shutdown_module(void){return live||workers?UI_STATUS_PENDING:UI_STATUS_OK;}
UI_APP_EXPORT const ui_app_descriptor_t *UI_APP_CALL ui_app_query_v1(void)
{static const ui_app_descriptor_t d={sizeof(d),1,UI_FRAMEWORK_API_VERSION,create,mount,NULL,close_app,unmount,destroy,shutdown_module};return &d;}
