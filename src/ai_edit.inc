/* P4A-1. UI controller plus jobs on the existing serial storage worker. */
#include "capability_internal.h"
#include <commctrl.h>
int ui_inputs_current_source(void);
struct ai_edit_button {ui_ai_service_t *service;HWND window;ui_web_view_t *view;char control[128];uint64_t id;ui_ai_edit_action_t action;char binding[65];int pressed,used;uint32_t modifiers;ai_edit_button_t *next;};
static void ae_json(yyjson_mut_doc *d,yyjson_mut_val *o,const char *key,const char *raw)
{yyjson_doc *p=yyjson_read(raw,strlen(raw),0);if(p)yyjson_mut_obj_add_val(d,o,key,yyjson_val_mut_copy(d,yyjson_doc_get_root(p)));yyjson_doc_free(p);}
static char *ae_snapshot(ai_edit_t *e,int phase)
{
 yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *o=d?yyjson_mut_obj(d):NULL,*t=d?yyjson_mut_obj(d):NULL;char *json;if(!o||!t){yyjson_mut_doc_free(d);return NULL;}yyjson_mut_doc_set_root(d,o);
 yyjson_mut_obj_add_strcpy(d,o,"id",e->id);yyjson_mut_obj_add_strcpy(d,o,"client_request_id",e->client);yyjson_mut_obj_add_strcpy(d,o,"capability_id",e->capability);yyjson_mut_obj_add_strcpy(d,o,"descriptor_hash",e->descriptor);yyjson_mut_obj_add_strcpy(d,o,"binding_hash",e->binding);
 yyjson_mut_obj_add_strcpy(d,o,"side_effect",e->side_effect);yyjson_mut_obj_add_strcpy(d,o,"policy_revision","p4a-1");yyjson_mut_obj_add_bool(d,o,"transaction_committed",e->committed!=0);yyjson_mut_obj_add_bool(d,o,"rolled_back",e->rolled_back!=0);yyjson_mut_obj_add_int(d,o,"phase",phase);yyjson_mut_obj_add_int(d,o,"schema",e->schema);yyjson_mut_obj_add_int(d,o,"risk",e->risk);yyjson_mut_obj_add_int(d,o,"effect",e->effect);
 yyjson_mut_obj_add_bool(d,o,"proposal_confirmed",e->proposal_confirmed!=0);yyjson_mut_obj_add_int(d,o,"last_host_status",e->host_status);yyjson_mut_obj_add_int(d,o,"execution_policy",e->execution_policy);yyjson_mut_obj_add_strcpy(d,o,"storage_path",e->storage_path);yyjson_mut_obj_add_bool(d,o,"terminal",phase==AE_DONE||phase==AE_UNKNOWN);yyjson_mut_obj_add_bool(d,o,"storage_confirmed",phase==AE_DONE&&!e->rejected);yyjson_mut_obj_add_bool(d,o,"rejected",e->rejected!=0);yyjson_mut_obj_add_bool(d,o,"cancel_requested",e->cancel!=0);yyjson_mut_obj_add_bool(d,o,"undo",e->undo!=0);
 yyjson_mut_obj_add_uint(d,o,"scope_until",e->scope_until);yyjson_mut_obj_add_uint(d,o,"approval_until",e->approval_until);{char call[32];snprintf(call,sizeof(call),"%llu",(unsigned long long)e->call);yyjson_mut_obj_add_strcpy(d,o,"call_id",call);}yyjson_mut_obj_add_int(d,o,"recovered_unresolved",e->recovered);
 yyjson_mut_obj_add_strcpy(d,o,"code",e->code);yyjson_mut_obj_add_strcpy(d,o,"transaction_id",e->transaction);yyjson_mut_obj_add_strcpy(d,o,"undo_id",e->undo_id);yyjson_mut_obj_add_strcpy(d,o,"revision_after",e->revision_after);yyjson_mut_obj_add_strcpy(d,o,"backup_path",e->backup);
 yyjson_mut_obj_add_strcpy(d,t,"instance_id","0");{char id[32];snprintf(id,sizeof(id),"%llu",(unsigned long long)e->target.resource.instance_id);yyjson_mut_obj_remove_key(t,"instance_id");yyjson_mut_obj_add_strcpy(d,t,"instance_id",id);}
 yyjson_mut_obj_add_strcpy(d,t,"host_epoch",e->target.resource.host_epoch);yyjson_mut_obj_add_strcpy(d,t,"resource_id",e->target.resource.resource_id);yyjson_mut_obj_add_strcpy(d,t,"open_epoch",e->target.resource.open_epoch);yyjson_mut_obj_add_strcpy(d,t,"revision",e->target.expected_revision);
 yyjson_mut_obj_add_val(d,o,"target",t);if(e->params[0])ae_json(d,o,"params",e->params);if(e->constraint[0])ae_json(d,o,"parameter_constraint",e->constraint);if(e->output[0])ae_json(d,o,"output",e->output);
 json=yyjson_mut_write(d,0,NULL);yyjson_mut_doc_free(d);return json;
}
static ui_status_t ae_binding_one(ui_ai_service_t *s,ai_edit_t *e)
{
 yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *o=d?yyjson_mut_obj(d):NULL;char *json;ui_status_t status;if(!o){yyjson_mut_doc_free(d);return UI_STATUS_OUT_OF_MEMORY;}yyjson_mut_doc_set_root(d,o);
 yyjson_mut_obj_add_strcpy(d,o,"authority_epoch",s->session);yyjson_mut_obj_add_strcpy(d,o,"id",e->id);yyjson_mut_obj_add_strcpy(d,o,"request_hash",e->hash);yyjson_mut_obj_add_strcpy(d,o,"path",s->path);yyjson_mut_obj_add_strcpy(d,o,"backup",e->backup);yyjson_mut_obj_add_int(d,o,"schema",e->schema);
 json=yyjson_mut_write(d,0,NULL);yyjson_mut_doc_free(d);if(!json)return UI_STATUS_OUT_OF_MEMORY;status=ui_cap_json_fingerprint(json,strlen(json),e->binding);free(json);return status;
}
static int ae_queue_one(ui_ai_service_t *s,ai_edit_t *e,int job)
{
 char *json;if(!ai_store_available(s))return 0;
#ifdef UI_AI_FIXTURE
 if(e->fixture_fail==9){e->fixture_fail=0;return 0;}
#endif
 json=ae_snapshot(e,(job==AE_FINAL||job==AE_PROOF)?(e->effect==UI_CAP2_EFFECT_UNKNOWN?AE_UNKNOWN:AE_DONE):job==AE_PROPOSE?AE_SCOPE_WAIT:e->phase);
 if(!json)return 0;if(strlen(json)>AI_PAGE){free(json);return 0;}
 EnterCriticalSection(&s->lock);if(e->job||e->busy){LeaveCriticalSection(&s->lock);free(json);return 0;}
 strcpy_s(e->payload,sizeof(e->payload),json);e->job=job;e->io_done=0;e->io_ok=0;(void)ai_start_locked(s);LeaveCriticalSection(&s->lock);free(json);return 1;
}
static int ae_pragma(sqlite3 *db,const char *sql)
{sqlite3_stmt *q=NULL;int n=-1;if(sqlite3_prepare_v2(db,sql,-1,&q,NULL)==SQLITE_OK&&sqlite3_step(q)==SQLITE_ROW)n=sqlite3_column_int(q,0);sqlite3_finalize(q);return n;}
static int ae_backup_one(ui_ai_service_t *s,ai_edit_t *e,int fault)
{
 sqlite3 *copy=NULL;sqlite3_backup *backup;wchar_t path[4096];HANDLE file;int ok=0;
 if(fault==1)return 0;if(!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,e->backup,-1,path,4096))return 0;
 file=CreateFileW(path,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);if(file==INVALID_HANDLE_VALUE)return 0;CloseHandle(file);
 if(sqlite3_open_v2(e->backup,&copy,SQLITE_OPEN_READWRITE|SQLITE_OPEN_FULLMUTEX,NULL)!=SQLITE_OK)goto done;
 backup=sqlite3_backup_init(copy,"main",s->db,"main");if(!backup)goto done;ok=sqlite3_backup_step(backup,-1)==SQLITE_DONE;if(sqlite3_backup_finish(backup)!=SQLITE_OK)ok=0;
 if(fault==6)(void)sqlite3_exec(copy,"PRAGMA user_version=99",NULL,NULL,NULL);
 if(ok){sqlite3_stmt *q=NULL;ok=sqlite3_prepare_v2(copy,"PRAGMA quick_check(1)",-1,&q,NULL)==SQLITE_OK&&sqlite3_step(q)==SQLITE_ROW&&!strcmp((const char *)sqlite3_column_text(q,0),"ok");sqlite3_finalize(q);}
 if(ok)ok=ae_pragma(copy,"PRAGMA user_version")==1&&ae_pragma(copy,"PRAGMA application_id")==0x58535033&&ae_pragma(copy,"SELECT count(*) FROM conversation")==1;
 if(fault==6)ok=0;
