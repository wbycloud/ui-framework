#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ai_edit_fixture_control.h"
#include "ui_framework/app_data.h"
#include "ui_framework/ai_edit_ui.h"
#include "ui_internal.h"
#include "yyjson.h"
#include "sqlite3.h"
__declspec(dllimport) ui_status_t ui_ai_edit_fixture_action(ui_ai_service_t *,int,const char *);
__declspec(dllimport) void ui_ai_edit_fixture(ui_ai_service_t *,int,uint64_t);
__declspec(dllimport) ui_status_t ui_ai_edit_fixture_manual(ui_host_t *,const char *,void *);
static int close_from_revision;static ui_status_t close_during_revision;
static ui_status_t reentry_status;static ui_ai_edit_request_t last_request;static char last_params[80];
static unsigned checks,failures;static ui_workspace_t *workspace;static ui_ai_service_t *service;
static int auto_migrate=1;static char override_path[4096];static const wchar_t *test_package,*test_directory;
static uint64_t a,b;static ui_app_instance_info_t app;static char id[65],dbpath[4096],state[524289],model[1024];
static const char constraint[]="{\"type\":\"object\",\"properties\":{\"value\":{\"type\":\"integer\",\"minimum\":0,\"maximum\":100}},\"required\":[\"value\"],\"additionalProperties\":false}";
enum {S_IDLE=0,S_SCOPE=1,S_APPROVAL=2,S_GATE=3,S_INTENT=4,S_BEGIN=5,S_EXECUTE=6,S_COMMIT=7,S_ROLLBACK=8,S_READBACK=9,S_RECORD=10,S_DONE=11,S_UNKNOWN=12};
#define CHECK(x) do{++checks;if(!(x)){++failures;fprintf(stderr,"line %d: %s\n",__LINE__,#x);}}while(0)
static void app_event(uint64_t instance,const char *type,const char *json,void *data)
{(void)data;if(instance==a&&!strcmp(type,"edit.reenter")){if(close_from_revision){CHECK(ui_ai_service_request_close(service)==UI_STATUS_OK);CHECK(ui_ai_service_poll(service,64)==UI_STATUS_OK);close_during_revision=ui_ai_service_destroy(service);return;}reentry_status=ui_ai_edit_propose(service,&last_request,id);return;}if(instance==a&&!strcmp(type,"edit.expire")){ui_ai_edit_fixture(service,3,1800000);return;}if(instance==a&&!strcmp(type,"edit.state")){CHECK(strlen(json)<sizeof(model));strcpy_s(model,sizeof(model),json);}}
static int number(const char *json,const char *key)
{yyjson_doc *d=yyjson_read(json,strlen(json),0);int n=d?(int)yyjson_get_int(yyjson_obj_get(yyjson_doc_get_root(d),key)):-999;yyjson_doc_free(d);return n;}
static void field(const char *json,const char *key,char *out,size_t cap)
{yyjson_doc *d=yyjson_read(json,strlen(json),0);const char *s=d?yyjson_get_str(yyjson_obj_get(yyjson_doc_get_root(d),key)):NULL;strcpy_s(out,cap,s?s:"");yyjson_doc_free(d);}
static void refresh(void)
{char *json=NULL;CHECK(ui_ai_edit_get_state(service,&json)==UI_STATUS_OK);if(json){CHECK(strlen(json)<sizeof(state));strcpy_s(state,sizeof(state),json);ui_ai_release(json);}}
static void pump(void)
{CHECK(ui_ai_service_poll(service,64)==UI_STATUS_OK);ui_workspace_poll(workspace);refresh();(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}
static int wait_phase(int expected)
{DWORD start=GetTickCount();do{pump();if(number(state,"phase")==expected)return 1;}while(GetTickCount()-start<5000);fprintf(stderr,"last state %.1500s\n",state);return 0;}
static int wait_code(const char *code)
{DWORD start=GetTickCount();char text[128];do{pump();field(state,"code",text,sizeof(text));if(!strcmp(text,code))return 1;}while(GetTickCount()-start<5000);fprintf(stderr,"last state %.1500s\n",state);return 0;}
static ui_status_t action(int which)
{char hash[65];refresh();field(state,"binding_hash",hash,sizeof(hash));return ui_ai_edit_fixture_action(service,which,hash);}
static void control(int mode)
{CHECK(ui_workspace_post_app_data(workspace,a,AE_APP_CONTROL,&mode,sizeof(mode))==UI_STATUS_OK);ui_workspace_poll(workspace);}
static int value(ui_host_t *host)
{ui_cap_target_t t={0};ui_cap_result_t r={0};uint64_t call=0;int n=-1;t.size=sizeof(t);t.version=1;CHECK(ui_capability_bind(host,"memory.document",&t)==UI_STATUS_OK);CHECK(ui_capability_prepare(host,"memory.read",&t,"{}",&call)==UI_STATUS_OK);CHECK(ui_capability_dispatch(host,call)==UI_STATUS_OK);r.size=sizeof(r);r.version=1;CHECK(ui_capability_get_result(host,call,&r)==UI_STATUS_OK&&r.state==UI_CAP_FINISHED&&r.outcome==UI_CAP_OK);if(r.output_json)n=number(r.output_json,"value");CHECK(ui_capability_forget(host,call)==UI_STATUS_OK);return n;}
static void target(ui_cap_target_v2_t *t)
{ui_cap_target_t read={0};memset(t,0,sizeof(*t));t->size=sizeof(*t);t->version=2;t->resource.size=sizeof(t->resource);t->resource.version=2;read.size=sizeof(read);read.version=1;CHECK(ui_capability_bind(app.host,"memory.document",&read)==UI_STATUS_OK);CHECK(ui_capability_resource_ref_v2(app.host,"memory.document",&t->resource)==UI_STATUS_OK);strcpy_s(t->expected_revision,128,read.expected_revision);strcpy_s(t->scope_json,sizeof(t->scope_json),"{\"access\":\"write\"}");}
static void propose(const char *client,const char *cap,int n)
{ui_ai_edit_request_t q={0};char params[80];q.size=sizeof(q);q.version=1;q.client_request_id=client;q.capability_id=cap;target(&q.target);q.parameter_constraint_schema=constraint;snprintf(params,sizeof(params),"{\"value\":%d}",n);q.params_json=params;last_request=q;strcpy_s(last_params,sizeof(last_params),params);last_request.params_json=last_params;{ui_status_t status=ui_ai_edit_propose(service,&q,id);if(status){char *description=NULL;ui_capability_describe_v2(app.host,cap,&description);fprintf(stderr,"propose status %d desc=%s\n",status,description?description:"null");ui_capability_release(description);}CHECK(status==UI_STATUS_OK);}CHECK(id[0]);CHECK(wait_phase(S_SCOPE));}
static void setup(const wchar_t *package,const wchar_t *directory,const wchar_t *name)
{
 wchar_t wide[4096];char path[4096];ui_workspace_config_t w={0};ui_ai_service_config_t c={0};ui_ai_service_state_t s={0};DWORD start;
 CHECK(CreateDirectoryW(directory,NULL)||GetLastError()==ERROR_ALREADY_EXISTS);CHECK(_snwprintf_s(wide,4096,_TRUNCATE,L"%s/%s-%lu.db",directory,name,GetCurrentProcessId())>=0);CHECK(WideCharToMultiByte(CP_UTF8,0,wide,-1,dbpath,sizeof(dbpath),NULL,NULL)>0);if(override_path[0])strcpy_s(dbpath,sizeof(dbpath),override_path);CHECK(WideCharToMultiByte(CP_UTF8,0,package,-1,path,sizeof(path),NULL,NULL)>0);
 w.size=sizeof(w);w.run_mode=UI_RUN_OFFSCREEN;w.max_permission=UI_ASSISTANT_PERMISSION_EDIT;w.event=app_event;workspace=ui_workspace_create(&w);CHECK(workspace!=NULL);{ui_status_t opened=ui_workspace_open(workspace,path,&a);CHECK(opened==UI_STATUS_OK);if(opened){fprintf(stderr,"fixture load: %s\n",ui_workspace_last_error(workspace));ExitProcess(1);}}CHECK(ui_workspace_open(workspace,path,&b)==UI_STATUS_OK);app.size=sizeof(app);CHECK(ui_workspace_get_instance(workspace,a,&app)==UI_STATUS_OK);
 c.size=sizeof(c);c.version=1;c.workspace=workspace;c.storage_path_utf8=dbpath;CHECK(ui_ai_service_create(&c,&service)==UI_STATUS_OK);CHECK(ui_ai_service_open(service)==UI_STATUS_OK);s.size=sizeof(s);s.version=1;start=GetTickCount();do{ui_ai_service_poll(service,64);ui_ai_service_get_state(service,&s);if(s.phase==UI_AI_READY&&!s.worker_alive)break;(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}while(GetTickCount()-start<5000);CHECK(s.phase==UI_AI_READY);
 CHECK(ui_ai_edit_enable(service)==UI_STATUS_OK);{DWORD begin=GetTickCount();char code[128];do{pump();field(state,"code",code,sizeof(code));if(code[0])break;}while(GetTickCount()-begin<5000);CHECK(code[0]);}
 if(number(state,"schema")==1&&auto_migrate){CHECK(action(UI_AI_EDIT_MIGRATE)==UI_STATUS_OK);CHECK(wait_code("ready"));}CHECK(number(state,"schema")==1||number(state,"schema")==2);
}
static void close_service(void)
{ui_ai_service_state_t s={0};DWORD start;ui_status_t status=UI_STATUS_PENDING;CHECK(ui_ai_service_request_close(service)==UI_STATUS_OK);s.size=sizeof(s);s.version=1;start=GetTickCount();do{pump();ui_ai_service_get_state(service,&s);if(s.phase==UI_AI_CLOSED&&!s.worker_alive){status=ui_ai_service_destroy(service);if(status==UI_STATUS_OK)break;}(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}while(GetTickCount()-start<5000);CHECK(status==UI_STATUS_OK);service=NULL;CHECK(ui_workspace_close_all(workspace)==UI_STATUS_OK);start=GetTickCount();do{ui_workspace_poll(workspace);if(!ui_workspace_count(workspace))break;(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}while(GetTickCount()-start<5000);CHECK(ui_workspace_count(workspace)==0);CHECK(ui_workspace_destroy(workspace)==UI_STATUS_OK);workspace=NULL;}
static int sql(const char *query)
{sqlite3 *db=NULL;sqlite3_stmt *q=NULL;int n=-1;CHECK(sqlite3_open_v2(dbpath,&db,SQLITE_OPEN_READONLY,NULL)==SQLITE_OK);CHECK(sqlite3_prepare_v2(db,query,-1,&q,NULL)==SQLITE_OK);if(q&&sqlite3_step(q)==SQLITE_ROW)n=sqlite3_column_int(q,0);sqlite3_finalize(q);if(db)CHECK(sqlite3_close(db)==SQLITE_OK);return n;}
static ui_status_t collect(const ui_cap_ui_context_t *context,char *json,size_t cap,size_t *length,void *data)
{(void)context;(void)data;strcpy_s(json,cap,"{\"value\":33}");*length=strlen(json);return UI_STATUS_OK;}
static void basic(void)
{
 uint64_t request=0,binding=0;ui_menu_item_desc_t menu={0};ui_cap_ui_binding_desc_t desc={0};ui_app_instance_info_t other={0};char hash[65];
 CHECK(value(app.host)==10);CHECK(ui_assistant_invoke(app.assistant,"memory.set","{\"value\":99}",&request)==UI_STATUS_PERMISSION_DENIED);CHECK(value(app.host)==10);
 menu.size=sizeof(menu);menu.id="manual.set";menu.title="Manual";menu.menu_path="Test";menu.command_id="memory.set";CHECK(ui_host_register_menu_item(app.host,&menu)==UI_STATUS_OK);desc.size=sizeof(desc);desc.version=1;desc.capability_id="memory.set";desc.menu_item_id="manual.set";desc.purpose=UI_CAP_UI_INVOKE;desc.resource.size=sizeof(desc.resource);desc.resource.version=2;CHECK(ui_capability_resource_ref_v2(app.host,"memory.document",&desc.resource)==UI_STATUS_OK);desc.collect=collect;CHECK(ui_capability_ui_bind(app.host,&desc,&binding)==UI_STATUS_OK);CHECK(ui_workspace_activate(workspace,a)==UI_STATUS_OK);CHECK(ui_ai_edit_fixture_manual(app.host,"manual.set",&binding)==UI_STATUS_OK);CHECK(value(app.host)==33);
 propose("basic","memory.set",40);refresh();field(state,"binding_hash",hash,sizeof(hash));CHECK(ui_ai_edit_fixture_action(service,UI_AI_EDIT_CONFIRM,"bad")==UI_STATUS_PERMISSION_DENIED);CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);CHECK(ui_ai_edit_fixture_action(service,UI_AI_EDIT_CONFIRM,hash)!=UI_STATUS_OK);CHECK(ui_workspace_activate(workspace,b)==UI_STATUS_OK);CHECK(wait_phase(S_DONE));CHECK(number(state,"effect")==UI_CAP2_EFFECT_APPLIED&&strstr(state,"txn-1")&&strstr(state,"undo-1"));CHECK(value(app.host)==40);other.size=sizeof(other);CHECK(ui_workspace_get_instance(workspace,b,&other)==UI_STATUS_OK);CHECK(value(other.host)==10);
 CHECK(ui_ai_edit_forget(service,id)==UI_STATUS_OK);CHECK(ui_ai_edit_propose(service,&last_request,id)==UI_STATUS_OK);CHECK(wait_phase(S_DONE));CHECK(value(app.host)==40);CHECK(action(UI_AI_EDIT_UNDO)==UI_STATUS_OK);CHECK(wait_phase(S_DONE));CHECK(value(app.host)==33);CHECK(number(state,"effect")==UI_CAP2_EFFECT_APPLIED);last_request.params_json="{\"value\":41}";CHECK(ui_ai_edit_propose(service,&last_request,id)==UI_STATUS_OK);CHECK(wait_code("request_id_conflict"));CHECK(value(app.host)==33);close_service();CHECK(sql("SELECT count(*) FROM events WHERE terminal=1")==2);CHECK(sql("SELECT count(*) FROM ai_edit_consumptions")==2);
}
static void policy(void)
{
 ui_ai_edit_request_t q={0};char duplicate[65];uint64_t binding;HWND parent,button;
 q.size=sizeof(q);q.version=1;q.client_request_id="bad";q.capability_id="memory.set";q.parameter_constraint_schema=constraint;q.params_json="{\"value\":101}";target(&q.target);CHECK(ui_ai_edit_propose(service,&q,id)==UI_STATUS_VALIDATION_FAILED);q.params_json="{\"value\":1,\"source\":\"human\"}";CHECK(ui_ai_edit_propose(service,&q,id)==UI_STATUS_VALIDATION_FAILED);q.params_json="{\"value\":1}";q.capability_id="memory.read";CHECK(ui_ai_edit_propose(service,&q,id)!=UI_STATUS_OK);
 propose("expires","memory.set",41);q.client_request_id="expires";q.capability_id="memory.set";q.params_json="{\"value\":41}";target(&q.target);CHECK(ui_ai_edit_propose(service,&q,duplicate)==UI_STATUS_OK&&!strcmp(duplicate,id));q.params_json="{\"value\":42}";CHECK(ui_ai_edit_propose(service,&q,duplicate)==UI_STATUS_ALREADY_EXISTS);
 parent=CreateWindowExW(0,L"STATIC",L"Host scope",WS_OVERLAPPED|WS_VISIBLE,0,0,200,200,NULL,NULL,GetModuleHandleW(NULL),NULL);button=CreateWindowExW(0,L"BUTTON",L"Confirm displayed task",WS_CHILD|WS_VISIBLE,0,0,120,30,parent,NULL,GetModuleHandleW(NULL),NULL);{char hash[65];field(state,"binding_hash",hash,sizeof(hash));CHECK(ui_ai_edit_bind_button(service,parent,UI_AI_EDIT_CONFIRM,hash,&binding)==UI_STATUS_INVALID_ARGUMENT);CHECK(ui_ai_edit_bind_button(service,button,UI_AI_EDIT_CONFIRM,"0000000000000000000000000000000000000000000000000000000000000000",&binding)==UI_STATUS_PERMISSION_DENIED);CHECK(ui_ai_edit_bind_button(service,button,UI_AI_EDIT_CONFIRM,hash,&binding)==UI_STATUS_OK);}CHECK(IsWindowVisible(button));SendMessageW(button,BM_CLICK,0,0);SendMessageW(button,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(5,5));SendMessageW(button,WM_LBUTTONUP,0,MAKELPARAM(5,5));pump();CHECK(number(state,"phase")==S_SCOPE);CHECK(ui_ai_edit_unbind_button(service,binding)==UI_STATUS_OK);DestroyWindow(parent);
 ui_ai_edit_fixture(service,1,1);CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);ui_ai_edit_fixture(service,3,1800000);ui_ai_edit_fixture(service,1,0);CHECK(wait_phase(S_DONE));CHECK(strstr(state,"authorization_expired")&&value(app.host)==10);CHECK(ui_ai_edit_forget(service,id)==UI_STATUS_OK);
 propose("destructive","memory.destructive",42);CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);CHECK(wait_phase(S_APPROVAL));CHECK(action(UI_AI_EDIT_APPROVE)==UI_STATUS_OK);ui_ai_edit_fixture(service,3,300000);CHECK(wait_phase(S_DONE));CHECK(value(app.host)==10);CHECK(ui_ai_edit_forget(service,id)==UI_STATUS_OK);
 for(unsigned i=0;i<3;++i){const char *caps[]={"memory.destructive","memory.external","memory.irreversible"};char client[32];snprintf(client,sizeof(client),"risk-%u",i);propose(client,caps[i],43);CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);CHECK(wait_phase(S_APPROVAL));CHECK(action(UI_AI_EDIT_APPROVE)==UI_STATUS_OK);CHECK(wait_phase(S_DONE));CHECK(strstr(state,"effect_not_enabled")&&value(app.host)==10);CHECK(ui_ai_edit_forget(service,id)==UI_STATUS_OK);}
 control(AE_MODE_REVISION_CHECK);propose("revision","memory.set",44);CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);CHECK(wait_phase(S_DONE));CHECK(value(app.host)==10);CHECK(ui_ai_edit_forget(service,id)==UI_STATUS_OK);
 control(AE_MODE_DISABLE_CHECK);propose("disabled","memory.set",45);CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);CHECK(wait_phase(S_DONE));CHECK(value(app.host)==10);close_service();CHECK(sql("SELECT count(*) FROM ai_edit_consumptions WHERE kind='approval'")==3);
}
static void cancellation(void)
{
 propose("cancel-before","memory.set",31);CHECK(ui_ai_edit_cancel(service,id)==UI_STATUS_OK);CHECK(wait_phase(S_DONE));CHECK(value(app.host)==10);CHECK(ui_ai_edit_forget(service,id)==UI_STATUS_OK);
 propose("cancel-executed","memory.set",32);CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);CHECK(wait_phase(S_COMMIT));CHECK(ui_ai_edit_cancel(service,id)==UI_STATUS_OK);CHECK(wait_phase(S_DONE));CHECK(value(app.host)==10&&strstr(state,"rolled_back"));CHECK(ui_ai_edit_forget(service,id)==UI_STATUS_OK);
 propose("cancel-committed","memory.set",33);CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);CHECK(wait_phase(S_READBACK));CHECK(ui_ai_edit_cancel(service,id)==UI_STATUS_OK);CHECK(wait_phase(S_DONE));CHECK(value(app.host)==33&&number(state,"effect")==UI_CAP2_EFFECT_APPLIED);close_service();CHECK(sql("SELECT count(*) FROM events WHERE terminal=1")==3);
}
static void unknown_case(void)
{
 control(AE_MODE_ROLLBACK_FAIL);propose("rollback-unknown","memory.set",30);CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);CHECK(wait_phase(S_UNKNOWN));CHECK(value(app.host)==77);CHECK(ui_ai_edit_forget(service,id)==UI_STATUS_PENDING);CHECK(ui_ai_edit_cancel(service,id)==UI_STATUS_OK);CHECK(number(state,"effect")==UI_CAP2_EFFECT_UNKNOWN);
 CHECK(ui_capability_begin_unregistration(app.host,"memory.set")==UI_STATUS_OK);CHECK(ui_capability_finish_unregistration(app.host,"memory.set")==UI_STATUS_PENDING);CHECK(ui_capability_begin_resource_unregistration(app.host,"memory.document")==UI_STATUS_OK);CHECK(ui_capability_finish_resource_unregistration(app.host,"memory.document")==UI_STATUS_PENDING);
 control(AE_MODE_SYNC);CHECK(action(UI_AI_EDIT_RECONCILE)==UI_STATUS_OK);CHECK(wait_phase(S_DONE));CHECK(strstr(state,"reconciled"));CHECK(ui_workspace_post_app_data(workspace,a,AE_APP_SNAPSHOT,NULL,0)==UI_STATUS_OK);ui_workspace_poll(workspace);CHECK(number(model,"value")==77);close_service();CHECK(sql("SELECT count(*) FROM ai_edits WHERE unknown=1")==0);CHECK(sql("SELECT count(*) FROM events WHERE kind='edit_reconciled'")==1);
}
static void lifecycle(void)
{
 uint64_t old=a;unsigned char filler[4096]={0};ui_status_t status;unsigned filled=0;
 control(AE_MODE_ASYNC);propose("async-close","memory.set",55);CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);CHECK(wait_phase(S_EXECUTE));
 {DWORD start=GetTickCount();do{CHECK(ui_workspace_post_app_data(workspace,a,AE_APP_SNAPSHOT,NULL,0)==UI_STATUS_OK);pump();if(number(model,"executes")==1)break;}while(GetTickCount()-start<5000);CHECK(number(model,"executes")==1);}
 do{status=ui_workspace_post_app_data(workspace,a,AE_APP_SNAPSHOT,filler,sizeof(filler));if(!status)++filled;}while(!status);CHECK(filled>0&&status==UI_STATUS_LIMIT_EXCEEDED);CHECK(ui_workspace_post_app_data(workspace,a,AE_APP_WORKER_EXIT,filler,sizeof(filler))==UI_STATUS_LIMIT_EXCEEDED);ui_workspace_poll(workspace);
 CHECK(ui_ai_edit_cancel(service,id)==UI_STATUS_OK);CHECK(ui_workspace_close(workspace,a,UI_APP_CLOSE_TAB)==UI_STATUS_OK);CHECK(ui_ai_service_request_close(service)==UI_STATUS_OK);CHECK(ui_ai_service_destroy(service)==UI_STATUS_PENDING);pump();CHECK(ui_workspace_count(workspace)==2&&GetModuleHandleW(L"ui_ai_edit_fixture.dll")!=NULL);CHECK(ui_workspace_post_app_data(workspace,a,AE_APP_WORKER_EXIT,NULL,0)==UI_STATUS_OK);
 {DWORD start=GetTickCount();do{pump();if(number(model,"worker_done"))break;CHECK(ui_workspace_post_app_data(workspace,a,AE_APP_SNAPSHOT,NULL,0)==UI_STATUS_OK);}while(GetTickCount()-start<5000);CHECK(number(model,"worker_done")&&!number(model,"callback_done"));CHECK(number(state,"phase")!=S_DONE);}
 CHECK(ui_workspace_post_app_data(workspace,a,AE_APP_CALLBACK_EXIT,NULL,0)==UI_STATUS_OK);CHECK(wait_phase(S_DONE));CHECK(ui_workspace_post_app_data(workspace,old,AE_APP_READY,NULL,0)==UI_STATUS_CANCELLED);close_service();
}

