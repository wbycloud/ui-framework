/* P5-1. One Coordinator, using the existing Service worker and P4 executor. */
static const char *ag_str(yyjson_val *o,const char *key){const char *s=yyjson_get_str(yyjson_obj_get(o,key));return s?s:"";}
static int ag_index(ai_graph_t *g,const char *id){for(unsigned i=0;i<g->count;++i)if(!strcmp(g->nodes[i].id,id))return (int)i;return -1;}
static int ag_cycle(ai_graph_t *g,unsigned at,uint32_t path,uint32_t *done)
{uint32_t bit=UINT32_C(1)<<at,edges=g->nodes[at].dependencies;if(path&bit)return 1;if(*done&bit)return 0;if(g->nodes[at].parent_index>=0)edges|=UINT32_C(1)<<g->nodes[at].parent_index;for(unsigned i=0;i<g->count;++i)if((edges&(UINT32_C(1)<<i))&&ag_cycle(g,i,path|bit,done))return 1;*done|=bit;return 0;}
static char *ai_graph_targets_json(ai_graph_t *g)
{yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *a=d?yyjson_mut_arr(d):NULL;char *json;if(!a){yyjson_mut_doc_free(d);return NULL;}yyjson_mut_doc_set_root(d,a);for(size_t i=0;i<g->target_count;++i){const ui_cap_target_v2_t *t=&g->targets[i];yyjson_mut_val *o=yyjson_mut_obj(d);char instance[32];snprintf(instance,32,"%llu",(unsigned long long)t->resource.instance_id);yyjson_mut_obj_add_strcpy(d,o,"instance_id",instance);yyjson_mut_obj_add_uint(d,o,"kind",t->resource.kind);yyjson_mut_obj_add_strcpy(d,o,"host_epoch",t->resource.host_epoch);yyjson_mut_obj_add_strcpy(d,o,"resource_id",t->resource.resource_id);yyjson_mut_obj_add_strcpy(d,o,"open_epoch",t->resource.open_epoch);yyjson_mut_obj_add_strcpy(d,o,"revision",t->expected_revision);yyjson_mut_obj_add_strcpy(d,o,"scope",t->scope_json);yyjson_mut_arr_add_val(a,o);}json=yyjson_mut_write(d,0,NULL);yyjson_mut_doc_free(d);return json;}
static ui_status_t ai_graph_request_hash(const char *input,const char *graph,const char *targets,const char *parent,char hash[65])
{char a[65],b[65],c[65],key[330];ui_status_t status=ui_cap_hash_bytes(input,strlen(input),a);if(!status)status=ui_cap_hash_bytes(graph,strlen(graph),b);if(!status)status=ui_cap_hash_bytes(targets,strlen(targets),c);if(status)return status;snprintf(key,sizeof(key),"{\"input\":\"%s\",\"graph\":\"%s\",\"targets\":\"%s\",\"parent\":\"%s\"}",a,b,c,parent?parent:"");return ui_cap_json_fingerprint(key,strlen(key),hash);}
static int ag_target_shape(const ui_cap_target_v2_t *t)
{return t&&t->size>=sizeof(*t)&&t->version==2&&t->resource.size>=sizeof(t->resource)&&t->resource.version==2&&memchr(t->resource.host_epoch,0,37)&&memchr(t->resource.open_epoch,0,37)&&memchr(t->resource.resource_id,0,256)&&memchr(t->expected_revision,0,128)&&memchr(t->scope_json,0,sizeof(t->scope_json));}
static int ai_graph_continue_valid(ui_ai_service_t *s,ai_run_t *r)
{sqlite3_stmt *q=NULL;yyjson_doc *d=NULL;yyjson_val *nodes,*node;size_t i,count;int ok=1;if(!r->graph)return 1;if(!ai_statement(s,"SELECT payload FROM ai_graphs WHERE run_id=?",&q))return 0;ai_bind(q,1,r->parent);if(sqlite3_step(q)!=SQLITE_ROW){sqlite3_finalize(q);return 0;}d=yyjson_read((const char *)sqlite3_column_text(q,0),(size_t)sqlite3_column_bytes(q,0),0);nodes=d?yyjson_obj_get(yyjson_doc_get_root(d),"nodes"):NULL;if(!yyjson_is_arr(nodes))ok=0;yyjson_arr_foreach(nodes,i,count,node){if(yyjson_is_true(yyjson_obj_get(node,"charged"))){const char *op=ag_str(node,"operation_id");for(unsigned j=0;j<r->graph->count;++j)if(!strcmp(op,r->graph->nodes[j].operation))ok=0;}}yyjson_doc_free(d);sqlite3_finalize(q);return ok;}
static int ag_has(const char *json,const char *id)
{yyjson_doc *d=yyjson_read(json,strlen(json),0);yyjson_val *a=d?yyjson_doc_get_root(d):NULL,*v;size_t i,n;int found=0;yyjson_arr_foreach(a,i,n,v)if(yyjson_is_str(v)&&!strcmp(yyjson_get_str(v),id))found=1;yyjson_doc_free(d);return found;}
static ui_status_t ag_app(ui_ai_service_t *s,const ui_cap_target_v2_t *t,ui_app_instance_info_t *app)
{ui_status_t status;memset(app,0,sizeof(*app));app->size=sizeof(*app);status=ui_workspace_get_instance(s->config.workspace,t->resource.instance_id,app);return status?status:app->closing?UI_STATUS_CANCELLED:UI_STATUS_OK;}
static ui_status_t ag_check(ui_ai_service_t *s,ai_graph_t *g,ag_node_t *n)
{
 ui_app_instance_info_t a;ui_cap_target_v2_t current={0};ui_cap_target_t r={0};char *desc=NULL,*profile=NULL,hash[65];ui_status_t status=ag_app(s,&g->targets[n->target],&a);if(status)return status;
 r.size=sizeof(r);r.version=1;status=ui_capability_bind(a.host,g->targets[n->target].resource.resource_id,&r);if(status)return status;
 current.size=sizeof(current);current.version=2;current.resource.size=sizeof(current.resource);current.resource.version=2;status=ui_capability_resource_ref_v2(a.host,r.resource_id,&current.resource);if(status)return status;
 {const ui_cap_target_v2_t *t=&g->targets[n->target];if(t->resource.instance_id!=current.resource.instance_id||t->resource.kind!=current.resource.kind||strcmp(t->resource.host_epoch,current.resource.host_epoch)||strcmp(t->resource.open_epoch,current.resource.open_epoch)||strcmp(t->expected_revision,r.expected_revision))return UI_STATUS_VALIDATION_FAILED;}
 status=n->write?ui_capability_describe_v2(a.host,n->capability,&desc):ui_capability_describe(a.host,n->capability,&desc);
 if(status)return status;
 {yyjson_doc *d=yyjson_read(desc,strlen(desc),0);yyjson_val *o=d?yyjson_doc_get_root(d):NULL;const char *effect=ag_str(o,"effect"),*descriptor=ag_str(o,"descriptor_hash");
  if(!n->write){if(ui_cap_json_fingerprint(desc,strlen(desc),hash))hash[0]=0;descriptor=hash;}if(strcmp(effect,n->write?"write":"read")||strcmp(descriptor,n->descriptor))status=UI_STATUS_VALIDATION_FAILED;yyjson_doc_free(d);}
 free(desc);if(status)return status;
 if(n->profile[0]){status=ui_ai_agent_profile_get(a.host,n->profile,&profile);if(!status)status=ui_cap_json_fingerprint(profile,strlen(profile),hash);free(profile);if(status)return status;if(strcmp(hash,n->profile_hash))return UI_STATUS_VALIDATION_FAILED;}
 if(s->phase!=UI_AI_READY||g->cancel)return UI_STATUS_CANCELLED;return UI_STATUS_OK;
}
static const char *ag_cap(ai_graph_t *g,ag_node_t *n,char out[256])
{(void)g;strcpy_s(out,256,n->capability);return out;}
static void ai_graph_free(ui_ai_service_t *s,ai_graph_t *g)
{
 if(!g)return;for(unsigned i=0;i<g->count;++i){ag_node_t *n=&g->nodes[i];free(n->params);free(n->constraint);free(n->instruction);free(n->context);free(n->question);free(n->answer);free(n->result);free(n->delegation);if(n->edit){s->bytes-=sizeof(*n->edit);free(n->edit);}}
 s->bytes-=g->bytes;free(g->targets);free(g->json);free(g);
}
static int ai_graph_retain(ai_graph_t *g)
{if(!g)return 0;for(unsigned i=0;i<g->count;++i)if(g->nodes[i].edit&&(g->nodes[i].edit->phase==AE_UNKNOWN||g->nodes[i].edit->holding||g->nodes[i].edit->live))return 1;return 0;}
static char *ag_snapshot(ui_ai_service_t *s,ai_run_t *r,int private_data)
{
 ai_graph_t *g=r->graph;yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *o=d?yyjson_mut_obj(d):NULL,*a=d?yyjson_mut_arr(d):NULL,*budget=d?yyjson_mut_obj(d):NULL;char *json;if(!o||!a||!budget){yyjson_mut_doc_free(d);return NULL;}yyjson_mut_doc_set_root(d,o);
 yyjson_mut_obj_add_strcpy(d,o,"run_id",r->id);yyjson_mut_obj_add_strcpy(d,o,"binding_hash",g->binding);yyjson_mut_obj_add_strcpy(d,o,"model",r->model&&r->model->assistant?"Same-Service model and application Coordinator":"Stub test model; explicit graph, not language understanding");yyjson_mut_obj_add_bool(d,o,"terminal",r->terminal!=0);yyjson_mut_obj_add_int(d,o,"phase",r->phase);yyjson_mut_obj_add_bool(d,o,"confirmed",g->confirmed!=0);yyjson_mut_obj_add_bool(d,o,"record_only",g->record_only!=0);yyjson_mut_obj_add_bool(d,o,"needs_scope",g->needs_scope!=0);yyjson_mut_obj_add_bool(d,o,"cancel_requested",r->cancel!=0);yyjson_mut_obj_add_bool(d,o,"authorization_restored",false);yyjson_mut_obj_add_bool(d,o,"control_suspended",g->suspended!=0);yyjson_mut_obj_add_uint(d,o,"scope_until",g->scope_until);yyjson_mut_obj_add_uint(d,o,"version",g->version);yyjson_mut_obj_add_uint(d,o,"persisted_version",g->persisted);yyjson_mut_obj_add_uint(d,o,"active",g->active);yyjson_mut_obj_add_uint(d,o,"peak",g->peak);
 {const char *approval="",*approval_hash="",*unknown="",*unknown_hash="";for(unsigned i=0;i<g->count;++i){ag_node_t *n=&g->nodes[i];if(n->edit&&n->edit->phase==AE_APPROVAL_WAIT&&!*approval){approval=n->id;approval_hash=n->edit->binding;}if(n->edit&&n->edit->phase==AE_UNKNOWN&&!*unknown){unknown=n->id;unknown_hash=n->edit->binding;}}yyjson_mut_obj_add_strcpy(d,o,"approval_node",approval);yyjson_mut_obj_add_strcpy(d,o,"approval_binding",approval_hash);yyjson_mut_obj_add_strcpy(d,o,"unknown_node",unknown);yyjson_mut_obj_add_strcpy(d,o,"unknown_binding",unknown_hash);}
 yyjson_mut_obj_add_uint(d,budget,"reserved_steps",g->steps);yyjson_mut_obj_add_uint(d,budget,"reserved_models",g->models);yyjson_mut_obj_add_uint(d,budget,"reserved_tools",g->tools);yyjson_mut_obj_add_uint(d,budget,"reserved_agents",g->agents);yyjson_mut_obj_add_uint(d,budget,"charged_steps",g->charged_steps);yyjson_mut_obj_add_uint(d,budget,"charged_models",g->charged_models);yyjson_mut_obj_add_uint(d,budget,"charged_tools",g->charged_tools);yyjson_mut_obj_add_uint(d,budget,"charged_agents",g->charged_agents);yyjson_mut_obj_add_uint(d,budget,"context_bytes",g->context_bytes);yyjson_mut_obj_add_uint(d,budget,"result_bytes",g->result_bytes);yyjson_mut_obj_add_strcpy(d,budget,"usage",r->model&&r->model->assistant?"P5 structural reservations; actual root request ledger is separate":"stub measured bytes; no real token usage");yyjson_mut_obj_add_val(d,o,"budget",budget);if(r->model&&r->model->assistant){yyjson_mut_obj_add_uint(d,o,"actual_root_requests_reserved",r->model->assistant->used);yyjson_mut_obj_add_uint(d,o,"actual_root_model_calls",r->model_calls);}
 for(unsigned i=0;i<g->count;++i){ag_node_t *n=&g->nodes[i];yyjson_mut_val *v=yyjson_mut_obj(d),*target=yyjson_mut_obj(d);char cap[256],instance[32];const ui_cap_target_v2_t *t=&g->targets[n->target];
  {char call[32];snprintf(call,sizeof(call),"%llu",(unsigned long long)(n->edit?n->edit->call:n->last_call));yyjson_mut_obj_add_strcpy(d,v,"call_id",call);}
  yyjson_mut_obj_add_strcpy(d,v,"id",n->id);yyjson_mut_obj_add_strcpy(d,v,"operation_id",n->operation);yyjson_mut_obj_add_strcpy(d,v,"parent",n->parent);yyjson_mut_obj_add_uint(d,v,"dependencies",n->dependencies);yyjson_mut_obj_add_strcpy(d,v,"agent_profile",n->profile);yyjson_mut_obj_add_strcpy(d,v,"capability",ag_cap(g,n,cap));yyjson_mut_obj_add_strcpy(d,v,"descriptor_hash",n->descriptor);yyjson_mut_obj_add_strcpy(d,v,"context_hash",n->context_hash);yyjson_mut_obj_add_strcpy(d,v,"profile_hash",n->profile_hash);yyjson_mut_obj_add_int(d,v,"state",n->state);yyjson_mut_obj_add_int(d,v,"risk",n->risk);yyjson_mut_obj_add_bool(d,v,"write",n->write!=0);yyjson_mut_obj_add_bool(d,v,"undo_running",n->undo_running!=0);yyjson_mut_obj_add_bool(d,v,"undo_finished",n->undo_finished!=0);yyjson_mut_obj_add_strcpy(d,v,"outcome",n->outcome);yyjson_mut_obj_add_strcpy(d,v,"code",n->code);yyjson_mut_obj_add_bool(d,v,"charged",n->charged!=0);yyjson_mut_obj_add_bool(d,v,"settled",n->settled!=0);if(r->model&&r->model->assistant)yyjson_mut_obj_add_strcpy(d,v,"specialist_summary",r->model->assistant->node_summary[i]);yyjson_mut_obj_add_strcpy(d,v,"question_id",n->question_id);yyjson_mut_obj_add_strcpy(d,v,"question",n->question?n->question:"");yyjson_mut_obj_add_strcpy(d,v,"answer_operation",n->answer_op);yyjson_mut_obj_add_strcpy(d,v,"answer_hash",n->answer_hash);yyjson_mut_obj_add_strcpy(d,v,"revision_after",n->revision);yyjson_mut_obj_add_strcpy(d,v,"approval_hash",n->edit?n->edit->binding:"");
  snprintf(instance,sizeof(instance),"%llu",(unsigned long long)t->resource.instance_id);yyjson_mut_obj_add_strcpy(d,target,"instance_id",instance);yyjson_mut_obj_add_strcpy(d,target,"resource_id",t->resource.resource_id);yyjson_mut_obj_add_strcpy(d,target,"host_epoch",t->resource.host_epoch);yyjson_mut_obj_add_strcpy(d,target,"open_epoch",t->resource.open_epoch);yyjson_mut_obj_add_strcpy(d,target,"revision",t->expected_revision);yyjson_mut_obj_add_strcpy(d,target,"scope",t->scope_json);yyjson_mut_obj_add_val(d,v,"target",target);ae_json(d,v,"params",n->params);ae_json(d,v,"constraint",n->constraint);
  if(n->result)ae_json(d,v,"result",n->result);if(n->edit){yyjson_mut_obj_add_strcpy(d,v,"edit_id",n->edit->id);yyjson_mut_obj_add_strcpy(d,v,"transaction_id",n->edit->transaction);yyjson_mut_obj_add_strcpy(d,v,"undo_id",n->edit->undo_id);yyjson_mut_obj_add_int(d,v,"effect",n->edit->effect);yyjson_mut_obj_add_int(d,v,"edit_phase",n->edit->phase);}
  if(private_data){yyjson_mut_obj_add_strcpy(d,v,"instruction",n->instruction);yyjson_mut_obj_add_strcpy(d,v,"private_context",n->context);yyjson_mut_obj_add_strcpy(d,v,"private_answer",n->answer?n->answer:"");}yyjson_mut_arr_add_val(a,v);
 }
 if(r->model&&r->model->assistant&&r->model->assistant->operation)ao_snapshot(s,r,d,o);
 yyjson_mut_obj_add_val(d,o,"nodes",a);yyjson_mut_obj_add_strcpy(d,o,"summary",r->reply?r->reply:"");yyjson_mut_obj_add_strcpy(d,o,"outcome",r->outcome);yyjson_mut_obj_add_strcpy(d,o,"storage_path",s->path);json=yyjson_mut_write(d,0,NULL);yyjson_mut_doc_free(d);return json;
}
static void ag_dirty(ai_graph_t *g){++g->version;g->dirty=1;}
static void ai_graph_cancelled(ui_ai_service_t *s,ai_graph_t *g)
{if(!g)return;g->cancel=1;g->confirmed=0;g->scope_until=0;for(unsigned i=0;i<g->count;++i)if(g->nodes[i].edit){ai_edit_t *e=g->nodes[i].edit;e->cancel=1;e->scope=e->approval=0;if(!e->cancel_at)e->cancel_at=ai_now(s);}ag_dirty(g);}

