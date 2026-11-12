/* Worker-only audit before READY. Historical data never re-establishes authority. */
typedef struct bv_request_record {
 av_ticket_t ticket;
 uint64_t response_cost;
 int reserved,intent,response,usage_valid,network_complete;
} bv_request_record_t;

static int bv_hash_text(const char *text)
{
 if(!text||strlen(text)!=64)return 0;
 for(unsigned i=0;i<64;++i)
  if(!((text[i]>='0'&&text[i]<='9')||(text[i]>='a'&&text[i]<='f')||(text[i]>='A'&&text[i]<='F')))return 0;
 return 1;
}

static int bv_open_record(yyjson_val *root,av_policy_t *policy)
{
 const char *kind=yyjson_get_str(yyjson_obj_get(root,"policy"));
 const char *id=yyjson_get_str(yyjson_obj_get(root,"id"));
 yyjson_val *input=yyjson_obj_get(root,"input_nano_per_token");
 yyjson_val *output=yyjson_obj_get(root,"output_nano_per_token");
 yyjson_val *budget=yyjson_obj_get(root,"budget_nano_usd");
 if(!kind||!ai_id(id)||!yyjson_is_uint(input)||!yyjson_is_uint(output)||!yyjson_is_uint(budget)||yyjson_get_uint(budget)!=AV_BUDGET)return 0;
 policy->input_rate=yyjson_get_uint(input);policy->output_rate=yyjson_get_uint(output);
 if(policy->input_rate>1000000||policy->output_rate>1000000)return 0;
 strcpy_s(policy->id,65,id);
 if(!strcmp(kind,"RPCP-1")){
  const char *session=yyjson_get_str(yyjson_obj_get(root,"session"));
  /* The old six-field record also identifies RPCB, whose batch is separate. */
  return yyjson_obj_size(root)==6&&session&&strlen(session)==36&&policy->input_rate>=300&&policy->output_rate>=1200;
 }
 yyjson_val *scene_value=yyjson_obj_get(root,"business_scene");const char *scene=yyjson_get_str(scene_value);unsigned extra=scene_value?1u:0u;
 int scene_kind=scene?av_scene_parse(scene):0;
 if(scene_value&&(!scene||scene_kind<=0))return 0;
 policy->body_scene=scene_kind;
 if(strcmp(kind,"BR-2")||(yyjson_obj_size(root)!=11+extra&&yyjson_obj_size(root)!=13+extra))return 0;
 if(yyjson_obj_size(root)==13+extra){
  const char *profile=yyjson_get_str(yyjson_obj_get(root,"meter_profile"));
  const char *evidence=yyjson_get_str(yyjson_obj_get(root,"meter_evidence_sha256"));
  if(!bm_profile_matches(profile,evidence))return 0;
  policy->meter_required=1;strcpy_s(policy->meter_profile,65,profile);strcpy_s(policy->meter_evidence,65,evidence);
 }else if(yyjson_obj_get(root,"meter_profile")||yyjson_obj_get(root,"meter_evidence_sha256"))return 0;
 policy->business=1;
 yyjson_val *limit=yyjson_obj_get(root,"request_limit"),*ceiling=yyjson_obj_get(root,"input_ceiling");
 const char *model=yyjson_get_str(yyjson_obj_get(root,"model"));
 const char *protocol=yyjson_get_str(yyjson_obj_get(root,"protocol"));
 const char *version=yyjson_get_str(yyjson_obj_get(root,"price_version"));
 const char *evidence=yyjson_get_str(yyjson_obj_get(root,"price_evidence_sha256"));
 if(!yyjson_is_uint(limit)||yyjson_get_uint(limit)!=3||!yyjson_is_uint(ceiling)||!model||!version||!evidence||!protocol||strcmp(protocol,"deepseek-responses-v1"))return 0;
 policy->input_ceiling=yyjson_get_uint(ceiling);
 /* A genuine unverified opening may exist, but cannot have validation tasks. */
 if(!strcmp(id,"BR-2-unverified"))return !*model&&!*version&&!*evidence&&!policy->input_ceiling&&!policy->input_rate&&!policy->output_rate;
 if(policy->body_scene&&!policy->meter_required)return 0;
 if(!ai_id(model)||!ai_id(version)||!bv_hash_text(evidence)||!policy->input_rate||!policy->output_rate||!policy->input_ceiling||policy->input_ceiling>10000000)return 0;
 strcpy_s(policy->business_model,65,model);strcpy_s(policy->price_version,65,version);strcpy_s(policy->price_evidence,65,evidence);
 return 1;
}

