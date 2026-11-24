/* Production host page and business applications, isolated credentials/model.
 * Fixture-qualified confirmation tests framework gates, never device origin. */
#define UHA_MAIN general_previous_host_main
#include "uha_host.c"
#include "ai_graph_internal.h"
#include "ai_assistant_internal.h"
#include "ai_edit_fixture_control.h"
#include "ui_framework/app_data.h"
#include "general_test_wire.h"
__declspec(dllimport) ui_status_t ui_ai_graph_fixture_light(ui_ai_service_t *,ui_web_view_t *,const char *);
__declspec(dllimport) void ui_ai_graph_fixture(ui_ai_service_t *,int,int);

static unsigned unchanged_sizes;
static ULONGLONG case_started;
static void case_stage(const char *stage)
{
    FILETIME created,exited,kernel,user;
    ULARGE_INTEGER cpu;
    cpu.QuadPart=0;
    if(GetThreadTimes(GetCurrentThread(),&created,&exited,&kernel,&user)){
        cpu.LowPart=user.dwLowDateTime;cpu.HighPart=user.dwHighDateTime;
    }
    fprintf(stderr,"UI-STAGE %s elapsed_ms=%llu ui_user_ms=%llu\n",stage,
        (unsigned long long)(GetTickCount64()-case_started),(unsigned long long)(cpu.QuadPart/10000));
}
static LRESULT CALLBACK general_size_probe(HWND window,UINT message,WPARAM wparam,LPARAM lparam,UINT_PTR id,DWORD_PTR data)
{
    (void)id;(void)data;if(message==WM_SIZE)++unchanged_sizes;
    return DefSubclassProc(window,message,wparam,lparam);
}

static int host_general_wait(host_window_t *s,const char *match)
{
    DWORD begin=GetTickCount();
    do{pump(s);char *state=NULL;if(!ui_ai_assistant_get(s->ai,s->ai_run,&state)){
        int found=strstr(state,match)!=NULL;
        if(found){ui_ai_release(state);return 1;}
        strcpy_s(snapshot,sizeof(snapshot),state);ui_ai_release(state);
    }}while(GetTickCount()-begin<5000);
    fprintf(stderr,"general host wait %s actual %.4000s\n",match,snapshot);return 0;
}

static void host_general_responses(host_window_t *s)
{
    const char *responses[]={
      "{\"nodes\":[{\"id\":\"browser\",\"operation_id\":\"read-browser\",\"target\":0,\"agent_profile\":\"browser.reader\",\"instruction\":\"核对合成标题\",\"context\":\"浏览器自身包内样本\",\"capability\":\"browser.snapshot\",\"params\":{},\"constraint\":{\"type\":\"object\",\"properties\":{},\"additionalProperties\":false}},{\"id\":\"cell\",\"operation_id\":\"edit-cell\",\"target\":1,\"agent_profile\":\"maintenance.editor\",\"instruction\":\"编辑明确选定的合成单元格\",\"context\":\"id=9007199254741000，value列，用户目标74\",\"capability\":\"test.edit\",\"params\":{\"id\":\"9007199254741000\",\"column\":\"value\",\"text\":\"73\"},\"constraint\":{\"type\":\"object\",\"properties\":{\"id\":{\"type\":\"string\"},\"column\":{\"type\":\"string\"},\"text\":{\"type\":\"string\"}},\"required\":[\"id\",\"column\",\"text\"],\"additionalProperties\":false}}]}",
      "{\"decision\":\"execute\",\"summary\":\"在固定浏览器资源读取\"}",
      "{\"decision\":\"propose\",\"summary\":\"按显式目标补齐74\",\"capability\":\"test.edit\",\"params\":{\"id\":\"9007199254741000\",\"column\":\"value\",\"text\":\"74\"}}",
      "{\"summary\":\"已核对浏览器来源\",\"quotes\":[\"browser:start\"],\"inferences\":[]}",
      "{\"summary\":\"维护单元格事务已经完成，结果以回执和实际读回核对\",\"quotes\":[],\"inferences\":[]}",
      "{\"conclusion\":\"读取合成标题并完成单元格修改\",\"facts\":[{\"node\":\"browser\",\"quote\":\"BR-1 合成交接单：设备巡检\"},{\"node\":\"browser\",\"quote\":\"browser:start\"}],\"todos\":[],\"inferences\":[]}"};
    yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *items=yyjson_mut_arr(d);yyjson_mut_doc_set_root(d,items);
    for(unsigned i=0;i<sizeof(responses)/sizeof(responses[0]);++i){char *wire=general_wire(responses[i],0);CHECK(wire!=NULL);if(wire)yyjson_mut_arr_add_strcpy(d,items,wire);free(wire);}
    char *wire=yyjson_mut_write(d,0,NULL);CHECK(ui_ai_model_fixture(s->ai,1,wire)==UI_STATUS_OK);
    CHECK(ui_ai_model_fixture(s->ai,5,"general-host-isolated-secret")==UI_STATUS_OK);
    free(wire);yyjson_mut_doc_free(d);
}

