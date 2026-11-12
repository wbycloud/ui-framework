/* Application-local transaction sequencing. Snapshot contents and actual
 * state restore/project are owned by each example, never by the AI database. */
static void p4_try_complete(p2_app_t *a)
{
 ui_status_t status;if(!a->edit_pending_call)return;
 a->edit_pending.executor_exited=!a->quiet||a->quiet(a->model,a->edit_pending_resource);
 status=ui_capability_edit_complete(a->context.host,a->edit_pending_call,&a->edit_pending);
 if(status==UI_STATUS_OK||status==UI_STATUS_ALREADY_EXISTS||status==UI_STATUS_NOT_FOUND){a->edit_pending_call=0;free(a->edit_pending_json);a->edit_pending_json=NULL;a->edit_pending.output_json=NULL;}
}
static void p4_complete(p2_cap_t *c,const ui_cap_edit_context_t *stage,ui_status_t status,const char *output,ui_cap_effect_v2_t effect)
{
 p2_app_t *a=c->app;ui_cap_edit_completion_t *result=&a->edit_pending;if(a->edit_pending_call)return;
 memset(result,0,sizeof(*result));result->size=sizeof(*result);result->version=1;result->stage_id=stage->stage_id;result->status=status;result->effect=effect;
 if(c->spec->resource==0){strcpy_s(result->transaction_id,65,a->transaction_id);strcpy_s(result->undo_id,65,a->undo_receipt);}
 a->edit_pending_json=output?_strdup(output):NULL;if(output&&!a->edit_pending_json){result->status=UI_STATUS_OUT_OF_MEMORY;result->effect=UI_CAP2_EFFECT_UNKNOWN;}
 result->output_json=a->edit_pending_json;a->edit_pending_resource=c->spec->resource;a->edit_pending_call=stage->call_id;p4_try_complete(a);
}
static void p4_execute(p2_cap_t *c,uint64_t call,const char *params)
{
 p2_app_t *a=c->app;ui_cap_edit_context_t stage={0};yyjson_doc *input=NULL;yyjson_mut_doc *out=NULL;yyjson_mut_val *root;char *json=NULL,before[128],after[128];ui_status_t status;
 stage.size=sizeof(stage);stage.version=1;if(ui_capability_edit_get_stage(a->context.host,call,&stage))return;
 if(!a->transaction_before){p4_complete(c,&stage,UI_STATUS_VALIDATION_FAILED,NULL,UI_CAP2_EFFECT_NONE);return;}
 input=yyjson_read(params,strlen(params),0);out=yyjson_mut_doc_new(NULL);root=out?yyjson_mut_obj(out):NULL;if(out)yyjson_mut_doc_set_root(out,root);
 status=a->revision(a->model,c->spec->resource,before);a->staging=1;a->staging_effect=0;
 if(!status)status=!input||!root?UI_STATUS_OUT_OF_MEMORY:!p2_c_text(yyjson_doc_get_root(input))?UI_STATUS_VALIDATION_FAILED:a->run(a->model,c->spec->id,yyjson_doc_get_root(input),out,root,call);
 if(!status)status=a->revision(a->model,c->spec->resource,after);
 free(a->transaction_staged);a->transaction_staged=NULL;a->staged_bytes=0;
 if(!status)status=a->capture_business(a->model,&a->transaction_staged,&a->staged_bytes);
 {ui_status_t restored=a->restore_business(a->model,a->transaction_before,a->before_bytes,0);if(restored)status=restored;}
 a->staging=0;if(!status&&!strcmp(before,after)&&!a->staging_effect)status=UI_STATUS_CANCELLED;
 if(!status)json=yyjson_mut_write(out,0,NULL);if(!status&&!json)status=UI_STATUS_OUT_OF_MEMORY;
 p4_complete(c,&stage,status,json,UI_CAP2_EFFECT_NONE);free(json);yyjson_mut_doc_free(out);yyjson_doc_free(input);
}
static void p4_transaction(const ui_cap_edit_context_t *stage,void *data)
{
 p2_cap_t *c=data;p2_app_t *a=c->app;ui_status_t status=UI_STATUS_OK;yyjson_mut_doc *doc=NULL;yyjson_mut_val *root=NULL;char *json=NULL;ui_cap_effect_v2_t effect=UI_CAP2_EFFECT_NONE;
 if(stage->stage==UI_EDIT_BEGIN){a->transaction_id[0]=0;
  if(a->transaction_sequence==UINT64_MAX||a->transaction_before||(a->quiet&&!a->quiet(a->model,c->spec->resource)))status=UI_STATUS_LIMIT_EXCEEDED;
  else{a->transaction_committed=a->transaction_is_undo=0;strcpy_s(a->transaction_capability,256,c->spec->id);a->transaction_effect=UI_CAP2_EFFECT_NONE;status=a->capture_business(a->model,&a->transaction_before,&a->before_bytes);if(!status)snprintf(a->transaction_id,65,"txn-%llu-%llu",(unsigned long long)a->context.instance_id,(unsigned long long)++a->transaction_sequence);}
 }else if(stage->stage==UI_EDIT_UNDO){a->transaction_is_undo=1;
  if(!a->undo_business||strcmp(stage->undo_id,a->undo_receipt))status=UI_STATUS_NOT_FOUND;
  else{a->staging=1;status=a->restore_business(a->model,a->undo_business,a->undo_bytes,0);free(a->transaction_staged);a->transaction_staged=NULL;
   if(!status)status=a->capture_business(a->model,&a->transaction_staged,&a->staged_bytes);
   {ui_status_t restored=a->restore_business(a->model,a->transaction_before,a->before_bytes,0);if(restored)status=restored;}a->staging=0;}
 }else if(stage->stage==UI_EDIT_COMMIT){
  if(!a->transaction_staged)status=UI_STATUS_VALIDATION_FAILED;
  else{a->transaction_effect=UI_CAP2_EFFECT_UNKNOWN;status=a->restore_business(a->model,a->transaction_staged,a->staged_bytes,1);
   if(!status){free(a->undo_business);a->undo_business=a->transaction_before;a->undo_bytes=a->before_bytes;a->transaction_before=NULL;snprintf(a->undo_receipt,65,"undo-%llu-%llu",(unsigned long long)a->context.instance_id,(unsigned long long)a->transaction_sequence);a->transaction_committed=1;effect=a->transaction_effect=UI_CAP2_EFFECT_APPLIED;}
   if(!status&&a->project_business)status=a->project_business(a->model);}
  free(a->transaction_staged);a->transaction_staged=NULL;
 }else if(stage->stage==UI_EDIT_ROLLBACK){
  a->transaction_committed=0;a->transaction_effect=UI_CAP2_EFFECT_UNKNOWN;
  if(a->transaction_before){status=a->restore_business(a->model,a->transaction_before,a->before_bytes,0);free(a->transaction_before);a->transaction_before=NULL;}
  free(a->transaction_staged);a->transaction_staged=NULL;if(!status)a->transaction_effect=UI_CAP2_EFFECT_NONE;if(!status&&a->project_business)status=a->project_business(a->model);
 }else if(stage->stage==UI_EDIT_READBACK||stage->stage==UI_EDIT_RECONCILE){
  doc=yyjson_mut_doc_new(NULL);root=doc?yyjson_mut_obj(doc):NULL;if(doc)yyjson_mut_doc_set_root(doc,root);
  status=root?a->describe(a->model,c->spec->resource,doc,root):UI_STATUS_OUT_OF_MEMORY;
  if(!status){json=yyjson_mut_write(doc,0,NULL);if(!json)status=UI_STATUS_OUT_OF_MEMORY;}
  effect=stage->stage==UI_EDIT_RECONCILE?a->transaction_effect:UI_CAP2_EFFECT_NONE;
 }
 p4_complete(c,stage,status,json,effect);free(json);yyjson_mut_doc_free(doc);
}

/* Policy-only AI calls never run a file transaction. A fresh explicit proof
 * may only read the original preference resource after its worker is quiet. */
static void p4_policy_transaction(const ui_cap_edit_context_t *stage,void *data)
{p2_cap_t *c=data;p2_app_t *a=c->app;yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *r=d?yyjson_mut_obj(d):NULL;char *json=NULL;ui_status_t status=UI_STATUS_PERMISSION_DENIED;
 if(d)yyjson_mut_doc_set_root(d,r);if(stage->stage==UI_EDIT_RECONCILE){status=r?a->describe(a->model,c->spec->resource,d,r):UI_STATUS_OUT_OF_MEMORY;if(!status){json=yyjson_mut_write(d,0,NULL);if(!json)status=UI_STATUS_OUT_OF_MEMORY;}}
 p4_complete(c,stage,status,json,UI_CAP2_EFFECT_NONE);free(json);yyjson_mut_doc_free(d);}
