/* Baseline repro: a conversational reply cannot satisfy a real edit goal. */
static int host_operation_red(host_window_t *s,const wchar_t *package)
{
    open_path(s,package);pump(s);
    uint64_t instance=ui_workspace_active(s->workspace);
    if(!instance){CHECK(instance);CHECK(close_host(s));return 2;}
    unsigned before=failures;
    cell_expect(s,instance,"42","doc-1");
    if(failures!=before){CHECK(close_host(s));return 2;}
    const char *claim="指定单元格已经改为已复核。";
    yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *responses=yyjson_mut_arr(d);
    yyjson_mut_doc_set_root(d,responses);char *wire=general_wire(claim,0);
    CHECK(wire&&yyjson_mut_arr_add_strcpy(d,responses,wire));free(wire);
    char *script=yyjson_mut_write(d,0,NULL);yyjson_mut_doc_free(d);
    CHECK(script&&ui_ai_model_fixture(s->ai,1,script)==UI_STATUS_OK);free(script);
    CHECK(ui_ai_model_fixture(s->ai,5,"operation-isolated-model")==UI_STATUS_OK);
    CHECK(host_daily_propose(s,"把指定单元格改为已复核，再读回核对。",1));
    CHECK(host_general_binding(s,0));
    CHECK(ui_ai_model_fixture_light(s->ai,s->view,"ai-model-confirm")==UI_STATUS_OK);
    CHECK(model_wait(s,"\"outcome\":\"completed\""));
    CHECK(host_general_wait(s,claim));
    CHECK(s->ai->model_config->requests==1&&model_wait(s,"\"tool_calls\":0"));
    yyjson_doc *response=host_daily_json(s->ai->model_config->fixture_body);
    yyjson_val *tools=yyjson_obj_get(yyjson_doc_get_root(response),"tools");
    CHECK(yyjson_is_arr(tools)&&!yyjson_arr_size(tools));yyjson_doc_free(response);
    if(failures!=before){CHECK(close_host(s));return 2;}
    fprintf(stderr,"Red direct: conversational reply saved, zero tools. Now require the actual edit.\n");
    cell_expect(s,instance,"已复核","doc-2");
    CHECK(close_host(s));
    fprintf(stderr,"Operation red checks=%u failures=%u\n",checks,failures);
    return failures?1:0;
}

typedef struct operation_case {
    const char *plan,*expected;int eda,illegal;
    unsigned initial;LONG rounds;
} operation_case_t;

