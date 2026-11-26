static int ai_submission_token(const char *id)
{
    return id&&id[0]&&strlen(id)<65&&strspn(id,"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_-")==strlen(id);
}

static void ai_assistant_submission_event(host_window_t *s,const ui_ai_event_t *event)
{
    assistant_submission_t *r=&s->ai_submission;
    if(r->phase!=AI_SUBMIT_SAVING&&r->phase!=AI_SUBMIT_CHECK)return;
    if(r->querying&&event->query_id&&!strcmp(event->query_id,r->id)){
        r->querying=0;
        if(!strcmp(event->type,"query")&&event->committed){
            yyjson_doc *d=yyjson_read(event->payload,strlen(event->payload),0);
            yyjson_val *o=yyjson_doc_get_root(d);
            const char *client=yyjson_get_str(yyjson_obj_get(o,"client_request_id"));
            const char *run=yyjson_get_str(yyjson_obj_get(o,"run_id"));
            if(client&&run&&!strcmp(client,r->id)&&!strcmp(run,r->run))r->phase=AI_SUBMIT_SAVED;
            yyjson_doc_free(d);
        }
        if(r->phase==AI_SUBMIT_CHECK)r->message="仍无法核对这条消息是否已保存，已保留草稿。";
        return;
    }
    if(!event->client_request_id||strcmp(event->client_request_id,r->id))return;
    if(!strcmp(event->type,"received")&&event->run_id&&event->run_id[0]){
        strcpy_s(r->run,sizeof(r->run),event->run_id);
        return;
    }
    if(!r->run[0]||!event->run_id||strcmp(event->run_id,r->run))return;
    if((!strcmp(event->type,"accepted")||!strcmp(event->type,"existing"))&&event->committed)
        r->phase=AI_SUBMIT_SAVED;
    else if(!strcmp(event->type,"rejected")){
        r->phase=AI_SUBMIT_REJECTED;
        r->message="这一条消息未被接纳。可修改消息后再试，草稿已保留。";
    }else if(!strcmp(event->type,"storage_error")){
        r->phase=AI_SUBMIT_CHECK;
        r->message="保存未能确认。草稿已保留，请先核对这条消息。";
    }
}

static void ai_assistant_submission_check(host_window_t *s,const host_action_t *a)
{
    assistant_submission_t *r=&s->ai_submission;
    if(!a->token||strcmp(a->token,r->id)||r->querying||
       (r->phase!=AI_SUBMIT_SAVING&&r->phase!=AI_SUBMIT_CHECK))return;
    if(!r->run[0]||ui_ai_query_run(s->ai,r->id,r->run)){
        r->message="暂时无法核对保存结果，草稿已保留。";
        return;
    }
    r->querying=1;
}

static void ai_assistant_submit(host_window_t *s,host_action_t *a)
{
    assistant_submission_t *r=&s->ai_submission;
    if(!ai_submission_token(a->token)||!a->answer||!a->params)return;
    char *version=ai_panel_edit_string(a->params,"draftVersion");
    char *attempt=ai_panel_edit_string(a->params,"attempt");
    if(!ai_submission_token(version)||!ai_submission_token(attempt)){free(version);free(attempt);return;}
    int same=!strcmp(a->token,r->id);
    if(r->phase==AI_SUBMIT_SAVING||r->phase==AI_SUBMIT_CHECK||
       (same&&(r->phase==AI_SUBMIT_SAVED||strcmp(version,r->version)||!strcmp(attempt,r->attempt)))){
        free(version);free(attempt);return;
    }
    if(!same){
        memset(r,0,sizeof(*r));
        strcpy_s(r->id,sizeof(r->id),a->token);
        strcpy_s(r->version,sizeof(r->version),version);
        char *mode=ai_panel_edit_string(a->params,"mode");
        r->operation=mode&&!strcmp(mode,"operation");free(mode);
    }
    strcpy_s(r->attempt,sizeof(r->attempt),attempt);free(version);free(attempt);
    r->phase=AI_SUBMIT_SAVING;r->message=NULL;
    if(!a->answer[strspn(a->answer," \r\n\t")]||strlen(a->answer)>UI_AI_MAX_INPUT){
        r->phase=AI_SUBMIT_REJECTED;r->message="请输入消息，或缩短超过上限的内容。";return;
    }
    if(r->operation){
        char *history=ai_panel_edit_string(a->params,"history");
        int incompatible=s->ai_attachment_count||(history&&strcmp(history,"[]"));free(history);
        if(incompatible){r->phase=AI_SUBMIT_REJECTED;r->message="代操作只使用本次消息和所指应用。请先取消历史引用及附件。";return;}
        if(!same||!r->count)ai_operation_resolve(s,a->answer,r);
    }else if(!same||!r->count)ai_context_resolve(s,a->answer,r);
    if(r->count>1&&!r->choice){
        uint64_t choice=same?read_id(a->panel):0;
        if(!choice||choice>r->count){r->phase=AI_SUBMIT_CHOOSE;return;}
        r->choice=(unsigned)choice;
    }else if(r->count==1)r->choice=1;
    if(r->message){r->phase=AI_SUBMIT_REJECTED;return;}
    ui_status_t status=ai_assistant_propose(s,a,r->choice?&r->choices[r->choice-1]:NULL,0);
    if(status){
        r->phase=r->run[0]?AI_SUBMIT_CHECK:AI_SUBMIT_REJECTED;
        r->message=r->run[0]?"保存未能确认。草稿已保留，请先核对这条消息。":
            status==UI_STATUS_VALIDATION_FAILED&&r->choice?"所指对象已变化。请修改消息并重新选定对象。":
            status==UI_STATUS_LIMIT_EXCEEDED?"服务暂忙或消息内容超过限制，草稿已保留。":
            status==UI_STATUS_CANCELLED?"保存服务尚未就绪，草稿已保留。":"消息未保存，草稿已保留。";
    }
}
