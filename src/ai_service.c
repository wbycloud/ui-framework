#include "ui_framework/ai_model_view.h"
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include "ai_internal.h"
#include "ui_internal.h"
#include <objbase.h>
#include "ai_edit_internal.h"
#include "ai_graph_internal.h"
#include "ai_model_internal.h"
#define UI_AI_ASSISTANT_IMPLEMENTATION
#include "ai_assistant_internal.h"
#include "ai_validation_batch_internal.h"

static DWORD WINAPI ai_worker(void *);
static void av_dispose(ui_ai_service_t *);
static void av_ui(ui_ai_service_t *);
static char *ai_copy(const char *p)
{size_t n=strlen(p?p:"")+1;char *q=malloc(n);if(q)memcpy(q,p?p:"",n);return q;}
static int ai_text(const char *p,size_t maximum,int empty)
{size_t n;if(!p)return empty;n=strlen(p);if(n>maximum||(!n&&!empty))return 0;return !n||MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,p,(int)n,NULL,0)>0;}
static int ai_id(const char *p)
{size_t n;if(!p||!(n=strlen(p))||n>64)return 0;for(size_t i=0;i<n;++i)if(!((p[i]>='0'&&p[i]<='9')||(p[i]>='a'&&p[i]<='z')||(p[i]>='A'&&p[i]<='Z')||strchr("_.:-",p[i])))return 0;return 1;}
static int ai_uuid(char out[37])
{GUID g;if(FAILED(CoCreateGuid(&g)))return 0;snprintf(out,37,"%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",(unsigned long)g.Data1,g.Data2,g.Data3,g.Data4[0],g.Data4[1],g.Data4[2],g.Data4[3],g.Data4[4],g.Data4[5],g.Data4[6],g.Data4[7]);return 1;}
static int ai_ui(ui_ai_service_t *s){return s&&s->thread==GetCurrentThreadId();}
static uint64_t ai_now(ui_ai_service_t *s){return GetTickCount64()+s->time_bias;}
static int ai_cancelled(ui_ai_service_t *s,ai_run_t *r)
{int cancelled;EnterCriticalSection(&s->lock);cancelled=r->cancel;LeaveCriticalSection(&s->lock);return cancelled;}
static int ai_find(ui_ai_service_t *s,const char *id)
{for(int i=0;i<AI_RUNS;++i)if(s->runs[i].id[0]&&!strcmp(s->runs[i].id,id))return i;return -1;}
static char *ai_run_json(ai_run_t *r)
{
 yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *o=d?yyjson_mut_obj(d):NULL;char *p;if(!o){yyjson_mut_doc_free(d);return NULL;}yyjson_mut_doc_set_root(d,o);
 yyjson_mut_obj_add_strcpy(d,o,"run_id",r->id);yyjson_mut_obj_add_strcpy(d,o,"client_request_id",r->client);yyjson_mut_obj_add_strcpy(d,o,"parent_run_id",r->parent);
 yyjson_mut_obj_add_int(d,o,"phase",r->phase);yyjson_mut_obj_add_bool(d,o,"committed",r->committed!=0);yyjson_mut_obj_add_bool(d,o,"terminal",r->terminal!=0);
 yyjson_mut_obj_add_strcpy(d,o,"outcome",r->outcome);yyjson_mut_obj_add_strcpy(d,o,"reply",r->reply?r->reply:"");yyjson_mut_obj_add_strcpy(d,o,"question_id",r->question);yyjson_mut_obj_add_strcpy(d,o,"error",r->error);
 yyjson_mut_obj_add_strcpy(d,o,"question",r->question_text?r->question_text:"");
 yyjson_mut_obj_add_uint(d,o,"steps",r->steps);yyjson_mut_obj_add_uint(d,o,"model_calls",r->model_calls);yyjson_mut_obj_add_uint(d,o,"tool_calls",r->tool_calls);
 if(r->model){yyjson_mut_obj_add_bool(d,o,"real_provider",true);yyjson_mut_obj_add_strcpy(d,o,"remote_delivery",!r->model->net.sent?"not_sent":r->model->net.response_complete?"response_received":"remote_processing_unknown");yyjson_mut_obj_add_strcpy(d,o,"binding_hash",r->model->binding);yyjson_mut_obj_add_strcpy(d,o,"model_id",r->model->model);yyjson_mut_obj_add_int(d,o,"provider_phase",r->model->phase);yyjson_mut_obj_add_bool(d,o,"disclosure_confirmed",r->model->confirmed!=0);yyjson_mut_obj_add_bool(d,o,"disclosure_saved",r->model->confirmation_saved!=0);yyjson_mut_obj_add_bool(d,o,"model_request_saved",r->model->request_saved!=0);yyjson_mut_obj_add_strcpy(d,o,"provider_request_id",r->model->request_id);yyjson_mut_obj_add_strcpy(d,o,"disclosure",r->plan);yyjson_mut_obj_add_strcpy(d,o,"input",r->input);yyjson_mut_obj_add_strcpy(d,o,"fixed_targets",r->targets_json);yyjson_mut_obj_add_strcpy(d,o,"context_preview",r->model->preview?r->model->preview:"");}
 p=yyjson_mut_write(d,0,NULL);yyjson_mut_doc_free(d);return p;
}
static int ai_start_locked(ui_ai_service_t *s)
{
 if(s->worker&&WaitForSingleObject(s->worker,0)==WAIT_OBJECT_0){CloseHandle(s->worker);s->worker=NULL;}
 WakeAllConditionVariable(&s->changed);if(s->worker)return 1;
 s->worker=CreateThread(NULL,0,ai_worker,s,0,NULL);if(!s->worker){snprintf(s->error,sizeof(s->error),"Cannot start background execution");s->phase=UI_AI_UNAVAILABLE;return 0;}return 1;
}
static void ai_notice_free(ai_notice_t *n){if(n){free(n->payload);free(n);}}
static int ai_emit(ui_ai_service_t *s,const char *type,ai_run_t *r,const char *query,uint64_t seq,int committed,const char *payload,int display)
{
 ai_notice_t *n;size_t bytes=sizeof(*n)+strlen(payload)+1;
 if(bytes>UI_AI_EVENT_BYTES)return 0;
 EnterCriticalSection(&s->lock);
 while(s->event_count==AI_EVENTS||bytes>UI_AI_EVENT_BYTES-s->event_bytes){
  if(display){LeaveCriticalSection(&s->lock);return 1;}/* authoritative cache retains full prefix */
  SleepConditionVariableCS(&s->changed,&s->lock,50);
 }
 n=calloc(1,sizeof(*n));if(!n){LeaveCriticalSection(&s->lock);return 0;}n->payload=ai_copy(payload);if(!n->payload){free(n);LeaveCriticalSection(&s->lock);return 0;}
 n->bytes=bytes;strcpy_s(n->type,sizeof(n->type),type);strcpy_s(n->conversation,sizeof(n->conversation),s->conversation);
 if(r){strcpy_s(n->client,sizeof(n->client),r->client);strcpy_s(n->run,sizeof(n->run),r->id);}if(query)strcpy_s(n->query,sizeof(n->query),query);
 n->event.size=sizeof(n->event);n->event.version=1;n->event.sequence=seq;n->event.committed=committed;
 s->events[(s->event_head+s->event_count)%AI_EVENTS]=n;++s->event_count;s->event_bytes+=bytes;LeaveCriticalSection(&s->lock);return 1;
}
static void ai_notify(ui_ai_service_t *s,ai_notice_t *n)
{
 n->event.type=n->type;n->event.conversation_id=n->conversation;n->event.client_request_id=n->client;n->event.run_id=n->run;n->event.query_id=n->query;n->event.payload=n->payload;
 ++s->callbacks;if(s->config.on_event)s->config.on_event(&n->event,s->config.user_data);--s->callbacks;
}
static void ai_store_error(ui_ai_service_t *s,ai_run_t *r,const char *where)
{
 char text[256],*json;snprintf(text,sizeof(text),"%s: storage confirmation unavailable (%s)",where,s->db?sqlite3_errmsg(s->db):"open/schema rejected");
 EnterCriticalSection(&s->lock);s->phase=UI_AI_UNAVAILABLE;s->store_failed=1;strcpy_s(s->error,sizeof(s->error),text);
 if(r){r->phase=UI_AI_UNCONFIRMED;r->committed=0;strcpy_s(r->error,sizeof(r->error),text);}json=r?ai_run_json(r):ai_copy("{\"error\":\"storage unavailable; original database retained\"}");LeaveCriticalSection(&s->lock);
 if(json){(void)ai_emit(s,"storage_error",r,NULL,0,0,json,0);free(json);}
}
#include "ai_store.inc"

