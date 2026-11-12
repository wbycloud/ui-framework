/* Direct public-entry regressions. Credentials, transport and input are isolated. */
#define GENERAL_ASSISTANT_MAIN closeout_previous_main
#include "general_assistant.c"
#undef GENERAL_ASSISTANT_MAIN
#ifdef GENERAL_CLOSEOUT_LIGHT
#include "ui_framework/light_web.h"
#include "ui_framework/capability_inputs.h"
__declspec(dllimport) ui_status_t ui_ai_graph_fixture_light(ui_ai_service_t *,ui_web_view_t *,const char *);
#endif

enum {CO_NONE,CO_HIDE_DETAIL,CO_HIDE_RANGE,CO_GENERATION,CO_TAKEOVER,CO_CLOSE,CO_LEASE,CO_QUERY_CLOSE,CO_CHANGED};
static int callback_action,callback_at,callback_count,callback_lease,query_destroy_attempted;
static int changed_revision,read_async,read_reentry,graph_reentry,read_value=41;
static int graph_save_failed;
static uint64_t graph_saved_before;
static unsigned executes,cancels,completions,manual_before_saved;
static uint64_t pending_read;
static ui_status_t query_destroy_status=UI_STATUS_VALIDATION_FAILED;

#ifdef GENERAL_CLOSEOUT_LIGHT
static ui_host_t *display_host;
static ui_web_backend_t *display_backend;
static ui_web_view_t *display_view;
static char *display_preview;
static uint64_t display_generation=1,display_binding;
static int display_valid=1;
static ui_status_t callback_unbind=UI_STATUS_OK;

static void display_message(const char *mode,const char *preview)
{
    yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);
    yyjson_mut_val *o=yyjson_mut_obj(d),*parts=yyjson_mut_arr(d);
    yyjson_mut_doc_set_root(d,o);
    yyjson_mut_obj_add_strcpy(d,o,"mode",mode);
    for(size_t at=0;preview&&preview[at];){
        size_t n=strlen(preview+at);
        if(n>3000)n=3000;
        while(preview[at+n]&&((unsigned char)preview[at+n]&0xc0)==0x80)--n;
        CHECK(n>0);
        if(!n)break;
        yyjson_mut_arr_add_strncpy(d,parts,preview+at,n);at+=n;
    }
    yyjson_mut_obj_add_val(d,o,"parts",parts);
    char *text=yyjson_mut_write(d,0,NULL);
    CHECK(text&&ui_web_view_post_json(display_view,text)==UI_STATUS_OK);
    free(text);yyjson_mut_doc_free(d);
}

static ui_status_t display_check(void *data,const char *id,const char *hash,uint64_t generation)
{
    (void)data;
    ai_run_t *r=general_run();
    if(!display_valid||generation!=display_generation||!r||strcmp(id,run)||
       !r->graph||strcmp(hash,r->graph->binding))return UI_STATUS_PERMISSION_DENIED;
    for(size_t at=0,index=0;display_preview&&display_preview[at];++index){
        char control[40];snprintf(control,sizeof(control),"preview-%zu",index);
        ui_element_presentation_t p={0};p.size=sizeof(p);
        size_t n=strlen(display_preview+at);
        if(n>3000)n=3000;
        while(display_preview[at+n]&&((unsigned char)display_preview[at+n]&0xc0)==0x80)--n;
        if(ui_web_view_get_presentation(display_view,control,&p)||p.rect.width<=0||
           p.rect.height<=0||!p.text_utf8||strlen(p.text_utf8)!=n||
           memcmp(p.text_utf8,display_preview+at,n))return UI_STATUS_PERMISSION_DENIED;
        at+=n;
    }
    return UI_STATUS_OK;
}

