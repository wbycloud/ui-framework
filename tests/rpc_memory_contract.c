/* Production DLL/fixture contract; no Key, transport replacement or real request. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include "ui_framework/ai_validation.h"
static unsigned checks,failures;
#define CHECK(x) do{++checks;if(!(x)){++failures;fprintf(stderr,"line %d: %s\n",__LINE__,#x);}}while(0)
static void pump(ui_ai_service_t *s,ui_workspace_t *w){CHECK(ui_ai_service_poll(s,64)==UI_STATUS_OK);ui_workspace_poll(w);(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}
static int exact_memory_result(const char *raw)
{
 char *canonical=NULL;size_t length=0;if(!raw)return 0;const char *schema="{\"type\":\"object\",\"properties\":{\"value\":{\"type\":\"integer\"},\"revision\":{\"type\":\"string\"}},\"required\":[\"value\",\"revision\"],\"additionalProperties\":false}";
 int ok=ui_cap_json_validate(schema,raw,strlen(raw))==UI_STATUS_OK&&ui_cap_json_canonicalize(raw,strlen(raw),&canonical,&length)==UI_STATUS_OK&&length==strlen("{\"revision\":\"r1\",\"value\":10}")&&!strcmp(canonical,"{\"revision\":\"r1\",\"value\":10}");ui_capability_release(canonical);return ok;
}
int wmain(int argc,wchar_t **argv)
{
 if(argc!=3)return 2;CHECK(ui_framework_initialize()==UI_STATUS_OK);wchar_t guid[40],path[4096];GUID g;CoCreateGuid(&g);StringFromGUID2(&g,guid,40);CHECK(CreateDirectoryW(argv[2],NULL)||GetLastError()==ERROR_ALREADY_EXISTS);char db[4096],config[4096],package[4096];swprintf_s(path,4096,L"%s/%s.sqlite",argv[2],guid);CHECK(WideCharToMultiByte(CP_UTF8,0,path,-1,db,4096,NULL,NULL)>0);CHECK(WideCharToMultiByte(CP_UTF8,0,argv[1],-1,package,4096,NULL,NULL)>0);snprintf(config,4096,"%s.model.json",db);
 ui_workspace_config_t wc={0};wc.size=sizeof(wc);wc.run_mode=UI_RUN_OFFSCREEN;ui_workspace_t *w=ui_workspace_create(&wc);CHECK(w!=NULL);
 CHECK(exact_memory_result("{\"value\":10,\"revision\":\"r1\"}"));CHECK(!exact_memory_result("{\"value\":100,\"revision\":\"r1\"}"));CHECK(!exact_memory_result("{\"value\":\"10\",\"revision\":\"r1\"}"));CHECK(!exact_memory_result("{\"value\":10,\"revision\":\"r10\"}"));
#ifndef UI_RPC_STATIC_ONLY
 uint64_t instance=0;CHECK(ui_workspace_open(w,package,&instance)==UI_STATUS_OK);ui_app_instance_info_t app={0};app.size=sizeof(app);CHECK(ui_workspace_get_instance(w,instance,&app)==UI_STATUS_OK);
 uint64_t generation=0;char *catalog=NULL;CHECK(ui_capability_discover(app.host,"memory",0,32,&generation,&catalog)==UI_STATUS_OK);CHECK(catalog&&strstr(catalog,"memory.read")&&!strstr(catalog,"memory.set"));ui_capability_release(catalog);
 ui_cap_target_t target={0};target.size=sizeof(target);target.version=1;CHECK(ui_capability_bind(app.host,"memory.document",&target)==UI_STATUS_OK);CHECK(!strcmp(target.expected_revision,"r1"));uint64_t call=0;CHECK(ui_capability_prepare(app.host,"memory.read",&target,"{}",&call)==UI_STATUS_OK);CHECK(ui_capability_dispatch(app.host,call)==UI_STATUS_OK);ui_cap_result_t result={0};result.size=sizeof(result);result.version=1;CHECK(ui_capability_get_result(app.host,call,&result)==UI_STATUS_OK);CHECK(result.state==UI_CAP_FINISHED&&result.outcome==UI_CAP_OK&&exact_memory_result(result.output_json));CHECK(!strcmp(result.revision,"r1"));CHECK(ui_capability_forget(app.host,call)==UI_STATUS_OK);

#endif
 for(unsigned round=0;round<2;++round){ui_ai_service_t *s=NULL;ui_ai_service_config_t c={0};c.size=sizeof(c);c.version=1;c.workspace=w;c.storage_path_utf8=db;CHECK(ui_ai_service_create(&c,&s)==UI_STATUS_OK);CHECK(ui_ai_service_open(s)==UI_STATUS_OK);ui_ai_service_state_t ss={0};ss.size=sizeof(ss);ss.version=1;DWORD start=GetTickCount();do{pump(s,w);ui_ai_service_get_state(s,&ss);if(ss.phase==UI_AI_READY)break;}while(GetTickCount()-start<5000);CHECK(ss.phase==UI_AI_READY);CHECK(ui_ai_model_open(s,config)==UI_STATUS_OK);ui_ai_validation_desc_t d={0};d.size=sizeof(d);d.version=1;d.validation_id="production-offline";d.input_nano_per_token=300;d.output_nano_per_token=1200;CHECK(ui_ai_validation_enable(s,&d)==UI_STATUS_OK);
  char *state=NULL;int ready=0;start=GetTickCount();do{pump(s,w);ui_ai_validation_get_state(s,&state);ready=state&&strstr(state,round?"validation_restart_review_required":"\"ready\":true")!=NULL;ui_ai_release(state);state=NULL;if(ready)break;}while(GetTickCount()-start<5000);CHECK(ready);CHECK(ui_ai_validation_get_state(s,&state)==UI_STATUS_OK);CHECK(strstr(state,"\"send_entries\":0")&&strstr(state,"\"reserved\":0"));if(round)CHECK(strstr(state,"\"send_count_known\":false"));ui_ai_release(state);
  ui_ai_validation_request_t q={0};q.size=sizeof(q);q.version=1;q.task=UI_AI_VALIDATION_TEXT;q.client_request_id="unconfigured-no-send";char run[65];CHECK(ui_ai_validation_propose(s,&q,run)!=UI_STATUS_OK);
  CHECK(ui_ai_service_request_close(s)==UI_STATUS_OK);start=GetTickCount();ui_status_t destroyed=UI_STATUS_PENDING;do{pump(s,w);destroyed=ui_ai_service_destroy(s);if(!destroyed)break;}while(GetTickCount()-start<5000);CHECK(destroyed==UI_STATUS_OK);
 }
 CHECK(ui_workspace_close_all(w)==UI_STATUS_OK);DWORD start=GetTickCount();while(ui_workspace_count(w)&&GetTickCount()-start<5000)ui_workspace_poll(w);CHECK(ui_workspace_destroy(w)==UI_STATUS_OK);printf("RPC production memory/isolation/restart checks=%u failures=%u; actual sends=0\n",checks,failures);return failures?1:0;
}
