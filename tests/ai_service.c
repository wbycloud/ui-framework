#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui_framework/ai_service.h"
#include "sqlite3.h"
void ui_ai_fixture(ui_ai_service_t *,int,uint64_t);
static unsigned checks,failures;static ui_ai_service_t *service;static char received[65],last[65],history[524289],query[524289];static unsigned finishes,errors,receipts;static int early,cancel_receipt;
#define CHECK(x) do{++checks;if(!(x)){++failures;fprintf(stderr,"line %d: %s\n",__LINE__,#x);}}while(0)
static void event(const ui_ai_event_t *e,void *data)
{(void)data;CHECK(e->size==sizeof(*e)&&e->version==1);if(!strcmp(e->type,"received")){char *json=NULL;++receipts;CHECK(!strcmp(e->run_id,received));CHECK(ui_ai_get_run(service,e->run_id,&json)==UI_STATUS_OK);ui_ai_release(json);early=1;if(cancel_receipt){CHECK(ui_ai_service_destroy(service)==UI_STATUS_PENDING);CHECK(ui_ai_service_poll(service,1)==UI_STATUS_PENDING);CHECK(ui_ai_cancel(service,e->run_id)==UI_STATUS_OK);}}
 if(!strcmp(e->type,"finished")){++finishes;strcpy_s(last,sizeof(last),e->run_id);CHECK(e->committed&&e->sequence>0);}if(!strcmp(e->type,"storage_error"))++errors;
 if(!strcmp(e->type,"history")){CHECK(strlen(e->payload)<sizeof(history));strcpy_s(history,sizeof(history),e->payload);}if(!strcmp(e->type,"query"))strcpy_s(query,sizeof(query),e->payload);
 if(!strcmp(e->type,"delta"))CHECK(!strlen(e->payload)||MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,e->payload,(int)strlen(e->payload),NULL,0)>0);}
