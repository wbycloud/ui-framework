#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ai_edit_fixture_control.h"
#include "ui_framework/app_data.h"
#include "ui_framework/app_poll.h"
#include "ui_internal.h"
typedef struct edit_fixture {
 ui_app_context_t context;int value,staged,before,undo_value,revision,mode,ready,close_wait;
 unsigned transactions,executes,commits,rollbacks,undos,proofs,revision_reads;
 uint64_t call,read_pending;ui_cap_edit_context_t pending;HANDLE worker,callback,worker_exit,callback_exit;
 char transaction[65],undo_id[65];
} edit_fixture_t;
static LONG live_workers,live_callbacks,errors;
#define REQUIRE(x) do{if(!(x)){InterlockedIncrement(&errors);fprintf(stderr,"edit DLL line %d: %s\n",__LINE__,#x);}}while(0)
static int stopped(HANDLE h){return !h||WaitForSingleObject(h,0)==WAIT_OBJECT_0;}
static void snapshot(edit_fixture_t *f)
{char json[400];snprintf(json,sizeof(json),"{\"value\":%d,\"revision\":%d,\"executes\":%u,\"commits\":%u,\"rollbacks\":%u,\"undos\":%u,\"proofs\":%u,\"worker_done\":%d,\"callback_done\":%d}",f->value,f->revision,f->executes,f->commits,f->rollbacks,f->undos,f->proofs,stopped(f->worker),stopped(f->callback));REQUIRE(ui_host_emit_event(f->context.host,"edit.state",json)==UI_STATUS_OK);}
static ui_status_t revision(char *out,size_t cap,void *data)
{edit_fixture_t *f=data;snprintf(out,cap,"r%d",f->revision);if(f->mode==AE_MODE_REENTER_PROPOSE){f->mode=0;REQUIRE(ui_host_emit_event(f->context.host,"edit.reenter","{} ")==UI_STATUS_OK);}if((f->mode==AE_MODE_LAST_MODAL||f->mode==AE_MODE_LAST_EXPIRY)&&++f->revision_reads==2){if(f->mode==AE_MODE_LAST_MODAL)f->context.host->modal_component=(ui_component_t *)(uintptr_t)1;else REQUIRE(ui_host_emit_event(f->context.host,"edit.expire","{} ")==UI_STATUS_OK);f->mode=0;}return UI_STATUS_OK;}
static ui_status_t check(const ui_cap_target_v2_t *target,const char *params,void *data)
{edit_fixture_t *f=data;(void)target;(void)params;if(f->mode==AE_MODE_REVISION_CHECK){++f->revision;f->mode=0;}if(f->mode==AE_MODE_DISABLE_CHECK||f->mode==AE_MODE_BUSY_CHECK){ui_command_state_t state={0};state.size=sizeof(state);state.visible=1;state.enabled=f->mode==AE_MODE_BUSY_CHECK;state.busy=f->mode==AE_MODE_BUSY_CHECK;REQUIRE(ui_host_set_command_state(f->context.host,"memory.set",&state)==UI_STATUS_OK);f->mode=0;}return UI_STATUS_OK;}
static ui_status_t reconcile(uint64_t id,const ui_cap_target_v2_t *target,const char *params,ui_cap_reconciliation_v2_t *proof,char *json,size_t capacity,size_t *length,void *data)
{(void)id;(void)target;(void)params;(void)proof;(void)json;(void)capacity;(void)length;(void)data;return UI_STATUS_PERMISSION_DENIED;}
static void complete(edit_fixture_t *f,const ui_cap_edit_context_t *stage,ui_status_t status,const char *output,ui_cap_effect_v2_t effect)
{
 ui_cap_edit_completion_t c={0};c.size=sizeof(c);c.version=1;c.stage_id=stage->stage_id;c.status=status;c.effect=effect;c.executor_exited=stopped(f->worker)&&stopped(f->callback);strcpy_s(c.transaction_id,65,f->transaction);strcpy_s(c.undo_id,65,f->undo_id);c.output_json=output;if(f->mode==AE_MODE_BAD_RECEIPT&&stage->stage==UI_EDIT_COMMIT)strcpy_s(c.transaction_id,65,"wrong-transaction");if(f->mode==AE_MODE_BAD_PROOF&&stage->stage==UI_EDIT_RECONCILE)c.transaction_id[0]=0;
 {ui_cap_edit_completion_t invalid=c;invalid.stage_id=0;REQUIRE(ui_capability_edit_complete(f->context.host,stage->call_id,&invalid)==UI_STATUS_INVALID_ARGUMENT);invalid=c;invalid.executor_exited=0;REQUIRE(ui_capability_edit_complete(f->context.host,stage->call_id,&invalid)==UI_STATUS_PENDING);}
 REQUIRE(ui_capability_edit_complete(f->context.host,stage->call_id,&c)==UI_STATUS_OK);REQUIRE(ui_capability_edit_complete(f->context.host,stage->call_id,&c)==UI_STATUS_ALREADY_EXISTS);
}
static DWORD WINAPI worker(void *data)
{edit_fixture_t *f=data;uint64_t id=f->pending.call_id;InterlockedIncrement(&live_workers);REQUIRE(ui_workspace_post_app_data(f->context.workspace,f->context.instance_id,AE_APP_READY,&id,sizeof(id))==UI_STATUS_OK);WaitForSingleObject(f->worker_exit,INFINITE);InterlockedDecrement(&live_workers);return 0;}
static DWORD WINAPI external(void *data)
{edit_fixture_t *f=data;InterlockedIncrement(&live_callbacks);WaitForSingleObject(f->callback_exit,INFINITE);InterlockedDecrement(&live_callbacks);return 0;}
static void execute(uint64_t id,const ui_cap_target_v2_t *target,const char *params,void *data)
{
 edit_fixture_t *f=data;ui_cap_edit_context_t stage={0};int value=0;(void)target;++f->executes;stage.size=sizeof(stage);stage.version=1;
 if(ui_capability_edit_get_stage(f->context.host,id,&stage)!=UI_STATUS_OK){ui_cap_completion_v2_t c={0};if(sscanf_s(params,"{\"value\":%d}",&value)==1){f->value=value;++f->revision;}c.size=sizeof(c);c.version=2;c.outcome=UI_CAP2_OK;c.effect=UI_CAP2_EFFECT_APPLIED;c.output_json="{\"value\":33}";REQUIRE(ui_capability_complete_v2(f->context.host,id,&c)==UI_STATUS_OK);return;}
 REQUIRE(sscanf_s(params,"{\"value\":%d}",&value)==1);f->staged=value;f->call=id;
 if(f->mode==AE_MODE_ASYNC){f->pending=stage;f->ready=0;f->worker_exit=CreateEventW(NULL,TRUE,FALSE,NULL);f->callback_exit=CreateEventW(NULL,TRUE,FALSE,NULL);f->worker=CreateThread(NULL,0,worker,f,0,NULL);f->callback=CreateThread(NULL,0,external,f,0,NULL);REQUIRE(f->worker&&f->callback);return;}
 complete(f,&stage,f->mode==AE_MODE_EXECUTE_FAIL||f->mode==AE_MODE_ROLLBACK_FAIL?UI_STATUS_VALIDATION_FAILED:UI_STATUS_OK,NULL,UI_CAP2_EFFECT_NONE);
 if(f->mode==AE_MODE_SYNC_RETIRE){REQUIRE(ui_capability_forget_v2(f->context.host,id)==UI_STATUS_PERMISSION_DENIED);REQUIRE(ui_capability_begin_unregistration(f->context.host,"memory.set")==UI_STATUS_OK);REQUIRE(ui_capability_finish_unregistration(f->context.host,"memory.set")==UI_STATUS_PENDING);REQUIRE(ui_workspace_close(f->context.workspace,f->context.instance_id,UI_APP_CLOSE_TAB)==UI_STATUS_OK);ui_workspace_poll(f->context.workspace);REQUIRE(ui_workspace_count(f->context.workspace)==1&&GetModuleHandleW(L"ui_ai_edit_fixture.dll")!=NULL);REQUIRE(!strcmp(target->resource.resource_id,"memory.document")&&strstr(params,"value"));}
}
static void transaction(const ui_cap_edit_context_t *c,void *data)
{
 edit_fixture_t *f=data;char json[96];ui_status_t status=UI_STATUS_OK;ui_cap_effect_v2_t effect=UI_CAP2_EFFECT_NONE;const char *output=NULL;
 if(c->stage==UI_EDIT_BEGIN){f->before=f->value;f->staged=f->value;snprintf(f->transaction,65,"txn-%u",++f->transactions);}
 else if(c->stage==UI_EDIT_COMMIT){f->undo_value=f->before;f->value=f->staged;++f->revision;++f->commits;snprintf(f->undo_id,65,"undo-%u",f->commits);effect=UI_CAP2_EFFECT_APPLIED;}
 else if(c->stage==UI_EDIT_ROLLBACK){++f->rollbacks;f->staged=f->before;if(f->mode==AE_MODE_ROLLBACK_FAIL){f->value=77;++f->revision;status=UI_STATUS_PLATFORM_ERROR;effect=UI_CAP2_EFFECT_UNKNOWN;}}
 else if(c->stage==UI_EDIT_UNDO){++f->undos;if(strcmp(c->undo_id,f->undo_id)||!f->undo_id[0])status=UI_STATUS_NOT_FOUND;else f->staged=f->undo_value;}
 else if(c->stage==UI_EDIT_READBACK||c->stage==UI_EDIT_RECONCILE){if(c->stage==UI_EDIT_RECONCILE)++f->proofs;snprintf(json,sizeof(json),"{\"value\":%d}",f->value);output=f->mode==AE_MODE_INVALID_OUTPUT&&f->commits?"{\"value\":\"invalid\"}":json;effect=f->value==f->before?UI_CAP2_EFFECT_NONE:UI_CAP2_EFFECT_APPLIED;}
 complete(f,c,status,output,effect);
 if(c->stage==UI_EDIT_READBACK||c->stage==UI_EDIT_RECONCILE)snapshot(f);
}
static void cancel(uint64_t id,void *data){edit_fixture_t *f=data;(void)id;snapshot(f);}
static void read_complete(edit_fixture_t *f,uint64_t id){ui_cap_result_t r={0};char json[64];snprintf(json,sizeof(json),"{\"value\":%d}",f->value);r.size=sizeof(r);r.version=1;r.outcome=UI_CAP_OK;r.output_json=json;REQUIRE(ui_capability_complete(f->context.host,id,&r)==UI_STATUS_OK);REQUIRE(ui_capability_complete(f->context.host,id,&r)==UI_STATUS_ALREADY_EXISTS);}
static void poll(const ui_app_context_t *context,void *data)
{edit_fixture_t *f=data;(void)context;if(f->ready&&stopped(f->worker)&&stopped(f->callback)){CloseHandle(f->worker);CloseHandle(f->callback);CloseHandle(f->worker_exit);CloseHandle(f->callback_exit);f->worker=f->callback=f->worker_exit=f->callback_exit=NULL;f->ready=0;if(f->read_pending){uint64_t id=f->read_pending;f->read_pending=0;read_complete(f,id);}else complete(f,&f->pending,UI_STATUS_OK,NULL,UI_CAP2_EFFECT_NONE);snapshot(f);}
 if(f->close_wait&&stopped(f->worker)&&stopped(f->callback)){f->close_wait=0;REQUIRE(ui_workspace_post_close_complete(f->context.workspace,f->context.instance_id,UI_APP_CLOSE_ALLOW)==UI_STATUS_OK);}}
