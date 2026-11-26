__declspec(dllimport) ui_status_t ui_ai_edit_fixture_manual(ui_host_t *,const char *,void *);

static yyjson_doc *host_daily_json(const char *raw)
{return raw?yyjson_read(raw,strlen(raw),0):NULL;}

static int host_daily_visible(host_window_t *s,const char *id,const char *phrase)
{
    ui_element_presentation_t p={0};p.size=sizeof(p);
    return !ui_web_view_get_presentation(s->view,id,&p)&&p.visible&&p.text_utf8&&strstr(p.text_utf8,phrase)!=NULL;
}

static int host_daily_hidden(host_window_t *s,const char *id)
{
    ui_element_presentation_t p={0};p.size=sizeof(p);
    return !ui_web_view_get_presentation(s->view,id,&p)&&!p.visible&&!p.rect.width&&!p.rect.height;
}

static int host_daily_idle(host_window_t *s)
{
    DWORD start=GetTickCount();
    do{
        ui_ai_service_state_t state={0};state.size=sizeof(state);state.version=1;
        pump(s);
        if(!ui_ai_service_get_state(s->ai,&state)&&!state.queued&&!state.active_run_id[0])return 1;
    }while(GetTickCount()-start<5000);
    return 0;
}

static void host_daily_responses(host_window_t *s,const char *revision,const char *alias)
{
    yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *items=yyjson_mut_arr(d);yyjson_mut_doc_set_root(d,items);
    if(revision){
        char *wire=general_read_wire_alias("minesweeper.board.read",alias);CHECK(wire&&yyjson_mut_arr_add_strcpy(d,items,wire));free(wire);
        wire=general_wire("{\"nodes\":[{\"id\":\"board\",\"operation_id\":\"read-board\",\"target\":0,\"capability\":\"minesweeper.board.read\",\"agent_profile\":\"\",\"instruction\":\"读取固定棋局摘要\",\"context\":\"只读\",\"params\":{},\"constraint\":{\"type\":\"object\",\"properties\":{},\"additionalProperties\":false}}]}",0);
        CHECK(wire&&yyjson_mut_arr_add_strcpy(d,items,wire));free(wire);
        char final[512];snprintf(final,sizeof(final),"{\"conclusion\":\"已读取棋局摘要\",\"facts\":[{\"node\":\"board\",\"quote\":\"%s\"}],\"todos\":[],\"inferences\":[]}",revision);
        wire=general_wire(final,0);CHECK(wire&&yyjson_mut_arr_add_strcpy(d,items,wire));free(wire);
    }else{
        char *wire=general_wire("只读可以查看数据。",0);CHECK(wire&&yyjson_mut_arr_add_strcpy(d,items,wire));free(wire);
    }
    char *json=yyjson_mut_write(d,0,NULL);
    CHECK(json&&ui_ai_model_fixture(s->ai,1,json)==UI_STATUS_OK);
    CHECK(ui_ai_model_fixture(s->ai,5,"daily-isolated-sentinel")==UI_STATUS_OK);
    free(json);yyjson_mut_doc_free(d);
}

static int host_daily_propose(host_window_t *s,const char *text_utf8,int expect_preview)
{
    text(s,"ai-input",text_utf8);click(s,"ai-model-propose");
    if(!expect_preview)return 1;
    int saved=model_wait(s,"\"phase\":3");CHECK(saved);if(!saved)return 0;
    DWORD start=GetTickCount();do{pump(s);}while(s->ai_submission.phase!=AI_SUBMIT_SAVED&&GetTickCount()-start<5000);
    CHECK(s->ai_submission.phase==AI_SUBMIT_SAVED);pump(s);
    CHECK(host_daily_visible(s,"ai-view-scope","查看本次发送内容"));
    CHECK(host_daily_visible(s,"ai-save-notice","已在本地保存"));
    CHECK(host_daily_hidden(s,"ai-chat")&&host_daily_hidden(s,"ai-empty-chat"));
    return 1;
}

static void host_shared_chat_controls(host_window_t *s)
{
    CHECK(host_daily_visible(s,"ai-main-title","玄枢 · 应用助手"));
    CHECK(host_daily_visible(s,"ai-input",""));
    CHECK(host_daily_visible(s,"ai-model-propose","发送消息"));
    CHECK(!host_daily_visible(s,"ai-draft-range-toggle",""));
    CHECK(!host_daily_visible(s,"ai-model-diagnostics-toggle","")&&!host_daily_visible(s,"ai-model-state",""));
    CHECK(!host_daily_visible(s,"ai-model-copy","")&&!host_daily_visible(s,"ai-model-name",""));
}

