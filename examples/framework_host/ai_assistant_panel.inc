/* Ordinary page actions propose data and request control transitions. Trusted
 * plan confirmation remains the framework Light binding with full readback. */
static int ai_assistant_enabled(host_window_t *s)
{
    int enabled=1;
#ifdef UI_HOST_TEST
    wchar_t value[8];DWORD n=GetEnvironmentVariableW(L"UI_AI_GENERAL_ASSISTANT",value,8);
    enabled=n==1&&value[0]==L'1';
#endif
    SetLastError(ERROR_SUCCESS);
    wchar_t policy[4096];DWORD n2=GetEnvironmentVariableW(L"UI_AI_BR2_POLICY",policy,4096);
    if(n2||GetLastError()!=ERROR_ENVVAR_NOT_FOUND)enabled=0;
    s->ai_general=enabled;return enabled;
}

static ui_status_t ai_assistant_enable(host_window_t *s)
{
    ui_ai_assistant_policy_t p={0};p.size=sizeof(p);p.version=1;
    p.request_limit=8;p.deadline_ms=600000;p.budget_nano_usd=100000000;
    p.price_version="";p.price_evidence_sha256="";
    p.meter_profile="deepseek-v41-responses-1017-v1";p.meter_evidence_sha256=BR2_METER_EVIDENCE;
    /* Daily defaults preserve a local, unpriced proposal. A separately reviewed
       concrete runtime policy is required before any real model send. */
    wchar_t path[4096];SetLastError(ERROR_SUCCESS);
    DWORD n=GetEnvironmentVariableW(L"UI_AI_ASSISTANT_POLICY",path,4096),error=GetLastError();
    char raw[8193];yyjson_doc *d=NULL;
    if(n||error!=ERROR_ENVVAR_NOT_FOUND){
        if(!n||n>=4096)return UI_STATUS_INVALID_ARGUMENT;
        FILE *f=_wfopen(path,L"rb");if(!f)return UI_STATUS_PLATFORM_ERROR;
        size_t bytes=fread(raw,1,8192,f);int complete=feof(f);fclose(f);
        raw[bytes]=0;if(!complete)return UI_STATUS_LIMIT_EXCEEDED;
        d=yyjson_read(raw,bytes,0);yyjson_val *o=d?yyjson_doc_get_root(d):NULL;
        const char *database=o?yyjson_get_str(yyjson_obj_get(o,"database")):NULL;
        if(!database||!s->ai_database_path||strcmp(database,s->ai_database_path)){yyjson_doc_free(d);return UI_STATUS_PERMISSION_DENIED;}
        if(!yyjson_is_uint(yyjson_obj_get(o,"request_limit"))||yyjson_get_uint(yyjson_obj_get(o,"request_limit"))>16||
           !yyjson_is_uint(yyjson_obj_get(o,"deadline_ms"))||yyjson_get_uint(yyjson_obj_get(o,"deadline_ms"))>1800000){yyjson_doc_free(d);return UI_STATUS_INVALID_ARGUMENT;}
        p.request_limit=(uint32_t)yyjson_get_uint(yyjson_obj_get(o,"request_limit"));
        p.deadline_ms=(uint32_t)yyjson_get_uint(yyjson_obj_get(o,"deadline_ms"));
        p.budget_nano_usd=yyjson_get_uint(yyjson_obj_get(o,"budget_nano_usd"));
        p.input_rate_nano_usd=yyjson_get_uint(yyjson_obj_get(o,"input_rate_nano_usd"));
        p.output_rate_nano_usd=yyjson_get_uint(yyjson_obj_get(o,"output_rate_nano_usd"));
        p.price_version=yyjson_get_str(yyjson_obj_get(o,"price_version"));
        p.price_evidence_sha256=yyjson_get_str(yyjson_obj_get(o,"price_evidence_sha256"));
    }
    ui_status_t status=ui_ai_assistant_enable(s->ai,&p);yyjson_doc_free(d);return status;
}

