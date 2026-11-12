/* Simulated frontend, never a physical-input or authority-positive test. */
#define UI_HOST_TEST 1
#include "../examples/framework_host/main.c"
static unsigned checks,failures;
#define CHECK(x) do{++checks;if(!(x)){++failures;fprintf(stderr,"line %d: %s\n",__LINE__,#x);}}while(0)
static void pump(void)
{MSG m;unsigned count=0;while(count++<256&&PeekMessageW(&m,NULL,0,0,PM_REMOVE)){if(m.message==WM_QUIT)continue;TranslateMessage(&m);DispatchMessageW(&m);}(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}
static int ready(host_window_t *s)
{DWORD start=GetTickCount();ui_ai_service_state_t state={0};state.size=sizeof(state);state.version=1;do{pump();CHECK(ui_ai_service_get_state(s->ai,&state)==UI_STATUS_OK);if(state.phase==UI_AI_READY)return 1;}while(GetTickCount()-start<5000);return 0;}
static int result(host_window_t *s,const char *value)
{DWORD start=GetTickCount();do{char *json=NULL;pump();if(ui_ai_get_run(s->ai,s->ai_run,&json)==UI_STATUS_OK){int found=strstr(json,value)!=NULL;ui_ai_release(json);if(found)return 1;}}while(GetTickCount()-start<5000);return 0;}
static int close_host(host_window_t *s)
{DWORD start=GetTickCount();PostMessageW(s->hwnd,WM_CLOSE,0,0);while(IsWindow(s->hwnd)&&GetTickCount()-start<5000)pump();return !IsWindow(s->hwnd)&&!s->workspace&&!s->ai;}
static void click(host_window_t *s,const char *id)
{ui_rect_t rect={0};ui_input_event_t e={0};CHECK(ui_web_view_get_element_rect(s->view,id,&rect)==UI_STATUS_OK&&rect.width>0&&rect.height>0);e.size=sizeof(e);e.x=rect.x+rect.width/2;e.y=rect.y+rect.height/2;e.pointer_button=1;e.kind=UI_INPUT_POINTER_DOWN;CHECK(ui_web_view_dispatch_input(s->view,&e)==UI_STATUS_OK);e.kind=UI_INPUT_POINTER_UP;CHECK(ui_web_view_dispatch_input(s->view,&e)==UI_STATUS_OK);pump();}
static void text(host_window_t *s,const char *id,const char *value)
{ui_input_event_t e={0};click(s,id);e.size=sizeof(e);e.kind=UI_INPUT_KEY_DOWN;e.key_code='A';e.modifiers=UI_INPUT_MODIFIER_CONTROL;CHECK(ui_web_view_dispatch_input(s->view,&e)==UI_STATUS_OK);e.kind=UI_INPUT_TEXT;e.modifiers=0;e.text_utf8=value;CHECK(ui_web_view_dispatch_input(s->view,&e)==UI_STATUS_OK);pump();}
int wmain(int argc,wchar_t **argv)
{
 host_window_t s={0};host_action_t action={0};ui_app_instance_info_t app={0};ui_element_presentation_t p={0};ui_ai_service_state_t before={0},after={0};uint64_t first,second;wchar_t db[4096];char number[32],run[65],conversation[37];
 if(argc!=4)return 2;CHECK(CreateDirectoryW(argv[2],NULL)||GetLastError()==ERROR_ALREADY_EXISTS);CHECK(_snwprintf_s(db,4096,_TRUNCATE,L"%s/panel-%lu.db",argv[2],GetCurrentProcessId())>=0);CHECK(SetEnvironmentVariableW(L"UI_AI_DB_PATH",db));CHECK(ui_framework_initialize()==UI_STATUS_OK);CHECK(create_host(&s,GetModuleHandleW(NULL))!=NULL);if(!s.hwnd||!s.ai)return 1;CHECK(ready(&s));ShowWindow(s.hwnd,SW_HIDE);
 open_path(&s,argv[1]);pump();first=ui_workspace_active(s.workspace);open_path(&s,argv[1]);pump();second=ui_workspace_active(s.workspace);CHECK(first&&second&&first!=second);CHECK(get_instance(&s,first,&app));
 action.action="assistant";process_action(&s,&action);pump();before.size=sizeof(before);before.version=1;CHECK(ui_ai_service_get_state(s.ai,&before)==UI_STATUS_OK);strcpy_s(conversation,37,before.conversation_id);
 snprintf(number,sizeof(number),"%llu",(unsigned long long)first);action.action="ai-submit";action.id=number;action.token="panel-read";action.answer="explicit read";action.panel="counter.document";action.params="{\"steps\":[{\"type\":\"discover\",\"category\":\"document\",\"target\":0},{\"type\":\"tool\",\"target\":0,\"tool_call_id\":\"counter\",\"capability_id\":\"counter.read\",\"arguments\":{}}]}";process_action(&s,&action);strcpy_s(run,65,s.ai_run);CHECK(run[0]);CHECK(ui_workspace_activate(s.workspace,second)==UI_STATUS_OK);
 action.action="assistant";process_action(&s,&action);CHECK(result(&s,"\"terminal\":true"));CHECK(result(&s,"\"outcome\":\"completed\""));CHECK(result(&s,"count\\\":0"));CHECK(result(&s,"counter-1"));CHECK(result(&s,"counter.read"));{char *json=NULL;CHECK(ui_ai_get_run(s.ai,run,&json)==UI_STATUS_OK);CHECK(json&&!strstr(json,"counter.increment"));ui_ai_release(json);}CHECK(!strcmp(s.ai_ack,"panel-read"));
 after.size=sizeof(after);after.version=1;CHECK(ui_ai_service_get_state(s.ai,&after)==UI_STATUS_OK&&!strcmp(conversation,after.conversation_id));
 /* A v2 write is rejected without visiting an Assistant/legacy handler. */
 action.action="ai-submit";action.token="write-denied";action.params="{\"steps\":[{\"type\":\"tool\",\"target\":0,\"tool_call_id\":\"write\",\"capability_id\":\"counter.increment\",\"arguments\":{}}]}";process_action(&s,&action);CHECK(result(&s,"\"terminal\":true"));CHECK(result(&s,"\"outcome\":\"failed\""));
 action.token="ask";action.params="{\"steps\":[{\"type\":\"ask\",\"text\":\"explicit question\"}]}";process_action(&s,&action);CHECK(result(&s,"\"phase\":3"));action.action="ai-answer";action.id=s.ai_run;action.command="question-0";action.token="answer-1";action.answer="明确回答";process_action(&s,&action);CHECK(result(&s,"\"terminal\":true"));CHECK(result(&s,"明确回答"));
 action.action="assistant";process_action(&s,&action);pump();p.size=sizeof(p);CHECK(ui_web_view_get_presentation(s.view,"ai-state",&p)==UI_STATUS_OK&&p.visible&&strstr(p.text_utf8,conversation));CHECK(ui_web_view_get_presentation(s.view,"ai-output",&p)==UI_STATUS_OK&&strstr(p.text_utf8,"明确回答"));
 /* Exercise the actual production page/JS bridge with explicitly simulated
    input. This is read-only chat, never a human-source authority positive. */
 CHECK(MoveWindow(s.hwnd,0,0,1200,1800,FALSE));pump();text(&s,"ai-plan","{\"steps\":[]}");text(&s,"ai-input","UI echo 中文");click(&s,"ai-send");CHECK(result(&s,"UI echo 中文"));CHECK(result(&s,"\"terminal\":true"));pump();CHECK(ui_web_view_get_presentation(s.view,"ai-input",&p)==UI_STATUS_OK&&!p.text_utf8[0]);
 text(&s,"ai-plan","{broken");text(&s,"ai-input","retain this draft");click(&s,"ai-send");CHECK(ui_web_view_get_presentation(s.view,"ai-input",&p)==UI_STATUS_OK&&!strcmp(p.text_utf8,"retain this draft"));CHECK(s.ai_error&&s.ai_error[0]);
 /* Invalid submission also retains input at the host boundary. */
 action.action="ai-submit";action.id=number;action.token="bad-script";action.answer="keep input";action.params="{broken";process_action(&s,&action);CHECK(s.ai_error&&s.ai_error[0]&&!strcmp(action.answer,"keep input"));
 /* This unmigrated legacy fixture controls REFUSE; it is not an AI tool or
    a human-source positive. New Service calls still only use the read catalog. */
 open_path(&s,argv[3]);pump();CHECK(get_instance(&s,ui_workspace_active(s.workspace),&app));CHECK(ui_host_invoke(app.host,"fixture.close_refuse","{}","fixture-control")!=0);
 action.action="ai-submit";action.token="refuse-cancel";action.answer="wait before exit";action.panel="";action.params="{\"steps\":[{\"type\":\"ask\",\"text\":\"cancelled on refused exit\"}]}";process_action(&s,&action);CHECK(result(&s,"\"phase\":3"));SendMessageW(s.hwnd,WM_CLOSE,0,0);pump();CHECK(IsWindow(s.hwnd)&&!s.exiting);after.size=sizeof(after);after.version=1;CHECK(ui_ai_service_get_state(s.ai,&after)==UI_STATUS_OK&&after.phase==UI_AI_READY);CHECK(result(&s,"\"outcome\":\"cancelled\""));CHECK(ui_host_invoke(app.host,"fixture.close_allow","{}","fixture-control")!=0);CHECK(close_host(&s));
 memset(&s,0,sizeof(s));CHECK(create_host(&s,GetModuleHandleW(NULL))!=NULL);CHECK(ready(&s));after.size=sizeof(after);after.version=1;CHECK(ui_ai_service_get_state(s.ai,&after)==UI_STATUS_OK&&!strcmp(conversation,after.conversation_id)&&!after.active_run_id[0]&&after.queued==0);{DWORD start=GetTickCount();while((!s.ai_messages||!strstr(s.ai_messages,"明确回答"))&&GetTickCount()-start<5000)pump();CHECK(s.ai_messages&&strstr(s.ai_messages,"明确回答"));}CHECK(close_host(&s));CHECK(SetEnvironmentVariableW(L"UI_AI_DB_PATH",NULL));printf("P3 Light panel/business checks=%u failures=%u\n",checks,failures);return failures?1:0;
}
