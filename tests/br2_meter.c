/* Production host/read chain, isolated transport and test-only trusted events. */
#define BR2_GUARD_MAIN meter_previous_guard_main
#include "br2_guard.c"
/* Re-measured with the pinned pure component after the discovery Schema change. */
#define BR2_INITIAL_INPUT_TOKENS 538u

static void meter_policy(const wchar_t *mode)
{
 make_policy(1,300,!wcscmp(mode,L"initial-limit")?1:!wcscmp(mode,L"continuation-limit")?BR2_INITIAL_INPUT_TOKENS:2000);
 yyjson_doc *d=yyjson_read(policy,strlen(policy),0);
 yyjson_mut_doc *m=yyjson_doc_mut_copy(d,NULL);
 yyjson_mut_val *root=yyjson_mut_doc_get_root(m);
 yyjson_mut_obj_remove_str(root,"fixture_legacy_meter");
 if(wcscmp(mode,L"missing-profile")){
  yyjson_mut_obj_add_strcpy(m,root,"meter_profile",!wcscmp(mode,L"bad-profile")?"unknown":"deepseek-v41-responses-1017-v1");
  yyjson_mut_obj_add_strcpy(m,root,"meter_evidence_sha256",BR2_METER_EVIDENCE);
 }
 char *encoded=yyjson_mut_write(m,0,NULL);
 CHECK(encoded!=NULL);
 if(encoded){strcpy_s(policy,sizeof(policy),encoded);free(encoded);}
 yyjson_mut_doc_free(m);yyjson_doc_free(d);
}

static unsigned meter_expected(const wchar_t *mode)
{
 if(wcsstr(mode,L"-meter")||!wcscmp(mode,L"unconfirmed")||!wcscmp(mode,L"initial-limit")||!wcsncmp(mode,L"component-",10)||!wcscmp(mode,L"receipt-mismatch"))return 0;
 if(!wcscmp(mode,L"continuation-limit")||!wcscmp(mode,L"usage-over"))return 1;
 return !wcscmp(mode,L"quota")?3:2;
}

static void meter_closed(const wchar_t *database,unsigned expected)
{
 char path[4096],uri[4200];wchar_t side[4100];sqlite3 *sql=NULL;sqlite3_stmt *query=NULL;
 CHECK(WideCharToMultiByte(CP_UTF8,0,database,-1,path,sizeof(path),NULL,NULL)>0);
 _snwprintf_s(side,4100,_TRUNCATE,L"%s-wal",database);CHECK(GetFileAttributesW(side)==INVALID_FILE_ATTRIBUTES);
 _snwprintf_s(side,4100,_TRUNCATE,L"%s-shm",database);CHECK(GetFileAttributesW(side)==INVALID_FILE_ATTRIBUTES);
 snprintf(uri,sizeof(uri),"file:%s?immutable=1",path);
 CHECK(sqlite3_open_v2(uri,&sql,SQLITE_OPEN_READONLY|SQLITE_OPEN_URI,NULL)==SQLITE_OK);
 CHECK(sqlite3_prepare_v2(sql,"SELECT payload FROM events WHERE kind='rpc_reserved' ORDER BY seq",-1,&query,NULL)==SQLITE_OK);
 unsigned count=0;uint64_t previous=0;
 while(sqlite3_step(query)==SQLITE_ROW){
  const char *raw=(const char *)sqlite3_column_text(query,0);yyjson_doc *d=yyjson_read(raw,strlen(raw),0);
  yyjson_val *root=yyjson_doc_get_root(d),*receipt=yyjson_obj_get(root,"meter");
  const char *profile=yyjson_get_str(yyjson_obj_get(receipt,"profile"));
  const char *evidence=yyjson_get_str(yyjson_obj_get(receipt,"evidence"));
  const char *hash=yyjson_get_str(yyjson_obj_get(receipt,"body_hash")),*actual=yyjson_get_str(yyjson_obj_get(root,"body_hash"));
  uint64_t tokens=yyjson_get_uint(yyjson_obj_get(receipt,"input_tokens"));
  CHECK(profile&&!strcmp(profile,"deepseek-v41-responses-1017-v1"));
  CHECK(evidence&&!_stricmp(evidence,BR2_METER_EVIDENCE));CHECK(hash&&actual&&!strcmp(hash,actual));
  CHECK(yyjson_get_uint(yyjson_obj_get(receipt,"output_limit"))==4096);
  CHECK(tokens>previous);if(!count)CHECK(tokens==BR2_INITIAL_INPUT_TOKENS);previous=tokens;
  fprintf(stderr,"meter persisted ordinal=%u input=%llu hash=%s\n",count+1,(unsigned long long)tokens,hash?hash:"");
  ++count;yyjson_doc_free(d);
 }
 CHECK(count==expected);sqlite3_finalize(query);sqlite3_close(sql);
}