static void host_daily_chat(host_window_t *s,int priced)
{
    CHECK(s->ai_price_ready==priced);
    host_shared_chat_controls(s);
    CHECK(!host_daily_visible(s,"ai-composer-model","deepseek-flash"));
    CHECK(priced?!host_daily_visible(s,"ai-composer-hint",""):host_daily_visible(s,"ai-composer-hint","计价缺失或已过期"));
    CHECK(!host_daily_visible(s,"ai-model-copy",""));
    text(s,"ai-input","这是未提交的草稿。");
    click(s,"ai-model-toggle");
    CHECK(host_daily_visible(s,"ai-model-toggle","返回聊天"));
    CHECK(host_daily_visible(s,"ai-model-name","deepseek-flash"));
    CHECK(!host_daily_visible(s,"ai-input","")&&!host_daily_visible(s,"ai-model-propose",""));
    CHECK(!host_daily_visible(s,"ai-model-copy","")&&!host_daily_visible(s,"ai-model-diagnostics-toggle",""));
    click(s,"ai-model-toggle");
    CHECK(host_daily_visible(s,"ai-input","这是未提交的草稿。"));
    CHECK(!host_daily_visible(s,"ai-model-name","")&&!host_daily_visible(s,"ai-model-copy",""));
    text(s,"ai-input","   ");
    click(s,"ai-model-propose");
    CHECK(host_daily_visible(s,"ai-save-notice","请输入消息"));
    schedule_refresh(s);pump(s);
    CHECK(host_daily_visible(s,"ai-save-notice","请输入消息")&&!s->ai_run[0]);
    text(s,"ai-input","这是未提交的草稿。");
    int maintenance=host_daily_visible(s,"ai-maintenance-toggle","维护");
    CHECK(maintenance);
    if(maintenance){
        click(s,"ai-maintenance-toggle");
        CHECK(host_daily_visible(s,"ai-maintenance-toggle","返回聊天"));
        CHECK(host_daily_hidden(s,"ai-input")&&host_daily_hidden(s,"ai-chat")&&host_daily_hidden(s,"ai-empty-chat"));
        CHECK(host_daily_visible(s,"ai-model-diagnostics-toggle","查看诊断")&&!host_daily_visible(s,"ai-model-copy",""));
        click(s,"ai-model-diagnostics-toggle");
        CHECK(host_daily_visible(s,"ai-model-copy","复制脱敏诊断"));
        click(s,"ai-maintenance-toggle");
        CHECK(host_daily_visible(s,"ai-input","这是未提交的草稿。")&&!host_daily_visible(s,"ai-model-copy",""));
    }
    CHECK(s->ai->model_config->requests==0&&!s->ai_run[0]);
    host_daily_responses(s,NULL,NULL);
    if(!host_daily_propose(s,"请用一句话解释只读。",1))return;
    int preview=host_general_binding(s,0);CHECK(preview);if(!preview)return;
    CHECK(!s->ai->model_config->requests);
    if(!priced){
        ui_element_presentation_t p={0};p.size=sizeof(p);
        CHECK(ui_web_view_get_presentation(s->view,"ai-model-confirm",&p)==UI_STATUS_OK&&!p.enabled);
        CHECK(host_daily_visible(s,"ai-composer-hint","消息只能保存到本地"));
        CHECK(ui_ai_cancel(s->ai,s->ai_run)==UI_STATUS_OK);CHECK(model_wait(s,"\"outcome\":\"cancelled\""));
        CHECK(s->ai->model_config->requests==0);
        return;
    }
    click(s,"ai-model-toggle");
    CHECK(!s->ai_preview_ready&&!host_daily_visible(s,"ai-model-confirm",""));
    CHECK(ui_ai_model_fixture_light(s->ai,s->view,"ai-model-confirm")!=UI_STATUS_OK);
    CHECK(s->ai->model_config->requests==0);
    click(s,"ai-model-toggle");
    CHECK(host_daily_visible(s,"ai-view-scope","查看本次发送内容"));
    click(s,"ai-view-scope");
    CHECK(host_general_binding(s,0));
    CHECK(ui_ai_model_fixture_light(s->ai,s->view,"ai-model-confirm")==UI_STATUS_OK);
    CHECK(model_wait(s,"\"outcome\":\"completed\""));
    CHECK(host_general_wait(s,"\"result\":\"只读可以查看数据。\""));
    CHECK(s->ai->model_config->requests==1);
    pump(s);
    CHECK(!host_daily_visible(s,"ai-view-scope","")&&!host_daily_visible(s,"ai-range-close",""));
    CHECK(host_daily_visible(s,"ai-message-1-body","请用一句话解释只读。"));
    click(s,"ai-task-facts-toggle");
    int review=host_daily_visible(s,"ai-view-saved-scope","查看发送内容");
    CHECK(review);
    if(review){
        click(s,"ai-view-saved-scope");
        CHECK(host_daily_visible(s,"ai-scope-title","本次允许发送的内容"));
        CHECK(ui_ai_model_fixture_light(s->ai,s->view,"ai-model-confirm")!=UI_STATUS_OK);
        CHECK(s->ai->model_config->requests==1);
    }
    yyjson_doc *d=host_daily_json(s->ai->model_config->fixture_body);
    yyjson_val *o=yyjson_doc_get_root(d);
    CHECK(yyjson_is_arr(yyjson_obj_get(o,"tools"))&&!yyjson_arr_size(yyjson_obj_get(o,"tools")));
    CHECK(yyjson_arr_size(yyjson_obj_get(o,"input"))==1);
    yyjson_doc_free(d);
}