static void ai_run_free(ui_ai_service_t *s,ai_run_t *r)
{am_free(s,r);ai_graph_free(s,r->graph);s->bytes-=r->bytes;free(r->input);free(r->plan);free(r->request);free(r->targets_json);free(r->targets);free(r->reply);memset(r,0,sizeof(*r));}
static int ai_evict(ui_ai_service_t *s)
{int found=-1;uint64_t order=UINT64_MAX;for(int i=0;i<AI_RUNS;++i)if(s->runs[i].id[0]&&s->runs[i].terminal&&!s->runs[i].refs&&!ai_graph_retain(s->runs[i].graph)&&s->runs[i].order<order){order=s->runs[i].order;found=i;}if(found>=0)ai_run_free(s,&s->runs[found]);return found;}
static int ai_json_texts(yyjson_val *v)
{size_t i,n;yyjson_val *item;if(yyjson_is_str(v))return strlen(yyjson_get_str(v))==yyjson_get_len(v);if(yyjson_is_arr(v)){yyjson_arr_foreach(v,i,n,item)if(!ai_json_texts(item))return 0;}if(yyjson_is_obj(v)){yyjson_obj_iter it=yyjson_obj_iter_with(v);while((item=yyjson_obj_iter_next(&it)))if(strlen(yyjson_get_str(item))!=yyjson_get_len(item)||!ai_json_texts(yyjson_obj_iter_get_val(item)))return 0;}return 1;}
static int ai_plan_valid(const char *plan)
{
 yyjson_doc *d=yyjson_read(plan,strlen(plan),0);yyjson_val *root=d?yyjson_doc_get_root(d):NULL,*steps=root?yyjson_obj_get(root,"steps"):NULL,*step;size_t i,n;int ok=root&&yyjson_is_obj(root)&&yyjson_obj_size(root)==1&&yyjson_is_arr(steps)&&yyjson_arr_size(steps)<=64&&ai_json_texts(root);
 if(ok){yyjson_arr_foreach(steps,i,n,step){const char *type=yyjson_get_str(yyjson_obj_get(step,"type"));if(!yyjson_is_obj(step)||!type||(strcmp(type,"text")&&strcmp(type,"discover")&&strcmp(type,"tool")&&strcmp(type,"ask")&&strcmp(type,"fail"))){ok=0;break;}}}
 yyjson_doc_free(d);return ok;
}
static char *ai_targets_json(const ui_ai_task_request_t *request)
{
 yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *a=d?yyjson_mut_arr(d):NULL;char *p;if(!a){yyjson_mut_doc_free(d);return NULL;}yyjson_mut_doc_set_root(d,a);
 for(size_t i=0;i<request->target_count;++i){const ui_cap_target_t *t=&request->targets[i];yyjson_mut_val *o=yyjson_mut_obj(d);char id[32];snprintf(id,sizeof(id),"%llu",(unsigned long long)t->instance_id);
  yyjson_mut_obj_add_strcpy(d,o,"instance_id",id);yyjson_mut_obj_add_uint(d,o,"kind",t->kind);yyjson_mut_obj_add_strcpy(d,o,"host_epoch",t->host_epoch);yyjson_mut_obj_add_strcpy(d,o,"resource_id",t->resource_id);yyjson_mut_obj_add_strcpy(d,o,"open_epoch",t->open_epoch);yyjson_mut_obj_add_strcpy(d,o,"revision",t->expected_revision);yyjson_mut_obj_add_strcpy(d,o,"scope",t->scope_json);yyjson_mut_arr_add_val(a,o);}
 p=yyjson_mut_write(d,0,NULL);yyjson_mut_doc_free(d);return p;
}
static ui_status_t ai_targets_check(ui_ai_service_t *s,const ui_ai_task_request_t *q,int fresh_required)
{
 for(size_t i=0;i<q->target_count;++i){const ui_cap_target_t *t=&q->targets[i];ui_app_instance_info_t app={0};ui_cap_target_t fresh={0};ui_status_t status;
  if(t->size<sizeof(*t)||t->version!=1||!memchr(t->host_epoch,0,37)||!memchr(t->resource_id,0,256)||!memchr(t->open_epoch,0,37)||!memchr(t->expected_revision,0,128)||!memchr(t->scope_json,0,sizeof(t->scope_json)))return UI_STATUS_INVALID_ARGUMENT;
  if(strcmp(t->scope_json,"{\"access\":\"read\"}"))return UI_STATUS_PERMISSION_DENIED;
  /* A persisted duplicate may carry a previous process's target. Resolve its
   * request identity before any execution. New tools still recheck P2 gates. */
  if(!fresh_required)continue;
  app.size=sizeof(app);status=ui_workspace_get_instance(s->config.workspace,t->instance_id,&app);if(status)return status;if(app.closing)return UI_STATUS_CANCELLED;
  fresh.size=sizeof(fresh);fresh.version=1;status=ui_capability_bind(app.host,t->resource_id,&fresh);if(status)return status;
  if(t->kind!=fresh.kind||strcmp(t->host_epoch,fresh.host_epoch)||strcmp(t->open_epoch,fresh.open_epoch)||strcmp(t->expected_revision,fresh.expected_revision)||strcmp(t->scope_json,fresh.scope_json))return UI_STATUS_VALIDATION_FAILED;
 }return UI_STATUS_OK;
}
static ui_status_t ai_submit_impl(ui_ai_service_t *s,const ui_ai_task_request_t *q,const char *parent,char out[65],ai_graph_t *graph,am_run_t *model,int *model_owned)
{
 char *plan=NULL,*targets=NULL,*request=NULL,hash[65],id[65];size_t length,bytes;ui_status_t status;yyjson_mut_doc *d=NULL;yyjson_mut_val *o;int slot=-1;ai_run_t *r;ai_notice_t receipt={0};
 if(s&&ab_restrict_submit(s,model))return UI_STATUS_PERMISSION_DENIED;if(model_owned)*model_owned=0;if(out)out[0]=0;if(!ai_ui(s)||!q||q->size<sizeof(*q)||q->version!=1||!out||!ai_id(q->client_request_id)||!ai_text(q->input_utf8,UI_AI_MAX_INPUT,0)||q->target_count>16||(q->target_count&&!q->targets)||!ai_text(q->stub_plan_json,UI_AI_MAX_PLAN,1))return UI_STATUS_INVALID_ARGUMENT;
 EnterCriticalSection(&s->lock);if(s->phase!=UI_AI_READY){LeaveCriticalSection(&s->lock);return UI_STATUS_CANCELLED;}if(s->callbacks){LeaveCriticalSection(&s->lock);return UI_STATUS_LIMIT_EXCEEDED;}LeaveCriticalSection(&s->lock);
 if(s->edit&&(s->edit->job||s->edit->busy||(s->edit->id[0]&&s->edit->phase!=AE_DONE&&s->edit->phase!=AE_UNKNOWN&&s->edit->phase!=AE_IDLE)))return UI_STATUS_LIMIT_EXCEEDED;
 status=ai_targets_check(s,q,parent!=NULL);if(status)return status;
 EnterCriticalSection(&s->lock);if(s->phase!=UI_AI_READY){LeaveCriticalSection(&s->lock);return UI_STATUS_CANCELLED;}LeaveCriticalSection(&s->lock);
 if(s->edit&&(s->edit->job||s->edit->busy||(s->edit->id[0]&&s->edit->phase!=AE_DONE&&s->edit->phase!=AE_UNKNOWN&&s->edit->phase!=AE_IDLE)))return UI_STATUS_LIMIT_EXCEEDED;
 status=ui_cap_json_canonicalize(q->stub_plan_json&&*q->stub_plan_json?q->stub_plan_json:"{\"steps\":[]}",q->stub_plan_json&&*q->stub_plan_json?strlen(q->stub_plan_json):12,&plan,&length);if(status)return status;
 if(!graph&&!model&&!ai_plan_valid(plan)){ui_capability_release(plan);return UI_STATUS_VALIDATION_FAILED;}if(model&&model->assistant){ai_graph_t fixed={0};fixed.targets=model->assistant->targets;fixed.target_count=model->assistant->target_count;targets=ai_graph_targets_json(&fixed);}else targets=graph?ai_graph_targets_json(graph):ai_targets_json(q);if(!targets){ui_capability_release(plan);return UI_STATUS_OUT_OF_MEMORY;}
 d=yyjson_mut_doc_new(NULL);o=d?yyjson_mut_obj(d):NULL;if(!o){status=UI_STATUS_OUT_OF_MEMORY;goto done;}yyjson_mut_doc_set_root(d,o);
 yyjson_mut_obj_add_strcpy(d,o,"input",q->input_utf8);yyjson_mut_obj_add_strcpy(d,o,"plan",plan);yyjson_mut_obj_add_strcpy(d,o,"targets",targets);yyjson_mut_obj_add_strcpy(d,o,"parent",parent?parent:"");request=yyjson_mut_write(d,0,NULL);yyjson_mut_doc_free(d);d=NULL;
 if(!request){status=UI_STATUS_OUT_OF_MEMORY;goto done;}if(strlen(request)>AI_CONTEXT){status=UI_STATUS_LIMIT_EXCEEDED;goto done;}status=graph?ai_graph_request_hash(q->input_utf8,plan,targets,parent,hash):ui_cap_json_fingerprint(request,strlen(request),hash);if(status)goto done;
 d=yyjson_mut_doc_new(NULL);o=d?yyjson_mut_obj(d):NULL;if(!o){status=UI_STATUS_OUT_OF_MEMORY;goto done;}yyjson_mut_doc_set_root(d,o);yyjson_mut_obj_add_strcpy(d,o,"conversation",s->conversation);yyjson_mut_obj_add_strcpy(d,o,"client",q->client_request_id);
 {char *key=yyjson_mut_write(d,0,NULL);if(!key){status=UI_STATUS_OUT_OF_MEMORY;goto done;}status=ui_cap_json_fingerprint(key,strlen(key),id);free(key);if(status)goto done;}
 bytes=(model?sizeof(*model):0)+(model&&model->assistant?model->assistant->bytes:0)+strlen(q->input_utf8)+1+strlen(plan)+1+strlen(targets)+1+strlen(request)+1+q->target_count*sizeof(ui_cap_target_t)+UI_AI_MAX_REPLY+1;
 EnterCriticalSection(&s->lock);
 slot=ai_find(s,id);if(slot>=0){r=&s->runs[slot];status=strcmp(r->hash,hash)?UI_STATUS_ALREADY_EXISTS:UI_STATUS_OK;if(!status)strcpy_s(out,65,id);LeaveCriticalSection(&s->lock);goto done;}
 if(s->phase!=UI_AI_READY){LeaveCriticalSection(&s->lock);status=UI_STATUS_CANCELLED;goto done;}
 if(s->queued>=UI_AI_MAX_QUEUED||s->cmd_count==AI_COMMANDS){LeaveCriticalSection(&s->lock);status=UI_STATUS_LIMIT_EXCEEDED;goto done;}
 while(bytes>UI_AI_MAX_BYTES-s->bytes)if(ai_evict(s)<0){LeaveCriticalSection(&s->lock);status=UI_STATUS_LIMIT_EXCEEDED;goto done;}
 for(int i=0;i<AI_RUNS;++i)if(!s->runs[i].id[0]){slot=i;break;}if(slot<0)slot=ai_evict(s);if(slot<0){LeaveCriticalSection(&s->lock);status=UI_STATUS_LIMIT_EXCEEDED;goto done;}
 r=&s->runs[slot];memset(r,0,sizeof(*r));r->input=ai_copy(q->input_utf8);r->plan=ai_copy(plan);r->request=ai_copy(request);r->targets_json=ai_copy(targets);r->reply=calloc(1,UI_AI_MAX_REPLY+1);r->targets=q->target_count?malloc(q->target_count*sizeof(ui_cap_target_t)):NULL;r->bytes=bytes;s->bytes+=bytes;
 if(!r->input||!r->plan||!r->request||!r->targets_json||!r->reply||(q->target_count&&!r->targets)){ai_run_free(s,r);LeaveCriticalSection(&s->lock);status=UI_STATUS_OUT_OF_MEMORY;goto done;}
 strcpy_s(r->id,sizeof(r->id),id);strcpy_s(r->client,sizeof(r->client),q->client_request_id);strcpy_s(r->hash,sizeof(r->hash),hash);if(parent)strcpy_s(r->parent,sizeof(r->parent),parent);r->target_count=q->target_count;if(r->target_count)memcpy(r->targets,q->targets,r->target_count*sizeof(*r->targets));r->phase=UI_AI_RECEIVED;r->order=++s->order;r->graph=graph;r->model=model;if(model){char bound[220];snprintf(bound,sizeof(bound),"%s:%s:%s",s->session,id,hash);ui_cap_hash_bytes(bound,strlen(bound),model->binding);if(model_owned)*model_owned=1;}
 {ai_command_t *c=&s->commands[(s->cmd_head+s->cmd_count)%AI_COMMANDS];memset(c,0,sizeof(*c));c->kind=0;c->run=slot;++s->cmd_count;++s->queued;}
 strcpy_s(out,65,id);status=ai_start_locked(s)?UI_STATUS_OK:UI_STATUS_PLATFORM_ERROR;
 strcpy_s(receipt.run,sizeof(receipt.run),id);strcpy_s(receipt.client,sizeof(receipt.client),r->client);strcpy_s(receipt.conversation,sizeof(receipt.conversation),s->conversation);strcpy_s(receipt.type,sizeof(receipt.type),"received");receipt.payload="{}";receipt.event.size=sizeof(receipt.event);receipt.event.version=1;
 LeaveCriticalSection(&s->lock);ai_notify(s,&receipt);
done:yyjson_mut_doc_free(d);ui_capability_release(plan);free(targets);free(request);return status;
}
static ui_status_t ai_submit(ui_ai_service_t *s,const ui_ai_task_request_t *q,const char *parent,char out[65])
{
 ui_status_t status;if(!ai_ui(s)){if(out)out[0]=0;return UI_STATUS_INVALID_ARGUMENT;}
 /* Covers application callbacks during fresh binding, not just on_event.
  * Reentrant controls/poll remain legal; destroy must wait for this frame. */
 ++s->methods;status=ai_submit_impl(s,q,parent,out,NULL,NULL,NULL);--s->methods;return status;
}
ui_status_t ui_ai_submit(ui_ai_service_t *s,const ui_ai_task_request_t *q,char out[65]){return ai_submit(s,q,NULL,out);}
ui_status_t ui_ai_continue(ui_ai_service_t *s,const char *old,const ui_ai_task_request_t *q,char out[65])
{if(!ai_id(old))return UI_STATUS_INVALID_ARGUMENT;return ai_submit(s,q,old,out);}
ui_status_t ui_ai_service_create(const ui_ai_service_config_t *c,ui_ai_service_t **out)
{
 ui_ai_service_t *s;ui_status_t status;if(out)*out=NULL;if(!c||c->size<sizeof(*c)||c->version!=1||!c->workspace||!out||!ai_text(c->storage_path_utf8,4095,0))return UI_STATUS_INVALID_ARGUMENT;
 s=calloc(1,sizeof(*s));if(!s)return UI_STATUS_OUT_OF_MEMORY;s->config=*c;s->thread=GetCurrentThreadId();s->path=ai_copy(c->storage_path_utf8);s->control=calloc(1,AI_CONTROL);s->active=s->answer_owner=-1;s->bytes=sizeof(*s)+strlen(c->storage_path_utf8)+1;InitializeCriticalSection(&s->lock);InitializeConditionVariable(&s->changed);
 if(!s->path||!s->control||!ai_uuid(s->session)){free(s->path);free(s->control);DeleteCriticalSection(&s->lock);free(s);return UI_STATUS_OUT_OF_MEMORY;}
 status=ui_workspace_ai_claim(c->workspace,s);if(status){free(s->path);free(s->control);DeleteCriticalSection(&s->lock);free(s);return status;}*out=s;return UI_STATUS_OK;
}
ui_status_t ui_ai_service_open(ui_ai_service_t *s)
{ui_status_t status;if(!ai_ui(s))return UI_STATUS_INVALID_ARGUMENT;EnterCriticalSection(&s->lock);if(s->phase!=UI_AI_CREATED){LeaveCriticalSection(&s->lock);return UI_STATUS_ALREADY_EXISTS;}s->phase=UI_AI_OPENING;status=ai_start_locked(s)?UI_STATUS_OK:UI_STATUS_PLATFORM_ERROR;LeaveCriticalSection(&s->lock);return status;}
ui_status_t ui_ai_service_get_state(ui_ai_service_t *s,ui_ai_service_state_t *out)
{if(!ai_ui(s)||!out||out->size<sizeof(*out)||out->version!=1)return UI_STATUS_INVALID_ARGUMENT;EnterCriticalSection(&s->lock);out->phase=s->phase;out->queued=s->queued;out->worker_alive=s->worker&&WaitForSingleObject(s->worker,0)!=WAIT_OBJECT_0;out->bytes=s->bytes;out->event_bytes=s->event_bytes;strcpy_s(out->conversation_id,37,s->conversation);strcpy_s(out->error,256,s->error);out->active_run_id[0]=0;if(s->active>=0)strcpy_s(out->active_run_id,65,s->runs[s->active].id);LeaveCriticalSection(&s->lock);return UI_STATUS_OK;}
ui_status_t ui_ai_get_run(ui_ai_service_t *s,const char *id,char **out)
{int at;if(out)*out=NULL;if(!ai_ui(s)||!ai_id(id)||!out)return UI_STATUS_INVALID_ARGUMENT;EnterCriticalSection(&s->lock);at=ai_find(s,id);if(at>=0)*out=ai_run_json(&s->runs[at]);LeaveCriticalSection(&s->lock);return at<0?UI_STATUS_NOT_FOUND:*out?UI_STATUS_OK:UI_STATUS_OUT_OF_MEMORY;}
static ui_status_t ai_command(ui_ai_service_t *s,const char *query,const char *run,ui_ai_history_kind_t kind,uint64_t after,size_t limit,int command)
{
 ai_command_t *c;if(!ai_ui(s)||!ai_id(query)||(run&&!ai_id(run))||kind<UI_AI_MESSAGES||kind>UI_AI_EVENTS||!limit||limit>32||after>INT64_MAX)return UI_STATUS_INVALID_ARGUMENT;
 EnterCriticalSection(&s->lock);if(s->phase!=UI_AI_READY&&s->phase!=UI_AI_QUIESCING){LeaveCriticalSection(&s->lock);return UI_STATUS_CANCELLED;}if(s->callbacks||s->cmd_count==AI_COMMANDS){LeaveCriticalSection(&s->lock);return UI_STATUS_LIMIT_EXCEEDED;}
 c=&s->commands[(s->cmd_head+s->cmd_count)%AI_COMMANDS];memset(c,0,sizeof(*c));c->kind=command;strcpy_s(c->query,sizeof(c->query),query);if(run)strcpy_s(c->id,sizeof(c->id),run);c->history=kind;c->after=after;c->limit=limit;++s->cmd_count;(void)ai_start_locked(s);LeaveCriticalSection(&s->lock);return UI_STATUS_OK;
}
ui_status_t ui_ai_history(ui_ai_service_t *s,const char *query,ui_ai_history_kind_t kind,uint64_t after,size_t limit){return ai_command(s,query,NULL,kind,after,limit,1);}
ui_status_t ui_ai_query_run(ui_ai_service_t *s,const char *query,const char *id){return ai_command(s,query,id,UI_AI_RUNS,0,1,2);}
ui_status_t ui_ai_cancel(ui_ai_service_t *s,const char *id)
{int at;if(!ai_ui(s)||!ai_id(id))return UI_STATUS_INVALID_ARGUMENT;EnterCriticalSection(&s->lock);at=ai_find(s,id);if(at<0){LeaveCriticalSection(&s->lock);return UI_STATUS_NOT_FOUND;}if(!s->runs[at].terminal&&!s->runs[at].cancel){s->runs[at].cancel=1;s->runs[at].cancel_at=ai_now(s);s->runs[at].phase=UI_AI_CANCEL_REQUESTED;s->runs[at].committed=0;ai_graph_cancelled(s,s->runs[at].graph);}(void)ai_start_locked(s);LeaveCriticalSection(&s->lock);return UI_STATUS_OK;}
ui_status_t ui_ai_answer(ui_ai_service_t *s,const char *id,const char *question,const char *operation,const char *answer)
{
 ai_run_t *r;int at;char hash[65];yyjson_mut_doc *d;yyjson_mut_val *o;char *text;if(!ai_ui(s)||!ai_id(id)||!ai_id(question)||!ai_id(operation)||!ai_text(answer,UI_AI_MAX_INPUT,1))return UI_STATUS_INVALID_ARGUMENT;
 d=yyjson_mut_doc_new(NULL);o=d?yyjson_mut_strcpy(d,answer):NULL;if(!o){yyjson_mut_doc_free(d);return UI_STATUS_OUT_OF_MEMORY;}yyjson_mut_doc_set_root(d,o);text=yyjson_mut_write(d,0,NULL);yyjson_mut_doc_free(d);if(!text)return UI_STATUS_OUT_OF_MEMORY;{ui_status_t status=ui_cap_json_fingerprint(text,strlen(text),hash);free(text);if(status)return status;}
 EnterCriticalSection(&s->lock);at=ai_find(s,id);if(at<0){LeaveCriticalSection(&s->lock);return UI_STATUS_NOT_FOUND;}r=&s->runs[at];
 if(strcmp(r->question,question)||r->terminal||r->cancel){LeaveCriticalSection(&s->lock);return UI_STATUS_NOT_FOUND;}
 if(r->answer_op[0]&&!strcmp(r->answer_op,operation)){ui_status_t st=!strcmp(r->answer_hash,hash)?UI_STATUS_OK:UI_STATUS_ALREADY_EXISTS;LeaveCriticalSection(&s->lock);return st;}
 if(r->phase!=UI_AI_WAITING_INPUT||r->answer_ready||s->answer_owner>=0){LeaveCriticalSection(&s->lock);return UI_STATUS_LIMIT_EXCEEDED;}
 s->answer_owner=at;r->answer=(char *)s->control;strcpy_s(r->answer,UI_AI_MAX_INPUT+1,answer);strcpy_s(r->answer_op,65,operation);strcpy_s(r->answer_hash,65,hash);r->answer_ready=1;(void)ai_start_locked(s);LeaveCriticalSection(&s->lock);return UI_STATUS_OK;
}
ui_status_t ui_ai_service_quiesce(ui_ai_service_t *s)
{if(!ai_ui(s))return UI_STATUS_INVALID_ARGUMENT;ai_edit_quiesce(s);EnterCriticalSection(&s->lock);if(s->phase==UI_AI_CLOSED||s->close_requested||s->phase==UI_AI_CREATED){LeaveCriticalSection(&s->lock);return UI_STATUS_CANCELLED;}if(s->phase!=UI_AI_UNAVAILABLE)s->phase=UI_AI_QUIESCING;for(int i=0;i<AI_RUNS;++i)if(s->runs[i].id[0]&&!s->runs[i].terminal&&s->runs[i].phase!=UI_AI_NEEDS_REVIEW){s->runs[i].cancel=1;if(!s->runs[i].cancel_at)s->runs[i].cancel_at=ai_now(s);ai_graph_cancelled(s,s->runs[i].graph);}(void)ai_start_locked(s);LeaveCriticalSection(&s->lock);return UI_STATUS_OK;}
ui_status_t ui_ai_service_resume(ui_ai_service_t *s)
{if(!ai_ui(s))return UI_STATUS_INVALID_ARGUMENT;EnterCriticalSection(&s->lock);if(s->phase!=UI_AI_QUIESCING||s->close_requested){LeaveCriticalSection(&s->lock);return UI_STATUS_CANCELLED;}s->phase=s->opened==1?UI_AI_READY:UI_AI_OPENING;LeaveCriticalSection(&s->lock);return UI_STATUS_OK;}
ui_status_t ui_ai_service_request_close(ui_ai_service_t *s)
{if(!ai_ui(s))return UI_STATUS_INVALID_ARGUMENT;(void)ui_ai_service_quiesce(s);EnterCriticalSection(&s->lock);if(s->phase==UI_AI_CLOSED){LeaveCriticalSection(&s->lock);return UI_STATUS_OK;}s->close_requested=1;s->phase=UI_AI_CLOSING;(void)ai_start_locked(s);LeaveCriticalSection(&s->lock);return UI_STATUS_OK;}
ui_status_t ui_ai_service_destroy(ui_ai_service_t *s)
{
 if(!ai_ui(s))return UI_STATUS_INVALID_ARGUMENT;EnterCriticalSection(&s->lock);
 if(s->phase!=UI_AI_CLOSED||s->polling||s->callbacks||s->methods||s->tool.state||(s->worker&&WaitForSingleObject(s->worker,0)!=WAIT_OBJECT_0)||s->event_count){LeaveCriticalSection(&s->lock);return UI_STATUS_PENDING;}
 if(s->worker)CloseHandle(s->worker);for(int i=0;i<AI_RUNS;++i)if(s->runs[i].id[0])ai_run_free(s,&s->runs[i]);LeaveCriticalSection(&s->lock);
 ai_edit_dispose(s);ai_graph_dispose(s);av_dispose(s);am_dispose(s);free(s->assistant_policy);ui_workspace_ai_release(s->config.workspace,s);free(s->path);free(s->control);DeleteCriticalSection(&s->lock);free(s);return UI_STATUS_OK;
}
void ui_ai_release(void *p){free(p);}