static void meter_reopen(host_window_t *host,const wchar_t *database,const wchar_t *mode)
{
 wchar_t copy[4096];int damaged=wcsstr(mode,L"restore-")!=NULL;
 if(damaged){
  _snwprintf_s(copy,4096,_TRUNCATE,L"%s.%ls.db",database,mode);
  CHECK(CopyFileW(database,copy,TRUE));char path[4096];
  CHECK(WideCharToMultiByte(CP_UTF8,0,copy,-1,path,sizeof(path),NULL,NULL)>0);
  fprintf(stderr,"METER_RESTORE_DB mode=%ls path=%s damaged=1\n",mode,path);
  sqlite3 *sql=NULL;CHECK(sqlite3_open_v2(path,&sql,SQLITE_OPEN_READWRITE,NULL)==SQLITE_OK);
  const char *change=!wcscmp(mode,L"restore-missing")?"UPDATE events SET payload=json_remove(payload,'$.meter') WHERE kind='rpc_reserved'":
   !wcscmp(mode,L"restore-downgrade")?"UPDATE events SET payload=json_remove(payload,'$.meter_profile','$.meter_evidence_sha256') WHERE kind='rpc_open'":
   "UPDATE events SET payload=json_set(payload,'$.meter.input_tokens',2001) WHERE kind='rpc_reserved'";
  CHECK(sqlite3_exec(sql,change,NULL,NULL,NULL)==SQLITE_OK);sqlite3_close(sql);
  CHECK(SetEnvironmentVariableW(L"UI_AI_DB_PATH",copy));
 }
 memset(host,0,sizeof(*host));CHECK(create_host(host,GetModuleHandleW(NULL))!=NULL);
 ShowWindow(host->hwnd,SW_HIDE);
 if(damaged){
  ui_ai_service_state_t state={0};state.size=sizeof(state);state.version=1;DWORD start=GetTickCount();
  do{pump(host);CHECK(ui_ai_service_get_state(host->ai,&state)==UI_STATUS_OK);}while(state.phase==UI_AI_OPENING&&GetTickCount()-start<5000);
  CHECK(state.phase==UI_AI_UNAVAILABLE);CHECK(strstr(state.error,"business_guard_restore")!=NULL);
  PostMessageW(host->hwnd,WM_CLOSE,0,0);start=GetTickCount();
  do{pump(host);}while(!host->ai_exit_error_ack&&GetTickCount()-start<5000);
  CHECK(host->ai_exit_error_ack&&host->ai&&host->ai->store_failed);
 }else{
  CHECK(guard_state(host,"validation_restart_review_required"));
  CHECK(guard_state(host,"\"reserved\":2"));CHECK(guard_state(host,"\"stopped\":true"));
 }
 CHECK(host->ai->model_config->requests==0);CHECK(close_host(host));
}

