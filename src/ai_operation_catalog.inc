#define OP_DECLARATIONS 8u
#define OP_DECLARATION_BYTES 8192u

static const char *op_text(yyjson_val *o,const char *key,size_t maximum)
{
    yyjson_val *v=yyjson_obj_get(o,key);const char *s=yyjson_get_str(v);
    return s&&strlen(s)==yyjson_get_len(v)&&ui_cap_text(s,maximum,0)?s:NULL;
}

static int op_action_sources(yyjson_val *action,yyjson_val *fields,yyjson_val *read,yyjson_val *write)
{
    yyjson_val *identity=yyjson_obj_get(action,"identity"),*sources=yyjson_obj_get(action,"sources");
    yyjson_val *rp=yyjson_obj_get(read,"properties"),*wp=yyjson_obj_get(write,"properties");
    if(!yyjson_is_arr(identity)||yyjson_arr_size(identity)>16||!yyjson_is_obj(sources)||yyjson_obj_size(sources)>32)return 0;
    size_t i,n;yyjson_val *key,*value;
    yyjson_arr_foreach(identity,i,n,value){
        const char *name=yyjson_get_str(value);
        if(!name||strlen(name)!=yyjson_get_len(value)||!yyjson_obj_get(rp,name)||!yyjson_obj_get(wp,name)||
           !yyjson_equals_str(yyjson_obj_get(fields,name),"value")||yyjson_obj_get(sources,name))return 0;
        for(size_t j=0;j<i;++j)if(yyjson_equals_str(yyjson_arr_get(identity,j),name))return 0;
    }
    yyjson_obj_foreach(sources,i,n,key,value){
        const char *parameter=yyjson_get_str(key);size_t j,count;yyjson_val *field;
        if(strlen(parameter)!=yyjson_get_len(key)||!yyjson_obj_get(wp,parameter)||!yyjson_is_arr(value)||
           !yyjson_arr_size(value)||yyjson_arr_size(value)>16)return 0;
        yyjson_arr_foreach(value,j,count,field){
            const char *name=yyjson_get_str(field);
            if(!name||strlen(name)!=yyjson_get_len(field)||!yyjson_obj_get(fields,name))return 0;
        }
    }
    return 1;
}

static int op_projection(yyjson_val *fields,yyjson_val *schema)
{
    yyjson_val *properties=yyjson_obj_get(schema,"properties");
    const char *revision=op_text(yyjson_obj_get(properties,"revision"),"type",16);
    if(!revision||strcmp(revision,"string")||!yyjson_is_obj(fields)||
       !yyjson_obj_size(fields)||yyjson_obj_size(fields)>16||yyjson_obj_get(fields,"revision"))return 0;
    size_t i,n;yyjson_val *key,*value;
    yyjson_obj_foreach(fields,i,n,key,value){
        const char *field=yyjson_get_str(key);
        if(strlen(field)!=yyjson_get_len(key)||!ui_cap_text(field,127,0))return 0;
        const char *type=op_text(yyjson_obj_get(properties,field),"type",16);
        if(!type)return 0;
        if(yyjson_equals_str(value,"count")){if(strcmp(type,"array"))return 0;}
        else if(!yyjson_equals_str(value,"value")||
            (strcmp(type,"string")&&strcmp(type,"integer")&&strcmp(type,"number")&&strcmp(type,"boolean")))return 0;
    }
    return 1;
}

static capability_t *op_capability(registry_t *r,resource_t *resource,yyjson_val *spec,int write)
{
    const char *id=op_text(spec,"capability",255);
    capability_t *c=id?find_cap(r,id):NULL;
    if(!c||c->retiring||c->desc.target_kind!=resource->desc.kind||
       strcmp(c->desc.effect,write?"write":"read"))return NULL;
    if(write)return c->v2&&c->v2->edit&&c->v2->edit->risk==UI_EDIT_REVERSIBLE&&
        !c->v2->execution_policy?c:NULL;
    return !c->v2&&(!c->entry_target||!strcmp(c->entry_target->resource_id,resource->id))?c:NULL;
}

