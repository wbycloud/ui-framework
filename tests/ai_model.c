/* Reuse the actual two-instance workspace/DLL setup, not a fixed host pointer. */
#define wmain graph_reference_main
#include "ai_coordinator.c"
#undef wmain
#include "ui_framework/ai_model.h"
#include "ui_internal.h"
#include "ai_model_internal.h"
#include "ai_model_wire.h"
#include "ai_model_extra_wire.h"
__declspec(dllimport) ui_status_t ui_ai_model_fixture(ui_ai_service_t *,int,const char *);
static int secret_leaked;static const char *rp_history="[]";
static void model_event(const ui_ai_event_t *e,void *data){if(strstr(e->payload,"fixture-key-no-leak"))secret_leaked=1;if(!strcmp(e->type,"history")&&!strcmp(e->query_id,"cold-list"))strcpy_s(query_result,sizeof(query_result),e->payload);event(e,data);}
static int ready_model(void)
{DWORD begin=GetTickCount();do{char *p=NULL;pump();if(!ui_ai_model_get_state(s,&p)){int pending=strstr(p,"\"pending\":true")!=NULL;ui_ai_release(p);if(!pending)return 1;}}while(GetTickCount()-begin<5000);return 0;}
static void model_setup(void)
{
 char path[4096],*p=NULL,ref[128];snprintf(path,4096,"%s.model.json",dbpath);CHECK(ui_ai_model_open(s,path)==UI_STATUS_OK);CHECK(ready_model());CHECK(ui_ai_model_get_state(s,&p)==UI_STATUS_OK);str(p,"credential_ref",ref,128);ui_ai_release(p);
 ui_ai_model_profile_t c={0};c.size=sizeof(c);c.version=1;c.profile_id="deepseek";c.protocol_profile="deepseek-responses-v1";c.origin="https://api.deepseek.com";c.model_id="offline-fixture";c.credential_ref=ref;c.revision=1;if(!s->model_config->revision)CHECK(ui_ai_model_configure(s,&c,"config-one")==UI_STATUS_OK);CHECK(ready_model());CHECK(s->model_config->ready);c.origin="https://api.openai.com";CHECK(ui_ai_model_configure(s,&c,"wrong-origin")==UI_STATUS_INVALID_ARGUMENT);c.origin="https://api.deepseek.com";c.credential_ref="some-existing-user-secret";CHECK(ui_ai_model_configure(s,&c,"wrong-ref")==UI_STATUS_PERMISSION_DENIED);s->config.on_event=model_event;
}
static void responses(const char *first,const char *second)
{yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *a=yyjson_mut_arr(d);yyjson_mut_doc_set_root(d,a);yyjson_mut_arr_add_strcpy(d,a,first);if(second)yyjson_mut_arr_add_strcpy(d,a,second);char *json=yyjson_mut_write(d,0,NULL);CHECK(ui_ai_model_fixture(s,1,json)==UI_STATUS_OK);CHECK(ui_ai_model_fixture(s,5,"fixture-key-no-leak")==UI_STATUS_OK);free(json);yyjson_mut_doc_free(d);}
static ui_status_t real_submit(const char *client,int two)
{
 ui_cap_target_t target[2]={0};for(unsigned i=0;i<(two?2u:1u);++i){target[i].size=sizeof(target[i]);target[i].version=1;CHECK(ui_capability_bind(apps[i].host,"memory.document",&target[i])==UI_STATUS_OK);}
 ui_ai_real_task_t q={0};q.size=sizeof(q);q.version=1;q.client_request_id=client;q.input_utf8="Read explicit memory values";q.profile_id="deepseek";q.profile_revision=1;q.targets=target;q.target_count=two?2:1;q.history_selection_json=rp_history;q.disclosure_projection_json="{\"capabilities\":[\"memory.read\"],\"fields\":[\"value\",\"revision\"]}";return ui_ai_real_propose(s,&q,run);
}
static int real_wait(const char *part)
{DWORD start=GetTickCount();do{char *json=NULL;pump();if(!ui_ai_get_run(s,run,&json)){int found=strstr(json,part)!=NULL;strcpy_s(state,sizeof(state),json);ui_ai_release(json);if(found)return 1;}}while(GetTickCount()-start<5000);fprintf(stderr,"real wait %.10000s\n",state);return 0;}
static void real_case(int kind)
{
 responses(kind==1?rp_bad_batch:kind==2?rp_text:rp_two_reads,kind?NULL:rp_text);
 CHECK(real_submit("real-one",1)==UI_STATUS_OK);CHECK(real_wait("\"phase\":3"));CHECK(s->model_config->requests==0);CHECK(ui_ai_model_fixture(s,2,"unknown-run")==UI_STATUS_NOT_FOUND);
 CHECK(ui_workspace_activate(w,apps[1].instance_id)==UI_STATUS_OK);CHECK(ui_ai_model_fixture(s,2,run)==UI_STATUS_OK);CHECK(ui_ai_model_fixture(s,2,run)==UI_STATUS_PERMISSION_DENIED);CHECK(real_wait("\"terminal\":true"));CHECK(value(0)==10&&value(1)==10);
 CHECK(strstr(state,kind==1?"\"outcome\":\"failed\"":"\"outcome\":\"completed\"")!=NULL);CHECK(!secret_leaked);printf("real snapshot %s\n",state);CHECK(!strstr(s->model_config->fixture_body,"fixture-key-no-leak"));CHECK(real_submit("real-one",1)==UI_STATUS_OK);CHECK(s->model_config->requests==(kind?1u:2u));
 if(!kind){CHECK(strstr(s->model_config->fixture_body,"function_call_output")!=NULL);CHECK(strstr(s->model_config->fixture_body,"value\\\":10")!=NULL);}
 close_service();if(!kind)CHECK(cr_sql("SELECT count(*) FROM tools WHERE status=0 AND result LIKE '%\"value\":10%'")==2);CHECK(cr_sql("SELECT count(*) FROM events WHERE terminal=1")==1);CHECK(cr_sql("SELECT count(*) FROM events WHERE payload LIKE '%fixture-key-no-leak%'")==0);
}
static void cancel_case(void)
{
 responses(rp_text,NULL);CHECK(ui_ai_model_fixture(s,3,"1")==UI_STATUS_OK);CHECK(real_submit("real-cancel",0)==UI_STATUS_OK);CHECK(real_wait("\"phase\":3"));CHECK(ui_ai_model_fixture(s,2,run)==UI_STATUS_OK);DWORD begin=GetTickCount();do{pump();if(s->model_config->requests)break;}while(GetTickCount()-begin<5000);CHECK(s->model_config->requests==1);CHECK(ui_ai_cancel(s,run)==UI_STATUS_OK);ui_ai_edit_fixture(s,3,5001);pump();CHECK(real_wait("\"terminal\":false"));CHECK(ui_ai_service_request_close(s)==UI_STATUS_OK);CHECK(ui_ai_service_destroy(s)==UI_STATUS_PENDING);CHECK(ui_ai_model_fixture(s,3,"0")==UI_STATUS_OK);CHECK(real_wait("\"terminal\":true"));CHECK(value(0)==10);
}