static int host_general_binding(host_window_t *s,int plan)
{
    DWORD begin=GetTickCount();do{pump(s);if(plan?s->ai_assistant_binding!=0:s->ai_model_bindings[1]!=0)return 1;}while(GetTickCount()-begin<5000);
    fprintf(stderr,"binding plan=%d preview=%zu valid=%d ready=%d projection=%llu/%llu display=%llu marker=%d displayed=%d hash=%.8s error=%s\n",plan,s->ai_preview?strlen(s->ai_preview):0,s->ai_display_valid,s->ai_preview_ready,(unsigned long long)s->ai_projection_generation,(unsigned long long)s->ai_projection_rendered,(unsigned long long)s->ai_display_generation,ai_projection_marker(s),ai_model_preview_displayed(s),s->ai_preview_hash,s->ai_error?s->ai_error:"");
    yyjson_doc *d=s->ai_chunks?yyjson_read(s->ai_chunks,strlen(s->ai_chunks),0):NULL;yyjson_val *v;size_t i,n;
    yyjson_arr_foreach(d?yyjson_doc_get_root(d):NULL,i,n,v){char id[64];snprintf(id,sizeof(id),"ai-disclosure-%u",(unsigned)i);ui_element_presentation_t p={0};p.size=sizeof(p);ui_status_t status=ui_web_view_get_presentation(s->view,id,&p);fprintf(stderr,"chunk%u status=%d actual=%zu expected=%zu rect=%d,%d,%d,%d visible=%d equal=%d\n",(unsigned)i,status,p.text_utf8?strlen(p.text_utf8):0,strlen(yyjson_get_str(v)),p.rect.x,p.rect.y,p.rect.width,p.rect.height,p.visible,p.text_utf8&&!strcmp(p.text_utf8,yyjson_get_str(v)));}yyjson_doc_free(d);
    return 0;
}

static void host_general_target(host_window_t *s,uint64_t instance,const char *resource,const char *first,const char *second,const char *role)
{
    CHECK(ui_workspace_activate(s->workspace,instance)==UI_STATUS_OK);pump(s);click(s,"ai-model-catalog");
    yyjson_doc *d=s->ai_general_catalog?yyjson_read(s->ai_general_catalog,strlen(s->ai_general_catalog),0):NULL;
    yyjson_val *o=d?yyjson_doc_get_root(d):NULL,*v;size_t i,n;char id[400];int found=0;
    yyjson_arr_foreach(yyjson_obj_get(o,"resources"),i,n,v)if(!strcmp(yyjson_get_str(yyjson_obj_get(v,"id")),resource)){
        snprintf(id,sizeof(id),"ai-resource-select-%u",(unsigned)i);click(s,id);found=1;break;
    }
    CHECK(found);
    yyjson_arr_foreach(yyjson_obj_get(o,"capabilities"),i,n,v){
        const char *cap=yyjson_get_str(yyjson_obj_get(v,"id"));if(strcmp(cap,first)&&(!second||strcmp(cap,second)))continue;
        snprintf(id,sizeof(id),"ai-cap-select-%u",(unsigned)i);click(s,id);
        yyjson_val *properties=yyjson_obj_get(yyjson_obj_get(v,"output_schema"),"properties");
        const char *fields[]={"state","revision","changed_cells","value"};
        for(unsigned f=0;f<4;++f)if(yyjson_obj_get(properties,fields[f])){snprintf(id,sizeof(id),"ai-field-%s-%s",cap,fields[f]);
            ui_element_presentation_t p={0};p.size=sizeof(p);CHECK(ui_web_view_get_presentation(s->view,id,&p)==UI_STATUS_OK);
            if(p.text_utf8&&strstr(p.text_utf8,"✓")==NULL)click(s,id);
        }
    }
    yyjson_arr_foreach(yyjson_obj_get(o,"profiles"),i,n,v)if(!strcmp(yyjson_get_str(yyjson_obj_get(v,"id")),role)){
        snprintf(id,sizeof(id),"ai-profile-%u",(unsigned)i);click(s,id);
    }
    yyjson_doc_free(d);click(s,"ai-target-add");
}