static int host_daily_mine_scope(host_window_t *s,uint64_t instance,const char *revision)
{
    yyjson_doc *d=host_daily_json(s->ai_preview);yyjson_val *root=yyjson_doc_get_root(d);
    yyjson_doc *targets=host_daily_json(yyjson_get_str(yyjson_obj_get(root,"fixed_targets")));
    yyjson_doc *plan=host_daily_json(yyjson_get_str(yyjson_obj_get(root,"scope")));
    yyjson_val *t=yyjson_arr_get(yyjson_doc_get_root(targets),0),*p=yyjson_doc_get_root(plan);
    yyjson_doc *scope=host_daily_json(yyjson_get_str(yyjson_obj_get(p,"assistant_scope")));
    yyjson_val *permission=yyjson_arr_get(yyjson_doc_get_root(scope),0),*fields=yyjson_obj_get(permission,"fields"),*caps=yyjson_obj_get(permission,"capabilities");
    char id[32];snprintf(id,sizeof(id),"%llu",(unsigned long long)instance);
    int ok=yyjson_arr_size(yyjson_doc_get_root(targets))==1&&yyjson_equals_str(yyjson_obj_get(t,"instance_id"),id)&&
        yyjson_equals_str(yyjson_obj_get(t,"resource_id"),"minesweeper.board")&&yyjson_equals_str(yyjson_obj_get(t,"revision"),revision)&&
        yyjson_equals_str(yyjson_obj_get(t,"scope"),"{\"access\":\"read\"}")&&
        yyjson_equals_str(yyjson_obj_get(p,"history"),"[]")&&yyjson_arr_size(caps)==1&&
        yyjson_equals_str(yyjson_obj_get(yyjson_arr_get(caps,0),"id"),"minesweeper.board.read")&&
        yyjson_arr_size(fields)==2&&yyjson_equals_str(yyjson_arr_get(fields,0),"state")&&yyjson_equals_str(yyjson_arr_get(fields,1),"revision");
    yyjson_doc_free(scope);yyjson_doc_free(plan);yyjson_doc_free(targets);yyjson_doc_free(d);return ok;
}

static void host_daily_board_evidence(host_window_t *s,int rows,int mines,const char *revision)
{
    yyjson_doc *body=host_daily_json(s->ai->model_config->fixture_body);
    yyjson_val *item=yyjson_arr_get(yyjson_obj_get(yyjson_doc_get_root(body),"input"),0);
    yyjson_doc *context=host_daily_json(yyjson_get_str(yyjson_obj_get(item,"content")));
    yyjson_val *node=yyjson_arr_get(yyjson_obj_get(yyjson_doc_get_root(context),"nodes"),0);
    yyjson_doc *result=host_daily_json(yyjson_get_str(yyjson_obj_get(node,"permitted_evidence")));
    yyjson_val *root=yyjson_doc_get_root(result),*data=yyjson_obj_get(root,"data"),*board=yyjson_obj_get(data,"state");
    CHECK(yyjson_equals_str(yyjson_obj_get(root,"framework_revision"),revision));
    CHECK(yyjson_obj_size(data)==2&&yyjson_equals_str(yyjson_obj_get(data,"revision"),revision));
    CHECK(yyjson_obj_size(board)==6&&yyjson_get_int(yyjson_obj_get(board,"rows"))==rows&&yyjson_get_int(yyjson_obj_get(board,"cols"))==rows);
    CHECK(yyjson_get_int(yyjson_obj_get(board,"mines"))==mines&&yyjson_is_int(yyjson_obj_get(board,"flags"))&&yyjson_get_int(yyjson_obj_get(board,"flags"))==0);
    CHECK(yyjson_is_int(yyjson_obj_get(board,"revealed"))&&yyjson_get_int(yyjson_obj_get(board,"revealed"))==0&&yyjson_equals_str(yyjson_obj_get(board,"status"),"ready"));
    fprintf(stderr,"Daily board model evidence rows=%d cols=%d mines=%d flags=0 revealed=0 revision=%s, state/revision only\n",rows,rows,mines,revision);
    yyjson_doc_free(result);yyjson_doc_free(context);yyjson_doc_free(body);
}

