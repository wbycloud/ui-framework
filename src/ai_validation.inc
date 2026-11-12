/* RPCP-1: restrictive validation policy; same worker, database and transport. */
#include "ui_framework/ai_validation.h"
#include <limits.h>
#define AV_BUDGET 100000000ull
static const char *av_inputs[]={"", "只用中文回复“玄枢流式验证成功”。不要调用工具。", "请先发现target-0的memory类只读能力，再调用memory.read读取数值与版本，用一句中文报告；不得修改或调用其他能力。", "请用中文解释什么是流式输出，最多三句；不要调用工具。"};
static const unsigned av_limits[]={0,1,3,1};
typedef struct av_ticket {char request[65],run[65],hash[65];uint64_t estimate,measured_input;unsigned ordinal,kind;int entered,simulated,ended,priced;} av_ticket_t;
struct av_policy {
 ab_batch_t *batch;
 int business,reserving,body_scene;char business_model[65],price_version[65],price_evidence[65];uint64_t input_ceiling;
 int meter_required,meter_started;char meter_profile[65],meter_evidence[65];uint64_t meter_last;DWORD meter_exit,meter_system_error;
 char id[65],error[96],client[3][65],run[3][65],admit_client[65];ui_cap_target_t target;
 int job,ready,stopped,admitting,prior,fee_unknown,send_pending,send_unconfirmed;unsigned manifests,reserved,sends,complete,unknown,mock_sends,mock_complete,mock_unknown,used[3];
 uint64_t begin,input_rate,output_rate,known,estimated;av_ticket_t tickets[5];
#ifdef UI_AI_FIXTURE
 int fixture_hold,fixture_duplicate;unsigned fixture_waits;
#endif
};
/* Private scene IDs preserve the old snapshot/body ledger semantics. */
static int av_scene_parse(const char *scene)
{
 if(!strcmp(scene,"snapshot-v1"))return 0;
 if(!strcmp(scene,"dom-body-v1"))return 1;
 if(!strcmp(scene,"public-sqlite-zeroconf-v1"))return 2;
 return -1;
}
static const char *av_scene_name(const av_policy_t *p)
{return p->body_scene==2?"public-sqlite-zeroconf-v1":p->body_scene?"dom-body-v1":"snapshot-v1";}
static void av_stop_locked(ui_ai_service_t *s,const char *reason)
{av_policy_t *p=s->validation;if(!p)return;p->stopped=1;if(!p->error[0])strcpy_s(p->error,96,reason);for(unsigned i=0;i<AI_RUNS;++i){ai_run_t *r=&s->runs[i];if(r->model&&r->model->validation_kind&&!r->terminal){r->cancel=1;if(!r->cancel_at)r->cancel_at=ai_now(s);}}}
static int av_allowed(ui_ai_service_t *s,ai_run_t *r)
{int ok=1;EnterCriticalSection(&s->lock);av_policy_t *p=s->validation;if(p){if(p->begin&&ai_now(s)-p->begin>=600000)av_stop_locked(s,"validation_deadline");ok=(!p->business||(r->model&&r->model->confirmed&&r->model->validation_kind==2&&!strcmp(p->run[1],r->id)&&!strcmp(p->business_model,r->model->model)&&p->begin))&&(!p->batch||(p->batch->accepted&&p->batch->saved&&r->model&&(r->model->validation_kind==2||r->model->validation_kind==3)&&!strcmp(p->run[r->model->validation_kind-1],r->id)&&s->model_config->revision==p->batch->revision&&!strcmp(s->model_config->binding,p->batch->profile)))&&p->ready&&!p->stopped&&!s->store_failed&&!s->close_requested&&s->phase==UI_AI_READY&&!r->cancel;}LeaveCriticalSection(&s->lock);return ok;}
static int av_event(ui_ai_service_t *s,ai_run_t *r,const char *suffix,const char *kind,const char *payload)
{uint64_t seq=0;int ok=ai_begin(s)&&ai_event_insert(s,r,suffix,kind,payload,0,&seq);ok=ai_commit(s,ok);if(!ok)ai_store_error(s,r,"validation_record");return ok;}
static unsigned av_count(ui_ai_service_t *s,const char *sql)
{sqlite3_stmt *q=NULL;unsigned n=UINT_MAX;if(ai_statement(s,sql,&q)&&sqlite3_step(q)==SQLITE_ROW){sqlite3_int64 v=sqlite3_column_int64(q,0);if(v>=0&&v<=UINT_MAX)n=(unsigned)v;}sqlite3_finalize(q);return n;}
#include "ai_business_meter.inc"
static int av_io(ui_ai_service_t *s)
{
 av_policy_t *p=s->validation;if(!p)return 0;EnterCriticalSection(&s->lock);int job=p->job;if(job>1&&p->batch)p->batch->busy=1;p->job=0;LeaveCriticalSection(&s->lock);if(!job)return 0;if(job>1)return ab_worker(s,job);
 unsigned runs=av_count(s,"SELECT count(*) FROM runs"),opens=av_count(s,"SELECT count(*) FROM events WHERE kind='rpc_open'");
 if(runs||opens){
  av_policy_t restored;EnterCriticalSection(&s->lock);restored=*p;LeaveCriticalSection(&s->lock);av_policy_t *live=p;p=&restored;unsigned reserves=av_count(s,"SELECT count(*) FROM events WHERE kind='rpc_reserved'"),manifests=av_count(s,"SELECT count(*) FROM events WHERE kind='provider_request'");sqlite3_stmt *q=NULL;int valid=reserves<=5;
  p->reserved=reserves;p->manifests=manifests;p->prior=1;
  if(valid&&ai_statement(s,"SELECT payload FROM events WHERE kind='rpc_reserved' ORDER BY seq",&q)){unsigned i=0;while(sqlite3_step(q)==SQLITE_ROW){ui_cap_json_doc_t d={0};const char *raw=(const char *)sqlite3_column_text(q,0);int ok=i<5&&raw&&am_json_parse(raw,strlen(raw),&d);yyjson_val *o=ok?yyjson_doc_get_root(d.doc):NULL;const char *id=yyjson_get_str(yyjson_obj_get(o,"request_id"));unsigned task=(unsigned)yyjson_get_uint(yyjson_obj_get(o,"task"));uint64_t estimate=yyjson_get_uint(yyjson_obj_get(o,"estimated_nano_usd"));if(!ok||!id||strlen(id)!=64||task<1||task>3||estimate>AV_BUDGET)valid=0;else{av_ticket_t *t=&p->tickets[i];strcpy_s(t->request,65,id);t->kind=task;t->estimate=estimate;p->estimated+=estimate;++p->used[task-1];}++i;am_json_free(&d);}if(i!=reserves)valid=0;}else valid=0;sqlite3_finalize(q);q=NULL;
  if(valid&&ai_statement(s,"SELECT kind,payload FROM events WHERE kind IN('rpc_send_entered','rpc_network_end','rpc_cost') ORDER BY seq",&q)){while(sqlite3_step(q)==SQLITE_ROW){ui_cap_json_doc_t d={0};const char *raw=(const char *)sqlite3_column_text(q,1),*kind=(const char *)sqlite3_column_text(q,0);int ok=raw&&am_json_parse(raw,strlen(raw),&d);yyjson_val *o=ok?yyjson_doc_get_root(d.doc):NULL;const char *id=yyjson_get_str(yyjson_obj_get(o,"request_id"));av_ticket_t *t=NULL;for(unsigned i=0;i<reserves&&i<5;++i)if(id&&!strcmp(id,p->tickets[i].request))t=&p->tickets[i];if(!t)valid=0;else if(!strcmp(kind,"rpc_send_entered")){t->entered=1;t->simulated=yyjson_get_bool(yyjson_obj_get(o,"simulated"));if(t->simulated)++p->mock_sends;else ++p->sends;}else if(!strcmp(kind,"rpc_network_end")){t->ended=1;if(yyjson_get_bool(yyjson_obj_get(o,"complete"))){if(t->simulated)++p->mock_complete;else ++p->complete;}else if(yyjson_get_bool(yyjson_obj_get(o,"unknown"))){if(t->simulated)++p->mock_unknown;else ++p->unknown;p->fee_unknown=1;}}else{const char *fee=yyjson_get_str(yyjson_obj_get(o,"fee_status"));if(fee&&!strcmp(fee,"known")){t->priced=1;p->known+=yyjson_get_uint(yyjson_obj_get(o,"known_nano_usd"));p->estimated-=t->estimate;}else p->fee_unknown=1;}am_json_free(&d);}}else if(valid)valid=0;sqlite3_finalize(q);
  for(unsigned i=0;i<reserves&&i<5;++i)if(!p->tickets[i].ended){p->fee_unknown=1;if(p->tickets[i].simulated)++p->mock_unknown;else ++p->unknown;}
  EnterCriticalSection(&s->lock);int was_stopped=live->stopped;char old_error[96];strcpy_s(old_error,96,live->error);*live=*p;live->stopped=was_stopped;if(was_stopped)strcpy_s(live->error,96,old_error);av_stop_locked(s,valid?"validation_restart_review_required":"validation_prior_inconsistent");LeaveCriticalSection(&s->lock);return 1;
 }
 char payload[1400];snprintf(payload,sizeof(payload),"{\"policy\":\"RPCP-1\",\"id\":\"%s\",\"session\":\"%s\",\"input_nano_per_token\":%llu,\"output_nano_per_token\":%llu,\"budget_nano_usd\":100000000}",p->id,s->session,(unsigned long long)p->input_rate,(unsigned long long)p->output_rate);
 if(p->business)snprintf(payload,sizeof(payload),"{\"policy\":\"BR-2\",\"id\":\"%s\",\"model\":\"%s\",\"protocol\":\"deepseek-responses-v1\",\"price_version\":\"%s\",\"price_evidence_sha256\":\"%s\",\"input_ceiling\":%llu,\"input_nano_per_token\":%llu,\"output_nano_per_token\":%llu,\"request_limit\":3,\"budget_nano_usd\":100000000}",p->id,p->business_model,p->price_version,p->price_evidence,(unsigned long long)p->input_ceiling,(unsigned long long)p->input_rate,(unsigned long long)p->output_rate);
 if(p->business&&p->meter_required){size_t n=strlen(payload);snprintf(payload+n-1,sizeof(payload)-n+1,",\"meter_profile\":\"%s\",\"meter_evidence_sha256\":\"%s\"}",p->meter_profile,p->meter_evidence);}
 if(p->business&&p->body_scene){size_t n=strlen(payload);snprintf(payload+n-1,sizeof(payload)-n+1,",\"business_scene\":\"%s\"}",av_scene_name(p));}
 int saved=av_event(s,NULL,"rpc_open","rpc_open",payload);EnterCriticalSection(&s->lock);p->ready=saved&&!p->stopped&&!s->store_failed&&!s->close_requested;if(!p->ready)av_stop_locked(s,"validation_open_unconfirmed");LeaveCriticalSection(&s->lock);return 1;
}
static void av_ui(ui_ai_service_t *s)
{EnterCriticalSection(&s->lock);av_policy_t *p=s->validation;if(p){if(s->close_requested)av_stop_locked(s,"validation_closed");if(s->store_failed){if(p->estimated)p->fee_unknown=1;av_stop_locked(s,"validation_storage_unavailable");}if(p->begin&&ai_now(s)-p->begin>=600000)av_stop_locked(s,"validation_deadline");}LeaveCriticalSection(&s->lock);}
static int av_target_equal(const ui_cap_target_t *a,const ui_cap_target_t *b)
{return a&&b&&a->instance_id==b->instance_id&&a->kind==b->kind&&!strcmp(a->resource_id,b->resource_id)&&!strcmp(a->host_epoch,b->host_epoch)&&!strcmp(a->open_epoch,b->open_epoch)&&!strcmp(a->expected_revision,b->expected_revision)&&!strcmp(a->scope_json,b->scope_json);}
ui_status_t ui_ai_validation_enable(ui_ai_service_t *s,const ui_ai_validation_desc_t *d)
{
 if(!ai_ui(s)||!d||d->size<sizeof(*d)||d->version!=1||!ai_id(d->validation_id)||d->input_nano_per_token<300||d->input_nano_per_token>1000000||d->output_nano_per_token<1200||d->output_nano_per_token>1000000)return UI_STATUS_INVALID_ARGUMENT;
 if(s->assistant_policy)return UI_STATUS_PERMISSION_DENIED;
 char path[4096];if(snprintf(path,sizeof(path),"%s.model.json",s->path)>=4096||!s->model_config||strcmp(path,s->model_config->path))return UI_STATUS_PERMISSION_DENIED;
 EnterCriticalSection(&s->lock);if(s->validation){LeaveCriticalSection(&s->lock);return UI_STATUS_ALREADY_EXISTS;}if(s->phase!=UI_AI_READY||s->close_requested){LeaveCriticalSection(&s->lock);return UI_STATUS_PENDING;}for(unsigned i=0;i<AI_RUNS;++i)if(s->runs[i].id[0]){LeaveCriticalSection(&s->lock);return UI_STATUS_PERMISSION_DENIED;}
 if(sizeof(av_policy_t)>UI_AI_MAX_BYTES-s->bytes){LeaveCriticalSection(&s->lock);return UI_STATUS_LIMIT_EXCEEDED;}av_policy_t *p=calloc(1,sizeof(*p));if(!p){LeaveCriticalSection(&s->lock);return UI_STATUS_OUT_OF_MEMORY;}strcpy_s(p->id,65,d->validation_id);p->input_rate=d->input_nano_per_token;p->output_rate=d->output_nano_per_token;p->job=1;s->validation=p;s->bytes+=sizeof(*p);(void)ai_start_locked(s);LeaveCriticalSection(&s->lock);return UI_STATUS_OK;
}
ui_status_t ui_ai_validation_stop(ui_ai_service_t *s)
{if(!ai_ui(s))return UI_STATUS_INVALID_ARGUMENT;EnterCriticalSection(&s->lock);if(!s->validation){LeaveCriticalSection(&s->lock);return UI_STATUS_NOT_FOUND;}av_stop_locked(s,"validation_stopped");(void)ai_start_locked(s);LeaveCriticalSection(&s->lock);return UI_STATUS_OK;}
ui_status_t ui_ai_validation_get_state(ui_ai_service_t *s,char **json)
{
 if(json)*json=NULL;if(!ai_ui(s)||!json)return UI_STATUS_INVALID_ARGUMENT;av_ui(s);EnterCriticalSection(&s->lock);av_policy_t *p=s->validation;if(!p){LeaveCriticalSection(&s->lock);return UI_STATUS_NOT_FOUND;}
 char out[2200];snprintf(out,sizeof(out),"{\"policy\":\"%s\",\"request_limit\":%u,\"version\":1,\"ready\":%s,\"stopped\":%s,\"prior_session\":%s,\"error\":\"%s\",\"manifest\":%u,\"reserved\":%u,\"send_entries\":%u,\"complete\":%u,\"unknown\":%u,\"simulated_send_entries\":%u,\"simulated_complete\":%u,\"simulated_unknown\":%u,\"task_used\":[%u,%u,%u],\"known_nano_usd\":%llu,\"estimated_nano_usd\":%llu,\"fee_unknown\":%s,\"send_count_known\":%s,\"budget_nano_usd\":100000000,\"send_count_saved\":%s}",p->business?"BR-2":"RPCP-1",p->business?3u:5u,p->ready?"true":"false",p->stopped?"true":"false",p->prior?"true":"false",p->error,p->manifests,p->reserved,p->sends,p->complete,p->unknown,p->mock_sends,p->mock_complete,p->mock_unknown,p->used[0],p->used[1],p->used[2],(unsigned long long)p->known,(unsigned long long)p->estimated,p->fee_unknown?"true":"false",(p->prior||p->send_pending)?"false":"true",p->send_unconfirmed?"false":"true");if(p->business){size_t n=strlen(out);char code[32];if(p->meter_started)snprintf(code,sizeof(code),"%lu",(unsigned long)p->meter_exit);else strcpy_s(code,sizeof(code),"null");snprintf(out+n-1,sizeof(out)-n+1,",\"business_scene\":\"%s\",\"meter_required\":%s,\"meter_profile\":\"%s\",\"meter_last_input\":%llu,\"meter_started\":%s,\"meter_exit_code\":%s,\"meter_system_error\":%lu}",av_scene_name(p),p->meter_required?"true":"false",p->meter_profile,(unsigned long long)p->meter_last,p->meter_started?"true":"false",code,(unsigned long)p->meter_system_error);}*json=ai_copy(out);LeaveCriticalSection(&s->lock);return *json?UI_STATUS_OK:UI_STATUS_OUT_OF_MEMORY;
}
ui_status_t ui_ai_validation_propose(ui_ai_service_t *s,const ui_ai_validation_request_t *d,char out[65])
{
 if(out)out[0]=0;if(!ai_ui(s)||!d||d->size<sizeof(*d)||d->version!=1||d->task<1||d->task>3||!ai_id(d->client_request_id)||!out||(d->task==2&&!d->target)||(d->task!=2&&d->target))return UI_STATUS_INVALID_ARGUMENT;
 ui_cap_target_t target={0};if(d->target){if(d->target->size<sizeof(target)||d->target->version!=1||!memchr(d->target->resource_id,0,sizeof(d->target->resource_id))||!memchr(d->target->host_epoch,0,sizeof(d->target->host_epoch))||!memchr(d->target->open_epoch,0,sizeof(d->target->open_epoch))||!memchr(d->target->expected_revision,0,sizeof(d->target->expected_revision))||!memchr(d->target->scope_json,0,sizeof(d->target->scope_json))||strcmp(d->target->resource_id,"memory.document")||strcmp(d->target->expected_revision,"r1"))return UI_STATUS_PERMISSION_DENIED;target=*d->target;}
 EnterCriticalSection(&s->lock);av_policy_t *p=s->validation;unsigned k=d->task-1;if(!p){LeaveCriticalSection(&s->lock);return UI_STATUS_PERMISSION_DENIED;}
 if(p->client[k][0]){int same=!strcmp(p->client[k],d->client_request_id)&&(d->task!=2||av_target_equal(&target,&p->target));if(same)strcpy_s(out,65,p->run[k]);LeaveCriticalSection(&s->lock);return same?UI_STATUS_OK:UI_STATUS_ALREADY_EXISTS;}
 if(p->business||(p->batch&&(!p->batch->submitting||d->task==1))||!p->ready||p->stopped||p->admitting||s->store_failed||s->close_requested||s->phase!=UI_AI_READY){LeaveCriticalSection(&s->lock);return UI_STATUS_PERMISSION_DENIED;}if(am_live(s)){LeaveCriticalSection(&s->lock);return UI_STATUS_PENDING;}
 p->admitting=d->task;strcpy_s(p->admit_client,65,d->client_request_id);++s->methods;LeaveCriticalSection(&s->lock);
 ui_ai_real_task_t q={0};q.size=sizeof(q);q.version=1;q.client_request_id=d->client_request_id;q.profile_id="deepseek";q.profile_revision=s->model_config->revision;q.input_utf8=av_inputs[d->task];q.history_selection_json="[]";q.disclosure_projection_json=d->task==2?"{\"capabilities\":[\"memory.read\"],\"fields\":[\"value\",\"revision\"]}":"{\"capabilities\":[],\"fields\":[]}";q.targets=d->task==2?&target:NULL;q.target_count=d->task==2?1:0;
 ui_status_t status=ui_ai_real_propose(s,&q,out);EnterCriticalSection(&s->lock);if(!status){strcpy_s(p->client[k],65,d->client_request_id);strcpy_s(p->run[k],65,out);if(d->task==2)p->target=target;}p->admitting=0;p->admit_client[0]=0;--s->methods;LeaveCriticalSection(&s->lock);return status;
}
static int av_manifest(ui_ai_service_t *s,ai_run_t *r,const char *suffix,const char *payload)
{uint64_t seq=0;int saved=ai_change(s,r,UI_AI_RUNNING,"",suffix,"provider_request",payload,NULL,NULL,&seq);if(!saved){ai_store_error(s,r,"model_record");return 0;}if(s->validation){EnterCriticalSection(&s->lock);++s->validation->manifests;LeaveCriticalSection(&s->lock);}return ai_store_available(s);}
static void av_confirm(ui_ai_service_t *s,ai_run_t *r)
{if(r->model->assistant)as_confirm(s,r);if(s->validation&&r->model->validation_kind&&!s->validation->begin)s->validation->begin=ai_now(s);}
static int av_paused(ui_ai_service_t *s)
{
#ifdef UI_AI_FIXTURE
 EnterCriticalSection(&s->lock);av_policy_t *p=s->validation;int held=p&&p->fixture_hold;if(held)++p->fixture_waits;LeaveCriticalSection(&s->lock);return held;
#else
 (void)s;return 0;
#endif
}
static int av_reserve(ui_ai_service_t *s,ai_run_t *r)
{
 if(r->model->assistant)return as_reserve(s,r);
 av_policy_t *p=s->validation;if(!p)return 1;am_run_t *m=r->model;unsigned k=m->validation_kind;
#ifdef UI_AI_FIXTURE
 if(p->fixture_duplicate&&p->reserved)strcpy_s(m->request_id,65,p->tickets[0].request);
#endif
 if(!k||k>3||!av_allowed(s,r))return 0;
 /* Exhausted validation cannot send, so it must not start another meter. */
 EnterCriticalSection(&s->lock);int exhausted=p->business&&p->reserved>=3;if(exhausted)av_stop_locked(s,"validation_quota_or_fee_limit");LeaveCriticalSection(&s->lock);if(exhausted)return 0;
 uint64_t measured=0;
 if(p->business&&p->meter_required){
  if(m->metered_ordinal!=m->ordinal){
   if(!bm_measure(s,r,&measured)||!av_allowed(s,r))return 0;
   /* UI-thread callbacks recheck targets/configuration after the child exits. */
   char hash[65];ui_cap_hash_bytes(m->body,strlen(m->body),hash);
   EnterCriticalSection(&s->lock);m->metered_input=measured;m->metered_ordinal=m->ordinal;strcpy_s(m->metered_hash,65,hash);m->validated=0;LeaveCriticalSection(&s->lock);return 2;
  }
  char hash[65];ui_cap_hash_bytes(m->body,strlen(m->body),hash);
  measured=m->metered_input;if(!measured||measured>p->input_ceiling||strcmp(hash,m->metered_hash))return 0;
 }
 EnterCriticalSection(&s->lock);int duplicate=0;for(unsigned i=0;i<p->reserved&&i<5;++i)if(!strcmp(p->tickets[i].request,m->request_id))duplicate=1;
 uint64_t estimate=(p->business?p->input_ceiling:((uint64_t)strlen(m->body)+1024))*p->input_rate+4096*p->output_rate;
 int ok=!p->reserving&&!duplicate&&p->reserved<(p->business?3u:(p->batch?4u:5u))&&(!p->batch||k!=1)&&p->used[k-1]<av_limits[k]&&!p->fee_unknown&&p->known<=AV_BUDGET&&p->estimated<=AV_BUDGET-p->known&&estimate<=AV_BUDGET-p->known-p->estimated;if(ok&&p->business)p->reserving=1;LeaveCriticalSection(&s->lock);
 if(!ok){EnterCriticalSection(&s->lock);av_stop_locked(s,duplicate?"validation_duplicate_request":"validation_quota_or_fee_limit");LeaveCriticalSection(&s->lock);return 0;}
 char payload[1200],suffix[96],hash[65];ui_cap_hash_bytes(m->body,strlen(m->body),hash);snprintf(suffix,sizeof(suffix),"rpc:%u:reserved",m->ordinal);snprintf(payload,sizeof(payload),"{\"session\":\"%s\",\"run_id\":\"%s\",\"request_id\":\"%s\",\"ordinal\":%u,\"task\":%u,\"body_hash\":\"%s\",\"estimated_nano_usd\":%llu}",s->session,r->id,m->request_id,m->ordinal,k,hash,(unsigned long long)estimate);
 if(p->meter_required){size_t n=strlen(payload);snprintf(payload+n-1,sizeof(payload)-n+1,",\"meter\":{\"profile\":\"%s\",\"evidence\":\"%s\",\"body_hash\":\"%s\",\"input_tokens\":%llu,\"output_limit\":4096}}",p->meter_profile,p->meter_evidence,hash,(unsigned long long)measured);}
 if(p->business){EnterCriticalSection(&s->lock);unsigned index=p->reserved++;av_ticket_t *t=&p->tickets[index];strcpy_s(t->request,65,m->request_id);strcpy_s(t->run,65,r->id);strcpy_s(t->hash,65,hash);t->kind=k;t->ordinal=m->ordinal;t->estimate=estimate;t->measured_input=measured;++p->used[k-1];p->estimated+=estimate;LeaveCriticalSection(&s->lock);int saved=av_event(s,r,suffix,"rpc_reserved",payload);EnterCriticalSection(&s->lock);p->reserving=0;if(saved)m->validation_ticket=index+1;else av_stop_locked(s,"business_reservation_not_saved");LeaveCriticalSection(&s->lock);return saved&&av_allowed(s,r);}
 int saved=av_event(s,r,suffix,"rpc_reserved",payload);if(saved){EnterCriticalSection(&s->lock);unsigned index=p->reserved++;av_ticket_t *t=&p->tickets[index];strcpy_s(t->request,65,m->request_id);strcpy_s(t->run,65,r->id);strcpy_s(t->hash,65,hash);t->kind=k;t->ordinal=m->ordinal;t->estimate=estimate;++p->used[k-1];p->estimated+=estimate;m->validation_ticket=index+1;LeaveCriticalSection(&s->lock);}return saved&&av_allowed(s,r);
}
static int av_before_send(ui_ai_service_t *s,ai_run_t *r,int simulated)
{
 if(r->model->assistant)return as_before_send(s,r,simulated);
 av_policy_t *p=s->validation;if(!p)return 1;am_run_t *m=r->model;if(!m->validation_ticket||m->validation_ticket>p->reserved||!av_allowed(s,r))return 0;av_ticket_t *t=&p->tickets[m->validation_ticket-1];char hash[65];ui_cap_hash_bytes(m->body,strlen(m->body),hash);
 if((p->meter_required&&(!t->measured_input||t->measured_input>p->input_ceiling))||t->entered||strcmp(t->request,m->request_id)||strcmp(t->run,r->id)||strcmp(t->hash,hash))return 0;
 char suffix[96],payload[256];snprintf(suffix,sizeof(suffix),"rpc:%u:send-intent",m->ordinal);snprintf(payload,sizeof(payload),"{\"request_id\":\"%s\",\"simulated\":%s}",m->request_id,simulated?"true":"false");if(!av_event(s,r,suffix,"rpc_send_intent",payload)||!av_allowed(s,r))return 0;
 EnterCriticalSection(&s->lock);t->entered=1;t->simulated=simulated;++p->send_pending;LeaveCriticalSection(&s->lock);return 1;
}
static void av_send_ack(ui_ai_service_t *s,ai_run_t *r)
{if(r->model->assistant){as_send_ack(s,r);return;}if(!s->validation)return;char suffix[96],payload[256];av_policy_t *p=s->validation;av_ticket_t *t=&p->tickets[r->model->validation_ticket-1];EnterCriticalSection(&s->lock);if(t->entered==1){t->entered=2;--p->send_pending;if(t->simulated)++p->mock_sends;else ++p->sends;}LeaveCriticalSection(&s->lock);snprintf(suffix,sizeof(suffix),"rpc:%u:send-entered",r->model->ordinal);snprintf(payload,sizeof(payload),"{\"request_id\":\"%s\",\"simulated\":%s}",t->request,t->simulated?"true":"false");int saved=av_event(s,r,suffix,"rpc_send_entered",payload)&&ai_store_available(s);if(!saved){EnterCriticalSection(&s->lock);p->send_unconfirmed=1;LeaveCriticalSection(&s->lock);}}
static void av_network_end(ui_ai_service_t *s,ai_run_t *r)
{
 if(r->model->assistant){as_network_end(s,r);return;}
 av_policy_t *p=s->validation;am_run_t *m=r->model;if(!p||!m->validation_ticket)return;av_ticket_t *t=&p->tickets[m->validation_ticket-1];if(t->ended)return;
 EnterCriticalSection(&s->lock);t->ended=1;if(m->net.response_complete){if(t->simulated)++p->mock_complete;else ++p->complete;}else{if(m->net.sent){if(t->simulated)++p->mock_unknown;else ++p->unknown;p->fee_unknown=1;}av_stop_locked(s,"validation_network_or_send_unknown");}LeaveCriticalSection(&s->lock);
 char suffix[96],payload[300];snprintf(suffix,sizeof(suffix),"rpc:%u:network-end",m->ordinal);snprintf(payload,sizeof(payload),"{\"request_id\":\"%s\",\"complete\":%s,\"unknown\":%s}",m->request_id,m->net.response_complete?"true":"false",m->net.sent&&!m->net.response_complete?"true":"false");if(ai_store_available(s))(void)av_event(s,r,suffix,"rpc_network_end",payload);
}
static int av_price(ui_ai_service_t *s,ai_run_t *r)
{
 if(r->model->assistant)return as_price(s,r);
 av_policy_t *p=s->validation;if(!p)return 1;am_run_t *m=r->model;av_ticket_t *t=&p->tickets[m->validation_ticket-1];if(t->priced)return 1;
 ui_cap_json_doc_t doc={0};int valid=am_json_parse(m->batch,strlen(m->batch),&doc);yyjson_val *u=valid?yyjson_obj_get(yyjson_doc_get_root(doc.doc),"usage"):NULL,*in=yyjson_obj_get(u,"input_tokens"),*out=yyjson_obj_get(u,"output_tokens");valid=yyjson_is_uint(in)&&yyjson_is_uint(out)&&yyjson_get_uint(in)<=(p->meter_required?t->measured_input:(p->business?p->input_ceiling:10000000))&&yyjson_get_uint(out)<=4096;uint64_t cost=valid?yyjson_get_uint(in)*p->input_rate+yyjson_get_uint(out)*p->output_rate:0;am_json_free(&doc);
 char suffix[96],payload[300],amount[32];if(valid)snprintf(amount,sizeof(amount),"%llu",(unsigned long long)cost);else strcpy_s(amount,sizeof(amount),"null");snprintf(suffix,sizeof(suffix),"rpc:%u:cost",m->ordinal);snprintf(payload,sizeof(payload),"{\"request_id\":\"%s\",\"fee_status\":\"%s\",\"known_nano_usd\":%s}",m->request_id,valid?"known":"unknown",amount);
 if(!av_event(s,r,suffix,"rpc_cost",payload)||!ai_store_available(s))return 0;
 EnterCriticalSection(&s->lock);t->priced=1;if(valid){p->estimated-=t->estimate;p->known+=cost;if(p->known>=AV_BUDGET)av_stop_locked(s,"validation_fee_limit");}else{p->fee_unknown=1;av_stop_locked(s,"validation_usage_unknown");}LeaveCriticalSection(&s->lock);return valid;
}
static void av_finished(ui_ai_service_t *s,ai_run_t *r,const char *outcome)
{if(s->validation&&r->model&&r->model->validation_kind&&(s->validation->business||strcmp(outcome,"completed"))){EnterCriticalSection(&s->lock);av_stop_locked(s,!strcmp(outcome,"completed")?"business_task_finished":"validation_task_not_completed");LeaveCriticalSection(&s->lock);}}
static void av_dispose(ui_ai_service_t *s)
{if(s->validation){if(s->validation->batch){s->bytes-=sizeof(ab_batch_t);free(s->validation->batch);}s->bytes-=sizeof(*s->validation);free(s->validation);s->validation=NULL;}}

#include "ai_business_validation.inc"

#ifdef UI_AI_FIXTURE
#ifdef UI_AI_EDIT_TEST_DLL
__declspec(dllexport)
#endif
unsigned ui_ai_validation_fixture(ui_ai_service_t *s,int action)
{if(!ai_ui(s)||!s->validation)return 0;EnterCriticalSection(&s->lock);av_policy_t *p=s->validation;if(action==1)p->fixture_hold=1;if(action==2||action==3||action==4){p->fixture_hold=0;if(action==3)s->fixture_fail_write=1;if(action==4)InterlockedExchange(&s->fixture_checkpoint,SQLITE_IOERR_WRITE);(void)ai_start_locked(s);}if(action==5)p->fixture_duplicate=1;if(action==6)s->fixture_fail_write=1;unsigned result=p->fixture_waits;LeaveCriticalSection(&s->lock);return result;}
#endif
