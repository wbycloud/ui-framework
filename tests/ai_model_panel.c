/* Real Light host + isolated runtime. Private acceptance is not physical input. */
#define wmain original_edit_panel_main
#include "ai_edit_panel.c"
#undef wmain
#include "ai_model_internal.h"
#include "ai_model_wire.h"
__declspec(dllimport) ui_status_t ui_ai_model_fixture(ui_ai_service_t *,int,const char *);
__declspec(dllimport) ui_status_t ui_ai_model_fixture_light(ui_ai_service_t *,ui_web_view_t *,const char *);
static void bridge(ui_web_view_t *v,const char *j,void *p){fprintf(stderr,"RP bridge %s\n",j);on_web_message(v,j,p);}
static int model_ready(host_window_t *s)
{DWORD begin=GetTickCount();do{pump(s);if(s->ai_model&&!strstr(s->ai_model,"\"pending\":true"))return 1;}while(GetTickCount()-begin<5000);fprintf(stderr,"model panel run=%s error=%s config=%s\n",s->ai_run,s->ai_error?s->ai_error:"",s->ai_model?s->ai_model:"");return 0;}
static int model_result(host_window_t *s,const char *part)
{DWORD begin=GetTickCount();do{char *j=NULL;pump(s);if(!ui_ai_get_run(s->ai,s->ai_run,&j)){int ok=strstr(j,part)!=NULL;ui_ai_release(j);if(ok)return 1;}}while(GetTickCount()-begin<5000);fprintf(stderr,"model panel run=%s error=%s config=%s\n",s->ai_run,s->ai_error?s->ai_error:"",s->ai_model?s->ai_model:"");return 0;}
int wmain(int argc,wchar_t **argv)
{
 host_window_t s={0};host_action_t a={0};wchar_t db[4096],unique[40];GUID guid;ui_element_presentation_t p={0};p.size=sizeof(p);
 if(argc!=3)return 2;CHECK(CreateDirectoryW(argv[2],NULL)||GetLastError()==ERROR_ALREADY_EXISTS);CoCreateGuid(&guid);StringFromGUID2(&guid,unique,40);_snwprintf_s(db,4096,_TRUNCATE,L"%s/rp-panel-%s.db",argv[2],unique);CHECK(SetEnvironmentVariableW(L"UI_AI_DB_PATH",db));CHECK(ui_framework_initialize()==UI_STATUS_OK);CHECK(create_host(&s,GetModuleHandleW(NULL))!=NULL);if(!s.ai)return 1;ui_web_view_set_message_callback(s.view,bridge,&s);CHECK(model_ready(&s));CHECK(MoveWindow(s.hwnd,-16000,-16000,1200,2600,FALSE));ShowWindow(s.hwnd,SW_HIDE);
 a.action="assistant";process_action(&s,&a);pump(&s);click(&s,"ai-model-toggle");text(&s,"ai-model-name","offline-fixture");click(&s,"ai-model-save");CHECK(model_ready(&s));CHECK(strstr(s.ai_model,"offline-fixture"));CHECK(strstr(s.ai_model,"\"ready\":true"));
 yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *list=yyjson_mut_arr(d);yyjson_mut_doc_set_root(d,list);yyjson_mut_arr_add_strcpy(d,list,rp_text);char *wire=yyjson_mut_write(d,0,NULL);CHECK(ui_ai_model_fixture(s.ai,1,wire)==UI_STATUS_OK);free(wire);yyjson_mut_doc_free(d);CHECK(ui_ai_model_fixture(s.ai,5,"fixture-key-no-leak")==UI_STATUS_OK);
 text(&s,"ai-input","明确只发此输入");click(&s,"ai-model-propose");CHECK(model_result(&s,"\"phase\":3"));DWORD begin=GetTickCount();do{pump(&s);if(s.ai_model_bindings[1])break;}while(GetTickCount()-begin<5000);CHECK(s.ai_model_bindings[1]!=0);
 CHECK(ui_web_view_get_presentation(s.view,"ai-model-scope",&p)==UI_STATUS_OK&&strstr(p.text_utf8,"明确只发此输入"));CHECK(ui_web_view_get_presentation(s.view,"ai-input",&p)==UI_STATUS_OK&&!p.text_utf8[0]);
 a.action="ai-model-confirm";process_action(&s,&a);CHECK(s.ai_error&&s.ai_error[0]);CHECK(s.ai->model_config->requests==0);CHECK(s.ai->model_config->requests==0);
 /* Public dispatch_input is simulated and must not authorize disclosure. */
 {ui_rect_t rect={0};ui_input_event_t input={0};CHECK(ui_web_view_get_element_rect(s.view,"ai-model-confirm",&rect)==UI_STATUS_OK);input.size=sizeof(input);input.x=rect.x+1;input.y=rect.y+1;input.pointer_button=1;input.kind=UI_INPUT_POINTER_DOWN;(void)ui_web_view_dispatch_input(s.view,&input);input.kind=UI_INPUT_POINTER_UP;(void)ui_web_view_dispatch_input(s.view,&input);CHECK(s.ai->model_config->requests==0);}
 CHECK(ui_ai_model_fixture_light(s.ai,s.view,"ai-model-confirm")==UI_STATUS_OK);CHECK(ui_ai_model_fixture_light(s.ai,s.view,"ai-model-confirm")==UI_STATUS_PERMISSION_DENIED);a.action="assistant";process_action(&s,&a);CHECK(model_result(&s,"\"terminal\":true"));process_action(&s,&a);pump(&s);CHECK(s.ai->model_config->requests==1);CHECK(ui_web_view_get_presentation(s.view,"ai-output",&p)==UI_STATUS_OK&&strstr(p.text_utf8,"你好"));
 text(&s,"ai-input","失败保留输入");a.action="ai-model-propose";a.token="bad-scope";a.answer="失败保留输入";a.params="{\"history\":\"[]\",\"projection\":\"{broken\"}";process_action(&s,&a);CHECK(s.ai_error&&s.ai_error[0]);CHECK(ui_web_view_get_presentation(s.view,"ai-input",&p)==UI_STATUS_OK&&!strcmp(p.text_utf8,"失败保留输入"));CHECK(close_host(&s));SetEnvironmentVariableW(L"UI_AI_DB_PATH",NULL);printf("RP Light checks=%u failures=%u; simulated only\n",checks,failures);return failures?1:0;
}