static void display_open(void)
{
    ui_host_config_t h={0};h.size=sizeof(h);h.api_version=UI_FRAMEWORK_API_VERSION;
    display_host=ui_host_create(&h);CHECK(display_host!=NULL);
    ui_light_web_config_t c={0};c.size=sizeof(c);
    display_backend=ui_light_web_backend_create(&c);CHECK(display_backend!=NULL);
    display_view=ui_web_view_create(display_host,display_backend);CHECK(display_view!=NULL);
    CHECK(ui_web_view_load_html(display_view,
      "<html><body><div id='range'><div id='details'></div></div>"
      "<button id='confirm'>确认有限计划</button><script>ui.onmessage=function(p){"
      "var range=document.getElementById('range'),details=document.getElementById('details');"
      "if(p.mode==='detail'){details.style.display='none';return;}"
      "if(p.mode==='range'){range.style.display='none';return;}"
      "range.style.display='flex';details.style.display='flex';"
      "while(details.firstChild)details.removeChild(details.firstChild);"
      "p.parts.forEach(function(text,i){var t=document.createElement('textarea');"
      "t.id='preview-'+i;t.readOnly=true;t.value=text;t.style.height='100px';"
      "details.appendChild(t);});};</script></body></html>")==UI_STATUS_OK);
    CHECK(ui_web_view_resize(display_view,600,1000,96)==UI_STATUS_OK);
    CHECK(ui_capability_workspace_input_bind(w,display_view)==UI_STATUS_OK);
}

static void display_bind(void)
{
    if(display_binding){CHECK(ui_ai_graph_unbind(s,display_binding)==UI_STATUS_OK);display_binding=0;}
    free(display_preview);display_preview=NULL;
    CHECK(ui_ai_assistant_get_disclosure(s,run,&display_preview)==UI_STATUS_OK);
    CHECK(display_preview!=NULL);display_valid=1;++display_generation;
    display_message("show",display_preview);
    ui_ai_assistant_display_binding_t b={0};b.size=sizeof(b);b.version=1;
    b.generation=display_generation;b.check=display_check;
    b.binding.size=sizeof(b.binding);b.binding.version=1;b.binding.view=display_view;
    b.binding.control_id="confirm";b.binding.run_id=run;b.binding.node_id="";
    b.binding.binding_hash=general_run()->graph->binding;b.binding.action=UI_AI_GRAPH_CONFIRM;
    CHECK(display_check(NULL,run,b.binding.binding_hash,display_generation)==UI_STATUS_OK);
    CHECK(ui_ai_assistant_bind_light_display(s,&b,&display_binding)==UI_STATUS_OK);
}
#endif

static ui_status_t closeout_revision(char *out,size_t capacity,void *data)
{
    (void)data;
    if(callback_at&&++callback_count==callback_at){
        int action=callback_action;callback_at=0;ai_run_t *r=general_run();
        EnterCriticalSection(&s->lock);
        callback_lease=s->methods>0&&r&&r->refs>0;
        LeaveCriticalSection(&s->lock);
#ifdef GENERAL_CLOSEOUT_LIGHT
        if(action>=CO_HIDE_DETAIL&&action<=CO_GENERATION){
            display_valid=0;
            if(action==CO_GENERATION)++display_generation;
            else display_message(action==CO_HIDE_DETAIL?"detail":"range",NULL);
            callback_unbind=ui_ai_graph_unbind(s,display_binding);
        }
#endif
        if(action==CO_TAKEOVER)CHECK(ui_ai_assistant_control(s,run,UI_AI_ASSISTANT_TAKEOVER)==UI_STATUS_PENDING);
        if(action==CO_CHANGED)changed_revision=1;
        if(action==CO_CLOSE||action==CO_QUERY_CLOSE){
            CHECK(ui_ai_service_request_close(s)==UI_STATUS_OK);
            if(action==CO_QUERY_CLOSE){
                DWORD begin=GetTickCount();ui_ai_service_state_t closing_state={0};
                closing_state.size=sizeof(closing_state);closing_state.version=1;
                do{
                    CHECK(ui_ai_service_poll(s,64)==UI_STATUS_OK);
                    CHECK(ui_ai_service_get_state(s,&closing_state)==UI_STATUS_OK);
                    if(closing_state.phase==UI_AI_CLOSED&&!closing_state.worker_alive)break;
                    (void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);
                }while(GetTickCount()-begin<5000);
                CHECK(closing_state.phase==UI_AI_CLOSED&&!closing_state.worker_alive);
                /* Keep the red capture safe when the outer query lacks a lease. */
                if(callback_lease){
                    query_destroy_attempted=1;query_destroy_status=ui_ai_service_destroy(s);
                    CHECK(query_destroy_status==UI_STATUS_PENDING);
                }
                CHECK(callback_lease&&query_destroy_attempted);
            }
        }
    }
    return strcpy_s(out,capacity,changed_revision?"r2":"r1")?UI_STATUS_INVALID_ARGUMENT:UI_STATUS_OK;
}

