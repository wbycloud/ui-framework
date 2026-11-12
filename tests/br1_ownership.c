/* Controlled production message ordering; isolated Service, no authorization. */
#define BR1_INPUT_MAIN ownership_unused_input_main
#include "br1_model_input.c"
static int bytes_equal(const char *stage,const char *value,const char *expected)
{
 size_t n=value?strlen(value):0;printf("OWN stage=%s address=%p length=%zu terminator=%u hex=",stage,(const void *)value,n,value?(unsigned char)value[n]:255);
 for(size_t i=0;value&&i<=n;++i)printf("%02x",(unsigned char)value[i]);puts("");
 int ok=value&&!strcmp(value,expected);CHECK(ok);return ok;
}
static int raw_click(host_window_t *s,const char *id)
{
 ui_element_presentation_t p={0};p.size=sizeof(p);ui_input_event_t e={0};e.size=sizeof(e);
 if(ui_web_view_get_presentation(s->view,id,&p)!=UI_STATUS_OK||!p.visible||!p.enabled||!p.clip.height){CHECK(0);return 0;}
 e.x=p.clip.x+p.clip.width/2;e.y=p.clip.y+p.clip.height/2;e.pointer_button=1;e.kind=UI_INPUT_POINTER_DOWN;
 if(ui_web_view_dispatch_input(s->view,&e)!=UI_STATUS_OK){CHECK(0);return 0;}
 e.kind=UI_INPUT_POINTER_UP;if(ui_web_view_dispatch_input(s->view,&e)!=UI_STATUS_OK){CHECK(0);return 0;}return 1;
}
static int raw_model(host_window_t *s,const char *value)
{
 if(!raw_click(s,"ai-model-name"))return 0;
 ui_input_event_t e={0};e.size=sizeof(e);e.kind=UI_INPUT_KEY_DOWN;e.key_code='A';e.modifiers=UI_INPUT_MODIFIER_CONTROL;
 CHECK(ui_web_view_dispatch_input(s->view,&e)==UI_STATUS_OK);e.kind=UI_INPUT_TEXT;e.modifiers=0;e.text_utf8=value;
 CHECK(ui_web_view_dispatch_input(s->view,&e)==UI_STATUS_OK);model_expect(s,value);return exact_model(s,"ownership-edited-control",value);
}
int wmain(int argc,wchar_t **argv)
{
 if(argc!=4)return 2;host_window_t s={0};GUID guid;wchar_t name[40],db[4096];MSG saved={0};host_action_t *action=NULL;int locked=0;
 const char *original="offline-fixture",*alternate="complete-alternate-model";char run[65]={0};
 CHECK(CreateDirectoryW(argv[2],NULL)||GetLastError()==ERROR_ALREADY_EXISTS);CoCreateGuid(&guid);StringFromGUID2(&guid,name,40);
 CHECK(_snwprintf_s(db,4096,_TRUNCATE,L"%s/ownership-%s.db",argv[2],name)>=0);CHECK(SetEnvironmentVariableW(L"UI_AI_DB_PATH",db));CHECK(ui_framework_initialize()==UI_STATUS_OK);
 start_page(&s);if(!s.ai)return 1;printf("OWN operation=%ls order=%ls\n",name,argv[3]);
 text(&s,"ai-model-name",original);if(!exact_model(&s,"ownership-input",original))goto done;
 if(!raw_click(&s,"ai-model-save"))goto done;
 /* Remove only the actual queued host action, preserving its allocated data. */
 if(!PeekMessageW(&saved,s.hwnd,HOST_ACTION_MESSAGE,HOST_ACTION_MESSAGE,PM_REMOVE)){CHECK(0);goto done;}
 action=(host_action_t *)saved.lParam;
 if(!action||strcmp(action->action,"ai-model-save")){CHECK(0);goto done;}
 if(!bytes_equal("queued-answer",action->answer,original))goto done;
 if(!wcscmp(argv[3],L"queued-edit")){
  if(!raw_model(&s,alternate))goto done;send_state(&s,NULL);pump(&s);
  if(!bytes_equal("queued-after-edit",action->answer,original))goto done;
 }else if(!wcscmp(argv[3],L"queued-rebuild")){
  char *page=load_page();CHECK(ui_web_view_load_html(s.view,page)==UI_STATUS_OK);free(page);schedule_refresh(&s);pump(&s);click(&s,"ai-model-toggle");
  if(!raw_model(&s,alternate))goto done;
  if(!bytes_equal("queued-after-rebuild",action->answer,original))goto done;
 }
 if(!wcscmp(argv[3],L"worker-edit")){EnterCriticalSection(&s.ai->lock);locked=1;}
 if(!model_dispatch_guard(&s)){CHECK(0);goto done;}
 DispatchMessageW(&saved);action=NULL; /* Normal WndProc owns and frees it. */
 if(locked){
  if(!bytes_equal("pending-after-action-free",s.ai->model_config->pending_model,original))goto done;
  if(!raw_model(&s,alternate))goto done;
  if(!bytes_equal("pending-after-control-edit",s.ai->model_config->pending_model,original))goto done;
  LeaveCriticalSection(&s.ai->lock);locked=0;
 }
 CHECK(model_wait(&s,NULL));
 {char *model=ai_panel_edit_string(s.ai_model,"model_id");int ok=bytes_equal("config-readback",model,original);free(model);if(!ok)goto done;}
 /* Pure-text local proposal tests task ownership; no fixture confirm or transport. */
 text(&s,"ai-input","BR-1 ownership: local unconfirmed proposal.");click(&s,"ai-model-propose");CHECK(model_wait(&s,"\"phase\":3"));
 if(!s.ai_run[0]){CHECK(0);goto done;}strcpy_s(run,sizeof(run),s.ai_run);
 {char *json=NULL;CHECK(ui_ai_get_run(s.ai,run,&json)==UI_STATUS_OK);char *model=json?ai_panel_edit_string(json,"model_id"):NULL;
  int ok=bytes_equal("task-fixed-model",model,original);free(model);ui_ai_release(json);if(!ok)goto done;}
 CHECK(s.ai->model_config->requests==0);CHECK(ui_ai_cancel(s.ai,run)==UI_STATUS_OK);CHECK(model_wait(&s,"\"outcome\":\"cancelled\""));
done:
 if(locked)LeaveCriticalSection(&s.ai->lock);if(action)action_free(action);
 CHECK(close_host(&s));SetEnvironmentVariableW(L"UI_AI_DB_PATH",NULL);model_message_dump();
#ifdef UI_BR1_WRITE_TRACE
 ui_br1_write_trace_dump();
#endif
 printf("ownership %ls checks=%u failures=%u alarm=%d\n",argv[3],checks,failures,message_fault);return failures||message_fault?1:0;
}