static void ai_assistant_json(host_window_t *s,json_buffer_t *json)
{
    if(strcmp(s->ai_undo_run,s->ai_run)){strcpy_s(s->ai_undo_run,65,s->ai_run);s->ai_undo_node[0]=0;}
    char *state=NULL,*active=NULL;ui_ai_service_state_t service={0};service.size=sizeof(service);service.version=1;
    if(s->ai_run[0])(void)ui_ai_assistant_get(s->ai,s->ai_run,&state);
    if(!state&&s->ai_stored_run&&!strcmp(s->ai_stored_id,s->ai_run)){
        yyjson_doc *stored=yyjson_read(s->ai_stored_run,strlen(s->ai_stored_run),0);
        yyjson_val *record=stored?yyjson_doc_get_root(stored):NULL;
        const char *raw=record?yyjson_get_str(yyjson_obj_get(record,"stub_plan")):NULL;
        yyjson_doc *plan=raw?yyjson_read(raw,strlen(raw),0):NULL;
        const char *version=plan?yyjson_get_str(yyjson_obj_get(yyjson_doc_get_root(plan),"assistant_version")):NULL;
        yyjson_mut_doc *d=version&&!strcmp(version,"general-v1")?yyjson_mut_doc_new(NULL):NULL;
        yyjson_mut_val *o=d?yyjson_mut_obj(d):NULL;
        if(o){
            yyjson_mut_doc_set_root(d,o);yyjson_mut_obj_add_strcpy(d,o,"root_run",s->ai_run);
            yyjson_mut_obj_add_bool(d,o,"record_only",true);yyjson_mut_obj_add_bool(d,o,"authorization_restored",false);
            yyjson_mut_obj_add_strcpy(d,o,"result",yyjson_get_str(yyjson_obj_get(record,"reply")));
            const char *files=yyjson_get_str(yyjson_obj_get(yyjson_doc_get_root(plan),"attachments"));
            yyjson_mut_obj_add_strcpy(d,o,"attachments",files?files:"[]");
            yyjson_val *graph=yyjson_obj_get(record,"graph_record"),*nodes=yyjson_obj_get(graph,"nodes");
            if(nodes)yyjson_mut_obj_add_val(d,o,"nodes",yyjson_val_mut_copy(d,nodes));
            state=yyjson_mut_write(d,0,NULL);
        }
        yyjson_mut_doc_free(d);yyjson_doc_free(plan);yyjson_doc_free(stored);
    }
    if(!ui_ai_service_get_state(s->ai,&service)){
        const char *control_run=service.active_run_id[0]?service.active_run_id:s->ai_handoff_run;
        if(control_run[0])(void)ui_ai_assistant_get(s->ai,control_run,&active);
    }
    if(active&&strstr(active,"\"control\":2"))s->assistant_manual=1;
    json_format(json,",\"general\":%s,\"manual\":%s",s->ai_general?"true":"false",s->assistant_manual?"true":"false");
    json_append(json,",\"assistantTask\":");json_append(json,state?state:"null");
    json_append(json,",\"activeAssistant\":");json_append(json,active?active:"null");
    json_append(json,",\"undoNode\":");json_string(json,s->ai_undo_node);
    json_append(json,",\"applicationCatalog\":");json_append(json,s->ai_general_catalog?s->ai_general_catalog:"null");
    json_append(json,",\"draftAttachments\":[");
    for(unsigned i=0;i<s->ai_attachment_count;++i){
        assistant_attachment_t *file=&s->ai_attachments[i];if(i)json_append(json,",");
        json_append(json,"{\"name\":");json_string(json,file->name);
        json_append(json,",\"media\":");json_string(json,file->media);
        json_append(json,",\"hash\":");json_string(json,file->hash);
        json_format(json,",\"bytes\":%u,\"image\":\"%llu\",\"localOnly\":%s}",(unsigned)file->length,(unsigned long long)file->image,file->image?"true":"false");
    }
    json_append(json,"]");ui_ai_release(state);ui_ai_release(active);
}

static void ai_assistant_attachments_clear(host_window_t *s)
{
    for(unsigned i=0;i<s->ai_attachment_count;++i){
        if(s->ai_attachments[i].image)(void)ui_image_release(s->chrome_host,s->ai_attachments[i].image);
        free(s->ai_attachments[i].bytes);
    }
    memset(s->ai_attachments,0,sizeof(s->ai_attachments));s->ai_attachment_count=0;
}