static void ai_finish(ui_ai_service_t *s,ai_run_t *r,const char *outcome,const char *error)
{
 uint64_t sequence=0;char *json;
 EnterCriticalSection(&s->lock);strcpy_s(r->outcome,sizeof(r->outcome),outcome);if(error)strcpy_s(r->error,sizeof(r->error),error);{ai_run_t final=*r;final.phase=UI_AI_FINISHED;final.terminal=1;final.committed=1;json=ai_run_json(&final);}LeaveCriticalSection(&s->lock);
 if(!json){ai_store_error(s,r,"final allocation");return;}
 if(!ai_change(s,r,UI_AI_FINISHED,outcome,"final","finished",json,"assistant",r->reply,&sequence)){free(json);ai_store_error(s,r,"final commit");return;}
 free(json);EnterCriticalSection(&s->lock);r->phase=UI_AI_FINISHED;r->terminal=1;r->committed=1;r->question_text=NULL;if(s->answer_owner==(int)(r-s->runs)){s->answer_owner=-1;r->answer=NULL;r->answer_ready=0;}json=ai_run_json(r);s->active=-1;LeaveCriticalSection(&s->lock);
 if(json){(void)ai_emit(s,"finished",r,NULL,sequence,1,json,0);free(json);}
}
static int ai_append(ui_ai_service_t *s,ai_run_t *r,const char *text)
{
 size_t n=strlen(text),at=0;char *snapshot;
 /* Provider transport deliberately splits inside UTF-8. Only complete prefixes
  * enter the display cache; no malformed partial is sent to the UI. */
 while(at<n){size_t end=at+3;if(end>n)end=n;while(end<n&&((unsigned char)text[end]&0xc0)==0x80)++end;
  EnterCriticalSection(&s->lock);if(r->cancel){LeaveCriticalSection(&s->lock);return 1;}if(end-at>UI_AI_MAX_REPLY-r->reply_length){LeaveCriticalSection(&s->lock);return 0;}
  memcpy(r->reply+r->reply_length,text+at,end-at);r->reply_length+=end-at;r->reply[r->reply_length]=0;snapshot=ai_copy(r->reply);LeaveCriticalSection(&s->lock);
  if(!snapshot)return 0;(void)ai_emit(s,"delta",r,NULL,0,0,snapshot,1);free(snapshot);at=end;
 }return 1;
}
static int ai_tool_begin(ui_ai_service_t *s,ai_run_t *r,yyjson_val *step,int discover)
{
 ai_tool_t t={0};yyjson_val *v=yyjson_obj_get(step,"target");uint64_t sequence=0;const char *id=yyjson_get_str(yyjson_obj_get(step,"tool_call_id"));char generated[65];
 if(!yyjson_is_uint(v)||yyjson_get_uint(v)>=r->target_count)return 0;t.target_index=(unsigned)yyjson_get_uint(v);t.target=r->targets[t.target_index];t.run=(int)(r-s->runs);t.discover=discover;
 if(discover){const char *category=yyjson_get_str(yyjson_obj_get(step,"category"));snprintf(generated,sizeof(generated),"discover-%u",r->step);id=generated;if(category&&!ai_text(category,127,1))return 0;if(category)strcpy_s(t.category,sizeof(t.category),category);strcpy_s(t.capability,sizeof(t.capability),"read-directory");t.params=ai_copy("{}");}
 else{const char *capability=yyjson_get_str(yyjson_obj_get(step,"capability_id"));yyjson_val *fragments=yyjson_obj_get(step,"fragments"),*argument=yyjson_obj_get(step,"arguments");char *raw=NULL,*canonical=NULL;size_t length;
  if(!ai_text(capability,255,0))return 0;strcpy_s(t.capability,sizeof(t.capability),capability);
  if(fragments){size_t at,n,used=0;yyjson_val *piece;if(!yyjson_is_arr(fragments)||yyjson_arr_size(fragments)>512||!yyjson_is_true(yyjson_obj_get(step,"complete")))return 0;raw=calloc(1,UI_CAPABILITY_INPUT_BYTES+1);if(!raw)return 0;
   yyjson_arr_foreach(fragments,at,n,piece){size_t bytes=yyjson_get_len(piece);if(!yyjson_is_str(piece)||bytes>UI_CAPABILITY_INPUT_BYTES-used){free(raw);return 0;}memcpy(raw+used,yyjson_get_str(piece),bytes);used+=bytes;}raw[used]=0;
  }else{if(!argument)return 0;raw=yyjson_val_write(argument,0,NULL);}
  if(!raw)return 0;if(ui_cap_json_canonicalize(raw,strlen(raw),&canonical,&length)!=UI_STATUS_OK||length>UI_CAPABILITY_INPUT_BYTES){free(raw);ui_capability_release(canonical);return 0;}free(raw);t.params=ai_copy(canonical);ui_capability_release(canonical);
 }
 if(!ai_id(id)||!t.params){free(t.params);return 0;}for(unsigned i=0;i<r->tool_id_count;++i)if(!strcmp(r->tool_ids[i],id)){free(t.params);return 0;}
 if(r->tool_id_count==64||r->tool_calls==64){free(t.params);return 0;}strcpy_s(t.id,sizeof(t.id),id);
 {ui_ai_task_request_t q={0};q.targets=&t.target;q.target_count=1;t.target_json=ai_targets_json(&q);}if(!t.target_json){free(t.params);return 0;}
 if(!ai_store_tool(s,r,&t,0,&sequence)){free(t.params);free(t.target_json);ai_store_error(s,r,"tool request");return -1;}
 if(!ai_store_available(s)){free(t.params);free(t.target_json);return -1;}
 EnterCriticalSection(&s->lock);strcpy_s(r->tool_ids[r->tool_id_count++],65,id);++r->tool_calls;r->phase=UI_AI_WAITING_TOOL;t.state=AI_TOOL_PREPARE;s->tool=t;LeaveCriticalSection(&s->lock);
 (void)ai_emit(s,"tool_requested",r,NULL,sequence,1,"{}",0);return 1;
}
static int ai_provider_tick(ui_ai_service_t *s,ai_run_t *r)
{
 int cancel,stalled,hold,tool;ui_ai_run_phase_t phase;uint64_t sequence=0;char *json;yyjson_doc *plan;yyjson_val *steps,*step;const char *type;
 EnterCriticalSection(&s->lock);cancel=r->cancel;stalled=r->stalled;hold=s->fixture_hold;tool=s->tool.state;phase=r->phase;LeaveCriticalSection(&s->lock);
 if(cancel&&!r->cancel_recorded){if(!ai_change(s,r,UI_AI_CANCEL_REQUESTED,"","cancel_requested","cancel_requested","{}",NULL,NULL,&sequence)){ai_store_error(s,r,"cancel");return 0;}EnterCriticalSection(&s->lock);r->cancel_recorded=1;r->committed=1;LeaveCriticalSection(&s->lock);(void)ai_emit(s,"cancel_requested",r,NULL,sequence,1,"{}",0);}
 if(stalled==1){if(!ai_change(s,r,UI_AI_CANCELLATION_STALLED,"","cancel_stalled","cancellation_stalled","{}",NULL,NULL,&sequence)){ai_store_error(s,r,"cancel stalled");return 0;}EnterCriticalSection(&s->lock);r->stalled=2;r->phase=UI_AI_CANCELLATION_STALLED;r->committed=1;LeaveCriticalSection(&s->lock);(void)ai_emit(s,"cancellation_stalled",r,NULL,sequence,1,"{}",0);}
 if(!ai_store_available(s)||hold)return 0;
 if(tool){
  if(s->tool.run!=(int)(r-s->runs))return 0;
  if(tool==AI_TOOL_PREPARED){char binding[192];if(!ai_store_tool(s,r,&s->tool,1,&sequence)){ai_store_error(s,r,"tool binding");return 0;}if(!ai_store_available(s))return 0;snprintf(binding,sizeof(binding),"{\"tool_call_id\":\"%s\",\"call_id\":\"%llu\"}",s->tool.id,(unsigned long long)s->tool.call);EnterCriticalSection(&s->lock);s->tool.state=AI_TOOL_DISPATCH;LeaveCriticalSection(&s->lock);(void)ai_emit(s,"tool_bound",r,NULL,sequence,1,binding,0);return 0;}
  if(tool!=AI_TOOL_RESULT)return 0;
  if(!ai_store_tool(s,r,&s->tool,2,&sequence)){ai_store_error(s,r,"tool result");return 0;}
  if(!ai_store_available(s))return 0;
  {ui_status_t status=s->tool.status;char *result=s->tool.result;free(s->tool.params);free(s->tool.target_json);EnterCriticalSection(&s->lock);memset(&s->tool,0,sizeof(s->tool));LeaveCriticalSection(&s->lock);
   if(!cancel&&status==UI_STATUS_OK&&result&&!ai_append(s,r,result)){free(result);ai_finish(s,r,"limit","reply_limit");return 1;}free(result);
   if(status!=UI_STATUS_OK&&!cancel){ai_finish(s,r,"failed","read_tool_rejected");return 1;}}
  if(r->model_calls>=16){ai_finish(s,r,"limit","model_cycle_limit");return 1;}EnterCriticalSection(&s->lock);++r->model_calls;++r->step;r->phase=UI_AI_RUNNING;phase=UI_AI_RUNNING;LeaveCriticalSection(&s->lock);
 }
 if(cancel){ai_finish(s,r,"cancelled",NULL);return 1;}
 if(phase==UI_AI_WAITING_INPUT){
  int ready;EnterCriticalSection(&s->lock);ready=r->answer_ready;LeaveCriticalSection(&s->lock);if(!ready)return 0;
  {char op[160];snprintf(op,sizeof(op),"answer:%s:%s",r->question,r->answer_op);if(!ai_change(s,r,UI_AI_RUNNING,"",op,"answer","{}","user",r->answer,&sequence)){ai_store_error(s,r,"answer");return 0;}
   if(!ai_store_available(s))return 0;if(!ai_append(s,r,r->answer)){ai_finish(s,r,"limit","reply_limit");return 1;}EnterCriticalSection(&s->lock);r->answer_ready=0;r->answer=NULL;s->answer_owner=-1;r->phase=UI_AI_RUNNING;LeaveCriticalSection(&s->lock);}
  if(r->model_calls>=16){ai_finish(s,r,"limit","model_cycle_limit");return 1;}EnterCriticalSection(&s->lock);++r->step;++r->model_calls;LeaveCriticalSection(&s->lock);
 }
 if(r->model_calls==0){
  if(!ai_change(s,r,UI_AI_RUNNING,"","running","running","{}",NULL,NULL,&sequence)){ai_store_error(s,r,"running");return 0;}EnterCriticalSection(&s->lock);r->phase=UI_AI_RUNNING;r->model_calls=1;LeaveCriticalSection(&s->lock);(void)ai_emit(s,"running",r,NULL,sequence,1,"{}",0);
 }
 if(!ai_store_available(s))return 0;
 plan=yyjson_read(r->plan,strlen(r->plan),0);steps=plan?yyjson_obj_get(yyjson_doc_get_root(plan),"steps"):NULL;
 if(!steps){yyjson_doc_free(plan);ai_finish(s,r,"failed","invalid_stub_plan");return 1;}
 if(yyjson_arr_size(steps)==0){int ok=ai_append(s,r,"[Stub test model] ")&&ai_append(s,r,r->input);yyjson_doc_free(plan);if(ok)ai_finish(s,r,ai_cancelled(s,r)?"cancelled":"completed",NULL);else ai_finish(s,r,"limit","reply_limit");return 1;}
 if(r->step>=yyjson_arr_size(steps)){yyjson_doc_free(plan);ai_finish(s,r,"completed",NULL);return 1;}
 if(r->steps>=64){yyjson_doc_free(plan);ai_finish(s,r,"limit","step_limit");return 1;}EnterCriticalSection(&s->lock);++r->steps;LeaveCriticalSection(&s->lock);
 step=yyjson_arr_get(steps,r->step);type=yyjson_get_str(yyjson_obj_get(step,"type"));
 if(!strcmp(type,"text")){const char *text=yyjson_get_str(yyjson_obj_get(step,"text"));int ok=text&&ai_append(s,r,text);++r->step;yyjson_doc_free(plan);if(!ok)ai_finish(s,r,"failed","invalid_or_large_provider_output");return 1;}
 if(!strcmp(type,"fail")){yyjson_doc_free(plan);ai_finish(s,r,"failed","stub_disconnect");return 1;}
 if(!strcmp(type,"ask")){
  const char *question=yyjson_get_str(yyjson_obj_get(step,"text"));char suffix[96];if(!ai_text(question,UI_AI_MAX_INPUT,0)){yyjson_doc_free(plan);ai_finish(s,r,"failed","invalid_question");return 1;}
  EnterCriticalSection(&s->lock);snprintf(r->question,sizeof(r->question),"question-%u",r->step);r->answer_op[0]=0;r->question_text=(char *)s->control+UI_AI_MAX_INPUT+1;strcpy_s(r->question_text,UI_AI_MAX_INPUT+1,question);json=ai_run_json(r);LeaveCriticalSection(&s->lock);snprintf(suffix,sizeof(suffix),"question:%u",r->step);
  if(!json||!ai_change(s,r,UI_AI_WAITING_INPUT,"",suffix,"question",json,"assistant",question,&sequence)){free(json);yyjson_doc_free(plan);ai_store_error(s,r,"question");return 0;}
  free(json);EnterCriticalSection(&s->lock);r->phase=UI_AI_WAITING_INPUT;json=ai_run_json(r);LeaveCriticalSection(&s->lock);if(json){(void)ai_emit(s,"question",r,NULL,sequence,1,json,0);free(json);}yyjson_doc_free(plan);return 1;
 }
 {int ok=ai_tool_begin(s,r,step,!strcmp(type,"discover"));yyjson_doc_free(plan);if(ok==0)ai_finish(s,r,"failed","invalid_or_duplicate_tool_request");return ok!=0;}
}
static int am_live(ui_ai_service_t *);
#ifdef UI_AI_FIXTURE
static int ai_tool_fixture_paused(ui_ai_service_t *s,int state)
{EnterCriticalSection(&s->lock);int paused=state&&s->fixture_tool_pause==state;if(paused)s->fixture_tool_seen=state;LeaveCriticalSection(&s->lock);return paused;}
#endif
#include "ai_validation.inc"
#include "ai_model.inc"
#include "ai_validation_batch.inc"

