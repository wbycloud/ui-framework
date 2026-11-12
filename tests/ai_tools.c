#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui_framework/ai_service.h"
#include "ui_framework/app_data.h"
#include "capability_fixture_control.h"
static unsigned checks,failures,drain_worker,drain_callback;
static ui_workspace_t *workspace;static ui_ai_service_t *service;static uint64_t call;
static char run[65],stored[2048];
static ui_host_t *test_host;static unsigned revision=1,executions;
#define CHECK(x) do{++checks;if(!(x)){++failures;fprintf(stderr,"line %d: %s\n",__LINE__,#x);}}while(0)
static ui_status_t get_revision(char *out,size_t capacity,void *data)
{(void)data;return snprintf(out,capacity,"%u",revision)>0?UI_STATUS_OK:UI_STATUS_LIMIT_EXCEEDED;}
static void read_value(uint64_t id,const ui_cap_target_t *target,const char *params,void *data)
{ui_cap_result_t result={0};(void)target;(void)params;++executions;result.size=sizeof(result);result.version=1;result.outcome=UI_CAP_OK;result.output_json=data?"{\"value\":\"invalid\"}":"{\"value\":73}";CHECK(ui_capability_complete(test_host,id,&result)==UI_STATUS_OK);CHECK(ui_capability_complete(test_host,id,&result)==UI_STATUS_ALREADY_EXISTS);}
static void event(const ui_ai_event_t *e,void *data)
{(void)data;if(!strcmp(e->type,"query")&&!strcmp(e->query_id,"inflight")){CHECK(strlen(e->payload)<sizeof(stored));strcpy_s(stored,sizeof(stored),e->payload);}if(!strcmp(e->type,"tool_bound")){char tool[65];unsigned long long value=0;CHECK(sscanf_s(e->payload,"{\"tool_call_id\":\"%64[^\"]\",\"call_id\":\"%llu\"}",tool,(unsigned)sizeof(tool),&value)==2);call=(uint64_t)value;CHECK(e->committed&&e->sequence>0&&call);}}
static void app_event(uint64_t instance,const char *type,const char *json,void *data)
{(void)instance;(void)data;if(!strcmp(type,"cap.drain"))CHECK(sscanf_s(json,"{\"worker_done\":%u,\"callback_done\":%u}",&drain_worker,&drain_callback)==2);}
static void pump(void)
{MSG m;unsigned count=0;while(count++<128&&PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}CHECK(ui_ai_service_poll(service,64)==UI_STATUS_OK);ui_workspace_poll(workspace);(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}
static int wait_run(const char *value)
{DWORD start=GetTickCount();do{char *json=NULL;pump();if(ui_ai_get_run(service,run,&json)==UI_STATUS_OK){int found=strstr(json,value)!=NULL;ui_ai_release(json);if(found)return 1;}}while(GetTickCount()-start<5000);return 0;}
static int wait_phase(ui_ai_service_phase_t phase)
{DWORD start=GetTickCount();ui_ai_service_state_t s={0};s.size=sizeof(s);s.version=1;do{pump();CHECK(ui_ai_service_get_state(service,&s)==UI_STATUS_OK);if(s.phase==phase)return 1;}while(GetTickCount()-start<5000);return 0;}
static void submit(const char *id,const char *plan,ui_cap_target_t *target)
{ui_ai_task_request_t r={0};r.size=sizeof(r);r.version=1;r.client_request_id=id;r.input_utf8="explicit stub read test";r.stub_plan_json=plan;r.targets=target;r.target_count=1;call=0;CHECK(ui_ai_submit(service,&r,run)==UI_STATUS_OK);}
static void signal_wait(HANDLE signal)
{DWORD start=GetTickCount();while(WaitForSingleObject(signal,0)!=WAIT_OBJECT_0&&GetTickCount()-start<5000)pump();CHECK(WaitForSingleObject(signal,0)==WAIT_OBJECT_0);}
int wmain(int argc,wchar_t **argv)
{
 HWND parent;ui_workspace_config_t wc={0};ui_ai_service_config_t config={0};ui_app_instance_info_t app={0};ui_cap_target_t target={0},old;uint64_t a=0,b=0;char package[4096],db[4096];wchar_t path[4096];
 if(argc!=3)return 2;CHECK(CreateDirectoryW(argv[2],NULL)||GetLastError()==ERROR_ALREADY_EXISTS);CHECK(WideCharToMultiByte(CP_UTF8,0,argv[1],-1,package,sizeof(package),NULL,NULL)>0);
 CHECK(_snwprintf_s(path,4096,_TRUNCATE,L"%s/tools-%lu.db",argv[2],GetCurrentProcessId())>=0);CHECK(WideCharToMultiByte(CP_UTF8,0,path,-1,db,sizeof(db),NULL,NULL)>0);
 parent=CreateWindowExW(0,L"STATIC",L"P3 real DLL",WS_OVERLAPPED,0,0,640,480,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(parent!=NULL);wc.size=sizeof(wc);wc.native_parent=parent;wc.event=app_event;workspace=ui_workspace_create(&wc);CHECK(workspace!=NULL);
 CHECK(ui_workspace_open(workspace,package,&a)==UI_STATUS_OK);CHECK(ui_workspace_open(workspace,package,&b)==UI_STATUS_OK);app.size=sizeof(app);CHECK(ui_workspace_get_instance(workspace,a,&app)==UI_STATUS_OK);target.size=sizeof(target);target.version=1;CHECK(ui_capability_bind(app.host,"fixture.document",&target)==UI_STATUS_OK);old=target;
 config.size=sizeof(config);config.version=1;config.workspace=workspace;config.storage_path_utf8=db;config.on_event=event;CHECK(ui_ai_service_create(&config,&service)==UI_STATUS_OK);CHECK(ui_ai_service_open(service)==UI_STATUS_OK);CHECK(wait_phase(UI_AI_READY));
 {ui_cap_resource_desc_t resource={0};ui_capability_desc_t capability={0};ui_cap_target_t own={0};unsigned before;
  test_host=app.host;resource.size=sizeof(resource);resource.version=1;resource.kind=UI_CAP_DOCUMENT;resource.id="service.document";resource.get_revision=get_revision;CHECK(ui_capability_register_resource(app.host,&resource)==UI_STATUS_OK);
  capability.size=sizeof(capability);capability.version=capability.capability_version=1;capability.id="service.read";capability.name_utf8="73";capability.description_utf8="Synchronous business read";capability.category="test";capability.effect="read";capability.input_schema_json="{\"type\":\"object\",\"additionalProperties\":false}";capability.output_schema_json="{\"type\":\"object\",\"properties\":{\"value\":{\"type\":\"integer\"}},\"required\":[\"value\"],\"additionalProperties\":false}";capability.target_kind=UI_CAP_DOCUMENT;capability.execute=read_value;CHECK(ui_capability_register(app.host,&capability)==UI_STATUS_OK);capability.id="service.invalid";capability.user_data=&revision;CHECK(ui_capability_register(app.host,&capability)==UI_STATUS_OK);
  own.size=sizeof(own);own.version=1;CHECK(ui_capability_bind(app.host,"service.document",&own)==UI_STATUS_OK);before=executions;
  submit("revision","{\"steps\":[{\"type\":\"tool\",\"target\":0,\"tool_call_id\":\"revision\",\"capability_id\":\"service.read\",\"arguments\":{}}]}",&own);++revision;CHECK(wait_run("\"terminal\":true"));CHECK(wait_run("\"outcome\":\"failed\""));CHECK(executions==before&&call!=0);/* prepare establishes the ID; dispatch rejects revision before execute. */
  CHECK(ui_capability_bind(app.host,"service.document",&own)==UI_STATUS_OK);submit("duplicate-tool","{\"steps\":[{\"type\":\"tool\",\"target\":0,\"tool_call_id\":\"once\",\"capability_id\":\"service.read\",\"arguments\":{}},{\"type\":\"tool\",\"target\":0,\"tool_call_id\":\"once\",\"capability_id\":\"service.read\",\"arguments\":{}}]}",&own);CHECK(wait_run("\"terminal\":true"));CHECK(wait_run("\"outcome\":\"failed\""));CHECK(wait_run("value\\\":73"));CHECK(executions==before+1);
  submit("invalid-output","{\"steps\":[{\"type\":\"tool\",\"target\":0,\"tool_call_id\":\"invalid\",\"capability_id\":\"service.invalid\",\"arguments\":{}}]}",&own);CHECK(wait_run("\"terminal\":true"));CHECK(wait_run("\"outcome\":\"failed\""));CHECK(executions==before+2);
 }
 /* The pending read is delivered to a, although b is the active real tab. */
 submit("fixed","{\"steps\":[{\"type\":\"tool\",\"target\":0,\"tool_call_id\":\"read\",\"capability_id\":\"fixture.read\",\"fragments\":[\"{\",\"}\"],\"complete\":true}]}",&target);CHECK(ui_workspace_activate(workspace,b)==UI_STATUS_OK);
 {DWORD start=GetTickCount();while(!call&&GetTickCount()-start<5000)pump();CHECK(call!=0);stored[0]=0;CHECK(ui_ai_query_run(service,"inflight",run)==UI_STATUS_OK);start=GetTickCount();while(!stored[0]&&GetTickCount()-start<5000)pump();CHECK(strstr(stored,"\"phase\":4")&&strstr(stored,"\"committed\":true"));CHECK(ui_workspace_post_app_data(workspace,a,CAP_FIXTURE_RESULT,&call,sizeof(call))==UI_STATUS_OK);CHECK(wait_run("\"terminal\":true"));CHECK(wait_run("\"outcome\":\"completed\""));CHECK(wait_run("value\\\":42"));}
 submit("fragment","{\"steps\":[{\"type\":\"tool\",\"target\":0,\"tool_call_id\":\"bad\",\"capability_id\":\"fixture.read\",\"fragments\":[\"{\"],\"complete\":true}]}",&target);CHECK(wait_run("\"terminal\":true"));CHECK(wait_run("\"outcome\":\"failed\""));CHECK(!call);
 submit("bad-params","{\"steps\":[{\"type\":\"tool\",\"target\":0,\"tool_call_id\":\"bad\",\"capability_id\":\"fixture.read\",\"arguments\":{\"unexpected\":true}}]}",&target);CHECK(wait_run("\"terminal\":true"));CHECK(wait_run("\"outcome\":\"failed\""));CHECK(!call);
 /* A DLL-owned worker and external callback keep both the app and Service alive. */
 submit("worker","{\"steps\":[{\"type\":\"tool\",\"target\":0,\"tool_call_id\":\"worker\",\"capability_id\":\"fixture.worker\",\"arguments\":{\"fill\":false}}]}",&target);
 {DWORD start=GetTickCount();HANDLE gates[CAP_FIXTURE_GATE_COUNT]={0};wchar_t name[128];while(!call&&GetTickCount()-start<5000)pump();CHECK(call!=0);/* tool_bound can precede the next UI dispatch. */
  do{pump();cap_fixture_gate_name(name,a,CAP_FIXTURE_WORKER_STARTED);gates[CAP_FIXTURE_WORKER_STARTED]=OpenEventW(SYNCHRONIZE|EVENT_MODIFY_STATE,FALSE,name);}while(!gates[CAP_FIXTURE_WORKER_STARTED]&&GetTickCount()-start<5000);
  for(int i=0;i<CAP_FIXTURE_GATE_COUNT;++i)if(!gates[i]){cap_fixture_gate_name(name,a,i);gates[i]=OpenEventW(SYNCHRONIZE|EVENT_MODIFY_STATE,FALSE,name);CHECK(gates[i]!=NULL);}signal_wait(gates[CAP_FIXTURE_WORKER_STARTED]);signal_wait(gates[CAP_FIXTURE_CALLBACK_STARTED]);
  CHECK(ui_ai_cancel(service,run)==UI_STATUS_OK);CHECK(ui_ai_service_quiesce(service)==UI_STATUS_OK);CHECK(ui_workspace_close(workspace,a,UI_APP_CLOSE_TAB)==UI_STATUS_OK);CHECK(ui_ai_service_request_close(service)==UI_STATUS_OK);pump();CHECK(ui_workspace_count(workspace)==2&&GetModuleHandleW(L"ui_capability_fixture.dll")!=NULL);CHECK(ui_ai_service_destroy(service)==UI_STATUS_PENDING);CHECK(wait_run("\"terminal\":false"));
  CHECK(SetEvent(gates[CAP_FIXTURE_POST]));signal_wait(gates[CAP_FIXTURE_POST_OK]);CHECK(SetEvent(gates[CAP_FIXTURE_WORKER_EXIT]));start=GetTickCount();do{CHECK(ui_workspace_post_app_data(workspace,a,CAP_FIXTURE_POLL,&call,sizeof(call))==UI_STATUS_OK);pump();}while(!drain_worker&&GetTickCount()-start<5000);CHECK(drain_worker&&!drain_callback);CHECK(ui_workspace_count(workspace)==2);
  CHECK(SetEvent(gates[CAP_FIXTURE_CALLBACK_EXIT]));start=GetTickCount();do{CHECK(ui_workspace_post_app_data(workspace,a,CAP_FIXTURE_POLL,&call,sizeof(call))==UI_STATUS_OK);pump();}while(!drain_callback&&GetTickCount()-start<5000);CHECK(drain_callback);CHECK(wait_phase(UI_AI_CLOSED));CHECK(wait_run("\"outcome\":\"cancelled\""));for(int i=0;i<CAP_FIXTURE_GATE_COUNT;++i)if(gates[i])CloseHandle(gates[i]);
 }
 {DWORD start=GetTickCount();ui_status_t status;do{pump();status=ui_ai_service_destroy(service);}while(status==UI_STATUS_PENDING&&GetTickCount()-start<5000);CHECK(status==UI_STATUS_OK);service=NULL;}
 CHECK(ui_workspace_post_app_data(workspace,a,CAP_FIXTURE_RESULT,&call,sizeof(call))==UI_STATUS_CANCELLED);CHECK(ui_workspace_close_all(workspace)==UI_STATUS_OK);ui_workspace_poll(workspace);CHECK(ui_workspace_count(workspace)==0);CHECK(ui_workspace_open(workspace,package,&a)==UI_STATUS_OK&&a!=old.instance_id);app.size=sizeof(app);{ui_status_t lookup=ui_workspace_get_instance(workspace,a,&app);CHECK(lookup==UI_STATUS_OK);if(lookup!=UI_STATUS_OK)return 1;}{uint64_t id=0;CHECK(ui_capability_prepare(app.host,"fixture.read",&old,"{}",&id)!=UI_STATUS_OK&&!id);}CHECK(ui_workspace_close_all(workspace)==UI_STATUS_OK);ui_workspace_poll(workspace);CHECK(ui_workspace_destroy(workspace)==UI_STATUS_OK); CHECK(_snwprintf_s(path,4096,_TRUNCATE,L"%s/sync-%lu.db",argv[2],GetCurrentProcessId())>=0);CHECK(WideCharToMultiByte(CP_UTF8,0,path,-1,db,sizeof(db),NULL,NULL)>0);workspace=ui_workspace_create(&wc);CHECK(workspace!=NULL);config.workspace=workspace;config.storage_path_utf8=db;CHECK(ui_ai_service_create(&config,&service)==UI_STATUS_OK);CHECK(ui_ai_service_open(service)==UI_STATUS_OK);CHECK(wait_phase(UI_AI_READY));
 /* A synchronous complete closes its own app while execute still has a stack. */
 {uint64_t sync=0;ui_app_instance_info_t info={0};ui_cap_target_t bound={0};CHECK(ui_workspace_open(workspace,package,&sync)==UI_STATUS_OK);info.size=sizeof(info);CHECK(ui_workspace_get_instance(workspace,sync,&info)==UI_STATUS_OK);bound.size=sizeof(bound);bound.version=1;CHECK(ui_capability_bind(info.host,"fixture.document",&bound)==UI_STATUS_OK);
  submit("sync-close","{\"steps\":[{\"type\":\"tool\",\"target\":0,\"tool_call_id\":\"sync\",\"capability_id\":\"fixture.sync\",\"arguments\":{}}]}",&bound);CHECK(wait_run("\"terminal\":true"));CHECK(wait_run("\"outcome\":\"completed\""));CHECK(wait_run("value\\\":42"));CHECK(ui_workspace_count(workspace)==0);
 }
 CHECK(ui_ai_service_request_close(service)==UI_STATUS_OK);CHECK(wait_phase(UI_AI_CLOSED));{DWORD start=GetTickCount();ui_status_t status;do{pump();status=ui_ai_service_destroy(service);}while(status==UI_STATUS_PENDING&&GetTickCount()-start<5000);CHECK(status==UI_STATUS_OK);}service=NULL;CHECK(ui_workspace_destroy(workspace)==UI_STATUS_OK);DestroyWindow(parent);
 printf("P3 real DLL read/close checks=%u failures=%u\n",checks,failures);return failures?1:0;
}
