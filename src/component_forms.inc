/* C2B-1: local draft lifecycle and a binding to the ONE business capability. */
struct component_form {
    ui_component_t *component;ui_cap_form_binding_desc_t desc;
    uint64_t binding,pending,generation[64],submitted[64];
    unsigned char conflict[64];char snapshot_hash[64][65];
    char model_revision[128],base_revision[128],endpoint[96];
    char read_json[UI_CAPABILITY_INPUT_BYTES+1u];
    int reading,projecting;size_t bytes;
};
static size_t form_dirty(ui_component_t *c)
{size_t i,n=0;for(i=0;i<c->desc.field_count;++i)if(c->drafts[i].flags&UI_VALUE_MODIFIED)++n;return n;}
static size_t form_conflicts(ui_component_t *c)
{size_t i,n=0;if(c->form)for(i=0;i<c->desc.field_count;++i)n+=c->form->conflict[i]!=0;return n;}
static int form_field_conflict(ui_component_t *c,size_t field)
{return c->form&&c->form->conflict[field];}
static void form_changed(ui_component_t *c,size_t field)
{if(c->form){if(c->form->generation[field]!=UINT64_MAX)++c->form->generation[field];else c->form->conflict[field]=1;}}
static int form_semantic_field(ui_component_t *c,size_t i)
{return !(c->fields[i].flags&UI_VALUE_READONLY)&&c->fields[i].kind>=UI_VALUE_TEXT&&c->fields[i].kind<=UI_VALUE_COLOR;}
static ui_status_t form_value(ui_component_t *c,size_t i,const ui_cell_t *cell,char **json,size_t *length)
{
    ui_json_t j={0};ui_cap_json_doc_t doc={0};ui_status_t s;const char *text=cell->text?cell->text:"";size_t at;
    if(strlen(text)>4096)return UI_STATUS_LIMIT_EXCEEDED;
    if((c->fields[i].flags&UI_VALUE_REQUIRED)&&!*text)return UI_STATUS_VALIDATION_FAILED;
    if(c->fields[i].kind==UI_VALUE_NUMBER){
      s=ui_cap_parse(text,strlen(text),UI_CAPABILITY_INPUT_BYTES,&doc);if(s)return s;
      if(!yyjson_is_num(yyjson_doc_get_root(doc.doc))){ui_cap_doc_free(&doc);return UI_STATUS_VALIDATION_FAILED;}
      s=ui_cap_jcs_value(yyjson_doc_get_root(doc.doc),json,length);ui_cap_doc_free(&doc);return s;}
    if(c->fields[i].kind==UI_VALUE_BOOLEAN){if(strcmp(text,"true")&&strcmp(text,"false"))return UI_STATUS_VALIDATION_FAILED;uj_add(&j,text);}
    else{
      if(c->fields[i].kind==UI_VALUE_ENUM){for(at=0;at<c->fields[i].option_count&&strcmp(c->fields[i].options[at],text);++at){}
        if(at==c->fields[i].option_count)return UI_STATUS_VALIDATION_FAILED;}
      if(c->fields[i].kind==UI_VALUE_COLOR){uint32_t color;if(!parse_color(text,&color))return UI_STATUS_VALIDATION_FAILED;}
      uj_string(&j,text);}
    if(j.failed){free(j.data);return UI_STATUS_OUT_OF_MEMORY;}*json=j.data;*length=j.length;return UI_STATUS_OK;
}
static void form_mark_conflict(ui_component_t *c)
{size_t i;for(i=0;i<c->desc.field_count;++i)if(c->drafts[i].flags&UI_VALUE_MODIFIED)c->form->conflict[i]=1;}
ui_status_t ui_capability_form_project(ui_component_t *c,const ui_cap_form_projection_t *p)
{
    component_form_t *f;ui_cap_json_doc_t doc={0};yyjson_val *root,*value;ui_cell_t values[64]={{0}},drafts[64]={{0}};
    size_t i,fields=0,bytes=0;ui_status_t s;int changed;
    if(!c||!ui_language_thread_ok(c->host)||!p||p->size<sizeof(*p)||p->version!=1||!ui_cap_text(p->revision,127,0))return UI_STATUS_INVALID_ARGUMENT;
    f=c->form;if(!f)return UI_STATUS_NOT_FOUND;if(f->projecting||f->reading)return UI_STATUS_PENDING;
    if(!p->fields_json||p->length>UI_CAPABILITY_INPUT_BYTES)return UI_STATUS_INVALID_ARGUMENT;
    f->projecting=1;ui_dispatch_enter(c->host);
    s=ui_cap_parse(p->fields_json,p->length,UI_CAPABILITY_INPUT_BYTES,&doc);if(s)goto done;
    root=yyjson_doc_get_root(doc.doc);if(!yyjson_is_obj(root)){s=UI_STATUS_VALIDATION_FAILED;goto done;}
    for(i=0;i<c->desc.field_count;++i)if(form_semantic_field(c,i)){
      ui_cell_t cell={0};char *number=NULL;size_t n=0;const char *text;char bool_text[6];++fields;
      value=yyjson_obj_get(root,c->fields[i].id);if(!value){s=UI_STATUS_VALIDATION_FAILED;goto done;}
      if(c->fields[i].kind==UI_VALUE_NUMBER){if(!yyjson_is_num(value)){s=UI_STATUS_VALIDATION_FAILED;goto done;}
        s=ui_cap_jcs_value(value,&number,&n);if(s)goto done;text=number;}
      else if(c->fields[i].kind==UI_VALUE_BOOLEAN){if(!yyjson_is_bool(value)){s=UI_STATUS_VALIDATION_FAILED;goto done;}
        snprintf(bool_text,sizeof(bool_text),"%s",yyjson_get_bool(value)?"true":"false");text=bool_text;}
      else{if(!yyjson_is_str(value)||yyjson_get_len(value)>4096||yyjson_get_len(value)!=strlen(yyjson_get_str(value))){s=UI_STATUS_VALIDATION_FAILED;goto done;}text=yyjson_get_str(value);}
      cell.size=sizeof(cell);cell.kind=c->fields[i].kind;cell.text=text;cell.flags=c->fields[i].flags;
      if(cell.kind==UI_VALUE_COLOR)(void)parse_color(text,&cell.color_rgba);
      {char *checked=NULL;size_t checked_length=0;s=form_value(c,i,&cell,&checked,&checked_length);free(checked);}
      if(!s&&!copy_cell(&values[i],&cell,&bytes))s=UI_STATUS_OUT_OF_MEMORY;
      if(!s&&!copy_cell(&drafts[i],&cell,&bytes))s=UI_STATUS_OUT_OF_MEMORY;
      free(number);if(s)goto done;
    }
    if(yyjson_obj_size(root)!=fields){s=UI_STATUS_VALIDATION_FAILED;goto done;}
    s=ui_capabilities_model_revision(c->host,&f->desc.resource,p->revision);if(s)goto done;
    changed=strcmp(f->model_revision,p->revision)!=0;
    for(i=0;i<c->desc.field_count;++i)if(form_semantic_field(c,i)){
      free_cell(&c->values[i]);c->values[i]=values[i];memset(&values[i],0,sizeof(values[i]));
      if(c->drafts[i].flags&UI_VALUE_MODIFIED){if(changed)f->conflict[i]=1;}
      else{free_cell(&c->drafts[i]);c->drafts[i]=drafts[i];memset(&drafts[i],0,sizeof(drafts[i]));form_changed(c,i);f->conflict[i]=0;}}
    snprintf(f->model_revision,sizeof(f->model_revision),"%s",p->revision);
    if(!form_dirty(c))snprintf(f->base_revision,sizeof(f->base_revision),"%s",p->revision);
done:
    for(i=0;i<64;++i){free_cell(&values[i]);free_cell(&drafts[i]);}ui_cap_doc_free(&doc);
    f->projecting=0;if(!s)render_component(c);ui_dispatch_leave(c->host);return s;
}
static ui_status_t form_read(ui_component_t *c)
{
    component_form_t *f=c->form;char revision[128]={0};size_t length=0;ui_cap_form_projection_t p={0};ui_status_t s;
    if(f->reading||f->projecting)return UI_STATUS_PENDING;f->reading=1;memset(f->read_json,0,sizeof(f->read_json));
    ++c->host->capability_ui_callbacks;++c->host->assistant_callbacks;ui_dispatch_enter(c->host);
    s=f->desc.read_model(revision,f->read_json,sizeof(f->read_json),&length,f->desc.user_data);f->reading=0;
    if(!s&&(!memchr(revision,0,sizeof(revision))||length>UI_CAPABILITY_INPUT_BYTES||f->read_json[length]))s=UI_STATUS_VALIDATION_FAILED;
    if(!s){p.size=sizeof(p);p.version=1;p.revision=revision;p.fields_json=f->read_json;p.length=length;s=ui_capability_form_project(c,&p);}
    --c->host->capability_ui_callbacks;--c->host->assistant_callbacks;ui_dispatch_leave(c->host);return s;
}
static ui_status_t form_collect(const ui_cap_ui_context_t *context,char *params,size_t capacity,size_t *length,void *data)
{
    component_form_t *f=data;ui_component_t *c=f->component;ui_json_t j={0};size_t i;ui_status_t s=UI_STATUS_OK;
    if(f->pending)return UI_STATUS_PENDING;if(form_conflicts(c))return UI_STATUS_VALIDATION_FAILED;
    if(strcmp(context->target.expected_revision,f->base_revision))return UI_STATUS_VALIDATION_FAILED;
    uj_add(&j,"{\"fields\":{");
    {int first=1;for(i=0;i<c->desc.field_count;++i)if(form_semantic_field(c,i)){
      char *value=NULL;size_t n=0;
      if(f->generation[i]==UINT64_MAX){s=UI_STATUS_LIMIT_EXCEEDED;break;}
      if((c->drafts[i].flags&(UI_VALUE_DISABLED|UI_VALUE_READONLY))&&(c->drafts[i].flags&UI_VALUE_MODIFIED)){s=UI_STATUS_PERMISSION_DENIED;break;}
      s=form_value(c,i,&c->drafts[i],&value,&n);if(s)break;
      if(!first)uj_add(&j,",");first=0;uj_string(&j,c->fields[i].id);uj_add(&j,":");uj_add(&j,value);
      s=ui_cap_hash_bytes(value,n,f->snapshot_hash[i]);free(value);if(s)break;f->submitted[i]=f->generation[i];}}
    uj_add(&j,"}}");if(j.failed&&!s)s=UI_STATUS_OUT_OF_MEMORY;
    if(!s&&(j.length>UI_CAPABILITY_INPUT_BYTES||j.length>=capacity))s=UI_STATUS_LIMIT_EXCEEDED;
    if(!s){memcpy(params,j.data,j.length+1);*length=j.length;}free(j.data);return s;
}
static void form_observe(const ui_cap_ui_notice_t *notice,void *data)
{
    component_form_t *f=data;ui_component_t *c=f->component;ui_cap_ui_notice_t n=*notice;size_t i;
    if(notice->stage==UI_CAP_UI_ADMITTED)f->pending=notice->call_id;
    if(notice->stage==UI_CAP_UI_FINISHED){ui_cap_result_v2_t r={0};ui_status_t s;
      r.size=sizeof(r);r.version=2;s=ui_capability_get_result_v2(c->host,notice->call_id,&r);
      if(!s)s=form_read(c);
      if(!s&&r.outcome==UI_CAP2_OK&&r.effect!=UI_CAP2_EFFECT_UNKNOWN){
        if(strcmp(f->model_revision,r.revision_after))s=UI_STATUS_VALIDATION_FAILED;
        for(i=0;!s&&i<c->desc.field_count;++i)if(form_semantic_field(c,i)){
          char *actual=NULL,hash[65];size_t length=0;s=form_value(c,i,&c->values[i],&actual,&length);
          if(!s)s=ui_cap_hash_bytes(actual,length,hash);free(actual);
          if(!s&&strcmp(hash,f->snapshot_hash[i]))s=UI_STATUS_VALIDATION_FAILED;}
        if(!s)for(i=0;i<c->desc.field_count;++i)if(form_semantic_field(c,i)&&f->generation[i]==f->submitted[i]){
          c->drafts[i].flags&=~UI_VALUE_MODIFIED;f->conflict[i]=0;}
        if(!s&&!form_dirty(c))snprintf(f->base_revision,sizeof(f->base_revision),"%s",f->model_revision);
      }
      if(s){form_mark_conflict(c);n.status=s;snprintf(n.code,sizeof(n.code),"business_readback_failed");}
      f->pending=0;c->dirty=form_dirty(c)!=0;render_component(c);
    }
    if(notice->stage==UI_CAP_UI_REJECTED&&form_conflicts(c)){snprintf(n.code,sizeof(n.code),"draft_conflict");}
    if(f->desc.observe)f->desc.observe(&n,f->desc.user_data);
}
static void form_free(void *data)
{
    component_form_t *f=data;ui_component_t *c=f->component;c->form=NULL;
    ui_capabilities_form_release(c->host,f->bytes);free((void *)f->desc.component_id);free((void *)f->desc.capability_id);free(f);
}
ui_status_t ui_capability_form_bind(ui_host_t *host,const ui_cap_form_binding_desc_t *d,uint64_t *id)
{
    ui_component_t *c;component_form_t *f;ui_cap_ui_binding_desc_t binding={0};ui_cap_resource_ref_v2_t actual={0};ui_status_t s;size_t bytes;
    if(id)*id=0;if(!host||!ui_language_thread_ok(host)||!id||!d||d->size<sizeof(*d)||d->version!=1||!d->read_model||
      !ui_cap_text(d->component_id,63,0)||!ui_cap_text(d->capability_id,255,0))return UI_STATUS_INVALID_ARGUMENT;
    if(host->api_version<10)return UI_STATUS_UNSUPPORTED;
    if(d->resource.size<sizeof(d->resource)||d->resource.version!=2||
      !memchr(d->resource.resource_id,0,sizeof(d->resource.resource_id))||!memchr(d->resource.host_epoch,0,sizeof(d->resource.host_epoch))||
      !memchr(d->resource.open_epoch,0,sizeof(d->resource.open_epoch)))return UI_STATUS_INVALID_ARGUMENT;
    if(!ui_capabilities_nonread(host,d->capability_id))return UI_STATUS_NOT_FOUND;
    actual.size=sizeof(actual);actual.version=2;s=ui_capability_resource_ref_v2(host,d->resource.resource_id,&actual);if(s)return s;
    if(actual.instance_id!=d->resource.instance_id||actual.kind!=d->resource.kind||strcmp(actual.host_epoch,d->resource.host_epoch)||strcmp(actual.open_epoch,d->resource.open_epoch))return UI_STATUS_NOT_FOUND;
    c=ui_component_find(host,d->component_id);if(!c)return UI_STATUS_NOT_FOUND;
    if(c->desc.kind!=UI_COMPONENT_FORM&&c->desc.kind!=UI_COMPONENT_DIALOG)return UI_STATUS_UNSUPPORTED;
    if(c->desc.web_backend)return UI_STATUS_UNSUPPORTED;if(c->form||ui_inputs_component_bound(host,c))return UI_STATUS_ALREADY_EXISTS;
    bytes=sizeof(*f)+3u*(UI_CAPABILITY_INPUT_BYTES+1u)+strlen(d->component_id)+strlen(d->capability_id)+2;
    s=ui_capabilities_form_reserve(host,bytes);if(s)return s;f=calloc(1,sizeof(*f));
    if(!f){ui_capabilities_form_release(host,bytes);return UI_STATUS_OUT_OF_MEMORY;}
    f->bytes=bytes;f->component=c;f->desc=*d;f->desc.component_id=ui_strdup(d->component_id);f->desc.capability_id=ui_strdup(d->capability_id);
    if(!f->desc.component_id||!f->desc.capability_id){form_free(f);return UI_STATUS_OUT_OF_MEMORY;}
    c->form=f;snprintf(f->endpoint,sizeof(f->endpoint),"@form:%s",d->component_id);
    ++host->capability_ui_callbacks;++host->assistant_callbacks;ui_dispatch_enter(host);s=form_read(c);
    if(!s){binding.size=sizeof(binding);binding.version=1;binding.capability_id=d->capability_id;binding.resource=d->resource;
      binding.menu_item_id=f->endpoint;binding.purpose=UI_CAP_UI_INVOKE;binding.collect=form_collect;binding.observe=form_observe;binding.user_data=f;
      s=ui_capabilities_bind_component(host,&binding,c,form_free,&f->binding);}
    if(s)form_free(f);else *id=f->binding;--host->capability_ui_callbacks;--host->assistant_callbacks;ui_dispatch_leave(host);return s;
}
ui_status_t ui_capability_form_get_field_state(ui_component_t *c,const char *id,ui_cap_form_field_state_t *out)
{
    size_t i;if(!c||!ui_language_thread_ok(c->host)||!id||!out||out->size<sizeof(*out)||out->version!=1)return UI_STATUS_INVALID_ARGUMENT;
    if(!c->form)return UI_STATUS_NOT_FOUND;for(i=0;i<c->desc.field_count&&strcmp(c->fields[i].id,id);++i){}if(i==c->desc.field_count)return UI_STATUS_NOT_FOUND;
    out->generation=c->form->generation[i];out->modified=(c->drafts[i].flags&UI_VALUE_MODIFIED)!=0;out->conflict=c->form->conflict[i];
    strcpy(out->base_revision,c->form->base_revision);strcpy(out->model_revision,c->form->model_revision);return UI_STATUS_OK;
}
ui_status_t ui_capability_form_get_state(ui_component_t *c,ui_cap_form_state_t *out)
{
    if(!c||!ui_language_thread_ok(c->host)||!out||out->size<sizeof(*out)||out->version!=1)return UI_STATUS_INVALID_ARGUMENT;
    if(!c->form)return UI_STATUS_NOT_FOUND;out->pending_call=c->form->pending;out->dirty_fields=(uint32_t)form_dirty(c);out->conflict_fields=(uint32_t)form_conflicts(c);
    strcpy(out->base_revision,c->form->base_revision);strcpy(out->model_revision,c->form->model_revision);return UI_STATUS_OK;
}
int ui_components_form_available(ui_component_t *c)
{return c&&c->form&&c->visible&&(!c->host->modal_component||c->host->modal_component==c);}
void ui_components_form_base(ui_component_t *c,char revision[128])
{if(c&&c->form)snprintf(revision,128,"%s",c->form->base_revision);}
static ui_status_t form_semantic(ui_web_view_t *view,const char *node,void *data)
{
    ui_component_t *c=data;unsigned key=0,modifiers=0;char field[80]={0};size_t i;(void)view;
    if(!c->form)return UI_STATUS_UNSUPPORTED;
    if(sscanf_s(node,"@key:%u:%u:%79s",&key,&modifiers,field,(unsigned)sizeof(field))==3){
      if(key==27)return ui_components_form_action(c,"cancel");
      if(key!=13)return UI_STATUS_UNSUPPORTED;
      for(i=0;i<c->desc.field_count;++i)if(!strncmp(field,"field-",6)&&!strcmp(c->fields[i].id,field+6)){
        if((c->fields[i].flags&UI_VALUE_MULTILINE)&&!(modifiers&UI_INPUT_MODIFIER_CONTROL))return UI_STATUS_UNSUPPORTED;
        return ui_components_form_action(c,"submit");}
      return UI_STATUS_UNSUPPORTED;
    }
    return ui_components_form_action(c,node);
}
ui_status_t ui_components_form_action(ui_component_t *c,const char *action)
{
    component_form_t *f;ui_status_t s;size_t i;uint64_t event=0;void *owner;ui_cell_t reset[64]={{0}};
    if(!c||!c->form)return UI_STATUS_UNSUPPORTED;f=c->form;owner=c->view?c->view->user_data:c;
    if(!ui_components_form_available(c))return UI_STATUS_PERMISSION_DENIED;
    if(!strcmp(action,"submit"))return ui_capabilities_ui_action(c->host,f->endpoint,owner);
    if(strcmp(action,"cancel")&&strcmp(action,"conflict-keep")&&strcmp(action,"conflict-reload"))return UI_STATUS_UNSUPPORTED;
    s=ui_capabilities_control_consume(c->host,owner,&event);if(s)return s;
    ++c->host->capability_ui_callbacks;++c->host->assistant_callbacks;ui_dispatch_enter(c->host);
    if(f->pending&&strcmp(action,"cancel")){s=UI_STATUS_PENDING;goto done;}
    if(f->pending)(void)ui_capability_cancel_v2(c->host,f->pending);
    s=form_read(c);if(s)goto done;
    if(!ui_capabilities_control_current(c->host,event)||!ui_components_form_available(c)){s=UI_STATUS_PERMISSION_DENIED;goto done;}
    if(strcmp(action,"conflict-keep")){
      for(i=0;i<c->desc.field_count;++i)if(form_semantic_field(c,i)){size_t bytes=0;
        if(!copy_cell(&reset[i],&c->values[i],&bytes)){s=UI_STATUS_OUT_OF_MEMORY;goto done;}}
      for(i=0;i<c->desc.field_count;++i)if(form_semantic_field(c,i)){
        free_cell(&c->drafts[i]);c->drafts[i]=reset[i];memset(&reset[i],0,sizeof(reset[i]));
        c->drafts[i].flags&=~UI_VALUE_MODIFIED;form_changed(c,i);}}

    for(i=0;i<c->desc.field_count;++i)f->conflict[i]=0;
    snprintf(f->base_revision,sizeof(f->base_revision),"%s",f->model_revision);c->dirty=form_dirty(c)!=0;
    if(!strcmp(action,"cancel")&&c->modal)(void)ui_component_close_dialog(c);render_component(c);
done:for(i=0;i<64;++i)free_cell(&reset[i]);--c->host->capability_ui_callbacks;--c->host->assistant_callbacks;ui_dispatch_leave(c->host);return s;
}
void ui_components_form_draft(ui_component_t *c,const char *field,const char *text)
{
    ui_json_t j={0};uj_add(&j,"{\"action\":\"draft\",\"field\":");uj_string(&j,field);uj_add(&j,",\"text\":");uj_string(&j,text);uj_add(&j,"}");
    if(!j.failed)component_message(c->view,j.data,c);free(j.data);
}
