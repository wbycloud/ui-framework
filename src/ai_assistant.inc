/* General assistant adapter. All execution remains in the original Coordinator,
 * model transport, capability handlers, transaction controller and DB worker. */
static int as_hash(const char *s)
{
    if(!s||strlen(s)!=64)return 0;
    for(unsigned i=0;i<64;++i)if(!strchr("0123456789abcdefABCDEF",s[i]))return 0;
    return 1;
}

static void as_free(ui_ai_service_t *s,as_run_t *a)
{
    (void)s;
    if(!a)return;
    free(a->scope);
    free(a->attachments);
    free(a);
}

ui_status_t ui_ai_assistant_enable(ui_ai_service_t *s,const ui_ai_assistant_policy_t *p)
{
    if(!ai_ui(s)||!p||p->size<sizeof(*p)||p->version!=1||
       !p->request_limit||p->request_limit>AS_REQUESTS||
       !p->deadline_ms||p->deadline_ms>1800000||
       !p->budget_nano_usd||p->budget_nano_usd>AV_BUDGET||
       p->input_rate_nano_usd>1000000||p->output_rate_nano_usd>1000000||
       !p->price_version||!p->price_evidence_sha256||!p->meter_profile||!p->meter_evidence_sha256||
       !ai_text(p->price_version,64,1)||!ai_text(p->price_evidence_sha256,64,1)||
       !ai_text(p->meter_profile,64,1)||!ai_text(p->meter_evidence_sha256,64,1))return UI_STATUS_INVALID_ARGUMENT;
    if(s->phase!=UI_AI_CREATED||s->validation||s->assistant_policy)return UI_STATUS_PERMISSION_DENIED;
    as_policy_t *copy=calloc(1,sizeof(*copy));
    if(!copy)return UI_STATUS_OUT_OF_MEMORY;
    copy->requests=p->request_limit;copy->deadline_ms=p->deadline_ms;
    copy->budget=p->budget_nano_usd;copy->input_rate=p->input_rate_nano_usd;
    copy->output_rate=p->output_rate_nano_usd;
    strcpy_s(copy->price_version,65,p->price_version);
    strcpy_s(copy->price_evidence,65,p->price_evidence_sha256);
    strcpy_s(copy->meter_profile,65,p->meter_profile);
    strcpy_s(copy->meter_evidence,65,p->meter_evidence_sha256);
    s->assistant_policy=copy;s->bytes+=sizeof(*copy);
    return UI_STATUS_OK;
}

ui_status_t ui_ai_model_open_profile(ui_ai_service_t *s,const char *path,const char *account)
{
    char location[4096],identity[256],hash[65],reference[128];
    wchar_t base[3500];
    if(!ai_ui(s)||!ai_id(account))return UI_STATUS_INVALID_ARGUMENT;
#ifdef UI_AI_FIXTURE
    if(!path)return UI_STATUS_PERMISSION_DENIED;
#endif
    if(!path){
        DWORD n=GetEnvironmentVariableW(L"LOCALAPPDATA",base,3500);
        if(!n||n>=3500||!WideCharToMultiByte(CP_UTF8,0,base,-1,location,3500,NULL,NULL))return UI_STATUS_PLATFORM_ERROR;
        size_t at=strlen(location);
        int n2=snprintf(location+at,sizeof(location)-at,"/ui-framework/xuanshu/profiles/%s.json",account);
        if(n2<0||(size_t)n2>=sizeof(location)-at)return UI_STATUS_LIMIT_EXCEEDED;
        path=location;
    }
    snprintf(identity,sizeof(identity),"deepseek-responses-v1:https://api.deepseek.com:%s",account);
    ui_cap_hash_bytes(identity,strlen(identity),hash);
    snprintf(reference,sizeof(reference),"ui-framework/xuanshu/profile/%s",hash);
    return am_open_reference(s,path,reference);
}