static int ag_queue(ui_ai_service_t *s,int job,int slot)
{
 ai_coordinator_t *c=s->coordinator;char *json=NULL;if(!ai_store_available(s))return 0;EnterCriticalSection(&s->lock);if(c->job||c->busy||c->done){LeaveCriticalSection(&s->lock);return 0;}LeaveCriticalSection(&s->lock);if(job==3){json=ag_snapshot(s,&s->runs[slot],0);if(!json||strlen(json)>AI_PAGE){free(json);return 0;}}
#ifdef UI_AI_FIXTURE
 /* Controlled worker-completion publication between optimistic check and copy. */
 if(job==3&&c->fixture_fail==6){EnterCriticalSection(&s->lock);c->fixture_fail=0;c->done=1;c->ok=1;c->slot=-1;LeaveCriticalSection(&s->lock);}
#endif
 EnterCriticalSection(&s->lock);if(c->job||c->busy||c->done){LeaveCriticalSection(&s->lock);free(json);return 0;}if(json)strcpy_s(c->payload,sizeof(c->payload),json);c->job=job;c->slot=slot;c->run_ref=slot>=0;c->run_id[0]=0;if(slot>=0){strcpy_s(c->run_id,65,s->runs[slot].id);++s->runs[slot].refs;}(void)ai_start_locked(s);LeaveCriticalSection(&s->lock);free(json);return 1;
}
static int ag_migrate(ui_ai_service_t *s)
{
 ai_coordinator_t *c=s->coordinator;sqlite3 *copy=NULL;sqlite3_backup *backup;wchar_t path[4096];HANDLE f;int ok=0;
 if(c->fixture_fail==1)return 0;if(ae_pragma(s->db,"PRAGMA user_version")!=2||!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,c->backup,-1,path,4096))return 0;
 f=CreateFileW(path,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);if(f==INVALID_HANDLE_VALUE)return 0;CloseHandle(f);
 if(sqlite3_open_v2(c->backup,&copy,SQLITE_OPEN_READWRITE|SQLITE_OPEN_FULLMUTEX,NULL)!=SQLITE_OK)goto done;backup=sqlite3_backup_init(copy,"main",s->db,"main");if(!backup)goto done;ok=sqlite3_backup_step(backup,-1)==SQLITE_DONE;if(sqlite3_backup_finish(backup)!=SQLITE_OK)ok=0;
 if(ok){sqlite3_stmt *q=NULL;ok=sqlite3_prepare_v2(copy,"PRAGMA quick_check(1)",-1,&q,NULL)==SQLITE_OK&&sqlite3_step(q)==SQLITE_ROW&&!strcmp((const char *)sqlite3_column_text(q,0),"ok");sqlite3_finalize(q);}
 if(ok)ok=ae_pragma(copy,"PRAGMA user_version")==2&&ae_pragma(copy,"PRAGMA application_id")==0x58535033&&ae_pragma(copy,"SELECT count(*) FROM runs")==ae_pragma(s->db,"SELECT count(*) FROM runs");
 if(ok&&ai_begin(s)){int changed=ai_sql(s,"CREATE TABLE ai_graphs(run_id TEXT PRIMARY KEY,payload TEXT NOT NULL,version INTEGER NOT NULL);PRAGMA user_version=3;");if(c->fixture_fail==2)changed=0;ok=ai_commit(s,changed);}else ok=0;
 done:if(copy&&sqlite3_close(copy)!=SQLITE_OK)ok=0;return ok;
}
static int ai_graph_io(ui_ai_service_t *s)
{
 ai_coordinator_t *c;int job,slot,ok=1,schema,recovered=0;uint64_t seq=0;ai_run_t *r=NULL;EnterCriticalSection(&s->lock);c=s->coordinator;schema=c?c->schema:0;LeaveCriticalSection(&s->lock);if(!c)return 0;
#ifdef UI_AI_FIXTURE
 EnterCriticalSection(&s->lock);if(c->fixture_fail==7&&ai_graph_jobs(s)){c->fixture_fail=0;s->fixture_worker_yield=1;++s->fixture_worker_yields;LeaveCriticalSection(&s->lock);return 0;}LeaveCriticalSection(&s->lock);
#endif
 /* Every edit uses exactly the existing P4 state machine and storage worker. */
 for(int i=0;i<AI_RUNS;++i)for(unsigned j=0;j<AG_NODES;++j){ai_edit_t *e=NULL;int advanced=0;EnterCriticalSection(&s->lock);if(s->runs[i].graph&&j<s->runs[i].graph->count){e=s->runs[i].graph->nodes[j].edit;if(e)++s->runs[i].refs;}LeaveCriticalSection(&s->lock);if(e){advanced=ae_io_one(s,e);EnterCriticalSection(&s->lock);--s->runs[i].refs;LeaveCriticalSection(&s->lock);}if(advanced)return 1;}
 EnterCriticalSection(&s->lock);if(!c->job||c->busy||c->fixture_hold){LeaveCriticalSection(&s->lock);return 0;}job=c->job;slot=c->slot;c->job=0;c->busy=1;LeaveCriticalSection(&s->lock);
 if((slot>=0&&(slot>=AI_RUNS||!c->run_ref||strcmp(c->run_id,s->runs[slot].id)))||!s->db||s->store_failed)ok=0;else if(job==1){schema=ae_pragma(s->db,"PRAGMA user_version");ok=schema>=1&&schema<=3;if(ok&&schema>=2){recovered=ae_pragma(s->db,"SELECT count(*) FROM ai_edits WHERE terminal=0 OR unknown=1");if(recovered<0)ok=0;}}
 else if(job==2){ok=ag_migrate(s);schema=ae_pragma(s->db,"PRAGMA user_version");}
 else if(job==4){r=&s->runs[slot];ar_crash(s,16);ai_finish(s,r,r->outcome,NULL);ok=r->terminal;if(ok)ar_crash(s,17);}
 else if(job==5){sqlite3_stmt *q=NULL;ai_run_t existing={0};int found=0;char *json=NULL;strcpy_s(existing.id,65,c->lookup_id);strcpy_s(existing.client,65,c->lookup_client);if(ai_statement(s,"SELECT hash FROM runs WHERE id=?",&q)){ai_bind(q,1,c->lookup_id);if(sqlite3_step(q)==SQLITE_ROW)found=!strcmp((const char *)sqlite3_column_text(q,0),c->lookup_hash)?1:-1;}sqlite3_finalize(q);if(found==1)json=ai_store_query(s,c->lookup_id);(void)ai_emit(s,found==1&&json?"existing":"rejected",&existing,NULL,0,found==1&&json,json?json:found<0?"{\"code\":\"request_id_conflict\"}":"{\"code\":\"target_invalid\"}",0);free(json);}

 else{
  sqlite3_stmt *q=NULL;yyjson_doc *d=yyjson_read(c->payload,strlen(c->payload),0);yyjson_val *o=d?yyjson_doc_get_root(d):NULL;uint64_t version=yyjson_get_uint(yyjson_obj_get(o,"version"));char op[96];r=&s->runs[slot];snprintf(op,sizeof(op),"graph:%llu",(unsigned long long)version);
  ok=c->fixture_fail!=3&&ai_begin(s);if(ok){ok=ai_statement(s,"INSERT INTO ai_graphs VALUES(?,?,?) ON CONFLICT(run_id) DO UPDATE SET payload=excluded.payload,version=excluded.version",&q);if(ok){ai_bind(q,1,r->id);ai_bind(q,2,c->payload);sqlite3_bind_int64(q,3,(sqlite3_int64)version);ok=ai_done(q);}if(ok){ok=ai_statement(s,"UPDATE runs SET phase=? WHERE id=? AND terminal=0",&q);if(ok){sqlite3_bind_int(q,1,(int)yyjson_get_int(yyjson_obj_get(o,"phase")));ai_bind(q,2,r->id);ok=ai_done(q);}}if(ok)ok=ai_event_insert(s,r,op,"graph",c->payload,0,&seq);ok=ai_commit(s,ok);}if(ok){yyjson_val *budget=yyjson_obj_get(o,"budget"),*nodes=yyjson_obj_get(o,"nodes"),*node;size_t at,count;int all=1;if(yyjson_get_uint(yyjson_obj_get(budget,"charged_tools")))ar_crash(s,14);yyjson_arr_foreach(nodes,at,count,node)if(!yyjson_is_true(yyjson_obj_get(node,"settled")))all=0;if(all)ar_crash(s,15);}yyjson_doc_free(d);
 }
 EnterCriticalSection(&s->lock);c->schema=schema;if(job==1)c->recovered=recovered;c->ok=ok;c->done=job;c->busy=0;/* The pending result now owns the queued reference. */LeaveCriticalSection(&s->lock);return 1;
}
/* Called with the Service lock: pending UI-owned edit jobs also need wakeup. */
static int ai_graph_jobs(ui_ai_service_t *s)
{if(!s->coordinator)return 0;if(s->coordinator->job)return 1;for(int i=0;i<AI_RUNS;++i)if(s->runs[i].graph)for(unsigned j=0;j<s->runs[i].graph->count;++j){ai_edit_t *e=s->runs[i].graph->nodes[j].edit;if(e&&e->job)return 1;}return 0;}
static int ai_graph_pending(ui_ai_service_t *s)
{
 ai_coordinator_t *c=s->coordinator;if(!c)return 0;if(c->job||c->busy||c->done)return 1;
 for(int i=0;i<AI_RUNS;++i)if(s->runs[i].graph){ai_graph_t *g=s->runs[i].graph;for(unsigned j=0;j<g->count;++j){ag_node_t *n=&g->nodes[j];if(n->call||(n->edit&&(n->edit->job||n->edit->busy||n->edit->holding||n->edit->live)))return 1;}}
 return 0;
}
static void ag_collect(ai_graph_t *g,ag_node_t *n,const char *outcome,const char *code,const char *result)
{
 size_t len=result?strlen(result):0;if(len>AG_RESULT||len>AG_RESULTS-g->result_bytes){outcome="limit";code="result_budget";result=NULL;len=0;}
 if(result){free(n->result);n->result=ai_copy(result);if(n->result)g->result_bytes+=len;else{outcome="failed";code="result_allocation";}}
 n->state=AG_COLLECTED;n->settled=1;if(outcome!=n->outcome)strcpy_s(n->outcome,sizeof(n->outcome),outcome);if(code!=n->code)strcpy_s(n->code,sizeof(n->code),code);if(strcmp(outcome,"completed"))g->failed=1;if(!strcmp(outcome,"unknown"))g->unknown=1;ag_dirty(g);
}
static void ag_read(ui_ai_service_t *s,ai_graph_t *g,ag_node_t *n)
{
 ui_app_instance_info_t a;ui_status_t status=ag_app(s,&g->targets[n->target],&a);ui_cap_result_t result={0};char cap[256];
 if(status){
  /* Workspace removal follows the capability/callback drain. A failed-store
     read has no outstanding executor once that instance is no longer present. */
  if(n->call&&s->store_failed&&status==UI_STATUS_NOT_FOUND){n->call=0;ag_collect(g,n,"cancelled","target_closed",NULL);return;}
  if(!(n->call&&g->cancel&&status==UI_STATUS_CANCELLED&&a.host)){if(n->call)return;ag_collect(g,n,"failed","target_closed",NULL);return;}
 }
 if(g->cancel){if(n->call&&!n->cancel_sent){(void)ui_capability_cancel(a.host,n->call);n->cancel_sent=1;}if(!n->call){ag_collect(g,n,"cancelled","cancelled",NULL);return;}}
 if(!n->call&&g->suspended)return;
 if(!n->call){ui_cap_target_t t={0};const ui_cap_target_v2_t *v=&g->targets[n->target];t.size=sizeof(t);t.version=1;t.instance_id=v->resource.instance_id;t.kind=v->resource.kind;strcpy_s(t.host_epoch,37,v->resource.host_epoch);strcpy_s(t.resource_id,256,v->resource.resource_id);strcpy_s(t.open_epoch,37,v->resource.open_epoch);strcpy_s(t.expected_revision,128,v->expected_revision);strcpy_s(t.scope_json,sizeof(t.scope_json),"{\"access\":\"read\"}");
  status=ag_check(s,g,n);if(!status)status=ui_capability_prepare(a.host,ag_cap(g,n,cap),&t,n->params,&n->call);if(status){ag_collect(g,n,"failed","read_prepare_rejected",NULL);return;}n->last_call=n->call;n->tool_phase=1;ag_dirty(g);return;}
 if(n->tool_phase==1){if(!g->cancel&&!g->suspended){status=ag_check(s,g,n);if(!status)status=ui_capability_dispatch(a.host,n->call);if(status){(void)ui_capability_cancel(a.host,n->call);n->cancel_sent=1;}}else (void)ui_capability_cancel(a.host,n->call);n->tool_phase=2;}
 result.size=sizeof(result);result.version=1;status=ui_capability_get_result(a.host,n->call,&result);if(status||result.state!=UI_CAP_FINISHED)return;
 if(n->tool_phase!=3){if(result.output_json&&strlen(result.output_json)<=AG_RESULT&&strlen(result.output_json)<=AG_RESULTS-g->result_bytes){n->result=ai_copy(result.output_json);if(n->result)g->result_bytes+=strlen(n->result);}strcpy_s(n->outcome,sizeof(n->outcome),!g->cancel&&result.outcome==UI_CAP_OK&&n->result?"completed":g->cancel?"cancelled":"failed");strcpy_s(n->revision,128,g->targets[n->target].expected_revision);strcpy_s(n->code,sizeof(n->code),result.code[0]?result.code:"read_result");n->tool_phase=3;}
 status=ui_capability_forget(a.host,n->call);if(status==UI_STATUS_PENDING)return;if(status&&status!=UI_STATUS_NOT_FOUND)return;n->call=0;ag_collect(g,n,n->outcome,n->code,NULL);
}
static void ag_edit(ui_ai_service_t *s,ai_graph_t *g,ag_node_t *n)
{
 ai_edit_t *e=n->edit;char cap[256],id[65],client[65],key[180];ui_status_t status;
 if(!e&&g->cancel){ag_collect(g,n,"cancelled","root_cancelled",NULL);return;}
 if(!e&&g->suspended)return;
 if(!e){ui_ai_edit_request_t q={0};
  EnterCriticalSection(&s->lock);
  while(sizeof(*e)>UI_AI_MAX_BYTES-s->bytes){
   if(ai_evict(s)>=0)continue;
   LeaveCriticalSection(&s->lock);ag_collect(g,n,"failed","edit_budget",NULL);return;
  }
  e=calloc(1,sizeof(*e));
  if(e){e->service=s;e->schema=3;s->bytes+=sizeof(*e);n->edit=e;}
  LeaveCriticalSection(&s->lock);
  if(!e){ag_collect(g,n,"failed","spawn_allocation",NULL);return;}
  q.size=sizeof(q);q.version=1;snprintf(key,sizeof(key),"{\"root\":\"%s\",\"operation\":\"%s\"}",g->binding,n->operation);if(ui_cap_json_fingerprint(key,strlen(key),client)){ag_collect(g,n,"failed","operation_identity",NULL);return;}q.client_request_id=client;q.capability_id=ag_cap(g,n,cap);q.target=g->targets[n->target];q.params_json=n->params;q.parameter_constraint_schema=n->constraint;
  status=ag_check(s,g,n);if(!status)status=ae_propose_one(s,e,&q,id,1);if(status){ag_collect(g,n,"failed","edit_prepare_rejected",NULL);return;}
 }
 if(g->cancel||g->suspended){e->cancel=1;e->scope=e->approval=0;if(!e->cancel_at)e->cancel_at=ai_now(s);}
 ae_ui_one(s,e);
 if(e->phase==AE_SCOPE_WAIT&&!e->job&&!e->busy){if(!g->confirmed||ai_now(s)>=g->scope_until||g->cancel){e->cancel=1;return;}e->scope=1;e->scope_until=g->scope_until;if(!ae_queue_one(s,e,AE_SCOPE)){e->scope=0;return;}}
 if(e->phase==AE_APPROVAL_WAIT){if(n->state!=AG_APPROVAL){n->state=AG_APPROVAL;ag_dirty(g);}return;}
 if(e->phase==AE_DONE||e->phase==AE_UNKNOWN){if(e->holding||e->live||e->job||e->busy)return;strcpy_s(n->revision,128,e->revision_after);ag_collect(g,n,e->phase==AE_UNKNOWN?"unknown":!strcmp(e->code,"ok")?"completed":g->cancel?"cancelled":"failed",e->code,e->output[0]?e->output:NULL);}
}
static void ai_graph_ui(ui_ai_service_t *s)
{
 ai_coordinator_t *c=s->coordinator;int done,ok,slot;if(!c)return;
 EnterCriticalSection(&s->lock);done=c->done;ok=c->ok;slot=c->slot;if(done)c->done=0;LeaveCriticalSection(&s->lock);
 if(done){
  int identity=slot>=0&&slot<AI_RUNS&&c->run_ref&&!strcmp(c->run_id,s->runs[slot].id)&&s->runs[slot].graph;
  if(slot>=0&&!identity){/* UI must not wait for notification queue space. */EnterCriticalSection(&s->lock);s->store_failed=1;s->phase=UI_AI_UNAVAILABLE;strcpy_s(s->error,sizeof(s->error),"graph_result_identity");LeaveCriticalSection(&s->lock);slot=-1;ok=0;}
  if(!ok){strcpy_s(c->error,256,"storage_unconfirmed; original database retained");c->failed=1;if(done==2){EnterCriticalSection(&s->lock);s->phase=UI_AI_UNAVAILABLE;strcpy_s(s->error,sizeof(s->error),c->error);LeaveCriticalSection(&s->lock);}if(slot>=0){ai_graph_t *g=s->runs[slot].graph;ai_graph_cancelled(s,g);EnterCriticalSection(&s->lock);s->store_failed=1;s->phase=UI_AI_UNAVAILABLE;s->runs[slot].phase=UI_AI_UNCONFIRMED;s->runs[slot].committed=0;LeaveCriticalSection(&s->lock);}}else if(done==3&&slot>=0){ai_graph_t *g=s->runs[slot].graph;yyjson_doc *d=yyjson_read(c->payload,strlen(c->payload),0);g->persisted=d?yyjson_get_uint(yyjson_obj_get(yyjson_doc_get_root(d),"version")):0;yyjson_doc_free(d);g->dirty=g->persisted!=g->version;s->runs[slot].committed=1;if(s->runs[slot].cancel)s->runs[slot].cancel_recorded=1;if(s->runs[slot].stalled==1)s->runs[slot].stalled=2;}else if(done==4&&slot>=0)s->runs[slot].graph->finished=1;
  EnterCriticalSection(&s->lock);if(identity)--s->runs[slot].refs;c->run_ref=0;c->run_id[0]=0;LeaveCriticalSection(&s->lock);
 }
 /* Failure disables new execution, but still drains existing application leases.
    Cleanup is independent of active, admission and root terminal state. */
 if(!ai_store_available(s)){
  for(int i=0;i<AI_RUNS;++i){ai_graph_t *g=s->runs[i].graph;if(!g)continue;g->cancel=1;g->confirmed=0;
   for(unsigned j=0;j<g->count;++j){ag_node_t *n=&g->nodes[j];if(n->edit){ae_unavailable(s,n->edit);if(n->edit->phase==AE_UNKNOWN)g->unknown=1;}else if(n->call)ag_read(s,g,n);}
  }return;
 }
 if(c->job||c->busy||c->done)return;
 for(int i=0;i<AI_RUNS;++i){ai_run_t *r=&s->runs[i];ai_graph_t *g=r->graph;unsigned active=0,complete=0;if(!g||g->record_only||(!r->committed&&!g->admitted))continue;
  g->cancel=r->cancel||s->phase!=UI_AI_READY||c->failed;
  if(g->cancel)g->confirmed=0;
  if(r->terminal){for(unsigned j=0;j<g->count;++j)if(g->nodes[j].undo_running){ag_node_t *n=&g->nodes[j];ae_ui_one(s,n->edit);if((n->edit->phase==AE_DONE||n->edit->phase==AE_UNKNOWN)&&!n->edit->holding&&!n->edit->live&&!n->edit->job&&!n->edit->busy){n->undo_running=0;n->undo_finished=1;ag_dirty(g);}}for(unsigned j=0;j<g->count;++j)if(g->nodes[j].edit&&(g->nodes[j].edit->phase==AE_RECONCILE||g->nodes[j].edit->phase==AE_RECORD||g->nodes[j].edit->io_done)){ae_ui_one(s,g->nodes[j].edit);if(g->nodes[j].edit->phase==AE_DONE)ag_dirty(g);}if(g->dirty&&!c->failed){(void)ag_queue(s,3,i);return;}continue;}
  if(s->active!=i&&!c->failed)continue;
  {ui_ai_run_phase_t phase=g->cancel?(r->stalled?UI_AI_CANCELLATION_STALLED:UI_AI_CANCEL_REQUESTED):g->needs_scope&&!g->confirmed?UI_AI_WAITING_INPUT:UI_AI_RUNNING;int running=0,waiting=0;for(unsigned j=0;j<g->count;++j){if(g->nodes[j].state==AG_RUNNING)running=1;if(g->nodes[j].state==AG_INPUT||g->nodes[j].state==AG_APPROVAL)waiting=1;}if(!running&&waiting&&!g->cancel)phase=UI_AI_WAITING_INPUT;if(r->phase!=phase&&!c->failed){r->phase=phase;if(!g->dirty)ag_dirty(g);}}
  if(g->dirty&&!c->failed){(void)ag_queue(s,3,i);return;}if(g->persisted!=g->version&&!c->failed)return;
  for(unsigned j=0;j<g->count;++j)if(g->nodes[j].state==AG_RUNNING)++active;
  for(unsigned j=0;j<g->count;++j){ag_node_t *n=&g->nodes[j];int deps=0,bad=0;
   if(n->state==AG_COLLECTED){++complete;continue;}
   if(n->state==AG_RUNNING||n->state==AG_APPROVAL){if(n->write)ag_edit(s,g,n);else ag_read(s,g,n);continue;}
   if(g->cancel){ag_collect(g,n,"cancelled","root_cancelled",NULL);continue;}
   for(unsigned k=0;k<g->count;++k)if((n->dependencies&(UINT32_C(1)<<k))||n->parent_index==(int)k){if(g->nodes[k].state!=AG_COLLECTED)deps=1;else if(strcmp(g->nodes[k].outcome,"completed"))bad=1;}
   if(bad){ag_collect(g,n,"blocked","dependency_failed",NULL);continue;}if(deps){n->state=AG_DEPENDENCY;continue;}
   if(g->needs_scope&&(!g->confirmed||ai_now(s)>=g->scope_until)){if(g->confirmed){ag_collect(g,n,"failed","scope_expired",NULL);}continue;}
   if(n->question&&*n->question&&!n->answer){if(n->state!=AG_INPUT){n->state=AG_INPUT;snprintf(n->question_id,65,"q-%.60s",n->operation);ag_dirty(g);}continue;}
   if(active>=4||!as_node_ready(s,r,j))continue;if(!n->charged){n->charged=1;++g->charged_tools;++g->charged_steps;if(n->profile[0]){++g->charged_models;++g->charged_steps;}if(r->model&&r->model->assistant){EnterCriticalSection(&s->lock);++r->tool_calls;++r->steps;LeaveCriticalSection(&s->lock);}if(n->profile[0])++g->charged_agents;n->state=AG_RUNNING;++active;ag_dirty(g);if(n->fail)ag_collect(g,n,"failed","stub_spawn_failed",NULL);continue;}
  }
  g->active=active;if(active>g->peak)g->peak=active;
  if(complete==g->count&&!g->finishing&&!c->failed&&r->model&&r->model->assistant){if(!as_collect(s,r))continue;g->finishing=1;g->confirmed=0;ag_dirty(g);}
   if(complete==g->count&&!g->finishing&&!c->failed){size_t pos=0;g->finishing=1;g->confirmed=0;r->reply[0]=0;pos+=(size_t)snprintf(r->reply,UI_AI_MAX_REPLY+1,"[Stub test model] Collected %u steps; %s.\n",g->count,g->unknown?"unknown resource results retained":g->failed?"partial failure":"completed");for(unsigned j=0;j<g->count&&pos<UI_AI_MAX_REPLY;++j){ag_node_t *n=&g->nodes[j];int added=snprintf(r->reply+pos,UI_AI_MAX_REPLY+1-pos,"%s: %s %s\n",n->id,n->outcome,n->result?n->result:"");if(added<0||(size_t)added>UI_AI_MAX_REPLY-pos){strcpy_s(r->error,256,"summary_limit");break;}pos+=(size_t)added;}r->reply_length=strlen(r->reply);strcpy_s(r->outcome,32,g->unknown?"unknown":r->cancel?"cancelled":g->failed?"partial_failure":"completed");ag_dirty(g);}
  if(g->dirty&&!c->failed){(void)ag_queue(s,3,i);return;}
  if(g->finishing&&!g->finished&&!c->failed){/* Final persistence stays on the same worker, no UI database access. */(void)ag_queue(s,4,i);return;}
 }
}