static int backup_version(const char *path)
{sqlite3 *db=NULL;sqlite3_stmt *q=NULL;int v=-1;CHECK(sqlite3_open_v2(path,&db,SQLITE_OPEN_READONLY,NULL)==SQLITE_OK);CHECK(sqlite3_prepare_v2(db,"PRAGMA user_version",-1,&q,NULL)==SQLITE_OK);if(q&&sqlite3_step(q)==SQLITE_ROW)v=sqlite3_column_int(q,0);sqlite3_finalize(q);if(db)sqlite3_close(db);return v;}
static void storage_case(void)
{
 char backup[4096],original[4096];ui_ai_edit_request_t q={0};char out[65];
 CHECK(number(state,"schema")==1);ui_ai_edit_fixture(service,2,1);CHECK(action(UI_AI_EDIT_MIGRATE)==UI_STATUS_OK);CHECK(wait_code("migration_or_backup_failed"));CHECK(number(state,"schema")==1);
 CHECK(ui_ai_edit_enable(service)==UI_STATUS_OK);CHECK(wait_code("migration_confirmation_required"));ui_ai_edit_fixture(service,2,6);CHECK(action(UI_AI_EDIT_MIGRATE)==UI_STATUS_OK);CHECK(wait_code("migration_or_backup_failed"));CHECK(number(state,"schema")==1);field(state,"backup_path",backup,sizeof(backup));CHECK(backup_version(backup)==99);
 CHECK(ui_ai_edit_enable(service)==UI_STATUS_OK);CHECK(wait_code("migration_confirmation_required"));ui_ai_edit_fixture(service,2,2);CHECK(action(UI_AI_EDIT_MIGRATE)==UI_STATUS_OK);CHECK(wait_code("migration_or_backup_failed"));field(state,"backup_path",backup,sizeof(backup));CHECK(number(state,"schema")==1&&backup_version(backup)==1);
 CHECK(ui_ai_edit_enable(service)==UI_STATUS_OK);CHECK(wait_code("migration_confirmation_required"));CHECK(action(UI_AI_EDIT_MIGRATE)==UI_STATUS_OK);CHECK(wait_code("ready"));CHECK(number(state,"schema")==2);
 propose("intent-unconfirmed","memory.set",65);CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);CHECK(wait_phase(S_GATE));ui_ai_edit_fixture(service,2,3);CHECK(wait_phase(S_UNKNOWN));CHECK(value(app.host)==10);CHECK(ui_ai_edit_forget(service,id)==UI_STATUS_PENDING);strcpy_s(original,sizeof(original),dbpath);close_service();CHECK(sql("SELECT count(*) FROM ai_edit_consumptions")==0);CHECK(sql("SELECT count(*) FROM runs WHERE terminal=0")==1);
 auto_migrate=1;strcpy_s(override_path,sizeof(override_path),original);setup(test_package,test_directory,L"storage-reopen");CHECK(number(state,"schema")==2&&number(state,"recovered_unresolved")==1);CHECK(value(app.host)==10);q.size=sizeof(q);q.version=1;q.client_request_id="must-not-replay";q.capability_id="memory.set";target(&q.target);q.parameter_constraint_schema=constraint;q.params_json="{\"value\":99}";CHECK(ui_ai_edit_propose(service,&q,out)==UI_STATUS_PENDING);close_service();override_path[0]=0;
}
static void result_storage_case(int fault)
{
 propose("result-unconfirmed","memory.set",66);CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);CHECK(wait_phase(S_READBACK));CHECK(value(app.host)==66);ui_ai_edit_fixture(service,2,(uint64_t)fault);CHECK(wait_phase(S_UNKNOWN));CHECK(value(app.host)==66);CHECK(ui_ai_edit_forget(service,id)==UI_STATUS_PENDING);CHECK(ui_ai_edit_cancel(service,id)==UI_STATUS_OK);
 if(fault==7){CHECK(action(UI_AI_EDIT_RECONCILE)==UI_STATUS_OK);CHECK(wait_phase(S_DONE));CHECK(value(app.host)==66);}close_service();CHECK(sql("SELECT count(*) FROM ai_edit_consumptions")==1);CHECK(sql("SELECT count(*) FROM events WHERE terminal=1")== (fault==7?1:0));
}
static void output_case(void)
{control(AE_MODE_INVALID_OUTPUT);propose("invalid-readback","memory.set",67);CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);CHECK(wait_phase(S_UNKNOWN));CHECK(value(app.host)==67);control(AE_MODE_SYNC);CHECK(action(UI_AI_EDIT_RECONCILE)==UI_STATUS_OK);CHECK(wait_phase(S_DONE));CHECK(value(app.host)==67);close_service();}
static void seed_case(const wchar_t *which)
{
 int expected=!wcscmp(which,L"seed-scope")?S_SCOPE:!wcscmp(which,L"seed-intent")?14:!wcscmp(which,L"seed-commit")?S_READBACK:S_DONE;
 if(expected!=S_DONE&&expected!=S_SCOPE)ui_ai_edit_fixture(service,4,(uint64_t)expected);
 propose("interrupted","memory.set",68);if(expected!=S_SCOPE){CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);CHECK(wait_phase(expected));}
 if(expected==S_READBACK||expected==S_DONE)CHECK(value(app.host)==68);else CHECK(value(app.host)==10);
 /* Every selected boundary follows the durable job acknowledgment. No restart
    operation is replayed, and no user process/database is involved. */
 ExitProcess(failures?1:0);
}
static void restart_case(const wchar_t *which)
{
 wchar_t executable[4096],path[4096],command[14000],seed[80];char original[4096];STARTUPINFOW startup={0};PROCESS_INFORMATION process={0};DWORD start,code=99;
 const wchar_t *suffix=which+8;CHECK(_snwprintf_s(seed,80,_TRUNCATE,L"seed-%s",suffix)>=0);CHECK(_snwprintf_s(path,4096,_TRUNCATE,L"%s/restart-%s-%lu.db",test_directory,suffix,GetCurrentProcessId())>=0);CHECK(WideCharToMultiByte(CP_UTF8,0,path,-1,original,sizeof(original),NULL,NULL)>0);
 close_service();CHECK(GetModuleFileNameW(NULL,executable,4096)>0);CHECK(_snwprintf_s(command,14000,_TRUNCATE,L"\"%s\" \"%s\" \"%s\" %s \"%s\"",executable,test_package,test_directory,seed,path)>=0);startup.cb=sizeof(startup);CHECK(CreateProcessW(executable,command,NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&startup,&process));if(!process.hProcess)return;start=GetTickCount();while(WaitForSingleObject(process.hProcess,0)!=WAIT_OBJECT_0&&GetTickCount()-start<7000)(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);CHECK(GetExitCodeProcess(process.hProcess,&code)&&code==0);CloseHandle(process.hProcess);CloseHandle(process.hThread);
 strcpy_s(override_path,sizeof(override_path),original);setup(test_package,test_directory,L"restart-read");CHECK(value(app.host)==10);CHECK(number(state,"recovered_unresolved")==(!wcscmp(suffix,L"final")?0:1));{ui_ai_service_state_t s={0};s.size=sizeof(s);s.version=1;CHECK(ui_ai_service_get_state(service,&s)==UI_STATUS_OK&&!s.active_run_id[0]&&s.queued==0);}close_service();CHECK(sql("SELECT count(*) FROM ai_edit_consumptions")==(!wcscmp(suffix,L"scope")?0:1));CHECK(sql("SELECT count(*) FROM events WHERE terminal=1")==(!wcscmp(suffix,L"final")?1:0));override_path[0]=0;
}