static void complete_read(uint64_t call)
{
    char text[80];snprintf(text,sizeof(text),"{\"value\":%d,\"revision\":\"r1\"}",read_value);
    ui_cap_result_t result={0};result.size=sizeof(result);result.version=1;
    result.outcome=UI_CAP_OK;result.output_json=text;
    CHECK(ui_capability_complete(apps[0].host,call,&result)==UI_STATUS_OK);++completions;
}

static void execute_read(uint64_t call,const ui_cap_target_t *target,const char *params,void *data)
{
    (void)data;
    CHECK(!strcmp(target->resource_id,"closeout.document")&&!strcmp(params,"{}"));++executes;
    if(read_async){pending_read=call;return;}
    complete_read(call);
    if(graph_reentry){
        CHECK(ui_ai_assistant_control(s,run,UI_AI_ASSISTANT_TAKEOVER)==UI_STATUS_PENDING);
        CHECK(ui_ai_service_poll(s,64)==UI_STATUS_PENDING);
        CHECK(ui_capability_forget(apps[0].host,call)==UI_STATUS_PENDING);
        ui_ai_graph_fixture(s,1,1);
    }
    if(read_reentry){
        EnterCriticalSection(&s->lock);s->fixture_tool_pause=AI_TOOL_RESULT;LeaveCriticalSection(&s->lock);
        CHECK(ui_ai_assistant_control(s,run,UI_AI_ASSISTANT_TAKEOVER)==UI_STATUS_PENDING);
        CHECK(ui_ai_service_poll(s,64)==UI_STATUS_PENDING);
        CHECK(ui_capability_forget(apps[0].host,call)==UI_STATUS_PENDING);
        CHECK(general_run()->model->assistant->control==AS_CONTROL_DRAIN);
    }
}

static void cancel_read(uint64_t call,void *data){(void)data;CHECK(call!=0);++cancels;}

static void register_read(void)
{
    ui_cap_resource_desc_t resource={0};resource.size=sizeof(resource);resource.version=1;
    resource.kind=UI_CAP_DOCUMENT;resource.id="closeout.document";resource.get_revision=closeout_revision;
    CHECK(ui_capability_register_resource(apps[0].host,&resource)==UI_STATUS_OK);
    ui_capability_desc_t cap={0};cap.size=sizeof(cap);cap.version=cap.capability_version=1;
    cap.id="closeout.read";cap.name_utf8="Controlled preparation read";
    cap.description_utf8="Test-owned mutable value";cap.category="closeout";
    cap.effect="read";cap.target_kind=UI_CAP_DOCUMENT;
    cap.input_schema_json="{\"type\":\"object\",\"properties\":{},\"additionalProperties\":false}";
    cap.output_schema_json="{\"type\":\"object\",\"properties\":{\"value\":{\"type\":\"integer\"},\"revision\":{\"type\":\"string\"}},\"required\":[\"value\",\"revision\"],\"additionalProperties\":false}";
    cap.execute=execute_read;cap.cancel=cancel_read;
    CHECK(ui_capability_register(apps[0].host,&cap)==UI_STATUS_OK);
}

