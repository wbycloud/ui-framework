/* Resolve a local proposal from the installed declarations and current public
 * registrations. Preview and approval still belong to the Service. */
static int ai_context_phrase(const char *input,yyjson_val *phrases)
{
    size_t i,n;yyjson_val *phrase;
    yyjson_arr_foreach(phrases,i,n,phrase){
        const char *text=yyjson_get_str(phrase);size_t length=yyjson_get_len(phrase);
        if(text&&length>=3&&length<=128&&strlen(text)==length&&strstr(input,text))return 1;
    }
    return 0;
}

static int ai_context_mentions(const char *input,const char *const *phrases,size_t count)
{
    for(size_t i=0;i<count;++i)if(strstr(input,phrases[i]))return 1;
    return 0;
}

static int ai_context_bind(host_window_t *s,yyjson_val *preset,uint64_t instance,ui_cap_target_v2_t *target)
{
    const char *app_id=ai_release_string(preset,"app_id",128);
    const char *resource=ai_release_string(preset,"resource",256);
    const char *id=ai_release_string(preset,"capability",256);
    ui_app_instance_info_t app;
    if(!app_id||!resource||!id||!get_instance(s,instance,&app)||app.closing||strcmp(app.app_id,app_id))return 0;
    memset(target,0,sizeof(*target));target->size=sizeof(*target);target->version=2;
    if(ui_ai_edit_bind_target(s->ai,instance,resource,target))return 0;
    if(!get_instance(s,instance,&app)||app.closing||strcmp(app.app_id,app_id))return 0;
    char *raw=NULL;
    ui_status_t status=ui_capability_describe(app.host,id,&raw);
    if(status==UI_STATUS_NOT_FOUND)status=ui_capability_describe_v2(app.host,id,&raw);
    yyjson_doc *d=raw?yyjson_read(raw,strlen(raw),0):NULL;
    yyjson_val *cap=yyjson_doc_get_root(d),*properties=yyjson_obj_get(yyjson_obj_get(cap,"output_schema"),"properties");
    const char *effect=ai_release_string(cap,"effect",16);
    char *schema=yyjson_val_write(yyjson_obj_get(cap,"input_schema"),0,NULL);
    int ok=!status&&effect&&!strcmp(effect,"read")&&schema&&!ui_cap_json_validate(schema,"{}",2)&&
        yyjson_get_uint(yyjson_obj_get(cap,"target_kind"))==target->resource.kind;
    yyjson_val *fields=yyjson_obj_get(preset,"fields");
    if(ok&&fields){
        size_t i,n;yyjson_val *field;
        ok=yyjson_is_arr(fields)&&yyjson_arr_size(fields)>0&&yyjson_arr_size(fields)<=32;
        yyjson_arr_foreach(fields,i,n,field){
            const char *name=yyjson_get_str(field);
            if(!name||!yyjson_get_len(field)||yyjson_get_len(field)>=128||
               strlen(name)!=yyjson_get_len(field)||!yyjson_obj_get(properties,name)){ok=0;break;}
        }
    }else if(ok){
        /* Version 1 single-preset releases predate explicit field lists. */
        ok=preset==yyjson_obj_get(yyjson_doc_get_root(s->ai_release),"read_preset")&&
           yyjson_obj_get(properties,"state")&&yyjson_obj_get(properties,"revision");
    }
    free(schema);yyjson_doc_free(d);ui_capability_release(raw);
    if(ok)strcpy_s(target->scope_json,sizeof(target->scope_json),"{\"access\":\"read\"}");
    return ok;
}