#include "ai_model_cases.inc"
#include "ai_model_closeout.inc"
#include "ai_validation_cases.inc"
#include "ai_validation_closeout.inc"
#include "ai_credential_cases.inc"
#include "ai_batch_cases.inc"
#include "ui_framework/ai_model_view.h"
#include "uha_disclosure.inc"
#ifndef AI_MODEL_MAIN
#define AI_MODEL_MAIN wmain
#endif
int AI_MODEL_MAIN(int argc,wchar_t **argv)
{
 if(argc!=4&&argc!=5)return 2;if(argc==5)CHECK(WideCharToMultiByte(CP_UTF8,0,argv[4],-1,override_db,sizeof(override_db),NULL,NULL)>0);CHECK(ui_framework_initialize()==UI_STATUS_OK);cr_package=argv[1];cr_directory=argv[2];setup(argv[1],argv[2]);model_setup();if(!wcscmp(argv[3],L"--rp-seed")){responses(rp_text,NULL);CHECK(real_submit("cold-proposal",0)==UI_STATUS_OK);CHECK(real_wait("\"phase\":3"));CHECK(s->model_config->requests==0);ExitProcess(failures?1:0);}else if(!wcsncmp(argv[3],L"uh-",3))uh_disclosure(argv[3]);else if(!wcsncmp(argv[3],L"bc-",3))bc_case(argv[3]);else if(!wcscmp(argv[3],L"pg-schema"))batch_page_contract();else if(!wcscmp(argv[3],L"pg-reject"))batch_page_reject();else if(!wcsncmp(argv[3],L"ck-",3))ck_case(argv[3]);else if(!wcsncmp(argv[3],L"vr-",3))vr_case(argv[3]);else if(!wcsncmp(argv[3],L"vc-",3))vc_case(argv[3]);else if(!wcscmp(argv[3],L"cr-discover"))rc_discover();else if(!wcscmp(argv[3],L"cr-scan"))rc_scan();else if(!wcscmp(argv[3],L"http401"))http_case(401);else if(!wcscmp(argv[3],L"http429"))http_case(429);else if(!wcscmp(argv[3],L"http503"))http_case(503);else if(!wcscmp(argv[3],L"expired"))expiration_case();else if(!wcscmp(argv[3],L"changed"))changed_case(0);else if(!wcscmp(argv[3],L"reopen"))changed_case(1);else if(!wcscmp(argv[3],L"truncated"))truncated_case();else if(!wcscmp(argv[3],L"checkpoint"))checkpoint_case();else if(!wcscmp(argv[3],L"store"))store_case();else if(!wcscmp(argv[3],L"native"))native_case();else if(!wcscmp(argv[3],L"queue"))queue_case();else if(!wcscmp(argv[3],L"restart"))restart_case();else if(!wcscmp(argv[3],L"discover"))extra_case(0);else if(!wcscmp(argv[3],L"arguments"))extra_case(1);else if(!wcscmp(argv[3],L"duplicate"))extra_case(2);else if(!wcscmp(argv[3],L"history"))history_case();else if(!wcscmp(argv[3],L"inflight-expiry"))inflight_expiry();else if(!wcscmp(argv[3],L"async"))async_case(0);else if(!wcscmp(argv[3],L"async-cancel"))async_case(1);else if(!wcscmp(argv[3],L"reentry"))reentry_case();else if(!wcscmp(argv[3],L"callback"))callback_case();else if(!wcscmp(argv[3],L"cache"))cache_case();else if(!wcscmp(argv[3],L"tls"))transport_case(0);else if(!wcscmp(argv[3],L"proxy"))transport_case(1);else if(!wcscmp(argv[3],L"timeout"))transport_case(2);else if(!wcscmp(argv[3],L"cancel"))cancel_case();else if(!wcscmp(argv[3],L"invalid"))real_case(1);else if(!wcscmp(argv[3],L"text"))real_case(2);else real_case(0);
 if(s)close_service();CHECK(ui_workspace_close_all(w)==UI_STATUS_OK);DWORD begin=GetTickCount();while(ui_workspace_count(w)&&GetTickCount()-begin<5000){ui_workspace_poll(w);(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}CHECK(ui_workspace_count(w)==0);CHECK(ui_workspace_destroy(w)==UI_STATUS_OK);printf("RP model checks=%u failures=%u; no Provider access\n",checks,failures);return failures?1:0;
}
