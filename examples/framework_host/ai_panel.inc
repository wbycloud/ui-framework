/* Global workspace projection; no Provider or business executor lives here. */
static void ai_panel_event(const ui_ai_event_t *event,void *data)
{
    host_window_t *s=data;
    if(s->ai_general)ai_assistant_submission_event(s,event);
    if(!strcmp(event->type,"ready"))s->ai_refresh_history=1;
    if(!strcmp(event->type,"received")){if(strcmp(s->ai_run,event->run_id)){s->ai_query_pending=0;s->ai_query_id[0]=0;s->ai_stored_id[0]=0;replace_text(&s->ai_query_error,"");}strcpy_s(s->ai_run,sizeof(s->ai_run),event->run_id);}
    if(!strcmp(event->type,"accepted")||!strcmp(event->type,"existing")){
        strcpy_s(s->ai_ack,sizeof(s->ai_ack),event->client_request_id);if(!strcmp(event->type,"existing")&&strstr(event->payload,"\"graph_record\"")){replace_text(&s->ai_stored_run,event->payload);strcpy_s(s->ai_stored_id,65,event->run_id);}s->ai_refresh_history=1;
    }
    if(!strcmp(event->type,"finished"))s->ai_refresh_history=1;
    if(!strcmp(event->type,"history")){
        if(!strcmp(event->query_id,"panel-runs"))replace_text(&s->ai_runs,event->payload);
        else replace_text(&s->ai_messages,event->payload);
    }
    if(!strcmp(event->type,"query")&&!strcmp(event->query_id,s->ai_query_id)){
        yyjson_doc *d=yyjson_read(event->payload,strlen(event->payload),0);const char *id=d?yyjson_get_str(yyjson_obj_get(yyjson_doc_get_root(d),"run_id")):NULL;
        s->ai_query_pending=0;if(id&&!strcmp(id,s->ai_run)){replace_text(&s->ai_stored_run,event->payload);strcpy_s(s->ai_stored_id,65,id);replace_text(&s->ai_query_error,"");}else replace_text(&s->ai_query_error,"任务快照身份不匹配");yyjson_doc_free(d);
    }
    if(!strcmp(event->type,"query_error")&&!strcmp(event->query_id,s->ai_query_id)){s->ai_query_pending=0;replace_text(&s->ai_query_error,event->payload);replace_text(&s->ai_error,event->payload);}
    if(!strcmp(event->type,"query_error")&&(!strcmp(event->query_id,"panel-messages")||!strcmp(event->query_id,"panel-runs")))replace_text(&s->ai_error,event->payload);
    if(!strcmp(event->type,"storage_error")||!strcmp(event->type,"rejected"))replace_text(&s->ai_error,event->payload);
    schedule_refresh(s);
}
/* Derivation must be exact; never fall back to the default on overflow. */
static ui_status_t ai_panel_open_model(host_window_t *s,const char *database,int isolated)
{
 char path[4096];if(isolated){int n=snprintf(path,sizeof(path),"%s.model.json",database);if(n<0||(size_t)n>=sizeof(path))return UI_STATUS_LIMIT_EXCEEDED;}
 ui_status_t status;wchar_t profile_path[4096],account[65];SetLastError(ERROR_SUCCESS);
 DWORD profile_length=GetEnvironmentVariableW(L"UI_AI_PROFILE_PATH",profile_path,4096),profile_error=GetLastError();
 if(s->ai_general&&(profile_length||profile_error!=ERROR_ENVVAR_NOT_FOUND)){
  DWORD account_length=GetEnvironmentVariableW(L"UI_AI_PROFILE_ID",account,65);
  char *profile=profile_length&&profile_length<4096?to_utf8(profile_path):NULL;
  char *identity=account_length&&account_length<65?to_utf8(account):NULL;
  status=profile&&identity?ui_ai_model_open_profile(s->ai,profile,identity):UI_STATUS_INVALID_ARGUMENT;
  if(!status)replace_text(&s->ai_config_location,profile);free(profile);free(identity);
 }else if(s->ai_general&&!isolated){status=ui_ai_model_open_profile(s->ai,NULL,"daily-default");if(!status)replace_text(&s->ai_config_location,"日常Profile：LOCALAPPDATA/ui-framework/xuanshu/profiles/daily-default.json");}
 else status=ui_ai_model_open(s->ai,isolated?path:NULL);
 if(!status&&!s->ai_config_location)replace_text(&s->ai_config_location,isolated?path:"用户默认配置：LOCALAPPDATA/ui-framework/xuanshu/model-config.json");
 wchar_t policy_path[4096];SetLastError(ERROR_SUCCESS);DWORD pn=GetEnvironmentVariableW(L"UI_AI_BR2_POLICY",policy_path,4096);DWORD pe=GetLastError();
 if(!status&&(pn||pe!=ERROR_ENVVAR_NOT_FOUND)){
  char raw[8193];const char *policy=NULL;FILE *f=NULL;
  if(isolated&&pn&&pn<4096)f=_wfopen(policy_path,L"rb");
  if(f){size_t bytes=fread(raw,1,8192,f);if(feof(f)){raw[bytes]=0;policy=raw;}fclose(f);}
  status=ui_ai_business_validation_enable(s->ai,policy);
  if(status){replace_text(&s->ai_error,"BR-2保护初始化失败，Service已停止；未回退普通发送");(void)ui_ai_service_request_close(s->ai);}
 }
 return status;
}
static void ai_panel_initialize(host_window_t *s)
{
    wchar_t path[4096];char *utf8;ui_ai_service_config_t config={0};DWORD n=GetEnvironmentVariableW(L"UI_AI_DB_PATH",path,4096);int isolated=n!=0;
#ifdef UI_HOST_TEST
    /* Existing host fixtures must never open a user's database. P3 fixtures set
       an isolated explicit path and exercise the same production Service. */
    if(!n)return;
#endif
    if(n>=4096){replace_text(&s->ai_error,"UI_AI_DB_PATH is too long");return;}
    if(!n){wchar_t base[3500];n=GetEnvironmentVariableW(L"LOCALAPPDATA",base,3500);
        if(!n||n>=3500||_snwprintf_s(path,4096,_TRUNCATE,L"%s\\ui-framework\\xuanshu\\conversation.sqlite",base)<0){replace_text(&s->ai_error,"Cannot determine user storage path");return;}}
    utf8=to_utf8(path);if(!utf8){replace_text(&s->ai_error,"Invalid storage path");return;}
    config.size=sizeof(config);config.version=1;config.workspace=s->workspace;config.storage_path_utf8=utf8;config.on_event=ai_panel_event;config.user_data=s;
    if(ui_ai_service_create(&config,&s->ai)!=UI_STATUS_OK)replace_text(&s->ai_error,"Cannot create workspace AI Service");
    else{replace_text(&s->ai_database_path,utf8);ui_status_t status=ai_assistant_enabled(s)?ai_assistant_enable(s):UI_STATUS_OK;
        if(status)replace_text(&s->ai_error,"助手保护配置无效，未启动Service");
        else if(ui_ai_service_open(s->ai)!=UI_STATUS_OK)replace_text(&s->ai_error,"Cannot start asynchronous storage open");}
    if(s->ai){ui_status_t status=ai_panel_open_model(s,utf8,isolated);if(status){char message[128];snprintf(message,sizeof(message),"模型配置路径/打开失败 (%d)；未回退默认配置",status);replace_text(&s->ai_error,message);}}
    free(utf8);
}
static void ai_panel_json(host_window_t *s,json_buffer_t *json)
{
    ui_ai_service_state_t state={0};char *run=NULL;state.size=sizeof(state);state.version=1;
    json_append(json,",\"ai\":{");json_format(json,"\"enabled\":%s",s->ai?"true":"false");
    if(s->ai&&ui_ai_service_get_state(s->ai,&state)==UI_STATUS_OK){
        json_format(json,",\"phase\":%d,\"queued\":%u,\"worker\":%u",state.phase,state.queued,state.worker_alive);json_append(json,",\"conversation\":");json_string(json,state.conversation_id);
        json_append(json,",\"active\":");json_string(json,state.active_run_id);json_append(json,",\"storageError\":");json_string(json,state.error);
        if(s->ai_run[0])(void)ui_ai_get_run(s->ai,s->ai_run,&run);
    }
    /* Stop a failed build before disclosure can reenter application code. */
    if(json->failed){ui_ai_release(run);ai_projection_invalidate(s);return;}
    ai_model_panel_projection(s,json);
    json_append(json,",\"modelConfig\":");json_append(json,s->ai_model?s->ai_model:"null");
    json_append(json,",\"selected\":");json_string(json,s->ai_run);json_append(json,",\"ack\":");json_string(json,s->ai_ack);
    json_append(json,",\"run\":");const char *source=NULL,*snapshot=ai_model_task_snapshot(s,run,&source);yyjson_doc *rd=snapshot?yyjson_read(snapshot,strlen(snapshot),0):NULL;yyjson_mut_doc *md=rd?yyjson_doc_mut_copy(rd,NULL):NULL;char *projected=NULL;if(md){yyjson_mut_obj_remove_key(yyjson_mut_doc_get_root(md),"context_preview");projected=yyjson_mut_write(md,0,NULL);}if(snapshot&&!projected)json->failed=1;json_append(json,projected?projected:"null");free(projected);yyjson_mut_doc_free(md);yyjson_doc_free(rd);ui_ai_release(run);
    ai_assistant_json(s,json);
    json_append(json,",\"taskSource\":");json_string(json,source);json_append(json,",\"taskQueryId\":");json_string(json,s->ai_query_id);json_append(json,",\"queryError\":");json_string(json,s->ai_query_error);
    json_append(json,",\"messages\":");json_append(json,s->ai_messages?s->ai_messages:"{\"items\":[]}");json_append(json,",\"runs\":");json_append(json,s->ai_runs?s->ai_runs:"{\"items\":[]}");
    json_append(json,",\"graph\":");json_append(json,s->ai_graph?s->ai_graph:"null");json_append(json,",\"coordinator\":");json_append(json,s->ai_coordinator?s->ai_coordinator:"null");
    json_append(json,",\"edit\":");json_append(json,s->ai_edit?s->ai_edit:"null");json_append(json,",\"editCatalog\":");json_append(json,s->ai_edit_catalog?s->ai_edit_catalog:"null");json_append(json,",\"editAck\":");json_string(json,s->ai_edit_ack);json_append(json,",\"error\":");json_string(json,s->ai_error);json_append(json,"}");
}
/* Projection reads only fixed fields of Service-produced JSON. Approval still
 * checks the complete Service-owned binding; no page data signs authority. */