static ui_status_t ai_assistant_attach(host_window_t *s)
{
    if(s->ai_attachment_count==4)return UI_STATUS_LIMIT_EXCEEDED;
    wchar_t path[4096]={0};OPENFILENAMEW dialog={0};dialog.lStructSize=sizeof(dialog);dialog.hwndOwner=s->hwnd;
    dialog.lpstrFilter=L"文本与本地PNG\0*.txt;*.md;*.json;*.png\0\0";
    dialog.lpstrFile=path;dialog.nMaxFile=4096;dialog.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
    if(!GetOpenFileNameW(&dialog))return CommDlgExtendedError()?UI_STATUS_PLATFORM_ERROR:UI_STATUS_CANCELLED;
    FILE *f=_wfopen(path,L"rb");if(!f)return UI_STATUS_PLATFORM_ERROR;
    unsigned char *bytes=malloc(256u*1024u+1);if(!bytes){fclose(f);return UI_STATUS_OUT_OF_MEMORY;}
    size_t length=fread(bytes,1,256u*1024u+1,f);int complete=feof(f);fclose(f);
    int image=length>=8&&!memcmp(bytes,"\x89PNG\r\n\x1a\n",8);
    if(!complete||!length||length>(image?256u*1024u:16384u)||
       (!image&&(memchr(bytes,0,length)||!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,(const char *)bytes,(int)length,NULL,0)))){
        free(bytes);return UI_STATUS_LIMIT_EXCEEDED;
    }
    const wchar_t *name=wcsrchr(path,L'\\');name=name?name+1:path;
    char *utf8=to_utf8(name);if(!utf8||strlen(utf8)>255){free(bytes);free(utf8);return UI_STATUS_INVALID_ARGUMENT;}
    assistant_attachment_t file={0};strcpy_s(file.name,256,utf8);free(utf8);
    strcpy_s(file.media,32,image?"image/png":"text/plain");file.bytes=bytes;file.length=length;
    ui_ai_assistant_attachment_t snapshot={0};snapshot.size=sizeof(snapshot);snapshot.version=1;
    snapshot.name_utf8=file.name;snapshot.media_type=file.media;snapshot.bytes=file.bytes;snapshot.byte_count=file.length;
    ui_status_t identity=ui_ai_assistant_attachment_hash(&snapshot,file.hash);
    if(identity){free(bytes);return identity;}
    if(image){ui_status_t status=ui_image_load_png(s->chrome_host,bytes,length,&file.image);if(status){free(bytes);return status;}}
    s->ai_attachments[s->ai_attachment_count++]=file;return UI_STATUS_OK;
}

static ui_status_t ai_assistant_snapshot_files(host_window_t *s)
{
    if(!s->ai_attachment_count)return UI_STATUS_OK;
    char directory[4096];
    int n=snprintf(directory,sizeof(directory),"%s.attachments",s->ai_database_path);
    if(n<0||(size_t)n>=sizeof(directory))return UI_STATUS_LIMIT_EXCEEDED;
    wchar_t wide[4096];if(!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,directory,-1,wide,4096))return UI_STATUS_INVALID_ARGUMENT;
    if(!CreateDirectoryW(wide,NULL)&&GetLastError()!=ERROR_ALREADY_EXISTS)return UI_STATUS_PLATFORM_ERROR;
    for(unsigned i=0;i<s->ai_attachment_count;++i){
        assistant_attachment_t *file=&s->ai_attachments[i];char path[4096];
        n=snprintf(path,sizeof(path),"%s/%s.blob",directory,file->hash);
        if(n<0||(size_t)n>=sizeof(path))return UI_STATUS_LIMIT_EXCEEDED;
        if(!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,path,-1,wide,4096))return UI_STATUS_INVALID_ARGUMENT;
        HANDLE h=CreateFileW(wide,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);DWORD error=GetLastError();
        if(h==INVALID_HANDLE_VALUE){
            if(error!=ERROR_FILE_EXISTS&&error!=ERROR_ALREADY_EXISTS)return UI_STATUS_PLATFORM_ERROR;
            FILE *f=_wfopen(wide,L"rb");unsigned char *saved=malloc(file->length+1);
            size_t count=f&&saved?fread(saved,1,file->length+1,f):0;
            int same=count==file->length&&saved&&!memcmp(saved,file->bytes,count);if(f)fclose(f);free(saved);
            if(!same)return UI_STATUS_VALIDATION_FAILED;
            continue;
        }
        DWORD written=0;int ok=WriteFile(h,file->bytes,(DWORD)file->length,&written,NULL)&&written==file->length&&FlushFileBuffers(h);
        CloseHandle(h);if(!ok)return UI_STATUS_PLATFORM_ERROR;
    }
    return UI_STATUS_OK;
}

