/* RPCB-1 is a restrictive policy on the original Service, never another agent. */
static int ab_restrict_submit(ui_ai_service_t *s,am_run_t *model)
{return s->validation&&s->validation->batch&&!model;}
static void ab_fail(ui_ai_service_t *s,const char *error)
{EnterCriticalSection(&s->lock);ab_batch_t *b=s->validation->batch;if(!b->error[0])strcpy_s(b->error,96,error);b->ending=1;av_stop_locked(s,error);LeaveCriticalSection(&s->lock);}
static void ab_credential_notice(ui_ai_service_t *s)
{
 if(!s->validation||!s->validation->batch)return;am_config_t *c=s->model_config;char json[512],utf[4096];wchar_t path[4096];
 EnterCriticalSection(&s->lock);snprintf(json,sizeof(json),"{\"policy\":\"RPCB-1\",\"local_readable\":%s,\"need_key\":%s,\"revision\":%llu,\"read_error\":%lu,\"credential_ref\":\"%s\"}",c->credential.readable?"true":"false",c->credential.readable?"false":"true",(unsigned long long)c->revision,(unsigned long)c->credential.read_code,c->ref);LeaveCriticalSection(&s->lock);
 snprintf(utf,sizeof(utf),"%s.credential-check.json",s->path);int ok=0;if(MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,utf,-1,path,4096)){HANDLE h=CreateFileW(path,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);if(h!=INVALID_HANDLE_VALUE){DWORD n=0;ok=WriteFile(h,json,(DWORD)strlen(json),&n,NULL)&&n==strlen(json)&&FlushFileBuffers(h);CloseHandle(h);}}if(!ok)ab_fail(s,"batch_credential_diagnostic_failed");
}
static int ab_conditions(ui_ai_service_t *s)
{
 ab_batch_t *b=s->validation->batch;am_config_t *c=s->model_config;char hash[65];ui_ai_task_request_t q={0};
 EnterCriticalSection(&s->lock);int ok=s->phase==UI_AI_READY&&!s->store_failed&&!s->close_requested&&!c->job&&!c->busy&&c->revision==b->revision&&!strcmp(c->binding,b->profile)&&c->credential.readable&&c->credential.revision==c->revision;LeaveCriticalSection(&s->lock);
 if(!ok)return 0;q.targets=&b->target;q.target_count=1;
 if(ai_targets_check(s,&q,1)||!am_catalog(s,&b->target,1,"{\"capabilities\":[\"memory.read\"],\"fields\":[\"value\",\"revision\"]}",hash)||strcmp(hash,b->catalog))return 0;
 EnterCriticalSection(&s->lock);ok=s->phase==UI_AI_READY&&!s->validation->stopped&&!s->store_failed&&!s->close_requested&&!c->job&&!c->busy&&c->revision==b->revision&&!strcmp(c->binding,b->profile);LeaveCriticalSection(&s->lock);return ok;
}
static int ab_preview(ui_ai_service_t *s)
{
 ab_batch_t *b=s->validation->batch;am_config_t *c=s->model_config;ui_ai_task_request_t q={0};q.targets=&b->target;q.target_count=1;char *targets=ai_targets_json(&q),*raw=NULL;ui_cap_json_doc_t tools={0};yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *o=d?yyjson_mut_obj(d):NULL;int ok=0;
 if(!o||!targets||!am_json_parse(am_tool_definitions,strlen(am_tool_definitions),&tools))goto done;yyjson_mut_doc_set_root(d,o);
 yyjson_mut_obj_add_strcpy(d,o,"policy","RPCB-1");yyjson_mut_obj_add_strcpy(d,o,"batch_id",b->id);yyjson_mut_obj_add_strcpy(d,o,"session",s->session);yyjson_mut_obj_add_strcpy(d,o,"origin","https://api.deepseek.com");yyjson_mut_obj_add_strcpy(d,o,"model","deepseek-flash");yyjson_mut_obj_add_strcpy(d,o,"credential_ref",c->ref);yyjson_mut_obj_add_uint(d,o,"profile_revision",c->revision);yyjson_mut_obj_add_strcpy(d,o,"profile_binding",c->binding);yyjson_mut_obj_add_strcpy(d,o,"catalog_hash",b->catalog);yyjson_mut_obj_add_strcpy(d,o,"fixed_target",targets);
 yyjson_mut_obj_add_strcpy(d,o,"T2",av_inputs[2]);yyjson_mut_obj_add_strcpy(d,o,"T3",av_inputs[3]);yyjson_mut_obj_add_strcpy(d,o,"history","[]");yyjson_mut_obj_add_strcpy(d,o,"capabilities","memory.read");yyjson_mut_obj_add_strcpy(d,o,"fields","value,revision");yyjson_mut_obj_add_strcpy(d,o,"result_required","integer10/string r1/framework revision r1, durable discovery and tool proof");yyjson_mut_obj_add_strcpy(d,o,"instructions",am_system_instructions);yyjson_mut_obj_add_val(d,o,"tools",yyjson_val_mut_copy(d,yyjson_doc_get_root(tools.doc)));
 yyjson_mut_obj_add_strcpy(d,o,"quotas","T1=0; T2<=3; T3<=1; totalPOST<=4; no retries/refunds");yyjson_mut_obj_add_uint(d,o,"budget_nano_usd",AV_BUDGET);yyjson_mut_obj_add_uint(d,o,"duration_ms",600000);yyjson_mut_obj_add_strcpy(d,o,"network_seconds","10/30/30/120");yyjson_mut_obj_add_strcpy(d,o,"fee_policy","local conservative usage/reference; not server hard cap; unknown stops");
 raw=yyjson_mut_write(d,0,NULL);if(raw&&strlen(raw)<sizeof(b->preview)&&!ui_cap_json_fingerprint(raw,strlen(raw),b->hash)){strcpy_s(b->preview,sizeof(b->preview),raw);b->revision=c->revision;strcpy_s(b->profile,65,c->binding);b->prepared=1;ok=1;}
done:free(raw);free(targets);am_json_free(&tools);yyjson_mut_doc_free(d);return ok;
}
ui_status_t ui_ai_validation_batch_open(ui_ai_service_t *s,const ui_ai_validation_batch_desc_t *d)
{
 if(!ai_ui(s)||!d||d->size<sizeof(*d)||d->version!=1||!d->target||!ai_text(d->model_config_path_utf8,3990,0)||strlen(s->path)>3990)return UI_STATUS_INVALID_ARGUMENT;
 if(s->model_config||s->validation)return UI_STATUS_ALREADY_EXISTS;
 if(s->assistant_policy)return UI_STATUS_PERMISSION_DENIED;if(s->phase!=UI_AI_READY||s->close_requested)return UI_STATUS_PENDING;
 if(d->target->size<sizeof(ui_cap_target_t)||d->target->version!=1)return UI_STATUS_INVALID_ARGUMENT;char config_path[3991];strcpy_s(config_path,sizeof(config_path),d->model_config_path_utf8);ui_cap_target_t target=*d->target;ui_ai_task_request_t q={0};q.targets=&target;q.target_count=1;++s->methods;ui_status_t st=ai_targets_check(s,&q,1);
 if(!st&&(strcmp(target.resource_id,"memory.document")||strcmp(target.expected_revision,"r1")))st=UI_STATUS_PERMISSION_DENIED;
 if(!st&&(s->phase!=UI_AI_READY||s->close_requested))st=UI_STATUS_CANCELLED;
 if(st){--s->methods;return st;}EnterCriticalSection(&s->lock);size_t bytes=sizeof(av_policy_t)+sizeof(ab_batch_t);if(bytes>UI_AI_MAX_BYTES-s->bytes){LeaveCriticalSection(&s->lock);--s->methods;return UI_STATUS_LIMIT_EXCEEDED;}
 av_policy_t *p=calloc(1,sizeof(*p));ab_batch_t *b=calloc(1,sizeof(*b));if(!p||!b){free(p);free(b);LeaveCriticalSection(&s->lock);--s->methods;return UI_STATUS_OUT_OF_MEMORY;}p->batch=b;b->target=target;if(!ai_uuid(b->id)){free(b);free(p);LeaveCriticalSection(&s->lock);--s->methods;return UI_STATUS_PLATFORM_ERROR;}strcpy_s(p->id,65,"rpc-batch-T2-T3");p->input_rate=300;p->output_rate=1200;s->validation=p;s->bytes+=bytes;LeaveCriticalSection(&s->lock);
 st=am_open_reference(s,config_path,AB_REFERENCE);if(st){av_dispose(s);--s->methods;return st;}EnterCriticalSection(&s->lock);p->job=1;(void)ai_start_locked(s);LeaveCriticalSection(&s->lock);--s->methods;return UI_STATUS_OK;
}
static ui_status_t ab_accept(ui_ai_service_t *s,const char *hash)
{
 av_policy_t *p=s->validation;ab_batch_t *b=p?p->batch:NULL;if(!b||!hash)return UI_STATUS_PERMISSION_DENIED;
 EnterCriticalSection(&s->lock);if(!b->prepared||b->accepting||b->accepted||p->stopped||strcmp(hash,b->hash)){LeaveCriticalSection(&s->lock);return UI_STATUS_PERMISSION_DENIED;}b->accepting=1;++s->methods;LeaveCriticalSection(&s->lock);
 int ok=ab_conditions(s);EnterCriticalSection(&s->lock);b->accepting=0;if(ok){if(s->phase!=UI_AI_READY||s->close_requested||s->store_failed||p->stopped||p->job||b->busy||b->accepted)ok=0;else{b->accepted=1;p->begin=ai_now(s);p->job=2;(void)ai_start_locked(s);}}--s->methods;LeaveCriticalSection(&s->lock);if(!ok)ab_fail(s,"batch_conditions_changed");return ok?UI_STATUS_OK:UI_STATUS_PERMISSION_DENIED;
}
ui_status_t ui_ai_validation_batch_bind_button(ui_ai_service_t *s,const ui_ai_validation_batch_button_t *d,uint64_t *out)
{
 if(out)*out=0;if(!ai_ui(s)||!d||d->size<sizeof(*d)||d->version!=1||!out||!d->batch_hash)return UI_STATUS_INVALID_ARGUMENT;
 if(!s->validation||!s->validation->batch||!s->validation->batch->prepared||s->validation->batch->accepted)return UI_STATUS_PENDING;
 ui_ai_model_button_binding_t bind={0};bind.size=sizeof(bind);bind.version=1;bind.button=d->button;bind.binding_hash=d->batch_hash;bind.action=(ui_ai_model_action_t)AB_START;return am_bind_button(s,&bind,out,1);
}
ui_status_t ui_ai_validation_batch_get_state(ui_ai_service_t *s,char **out)
{
 if(out)*out=NULL;if(!ai_ui(s)||!out)return UI_STATUS_INVALID_ARGUMENT;av_policy_t *p=s->validation;ab_batch_t *b=p?p->batch:NULL;if(!b)return UI_STATUS_NOT_FOUND;
 yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *o=d?yyjson_mut_obj(d):NULL;if(!o){yyjson_mut_doc_free(d);return UI_STATUS_OUT_OF_MEMORY;}yyjson_mut_doc_set_root(d,o);EnterCriticalSection(&s->lock);
 yyjson_mut_obj_add_strcpy(d,o,"batch_id",b->id);yyjson_mut_obj_add_strcpy(d,o,"batch_hash",b->hash);yyjson_mut_obj_add_strcpy(d,o,"preview",b->preview);yyjson_mut_obj_add_strcpy(d,o,"error",b->error);yyjson_mut_obj_add_bool(d,o,"ready",b->prepared&&!b->accepted&&!p->stopped);yyjson_mut_obj_add_bool(d,o,"need_key",b->need_key!=0);yyjson_mut_obj_add_bool(d,o,"accepted",b->accepted!=0);yyjson_mut_obj_add_bool(d,o,"saved",b->saved!=0);yyjson_mut_obj_add_bool(d,o,"discovery_proven",b->discovered!=0);yyjson_mut_obj_add_bool(d,o,"read10_r1_proven",b->read_proven!=0);yyjson_mut_obj_add_bool(d,o,"cancel_requested",b->cancel_requested!=0);yyjson_mut_obj_add_bool(d,o,"cancel_observed_inflight",b->cancel_inflight!=0);yyjson_mut_obj_add_bool(d,o,"cancel_window_missed",b->cancel_missed!=0);yyjson_mut_obj_add_bool(d,o,"ending",b->ending!=0);yyjson_mut_obj_add_bool(d,o,"export_done",b->export_done!=0);yyjson_mut_obj_add_bool(d,o,"export_ok",b->export_ok!=0);yyjson_mut_obj_add_strcpy(d,o,"T2_run",p->run[1]);yyjson_mut_obj_add_strcpy(d,o,"T3_run",p->run[2]);LeaveCriticalSection(&s->lock);*out=yyjson_mut_write(d,0,NULL);yyjson_mut_doc_free(d);return *out?UI_STATUS_OK:UI_STATUS_OUT_OF_MEMORY;
}
static int ab_tool_allowed(ui_ai_service_t *s,ai_run_t *r,yyjson_val *step)
{
 ab_batch_t *b=s->validation?s->validation->batch:NULL;if(!b)return 1;if(r->model->validation_kind!=2)return 0;const char *type=am_s(step,"type");if(type&&!strcmp(type,"discover"))return am_s(step,"category")&&!strcmp(am_s(step,"category"),"memory");return b->discovered&&am_s(step,"capability_id")&&!strcmp(am_s(step,"capability_id"),"memory.read");
}
static int ab_result(ui_ai_service_t *s,ai_run_t *r,ai_tool_t *t)
{
 ab_batch_t *b=s->validation?s->validation->batch:NULL;if(!b)return 1;if(t->status||r->model->validation_kind!=2||!t->result)return 0;
 ui_cap_json_doc_t d={0};int valid=am_json_parse(t->result,strlen(t->result),&d);yyjson_val *o=valid?yyjson_doc_get_root(d.doc):NULL;
 if(t->discover){size_t i,n;yyjson_val *v;valid=0;yyjson_arr_foreach(o,i,n,v){const char *id=am_s(v,"id");if(id&&!strcmp(id,"memory.read")){yyjson_val *schema=am_obj(v,"output_schema"),*props=am_obj(schema,"properties");valid=am_s(v,"effect")&&!strcmp(am_s(v,"effect"),"read")&&yyjson_get_uint(am_obj(v,"capability_version"))==1&&am_s(v,"name")&&!strcmp(am_s(v,"name"),"Read synthetic memory value and revision")&&am_s(am_obj(props,"value"),"type")&&!strcmp(am_s(am_obj(props,"value"),"type"),"integer")&&am_s(am_obj(props,"revision"),"type")&&!strcmp(am_s(am_obj(props,"revision"),"type"),"string");break;}}}
 else valid=valid&&b->discovered&&yyjson_is_obj(o)&&yyjson_obj_size(o)==2&&yyjson_is_uint(am_obj(o,"value"))&&yyjson_get_uint(am_obj(o,"value"))==10&&am_s(o,"revision")&&!strcmp(am_s(o,"revision"),"r1")&&!strcmp(t->result_revision,"r1");am_json_free(&d);if(!valid)return 0;
 char payload[512],hash[65],suffix[128];ui_cap_hash_bytes(t->result,strlen(t->result),hash);snprintf(payload,sizeof(payload),"{\"tool_call_id\":\"%s\",\"kind\":\"%s\",\"result_hash\":\"%s\",\"framework_revision\":\"%s\",\"value\":%d,\"revision\":\"r1\"}",t->id,t->discover?"discovery":"read",hash,t->discover?"":t->result_revision,t->discover?0:10);snprintf(suffix,sizeof(suffix),"batch:proof:%s",t->id);
 if(!av_event(s,r,suffix,"rpc_batch_tool_proof",payload)||!ai_store_available(s))return 0;EnterCriticalSection(&s->lock);if(t->discover)b->discovered=1;else b->read_proven=1;LeaveCriticalSection(&s->lock);return 1;
}
static int ab_export(ui_ai_service_t *s)
{
 av_policy_t *p=s->validation;ab_batch_t *b=p->batch;void *pool=NULL;char *json=NULL;yyjson_mut_doc *d=NULL;int ok=0;char fallback[256];
 EnterCriticalSection(&s->lock);int charged=AB_EXPORT_BYTES<=UI_AI_MAX_BYTES-s->bytes;if(charged)s->bytes+=AB_EXPORT_BYTES;LeaveCriticalSection(&s->lock);
 if(charged)pool=malloc(AB_EXPORT_BYTES);yyjson_alc alc;if(pool&&yyjson_alc_pool_init(&alc,pool,AB_EXPORT_BYTES)){d=yyjson_mut_doc_new(&alc);yyjson_mut_val *root=d?yyjson_mut_obj(d):NULL;if(root){yyjson_mut_doc_set_root(d,root);ok=1;
  const char *tables[]={"runs","messages","events","tools"};for(unsigned k=0;k<4&&ok;++k){char sql[96];snprintf(sql,sizeof(sql),"SELECT * FROM %s",tables[k]);sqlite3_stmt *q=NULL;yyjson_mut_val *a=yyjson_mut_arr(d);ok=a&&yyjson_mut_obj_add_val(d,root,tables[k],a)&&ai_statement(s,sql,&q);unsigned count=0;int rc=SQLITE_DONE;
   while(ok&&(rc=sqlite3_step(q))==SQLITE_ROW){if(++count>256){ok=0;break;}yyjson_mut_val *row=yyjson_mut_obj(d);ok=row&&yyjson_mut_arr_add_val(a,row);for(int i=0;i<sqlite3_column_count(q)&&ok;++i){yyjson_mut_val *key=yyjson_mut_strcpy(d,sqlite3_column_name(q,i)),*value=NULL;int type=sqlite3_column_type(q,i);if(type==SQLITE_NULL)value=yyjson_mut_null(d);else if(type==SQLITE_INTEGER)value=yyjson_mut_sint(d,sqlite3_column_int64(q,i));else if(type==SQLITE_TEXT)value=yyjson_mut_strcpy(d,(const char *)sqlite3_column_text(q,i));ok=key&&value&&yyjson_mut_obj_add(row,key,value);}}
   if(rc!=SQLITE_DONE)ok=0;sqlite3_finalize(q);
  }
  if(ok){yyjson_mut_obj_add_strcpy(d,root,"policy","RPCB-1");yyjson_mut_obj_add_strcpy(d,root,"batch_id",b->id);yyjson_mut_obj_add_strcpy(d,root,"batch_hash",b->hash);yyjson_mut_obj_add_strcpy(d,root,"preview",b->preview);yyjson_mut_obj_add_strcpy(d,root,"error",b->error);am_credential_json(d,root,&s->model_config->credential,s->model_config->revision);yyjson_mut_obj_add_uint(d,root,"profile_revision",s->model_config->revision);yyjson_mut_obj_add_strcpy(d,root,"policy_error",p->error);yyjson_mut_obj_add_strcpy(d,root,"service_error",s->error);yyjson_mut_obj_add_bool(d,root,"send_count_saved",!p->send_unconfirmed);yyjson_mut_obj_add_bool(d,root,"send_count_known",!p->send_pending&&!p->prior);yyjson_mut_obj_add_uint(d,root,"M",p->manifests);yyjson_mut_obj_add_uint(d,root,"R",p->reserved);yyjson_mut_obj_add_uint(d,root,"S",p->sends);yyjson_mut_obj_add_uint(d,root,"C",p->complete);yyjson_mut_obj_add_uint(d,root,"U",p->unknown);yyjson_mut_obj_add_uint(d,root,"simulated_S",p->mock_sends);yyjson_mut_obj_add_uint(d,root,"known_nano_usd",p->known);yyjson_mut_obj_add_uint(d,root,"estimated_nano_usd",p->estimated);yyjson_mut_obj_add_bool(d,root,"fee_unknown",p->fee_unknown!=0);yyjson_mut_obj_add_bool(d,root,"read10_r1_proven",b->read_proven!=0);yyjson_mut_obj_add_bool(d,root,"discovery_proven",b->discovered!=0);yyjson_mut_obj_add_bool(d,root,"cancel_observed_inflight",b->cancel_inflight!=0);yyjson_mut_obj_add_bool(d,root,"cancel_window_missed",b->cancel_missed!=0);json=yyjson_mut_write_opts(d,0,&alc,NULL,NULL);ok=json!=NULL;}
 }}
 if(!json){snprintf(fallback,sizeof(fallback),"{\"policy\":\"RPCB-1\",\"error\":\"batch_export_unconfirmed\",\"M\":%u,\"R\":%u,\"S\":%u}",p->manifests,p->reserved,p->sends);json=fallback;ok=0;}
 wchar_t path[4096];char utf[4096];snprintf(utf,sizeof(utf),"%s.batch.json",s->path);DWORD error=0,written=0;int file_ok=0;if(MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,utf,-1,path,4096)){HANDLE h=CreateFileW(path,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);if(h!=INVALID_HANDLE_VALUE){file_ok=WriteFile(h,json,(DWORD)strlen(json),&written,NULL)&&written==strlen(json)&&FlushFileBuffers(h);if(!file_ok)error=GetLastError();CloseHandle(h);}else error=GetLastError();}else error=GetLastError();
 yyjson_mut_doc_free(d);free(pool);EnterCriticalSection(&s->lock);if(charged)s->bytes-=AB_EXPORT_BYTES;b->export_ok=ok&&file_ok;b->export_done=1;if(!b->export_ok&&!b->error[0])snprintf(b->error,96,"batch_export_failed_%lu",(unsigned long)error);LeaveCriticalSection(&s->lock);return 1;
}
static int ab_worker(ui_ai_service_t *s,int job)
{
 ab_batch_t *b=s->validation->batch;if(!b)return 0;EnterCriticalSection(&s->lock);b->busy=1;LeaveCriticalSection(&s->lock);
 if(job==2){char record[8400];snprintf(record,sizeof(record),"{\"batch_hash\":\"%s\",\"preview\":%s}",b->hash,b->preview);int saved=av_event(s,NULL,"batch:confirm","rpc_batch_confirm",record)&&ai_store_available(s);EnterCriticalSection(&s->lock);b->saved=saved;b->busy=0;LeaveCriticalSection(&s->lock);if(!saved)ab_fail(s,"batch_confirmation_unconfirmed");return 1;}
 int result=ab_export(s);EnterCriticalSection(&s->lock);b->busy=0;LeaveCriticalSection(&s->lock);return result;
}
static void ab_ui(ui_ai_service_t *s)
{
 av_policy_t *p=s->validation;ab_batch_t *b=p?p->batch:NULL;am_config_t *c=s->model_config;if(!b||!c)return;
 ab_batch_t view;am_credential_status_t credential;int pending,ready,blocked,policy_ready;char model[128];
 EnterCriticalSection(&s->lock);if(s->phase==UI_AI_CLOSED){LeaveCriticalSection(&s->lock);return;}
 if(p->stopped||s->store_failed||s->close_requested){b->ending=1;if(!b->error[0])strcpy_s(b->error,96,p->error[0]?p->error:"batch_closed");}
 view=*b;credential=c->credential;pending=c->job||c->busy;ready=c->ready;blocked=c->blocked;policy_ready=p->ready;strcpy_s(model,128,c->model);LeaveCriticalSection(&s->lock);
 if(!view.ending&&!view.accepted&&policy_ready&&!pending){
  int entry_failed=!strcmp(credential.stage,"cancelled")||!strcmp(credential.stage,"prompt_failed")||!strcmp(credential.stage,"write_failed")||!strcmp(credential.stage,"config_failed")||(credential.submitted&&!strcmp(credential.stage,"read_failed"));
  if(entry_failed)ab_fail(s,"batch_credential_entry_failed");
  else if(blocked)ab_fail(s,"batch_configuration_unavailable");
  else if(!ready&&!view.configure_requested){ui_ai_model_profile_t v={sizeof(v),1,"deepseek","deepseek-responses-v1","https://api.deepseek.com","deepseek-flash",c->ref,c->revision+1};EnterCriticalSection(&s->lock);b->configure_requested=1;LeaveCriticalSection(&s->lock);if(ui_ai_model_configure(s,&v,"batch-profile"))ab_fail(s,"batch_configuration_failed");}
  else if(ready&&strcmp(model,"deepseek-flash"))ab_fail(s,"batch_model_changed");
  else if(ready&&!view.check_requested){EnterCriticalSection(&s->lock);b->check_requested=1;c->job=4;(void)ai_start_locked(s);LeaveCriticalSection(&s->lock);}
  else if(ready&&view.check_requested){int need=!(credential.readable&&credential.revision==c->revision);EnterCriticalSection(&s->lock);b->need_key=need;LeaveCriticalSection(&s->lock);if(!need&&!view.prepared){char hash[65];if(!am_catalog(s,&b->target,1,"{\"capabilities\":[\"memory.read\"],\"fields\":[\"value\",\"revision\"]}",hash))ab_fail(s,"batch_catalog_invalid");else{strcpy_s(b->catalog,65,hash);if(!ab_preview(s))ab_fail(s,"batch_preview_limit");}}}
 }
 EnterCriticalSection(&s->lock);view=*b;LeaveCriticalSection(&s->lock);
 if(!view.ending&&view.prepared&&!ab_conditions(s))ab_fail(s,"batch_conditions_changed");
 EnterCriticalSection(&s->lock);view=*b;LeaveCriticalSection(&s->lock);
 if(!view.ending&&view.accepted&&view.saved){
  unsigned task=2;int invalid=0;
  EnterCriticalSection(&s->lock);int prior=ai_find(s,p->run[1]);if(prior>=0&&s->runs[prior].terminal){task=3;ai_run_t *read=&s->runs[prior];invalid=strcmp(read->outcome,"completed")||!read->committed||!b->discovered||!b->read_proven||p->fee_unknown;}int missing=!p->run[task-1][0];LeaveCriticalSection(&s->lock);
  if(invalid)ab_fail(s,"batch_read_proof_missing");
  else{
   if(missing){ui_ai_validation_request_t q={0};char id[65];q.size=sizeof(q);q.version=1;q.task=(ui_ai_validation_task_t)task;q.client_request_id=task==2?"batch-T2":"batch-T3";q.target=task==2?&b->target:NULL;EnterCriticalSection(&s->lock);b->submitting=1;LeaveCriticalSection(&s->lock);ui_status_t status=ui_ai_validation_propose(s,&q,id);EnterCriticalSection(&s->lock);b->submitting=0;LeaveCriticalSection(&s->lock);if(status)ab_fail(s,"batch_submit_rejected");}
   char run_id[65]={0},binding[65]={0};int confirm=0,cancel=0;
   EnterCriticalSection(&s->lock);int at=ai_find(s,p->run[task-1]);if(!b->ending&&at>=0){ai_run_t *r=&s->runs[at];am_run_t *m=r->model;strcpy_s(run_id,65,r->id);if(m&&!r->terminal&&m->wait_saved&&!m->confirmed){confirm=1;strcpy_s(binding,65,m->binding);b->confirming=1;}}LeaveCriticalSection(&s->lock);
   if(confirm){ui_status_t status=am_accept(s,run_id,binding,NULL);EnterCriticalSection(&s->lock);b->confirming=0;LeaveCriticalSection(&s->lock);if(status)ab_fail(s,"batch_task_binding_changed");}
   EnterCriticalSection(&s->lock);at=ai_find(s,p->run[task-1]);if(!b->ending&&task==3&&at>=0){ai_run_t *r=&s->runs[at];am_run_t *m=r->model;if(m&&!r->terminal&&!b->cancel_requested&&m->net.sent&&m->net.started&&!m->net.finished&&!m->net.closed&&!m->net.closing&&m->validation_ticket&&p->tickets[m->validation_ticket-1].entered==2){b->cancel_requested=1;b->cancel_inflight=!p->tickets[m->validation_ticket-1].simulated;cancel=1;}if(r->terminal){if(!b->cancel_requested)b->cancel_missed=1;b->ending=1;}}LeaveCriticalSection(&s->lock);
   if(cancel)(void)ui_ai_cancel(s,run_id);
  }
 }
 int quiesce=0,close=0;EnterCriticalSection(&s->lock);if(b->ending&&!b->quiesced){b->quiesced=1;quiesce=1;}LeaveCriticalSection(&s->lock);if(quiesce)(void)ui_ai_service_quiesce(s);
 EnterCriticalSection(&s->lock);if(b->ending){int drained=s->tool.state==AI_TOOL_NONE&&!c->job&&!c->busy;for(unsigned i=0;i<AI_RUNS;++i){ai_run_t *r=&s->runs[i];if(r->model&&((r->model->net.started&&!r->model->net.finished)||(!r->terminal&&!s->store_failed&&!r->model->recovered)))drained=0;}if(drained&&!b->export_done&&!p->job&&!b->busy){p->job=3;(void)ai_start_locked(s);}close=b->export_done;}LeaveCriticalSection(&s->lock);if(close)(void)ui_ai_service_request_close(s);
}