static int host_operation_reply(void *data,unsigned round,const char *body,char *out,size_t capacity)
{
    operation_case_t *c=data;const char *reply=c->plan;char *generated=NULL;
    yyjson_doc *request=host_daily_json(body),*context=NULL,*approval=NULL;
    yyjson_val *root=yyjson_doc_get_root(request),*tools=yyjson_obj_get(root,"tools"),*input=yyjson_obj_get(root,"input");
    int ok=yyjson_is_arr(tools)&&!yyjson_arr_size(tools);
    if(round==1){
        approval=host_daily_json(yyjson_get_str(yyjson_obj_get(yyjson_arr_get(input,0),"content")));
        ok=ok&&yyjson_is_str(yyjson_obj_get(yyjson_doc_get_root(approval),"operation_declaration"))&&
            (c->eda||!c->initial||strstr(body,c->expected)==NULL);
        fprintf(stderr,"Operation planner request=%u tools=0 verified_app_values_absent=%d\n",round,ok);
    }else{
        context=host_daily_json(yyjson_get_str(yyjson_obj_get(yyjson_arr_get(input,0),"content")));
        yyjson_val *evidence=yyjson_doc_get_root(context),*observations=yyjson_obj_get(evidence,"observations");
        yyjson_val *option=yyjson_arr_get(yyjson_obj_get(evidence,"permitted_actions"),0),*params=yyjson_obj_get(option,"parameters");
        const char *revision=yyjson_get_str(yyjson_obj_get(evidence,"revision"));
        const char *id=yyjson_get_str(yyjson_obj_get(option,"id"));
        const char *value=yyjson_get_str(yyjson_obj_get(params,"text"));
        unsigned count=(unsigned)yyjson_get_uint(yyjson_obj_get(yyjson_obj_get(observations,"document"),"objects"));
        ok=ok&&revision&&id&&yyjson_is_obj(params)&&yyjson_arr_size(input)==1;
        if(c->eda)ok=ok&&count==c->initial+round-2&&yyjson_obj_size(yyjson_obj_get(observations,"document"))==1&&
            !yyjson_obj_size(params)&&strstr(body,"operation_declaration")==NULL;
        else ok=ok&&value&&!strcmp(value,c->expected)&&
            (!yyjson_obj_get(observations,"source")||yyjson_equals_str(yyjson_obj_get(yyjson_obj_get(observations,"source"),"text"),c->expected));
        fprintf(stderr,"Operation next request=%u revision=%s observed_%s=%s count=%u valid=%d\n",round,
            revision?revision:"",c->eda?"count":"value",value?value:"",count,ok);
        yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *o=d?yyjson_mut_obj(d):NULL;
        if(ok&&o){
            yyjson_mut_doc_set_root(d,o);
            ok=yyjson_mut_obj_add_strcpy(d,o,"revision",revision)&&yyjson_mut_obj_add_strcpy(d,o,"action",id)&&
                yyjson_mut_obj_add_val(d,o,"parameters",yyjson_val_mut_copy(d,params));
            if(ok&&c->illegal){
                yyjson_mut_val *p=yyjson_mut_obj_get(o,"parameters");
                ok=yyjson_mut_obj_put(p,yyjson_mut_str(d,"id"),yyjson_mut_str(d,"9007199254741002"));
            }
            if(ok)generated=yyjson_mut_write(d,0,NULL);
        }
        yyjson_mut_doc_free(d);reply=generated;
    }
    char *wire=ok&&reply?general_wire(reply,0):NULL;
    ok=wire&&strlen(wire)<capacity&&!strcpy_s(out,capacity,wire);
    free(wire);free(generated);yyjson_doc_free(approval);yyjson_doc_free(context);yyjson_doc_free(request);
    if(ok)InterlockedIncrement(&c->rounds);
    return ok;
}

typedef struct operation_seed {const char *params;uint64_t call;} operation_seed_t;

static ui_status_t host_operation_collect(const ui_cap_ui_context_t *ui,char *out,size_t capacity,size_t *length,void *data)
{
    (void)ui;const char *params=((operation_seed_t *)data)->params;*length=strlen(params);
    if(*length>=capacity)return UI_STATUS_LIMIT_EXCEEDED;
    memcpy(out,params,*length+1);return UI_STATUS_OK;
}

static void host_operation_notice(const ui_cap_ui_notice_t *notice,void *data)
{
    if(notice->call_id)((operation_seed_t *)data)->call=notice->call_id;
}