static void cell_expect(host_window_t *s,uint64_t instance,const char *expected,const char *revision)
{
    ui_app_instance_info_t app={0};app.size=sizeof(app);ui_cap_target_t target={0};target.size=sizeof(target);target.version=1;
    CHECK(ui_workspace_get_instance(s->workspace,instance,&app)==UI_STATUS_OK);
    CHECK(ui_capability_bind(app.host,"state.document",&target)==UI_STATUS_OK);uint64_t call=0;
    CHECK(ui_capability_prepare(app.host,"state.cell.read",&target,"{\"id\":\"9007199254741000\",\"column\":\"value\"}",&call)==UI_STATUS_OK);
    CHECK(ui_capability_dispatch(app.host,call)==UI_STATUS_OK);
    ui_cap_result_t result={0};result.size=sizeof(result);result.version=1;
    CHECK(ui_capability_get_result(app.host,call,&result)==UI_STATUS_OK&&result.outcome==UI_CAP_OK);
    yyjson_doc *d=result.output_json?yyjson_read(result.output_json,strlen(result.output_json),0):NULL;
    const char *text=d?yyjson_get_str(yyjson_obj_get(yyjson_doc_get_root(d),"text")):NULL;
    CHECK(text&&!strcmp(text,expected));CHECK(revision&&!strcmp(target.expected_revision,revision));
    fprintf(stderr,"actual cell text=%s framework revision=%s\n",text?text:"",target.expected_revision);
    yyjson_doc_free(d);CHECK(ui_capability_forget(app.host,call)==UI_STATUS_OK);
}

static void host_graph_responses(host_window_t *s)
{
    const char *plan="{\"nodes\":[{\"id\":\"read\",\"operation_id\":\"host-read\",\"target\":0,\"capability\":\"memory.read\",\"agent_profile\":\"\",\"instruction\":\"读取本轮样本\",\"context\":\"测试自有内存\",\"params\":{},\"constraint\":{\"type\":\"object\",\"properties\":{},\"additionalProperties\":false}}]}";
    char *wire=general_wire(plan,0);CHECK(wire!=NULL);
    yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *items=yyjson_mut_arr(d);
    yyjson_mut_doc_set_root(d,items);if(wire)yyjson_mut_arr_add_strcpy(d,items,wire);free(wire);
    wire=yyjson_mut_write(d,0,NULL);CHECK(wire&&ui_ai_model_fixture(s->ai,1,wire)==UI_STATUS_OK);
    free(wire);yyjson_mut_doc_free(d);
}

static int host_control_display(host_window_t *s,const char *text)
{
    DWORD begin=GetTickCount();ui_element_presentation_t p={0};p.size=sizeof(p);
    do{
        pump(s);
        if(s->ai_projection_rendered==s->ai_projection_generation&&ai_projection_marker(s)){
            CHECK(ui_web_view_get_presentation(s->view,"ai-control-state",&p)==UI_STATUS_OK);
            fprintf(stderr,"host graph control text=%s manual=%d\n",p.text_utf8?p.text_utf8:"",s->assistant_manual);
            return p.text_utf8&&strstr(p.text_utf8,text)!=NULL;
        }
    }while(GetTickCount()-begin<5000);
    return 0;
}

