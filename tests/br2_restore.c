/* Restore via the real host/Service. All transport and credentials are isolated. */
#include <limits.h>
#define BR2_GUARD_MAIN br2_seed_main
#include "br2_guard.c"

static int restore_sql(sqlite3 *db,const char *sql)
{
 char *error=NULL;
 int status=sqlite3_exec(db,sql,NULL,NULL,&error);
 if(status!=SQLITE_OK)fprintf(stderr,"restore SQL error %s\n",error?error:"");
 sqlite3_free(error);
 return status==SQLITE_OK;
}

static unsigned restore_count(sqlite3 *db,const char *kind)
{
 sqlite3_stmt *q=NULL;
 unsigned result=UINT_MAX;
 if(sqlite3_prepare_v2(db,"SELECT count(*) FROM events WHERE kind=?",-1,&q,NULL)==SQLITE_OK){
  sqlite3_bind_text(q,1,kind,-1,SQLITE_STATIC);
  if(sqlite3_step(q)==SQLITE_ROW)result=(unsigned)sqlite3_column_int(q,0);
 }
 sqlite3_finalize(q);
 return result;
}

static int restore_set_policy(sqlite3 *db,const char *key,const char *value)
{
 sqlite3_stmt *q=NULL;
 if(sqlite3_prepare_v2(db,"SELECT payload FROM events WHERE kind='rpc_open'",-1,&q,NULL)!=SQLITE_OK)return 0;
 int ok=sqlite3_step(q)==SQLITE_ROW;
 const char *raw=ok?(const char *)sqlite3_column_text(q,0):NULL;
 yyjson_doc *parsed=raw?yyjson_read(raw,strlen(raw),0):NULL;
 yyjson_mut_doc *changed=parsed?yyjson_doc_mut_copy(parsed,NULL):NULL;
 if(changed){
  yyjson_mut_val *root=yyjson_mut_doc_get_root(changed);
  yyjson_mut_obj_remove_str(root,key);
  if(!strcmp(key,"request_limit")||!strcmp(key,"input_nano_per_token"))yyjson_mut_obj_add_uint(changed,root,key,strtoul(value,NULL,10));
  else yyjson_mut_obj_add_strcpy(changed,root,key,value);
 }
 sqlite3_finalize(q);q=NULL;
 char *encoded=changed?yyjson_mut_write(changed,0,NULL):NULL;
 ok=encoded&&sqlite3_prepare_v2(db,"UPDATE events SET payload=? WHERE kind='rpc_open'",-1,&q,NULL)==SQLITE_OK;
 if(ok){sqlite3_bind_text(q,1,encoded,-1,SQLITE_TRANSIENT);ok=sqlite3_step(q)==SQLITE_DONE;}
 sqlite3_finalize(q);free(encoded);yyjson_mut_doc_free(changed);yyjson_doc_free(parsed);
 return ok;
}

static int restore_ordinary(const wchar_t *directory)
{
 CHECK(CreateDirectoryW(directory,NULL));wchar_t database[4096];
 _snwprintf_s(database,4096,_TRUNCATE,L"%s/ordinary.db",directory);
 CHECK(SetEnvironmentVariableW(L"UI_AI_DB_PATH",database));
 CHECK(SetEnvironmentVariableW(L"UI_AI_BR2_POLICY",NULL));
 CHECK(ui_framework_initialize()==UI_STATUS_OK);
 for(unsigned attempt=0;attempt<2;++attempt){
  host_window_t host={0};CHECK(create_host(&host,GetModuleHandleW(NULL))!=NULL);if(!host.ai)return 0;
  ShowWindow(host.hwnd,SW_HIDE);DWORD begin=GetTickCount();ui_ai_service_state_t state={0};state.size=sizeof(state);state.version=1;
  do{pump(&host);CHECK(ui_ai_service_get_state(host.ai,&state)==UI_STATUS_OK);}while(state.phase==UI_AI_OPENING&&GetTickCount()-begin<5000);
  CHECK(state.phase==UI_AI_READY);char *guard=NULL;CHECK(ui_ai_validation_get_state(host.ai,&guard)==UI_STATUS_NOT_FOUND);ui_ai_release(guard);
  if(!attempt){
   ui_ai_task_request_t q={0};char run[65];q.size=sizeof(q);q.version=1;q.client_request_id="ordinary-record";q.input_utf8="普通本地记录";q.stub_plan_json="{\"steps\":[]}";
   CHECK(ui_ai_submit(host.ai,&q,run)==UI_STATUS_OK);int done=0;begin=GetTickCount();
   do{char *task_snapshot=NULL;pump(&host);if(!ui_ai_get_run(host.ai,run,&task_snapshot))done=strstr(task_snapshot,"\"terminal\":true")!=NULL;ui_ai_release(task_snapshot);}while(!done&&GetTickCount()-begin<5000);
   CHECK(done);
  }
  CHECK(host.ai->model_config->requests==0&&host.ai->methods==0&&host.ai->callbacks==0);CHECK(close_host(&host));
 }
 SetEnvironmentVariableW(L"UI_AI_DB_PATH",NULL);return failures==0;
}