ui_status_t ui_ai_assistant_catalog(ui_ai_service_t *s,uint64_t instance,char **out)
{
    if(out)*out=NULL;
    if(!ai_ui(s)||!out||!instance)return UI_STATUS_INVALID_ARGUMENT;
    ui_app_instance_info_t app={0};app.size=sizeof(app);
    ui_status_t status=ui_workspace_get_instance(s->config.workspace,instance,&app);
    if(status||app.closing)return status?status:UI_STATUS_CANCELLED;
    ++s->methods;
    yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);
    yyjson_mut_val *o=d?yyjson_mut_obj(d):NULL,*caps=d?yyjson_mut_arr(d):NULL,*resources=d?yyjson_mut_arr(d):NULL;
    char *profiles=NULL;uint64_t generation=0;size_t offset=0,next=0,total=0;
    if(!o||!caps||!resources){status=UI_STATUS_OUT_OF_MEMORY;goto done;}
    yyjson_mut_doc_set_root(d,o);
    char instance_text[32];snprintf(instance_text,sizeof(instance_text),"%llu",(unsigned long long)instance);
    yyjson_mut_obj_add_strcpy(d,o,"instance",instance_text);
    do{
        ui_cap_target_t targets[32];size_t count=0;
        status=ui_capability_resources(app.host,offset,targets,32,&count,&next,&generation);
        if(status)goto done;
        for(size_t i=0;i<count;++i){
            yyjson_mut_val *r=yyjson_mut_obj(d);
            yyjson_mut_obj_add_strcpy(d,r,"id",targets[i].resource_id);
            yyjson_mut_obj_add_uint(d,r,"kind",targets[i].kind);
            yyjson_mut_obj_add_strcpy(d,r,"revision",targets[i].expected_revision);
            yyjson_mut_arr_add_val(resources,r);
        }
        offset=next;total+=count;
        if(total>256){status=UI_STATUS_LIMIT_EXCEEDED;goto done;}
    }while(next);
    status=ui_workspace_get_instance(s->config.workspace,instance,&app);
    if(status||app.closing){status=status?status:UI_STATUS_CANCELLED;goto done;}
    offset=0;generation=0;total=0;
    do{
        char *page=NULL;
        status=ui_capability_discover_v2(app.host,NULL,offset,32,&generation,&page);
        yyjson_doc *pd=page?yyjson_read(page,strlen(page),0):NULL;
        yyjson_val *rows=pd?yyjson_obj_get(yyjson_doc_get_root(pd),"items"):NULL,*row;
        size_t i,n;
        if(!status&&!yyjson_is_arr(rows))status=UI_STATUS_VALIDATION_FAILED;
        if(!status){
            yyjson_arr_foreach(rows,i,n,row){
                const char *id=ag_str(row,"id");char *desc=NULL;
                status=ui_capability_describe(app.host,id,&desc);
                if(status==UI_STATUS_NOT_FOUND)status=ui_capability_describe_v2(app.host,id,&desc);
                yyjson_doc *dd=desc?yyjson_read(desc,strlen(desc),0):NULL;
                yyjson_val *v=dd?yyjson_doc_get_root(dd):NULL;
                if(!status&&(!strcmp(ag_str(v,"effect"),"read")||
                   (!strcmp(ag_str(v,"effect"),"write")&&yyjson_get_int(yyjson_obj_get(v,"edit_contract"))==1&&yyjson_get_int(yyjson_obj_get(v,"risk"))==0)))
                    if(!yyjson_mut_arr_add_val(caps,yyjson_val_mut_copy(d,v)))status=UI_STATUS_OUT_OF_MEMORY;
                yyjson_doc_free(dd);ui_capability_release(desc);
                if(status)break;
            }
            next=(size_t)yyjson_get_uint(yyjson_obj_get(yyjson_doc_get_root(pd),"next"));
            total+=yyjson_arr_size(rows);
        }
        yyjson_doc_free(pd);ui_capability_release(page);
        if(status)goto done;
        if(total>256){status=UI_STATUS_LIMIT_EXCEEDED;goto done;}
        offset=next;
    }while(next);
    status=ui_ai_agent_profile_list(app.host,&profiles);
    if(status)goto done;
    yyjson_doc *pd=yyjson_read(profiles,strlen(profiles),0);
    int ok=pd&&yyjson_mut_obj_add_val(d,o,"profiles",yyjson_val_mut_copy(d,yyjson_doc_get_root(pd)));
    yyjson_doc_free(pd);
    ok=ok&&yyjson_mut_obj_add_val(d,o,"resources",resources)&&yyjson_mut_obj_add_val(d,o,"capabilities",caps);
    if(ok)*out=yyjson_mut_write(d,0,NULL);
    status=*out?UI_STATUS_OK:UI_STATUS_OUT_OF_MEMORY;
    if(*out&&strlen(*out)>AI_CONTEXT){free(*out);*out=NULL;status=UI_STATUS_LIMIT_EXCEEDED;}