static ui_status_t submit_closeout(void)
{
    ui_cap_target_t read={0};read.size=sizeof(read);read.version=1;
    CHECK(ui_capability_bind(apps[0].host,"closeout.document",&read)==UI_STATUS_OK);
    ui_cap_target_v2_t target={0};target.size=sizeof(target);target.version=2;
    CHECK(ui_ai_edit_bind_target(s,apps[0].instance_id,"closeout.document",&target)==UI_STATUS_OK);
    strcpy_s(target.scope_json,sizeof(target.scope_json),"{\"access\":\"read\"}");
    ui_ai_assistant_request_t q={0};q.size=sizeof(q);q.version=1;
    q.task.size=sizeof(q.task);q.task.version=1;q.task.client_request_id="closeout-task";
    q.task.input_utf8="限定回调与准备读取";q.task.profile_id="deepseek";
    q.task.profile_revision=s->model_config->revision;q.task.targets=&read;q.task.target_count=1;
    q.task.history_selection_json="[]";
    q.task.disclosure_projection_json="{\"capabilities\":[\"closeout.read\"],\"fields\":[\"value\",\"revision\"]}";
    q.targets=&target;q.target_count=1;
    q.permissions_json="[{\"capabilities\":[\"closeout.read\"],\"profiles\":[]}]";
    return ui_ai_assistant_propose(s,&q,run);
}

static void closeout_responses(int preparation)
{
    const char *plan="{\"nodes\":[{\"id\":\"read\",\"operation_id\":\"closeout-op\",\"target\":0,\"capability\":\"closeout.read\",\"agent_profile\":\"\",\"instruction\":\"读取明确样本\",\"context\":\"本轮自有样本\",\"params\":{},\"constraint\":{\"type\":\"object\",\"properties\":{},\"additionalProperties\":false}}]}";
    const char *final="{\"conclusion\":\"已核对实际读取\",\"facts\":[{\"node\":\"read\",\"quote\":\"r1\"}],\"todos\":[],\"inferences\":[]}";
    yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *items=yyjson_mut_arr(d);
    yyjson_mut_doc_set_root(d,items);
    char *wire=preparation?general_read_wire("closeout.read"):NULL;
    if(wire){yyjson_mut_arr_add_strcpy(d,items,wire);free(wire);}
    wire=general_wire(plan,0);CHECK(wire!=NULL);
    if(wire)yyjson_mut_arr_add_strcpy(d,items,wire);free(wire);
    wire=general_wire(final,0);CHECK(wire!=NULL);
    if(wire)yyjson_mut_arr_add_strcpy(d,items,wire);free(wire);
    wire=yyjson_mut_write(d,0,NULL);
    CHECK(wire&&ui_ai_model_fixture(s,1,wire)==UI_STATUS_OK);
    free(wire);yyjson_mut_doc_free(d);
    CHECK(ui_ai_model_fixture(s,5,"fixture-key-no-leak")==UI_STATUS_OK);
}

static int ready_plan(void)
{
    CHECK(submit_closeout()==UI_STATUS_OK);
    if(!real_wait("\"phase\":3"))return 0;
    CHECK(ui_ai_model_fixture(s,2,run)==UI_STATUS_OK);
    if(!general_wait("\"plan_waiting\":true"))return 0;
    DWORD begin=GetTickCount();
    do{
        pump();ai_graph_t *g=general_run()->graph;
        if(!g->dirty&&g->persisted==g->version&&!s->coordinator->job&&
           !s->coordinator->busy&&!s->coordinator->done)return 1;
    }while(GetTickCount()-begin<5000);
    return 0;
}