static int op_check_spec(registry_t *r,yyjson_val *root,yyjson_mut_doc *normalized)
{
    const char *resource=op_text(root,"resource",255);
    resource_t *target=resource?find_resource(r,resource):NULL;
    yyjson_val *observe=yyjson_obj_get(root,"observe"),*fields=yyjson_obj_get(observe,"fields");
    yyjson_val *actions=yyjson_obj_get(root,"actions"),*max=yyjson_obj_get(root,"max_actions");
    if(!target||target->retiring||!yyjson_is_obj(root)||yyjson_obj_size(root)!=7||
       !yyjson_is_uint(yyjson_obj_get(root,"version"))||yyjson_get_uint(yyjson_obj_get(root,"version"))!=1||
       !op_text(root,"id",64)||!op_text(root,"title",127)||
       !yyjson_is_arr(actions)||!yyjson_arr_size(actions)||yyjson_arr_size(actions)>8||
       !yyjson_is_uint(max)||!yyjson_get_uint(max)||yyjson_get_uint(max)>8)return 0;

    yyjson_mut_val *copy=normalized?yyjson_mut_doc_get_root(normalized):NULL;
    capability_t *reader=op_capability(r,target,observe,0);if(!reader)return 0;
    for(size_t i=0;i<=yyjson_arr_size(actions);++i){
        yyjson_val *spec=i?yyjson_arr_get(actions,i-1):observe;
        capability_t *cap=i?op_capability(r,target,spec,1):reader;
        if(!cap)return 0;
        if(normalized){
            if(yyjson_obj_size(spec)!=(i?3u:2u))return 0;
            for(size_t j=0;i&&j+1<i;++j)
                if(yyjson_equals_str(yyjson_obj_get(yyjson_arr_get(actions,j),"capability"),cap->id))return 0;
            yyjson_doc *input=yyjson_read(cap->input,strlen(cap->input),0),*output=yyjson_read(cap->output,strlen(cap->output),0);
            yyjson_mut_val *row=i?yyjson_mut_arr_get(yyjson_mut_obj_get(copy,"actions"),i-1):yyjson_mut_obj_get(copy,"observe");
            yyjson_doc *read=i?yyjson_read(reader->input,strlen(reader->input),0):NULL;
            int ok=input&&output&&op_projection(fields,yyjson_doc_get_root(output))&&
                (!i||(read&&op_action_sources(spec,fields,yyjson_doc_get_root(read),yyjson_doc_get_root(input))))&&
                yyjson_mut_obj_add_val(normalized,row,"input_schema",yyjson_val_mut_copy(normalized,yyjson_doc_get_root(input)))&&
                yyjson_mut_obj_add_strcpy(normalized,row,"descriptor_hash",cap->hash);
            yyjson_doc_free(input);yyjson_doc_free(output);yyjson_doc_free(read);
            if(!ok)return 0;
        }else if(!yyjson_equals_str(yyjson_obj_get(spec,"descriptor_hash"),cap->hash))return 0;
    }
    return 1;
}

