/* Public restore entry, original settlement, isolated SSE and trusted fixtures. */
#define AI_MODEL_MAIN fee_original_model_main
#include "ai_model.c"
#include <bcrypt.h>

static char *fee_wire(const char *wire,uint64_t input,uint64_t output,int usage)
{
 const char *event=strstr(wire,"event: response.completed");
 const char *data=event?strstr(event,"data: "):NULL;
 if(!data)return NULL;
 data+=6;const char *end=strchr(data,'\n');if(!end)return NULL;
 if(end>data&&end[-1]=='\r')--end;
 yyjson_doc *parsed=yyjson_read(data,(size_t)(end-data),0);
 yyjson_mut_doc *doc=parsed?yyjson_doc_mut_copy(parsed,NULL):NULL;
 yyjson_mut_val *response=doc?yyjson_mut_obj_get(yyjson_mut_doc_get_root(doc),"response"):NULL;
 if(response){
  yyjson_mut_obj_remove_str(response,"usage");
  if(usage){
   yyjson_mut_val *value=yyjson_mut_obj(doc);
   yyjson_mut_obj_add_uint(doc,value,"input_tokens",input);
   yyjson_mut_obj_add_uint(doc,value,"output_tokens",output);
   yyjson_mut_obj_add_val(doc,response,"usage",value);
  }
 }
 char *json=response?yyjson_mut_write(doc,0,NULL):NULL,*result=NULL;
 if(json){
  size_t prefix=(size_t)(data-wire),size=prefix+strlen(json)+strlen(end)+1;
  result=malloc(size);
  if(result){memcpy(result,wire,prefix);strcpy_s(result+prefix,size-prefix,json);strcat_s(result,size,end);}
 }
 free(json);yyjson_mut_doc_free(doc);yyjson_doc_free(parsed);return result;
}

static int fee_responses(uint64_t input,uint64_t output,int usage)
{
 const char *wires[]={rp_discover,rp_read,rp_text};
 yyjson_mut_doc *doc=yyjson_mut_doc_new(NULL);
 yyjson_mut_val *rows=doc?yyjson_mut_arr(doc):NULL;
 int ok=rows!=NULL;
 if(rows)yyjson_mut_doc_set_root(doc,rows);
 for(unsigned i=0;ok&&i<3;++i){
  char *wire=fee_wire(wires[i],i?100:input,i?10:output,i?1:usage);
  ok=wire&&yyjson_mut_arr_add_strcpy(doc,rows,wire);free(wire);
 }
 char *encoded=ok?yyjson_mut_write(doc,0,NULL):NULL;
 ok=encoded&&ui_ai_model_fixture(s,1,encoded)==UI_STATUS_OK;
 free(encoded);yyjson_mut_doc_free(doc);
 if(ok)ok=ui_ai_model_fixture(s,5,"fixture-key-no-leak")==UI_STATUS_OK;
 return ok;
}

static ui_ai_service_phase_t fee_idle(void)
{
 ui_ai_service_state_t info={0};info.size=sizeof(info);info.version=1;
 DWORD begin=GetTickCount();
 do{
  pump();CHECK(ui_ai_service_get_state(s,&info)==UI_STATUS_OK);
  if(info.phase!=UI_AI_OPENING&&!info.worker_alive)break;
 }while(GetTickCount()-begin<5000);
 CHECK(!info.worker_alive&&s->methods==0&&s->callbacks==0);
 for(unsigned i=0;i<AI_RUNS;++i)CHECK(!s->runs[i].refs);
 return info.phase;
}