static void ai_context_resolve(host_window_t *s,const char *input,assistant_submission_t *receipt)
{
    static const char *const read_words[]={"读","查看","看看","概括","总结","核对","分析","告诉我","read ","summarize "};
    static const char *const current_words[]={"当前","这份","这篇","这局","本局","本页","这个应用","current "};
    static const char *const objects[]={"当前应用","这个应用","这份文档","当前文档","当前页面","当前表格","当前文件","current document","current app"};
    if(!ai_context_mentions(input,read_words,sizeof(read_words)/sizeof(*read_words)))return;
    int current=ai_context_mentions(input,current_words,sizeof(current_words)/sizeof(*current_words));
    yyjson_val *root=yyjson_doc_get_root(s->ai_release),*presets=yyjson_obj_get(root,"read_presets");
    size_t count=yyjson_arr_size(presets),named_length=0;
    yyjson_val *legacy=count?NULL:yyjson_obj_get(root,"read_preset");
    uint64_t active=ui_workspace_active(s->workspace);
    if(count>32){receipt->message="应用读取配置不可用。";return;}
    int requested=ai_context_mentions(input,objects,sizeof(objects)/sizeof(*objects)),rank=0,overflow=0;
    for(size_t i=0;i<=count;++i){
        yyjson_val *preset=i<count?yyjson_arr_get(presets,i):legacy;
        const char *app_id=ai_release_string(preset,"app_id",128);
        const char *label=ai_release_string(preset,"label",128);
        if(!app_id||!label)continue;
        int intent=ai_context_phrase(input,yyjson_obj_get(preset,"intents"));
        if(preset==legacy&&!strncmp(label,"允许",strlen("允许")))intent|=strstr(input,label+strlen("允许"))!=NULL;
        requested|=intent;
        for(size_t j=0;j<ui_workspace_count(s->workspace);++j){
            uint64_t id=ui_workspace_instance_at(s->workspace,j);ui_app_instance_info_t app;
            if(!get_instance(s,id,&app)||app.closing||strcmp(app.app_id,app_id))continue;
            char title[UI_WORKSPACE_TITLE_MAX_BYTES+1],display[512];size_t length=0;
            instance_label(s,&app,display,sizeof(display));
            if(ui_workspace_get_instance_title(s->workspace,id,title,sizeof(title),&length))title[0]=0;
            size_t named=title[0]&&strstr(input,title)?strlen(title):0;
            const char *display_match=strstr(input,display);
            char after=display_match?display_match[strlen(display)]:0;
            int exact=display_match&&(after<'0'||after>'9');
            int score=exact?4:named?3:intent&&id==active&&current?2:intent&&!current?1:0;
            if(!score||score<rank||(score==rank&&named<named_length))continue;
            requested=1;
            if(score>rank||named>named_length){rank=score;named_length=named;receipt->count=0;overflow=0;}
            ui_cap_target_v2_t target;
            if(!ai_context_bind(s,preset,id,&target))continue;
            if(receipt->count==sizeof(receipt->choices)/sizeof(receipt->choices[0])){overflow=1;continue;}
            assistant_context_choice_t *choice=&receipt->choices[receipt->count++];
            choice->preset=preset;choice->target=target;
            strcpy_s(choice->title,sizeof(choice->title),display);
        }
    }
    if(overflow){receipt->count=0;receipt->message="有多个可读对象。请在消息里写出所指的标签名称。";}
    else if(!receipt->count&&requested)receipt->message="当前没有可读取的对应内容。可以指定另一个已打开的对象，或继续咨询一般问题。";
    else if(receipt->count>1)receipt->phase=AI_SUBMIT_CHOOSE;
}

static void ai_operation_resolve(host_window_t *s,const char *input,assistant_submission_t *receipt)
{
    uint64_t active=ui_workspace_active(s->workspace);int rank=0,overflow=0;
    for(size_t i=0;i<ui_workspace_count(s->workspace);++i){
        uint64_t instance=ui_workspace_instance_at(s->workspace,i);ui_app_instance_info_t app;
        if(!get_instance(s,instance,&app)||app.closing)continue;
        char title[UI_WORKSPACE_TITLE_MAX_BYTES+1],label[512];size_t length=0;
        instance_label(s,&app,label,sizeof(label));
        if(ui_workspace_get_instance_title(s->workspace,instance,title,sizeof(title),&length))title[0]=0;
        const char *named=strstr(input,label);char after=named?named[strlen(label)]:0;
        int score=named&&(after<'0'||after>'9')?3:title[0]&&strstr(input,title)?2:instance==active?1:0;
        if(!score||score<rank)continue;
        if(score>rank){rank=score;receipt->count=0;overflow=0;}
        char *raw=NULL;if(ui_ai_operation_list(app.host,&raw))continue;
        yyjson_doc *d=yyjson_read(raw,strlen(raw),0);ui_capability_release(raw);
        size_t j,n;yyjson_val *row;
        yyjson_arr_foreach(yyjson_doc_get_root(d),j,n,row){
            const char *id=ai_release_string(row,"id",64),*resource=ai_release_string(row,"resource",255),
                *business=ai_release_string(row,"title",127);
            if(!id||!resource||!business)continue;
            if(receipt->count==sizeof(receipt->choices)/sizeof(receipt->choices[0])){overflow=1;break;}
            ui_cap_target_v2_t target={0};target.size=sizeof(target);target.version=2;
            if(ui_ai_edit_bind_target(s->ai,instance,resource,&target))continue;
            strcpy_s(target.scope_json,sizeof(target.scope_json),"{\"access\":\"write\"}");
            assistant_context_choice_t *choice=&receipt->choices[receipt->count++];
            memset(choice,0,sizeof(*choice));choice->target=target;
            strcpy_s(choice->operation,sizeof(choice->operation),id);
            _snprintf_s(choice->title,sizeof(choice->title),_TRUNCATE,"%s · %s",label,business);
        }
        yyjson_doc_free(d);
    }
    if(overflow){receipt->count=0;receipt->message="有多个可代操作对象，请写出确切的标签名称。";}
    else if(!receipt->count)receipt->message="所指应用没有登记可代操作的业务范围。可以切换应用或改为提问。";
}