static void query_case(int action,int initial)
{
    if(initial){
        CHECK(submit_closeout()==UI_STATUS_OK);
        if(!real_wait("\"phase\":3")){CHECK(0);return;}
    }else if(!ready_plan()){CHECK(0);return;}
    ai_run_t *r=general_run();char *preview=NULL;
    callback_action=action;callback_count=0;callback_at=1;
    ui_status_t status=ui_ai_assistant_get_disclosure(s,run,&preview);
    CHECK(callback_count>=1&&callback_lease);CHECK(s->methods==0);
    if(action==CO_QUERY_CLOSE){
        CHECK(status!=UI_STATUS_OK&&!preview);
        CHECK(query_destroy_attempted&&query_destroy_status==UI_STATUS_PENDING);
        CHECK(r->refs==0);
        CHECK(ui_ai_service_destroy(s)==UI_STATUS_OK);s=NULL;
    }else if(action==CO_TAKEOVER){
        printf("initial query status=%d output_bytes=%zu control=%d methods=%u refs=%d requests=%u reserved=%u saved=%d\n",status,preview?strlen(preview):0,r->model->assistant->control,s->methods,r->refs,s->model_config->requests,r->model->assistant->used,r->model->request_saved);
        CHECK(status==UI_STATUS_PERMISSION_DENIED&&!preview);
        CHECK(r->model->assistant->control!=AS_CONTROL_AI);
        ui_ai_release(preview);preview=NULL;
    }else if(action==CO_CHANGED){
        CHECK(status==(initial?UI_STATUS_VALIDATION_FAILED:UI_STATUS_PERMISSION_DENIED)&&!preview);changed_revision=0;
        CHECK(ui_ai_assistant_get_disclosure(s,run,&preview)==UI_STATUS_OK&&preview);
        ui_ai_release(preview);
    }else{
        CHECK(status==UI_STATUS_OK&&preview&&strstr(preview,initial?"\"request\"":"finite_plan"));
        yyjson_doc *d=preview?yyjson_read(preview,strlen(preview),0):NULL;
        CHECK(d&&!strcmp(yyjson_get_str(yyjson_obj_get(yyjson_doc_get_root(d),"run_id")),run));
        if(initial&&d){
            yyjson_val *request=yyjson_obj_get(yyjson_doc_get_root(d),"request");
            CHECK(!strcmp(yyjson_get_str(yyjson_obj_get(request,"model")),"deepseek-flash"));
            CHECK(strstr(preview,"限定回调与准备读取")!=NULL);
        }
        yyjson_doc_free(d);ui_ai_release(preview);
    }
    if(s){
        CHECK(s->model_config->requests==(initial?0u:1u)&&(!r->graph||!r->graph->confirmed));
        if(initial)CHECK(!r->model->confirmed&&!r->model->request_saved&&!r->model->confirmation_saved&&!r->model->assistant->used);
        CHECK(ui_ai_cancel(s,run)==UI_STATUS_OK);CHECK(real_wait("\"terminal\":true"));
        DWORD begin=GetTickCount();int references;
        do{pump();EnterCriticalSection(&s->lock);references=r->refs;LeaveCriticalSection(&s->lock);}while(references&&GetTickCount()-begin<5000);
        CHECK(references==0&&s->methods==0);
    }
}

#ifdef GENERAL_CLOSEOUT_LIGHT
static void confirm_case(int action)
{
    if(!ready_plan()){CHECK(0);return;}
    display_open();display_bind();ai_run_t *r=general_run();
    unsigned requests=s->model_config->requests,used=r->model->assistant->used;
    callback_action=action;callback_count=0;callback_at=action==CO_NONE?0:2;
    ui_status_t status=ui_ai_graph_fixture_light(s,display_view,"confirm");
    if(action==CO_NONE)CHECK(status==UI_STATUS_OK);
    else{
        CHECK(callback_count>=2&&callback_at==0&&callback_lease);
        CHECK(status!=UI_STATUS_OK&&!r->graph->confirmed);
        CHECK(executes==0&&r->graph->charged_tools==0&&s->model_config->requests==requests&&r->model->assistant->used==used);
        printf("rejected confirmation action=%d accepted=%d executed=%u reserved_delta=%u transport_delta=%u\n",action,r->graph->confirmed,executes,r->model->assistant->used-used,s->model_config->requests-requests);
        if(action>=CO_HIDE_DETAIL&&action<=CO_GENERATION)CHECK(callback_unbind==UI_STATUS_PENDING);
    }
    if(!r->graph->confirmed&&(action==CO_HIDE_DETAIL||action==CO_HIDE_RANGE||action==CO_GENERATION)){
        display_bind();CHECK(ui_ai_graph_fixture_light(s,display_view,"confirm")==UI_STATUS_OK);
        CHECK(ui_ai_graph_fixture_light(s,display_view,"confirm")!=UI_STATUS_OK);
        CHECK(real_wait("\"terminal\":true"));CHECK(executes==1&&completions==1);
    }else if(action==CO_NONE){
        CHECK(ui_ai_graph_fixture_light(s,display_view,"confirm")!=UI_STATUS_OK);
        CHECK(real_wait("\"terminal\":true"));CHECK(executes==1&&completions==1);
    }else{
        CHECK(ui_ai_cancel(s,run)==UI_STATUS_OK);CHECK(real_wait("\"terminal\":true"));CHECK(executes==0);
    }
    CHECK(ui_capability_workspace_input_unbind(display_view)==UI_STATUS_OK);display_binding=0;
    ui_web_view_destroy(display_view);display_view=NULL;ui_light_web_backend_destroy(display_backend);
    ui_host_destroy(display_host);free(display_preview);display_preview=NULL;
}
#endif