static int fee_same_policy(const char *before,const char *after)
{
 const char *keys[]={"known_nano_usd","estimated_nano_usd","reserved","manifest","send_entries","complete","unknown","simulated_send_entries","simulated_complete","simulated_unknown"};
 yyjson_doc *a=yyjson_read(before,strlen(before),0),*b=yyjson_read(after,strlen(after),0);
 yyjson_val *left=a?yyjson_doc_get_root(a):NULL,*right=b?yyjson_doc_get_root(b):NULL;
 int ok=left&&right;
 for(unsigned i=0;ok&&i<sizeof(keys)/sizeof(keys[0]);++i){
  yyjson_val *x=yyjson_obj_get(left,keys[i]),*y=yyjson_obj_get(right,keys[i]);
  ok=yyjson_is_uint(x)&&yyjson_is_uint(y)&&yyjson_get_uint(x)==yyjson_get_uint(y);
 }
 yyjson_val *x=yyjson_obj_get(left,"fee_unknown"),*y=yyjson_obj_get(right,"fee_unknown");
 ok=ok&&yyjson_is_bool(x)&&yyjson_is_bool(y)&&yyjson_get_bool(x)==yyjson_get_bool(y);
 for(unsigned i=0;ok&&i<3;++i){
  x=yyjson_arr_get(yyjson_obj_get(left,"task_used"),i);y=yyjson_arr_get(yyjson_obj_get(right,"task_used"),i);
  ok=yyjson_is_uint(x)&&yyjson_is_uint(y)&&yyjson_get_uint(x)==yyjson_get_uint(y);
 }
 yyjson_doc_free(a);yyjson_doc_free(b);return ok;
}

static char *fee_ledger(void)
{
 char sidecar[4200],uri[4300];wchar_t wide[4300];
 const char *suffixes[]={"-wal","-shm"};
 for(unsigned i=0;i<2;++i){
  snprintf(sidecar,sizeof(sidecar),"%s%s",dbpath,suffixes[i]);
  CHECK(MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,sidecar,-1,wide,4300)>0);
  CHECK(GetFileAttributesW(wide)==INVALID_FILE_ATTRIBUTES);
 }
 snprintf(uri,sizeof(uri),"file:%s?immutable=1",dbpath);
 sqlite3 *db=NULL;CHECK(sqlite3_open_v2(uri,&db,SQLITE_OPEN_READONLY|SQLITE_OPEN_URI,NULL)==SQLITE_OK);
 yyjson_mut_doc *doc=yyjson_mut_doc_new(NULL);yyjson_mut_val *root=doc?yyjson_mut_arr(doc):NULL;
 sqlite3_stmt *query=NULL;int ok=db&&root&&sqlite3_prepare_v2(db,"SELECT op_id,run_id,kind,payload,terminal FROM events ORDER BY seq",-1,&query,NULL)==SQLITE_OK;
 if(root)yyjson_mut_doc_set_root(doc,root);
 int step=SQLITE_DONE;
 while(ok&&(step=sqlite3_step(query))==SQLITE_ROW){
  yyjson_mut_val *row=yyjson_mut_arr(doc);ok=row&&yyjson_mut_arr_add_val(root,row);
  for(int i=0;ok&&i<4;++i)ok=yyjson_mut_arr_add_strcpy(doc,row,(const char *)sqlite3_column_text(query,i));
  if(ok)ok=yyjson_mut_arr_add_int(doc,row,sqlite3_column_int(query,4));
 }
 CHECK(ok&&step==SQLITE_DONE);sqlite3_finalize(query);sqlite3_close(db);
 char *result=ok?yyjson_mut_write(doc,0,NULL):NULL;yyjson_mut_doc_free(doc);return result;
}

