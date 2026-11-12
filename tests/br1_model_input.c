/* Causal input boundaries; synthetic events, no authorization or transport. */
#include <windows.h>
static int message_fault,model_unarmed=1;
static int model_dispatch_guard(void *);
#define UI_MODEL_DISPATCH_GUARD(s) model_dispatch_guard(s)
#define UI_HOST_MODEL_ALLOW(s) model_dispatch_guard(s)
static char protected_model[128];
static void model_expect(void *,const char *);
#define UI_MODEL_EXPECT(s,value) model_expect(s,value)
#define UI_MODEL_ALLOW_SAVE() (!message_fault)
static void model_message_observe(void *,const MSG *,unsigned);
#define UI_MODEL_MESSAGE_OBSERVER(s,m,stage) model_message_observe(s,m,stage)
#define SUMMARY_MAIN br1_previous_summary_main
#include "uha_summary.c"
#ifdef UI_BR1_WRITE_TRACE
__declspec(dllimport) void ui_br1_write_trace_dump(void);
__declspec(dllimport) uint64_t ui_br1_write_node_epoch(void);
#endif

/* No per-message output: bounded ring, synthetic model value only. */
typedef struct model_message_record {unsigned sequence,stage,message;DWORD thread;HWND window;char value[128];} model_message_record;
static model_message_record message_records[128];
static unsigned message_sequence;
static const void *protected_view;
static uint64_t protected_epoch;
static void model_expect(void *state,const char *value)
{host_window_t *s=state;protected_view=s->view;model_unarmed=0;
#ifdef UI_BR1_WRITE_TRACE
 protected_epoch=ui_br1_write_node_epoch();
#endif
 strcpy_s(protected_model,sizeof(protected_model),value);model_message_observe(state,NULL,5);}
