/* Static embedding: same Service and Provider contract, no second engine. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include "ui_framework/ai_model.h"
#include "yyjson.h"
#include "ai_model_wire.h"
ui_status_t ui_ai_model_fixture(ui_ai_service_t *,int,const char *);
static unsigned checks,failures;
static ui_ai_service_t *service;
#define CHECK(x) do{++checks;if(!(x)){++failures;fprintf(stderr,"line %d: %s\n",__LINE__,#x);}}while(0)
static void pump(void){CHECK(ui_ai_service_poll(service,64)==UI_STATUS_OK);(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}
static int state(const char *id,const char *part)
{DWORD start=GetTickCount();do{char *text=NULL;pump();ui_status_t status=id?ui_ai_get_run(service,id,&text):ui_ai_model_get_state(service,&text);int ok=!status&&strstr(text,part)!=NULL;ui_ai_release(text);if(ok)return 1;}while(GetTickCount()-start<5000);return 0;}
int wmain(int argc,wchar_t **argv)
{
 wchar_t unique[40],path[4096];char db[4096],config[4096],ref[128],run[65],*json=NULL;GUID guid;ui_workspace_config_t wc={0};ui_ai_service_config_t sc={0};ui_ai_service_state_t ss={0};
 if(argc!=2)return 2;CHECK(CreateDirectoryW(argv[1],NULL)||GetLastError()==ERROR_ALREADY_EXISTS);CoCreateGuid(&guid);StringFromGUID2(&guid,unique,40);_snwprintf_s(path,4096,_TRUNCATE,L"%s/static-%s.db",argv[1],unique);CHECK(WideCharToMultiByte(CP_UTF8,0,path,-1,db,4096,NULL,NULL)>0);snprintf(config,sizeof(config),"%s.model.json",db);CHECK(ui_framework_initialize()==UI_STATUS_OK);wc.size=sizeof(wc);wc.run_mode=UI_RUN_OFFSCREEN;ui_workspace_t *w=ui_workspace_create(&wc);CHECK(w!=NULL);sc.size=sizeof(sc);sc.version=1;sc.workspace=w;sc.storage_path_utf8=db;CHECK(ui_ai_service_create(&sc,&service)==UI_STATUS_OK);CHECK(ui_ai_service_open(service)==UI_STATUS_OK);ss.size=sizeof(ss);ss.version=1;DWORD begin=GetTickCount();do{pump();ui_ai_service_get_state(service,&ss);if(ss.phase==UI_AI_READY)break;}while(GetTickCount()-begin<5000);CHECK(ss.phase==UI_AI_READY);CHECK(ui_ai_model_open(service,config)==UI_STATUS_OK);CHECK(state(NULL,"\"pending\":false"));CHECK(ui_ai_model_get_state(service,&json)==UI_STATUS_OK);yyjson_doc *d=yyjson_read(json,strlen(json),0);strcpy_s(ref,sizeof(ref),yyjson_get_str(yyjson_obj_get(yyjson_doc_get_root(d),"credential_ref")));yyjson_doc_free(d);ui_ai_release(json);
 ui_ai_model_profile_t p={0};p.size=sizeof(p);p.version=1;p.profile_id="deepseek";p.protocol_profile="deepseek-responses-v1";p.origin="https://api.deepseek.com";p.model_id="offline-fixture";p.credential_ref=ref;p.revision=1;CHECK(ui_ai_model_configure(service,&p,"static-config")==UI_STATUS_OK);CHECK(state(NULL,"\"ready\":true"));yyjson_mut_doc *m=yyjson_mut_doc_new(NULL);yyjson_mut_val *a=yyjson_mut_arr(m);yyjson_mut_doc_set_root(m,a);yyjson_mut_arr_add_strcpy(m,a,rp_text);json=yyjson_mut_write(m,0,NULL);CHECK(ui_ai_model_fixture(service,1,json)==UI_STATUS_OK);free(json);yyjson_mut_doc_free(m);CHECK(ui_ai_model_fixture(service,5,"fixture-static-key")==UI_STATUS_OK);
 ui_ai_real_task_t q={0};q.size=sizeof(q);q.version=1;q.client_request_id="static-text";q.input_utf8="Only this input";q.profile_id="deepseek";q.profile_revision=1;q.history_selection_json="[]";q.disclosure_projection_json="{\"capabilities\":[],\"fields\":[]}";CHECK(ui_ai_real_propose(service,&q,run)==UI_STATUS_OK);CHECK(state(run,"\"phase\":3"));CHECK(ui_ai_model_fixture(service,2,run)==UI_STATUS_OK);CHECK(state(run,"\"terminal\":true"));CHECK(state(run,"你好"));CHECK(ui_ai_service_request_close(service)==UI_STATUS_OK);begin=GetTickCount();ui_status_t status;do{pump();status=ui_ai_service_destroy(service);if(!status)break;}while(GetTickCount()-begin<5000);CHECK(status==UI_STATUS_OK);CHECK(ui_workspace_destroy(w)==UI_STATUS_OK);printf("RP static checks=%u failures=%u\n",checks,failures);return failures?1:0;
}