static void host_daily_tool_projection(host_window_t *s,int rows,const char *revision)
{
    yyjson_doc *body=host_daily_json(s->ai->model_config->fixture_body);
    yyjson_val *items=yyjson_obj_get(yyjson_doc_get_root(body),"input"),*item;
    size_t i,n;unsigned outputs=0;
    yyjson_arr_foreach(items,i,n,item){
        if(!yyjson_equals_str(yyjson_obj_get(item,"type"),"function_call_output"))continue;
        ++outputs;
        yyjson_doc *result=host_daily_json(yyjson_get_str(yyjson_obj_get(item,"output")));
        yyjson_val *o=yyjson_doc_get_root(result);
        CHECK(yyjson_obj_size(o)==2&&yyjson_equals_str(yyjson_obj_get(o,"revision"),revision));
        CHECK(yyjson_get_int(yyjson_obj_get(yyjson_obj_get(o,"state"),"rows"))==rows);
        yyjson_doc_free(result);
    }
    CHECK(outputs==1);
    yyjson_doc_free(body);
}

static void host_daily_mines(host_window_t *s,const wchar_t *package,int stale,int bad_alias)
{
    CHECK(s->ai_price_ready);
    open_path(s,package);pump(s);uint64_t first=ui_workspace_active(s->workspace);
    open_path(s,package);pump(s);uint64_t second=ui_workspace_active(s->workspace);
    CHECK(first&&second&&first!=second);
    CHECK(ui_workspace_set_instance_title(s->workspace,first,"巡检棋局甲")==UI_STATUS_OK);
    CHECK(ui_workspace_set_instance_title(s->workspace,second,"巡检棋局乙")==UI_STATUS_OK);
    ui_app_instance_info_t app={0};CHECK(get_instance(s,second,&app));
    if(!app.host)return;
    CHECK(ui_ai_edit_fixture_manual(app.host,"minesweeper.intermediate",NULL)==UI_STATUS_OK);pump(s);
    uint64_t instances[]={first,second};const char *revisions[]={"board-1","board-2"};
    for(unsigned i=0;i<(stale||bad_alias?1u:2u);++i){
        uint64_t instance=instances[i];
        CHECK(host_daily_idle(s));
        CHECK(ui_workspace_activate(s->workspace,instances[1-i])==UI_STATUS_OK);pump(s);
        host_daily_responses(s,revisions[i],bad_alias?"1":i?"target-0":"0");
        if(!stale||stale==3){
            if(!i){
                host_daily_propose(s,"读取棋局摘要。",0);
                CHECK(host_daily_visible(s,"ai-context-question","你指的是哪一个？"));
                CHECK(host_daily_visible(s,"ai-context-choice-0","巡检棋局甲"));
                CHECK(host_daily_visible(s,"ai-context-choice-1","巡检棋局乙"));
                CHECK(s->ai_submission.phase==AI_SUBMIT_CHOOSE&&!s->ai_run[0]&&s->ai->model_config->requests==0);
                if(stale==3){
                    CHECK(ui_workspace_activate(s->workspace,first)==UI_STATUS_OK);pump(s);
                    CHECK(get_instance(s,first,&app));
                    CHECK(ui_ai_edit_fixture_manual(app.host,"minesweeper.intermediate",NULL)==UI_STATUS_OK);pump(s);
                    ui_cap_target_t fresh={0};fresh.size=sizeof(fresh);fresh.version=1;
                    CHECK(ui_capability_bind(app.host,"minesweeper.board",&fresh)==UI_STATUS_OK);
                    CHECK(!strcmp(fresh.expected_revision,"board-2"));
                }
                click(s,"ai-context-choice-0");
                if(stale==3){
                    CHECK(!s->ai_run[0]&&s->ai_submission.phase==AI_SUBMIT_REJECTED);
                    CHECK(host_daily_visible(s,"ai-save-notice","所指对象已变化")&&s->ai->model_config->requests==0);
                    return;
                }
                CHECK(model_wait(s,"\"phase\":3"));
            }else if(!host_daily_propose(s,"读取巡检棋局乙的棋局摘要。",1))return;
        }else if(!host_daily_propose(s,"读取巡检棋局甲的棋局摘要。",1))return;
        int preview=host_general_binding(s,0);CHECK(preview);if(!preview)return;
        CHECK(host_daily_mine_scope(s,instance,revisions[i]));
        CHECK(s->ai->model_config->requests==i*3);
        CHECK(ui_workspace_activate(s->workspace,instance)==UI_STATUS_OK);pump(s);
        CHECK(ui_workspace_activate(s->workspace,instances[1-i])==UI_STATUS_OK);pump(s);
        CHECK(host_daily_mine_scope(s,instance,revisions[i]));
        if(stale){
            if(stale==1){
                CHECK(ui_workspace_activate(s->workspace,first)==UI_STATUS_OK);CHECK(get_instance(s,first,&app));
                CHECK(ui_ai_edit_fixture_manual(app.host,"minesweeper.intermediate",NULL)==UI_STATUS_OK);pump(s);
            }else{
                CHECK(ui_workspace_close(s->workspace,first,UI_APP_CLOSE_TAB)==UI_STATUS_OK);pump(s);
                open_path(s,package);pump(s);CHECK(ui_workspace_active(s->workspace)!=first);
            }
            CHECK(ui_ai_model_fixture_light(s->ai,s->view,"ai-model-confirm")!=UI_STATUS_OK);
            CHECK(ui_ai_cancel(s->ai,s->ai_run)==UI_STATUS_OK);CHECK(model_wait(s,"\"outcome\":\"cancelled\""));
            CHECK(s->ai->model_config->requests==0);return;
        }
        click(s,"ai-detail-toggle");CHECK(ui_ai_model_fixture_light(s->ai,s->view,"ai-model-confirm")!=UI_STATUS_OK);
        CHECK(s->ai->model_config->requests==i*3);
        click(s,"ai-detail-toggle");CHECK(host_general_binding(s,0));
        CHECK(ui_ai_model_fixture_light(s->ai,s->view,"ai-model-confirm")==UI_STATUS_OK);
        if(bad_alias){
            CHECK(model_wait(s,"\"outcome\":\"failed\""));
            CHECK(model_wait(s,"\"error\":\"batch_wrapper_invalid\""));
            CHECK(model_wait(s,"\"tool_calls\":0")&&s->ai->model_config->requests==1);
            pump(s);
            CHECK(host_daily_visible(s,"ai-status-line","模型已回复，任务处理未完成"));
            CHECK(!host_daily_visible(s,"ai-composer-hint",""));
            return;
        }
        int waiting=host_general_wait(s,"\"plan_waiting\":true");CHECK(waiting);if(!waiting)return;
        host_daily_tool_projection(s,i?16:9,revisions[i]);
        CHECK(host_general_binding(s,1));
        CHECK(ui_ai_graph_fixture_light(s->ai,s->view,"ai-model-confirm")==UI_STATUS_OK);
        CHECK(model_wait(s,"\"outcome\":\"completed\""));
        CHECK(s->ai->model_config->requests==(i+1)*3);
        host_daily_board_evidence(s,i?16:9,i?40:10,revisions[i]);
    }
}