static int host_operation_seed(host_window_t *s,ui_host_t *host,const char *resource,const char *capability,const char *args,unsigned sequence)
{
    char menu[64];snprintf(menu,sizeof(menu),"operation-test-seed-%u",sequence);
    ui_menu_item_desc_t item={0};item.size=sizeof(item);item.id=menu;item.title=menu;item.menu_path="Test";item.command_id=capability;
    if(ui_host_register_menu_item(host,&item))return 0;
    ui_cap_ui_binding_desc_t d={0};d.size=sizeof(d);d.version=1;d.capability_id=capability;d.menu_item_id=menu;
    operation_seed_t seed={args,0};
    d.purpose=UI_CAP_UI_INVOKE;d.collect=host_operation_collect;d.observe=host_operation_notice;d.user_data=&seed;
    d.resource.size=sizeof(d.resource);d.resource.version=2;
    uint64_t binding=0;
    if(ui_capability_resource_ref_v2(host,resource,&d.resource)||ui_capability_ui_bind(host,&d,&binding))return 0;
    ui_status_t status=ui_ai_edit_fixture_manual(host,menu,&d);
    ui_cap_result_v2_t result={0};result.size=sizeof(result);result.version=2;
    ui_status_t read=UI_STATUS_NOT_FOUND;DWORD start=GetTickCount();
    while(!status&&seed.call){
        read=ui_capability_get_result_v2(host,seed.call,&result);
        if((!read&&result.state==UI_CAP_FINISHED)||GetTickCount()-start>=2000)break;
        pump(s);
    }
    int saved=!read&&result.state==UI_CAP_FINISHED&&result.outcome==UI_CAP2_OK;
    fprintf(stderr,"Operation seed %s admit=%d call=%llu result=%d state=%d outcome=%d code=%s\n",
        capability,status,(unsigned long long)seed.call,read,result.state,result.outcome,result.code);
    int consumed=seed.call&&!ui_capability_forget_v2(host,seed.call);
    return !ui_capability_ui_begin_unbind(host,binding)&&!ui_capability_ui_finish_unbind(host,binding)&&!status&&consumed&&saved;
}

static int host_operation_read(ui_host_t *host,int eda,const char *expected,unsigned count,unsigned number)
{
    ui_cap_target_t target={0};target.size=sizeof(target);target.version=1;
    const char *resource=eda?"eda.document":"state.document",*cap=eda?"eda.document.read":"state.cell.read";
    const char *params=eda?"{}":"{\"id\":\"9007199254741000\",\"column\":\"name\"}";
    uint64_t call=0;if(ui_capability_bind(host,resource,&target)||ui_capability_prepare(host,cap,&target,params,&call))return 0;
    ui_cap_result_t result={0};result.size=sizeof(result);result.version=1;
    ui_status_t dispatched=ui_capability_dispatch(host,call),received=ui_capability_get_result(host,call,&result);
    int ok=!dispatched&&!received&&result.outcome==UI_CAP_OK;
    yyjson_doc *d=ok?host_daily_json(result.output_json):NULL;yyjson_val *o=yyjson_doc_get_root(d);
    char revision[32];snprintf(revision,sizeof(revision),"doc-%u",number);
    const char *text=yyjson_get_str(yyjson_obj_get(o,"text"));yyjson_val *objects=yyjson_obj_get(o,"objects");
    ok=ok&&d&&yyjson_equals_str(yyjson_obj_get(o,"revision"),target.expected_revision)&&!strcmp(target.expected_revision,revision)&&
        (eda?(yyjson_is_arr(objects)&&yyjson_arr_size(objects)==count):(text&&!strcmp(text,expected)));
    fprintf(stderr,"Operation actual read %s revision=%s %s=%s count=%zu dispatch=%d result=%d outcome=%d\n",resource,target.expected_revision,
        eda?"objects":"text",text?text:"",yyjson_arr_size(objects),dispatched,received,result.outcome);
    yyjson_doc_free(d);return !ui_capability_forget(host,call)&&ok;
}

static int host_operation_table_ready(host_window_t *s,ui_host_t *host)
{
    DWORD start=GetTickCount();
    do{
        ui_cap_target_t target={0};target.size=sizeof(target);target.version=1;
        uint64_t call=0;
        if(!ui_capability_bind(host,"state.preferences",&target)&&
           !ui_capability_prepare(host,"state.preferences.read",&target,"{}",&call)){
            ui_cap_result_t result={0};result.size=sizeof(result);result.version=1;
            ui_status_t dispatched=ui_capability_dispatch(host,call),received=ui_capability_get_result(host,call,&result);
            yyjson_doc *d=!dispatched&&!received&&result.outcome==UI_CAP_OK?host_daily_json(result.output_json):NULL;
            int ready=yyjson_is_false(yyjson_obj_get(yyjson_doc_get_root(d),"io_pending"));
            yyjson_doc_free(d);
            if(ui_capability_forget(host,call))return 0;
            if(ready)return 1;
        }
        pump(s);
    }while(GetTickCount()-start<2000);
    return 0;
}