ui_status_t ui_ai_operation_register(ui_host_t *host,const char *raw)
{
    registry_t *r;ui_status_t status=registry(host,&r);
    if(status)return status;
    if(!host->capability_read_entries||is_closing(r)||host->dispatch_blocked)return UI_STATUS_PERMISSION_DENIED;
    size_t length=bounded_length(raw,OP_DECLARATION_BYTES);
    if(!raw||length>OP_DECLARATION_BYTES||r->operation_count>=OP_DECLARATIONS)return UI_STATUS_LIMIT_EXCEEDED;
    char *canonical=NULL;status=ui_cap_json_canonicalize(raw,length,&canonical,&length);
    if(status)return status;
    yyjson_doc *d=yyjson_read(canonical,length,0);free(canonical);
    yyjson_val *root=yyjson_doc_get_root(d);
    const char *id=op_text(root,"id",64);
    if(!id){yyjson_doc_free(d);return UI_STATUS_VALIDATION_FAILED;}
    for(agent_profile_t *p=r->operations;p;p=p->next)if(!strcmp(p->id,id)){yyjson_doc_free(d);return UI_STATUS_ALREADY_EXISTS;}
    yyjson_mut_doc *normalized=yyjson_doc_mut_copy(d,NULL);
    if(!status&&!normalized)status=UI_STATUS_OUT_OF_MEMORY;
    if(!status&&!op_check_spec(r,root,normalized))status=UI_STATUS_VALIDATION_FAILED;
    char *json=!status?yyjson_mut_write(normalized,0,&length):NULL;
    yyjson_mut_doc_free(normalized);
    if(!status&&!json)status=UI_STATUS_OUT_OF_MEMORY;
    if(!status&&length>OP_DECLARATION_BYTES)status=UI_STATUS_LIMIT_EXCEEDED;
    size_t bytes=json?sizeof(agent_profile_t)+length+strlen(id)+2:0;
    if(!status&&bytes>UI_CAPABILITY_CATALOG_BYTES-r->catalog_bytes)status=UI_STATUS_LIMIT_EXCEEDED;
    if(!status){
        agent_profile_t *p=calloc(1,sizeof(*p));
        if(!p||!(p->id=ui_strdup(id))){free(p);status=UI_STATUS_OUT_OF_MEMORY;}
        else{p->json=json;p->bytes=bytes;p->next=r->operations;r->operations=p;r->catalog_bytes+=bytes;++r->operation_count;json=NULL;}
    }
    free(json);yyjson_doc_free(d);return status;
}

ui_status_t ui_cap_operation_get(ui_host_t *host,const char *id,char **out)
{
    registry_t *r;ui_status_t status=registry(host,&r);*out=NULL;
    if(status)return status;
    if(is_closing(r)||host->dispatch_blocked)return UI_STATUS_CANCELLED;
    for(agent_profile_t *p=r->operations;p;p=p->next)if(!strcmp(p->id,id)){
        yyjson_doc *d=yyjson_read(p->json,strlen(p->json),0);
        int live=d&&op_check_spec(r,yyjson_doc_get_root(d),NULL);yyjson_doc_free(d);
        if(!live)return UI_STATUS_NOT_FOUND;
        *out=ui_strdup(p->json);return *out?UI_STATUS_OK:UI_STATUS_OUT_OF_MEMORY;
    }
    return UI_STATUS_NOT_FOUND;
}

ui_status_t ui_ai_operation_list(ui_host_t *host,char **out)
{
    if(!out)return UI_STATUS_INVALID_ARGUMENT;
    *out=NULL;registry_t *r;ui_status_t status=registry(host,&r);
    if(status)return status;
    if(is_closing(r)||host->dispatch_blocked)return UI_STATUS_CANCELLED;
    yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *rows=d?yyjson_mut_arr(d):NULL;
    if(!rows){yyjson_mut_doc_free(d);return UI_STATUS_OUT_OF_MEMORY;}yyjson_mut_doc_set_root(d,rows);
    for(agent_profile_t *p=r->operations;p;p=p->next){
        yyjson_doc *v=yyjson_read(p->json,strlen(p->json),0);
        yyjson_val *root=yyjson_doc_get_root(v);
        int ok=v&&(!op_check_spec(r,root,NULL)||yyjson_mut_arr_add_val(rows,yyjson_val_mut_copy(d,root)));
        yyjson_doc_free(v);if(!ok){yyjson_mut_doc_free(d);return UI_STATUS_OUT_OF_MEMORY;}
    }
    *out=yyjson_mut_write(d,0,NULL);yyjson_mut_doc_free(d);
    return *out?UI_STATUS_OK:UI_STATUS_OUT_OF_MEMORY;
}