static char *ai_panel_edit_string(const char *json,const char *key)
{char find[80];const char *p;if(!json)return NULL;snprintf(find,sizeof(find),"\"%s\":",key);p=strstr(json,find);if(!p)return NULL;p+=strlen(find);return read_string(&p);}
static int ai_panel_edit_number(const char *json,const char *key)
{char find[80];const char *p;if(!json)return -1;snprintf(find,sizeof(find),"\"%s\":",key);p=strstr(json,find);return p?(int)strtol(p+strlen(find),NULL,10):-1;}
static void ai_panel_edit_bindings(host_window_t *s)
{
 int stable=1,projected=s->ai_projection_generation==s->ai_projection_rendered;const char *nodes[]={"ai-edit-confirm","ai-edit-approve","ai-edit-undo","ai-edit-check","ai-edit-migrate"};int phase=ai_panel_edit_number(s->ai_edit,"phase"),schema=ai_panel_edit_number(s->ai_edit,"schema"),last=ai_panel_edit_number(s->ai_edit,"last_host_status");char *hash=ai_panel_edit_string(s->ai_edit,"binding_hash");
 if(!hash||!s->view){free(hash);return;}if(projected&&!strcmp(hash,s->ai_edit_bound)&&phase==s->ai_edit_bound_phase&&schema==s->ai_edit_bound_schema&&last==UI_STATUS_OK){free(hash);return;}
 for(unsigned i=0;i<5;++i)if(s->ai_edit_bindings[i]){ui_status_t status=ui_ai_edit_unbind_button(s->ai,s->ai_edit_bindings[i]);if(status==UI_STATUS_OK||status==UI_STATUS_NOT_FOUND)s->ai_edit_bindings[i]=0;else stable=0;}
 if(!stable||!projected){if(stable&&!projected)s->ai_edit_bound[0]=0;free(hash);return;}
 for(unsigned i=0;i<5;++i){int enabled=i==0?phase==UI_AI_EDIT_SCOPE_WAIT:i==1?phase==UI_AI_EDIT_APPROVAL_WAIT:i==2?phase==UI_AI_EDIT_DONE:i==3?phase==UI_AI_EDIT_UNKNOWN:schema==1;
  if(enabled&&!s->ai_edit_bindings[i]){ui_ai_edit_light_binding_t d={0};d.size=sizeof(d);d.version=1;d.view=s->view;d.control_id=nodes[i];d.binding_hash=hash;d.action=(ui_ai_edit_action_t)(i+1);{ui_status_t status=ui_ai_edit_bind_light(s->ai,&d,&s->ai_edit_bindings[i]);if(status){char error[100];snprintf(error,sizeof(error),"AI authority binding unavailable (%d); no action accepted",status);replace_text(&s->ai_error,error);stable=0;}}}}
 if(stable)strcpy_s(s->ai_edit_bound,65,hash);else s->ai_edit_bound[0]=0;s->ai_edit_bound_phase=phase;s->ai_edit_bound_schema=schema;free(hash);
}
#include "ai_graph_panel.inc"
#include "ai_model_panel.inc"
#include "ai_assistant_panel.inc"
static int ai_panel_action(host_window_t *s,host_action_t *action)
{
    ui_status_t status=UI_STATUS_INVALID_ARGUMENT;ui_ai_task_request_t request={0};ui_cap_target_t target={0};ui_app_instance_info_t app;char id[65];
    if(strncmp(action->action,"ai-",3))return 0;
    if(!s->ai){replace_text(&s->ai_error,"Service not configured in this host fixture");schedule_refresh(s);return 1;}
    if(ai_assistant_action(s,action))return 1;
    if(ai_model_panel_action(s,action))return 1;
    if(ai_panel_graph_action(s,action))return 1;
    if(!strcmp(action->action,"ai-edit-propose")){
        ui_ai_edit_request_t edit={0};edit.size=sizeof(edit);edit.version=1;edit.client_request_id=action->token;edit.capability_id=action->command;edit.params_json=action->params;edit.parameter_constraint_schema=action->answer;edit.target.size=sizeof(edit.target);edit.target.version=2;
        status=ui_ai_edit_bind_target(s->ai,read_id(action->id),action->panel,&edit.target);if(!status)status=ui_ai_edit_propose(s->ai,&edit,id);
    }else if(!strcmp(action->action,"ai-edit-catalog")){
        char *catalog=NULL;uint64_t generation=0;if(!get_instance(s,read_id(action->id),&app)||app.closing)goto failed;
        status=ui_capability_discover_v2(app.host,NULL,0,32,&generation,&catalog);if(!status)replace_text(&s->ai_edit_catalog,catalog);ui_capability_release(catalog);
    }else if(!strcmp(action->action,"ai-edit-reject"))status=ui_ai_edit_cancel(s->ai,action->id);
    else if(!strncmp(action->action,"ai-edit-",8))status=UI_STATUS_PERMISSION_DENIED;
    else if(!strcmp(action->action,"ai-submit")||!strcmp(action->action,"ai-continue")){
        request.size=sizeof(request);request.version=1;request.client_request_id=action->token;request.input_utf8=action->answer;request.stub_plan_json=action->params;
        if(action->panel&&action->panel[0]){
            if(!get_instance(s,read_id(action->id),&app)||app.closing)goto failed;
            target.size=sizeof(target);target.version=1;status=ui_capability_bind(app.host,action->panel,&target);if(status)goto failed;request.targets=&target;request.target_count=1;
        }
        free(s->ai_stored_run);s->ai_stored_run=NULL;
        status=!strcmp(action->action,"ai-continue")?ui_ai_continue(s->ai,action->command,&request,id):ui_ai_submit(s->ai,&request,id);
        if(status==UI_STATUS_OK)strcpy_s(s->ai_run,sizeof(s->ai_run),id);
    }else if(!strcmp(action->action,"ai-cancel"))status=ui_ai_cancel(s->ai,action->id);
    else if(!strcmp(action->action,"ai-answer"))status=ui_ai_answer(s->ai,action->id,action->command,action->token,action->answer);
    else if(!strcmp(action->action,"ai-history"))status=ui_ai_history(s->ai,action->command&&!strcmp(action->command,"runs")?"panel-runs":"panel-messages",action->command&&!strcmp(action->command,"runs")?UI_AI_RUNS:UI_AI_MESSAGES,read_id(action->id),16);
    else if(!strcmp(action->action,"ai-select")){
        if(!action->id||strlen(action->id)!=64||s->ai_query_generation==UINT64_MAX)goto failed;strcpy_s(s->ai_run,sizeof(s->ai_run),action->id);free(s->ai_stored_run);s->ai_stored_run=NULL;s->ai_stored_id[0]=0;s->ai_query_pending=1;replace_text(&s->ai_query_error,"");snprintf(s->ai_query_id,sizeof(s->ai_query_id),"host-run-%llu",(unsigned long long)++s->ai_query_generation);status=ui_ai_query_run(s->ai,s->ai_query_id,action->id);if(status){s->ai_query_pending=0;replace_text(&s->ai_query_error,"异步任务读取未接纳");}
    }
failed:if(status!=UI_STATUS_OK){char message[128];snprintf(message,sizeof(message),"AI request rejected (%d); input retained, no automatic retry",status);replace_text(&s->ai_error,message);}else replace_text(&s->ai_error,"");schedule_refresh(s);return 1;
}
static void ai_panel_tick(host_window_t *s)
{
    ui_ai_service_state_t state={0};if(!s->ai)return;(void)ui_ai_service_poll(s->ai,64);ai_model_panel_tick(s);ai_assistant_tick(s);state.size=sizeof(state);state.version=1;
    if(ui_ai_service_get_state(s->ai,&state)!=UI_STATUS_OK)return;
    {char *edit=NULL;ui_status_t status=ui_ai_edit_get_state(s->ai,&edit);if(status==UI_STATUS_NOT_FOUND&&state.phase==UI_AI_READY)(void)ui_ai_edit_enable(s->ai);
     if(!status&&edit){if(!s->ai_edit||strcmp(edit,s->ai_edit)){replace_text(&s->ai_edit,edit);++s->ai_projection_generation;schedule_refresh(s);if(strstr(edit,"\"proposal_confirmed\":true")){char *client=ai_panel_edit_string(edit,"client_request_id");if(client){strcpy_s(s->ai_edit_ack,65,client);free(client);}if(ai_panel_edit_number(edit,"phase")==UI_AI_EDIT_DONE||ai_panel_edit_number(edit,"phase")==UI_AI_EDIT_UNKNOWN)s->ai_refresh_history=1;}}
      if(state.phase==UI_AI_READY)ai_panel_edit_bindings(s);}ui_ai_release(edit);}
    ai_panel_graph_tick(s,state.phase);
    if(s->ai_refresh_history&&state.phase==UI_AI_READY){
        if(ui_ai_history(s->ai,"panel-messages",UI_AI_MESSAGES,0,16)==UI_STATUS_OK&&ui_ai_history(s->ai,"panel-runs",UI_AI_RUNS,0,16)==UI_STATUS_OK)s->ai_refresh_history=0;
    }
}
static void ai_panel_free(host_window_t *s)
{ai_assistant_free(s);free(s->ai_state_sent);s->ai_state_sent=NULL;free(s->ai_query_error);s->ai_query_error=NULL;free(s->ai_draft);s->ai_draft=NULL;free(s->ai_choices);free(s->ai_preview);free(s->ai_chunks);free(s->ai_config_location);s->ai_choices=s->ai_preview=s->ai_chunks=s->ai_config_location=NULL;free(s->ai_model);s->ai_model=NULL;free(s->ai_graph);free(s->ai_coordinator);s->ai_graph=s->ai_coordinator=NULL;free(s->ai_messages);free(s->ai_runs);free(s->ai_stored_run);free(s->ai_error);free(s->ai_edit);free(s->ai_edit_catalog);s->ai_edit=s->ai_edit_catalog=NULL;s->ai_messages=s->ai_runs=s->ai_stored_run=s->ai_error=NULL;}