static model_message_record first_message_fault;
static void model_message_observe(void *state,const MSG *message,unsigned stage)
{
 host_window_t *s=state;ui_element_presentation_t p={0};p.size=sizeof(p);
 #ifdef UI_BR1_WRITE_TRACE
 if(protected_epoch!=ui_br1_write_node_epoch()){protected_model[0]=0;model_unarmed=1;protected_epoch=ui_br1_write_node_epoch();}
#endif
 if(protected_view!=s->view){protected_model[0]=0;model_unarmed=1;protected_view=s->view;}
 if(!s->view||ui_web_view_get_presentation(s->view,"ai-model-name",&p)!=UI_STATUS_OK)return;
 model_message_record *r=&message_records[message_sequence%128];r->sequence=++message_sequence;r->stage=stage;r->message=message?message->message:0;r->window=message?message->hwnd:NULL;r->thread=GetCurrentThreadId();
 strcpy_s(r->value,sizeof(r->value),p.text_utf8);
 if(protected_model[0]&&strcmp(p.text_utf8,protected_model)&&!message_fault){first_message_fault=*r;message_fault=1;}
}
static int model_dispatch_guard(void *state)
{model_message_observe(state,NULL,4);return !message_fault&&!model_unarmed;}
static void model_message_dump(void)
{
 if(message_fault)fprintf(stderr,"FIRST-MESSAGE-DIFFERENCE seq=%u stage=%u thread=%lu hwnd=%p msg=%u value=%s\n",first_message_fault.sequence,first_message_fault.stage,first_message_fault.thread,(void *)first_message_fault.window,first_message_fault.message,first_message_fault.value);
 unsigned first=message_sequence>128?message_sequence-128:0;
 for(unsigned i=first;i<message_sequence;++i){model_message_record *r=&message_records[i%128];
  fprintf(stderr,"MESSAGE seq=%u stage=%u thread=%lu hwnd=%p msg=%u length=%zu value=%s\n",r->sequence,r->stage,r->thread,(void *)r->window,r->message,strlen(r->value),r->value);}
}
static unsigned boundary_sequence;
static int exact_model(host_window_t *s,const char *stage,const char *expected)
{
 ui_element_presentation_t p={0};p.size=sizeof(p);
 ui_status_t status=ui_web_view_get_presentation(s->view,"ai-model-name",&p);
 fprintf(stderr,"CAUSAL seq=%u thread=%lu stage=%s status=%d length=%zu value=%s\n",
  ++boundary_sequence,GetCurrentThreadId(),stage,status,status?0:strlen(p.text_utf8),status?"":p.text_utf8);
 CHECK(status==UI_STATUS_OK);if(status)return 0;
 CHECK(!strcmp(p.text_utf8,expected));CHECK(!message_fault);return !message_fault&&!strcmp(p.text_utf8,expected);
}
static int condition_case(int argc,wchar_t **argv)
{
 (void)argc;host_window_t s={0};GUID g;wchar_t db[4096],u[40];
 CHECK(CreateDirectoryW(argv[2],NULL)||GetLastError()==ERROR_ALREADY_EXISTS);CoCreateGuid(&g);StringFromGUID2(&g,u,40);
 CHECK(_snwprintf_s(db,4096,_TRUNCATE,L"%s/condition-%s.db",argv[2],u)>=0);
 CHECK(SetEnvironmentVariableW(L"UI_AI_DB_PATH",db));CHECK(ui_framework_initialize()==UI_STATUS_OK);start_page(&s);if(!s.ai)return 1;
 printf("CONDITION operation=%ls mode=%ls\n",u,argv[3]);
 text(&s,"ai-model-name","offline-fixture");if(!exact_model(&s,"condition-input","offline-fixture"))goto finish;
 if(!wcscmp(argv[3],L"save-scroll")){
  ui_input_event_t e={0};ui_element_presentation_t before={0},after={0};before.size=sizeof(before);after.size=sizeof(after);CHECK(ui_web_view_get_presentation(s.view,"ai-model-save",&before)==UI_STATUS_OK);RECT client;GetClientRect(s.hwnd,&client);e.size=sizeof(e);e.kind=UI_INPUT_WHEEL;e.x=client.right-60;e.y=client.bottom-100;e.wheel_delta=-2400;
  CHECK(ui_web_view_dispatch_input(s.view,&e)==UI_STATUS_OK);pump(&s);CHECK(ui_web_view_get_presentation(s.view,"ai-model-save",&after)==UI_STATUS_OK);printf("SCROLL before_y=%d after_y=%d before_clip=%d after_clip=%d client_height=%ld\n",before.rect.y,after.rect.y,before.clip.height,after.clip.height,client.bottom);CHECK(before.rect.y!=after.rect.y);send_state(&s,NULL);pump(&s);
  if(!exact_model(&s,"condition-after-scroll-projection","offline-fixture"))goto finish;
 }
 click(&s,"ai-model-save");if(message_fault)goto finish;CHECK(model_wait(&s,NULL));
 {char *v=ai_panel_edit_string(s.ai_model,"model_id");CHECK(v&&!strcmp(v,"offline-fixture"));model_trace("condition-config",v,"save");free(v);}
 if(!wcscmp(argv[3],L"observer-check")){
#ifdef UI_BR1_WRITE_TRACE
  uint64_t epoch=ui_br1_write_node_epoch();
#endif
  CHECK(close_host(&s));memset(&s,0,sizeof(s));start_page(&s);
#ifdef UI_BR1_WRITE_TRACE
  CHECK(ui_br1_write_node_epoch()!=epoch);
#endif
  model_expect(&s,"offline-fixture");CHECK(exact_model(&s,"recreated-config","offline-fixture"));
#ifdef UI_BR1_WRITE_TRACE
  epoch=ui_br1_write_node_epoch();
#endif
  char *page=load_page();CHECK(ui_web_view_load_html(s.view,page)==UI_STATUS_OK);free(page);
  CHECK(!model_dispatch_guard(&s)); /* No save before the replacement node is associated. */
  schedule_refresh(&s);pump(&s);click(&s,"ai-model-toggle");
#ifdef UI_BR1_WRITE_TRACE
  CHECK(ui_br1_write_node_epoch()!=epoch);
#endif
  model_expect(&s,"offline-fixture");CHECK(exact_model(&s,"same-view-rebuild","offline-fixture"));
  text(&s,"ai-model-name","complete-alternate-model");CHECK(exact_model(&s,"legal-switch","complete-alternate-model"));
  /* Observer self-test: a complete legitimate value changes without rearming.
     This is not a missing-character reproduction. */
  ui_input_event_t e={0};e.size=sizeof(e);e.kind=UI_INPUT_KEY_DOWN;e.key_code='A';e.modifiers=UI_INPUT_MODIFIER_CONTROL;
  CHECK(ui_web_view_dispatch_input(s.view,&e)==UI_STATUS_OK);e.kind=UI_INPUT_TEXT;e.modifiers=0;e.text_utf8="complete-observer-control";
  CHECK(ui_web_view_dispatch_input(s.view,&e)==UI_STATUS_OK);CHECK(!model_dispatch_guard(&s));CHECK(message_fault);
  unsigned first=first_message_fault.sequence;
  host_action_t a={0};a.action="ai-model-save";a.token="blocked-observer-save";a.answer="complete-observer-control";process_action(&s,&a);
  CHECK(s.ai->model_config->revision==1&&!strcmp(s.ai->model_config->model,"offline-fixture"));
  a.action="ai-model-propose";a.answer="must not submit";process_action(&s,&a);CHECK(!s.ai_run[0]);
  e.kind=UI_INPUT_KEY_DOWN;e.key_code='A';e.modifiers=UI_INPUT_MODIFIER_CONTROL;CHECK(ui_web_view_dispatch_input(s.view,&e)==UI_STATUS_OK);
  e.kind=UI_INPUT_TEXT;e.modifiers=0;e.text_utf8="complete-alternate-model";CHECK(ui_web_view_dispatch_input(s.view,&e)==UI_STATUS_OK);
  model_message_observe(&s,NULL,6);CHECK(message_fault&&first_message_fault.sequence==first);CHECK(!model_dispatch_guard(&s));
 }
finish:
 CHECK(!s.ai_run[0]);CHECK(!s.ai->model_config->requests);CHECK(close_host(&s));model_message_dump();SetEnvironmentVariableW(L"UI_AI_DB_PATH",NULL);
#ifdef UI_BR1_WRITE_TRACE
 ui_br1_write_trace_dump();
#endif
 printf("condition %ls checks=%u failures=%u observer_alarm=%d\n",argv[3],checks,failures,message_fault);
 return failures?1:0;
}
#ifndef BR1_INPUT_MAIN
#define BR1_INPUT_MAIN wmain
#endif
int BR1_INPUT_MAIN(int argc,wchar_t **argv)
{
 if(argc==4&&(!wcscmp(argv[3],L"save-direct")||!wcscmp(argv[3],L"save-scroll")||!wcscmp(argv[3],L"observer-check")))return condition_case(argc,argv);
 if(argc==4&&!wcscmp(argv[3],L"persisted-watch")){argv[3]=L"persisted";int result=br1_previous_summary_main(argc,argv);model_message_dump();
#ifdef UI_BR1_WRITE_TRACE
  ui_br1_write_trace_dump();
#endif
  return result||message_fault;
 }
 if(argc!=4)return 2;host_window_t s={0};GUID g;wchar_t db[4096],u[40];
 const char *name="offline-fixture";ui_input_event_t e={0};
 CHECK(CreateDirectoryW(argv[2],NULL)||GetLastError()==ERROR_ALREADY_EXISTS);
 CHECK(CoCreateGuid(&g)==S_OK);CHECK(StringFromGUID2(&g,u,40)>0);
 CHECK(_snwprintf_s(db,4096,_TRUNCATE,L"%s/input-%s.db",argv[2],u)>=0);
 CHECK(SetEnvironmentVariableW(L"UI_AI_DB_PATH",db));CHECK(ui_framework_initialize()==UI_STATUS_OK);
 start_page(&s);if(!s.ai)return 1;
 text(&s,"ai-model-name",name);if(!exact_model(&s,"bulk-return",name))goto done;
 strcpy_s(protected_model,sizeof(protected_model),name);
 text(&s,"ai-model-name",name);if(!exact_model(&s,"same-replacement",name))goto done;
 click(&s,"ai-input");if(!exact_model(&s,"focus-away",name))goto done;
 for(unsigned i=0;i<3;++i){send_state(&s,NULL);pump(&s);if(!exact_model(&s,"projection",name))goto done;}
 /* Replace the selection with a queued ordinary character stream. No deletion
    key is injected; every prefix is a completion barrier, not a timed sleep. */
 protected_model[0]=0;click(&s,"ai-model-name");e.size=sizeof(e);e.kind=UI_INPUT_KEY_DOWN;e.key_code='A';e.modifiers=UI_INPUT_MODIFIER_CONTROL;
 CHECK(ui_web_view_dispatch_input(s.view,&e)==UI_STATUS_OK);
 for(size_t i=0;i<strlen(name);++i){char prefix[32];memcpy(prefix,name,i+1);prefix[i+1]=0;
  CHECK(PostMessageW((HWND)ui_web_view_native_handle(s.view),WM_CHAR,(WPARAM)(unsigned char)name[i],1));
  pump(&s);if(!exact_model(&s,"native-prefix",prefix))goto done;
 }
 if(!exact_model(&s,"before-immediate-save",name))goto done;
 strcpy_s(protected_model,sizeof(protected_model),name);
 click(&s,"ai-model-save");CHECK(model_wait(&s,NULL));
 {char *value=ai_panel_edit_string(s.ai_model,"model_id");model_trace("causal-config-readback",value,"native-save");CHECK(value&&!strcmp(value,name));free(value);}
 CHECK(!s.ai_run[0]);CHECK(!s.ai->model_config->requests);
done:
 CHECK(close_host(&s));model_message_dump();CHECK(SetEnvironmentVariableW(L"UI_AI_DB_PATH",NULL));
#ifdef UI_BR1_WRITE_TRACE
 ui_br1_write_trace_dump();
#endif
 printf("BR1 causal checks=%u failures=%u\n",checks,failures);return failures?1:0;
}
