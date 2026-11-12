#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "ui_framework/ai_service.h"
static ui_ai_service_t *service;
static unsigned checks,failures;
static int reenter,freed;
static char old_run[65];
#define CHECK(x) do{++checks;if(!(x)){++failures;fprintf(stderr,"line %d: %s\n",__LINE__,#x);}}while(0)
static void event(const ui_ai_event_t *e,void *data)
{(void)data;if(!strcmp(e->type,"history")&&!strcmp(e->query_id,"parents")){const char *p=strstr(e->payload,"\"run_id\":\"");if(p){memcpy(old_run,p+10,64);old_run[64]=0;}}}
static int phase(ui_ai_service_phase_t expected)
{DWORD start=GetTickCount();ui_ai_service_state_t s={0};s.size=sizeof(s);s.version=1;do{CHECK(ui_ai_service_poll(service,64)==UI_STATUS_OK);CHECK(ui_ai_service_get_state(service,&s)==UI_STATUS_OK);if(s.phase==expected&&!s.worker_alive)return 1;(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}while(GetTickCount()-start<5000);return 0;}
static ui_status_t revision(char *out,size_t capacity,void *data)
{(void)data;if(reenter){ui_status_t status;reenter=0;CHECK(ui_ai_service_request_close(service)==UI_STATUS_OK);CHECK(phase(UI_AI_CLOSED));status=ui_ai_service_destroy(service);CHECK(status==UI_STATUS_PENDING);if(status==UI_STATUS_OK){freed=1;service=NULL;return UI_STATUS_CANCELLED;}}
 strcpy_s(out,capacity,"1");return UI_STATUS_OK;}
static ui_workspace_t *open_service(const char *path)
{ui_workspace_config_t wc={0};ui_ai_service_config_t c={0};ui_workspace_t *w;wc.size=sizeof(wc);wc.run_mode=UI_RUN_OFFSCREEN;w=ui_workspace_create(&wc);CHECK(w!=NULL);c.size=sizeof(c);c.version=1;c.workspace=w;c.storage_path_utf8=path;c.on_event=event;CHECK(ui_ai_service_create(&c,&service)==UI_STATUS_OK);CHECK(ui_ai_service_open(service)==UI_STATUS_OK);CHECK(phase(UI_AI_READY));return w;}
int wmain(int argc,wchar_t **argv)
{
 wchar_t path[4096],exe[4096],command[10000];char db[4096],run[65],package[4096];ui_workspace_t *w;ui_ai_task_request_t q={0};ui_cap_target_t target={0};ui_cap_resource_desc_t resource={0};ui_app_instance_info_t app={0};uint64_t instance=0;
 if(argc!=3)return 2;
 if(!wcscmp(argv[1],L"--seed")){CHECK(WideCharToMultiByte(CP_UTF8,0,argv[2],-1,db,sizeof(db),NULL,NULL)>0);w=open_service(db);q.size=sizeof(q);q.version=1;q.client_request_id="parent";q.input_utf8="unfinished";q.stub_plan_json="{\"steps\":[{\"type\":\"ask\",\"text\":\"wait\"}]}";CHECK(ui_ai_submit(service,&q,run)==UI_STATUS_OK);
  {DWORD start=GetTickCount();int ready=0;do{char *json=NULL;ui_ai_service_state_t s={0};s.size=sizeof(s);s.version=1;ui_ai_service_poll(service,64);ui_ai_get_run(service,run,&json);ui_ai_service_get_state(service,&s);ready=json&&strstr(json,"\"phase\":3")&&!s.worker_alive;ui_ai_release(json);if(ready)break;(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}while(GetTickCount()-start<5000);CHECK(ready);}ExitProcess(failures?1:0);}
 CHECK(CreateDirectoryW(argv[2],NULL)||GetLastError()==ERROR_ALREADY_EXISTS);CHECK(_snwprintf_s(path,4096,_TRUNCATE,L"%s/continue-%lu.db",argv[2],GetCurrentProcessId())>=0);CHECK(WideCharToMultiByte(CP_UTF8,0,path,-1,db,sizeof(db),NULL,NULL)>0);
 {STARTUPINFOW startup={0};PROCESS_INFORMATION p={0};DWORD start,code=99;GetModuleFileNameW(NULL,exe,4096);_snwprintf_s(command,10000,_TRUNCATE,L"\"%s\" --seed \"%s\"",exe,path);startup.cb=sizeof(startup);CHECK(CreateProcessW(exe,command,NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&startup,&p));if(!p.hProcess)return 1;start=GetTickCount();while(WaitForSingleObject(p.hProcess,0)!=WAIT_OBJECT_0&&GetTickCount()-start<5000)(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);CHECK(GetExitCodeProcess(p.hProcess,&code)&&code==0);CloseHandle(p.hProcess);CloseHandle(p.hThread);}
 w=open_service(db);CHECK(ui_ai_history(service,"parents",UI_AI_RUNS,0,32)==UI_STATUS_OK);{DWORD start=GetTickCount();while(!old_run[0]&&GetTickCount()-start<5000){ui_ai_service_poll(service,64);(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}CHECK(old_run[0]);}
 /* A real workspace instance is required for the fresh target lookup. */
 CHECK(WideCharToMultiByte(CP_UTF8,0,argv[1],-1,package,sizeof(package),NULL,NULL)>0);CHECK(ui_workspace_open(w,package,&instance)==UI_STATUS_OK);app.size=sizeof(app);CHECK(ui_workspace_get_instance(w,instance,&app)==UI_STATUS_OK);
 resource.size=sizeof(resource);resource.version=1;resource.kind=UI_CAP_DOCUMENT;resource.id="continue.resource";resource.get_revision=revision;CHECK(ui_capability_register_resource(app.host,&resource)==UI_STATUS_OK);target.size=sizeof(target);target.version=1;CHECK(ui_capability_bind(app.host,resource.id,&target)==UI_STATUS_OK);
 q.size=sizeof(q);q.version=1;q.client_request_id="normal";q.input_utf8="fresh target";q.stub_plan_json="{\"steps\":[]}";q.targets=&target;q.target_count=1;CHECK(ui_ai_continue(service,old_run,&q,run)==UI_STATUS_OK);
 {DWORD start=GetTickCount();int done=0;do{char *json=NULL;ui_ai_service_poll(service,64);ui_ai_get_run(service,run,&json);done=json&&strstr(json,"\"terminal\":true")&&strstr(json,"\"outcome\":\"completed\"");ui_ai_release(json);if(done)break;(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}while(GetTickCount()-start<5000);CHECK(done);}
 q.client_request_id="reenter";reenter=1;CHECK(ui_ai_continue(service,old_run,&q,run)==UI_STATUS_CANCELLED);CHECK(!freed&&!run[0]);if(!freed){CHECK(phase(UI_AI_CLOSED));CHECK(ui_ai_service_destroy(service)==UI_STATUS_OK);}service=NULL;
 CHECK(ui_workspace_close_all(w)==UI_STATUS_OK);ui_workspace_poll(w);CHECK(ui_workspace_count(w)==0);CHECK(ui_workspace_destroy(w)==UI_STATUS_OK);
 printf("P3 continue checks=%u failures=%u freed=%d\n",checks,failures,freed);return failures?1:0;
}