static void boundaries(void)
{
 uint64_t binding=0,old;ui_menu_item_desc_t menu={0};ui_cap_ui_binding_desc_t d={0};ui_command_state_t command={0};char package[4096];
 propose("last-modal","memory.set",50);CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);CHECK(wait_phase(S_BEGIN));control(AE_MODE_LAST_MODAL);CHECK(wait_phase(S_DONE));CHECK(number(state,"effect")==UI_CAP2_EFFECT_NONE);app.host->modal_component=NULL;CHECK(value(app.host)==10&&number(model,"executes")==0);CHECK(ui_ai_edit_forget(service,id)==UI_STATUS_OK);
 propose("last-expiry","memory.set",51);CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);CHECK(wait_phase(S_BEGIN));control(AE_MODE_LAST_EXPIRY);CHECK(wait_phase(S_DONE));CHECK(value(app.host)==10&&number(model,"executes")==0);CHECK(ui_ai_edit_forget(service,id)==UI_STATUS_OK);
 propose("last-busy","memory.set",52);CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);CHECK(wait_phase(S_BEGIN));control(AE_MODE_BUSY_CHECK);CHECK(wait_phase(S_DONE));command.size=sizeof(command);command.enabled=command.visible=1;CHECK(ui_host_set_command_state(app.host,"memory.set",&command)==UI_STATUS_OK);CHECK(value(app.host)==10&&number(model,"executes")==0);CHECK(ui_ai_edit_forget(service,id)==UI_STATUS_OK);
 menu.size=sizeof(menu);menu.id="manual.busy";menu.title="Manual busy";menu.menu_path="Test";menu.command_id="memory.set";CHECK(ui_host_register_menu_item(app.host,&menu)==UI_STATUS_OK);d.size=sizeof(d);d.version=1;d.capability_id="memory.set";d.menu_item_id="manual.busy";d.purpose=UI_CAP_UI_INVOKE;d.resource.size=sizeof(d.resource);d.resource.version=2;CHECK(ui_capability_resource_ref_v2(app.host,"memory.document",&d.resource)==UI_STATUS_OK);d.collect=collect;CHECK(ui_capability_ui_bind(app.host,&d,&binding)==UI_STATUS_OK);CHECK(ui_workspace_activate(workspace,a)==UI_STATUS_OK);
 propose("intent-hold","memory.set",53);CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);CHECK(wait_phase(S_GATE));ui_ai_edit_fixture(service,1,1);CHECK(wait_phase(S_INTENT));CHECK(value(app.host)==10);CHECK(ui_workspace_post_app_data(workspace,a,AE_APP_SNAPSHOT,NULL,0)==UI_STATUS_OK);ui_workspace_poll(workspace);CHECK(number(model,"executes")==0);CHECK(ui_ai_edit_fixture_manual(app.host,"manual.busy",&binding)==UI_STATUS_PERMISSION_DENIED);CHECK(value(app.host)==10);ui_ai_edit_fixture(service,1,0);CHECK(wait_phase(S_DONE));CHECK(value(app.host)==53);CHECK(ui_ai_edit_forget(service,id)==UI_STATUS_OK);
 propose("old-open","memory.set",54);old=a;CHECK(ui_workspace_close(workspace,a,UI_APP_CLOSE_TAB)==UI_STATUS_OK);ui_workspace_poll(workspace);CHECK(ui_workspace_count(workspace)==1);CHECK(WideCharToMultiByte(CP_UTF8,0,test_package,-1,package,sizeof(package),NULL,NULL)>0);CHECK(ui_workspace_open(workspace,package,&a)==UI_STATUS_OK&&a!=old);app.size=sizeof(app);CHECK(ui_workspace_get_instance(workspace,a,&app)==UI_STATUS_OK);CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);CHECK(wait_phase(S_DONE));CHECK(strstr(state,"target_expired")&&value(app.host)==10);close_service();
}
static void sync_retire(void)
{CHECK(ui_workspace_close(workspace,b,UI_APP_CLOSE_TAB)==UI_STATUS_OK);ui_workspace_poll(workspace);CHECK(ui_workspace_count(workspace)==1);control(AE_MODE_SYNC_RETIRE);propose("sync-retire","memory.set",69);CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);CHECK(wait_phase(S_DONE));CHECK(strstr(state,"rolled_back")&&number(model,"value")==10&&number(model,"rollbacks")==1);CHECK(ui_workspace_count(workspace)==0&&GetModuleHandleW(L"ui_ai_edit_fixture.dll")==NULL);close_service();}

