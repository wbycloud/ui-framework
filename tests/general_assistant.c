/* Isolated model/credential/confirmation fixtures. No real device or Provider. */
#include "ui_framework/ai_assistant.h"
#include "br2_meter_config.h"
static int general_unpriced,general_read_first,general_bad_agent,general_bad_specialist;
static int general_expired;
static ui_status_t general_open(ui_ai_service_t *service)
{
    ui_ai_assistant_policy_t p={0};p.size=sizeof(p);p.version=1;
    p.request_limit=8;p.deadline_ms=600000;p.budget_nano_usd=100000000;
    p.input_rate_nano_usd=general_unpriced?0:100;p.output_rate_nano_usd=general_unpriced?0:200;
    p.price_version=general_expired?"release-until-20000101-isolated":"isolated-general-price-v1";
    p.price_evidence_sha256="1111111111111111111111111111111111111111111111111111111111111111";
    p.meter_profile="deepseek-v41-responses-1017-v1";p.meter_evidence_sha256=BR2_METER_EVIDENCE;
    ui_status_t status=ui_ai_assistant_enable(service,&p);
    return status?status:ui_ai_service_open(service);
}
#define ui_ai_service_open general_open
#define AI_MODEL_MAIN general_legacy_model_main
#include "ai_model.c"
#undef ui_ai_service_open
#undef AI_MODEL_MAIN
#include "ai_assistant_internal.h"
__declspec(dllimport) ui_status_t ui_ai_assistant_fixture_confirm(ui_ai_service_t *,const char *,int);

#include "general_test_wire.h"

static void general_responses(int question,int invalid_plan,int bad_quote,int missing_usage)
{
    char step[5000],plan[6000];
    node(step,sizeof(step),"read","read-op",invalid_plan?"memory.set":"memory.read",0,42,
      ",\"agent_profile\":\"specialist\",\"instruction\":\"只核对本节点\",\"context\":\"node-context\"");
    snprintf(plan,sizeof(plan),"{\"nodes\":[%s]}",step);
    const char *agent="{\"decision\":\"execute\",\"summary\":\"按固定只读范围读取\"}";
    if(general_bad_agent)agent="{\"decision\":\"propose\",\"summary\":\"越界候选\",\"capability\":\"memory.set\",\"params\":{\"value\":99}}";
    const char *ask="{\"decision\":\"ask\",\"summary\":\"需补充用途\",\"question\":\"请说明本次读取用途\"}";
    const char *result=bad_quote?
      "{\"conclusion\":\"完成\",\"facts\":[{\"node\":\"read\",\"quote\":\"嵌入式进程内\"}],\"todos\":[],\"inferences\":[]}":
      "{\"conclusion\":\"已核对合成只读结果\",\"facts\":[{\"node\":\"read\",\"quote\":\"r1\"}],\"todos\":[],\"inferences\":[\"用途说明属于模型解读\"]}";
    const char *post="{\"summary\":\"已核对当前节点结果\",\"quotes\":[\"r1\"],\"inferences\":[]}";
    if(general_bad_specialist)post="{\"summary\":\"未经依据的专业解读\",\"quotes\":[\"嵌入式进程内\"],\"inferences\":[]}";
    const char *texts[5]={plan,question?ask:agent,question?agent:post,question?post:result,result};
    yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *a=yyjson_mut_arr(d);yyjson_mut_doc_set_root(d,a);
    if(general_read_first)yyjson_mut_arr_add_strcpy(d,a,rp_read);
    for(unsigned i=0;i<(question?5u:4u);++i){char *wire=general_wire(texts[i],missing_usage&&i==0);CHECK(wire!=NULL);if(wire)yyjson_mut_arr_add_strcpy(d,a,wire);free(wire);}
    char *json=yyjson_mut_write(d,0,NULL);CHECK(json!=NULL);
    if(json)CHECK(ui_ai_model_fixture(s,1,json)==UI_STATUS_OK);
    CHECK(ui_ai_model_fixture(s,5,"fixture-key-no-leak")==UI_STATUS_OK);
    free(json);yyjson_mut_doc_free(d);
}