static void host_graph_handoff(host_window_t *s,uint64_t instance)
{
    CHECK(ui_ai_model_fixture_light(s->ai,s->view,"ai-model-confirm")==UI_STATUS_OK);
    CHECK(host_general_wait(s,"\"plan_waiting\":true"));CHECK(host_general_binding(s,1));
    ui_app_instance_info_t app={0};app.size=sizeof(app);
    CHECK(ui_workspace_get_instance(s->workspace,instance,&app)==UI_STATUS_OK);
    int mode=AE_MODE_READ_ASYNC;
    CHECK(ui_workspace_post_app_data(s->workspace,instance,AE_APP_CONTROL,&mode,sizeof(mode))==UI_STATUS_OK);
    ui_workspace_poll(s->workspace);
    CHECK(ui_ai_graph_fixture_light(s->ai,s->view,"ai-model-confirm")==UI_STATUS_OK);
    ai_run_t *r=NULL;
    for(unsigned i=0;i<AI_RUNS;++i)if(!strcmp(s->ai->runs[i].id,s->ai_run))r=&s->ai->runs[i];
    CHECK(r&&r->graph);if(!r||!r->graph)return;
    ai_graph_t *g=r->graph;ai_coordinator_t *c=s->ai->coordinator;DWORD begin=GetTickCount();
    do{pump(s);if(g->nodes[0].call&&g->nodes[0].tool_phase==2)break;}while(GetTickCount()-begin<5000);
    uint64_t call=g->nodes[0].call;CHECK(call&&g->nodes[0].tool_phase==2);
    host_action_t action={0};action.action="ai-takeover";process_action(s,&action);
    begin=GetTickCount();
    do{pump(s);if(!g->dirty&&g->persisted==g->version&&!c->job&&!c->busy&&!c->done)break;}while(GetTickCount()-begin<5000);
    CHECK(g->nodes[0].call&&r->model->assistant->control==AS_CONTROL_DRAIN);
    ui_ai_graph_fixture(s->ai,1,1);
    CHECK(ui_workspace_post_app_data(s->workspace,instance,AE_APP_READY,&call,sizeof(call))==UI_STATUS_OK);
    CHECK(ui_workspace_post_app_data(s->workspace,instance,AE_APP_WORKER_EXIT,NULL,0)==UI_STATUS_OK);
    CHECK(ui_workspace_post_app_data(s->workspace,instance,AE_APP_CALLBACK_EXIT,NULL,0)==UI_STATUS_OK);
    begin=GetTickCount();
    do{ui_workspace_poll(s->workspace);pump(s);if(!g->nodes[0].call&&g->nodes[0].state==AG_COLLECTED&&c->job==3&&c->run_ref)break;}while(GetTickCount()-begin<5000);
    CHECK(!g->nodes[0].call&&g->nodes[0].state==AG_COLLECTED&&c->job==3&&c->run_ref);
    CHECK(host_control_display(s,"正在交接，等待在途调用和保存收尾"));
    CHECK(!s->assistant_manual&&r->model->assistant->control==AS_CONTROL_DRAIN&&r->refs>0);
    yyjson_doc *result=g->nodes[0].result?yyjson_read(g->nodes[0].result,strlen(g->nodes[0].result),0):NULL;
    CHECK(result&&yyjson_get_int(yyjson_obj_get(yyjson_doc_get_root(result),"value"))==10);
    CHECK(!strcmp(g->nodes[0].revision,"r1"));yyjson_doc_free(result);
    ui_ai_graph_fixture(s->ai,1,0);begin=GetTickCount();
    do{pump(s);if(!g->dirty&&g->persisted==g->version&&!c->run_ref&&r->model->assistant->control==AS_CONTROL_MANUAL)break;}while(GetTickCount()-begin<5000);
    CHECK(!g->dirty&&g->persisted==g->version&&!c->run_ref&&r->refs==0);
    CHECK(host_control_display(s,"应用已交还手动操作"));CHECK(s->assistant_manual);
    CHECK(s->ai->model_config->requests==1&&r->model->assistant->used==1);
    action.action="ai-cancel";action.id=s->ai_run;process_action(s,&action);CHECK(model_wait(s,"\"terminal\":true"));
}

#include "general_daily_release.inc"