static void large_scope(void)
{
 ui_ai_edit_request_t q={0};char *schema=malloc(65000);size_t used;CHECK(schema!=NULL);if(!schema){close_service();return;}
 strcpy_s(schema,65000,"{\"description\":\"");used=strlen(schema);memset(schema+used,'x',63000);used+=63000;strcpy_s(schema+used,65000-used,"\",\"type\":\"object\",\"properties\":{\"value\":{\"type\":\"integer\"}},\"required\":[\"value\"],\"additionalProperties\":false}");
 q.size=sizeof(q);q.version=1;q.client_request_id="large-range";q.capability_id="memory.set";target(&q.target);q.parameter_constraint_schema=schema;q.params_json="{\"value\":49}";CHECK(ui_ai_edit_propose(service,&q,id)==UI_STATUS_OK);free(schema);CHECK(wait_phase(S_SCOPE));CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);CHECK(wait_phase(S_DONE));CHECK(value(app.host)==49);close_service();
}

static void intent_ack(void)
{propose("intent-ack-lost","memory.set",64);CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);CHECK(wait_phase(S_GATE));ui_ai_edit_fixture(service,2,8);CHECK(wait_phase(S_UNKNOWN));CHECK(value(app.host)==10);CHECK(ui_workspace_post_app_data(workspace,a,AE_APP_SNAPSHOT,NULL,0)==UI_STATUS_OK);ui_workspace_poll(workspace);CHECK(number(model,"executes")==0);CHECK(ui_ai_edit_forget(service,id)==UI_STATUS_PENDING);CHECK(action(UI_AI_EDIT_RECONCILE)==UI_STATUS_OK);CHECK(wait_phase(S_DONE));CHECK(value(app.host)==10);CHECK(ui_ai_edit_forget(service,id)==UI_STATUS_OK);propose("after-proof","memory.set",65);CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);CHECK(wait_phase(S_DONE));CHECK(value(app.host)==65);close_service();CHECK(sql("SELECT count(*) FROM events WHERE terminal=1")==2);CHECK(sql("SELECT count(*) FROM ai_edit_consumptions")==2);}