static void preparation_case(int boundary,int completion_reentry,int dispatch_reentry)
{
    read_async=boundary==AI_TOOL_RUNNING&&!completion_reentry;read_reentry=completion_reentry;
    EnterCriticalSection(&s->lock);s->fixture_tool_pause=boundary<AI_TOOL_RUNNING?boundary:0;LeaveCriticalSection(&s->lock);
    CHECK(submit_closeout()==UI_STATUS_OK);CHECK(real_wait("\"phase\":3"));
    CHECK(ui_ai_model_fixture(s,2,run)==UI_STATUS_OK);
    DWORD begin=GetTickCount();int arrived=0;
    do{
        pump();EnterCriticalSection(&s->lock);
        arrived=completion_reentry?executes>0:read_async?pending_read!=0:
            s->fixture_tool_seen==boundary&&s->tool.state==boundary;
        LeaveCriticalSection(&s->lock);if(arrived)break;
    }while(GetTickCount()-begin<5000);
    CHECK(arrived);if(!arrived)return;
    ai_run_t *r=general_run();CHECK(!r->graph&&s->model_config->requests==1);
    if(completion_reentry){CHECK(s->tool.state==AI_TOOL_RESULT&&r->model->assistant->control==AS_CONTROL_DRAIN);}
    else if(dispatch_reentry){callback_action=CO_TAKEOVER;callback_count=0;callback_at=1;}
    else CHECK(ui_ai_assistant_control(s,run,UI_AI_ASSISTANT_TAKEOVER)==UI_STATUS_PENDING);
    EnterCriticalSection(&s->lock);s->fixture_tool_pause=0;LeaveCriticalSection(&s->lock);
    if(read_async){
        begin=GetTickCount();
        do{pump();if(r->model->assistant->control==AS_CONTROL_MANUAL&&s->tool.state)++manual_before_saved;}while(!cancels&&GetTickCount()-begin<5000);
        CHECK(cancels==1&&r->model->assistant->control==AS_CONTROL_DRAIN&&pending_read);
        complete_read(pending_read);pending_read=0;
    }
    begin=GetTickCount();
    do{
        pump();if(r->model->assistant->control==AS_CONTROL_MANUAL&&s->tool.state)++manual_before_saved;
        if(!s->tool.state&&r->model->assistant->control==AS_CONTROL_MANUAL)break;
    }while(GetTickCount()-begin<5000);
    CHECK(manual_before_saved==0);
    CHECK(!s->tool.state&&r->model->assistant->control==AS_CONTROL_MANUAL);
    CHECK(s->model_config->requests==1&&r->model->assistant->used==1);
    CHECK(executes==(read_async||completion_reentry?1u:0u));CHECK(completions==executes);
    if(!r->terminal){CHECK(ui_ai_cancel(s,run)==UI_STATUS_OK);CHECK(real_wait("\"terminal\":true"));}
    if(dispatch_reentry)CHECK(callback_count>=1&&callback_at==0);
    printf("preparation boundary=%d completion_reentry=%d dispatch_reentry=%d executes=%u cancels=%u completions=%u early_manual=%u\n",
           boundary,completion_reentry,dispatch_reentry,executes,cancels,completions,manual_before_saved);
}

