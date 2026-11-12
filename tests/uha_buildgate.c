/* Host-only allocation fault, isolated credentials/transport and input fixture. */
#include <stdlib.h>
#include <string.h>
static int gate_fail_allocation;
static unsigned gate_allocation_hits;
static void *gate_realloc(void *memory,size_t length)
{
    if(gate_fail_allocation){gate_fail_allocation=0;++gate_allocation_hits;return NULL;}
    return realloc(memory,length);
}
#define realloc gate_realloc
#define UHA_MAIN inherited_uha_main
#include "uha_host.c"
#undef realloc
#undef wmain

static host_window_t *gate_host;
static int gate_armed,gate_close,gate_invalid,gate_lease,gate_closed_pending;
static unsigned gate_callbacks,gate_accepted;
static ui_status_t gate_confirm_status,gate_unbind_status;
static ui_status_t gate_revision(char *out,size_t capacity,void *data)
{
    (void)data;
    if(gate_armed){
        host_window_t *h=gate_host;gate_armed=0;++gate_callbacks;
        gate_invalid=!h->ai_display_valid&&!h->ai_preview_ready;
        int slot=-1;
        EnterCriticalSection(&h->ai->lock);
        for(int i=0;i<AI_RUNS;++i)if(!strcmp(h->ai->runs[i].id,h->ai_run))slot=i;
        gate_lease=h->ai->methods>0&&slot>=0&&h->ai->runs[slot].refs>0;
        LeaveCriticalSection(&h->ai->lock);
        gate_unbind_status=ui_ai_model_unbind(h->ai,h->ai_model_bindings[1]);
        gate_confirm_status=ui_ai_model_fixture_light(h->ai,h->view,"ai-model-confirm");
        if(gate_confirm_status==UI_STATUS_OK)++gate_accepted;
        if(gate_close){
            CHECK(ui_ai_service_request_close(h->ai)==UI_STATUS_OK);
            DWORD begin=GetTickCount();ui_ai_service_state_t state={0};state.size=sizeof(state);state.version=1;
            do{
                CHECK(ui_ai_service_poll(h->ai,64)==UI_STATUS_OK);
                CHECK(ui_ai_service_get_state(h->ai,&state)==UI_STATUS_OK);
                if(state.phase==UI_AI_CLOSED&&!state.worker_alive)break;
                (void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);
            }while(GetTickCount()-begin<5000);
            CHECK(state.phase==UI_AI_CLOSED&&!state.worker_alive);
            gate_closed_pending=ui_ai_service_destroy(h->ai)==UI_STATUS_PENDING;
        }
    }
    return strcpy_s(out,capacity,"r1")?UI_STATUS_INVALID_ARGUMENT:UI_STATUS_OK;
}
static int gate_confirmed(host_window_t *s)
{
    char *raw=NULL;CHECK(ui_ai_get_run(s->ai,s->ai_run,&raw)==UI_STATUS_OK);
    yyjson_doc *d=raw?yyjson_read(raw,strlen(raw),0):NULL;CHECK(d!=NULL);
    yyjson_val *root=yyjson_doc_get_root(d);
    int confirmed=yyjson_get_bool(yyjson_obj_get(root,"disclosure_confirmed"));
    CHECK(yyjson_get_uint(yyjson_obj_get(root,"model_calls"))==0);
    CHECK(!yyjson_get_bool(yyjson_obj_get(root,"model_request_saved")));
    yyjson_doc_free(d);ui_ai_release(raw);return confirmed;
}
int wmain(int argc,wchar_t **argv)
{
    if(argc!=4)return 2;
    host_window_t s={0};host_action_t action={0};GUID guid;wchar_t db[4096],unique[40];
    CHECK(CreateDirectoryW(argv[2],NULL)||GetLastError()==ERROR_ALREADY_EXISTS);
    CoCreateGuid(&guid);StringFromGUID2(&guid,unique,40);
    _snwprintf_s(db,4096,_TRUNCATE,L"%s/buildgate-%s.db",argv[2],unique);
    CHECK(SetEnvironmentVariableW(L"UI_AI_DB_PATH",db));CHECK(ui_framework_initialize()==UI_STATUS_OK);
    CHECK(create_host(&s,GetModuleHandleW(NULL))!=NULL);if(!s.ai)return 1;
    CHECK(model_wait(&s,NULL));MoveWindow(s.hwnd,-16000,-16000,1200,3200,FALSE);ShowWindow(s.hwnd,SW_HIDE);
    action.action="assistant";process_action(&s,&action);pump(&s);click(&s,"ai-model-toggle");
    text(&s,"ai-model-name","offline-fixture");click(&s,"ai-model-save");CHECK(model_wait(&s,NULL));response(&s,0);
    open_path(&s,argv[1]);pump(&s);ui_app_instance_info_t app={0};app.size=sizeof(app);
    CHECK(ui_workspace_get_instance(s.workspace,ui_workspace_active(s.workspace),&app)==UI_STATUS_OK);
    ui_cap_resource_desc_t resource={0};resource.size=sizeof(resource);resource.version=1;
    resource.kind=UI_CAP_DOCUMENT;resource.id="buildgate.resource";resource.get_revision=gate_revision;
    CHECK(ui_capability_register_resource(app.host,&resource)==UI_STATUS_OK);gate_host=&s;
    char instance[32];snprintf(instance,sizeof(instance),"%llu",(unsigned long long)app.instance_id);
    action.action="ai-model-propose";action.token="buildgate-task";action.answer="Synthetic build failure regression";
    action.id=instance;action.panel="buildgate.resource";action.params="{\"history\":\"[]\",\"projection\":\"{\\\"capabilities\\\":[],\\\"fields\\\":[]}\"}";
    process_action(&s,&action);CHECK(model_wait(&s,"\"phase\":3"));click(&s,"ai-view-scope");CHECK(bound(&s)&&s.ai_preview_ready);
    text(&s,"ai-input","保留未提交草稿");pump(&s);CHECK(bound(&s)&&s.ai_draft);
    char draft_json[512];snprintf(draft_json,sizeof(draft_json),"{\"input\":\"保留未提交草稿\",\"history\":{\"7\":true},\"capabilities\":{\"memory.read\":true},\"fields\":{\"value\":true},\"resource\":\"memory.document\",\"instance\":\"%s\"}",instance);
    host_action_t draft_action={0};draft_action.action="ai-model-draft";draft_action.params=draft_json;
    process_action(&s,&draft_action);schedule_refresh(&s);pump(&s);CHECK(bound(&s)&&s.ai_preview_ready);
    char run[65],hash[65];strcpy_s(run,65,s.ai_run);strcpy_s(hash,65,s.ai_preview_hash);
    char *draft=_strdup(s.ai_draft),*preview=_strdup(s.ai_preview);uint64_t binding=s.ai_model_bindings[1];
    CHECK(draft&&preview&&binding);gate_armed=1;gate_fail_allocation=1;
    if(!wcscmp(argv[3],L"entry")){char *state=NULL;send_state(&s,&state);free(state);}
    else {json_buffer_t json={0};ai_panel_json(&s,&json);CHECK(json.failed);free(json.text);}
    gate_fail_allocation=0;
    CHECK(gate_allocation_hits==1);CHECK(gate_callbacks==0);
    CHECK(!s.ai_display_valid&&!s.ai_preview_ready);CHECK(gate_accepted==0);
    int confirmed=gate_confirmed(&s);CHECK(!confirmed);
    CHECK(!strcmp(run,s.ai_run)&&!strcmp(hash,s.ai_preview_hash)&&!strcmp(preview,s.ai_preview));
    CHECK(s.ai_draft&&!strcmp(draft,s.ai_draft));
    CHECK(s.ai->model_config->requests==0);
    EnterCriticalSection(&s.ai->lock);
    for(int i=0;i<AI_RUNS;++i)if(!strcmp(s.ai->runs[i].id,run))CHECK(s.ai->runs[i].model->ordinal==0);
    LeaveCriticalSection(&s.ai->lock);
    if(!confirmed){
        /* A later permitted read callback must also reject old confirmation,
           including when the old binding cannot yet be unbound. */
        gate_armed=1;gate_close=!wcscmp(argv[3],L"close");char *readback=NULL;
        ui_status_t status=ui_ai_model_get_disclosure(s.ai,s.ai_run,&readback);
        CHECK(gate_callbacks==1&&gate_invalid&&gate_lease);
        CHECK(gate_unbind_status==UI_STATUS_PENDING&&gate_confirm_status!=UI_STATUS_OK);
        CHECK(gate_accepted==0&&s.ai->model_config->requests==0);
        if(gate_close){CHECK(status!=UI_STATUS_OK&&gate_closed_pending);}
        else{
            CHECK(status==UI_STATUS_OK&&readback&&!strcmp(readback,preview));
            CHECK(!gate_confirmed(&s));schedule_refresh(&s);pump(&s);CHECK(bound(&s)&&s.ai_preview_ready);
            CHECK(ui_ai_model_fixture_light(s.ai,s.view,"ai-model-confirm")==UI_STATUS_OK);
            CHECK(ui_ai_model_fixture_light(s.ai,s.view,"ai-model-confirm")!=UI_STATUS_OK);
            CHECK(model_wait(&s,"\"outcome\":\"completed\""));CHECK(s.ai->model_config->requests==1);
        }
        ui_ai_release(readback);
    }
    /* A terminal message precedes network cleanup; poll its actual lease. */
    DWORD release_begin=GetTickCount();unsigned remaining=1;
    do{
        pump(&s);remaining=0;EnterCriticalSection(&s.ai->lock);
        for(int i=0;i<AI_RUNS;++i)if(!strcmp(s.ai->runs[i].id,run))remaining=s.ai->runs[i].refs;
        LeaveCriticalSection(&s.ai->lock);
    }while(remaining&&GetTickCount()-release_begin<5000);
    /* Every method/run lease acquired by the controlled callbacks is returned. */
    CHECK(s.ai->methods==0);EnterCriticalSection(&s.ai->lock);
    for(int i=0;i<AI_RUNS;++i)if(!strcmp(s.ai->runs[i].id,run)){CHECK(s.ai->runs[i].refs==0);if(!confirmed)CHECK(s.ai->runs[i].model->ordinal<=1);}
    LeaveCriticalSection(&s.ai->lock);
    printf("buildgate %ls allocation_failures=%u callbacks=%u accepted=%u checks=%u failures=%u\n",argv[3],gate_allocation_hits,gate_callbacks,gate_accepted,checks,failures);
    free(draft);free(preview);CHECK(close_host(&s));SetEnvironmentVariableW(L"UI_AI_DB_PATH",NULL);return failures?1:0;
}