int wmain(int argc,wchar_t **argv)
{
 if(argc!=4)return 2;
 const wchar_t *mode=argv[3];GUID attempt;wchar_t unique[40],directory[4096];
 CHECK(CoCreateGuid(&attempt)==S_OK);CHECK(StringFromGUID2(&attempt,unique,40)>0);
 CHECK(CreateDirectoryW(argv[2],NULL)||GetLastError()==ERROR_ALREADY_EXISTS);
 _snwprintf_s(directory,4096,_TRUNCATE,L"%s/attempt-%s",argv[2],unique);
 if(!wcscmp(mode,L"ordinary"))return restore_ordinary(directory)?0:1;
 wchar_t *seed_mode=L"read";
 if(!wcscmp(mode,L"intact-unknown"))seed_mode=L"unknown";
 if(!wcscmp(mode,L"intact-remote-unknown"))seed_mode=L"cancel";
 if(!wcscmp(mode,L"unknown-conflict"))seed_mode=L"cancel";
 if(!wcscmp(mode,L"intact-unverified"))seed_mode=L"host-unverified";
 if(!wcscmp(mode,L"intact-storage-prefix"))seed_mode=L"storage";
 wchar_t *seed_args[]={argv[0],argv[1],directory,seed_mode};
 if(br2_seed_main(4,seed_args))return 1;
 wchar_t pattern[4096],database[4096];WIN32_FIND_DATAW found;
 _snwprintf_s(pattern,4096,_TRUNCATE,L"%s/br2-*.db",directory);
 HANDLE search=FindFirstFileW(pattern,&found);
 CHECK(search!=INVALID_HANDLE_VALUE);
 if(search==INVALID_HANDLE_VALUE)return 1;
 _snwprintf_s(database,4096,_TRUNCATE,L"%s/%s",directory,found.cFileName);
 CHECK(!FindNextFileW(search,&found));FindClose(search);
 char path[4096];CHECK(WideCharToMultiByte(CP_UTF8,0,database,-1,path,sizeof(path),NULL,NULL)>0);
 sqlite3 *db=NULL;CHECK(sqlite3_open_v2(path,&db,SQLITE_OPEN_READWRITE,NULL)==SQLITE_OK);
 if(!db)return 1;
 if(!wcscmp(mode,L"missing-open"))CHECK(restore_sql(db,"DELETE FROM events WHERE kind='rpc_open'"));
 else if(!wcscmp(mode,L"misclassified"))CHECK(restore_set_policy(db,"policy","RPCP-1"));
 else if(!wcscmp(mode,L"orphan-reserve"))CHECK(restore_sql(db,"DELETE FROM events WHERE kind='provider_request'"));
 else if(!wcscmp(mode,L"orphan-intent"))CHECK(restore_sql(db,"DELETE FROM events WHERE kind='rpc_reserved'"));
 else if(!wcscmp(mode,L"orphan-send"))CHECK(restore_sql(db,"DELETE FROM events WHERE kind='rpc_send_intent'"));
 else if(!wcscmp(mode,L"orphan-cost"))CHECK(restore_sql(db,"DELETE FROM events WHERE kind='rpc_network_end'"));
 else if(!wcscmp(mode,L"parameter-conflict"))CHECK(restore_set_policy(db,"request_limit","4"));
 else if(!wcscmp(mode,L"price-conflict"))CHECK(restore_set_policy(db,"input_nano_per_token","301"));
 else if(!wcscmp(mode,L"scope-conflict"))CHECK(restore_sql(db,"UPDATE runs SET input='wrong business scope'"));
 else if(!wcscmp(mode,L"unknown-conflict"))CHECK(restore_sql(db,"UPDATE events SET payload=replace(payload,'\"unknown\":true','\"unknown\":false') WHERE kind='rpc_network_end'"));
 else if(!wcscmp(mode,L"unmarked-real"))CHECK(restore_sql(db,"DELETE FROM events WHERE kind GLOB 'rpc_*' AND kind<>'rpc_open'; UPDATE runs SET plan=replace(plan,',\"validation_kind\":2','')"));
 else if(!wcscmp(mode,L"task-only"))CHECK(restore_sql(db,"DELETE FROM events WHERE kind GLOB 'rpc_*'"));
 else if(!wcscmp(mode,L"unknown-policy"))CHECK(restore_set_policy(db,"policy","unrecognized"));
 else if(!wcscmp(mode,L"query-failure"))CHECK(restore_sql(db,"ALTER TABLE events RENAME TO unavailable_events"));
 else CHECK(!wcsncmp(mode,L"intact",6));
 unsigned reserved=restore_count(db,"rpc_reserved"),sends=restore_count(db,"rpc_send_entered"),confirms=restore_count(db,"disclosure");
 if(wcscmp(mode,L"query-failure"))CHECK(reserved!=UINT_MAX&&sends!=UINT_MAX&&confirms!=UINT_MAX);
 sqlite3_close(db);db=NULL;
 CHECK(SetEnvironmentVariableW(L"UI_AI_DB_PATH",database));
 CHECK(SetEnvironmentVariableW(L"UI_AI_BR2_POLICY",NULL));
 host_window_t host={0};CHECK(create_host(&host,GetModuleHandleW(NULL))!=NULL);
 if(!host.ai)return 1;
 ShowWindow(host.hwnd,SW_HIDE);
 DWORD start=GetTickCount();ui_ai_service_state_t state={0};state.size=sizeof(state);state.version=1;
 do{
  pump(&host);CHECK(ui_ai_service_get_state(host.ai,&state)==UI_STATUS_OK);
  if(state.phase==UI_AI_READY||state.phase==UI_AI_UNAVAILABLE)break;
 }while(GetTickCount()-start<5000);
 char *guard=NULL;ui_status_t guard_status=ui_ai_validation_get_state(host.ai,&guard);
 int stopped=guard_status==UI_STATUS_OK&&guard&&strstr(guard,"\"stopped\":true")&&strstr(guard,"\"ready\":false");
 fprintf(stderr,"RESTORE mode=%ls database=%s phase=%d guard=%s error=%s\n",mode,path,state.phase,guard?guard:"<none>",state.error);
 int intact=!wcsncmp(mode,L"intact",6);
 CHECK(intact?(state.phase==UI_AI_READY&&stopped):state.phase==UI_AI_UNAVAILABLE);
 if(state.phase==UI_AI_UNAVAILABLE)CHECK(state.error[0]&&host.ai->store_failed);
 if(!wcscmp(mode,L"intact")){
  CHECK(state.phase==UI_AI_READY&&stopped);
  CHECK(guard&&strstr(guard,"\"reserved\":2")&&strstr(guard,"\"simulated_send_entries\":2")&&strstr(guard,"\"known_nano_usd\":15600"));
 }
 if(!wcscmp(mode,L"intact-unknown"))CHECK(guard&&strstr(guard,"\"reserved\":1")&&strstr(guard,"\"estimated_nano_usd\":5515200")&&strstr(guard,"\"fee_unknown\":true"));
 if(!wcscmp(mode,L"intact-remote-unknown"))CHECK(guard&&strstr(guard,"\"reserved\":1")&&strstr(guard,"\"simulated_unknown\":1")&&strstr(guard,"\"fee_unknown\":true"));
 if(!wcscmp(mode,L"intact-unverified")||!wcscmp(mode,L"intact-storage-prefix"))CHECK(guard&&strstr(guard,"\"reserved\":0")&&strstr(guard,"\"stopped\":true"));
 ui_ai_release(guard);
 ui_ai_real_task_t proposal={0};char run[65];
 proposal.size=sizeof(proposal);proposal.version=1;proposal.client_request_id="restore-must-not-admit";
 proposal.profile_id="deepseek";proposal.profile_revision=1;
 proposal.input_utf8="BR-1：读取本轮合成交接单标题、来源与版本，不导航或修改。";
 proposal.history_selection_json="[]";proposal.disclosure_projection_json="{\"capabilities\":[],\"fields\":[]}";
 CHECK(ui_ai_real_propose(host.ai,&proposal,run)!=UI_STATUS_OK);
 CHECK(ui_ai_model_fixture_light(host.ai,host.view,"ai-model-confirm")!=UI_STATUS_OK);
 CHECK(host.ai->model_config->requests==0&&host.ai->methods==0&&host.ai->callbacks==0);
 if(state.phase==UI_AI_UNAVAILABLE){
  PostMessageW(host.hwnd,WM_CLOSE,0,0);start=GetTickCount();
  do{pump(&host);}while(!host.ai_exit_error_ack&&GetTickCount()-start<5000);
  CHECK(host.ai_exit_error_ack&&host.ai&&host.ai->store_failed);
 }
 CHECK(close_host(&host));
 char uri[4200];snprintf(uri,sizeof(uri),"file:%s?immutable=1",path);
 CHECK(sqlite3_open_v2(uri,&db,SQLITE_OPEN_READONLY|SQLITE_OPEN_URI,NULL)==SQLITE_OK);
 CHECK(restore_count(db,"rpc_reserved")==reserved);CHECK(restore_count(db,"rpc_send_entered")==sends);CHECK(restore_count(db,"disclosure")==confirms);
 sqlite3_close(db);
 SetEnvironmentVariableW(L"UI_AI_DB_PATH",NULL);
 printf("restore %ls checks=%u failures=%u original_R=%u original_S=%u original_confirm=%u\n",mode,checks,failures,reserved,sends,confirms);
 return failures?1:0;
}