static int bv_task_record(av_policy_t *policy,unsigned kind,const char *input,yyjson_val *plan,const char *targets)
{
 const char *model=yyjson_get_str(yyjson_obj_get(plan,"model"));
 const char *protocol=yyjson_get_str(yyjson_obj_get(plan,"request_kind"));
 const char *history=yyjson_get_str(yyjson_obj_get(plan,"history"));
 const char *projection=yyjson_get_str(yyjson_obj_get(plan,"projection"));
 const char *scene=yyjson_get_str(yyjson_obj_get(plan,"validation_scene"));
 if(policy->body_scene?(!scene||strcmp(scene,av_scene_name(policy))):yyjson_obj_get(plan,"validation_scene")!=NULL)return 0;
 if(!input||strcmp(input,policy->business?bv_task_input(policy):av_inputs[kind])||!model||strcmp(model,policy->business?policy->business_model:"deepseek-flash")||!protocol||strcmp(protocol,"deepseek-responses-v1")||!history||strcmp(history,"[]")||!projection||!targets)return 0;
 if(policy->business&&kind!=2)return 0;
 ui_cap_json_doc_t p={0},t={0};
 int ok=am_json_parse(projection,strlen(projection),&p)&&am_json_parse(targets,strlen(targets),&t);
 yyjson_val *scope=p.doc?yyjson_doc_get_root(p.doc):NULL,*array=t.doc?yyjson_doc_get_root(t.doc):NULL;
 yyjson_val *caps=yyjson_obj_get(scope,"capabilities"),*fields=yyjson_obj_get(scope,"fields");
 ok=ok&&yyjson_obj_size(scope)==2&&yyjson_is_arr(caps)&&yyjson_is_arr(fields)&&yyjson_is_arr(array);
 if(kind==2){
  const char *cap=yyjson_get_str(yyjson_arr_get(caps,0));
  const char *a=yyjson_get_str(yyjson_arr_get(fields,0)),*b=yyjson_get_str(yyjson_arr_get(fields,1));
  const char *value=policy->business?(policy->body_scene?"body":"state"):"value";
  ok=ok&&yyjson_arr_size(caps)==1&&cap&&!strcmp(cap,policy->business?(policy->body_scene?"browser.body":"browser.snapshot"):"memory.read")&&yyjson_arr_size(fields)==2&&a&&b&&((!strcmp(a,value)&&!strcmp(b,"revision"))||(!strcmp(b,value)&&!strcmp(a,"revision")))&&yyjson_arr_size(array)==1;
  yyjson_val *target=yyjson_arr_get(array,0),*target_kind=yyjson_obj_get(target,"kind");
  const char *resource=yyjson_get_str(yyjson_obj_get(target,"resource_id"));
  const char *instance=yyjson_get_str(yyjson_obj_get(target,"instance_id"));
  const char *host=yyjson_get_str(yyjson_obj_get(target,"host_epoch")),*open=yyjson_get_str(yyjson_obj_get(target,"open_epoch"));
  const char *revision=yyjson_get_str(yyjson_obj_get(target,"revision")),*access=yyjson_get_str(yyjson_obj_get(target,"scope"));
  ok=ok&&yyjson_obj_size(target)==7&&yyjson_is_uint(target_kind)&&yyjson_get_uint(target_kind)==(uint64_t)(policy->business?UI_CAP_INSTANCE_VIEW:UI_CAP_DOCUMENT)&&resource&&!strcmp(resource,policy->business?"browser.view":"memory.document")&&instance&&*instance&&strspn(instance,"0123456789")==strlen(instance)&&strspn(instance,"0")!=strlen(instance)&&host&&strlen(host)==36&&open&&strlen(open)==36&&revision&&*revision&&strlen(revision)<128&&access&&!strcmp(access,"{\"access\":\"read\"}");
  if(!policy->business)ok=ok&&revision&&!strcmp(revision,"r1");
 }else ok=ok&&!yyjson_arr_size(caps)&&!yyjson_arr_size(fields)&&!yyjson_arr_size(array);
 am_json_free(&p);am_json_free(&t);return ok;
}