done:if(copy&&sqlite3_close(copy)!=SQLITE_OK)ok=0;return ok;
}
static int ae_migrate_one(ui_ai_service_t *s,ai_edit_t *e,int fault)
{
 if(ae_pragma(s->db,"PRAGMA user_version")!=1)return 0;
 if(!ae_backup_one(s,e,fault)||!ai_begin(s))return 0;
 {int ok=ai_sql(s,"CREATE TABLE ai_edits(id TEXT PRIMARY KEY,client TEXT UNIQUE NOT NULL,hash TEXT NOT NULL,resource TEXT NOT NULL,payload TEXT NOT NULL,terminal INTEGER NOT NULL,unknown INTEGER NOT NULL);"
  "CREATE TABLE ai_edit_consumptions(id TEXT PRIMARY KEY,edit_id TEXT NOT NULL,binding TEXT NOT NULL,kind TEXT NOT NULL);PRAGMA user_version=2;");if(fault==2)ok=0;return ai_commit(s,ok);}
}
static int ae_store_one(ui_ai_service_t *s,ai_edit_t *e,int job,const char *payload,int fault)
{
 sqlite3_stmt *q=NULL;ai_run_t run={0};uint64_t sequence=0;int terminal=job==AE_FINAL;int ok,existing=0,cancelled;yyjson_doc *snapshot=yyjson_read(payload,strlen(payload),0);cancelled=snapshot&&yyjson_is_true(yyjson_obj_get(yyjson_doc_get_root(snapshot),"cancel_requested"));yyjson_doc_free(snapshot);const char *kind=job==AE_PROPOSE?"edit_proposed":job==AE_SCOPE?"edit_scope":job==AE_INTENT?"edit_intent":job==AE_FINAL?"edit_final":"edit_reconciled";
 if(job==AE_PROPOSE){if(!ai_statement(s,"SELECT hash,payload FROM ai_edits WHERE id=?",&q))return 0;ai_bind(q,1,e->id);if(sqlite3_step(q)==SQLITE_ROW){existing=strcmp((const char *)sqlite3_column_text(q,0),e->hash)?-1:1;if(existing==1){if(sqlite3_column_bytes(q,1)>AI_PAGE){sqlite3_finalize(q);return 0;}strcpy_s(e->payload,sizeof(e->payload),(const char *)sqlite3_column_text(q,1));}}sqlite3_finalize(q);if(existing)return existing==1?2:3;}
 if((fault==3&&job==AE_INTENT)||(fault==4&&job==AE_FINAL)||(fault==5&&job==AE_PROOF))return 0;
 if(job==AE_INTENT)ar_crash(s,3);if(job==AE_FINAL)ar_crash(s,11);
 if(!ai_begin(s))return 0;strcpy_s(run.id,65,e->id);strcpy_s(run.client,65,e->client);run.reply=e->output;
 if(job==AE_PROPOSE){ok=ai_statement(s,"INSERT INTO ai_edits VALUES(?,?,?,?,?,0,0)",&q);if(ok){ai_bind(q,1,e->id);ai_bind(q,2,e->client);ai_bind(q,3,e->hash);ai_bind(q,4,e->target.resource.resource_id);ai_bind(q,5,payload);ok=ai_done(q);}
  if(ok){ok=ai_statement(s,"INSERT INTO runs(id,client,hash,input,plan,targets,parent,phase) VALUES(?,?,?,?,?,?,?,3)",&q);if(ok){ai_bind(q,1,e->id);ai_bind(q,2,e->client);ai_bind(q,3,e->hash);ai_bind(q,4,e->capability);ai_bind(q,5,"{\"edit\":true}");ai_bind(q,6,payload);ai_bind(q,7,"");ok=ai_done(q);}}if(ok)ok=ai_message(s,&run,"edit-user","user",payload);
 }else{ok=ai_statement(s,"UPDATE ai_edits SET payload=?,terminal=?,unknown=? WHERE id=?",&q);if(ok){ai_bind(q,1,payload);sqlite3_bind_int(q,2,job==AE_FINAL||job==AE_PROOF);sqlite3_bind_int(q,3,job==AE_FINAL&&e->effect==UI_CAP2_EFFECT_UNKNOWN);ai_bind(q,4,e->id);ok=ai_done(q);}}
 if(ok&&job==AE_INTENT){const char *types[]={"scope","approval"};for(unsigned i=0;i<(e->risk==UI_EDIT_REVERSIBLE?1u:2u)&&ok;++i){char id[96];snprintf(id,sizeof(id),"%s:%s",e->id,types[i]);ok=ai_statement(s,"INSERT INTO ai_edit_consumptions VALUES(?,?,?,?)",&q);if(ok){ai_bind(q,1,id);ai_bind(q,2,e->id);ai_bind(q,3,e->binding);ai_bind(q,4,types[i]);ok=ai_done(q);}}}
 if(ok&&job==AE_FINAL){ok=ai_statement(s,"UPDATE runs SET terminal=1,phase=7,outcome=?,reply=? WHERE id=? AND terminal=0",&q);if(ok){ai_bind(q,1,e->effect==UI_CAP2_EFFECT_UNKNOWN?"unknown":cancelled?"cancelled":!strcmp(e->code,"ok")?"completed":"failed");ai_bind(q,2,e->output);ai_bind(q,3,e->id);ok=ai_done(q);}if(ok)ok=ai_message(s,&run,"edit-final","assistant",payload);}
 if(ok&&job==AE_PROOF){ok=ai_statement(s,"UPDATE runs SET terminal=1,phase=7,outcome='failed',reply=? WHERE id=? AND terminal=0",&q);if(ok){ai_bind(q,1,e->output);ai_bind(q,2,e->id);ok=ai_done(q);if(ok)terminal=sqlite3_changes(s->db)!=0;}}
 if(ok)ok=ai_event_insert(s,&run,kind,kind,payload,terminal,&sequence);
 ok=ai_commit(s,ok);if(ok){if(job==AE_INTENT)ar_crash(s,4);if(job==AE_FINAL)ar_crash(s,12);}if(ok){EnterCriticalSection(&s->lock);e->sequence=sequence;LeaveCriticalSection(&s->lock);}return (fault==7&&job==AE_FINAL)||(fault==8&&job==AE_INTENT)?0:ok;
}
static int ae_io_one(ui_ai_service_t *s,ai_edit_t *e)
{
 int job,fault,ok=0,schema=0,recovered=0;const char *payload;
 EnterCriticalSection(&s->lock);if(!e||!e->job||e->busy||e->fixture_hold){LeaveCriticalSection(&s->lock);return 0;}job=e->job;e->job=0;e->busy=1;fault=e->fixture_fail;if((job==AE_MIGRATE&&(fault==1||fault==2||fault==6))||(job==AE_INTENT&&(fault==3||fault==8))||(job==AE_FINAL&&(fault==4||fault==7))||(job==AE_PROOF&&fault==5))e->fixture_fail=0;else fault=0;payload=e->payload;LeaveCriticalSection(&s->lock);
 if(payload&&s->db&&!s->store_failed){schema=ae_pragma(s->db,"PRAGMA user_version");
  if(job==AE_CHECK){ok=schema==1||schema==2||schema==3;if(schema>=2)recovered=ae_pragma(s->db,"SELECT count(*) FROM ai_edits WHERE terminal=0 OR unknown=1");if(recovered<0)ok=0;}
  else if(job==AE_MIGRATE){ok=ae_migrate_one(s,e,fault);schema=ae_pragma(s->db,"PRAGMA user_version");}
  else ok=ae_store_one(s,e,job,payload,fault);
 }
 EnterCriticalSection(&s->lock);e->schema=schema;e->recovered=recovered;e->busy=0;e->io_ok=ok;e->io_done=job;LeaveCriticalSection(&s->lock);return 1;
}
static int ai_edit_pending(ui_ai_service_t *s)
{ai_edit_t *e=s->edit;return ai_graph_pending(s)||(e&&(e->job||e->busy||e->holding||e->live));}
static void ai_edit_quiesce(ui_ai_service_t *s)
{EnterCriticalSection(&s->lock);if(s->edit){s->edit->cancel=1;s->edit->scope=0;s->edit->approval=0;}for(unsigned i=0;i<AI_RUNS;++i)if(s->runs[i].graph)for(unsigned j=0;j<s->runs[i].graph->count;++j)if(s->runs[i].graph->nodes[j].undo_running){ai_edit_t *e=s->runs[i].graph->nodes[j].edit;e->cancel=1;e->scope=e->approval=0;}LeaveCriticalSection(&s->lock);}
static ui_status_t ae_host_one(ui_ai_service_t *s,ai_edit_t *e,ui_host_t **host)
{ui_app_instance_info_t app={0};ui_status_t status;app.size=sizeof(app);status=ui_workspace_get_instance(s->config.workspace,e->target.resource.instance_id,&app);if(status)return status;*host=app.host;return UI_STATUS_OK;}
static int ae_valid(void *data)
{
 ai_edit_t *e=data;ui_ai_service_t *s=e->service;
 if(s->phase!=UI_AI_READY||e->cancel||!e->scope)return 0;
 if(!e->started&&(ai_now(s)>=e->scope_until||(e->risk!=UI_EDIT_REVERSIBLE&&(!e->approval||ai_now(s)>=e->approval_until))))return 0;
 e->started=1;return 1;
}
static int ae_store_valid(void *data)
{return ai_store_available(((ai_edit_t *)data)->service);}
static void ae_record_one(ui_ai_service_t *s,ai_edit_t *e,ui_cap_effect_v2_t effect,const char *code)
{
 e->effect=effect;snprintf(e->code,sizeof(e->code),"%s",code);e->phase=AE_RECORD;e->scope=e->approval=0;
 if(!ae_queue_one(s,e,AE_FINAL)){e->phase=AE_UNKNOWN;e->effect=UI_CAP2_EFFECT_UNKNOWN;strcpy_s(e->code,sizeof(e->code),"result_queue_unconfirmed");}
}
static void ae_release_one(ui_ai_service_t *s,ai_edit_t *e,int resolved)
{
 ui_host_t *host=NULL;ui_status_t status;
 if(e->call&&ae_host_one(s,e,&host)==UI_STATUS_OK){
  if(!e->call_finished){status=ui_cap_edit_finish(host,e->call,e->effect==UI_CAP2_EFFECT_UNKNOWN?UI_CAP2_UNKNOWN:!strcmp(e->code,"ok")?UI_CAP2_OK:e->cancel?UI_CAP2_CANCELLED:UI_CAP2_FAILED,e->effect,e->code);if(status==UI_STATUS_PENDING)return;if(status==UI_STATUS_OK||status==UI_STATUS_ALREADY_EXISTS)e->call_finished=1;}
  status=ui_cap_edit_release(host,e->call,resolved);if(status==UI_STATUS_PENDING&&e->effect!=UI_CAP2_EFFECT_UNKNOWN)return;
 }
 EnterCriticalSection(&s->lock);e->holding=0;e->live=0;LeaveCriticalSection(&s->lock);
}
/* Storage failure is a cleanup-only mode: no new transaction/stage/job.
   An accepted application stage must explicitly finish its worker/callbacks
   before the execution lease can be released; UNKNOWN keeps the resource gate. */