static void graph_handoff_case(int reentrant,int fail_save)
{
    graph_reentry=reentrant;read_async=!reentrant;
    if(!ready_plan()){CHECK(0);return;}
    ai_run_t *r=general_run();ai_graph_t *g=r->graph;ai_coordinator_t *c=s->coordinator;
    CHECK(ui_ai_assistant_fixture_confirm(s,run,1)==UI_STATUS_OK);
    DWORD begin=GetTickCount();int arrived=0;
    do{pump();arrived=executes==1&&(reentrant?!g->nodes[0].call:pending_read!=0);if(arrived)break;}while(GetTickCount()-begin<5000);
    CHECK(arrived);if(!arrived)return;
    if(!reentrant){
        CHECK(ui_ai_assistant_control(s,run,UI_AI_ASSISTANT_TAKEOVER)==UI_STATUS_PENDING);
        begin=GetTickCount();
        do{pump();if(!g->dirty&&g->persisted==g->version&&!c->job&&!c->busy&&!c->done)break;}while(GetTickCount()-begin<5000);
        CHECK(pending_read&&g->nodes[0].call&&r->model->assistant->control==AS_CONTROL_DRAIN);
        ui_ai_graph_fixture(s,1,1);complete_read(pending_read);pending_read=0;
    }
    begin=GetTickCount();
    do{pump();if(!g->nodes[0].call&&g->nodes[0].state==AG_COLLECTED&&c->job==3&&c->run_ref)break;}while(GetTickCount()-begin<5000);
    CHECK(!g->nodes[0].call&&g->nodes[0].state==AG_COLLECTED&&c->job==3&&c->run_ref);
    graph_saved_before=g->persisted;CHECK(g->dirty&&g->version>g->persisted&&r->refs>0);
    yyjson_doc *result=g->nodes[0].result?yyjson_read(g->nodes[0].result,strlen(g->nodes[0].result),0):NULL;
    CHECK(result&&yyjson_get_int(yyjson_obj_get(yyjson_doc_get_root(result),"value"))==41);
    CHECK(result&&!strcmp(yyjson_get_str(yyjson_obj_get(yyjson_doc_get_root(result),"revision")),"r1"));
    yyjson_doc_free(result);
    ui_cap_result_t forgotten={0};forgotten.size=sizeof(forgotten);forgotten.version=1;
    CHECK(ui_capability_get_result(apps[0].host,g->nodes[0].last_call,&forgotten)==UI_STATUS_NOT_FOUND);
    uint64_t version=g->version;
    CHECK(ui_ai_assistant_control(s,run,UI_AI_ASSISTANT_TAKEOVER)==UI_STATUS_PENDING);
    CHECK(g->version==version);pump();
    char *state_json=NULL;CHECK(ui_ai_assistant_get(s,run,&state_json)==UI_STATUS_OK);
    CHECK(state_json&&strstr(state_json,"\"control\":1"));ui_ai_release(state_json);
    CHECK(r->model->assistant->control==AS_CONTROL_DRAIN&&c->run_ref&&r->refs>0);
    CHECK(s->methods==0&&s->model_config->requests==1&&r->model->assistant->used==1);
    printf("graph save held reentry=%d control=%d persisted=%llu version=%llu refs=%d result=41/r1\n",reentrant,r->model->assistant->control,(unsigned long long)g->persisted,(unsigned long long)g->version,r->refs);
    if(fail_save)ui_ai_graph_fixture(s,2,3);
    ui_ai_graph_fixture(s,1,0);begin=GetTickCount();
    do{
        pump();
        if(fail_save?s->store_failed:(!g->dirty&&g->persisted==g->version&&!c->run_ref&&r->model->assistant->control==AS_CONTROL_MANUAL))break;
    }while(GetTickCount()-begin<5000);
    if(fail_save){
        CHECK(s->store_failed&&s->phase==UI_AI_UNAVAILABLE&&!r->committed);
        CHECK(r->model->assistant->control!=AS_CONTROL_MANUAL&&g->persisted==graph_saved_before);
        char *error_json=NULL;CHECK(ui_ai_coordinator_get_state(s,&error_json)==UI_STATUS_OK);
        CHECK(error_json&&strstr(error_json,"storage_unconfirmed"));ui_ai_release(error_json);graph_save_failed=1;
    }else{
        CHECK(!g->dirty&&g->persisted==g->version&&!c->run_ref&&r->refs==0);
        CHECK(r->model->assistant->control==AS_CONTROL_MANUAL);
        CHECK(ui_ai_cancel(s,run)==UI_STATUS_OK);CHECK(real_wait("\"terminal\":true"));
    }
    CHECK(executes==1&&completions==1&&s->model_config->requests==1&&r->model->assistant->used==1);
}