static void receipt_case(void)
{control(AE_MODE_BAD_RECEIPT);propose("receipt-conflict","memory.set",70);CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);CHECK(wait_phase(S_UNKNOWN));CHECK(value(app.host)==70);CHECK(ui_ai_edit_forget(service,id)==UI_STATUS_PENDING);control(AE_MODE_BAD_PROOF);CHECK(action(UI_AI_EDIT_RECONCILE)==UI_STATUS_OK);CHECK(wait_code("reconciliation_failed"));CHECK(number(state,"phase")==S_UNKNOWN&&ui_ai_edit_forget(service,id)==UI_STATUS_PENDING);control(AE_MODE_SYNC);CHECK(action(UI_AI_EDIT_RECONCILE)==UI_STATUS_OK);CHECK(wait_phase(S_DONE));CHECK(value(app.host)==70);close_service();CHECK(sql("SELECT count(*) FROM events WHERE terminal=1")==1);}

static void receipt_backpressure(void)
{
 ui_ai_edit_request_t q={0};q.size=sizeof(q);q.version=1;q.client_request_id="receipt-backpressure";q.capability_id="memory.set";q.parameter_constraint_schema=constraint;q.params_json="{\"value\":55}";target(&q.target);
 ui_ai_edit_fixture(service,2,9);CHECK(ui_ai_edit_propose(service,&q,id)==UI_STATUS_LIMIT_EXCEEDED&&!id[0]);refresh();CHECK(number(state,"phase")==S_IDLE&&value(app.host)==10);
 CHECK(ui_ai_edit_propose(service,&q,id)==UI_STATUS_OK);CHECK(wait_phase(S_SCOPE));ui_ai_edit_fixture(service,2,9);CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_LIMIT_EXCEEDED);CHECK(value(app.host)==10);CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);CHECK(wait_phase(S_DONE));CHECK(value(app.host)==55);close_service();CHECK(sql("SELECT count(*) FROM events WHERE terminal=1")==1);CHECK(sql("SELECT count(*) FROM ai_edit_consumptions")==1);
}