static int fee_corrupt_copy(int late_reserve)
{
 wchar_t original[4096],copy[4096];char original_path[4096];
 strcpy_s(original_path,sizeof(original_path),dbpath);
 if(!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,dbpath,-1,original,4096))return 0;
 if(_snwprintf_s(copy,4096,_TRUNCATE,L"%s.invalid-copy.db",original)<0||!CopyFileW(original,copy,TRUE))return 0;
 if(!WideCharToMultiByte(CP_UTF8,0,copy,-1,dbpath,sizeof(dbpath),NULL,NULL))return 0;
 sqlite3 *db=NULL;sqlite3_stmt *query=NULL;int ok=sqlite3_open_v2(dbpath,&db,SQLITE_OPEN_READWRITE,NULL)==SQLITE_OK;
 if(ok&&!late_reserve)ok=sqlite3_exec(db,"UPDATE events SET payload=replace(payload,'\"known_nano_usd\":','\"known_nano_usd\":1') WHERE kind='rpc_cost'",NULL,NULL,NULL)==SQLITE_OK;
 if(ok&&late_reserve){
  char id[65]={0},hash[65]={0},identity[160],request[65];unsigned char bytes[32];
  ok=sqlite3_prepare_v2(db,"SELECT run_id,payload FROM events WHERE kind='provider_request' ORDER BY seq LIMIT 1",-1,&query,NULL)==SQLITE_OK&&sqlite3_step(query)==SQLITE_ROW;
  yyjson_doc *doc=ok?yyjson_read((const char *)sqlite3_column_text(query,1),(size_t)sqlite3_column_bytes(query,1),0):NULL;
  if(doc){strcpy_s(id,sizeof(id),(const char *)sqlite3_column_text(query,0));strcpy_s(hash,sizeof(hash),yyjson_get_str(yyjson_obj_get(yyjson_doc_get_root(doc),"body_hash")));}
  else ok=0;
  yyjson_doc_free(doc);sqlite3_finalize(query);query=NULL;
  snprintf(identity,sizeof(identity),"%s:model:2",id);
  if(ok)ok=BCryptHash(BCRYPT_SHA256_ALG_HANDLE,NULL,0,(PUCHAR)identity,(ULONG)strlen(identity),bytes,sizeof(bytes))==0;
  if(ok)for(unsigned i=0;i<32;++i)snprintf(request+i*2,3,"%02x",bytes[i]);
  const char *kinds[]={"provider_request","rpc_reserved"};
  for(unsigned i=0;ok&&i<2;++i){
   char payload[700],op[160];
   if(!i){snprintf(op,sizeof(op),"%s:model:2:request",id);snprintf(payload,sizeof(payload),"{\"request_id\":\"%s\",\"body_hash\":\"%s\",\"ordinal\":2}",request,hash);}
   else{snprintf(op,sizeof(op),"%s:rpc:2:reserved",id);snprintf(payload,sizeof(payload),"{\"request_id\":\"%s\",\"run_id\":\"%s\",\"body_hash\":\"%s\",\"ordinal\":2,\"task\":2,\"estimated_nano_usd\":1}",request,id,hash);}
   ok=sqlite3_prepare_v2(db,"INSERT INTO events(op_id,run_id,kind,payload,terminal) VALUES(?,?,?,?,0)",-1,&query,NULL)==SQLITE_OK;
   if(ok){sqlite3_bind_text(query,1,op,-1,SQLITE_TRANSIENT);sqlite3_bind_text(query,2,id,-1,SQLITE_TRANSIENT);sqlite3_bind_text(query,3,kinds[i],-1,SQLITE_STATIC);sqlite3_bind_text(query,4,payload,-1,SQLITE_TRANSIENT);ok=sqlite3_step(query)==SQLITE_DONE;}
   sqlite3_finalize(query);query=NULL;
  }
 }
 sqlite3_close(db);
 printf("FEE-INVALID original=%s copy=%s late_reserve=%d\n",original_path,dbpath,late_reserve);return ok;
}