static void received(uint64_t kind,const void *data,size_t bytes,void *user)
{edit_fixture_t *f=user;if(kind==AE_APP_BUMP_REVISION){++f->revision;snapshot(f);}else if(kind==AE_APP_CONTROL&&bytes==sizeof(int)){memcpy(&f->mode,data,sizeof(int));f->revision_reads=0;snapshot(f);}else if(kind==AE_APP_READY&&bytes==sizeof(uint64_t)){uint64_t id;memcpy(&id,data,sizeof(id));if(id==f->pending.call_id)f->ready=1;}else if(kind==AE_APP_WORKER_EXIT&&f->worker_exit)SetEvent(f->worker_exit);else if(kind==AE_APP_CALLBACK_EXIT&&f->callback_exit)SetEvent(f->callback_exit);else if(kind==AE_APP_SNAPSHOT)snapshot(f);}
static void read_value(uint64_t id,const ui_cap_target_t *target,const char *params,void *data)
{edit_fixture_t *f=data;(void)target;(void)params;if(f->mode==AE_MODE_READ_ASYNC){f->read_pending=id;f->pending.call_id=id;f->ready=0;f->worker_exit=CreateEventW(NULL,TRUE,FALSE,NULL);f->callback_exit=CreateEventW(NULL,TRUE,FALSE,NULL);f->worker=CreateThread(NULL,0,worker,f,0,NULL);f->callback=CreateThread(NULL,0,external,f,0,NULL);REQUIRE(f->worker&&f->callback);}else read_complete(f,id);}

