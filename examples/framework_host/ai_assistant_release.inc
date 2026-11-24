#include "../../src/ai_assistant_price.h"

static const char *ai_release_string(yyjson_val *o,const char *key,size_t capacity)
{
    yyjson_val *v=yyjson_obj_get(o,key);const char *text=yyjson_get_str(v);
    size_t length=yyjson_get_len(v);
    return text&&length&&length<capacity&&strlen(text)==length?text:NULL;
}

/* Read only the release descriptor beside the actual host executable. It is
 * deployment input for this Service, independent of the profile and its key. */
static yyjson_doc *ai_assistant_release(host_window_t *s,char evidence[65])
{
    wchar_t path[4096];DWORD n=GetModuleFileNameW(NULL,path,4096);
    wchar_t *name=n&&n<4096?wcsrchr(path,L'\\'):NULL;
    if(!name||wcscpy_s(name+1,4096-(size_t)(name+1-path),L"assistant-release.json"))return NULL;
    FILE *f=_wfopen(path,L"rb");if(!f)return NULL;
    char raw[8193];size_t length=fread(raw,1,sizeof(raw),f);
    int complete=feof(f)&&!ferror(f);fclose(f);
    if(!complete||length>8192||!length)return NULL;
    yyjson_doc *d=yyjson_read(raw,length,0);yyjson_val *root=yyjson_doc_get_root(d);
    if(!yyjson_is_uint(yyjson_obj_get(root,"version"))||yyjson_get_uint(yyjson_obj_get(root,"version"))!=1){yyjson_doc_free(d);return NULL;}
    ui_ai_assistant_attachment_t blob={0};blob.size=sizeof(blob);blob.version=1;
    blob.name_utf8="assistant-release.json";blob.media_type="text/plain";blob.bytes=raw;blob.byte_count=length;
    if(ui_ai_assistant_attachment_hash(&blob,evidence)){yyjson_doc_free(d);return NULL;}
    yyjson_val *preset=yyjson_obj_get(root,"read_preset");
    const char *app=ai_release_string(preset,"app_id",sizeof(s->ai_quick_app));
    const char *resource=ai_release_string(preset,"resource",sizeof(s->ai_quick_resource));
    const char *capability=ai_release_string(preset,"capability",sizeof(s->ai_quick_capability));
    const char *label=ai_release_string(preset,"label",sizeof(s->ai_quick_label));
    if(app&&resource&&capability&&label){
        strcpy_s(s->ai_quick_app,sizeof(s->ai_quick_app),app);
        strcpy_s(s->ai_quick_resource,sizeof(s->ai_quick_resource),resource);
        strcpy_s(s->ai_quick_capability,sizeof(s->ai_quick_capability),capability);
        strcpy_s(s->ai_quick_label,sizeof(s->ai_quick_label),label);
    }
    return d;
}

static void ai_assistant_release_price(yyjson_doc *d,const char *evidence,ui_ai_assistant_policy_t *p)
{
    yyjson_val *price=yyjson_obj_get(yyjson_doc_get_root(d),"price");
    const char *model=ai_release_string(price,"model",128),*protocol=ai_release_string(price,"protocol",64);
    const char *version=ai_release_string(price,"version",65),*profile=ai_release_string(price,"meter_profile",65);
    yyjson_val *input=yyjson_obj_get(price,"input_nano_usd_per_token"),*output=yyjson_obj_get(price,"output_nano_usd_per_token");
    if(!model||strcmp(model,"deepseek-flash")||!protocol||strcmp(protocol,"deepseek-responses-v1")||
       !profile||strcmp(profile,p->meter_profile)||!version||strncmp(version,AI_RELEASE_PRICE_PREFIX,sizeof(AI_RELEASE_PRICE_PREFIX)-1)||
       !ai_assistant_price_current(version)||!ai_release_string(price,"pricing_source",256)||!ai_release_string(price,"protocol_source",256)||
       !yyjson_is_uint(input)||!yyjson_is_uint(output)||!yyjson_get_uint(input)||yyjson_get_uint(input)>1000000||
       !yyjson_get_uint(output)||yyjson_get_uint(output)>1000000)return;
    p->price_version=version;p->price_evidence_sha256=evidence;
    p->input_rate_nano_usd=yyjson_get_uint(input);p->output_rate_nano_usd=yyjson_get_uint(output);
}