static const char daily_body_text[]="设备巡检交接单\n设备总数：12台。\n已完成：9台；待处理：3台。\n负责人：林青 & 周明。\n下一步\n复核三台设备。";

static void host_daily_body_value(const char *raw,const char *revision)
{
    yyjson_doc *d=host_daily_json(raw);
    yyjson_val *o=yyjson_doc_get_root(d),*body=yyjson_obj_get(o,"body");
    CHECK(yyjson_obj_size(o)==2&&yyjson_equals_str(yyjson_obj_get(o,"revision"),revision));
    CHECK(yyjson_equals_str(yyjson_obj_get(body,"text"),daily_body_text));
    CHECK(yyjson_get_uint(yyjson_obj_get(body,"bytes"))==strlen(daily_body_text));
    CHECK(yyjson_is_true(yyjson_obj_get(body,"complete"))&&yyjson_equals_str(yyjson_obj_get(body,"semantics"),"dom-body-v1"));
    yyjson_doc_free(d);
}

static void host_daily_body(host_window_t *s,const wchar_t *package,char revision[128])
{
    open_path(s,package);pump(s);
    ui_app_instance_info_t app={0};
    CHECK(get_instance(s,ui_workspace_active(s->workspace),&app));
    CHECK(ui_workspace_set_instance_title(s->workspace,app.instance_id,"合成交接单正文")==UI_STATUS_OK);
    ui_cap_target_t target={0};target.size=sizeof(target);target.version=1;
    CHECK(ui_capability_bind(app.host,"browser.view",&target)==UI_STATUS_OK);
    strcpy_s(revision,128,target.expected_revision);
    CHECK(revision[0]);
    yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *responses=yyjson_mut_arr(d);
    yyjson_mut_doc_set_root(d,responses);
    char *wire=general_read_wire("browser.body");CHECK(wire&&yyjson_mut_arr_add_strcpy(d,responses,wire));free(wire);
    wire=general_wire("{\"nodes\":[{\"id\":\"body\",\"operation_id\":\"read-body\",\"target\":0,\"capability\":\"browser.body\",\"agent_profile\":\"\",\"instruction\":\"读取已选页面正文\",\"context\":\"只读\",\"params\":{},\"constraint\":{\"type\":\"object\",\"properties\":{},\"additionalProperties\":false}}]}",0);
    CHECK(wire&&yyjson_mut_arr_add_strcpy(d,responses,wire));free(wire);
    wire=general_wire("{\"conclusion\":\"设备12台，完成9台，待处理3台。\",\"facts\":[{\"node\":\"body\",\"quote\":\"林青 & 周明\"}],\"todos\":[],\"inferences\":[]}",0);
    CHECK(wire&&yyjson_mut_arr_add_strcpy(d,responses,wire));free(wire);
    wire=yyjson_mut_write(d,0,NULL);CHECK(wire&&ui_ai_model_fixture(s->ai,1,wire)==UI_STATUS_OK);free(wire);yyjson_mut_doc_free(d);
    CHECK(ui_ai_model_fixture(s->ai,5,"daily-isolated-sentinel")==UI_STATUS_OK);
    host_shared_chat_controls(s);
    if(!host_daily_propose(s,"概括当前页面的网页正文。",1))return;
    CHECK(host_general_binding(s,0)&&s->ai->model_config->requests==0);
    CHECK(s->ai_submission.choice==1&&s->ai_submission.choices[0].target.resource.instance_id==app.instance_id);
    CHECK(strstr(s->ai_submission.choices[0].title,"合成交接单正文")!=NULL);
    yyjson_doc *preview=host_daily_json(s->ai_preview);
    yyjson_doc *plan=host_daily_json(yyjson_get_str(yyjson_obj_get(yyjson_doc_get_root(preview),"scope")));
    yyjson_doc *projection=host_daily_json(yyjson_get_str(yyjson_obj_get(yyjson_doc_get_root(plan),"projection")));
    yyjson_val *p=yyjson_doc_get_root(projection),*caps=yyjson_obj_get(p,"capabilities"),*fields=yyjson_obj_get(p,"fields");
    CHECK(yyjson_arr_size(caps)==1&&yyjson_equals_str(yyjson_arr_get(caps,0),"browser.body"));
    CHECK(yyjson_arr_size(fields)==2&&yyjson_equals_str(yyjson_arr_get(fields,0),"body")&&yyjson_equals_str(yyjson_arr_get(fields,1),"revision"));
    CHECK(yyjson_equals_str(yyjson_obj_get(yyjson_doc_get_root(plan),"history"),"[]"));
    yyjson_doc_free(projection);yyjson_doc_free(plan);yyjson_doc_free(preview);
    CHECK(ui_ai_model_fixture_light(s->ai,s->view,"ai-model-confirm")==UI_STATUS_OK);
    int waiting=host_general_wait(s,"\"plan_waiting\":true");CHECK(waiting);if(!waiting)return;
    yyjson_doc *actual=host_daily_json(s->ai->model_config->fixture_body);
    yyjson_val *items=yyjson_obj_get(yyjson_doc_get_root(actual),"input"),*item;size_t i,n;unsigned seen=0;
    yyjson_arr_foreach(items,i,n,item)if(yyjson_equals_str(yyjson_obj_get(item,"type"),"function_call_output")){
        host_daily_body_value(yyjson_get_str(yyjson_obj_get(item,"output")),revision);++seen;
    }
    CHECK(seen==1);yyjson_doc_free(actual);
    CHECK(host_general_binding(s,1));
    CHECK(ui_ai_graph_fixture_light(s->ai,s->view,"ai-model-confirm")==UI_STATUS_OK);
    CHECK(model_wait(s,"\"outcome\":\"completed\"")&&s->ai->model_config->requests==3);
    CHECK(host_general_wait(s,"设备12台，完成9台，待处理3台。"));
}

