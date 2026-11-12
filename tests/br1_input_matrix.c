/* Whole-string input boundaries; isolated synthetic events, no task or save. */
#define BR1_INPUT_MAIN matrix_unused_input_main
#include "br1_model_input.c"
__declspec(dllimport) int ui_br1_input_verify(const char *,int);
__declspec(dllimport) void ui_br1_input_dump(void);
static int matrix_check(host_window_t *s,const char *expected,int moved)
{int ok=ui_br1_input_verify(expected,moved);CHECK(ok);return exact_model(s,"matrix-return",expected)&&ok;}
static int matrix_text(host_window_t *s,const char *value)
{text(s,"ai-model-name",value);return matrix_check(s,value,0);}
static int matrix_key(host_window_t *s,unsigned key,unsigned modifiers)
{ui_input_event_t e={0};e.size=sizeof(e);e.kind=UI_INPUT_KEY_DOWN;e.key_code=key;e.modifiers=modifiers;ui_status_t r=ui_web_view_dispatch_input(s->view,&e);CHECK(r==UI_STATUS_OK);return r==UI_STATUS_OK;}
int wmain(int argc,wchar_t **argv)
{
 if(argc==4&&!wcscmp(argv[3],L"persisted-chain")){argv[3]=L"persisted";int result=br1_previous_summary_main(argc,argv);model_message_dump();ui_br1_input_dump();ui_br1_write_trace_dump();return result||message_fault;}
 if(argc!=4)return 2;host_window_t s={0};GUID g;wchar_t db[4096],u[40];const char *value="offline-fixture";
 CHECK(CreateDirectoryW(argv[2],NULL)||GetLastError()==ERROR_ALREADY_EXISTS);CHECK(CoCreateGuid(&g)==S_OK);CHECK(StringFromGUID2(&g,u,40)>0);
 CHECK(_snwprintf_s(db,4096,_TRUNCATE,L"%s/matrix-%s.db",argv[2],u)>=0);CHECK(SetEnvironmentVariableW(L"UI_AI_DB_PATH",db));CHECK(ui_framework_initialize()==UI_STATUS_OK);
 start_page(&s);if(!s.ai)return 1;printf("MATRIX operation=%ls case=%ls\n",u,argv[3]);
 if(!wcscmp(argv[3],L"migration")){
  const char *html="<html><body><input id='ai-model-name' style='width:240px;height:32px'><div id='grow'></div><script>var once=false;document.getElementById('ai-model-name').addEventListener('input',function(){if(once)return;once=true;for(var i=0;i<80;i++){var n=document.createElement('span');n.textContent='x';document.getElementById('grow').appendChild(n);}});</script></body></html>";
  CHECK(ui_web_view_load_html(s.view,html)==UI_STATUS_OK);protected_model[0]=0;model_unarmed=1;
  text(&s,"ai-model-name",value);if(!matrix_check(&s,value,1))goto done;
 }else if(!wcscmp(argv[3],L"selection")){
  if(!matrix_text(&s,"prefix-original-suffix"))goto done;
  if(!matrix_key(&s,VK_HOME,0))goto done;
  for(unsigned i=0;i<7;++i)if(!matrix_key(&s,VK_RIGHT,0))goto done;
  for(unsigned i=0;i<8;++i)if(!matrix_key(&s,VK_RIGHT,UI_INPUT_MODIFIER_SHIFT))goto done;
  ui_input_event_t e={0};e.size=sizeof(e);e.kind=UI_INPUT_TEXT;e.text_utf8=value;CHECK(ui_web_view_dispatch_input(s.view,&e)==UI_STATUS_OK);
  model_expect(&s,"prefix-offline-fixture-suffix");if(!matrix_check(&s,"prefix-offline-fixture-suffix",0))goto done;pump(&s);
 }else if(!wcscmp(argv[3],L"lengths")){
  char long_value[128];memset(long_value,'a',127);long_value[127]=0;
  const char *values[]={"x",value,"offline-fixture-X",long_value};
  for(unsigned i=0;i<4;++i)if(!matrix_text(&s,values[i]))goto done;
 }else if(!wcscmp(argv[3],L"utf8")){
  if(!matrix_text(&s,"模型-😀-offline-fixture"))goto done;
 }else{
  if(!wcscmp(argv[3],L"replace")&&!matrix_text(&s,"previous-complete-model"))goto done;
  if(!matrix_text(&s,value))goto done;
  if(!wcscmp(argv[3],L"focus")){click(&s,"ai-input");if(!exact_model(&s,"focus-away",value)||!matrix_text(&s,value))goto done;}
 }
done:
 CHECK(!s.ai_run[0]);CHECK(!s.ai->model_config->requests);CHECK(close_host(&s));SetEnvironmentVariableW(L"UI_AI_DB_PATH",NULL);
 model_message_dump();ui_br1_input_dump();ui_br1_write_trace_dump();printf("matrix %ls checks=%u failures=%u alarm=%d\n",argv[3],checks,failures,message_fault);return failures||message_fault?1:0;
}