static DWORD WINAPI ai_worker(void *data)
{
 ui_ai_service_t *s=data;int open,close;
 EnterCriticalSection(&s->lock);open=!s->opened;close=s->close_requested;LeaveCriticalSection(&s->lock);
 if(open&&!close){
  if(!ai_store_open(s)){char reason[256];EnterCriticalSection(&s->lock);strcpy_s(reason,sizeof(reason),s->error[0]?s->error:"open/schema/lock");LeaveCriticalSection(&s->lock);ai_store_error(s,NULL,reason);EnterCriticalSection(&s->lock);s->opened=-1;LeaveCriticalSection(&s->lock);return 0;}
  if(!bv_restore_guard(s)){
   ai_store_error(s,NULL,"business_guard_restore");
   EnterCriticalSection(&s->lock);s->opened=-1;LeaveCriticalSection(&s->lock);
   return 0;
  }
  {ai_command_t c={0};char *history;c.history=UI_AI_MESSAGES;c.limit=20;history=ai_store_page(s,&c);if(!history){ai_store_error(s,NULL,"history restore");return 0;}(void)ai_emit(s,"history",NULL,"open-history",0,1,history,0);free(history);}
  EnterCriticalSection(&s->lock);s->opened=1;if(s->phase==UI_AI_OPENING)s->phase=UI_AI_READY;LeaveCriticalSection(&s->lock);(void)ai_emit(s,"ready",NULL,NULL,0,1,"{}",0);
 }
 for(;;){
  ai_command_t command={0};int has=0,index=-1,progress=0,unavailable;
  progress=ai_edit_io(s)|am_config_io(s);
#ifdef UI_AI_FIXTURE
  EnterCriticalSection(&s->lock);if(s->fixture_worker_yield){s->fixture_worker_yield=0;LeaveCriticalSection(&s->lock);return 0;}LeaveCriticalSection(&s->lock);
#endif
  EnterCriticalSection(&s->lock);unavailable=s->store_failed;
  if(unavailable){for(int i=0;i<AI_RUNS;++i)if(s->runs[i].id[0]&&!s->runs[i].terminal&&s->runs[i].phase!=UI_AI_NEEDS_REVIEW){s->runs[i].phase=UI_AI_UNCONFIRMED;s->runs[i].committed=0;}s->cmd_count=0;s->queued=0;
    if(s->tool.state==AI_TOOL_RESULT){free(s->tool.params);free(s->tool.result);free(s->tool.target_json);memset(&s->tool,0,sizeof(s->tool));}if(!s->tool.state)s->active=-1;}
  if(s->cmd_count&&!unavailable){command=s->commands[s->cmd_head];s->cmd_head=(s->cmd_head+1)%AI_COMMANDS;--s->cmd_count;has=1;if(command.kind==0)++s->runs[command.run].refs;}
  LeaveCriticalSection(&s->lock);
  if(has){
   if(command.kind==0){ai_run_t *r=&s->runs[command.run];uint64_t sequence=0;int existing=0;char *json;
    if(!ai_store_accept(s,r,&existing,&sequence))ai_store_error(s,r,"accept");
    else{EnterCriticalSection(&s->lock);if(existing){if(r->model)r->model->recovered=1;if(r->graph){r->graph->record_only=1;r->graph->dirty=0;r->graph->version=r->graph->persisted=0;r->graph->confirmed=0;}if(s->queued)--s->queued;if(existing==-2){r->terminal=1;r->phase=UI_AI_FINISHED;strcpy_s(r->error,sizeof(r->error),r->graph?"continue_precondition_rejected":"missing_continue_parent");}}
      else{if(r->graph)r->graph->admitted=1;if(r->model)r->model->admitted=1;r->committed=1;r->phase=r->cancel?UI_AI_CANCEL_REQUESTED:UI_AI_QUEUED;}json=ai_run_json(r);LeaveCriticalSection(&s->lock);
      if(json){(void)ai_emit(s,existing<0?"rejected":existing?"existing":"accepted",r,NULL,sequence,existing==-2?0:1,json,0);free(json);}}
    EnterCriticalSection(&s->lock);--r->refs;LeaveCriticalSection(&s->lock);
   }else{char *json=command.kind==1?ai_store_page(s,&command):ai_store_query(s,command.id);(void)ai_emit(s,json?(command.kind==1?"history":"query"):"query_error",NULL,command.query,0,json?1:0,json?json:"{\"error\":\"not_found_or_query_limit\"}",0);free(json);}progress=1;
  }
  EnterCriticalSection(&s->lock);
  if(s->active<0&&!s->store_failed){uint64_t first=UINT64_MAX;for(int i=0;i<AI_RUNS;++i){ai_run_t *r=&s->runs[i];if(r->id[0]&&!r->terminal&&(r->committed||(r->model&&r->model->admitted&&!r->model->recovered&&r->cancel)||(r->graph&&r->graph->admitted&&!r->graph->record_only))&&r->phase!=UI_AI_NEEDS_REVIEW&&r->phase!=UI_AI_UNCONFIRMED&&r->order<first){index=i;first=r->order;}}
    if(index>=0){s->active=index;if(s->queued)--s->queued;}}
  index=s->active;if(index>=0&&!s->store_failed)++s->runs[index].refs;else index=-1;LeaveCriticalSection(&s->lock);
  if(index>=0){if(s->runs[index].model)progress|=am_tick(s,&s->runs[index]);else if(!s->runs[index].graph)progress|=ai_provider_tick(s,&s->runs[index]);EnterCriticalSection(&s->lock);--s->runs[index].refs;LeaveCriticalSection(&s->lock);}
  EnterCriticalSection(&s->lock);close=s->close_requested;unavailable=s->store_failed;
  if(close&&(s->active<0||unavailable)&&s->tool.state==AI_TOOL_NONE&&!s->cmd_count&&!ai_edit_pending(s)&&!am_pending(s)){LeaveCriticalSection(&s->lock);
    if(s->db){int rc;ar_crash(s,18);rc=sqlite3_wal_checkpoint_v2(s->db,NULL,SQLITE_CHECKPOINT_FULL,NULL,NULL);if(rc!=SQLITE_OK&&rc!=SQLITE_NOTFOUND)ai_store_error(s,NULL,"checkpoint");ar_crash(s,19);if(sqlite3_close(s->db)!=SQLITE_OK){ai_store_error(s,NULL,"close");return 0;}s->db=NULL;ar_crash(s,20);}
    EnterCriticalSection(&s->lock);s->phase=UI_AI_CLOSED;LeaveCriticalSection(&s->lock);(void)ai_emit(s,"closed",NULL,NULL,0,s->error[0]?0:1,"{}",0);return 0;}
  if(unavailable||(!progress&&!s->cmd_count)){LeaveCriticalSection(&s->lock);return 0;}LeaveCriticalSection(&s->lock);
 }
}