static int host_operation_restore(const wchar_t *identity)
{
    char run[65];host_window_t restored={0};host_action_t select={0};
    if(wcslen(identity)!=64||!WideCharToMultiByte(CP_UTF8,0,identity,-1,run,sizeof(run),NULL,NULL))return 2;
    if(ui_framework_initialize()||!create_host(&restored,GetModuleHandleW(NULL))||!restored.ai)return 2;
    ui_ai_service_state_t state={0};state.size=sizeof(state);state.version=1;
    DWORD start=GetTickCount();
    do{pump(&restored);if(ui_ai_service_get_state(restored.ai,&state))break;}
    while(state.phase!=UI_AI_READY&&state.phase!=UI_AI_UNAVAILABLE&&GetTickCount()-start<5000);
    if(state.phase!=UI_AI_READY){
        fprintf(stderr,"Operation restart failed before READY phase=%d error=%s\n",state.phase,state.error);
        CHECK(state.phase==UI_AI_READY);restored.ai_exit_error_ack=1;CHECK(close_host(&restored));return 1;
    }
    select.action="ai-select";select.id=run;process_action(&restored,&select);
    start=GetTickCount();
    do{pump(&restored);}while(!restored.ai_stored_run&&restored.ai_query_pending&&GetTickCount()-start<5000);
    CHECK(restored.ai_stored_run&&strstr(restored.ai_stored_run,"已核对目标")&&strstr(restored.ai_stored_run,"operation_task"));
    CHECK(restored.ai->model_config&&restored.ai->model_config->requests==0&&host_daily_idle(&restored));
    char *preview=NULL;CHECK(ui_ai_assistant_get_disclosure(restored.ai,run,&preview)!=UI_STATUS_OK&&!preview);ui_ai_release(preview);
    fprintf(stderr,"Operation restart pid=%lu record=%d query_error=%s\n",(unsigned long)GetCurrentProcessId(),restored.ai_stored_run!=NULL,
        restored.ai_query_error?restored.ai_query_error:"");
    CHECK(close_host(&restored));return failures?1:0;
}

static int host_operation_restart(const char *run)
{
    wchar_t path[4096],identity[65],command[4300];STARTUPINFOW start={0};PROCESS_INFORMATION process={0};
    DWORD length=GetModuleFileNameW(NULL,path,(DWORD)(sizeof(path)/sizeof(*path)));start.cb=sizeof(start);
    if(!length||length>=sizeof(path)/sizeof(*path)||!MultiByteToWideChar(CP_UTF8,0,run,-1,identity,65)||
       swprintf_s(command,sizeof(command)/sizeof(*command),L"\"%s\" operation-restore %s",path,identity)<0)return 0;
    start.dwFlags=STARTF_USESTDHANDLES;start.hStdInput=GetStdHandle(STD_INPUT_HANDLE);
    start.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);start.hStdError=GetStdHandle(STD_ERROR_HANDLE);
    if(!CreateProcessW(path,command,NULL,NULL,TRUE,CREATE_NO_WINDOW,NULL,NULL,&start,&process))return 0;
    DWORD code=0;int ok=WaitForSingleObject(process.hProcess,17000)==WAIT_OBJECT_0&&GetExitCodeProcess(process.hProcess,&code)&&!code;
    CloseHandle(process.hThread);CloseHandle(process.hProcess);return ok;
}

static ai_run_t *host_operation_pending(host_window_t *s,const char *run,unsigned requests)
{
    DWORD start=GetTickCount();
    do{
        pump(s);EnterCriticalSection(&s->ai->lock);
        int active=s->ai->active;ai_run_t *r=active>=0?&s->ai->runs[active]:NULL;
        int pending=r&&!strcmp(r->id,run)&&r->model&&r->model->net.started&&!r->model->net.finished&&
            s->ai->model_config->requests==requests;
        LeaveCriticalSection(&s->ai->lock);
        if(pending)return r;
    }while(GetTickCount()-start<5000);
    return NULL;
}

