/* P6R-1 isolated processes/DBs, same real DLL and P4 helpers. */
#define wmain p4_regression_main
#include "ai_edit.c"
#undef wmain
#include "ai_internal.h"
#include "ai_graph_internal.h"
#include <psapi.h>
#include <tlhelp32.h>
#include <objbase.h>
__declspec(dllimport) ui_status_t ui_ai_graph_fixture_action(ui_ai_service_t *,int,const char *,const char *,const char *);
static FILE *samples;static int physical_positive=0;
static FILE *file_open(const char *path,const wchar_t *mode)
{wchar_t wide[4096];return MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,path,-1,wide,4096)?_wfopen(wide,mode):NULL;}
static uint64_t file_bytes(const char *path)
{WIN32_FILE_ATTRIBUTE_DATA f;wchar_t wide[4096];if(!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,path,-1,wide,4096)||!GetFileAttributesExW(wide,GetFileExInfoStandard,&f))return 0;return ((uint64_t)f.nFileSizeHigh<<32)|f.nFileSizeLow;}
static void probe(const char *path,ui_ai_service_phase_t expected)
{
 ui_workspace_config_t wc={0};ui_ai_service_config_t c={0};ui_ai_service_state_t st={0};ui_workspace_t *w;ui_ai_service_t *p=NULL;DWORD start;
 wc.size=sizeof(wc);wc.run_mode=UI_RUN_OFFSCREEN;w=ui_workspace_create(&wc);CHECK(w!=NULL);c.size=sizeof(c);c.version=1;c.workspace=w;c.storage_path_utf8=path;CHECK(ui_ai_service_create(&c,&p)==UI_STATUS_OK);if(!p)return;CHECK(ui_ai_service_open(p)==UI_STATUS_OK);st.size=sizeof(st);st.version=1;start=GetTickCount();do{ui_ai_service_poll(p,64);ui_ai_service_get_state(p,&st);if((st.phase==UI_AI_READY||st.phase==UI_AI_UNAVAILABLE)&&!st.worker_alive)break;(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}while(GetTickCount()-start<5000);CHECK(st.phase==expected&&!st.active_run_id[0]&&!st.queued);printf("P6 reopen phase=%d expected=%d error=%s\n",st.phase,expected,st.error);
 CHECK(ui_ai_service_request_close(p)==UI_STATUS_OK);start=GetTickCount();do{ui_ai_service_poll(p,64);ui_ai_service_get_state(p,&st);if(st.phase==UI_AI_CLOSED&&!st.worker_alive)break;(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}while(GetTickCount()-start<5000);CHECK(st.phase==UI_AI_CLOSED);CHECK(ui_ai_service_destroy(p)==UI_STATUS_OK);CHECK(ui_workspace_destroy(w)==UI_STATUS_OK);
}
static void query_exec(const char *sql_text)
{sqlite3 *db=NULL;char *error=NULL;CHECK(sqlite3_open(dbpath,&db)==SQLITE_OK);int rc=sqlite3_exec(db,sql_text,NULL,NULL,&error);if(rc)fprintf(stderr,"P6 SQL rc=%d %s\n",rc,error?error:"");CHECK(rc==SQLITE_OK);sqlite3_free(error);CHECK(sqlite3_close(db)==SQLITE_OK);}
static void audit_case(int fault)
{
 char original[4096];setup(test_package,test_directory,L"audit");ui_ai_task_request_t q={0};q.size=sizeof(q);q.version=1;q.client_request_id="audit-run";q.input_utf8="Stub audit";q.stub_plan_json="{\"steps\":[]}";CHECK(ui_ai_submit(service,&q,id)==UI_STATUS_OK);
 {DWORD begin=GetTickCount();char *json=NULL;do{pump();ui_ai_get_run(service,id,&json);int done=json&&strstr(json,"\"terminal\":true");ui_ai_release(json);json=NULL;if(done)break;}while(GetTickCount()-begin<5000);}strcpy_s(original,sizeof(original),dbpath);close_service();
 const char *faults[]={"INSERT INTO conversation SELECT id FROM conversation", "UPDATE events SET run_id='missing' WHERE terminal=1", "DELETE FROM events WHERE terminal=1", "INSERT INTO messages(op_id,run_id,role,content) VALUES('orphan','missing','user','x')", "INSERT INTO ai_edit_consumptions VALUES('orphan','missing','binding','scope')", "PRAGMA user_version=99", "DROP INDEX one_terminal"};
 CHECK(fault>=0&&fault<8);if(fault==7){FILE *bad=file_open(original,L"r+b");CHECK(bad!=NULL);if(bad){CHECK(fwrite("damaged header!!",1,16,bad)==16);fclose(bad);}}else query_exec(faults[fault]);uint64_t bytes=file_bytes(original);FILE *f=file_open(original,L"rb");unsigned char *before=bytes<=16u*1024u*1024u?malloc((size_t)bytes):NULL;CHECK(f&&before);if(f&&before)CHECK(fread(before,1,(size_t)bytes,f)==bytes);if(f)fclose(f);
 probe(original,UI_AI_UNAVAILABLE);CHECK(file_bytes(original)==bytes);f=file_open(original,L"rb");unsigned char *after=before?malloc((size_t)bytes):NULL;CHECK(f&&after);if(f&&after){CHECK(fread(after,1,(size_t)bytes,f)==bytes);CHECK(!memcmp(before,after,(size_t)bytes));}if(f)fclose(f);free(before);free(after);
}
static int wait_run(const char *run)
{DWORD begin=GetTickCount();do{char *json=NULL;CHECK(ui_ai_service_poll(service,64)==UI_STATUS_OK);ui_workspace_poll(workspace);CHECK(ui_ai_get_run(service,run,&json)==UI_STATUS_OK);int terminal=json&&strstr(json,"\"terminal\":true")!=NULL;ui_ai_release(json);if(terminal)return 1;(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}while(GetTickCount()-begin<5000);return 0;}
static void graph_start(void)
{
 char *json=NULL,hash[65];DWORD start=GetTickCount();CHECK(ui_ai_coordinator_enable(service)==UI_STATUS_OK);do{pump();ui_ai_release(json);json=NULL;CHECK(ui_ai_coordinator_get_state(service,&json)==UI_STATUS_OK);if(json&&number(json,"schema")==2&&!strstr(json,"\"pending\":true"))break;}while(GetTickCount()-start<5000);field(json?json:"{}","binding_hash",hash,sizeof(hash));ui_ai_release(json);CHECK(hash[0]);CHECK(ui_ai_graph_fixture_action(service,UI_AI_GRAPH_MIGRATE,"","",hash)==UI_STATUS_OK);
 start=GetTickCount();do{pump();json=NULL;ui_ai_coordinator_get_state(service,&json);int ready=json&&number(json,"schema")==3;ui_ai_release(json);if(ready)break;}while(GetTickCount()-start<5000);
 ui_ai_graph_request_t q={0};ui_cap_target_v2_t t={0};target(&t);q.size=sizeof(q);q.version=1;q.client_request_id="crash-graph";q.input_utf8="Explicit stub graph";q.targets=&t;q.target_count=1;q.graph_json="{\"nodes\":[{\"id\":\"n\",\"operation_id\":\"op\",\"capability\":\"memory.read\",\"target\":0,\"params\":{},\"constraint\":{\"type\":\"object\",\"properties\":{},\"additionalProperties\":false}}]}";CHECK(ui_ai_graph_submit(service,&q,id)==UI_STATUS_OK);
}
static void child_window(int point)
{
 setup(test_package,test_directory,L"child");ui_ai_edit_fixture(service,6,(uint64_t)point);
 if(point<=2||point>=18){ui_ai_task_request_t q={0};q.size=sizeof(q);q.version=1;q.client_request_id="interruption";q.input_utf8="Stub controlled window";q.stub_plan_json="{\"steps\":[]}";CHECK(ui_ai_submit(service,&q,id)==UI_STATUS_OK);if(point>=18){CHECK(wait_run(id));CHECK(ui_ai_service_request_close(service)==UI_STATUS_OK);}}
 else if(point>=14)graph_start();
 else {if(point==6)control(AE_MODE_ASYNC);propose("interruption-edit","memory.set",69);CHECK(value(app.host)==10);CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);}
 DWORD start=GetTickCount();while(GetTickCount()-start<7000)pump();fprintf(stderr,"P6 window not reached %d state=%.900s\n",point,state);ExitProcess(2);
}
static void window_case(int point)
{
 wchar_t executable[4096],command[14000],path[4096];STARTUPINFOW si={0};PROCESS_INFORMATION pi={0};DWORD code=99,start;char expected_path[4096];
 CHECK(_snwprintf_s(path,4096,_TRUNCATE,L"%s/window-%d-%lu.sqlite",test_directory,point,GetCurrentProcessId())>=0);CHECK(WideCharToMultiByte(CP_UTF8,0,path,-1,expected_path,sizeof(expected_path),NULL,NULL)>0);CHECK(GetModuleFileNameW(NULL,executable,4096)>0);CHECK(_snwprintf_s(command,14000,_TRUNCATE,L"\"%s\" \"%s\" \"%s\" child-%d \"%s\"",executable,test_package,test_directory,point,path)>=0);si.cb=sizeof(si);CHECK(CreateProcessW(executable,command,NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&si,&pi));if(!pi.hProcess)return;start=GetTickCount();while(WaitForSingleObject(pi.hProcess,0)!=WAIT_OBJECT_0&&GetTickCount()-start<15000)(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);CHECK(GetExitCodeProcess(pi.hProcess,&code)&&code==86);CloseHandle(pi.hProcess);CloseHandle(pi.hThread);strcpy_s(dbpath,sizeof(dbpath),expected_path);
 probe(dbpath,UI_AI_READY);int events=sql("SELECT count(*) FROM events"),term=sql("SELECT count(*) FROM events WHERE terminal=1");probe(dbpath,UI_AI_READY);CHECK(sql("SELECT count(*) FROM events")==events);CHECK(sql("SELECT count(*) FROM events WHERE terminal=1")==term);CHECK(sql("SELECT count(*) FROM runs WHERE terminal=0 AND phase<>8")==0);CHECK(sql("SELECT count(*) FROM (SELECT run_id FROM events WHERE terminal=1 GROUP BY run_id HAVING count(*)>1)")==0);
 if(point>=3&&point<=13){CHECK(sql("SELECT count(*) FROM ai_edits")==1);CHECK(sql("SELECT count(*) FROM ai_edit_consumptions")==((point==3)?0:1));CHECK(term==((point==12||point==13)?1:0));}
 if(point>=14&&point<=17){CHECK(sql("SELECT count(*) FROM ai_graphs")==1);CHECK(term==(point==17?1:0));}
 printf("P6 window=%d exit=%lu events=%d terminal=%d no automatic execution\n",point,code,events,term);
}
static void graph_audit(void)
{setup(test_package,test_directory,L"graph-audit");graph_start();CHECK(wait_run(id));close_service();query_exec("UPDATE ai_graphs SET payload='{}'");probe(dbpath,UI_AI_UNAVAILABLE);}
static void disk_case(int code)
{setup(test_package,test_directory,L"disk");propose("disk-edit","memory.set",69);CHECK(value(app.host)==10);ui_ai_edit_fixture(service,5,(uint64_t)code);CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);CHECK(wait_phase(S_UNKNOWN));CHECK(value(app.host)==10);CHECK(number(state,"effect")==UI_CAP2_EFFECT_UNKNOWN);ui_ai_edit_fixture(service,5,0);close_service();CHECK(sql("SELECT count(*) FROM ai_edit_consumptions")==0);probe(dbpath,UI_AI_READY);}
static void restart_guard(void)
{
 setup(test_package,test_directory,L"unknown");control(AE_MODE_BAD_RECEIPT);propose("unknown-edit","memory.set",69);CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);CHECK(wait_phase(S_UNKNOWN));CHECK(value(app.host)==69);char original[4096];strcpy_s(original,sizeof(original),dbpath);close_service();CHECK(sql("SELECT count(*) FROM ai_edits WHERE unknown=1")==1);
 for(unsigned i=0;i<2;++i){strcpy_s(override_path,sizeof(override_path),original);setup(test_package,test_directory,L"unknown-reopen");CHECK(number(state,"recovered_unresolved")==1);CHECK(value(app.host)==10);ui_ai_edit_request_t q={0};q.size=sizeof(q);q.version=1;q.client_request_id="new-after-unknown";q.capability_id="memory.set";target(&q.target);q.params_json="{\"value\":77}";q.parameter_constraint_schema=constraint;char rejected[65];CHECK(ui_ai_edit_propose(service,&q,rejected)==UI_STATUS_PENDING);CHECK(!rejected[0]);CHECK(ui_ai_edit_cancel(service,"unknown-old")==UI_STATUS_NOT_FOUND);CHECK(ui_ai_edit_forget(service,"unknown-old")==UI_STATUS_NOT_FOUND);CHECK(value(app.host)==10);close_service();CHECK(sql("SELECT count(*) FROM ai_edits WHERE unknown=1")==1);CHECK(sql("SELECT count(*) FROM ai_edit_consumptions")==1);}
}
static void consumption_audit(void)
{setup(test_package,test_directory,L"consumption-audit");propose("consumption","memory.set",69);CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);CHECK(wait_phase(S_DONE));CHECK(value(app.host)==69);close_service();CHECK(sql("SELECT count(*) FROM ai_edit_consumptions")==1);query_exec("UPDATE ai_edit_consumptions SET binding='changed'");probe(dbpath,UI_AI_UNAVAILABLE);}
static void close_sync_fault(void)
{setup(test_package,test_directory,L"close-sync");propose("before-close","memory.set",69);CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);CHECK(wait_phase(S_DONE));CHECK(value(app.host)==69);ui_ai_edit_fixture(service,5,SQLITE_IOERR_FSYNC);close_service();CHECK(sql("SELECT count(*) FROM events WHERE terminal=1")==1);probe(dbpath,UI_AI_READY);}
static void passive_fault(void)
{
 setup(test_package,test_directory,L"passive-fault");ui_ai_edit_fixture(service,7,SQLITE_IOERR_WRITE);ui_ai_service_state_t st={0};st.size=sizeof(st);st.version=1;
 for(unsigned i=0;i<32;++i){char client[65];ui_ai_task_request_t q={0};snprintf(client,65,"checkpoint-%u",i);q.size=sizeof(q);q.version=1;q.client_request_id=client;q.input_utf8="Stub checkpoint boundary";q.stub_plan_json="{\"steps\":[]}";CHECK(ui_ai_submit(service,&q,id)==UI_STATUS_OK);DWORD start=GetTickCount();do{char *run=NULL;pump();CHECK(ui_ai_service_get_state(service,&st)==UI_STATUS_OK);CHECK(ui_ai_get_run(service,id,&run)==UI_STATUS_OK);int terminal=run&&strstr(run,"\"terminal\":true");ui_ai_release(run);if(terminal||st.phase==UI_AI_UNAVAILABLE)break;}while(GetTickCount()-start<5000);if(st.phase==UI_AI_UNAVAILABLE)break;}
 CHECK(st.phase==UI_AI_UNAVAILABLE);CHECK(strstr(st.error,"passive_checkpoint"));CHECK(value(app.host)==10);ui_ai_task_request_t q={0};q.size=sizeof(q);q.version=1;q.client_request_id="after-checkpoint-error";q.input_utf8="Must reject";q.stub_plan_json="{\"steps\":[]}";char rejected[65];CHECK(ui_ai_submit(service,&q,rejected)!=UI_STATUS_OK&&!rejected[0]);ui_ai_edit_fixture(service,5,0);close_service();CHECK(sql("SELECT count(*) FROM events WHERE terminal=1")>0);CHECK(sql("SELECT count(*) FROM (SELECT run_id FROM events WHERE terminal=1 GROUP BY run_id HAVING count(*)>1)")==0);probe(dbpath,UI_AI_READY);
}
static void lock_case(void)
{setup(test_package,test_directory,L"locked");close_service();sqlite3 *db=NULL;CHECK(sqlite3_open(dbpath,&db)==SQLITE_OK);CHECK(sqlite3_exec(db,"PRAGMA locking_mode=EXCLUSIVE;BEGIN EXCLUSIVE;UPDATE conversation SET id=id",NULL,NULL,NULL)==SQLITE_OK);probe(dbpath,UI_AI_UNAVAILABLE);CHECK(sqlite3_exec(db,"ROLLBACK",NULL,NULL,NULL)==SQLITE_OK);CHECK(sqlite3_close(db)==SQLITE_OK);probe(dbpath,UI_AI_READY);}
static SIZE_T memory_now(void)
{PROCESS_MEMORY_COUNTERS_EX m={0};m.cb=sizeof(m);CHECK(GetProcessMemoryInfo(GetCurrentProcess(),(PROCESS_MEMORY_COUNTERS *)&m,sizeof(m)));return m.PrivateUsage;}
static DWORD threads_now(void)
{DWORD count=0;HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);THREADENTRY32 e={0};e.dwSize=sizeof(e);if(snap!=INVALID_HANDLE_VALUE){if(Thread32First(snap,&e))do{if(e.th32OwnerProcessID==GetCurrentProcessId())++count;}while(Thread32Next(snap,&e));CloseHandle(snap);}return count;}
static void sample(unsigned iteration,ui_ai_service_state_t *st)
{DWORD handles=0;CHECK(GetProcessHandleCount(GetCurrentProcess(),&handles));char wal[4101];snprintf(wal,sizeof(wal),"%s-wal",dbpath);if(samples){fprintf(samples,"%u,%llu,%zu,%zu,%u,%u,%lu,%lu,%zu,%llu,%llu\n",iteration,(unsigned long long)GetTickCount64(),st?st->bytes:0,st?st->event_bytes:0,st?st->queued:0,st?st->worker_alive:0,handles,threads_now(),memory_now(),(unsigned long long)file_bytes(dbpath),(unsigned long long)file_bytes(wal));fflush(samples);}}
static void cycles(void)
{
 DWORD base_handles=0,base_threads=0;SIZE_T base_bytes=0;for(unsigned i=0;i<60;++i){wchar_t name[64];_snwprintf_s(name,64,_TRUNCATE,L"cycle-%u",i);override_path[0]=0;setup(test_package,test_directory,name);CHECK(value(app.host)==10);close_service();sample(i,NULL);DWORD handles=0;CHECK(GetProcessHandleCount(GetCurrentProcess(),&handles));if(i==15){base_handles=handles;base_threads=threads_now();base_bytes=memory_now();}if(i>15){CHECK(handles<=base_handles+8);CHECK(threads_now()<=base_threads+2);
#if !defined(__SANITIZE_ADDRESS__) && !defined(UI_P6_ASAN)
 CHECK(memory_now()<=base_bytes+32u*1024u*1024u);
#else
 (void)base_bytes;
#endif
 }CHECK(GetModuleHandleW(L"ui_ai_edit_fixture.dll")==NULL);}
}
static void sustained(void)
{
 setup(test_package,test_directory,L"sustained");ULONGLONG start=GetTickCount64();unsigned done=0;DWORD base_handles=0,base_threads=0;SIZE_T baseline=0;ui_ai_service_state_t st={0};st.size=sizeof(st);st.version=1;
 do{char client[65];ui_ai_task_request_t q={0};snprintf(client,65,"sustain-%u",done);q.size=sizeof(q);q.version=1;q.client_request_id=client;q.input_utf8="Stub resource stability";q.stub_plan_json="{\"steps\":[{\"type\":\"text\",\"text\":\"complete\"}]}";CHECK(ui_ai_submit(service,&q,id)==UI_STATUS_OK);CHECK(wait_run(id));CHECK(ui_ai_service_get_state(service,&st)==UI_STATUS_OK);CHECK(st.bytes<=UI_AI_MAX_BYTES&&st.event_bytes<=UI_AI_EVENT_BYTES&&st.queued==0);++done;
  if(done%16==0){sample(done,&st);DWORD handles=0;CHECK(GetProcessHandleCount(GetCurrentProcess(),&handles));if(done==16){base_handles=handles;base_threads=threads_now();baseline=memory_now();}else{CHECK(handles<=base_handles+8);CHECK(threads_now()<=base_threads+2);
#if !defined(__SANITIZE_ADDRESS__) && !defined(UI_P6_ASAN)
 CHECK(memory_now()<=baseline+32u*1024u*1024u);
#else
 (void)baseline;
#endif
 }}
 }while(GetTickCount64()-start<600000||done<256);
 unsigned checkpoints=service->store_checkpoints;CHECK(checkpoints>0);CHECK(value(app.host)==10);close_service();CHECK(sql("SELECT count(*) FROM runs")== (int)done);CHECK(sql("SELECT count(*) FROM events WHERE terminal=1")== (int)done);probe(dbpath,UI_AI_READY);printf("P6 sustained tasks=%u elapsed_ms=%llu passive_checkpoints=%u\n",done,(unsigned long long)(GetTickCount64()-start),checkpoints);
}
int wmain(int argc,wchar_t **argv)
{
 if(argc!=4&&argc!=5)return 2;test_package=argv[1];test_directory=argv[2];CHECK(CreateDirectoryW(test_directory,NULL)||GetLastError()==ERROR_ALREADY_EXISTS);CHECK(ui_framework_initialize()==UI_STATUS_OK);if(argc==5)CHECK(WideCharToMultiByte(CP_UTF8,0,argv[4],-1,override_path,sizeof(override_path),NULL,NULL)>0);
 wchar_t report[4096];_snwprintf_s(report,4096,_TRUNCATE,L"%s/resources-%s-%lu.csv",test_directory,argv[3],GetCurrentProcessId());samples=_wfopen(report,L"w");if(samples)fputs("iteration,tick_ms,service_bytes,event_bytes,queued,worker,handles,threads,private_bytes,db_bytes,wal_bytes\n",samples);
 if(!wcsncmp(argv[3],L"child-",6))child_window(_wtoi(argv[3]+6));else if(!wcsncmp(argv[3],L"window-",7))window_case(_wtoi(argv[3]+7));else if(!wcsncmp(argv[3],L"audit-",6))audit_case(_wtoi(argv[3]+6));else if(!wcscmp(argv[3],L"io-write"))disk_case(SQLITE_IOERR_WRITE);else if(!wcscmp(argv[3],L"disk-full"))disk_case(SQLITE_FULL);else if(!wcscmp(argv[3],L"io-sync"))disk_case(SQLITE_IOERR_FSYNC);else if(!wcscmp(argv[3],L"consumption-audit"))consumption_audit();else if(!wcscmp(argv[3],L"restart-unknown"))restart_guard();else if(!wcscmp(argv[3],L"close-sync"))close_sync_fault();else if(!wcscmp(argv[3],L"passive-fault"))passive_fault();else if(!wcscmp(argv[3],L"lock"))lock_case();else if(!wcscmp(argv[3],L"graph-audit"))graph_audit();else if(!wcscmp(argv[3],L"cycles"))cycles();else if(!wcscmp(argv[3],L"sustained"))sustained();else CHECK(0);
 if(samples)fclose(samples);CHECK(!physical_positive);printf("P6 checks=%u failures=%u; no physical source claim\n",checks,failures);return failures?1:0;
}