static ui_status_t UI_APP_CALL create(const ui_app_context_t *context,void **out,ui_assistant_config_t *assistant)
{
 edit_fixture_t *f=calloc(1,sizeof(*f));ui_cap_resource_desc_t r={0};ui_cap_edit_desc_t d={0};ui_capability_desc_t read={0};ui_status_t status;(void)assistant;if(!f)return UI_STATUS_OUT_OF_MEMORY;*out=f;f->context=*context;f->before=f->value=10;f->revision=1;
 status=ui_capability_enable_read_entries(context->host);if(status)return status;r.size=sizeof(r);r.version=1;r.id="memory.document";r.kind=UI_CAP_DOCUMENT;r.get_revision=revision;r.user_data=f;status=ui_capability_register_resource(context->host,&r);if(status)return status;
 d.size=sizeof(d);d.version=1;d.risk=UI_EDIT_REVERSIBLE;d.transaction=transaction;d.capability.size=sizeof(d.capability);d.capability.version=2;d.capability.capability_version=1;d.capability.id="memory.set";d.capability.name_utf8="Set memory value";d.capability.description_utf8="Application-owned staged memory transaction";d.capability.category="memory";d.capability.target_kind=UI_CAP_DOCUMENT;d.capability.effect="write";
 d.capability.input_schema_json="{\"type\":\"object\",\"properties\":{\"value\":{\"type\":\"integer\",\"minimum\":0,\"maximum\":100}},\"required\":[\"value\"],\"additionalProperties\":false}";
 d.capability.output_schema_json=d.capability.reconcile_schema_json="{\"type\":\"object\",\"properties\":{\"value\":{\"type\":\"integer\"}},\"required\":[\"value\"],\"additionalProperties\":false}";d.capability.check=check;d.capability.execute=execute;d.capability.cancel=cancel;d.capability.reconcile=reconcile;d.capability.user_data=f;
 status=ui_capability_register_edit(context->host,&d);if(status)return status;d.risk=UI_EDIT_DESTRUCTIVE;d.capability.id="memory.destructive";status=ui_capability_register_edit(context->host,&d);if(status)return status;d.risk=UI_EDIT_EXTERNAL;d.capability.id="memory.external";status=ui_capability_register_edit(context->host,&d);if(status)return status;d.risk=UI_EDIT_IRREVERSIBLE;d.capability.id="memory.irreversible";status=ui_capability_register_edit(context->host,&d);if(status)return status;
 read.size=sizeof(read);read.version=read.capability_version=1;read.id="memory.read";read.name_utf8="Read actual memory value";read.description_utf8="";read.category="memory";read.effect="read";read.target_kind=UI_CAP_DOCUMENT;read.input_schema_json="{\"type\":\"object\",\"additionalProperties\":false}";read.output_schema_json=d.capability.output_schema_json;read.execute=read_value;read.user_data=f;return ui_capability_register(context->host,&read);
}
static ui_status_t UI_APP_CALL mount(void *data,const ui_app_context_t *context)
{edit_fixture_t *f=data;f->context=*context;if(ui_app_set_data_callback(context,received,f)!=UI_STATUS_OK)return UI_STATUS_PLATFORM_ERROR;return ui_app_set_poll_callback(context,poll,f);}
static ui_app_close_decision_t UI_APP_CALL close_app(void *data,ui_app_close_reason_t reason)
{edit_fixture_t *f=data;(void)reason;if(!stopped(f->worker)||!stopped(f->callback)){f->close_wait=1;return UI_APP_CLOSE_WAIT;}return UI_APP_CLOSE_ALLOW;}
static void UI_APP_CALL destroy(void *data)
{edit_fixture_t *f=data;REQUIRE(stopped(f->worker)&&stopped(f->callback));if(f->worker)CloseHandle(f->worker);if(f->callback)CloseHandle(f->callback);if(f->worker_exit)CloseHandle(f->worker_exit);if(f->callback_exit)CloseHandle(f->callback_exit);free(f);}
static ui_status_t UI_APP_CALL shutdown(void){return live_workers||live_callbacks||errors?UI_STATUS_PLATFORM_ERROR:UI_STATUS_OK;}
static void UI_APP_CALL unmount(void *data)
{edit_fixture_t *f=data;REQUIRE(stopped(f->worker)&&stopped(f->callback));}
UI_APP_EXPORT const ui_app_descriptor_t *UI_APP_CALL ui_app_query_v1(void)
{static const ui_app_descriptor_t d={sizeof(d),1,UI_FRAMEWORK_API_VERSION,create,mount,NULL,close_app,unmount,destroy,shutdown};return &d;}