int wmain(int argc,wchar_t **argv)
{
    case_started=GetTickCount64();
    if(argc!=5&&argc!=6)return 2;SetEnvironmentVariableW(L"UI_LIGHT_SCRIPT_DIAGNOSTICS",L"1");host_window_t s={0};host_action_t action={0};GUID guid;wchar_t db[4096],folder[4096],id[40],settings[4096];
    CHECK(CreateDirectoryW(argv[3],NULL)||GetLastError()==ERROR_ALREADY_EXISTS);CoCreateGuid(&guid);StringFromGUID2(&guid,id,40);
    _snwprintf_s(folder,4096,_TRUNCATE,L"%s/host-%s",argv[3],id);CHECK(CreateDirectoryW(folder,NULL));
    _snwprintf_s(db,4096,_TRUNCATE,L"%s/conversation.db",folder);_snwprintf_s(settings,4096,_TRUNCATE,L"%s/origins.txt",folder);
    HANDLE f=CreateFileW(settings,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);CHECK(f!=INVALID_HANDLE_VALUE);if(f!=INVALID_HANDLE_VALUE)CloseHandle(f);
    CHECK(SetEnvironmentVariableW(L"UI_AI_DB_PATH",db));CHECK(SetEnvironmentVariableW(L"UI_BROWSER_SETTINGS_PATH",settings));
    CHECK(SetEnvironmentVariableW(L"UI_STATE_EXAMPLE_HOME",folder));CHECK(SetEnvironmentVariableW(L"UI_API7_BACKEND",L"light"));
    CHECK(SetEnvironmentVariableW(L"UI_API7_OSMESA_DLL",argv[4]));CHECK(SetEnvironmentVariableW(L"UI_AI_GENERAL_ASSISTANT",L"1"));
    SetEnvironmentVariableW(L"UI_AI_ASSISTANT_POLICY",NULL);SetEnvironmentVariableW(L"UI_AI_BR2_POLICY",NULL);
    SetEnvironmentVariableW(L"UI_AI_PROFILE_PATH",NULL);SetEnvironmentVariableW(L"UI_AI_PROFILE_ID",NULL);
    CHECK(ui_framework_initialize()==UI_STATUS_OK);CHECK(create_host(&s,GetModuleHandleW(NULL))!=NULL);if(!s.ai)return 1;
    CHECK(model_wait(&s,NULL));CHECK(s.ai_general&&s.ai->assistant_policy);
    if(!s.ai_general||!s.ai->assistant_policy){CHECK(close_host(&s));return 1;}
    int daily=argc==6&&!wcsncmp(argv[5],L"daily-",6);
    if(!daily){
        /* Existing synthetic-policy cases do not require a release descriptor. */
        s.ai->assistant_policy->input_rate=100;s.ai->assistant_policy->output_rate=200;
        strcpy_s(s.ai->assistant_policy->price_version,65,"isolated-host-v1");memset(s.ai->assistant_policy->price_evidence,'1',64);s.ai->assistant_policy->price_evidence[64]=0;
        strcpy_s(s.ai_price_version,65,"isolated-host-v1");
    }
    CHECK(MoveWindow(s.hwnd,-16000,-16000,1280,1000,FALSE));ShowWindow(s.hwnd,SW_HIDE);
    action.action="assistant";process_action(&s,&action);pump(&s);
    ui_rect_t assistant={0},workspace={0};CHECK(ui_web_view_get_element_rect(s.view,"assistant",&assistant)==UI_STATUS_OK);
    CHECK(ui_web_view_get_element_rect(s.view,"workspace",&workspace)==UI_STATUS_OK);CHECK(assistant.x==0&&assistant.width==380&&workspace.x==assistant.width&&workspace.width>0);
    click(&s,"ai-model-toggle");text(&s,"ai-model-name","deepseek-flash");
    if(failures){CHECK(close_host(&s));return 1;}
    click(&s,"ai-model-save");CHECK(model_wait(&s,NULL));click(&s,"ai-model-toggle");
    if(daily)return host_daily_run(&s,argv[1],db,argv[5]);
    host_general_responses(&s);open_path(&s,argv[1]);pump(&s);uint64_t browser=ui_workspace_active(s.workspace);
    host_shared_chat_controls(&s);
    open_path(&s,argv[2]);pump(&s);uint64_t maintenance=ui_workspace_active(s.workspace);CHECK(browser!=maintenance);
    host_shared_chat_controls(&s);
    click(&s,"ai-draft-range-toggle");int graph_save=argc==6&&!wcscmp(argv[5],L"graph-save");
    unsigned before=failures;
    if(graph_save)host_general_target(&s,browser,"memory.document","memory.read",NULL,"");
    else{
        host_general_target(&s,browser,"browser.view","browser.snapshot",NULL,"browser.reader");
        if(failures!=before){CHECK(close_host(&s));return 1;}
        host_general_target(&s,maintenance,"state.document","state.document.read","test.edit","maintenance.editor");
    }
    if(failures!=before){CHECK(close_host(&s));return 1;}
    ui_app_instance_info_t maintenance_app={0};maintenance_app.size=sizeof(maintenance_app);
    CHECK(ui_workspace_get_instance(s.workspace,maintenance,&maintenance_app)==UI_STATUS_OK);
    CHECK(SetWindowSubclass((HWND)maintenance_app.native_container,general_size_probe,78,0));
    layout(&s);unchanged_sizes=0;layout(&s);layout(&s);CHECK(unchanged_sizes==0);
    CHECK(RemoveWindowSubclass((HWND)maintenance_app.native_container,general_size_probe,78));
    if(argc==6&&!wcscmp(argv[5],L"takeover-read")){
        char *wire=general_read_wire("browser.snapshot");CHECK(wire!=NULL);
        yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *responses=yyjson_mut_arr(d);
        yyjson_mut_doc_set_root(d,responses);if(wire)yyjson_mut_arr_add_strcpy(d,responses,wire);free(wire);
        wire=yyjson_mut_write(d,0,NULL);CHECK(ui_ai_model_fixture(s.ai,1,wire)==UI_STATUS_OK);free(wire);yyjson_mut_doc_free(d);
        EnterCriticalSection(&s.ai->lock);s.ai->fixture_tool_pause=AI_TOOL_PREPARED;LeaveCriticalSection(&s.ai->lock);
    }
    if(graph_save)host_graph_responses(&s);
    text(&s,"ai-input",graph_save?"读取本轮实际内存样本，检查保存后手动交接":"读取合成标题并将明确单元格改为74");click(&s,"ai-model-propose");
    CHECK(model_wait(&s,"\"phase\":3"));CHECK(host_general_binding(&s,0));CHECK(s.ai->model_config->requests==0);
    if(graph_save){host_graph_handoff(&s,browser);CHECK(close_host(&s));printf("Host graph handoff checks=%u failures=%u\n",checks,failures);return failures?1:0;}
    if(argc==6&&!wcscmp(argv[5],L"takeover-read")){
        CHECK(ui_ai_model_fixture_light(s.ai,s.view,"ai-model-confirm")==UI_STATUS_OK);
        DWORD begin=GetTickCount();int arrived=0;
        do{pump(&s);EnterCriticalSection(&s.ai->lock);arrived=s.ai->fixture_tool_seen==AI_TOOL_PREPARED;LeaveCriticalSection(&s.ai->lock);}while(!arrived&&GetTickCount()-begin<5000);
        CHECK(arrived);action.action="ai-takeover";process_action(&s,&action);
        EnterCriticalSection(&s.ai->lock);s.ai->fixture_tool_pause=AI_TOOL_RESULT;LeaveCriticalSection(&s.ai->lock);
        CHECK(ui_ai_service_poll(s.ai,64)==UI_STATUS_OK);
        EnterCriticalSection(&s.ai->lock);CHECK(s.ai->tool.state==AI_TOOL_RESULT);LeaveCriticalSection(&s.ai->lock);
        EnterCriticalSection(&s.ai->lock);s.ai->fixture_tool_pause=0;LeaveCriticalSection(&s.ai->lock);
        /* Hold host projection until the worker has persisted the cancelled read
           and terminal task. Handoff must survive the active-run slot clearing. */
        int terminal=0;begin=GetTickCount();
        do{char *record=NULL;(void)ui_ai_service_poll(s.ai,64);if(!ui_ai_get_run(s.ai,s.ai_run,&record)){terminal=strstr(record,"\"terminal\":true")!=NULL;ui_ai_release(record);}if(terminal)break;(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}while(GetTickCount()-begin<5000);
        CHECK(terminal);
        CHECK(model_wait(&s,"\"terminal\":true"));CHECK(host_general_wait(&s,"\"control\":2"));
        pump(&s);ui_element_presentation_t presentation={0};presentation.size=sizeof(presentation);
        CHECK(s.assistant_manual);
        CHECK(ui_web_view_get_presentation(s.view,"ai-control-state",&presentation)==UI_STATUS_OK&&presentation.text_utf8&&strstr(presentation.text_utf8,"应用已交还手动操作"));
        CHECK(s.ai->model_config->requests==1&&!s.ai->tool.state);
        cell_expect(&s,maintenance,"42","doc-1");CHECK(close_host(&s));
        printf("General preparation handoff checks=%u failures=%u\n",checks,failures);return failures?1:0;
    }
    if(argc==6){
        ui_element_presentation_t status={0};status.size=sizeof(status);
        CHECK(ui_web_view_get_presentation(s.view,"ai-status-line",&status)==UI_STATUS_OK);
        CHECK(status.text_utf8&&strstr(status.text_utf8,"等待外发确认"));
        click(&s,"ai-cancel");CHECK(model_wait(&s,"\"terminal\":true"));
        DWORD begin=GetTickCount();int displayed=0;
        do{
            pump(&s);
            CHECK(ui_web_view_get_presentation(s.view,"ai-status-line",&status)==UI_STATUS_OK);
            displayed=status.text_utf8&&strstr(status.text_utf8,"已取消")&&strstr(status.text_utf8,"已保存");
            if(displayed)break;
        }while(GetTickCount()-begin<5000);
        CHECK(status.text_utf8&&strstr(status.text_utf8,"已取消")&&strstr(status.text_utf8,"已保存"));
        CHECK(s.ai->model_config->requests==0);CHECK(close_host(&s));
        printf("General cancellation status checks=%u failures=%u\n",checks,failures);return failures?1:0;
    }
    case_stage("initial-preview");char run[65];strcpy_s(run,65,s.ai_run);CHECK(ui_ai_model_fixture_light(s.ai,s.view,"ai-model-confirm")==UI_STATUS_OK);
    if(!host_general_wait(&s,"在固定浏览器资源读取")){CHECK(close_host(&s));return 1;}
    if(!host_general_wait(&s,"\"plan_waiting\":true")){CHECK(close_host(&s));return 1;}
    if(!host_general_binding(&s,1)){CHECK(0);CHECK(close_host(&s));return 1;}CHECK(s.ai_preview&&strstr(s.ai_preview,"74"));CHECK(s.ai->model_config->requests==3);
    case_stage("plan-ready");text(&s,"ai-input","未提交草稿保留");CHECK(ui_workspace_activate(s.workspace,browser)==UI_STATUS_OK);pump(&s);
    if(!host_general_binding(&s,1)){CHECK(0);CHECK(close_host(&s));return 1;}char *fixed=_strdup(s.ai_preview);CHECK(fixed!=NULL);
    click(&s,"ai-detail-toggle");pump(&s);CHECK(!s.ai_assistant_binding);CHECK(ui_ai_graph_fixture_light(s.ai,s.view,"ai-model-confirm")==UI_STATUS_UNSUPPORTED);
    click(&s,"ai-detail-toggle");if(!host_general_binding(&s,1)){CHECK(0);CHECK(close_host(&s));return 1;}CHECK(!strcmp(fixed,s.ai_preview));free(fixed);
    CHECK(ui_ai_graph_fixture_light(s.ai,s.view,"ai-model-confirm")==UI_STATUS_OK);
    CHECK(host_general_wait(&s,"browser:start"));CHECK(host_general_wait(&s,"doc-2"));
    CHECK(host_general_wait(&s,"原文没有待办"));CHECK(model_wait(&s,"\"terminal\":true"));CHECK(s.ai->model_config->requests==6);
    ui_element_presentation_t draft={0};draft.size=sizeof(draft);CHECK(ui_web_view_get_presentation(s.view,"ai-input",&draft)==UI_STATUS_OK&&!strcmp(draft.text_utf8,"未提交草稿保留"));
    case_stage("root-complete");cell_expect(&s,maintenance,"74","doc-2");
    click(&s,"ai-task-facts-toggle");
    click(&s,"ai-undo-view-cell");
    DWORD begin=GetTickCount();do{pump(&s);}while(!s.ai_undo_binding&&GetTickCount()-begin<5000);
    if(!s.ai_undo_binding){
        ui_element_presentation_t undo={0};undo.size=sizeof(undo);
        ui_web_view_get_presentation(s.view,"ai-node-undo-preview",&undo);
        fprintf(stderr,"undo preview visible=%d y=%d height=%d clip=%d/%d projection=%llu/%llu\n",undo.visible,undo.rect.y,undo.rect.height,undo.clip.y,undo.clip.height,(unsigned long long)s.ai_projection_rendered,(unsigned long long)s.ai_projection_generation);
    }
    CHECK(s.ai_undo_binding!=0);CHECK(ui_ai_graph_fixture_light(s.ai,s.view,"ai-node-undo")==UI_STATUS_OK);CHECK(host_general_wait(&s,"\"undo_finished\":true"));cell_expect(&s,maintenance,"42","doc-3");
    case_stage("undo-complete");CHECK(close_host(&s));case_stage("first-closed");
    char path[4096];CHECK(WideCharToMultiByte(CP_UTF8,0,db,-1,path,sizeof(path),NULL,NULL)>0);char uri[4300];snprintf(uri,sizeof(uri),"file:%s?immutable=1",path);
    sqlite3 *sql=NULL;CHECK(sqlite3_open_v2(uri,&sql,SQLITE_OPEN_READONLY|SQLITE_OPEN_URI,NULL)==SQLITE_OK);sqlite3_stmt *q=NULL;
    CHECK(sqlite3_prepare_v2(sql,"SELECT terminal,outcome,reply FROM runs WHERE id=?",-1,&q,NULL)==SQLITE_OK);sqlite3_bind_text(q,1,run,-1,SQLITE_STATIC);CHECK(sqlite3_step(q)==SQLITE_ROW&&sqlite3_column_int(q,0)==1&&!strcmp((const char *)sqlite3_column_text(q,1),"completed"));CHECK(strstr((const char *)sqlite3_column_text(q,2),"原文没有待办"));sqlite3_finalize(q);
    CHECK(sqlite3_prepare_v2(sql,"SELECT payload FROM ai_graphs WHERE run_id=?",-1,&q,NULL)==SQLITE_OK);sqlite3_bind_text(q,1,run,-1,SQLITE_STATIC);CHECK(sqlite3_step(q)==SQLITE_ROW);const char *graph=(const char *)sqlite3_column_text(q,0);CHECK(graph&&strstr(graph,"transaction_id")&&strstr(graph,"browser:start")&&strstr(graph,"\"undo_finished\":true"));sqlite3_finalize(q);sqlite3_close(sql);
    memset(&s,0,sizeof(s));CHECK(create_host(&s,GetModuleHandleW(NULL))!=NULL);CHECK(model_wait(&s,NULL));ShowWindow(s.hwnd,SW_HIDE);
    case_stage("records-reopened");
    action.action="ai-select";action.id=run;process_action(&s,&action);begin=GetTickCount();do{pump(&s);}while((!s.ai_stored_run||s.ai_query_pending)&&GetTickCount()-begin<5000);
    CHECK(s.ai_stored_run&&strstr(s.ai_stored_run,"completed")&&strstr(s.ai_stored_run,"authorization_restored\":false"));CHECK(s.ai->model_config->requests==0&&!s.ai_preview_ready);CHECK(close_host(&s));
    case_stage("records-closed");
    SetEnvironmentVariableW(L"UI_AI_DB_PATH",NULL);SetEnvironmentVariableW(L"UI_BROWSER_SETTINGS_PATH",NULL);SetEnvironmentVariableW(L"UI_STATE_EXAMPLE_HOME",NULL);SetEnvironmentVariableW(L"UI_AI_GENERAL_ASSISTANT",NULL);
    printf("General host business checks=%u failures=%u; isolated authorization only\n",checks,failures);return failures?1:0;
}
