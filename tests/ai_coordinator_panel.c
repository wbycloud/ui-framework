#define wmain previous_panel_fixture_main
#include "ai_edit_panel.c"
#undef wmain
__declspec(dllimport) ui_status_t ui_ai_graph_fixture_light(ui_ai_service_t *,ui_web_view_t *,const char *);
static int projected(host_window_t *s)
{DWORD begin=GetTickCount();do{pump(s);if(s->ai_projection_generation==s->ai_projection_rendered)return 1;}while(GetTickCount()-begin<5000);return 0;}
static int graph_ready(host_window_t *s,int schema)
{DWORD begin=GetTickCount();do{pump(s);if(s->ai_coordinator&&ai_panel_edit_number(s->ai_coordinator,"schema")==schema&&!strstr(s->ai_coordinator,"\"pending\":true"))return 1;}while(GetTickCount()-begin<5000);return 0;}
static int graph_status(host_window_t *s,const char *fragment)
{DWORD begin=GetTickCount();do{pump(s);if(s->ai_graph&&strstr(s->ai_graph,fragment))return 1;}while(GetTickCount()-begin<5000);fprintf(stderr,"panel graph %.3000s error %s\n",s->ai_graph?s->ai_graph:"none",s->ai_error?s->ai_error:"");return 0;}
static int graph_panel_fixture_run(int argc,wchar_t **argv)
{
 host_window_t s={0};host_action_t a={0};GUID guid;wchar_t unique[40],db[4096];char target[300];uint64_t instance;DWORD begin;ui_element_presentation_t p={0};
 if(argc!=3)return 2;CHECK(CreateDirectoryW(argv[2],NULL)||GetLastError()==ERROR_ALREADY_EXISTS);CHECK(CoCreateGuid(&guid)==S_OK);CHECK(StringFromGUID2(&guid,unique,40)>0);_snwprintf_s(db,4096,_TRUNCATE,L"%s/p5-panel-%s.db",argv[2],unique);SetEnvironmentVariableW(L"UI_AI_DB_PATH",db);CHECK(ui_framework_initialize()==UI_STATUS_OK);CHECK(create_host(&s,GetModuleHandleW(NULL))!=NULL);if(!s.ai)return 1;
 begin=GetTickCount();do{pump(&s);if(s.ai_edit&&strstr(s.ai_edit,"migration_confirmation_required"))break;}while(GetTickCount()-begin<5000);CHECK(projected(&s));CHECK(ui_ai_edit_fixture_light(s.ai,s.view,"ai-edit-migrate")==UI_STATUS_OK);CHECK(phase(&s,UI_AI_EDIT_IDLE));CHECK(graph_ready(&s,2));
 a.action="ai-graph-migrate";process_action(&s,&a);CHECK(s.ai_error&&strstr(s.ai_error,"rejected"));CHECK(graph_ready(&s,2));CHECK(projected(&s));CHECK(s.ai_graph_bindings[3]!=0);CHECK(ui_ai_graph_fixture_light(s.ai,s.view,"ai-graph-migrate")==UI_STATUS_OK);CHECK(graph_ready(&s,3));
 open_path(&s,argv[1]);pump(&s);instance=ui_workspace_active(s.workspace);CHECK(instance!=0);
#ifdef UI_P6_PANEL
 CHECK(GetModuleHandleW(L"web_counter_app.dll")!=NULL);
#endif
 snprintf(target,sizeof(target),"%llu:counter.document",(unsigned long long)instance);
 a.action="ai-graph-propose";a.answer=target;a.command="Explicit stub panel graph";a.token="panel-graph";a.params="{\"nodes\":[{\"id\":\"counter\",\"operation_id\":\"counter-op\",\"agent_profile\":\"application.specialist\",\"capability\":\"counter.increment\",\"target\":0,\"params\":{},\"constraint\":{\"type\":\"object\",\"properties\":{},\"additionalProperties\":false}}]}";process_action(&s,&a);CHECK(graph_status(&s,"\"persisted_version\":1"));CHECK(count(&s,instance)==0);
 a.action="ai-graph-confirm";process_action(&s,&a);CHECK(s.ai_error&&strstr(s.ai_error,"rejected"));CHECK(count(&s,instance)==0);pump(&s);++s.ai_projection_generation;ai_panel_graph_bindings(&s);CHECK(s.ai_graph_bindings[0]==0);CHECK(ui_ai_graph_fixture_light(s.ai,s.view,"ai-graph-confirm")==UI_STATUS_UNSUPPORTED);schedule_refresh(&s);CHECK(projected(&s));CHECK(s.ai_graph_bindings[0]!=0);CHECK(ui_ai_graph_fixture_light(s.ai,s.view,"ai-graph-confirm")==UI_STATUS_OK);CHECK(graph_status(&s,"\"terminal\":true"));CHECK(count(&s,instance)==1);CHECK(strstr(s.ai_graph,"transaction_id")&&strstr(s.ai_graph,"undo_id"));
 p.size=sizeof(p);CHECK(ui_web_view_get_presentation(s.view,"ai-graph-data",&p)==UI_STATUS_OK);CHECK(p.text_utf8&&strstr(p.text_utf8,"database"));a.action="assistant";process_action(&s,&a);pump(&s);process_action(&s,&a);pump(&s);CHECK(count(&s,instance)==1);CHECK(graph_status(&s,"\"terminal\":true"));CHECK(close_host(&s));printf("P5 formal panel checks=%u failures=%u\n",checks,failures);return failures?1:0;
}

#ifndef UI_P6_PANEL
int wmain(int argc,wchar_t **argv){return graph_panel_fixture_run(argc,argv);}
#endif