static int host_operation_check_draining(host_window_t *s,ai_run_t *r)
{
    DWORD start=GetTickCount();
    do{
        pump(s);EnterCriticalSection(&s->ai->lock);
        am_net_t *net=&r->model->net;ai_graph_t *graph=r->graph;
        int early=r->terminal||(graph&&graph->finishing);
        int observed=early||net->error;
        int held=!early&&net->started&&!net->finished&&r->refs&&r->model->body&&r->model->stream;
        LeaveCriticalSection(&s->ai->lock);
        if(observed){fprintf(stderr,"Operation pending stop early_final=%d inflight_retained=%d\n",early,held);return held;}
    }while(GetTickCount()-start<5000);
    return 0;
}

static int host_operation_run(host_window_t *s,const wchar_t *package,const wchar_t *mode)
{
    static const char fixed[]="{\"observations\":[{\"id\":\"target\",\"parameters\":{\"id\":\"9007199254741000\",\"column\":\"name\"}}],"
        "\"actions\":[{\"id\":\"set\",\"capability\":\"test.edit\",\"readback\":\"target\",\"parameters\":{\"id\":{\"value\":\"9007199254741000\"},\"column\":{\"value\":\"name\"},\"text\":{\"value\":\"已复核\"}}}],"
        "\"goal\":{\"observation\":\"target\",\"field\":\"text\",\"equals\":{\"value\":\"已复核\"}}}";
    static const char copy[]="{\"observations\":[{\"id\":\"source\",\"parameters\":{\"id\":\"9007199254741001\",\"column\":\"name\"}},{\"id\":\"target\",\"parameters\":{\"id\":\"9007199254741000\",\"column\":\"name\"}}],"
        "\"actions\":[{\"id\":\"set\",\"capability\":\"test.edit\",\"readback\":\"target\",\"parameters\":{\"id\":{\"value\":\"9007199254741000\"},\"column\":{\"value\":\"name\"},\"text\":{\"from\":\"source\",\"field\":\"text\"}}}],"
        "\"goal\":{\"observation\":\"target\",\"field\":\"text\",\"equals\":{\"from\":\"source\",\"field\":\"text\"}}}";
    static const char eda[]="{\"observations\":[{\"id\":\"document\",\"parameters\":{}}],"
        "\"actions\":[{\"id\":\"add\",\"capability\":\"eda.add_block\",\"readback\":\"document\",\"parameters\":{}}],"
        "\"goal\":{\"observation\":\"document\",\"field\":\"objects\",\"equals\":{\"value\":3},\"progress\":\"increase\"}}";
    operation_case_t c={0};int copied=!wcsncmp(mode,L"operation-copy",14);
    int stale=!wcscmp(mode,L"operation-stale"),takeover=!wcscmp(mode,L"operation-takeover"),storage=!wcscmp(mode,L"operation-storage");
    int deadline=!wcscmp(mode,L"operation-deadline-inflight"),network_takeover=!wcscmp(mode,L"operation-takeover-inflight");
    c.illegal=!wcscmp(mode,L"operation-denied");c.eda=!wcsncmp(mode,L"operation-eda-",14);
    c.initial=c.eda?(unsigned)_wtoi(mode+14):copied;
    const char *source=copied?(!wcscmp(mode,L"operation-copy-a")?"源格·样本甲":"源格·样本乙"):NULL;
    c.expected=c.eda?NULL:copied?source:"已复核";
    c.plan=c.eda?eda:copied?copy:fixed;
    const char *message=c.eda?"查看当前图块数，补到三个。":copied?"把第 2 行名称列的值复制到第 1 行名称列，再核对。":"把第 1 行名称列改为已复核，再读回核对。";
    unsigned programs=0,edits=0,final_revision=c.initial+1;
    char run[65]={0};ui_app_instance_info_t app={0};app.size=sizeof(app);
    open_path(s,package);pump(s);
    uint64_t instance=ui_workspace_active(s->workspace);
    if(!instance||ui_workspace_get_instance(s->workspace,instance,&app)){CHECK(instance&&app.host);goto finish;}
    if(!c.eda){CHECK(host_operation_table_ready(s,app.host));if(failures)goto finish;}
    if(source){
        char args[256];snprintf(args,sizeof(args),"{\"id\":\"9007199254741001\",\"column\":\"name\",\"text\":\"%s\"}",source);
        CHECK(host_operation_seed(s,app.host,"state.document","test.edit",args,0));
    }
    if(c.eda)for(unsigned i=0;i<c.initial;++i)CHECK(host_operation_seed(s,app.host,"eda.document","eda.add_block","{}",i));
    CHECK(host_operation_read(app.host,c.eda,"editable",c.initial,final_revision));
    CHECK(ui_ai_model_fixture(s->ai,1,"[\"isolated-dynamic-response\"]")==UI_STATUS_OK);
    CHECK(ui_ai_model_fixture(s->ai,5,"operation-isolated-model")==UI_STATUS_OK);
    s->ai->model_config->fixture_reply=host_operation_reply;s->ai->model_config->fixture_reply_data=&c;
    CHECK(host_daily_visible(s,"ai-operate","代操作"));click(s,"ai-operate");
    if(failures||!host_daily_propose(s,message,1))goto finish;
    strcpy_s(run,sizeof(run),s->ai_run);
    if(!host_general_binding(s,0)){CHECK(0);goto finish;}
    CHECK(ui_ai_model_fixture(s->ai,2,run)!=UI_STATUS_OK);
    CHECK(s->ai->model_config->requests==0);
    if(deadline)CHECK(ui_ai_model_fixture(s->ai,3,"1")==UI_STATUS_OK);
    CHECK(ui_ai_model_fixture_light(s->ai,s->view,"ai-model-confirm")==UI_STATUS_OK);
    if(deadline){
        ai_run_t *pending=host_operation_pending(s,run,1);CHECK(pending!=NULL);
        if(pending){ui_ai_edit_fixture(s->ai,3,600001);CHECK(host_operation_check_draining(s,pending));}
        CHECK(ui_ai_model_fixture(s->ai,3,"0")==UI_STATUS_OK);
        CHECK(model_wait(s,"\"outcome\":\"failed\""));CHECK(host_general_wait(s,"operation_deadline"));
        CHECK(s->ai->model_config->requests==1&&host_operation_read(app.host,0,"editable",0,1));
        programs=1;goto finish;
    }
    click(s,"ai-range-close");
    if(!host_general_wait(s,"\"plan_waiting\":true")){CHECK(0);goto finish;}
    CHECK(s->ai->model_config->requests==1);
    CHECK(host_operation_read(app.host,c.eda,"editable",c.initial,final_revision));
    CHECK(ui_ai_graph_fixture_light(s->ai,s->view,"ai-model-confirm")!=UI_STATUS_OK);
    click(s,"ai-view-scope");
    if(!host_general_binding(s,1)){CHECK(0);goto finish;}
    ui_element_presentation_t approval={0};approval.size=sizeof(approval);
    ui_status_t display=ui_web_view_get_presentation(s->view,"ai-model-confirm",&approval);
    fprintf(stderr,"Operation approval label status=%d visible=%d enabled=%d text=%s\n",display,approval.visible,approval.enabled,approval.text_utf8?approval.text_utf8:"");
    CHECK(!display&&approval.enabled&&approval.text_utf8&&!strcmp(approval.text_utf8,"授权本次代操作"));
    CHECK(s->ai_preview&&strstr(s->ai_preview,"operation_review"));
    if(stale){
        CHECK(host_operation_seed(s,app.host,"state.document","test.edit","{\"id\":\"9007199254741000\",\"column\":\"name\",\"text\":\"人工改动\"}",1));
        ++final_revision;pump(s);
        CHECK(ui_ai_graph_fixture_light(s->ai,s->view,"ai-model-confirm")!=UI_STATUS_OK);
    }else if(takeover){
        CHECK(ui_ai_assistant_control(s->ai,run,UI_AI_ASSISTANT_TAKEOVER)==UI_STATUS_PENDING);
        CHECK(ui_ai_graph_fixture_light(s->ai,s->view,"ai-model-confirm")!=UI_STATUS_OK);
        CHECK(ui_ai_assistant_control(s->ai,run,UI_AI_ASSISTANT_RETURN)!=UI_STATUS_OK);
    }else{
        if(storage)ui_ai_graph_fixture(s->ai,2,3);
        if(network_takeover)CHECK(ui_ai_model_fixture(s->ai,3,"1")==UI_STATUS_OK);
        CHECK(ui_ai_graph_fixture_light(s->ai,s->view,"ai-model-confirm")==UI_STATUS_OK);
        CHECK(ui_ai_graph_fixture_light(s->ai,s->view,"ai-model-confirm")!=UI_STATUS_OK);
        if(network_takeover){
            ai_run_t *pending=host_operation_pending(s,run,2);CHECK(pending!=NULL);
            if(pending){
                CHECK(ui_ai_assistant_control(s->ai,run,UI_AI_ASSISTANT_TAKEOVER)==UI_STATUS_PENDING);
                ui_ai_edit_fixture(s->ai,3,31000);CHECK(host_operation_check_draining(s,pending));
            }
            CHECK(ui_ai_model_fixture(s->ai,3,"0")==UI_STATUS_OK);
            CHECK(model_wait(s,"\"outcome\":\"cancelled\""));CHECK(host_general_wait(s,"\"control\":2"));
            CHECK(model_wait(s,"\"remote_delivery\":\"remote_processing_unknown\""));
            CHECK(host_general_wait(s,"\"fee_unknown\":true"));
            CHECK(s->ai->model_config->requests==2&&host_operation_read(app.host,0,"editable",0,1));
            programs=2;goto finish;
        }
        edits=c.illegal||storage?0:c.eda?3-c.initial:1;
    }
    final_revision+=edits;programs=1+edits+c.illegal;
    if(storage){
        ui_ai_service_state_t state={0};state.size=sizeof(state);state.version=1;
        DWORD start=GetTickCount();
        do{pump(s);if(ui_ai_service_get_state(s->ai,&state))break;}while(state.phase!=UI_AI_UNAVAILABLE&&GetTickCount()-start<5000);
        CHECK(state.phase==UI_AI_UNAVAILABLE&&s->ai->model_config->requests==1);
        CHECK(host_operation_read(app.host,0,"editable",0,1));
        goto finish;
    }
    CHECK(model_wait(s,takeover?"\"outcome\":\"cancelled\"":stale||c.illegal?"\"outcome\":\"failed\"":"\"outcome\":\"completed\""));
    if(stale)CHECK(host_general_wait(s,"operation_target_changed"));
    CHECK(host_daily_idle(s));
    CHECK(s->ai->model_config->requests==programs&&InterlockedCompareExchange(&c.rounds,0,0)==(LONG)programs);
    CHECK(host_operation_read(app.host,c.eda,stale?"人工改动":edits?c.expected:"editable",c.initial+edits,final_revision));
    char verified[44];snprintf(verified,sizeof(verified),"\"verified\":%u",edits);
    CHECK(host_general_wait(s,verified));
 finish:
    if(failures&&s->ai_run[0])fprintf(stderr,"Operation stopped after failure run=%s\n",s->ai_run);
    CHECK(close_host(s));
    if(!failures&&!wcscmp(mode,L"operation-fixed"))CHECK(host_operation_restart(run));
    fprintf(stderr,"Operation run=%s requests=%u verified=%u checks=%u failures=%u\n",run,programs,edits,checks,failures);
    return failures?1:0;
}