static void pump(void){MSG m;unsigned n=0;while(n++<100&&PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}CHECK(ui_ai_service_poll(service,64)==UI_STATUS_OK);(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}
static int wait_phase(ui_ai_service_phase_t phase)
{DWORD start=GetTickCount();ui_ai_service_state_t state={0};state.size=sizeof(state);state.version=1;do{pump();CHECK(ui_ai_service_get_state(service,&state)==UI_STATUS_OK);if(state.phase==phase)return 1;}while(GetTickCount()-start<5000);return 0;}
static int wait_run(const char *id,const char *condition)
{DWORD start=GetTickCount();do{char *json=NULL;pump();if(ui_ai_get_run(service,id,&json)==UI_STATUS_OK){int found=strstr(json,condition)!=NULL;ui_ai_release(json);if(found)return 1;}}while(GetTickCount()-start<5000);return 0;}
static void close_service(ui_workspace_t *w)
{CHECK(ui_ai_service_request_close(service)==UI_STATUS_OK);CHECK(wait_phase(UI_AI_CLOSED));{DWORD start=GetTickCount();ui_status_t status;do{pump();status=ui_ai_service_destroy(service);if(status==UI_STATUS_OK)break;CHECK(status==UI_STATUS_PENDING);}while(GetTickCount()-start<5000);CHECK(status==UI_STATUS_OK);}service=NULL;CHECK(ui_workspace_destroy(w)==UI_STATUS_OK);}
static ui_workspace_t *open_service(const char *path)
{ui_workspace_config_t w={0};ui_ai_service_config_t c={0};ui_workspace_t *workspace;w.size=sizeof(w);w.run_mode=UI_RUN_OFFSCREEN;workspace=ui_workspace_create(&w);CHECK(workspace!=NULL);c.size=sizeof(c);c.version=1;c.workspace=workspace;c.storage_path_utf8=path;c.on_event=event;CHECK(ui_ai_service_create(&c,&service)==UI_STATUS_OK);CHECK(ui_ai_service_open(service)==UI_STATUS_OK);return workspace;}
static void make_path(const wchar_t *directory,const wchar_t *name,wchar_t wide[4096],char utf8[4096])
{CHECK(_snwprintf_s(wide,4096,_TRUNCATE,L"%s/%s-%lu.db",directory,name,GetCurrentProcessId())>=0);CHECK(WideCharToMultiByte(CP_UTF8,0,wide,-1,utf8,4096,NULL,NULL)>0);}
static int sql_count(const char *path,const char *sql)
{sqlite3 *db=NULL;sqlite3_stmt *q=NULL;int value=-1;CHECK(sqlite3_open(path,&db)==SQLITE_OK);CHECK(sqlite3_prepare_v2(db,sql,-1,&q,NULL)==SQLITE_OK);if(q&&sqlite3_step(q)==SQLITE_ROW)value=sqlite3_column_int(q,0);sqlite3_finalize(q);CHECK(sqlite3_close(db)==SQLITE_OK);return value;}
static void control_case(const wchar_t *directory)
{
 wchar_t wide[4096];char path[4096],ids[17][65];ui_workspace_t *w;ui_ai_task_request_t q={0};ui_ai_service_state_t state={0};unsigned before;
 make_path(directory,L"control",wide,path);w=open_service(path);CHECK(wait_phase(UI_AI_READY));ui_ai_fixture(service,2,1);
 q.size=sizeof(q);q.version=1;q.input_utf8="queued";q.stub_plan_json="{\"steps\":[]}";q.client_request_id="held";CHECK(ui_ai_submit(service,&q,received)==UI_STATUS_OK);strcpy_s(ids[0],65,received);
 state.size=sizeof(state);state.version=1;{DWORD start=GetTickCount();do{pump();CHECK(ui_ai_service_get_state(service,&state)==UI_STATUS_OK);if(state.active_run_id[0])break;}while(GetTickCount()-start<5000);CHECK(!strcmp(state.active_run_id,ids[0]));}
 for(unsigned i=1;i<=16;++i){char id[32];snprintf(id,sizeof(id),"queued-%u",i);q.client_request_id=id;CHECK(ui_ai_submit(service,&q,received)==UI_STATUS_OK);strcpy_s(ids[i],65,received);}
 q.client_request_id="overflow";CHECK(ui_ai_submit(service,&q,received)==UI_STATUS_LIMIT_EXCEEDED&&received[0]==0);CHECK(!strcmp(q.input_utf8,"queued"));
 {int limited=0;for(unsigned i=0;i<1024;++i){char id[32];ui_status_t st;snprintf(id,sizeof(id),"pressure-%u",i);st=ui_ai_history(service,id,UI_AI_MESSAGES,0,32);if(st==UI_STATUS_LIMIT_EXCEEDED){limited=1;break;}CHECK(st==UI_STATUS_OK);}CHECK(limited);}
 before=finishes;CHECK(ui_ai_cancel(service,ids[0])==UI_STATUS_OK);CHECK(ui_ai_cancel(service,ids[16])==UI_STATUS_OK);ui_ai_fixture(service,3,5001);CHECK(wait_run(ids[0],"\"phase\":6"));CHECK(finishes==before);CHECK(wait_run(ids[0],"\"terminal\":false"));
 ui_ai_fixture(service,2,0);CHECK(wait_run(ids[16],"\"terminal\":true"));CHECK(wait_run(ids[0],"\"outcome\":\"cancelled\""));CHECK(wait_run(ids[16],"\"model_calls\":0"));
 CHECK(ui_ai_service_quiesce(service)==UI_STATUS_OK);CHECK(ui_ai_service_resume(service)==UI_STATUS_OK);q.client_request_id="after-resume";CHECK(ui_ai_submit(service,&q,received)==UI_STATUS_OK);CHECK(wait_run(received,"\"outcome\":\"completed\""));close_service(w);
 CHECK(sql_count(path,"SELECT count(*) FROM events WHERE terminal=1")==18);CHECK(sql_count(path,"SELECT count(*) FROM (SELECT run_id,count(*) c FROM events WHERE terminal=1 GROUP BY run_id HAVING c<>1)")==0);
 make_path(directory,L"question",wide,path);w=open_service(path);CHECK(wait_phase(UI_AI_READY));q.client_request_id="question";q.stub_plan_json="{\"steps\":[{\"type\":\"ask\",\"text\":\"explicit question\"},{\"type\":\"text\",\"text\":\" done\"}]}";CHECK(ui_ai_submit(service,&q,received)==UI_STATUS_OK);strcpy_s(ids[0],65,received);CHECK(wait_run(ids[0],"\"phase\":3"));
 ui_ai_fixture(service,2,1);CHECK(ui_ai_answer(service,ids[0],"wrong","answer","yes")==UI_STATUS_NOT_FOUND);CHECK(ui_ai_answer(service,ids[0],"question-0","answer","yes")==UI_STATUS_OK);CHECK(ui_ai_answer(service,ids[0],"question-0","answer","yes")==UI_STATUS_OK);CHECK(ui_ai_answer(service,ids[0],"question-0","answer","different")==UI_STATUS_ALREADY_EXISTS);ui_ai_fixture(service,2,0);
 CHECK(wait_run(ids[0],"yes done"));CHECK(wait_run(ids[0],"\"terminal\":true"));close_service(w);CHECK(sql_count(path,"SELECT count(*) FROM messages WHERE role='user'")==2);
}
static void limits_case(const wchar_t *directory)
{
 wchar_t wide[4096];char path[4096];ui_workspace_t *w;ui_ai_task_request_t q={0};ui_ai_service_state_t state={0};char *plan=malloc(61000);unsigned before;CHECK(plan!=NULL);if(!plan)return;
 make_path(directory,L"limits",wide,path);w=open_service(path);CHECK(wait_phase(UI_AI_READY));q.size=sizeof(q);q.version=1;q.client_request_id="cancel-receipt";q.input_utf8="never run after early cancel";q.stub_plan_json="{\"steps\":[]}";cancel_receipt=1;CHECK(ui_ai_submit(service,&q,received)==UI_STATUS_OK);cancel_receipt=0;CHECK(wait_run(received,"\"outcome\":\"cancelled\""));CHECK(wait_run(received,"\"model_calls\":0"));
 q.client_request_id="duplicate-key";q.stub_plan_json="{\"steps\":[],\"steps\":[]}";CHECK(ui_ai_submit(service,&q,received)==UI_STATUS_VALIDATION_FAILED);q.stub_plan_json="{\"steps\":[{\"type\":\"text\",\"text\":\"\\u0000\"}]}";CHECK(ui_ai_submit(service,&q,received)==UI_STATUS_VALIDATION_FAILED);
 q.client_request_id="disconnect";q.stub_plan_json="{\"steps\":[{\"type\":\"fail\"}]}";CHECK(ui_ai_submit(service,&q,received)==UI_STATUS_OK);CHECK(wait_run(received,"stub_disconnect"));CHECK(wait_run(received,"\"terminal\":true"));
 strcpy_s(plan,61000,"{\"steps\":[{\"type\":\"text\",\"text\":\"");{size_t n=strlen(plan);memset(plan+n,'x',50000);strcpy_s(plan+n+50000,61000-n-50000,"\"}]}");}q.client_request_id="stream-pressure";q.stub_plan_json=plan;before=finishes;CHECK(ui_ai_submit(service,&q,received)==UI_STATUS_OK);
 /* Deliberately do not poll notices: a complete authoritative result remains
    queryable while the full display queue backpressures final delivery. */
 {DWORD start=GetTickCount();int terminal=0;do{char *json=NULL;CHECK(ui_ai_get_run(service,received,&json)==UI_STATUS_OK);terminal=json&&strstr(json,"\"terminal\":true")!=NULL;ui_ai_release(json);(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}while(!terminal&&GetTickCount()-start<5000);CHECK(terminal);}
 state.size=sizeof(state);state.version=1;CHECK(ui_ai_service_get_state(service,&state)==UI_STATUS_OK&&state.event_bytes>0&&state.event_bytes<=UI_AI_EVENT_BYTES&&state.bytes<=UI_AI_MAX_BYTES);CHECK(finishes==before);CHECK(wait_run(received,"\"committed\":true"));{DWORD start=GetTickCount();while(finishes==before&&GetTickCount()-start<5000)pump();CHECK(finishes==before+1);}
 history[0]=0;CHECK(ui_ai_history(service,"one",UI_AI_MESSAGES,0,1)==UI_STATUS_OK);{DWORD start=GetTickCount();while(!strstr(history,"\"has_more\":true")&&GetTickCount()-start<5000)pump();CHECK(strstr(history,"\"has_more\":true")!=NULL);}CHECK(ui_ai_history(service,"bad",UI_AI_MESSAGES,0,33)==UI_STATUS_INVALID_ARGUMENT);close_service(w);free(plan);
}
static unsigned char *file_bytes(const wchar_t *path,size_t *length)
{FILE *f=NULL;unsigned char *bytes;long size;CHECK(_wfopen_s(&f,path,L"rb")==0);if(!f)return NULL;CHECK(fseek(f,0,SEEK_END)==0);size=ftell(f);CHECK(size>=0);rewind(f);*length=(size_t)size;bytes=malloc(*length+1);CHECK(bytes!=NULL);if(bytes)CHECK(fread(bytes,1,*length,f)==*length);fclose(f);return bytes;}
static void storage_case(const wchar_t *directory)
{
 wchar_t wide[4096],seed[4096];char path[4096],seed_utf8[4096];ui_workspace_t *w;ui_ai_task_request_t q={0};size_t before_size,after_size;unsigned char *before,*after;sqlite3 *db=NULL;
 make_path(directory,L"fault",wide,path);w=open_service(path);CHECK(wait_phase(UI_AI_READY));ui_ai_fixture(service,1,1);q.size=sizeof(q);q.version=1;q.client_request_id="unconfirmed";q.input_utf8="must not fake persistence";CHECK(ui_ai_submit(service,&q,received)==UI_STATUS_OK);CHECK(wait_phase(UI_AI_UNAVAILABLE));CHECK(wait_run(received,"\"committed\":false"));CHECK(wait_run(received,"\"terminal\":false"));close_service(w);CHECK(sql_count(path,"SELECT count(*) FROM runs")==0);CHECK(sql_count(path,"SELECT count(*) FROM messages")==0);
 make_path(directory,L"future",wide,path);CHECK(sqlite3_open(path,&db)==SQLITE_OK);CHECK(sqlite3_exec(db,"PRAGMA application_id=1481855027;PRAGMA user_version=99;CREATE TABLE future(data TEXT);INSERT INTO future VALUES('keep')",NULL,NULL,NULL)==SQLITE_OK);CHECK(sqlite3_close(db)==SQLITE_OK);before=file_bytes(wide,&before_size);w=open_service(path);CHECK(wait_phase(UI_AI_UNAVAILABLE));close_service(w);after=file_bytes(wide,&after_size);CHECK(before_size==after_size&&!memcmp(before,after,before_size));free(before);free(after);
 make_path(directory,L"corrupt",wide,path);{FILE *f=NULL;CHECK(_wfopen_s(&f,wide,L"wb")==0);if(f){CHECK(fwrite("not a SQLite database",1,21,f)==21);fclose(f);}}before=file_bytes(wide,&before_size);w=open_service(path);CHECK(wait_phase(UI_AI_UNAVAILABLE));close_service(w);after=file_bytes(wide,&after_size);CHECK(before_size==after_size&&!memcmp(before,after,before_size));free(before);free(after);
 make_path(directory,L"locked",wide,path);w=open_service(path);CHECK(wait_phase(UI_AI_READY));close_service(w);CHECK(sqlite3_open(path,&db)==SQLITE_OK);CHECK(sqlite3_exec(db,"BEGIN EXCLUSIVE",NULL,NULL,NULL)==SQLITE_OK);w=open_service(path);CHECK(wait_phase(UI_AI_UNAVAILABLE));close_service(w);CHECK(sqlite3_exec(db,"ROLLBACK",NULL,NULL,NULL)==SQLITE_OK);CHECK(sqlite3_close(db)==SQLITE_OK);
 make_path(directory,L"restart",seed,seed_utf8);{wchar_t executable[4096],command[10000];STARTUPINFOW startup={0};PROCESS_INFORMATION process={0};DWORD start,exit=99;CHECK(GetModuleFileNameW(NULL,executable,4096)>0);CHECK(_snwprintf_s(command,10000,_TRUNCATE,L"\"%s\" --seed \"%s\"",executable,seed)>=0);startup.cb=sizeof(startup);CHECK(CreateProcessW(executable,command,NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&startup,&process));if(process.hProcess){start=GetTickCount();while(WaitForSingleObject(process.hProcess,0)!=WAIT_OBJECT_0&&GetTickCount()-start<5000)(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);CHECK(GetExitCodeProcess(process.hProcess,&exit)&&exit==0);CloseHandle(process.hThread);CloseHandle(process.hProcess);}}
 CHECK(sql_count(seed_utf8,"SELECT count(*) FROM runs WHERE terminal=0")==1);history[0]=0;w=open_service(seed_utf8);CHECK(wait_phase(UI_AI_READY));CHECK(ui_ai_history(service,"recovered",UI_AI_RUNS,0,32)==UI_STATUS_OK);{DWORD start=GetTickCount();while(!strstr(history,"\"phase\":8")&&GetTickCount()-start<5000)pump();CHECK(strstr(history,"\"phase\":8")!=NULL);}
 {ui_ai_service_state_t state={0};state.size=sizeof(state);state.version=1;CHECK(ui_ai_service_get_state(service,&state)==UI_STATUS_OK&&!state.active_run_id[0]&&state.queued==0);}
 {char old[65];const char *p=strstr(history,"\"run_id\":\"");CHECK(p!=NULL);if(p){memcpy(old,p+10,64);old[64]=0;q.client_request_id="explicit-continue";q.input_utf8="continued explicitly";q.stub_plan_json="{\"steps\":[]}";CHECK(ui_ai_continue(service,old,&q,received)==UI_STATUS_OK);CHECK(wait_run(received,"continued explicitly"));CHECK(wait_run(received,"\"terminal\":true"));}}
 close_service(w);CHECK(sql_count(seed_utf8,"SELECT count(*) FROM runs")==2);CHECK(sql_count(seed_utf8,"SELECT count(*) FROM events WHERE terminal=1")==1);
}
int wmain(int argc,wchar_t **argv)
{
 char path[4096];wchar_t database[4096];ui_workspace_t *workspace;ui_ai_task_request_t q={0};unsigned before;ui_ai_service_state_t state={0};
 if(argc==3&&!wcscmp(argv[1],L"--seed")){CHECK(WideCharToMultiByte(CP_UTF8,0,argv[2],-1,path,sizeof(path),NULL,NULL)>0);workspace=open_service(path);CHECK(wait_phase(UI_AI_READY));q.size=sizeof(q);q.version=1;q.client_request_id="seed";q.input_utf8="unfinished";q.stub_plan_json="{\"steps\":[{\"type\":\"ask\",\"text\":\"must explicitly continue\"},{\"type\":\"text\",\"text\":\"must not automatically run\"}]}";CHECK(ui_ai_submit(service,&q,received)==UI_STATUS_OK);CHECK(wait_run(received,"\"phase\":3"));{DWORD start=GetTickCount();state.size=sizeof(state);state.version=1;do{pump();CHECK(ui_ai_service_get_state(service,&state)==UI_STATUS_OK);}while(state.worker_alive&&GetTickCount()-start<5000);CHECK(!state.worker_alive);}ExitProcess(failures?1:0);}
 if(argc==3){CHECK(CreateDirectoryW(argv[2],NULL)||GetLastError()==ERROR_ALREADY_EXISTS);if(!wcscmp(argv[1],L"--control"))control_case(argv[2]);else if(!wcscmp(argv[1],L"--storage"))storage_case(argv[2]);else if(!wcscmp(argv[1],L"--limits"))limits_case(argv[2]);else return 2;printf("P3 %ls checks=%u failures=%u\n",argv[1],checks,failures);return failures?1:0;}
 if(argc!=2)return 2;CHECK(CreateDirectoryW(argv[1],NULL)||GetLastError()==ERROR_ALREADY_EXISTS);CHECK(_snwprintf_s(database,4096,_TRUNCATE,L"%s/basic-%lu.db",argv[1],GetCurrentProcessId())>=0);CHECK(WideCharToMultiByte(CP_UTF8,0,database,-1,path,sizeof(path),NULL,NULL)>0);
 workspace=open_service(path);CHECK(ui_ai_service_quiesce(service)==UI_STATUS_OK);CHECK(ui_ai_service_resume(service)==UI_STATUS_OK);state.size=sizeof(state);state.version=1;CHECK(ui_ai_service_get_state(service,&state)==UI_STATUS_OK);CHECK(state.phase==UI_AI_OPENING||(state.phase==UI_AI_READY&&strlen(state.conversation_id)==36));CHECK(wait_phase(UI_AI_READY));CHECK(ui_ai_service_get_state(service,&state)==UI_STATUS_OK&&strlen(state.conversation_id)==36);
 {ui_ai_service_config_t c={0};ui_ai_service_t *other=NULL;c.size=sizeof(c);c.version=1;c.workspace=workspace;c.storage_path_utf8=path;CHECK(ui_ai_service_create(&c,&other)==UI_STATUS_ALREADY_EXISTS&&!other);CHECK(ui_workspace_destroy(workspace)==UI_STATUS_CANCELLED);}
 q.size=sizeof(q);q.version=1;q.client_request_id="basic";q.input_utf8="中文🙂";q.stub_plan_json="{\"steps\":[]}";early=0;CHECK(ui_ai_submit(service,&q,received)==UI_STATUS_OK&&early);CHECK(wait_run(received,"\"terminal\":true"));CHECK(wait_run(received,"[Stub test model] 中文🙂"));
 before=finishes;{char duplicate[65];CHECK(ui_ai_submit(service,&q,duplicate)==UI_STATUS_OK&&!strcmp(duplicate,received));pump();CHECK(finishes==before);q.input_utf8="different";CHECK(ui_ai_submit(service,&q,duplicate)==UI_STATUS_ALREADY_EXISTS);q.input_utf8="中文🙂";}
 CHECK(ui_ai_history(service,"history",UI_AI_MESSAGES,0,32)==UI_STATUS_OK);{DWORD start=GetTickCount();while(!strstr(history,"中文")&&GetTickCount()-start<5000)pump();CHECK(strstr(history,"中文")!=NULL);}
 CHECK(errors==0&&receipts==1);close_service(workspace);
 workspace=open_service(path);CHECK(wait_phase(UI_AI_READY));CHECK(strstr(history,"中文")!=NULL);{char id[65];CHECK(ui_ai_submit(service,&q,id)==UI_STATUS_OK);CHECK(wait_run(id,"\"terminal\":true"));CHECK(finishes==before);}close_service(workspace);
 printf("P3 service basic checks=%u failures=%u\n",checks,failures);return failures?1:0;
}