int wmain(int argc,wchar_t **argv)
{
    if(argc!=4)return 2;
    CHECK(ui_framework_initialize()==UI_STATUS_OK);cr_package=argv[1];cr_directory=argv[2];
    setup(argv[1],argv[2]);general_model();register_read();
    const wchar_t *kind=argv[3];int preparation=!wcsncmp(kind,L"read-",5);
    closeout_responses(preparation);
    if(!wcscmp(kind,L"query-close"))query_case(CO_QUERY_CLOSE,0);
    else if(!wcscmp(kind,L"query-invalid"))query_case(CO_CHANGED,0);
    else if(!wcscmp(kind,L"query-normal"))query_case(CO_LEASE,0);
    else if(!wcscmp(kind,L"initial-normal"))query_case(CO_LEASE,1);
    else if(!wcscmp(kind,L"initial-takeover"))query_case(CO_TAKEOVER,1);
    else if(!wcscmp(kind,L"initial-invalid"))query_case(CO_CHANGED,1);
    else if(!wcscmp(kind,L"initial-close"))query_case(CO_QUERY_CLOSE,1);
    else if(!wcsncmp(kind,L"graph-save",10))graph_handoff_case(!wcscmp(kind,L"graph-save-reentry"),!wcscmp(kind,L"graph-save-failure"));
    else if(preparation){
        int boundary=!wcscmp(kind,L"read-prepare")?AI_TOOL_PREPARE:
            !wcscmp(kind,L"read-prepared")?AI_TOOL_PREPARED:
            !wcscmp(kind,L"read-dispatch")||!wcscmp(kind,L"read-dispatch-reentry")?AI_TOOL_DISPATCH:AI_TOOL_RUNNING;
        preparation_case(boundary,!wcscmp(kind,L"read-reentry"),!wcscmp(kind,L"read-dispatch-reentry"));
    }
#ifdef GENERAL_CLOSEOUT_LIGHT
    else{
        int action=!wcscmp(kind,L"confirm-detail")?CO_HIDE_DETAIL:
            !wcscmp(kind,L"confirm-range")?CO_HIDE_RANGE:
            !wcscmp(kind,L"confirm-generation")?CO_GENERATION:
            !wcscmp(kind,L"confirm-takeover")?CO_TAKEOVER:
            !wcscmp(kind,L"confirm-close")?CO_CLOSE:CO_NONE;
        confirm_case(action);
    }
#endif
    if(s)close_service();
    CHECK(cr_sql("SELECT count(*) FROM events WHERE kind='assistant_reserved'")==
          cr_sql("SELECT count(*) FROM events WHERE kind='assistant_transport_entry'"));
    CHECK(cr_sql("SELECT count(*) FROM events WHERE terminal=1")==!graph_save_failed);
    if(!wcsncmp(kind,L"initial-",8)){
        CHECK(cr_sql("SELECT count(*) FROM events WHERE kind IN ('disclosure','model_request','assistant_reserved','assistant_transport_entry')")==0);
    }
    if(!wcsncmp(kind,L"graph-save",10)){
        CHECK(cr_sql("SELECT count(*) FROM tools")==0);
        CHECK(cr_sql("SELECT count(*) FROM ai_graphs WHERE json_extract(payload,'$.nodes[0].result.value')=41")==!graph_save_failed);
    }
    if(preparation){
        CHECK(cr_sql("SELECT count(*) FROM tools")==1);
        CHECK(cr_sql("SELECT count(*) FROM tools WHERE result LIKE '%41%'")==(executes?1:0));
    }
    CHECK(ui_workspace_close_all(w)==UI_STATUS_OK);DWORD begin=GetTickCount();
    while(ui_workspace_count(w)&&GetTickCount()-begin<5000){ui_workspace_poll(w);(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}
    CHECK(ui_workspace_count(w)==0);CHECK(ui_workspace_destroy(w)==UI_STATUS_OK);
    printf("closeout %ls callbacks=%d lease=%d destroy_attempted=%d checks=%u failures=%u\n",
           kind,callback_count,callback_lease,query_destroy_attempted,checks,failures);
    return failures?1:0;
}