static void general_model(void)
{
    model_setup();char *json=NULL,ref[128];CHECK(ui_ai_model_get_state(s,&json)==UI_STATUS_OK);
    str(json,"credential_ref",ref,sizeof(ref));ui_ai_release(json);
    ui_ai_model_profile_t p={0};p.size=sizeof(p);p.version=1;p.profile_id="deepseek";
    p.origin="https://api.deepseek.com";p.protocol_profile="deepseek-responses-v1";
    p.model_id="deepseek-flash";p.credential_ref=ref;p.revision=s->model_config->revision+1;
    CHECK(ui_ai_model_configure(s,&p,"general-model")==UI_STATUS_OK);CHECK(ready_model());
}

static int general_wait(const char *fragment)
{
    DWORD begin=GetTickCount();
    do{
        char *json=NULL;pump();
        if(!ui_ai_assistant_get(s,run,&json)){
            int found=strstr(json,fragment)!=NULL;strcpy_s(state,sizeof(state),json);ui_ai_release(json);
            if(found)return 1;
        }
    }while(GetTickCount()-begin<5000);
    fprintf(stderr,"general wait %s actual %.6000s\n",fragment,state);return 0;
}

static ui_status_t general_submit(void)
{
    ui_cap_target_t reads[2]={0};
    for(unsigned i=0;i<2;++i){
        reads[i].size=sizeof(reads[i]);reads[i].version=1;
        CHECK(ui_capability_bind(apps[i].host,"memory.document",&reads[i])==UI_STATUS_OK);
        strcpy_s(targets[i].scope_json,sizeof(targets[i].scope_json),"{\"access\":\"read\"}");
    }
    char attachment[]="attachment-original";
    ui_ai_assistant_attachment_t file={0};file.size=sizeof(file);file.version=1;
    file.name_utf8="合成说明.txt";file.media_type="text/plain";file.bytes=attachment;file.byte_count=strlen(attachment);
    ui_ai_assistant_request_t q={0};q.size=sizeof(q);q.version=1;
    q.task.size=sizeof(q.task);q.task.version=1;q.task.client_request_id="general-task";
    q.task.input_utf8="通用首版隔离任务";q.task.profile_id="deepseek";q.task.profile_revision=s->model_config->revision;
    q.task.targets=reads;q.task.target_count=2;q.task.history_selection_json="[]";
    q.task.disclosure_projection_json="{\"capabilities\":[\"memory.read\"],\"fields\":[\"value\",\"revision\"]}";
    q.targets=targets;q.target_count=2;
    q.permissions_json="[{\"capabilities\":[\"memory.read\"],\"profiles\":[\"specialist\"]},{\"capabilities\":[\"memory.read\"],\"profiles\":[\"specialist\"]}]";
    static const unsigned char png[]={0x89,0x50,0x4e,0x47,0xd,0xa,0x1a,0xa,0x0,0x0,0x0,0xd,0x49,0x48,0x44,0x52,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x1,0x8,0x4,0x0,0x0,0x0,0xb5,0x1c,0xc,0x2,0x0,0x0,0x0,0xb,0x49,0x44,0x41,0x54,0x78,0xda,0x63,0xfc,0xff,0x1f,0x0,0x3,0x3,0x2,0x0,0xef,0xa3,0xc0,0xa4,0x0,0x0,0x0,0x0,0x49,0x45,0x4e,0x44,0xae,0x42,0x60,0x82};
    ui_ai_assistant_attachment_t files[2]={file,{0}};files[1].size=sizeof(files[1]);files[1].version=1;
    files[1].name_utf8="仅本地.png";files[1].media_type="image/png";files[1].bytes=png;files[1].byte_count=sizeof(png);
    char hash[65];CHECK(ui_ai_assistant_attachment_hash(&files[1],hash)==UI_STATUS_OK&&strlen(hash)==64);
    q.attachments=files;q.attachment_count=2;
    ui_status_t status=ui_ai_assistant_propose(s,&q,run);
    memset(attachment,'x',strlen(attachment));return status;
}

