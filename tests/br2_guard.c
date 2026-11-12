/* BR-2 actual browser/host, isolated transport and synthetic trusted fixture only. */
#define BR1_BUSINESS_MAIN br2_previous_business_main
#include "business_read.c"
#include "ui_framework/ai_business_validation.h"
__declspec(dllimport) unsigned ui_ai_validation_fixture(ui_ai_service_t *,int);
static char policy[8192],policy_database[4096];
static DWORD WINAPI wrong_thread(void *data){return (DWORD)ui_ai_business_validation_enable(data,policy);}
static void make_policy(int verified,unsigned rate,unsigned ceiling)
{snprintf(policy,sizeof(policy),"{\"version\":1,\"validation_id\":\"br2-offline\",\"model\":\"offline-fixture\",\"protocol\":\"deepseek-responses-v1\",\"price_version\":\"fixture-1\",\"price_evidence_sha256\":\"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\",\"price_and_ceiling_verified\":%s,\"input_nano_per_token\":%u,\"output_nano_per_token\":1200,\"billable_input_token_ceiling\":%u}",verified?"true":"false",rate,ceiling);yyjson_doc *d=yyjson_read(policy,strlen(policy),0);yyjson_mut_doc *m=yyjson_doc_mut_copy(d,NULL);yyjson_mut_obj_add_strcpy(m,yyjson_mut_doc_get_root(m),"database_path_utf8",policy_database);yyjson_mut_obj_add_bool(m,yyjson_mut_doc_get_root(m),"fixture_legacy_meter",true);char *encoded=yyjson_mut_write(m,0,NULL);strcpy_s(policy,sizeof(policy),encoded);free(encoded);yyjson_mut_doc_free(m);yyjson_doc_free(d);}
static int guard_state(host_window_t *s,const char *part)
{DWORD begin=GetTickCount();do{char *j=NULL;pump(s);if(!ui_ai_validation_get_state(s->ai,&j)){int ok=strstr(j,part)!=NULL;if(ok)fprintf(stderr,"BR2 state %s\n",j);ui_ai_release(j);if(ok)return 1;}}while(GetTickCount()-begin<5000);return 0;}
static host_window_t *callback_host;
static ui_ai_model_display_check_fn original_check;
static void *original_data;
static unsigned callback_seen;
static int callback_close;
static ui_status_t reentrant_display(void *data,const char *run,const char *hash,uint64_t generation)
{
 (void)data;host_window_t *s=callback_host;
 if(!callback_seen++){
  CHECK(s->ai->methods>0);CHECK(ui_ai_model_unbind(s->ai,s->ai_model_bindings[1])==UI_STATUS_PENDING);
  CHECK(ui_ai_model_fixture_light(s->ai,s->view,"ai-model-confirm")!=UI_STATUS_OK);
  if(callback_close)CHECK(ui_ai_service_request_close(s->ai)==UI_STATUS_OK);
  CHECK(ui_ai_service_destroy(s->ai)==UI_STATUS_PENDING);
 }
 return original_check(original_data,run,hash,generation);
}
static char *wire_usage(const char *wire,int unknown)
{const char *event=strstr(wire,"event: response.completed"),*data=event?strstr(event,"data: "):NULL;if(!data)return _strdup(wire);data+=6;const char *end=strchr(data,'\n');if(!end)return _strdup(wire);if(end[-1]=='\r')--end;yyjson_doc *d=yyjson_read(data,(size_t)(end-data),0);yyjson_mut_doc *m=yyjson_doc_mut_copy(d,NULL);yyjson_mut_val *r=yyjson_mut_obj_get(yyjson_mut_doc_get_root(m),"response");if(unknown)yyjson_mut_obj_remove_str(r,"usage");char *json=yyjson_mut_write(m,0,NULL);size_t prefix=(size_t)(data-wire),n=prefix+strlen(json)+strlen(end)+1;char *out=malloc(n);memcpy(out,wire,prefix);strcpy_s(out+prefix,n-prefix,json);strcat_s(out,n,end);free(json);yyjson_doc_free(d);yyjson_mut_doc_free(m);return out;}
static void closed_proof(const wchar_t *db,unsigned requests,unsigned reserved,int business)
{
 char path[4096],uri[4200];CHECK(WideCharToMultiByte(CP_UTF8,0,db,-1,path,sizeof(path),NULL,NULL)>0);snprintf(uri,sizeof(uri),"file:%s?immutable=1",path);
 wchar_t side[4100];_snwprintf_s(side,4100,_TRUNCATE,L"%s-wal",db);CHECK(GetFileAttributesW(side)==INVALID_FILE_ATTRIBUTES);
 sqlite3 *sql=NULL;sqlite3_stmt *q=NULL;CHECK(sqlite3_open_v2(uri,&sql,SQLITE_OPEN_READONLY|SQLITE_OPEN_URI,NULL)==SQLITE_OK);
 CHECK(sqlite3_prepare_v2(sql,"SELECT (SELECT count(*) FROM runs),(SELECT count(*) FROM events WHERE kind='rpc_reserved'),(SELECT count(*) FROM events WHERE kind='rpc_send_entered')",-1,&q,NULL)==SQLITE_OK);
 CHECK(sqlite3_step(q)==SQLITE_ROW);CHECK(sqlite3_column_int(q,0)==1);CHECK((unsigned)sqlite3_column_int(q,1)==reserved);CHECK((unsigned)sqlite3_column_int(q,2)==requests);sqlite3_finalize(q);q=NULL;
 if(business){
  CHECK(sqlite3_prepare_v2(sql,"SELECT tools.result,runs.targets FROM tools JOIN runs ON runs.id=tools.run_id",-1,&q,NULL)==SQLITE_OK);CHECK(sqlite3_step(q)==SQLITE_ROW);
  const char *raw=(const char *)sqlite3_column_text(q,0),*targets=(const char *)sqlite3_column_text(q,1);fprintf(stderr,"BR2 stored result %s targets %s\n",raw?raw:"",targets?targets:"");
  yyjson_doc *d=raw?yyjson_read(raw,strlen(raw),0):NULL,*t=targets?yyjson_read(targets,strlen(targets),0):NULL;yyjson_val *root=yyjson_doc_get_root(d),*state=yyjson_obj_get(root,"state"),*target=yyjson_arr_get(yyjson_doc_get_root(t),0);
  const char *title=yyjson_get_str(yyjson_obj_get(state,"title")),*address=yyjson_get_str(yyjson_obj_get(state,"address")),*revision=yyjson_get_str(yyjson_obj_get(root,"revision")),*fixed=yyjson_get_str(yyjson_obj_get(target,"revision"));
  CHECK(title&&!strcmp(title,"BR-1 合成交接单：设备巡检"));CHECK(address&&!strcmp(address,"browser:start"));CHECK(revision&&fixed&&!strcmp(revision,fixed));CHECK(yyjson_is_int(yyjson_obj_get(state,"policy_count"))&&yyjson_get_int(yyjson_obj_get(state,"policy_count"))==0);
  yyjson_doc_free(d);yyjson_doc_free(t);sqlite3_finalize(q);
 }
 sqlite3_close(sql);
}
#ifndef BR2_GUARD_MAIN
#define BR2_GUARD_MAIN wmain
#endif
int BR2_GUARD_MAIN(int argc,wchar_t **argv)
{
 if(argc!=4)return 2;host_window_t s={0};host_action_t action={0};GUID g;wchar_t db[4096],u[40],settings[4096],policyfile[4096];const wchar_t *mode=argv[3];
 CHECK(CreateDirectoryW(argv[2],NULL)||GetLastError()==ERROR_ALREADY_EXISTS);CHECK(CoCreateGuid(&g)==S_OK);StringFromGUID2(&g,u,40);
 _snwprintf_s(db,4096,_TRUNCATE,L"%s/br2-%s.db",argv[2],u);_snwprintf_s(settings,4096,_TRUNCATE,L"%s/br2-%s.origins",argv[2],u);HANDLE file=CreateFileW(settings,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);CHECK(file!=INVALID_HANDLE_VALUE);if(file!=INVALID_HANDLE_VALUE)CloseHandle(file);
 CHECK(WideCharToMultiByte(CP_UTF8,0,db,-1,policy_database,sizeof(policy_database),NULL,NULL)>0);if(!wcscmp(mode,L"wrong-db"))strcat_s(policy_database,sizeof(policy_database),".other");
 int host_policy=!wcsncmp(mode,L"host-",5);int success=!wcscmp(mode,L"read")||!wcscmp(mode,L"host-policy")||!wcscmp(mode,L"ledger-corrupt");
 if(host_policy){make_policy(!wcscmp(mode,L"host-policy"),300,2000);_snwprintf_s(policyfile,4096,_TRUNCATE,L"%s/br2-%s.policy.json",argv[2],u);if(wcscmp(mode,L"host-missing")){FILE *pf=_wfopen(policyfile,L"wb");CHECK(pf!=NULL);if(pf){CHECK(fwrite(policy,1,strlen(policy),pf)==strlen(policy));fclose(pf);}}CHECK(SetEnvironmentVariableW(L"UI_AI_BR2_POLICY",policyfile));}
 CHECK(SetEnvironmentVariableW(L"UI_AI_DB_PATH",db));CHECK(SetEnvironmentVariableW(L"UI_BROWSER_SETTINGS_PATH",settings));CHECK(ui_framework_initialize()==UI_STATUS_OK);CHECK(create_host(&s,GetModuleHandleW(NULL))!=NULL);if(!s.ai)return 1;CHECK(model_wait(&s,NULL));CHECK(MoveWindow(s.hwnd,-16000,-16000,1200,3200,FALSE));ShowWindow(s.hwnd,SW_HIDE);action.action="assistant";process_action(&s,&action);pump(&s);
 click(&s,"ai-model-toggle");text(&s,"ai-model-name","offline-fixture");click(&s,"ai-model-save");CHECK(model_wait(&s,NULL));if(!s.ai_model||!strstr(s.ai_model,"\"model_id\":\"offline-fixture\"")){CHECK(0);goto finish;}click(&s,"ai-model-toggle");
 make_policy(wcscmp(mode,L"unverified")!=0,!wcscmp(mode,L"fee")?1000000:!wcscmp(mode,L"fee-edge")?950848:300,!wcscmp(mode,L"fee-edge")?100:!wcscmp(mode,L"usage-over")?9:2000);CHECK(ui_ai_business_validation_enable(s.ai,policy)==(host_policy?UI_STATUS_ALREADY_EXISTS:UI_STATUS_OK));CHECK(ui_ai_business_validation_enable(s.ai,policy)==UI_STATUS_ALREADY_EXISTS);
 HANDLE wrong=CreateThread(NULL,0,wrong_thread,s.ai,0,NULL);CHECK(wrong!=NULL);if(wrong){CHECK(WaitForSingleObject(wrong,5000)==WAIT_OBJECT_0);DWORD status=0;CHECK(GetExitCodeThread(wrong,&status)&&status==UI_STATUS_INVALID_ARGUMENT);CloseHandle(wrong);}
 if(!wcscmp(mode,L"unverified")||!wcscmp(mode,L"wrong-db")||!wcscmp(mode,L"host-unverified")||!wcscmp(mode,L"host-missing")){CHECK(guard_state(&s,"business_price_or_ceiling_unverified"));CHECK(s.ai->model_config->requests==0);goto finish;}
 CHECK(guard_state(&s,"\"ready\":true"));
 yyjson_mut_doc *responses=yyjson_mut_doc_new(NULL);yyjson_mut_val *wires=yyjson_mut_arr(responses);yyjson_mut_doc_set_root(responses,wires);unsigned reads=!wcscmp(mode,L"quota")?3:1;
 for(unsigned i=0;i<reads;++i){char *read=replace_read(rp_read);for(char *at=read;(at=strstr(at,"call-0"))!=NULL;at+=6)at[5]=(char)('0'+i);char *wire=wire_usage(read,!wcscmp(mode,L"unknown"));if(!wcscmp(mode,L"disconnect")){char *cut=strstr(wire,"event: response.completed");if(cut)*cut=0;}yyjson_mut_arr_add_strcpy(responses,wires,wire);free(wire);free(read);}
 yyjson_mut_arr_add_strcpy(responses,wires,rp_text);char *encoded=yyjson_mut_write(responses,0,NULL);CHECK(ui_ai_model_fixture(s.ai,1,encoded)==UI_STATUS_OK);CHECK(ui_ai_model_fixture(s.ai,5,"br2-isolated-sentinel")==UI_STATUS_OK);free(encoded);yyjson_mut_doc_free(responses);
 open_path(&s,argv[1]);pump(&s);click(&s,"ai-draft-range-toggle");click(&s,"ai-model-catalog");pump(&s);int ix=browser_index(&s);CHECK(ix>=0);if(ix<0)goto finish;click(&s,"ai-resource-select-0");char node[80];snprintf(node,80,"ai-cap-select-%d",ix);click(&s,node);
 yyjson_doc *choices=yyjson_read(s.ai_choices,strlen(s.ai_choices),0);yyjson_val *cap=yyjson_arr_get(yyjson_obj_get(yyjson_doc_get_root(choices),"capabilities"),(size_t)ix),*field;size_t fi,fn;unsigned fields=0;yyjson_arr_foreach(yyjson_obj_get(cap,"fields"),fi,fn,field){const char *name=yyjson_get_str(field);if(!strcmp(name,"state")||!strcmp(name,"revision")){snprintf(node,80,"ai-field-select-%d-%u",ix,(unsigned)fi);click(&s,node);++fields;}}CHECK(fields==2);yyjson_doc_free(choices);
 text(&s,"ai-input","BR-1：读取本轮合成交接单标题、来源与版本，不导航或修改。");click(&s,"ai-model-propose");CHECK(model_wait(&s,"\"phase\":3"));if(!s.ai_run[0]){CHECK(0);goto finish;}CHECK(bound(&s));char run[65];strcpy_s(run,65,s.ai_run);CHECK(s.ai->model_config->requests==0);CHECK(guard_state(&s,"\"reserved\":0"));
 if(success){
  ai_run_t *r=NULL;for(unsigned k=0;k<AI_RUNS;++k)if(!strcmp(s.ai->runs[k].id,run))r=&s.ai->runs[k];CHECK(r!=NULL);if(!r)goto finish;
  ui_ai_real_task_t again={0};again.size=sizeof(again);again.version=1;again.client_request_id=r->client;again.profile_id="deepseek";again.profile_revision=r->model->revision;again.input_utf8=r->input;again.history_selection_json="[]";yyjson_doc *plan=yyjson_read(r->plan,strlen(r->plan),0);again.disclosure_projection_json=yyjson_get_str(yyjson_obj_get(yyjson_doc_get_root(plan),"projection"));again.targets=r->targets;again.target_count=1;char same[65];ui_cap_target_t invalid=r->targets[0];memset(invalid.host_epoch,'x',sizeof(invalid.host_epoch));again.targets=&invalid;CHECK(ui_ai_real_propose(s.ai,&again,same)==UI_STATUS_PERMISSION_DENIED);again.targets=r->targets;CHECK(ui_ai_real_propose(s.ai,&again,same)==UI_STATUS_OK&&!strcmp(same,run));yyjson_doc_free(plan);
  ui_cap_target_t fixed=r->targets[0];uint64_t first=ui_workspace_active(s.workspace),second=0;char package[4096];CHECK(WideCharToMultiByte(CP_UTF8,0,argv[1],-1,package,sizeof(package),NULL,NULL)>0);CHECK(ui_workspace_open(s.workspace,package,&second)==UI_STATUS_OK&&second!=first);pump(&s);CHECK(!memcmp(&fixed,&r->targets[0],sizeof(fixed)));CHECK(!strcmp(s.ai_run,run));CHECK(ui_workspace_activate(s.workspace,first)==UI_STATUS_OK);pump(&s);CHECK(!memcmp(&fixed,&r->targets[0],sizeof(fixed)));CHECK(bound(&s));
 }
 text(&s,"ai-input","BR-2未提交草稿");++s.ai->methods;click(&s,"ai-range-close");CHECK(!s.ai_preview_ready);CHECK(ui_ai_model_fixture_light(s.ai,s.view,"ai-model-confirm")!=UI_STATUS_OK);CHECK(s.ai->model_config->requests==0);--s.ai->methods;click(&s,"ai-view-scope");CHECK(bound(&s));
 if(!wcscmp(mode,L"unconfirmed")){CHECK(ui_ai_cancel(s.ai,run)==UI_STATUS_OK);CHECK(model_wait(&s,"\"terminal\":true"));goto finish;}
 if(!wcscmp(mode,L"deadline"))CHECK(ui_ai_model_fixture(s.ai,7,"1")==UI_STATUS_OK);
 if(!wcscmp(mode,L"storage"))ui_ai_validation_fixture(s.ai,1);
 if(!wcscmp(mode,L"duplicate"))ui_ai_validation_fixture(s.ai,5);
 if(!wcscmp(mode,L"http"))CHECK(ui_ai_model_fixture(s.ai,4,"503")==UI_STATUS_OK);
 if(!wcscmp(mode,L"cancel")||!wcscmp(mode,L"inflight-expiry"))CHECK(ui_ai_model_fixture(s.ai,3,"1")==UI_STATUS_OK);
 am_binding_t *binding=s.ai->model_config->bindings;while(binding&&binding->id!=s.ai_model_bindings[1])binding=binding->next;CHECK(binding!=NULL);if(!binding)goto finish;
 original_check=binding->display_check;original_data=binding->display_data;callback_host=&s;callback_close=!wcscmp(mode,L"reentry-close");callback_seen=0;binding->display_check=reentrant_display;
 ui_status_t confirmed=ui_ai_model_fixture_light(s.ai,s.view,"ai-model-confirm");binding->display_check=original_check;CHECK(callback_seen>0);
 if(callback_close){CHECK(confirmed!=UI_STATUS_OK);CHECK(s.ai->model_config->requests==0);goto finish;}
 CHECK(confirmed==UI_STATUS_OK);CHECK(ui_ai_model_fixture_light(s.ai,s.view,"ai-model-confirm")!=UI_STATUS_OK);
 if(!wcscmp(mode,L"deadline")){ui_ai_edit_fixture(s.ai,3,600000);CHECK(ui_ai_model_fixture(s.ai,7,"0")==UI_STATUS_OK);}
 if(!wcscmp(mode,L"storage")){DWORD begin=GetTickCount();while(!ui_ai_validation_fixture(s.ai,0)&&GetTickCount()-begin<5000)pump(&s);CHECK(ui_ai_validation_fixture(s.ai,0)>0);ui_ai_validation_fixture(s.ai,3);CHECK(guard_state(&s,"\"stopped\":true"));CHECK(s.ai->model_config->requests==0);goto finish;}
 if(!wcscmp(mode,L"cancel")||!wcscmp(mode,L"inflight-expiry")){DWORD begin=GetTickCount();do{pump(&s);}while(!s.ai->model_config->requests&&GetTickCount()-begin<5000);CHECK(s.ai->model_config->requests==1);if(!wcscmp(mode,L"inflight-expiry"))ui_ai_edit_fixture(s.ai,3,600000);else CHECK(ui_ai_cancel(s.ai,run)==UI_STATUS_OK);CHECK(ui_ai_model_fixture(s.ai,3,"0")==UI_STATUS_OK);}
 CHECK(model_wait(&s,"\"terminal\":true"));unsigned expected=success?2:!wcscmp(mode,L"quota")?3:(!wcscmp(mode,L"fee")||!wcscmp(mode,L"deadline"))?0:1;CHECK(s.ai->model_config->requests==expected);char guard[80];snprintf(guard,80,"\"reserved\":%u",expected);CHECK(guard_state(&s,guard));
 if(success)CHECK(model_wait(&s,"\"outcome\":\"completed\""));
 CHECK(guard_state(&s,"\"stopped\":true"));
 action.action="ai-model-propose";action.token="br2-new-task-denied";action.answer="BR-1：读取本轮合成交接单标题、来源与版本，不导航或修改。";process_action(&s,&action);CHECK(!strcmp(s.ai_run,run));CHECK(s.ai->model_config->requests==expected);
 {ui_element_presentation_t p={0};p.size=sizeof(p);CHECK(ui_web_view_get_presentation(s.view,"ai-input",&p)==UI_STATUS_OK);CHECK(!strcmp(p.text_utf8,"BR-2未提交草稿"));}
finish:
 if(!wcscmp(mode,L"storage")){
  CHECK(s.ai->store_failed);CHECK(guard_state(&s,"\"reserved\":1"));
  PostMessageW(s.hwnd,WM_CLOSE,0,0);DWORD begin=GetTickCount();do{pump(&s);}while(!s.ai_exit_error_ack&&GetTickCount()-begin<5000);
  CHECK(s.ai_exit_error_ack&&!s.exiting&&s.ai&&s.ai->store_failed);
 }
 CHECK(close_host(&s));
 if(success)closed_proof(db,2,2,1);
 if(!wcscmp(mode,L"quota"))closed_proof(db,3,3,0);
 if(!wcscmp(mode,L"storage"))closed_proof(db,0,0,0);
 if(!wcscmp(mode,L"ledger-corrupt")){
 char path[4096];CHECK(WideCharToMultiByte(CP_UTF8,0,db,-1,path,sizeof(path),NULL,NULL)>0);sqlite3 *sql=NULL;CHECK(sqlite3_open_v2(path,&sql,SQLITE_OPEN_READWRITE,NULL)==SQLITE_OK);CHECK(sqlite3_exec(sql,"UPDATE events SET payload='{}' WHERE kind='rpc_reserved'",NULL,NULL,NULL)==SQLITE_OK);sqlite3_close(sql);
 }
 if(success){
  SetEnvironmentVariableW(L"UI_AI_BR2_POLICY",NULL);memset(&s,0,sizeof(s));
  CHECK(create_host(&s,GetModuleHandleW(NULL))!=NULL);CHECK(model_wait(&s,NULL));ShowWindow(s.hwnd,SW_HIDE);
  if(!wcscmp(mode,L"ledger-corrupt")){
   DWORD begin=GetTickCount();ui_ai_service_state_t state={0};state.size=sizeof(state);state.version=1;
   do{pump(&s);CHECK(ui_ai_service_get_state(s.ai,&state)==UI_STATUS_OK);}while(state.phase!=UI_AI_UNAVAILABLE&&GetTickCount()-begin<5000);
   CHECK(state.phase==UI_AI_UNAVAILABLE&&strstr(state.error,"business_guard_restore"));
   PostMessageW(s.hwnd,WM_CLOSE,0,0);begin=GetTickCount();
   do{pump(&s);}while(!s.ai_exit_error_ack&&GetTickCount()-begin<5000);
   CHECK(s.ai_exit_error_ack&&s.ai&&s.ai->store_failed);
  }else{
   CHECK(ui_ai_business_validation_enable(s.ai,policy)==UI_STATUS_ALREADY_EXISTS);
   CHECK(guard_state(&s,"validation_restart_review_required"));
  }
  CHECK(s.ai->model_config->requests==0);CHECK(close_host(&s));
 }
 SetEnvironmentVariableW(L"UI_AI_BR2_POLICY",NULL);SetEnvironmentVariableW(L"UI_AI_DB_PATH",NULL);SetEnvironmentVariableW(L"UI_BROWSER_SETTINGS_PATH",NULL);printf("BR2 %ls checks=%u failures=%u\n",mode,checks,failures);return failures?1:0;
}