static void host_daily_receipt(host_window_t *s,const wchar_t *package,int storage)
{
    const char *original=storage?"隔离保存故障。":"概括当前文档的正文。";
    host_shared_chat_controls(s);
    if(storage){
        host_daily_responses(s,NULL,NULL);
        EnterCriticalSection(&s->ai->lock);s->ai->fixture_fail_write=1;LeaveCriticalSection(&s->ai->lock);
        host_daily_propose(s,original,0);
        DWORD start=GetTickCount();do{pump(s);}while(s->ai_submission.phase!=AI_SUBMIT_CHECK&&GetTickCount()-start<5000);
        CHECK(s->ai_submission.phase==AI_SUBMIT_CHECK);
        CHECK(host_daily_visible(s,"ai-save-notice","保存未能确认")&&host_daily_visible(s,"ai-input",original));
        CHECK(host_daily_visible(s,"ai-save-check","核对保存")&&s->ai->model_config->requests==0);
        click(s,"ai-save-check");
        CHECK(s->ai_submission.phase==AI_SUBMIT_CHECK);
        PostMessageW(s->hwnd,WM_CLOSE,0,0);start=GetTickCount();
        do{pump(s);}while(!s->ai_exit_error_ack&&GetTickCount()-start<5000);
        CHECK(s->ai_exit_error_ack&&!s->exiting&&s->reported_error[0]&&s->ai->store_failed);
        return;
    }
    host_daily_propose(s,original,0);
    CHECK(s->ai_submission.phase==AI_SUBMIT_REJECTED&&!s->ai_run[0]);
    CHECK(host_daily_visible(s,"ai-save-notice","没有可读取的对应内容"));
    char request[65],first_attempt[65];
    strcpy_s(request,65,s->ai_submission.id);strcpy_s(first_attempt,65,s->ai_submission.attempt);
    click(s,"ai-model-toggle");click(s,"ai-model-toggle");schedule_refresh(s);pump(s);
    CHECK(host_daily_visible(s,"ai-save-notice","没有可读取的对应内容")&&host_daily_visible(s,"ai-input",original));
    char *page=load_page();CHECK(page&&ui_web_view_load_html(s->view,page)==UI_STATUS_OK);free(page);
    schedule_refresh(s);pump(s);
    CHECK(host_daily_visible(s,"ai-save-notice","没有可读取的对应内容")&&host_daily_visible(s,"ai-input",original));
    open_path(s,package);pump(s);
    host_daily_responses(s,NULL,NULL);
    EnterCriticalSection(&s->ai->lock); /* Hold acceptance until the draft and unrelated receipts have been projected. */
    click(s,"ai-model-propose");
    CHECK(!strcmp(s->ai_submission.id,request)&&strcmp(s->ai_submission.attempt,first_attempt));
    CHECK(s->ai_submission.phase==AI_SUBMIT_SAVING&&host_daily_visible(s,"ai-save-notice","正在保存消息"));
    ui_ai_event_t event={0};event.type="storage_error";event.client_request_id="old-session-action";event.payload="{\"error\":\"prior write\"}";
    ai_panel_event(&event,s);pump(s);
    event.type="accepted";event.committed=1;event.run_id="other-run";
    ai_panel_event(&event,s);pump(s);
    CHECK(s->ai_submission.phase==AI_SUBMIT_SAVING&&host_daily_visible(s,"ai-save-notice","正在保存消息"));
    text(s,"ai-input","下一条尚未提交的消息。");
    ui_input_event_t repeat={0};repeat.size=sizeof(repeat);repeat.kind=UI_INPUT_KEY_DOWN;
    repeat.key_code=VK_RETURN;repeat.modifiers=UI_INPUT_MODIFIER_CONTROL;
    CHECK(!ui_web_view_dispatch_input(s->view,&repeat)&&!ui_web_view_dispatch_input(s->view,&repeat));pump(s);
    CHECK(!strcmp(s->ai_submission.id,request)&&s->ai_submission.phase==AI_SUBMIT_SAVING);
    LeaveCriticalSection(&s->ai->lock);
    CHECK(model_wait(s,"\"phase\":3"));pump(s);
    CHECK(s->ai_submission.phase==AI_SUBMIT_SAVED&&host_daily_visible(s,"ai-input","下一条尚未提交的消息。"));
    CHECK(s->ai->model_config->requests==0);
    s->ai_submission.phase=AI_SUBMIT_CHECK; /* Isolate a lost receipt after the real commit. */
    schedule_refresh(s);pump(s);click(s,"ai-save-check");
    DWORD start=GetTickCount();do{pump(s);}while(s->ai_submission.phase!=AI_SUBMIT_SAVED&&GetTickCount()-start<5000);
    CHECK(s->ai_submission.phase==AI_SUBMIT_SAVED&&!strcmp(s->ai_submission.id,request));
    CHECK(host_daily_visible(s,"ai-input","下一条尚未提交的消息。")&&s->ai->model_config->requests==0);
    CHECK(ui_ai_cancel(s->ai,s->ai_run)==UI_STATUS_OK&&model_wait(s,"\"outcome\":\"cancelled\""));
}