static void undo_gate(void)
{
 propose("undo-modal","memory.set",59);CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);CHECK(wait_phase(S_DONE));CHECK(value(app.host)==59);CHECK(action(UI_AI_EDIT_UNDO)==UI_STATUS_OK);CHECK(wait_phase(S_EXECUTE));app.host->modal_component=(ui_component_t *)(uintptr_t)1;CHECK(wait_phase(S_DONE));app.host->modal_component=NULL;CHECK(value(app.host)==59&&number(model,"undos")==0);field(state,"id",id,sizeof(id));CHECK(ui_ai_edit_forget(service,id)==UI_STATUS_OK);
 propose("undo-revision","memory.set",60);CHECK(action(UI_AI_EDIT_CONFIRM)==UI_STATUS_OK);CHECK(wait_phase(S_DONE));CHECK(value(app.host)==60);CHECK(action(UI_AI_EDIT_UNDO)==UI_STATUS_OK);CHECK(wait_phase(S_EXECUTE));control(AE_MODE_REVISION_CHECK);CHECK(wait_phase(S_DONE));CHECK(value(app.host)==60&&number(model,"undos")==0);close_service();CHECK(sql("SELECT count(*) FROM events WHERE terminal=1")==4);
}

static void reentry(void)
{
 ui_ai_task_request_t read={0};ui_cap_target_t t={0};char read_id[65];memset(&last_request,0,sizeof(last_request));last_request.size=sizeof(last_request);last_request.version=1;last_request.client_request_id="reentrant-edit";last_request.capability_id="memory.set";last_request.parameter_constraint_schema=constraint;last_request.params_json="{\"value\":51}";target(&last_request.target);
 t.size=sizeof(t);t.version=1;CHECK(ui_capability_bind(app.host,"memory.document",&t)==UI_STATUS_OK);read.size=sizeof(read);read.version=1;read.client_request_id="continue-reentry";read.input_utf8="read-only continuation";read.stub_plan_json="{\"steps\":[]}";read.targets=&t;read.target_count=1;
 ui_ai_edit_fixture(service,1,1);control(AE_MODE_REENTER_PROPOSE);CHECK(ui_ai_continue(service,"ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",&read,read_id)==UI_STATUS_LIMIT_EXCEEDED);CHECK(reentry_status==UI_STATUS_OK&&!read_id[0]);ui_ai_edit_fixture(service,1,0);CHECK(wait_phase(S_SCOPE));CHECK(value(app.host)==10);CHECK(ui_ai_edit_cancel(service,id)==UI_STATUS_OK);CHECK(wait_phase(S_DONE));CHECK(value(app.host)==10);close_service();
}