static void ai_tool_ui(ui_ai_service_t *s)
{
 ai_tool_t *t=&s->tool;ui_app_instance_info_t app={0};int state,cancel;ui_status_t status=UI_STATUS_OK;char *copy=NULL;
#ifdef UI_AI_FIXTURE
 if(ai_tool_fixture_paused(s,t->state))return;
#endif
 EnterCriticalSection(&s->lock);state=t->state;
 int handing_off=state&&s->runs[t->run].model&&s->runs[t->run].model->assistant&&s->runs[t->run].model->assistant->control!=AS_CONTROL_AI;
 cancel=state?(s->runs[t->run].cancel||s->store_failed||handing_off):0;
 if((s->store_failed||handing_off)&&state==AI_TOOL_PREPARED)state=AI_TOOL_DISPATCH;
 LeaveCriticalSection(&s->lock);if(!state||state==AI_TOOL_PREPARED||state==AI_TOOL_RESULT)return;
 app.size=sizeof(app);status=ui_workspace_get_instance(s->config.workspace,t->target.instance_id,&app);
 if(state==AI_TOOL_PREPARE){
  if(status==UI_STATUS_OK&&(app.closing||cancel))status=UI_STATUS_CANCELLED;
  if(status==UI_STATUS_OK&&t->discover){ui_cap_target_t current={0};current.size=sizeof(current);current.version=1;status=ui_capability_bind(app.host,t->target.resource_id,&current);
   if(!status&&(current.instance_id!=t->target.instance_id||current.kind!=t->target.kind||strcmp(current.host_epoch,t->target.host_epoch)||strcmp(current.open_epoch,t->target.open_epoch)||strcmp(current.expected_revision,t->target.expected_revision)||strcmp(current.scope_json,t->target.scope_json)))status=UI_STATUS_VALIDATION_FAILED;}
  if(status==UI_STATUS_OK&&t->discover){uint64_t generation=0;size_t offset=0,bytes=0,model_bytes=2;yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *items=d?yyjson_mut_arr(d):NULL;
   if(!items)status=UI_STATUS_OUT_OF_MEMORY;else{yyjson_mut_doc_set_root(d,items);for(unsigned page=0;page<16;++page){char *raw=NULL;yyjson_doc *parsed;yyjson_val *rows,*row;size_t i,n;
     status=ui_capability_discover(app.host,t->category,offset,32,&generation,&raw);if(status)break;bytes+=strlen(raw);if(bytes>UI_CAPABILITY_OUTPUT_BYTES){ui_capability_release(raw);status=UI_STATUS_LIMIT_EXCEEDED;break;}
     parsed=yyjson_read(raw,strlen(raw),0);rows=parsed?yyjson_obj_get(yyjson_doc_get_root(parsed),"items"):NULL;if(!rows){yyjson_doc_free(parsed);ui_capability_release(raw);status=UI_STATUS_VALIDATION_FAILED;break;}
     yyjson_arr_foreach(rows,i,n,row){const char *effect=yyjson_get_str(yyjson_obj_get(row,"effect"));if(effect&&!strcmp(effect,"read")){if(s->runs[t->run].model)status=am_discovery_append(app.host,row,d,items,&model_bytes);else if(!yyjson_mut_arr_add_val(items,yyjson_val_mut_copy(d,row)))status=UI_STATUS_OUT_OF_MEMORY;if(status)break;}}offset=(size_t)yyjson_get_uint(yyjson_obj_get(yyjson_doc_get_root(parsed),"next"));yyjson_doc_free(parsed);ui_capability_release(raw);if(status||!offset)break;}
    if(status==UI_STATUS_OK)copy=yyjson_mut_write(d,0,NULL);}yyjson_mut_doc_free(d);if(status==UI_STATUS_OK&&!copy)status=UI_STATUS_OUT_OF_MEMORY;
  }else if(status==UI_STATUS_OK){char *description=NULL;status=ui_capability_describe(app.host,t->capability,&description);
   if(!status){yyjson_doc *d=yyjson_read(description,strlen(description),0);const char *effect=d?yyjson_get_str(yyjson_obj_get(yyjson_doc_get_root(d),"effect")):NULL;if(!effect||strcmp(effect,"read"))status=UI_STATUS_PERMISSION_DENIED;yyjson_doc_free(d);}ui_capability_release(description);
   if(!status)status=ui_capability_prepare(app.host,t->capability,&t->target,t->params,&t->call);
  }
  EnterCriticalSection(&s->lock);t->status=status;if(status||t->discover){t->result=copy;t->state=AI_TOOL_RESULT;}else t->state=AI_TOOL_PREPARED;(void)ai_start_locked(s);LeaveCriticalSection(&s->lock);return;
 }
 if(status==UI_STATUS_OK){ui_cap_result_t result={0};if(cancel||app.closing){if(!t->cancel_sent){(void)ui_capability_cancel(app.host,t->call);t->cancel_sent=1;}}
  if(state==AI_TOOL_DISPATCH&&!cancel&&!app.closing)status=ui_capability_dispatch(app.host,t->call);
  if(state==AI_TOOL_DISPATCH&&(cancel||app.closing))status=UI_STATUS_CANCELLED;
  result.size=sizeof(result);result.version=1;
  if(status==UI_STATUS_OK){status=ui_capability_get_result(app.host,t->call,&result);if(!status&&result.state!=UI_CAP_FINISHED){EnterCriticalSection(&s->lock);t->state=AI_TOOL_RUNNING;LeaveCriticalSection(&s->lock);return;}
   if(!status){if(result.outcome==UI_CAP_OK&&result.output_json){strcpy_s(t->result_revision,128,result.revision);copy=ai_copy(result.output_json);if(!copy)status=UI_STATUS_OUT_OF_MEMORY;}else status=UI_STATUS_VALIDATION_FAILED;}}
  {ui_status_t released=ui_capability_forget(app.host,t->call);if(released==UI_STATUS_PENDING){free(copy);return;}if(released!=UI_STATUS_OK&&released!=UI_STATUS_NOT_FOUND){free(copy);return;}}
 }
 EnterCriticalSection(&s->lock);t->status=status;t->result=copy;t->state=AI_TOOL_RESULT;(void)ai_start_locked(s);LeaveCriticalSection(&s->lock);
}
ui_status_t ui_ai_service_poll(ui_ai_service_t *s,uint32_t maximum)
{
 if(!ai_ui(s)||!maximum||maximum>256)return UI_STATUS_INVALID_ARGUMENT;if(s->polling||s->callbacks)return UI_STATUS_PENDING;++s->polling;
 av_ui(s);
 ab_ui(s);
 am_ui(s);
 ai_tool_ui(s);
 as_ui(s);
 ai_edit_ui(s);
 EnterCriticalSection(&s->lock);
 if(s->worker&&WaitForSingleObject(s->worker,0)==WAIT_OBJECT_0){CloseHandle(s->worker);s->worker=NULL;}
 for(int i=0;i<AI_RUNS;++i){ai_run_t *r=&s->runs[i];if(r->id[0]&&r->cancel&&!r->terminal&&!r->stalled&&ai_now(s)-r->cancel_at>=5000){r->stalled=1;r->phase=UI_AI_CANCELLATION_STALLED;r->committed=0;}}
 if(!s->worker&&s->phase!=UI_AI_CLOSED&&(s->cmd_count||am_pending(s)||ai_graph_jobs(s)||(s->edit&&s->edit->job)||(s->tool.state==AI_TOOL_PREPARED||s->tool.state==AI_TOOL_RESULT)||(s->active>=0&&(s->runs[s->active].stalled==1||(s->runs[s->active].cancel&&!s->runs[s->active].cancel_recorded)||s->runs[s->active].answer_ready))||s->close_requested))(void)ai_start_locked(s);
 LeaveCriticalSection(&s->lock);
 for(uint32_t i=0;i<maximum;++i){ai_notice_t *n;EnterCriticalSection(&s->lock);if(!s->event_count){LeaveCriticalSection(&s->lock);break;}n=s->events[s->event_head];s->event_head=(s->event_head+1)%AI_EVENTS;--s->event_count;s->event_bytes-=n->bytes;WakeAllConditionVariable(&s->changed);LeaveCriticalSection(&s->lock);ai_notify(s,n);ai_notice_free(n);}
 --s->polling;return UI_STATUS_OK;
}
#include "ai_edit.inc"
#include "ai_graph.inc"
#include "ai_assistant.inc"
#include "ai_assistant_runtime.inc"
#include "ai_assistant_meter.inc"
#ifdef UI_AI_FIXTURE
/* Private fault/time control; never built/exported in the production DLL. */
void ui_ai_fixture(ui_ai_service_t *s,int mode,uint64_t value)
{EnterCriticalSection(&s->lock);if(mode==1)s->fixture_fail_write=(int)value;if(mode==2)s->fixture_hold=(int)value;if(mode==3)s->time_bias+=value;(void)ai_start_locked(s);LeaveCriticalSection(&s->lock);}
#endif