static int ai_assistant_quick_app(host_window_t *s,uint64_t instance,ui_app_instance_info_t *app)
{
    return s->ai_quick_label[0]&&get_instance(s,instance,app)&&!app->closing&&app->app_id&&!strcmp(app->app_id,s->ai_quick_app);
}

static ui_status_t ai_assistant_quick_bind(host_window_t *s,uint64_t instance)
{
    ui_app_instance_info_t app;
    if(instance!=ui_workspace_active(s->workspace)||!ai_assistant_quick_app(s,instance,&app))return UI_STATUS_VALIDATION_FAILED;
    ui_cap_target_v2_t target={0};target.size=sizeof(target);target.version=2;
    ui_status_t status=ui_ai_edit_bind_target(s->ai,instance,s->ai_quick_resource,&target);
    if(status)return status;
    if(!ai_assistant_quick_app(s,instance,&app))return UI_STATUS_VALIDATION_FAILED;
    char *json=NULL;status=ui_capability_describe(app.host,s->ai_quick_capability,&json);
    yyjson_doc *d=json?yyjson_read(json,strlen(json),0):NULL;
    yyjson_val *cap=yyjson_doc_get_root(d),*required=yyjson_obj_get(yyjson_obj_get(cap,"input_schema"),"required");
    yyjson_val *fields=yyjson_obj_get(yyjson_obj_get(cap,"output_schema"),"properties");
    const char *effect=ai_release_string(cap,"effect",16);
    int allowed=!status&&effect&&!strcmp(effect,"read")&&yyjson_get_uint(yyjson_obj_get(cap,"target_kind"))==target.resource.kind&&
        (!required||(yyjson_is_arr(required)&&!yyjson_arr_size(required)))&&yyjson_obj_get(fields,"state")&&yyjson_obj_get(fields,"revision");
    yyjson_doc_free(d);ui_capability_release(json);
    if(!allowed)return UI_STATUS_VALIDATION_FAILED;
    strcpy_s(target.scope_json,sizeof(target.scope_json),"{\"access\":\"read\"}");
    s->ai_quick_target=target;s->ai_quick_pending[0]=0;
    return UI_STATUS_OK;
}

static int ai_assistant_quick_scope(host_window_t *s,yyjson_mut_doc *d,yyjson_mut_val *allowed,
                                    ui_cap_target_t *read,ui_cap_target_v2_t *fixed,char **projection)
{
    *fixed=s->ai_quick_target;
    const ui_cap_resource_ref_v2_t *resource=&fixed->resource;
    memset(read,0,sizeof(*read));read->size=sizeof(*read);read->version=1;
    read->instance_id=resource->instance_id;read->kind=resource->kind;
    strcpy_s(read->resource_id,sizeof(read->resource_id),resource->resource_id);
    strcpy_s(read->host_epoch,sizeof(read->host_epoch),resource->host_epoch);
    strcpy_s(read->open_epoch,sizeof(read->open_epoch),resource->open_epoch);
    strcpy_s(read->expected_revision,sizeof(read->expected_revision),fixed->expected_revision);
    strcpy_s(read->scope_json,sizeof(read->scope_json),fixed->scope_json);
    yyjson_mut_val *item=yyjson_mut_obj(d),*caps=yyjson_mut_arr(d),*fields=yyjson_mut_arr(d),*o=yyjson_mut_obj(d);
    if(!item||!caps||!fields||!o||!yyjson_mut_arr_add_strcpy(d,caps,s->ai_quick_capability)||
       !yyjson_mut_arr_add_str(d,fields,"state")||!yyjson_mut_arr_add_str(d,fields,"revision")||
       !yyjson_mut_obj_add_val(d,o,"capabilities",yyjson_mut_val_mut_copy(d,caps))||
       !yyjson_mut_obj_add_val(d,o,"fields",yyjson_mut_val_mut_copy(d,fields))||
       !yyjson_mut_obj_add_val(d,item,"capabilities",caps)||!yyjson_mut_obj_add_val(d,item,"fields",fields)||
       !yyjson_mut_obj_add_val(d,item,"profiles",yyjson_mut_arr(d))||!yyjson_mut_arr_add_val(allowed,item))return 0;
    *projection=yyjson_mut_val_write(o,0,NULL);
    return *projection!=NULL;
}