static void host_daily_attachments(host_window_t *s)
{
    const char *original="第一条历史样本。",*file_text="明确选择的合成附件。";
    host_daily_responses(s,NULL,NULL);
    if(!host_daily_propose(s,original,1))return;
    CHECK(host_general_binding(s,0));
    CHECK(ui_ai_model_fixture_light(s->ai,s->view,"ai-model-confirm")==UI_STATUS_OK);
    CHECK(model_wait(s,"\"outcome\":\"completed\"")&&host_daily_idle(s));
    DWORD start=GetTickCount();do{pump(s);}while(!host_daily_visible(s,"ai-message-1-body",original)&&GetTickCount()-start<5000);
    CHECK(host_daily_visible(s,"ai-message-1-body",original));
    click(s,"ai-history-toggle");click(s,"ai-history-include");click(s,"ai-history-select-0");
    assistant_attachment_t *file=&s->ai_attachments[0];
    strcpy_s(file->name,256,"synthetic-note.txt");strcpy_s(file->media,32,"text/plain");
    file->bytes=(unsigned char *)_strdup(file_text);file->length=strlen(file_text);
    ui_ai_assistant_attachment_t attachment={0};attachment.size=sizeof(attachment);attachment.version=1;
    attachment.name_utf8=file->name;attachment.media_type=file->media;attachment.bytes=file->bytes;attachment.byte_count=file->length;
    CHECK(file->bytes&&ui_ai_assistant_attachment_hash(&attachment,file->hash)==UI_STATUS_OK);
    s->ai_attachment_count=1;schedule_refresh(s);pump(s);
    if(!host_daily_propose(s,"概括附件内容。",1))return;
    CHECK(host_general_binding(s,0)&&s->ai->model_config->requests==1);
    yyjson_doc *preview=host_daily_json(s->ai_preview);
    yyjson_val *request=yyjson_obj_get(yyjson_doc_get_root(preview),"request"),*input=yyjson_obj_get(request,"input");
    CHECK(yyjson_arr_size(yyjson_obj_get(request,"tools"))==0&&yyjson_arr_size(input)==3);
    CHECK(yyjson_equals_str(yyjson_obj_get(yyjson_arr_get(input,0),"content"),original));
    yyjson_doc *scope=host_daily_json(yyjson_get_str(yyjson_obj_get(yyjson_arr_get(input,1),"content")));
    yyjson_val *files=yyjson_obj_get(yyjson_doc_get_root(scope),"attachment_snapshots");
    CHECK(yyjson_arr_size(files)==1&&yyjson_equals_str(yyjson_obj_get(yyjson_arr_get(files,0),"text"),file_text));
    yyjson_doc_free(scope);
    CHECK(ui_ai_model_fixture_light(s->ai,s->view,"ai-model-confirm")==UI_STATUS_OK);
    CHECK(model_wait(s,"\"outcome\":\"completed\"")&&s->ai->model_config->requests==2);
    yyjson_doc *actual=host_daily_json(s->ai->model_config->fixture_body);
    CHECK(yyjson_equals(request,yyjson_doc_get_root(actual)));
    yyjson_doc_free(actual);yyjson_doc_free(preview);
}