static int bv_restore_guard(ui_ai_service_t *s)
{
 av_policy_t restored={0};bv_request_record_t requests[8]={0};
 unsigned opens=0,task_count=0,request_count=0,batches=0;int completed[3]={0};
 uint64_t open_sequence=0;sqlite3_stmt *q=NULL;int step=SQLITE_DONE,ok=1;
 if(!ai_statement(s,"SELECT seq,run_id,payload FROM events WHERE kind='rpc_open' ORDER BY seq",&q))return 0;
 while((step=sqlite3_step(q))==SQLITE_ROW){
  const char *run=(const char *)sqlite3_column_text(q,1),*raw=(const char *)sqlite3_column_text(q,2);
  ui_cap_json_doc_t parsed={0};
  ok=++opens==1&&run&&!*run&&raw&&am_json_parse(raw,strlen(raw),&parsed)&&bv_open_record(yyjson_doc_get_root(parsed.doc),&restored);
  open_sequence=(uint64_t)sqlite3_column_int64(q,0);am_json_free(&parsed);if(!ok)break;
 }
 sqlite3_finalize(q);q=NULL;if(!ok||step!=SQLITE_DONE)return 0;
 if(!ai_statement(s,"SELECT id,client,input,plan,targets,outcome FROM runs",&q))return 0;
 while((step=sqlite3_step(q))==SQLITE_ROW){
  const char *raw=(const char *)sqlite3_column_text(q,3);ui_cap_json_doc_t parsed={0};
  ok=raw&&am_json_parse(raw,strlen(raw),&parsed);
  yyjson_val *plan=parsed.doc?yyjson_doc_get_root(parsed.doc):NULL,*mark=yyjson_obj_get(plan,"validation_kind");
  if(ok&&opens&&!mark&&yyjson_obj_get(plan,"request_kind"))ok=0;
  if(ok&&yyjson_obj_get(plan,"validation_scene")&&(!mark||!opens))ok=0;
  if(ok&&mark){
   uint64_t kind=yyjson_get_uint(mark);
   const char *run=(const char *)sqlite3_column_text(q,0),*client=(const char *)sqlite3_column_text(q,1);
   ok=opens==1&&yyjson_is_uint(mark)&&kind>=1&&kind<=3&&run&&bv_hash_text(run)&&ai_id(client);
   if(ok)ok=!restored.run[kind-1][0]&&bv_task_record(&restored,(unsigned)kind,(const char *)sqlite3_column_text(q,2),plan,(const char *)sqlite3_column_text(q,4));
   if(ok){strcpy_s(restored.run[kind-1],65,run);strcpy_s(restored.client[kind-1],65,client);completed[kind-1]=!strcmp((const char *)sqlite3_column_text(q,5),"completed");++task_count;}
  }
  am_json_free(&parsed);if(!ok)break;
 }
 sqlite3_finalize(q);q=NULL;if(!ok||step!=SQLITE_DONE)return 0;
 if(!ai_statement(s,"SELECT seq,op_id,run_id,kind,payload FROM events WHERE kind GLOB 'rpc_*' OR kind IN('provider_request','provider_response') ORDER BY seq",&q))return 0;
 while((step=sqlite3_step(q))==SQLITE_ROW){
  const char *op=(const char *)sqlite3_column_text(q,1),*run=(const char *)sqlite3_column_text(q,2),*kind=(const char *)sqlite3_column_text(q,3),*raw=(const char *)sqlite3_column_text(q,4);
  if(!strcmp(kind,"rpc_open"))continue;
  unsigned task=0;for(unsigned i=0;i<3;++i)if(restored.run[i][0]&&!strcmp(run,restored.run[i]))task=i+1;
  if(!task&&(!strcmp(kind,"provider_request")||!strcmp(kind,"provider_response")))continue;
  ui_cap_json_doc_t parsed={0};
  ok=opens==1&&(uint64_t)sqlite3_column_int64(q,0)>open_sequence&&raw&&am_json_parse(raw,strlen(raw),&parsed);
  yyjson_val *root=parsed.doc?yyjson_doc_get_root(parsed.doc):NULL;
  if(ok&&!strcmp(kind,"rpc_batch_confirm")){
   yyjson_val *preview=yyjson_obj_get(root,"preview");
   const char *policy=yyjson_get_str(yyjson_obj_get(preview,"policy"));
   const char *hash=yyjson_get_str(yyjson_obj_get(root,"batch_hash"));
   ok=!restored.business&&!*run&&++batches==1&&policy&&!strcmp(policy,"RPCB-1")&&bv_hash_text(hash);
   am_json_free(&parsed);if(!ok)break;continue;
  }
  if(ok&&!strcmp(kind,"rpc_batch_tool_proof")){
   const char *call=yyjson_get_str(yyjson_obj_get(root,"tool_call_id"));
   const char *hash=yyjson_get_str(yyjson_obj_get(root,"result_hash"));
   const char *proof=yyjson_get_str(yyjson_obj_get(root,"kind"));sqlite3_stmt *tool=NULL;
   ok=!restored.business&&batches==1&&task==2&&call&&bv_hash_text(hash)&&proof&&(!strcmp(proof,"read")||!strcmp(proof,"discovery"));
   if(ok)ok=ai_statement(s,"SELECT result,status FROM tools WHERE run_id=? AND tool_id=?",&tool);
   if(ok){
    ai_bind(tool,1,run);ai_bind(tool,2,call);ok=sqlite3_step(tool)==SQLITE_ROW;
    const char *result=ok?(const char *)sqlite3_column_text(tool,0):NULL;char actual[65];
    ok=ok&&result&&sqlite3_column_int(tool,1)==UI_STATUS_OK&&!ui_cap_json_fingerprint(result,strlen(result),actual)&&!strcmp(actual,hash);
   }
   sqlite3_finalize(tool);am_json_free(&parsed);if(!ok)break;continue;
  }
  ok=ok&&task!=0;
  const char *id=yyjson_get_str(yyjson_obj_get(root,"request_id"));
  bv_request_record_t *request=NULL;
  for(unsigned i=0;i<request_count;++i)if(id&&!strcmp(id,requests[i].ticket.request))request=&requests[i];
  if(ok&&!strcmp(kind,"provider_request")){
   uint64_t ordinal=yyjson_get_uint(yyjson_obj_get(root,"ordinal"));
   const char *hash=yyjson_get_str(yyjson_obj_get(root,"body_hash"));
   char identity[160],expected[65];snprintf(identity,sizeof(identity),"%s:model:%llu",run,(unsigned long long)ordinal);ui_cap_hash_bytes(identity,strlen(identity),expected);
   unsigned previous=0;for(unsigned i=0;i<request_count;++i)if(requests[i].ticket.kind==task)++previous;
   ok=!request&&request_count<8&&yyjson_is_uint(yyjson_obj_get(root,"ordinal"))&&ordinal==previous+1&&bv_hash_text(id)&&!strcmp(id,expected)&&bv_hash_text(hash);
   if(ok){request=&requests[request_count++];strcpy_s(request->ticket.request,65,id);strcpy_s(request->ticket.run,65,run);strcpy_s(request->ticket.hash,65,hash);request->ticket.ordinal=(unsigned)ordinal;request->ticket.kind=task;++restored.manifests;}
  }else if(ok&&!strcmp(kind,"provider_response")){
   request=NULL;
   for(unsigned i=0;i<request_count;++i){char expected[160];snprintf(expected,sizeof(expected),"%s:model:%u:response",run,requests[i].ticket.ordinal);if(!strcmp(op,expected)&&!strcmp(run,requests[i].ticket.run))request=&requests[i];}
   ok=request&&request->network_complete&&!request->response;
   if(ok){
    yyjson_val *usage=yyjson_obj_get(root,"usage"),*input=yyjson_obj_get(usage,"input_tokens"),*output=yyjson_obj_get(usage,"output_tokens");
    request->response=1;request->usage_valid=yyjson_is_uint(input)&&yyjson_is_uint(output)&&yyjson_get_uint(input)<=(restored.meter_required?request->ticket.measured_input:(restored.business?restored.input_ceiling:10000000))&&yyjson_get_uint(output)<=4096;
    if(request->usage_valid)request->response_cost=yyjson_get_uint(input)*restored.input_rate+yyjson_get_uint(output)*restored.output_rate;
   }
  }else if(ok){
   ok=request&&!strcmp(run,request->ticket.run);
   if(ok&&!strcmp(kind,"rpc_reserved")){
    const char *hash=yyjson_get_str(yyjson_obj_get(root,"body_hash")),*recorded_run=yyjson_get_str(yyjson_obj_get(root,"run_id"));
    yyjson_val *estimate=yyjson_obj_get(root,"estimated_nano_usd");
    ok=!request->reserved&&restored.reserved<(restored.business?3u:5u)&&yyjson_is_uint(yyjson_obj_get(root,"task"))&&yyjson_get_uint(yyjson_obj_get(root,"task"))==task&&yyjson_is_uint(yyjson_obj_get(root,"ordinal"))&&yyjson_get_uint(yyjson_obj_get(root,"ordinal"))==request->ticket.ordinal&&hash&&!strcmp(hash,request->ticket.hash)&&recorded_run&&!strcmp(recorded_run,run)&&yyjson_is_uint(estimate);
    ok=ok&&restored.known<AV_BUDGET&&restored.estimated<=AV_BUDGET-restored.known&&yyjson_get_uint(estimate)<=AV_BUDGET-restored.known-restored.estimated;
    if(ok&&restored.business)ok=yyjson_get_uint(estimate)==restored.input_ceiling*restored.input_rate+4096*restored.output_rate;
    if(ok&&restored.meter_required)ok=bm_receipt(&restored,yyjson_obj_get(root,"meter"),request->ticket.hash,&request->ticket.measured_input);
    else if(ok&&yyjson_obj_get(root,"meter"))ok=0;
    if(ok){request->reserved=1;request->ticket.estimate=yyjson_get_uint(estimate);restored.estimated+=request->ticket.estimate;++restored.reserved;++restored.used[task-1];ok=restored.used[task-1]<=av_limits[task];}
   }else if(ok&&!strcmp(kind,"rpc_send_intent")){
    yyjson_val *simulated=yyjson_obj_get(root,"simulated");ok=request->reserved&&!request->intent&&!request->ticket.ended&&yyjson_is_bool(simulated);
    if(ok){request->intent=1;request->ticket.simulated=yyjson_get_bool(simulated);}
   }else if(ok&&!strcmp(kind,"rpc_send_entered")){
    yyjson_val *simulated=yyjson_obj_get(root,"simulated");ok=request->intent&&!request->ticket.entered&&!request->ticket.ended&&yyjson_is_bool(simulated)&&yyjson_get_bool(simulated)==request->ticket.simulated;
    if(ok){request->ticket.entered=1;if(request->ticket.simulated)++restored.mock_sends;else ++restored.sends;}
   }else if(ok&&!strcmp(kind,"rpc_network_end")){
    yyjson_val *complete=yyjson_obj_get(root,"complete"),*unknown=yyjson_obj_get(root,"unknown");
    ok=request->reserved&&!request->ticket.ended&&yyjson_is_bool(complete)&&yyjson_is_bool(unknown)&&yyjson_get_bool(unknown)==(request->ticket.entered&&!yyjson_get_bool(complete))&&(!yyjson_get_bool(complete)||request->ticket.entered);
    if(ok){request->ticket.ended=1;request->network_complete=yyjson_get_bool(complete);if(request->network_complete){if(request->ticket.simulated)++restored.mock_complete;else ++restored.complete;}else if(yyjson_get_bool(unknown)){restored.fee_unknown=1;if(request->ticket.simulated)++restored.mock_unknown;else ++restored.unknown;}}
   }else if(ok&&!strcmp(kind,"rpc_cost")){
    const char *fee=yyjson_get_str(yyjson_obj_get(root,"fee_status"));yyjson_val *amount=yyjson_obj_get(root,"known_nano_usd");
    ok=request->response&&!request->ticket.priced&&fee;
    if(ok&&!strcmp(fee,"known")){
     uint64_t cost=yyjson_get_uint(amount);
     ok=request->usage_valid&&yyjson_is_uint(amount)&&cost==request->response_cost&&request->ticket.estimate<=restored.estimated&&cost<=UINT64_MAX-restored.known;
     /* Legacy byte estimates were not billable bounds. Keep their settlement facts. */
     if(ok&&restored.business)ok=cost<=request->ticket.estimate&&restored.known<=AV_BUDGET&&cost<=AV_BUDGET-restored.known;
     if(ok){restored.known+=cost;restored.estimated-=request->ticket.estimate;}
    }else if(ok&&!strcmp(fee,"unknown")){ok=!request->usage_valid&&yyjson_is_null(amount);restored.fee_unknown=1;}
    else ok=0;
    if(ok)request->ticket.priced=1;
   }else ok=0;
  }
  am_json_free(&parsed);if(!ok)break;
 }
 sqlite3_finalize(q);if(!ok||step!=SQLITE_DONE)return 0;
 if(batches&&(restored.run[0][0]||restored.reserved>4))return 0;
 for(unsigned task=1;task<=3;++task)if(completed[task-1]){
  bv_request_record_t *last=NULL;
  for(unsigned i=0;i<request_count;++i)if(requests[i].ticket.kind==task)last=&requests[i];
  if(!last||!last->response||!last->usage_valid||!last->ticket.priced)return 0;
 }
 if(!restored.business)return 1; /* Preserve explicit RPCP/RPCB reopening behavior. */
 if(!restored.business_model[0]&&task_count)return 0;
 unsigned reserved_index=0;
 for(unsigned i=0;i<request_count;++i)if(requests[i].reserved){
  restored.tickets[reserved_index++]=requests[i].ticket;
  if(!requests[i].ticket.ended&&requests[i].intent){restored.fee_unknown=1;if(requests[i].ticket.simulated)++restored.mock_unknown;else ++restored.unknown;}
 }
 restored.prior=1;
 EnterCriticalSection(&s->lock);
 if(s->validation&&!s->validation->business){LeaveCriticalSection(&s->lock);return 0;}
 if(!s->validation){
  if(sizeof(av_policy_t)>UI_AI_MAX_BYTES-s->bytes){LeaveCriticalSection(&s->lock);return 0;}
  s->validation=calloc(1,sizeof(av_policy_t));if(!s->validation){LeaveCriticalSection(&s->lock);return 0;}
  s->bytes+=sizeof(av_policy_t);
 }
 char original_error[96];strcpy_s(original_error,96,s->validation->error);
 *s->validation=restored;
 if(original_error[0])strcpy_s(s->validation->error,96,original_error);
 av_stop_locked(s,"validation_restart_review_required");LeaveCriticalSection(&s->lock);return 1;
}