static void fee_restore(int batch,int invalid,const char *expected,const char *old_run)
{
 char *ledger_before=fee_ledger();CHECK(ledger_before!=NULL);
 ui_ai_service_config_t config={0};config.size=sizeof(config);config.version=1;config.workspace=w;config.storage_path_utf8=dbpath;config.on_event=model_event;
 CHECK(ui_ai_service_create(&config,&s)==UI_STATUS_OK);CHECK(ui_ai_service_open(s)==UI_STATUS_OK);
 ui_ai_service_phase_t phase=fee_idle();CHECK(phase==(invalid?UI_AI_UNAVAILABLE:UI_AI_READY));
 printf("FEE-OPEN database=%s invalid=%d phase=%d\n",dbpath,invalid,phase);
 if(phase==UI_AI_READY&&!invalid){
  if(batch){
   ui_cap_target_t target={0};target.size=sizeof(target);target.version=1;CHECK(ui_capability_bind(apps[0].host,"memory.document",&target)==UI_STATUS_OK);
   char profile[4096];snprintf(profile,sizeof(profile),"%s.validation-profile.json",dbpath);
   ui_ai_validation_batch_desc_t description={sizeof(description),1,profile,&target};CHECK(ui_ai_validation_batch_open(s,&description)==UI_STATUS_OK);
   CHECK(ui_ai_model_fixture(s,20,"1")==UI_STATUS_OK);wcscpy_s(s->model_config->fixture_credential,513,L"fixture-key-no-leak");
   CHECK(fee_responses(100,10,1));
   CHECK(bc_wait("\"export_done\":true"));
  }else{
   char profile[4096];snprintf(profile,sizeof(profile),"%s.model.json",dbpath);CHECK(ui_ai_model_open(s,profile)==UI_STATUS_OK);CHECK(fee_responses(100,10,1));CHECK(ready_model());
   ui_ai_validation_desc_t description={sizeof(description),1,"fee-restored",1000,2000};CHECK(ui_ai_validation_enable(s,&description)==UI_STATUS_OK);CHECK(vc_state("validation_restart_review_required"));
  }
  char *after=NULL;CHECK(ui_ai_validation_get_state(s,&after)==UI_STATUS_OK);
  CHECK(after&&fee_same_policy(expected,after));CHECK(after&&strstr(after,"\"stopped\":true")&&strstr(after,"\"ready\":false")&&strstr(after,"\"prior_session\":true"));
  printf("FEE-RESTORED %s\n",after?after:"<none>");ui_ai_release(after);
  CHECK(s->model_config->requests==0);
  if(batch)CHECK(ui_ai_model_fixture(s,30,"old-authorization")==UI_STATUS_PERMISSION_DENIED);
  else CHECK(vc_submit(2,"fee-new-task")!=UI_STATUS_OK);
  CHECK(ui_ai_model_fixture(s,2,old_run)!=UI_STATUS_OK);
 }else if(phase==UI_AI_UNAVAILABLE){
  ui_ai_service_state_t info={0};info.size=sizeof(info);info.version=1;CHECK(ui_ai_service_get_state(s,&info)==UI_STATUS_OK);CHECK(info.error[0]&&s->store_failed);
  ui_ai_task_request_t request={0};char id[65];request.size=sizeof(request);request.version=1;request.client_request_id="fee-invalid-new-task";request.input_utf8="must remain stopped";request.stub_plan_json="{\"steps\":[]}";
  CHECK(ui_ai_submit(s,&request,id)!=UI_STATUS_OK);
 }
 (void)fee_idle();CHECK(s->methods==0&&s->callbacks==0);close_service();
 char *ledger_after=fee_ledger();CHECK(ledger_before&&ledger_after&&!strcmp(ledger_before,ledger_after));free(ledger_before);free(ledger_after);
}