static void target_close(void)
{ui_cap_target_v2_t t={0};t.size=sizeof(t);t.version=2;close_from_revision=1;control(AE_MODE_REENTER_PROPOSE);CHECK(ui_ai_edit_bind_target(service,a,"memory.document",&t)==UI_STATUS_CANCELLED);CHECK(close_during_revision==UI_STATUS_PENDING);CHECK(t.resource.instance_id==0);close_from_revision=0;close_service();}

int wmain(int argc,wchar_t **argv)
{
 if(argc!=4&&argc!=5)return 2;test_package=argv[1];test_directory=argv[2];auto_migrate=wcscmp(argv[3],L"storage")!=0;if(argc==5)CHECK(WideCharToMultiByte(CP_UTF8,0,argv[4],-1,override_path,sizeof(override_path),NULL,NULL)>0);setup(argv[1],argv[2],argv[3]);if(!wcsncmp(argv[3],L"seed-",5))seed_case(argv[3]);
 if(!wcscmp(argv[3],L"basic"))basic();else if(!wcscmp(argv[3],L"policy"))policy();else if(!wcscmp(argv[3],L"cancellation"))cancellation();else if(!wcscmp(argv[3],L"unknown"))unknown_case();else if(!wcscmp(argv[3],L"lifecycle"))lifecycle();else if(!wcscmp(argv[3],L"storage"))storage_case();else if(!wcscmp(argv[3],L"storage-result"))result_storage_case(4);else if(!wcscmp(argv[3],L"storage-ack"))result_storage_case(7);else if(!wcscmp(argv[3],L"output"))output_case();else if(!wcsncmp(argv[3],L"restart-",8))restart_case(argv[3]);else if(!wcscmp(argv[3],L"boundaries"))boundaries();else if(!wcscmp(argv[3],L"sync-retire"))sync_retire();else if(!wcscmp(argv[3],L"large-scope"))large_scope();else if(!wcscmp(argv[3],L"intent-ack"))intent_ack();else if(!wcscmp(argv[3],L"receipt"))receipt_case();else if(!wcscmp(argv[3],L"receipt-backpressure"))receipt_backpressure();else if(!wcscmp(argv[3],L"undo-gate"))undo_gate();else if(!wcscmp(argv[3],L"reentry"))reentry();else if(!wcscmp(argv[3],L"target-close"))target_close();else close_service();
 printf("P4-A %ls checks=%u failures=%u\n",argv[3],checks,failures);return failures?1:0;
}