done:
    free(profiles);yyjson_mut_doc_free(d);--s->methods;
    return status;
}

ui_status_t ui_ai_assistant_attachment_hash(const ui_ai_assistant_attachment_t *v,char out[65])
{
    if(out)out[0]=0;
    if(!v||!out)return UI_STATUS_INVALID_ARGUMENT;
    int image=v->media_type&&!strcmp(v->media_type,"image/png");
    if(v->size<sizeof(*v)||v->version!=1||!ai_text(v->name_utf8,255,0)||
       !v->media_type||(!image&&strcmp(v->media_type,"text/plain"))||
       !v->bytes||!v->byte_count||v->byte_count>(image?256u*1024u:AS_ATTACHMENT_BYTES)||
       (image&&(v->byte_count<8||memcmp(v->bytes,"\x89PNG\r\n\x1a\n",8)))||
       (!image&&(memchr(v->bytes,0,v->byte_count)||
       !MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,v->bytes,(int)v->byte_count,NULL,0))))return UI_STATUS_INVALID_ARGUMENT;
    return ui_cap_hash_bytes(v->bytes,v->byte_count,out);
}

static char *as_attachments(const ui_ai_assistant_request_t *q)
{
    if(q->attachment_count>AS_ATTACHMENTS||(q->attachment_count&&!q->attachments))return NULL;
    yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *a=d?yyjson_mut_arr(d):NULL;
    if(!a){yyjson_mut_doc_free(d);return NULL;}yyjson_mut_doc_set_root(d,a);
    for(size_t i=0;i<q->attachment_count;++i){
        const ui_ai_assistant_attachment_t *v=&q->attachments[i];
        int image=v->media_type&&!strcmp(v->media_type,"image/png");
        char hash[65];
        if(ui_ai_assistant_attachment_hash(v,hash)){
            yyjson_mut_doc_free(d);return NULL;
        }
        yyjson_mut_val *o=yyjson_mut_obj(d);
        yyjson_mut_obj_add_strcpy(d,o,"name",v->name_utf8);
        yyjson_mut_obj_add_strcpy(d,o,"media_type",v->media_type);
        yyjson_mut_obj_add_uint(d,o,"bytes",v->byte_count);
        yyjson_mut_obj_add_strcpy(d,o,"sha256",hash);
        yyjson_mut_obj_add_bool(d,o,"local_only",image!=0);
        if(!image)yyjson_mut_obj_add_strncpy(d,o,"text",v->bytes,v->byte_count);
        yyjson_mut_arr_add_val(a,o);
    }
    char *result=yyjson_mut_write(d,0,NULL);yyjson_mut_doc_free(d);return result;
}