int wmain(int argc,wchar_t **argv)
{
 if(argc!=4)return 2;
 host_window_t s={0};host_action_t action={0};GUID guid;
 wchar_t db[4096],unique[40],settings[4096];const wchar_t *mode=argv[3];int barrier=wcsstr(mode,L"-meter")!=NULL;
 CHECK(CreateDirectoryW(argv[2],NULL)||GetLastError()==ERROR_ALREADY_EXISTS);
 CHECK(CoCreateGuid(&guid)==S_OK);StringFromGUID2(&guid,unique,40);
 _snwprintf_s(db,4096,_TRUNCATE,L"%s/meter-%s.db",argv[2],unique);
 _snwprintf_s(settings,4096,_TRUNCATE,L"%s/meter-%s.origins",argv[2],unique);
 HANDLE file=CreateFileW(settings,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
 CHECK(file!=INVALID_HANDLE_VALUE);if(file!=INVALID_HANDLE_VALUE)CloseHandle(file);
 CHECK(WideCharToMultiByte(CP_UTF8,0,db,-1,policy_database,sizeof(policy_database),NULL,NULL)>0);
 fprintf(stderr,"METER_DB mode=%ls path=%s\n",mode,policy_database);
 CHECK(SetEnvironmentVariableW(L"UI_AI_DB_PATH",db));
 CHECK(SetEnvironmentVariableW(L"UI_BROWSER_SETTINGS_PATH",settings));
 CHECK(ui_framework_initialize()==UI_STATUS_OK);
 CHECK(create_host(&s,GetModuleHandleW(NULL))!=NULL);if(!s.ai)return 1;
 CHECK(model_wait(&s,NULL));CHECK(MoveWindow(s.hwnd,-16000,-16000,1200,3200,FALSE));
 ShowWindow(s.hwnd,SW_HIDE);action.action="assistant";process_action(&s,&action);pump(&s);
 click(&s,"ai-model-toggle");text(&s,"ai-model-name","offline-fixture");click(&s,"ai-model-save");
 CHECK(model_wait(&s,NULL));
 if(!s.ai_model||!strstr(s.ai_model,"\"model_id\":\"offline-fixture\"")){CHECK(0);goto finish;}
 click(&s,"ai-model-toggle");meter_policy(mode);
 CHECK(ui_ai_business_validation_enable(s.ai,policy)==UI_STATUS_OK);
 if(!wcscmp(mode,L"missing-profile")||!wcscmp(mode,L"bad-profile")){
  CHECK(guard_state(&s,"\"stopped\":true"));
  CHECK(guard_state(&s,"\"reserved\":0"));
  CHECK(s.ai->model_config->requests==0);goto finish;
 }
 CHECK(guard_state(&s,"\"ready\":true"));
 char *read=replace_read(rp_read);
 if(!wcscmp(mode,L"usage-over")){
  const char *at=strstr(read,"\"input_tokens\":10");CHECK(at!=NULL);
  if(at){size_t size=strlen(read)+3;char *changed=malloc(size);CHECK(changed!=NULL);if(changed){snprintf(changed,size,"%.*s\"input_tokens\":1000%s",(int)(at-read),read,at+17);free(read);read=changed;}}
 }
 yyjson_mut_doc *responses=yyjson_mut_doc_new(NULL);yyjson_mut_val *wires=yyjson_mut_arr(responses);
 yyjson_mut_doc_set_root(responses,wires);
 unsigned reads=!wcscmp(mode,L"quota")?3:1;
 for(unsigned ri=0;ri<reads;++ri){
  char *wire=_strdup(read);for(char *at=wire;(at=strstr(at,"call-0"))!=NULL;at+=6)at[5]=(char)('0'+ri);
  yyjson_mut_arr_add_strcpy(responses,wires,wire);free(wire);
 }
 yyjson_mut_arr_add_strcpy(responses,wires,rp_text);char *encoded=yyjson_mut_write(responses,0,NULL);
 CHECK(ui_ai_model_fixture(s.ai,1,encoded)==UI_STATUS_OK);
 CHECK(ui_ai_model_fixture(s.ai,5,"meter-isolated-sentinel")==UI_STATUS_OK);
 free(read);free(encoded);yyjson_mut_doc_free(responses);
 open_path(&s,argv[1]);pump(&s);click(&s,"ai-draft-range-toggle");click(&s,"ai-model-catalog");pump(&s);
 int index=browser_index(&s);CHECK(index>=0);if(index<0)goto finish;
 click(&s,"ai-resource-select-0");char node[80];snprintf(node,80,"ai-cap-select-%d",index);click(&s,node);
 yyjson_doc *choices=yyjson_read(s.ai_choices,strlen(s.ai_choices),0);
 yyjson_val *cap=yyjson_arr_get(yyjson_obj_get(yyjson_doc_get_root(choices),"capabilities"),(size_t)index),*field;
 size_t i,n;unsigned fields=0;
 yyjson_arr_foreach(yyjson_obj_get(cap,"fields"),i,n,field){
  const char *name=yyjson_get_str(field);
  if(!strcmp(name,"state")||!strcmp(name,"revision")){snprintf(node,80,"ai-field-select-%d-%u",index,(unsigned)i);click(&s,node);++fields;}
 }
 CHECK(fields==2);yyjson_doc_free(choices);
 text(&s,"ai-input","BR-1：读取本轮合成交接单标题、来源与版本，不导航或修改。");
 click(&s,"ai-model-propose");CHECK(model_wait(&s,"\"phase\":3"));
 if(!s.ai_run[0]){CHECK(0);goto finish;}
 CHECK(bound(&s));CHECK(s.ai->model_config->requests==0);CHECK(guard_state(&s,"\"reserved\":0"));
 text(&s,"ai-input","BR-2计量未提交草稿");
 if(!wcscmp(mode,L"unconfirmed")){
  ++s.ai->methods;click(&s,"ai-range-close");CHECK(!s.ai_preview_ready);
  CHECK(ui_ai_model_fixture_light(s.ai,s.view,"ai-model-confirm")!=UI_STATUS_OK);--s.ai->methods;
  CHECK(s.ai->model_config->requests==0);CHECK(guard_state(&s,"\"meter_started\":false"));
  click(&s,"ai-view-scope");CHECK(bound(&s));CHECK(ui_ai_cancel(s.ai,s.ai_run)==UI_STATUS_OK);CHECK(model_wait(&s,"\"terminal\":true"));goto finish;
 }
 if(!wcscmp(mode,L"component-missing"))s.ai->model_config->fixture_meter_fault=1;
 if(!wcscmp(mode,L"receipt-mismatch"))s.ai->model_config->fixture_meter_fault=2;
 if(!wcscmp(mode,L"component-identity"))s.ai->model_config->fixture_meter_fault=3;
 if(!wcscmp(mode,L"component-exit"))s.ai->model_config->fixture_meter_fault=4;
 if(barrier){s.ai->model_config->fixture_meter_enter=CreateEventW(NULL,TRUE,FALSE,NULL);s.ai->model_config->fixture_meter_release=CreateEventW(NULL,TRUE,FALSE,NULL);}
 CHECK(ui_ai_model_fixture_light(s.ai,s.view,"ai-model-confirm")==UI_STATUS_OK);
 CHECK(ui_ai_model_fixture_light(s.ai,s.view,"ai-model-confirm")!=UI_STATUS_OK);
 if(barrier){
  DWORD start=GetTickCount();while(WaitForSingleObject(s.ai->model_config->fixture_meter_enter,0)!=WAIT_OBJECT_0&&GetTickCount()-start<5000)pump(&s);
  CHECK(WaitForSingleObject(s.ai->model_config->fixture_meter_enter,0)==WAIT_OBJECT_0);
  CHECK(s.ai->model_config->requests==0);CHECK(ui_ai_service_destroy(s.ai)==UI_STATUS_PENDING);
  if(!wcscmp(mode,L"close-meter"))CHECK(ui_ai_service_request_close(s.ai)==UI_STATUS_OK);
  else if(!wcscmp(mode,L"deadline-meter"))ui_ai_edit_fixture(s.ai,3,600000);
  else if(!wcscmp(mode,L"target-meter"))CHECK(ui_workspace_close(s.workspace,ui_workspace_active(s.workspace),UI_APP_CLOSE_TAB)==UI_STATUS_OK);
  else if(!wcscmp(mode,L"storage-meter"))ui_ai_validation_fixture(s.ai,6);
  else CHECK(ui_ai_cancel(s.ai,s.ai_run)==UI_STATUS_OK);
  SetEvent(s.ai->model_config->fixture_meter_release);
 }
 if(!wcscmp(mode,L"close-meter"))goto finish;
 if(!wcscmp(mode,L"storage-meter")){
  CHECK(guard_state(&s,"\"reserved\":1"));CHECK(s.ai->model_config->requests==0);CHECK(s.ai->store_failed);
  PostMessageW(s.hwnd,WM_CLOSE,0,0);DWORD start=GetTickCount();do{pump(&s);}while(!s.ai_exit_error_ack&&GetTickCount()-start<5000);
  CHECK(s.ai_exit_error_ack&&s.ai&&s.ai->store_failed);goto finish;
 }
 CHECK(model_wait(&s,"\"terminal\":true"));
 unsigned expected=meter_expected(mode);
 CHECK(s.ai->model_config->requests==expected);
 char count[80];snprintf(count,80,"\"reserved\":%u",expected);CHECK(guard_state(&s,count));
 if(expected==2){CHECK(model_wait(&s,"\"outcome\":\"completed\""));}
 CHECK(guard_state(&s,"\"stopped\":true"));
 if(!wcscmp(mode,L"usage-over"))CHECK(guard_state(&s,"\"fee_unknown\":true"));
 if(s.ai->model_config->requests){
  wchar_t body_path[4120];_snwprintf_s(body_path,4120,_TRUNCATE,L"%s.last-body.json",db);
  FILE *body_file=_wfopen(body_path,L"wbx");CHECK(body_file!=NULL);
  if(body_file){const char *body=s.ai->model_config->fixture_body;CHECK(fwrite(body,1,strlen(body),body_file)==strlen(body));fclose(body_file);}
 }
 {ui_element_presentation_t p={0};p.size=sizeof(p);CHECK(ui_web_view_get_presentation(s.view,"ai-input",&p)==UI_STATUS_OK);CHECK(p.text_utf8&&!strcmp(p.text_utf8,"BR-2计量未提交草稿"));}
finish:
 {HANDLE enter=s.ai->model_config->fixture_meter_enter,release=s.ai->model_config->fixture_meter_release;
 CHECK(close_host(&s));
 if(enter)CloseHandle(enter);if(release)CloseHandle(release);}
 if(wcscmp(mode,L"missing-profile")&&wcscmp(mode,L"bad-profile")){
  unsigned persisted=meter_expected(mode);
  meter_closed(db,persisted);
  if(persisted==2){closed_proof(db,2,2,1);meter_reopen(&s,db,mode);meter_closed(db,2);}
 }
 SetEnvironmentVariableW(L"UI_AI_DB_PATH",NULL);SetEnvironmentVariableW(L"UI_BROWSER_SETTINGS_PATH",NULL);
 printf("BR2 meter %ls checks=%u failures=%u\n",mode,checks,failures);return failures?1:0;
}