static int general_plan(int question)
{
    if(!real_wait("\"phase\":3"))return 0;
    CHECK(s->model_config->requests==0);
    char *json=NULL;CHECK(ui_ai_model_get_disclosure(s,run,&json)==UI_STATUS_OK);
    CHECK(json&&strstr(json,"attachment-original")&&strstr(json,"allowed_application_scope"));
    yyjson_doc *preview=json?yyjson_read(json,strlen(json),0):NULL;
    char *request=preview?yyjson_val_write(yyjson_obj_get(yyjson_doc_get_root(preview),"request"),0,NULL):NULL;
    CHECK(request&&strstr(request,"仅本地.png")==NULL);free(request);yyjson_doc_free(preview);ui_ai_release(json);
    CHECK(general_wait("仅本地.png"));
    CHECK(ui_ai_model_fixture(s,2,run)==UI_STATUS_OK);
    if(question){
        if(!general_wait("请说明本次读取用途"))return 0;
        CHECK(ui_ai_graph_answer(s,run,"read","wrong-question","answer-1","核对版本")==UI_STATUS_PERMISSION_DENIED);
        CHECK(ui_ai_graph_answer(s,run,"read","q-read-op","answer-1","核对版本")==UI_STATUS_OK);
        CHECK(ui_ai_graph_answer(s,run,"read","q-read-op","answer-1","核对版本")==UI_STATUS_OK);
        CHECK(ui_ai_graph_answer(s,run,"read","q-read-op","answer-1","changed")==UI_STATUS_ALREADY_EXISTS);
    }
    if(!general_wait("\"plan_waiting\":true"))return 0;
    return 1;
}

static ai_run_t *general_run(void)
{
    for(unsigned i=0;i<AI_RUNS;++i)if(!strcmp(s->runs[i].id,run))return &s->runs[i];
    return NULL;
}

static void general_text_case(int unexpected_tool)
{
    const char *input="请用一句话解释什么是只读。";
    char *wire=unexpected_tool?general_read_wire("memory.read"):general_wire("只读可以查看数据。",0);
    yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *frames=yyjson_mut_arr(d);
    yyjson_mut_doc_set_root(d,frames);
    CHECK(wire&&yyjson_mut_arr_add_strcpy(d,frames,wire));
    char *fixture=yyjson_mut_write(d,0,NULL);
    CHECK(fixture&&ui_ai_model_fixture(s,1,fixture)==UI_STATUS_OK);
    CHECK(ui_ai_model_fixture(s,5,"isolated-chat-sentinel")==UI_STATUS_OK);
    free(fixture);free(wire);yyjson_mut_doc_free(d);
    ui_ai_assistant_request_t q={0};q.size=sizeof(q);q.version=1;
    q.task.size=sizeof(q.task);q.task.version=1;q.task.client_request_id="text-only";
    q.task.input_utf8=input;q.task.profile_id="deepseek";q.task.profile_revision=s->model_config->revision;
    q.task.history_selection_json="[]";q.permissions_json="[]";
    q.task.disclosure_projection_json="{\"fields\":[\"value\"],\"capabilities\":[]}";
    CHECK(ui_ai_assistant_propose(s,&q,run)==UI_STATUS_VALIDATION_FAILED);
    q.task.disclosure_projection_json="{\"capabilities\":[],\"fields\":[]}";
    CHECK(ui_ai_assistant_propose(s,&q,run)==UI_STATUS_OK);
    int waiting=real_wait("\"phase\":3");CHECK(waiting);if(!waiting)return;
    ai_run_t *r=general_run();CHECK(r&&r->target_count==0&&!r->graph);
    char *preview=NULL;CHECK(ui_ai_assistant_get_disclosure(s,run,&preview)==UI_STATUS_OK);
    yyjson_doc *pd=preview?yyjson_read(preview,strlen(preview),0):NULL;
    yyjson_val *root=yyjson_doc_get_root(pd),*req=yyjson_obj_get(root,"request");
    yyjson_val *tools=yyjson_obj_get(req,"tools"),*items=yyjson_obj_get(req,"input");
    CHECK(yyjson_equals_str(yyjson_obj_get(root,"fixed_targets"),"[]"));
    CHECK(yyjson_is_arr(tools)&&!yyjson_arr_size(tools));
    CHECK(yyjson_arr_size(items)==1&&yyjson_equals_str(yyjson_obj_get(yyjson_arr_get(items,0),"content"),input));
    CHECK(s->model_config->requests==0);
    if(general_expired)CHECK(general_wait("\"price_ready\":false"));
    CHECK(ui_ai_model_fixture(s,2,run)==UI_STATUS_OK);
    CHECK(ui_ai_model_fixture(s,2,run)!=UI_STATUS_OK);
    CHECK(real_wait("\"terminal\":true"));
    CHECK(s->model_config->requests==(general_expired?0u:1u));
    if(!general_expired){
        yyjson_doc *bd=yyjson_read(s->model_config->fixture_body,strlen(s->model_config->fixture_body),0);
        CHECK(req&&yyjson_equals(req,yyjson_doc_get_root(bd)));yyjson_doc_free(bd);
    }
    if(general_expired)CHECK(general_wait("\"error\":\"assistant_send_conditions\""));
    else if(unexpected_tool)CHECK(strstr(state,"assistant_unplanned_function")!=NULL);
    else CHECK(general_wait("\"result\":\"只读可以查看数据。\""));
    CHECK(value(0)==10&&value(1)==10);
    yyjson_doc_free(pd);ui_ai_release(preview);
}