static char *as_scope(ui_ai_service_t *s,const ui_ai_assistant_request_t *q)
{
    if(!q->permissions_json||strlen(q->permissions_json)>65536)return NULL;
    yyjson_doc *permissions=yyjson_read(q->permissions_json,strlen(q->permissions_json),0);
    yyjson_val *p=permissions?yyjson_doc_get_root(permissions):NULL;
    yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *root=d?yyjson_mut_arr(d):NULL;
    int ok=root&&yyjson_is_arr(p)&&yyjson_arr_size(p)==q->target_count&&ai_json_texts(p);
    if(root)yyjson_mut_doc_set_root(d,root);
    for(size_t i=0;ok&&i<q->target_count;++i){
        const ui_cap_target_v2_t *target=&q->targets[i];
        yyjson_val *allowed=yyjson_arr_get(p,i),*caps=yyjson_obj_get(allowed,"capabilities"),*profiles=yyjson_obj_get(allowed,"profiles");
        ui_app_instance_info_t app={0};
        ok=ag_target_shape(target)&&!ag_app(s,target,&app)&&
           yyjson_is_obj(allowed)&&yyjson_obj_size(allowed)>=2&&yyjson_obj_size(allowed)<=4&&yyjson_is_arr(caps)&&
           yyjson_arr_size(caps)>0&&yyjson_arr_size(caps)<=32&&yyjson_is_arr(profiles)&&yyjson_arr_size(profiles)<=8;
        if(!ok)break;
        yyjson_mut_val *item=yyjson_mut_obj(d),*cd=yyjson_mut_arr(d),*roles=yyjson_mut_arr(d);
        yyjson_mut_obj_add_uint(d,item,"target",i);
        yyjson_mut_obj_add_strcpy(d,item,"resource",target->resource.resource_id);
        yyjson_mut_obj_add_strcpy(d,item,"revision",target->expected_revision);
        yyjson_mut_obj_add_strcpy(d,item,"scope",target->scope_json);
        yyjson_val *v;size_t j,n;
        yyjson_arr_foreach(caps,j,n,v){
            const char *id=yyjson_get_str(v);char *desc=NULL;
            if(!id||!ai_text(id,255,0)){ok=0;break;}
            ui_status_t status=ui_capability_describe(app.host,id,&desc);
            if(status==UI_STATUS_NOT_FOUND)status=ui_capability_describe_v2(app.host,id,&desc);
            yyjson_doc *dd=desc?yyjson_read(desc,strlen(desc),0):NULL;yyjson_val *row=dd?yyjson_doc_get_root(dd):NULL;
            int read=!strcmp(ag_str(row,"effect"),"read"),write=!strcmp(ag_str(row,"effect"),"write");
            ok=!status&&row&&(read||(write&&yyjson_get_int(yyjson_obj_get(row,"edit_contract"))==1&&
               yyjson_get_int(yyjson_obj_get(row,"risk"))==0&&!strcmp(target->scope_json,"{\"access\":\"write\"}")))&&
               yyjson_get_uint(yyjson_obj_get(row,"target_kind"))==target->resource.kind;
            yyjson_val *values=yyjson_obj_get(yyjson_obj_get(allowed,"parameters"),id);
            if(ok&&values){
                char *schema=yyjson_val_write(yyjson_obj_get(row,"input_schema"),0,NULL);
                char *params=yyjson_val_write(values,0,NULL);
                ok=schema&&params&&!ui_cap_json_validate(schema,params,strlen(params));free(schema);free(params);
            }
            if(ok)ok=yyjson_mut_arr_add_val(cd,yyjson_val_mut_copy(d,row));
            yyjson_doc_free(dd);free(desc);if(!ok)break;
        }
        yyjson_arr_foreach(profiles,j,n,v){
            const char *id=yyjson_get_str(v);char *profile=NULL;
            if(!ok||!id||ui_ai_agent_profile_get(app.host,id,&profile)){ok=0;break;}
            yyjson_doc *pd=yyjson_read(profile,strlen(profile),0);
            ok=pd&&yyjson_mut_arr_add_val(roles,yyjson_val_mut_copy(d,yyjson_doc_get_root(pd)));
            yyjson_doc_free(pd);free(profile);if(!ok)break;
        }
        yyjson_val *parameters=yyjson_obj_get(allowed,"parameters");
        if(parameters){
            if(!yyjson_is_obj(parameters))ok=0;
            else{
                yyjson_obj_iter it=yyjson_obj_iter_with(parameters);yyjson_val *key;
                while((key=yyjson_obj_iter_next(&it))!=NULL)if(!am_contains(caps,yyjson_get_str(key)))ok=0;
                yyjson_mut_obj_add_val(d,item,"parameter_values",yyjson_val_mut_copy(d,parameters));
            }
        }
        yyjson_val *fields=yyjson_obj_get(allowed,"fields");
        if(fields){
            if(!yyjson_is_arr(fields)||yyjson_arr_size(fields)>128)ok=0;
            else{yyjson_arr_foreach(fields,j,n,v)if(!ai_text(yyjson_get_str(v),127,0))ok=0;
                yyjson_mut_obj_add_val(d,item,"fields",yyjson_val_mut_copy(d,fields));}
        }
        ok=ok&&yyjson_mut_obj_add_val(d,item,"capabilities",cd)&&yyjson_mut_obj_add_val(d,item,"profiles",roles)&&yyjson_mut_arr_add_val(root,item);
    }
    char *result=ok?yyjson_mut_write(d,0,NULL):NULL;
    if(result&&strlen(result)>65536){free(result);result=NULL;}
    yyjson_doc_free(permissions);yyjson_mut_doc_free(d);return result;
}