static ui_status_t ai_assistant_propose(host_window_t *s,host_action_t *action)
{
    yyjson_doc *d=action->params?yyjson_read(action->params,strlen(action->params),0):NULL;
    yyjson_val *o=d?yyjson_doc_get_root(d):NULL,*selected=yyjson_obj_get(o,"targets"),*row;
    size_t count=yyjson_arr_size(selected);ui_status_t status=UI_STATUS_VALIDATION_FAILED;
    ui_cap_target_t reads[16];ui_cap_target_v2_t fixed[16];
    char *history=NULL,*projection=NULL,*permissions=NULL;
    yyjson_mut_doc *pd=yyjson_mut_doc_new(NULL);yyjson_mut_val *allowed=pd?yyjson_mut_arr(pd):NULL;
    if(!yyjson_is_arr(selected)||!count||count>16||!allowed)goto done;
    yyjson_mut_doc_set_root(pd,allowed);
    for(size_t i=0;i<count;++i){
        row=yyjson_arr_get(selected,i);const char *id=yyjson_get_str(yyjson_obj_get(row,"instance")),*resource=yyjson_get_str(yyjson_obj_get(row,"resource"));
        yyjson_val *caps=yyjson_obj_get(row,"capabilities"),*profiles=yyjson_obj_get(row,"profiles");
        ui_app_instance_info_t app;if(!id||!resource||!get_instance(s,read_id(id),&app)||app.closing||!yyjson_is_arr(caps)||!yyjson_is_arr(profiles))goto done;
        memset(&reads[i],0,sizeof(reads[i]));reads[i].size=sizeof(reads[i]);reads[i].version=1;
        status=ui_capability_bind(app.host,resource,&reads[i]);if(status)goto done;
        memset(&fixed[i],0,sizeof(fixed[i]));fixed[i].size=sizeof(fixed[i]);fixed[i].version=2;
        status=ui_ai_edit_bind_target(s->ai,app.instance_id,resource,&fixed[i]);if(status)goto done;
        int write=0;size_t j,n;yyjson_val *cap;
        yyjson_arr_foreach(caps,j,n,cap){
            const char *name=yyjson_get_str(cap);char *raw=NULL;
            if(!name)goto done;
            ui_status_t described=ui_capability_describe_v2(app.host,name,&raw);
            yyjson_doc *desc=raw?yyjson_read(raw,strlen(raw),0):NULL;
            const char *effect=desc?yyjson_get_str(yyjson_obj_get(yyjson_doc_get_root(desc),"effect")):NULL;
            if(!described&&effect&&!strcmp(effect,"write"))write=1;
            yyjson_doc_free(desc);ui_capability_release(raw);
        }
        strcpy_s(fixed[i].scope_json,sizeof(fixed[i].scope_json),write?"{\"access\":\"write\"}":"{\"access\":\"read\"}");
        yyjson_mut_val *permission=yyjson_mut_obj(pd);
        yyjson_mut_obj_add_val(pd,permission,"capabilities",yyjson_val_mut_copy(pd,caps));
        yyjson_mut_obj_add_val(pd,permission,"profiles",yyjson_val_mut_copy(pd,profiles));
        yyjson_val *parameters=yyjson_obj_get(row,"parameters");
        if(parameters)yyjson_mut_obj_add_val(pd,permission,"parameters",yyjson_val_mut_copy(pd,parameters));
        yyjson_val *fields=yyjson_obj_get(row,"fields");
        if(fields)yyjson_mut_obj_add_val(pd,permission,"fields",yyjson_val_mut_copy(pd,fields));
        yyjson_mut_arr_add_val(allowed,permission);
    }
    history=ai_panel_edit_string(action->params,"history");projection=ai_panel_edit_string(action->params,"projection");
    permissions=yyjson_mut_write(pd,0,NULL);
    ui_ai_assistant_attachment_t files[4];memset(files,0,sizeof(files));
    for(unsigned i=0;i<s->ai_attachment_count;++i){
        files[i].size=sizeof(files[i]);files[i].version=1;files[i].name_utf8=s->ai_attachments[i].name;
        files[i].media_type=s->ai_attachments[i].media;files[i].bytes=s->ai_attachments[i].bytes;files[i].byte_count=s->ai_attachments[i].length;
    }
    ui_ai_assistant_request_t q={0};q.size=sizeof(q);q.version=1;
    q.task.size=sizeof(q.task);q.task.version=1;q.task.client_request_id=action->token;q.task.input_utf8=action->answer;
    char *configuration=NULL;status=ui_ai_model_get_state(s->ai,&configuration);if(status)goto done;
    yyjson_doc *config=yyjson_read(configuration,strlen(configuration),0);
    q.task.profile_id="deepseek";q.task.profile_revision=config?yyjson_get_uint(yyjson_obj_get(yyjson_doc_get_root(config),"revision")):0;
    yyjson_doc_free(config);ui_ai_release(configuration);
    q.task.targets=reads;q.task.target_count=count;q.task.history_selection_json=history;q.task.disclosure_projection_json=projection;
    q.parent_run_id=action->command;
    q.targets=fixed;q.target_count=count;q.permissions_json=permissions;q.attachments=files;q.attachment_count=s->ai_attachment_count;
    status=ai_assistant_snapshot_files(s);if(status)goto done;
    char run[65];status=ui_ai_assistant_propose(s->ai,&q,run);
    if(!status){strcpy_s(s->ai_run,65,run);free(s->ai_stored_run);s->ai_stored_run=NULL;if(s->assistant_manual){strcpy_s(s->ai_handoff_run,65,run);(void)ui_ai_assistant_control(s->ai,run,UI_AI_ASSISTANT_TAKEOVER);}}
done:
    free(history);free(projection);free(permissions);yyjson_mut_doc_free(pd);yyjson_doc_free(d);return status;
}