static void general_case(const wchar_t *kind)
{
    int question=!wcscmp(kind,L"question"),invalid=!wcscmp(kind,L"scope"),bad=!wcscmp(kind,L"inference"),missing=!wcscmp(kind,L"usage");
    general_responses(question,invalid,bad,missing);CHECK(general_submit()==UI_STATUS_OK);
    if(!wcscmp(kind,L"unpriced")){
        CHECK(real_wait("\"phase\":3"));CHECK(ui_ai_model_fixture(s,2,run)==UI_STATUS_OK);
        CHECK(real_wait("\"terminal\":true"));CHECK(s->model_config->requests==0);return;
    }
    if(invalid||missing||general_bad_agent){
        CHECK(real_wait("\"phase\":3"));CHECK(ui_ai_model_fixture(s,2,run)==UI_STATUS_OK);
        CHECK(real_wait("\"terminal\":true"));CHECK(s->model_config->requests==(general_bad_agent?2u:1u));
        CHECK(value(0)==10&&value(1)==10);return;
    }
    if(!wcscmp(kind,L"manual-before")){
        CHECK(real_wait("\"phase\":3"));CHECK(ui_ai_assistant_control(s,run,UI_AI_ASSISTANT_TAKEOVER)==UI_STATUS_PENDING);
        CHECK(general_wait("\"control\":2"));CHECK(ui_ai_model_fixture(s,2,run)==UI_STATUS_PERMISSION_DENIED);CHECK(s->model_config->requests==0);
        CHECK(ui_ai_assistant_control(s,run,UI_AI_ASSISTANT_RETURN)==UI_STATUS_OK);
    }
    int planned=general_plan(question);
    CHECK(planned);
    if(!planned)return;
    ai_run_t *r=general_run();CHECK(r&&r->graph);if(!r||!r->graph)return;
    DWORD begin=GetTickCount();
    do{pump();}while((r->graph->dirty||r->graph->persisted!=r->graph->version)&&GetTickCount()-begin<5000);
    CHECK(!r->graph->dirty&&r->graph->persisted==r->graph->version);
    CHECK(ui_ai_assistant_fixture_confirm(s,run,0)==UI_STATUS_PERMISSION_DENIED);
    CHECK(s->model_config->requests==(question?3u:2u)+(general_read_first?1:0));
    if(!wcscmp(kind,L"takeover")||!wcscmp(kind,L"manual-change")){
        CHECK(ui_ai_assistant_control(s,run,UI_AI_ASSISTANT_TAKEOVER)==UI_STATUS_PENDING);
        CHECK(general_wait("\"control\":2"));CHECK(s->model_config->requests==2);
        if(!wcscmp(kind,L"manual-change")){CHECK(ui_workspace_post_app_data(w,apps[0].instance_id,AE_APP_BUMP_REVISION,NULL,0)==UI_STATUS_OK);ui_workspace_poll(w);}
        ui_status_t returned=ui_ai_assistant_control(s,run,UI_AI_ASSISTANT_RETURN);
        if(!wcscmp(kind,L"manual-change")){CHECK(returned==UI_STATUS_VALIDATION_FAILED);CHECK(real_wait("\"terminal\":true"));return;}
        CHECK(returned==UI_STATUS_OK);
    }
    begin=GetTickCount();do{pump();}while((r->graph->dirty||r->graph->persisted!=r->graph->version||s->coordinator->job||s->coordinator->busy||s->coordinator->done)&&GetTickCount()-begin<5000);
    if(!wcscmp(kind,L"deadline")){
        ui_ai_edit_fixture(s,3,600001);CHECK(ui_ai_assistant_fixture_confirm(s,run,1)==UI_STATUS_PERMISSION_DENIED);
        CHECK(ui_ai_cancel(s,run)==UI_STATUS_OK);CHECK(real_wait("\"terminal\":true"));CHECK(s->model_config->requests==2);return;
    }
    if(!wcscmp(kind,L"quota")){
        /* The isolated bound policy is reduced after specialist preparation to
           exercise the common actual-send boundary without changing a model output. */
        r->model->assistant->policy.requests=2;
    }
    if(!wcscmp(kind,L"cancel")){CHECK(ui_ai_cancel(s,run)==UI_STATUS_OK);CHECK(real_wait("\"terminal\":true"));CHECK(s->model_config->requests==2);return;}
    if(!wcscmp(kind,L"storage")){
        EnterCriticalSection(&s->lock);s->fixture_fail_write=1;LeaveCriticalSection(&s->lock);CHECK(ui_ai_cancel(s,run)==UI_STATUS_OK);
        DWORD until=GetTickCount();while(!s->store_failed&&GetTickCount()-until<5000)pump();
        CHECK(s->store_failed&&s->model_config->requests==2);return;
    }
    CHECK(ui_ai_assistant_fixture_confirm(s,run,1)==UI_STATUS_OK);
    CHECK(ui_ai_assistant_fixture_confirm(s,run,1)==UI_STATUS_PERMISSION_DENIED);
    if(!wcscmp(kind,L"takeover-charged")){
        begin=GetTickCount();do{pump();}while(!r->graph->nodes[0].charged&&GetTickCount()-begin<5000);
        CHECK(r->graph->nodes[0].charged&&!r->graph->nodes[0].call&&!r->graph->nodes[0].tool_phase);
        CHECK(ui_ai_assistant_control(s,run,UI_AI_ASSISTANT_TAKEOVER)==UI_STATUS_PENDING);CHECK(general_wait("\"control\":2"));
        CHECK(!r->graph->nodes[0].call&&!r->graph->nodes[0].tool_phase&&s->model_config->requests==2);
        CHECK(ui_ai_assistant_control(s,run,UI_AI_ASSISTANT_RETURN)==UI_STATUS_OK);
    }
    CHECK(real_wait("\"terminal\":true"));
    if(general_bad_specialist){CHECK(s->model_config->requests==3);CHECK(strstr(r->model->assistant->error,"unverified_specialist_result"));return;}
    if(!wcscmp(kind,L"quota")){CHECK(s->model_config->requests==2&&r->model->assistant->used==2);CHECK(strstr(r->model->assistant->error,"request_limit"));return;}
    CHECK(general_wait(bad?"assistant_unverified_result":"原文没有待办"));
    CHECK(s->model_config->requests==(question?5u:4u)+(general_read_first?1:0));CHECK(value(0)==10&&value(1)==10);
    CHECK(strstr(s->model_config->fixture_body,"permitted_evidence")!=NULL);
    CHECK(strstr(s->model_config->fixture_body,"attachment-original")==NULL);
    CHECK(r->model->assistant->used==(question?5u:4u)+(general_read_first?1:0));
    CHECK(!secret_leaked);
    if(!wcscmp(kind,L"config-change")){
        char *before=_strdup(r->plan);uint64_t revision=r->model->revision;
        ui_ai_model_profile_t p={0};p.size=sizeof(p);p.version=1;p.profile_id="deepseek";p.origin="https://api.deepseek.com";
        p.protocol_profile="deepseek-responses-v1";p.model_id="next-isolated-model";p.credential_ref=s->model_config->ref;p.revision=s->model_config->revision+1;
        CHECK(ui_ai_model_configure(s,&p,"after-root-config")==UI_STATUS_OK);CHECK(ready_model());
        CHECK(!strcmp(before,r->plan)&&r->model->revision==revision&&!strcmp(r->model->model,"deepseek-flash"));free(before);
    }
}

