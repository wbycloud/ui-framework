/* Isolated post-acceptance positive; program input proves rejection only. */
#include <stdio.h>
#include <string.h>
static unsigned model_trace_serial;
static void model_trace(const char *stage,const char *value,const char *id)
{fprintf(stderr,"MODEL-BOUNDARY op=%u stage=%s id=%s bytes=%zu value=%s\n",model_trace_serial,stage,id?id:"",value?strlen(value):0,value?value:"");}
#define UI_HOST_MODEL_TRACE(stage,value,id) model_trace(stage,value,id)
#define UI_HOST_TEST 1
#include "../examples/framework_host/main.c"
#include "ui_framework/ai_edit_ui.h"
#include <objbase.h>
#include "ai_internal.h"
__declspec(dllimport) ui_status_t ui_ai_edit_fixture_light(ui_ai_service_t *,ui_web_view_t *,const char *);
__declspec(dllimport) void ui_ai_edit_fixture(ui_ai_service_t *,int,uint64_t);
static unsigned checks,failures;static char snapshot[524289];
#define CHECK(x) do{++checks;if(!(x)){++failures;fprintf(stderr,"line %d: %s\n",__LINE__,#x);}}while(0)
#ifndef UI_MODEL_MESSAGE_OBSERVER
#define UI_MODEL_MESSAGE_OBSERVER(s,m,stage) ((void)0)
#endif
#ifndef UI_MODEL_ALLOW_SAVE
#define UI_MODEL_ALLOW_SAVE() 1
#endif
#ifndef UI_MODEL_EXPECT
#define UI_MODEL_EXPECT(s,value) ((void)0)
#endif
#ifndef UI_MODEL_DISPATCH_GUARD
#define UI_MODEL_DISPATCH_GUARD(s) 1
#endif
#ifndef UI_MODEL_DISPATCH_GUARD
#define UI_MODEL_DISPATCH_GUARD(s) 1
#endif
static void pump(host_window_t *s)
{MSG m;unsigned count=0;while(count++<128&&PeekMessageW(&m,NULL,0,0,PM_REMOVE)){if(m.message!=WM_QUIT){UI_MODEL_MESSAGE_OBSERVER(s,&m,0);TranslateMessage(&m);DispatchMessageW(&m);UI_MODEL_MESSAGE_OBSERVER(s,&m,1);}}UI_MODEL_MESSAGE_OBSERVER(s,NULL,2);if(s->ai)ai_panel_tick(s);UI_MODEL_MESSAGE_OBSERVER(s,NULL,3);(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}
static int phase(host_window_t *s,int wanted)
{DWORD start=GetTickCount();do{char *json=NULL;pump(s);if(ui_ai_edit_get_state(s->ai,&json)==UI_STATUS_OK){strcpy_s(snapshot,sizeof(snapshot),json);ui_ai_release(json);if(ai_panel_edit_number(snapshot,"phase")==wanted){pump(s);return 1;}}}while(GetTickCount()-start<5000);fprintf(stderr,"last %.1800s\n",snapshot);return 0;}
static void click(host_window_t *s,const char *id)
{if((!strcmp(id,"ai-model-save")||!strcmp(id,"ai-model-propose"))&&!UI_MODEL_ALLOW_SAVE()){CHECK(0);return;}if(!strcmp(id,"ai-model-save")){ui_element_presentation_t m={0};m.size=sizeof(m);if(ui_web_view_get_presentation(s->view,"ai-model-name",&m)==UI_STATUS_OK)model_trace("before-save",m.text_utf8,id);}
ui_rect_t r={0};ui_input_event_t e={0};ui_element_presentation_t p={0};RECT client;p.size=sizeof(p);e.size=sizeof(e);GetClientRect(s->hwnd,&client);
 for(unsigned i=0;i<32;++i){CHECK(ui_web_view_get_presentation(s->view,id,&p)==UI_STATUS_OK);r=p.rect;if(p.visible&&p.clip.height==r.height)break;e.kind=UI_INPUT_WHEEL;{ui_rect_t area={0},developer={0};const char *scroll=ui_web_view_get_element_rect(s->view,"ai-developer-toggle",&developer)==UI_STATUS_OK?"assistant":"ai-scroll";CHECK(ui_web_view_get_element_rect(s->view,scroll,&area)==UI_STATUS_OK);e.x=area.x+area.width-4;e.y=area.y+area.height/2;}e.wheel_delta=r.y<p.clip.y?480:-480;CHECK(ui_web_view_dispatch_input(s->view,&e)==UI_STATUS_OK);pump(s);}
 if((!strcmp(id,"ai-model-save")||!strcmp(id,"ai-model-propose"))&&!UI_MODEL_DISPATCH_GUARD(s)){CHECK(0);return;}
 if((!strcmp(id,"ai-model-save")||!strcmp(id,"ai-model-propose"))&&!UI_MODEL_DISPATCH_GUARD(s)){CHECK(0);return;}
 if(!p.visible||!p.enabled||p.clip.height!=r.height)fprintf(stderr,"control %s visible=%d enabled=%d rect=%d,%d,%d,%d clip=%d,%d,%d,%d\n",id,p.visible,p.enabled,r.x,r.y,r.width,r.height,p.clip.x,p.clip.y,p.clip.width,p.clip.height);
 CHECK(p.visible&&p.enabled&&p.clip.height==r.height);e.x=p.clip.x+p.clip.width/2;e.y=r.y+r.height/2;e.pointer_button=1;e.kind=UI_INPUT_POINTER_DOWN;CHECK(ui_web_view_dispatch_input(s->view,&e)==UI_STATUS_OK);e.kind=UI_INPUT_POINTER_UP;if((!strcmp(id,"ai-model-save")||!strcmp(id,"ai-model-propose"))&&!UI_MODEL_DISPATCH_GUARD(s)){CHECK(0);return;}{ui_status_t status=ui_web_view_dispatch_input(s->view,&e);int authority=!strcmp(id,"ai-edit-confirm")||!strcmp(id,"ai-edit-approve")||!strcmp(id,"ai-edit-migrate");CHECK(status==(authority?UI_STATUS_PERMISSION_DENIED:UI_STATUS_OK));}pump(s);}
static void text(host_window_t *s,const char *id,const char *value)
{if(!UI_MODEL_ALLOW_SAVE())return;ui_input_event_t e={0};click(s,id);e.size=sizeof(e);e.kind=UI_INPUT_KEY_DOWN;e.key_code='A';e.modifiers=UI_INPUT_MODIFIER_CONTROL;CHECK(ui_web_view_dispatch_input(s->view,&e)==UI_STATUS_OK);e.kind=UI_INPUT_TEXT;e.modifiers=0;e.text_utf8=value;CHECK(ui_web_view_dispatch_input(s->view,&e)==UI_STATUS_OK);if(!strcmp(id,"ai-model-name")){ui_element_presentation_t m={0};m.size=sizeof(m);++model_trace_serial;model_trace("requested",value,id);CHECK(ui_web_view_get_presentation(s->view,id,&m)==UI_STATUS_OK);model_trace("input-return",m.text_utf8,id);CHECK(m.text_utf8&&!strcmp(m.text_utf8,value));UI_MODEL_EXPECT(s,value);}pump(s);if(!strcmp(id,"ai-model-name")){ui_element_presentation_t m={0};m.size=sizeof(m);CHECK(ui_web_view_get_presentation(s->view,id,&m)==UI_STATUS_OK);model_trace("after-pump",m.text_utf8,id);CHECK(m.text_utf8&&!strcmp(m.text_utf8,value));}}
static int count(host_window_t *s,uint64_t instance)
{ui_app_instance_info_t a={0};ui_cap_target_t t={0};ui_cap_result_t r={0};uint64_t call=0;int value=-1;a.size=sizeof(a);CHECK(ui_workspace_get_instance(s->workspace,instance,&a)==UI_STATUS_OK);t.size=sizeof(t);t.version=1;CHECK(ui_capability_bind(a.host,"counter.document",&t)==UI_STATUS_OK);CHECK(ui_capability_prepare(a.host,"counter.read",&t,"{}",&call)==UI_STATUS_OK);CHECK(ui_capability_dispatch(a.host,call)==UI_STATUS_OK);r.size=sizeof(r);r.version=1;CHECK(ui_capability_get_result(a.host,call,&r)==UI_STATUS_OK&&r.outcome==UI_CAP_OK);if(r.output_json)value=ai_panel_edit_number(r.output_json,"count");CHECK(ui_capability_forget(a.host,call)==UI_STATUS_OK);return value;}
static void propose(host_window_t *s,uint64_t instance,char *client,char *cap,char *resource)
{host_action_t action={0};char number[32];snprintf(number,32,"%llu",(unsigned long long)instance);action.action="ai-edit-propose";action.id=number;action.token=client;action.command=cap;action.panel=resource;action.params="{}";action.answer="{\"type\":\"object\",\"properties\":{},\"additionalProperties\":false}";process_action(s,&action);CHECK(phase(s,UI_AI_EDIT_SCOPE_WAIT));}
static void accepted(host_window_t *s,const char *node)
{DWORD start=GetTickCount();do{pump(s);if(s->ai_projection_generation==s->ai_projection_rendered)break;}while(GetTickCount()-start<5000);CHECK(s->ai_projection_generation==s->ai_projection_rendered);CHECK(ui_ai_edit_fixture_light(s->ai,s->view,node)==UI_STATUS_OK);}
static int close_host(host_window_t *s)
{HWND hwnd=s->hwnd;DWORD start=GetTickCount();PostMessageW(hwnd,WM_CLOSE,0,0);while(IsWindow(hwnd)&&GetTickCount()-start<5000)pump(s);return !IsWindow(hwnd)&&!s->ai&&!s->workspace;}
int wmain(int argc,wchar_t **argv)
{
 host_window_t s={0};host_action_t action={0};ui_ai_service_state_t ss={0};ui_element_presentation_t p={0};uint64_t first,second;DWORD start;wchar_t db[4096],unique_text[40];GUID unique;char *hash=NULL;
 p.size=sizeof(p);if(argc!=4)return 2;CHECK(CreateDirectoryW(argv[3],NULL)||GetLastError()==ERROR_ALREADY_EXISTS);CHECK(CoCreateGuid(&unique)==S_OK);CHECK(StringFromGUID2(&unique,unique_text,40)>0);CHECK(_snwprintf_s(db,4096,_TRUNCATE,L"%s/panel-edit-%s.db",argv[3],unique_text)>=0);CHECK(SetEnvironmentVariableW(L"UI_AI_DB_PATH",db));CHECK(ui_framework_initialize()==UI_STATUS_OK);CHECK(create_host(&s,GetModuleHandleW(NULL))!=NULL);if(!s.ai)return 1;
 ss.size=sizeof(ss);ss.version=1;start=GetTickCount();do{pump(&s);CHECK(ui_ai_service_get_state(s.ai,&ss)==UI_STATUS_OK);if(ss.phase==UI_AI_READY&&s.ai_edit&&ai_panel_edit_number(s.ai_edit,"schema")==1)break;}while(GetTickCount()-start<5000);CHECK(ss.phase==UI_AI_READY);CHECK(s.ai_edit&&strstr(s.ai_edit,"migration_confirmation_required"));
 CHECK(MoveWindow(s.hwnd,-16000,-16000,1200,2600,FALSE));ShowWindow(s.hwnd,SW_HIDE);action.action="assistant";process_action(&s,&action);pump(&s);click(&s,"ai-developer-toggle");
 /* Public page messages and injected clicks cannot grant authority. */
 /* White-box callback-lease model: a PENDING unbind must not cache success.
    This proves retry logic, not a physical source or application callback. */
 {uint64_t old=s.ai_edit_bindings[4];CHECK(old!=0);strcpy_s(s.ai_edit_bound,65,"stale");++s.ai->methods;ai_panel_edit_bindings(&s);CHECK(s.ai_edit_bindings[4]==old&&!strcmp(s.ai_edit_bound,"stale"));--s.ai->methods;ai_panel_edit_bindings(&s);CHECK(s.ai_edit_bindings[4]!=old&&strcmp(s.ai_edit_bound,"stale"));}
 action.action="ai-edit-migrate";process_action(&s,&action);CHECK(s.ai_error&&strstr(s.ai_error,"rejected"));click(&s,"ai-edit-migrate");CHECK(ai_panel_edit_number(s.ai_edit,"schema")==1);accepted(&s,"ai-edit-migrate");start=GetTickCount();do{pump(&s);if(s.ai_edit&&ai_panel_edit_number(s.ai_edit,"schema")==2)break;}while(GetTickCount()-start<5000);CHECK(phase(&s,UI_AI_EDIT_IDLE));CHECK(ai_panel_edit_number(snapshot,"schema")==2);CHECK(strstr(snapshot,"backup_path")&&strstr(snapshot,"storage_path"));
 /* Both independent binding structures consume the original shared 16 slots. */
 {uint64_t ids[16],rejected=0;char node[32];HWND parent=CreateWindowExW(0,L"STATIC",L"budget",0,0,0,100,100,NULL,NULL,GetModuleHandleW(NULL),NULL),button=CreateWindowExW(0,L"BUTTON",L"confirm",WS_CHILD,0,0,50,20,parent,NULL,GetModuleHandleW(NULL),NULL);char *binding=ai_panel_edit_string(s.ai_edit,"binding_hash");
  for(unsigned i=0;i<16;++i){ui_ai_edit_light_binding_t d={0};snprintf(node,32,"budget-%u",i);d.size=sizeof(d);d.version=1;d.view=s.view;d.control_id=node;d.action=UI_AI_EDIT_CONFIRM;d.binding_hash=binding;CHECK(ui_ai_edit_bind_light(s.ai,&d,&ids[i])==UI_STATUS_OK);}
  CHECK(ui_ai_edit_bind_button(s.ai,button,UI_AI_EDIT_CONFIRM,binding,&rejected)==UI_STATUS_LIMIT_EXCEEDED&&!rejected);for(unsigned i=0;i<16;++i)CHECK(ui_ai_edit_unbind_button(s.ai,ids[i])==UI_STATUS_OK);free(binding);DestroyWindow(parent);}
 open_path(&s,argv[1]);pump(&s);first=ui_workspace_active(s.workspace);open_path(&s,argv[1]);pump(&s);second=ui_workspace_active(s.workspace);CHECK(first&&second&&first!=second);CHECK(ui_workspace_activate(s.workspace,first)==UI_STATUS_OK);pump(&s);
 /* The production page gathers a proposal, not authority. */
 text(&s,"ai-edit-cap","counter.increment");text(&s,"ai-edit-resource","counter.document");text(&s,"ai-edit-params","{}");CHECK(ui_web_view_get_presentation(s.view,"ai-edit-cap",&p)==UI_STATUS_OK);CHECK(ui_web_view_get_presentation(s.view,"ai-edit-resource",&p)==UI_STATUS_OK);CHECK(ui_web_view_get_presentation(s.view,"ai-edit-schema",&p)==UI_STATUS_OK);CHECK(ui_web_view_get_presentation(s.view,"ai-edit-params",&p)==UI_STATUS_OK);click(&s,"ai-edit-propose");CHECK(phase(&s,UI_AI_EDIT_SCOPE_WAIT));if(failures)return 1;CHECK(count(&s,first)==0);CHECK(s.ai_edit_ack[0]);p.size=sizeof(p);CHECK(ui_web_view_get_presentation(s.view,"ai-edit-scope",&p)==UI_STATUS_OK&&strstr(p.text_utf8,"counter.increment")&&strstr(p.text_utf8,"counter.document"));
 hash=ai_panel_edit_string(s.ai_edit,"binding_hash");CHECK(hash!=NULL);action.action="ai-edit-confirm";process_action(&s,&action);CHECK(s.ai_error&&strstr(s.ai_error,"rejected"));click(&s,"ai-edit-confirm");CHECK(phase(&s,UI_AI_EDIT_SCOPE_WAIT));CHECK(count(&s,first)==0);CHECK(ui_workspace_activate(s.workspace,second)==UI_STATUS_OK);action.action="assistant";process_action(&s,&action);accepted(&s,"ai-edit-confirm");CHECK(ui_ai_edit_fixture_light(s.ai,s.view,"ai-edit-confirm")!=UI_STATUS_OK);CHECK(phase(&s,UI_AI_EDIT_DONE));CHECK(count(&s,first)==1&&count(&s,second)==0);CHECK(strstr(snapshot,"undo-")&&strstr(snapshot,"\"storage_confirmed\":true"));free(hash);hash=NULL;
 action.action="assistant";process_action(&s,&action);pump(&s);CHECK(ui_web_view_get_presentation(s.view,"ai-edit-result",&p)==UI_STATUS_OK&&strstr(p.text_utf8,"undo-")&&strstr(p.text_utf8,"revision"));accepted(&s,"ai-edit-undo");CHECK(phase(&s,UI_AI_EDIT_DONE));CHECK(count(&s,first)==0&&count(&s,second)==0);
 /* New invalid proposal preserves its input and cannot reuse old authority. */
 text(&s,"ai-edit-params","{broken");click(&s,"ai-edit-propose");CHECK(ui_web_view_get_presentation(s.view,"ai-edit-params",&p)==UI_STATUS_OK&&!strcmp(p.text_utf8,"{broken"));CHECK(count(&s,first)==0&&s.ai_error&&s.ai_error[0]);
 /* A final-store failure retains the committed business value and blocks writes. */
 propose(&s,first,"panel-unknown","counter.increment","counter.document");ui_ai_edit_fixture(s.ai,4,UI_AI_EDIT_READBACK);accepted(&s,"ai-edit-confirm");CHECK(phase(&s,UI_AI_EDIT_READBACK));ui_ai_edit_fixture(s.ai,2,4);ui_ai_edit_fixture(s.ai,4,0);CHECK(phase(&s,UI_AI_EDIT_UNKNOWN));CHECK(count(&s,first)==1);{char *operation=ai_panel_edit_string(snapshot,"id");CHECK(operation!=NULL);CHECK(ui_ai_edit_forget(s.ai,operation)==UI_STATUS_PENDING);CHECK(ui_ai_edit_cancel(s.ai,operation)==UI_STATUS_OK);free(operation);}accepted(&s,"ai-edit-check");CHECK(phase(&s,UI_AI_EDIT_DONE));CHECK(strstr(snapshot,"reconciled")&&count(&s,first)==1);
 /* Destructive clear requires a separate qualified action even with a task scope. */
 open_path(&s,argv[2]);pump(&s);{uint64_t eda=ui_workspace_active(s.workspace);propose(&s,eda,"panel-clear","eda.clear","eda.document");accepted(&s,"ai-edit-confirm");CHECK(phase(&s,UI_AI_EDIT_APPROVAL_WAIT));action.action="ai-edit-approve";process_action(&s,&action);CHECK(s.ai_error&&strstr(s.ai_error,"rejected"));click(&s,"ai-edit-approve");CHECK(phase(&s,UI_AI_EDIT_APPROVAL_WAIT));CHECK(ui_web_view_get_presentation(s.view,"ai-edit-scope",&p)==UI_STATUS_OK&&strstr(p.text_utf8,"eda.clear"));accepted(&s,"ai-edit-approve");CHECK(phase(&s,UI_AI_EDIT_DONE));CHECK(strstr(snapshot,"\"effect\":0"));}
 /* History can be inspected, but recovered entries are explicitly non-authority. */
 action.action="ai-history";action.command="runs";action.id="0";process_action(&s,&action);start=GetTickCount();while((!s.ai_runs||!strstr(s.ai_runs,"panel-unknown"))&&GetTickCount()-start<5000)pump(&s);CHECK(s.ai_runs&&strstr(s.ai_runs,"panel-unknown"));
 CHECK(ui_web_view_get_presentation(s.view,"ai-history-page",&p)==UI_STATUS_OK&&strstr(p.text_utf8,"展示"));CHECK(ui_web_view_get_presentation(s.view,"ai-edit-result-page",&p)==UI_STATUS_OK&&strstr(p.text_utf8,"展示"));CHECK(close_host(&s));CHECK(SetEnvironmentVariableW(L"UI_AI_DB_PATH",NULL));printf("P4 formal Light panel checks=%u failures=%u\n",checks,failures);return failures?1:0;
}