static void ae_unavailable(ui_ai_service_t *s,ai_edit_t *e)
{
 ui_host_t *host=NULL;ui_cap_edit_completion_t result={0};char rev[128];ui_status_t status;
 if(!e)return;e->cancel=1;e->scope=e->approval=0;
 EnterCriticalSection(&s->lock);if(e->job||e->busy){LeaveCriticalSection(&s->lock);return;}e->io_done=0;LeaveCriticalSection(&s->lock);
 if(!e->holding&&!e->live&&!e->stage)return;
 if(e->call){status=ae_host_one(s,e,&host);if(status)return;
  if(!e->call_finished)(void)ui_capability_cancel_v2(host,e->call);
  if(e->stage){status=ui_cap_edit_result(host,e->call,&result,rev);if(status)return;
   if(result.transaction_id[0])strcpy_s(e->transaction,65,result.transaction_id);
   if(result.undo_id[0])strcpy_s(e->undo_id,65,result.undo_id);
   strcpy_s(e->revision_after,128,rev);
   if(result.status==UI_STATUS_OK){if(e->stage==UI_EDIT_COMMIT)e->committed=1;if(e->stage==UI_EDIT_ROLLBACK)e->rolled_back=1;}
   e->stage=0;
  }
 }
 e->effect=UI_CAP2_EFFECT_UNKNOWN;e->phase=AE_UNKNOWN;strcpy_s(e->code,sizeof(e->code),"storage_unconfirmed");ae_release_one(s,e,0);
}
static void ae_ui_one(ui_ai_service_t *s,ai_edit_t *e)
{
 ui_host_t *host=NULL;int done,ok;ui_status_t status;ui_cap_edit_completion_t result={0};char rev[128];
 if(!e)return;if(!ai_store_available(s)){ae_unavailable(s,e);return;}EnterCriticalSection(&s->lock);done=e->io_done;ok=e->io_ok;if(done)e->io_done=0;LeaveCriticalSection(&s->lock);
 if(done){if(!ok){if(done==AE_CHECK||done==AE_MIGRATE){strcpy_s(e->code,sizeof(e->code),"migration_or_backup_failed");e->phase=AE_IDLE;(void)ae_binding_one(s,e);}
   else{e->effect=UI_CAP2_EFFECT_UNKNOWN;e->phase=AE_UNKNOWN;e->scope=e->approval=0;strcpy_s(e->code,sizeof(e->code),"storage_unconfirmed");ae_release_one(s,e,0);}return;}
  if(done==AE_CHECK||done==AE_MIGRATE){e->phase=AE_IDLE;strcpy_s(e->code,sizeof(e->code),e->schema==1?"migration_confirmation_required":e->recovered?"recovered_unresolved":"ready");(void)ae_binding_one(s,e);return;}
  if(done==AE_PROPOSE){e->proposal_confirmed=ok!=3;if(ok==2){yyjson_doc *d=yyjson_read(e->payload,strlen(e->payload),0);yyjson_val *root=d?yyjson_doc_get_root(d):NULL;const char *text;char *output;
    e->phase=root&&yyjson_is_true(yyjson_obj_get(root,"terminal"))&&yyjson_get_int(yyjson_obj_get(root,"effect"))!=UI_CAP2_EFFECT_UNKNOWN?AE_DONE:AE_UNKNOWN;e->effect=root?(ui_cap_effect_v2_t)yyjson_get_int(yyjson_obj_get(root,"effect")):UI_CAP2_EFFECT_UNKNOWN;
    text=root?yyjson_get_str(yyjson_obj_get(root,"revision_after")):NULL;if(text)strcpy_s(e->revision_after,128,text);text=root?yyjson_get_str(yyjson_obj_get(root,"transaction_id")):NULL;if(text)strcpy_s(e->transaction,65,text);text=root?yyjson_get_str(yyjson_obj_get(root,"undo_id")):NULL;if(text)strcpy_s(e->undo_id,65,text);
    text=root?yyjson_get_str(yyjson_obj_get(root,"code")):NULL;if(text)strcpy_s(e->code,sizeof(e->code),text);output=root?yyjson_val_write(yyjson_obj_get(root,"output"),0,NULL):NULL;if(output){if(strlen(output)<=UI_CAPABILITY_OUTPUT_BYTES)strcpy_s(e->output,sizeof(e->output),output);free(output);}e->committed=root&&yyjson_is_true(yyjson_obj_get(root,"transaction_committed"));e->rolled_back=root&&yyjson_is_true(yyjson_obj_get(root,"rolled_back"));yyjson_doc_free(d);e->scope=e->approval=0;EnterCriticalSection(&s->lock);e->live=0;LeaveCriticalSection(&s->lock);
   }else if(ok==3){e->phase=AE_DONE;e->effect=UI_CAP2_EFFECT_NONE;e->rejected=1;strcpy_s(e->code,sizeof(e->code),"request_id_conflict");EnterCriticalSection(&s->lock);e->live=0;LeaveCriticalSection(&s->lock);}else{e->phase=AE_SCOPE_WAIT;if(e->undo&&e->scope)(void)ae_queue_one(s,e,AE_SCOPE);}return;}
  if(done==AE_SCOPE){e->phase=e->risk==UI_EDIT_REVERSIBLE?AE_GATE:AE_APPROVAL_WAIT;return;}
  if(done==AE_INTENT){if(e->risk!=UI_EDIT_REVERSIBLE&&e->execution_policy!=UI_EDIT_APPROVED_DESTRUCTIVE){ae_record_one(s,e,UI_CAP2_EFFECT_NONE,"effect_not_enabled");return;}e->phase=AE_BASELINE;}
  if(done==AE_FINAL){ar_crash(s,13);e->phase=e->effect==UI_CAP2_EFFECT_UNKNOWN?AE_UNKNOWN:AE_DONE;ae_release_one(s,e,0);return;}
  if(done==AE_PROOF){e->phase=AE_DONE;ae_release_one(s,e,1);strcpy_s(e->code,sizeof(e->code),"reconciled");return;}
 }
 if(e->fixture_pause&&e->fixture_pause==e->phase)return;
 if(e->cancel&&e->stage&&e->cancel_at&&ai_now(s)-e->cancel_at>=5000)strcpy_s(e->code,sizeof(e->code),"cancellation_stalled");
 if(e->phase==AE_DONE&&(e->holding||e->live)){ae_release_one(s,e,0);return;}
 if(e->job||e->busy||e->phase==AE_IDLE||e->phase==AE_DONE||e->phase==AE_RECORD||e->phase==AE_INTENT_WAIT||e->phase==AE_PROPOSE_WAIT)return;
 if(e->phase==AE_UNKNOWN){ae_release_one(s,e,0);return;}
 if(e->cancel&&!e->started&&e->phase!=AE_READBACK){ae_record_one(s,e,UI_CAP2_EFFECT_NONE,"cancelled_before_begin");return;}
 if(e->phase==AE_SCOPE_WAIT||e->phase==AE_APPROVAL_WAIT)return;
 status=ae_host_one(s,e,&host);if(!status&&e->call&&!e->call_finished&&ui_cap_edit_cancelled(host,e->call)){e->cancel=1;e->scope=e->approval=0;}if(!status&&e->cancel&&e->call&&!e->call_finished)(void)ui_capability_cancel_v2(host,e->call);if(status){if(e->started)ae_record_one(s,e,UI_CAP2_EFFECT_UNKNOWN,"target_lost");else ae_record_one(s,e,UI_CAP2_EFFECT_NONE,"target_expired");return;}
 if(e->phase==AE_GATE){
  if(!e->scope||ai_now(s)>=e->scope_until||(e->risk!=UI_EDIT_REVERSIBLE&&(!e->approval||ai_now(s)>=e->approval_until))){ae_record_one(s,e,UI_CAP2_EFFECT_NONE,"authorization_expired");return;}
  status=ui_cap_edit_prepare(host,e->capability,&e->target,e->params,e->descriptor,e->id,&e->call);if(status){ae_record_one(s,e,UI_CAP2_EFFECT_NONE,status==UI_STATUS_PERMISSION_DENIED?"busy_or_unknown":"prepare_rejected");return;}
  if(e->undo)(void)ui_cap_edit_undo_receipt(host,e->call,e->undo_id);
  EnterCriticalSection(&s->lock);e->holding=1;LeaveCriticalSection(&s->lock);(void)ae_binding_one(s,e);e->phase=AE_INTENT_WAIT;if(!ae_queue_one(s,e,AE_INTENT))ae_record_one(s,e,UI_CAP2_EFFECT_UNKNOWN,"intent_queue_failed");return;
 }
 if(e->cancel&&e->started&&!e->committed&&!e->stage&&e->phase==AE_COMMIT)e->phase=AE_ROLLBACK;
 if(!e->stage){ui_cap_edit_stage_t stage=e->phase==AE_BEGIN?UI_EDIT_BEGIN:e->phase==AE_EXECUTE?(e->undo?UI_EDIT_UNDO:UI_EDIT_EXECUTE):e->phase==AE_COMMIT?UI_EDIT_COMMIT:e->phase==AE_ROLLBACK?UI_EDIT_ROLLBACK:e->phase==AE_RECONCILE?UI_EDIT_RECONCILE:UI_EDIT_READBACK;
  if(stage==UI_EDIT_COMMIT)ar_crash(s,7);if(stage==UI_EDIT_READBACK&&e->committed)ar_crash(s,9);
  status=ui_cap_edit_invoke(host,e->call,stage,stage==UI_EDIT_BEGIN||stage==UI_EDIT_EXECUTE||stage==UI_EDIT_UNDO?ae_valid:ae_store_valid,e);
  if(status==UI_STATUS_PENDING)return;if(status){if(e->started&&e->transaction[0]&&stage!=UI_EDIT_ROLLBACK){e->phase=AE_ROLLBACK;e->stage=0;return;}ae_record_one(s,e,e->started?UI_CAP2_EFFECT_UNKNOWN:UI_CAP2_EFFECT_NONE,"dispatch_rejected");return;}e->stage=stage;
 }
 if(!ai_store_available(s)){ae_unavailable(s,e);return;}status=ui_cap_edit_result(host,e->call,&result,rev);if(status==UI_STATUS_PENDING){if(e->stage==UI_EDIT_EXECUTE)ar_crash(s,6);return;}if(status){ae_record_one(s,e,UI_CAP2_EFFECT_UNKNOWN,"stage_result_unavailable");return;}
 {ui_cap_edit_stage_t stage=e->stage;e->stage=0;if(stage==UI_EDIT_BEGIN&&result.transaction_id[0])strcpy_s(e->transaction,sizeof(e->transaction),result.transaction_id);if((stage==UI_EDIT_COMMIT||stage==UI_EDIT_RECONCILE)&&result.undo_id[0])strcpy_s(e->undo_id,sizeof(e->undo_id),result.undo_id);strcpy_s(e->revision_after,sizeof(e->revision_after),rev);
  if(result.status!=UI_STATUS_OK){if(stage==UI_EDIT_RECONCILE){e->phase=AE_UNKNOWN;strcpy_s(e->code,sizeof(e->code),"reconciliation_failed");ae_release_one(s,e,0);return;}if((stage==UI_EDIT_BEGIN||stage==UI_EDIT_EXECUTE||stage==UI_EDIT_UNDO)&&e->transaction[0])e->phase=AE_ROLLBACK;else ae_record_one(s,e,UI_CAP2_EFFECT_UNKNOWN,"application_stage_failed");return;}
  if(stage==UI_EDIT_BEGIN){if(!e->transaction[0]){ae_record_one(s,e,UI_CAP2_EFFECT_UNKNOWN,"missing_transaction_receipt");return;}ar_crash(s,5);e->phase=e->cancel?AE_ROLLBACK:AE_EXECUTE;}
  else if(stage==UI_EDIT_EXECUTE||stage==UI_EDIT_UNDO)e->phase=e->cancel?AE_ROLLBACK:AE_COMMIT;
  else if(stage==UI_EDIT_COMMIT){ar_crash(s,8);e->committed=1;e->phase=AE_READBACK;}
  else if(stage==UI_EDIT_ROLLBACK){e->rolled_back=1;e->phase=AE_READBACK;}
  else if(stage==UI_EDIT_RECONCILE){if(!result.executor_exited||!result.output_json||result.effect==UI_CAP2_EFFECT_UNKNOWN){e->phase=AE_UNKNOWN;return;}strcpy_s(e->output,sizeof(e->output),result.output_json);e->effect=result.effect;strcpy_s(e->code,sizeof(e->code),"reconciled");e->phase=AE_RECORD;if(!ae_queue_one(s,e,AE_PROOF)){e->phase=AE_UNKNOWN;}return;}
  else if(stage==UI_EDIT_READBACK){
   if(e->phase==AE_BASELINE){if(!result.output_json||strcmp(rev,e->target.expected_revision)||ui_cap_json_fingerprint(result.output_json,strlen(result.output_json),e->before_hash)){ae_record_one(s,e,UI_CAP2_EFFECT_NONE,"baseline_unavailable");return;}e->phase=AE_BEGIN;return;}
   if(!result.output_json||(e->committed&&(!e->undo_id[0]||!strcmp(rev,e->target.expected_revision)))||result.effect==UI_CAP2_EFFECT_UNKNOWN){ae_record_one(s,e,UI_CAP2_EFFECT_UNKNOWN,"readback_unproven");return;}
   {char proof[65];if(!e->committed&&(ui_cap_json_fingerprint(result.output_json,strlen(result.output_json),proof)||strcmp(proof,e->before_hash))){ae_record_one(s,e,UI_CAP2_EFFECT_UNKNOWN,"rollback_readback_mismatch");return;}}
   strcpy_s(e->output,sizeof(e->output),result.output_json);ar_crash(s,10);ae_record_one(s,e,e->committed?UI_CAP2_EFFECT_APPLIED:UI_CAP2_EFFECT_NONE,e->committed?"ok":e->rolled_back?"rolled_back":"not_applied");
  }
 }
}
ui_status_t ui_ai_edit_enable(ui_ai_service_t *s)
{
 ai_edit_t *e;if(!ai_ui(s))return UI_STATUS_INVALID_ARGUMENT;if(s->callbacks||s->polling||s->phase!=UI_AI_READY)return UI_STATUS_PENDING;if(s->edit){char nonce[37];e=s->edit;if(e->schema!=1||e->phase!=AE_IDLE||e->job||e->busy||!ai_uuid(nonce))return UI_STATUS_ALREADY_EXISTS;
  if(snprintf(e->backup,sizeof(e->backup),"%s.schema1-%s.sqlite",s->path,nonce)<0)return UI_STATUS_LIMIT_EXCEEDED;(void)ae_binding_one(s,e);return ae_queue_one(s,e,AE_CHECK)?UI_STATUS_OK:UI_STATUS_LIMIT_EXCEEDED;}
 if(sizeof(*e)>UI_AI_MAX_BYTES-s->bytes)return UI_STATUS_LIMIT_EXCEEDED;e=calloc(1,sizeof(*e));if(!e)return UI_STATUS_OUT_OF_MEMORY;
 if(snprintf(e->backup,sizeof(e->backup),"%s.schema1-%s.sqlite",s->path,s->session)<0||strlen(s->path)+strlen(s->session)+17>=sizeof(e->backup)){free(e);return UI_STATUS_LIMIT_EXCEEDED;}
 EnterCriticalSection(&s->lock);e->service=s;s->edit=e;s->bytes+=sizeof(*e);strcpy_s(e->storage_path,sizeof(e->storage_path),s->path);LeaveCriticalSection(&s->lock);(void)ae_binding_one(s,e);if(!ae_queue_one(s,e,AE_CHECK)){EnterCriticalSection(&s->lock);s->bytes-=sizeof(*e);s->edit=NULL;LeaveCriticalSection(&s->lock);free(e);return UI_STATUS_PLATFORM_ERROR;}return UI_STATUS_OK;
}
ui_status_t ui_ai_edit_get_state(ui_ai_service_t *s,char **out)
{if(out)*out=NULL;if(!ai_ui(s)||!out)return UI_STATUS_INVALID_ARGUMENT;if(!s->edit)return UI_STATUS_NOT_FOUND;*out=ae_snapshot(s->edit,s->edit->phase);return *out?UI_STATUS_OK:UI_STATUS_OUT_OF_MEMORY;}
static ui_status_t ae_propose_one(ui_ai_service_t *s,ai_edit_t *e,const ui_ai_edit_request_t *q,char out[65],int delegated)
{
 ui_app_instance_info_t app={0};char *description=NULL,*params=NULL,*constraint=NULL,*json=NULL,*canonical=NULL;size_t n;yyjson_doc *d=NULL;ui_status_t status;char request_hash[65],id[65];
 if(out)out[0]=0;if(!ai_ui(s)||!q||q->size<sizeof(*q)||q->version!=1||!out||!ai_id(q->client_request_id)||!ai_text(q->capability_id,255,0)||q->target.size<sizeof(q->target)||q->target.version!=2)return UI_STATUS_INVALID_ARGUMENT;
 if(q->target.resource.size<sizeof(q->target.resource)||q->target.resource.version!=2||!memchr(q->target.resource.host_epoch,0,37)||!memchr(q->target.resource.resource_id,0,256)||!memchr(q->target.resource.open_epoch,0,37)||!memchr(q->target.expected_revision,0,128)||!memchr(q->target.scope_json,0,sizeof(q->target.scope_json))||strcmp(q->target.scope_json,"{\"access\":\"write\"}")||!ai_text(q->params_json,UI_CAPABILITY_INPUT_BYTES,0)||!ai_text(q->parameter_constraint_schema,65536,0))return UI_STATUS_INVALID_ARGUMENT;
 if(!e||(e->schema!=2&&e->schema!=3))return UI_STATUS_UNSUPPORTED;if(s->phase!=UI_AI_READY||s->callbacks||(!delegated&&s->polling))return UI_STATUS_PENDING;
 
 ++s->methods;app.size=sizeof(app);status=ui_workspace_get_instance(s->config.workspace,q->target.resource.instance_id,&app);if(status)goto done;
 status=ui_capability_describe_v2(app.host,q->capability_id,&description);if(status)goto done;d=yyjson_read(description,strlen(description),0);
 if(!d||yyjson_get_int(yyjson_obj_get(yyjson_doc_get_root(d),"edit_contract"))!=1){status=UI_STATUS_PERMISSION_DENIED;goto done;}
 status=ui_cap_schema_canonical(q->parameter_constraint_schema,&constraint);if(status)goto done;status=ui_cap_json_validate(constraint,q->params_json,q->params_json?strlen(q->params_json):0);if(status)goto done;
 status=ui_cap_json_canonicalize(q->params_json,strlen(q->params_json),&params,&n);if(status)goto done;
 /* Hash the individually validated canonical Schema/parameters, then JCS
    the fixed identity envelope. This does not lower their independent 64KiB
    limits by wrapping both in a second 64KiB input parser. */
 {yyjson_mut_doc *m=yyjson_mut_doc_new(NULL);yyjson_mut_val *o=m?yyjson_mut_obj(m):NULL;char params_hash[65],scope_hash[65],instance[32];
  if(!o){yyjson_mut_doc_free(m);status=UI_STATUS_OUT_OF_MEMORY;goto done;}yyjson_mut_doc_set_root(m,o);
  status=ui_cap_hash_bytes(params,strlen(params),params_hash);if(!status)status=ui_cap_hash_bytes(constraint,strlen(constraint),scope_hash);if(status){yyjson_mut_doc_free(m);goto done;}
  yyjson_mut_obj_add_strcpy(m,o,"client",q->client_request_id);yyjson_mut_obj_add_strcpy(m,o,"capability",q->capability_id);yyjson_mut_obj_add_strcpy(m,o,"descriptor",yyjson_get_str(yyjson_obj_get(yyjson_doc_get_root(d),"descriptor_hash")));yyjson_mut_obj_add_strcpy(m,o,"policy","p4a-1");
  snprintf(instance,sizeof(instance),"%llu",(unsigned long long)q->target.resource.instance_id);yyjson_mut_obj_add_strcpy(m,o,"instance",instance);yyjson_mut_obj_add_int(m,o,"kind",q->target.resource.kind);
  yyjson_mut_obj_add_strcpy(m,o,"host_epoch",q->target.resource.host_epoch);yyjson_mut_obj_add_strcpy(m,o,"resource",q->target.resource.resource_id);yyjson_mut_obj_add_strcpy(m,o,"open_epoch",q->target.resource.open_epoch);yyjson_mut_obj_add_strcpy(m,o,"revision",q->target.expected_revision);yyjson_mut_obj_add_strcpy(m,o,"scope",q->target.scope_json);
  yyjson_mut_obj_add_strcpy(m,o,"params_sha256",params_hash);yyjson_mut_obj_add_strcpy(m,o,"constraint_sha256",scope_hash);json=yyjson_mut_write(m,0,NULL);yyjson_mut_doc_free(m);
 }
 if(!json){status=UI_STATUS_OUT_OF_MEMORY;goto done;}status=ui_cap_json_fingerprint(json,strlen(json),request_hash);if(status)goto done;
 {yyjson_mut_doc *m=yyjson_mut_doc_new(NULL);yyjson_mut_val *o=m?yyjson_mut_obj(m):NULL;if(!o){yyjson_mut_doc_free(m);status=UI_STATUS_OUT_OF_MEMORY;goto done;}yyjson_mut_doc_set_root(m,o);yyjson_mut_obj_add_strcpy(m,o,"conversation",s->conversation);yyjson_mut_obj_add_strcpy(m,o,"edit_client",q->client_request_id);canonical=yyjson_mut_write(m,0,NULL);yyjson_mut_doc_free(m);if(!canonical){status=UI_STATUS_OUT_OF_MEMORY;goto done;}status=ui_cap_json_fingerprint(canonical,strlen(canonical),id);if(status)goto done;}
 if(!strcmp(e->id,id)){status=strcmp(e->hash,request_hash)?UI_STATUS_ALREADY_EXISTS:UI_STATUS_OK;if(!status)strcpy_s(out,65,id);goto done;}
 if(e->job||e->busy||e->holding||(!delegated&&(s->active>=0||s->queued))||e->recovered||(e->id[0]&&e->phase!=AE_DONE)){status=UI_STATUS_PENDING;goto done;}
 e->target=q->target;strcpy_s(e->client,65,q->client_request_id);strcpy_s(e->capability,256,q->capability_id);strcpy_s(e->params,sizeof(e->params),params);strcpy_s(e->constraint,sizeof(e->constraint),constraint);strcpy_s(e->descriptor,65,yyjson_get_str(yyjson_obj_get(yyjson_doc_get_root(d),"descriptor_hash")));e->risk=yyjson_get_int(yyjson_obj_get(yyjson_doc_get_root(d),"risk"));e->execution_policy=yyjson_get_int(yyjson_obj_get(yyjson_doc_get_root(d),"execution_policy"));{const char *side=yyjson_get_str(yyjson_obj_get(yyjson_doc_get_root(d),"side_effect"));strcpy_s(e->side_effect,64,side?side:"memory-document");}strcpy_s(e->id,65,id);strcpy_s(e->hash,65,request_hash);
 e->proposal_confirmed=0;e->rejected=0;e->call=0;e->scope=e->approval=e->cancel=e->started=e->committed=e->rolled_back=e->undo=e->call_finished=0;e->scope_until=e->approval_until=e->cancel_at=0;e->effect=UI_CAP2_EFFECT_NONE;e->stage=0;e->transaction[0]=e->undo_id[0]=e->output[0]=e->code[0]=0;e->phase=AE_PROPOSE_WAIT;e->before_hash[0]=0;
 EnterCriticalSection(&s->lock);e->live=1;LeaveCriticalSection(&s->lock);
 status=ae_binding_one(s,e);if(!status){strcpy_s(out,65,id);if(!ae_queue_one(s,e,AE_PROPOSE))status=UI_STATUS_LIMIT_EXCEEDED;}
 if(status){out[0]=0;e->id[0]=0;e->phase=AE_IDLE;EnterCriticalSection(&s->lock);e->live=0;LeaveCriticalSection(&s->lock);}
done:--s->methods;free(description);free(params);free(constraint);free(json);free(canonical);yyjson_doc_free(d);return status;
}
ui_status_t ui_ai_edit_cancel(ui_ai_service_t *s,const char *id)
{ui_host_t *host=NULL;if(!ai_ui(s)||!ai_id(id))return UI_STATUS_INVALID_ARGUMENT;if(!s->edit||strcmp(s->edit->id,id))return UI_STATUS_NOT_FOUND;++s->methods;s->edit->cancel=1;if(!s->edit->cancel_at)s->edit->cancel_at=ai_now(s);s->edit->scope=s->edit->approval=0;if(s->edit->call&&ae_host_one(s,s->edit,&host)==UI_STATUS_OK)(void)ui_capability_cancel_v2(host,s->edit->call);--s->methods;return UI_STATUS_OK;}
ui_status_t ui_ai_edit_forget(ui_ai_service_t *s,const char *id)
{ai_edit_t *e;if(!ai_ui(s)||!ai_id(id))return UI_STATUS_INVALID_ARGUMENT;e=s->edit;if(!e||strcmp(e->id,id))return UI_STATUS_NOT_FOUND;if(e->phase!=AE_DONE||e->job||e->busy||e->holding)return UI_STATUS_PENDING;e->id[0]=0;e->phase=AE_IDLE;return UI_STATUS_OK;}
static ui_status_t ae_action_one(ui_ai_service_t *s,ai_edit_t *e,ui_ai_edit_action_t action,const char *binding)
{
 if(!e||strcmp(binding,e->binding))return UI_STATUS_PERMISSION_DENIED;if(e->job||e->busy||s->phase!=UI_AI_READY)return UI_STATUS_PENDING;
 if(action==UI_AI_EDIT_MIGRATE){if(e->schema!=1||s->active>=0||s->queued||s->cmd_count)return UI_STATUS_PERMISSION_DENIED;return ae_queue_one(s,e,AE_MIGRATE)?UI_STATUS_OK:UI_STATUS_LIMIT_EXCEEDED;}
 if(action==UI_AI_EDIT_CONFIRM){if(e->phase!=AE_SCOPE_WAIT||e->scope||e->cancel)return UI_STATUS_PERMISSION_DENIED;e->scope=1;e->scope_until=ai_now(s)+UI_AI_EDIT_SCOPE_MS;if(ae_queue_one(s,e,AE_SCOPE))return UI_STATUS_OK;e->scope=0;e->scope_until=0;return UI_STATUS_LIMIT_EXCEEDED;}
 if(action==UI_AI_EDIT_APPROVE){if(e->phase!=AE_APPROVAL_WAIT||e->approval||!e->scope||ai_now(s)>=e->scope_until)return UI_STATUS_PERMISSION_DENIED;e->approval=1;e->approval_until=ai_now(s)+UI_AI_EDIT_APPROVAL_MS;e->phase=AE_GATE;return UI_STATUS_OK;}
 if(action==UI_AI_EDIT_RECONCILE){ui_host_t *host=NULL;if(e->phase!=AE_UNKNOWN||!e->call||e->job||e->busy||ae_host_one(s,e,&host))return UI_STATUS_PERMISSION_DENIED;EnterCriticalSection(&s->lock);e->live=e->holding=1;LeaveCriticalSection(&s->lock);e->phase=AE_RECONCILE;e->stage=0;e->reconciling=1;return UI_STATUS_OK;}
 if(action==UI_AI_EDIT_UNDO){if(e->phase!=AE_DONE||e->effect!=UI_CAP2_EFFECT_APPLIED||!e->undo_id[0])return UI_STATUS_PERMISSION_DENIED;
  /* A new explicit operation, bound to the previous confirmed revision/receipt. */
  {char key[160],id[65];snprintf(key,sizeof(key),"{\"parent\":\"%s\",\"undo\":true}",e->id);if(ui_cap_json_fingerprint(key,strlen(key),id))return UI_STATUS_VALIDATION_FAILED;strcpy_s(e->id,65,id);snprintf(e->client,65,"undo-%.59s",id);}
  strcpy_s(e->target.expected_revision,128,e->revision_after);EnterCriticalSection(&s->lock);e->live=1;LeaveCriticalSection(&s->lock);e->proposal_confirmed=0;e->effect=UI_CAP2_EFFECT_NONE;e->cancel_at=0;e->undo=1;e->call=e->started=e->committed=e->rolled_back=e->call_finished=e->cancel=0;e->scope=1;e->approval=e->risk==UI_EDIT_REVERSIBLE;e->approval_until=0;e->scope_until=ai_now(s)+UI_AI_EDIT_SCOPE_MS;e->phase=AE_PROPOSE_WAIT;e->stage=0;e->output[0]=0;(void)ae_binding_one(s,e);if(ae_queue_one(s,e,AE_PROPOSE))return UI_STATUS_OK;e->phase=AE_DONE;e->rejected=1;e->scope=e->approval=0;strcpy_s(e->code,sizeof(e->code),"receipt_queue_failed");EnterCriticalSection(&s->lock);e->live=0;LeaveCriticalSection(&s->lock);return UI_STATUS_LIMIT_EXCEEDED;
 }return UI_STATUS_UNSUPPORTED;
}
static LRESULT CALLBACK ae_button_proc(HWND window,UINT message,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR data)
{
 ai_edit_button_t *b=(ai_edit_button_t *)data;ui_ai_service_t *s=b->service;int accept=0;(void)id;++s->methods;
 if(message==WM_LBUTTONDOWN){b->pressed=ui_inputs_current_source();b->modifiers=(uint32_t)wp&(MK_SHIFT|MK_CONTROL);}
 if((message==WM_MOUSEMOVE||message==WM_KEYUP)&&!ui_inputs_current_source())b->pressed=0;
 if(message==WM_LBUTTONUP){RECT r;GetClientRect(window,&r);accept=b->pressed&&b->modifiers==((uint32_t)wp&(MK_SHIFT|MK_CONTROL))&&ui_inputs_current_source()&&(short)LOWORD(lp)>=0&&(short)HIWORD(lp)>=0&&(short)LOWORD(lp)<r.right&&(short)HIWORD(lp)<r.bottom;b->pressed=0;}
 if(message==WM_KEYDOWN){b->pressed=0;accept=(wp==VK_RETURN||wp==VK_SPACE)&&!(lp&(1L<<30))&&ui_inputs_current_source();}
 if(message==WM_KILLFOCUS||message==WM_CANCELMODE||message==WM_CAPTURECHANGED)b->pressed=0;
 if(accept&&!b->used&&IsWindowEnabled(window)&&IsWindowVisible(window)){b->used=1;s->edit->host_status=(int)ae_action_one(s,s->edit,b->action,b->binding);}
 if(message==WM_NCDESTROY)b->window=NULL;--s->methods;
 if(message==BM_CLICK||message==WM_LBUTTONUP||message==WM_KEYDOWN)return 0;return DefSubclassProc(window,message,wp,lp);
}
ui_status_t ui_ai_edit_bind_button(ui_ai_service_t *s,void *button,ui_ai_edit_action_t action,const char *hash,uint64_t *id)
{
 ai_edit_t *e;ai_edit_button_t *b;wchar_t class_name[32];unsigned count=0;if(id)*id=0;if(!ai_ui(s)||!id||!button||!hash||strlen(hash)!=64||action<UI_AI_EDIT_CONFIRM||action>UI_AI_EDIT_MIGRATE)return UI_STATUS_INVALID_ARGUMENT;e=s->edit;if(!GetClassNameW((HWND)button,class_name,32)||_wcsicmp(class_name,L"Button"))return UI_STATUS_INVALID_ARGUMENT;if(!e||strcmp(e->binding,hash)||GetWindowThreadProcessId((HWND)button,NULL)!=s->thread)return UI_STATUS_PERMISSION_DENIED;
 for(b=e->buttons;b;b=b->next){++count;if(b->window==(HWND)button)return UI_STATUS_ALREADY_EXISTS;}if(count==16||sizeof(*b)>UI_AI_MAX_BYTES-s->bytes)return UI_STATUS_LIMIT_EXCEEDED;
 b=calloc(1,sizeof(*b));if(!b)return UI_STATUS_OUT_OF_MEMORY;b->service=s;b->window=(HWND)button;b->action=action;b->id=++e->button_sequence;strcpy_s(b->binding,65,hash);
 if(!SetWindowSubclass(b->window,ae_button_proc,(UINT_PTR)b->id,(DWORD_PTR)b)){free(b);return UI_STATUS_PLATFORM_ERROR;}b->next=e->buttons;e->buttons=b;EnterCriticalSection(&s->lock);s->bytes+=sizeof(*b);LeaveCriticalSection(&s->lock);*id=b->id;return UI_STATUS_OK;
}
ui_status_t ui_ai_edit_unbind_button(ui_ai_service_t *s,uint64_t id)
{ai_edit_button_t **p,*b;if(!ai_ui(s)||!s->edit)return UI_STATUS_INVALID_ARGUMENT;if(s->methods||s->polling)return UI_STATUS_PENDING;for(p=&s->edit->buttons;*p&&(*p)->id!=id;p=&(*p)->next){}if(!*p)return UI_STATUS_NOT_FOUND;b=*p;*p=b->next;if(b->window)RemoveWindowSubclass(b->window,ae_button_proc,(UINT_PTR)b->id);EnterCriticalSection(&s->lock);s->bytes-=sizeof(*b);LeaveCriticalSection(&s->lock);free(b);return UI_STATUS_OK;}
ui_status_t ui_ai_edit_bind_target(ui_ai_service_t *s,uint64_t instance,const char *resource,ui_cap_target_v2_t *out)
{
 ui_app_instance_info_t app={0};ui_cap_target_t read={0};ui_cap_target_v2_t target={0};ui_status_t status;
 if(!ai_ui(s)||!out||out->size<sizeof(*out)||out->version!=2||!instance||!ai_text(resource,255,0))return UI_STATUS_INVALID_ARGUMENT;
 if(s->phase!=UI_AI_READY)return UI_STATUS_CANCELLED;++s->methods;app.size=sizeof(app);status=ui_workspace_get_instance(s->config.workspace,instance,&app);
 if(!status&&app.closing)status=UI_STATUS_CANCELLED;
 if(!status){read.size=sizeof(read);read.version=1;status=ui_capability_bind(app.host,resource,&read);}
 if(!status){app.size=sizeof(app);status=ui_workspace_get_instance(s->config.workspace,instance,&app);if(!status&&(app.closing||s->phase!=UI_AI_READY))status=UI_STATUS_CANCELLED;}
 if(!status){target.size=sizeof(target);target.version=2;target.resource.size=sizeof(target.resource);target.resource.version=2;status=ui_capability_resource_ref_v2(app.host,resource,&target.resource);if(!status){strcpy_s(target.expected_revision,128,read.expected_revision);strcpy_s(target.scope_json,sizeof(target.scope_json),"{\"access\":\"write\"}");*out=target;}}
 --s->methods;return status;
}
ui_status_t ui_ai_edit_bind_light(ui_ai_service_t *s,const ui_ai_edit_light_binding_t *d,uint64_t *id)
{
 ai_edit_t *e;ai_edit_button_t *b;unsigned count=0;if(id)*id=0;
 if(!ai_ui(s)||!id||!d||d->size<sizeof(*d)||d->version!=1||!d->view||!ai_text(d->control_id,127,0)||!ai_text(d->binding_hash,64,0)||d->action<UI_AI_EDIT_CONFIRM||d->action>UI_AI_EDIT_MIGRATE)return UI_STATUS_INVALID_ARGUMENT;
 e=s->edit;if(!e||strcmp(e->binding,d->binding_hash)||d->view->input_workspace!=s->config.workspace||!d->view->backend->trusted_light)return UI_STATUS_PERMISSION_DENIED;
 for(b=e->buttons;b;b=b->next){++count;if(b->view==d->view&&!strcmp(b->control,d->control_id))return UI_STATUS_ALREADY_EXISTS;}
 if(count==16||sizeof(*b)>UI_AI_MAX_BYTES-s->bytes||e->button_sequence==UINT64_MAX)return UI_STATUS_LIMIT_EXCEEDED;
 b=calloc(1,sizeof(*b));if(!b)return UI_STATUS_OUT_OF_MEMORY;b->service=s;b->view=d->view;b->action=d->action;b->id=++e->button_sequence;strcpy_s(b->binding,65,d->binding_hash);strcpy_s(b->control,sizeof(b->control),d->control_id);b->next=e->buttons;e->buttons=b;
 EnterCriticalSection(&s->lock);s->bytes+=sizeof(*b);LeaveCriticalSection(&s->lock);*id=b->id;return UI_STATUS_OK;
}
ui_status_t ui_ai_edit_light_action(ui_ai_service_t *s,ui_web_view_t *view,const char *node)
{
 ai_edit_button_t *b;ui_status_t status;if(!ai_ui(s)||!s->edit)return UI_STATUS_UNSUPPORTED;
 for(b=s->edit->buttons;b;b=b->next)if(b->view==view&&!strcmp(b->control,node))break;if(!b)return UI_STATUS_UNSUPPORTED;
 ++s->methods;
 if(!ui_inputs_consume_action(view->host,view->user_data)||b->used||view->input_workspace!=s->config.workspace||(view->private_input.kind!=UI_INPUT_POINTER_UP&&view->private_input.kind!=UI_INPUT_KEY_DOWN)||(view->private_input.kind==UI_INPUT_POINTER_UP?view->private_input.pointer_button!=1:(view->private_input.key_code!=13&&view->private_input.key_code!=32)))status=UI_STATUS_PERMISSION_DENIED;
 else{b->used=1;status=ae_action_one(s,s->edit,b->action,b->binding);}
 s->edit->host_status=(int)status;--s->methods;return status;
}
void ui_ai_edit_light_detach(ui_ai_service_t *s,ui_web_view_t *view)
{ai_edit_button_t *b;if(!ai_ui(s)||!s->edit)return;for(b=s->edit->buttons;b;b=b->next)if(b->view==view){b->view=NULL;b->used=1;}}
static void ai_edit_dispose(ui_ai_service_t *s)
{ai_edit_t *e=s->edit;if(!e)return;while(e->buttons){ai_edit_button_t *b=e->buttons;e->buttons=b->next;if(b->window)RemoveWindowSubclass(b->window,ae_button_proc,(UINT_PTR)b->id);free(b);}free(e);s->edit=NULL;}
#ifdef UI_AI_FIXTURE
extern void ui_capabilities_human_begin(ui_host_t *,void *);
extern void ui_capabilities_human_end(ui_host_t *);
extern ui_status_t ui_capabilities_ui_action(ui_host_t *,const char *,void *);
#ifdef UI_AI_EDIT_TEST_DLL
__declspec(dllexport) ui_status_t ui_ai_edit_fixture_form(ui_host_t *host,const char *component,const char *field,const char *value)
{ui_component_t *c=ui_component_find(host,component);ui_web_view_t *v;ui_status_t status;if(!c)return UI_STATUS_NOT_FOUND;if(field){ui_components_form_draft(c,field,value);return UI_STATUS_OK;}v=ui_components_view(c);if(!v)return UI_STATUS_NOT_FOUND;ui_capabilities_human_begin(host,v->user_data);status=ui_components_form_action(c,value);ui_capabilities_human_end(host);return status;}
__declspec(dllexport) ui_status_t ui_ai_edit_fixture_light(ui_ai_service_t *s,ui_web_view_t *view,const char *node)
{ui_status_t status;view->private_input.size=sizeof(view->private_input);view->private_input.kind=UI_INPUT_POINTER_UP;view->private_input.pointer_button=1;ui_capabilities_human_begin(view->host,view->user_data);status=ui_ai_edit_light_action(s,view,node);ui_capabilities_human_end(view->host);return status;}
#endif
#ifdef UI_AI_EDIT_TEST_DLL
__declspec(dllexport)
#endif
ui_status_t ui_ai_edit_fixture_manual(ui_host_t *host,const char *item,void *owner)
{ui_status_t status;ui_capabilities_human_begin(host,owner);status=ui_capabilities_ui_action(host,item,owner);ui_capabilities_human_end(host);return status;}
#ifdef UI_AI_EDIT_TEST_DLL
__declspec(dllexport)
#endif
ui_status_t ui_ai_edit_fixture_action(ui_ai_service_t *s,int action,const char *binding)
{ui_status_t status;if(!ai_ui(s))return UI_STATUS_INVALID_ARGUMENT;++s->methods;status=ae_action_one(s,s->edit,(ui_ai_edit_action_t)action,binding);--s->methods;return status;}
#ifdef UI_AI_EDIT_TEST_DLL
__declspec(dllexport)
#endif
void ui_ai_edit_fixture(ui_ai_service_t *s,int mode,uint64_t value)
{EnterCriticalSection(&s->lock);if(mode==1)s->edit->fixture_hold=(int)value;if(mode==2)s->edit->fixture_fail=(int)value;if(mode==3)s->time_bias+=value;if(mode==4)s->edit->fixture_pause=(int)value;if(mode==5)InterlockedExchange(&ar_disk_fault,(LONG)value);if(mode==6)InterlockedExchange(&s->fixture_crash,(LONG)value);if(mode==7)InterlockedExchange(&s->fixture_checkpoint,(LONG)value);(void)ai_start_locked(s);LeaveCriticalSection(&s->lock);}
#endif

static int ai_edit_io(ui_ai_service_t *s){return ae_io_one(s,s->edit)|ai_graph_io(s);}
static void ai_edit_ui(ui_ai_service_t *s){ae_ui_one(s,s->edit);ai_graph_ui(s);}
ui_status_t ui_ai_edit_propose(ui_ai_service_t *s,const ui_ai_edit_request_t *q,char out[65])
{return ae_propose_one(s,s?s->edit:NULL,q,out,0);}