static int ai_assistant_action(host_window_t *s,host_action_t *a)
{
    if(!s->ai_general)return 0;
    ui_status_t status;
    if(!strcmp(a->action,"ai-model-catalog")){
        char *catalog=NULL;status=ui_ai_assistant_catalog(s->ai,read_id(a->id),&catalog);
        if(!status){replace_text(&s->ai_general_catalog,catalog);}ui_ai_release(catalog);
    }else if(!strcmp(a->action,"ai-assistant-propose"))status=ai_assistant_propose(s,a);
    else if(!strcmp(a->action,"ai-attachment-add"))status=ai_assistant_attach(s);
    else if(!strcmp(a->action,"ai-attachment-clear")){ai_assistant_attachments_clear(s);status=UI_STATUS_OK;}
    else if(!strcmp(a->action,"ai-undo-view")){
        if(a->id&&!strcmp(a->id,s->ai_run)&&a->panel&&strlen(a->panel)<=64){
            strcpy_s(s->ai_undo_run,65,s->ai_run);strcpy_s(s->ai_undo_node,65,a->panel);
            ai_projection_invalidate(s);++s->ai_projection_generation;status=UI_STATUS_OK;
        }else status=UI_STATUS_INVALID_ARGUMENT;
    }
    else if(!strcmp(a->action,"ai-takeover")||!strcmp(a->action,"ai-return")){
        ui_ai_service_state_t state={0};state.size=sizeof(state);state.version=1;
        status=ui_ai_service_get_state(s->ai,&state);
        int manual=!strcmp(a->action,"ai-takeover");
        if(!status&&state.active_run_id[0]){
            status=ui_ai_assistant_control(s->ai,state.active_run_id,manual?UI_AI_ASSISTANT_TAKEOVER:UI_AI_ASSISTANT_RETURN);
            if(manual&&(status==UI_STATUS_OK||status==UI_STATUS_PENDING))strcpy_s(s->ai_handoff_run,65,state.active_run_id);
            if(!status&&!manual){s->assistant_manual=0;s->ai_handoff_run[0]=0;}
        }else if(!status){s->assistant_manual=manual;s->ai_handoff_run[0]=0;}
    }else if(!strcmp(a->action,"ai-assistant-answer"))
        status=ui_ai_graph_answer(s->ai,a->id,a->panel,a->command,a->token,a->answer);
    else return 0;
    if(status&&status!=UI_STATUS_PENDING&&status!=UI_STATUS_CANCELLED){
        char message[160];snprintf(message,sizeof(message),"助手操作未接纳（%d），草稿及原任务保持。",status);replace_text(&s->ai_error,message);
    }else replace_text(&s->ai_error,"");
    schedule_refresh(s);return 1;
}