static ui_status_t ag_manifest(ag_node_t *n,const char *role,const char *description)
{yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *o=d?yyjson_mut_obj(d):NULL;char *raw;ui_status_t status;if(!o){yyjson_mut_doc_free(d);return UI_STATUS_OUT_OF_MEMORY;}yyjson_mut_doc_set_root(d,o);yyjson_mut_obj_add_strcpy(d,o,"role_data",role?role:"");yyjson_mut_obj_add_strcpy(d,o,"task",n->instruction);yyjson_mut_obj_add_strcpy(d,o,"private_context",n->context);yyjson_mut_obj_add_strcpy(d,o,"capability",n->capability);yyjson_mut_obj_add_strcpy(d,o,"schema_data",description);yyjson_mut_obj_add_strcpy(d,o,"canonical_parameters",n->params);yyjson_mut_obj_add_strcpy(d,o,"question",n->question);raw=yyjson_mut_write(d,0,NULL);yyjson_mut_doc_free(d);if(!raw)return UI_STATUS_OUT_OF_MEMORY;n->input_bytes=strlen(raw);status=n->input_bytes>AG_CONTEXT?UI_STATUS_LIMIT_EXCEEDED:ui_cap_hash_bytes(raw,strlen(raw),n->context_hash);free(raw);return status;}
static ui_status_t ag_build(ui_ai_service_t *s,const ui_ai_graph_request_t *q,ai_graph_t **out)
{
 ai_graph_t *g=NULL;ui_cap_json_doc_t parsed={0};yyjson_val *root,*nodes,*v;char *canonical=NULL;size_t len,total=0;ui_status_t status;unsigned children[AG_NODES+1]={0};uint32_t done=0;
 *out=NULL;if(!q||q->size<sizeof(*q)||q->version!=1||!ai_id(q->client_request_id)||!ai_text(q->input_utf8,UI_AI_MAX_INPUT,0)||!q->graph_json||!q->targets||!q->target_count||q->target_count>16)return UI_STATUS_INVALID_ARGUMENT;
 status=ui_cap_json_canonicalize(q->graph_json,strlen(q->graph_json),&canonical,&len);if(status)return status;status=ui_cap_parse(canonical,len,65536,&parsed);if(status){free(canonical);return status;}
 root=yyjson_doc_get_root(parsed.doc);nodes=yyjson_obj_get(root,"nodes");if(!ai_json_texts(root)||!yyjson_is_obj(root)||yyjson_obj_size(root)!=1||!yyjson_is_arr(nodes)||!yyjson_arr_size(nodes)||yyjson_arr_size(nodes)>AG_NODES){status=UI_STATUS_VALIDATION_FAILED;goto done;}
 g=calloc(1,sizeof(*g));if(!g){status=UI_STATUS_OUT_OF_MEMORY;goto done;}g->count=(unsigned)yyjson_arr_size(nodes);g->json=canonical;canonical=NULL;g->targets=malloc(q->target_count*sizeof(*g->targets));if(!g->targets){status=UI_STATUS_OUT_OF_MEMORY;goto done;}memcpy(g->targets,q->targets,q->target_count*sizeof(*g->targets));g->target_count=q->target_count;
 /* Reserve all bounded copied buffers BEFORE any callback or execution. */
 total=sizeof(*g)+strlen(g->json)+1+q->target_count*sizeof(*g->targets)+AG_CONTEXT+AG_RESULTS+g->count*(AG_RESULT+1u);
 while(total>UI_AI_MAX_BYTES-s->bytes)if(ai_evict(s)<0){status=UI_STATUS_LIMIT_EXCEEDED;goto done;}g->bytes=total;s->bytes+=total;
 for(size_t i=0;i<q->target_count;++i){const ui_cap_target_v2_t *t=&g->targets[i];if(t->size<sizeof(*t)||t->version!=2||t->resource.size<sizeof(t->resource)||t->resource.version!=2||!t->resource.instance_id||!memchr(t->resource.host_epoch,0,37)||!memchr(t->resource.open_epoch,0,37)||!memchr(t->resource.resource_id,0,256)||!memchr(t->expected_revision,0,128)||!memchr(t->scope_json,0,sizeof(t->scope_json))||(strcmp(t->scope_json,"{\"access\":\"read\"}")&&strcmp(t->scope_json,"{\"access\":\"write\"}"))){status=UI_STATUS_INVALID_ARGUMENT;goto done;}}
 for(unsigned i=0;i<g->count;++i){ag_node_t *n=&g->nodes[i];ui_app_instance_info_t app;char *description=NULL,*profile=NULL;yyjson_doc *d=NULL;yyjson_val *o,*params,*constraint;char *params_raw=NULL,*constraint_raw=NULL;const char *effect;
  v=yyjson_arr_get(nodes,i);if(!yyjson_is_obj(v)){status=UI_STATUS_VALIDATION_FAILED;goto done;}
  {yyjson_obj_iter it=yyjson_obj_iter_with(v);yyjson_val *key;while((key=yyjson_obj_iter_next(&it))!=NULL){const char *k=yyjson_get_str(key);if(strcmp(k,"id")&&strcmp(k,"operation_id")&&strcmp(k,"parent")&&strcmp(k,"dependencies")&&strcmp(k,"agent_profile")&&strcmp(k,"instruction")&&strcmp(k,"context")&&strcmp(k,"target")&&strcmp(k,"capability")&&strcmp(k,"params")&&strcmp(k,"constraint")&&strcmp(k,"delegation")&&strcmp(k,"question")&&strcmp(k,"fail")){status=UI_STATUS_VALIDATION_FAILED;goto done;}}}
  if(!ai_id(ag_str(v,"id"))||!ai_id(ag_str(v,"operation_id"))||strlen(ag_str(v,"capability"))>255||!ai_text(ag_str(v,"instruction"),16384,1)||!ai_text(ag_str(v,"context"),16384,1)||!ai_text(ag_str(v,"question"),UI_AI_MAX_INPUT,1)||strlen(ag_str(v,"agent_profile"))>64||strlen(ag_str(v,"parent"))>64||!yyjson_is_uint(yyjson_obj_get(v,"target"))||yyjson_get_uint(yyjson_obj_get(v,"target"))>=g->target_count){status=UI_STATUS_VALIDATION_FAILED;goto done;}
  strcpy_s(n->id,65,ag_str(v,"id"));strcpy_s(n->operation,65,ag_str(v,"operation_id"));strcpy_s(n->parent,65,ag_str(v,"parent"));strcpy_s(n->profile,65,ag_str(v,"agent_profile"));strcpy_s(n->capability,256,ag_str(v,"capability"));n->target=(unsigned)yyjson_get_uint(yyjson_obj_get(v,"target"));n->parent_index=-1;n->fail=yyjson_is_true(yyjson_obj_get(v,"fail"));
  for(unsigned j=0;j<i;++j)if(!strcmp(n->id,g->nodes[j].id)||!strcmp(n->operation,g->nodes[j].operation)){status=UI_STATUS_ALREADY_EXISTS;goto done;}
  n->instruction=ai_copy(ag_str(v,"instruction"));n->context=ai_copy(ag_str(v,"context"));n->question=ai_copy(ag_str(v,"question"));n->delegation=yyjson_obj_get(v,"delegation")?yyjson_val_write(yyjson_obj_get(v,"delegation"),0,NULL):ai_copy("[]");
  if(!n->instruction||!n->context||!n->question||!n->delegation){status=UI_STATUS_OUT_OF_MEMORY;goto done;}g->context_bytes+=strlen(n->instruction)+strlen(n->context)+strlen(n->question);if(g->context_bytes>AG_CONTEXT){status=UI_STATUS_LIMIT_EXCEEDED;goto done;}
  {char *ctx=ui_cap_quote(n->context);if(!ctx){status=UI_STATUS_OUT_OF_MEMORY;goto done;}status=ui_cap_json_fingerprint(ctx,strlen(ctx),n->context_hash);free(ctx);if(status)goto done;}
  status=ag_app(s,&g->targets[n->target],&app);if(status){status=UI_STATUS_PENDING;goto done;}status=ui_capability_describe(app.host,n->capability,&description);if(status==UI_STATUS_NOT_FOUND)status=ui_capability_describe_v2(app.host,n->capability,&description);if(status)goto done;d=yyjson_read(description,strlen(description),0);o=d?yyjson_doc_get_root(d):NULL;effect=ag_str(o,"effect");n->write=!strcmp(effect,"write");
  if(strcmp(effect,"read")&&(!n->write||yyjson_get_int(yyjson_obj_get(o,"edit_contract"))!=1)){status=UI_STATUS_PERMISSION_DENIED;yyjson_doc_free(d);free(description);goto done;}
  if(n->write){if(strcmp(g->targets[n->target].scope_json,"{\"access\":\"write\"}")){status=UI_STATUS_PERMISSION_DENIED;yyjson_doc_free(d);free(description);goto done;}g->needs_scope=1;n->risk=yyjson_get_int(yyjson_obj_get(o,"risk"));strcpy_s(n->descriptor,65,ag_str(o,"descriptor_hash"));}else{status=ui_cap_json_fingerprint(description,strlen(description),n->descriptor);if(status){yyjson_doc_free(d);free(description);goto done;}}
  params=yyjson_obj_get(v,"params");constraint=yyjson_obj_get(v,"constraint");params_raw=params?yyjson_val_write(params,0,NULL):NULL;constraint_raw=constraint?yyjson_val_write(constraint,0,NULL):NULL;
  if(!params_raw||!constraint_raw){status=UI_STATUS_VALIDATION_FAILED;}else{char *schema=yyjson_val_write(yyjson_obj_get(o,"input_schema"),0,NULL);status=schema?ui_cap_json_validate(schema,params_raw,strlen(params_raw)):UI_STATUS_VALIDATION_FAILED;free(schema);if(!status)status=ui_cap_schema_canonical(constraint_raw,&n->constraint);if(!status)status=ui_cap_json_validate(n->constraint,params_raw,strlen(params_raw));if(!status)status=ui_cap_json_canonicalize(params_raw,strlen(params_raw),&n->params,&len);}
  free(params_raw);free(constraint_raw);yyjson_doc_free(d);if(!status)status=ag_manifest(n,"",description);free(description);if(status)goto done;
  if(n->profile[0]){status=ui_ai_agent_profile_get(app.host,n->profile,&profile);if(status)goto done;status=ui_cap_json_fingerprint(profile,strlen(profile),n->profile_hash);d=yyjson_read(profile,strlen(profile),0);o=d?yyjson_doc_get_root(d):NULL;char *caps=o?yyjson_val_write(yyjson_obj_get(o,"capabilities"),0,NULL):NULL;if(!caps||!ag_has(caps,n->capability))status=UI_STATUS_PERMISSION_DENIED;free(caps);yyjson_doc_free(d);if(!status){char manifest[65];strcpy_s(manifest,65,n->context_hash);yyjson_mut_doc *m=yyjson_mut_doc_new(NULL);yyjson_mut_val *manifest_object=m?yyjson_mut_obj(m):NULL;char *raw=NULL;if(manifest_object){yyjson_mut_doc_set_root(m,manifest_object);yyjson_mut_obj_add_strcpy(m,manifest_object,"input_manifest",manifest);yyjson_mut_obj_add_strcpy(m,manifest_object,"role_data",profile);raw=yyjson_mut_write(m,0,NULL);}if(!raw)status=UI_STATUS_OUT_OF_MEMORY;else status=ui_cap_hash_bytes(raw,strlen(raw),n->context_hash);free(raw);yyjson_mut_doc_free(m);}n->input_bytes+=profile?strlen(profile):0;free(profile);if(status)goto done;++g->agents;}
  g->context_bytes+=n->input_bytes-strlen(n->instruction)-strlen(n->context)-strlen(n->question);if(g->context_bytes>AG_CONTEXT){status=UI_STATUS_LIMIT_EXCEEDED;goto done;}
  g->tools++;g->models=1+g->agents;g->steps=g->tools+g->models;if(g->models>16||g->steps>64||g->tools>64){status=UI_STATUS_LIMIT_EXCEEDED;goto done;}
 }
 for(unsigned i=0;i<g->count;++i){ag_node_t *n=&g->nodes[i];v=yyjson_arr_get(nodes,i);if(n->parent[0]){int at=ag_index(g,n->parent);if(at<0||at>=(int)i){status=UI_STATUS_VALIDATION_FAILED;goto done;}n->parent_index=at;n->depth=g->nodes[at].depth+1;if(n->target!=g->nodes[at].target||!ag_has(g->nodes[at].delegation,n->capability)){status=UI_STATUS_PERMISSION_DENIED;goto done;}}else n->depth=1;
  if(n->depth>3||++children[n->parent_index+1]>8){status=UI_STATUS_LIMIT_EXCEEDED;goto done;}
  {yyjson_val *deps=yyjson_obj_get(v,"dependencies"),*dep;size_t j,count;if(deps&&!yyjson_is_arr(deps)){status=UI_STATUS_VALIDATION_FAILED;goto done;}yyjson_arr_foreach(deps,j,count,dep){const char *id=yyjson_get_str(dep);int at=id?ag_index(g,id):-1;if(at<0||(n->dependencies&(UINT32_C(1)<<at))){status=UI_STATUS_VALIDATION_FAILED;goto done;}n->dependencies|=UINT32_C(1)<<at;}}
  {int parent=n->parent_index;while(parent>=0){ag_node_t *p=&g->nodes[parent];if(p->profile[0]){ui_app_instance_info_t a;char *profile=NULL;yyjson_doc *d=NULL;char *caps=NULL;status=ag_app(s,&g->targets[p->target],&a);if(!status)status=ui_ai_agent_profile_get(a.host,p->profile,&profile);if(!status){d=yyjson_read(profile,strlen(profile),0);caps=d?yyjson_val_write(yyjson_obj_get(yyjson_doc_get_root(d),"capabilities"),0,NULL):NULL;if(!caps||!ag_has(caps,n->capability))status=UI_STATUS_PERMISSION_DENIED;}free(caps);yyjson_doc_free(d);free(profile);if(status)goto done;}parent=p->parent_index;}}
  status=ag_check(s,g,n);if(status){status=UI_STATUS_PENDING;goto done;}
 }
 if(g->needs_scope&&(s->coordinator->recovered||(s->edit&&s->edit->recovered))){status=UI_STATUS_PERMISSION_DENIED;goto done;}
 for(unsigned i=0;i<g->count;++i)if(ag_cycle(g,i,0,&done)){status=UI_STATUS_VALIDATION_FAILED;goto done;}
 if(s->phase!=UI_AI_READY){status=UI_STATUS_CANCELLED;goto done;}g->charged_models=1;g->charged_steps=1;g->version=1;g->dirty=1;*out=g;g=NULL;status=UI_STATUS_OK;
 done:ai_graph_free(s,g);free(canonical);ui_cap_doc_free(&parsed);return status;
}
ui_status_t ui_ai_coordinator_enable(ui_ai_service_t *s)
{
 ai_coordinator_t *c;if(!ai_ui(s))return UI_STATUS_INVALID_ARGUMENT;if(s->coordinator){c=s->coordinator;if(c->schema==1&&!c->failed&&!c->job&&!c->busy&&!c->done&&s->phase==UI_AI_READY)return ag_queue(s,1,-1)?UI_STATUS_OK:UI_STATUS_LIMIT_EXCEEDED;return UI_STATUS_ALREADY_EXISTS;}if(s->phase!=UI_AI_READY||s->callbacks||s->polling)return UI_STATUS_PENDING;
 if(sizeof(*c)>UI_AI_MAX_BYTES-s->bytes)return UI_STATUS_LIMIT_EXCEEDED;c=calloc(1,sizeof(*c));if(!c)return UI_STATUS_OUT_OF_MEMORY;
 snprintf(c->backup,sizeof(c->backup),"%s.schema2-%s.sqlite",s->path,s->session);{char *q=ui_cap_quote(c->backup);if(!q){free(c);return UI_STATUS_OUT_OF_MEMORY;}if(ui_cap_json_fingerprint(q,strlen(q),c->binding)){free(q);free(c);return UI_STATUS_INVALID_ARGUMENT;}free(q);}
 EnterCriticalSection(&s->lock);s->coordinator=c;s->bytes+=sizeof(*c);LeaveCriticalSection(&s->lock);return ag_queue(s,1,-1)?UI_STATUS_OK:UI_STATUS_LIMIT_EXCEEDED;
}
ui_status_t ui_ai_coordinator_get_state(ui_ai_service_t *s,char **out)
{
 ai_coordinator_t *c;yyjson_mut_doc *d;yyjson_mut_val *o;if(out)*out=NULL;if(!ai_ui(s)||!out)return UI_STATUS_INVALID_ARGUMENT;if(!(c=s->coordinator))return UI_STATUS_NOT_FOUND;d=yyjson_mut_doc_new(NULL);o=d?yyjson_mut_obj(d):NULL;if(!o){yyjson_mut_doc_free(d);return UI_STATUS_OUT_OF_MEMORY;}yyjson_mut_doc_set_root(d,o);EnterCriticalSection(&s->lock);yyjson_mut_obj_add_int(d,o,"schema",c->schema);yyjson_mut_obj_add_int(d,o,"recovered_unresolved",c->recovered);yyjson_mut_obj_add_strcpy(d,o,"storage_path",s->path);yyjson_mut_obj_add_strcpy(d,o,"backup_path",c->backup);yyjson_mut_obj_add_strcpy(d,o,"binding_hash",c->binding);yyjson_mut_obj_add_strcpy(d,o,"error",c->error);yyjson_mut_obj_add_bool(d,o,"pending",c->job||c->busy||c->done);*out=yyjson_mut_write(d,0,NULL);LeaveCriticalSection(&s->lock);yyjson_mut_doc_free(d);return *out?UI_STATUS_OK:UI_STATUS_OUT_OF_MEMORY;
}
static ui_status_t ag_submit(ui_ai_service_t *s,const ui_ai_graph_request_t *q,const char *parent,char out[65])
{
 ai_graph_t *g=NULL;ui_ai_task_request_t request={0};ui_status_t status;int at;if(out)out[0]=0;if(!ai_ui(s)||!out||!q||q->size<sizeof(*q)||q->version!=1||!q->targets||!q->target_count||q->target_count>16)return UI_STATUS_INVALID_ARGUMENT;if(!s->coordinator||s->coordinator->schema!=3||s->coordinator->failed)return UI_STATUS_UNSUPPORTED;
 if(s->callbacks||s->polling)return UI_STATUS_LIMIT_EXCEEDED;
 if(q&&q->size>=sizeof(*q)&&q->version==1&&q->targets&&q->target_count<=16){for(size_t i=0;i<q->target_count;++i)if(!ag_target_shape(&q->targets[i]))return UI_STATUS_INVALID_ARGUMENT;}
 if(q&&q->size>=sizeof(*q)&&q->version==1&&ai_id(q->client_request_id)&&q->graph_json&&q->targets&&q->target_count<=16){for(int i=0;i<AI_RUNS;++i)if(s->runs[i].graph&&!strcmp(s->runs[i].client,q->client_request_id)){char *canon=NULL,*old_targets=NULL,*new_targets=NULL;size_t len;ai_graph_t temporary={0};ui_status_t duplicate=ui_cap_json_canonicalize(q->graph_json,strlen(q->graph_json),&canon,&len);temporary.targets=(ui_cap_target_v2_t *)q->targets;temporary.target_count=q->target_count;if(!duplicate){old_targets=ai_graph_targets_json(s->runs[i].graph);new_targets=ai_graph_targets_json(&temporary);duplicate=old_targets&&new_targets&&!strcmp(canon,s->runs[i].graph->json)&&!strcmp(old_targets,new_targets)&&q->input_utf8&&!strcmp(q->input_utf8,s->runs[i].input)?UI_STATUS_OK:UI_STATUS_ALREADY_EXISTS;}free(canon);free(old_targets);free(new_targets);if(!duplicate)strcpy_s(out,65,s->runs[i].id);return duplicate;}}
 for(size_t i=0;q&&i<q->target_count&&i<16;++i)if(!ag_target_shape(&q->targets[i]))return UI_STATUS_INVALID_ARGUMENT;
 ++s->methods;status=ag_build(s,q,&g);
 if(status==UI_STATUS_PENDING){ai_coordinator_t *c=s->coordinator;ai_graph_t temporary={0};char *canonical=NULL,*targets=NULL,lookup_hash[65],lookup_id[65];size_t length;temporary.targets=(ui_cap_target_v2_t *)q->targets;temporary.target_count=q->target_count;
  if(c->job||c->busy||c->done){status=UI_STATUS_LIMIT_EXCEEDED;goto done;}status=ui_cap_json_canonicalize(q->graph_json,strlen(q->graph_json),&canonical,&length);targets=ai_graph_targets_json(&temporary);if(!status&&!targets)status=UI_STATUS_OUT_OF_MEMORY;if(!status)status=ai_graph_request_hash(q->input_utf8,canonical,targets,parent,lookup_hash);
  if(!status){char key[190];snprintf(key,sizeof(key),"{\"conversation\":\"%s\",\"client\":\"%s\"}",s->conversation,q->client_request_id);status=ui_cap_json_fingerprint(key,strlen(key),lookup_id);}
  free(canonical);free(targets);if(!status){EnterCriticalSection(&s->lock);if(c->job||c->busy||c->done)status=UI_STATUS_LIMIT_EXCEEDED;else{strcpy_s(c->lookup_hash,65,lookup_hash);strcpy_s(c->lookup_id,65,lookup_id);strcpy_s(c->lookup_client,65,q->client_request_id);strcpy_s(out,65,lookup_id);status=ag_queue(s,5,-1)?UI_STATUS_PENDING:UI_STATUS_LIMIT_EXCEEDED;}LeaveCriticalSection(&s->lock);}goto done;
 }
 if(status)goto done;
 request.size=sizeof(request);request.version=1;request.client_request_id=q->client_request_id;request.input_utf8=q->input_utf8;request.stub_plan_json=g->json;
 status=ai_submit_impl(s,&request,parent,out,g,NULL,NULL);if(status)goto done;at=ai_find(s,out);if(at>=0&&s->runs[at].graph==g){char key[256];snprintf(key,sizeof(key),"{\"graph\":\"%s\",\"epoch\":\"%s\"}",out,s->session);status=ui_cap_json_fingerprint(key,strlen(key),g->binding);g=NULL;}
 done:ai_graph_free(s,g);--s->methods;return status;
}
ui_status_t ui_ai_graph_submit(ui_ai_service_t *s,const ui_ai_graph_request_t *q,char out[65]){return ag_submit(s,q,NULL,out);}
ui_status_t ui_ai_graph_continue(ui_ai_service_t *s,const char *old,const ui_ai_graph_request_t *q,char out[65]){if(!ai_id(old))return UI_STATUS_INVALID_ARGUMENT;return ag_submit(s,q,old,out);}
ui_status_t ui_ai_graph_get(ui_ai_service_t *s,const char *id,char **out)
{int at;if(out)*out=NULL;if(!ai_ui(s)||!out||!ai_id(id))return UI_STATUS_INVALID_ARGUMENT;at=ai_find(s,id);if(at<0||!s->runs[at].graph)return UI_STATUS_NOT_FOUND;*out=ag_snapshot(s,&s->runs[at],0);return *out?UI_STATUS_OK:UI_STATUS_OUT_OF_MEMORY;}
ui_status_t ui_ai_graph_cancel(ui_ai_service_t *s,const char *id){return ui_ai_cancel(s,id);}
ui_status_t ui_ai_graph_answer(ui_ai_service_t *s,const char *id,const char *node,const char *question,const char *op,const char *answer)
{
 int at;ai_graph_t *g;ag_node_t *n;char hash[65],*quoted;if(!ai_ui(s)||!ai_id(id)||!ai_id(node)||!ai_id(question)||!ai_id(op)||!ai_text(answer,UI_AI_MAX_INPUT,1))return UI_STATUS_INVALID_ARGUMENT;at=ai_find(s,id);if(at<0||!(g=s->runs[at].graph))return UI_STATUS_NOT_FOUND;at=ag_index(g,node);if(at<0)return UI_STATUS_NOT_FOUND;n=&g->nodes[at];if(g->cancel||s->phase!=UI_AI_READY)return UI_STATUS_CANCELLED;
 quoted=ui_cap_quote(answer);if(!quoted)return UI_STATUS_OUT_OF_MEMORY;if(ui_cap_json_fingerprint(quoted,strlen(quoted),hash)){free(quoted);return UI_STATUS_INVALID_ARGUMENT;}free(quoted);
 if(n->answer_op[0])return !strcmp(n->answer_op,op)&&!strcmp(n->answer_hash,hash)?UI_STATUS_OK:UI_STATUS_ALREADY_EXISTS;
 if(n->state!=AG_INPUT||strcmp(n->question_id,question))return UI_STATUS_PERMISSION_DENIED;if(strlen(answer)>AG_CONTEXT-g->context_bytes)return UI_STATUS_LIMIT_EXCEEDED;n->answer=ai_copy(answer);if(!n->answer)return UI_STATUS_OUT_OF_MEMORY;g->context_bytes+=strlen(answer);strcpy_s(n->answer_op,65,op);strcpy_s(n->answer_hash,65,hash);n->state=AG_RESERVED;ag_dirty(g);return UI_STATUS_OK;
}
static ui_status_t ag_action(ui_ai_service_t *s,ui_ai_graph_action_t action,const char *id,const char *node,const char *binding,ag_binding_t *display)
{
 ai_coordinator_t *c=s->coordinator;int at;ai_graph_t *g;ag_node_t *n=NULL;if(!c||s->phase!=UI_AI_READY||c->job||c->busy||c->done)return UI_STATUS_PENDING;
 if(action==UI_AI_GRAPH_MIGRATE){if(strcmp(binding,c->binding)||c->schema!=2||s->active>=0||s->queued||s->cmd_count||ai_graph_pending(s)||(s->edit&&(s->edit->job||s->edit->busy||s->edit->live||s->edit->recovered)))return UI_STATUS_PERMISSION_DENIED;return ag_queue(s,2,-1)?UI_STATUS_OK:UI_STATUS_LIMIT_EXCEEDED;}
 at=ai_find(s,id);if(at<0||!(g=s->runs[at].graph)||g->cancel||g->record_only)return UI_STATUS_NOT_FOUND;if(node&&*node){at=ag_index(g,node);if(at<0)return UI_STATUS_NOT_FOUND;n=&g->nodes[at];}
 if(action==UI_AI_GRAPH_CONFIRM){
  ai_run_t *root=&s->runs[ai_find(s,id)];as_run_t *assistant=root->model?root->model->assistant:NULL;
  if(assistant&&(!display||!display->display_check||assistant->stage!=AS_PLAN_WAIT||assistant->control!=AS_CONTROL_AI))return UI_STATUS_PERMISSION_DENIED;
  ui_status_t status=UI_STATUS_PERMISSION_DENIED;
  EnterCriticalSection(&s->lock);++root->refs;LeaveCriticalSection(&s->lock);
  if(display&&display->display_check&&display->display_check(display->display_data,id,binding,display->display_generation))goto confirm_done;
  if(root->cancel||strcmp(binding,g->binding)||g->confirmed||!g->needs_scope||g->persisted!=g->version||root->terminal)goto confirm_done;
  for(unsigned i=0;i<g->count;++i){status=ag_check(s,g,&g->nodes[i]);if(status)goto confirm_done;}
  if(assistant&&!as_current(s,root)){status=UI_STATUS_PERMISSION_DENIED;goto confirm_done;}
  /* No application callback follows this final display check. */
  if(display&&display->display_check&&display->display_check(display->display_data,id,binding,display->display_generation)){status=UI_STATUS_PERMISSION_DENIED;goto confirm_done;}
  EnterCriticalSection(&s->lock);
  if(s->phase!=UI_AI_READY||s->close_requested||root->cancel||root->terminal||s->store_failed||
     root->graph!=g||g->cancel||g->confirmed||!g->needs_scope||g->persisted!=g->version||strcmp(binding,g->binding))status=UI_STATUS_CANCELLED;
  else if(assistant&&(assistant->stage!=AS_PLAN_WAIT||assistant->control!=AS_CONTROL_AI||assistant->stopped||
      (assistant->operation&&(assistant->operation->phase!=AO_REVIEW||assistant->operation->granted||
      !root->model->confirmation_saved||assistant->fee_unknown||ai_now(s)>=assistant->until))))status=UI_STATUS_PERMISSION_DENIED;
  else{g->confirmed=1;g->scope_until=assistant&&assistant->operation?assistant->until:ai_now(s)+UI_AI_EDIT_SCOPE_MS;if(assistant)assistant->stage=AS_EXECUTING;ag_dirty(g);status=UI_STATUS_OK;}
  LeaveCriticalSection(&s->lock);
 confirm_done:
  EnterCriticalSection(&s->lock);--root->refs;LeaveCriticalSection(&s->lock);return status;}
 if(!n||!n->edit||strcmp(binding,n->edit->binding))return UI_STATUS_PERMISSION_DENIED;
 if(action==UI_AI_GRAPH_APPROVE){if(g->active>=4)return UI_STATUS_LIMIT_EXCEEDED;if(n->state!=AG_APPROVAL||!g->confirmed||ai_now(s)>=g->scope_until)return UI_STATUS_PERMISSION_DENIED;{ui_status_t status=ae_action_one(s,n->edit,UI_AI_EDIT_APPROVE,binding);if(!status){n->state=AG_RUNNING;ag_dirty(g);}return status;}}
 if(action==UI_AI_GRAPH_RECONCILE)return ae_action_one(s,n->edit,UI_AI_EDIT_RECONCILE,binding);
 return UI_STATUS_PERMISSION_DENIED;
}
ui_status_t ui_ai_graph_bind_light(ui_ai_service_t *s,const ui_ai_graph_light_binding_t *d,uint64_t *id)
{
 ai_coordinator_t *c;ag_binding_t *b;unsigned count=0;if(id)*id=0;if(!ai_ui(s)||!id||!d||d->size<sizeof(*d)||d->version!=1||!d->view||!ai_text(d->control_id,127,0)||!ai_text(d->binding_hash,64,0)||!ai_text(d->run_id,64,1)||!ai_text(d->node_id,64,1)||d->action<UI_AI_GRAPH_CONFIRM||d->action>UI_AI_GRAPH_MIGRATE)return UI_STATUS_INVALID_ARGUMENT;
 c=s->coordinator;if(!c||d->view->input_workspace!=s->config.workspace||!d->view->backend->trusted_light)return UI_STATUS_PERMISSION_DENIED;for(b=c->bindings;b;b=b->next){++count;if(b->d.view==d->view&&!strcmp(b->control,d->control_id))return UI_STATUS_ALREADY_EXISTS;}
 if(count==16||sizeof(*b)>UI_AI_MAX_BYTES-s->bytes||c->sequence==UINT64_MAX)return UI_STATUS_LIMIT_EXCEEDED;b=calloc(1,sizeof(*b));if(!b)return UI_STATUS_OUT_OF_MEMORY;b->d=*d;strcpy_s(b->control,128,d->control_id);strcpy_s(b->run,65,d->run_id);strcpy_s(b->node,65,d->node_id);strcpy_s(b->hash,65,d->binding_hash);b->id=++c->sequence;b->next=c->bindings;c->bindings=b;s->bytes+=sizeof(*b);*id=b->id;return UI_STATUS_OK;
}
ui_status_t ui_ai_graph_unbind(ui_ai_service_t *s,uint64_t id)
{ai_coordinator_t *c;ag_binding_t **at,*b;if(!ai_ui(s)||!(c=s->coordinator))return UI_STATUS_INVALID_ARGUMENT;if(s->methods||s->polling)return UI_STATUS_PENDING;for(at=&c->bindings;*at&&(*at)->id!=id;at=&(*at)->next){}if(!*at)return UI_STATUS_NOT_FOUND;b=*at;*at=b->next;s->bytes-=sizeof(*b);free(b);return UI_STATUS_OK;}
ui_status_t ui_ai_graph_light_action(ui_ai_service_t *s,ui_web_view_t *view,const char *node)
{ag_binding_t *b;ui_status_t status;if(!ai_ui(s)||!s->coordinator)return UI_STATUS_UNSUPPORTED;for(b=s->coordinator->bindings;b;b=b->next)if(b->d.view==view&&!strcmp(b->control,node))break;if(!b)return UI_STATUS_UNSUPPORTED;++s->methods;
 if(!ui_inputs_consume_action(view->host,view->user_data)||b->used||view->input_workspace!=s->config.workspace||(view->private_input.kind!=UI_INPUT_POINTER_UP&&view->private_input.kind!=UI_INPUT_KEY_DOWN)||(view->private_input.kind==UI_INPUT_POINTER_UP?view->private_input.pointer_button!=1:(view->private_input.key_code!=13&&view->private_input.key_code!=32)))status=UI_STATUS_PERMISSION_DENIED;
 else{b->used=1;status=b->node_undo?as_undo_action(s,b):ag_action(s,b->d.action,b->run,b->node,b->hash,b);}--s->methods;return status;
}
void ui_ai_graph_light_detach(ui_ai_service_t *s,ui_web_view_t *view)
{ag_binding_t *b;if(!ai_ui(s)||!s->coordinator)return;for(b=s->coordinator->bindings;b;b=b->next)if(b->d.view==view){b->d.view=NULL;b->used=1;}}
static void ai_graph_dispose(ui_ai_service_t *s)
{ai_coordinator_t *c=s->coordinator;if(!c)return;while(c->bindings){ag_binding_t *b=c->bindings;c->bindings=b->next;free(b);}free(c);s->coordinator=NULL;}
#ifdef UI_AI_EDIT_TEST_DLL
__declspec(dllexport) ui_status_t ui_ai_graph_fixture_light(ui_ai_service_t *s,ui_web_view_t *view,const char *node)
{ui_status_t status;view->private_input.size=sizeof(view->private_input);view->private_input.kind=UI_INPUT_POINTER_UP;view->private_input.pointer_button=1;ui_capabilities_human_begin(view->host,view->user_data);status=ui_ai_graph_light_action(s,view,node);ui_capabilities_human_end(view->host);return status;}
#endif
#ifdef UI_AI_FIXTURE
#ifdef UI_AI_EDIT_TEST_DLL
__declspec(dllexport)
#endif
ui_status_t ui_ai_graph_fixture_action(ui_ai_service_t *s,int action,const char *id,const char *node,const char *hash)
{ui_status_t status;if(!ai_ui(s))return UI_STATUS_INVALID_ARGUMENT;++s->methods;status=ag_action(s,(ui_ai_graph_action_t)action,id,node,hash,NULL);--s->methods;return status;}
#ifdef UI_AI_EDIT_TEST_DLL
__declspec(dllexport)
#endif
void ui_ai_graph_fixture(ui_ai_service_t *s,int mode,int value)
{if(!ai_ui(s)||!s->coordinator)return;EnterCriticalSection(&s->lock);if(mode==1)s->coordinator->fixture_hold=value;if(mode==2)s->coordinator->fixture_fail=value;(void)ai_start_locked(s);LeaveCriticalSection(&s->lock);}
#endif