static ui_status_t ai_context_scope(host_window_t *s,const assistant_context_choice_t *choice,
                                    yyjson_mut_doc *d,yyjson_mut_val *allowed,ui_cap_target_t *read,
                                    ui_cap_target_v2_t *fixed,char **projection)
{
    const ui_cap_target_v2_t *prior=&choice->target;
    const ui_cap_resource_ref_v2_t *resource=&prior->resource;
    int bound;
    if(choice->operation[0]){
        memset(fixed,0,sizeof(*fixed));fixed->size=sizeof(*fixed);fixed->version=2;
        bound=!ui_ai_edit_bind_target(s->ai,resource->instance_id,resource->resource_id,fixed);
        strcpy_s(fixed->scope_json,sizeof(fixed->scope_json),"{\"access\":\"write\"}");
    }else bound=ai_context_bind(s,choice->preset,resource->instance_id,fixed);
    if(!bound||
       fixed->resource.kind!=resource->kind||strcmp(fixed->resource.resource_id,resource->resource_id)||
       strcmp(fixed->resource.host_epoch,resource->host_epoch)||strcmp(fixed->resource.open_epoch,resource->open_epoch)||
       strcmp(fixed->expected_revision,prior->expected_revision)||strcmp(fixed->scope_json,prior->scope_json))
        return UI_STATUS_VALIDATION_FAILED;
    memset(read,0,sizeof(*read));read->size=sizeof(*read);read->version=1;
    read->instance_id=resource->instance_id;read->kind=resource->kind;
    strcpy_s(read->resource_id,sizeof(read->resource_id),resource->resource_id);
    strcpy_s(read->host_epoch,sizeof(read->host_epoch),resource->host_epoch);
    strcpy_s(read->open_epoch,sizeof(read->open_epoch),resource->open_epoch);
    strcpy_s(read->expected_revision,sizeof(read->expected_revision),prior->expected_revision);
    strcpy_s(read->scope_json,sizeof(read->scope_json),"{\"access\":\"read\"}");

    if(choice->operation[0]){
        yyjson_mut_val *operation=yyjson_mut_obj(d);
        if(!operation||!yyjson_mut_obj_add_strcpy(d,operation,"operation",choice->operation)||
           !yyjson_mut_arr_add_val(allowed,operation))return UI_STATUS_OUT_OF_MEMORY;
        *projection=_strdup("{\"capabilities\":[],\"fields\":[]}");
        return *projection?UI_STATUS_OK:UI_STATUS_OUT_OF_MEMORY;
    }

    yyjson_val *declared=yyjson_obj_get(choice->preset,"fields");
    yyjson_mut_val *fields=declared?yyjson_val_mut_copy(d,declared):yyjson_mut_arr_with_str(d,(const char *[]){"state","revision"},2);
    yyjson_mut_val *caps=yyjson_mut_arr(d),*item=yyjson_mut_obj(d),*o=yyjson_mut_obj(d);
    const char *id=yyjson_get_str(yyjson_obj_get(choice->preset,"capability"));
    if(!fields||!caps||!item||!o||!yyjson_mut_arr_add_strcpy(d,caps,id)||
       !yyjson_mut_obj_add_val(d,o,"capabilities",yyjson_mut_val_mut_copy(d,caps))||
       !yyjson_mut_obj_add_val(d,o,"fields",yyjson_mut_val_mut_copy(d,fields))||
       !yyjson_mut_obj_add_val(d,item,"capabilities",caps)||!yyjson_mut_obj_add_val(d,item,"fields",fields)||
       !yyjson_mut_obj_add_val(d,item,"profiles",yyjson_mut_arr(d))||!yyjson_mut_arr_add_val(allowed,item))
        return UI_STATUS_OUT_OF_MEMORY;
    *projection=yyjson_mut_val_write(o,0,NULL);
    return *projection?UI_STATUS_OK:UI_STATUS_OUT_OF_MEMORY;
}