static int ai_assistant_undo_snapshot(host_window_t *s,char hash[65],char **preview)
{
    *preview=NULL;hash[0]=0;char *raw=NULL;
    if(!s->ai_undo_node[0]||ui_ai_assistant_get(s->ai,s->ai_run,&raw))return 0;
    yyjson_doc *d=yyjson_read(raw,strlen(raw),0);yyjson_val *nodes=d?yyjson_obj_get(yyjson_doc_get_root(d),"nodes"):NULL,*n;
    size_t i,count;int ok=0;
    yyjson_arr_foreach(nodes,i,count,n){
        const char *id=yyjson_get_str(yyjson_obj_get(n,"node")),*binding=yyjson_get_str(yyjson_obj_get(n,"undo_binding")),*text=yyjson_get_str(yyjson_obj_get(n,"undo_preview"));
        if(id&&!strcmp(id,s->ai_undo_node)&&binding&&strlen(binding)==64&&text&&*text&&yyjson_is_true(yyjson_obj_get(n,"saved"))){
            strcpy_s(hash,65,binding);*preview=_strdup(text);ok=*preview!=NULL;break;
        }
    }
    yyjson_doc_free(d);ui_ai_release(raw);return ok;
}

static ui_status_t ai_assistant_undo_check(void *data,const char *run,const char *hash,uint64_t generation)
{
    host_window_t *s=data;char binding[65],*preview=NULL;
    if(s->exiting||!assistant_visible(s)||!s->ai_display_valid||strcmp(run,s->ai_run)||
       generation!=s->ai_display_generation||s->ai_projection_generation!=s->ai_projection_rendered||!ai_projection_marker(s))return UI_STATUS_PERMISSION_DENIED;
    int ok=ai_assistant_undo_snapshot(s,binding,&preview)&&!strcmp(hash,binding);
    ui_element_presentation_t p={0};p.size=sizeof(p);
    if(ok)ok=!ui_web_view_get_presentation(s->view,"ai-node-undo-preview",&p)&&p.visible&&p.rect.width>0&&p.rect.height>0&&!strcmp(preview,p.text_utf8);
    free(preview);return ok?UI_STATUS_OK:UI_STATUS_PERMISSION_DENIED;
}

static void ai_assistant_undo_bindings(host_window_t *s)
{
    char hash[65],*preview=NULL;
    int valid=ai_assistant_undo_snapshot(s,hash,&preview)&&
        ai_assistant_undo_check(s,s->ai_run,hash,s->ai_display_generation)==UI_STATUS_OK;
    free(preview);
    int same=valid&&!strcmp(hash,s->ai_undo_hash)&&s->ai_undo_generation==s->ai_display_generation;
    if(s->ai_undo_binding&&!same){
        ui_status_t status=ui_ai_graph_unbind(s->ai,s->ai_undo_binding);
        if(!status||status==UI_STATUS_NOT_FOUND){s->ai_undo_binding=0;s->ai_undo_hash[0]=0;}
    }
    if(valid&&!s->ai_undo_binding){
        ui_ai_assistant_undo_binding_t d={0};d.size=sizeof(d);d.version=1;d.view=s->view;
        d.control_id="ai-node-undo";d.run_id=s->ai_run;d.node_id=s->ai_undo_node;d.binding_hash=hash;
        d.generation=s->ai_display_generation;d.check=ai_assistant_undo_check;d.user_data=s;
        if(!ui_ai_assistant_bind_node_undo(s->ai,&d,&s->ai_undo_binding)){
            strcpy_s(s->ai_undo_hash,65,hash);s->ai_undo_generation=s->ai_display_generation;
        }
    }
}