static int host_daily_run(host_window_t *s,const wchar_t *package,const wchar_t *db,const wchar_t *mode)
{
    int mines=!wcsncmp(mode,L"daily-mines",11),priced=wcscmp(mode,L"daily-chat-unpriced")!=0;
    int stale=!wcscmp(mode,L"daily-mines-stale")?1:!wcscmp(mode,L"daily-mines-reopened")?2:!wcscmp(mode,L"daily-mines-stale-draft")?3:0;
    int bad_alias=!wcscmp(mode,L"daily-mines-outside-alias"),two_boards=mines&&!stale&&!bad_alias;
    int body=!wcscmp(mode,L"daily-body"),receipt=!wcscmp(mode,L"daily-receipt"),storage=!wcscmp(mode,L"daily-storage");
    int attachments=!wcscmp(mode,L"daily-attachments"),tasks=two_boards||attachments?2:stale==3||storage?0:1;
    char revision[128]={0};
    if(body)host_daily_body(s,package,revision);
    else if(attachments)host_daily_attachments(s);
    else if(receipt||storage)host_daily_receipt(s,package,storage);
    else if(mines)host_daily_mines(s,package,stale,bad_alias);
    else{if(!wcscmp(mode,L"daily-chat-with-app")){open_path(s,package);pump(s);}host_daily_chat(s,priced);}
    CHECK(close_host(s));
    char path[4096];sqlite3 *sql=NULL;sqlite3_stmt *q=NULL;
    CHECK(WideCharToMultiByte(CP_UTF8,0,db,-1,path,sizeof(path),NULL,NULL)>0);
    int status=sqlite3_open_v2(path,&sql,SQLITE_OPEN_READONLY,NULL);CHECK(status==SQLITE_OK);
    if(status==SQLITE_OK){
        CHECK(sqlite3_prepare_v2(sql,"SELECT count(*),sum(outcome='completed') FROM runs WHERE terminal=1",-1,&q,NULL)==SQLITE_OK);
        CHECK(sqlite3_step(q)==SQLITE_ROW&&sqlite3_column_int(q,0)==tasks&&
            sqlite3_column_int(q,1)==(tasks==2?2:!mines&&priced&&!receipt&&!storage));
        sqlite3_finalize(q);q=NULL;
        CHECK(sqlite3_prepare_v2(sql,"SELECT count(*) FROM events WHERE kind='assistant_transport_entry'",-1,&q,NULL)==SQLITE_OK);
        CHECK(sqlite3_step(q)==SQLITE_ROW&&sqlite3_column_int(q,0)==(two_boards?6:body?3:attachments?2:bad_alias||(!mines&&priced&&!receipt&&!storage)));
        sqlite3_finalize(q);q=NULL;
        CHECK(sqlite3_prepare_v2(sql,"SELECT count(*) FROM runs",-1,&q,NULL)==SQLITE_OK);
        CHECK(sqlite3_step(q)==SQLITE_ROW&&sqlite3_column_int(q,0)==tasks);
        sqlite3_finalize(q);q=NULL;
        if(body){
            CHECK(sqlite3_prepare_v2(sql,"SELECT result,status FROM tools WHERE capability='browser.body'",-1,&q,NULL)==SQLITE_OK);
            CHECK(sqlite3_step(q)==SQLITE_ROW&&sqlite3_column_int(q,1)==UI_STATUS_OK);
            host_daily_body_value((const char *)sqlite3_column_text(q,0),revision);
            CHECK(sqlite3_step(q)==SQLITE_DONE);sqlite3_finalize(q);
        }
    }
    sqlite3_close(sql);
    printf("Daily release host checks=%u failures=%u; isolated credentials and transport\n",checks,failures);
    return failures?1:0;
}