ui_status_t ui_ai_assistant_propose(ui_ai_service_t *s,const ui_ai_assistant_request_t *q,char out[65])
{
    if(out)out[0]=0;
    if(!ai_ui(s)||!out||!q||q->size<sizeof(*q)||q->version!=1||
       !s->assistant_policy||s->validation||!s->coordinator||s->coordinator->schema!=3||
       q->target_count>16||(q->target_count&&(!q->targets||!q->task.targets))||
       q->target_count!=q->task.target_count||(!q->target_count&&q->attachment_count)||
       (q->parent_run_id&&*q->parent_run_id&&!ai_id(q->parent_run_id)))return UI_STATUS_INVALID_ARGUMENT;
    ++s->methods;
    as_run_t *a=calloc(1,sizeof(*a));ui_status_t status=UI_STATUS_OUT_OF_MEMORY;
    if(!a)goto done;
    a->scope=as_scope(s,q);a->attachments=as_attachments(q);
    if(!a->scope||!a->attachments){status=UI_STATUS_VALIDATION_FAILED;goto done;}
    ui_ai_task_request_t binding={0};binding.targets=q->task.targets;binding.target_count=q->task.target_count;
    status=ai_targets_check(s,&binding,1);if(status)goto done;
    for(size_t i=0;i<q->target_count;++i){
        const ui_cap_target_t *read=&q->task.targets[i];const ui_cap_target_v2_t *target=&q->targets[i];
        if(read->instance_id!=target->resource.instance_id||read->kind!=target->resource.kind||
           strcmp(read->host_epoch,target->resource.host_epoch)||strcmp(read->open_epoch,target->resource.open_epoch)||
           strcmp(read->resource_id,target->resource.resource_id)||strcmp(read->expected_revision,target->expected_revision)){
            status=UI_STATUS_VALIDATION_FAILED;goto done;
        }
    }
    a->bytes=sizeof(*a)+strlen(a->scope)+strlen(a->attachments)+2;
    a->policy=*s->assistant_policy;a->target_count=q->target_count;
    if(q->target_count)memcpy(a->targets,q->targets,q->target_count*sizeof(*q->targets));
    a->stage=AS_PLANNING;
    int owned=0;
    status=am_propose_common(s,q->parent_run_id&&*q->parent_run_id?q->parent_run_id:NULL,&q->task,out,a,&owned);
    if(owned)a=NULL;
done:
    as_free(s,a);--s->methods;return status;
}