static void ai_assistant_bindings(host_window_t *s)
{
    if(!s->ai||!s->ai_general||!s->view)return;
    char *disclosure=NULL;ui_status_t status=ui_ai_assistant_get_disclosure(s->ai,s->ai_run,&disclosure);
    int plan=!status&&disclosure&&strstr(disclosure,"\"finite_plan\"")!=NULL;
    yyjson_doc *current=plan?yyjson_read(disclosure,strlen(disclosure),0):NULL;
    const char *current_hash=current?yyjson_get_str(yyjson_obj_get(yyjson_doc_get_root(current),"binding_hash")):NULL;
    int valid=plan&&current_hash&&!strcmp(current_hash,s->ai_preview_hash)&&s->ai_preview_ready&&
        ai_model_display_check(s,s->ai_run,current_hash,s->ai_display_generation)==UI_STATUS_OK;
    yyjson_doc_free(current);
    int same=valid&&!strcmp(s->ai_preview_hash,s->ai_assistant_binding_hash)&&s->ai_assistant_generation==s->ai_display_generation;
    if(s->ai_assistant_binding&&!same){
        status=ui_ai_graph_unbind(s->ai,s->ai_assistant_binding);
        if(!status||status==UI_STATUS_NOT_FOUND){s->ai_assistant_binding=0;s->ai_assistant_binding_hash[0]=0;}
    }
    if(valid&&!s->ai_assistant_binding){
        ui_ai_assistant_display_binding_t d={0};d.size=sizeof(d);d.version=1;
        d.binding.size=sizeof(d.binding);d.binding.version=1;d.binding.view=s->view;
        d.binding.control_id="ai-model-confirm";d.binding.run_id=s->ai_run;d.binding.node_id="";
        d.binding.binding_hash=s->ai_preview_hash;d.binding.action=UI_AI_GRAPH_CONFIRM;
        d.generation=s->ai_display_generation;d.check=ai_model_display_check;d.user_data=s;
        if(!ui_ai_assistant_bind_light_display(s->ai,&d,&s->ai_assistant_binding)){
            strcpy_s(s->ai_assistant_binding_hash,65,s->ai_preview_hash);s->ai_assistant_generation=s->ai_display_generation;
        }
    }
    ui_ai_release(disclosure);
    ai_assistant_undo_bindings(s);
}

static void ai_assistant_tick(host_window_t *s)
{
    if(!s->ai_general)return;
    char *selected=NULL,*active=NULL;ui_ai_service_state_t state={0};state.size=sizeof(state);state.version=1;
    if(s->ai_run[0])(void)ui_ai_assistant_get(s->ai,s->ai_run,&selected);
    if(!ui_ai_service_get_state(s->ai,&state)){
        const char *control_run=state.active_run_id[0]?state.active_run_id:s->ai_handoff_run;
        if(control_run[0])(void)ui_ai_assistant_get(s->ai,control_run,&active);
    }
    if(strcmp(selected?selected:"",s->ai_general_state?s->ai_general_state:"")||strcmp(active?active:"",s->ai_general_active?s->ai_general_active:"")){
        replace_text(&s->ai_general_state,selected?selected:"");replace_text(&s->ai_general_active,active?active:"");
        ++s->ai_projection_generation;schedule_refresh(s);
    }
    ui_ai_release(selected);ui_ai_release(active);
}

static void ai_assistant_free(host_window_t *s)
{
    ai_assistant_attachments_clear(s);free(s->ai_general_catalog);free(s->ai_database_path);free(s->ai_general_state);free(s->ai_general_active);s->ai_general_state=s->ai_general_active=NULL;
    s->ai_general_catalog=s->ai_database_path=NULL;
}