#ifndef GENERAL_ASSISTANT_MAIN
#define GENERAL_ASSISTANT_MAIN wmain
#endif
int GENERAL_ASSISTANT_MAIN(int argc,wchar_t **argv)
{
    if(argc!=4)return 2;
    CHECK(ui_framework_initialize()==UI_STATUS_OK);cr_package=argv[1];cr_directory=argv[2];
    general_bad_agent=!wcscmp(argv[3],L"specialist-scope");general_bad_specialist=!wcscmp(argv[3],L"specialist-inference");
    general_read_first=!wcscmp(argv[3],L"preparation-read");general_unpriced=!wcscmp(argv[3],L"unpriced");
    general_expired=!wcscmp(argv[3],L"text-expired");
    int chat=!wcsncmp(argv[3],L"text",4),unexpected_tool=!wcscmp(argv[3],L"text-tool");
    setup(argv[1],argv[2]);general_model();
    if(chat)general_text_case(unexpected_tool);else general_case(argv[3]);
    if(s)close_service();
    CHECK(cr_sql("SELECT count(*) FROM events WHERE kind='assistant_reserved'")>=((general_unpriced||general_expired)?0:1));
    CHECK(cr_sql("SELECT count(*) FROM events WHERE kind='assistant_reserved'")==cr_sql("SELECT count(*) FROM events WHERE kind='assistant_transport_entry'"));
    if(wcscmp(argv[3],L"storage"))CHECK(cr_sql("SELECT count(*) FROM events WHERE terminal=1")==1);
    if(chat){
        CHECK(cr_sql("SELECT count(*) FROM tools")==0);
        CHECK(cr_sql("SELECT count(*) FROM ai_graphs")==0);
        CHECK(cr_sql("SELECT count(*) FROM events WHERE kind='assistant_reserved'")==!general_expired);
        if(!general_expired)CHECK(cr_sql("SELECT count(*) FROM events WHERE kind='assistant_cost' AND json_extract(payload,'$.known_nano_usd')=1800")==1);
        if(!general_expired&&!unexpected_tool)
            CHECK(cr_sql("SELECT count(*) FROM runs WHERE outcome='completed' AND reply='只读可以查看数据。'")==1);
    }
    CHECK(ui_workspace_close_all(w)==UI_STATUS_OK);DWORD begin=GetTickCount();
    while(ui_workspace_count(w)&&GetTickCount()-begin<5000){ui_workspace_poll(w);MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}
    CHECK(ui_workspace_destroy(w)==UI_STATUS_OK);
    printf("General assistant checks=%u failures=%u; isolated model and credential only\n",checks,failures);
    return failures?1:0;
}