int wmain(int argc,wchar_t **argv)
{
 if(argc!=4)return 2;
 const wchar_t *mode=argv[3];int batch=!wcsncmp(mode,L"rpcb-",5),unknown=wcsstr(mode,L"unknown")!=NULL;
 int late=wcsstr(mode,L"late-reserve")!=NULL,invalid=late||wcsstr(mode,L"cost-conflict")!=NULL;
 int edge=wcsstr(mode,L"edge")!=NULL,over=late||wcsstr(mode,L"over")!=NULL;
 uint64_t input=wcsstr(mode,L"above")?30000:edge?100000:over?(batch?333334:100001):100;
 uint64_t output=edge||over?0:10;
 CHECK(ui_framework_initialize()==UI_STATUS_OK);cr_package=argv[1];cr_directory=argv[2];setup(argv[1],argv[2]);
 if(failures)goto cleanup;
 model_setup();
 if(batch)bc_setup(L"bc-success");else vc_prepare(1000,2000);
 CHECK(s->model_config&&!strcmp(s->model_config->model,"deepseek-flash"));CHECK(fee_responses(input,output,!unknown));
 if(failures)goto cleanup;
 if(batch){bc_authorize();bc_closed();}
 else{CHECK(vc_submit(2,"fee-seed-task")==UI_STATUS_OK);authorize();CHECK(real_wait("\"terminal\":true"));}
 (void)fee_idle();char *expected=NULL;CHECK(ui_ai_validation_get_state(s,&expected)==UI_STATUS_OK);CHECK(expected!=NULL);
 char old_run[65]={0};
 if(batch){char *view=NULL;CHECK(ui_ai_validation_batch_get_state(s,&view)==UI_STATUS_OK);if(view)str(view,"T2_run",old_run,sizeof(old_run));ui_ai_release(view);}
 else strcpy_s(old_run,sizeof(old_run),run);
 CHECK(old_run[0]);
 yyjson_doc *policy=expected?yyjson_read(expected,strlen(expected),0):NULL;yyjson_val *root=policy?yyjson_doc_get_root(policy):NULL;
 uint64_t known=yyjson_get_uint(yyjson_obj_get(root,"known_nano_usd"));
 if(edge)CHECK(known==100000000);else if(over)CHECK(known>100000000);else if(!unknown)CHECK(known>0&&known<100000000);
 if(unknown)CHECK(yyjson_is_true(yyjson_obj_get(root,"fee_unknown"))&&known==0);
 yyjson_doc_free(policy);printf("FEE-SEED case=%ls database=%s policy=%s\n",mode,dbpath,expected?expected:"<none>");close_service();
 char *seed_ledger=fee_ledger();CHECK(seed_ledger!=NULL);
 yyjson_doc *events=seed_ledger?yyjson_read(seed_ledger,strlen(seed_ledger),0):NULL;
 yyjson_val *rows=events?yyjson_doc_get_root(events):NULL,*row;size_t at,count;
 uint64_t first_estimate=0,first_cost=0;int found_estimate=0,found_cost=0;
 yyjson_arr_foreach(rows,at,count,row){
  const char *kind=yyjson_get_str(yyjson_arr_get(row,2)),*raw=yyjson_get_str(yyjson_arr_get(row,3));
  yyjson_doc *payload=raw?yyjson_read(raw,strlen(raw),0):NULL;yyjson_val *object=payload?yyjson_doc_get_root(payload):NULL;
  if(kind&&!strcmp(kind,"rpc_reserved")&&!found_estimate){first_estimate=yyjson_get_uint(yyjson_obj_get(object,"estimated_nano_usd"));found_estimate=1;}
  if(kind&&!strcmp(kind,"rpc_cost")&&!found_cost){first_cost=yyjson_get_uint(yyjson_obj_get(object,"known_nano_usd"));found_cost=1;}
  yyjson_doc_free(payload);
 }
 CHECK(found_estimate&&found_cost);
 if(wcsstr(mode,L"above")||edge||over)CHECK(first_cost>first_estimate);
 printf("FEE-AMOUNTS estimate=%llu first_known=%llu cumulative_known=%llu\n",(unsigned long long)first_estimate,(unsigned long long)first_cost,(unsigned long long)known);
 yyjson_doc_free(events);free(seed_ledger);
 if(invalid)CHECK(fee_corrupt_copy(late));
 if(expected)fee_restore(batch,invalid,expected,old_run);ui_ai_release(expected);
cleanup:
 if(s)close_service();
 CHECK(!secret_leaked);CHECK(ui_workspace_close_all(w)==UI_STATUS_OK);DWORD begin=GetTickCount();
 while(ui_workspace_count(w)&&GetTickCount()-begin<5000){ui_workspace_poll(w);(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}
 CHECK(!ui_workspace_count(w));CHECK(ui_workspace_destroy(w)==UI_STATUS_OK);
 printf("fee restore %ls checks=%u failures=%u\n",mode,checks,failures);return failures?1:0;
}
