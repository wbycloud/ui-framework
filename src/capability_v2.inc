/* Private continuation of capability.c: one registry, ID space and executor.
 * No production export accepts a claimed human source or a credential. */
static size_t work_bytes(void)
{return sizeof(ui_work_t)+UI_CAPABILITY_OUTPUT_BYTES+1u;}
static ui_status_t ensure_work(registry_t *r)
{
    if(r->work)return UI_STATUS_OK;
    if(work_bytes()>UI_CAPABILITY_CALL_BYTES-r->call_bytes)return UI_STATUS_LIMIT_EXCEEDED;
    r->work=calloc(1,sizeof(*r->work));if(!r->work)return UI_STATUS_OUT_OF_MEMORY;
    r->call_bytes+=work_bytes();return UI_STATUS_OK;
}
static char *metadata_v2(capability_t *c,int schemas)
{
    ui_json_t j={0};uj_add(&j,"{\"id\":");uj_string(&j,c->id);
    uj_add(&j,",\"name\":");uj_string(&j,c->name);uj_add(&j,",\"description\":");uj_string(&j,c->description);
    uj_add(&j,",\"category\":");uj_string(&j,c->category);
    uj_fmt(&j,",\"contract_version\":2,\"capability_version\":%u,\"target_kind\":%u,\"effect\":",c->desc.capability_version,(unsigned)c->desc.target_kind);
    uj_string(&j,c->v2->desc.effect);uj_fmt(&j,",\"cancellable\":%s",c->v2->desc.cancel?"true":"false");
    if(c->v2->edit)uj_fmt(&j,",\"edit_contract\":1,\"risk\":%u",(unsigned)c->v2->edit->risk);
    if(c->v2->execution_policy){uj_fmt(&j,",\"execution_policy\":%u,\"side_effect\":",c->v2->execution_policy);uj_string(&j,c->v2->side_effect);}
    if(schemas){uj_add(&j,",\"input_schema\":");uj_add(&j,c->input);uj_add(&j,",\"output_schema\":");uj_add(&j,c->output);
      uj_add(&j,",\"reconcile_schema\":");uj_add(&j,c->v2->reconcile_schema);}
    uj_add(&j,"}");if(j.failed){free(j.data);return NULL;}return j.data;
}
static void free_v2_call(call_t *c){free_edit(c);free(c->v2);}
static int unknown_cap(registry_t *r,capability_t *cap)
{(void)r;return cap->unknown_references!=0;}
static void release_unknown(call_t *c)
{
    if(!c->v2->unknown_held)return;
    if(c->capability)--c->capability->unknown_references;
    if(c->resource)--c->resource->unknown_references;
    c->v2->unknown_held=0;
}
static void detach_unknown(registry_t *r,capability_t *cap,resource_t *res)
{
    call_t *c;for(c=r->calls;c;c=c->next)if(c->v2&&c->v2->unknown_held){
      if(cap&&c->capability==cap){--cap->unknown_references;c->capability=NULL;}
      if(res&&c->resource==res){--res->unknown_references;c->resource=NULL;}}
}
static void destroy_v2(registry_t *r)
{
    ui_binding_t *b;while((b=r->bindings)!=NULL){r->bindings=b->next;if(b->cleanup)b->cleanup(b->desc.user_data);free((void *)b->desc.capability_id);free((void *)b->desc.menu_item_id);free(b);}
    free(r->work);r->work=NULL;
}
int ui_capabilities_nonread(const ui_host_t *host,const char *id)
{capability_t *c=host&&host->capabilities&&id?find_cap(host->capabilities,id):NULL;return c&&c->v2;}
static ui_binding_t *find_binding(registry_t *r,uint64_t id)
{ui_binding_t *b;for(b=r->bindings;b;b=b->next)if(b->id==id)return b;return NULL;}
static void observe_v2(ui_host_t *host,registry_t *r,ui_binding_t *b,uint64_t id,
    ui_cap_ui_stage_t stage,ui_status_t status,const char *code,const char *evidence)
{
    ui_cap_ui_notice_t n={0};if(!b||b->retiring||!b->desc.observe||r->ui_notifying)return;
    n.size=sizeof(n);n.version=1;n.binding_id=b->id;n.call_id=id;n.stage=stage;n.status=status;
    snprintf(n.code,sizeof(n.code),"%s",code);n.evidence_json=evidence;
    ++b->leases;++r->ui_notifying;++host->capability_ui_callbacks;++host->assistant_callbacks;ui_dispatch_enter(host);
    b->desc.observe(&n,b->desc.user_data);
    --host->capability_ui_callbacks;--host->assistant_callbacks;--r->ui_notifying;--b->leases;ui_dispatch_leave(host);
}
static void release_gate_refs(call_t *c)
{
    if(c->capability)--c->capability->references;
    if(c->resource)--c->resource->references;
}
static void deliver_v2(ui_host_t *host,registry_t *r)
{
    call_t *c;if(r->callbacks||r->ui_notifying||r->delivering)return;
    r->delivering=1;
    for(;;){
      for(c=r->calls;c&&!(c->v2&&c->v2->notify_pending);c=c->next){}
      if(!c)break;c->v2->notify_pending=0;++c->v2->leases;
      observe_v2(host,r,find_binding(r,c->v2->binding_id),c->id,UI_CAP_UI_FINISHED,UI_STATUS_OK,c->v2->result.code,NULL);
      --c->v2->leases;}
    r->delivering=0;
}
static void finish_v2(ui_host_t *host,registry_t *r,call_t *c,ui_cap_outcome_v2_t outcome,
    ui_cap_effect_v2_t effect,const char *code)
{
    call_v2_t *v=c->v2;v->result.state=UI_CAP_FINISHED;v->result.outcome=outcome;v->result.effect=effect;
    c->result.state=UI_CAP_FINISHED;snprintf(v->result.code,sizeof(v->result.code),"%s",code);
    if(outcome==UI_CAP2_UNKNOWN){v->unknown_held=1;++c->capability->unknown_references;++c->resource->unknown_references;}
    v->notify_pending=1;++v->leases;deliver_v2(host,r);--v->leases;
    release_gate_refs(c);
    if(!v->unknown_held){c->capability=NULL;c->resource=NULL;}
}
static int ref_valid(const ui_cap_resource_ref_v2_t *t)
{
    return t&&t->size>=sizeof(*t)&&t->version==2&&kind_ok(t->kind)&&
      memchr(t->host_epoch,0,sizeof(t->host_epoch))&&memchr(t->resource_id,0,sizeof(t->resource_id))&&
      memchr(t->open_epoch,0,sizeof(t->open_epoch))&&uuid_ok(t->host_epoch)&&uuid_ok(t->open_epoch)&&ui_cap_text(t->resource_id,255,0);
}
static int ref_matches(registry_t *r,resource_t *res,const ui_cap_resource_ref_v2_t *t)
{return res&&t->instance_id==r->instance&&t->kind==res->desc.kind&&!strcmp(t->host_epoch,r->epoch)&&!strcmp(t->open_epoch,res->epoch);}
ui_status_t ui_capability_register_v2(ui_host_t *host,const ui_capability_desc_v2_t *d)
{
    ui_capability_desc_t n={0};registry_t *r;ui_status_t s=registry(host,&r);if(s)return s;
    if(!d||d->size<sizeof(*d)||d->version!=2||!d->execute||!d->reconcile)return UI_STATUS_INVALID_ARGUMENT;
    if(!d->effect||(strcmp(d->effect,"view")&&strcmp(d->effect,"draft")&&strcmp(d->effect,"write")))return UI_STATUS_UNSUPPORTED;
    if(!host->capability_read_entries)return UI_STATUS_PERMISSION_DENIED;
    s=ensure_work(r);if(s)return s;
    n.size=sizeof(n);n.version=1;n.capability_version=d->capability_version;n.id=d->id;n.name_utf8=d->name_utf8;
    n.description_utf8=d->description_utf8;n.category=d->category;n.input_schema_json=d->input_schema_json;
    n.output_schema_json=d->output_schema_json;n.target_kind=d->target_kind;n.effect=d->effect;
    return register_common(host,&n,d,NULL,NULL);
}
ui_status_t ui_capability_resource_ref_v2(ui_host_t *host,const char *id,ui_cap_resource_ref_v2_t *out)
{
    registry_t *r;resource_t *res;ui_status_t s=registry(host,&r);if(s)return s;
    if(!out||out->size<sizeof(*out)||out->version!=2||!ui_cap_text(id,255,0))return UI_STATUS_INVALID_ARGUMENT;
    res=find_resource(r,id);if(!res||res->retiring)return UI_STATUS_NOT_FOUND;if(is_closing(r))return UI_STATUS_CANCELLED;
    memset(out,0,sizeof(*out));out->size=sizeof(*out);out->version=2;out->instance_id=r->instance;out->kind=res->desc.kind;
    strcpy(out->host_epoch,r->epoch);strcpy(out->resource_id,res->id);strcpy(out->open_epoch,res->epoch);return UI_STATUS_OK;
}
ui_status_t ui_capability_describe_v2(ui_host_t *host,const char *id,char **json)
{
    registry_t *r;capability_t *c;ui_status_t s=registry(host,&r);ui_json_t j={0};size_t length;
    if(s)return s;if(!json||!ui_cap_text(id,255,0))return UI_STATUS_INVALID_ARGUMENT;*json=NULL;
    c=find_cap(r,id);if(!c||c->retiring||is_closing(r))return UI_STATUS_NOT_FOUND;
    uj_fmt(&j,"{\"descriptor_hash\":\"%s\",",c->hash);uj_add(&j,c->metadata+1);
    if(j.failed){free(j.data);return UI_STATUS_OUT_OF_MEMORY;}
    s=ui_cap_json_canonicalize(j.data,strlen(j.data),json,&length);free(j.data);return s;
}
ui_status_t ui_capability_discover_v2(ui_host_t *host,const char *category,size_t offset,size_t limit,uint64_t *generation,char **json)
{
    registry_t *r;capability_t *c;ui_json_t j={0};size_t seen=0,n=0,next=0;ui_status_t s=registry(host,&r);
    if(s)return s;if(!generation||!json||!limit||limit>32||(category&&!ui_cap_text(category,127,1)))return UI_STATUS_INVALID_ARGUMENT;
    *json=NULL;if(*generation&&*generation!=r->generation)return UI_STATUS_CANCELLED;
    uj_fmt(&j,"{\"generation\":\"%llu\",\"items\":[",(unsigned long long)r->generation);
    for(c=r->capabilities;c&&!is_closing(r);c=c->next){char *m;if(c->retiring||(category&&*category&&strcmp(category,c->category)))continue;
      if(seen++<offset)continue;if(n==limit){next=offset+n;break;}m=metadata(c,0);if(!m){free(j.data);return UI_STATUS_OUT_OF_MEMORY;}
      if(n)uj_add(&j,",");uj_fmt(&j,"{\"descriptor_hash\":\"%s\",",c->hash);uj_add(&j,m+1);free(m);++n;}
    uj_fmt(&j,"],\"next\":%zu}",next);if(j.failed){free(j.data);return UI_STATUS_OUT_OF_MEMORY;}
    if(strlen(j.data)>UI_CAPABILITY_OUTPUT_BYTES){free(j.data);return UI_STATUS_LIMIT_EXCEEDED;}*json=j.data;*generation=r->generation;return UI_STATUS_OK;
}
static ui_status_t bind_ui_common(ui_host_t *host,const ui_cap_ui_binding_desc_t *d,uint64_t *id,ui_component_t *component,void (*cleanup)(void *),int independent)
{
    registry_t *r;capability_t *cap;resource_t *res;ui_binding_t *b;ui_menu_entry_t *m;size_t bytes;ui_status_t s=registry(host,&r);
    if(s)return s;if(id)*id=0;if(!id||!d||d->size<sizeof(*d)||d->version!=1||!ref_valid(&d->resource)||
      !ui_cap_text(d->capability_id,255,0)||!ui_cap_text(d->menu_item_id,255,0))return UI_STATUS_INVALID_ARGUMENT;
    if(d->purpose!=UI_CAP_UI_INVOKE&&d->purpose!=UI_CAP_UI_RECONCILE)return UI_STATUS_INVALID_ARGUMENT;
    if((d->purpose==UI_CAP_UI_INVOKE&&d->reconcile_call_id)||(d->purpose==UI_CAP_UI_RECONCILE&&(!d->reconcile_call_id||d->collect)))return UI_STATUS_INVALID_ARGUMENT;
    if(is_closing(r)||host->dispatch_blocked)return UI_STATUS_CANCELLED;
    cap=find_cap(r,d->capability_id);res=find_resource(r,d->resource.resource_id);
    if(!cap||!cap->v2||!ref_matches(r,res,&d->resource)||cap->desc.target_kind!=d->resource.kind)return UI_STATUS_NOT_FOUND;
    if(d->purpose==UI_CAP_UI_INVOKE&&(cap->retiring||res->retiring))return UI_STATUS_CANCELLED;
    if(d->purpose==UI_CAP_UI_RECONCILE){call_t *c=find_call(r,d->reconcile_call_id);
      if(!c||!c->v2||!c->v2->unknown_held||c->capability!=cap||c->resource!=res)return UI_STATUS_NOT_FOUND;}
    for(m=host->menus;m&&strcmp(m->id,d->menu_item_id);m=m->next){}if(!component&&!independent&&(!m||strcmp(m->command_id,d->capability_id)))return UI_STATUS_UNSUPPORTED;
    for(b=r->bindings;b;b=b->next)if(!strcmp(b->desc.menu_item_id,d->menu_item_id))return UI_STATUS_ALREADY_EXISTS;
    bytes=sizeof(*b)+strlen(d->capability_id)+strlen(d->menu_item_id)+2;
    if(bytes>UI_CAPABILITY_CATALOG_BYTES-r->catalog_bytes||r->binding_sequence==UINT64_MAX)return UI_STATUS_LIMIT_EXCEEDED;
    b=calloc(1,sizeof(*b));if(!b)return UI_STATUS_OUT_OF_MEMORY;b->desc=*d;
    b->desc.capability_id=ui_strdup(d->capability_id);b->desc.menu_item_id=ui_strdup(d->menu_item_id);
    if(!b->desc.capability_id||!b->desc.menu_item_id){free((void *)b->desc.capability_id);free((void *)b->desc.menu_item_id);free(b);return UI_STATUS_OUT_OF_MEMORY;}
    b->cap_incarnation=cap->incarnation;b->component=component;b->cleanup=cleanup;b->bytes=bytes;b->id=++r->binding_sequence;b->next=r->bindings;r->bindings=b;r->catalog_bytes+=bytes;++r->generation;*id=b->id;return UI_STATUS_OK;
}
ui_status_t ui_capability_ui_bind(ui_host_t *host,const ui_cap_ui_binding_desc_t *d,uint64_t *id)
{return bind_ui_common(host,d,id,NULL,NULL,0);}
ui_status_t ui_capabilities_bind_component(ui_host_t *host,const ui_cap_ui_binding_desc_t *d,ui_component_t *component,void (*cleanup)(void *),uint64_t *id)
{return bind_ui_common(host,d,id,component,cleanup,0);}
ui_status_t ui_capability_ui_begin_unbind(ui_host_t *host,uint64_t id)
{
    registry_t *r;ui_binding_t *b;ui_status_t s=registry(host,&r);if(s)return s;b=find_binding(r,id);if(!b)return UI_STATUS_NOT_FOUND;
    if(!b->retiring){b->retiring=1;++r->generation;}return UI_STATUS_OK;
}
ui_status_t ui_capability_ui_finish_unbind(ui_host_t *host,uint64_t id)
{
    registry_t *r;ui_binding_t **p,*b;ui_status_t s=registry(host,&r);if(s)return s;
    for(p=&r->bindings;*p&&(*p)->id!=id;p=&(*p)->next){}if(!*p)return UI_STATUS_NOT_FOUND;b=*p;
    if(!b->retiring)return UI_STATUS_INVALID_ARGUMENT;if(b->leases||r->callbacks||host->assistant_callbacks)return UI_STATUS_PENDING;
    *p=b->next;r->catalog_bytes-=b->bytes;if(b->cleanup)b->cleanup(b->desc.user_data);free((void *)b->desc.capability_id);free((void *)b->desc.menu_item_id);free(b);return UI_STATUS_OK;
}
static int resource_busy(registry_t *r,resource_t *res)
{
    call_t *c;for(c=r->calls;c;c=c->next)if(c->v2&&c->resource==res&&
      (c->result.state!=UI_CAP_FINISHED||c->v2->leases||c->v2->unknown_held))return 1;return 0;
}
static int binding_available(ui_host_t *host,ui_binding_t *b)
{
    ui_menu_entry_t *m;if(b->input)return input_available(host,b->input);if(b->component)return ui_components_form_available(b->component);
    for(m=host->menus;m;m=m->next)if(!strcmp(m->id,b->desc.menu_item_id))
      return !strcmp(m->command_id,b->desc.capability_id)&&m->state.enabled&&m->state.visible&&!m->state.busy;
    return 0;
}
static ui_status_t gate_v2(ui_host_t *host,registry_t *r,call_t *c,ui_binding_t *b,uint64_t event,int (*valid)(void *),void *valid_data)
{
    char rev[128];ui_status_t s;
    if(is_closing(r)||host->dispatch_blocked||b->retiring||c->capability->retiring||c->resource->retiring)return UI_STATUS_CANCELLED;
    if(host->modal_component!=c->v2->modal_owner||(host->modal_component&&host->modal_component!=b->component)||!host->app_active||!binding_available(host,b)||c->generation!=r->generation)return UI_STATUS_PERMISSION_DENIED;
    if(c->capability->entry&&(!c->capability->entry->state.enabled||!c->capability->entry->state.visible||c->capability->entry->state.busy))return UI_STATUS_PERMISSION_DENIED;
    s=revision(host,r,c->resource,rev);if(s)return s;if(strcmp(rev,c->v2->target.expected_revision))return UI_STATUS_VALIDATION_FAILED;
    if(c->result.state!=UI_CAP_PREPARED)return UI_STATUS_CANCELLED;
    if(c->capability->v2->desc.check){enter(host,r);s=c->capability->v2->desc.check(&c->v2->target,c->params,c->capability->v2->desc.user_data);leave(host,r);if(s)return s;}
    if(c->result.state!=UI_CAP_PREPARED)return UI_STATUS_CANCELLED;
    s=revision(host,r,c->resource,rev);if(s)return s;if(c->result.state!=UI_CAP_PREPARED)return UI_STATUS_CANCELLED;if(strcmp(rev,c->v2->target.expected_revision))return UI_STATUS_VALIDATION_FAILED;
    if(is_closing(r)||host->dispatch_blocked||b->retiring||c->resource->retiring||c->capability->retiring||c->generation!=r->generation)return UI_STATUS_CANCELLED;
    if(host->modal_component!=c->v2->modal_owner||(host->modal_component&&host->modal_component!=b->component)||!host->app_active||!binding_available(host,b))return UI_STATUS_PERMISSION_DENIED;
    if(!c->capability->entry->state.enabled||!c->capability->entry->state.visible||c->capability->entry->state.busy)return UI_STATUS_PERMISSION_DENIED;
    if(event!=r->human_sequence||(valid&&!valid(valid_data)))return UI_STATUS_PERMISSION_DENIED;
    return UI_STATUS_OK;
}
static ui_status_t reconcile_v2(ui_host_t *host,registry_t *r,ui_binding_t *b,uint64_t event)
{
    call_t *c=find_call(r,b->desc.reconcile_call_id);ui_cap_reconciliation_v2_t proof={0};
    char before[128],after[128],*canonical=NULL;size_t length=0,canonical_length=0;ui_status_t s;
    int (*valid)(void *)=r->human_valid;void *valid_data=r->human_valid_data;
    if(!c||!c->v2||!c->v2->unknown_held)return UI_STATUS_NOT_FOUND;
    if(c->edit)return UI_STATUS_PERMISSION_DENIED;
    if(c->v2->leases||r->callbacks)return UI_STATUS_PENDING;
    proof.size=sizeof(proof);proof.version=2;++c->v2->leases;
    s=revision(host,r,c->resource,before);if(s)goto done;
    if(event!=r->human_sequence||!r->human_active||(r->human_valid&&!r->human_valid(r->human_valid_data))||is_closing(r)||host->modal_component||b->retiring){s=UI_STATUS_PERMISSION_DENIED;goto done;}
    r->human_active=0;memset(r->work->evidence,0,sizeof(r->work->evidence));enter(host,r);
    s=c->capability->v2->desc.reconcile(c->id,&c->v2->target,c->params,&proof,r->work->evidence,sizeof(r->work->evidence),&length,c->capability->v2->desc.user_data);leave(host,r);
    if(s)goto done;s=revision(host,r,c->resource,after);if(s)goto done;
    if(is_closing(r)||b->retiring||host->dispatch_blocked||host->modal_component||event!=r->human_sequence||(valid&&!valid(valid_data))||!binding_available(host,b)||strcmp(before,after)||
      proof.size<sizeof(proof)||proof.version!=2||!memchr(proof.observed_revision,0,sizeof(proof.observed_revision))||strcmp(after,proof.observed_revision)){
      s=UI_STATUS_VALIDATION_FAILED;goto done;}
    if(proof.resolution!=UI_CAP2_RESOLVED_APPLIED&&proof.resolution!=UI_CAP2_RESOLVED_NOT_APPLIED){s=UI_STATUS_PENDING;goto done;}
    if(length>UI_CAPABILITY_OUTPUT_BYTES||r->work->evidence[length]){s=UI_STATUS_VALIDATION_FAILED;goto done;}
    s=ui_cap_json_validate(c->capability->v2->reconcile_schema,r->work->evidence,length);if(s)goto done;
    s=ui_cap_json_canonicalize(r->work->evidence,length,&canonical,&canonical_length);if(s)goto done;
    s=ui_cap_hash_bytes(canonical,canonical_length,c->v2->result.evidence_sha256);if(s)goto done;
    c->v2->result.resolution=proof.resolution;strcpy(c->v2->result.reconciled_revision,after);
    observe_v2(host,r,b,c->id,UI_CAP_UI_RECONCILED,UI_STATUS_OK,"reconciled",canonical);
    release_unknown(c);c->capability=NULL;c->resource=NULL;
done:free(canonical);--c->v2->leases;return s;
}
void ui_capabilities_human_begin(ui_host_t *host,void *owner)
{
    registry_t *r=host?host->capabilities:NULL;if(!r)return;
    ++r->human_sequence;r->human_owner=owner;r->human_active=1;r->human_valid=NULL;r->human_valid_data=NULL;
}
void ui_capabilities_human_end(ui_host_t *host)
{if(host&&host->capabilities){host->capabilities->human_active=0;host->capabilities->human_owner=NULL;host->capabilities->human_valid=NULL;host->capabilities->human_valid_data=NULL;}}
ui_status_t ui_capabilities_ui_action(ui_host_t *host,const char *item,void *owner)
{
    registry_t *r;ui_binding_t *b;capability_t *cap;resource_t *res;call_t *c=NULL;ui_cap_ui_context_t context={0};
    ui_status_t s=registry(host,&r);size_t length=0,canonical_length=0,bytes;char *canonical=NULL;uint64_t event,generation;const char *rejection=NULL;ui_component_t *modal_owner=host?host->modal_component:NULL;
    int (*valid)(void *)=s?NULL:r->human_valid;void *valid_data=s?NULL:r->human_valid_data;
    if(s)return s;for(b=r->bindings;b;b=b->next)if(!strcmp(b->desc.menu_item_id,item))break;
    if(!b)return UI_STATUS_UNSUPPORTED;
    if(r->ui_notifying||r->failure_notifying||r->work->used)return UI_STATUS_LIMIT_EXCEEDED;
    ++b->leases;++host->capability_ui_callbacks;++host->assistant_callbacks;ui_dispatch_enter(host);r->work->used=1;event=r->human_sequence;generation=r->generation;
    if(!r->human_active||r->human_owner!=owner||(r->human_valid&&!r->human_valid(r->human_valid_data))){s=UI_STATUS_PERMISSION_DENIED;goto rejected;}
    if(b->retiring||is_closing(r)||host->dispatch_blocked){s=UI_STATUS_CANCELLED;goto rejected;}
    cap=find_cap(r,b->desc.capability_id);res=find_resource(r,b->desc.resource.resource_id);
    if(!cap||!cap->v2||cap->incarnation!=b->cap_incarnation||!ref_matches(r,res,&b->desc.resource)){s=UI_STATUS_NOT_FOUND;goto rejected;}
    if(b->desc.purpose==UI_CAP_UI_RECONCILE){s=reconcile_v2(host,r,b,event);if(s)goto rejected;goto done;}
    if(cap->retiring||res->retiring){s=UI_STATUS_CANCELLED;goto rejected;}
    if(cap->v2->edit&&cap->v2->edit->risk!=UI_EDIT_REVERSIBLE&&!cap->v2->execution_policy){s=UI_STATUS_PERMISSION_DENIED;goto rejected;}
    if(resource_busy(r,res)){s=UI_STATUS_PERMISSION_DENIED;rejection=res->unknown_references?"result_unknown":"busy";goto rejected;}
    context.size=sizeof(context);context.version=1;context.binding_id=b->id;context.target.size=sizeof(context.target);context.target.version=2;
    context.target.resource=b->desc.resource;
    snprintf(context.target.scope_json,sizeof(context.target.scope_json),"{\"access\":\"%s\"}",cap->v2->desc.effect);
    s=revision(host,r,res,context.target.expected_revision);if(s)goto rejected;
    if(b->component)ui_components_form_base(b->component,context.target.expected_revision);
    if(b->input){s=input_expected_revision(b->input,context.target.expected_revision);if(s)goto rejected;}
    memset(r->work->params,0,sizeof(r->work->params));
    if(b->desc.collect)s=b->desc.collect(&context,r->work->params,sizeof(r->work->params),&length,b->desc.user_data);
    else{strcpy(r->work->params,"{}");length=2;s=UI_STATUS_OK;}
    if(s)goto rejected;
    if(length>UI_CAPABILITY_INPUT_BYTES||r->work->params[length]){s=UI_STATUS_VALIDATION_FAILED;goto rejected;}
    s=ui_cap_json_validate(cap->input,r->work->params,length);if(s)goto rejected;
    s=ui_cap_json_canonicalize(r->work->params,length,&canonical,&canonical_length);if(s)goto rejected;
    if(event!=r->human_sequence||!r->human_active||r->human_owner!=owner||(r->human_valid&&!r->human_valid(r->human_valid_data))||generation!=r->generation||b->retiring||cap->retiring||res->retiring||is_closing(r)){
      s=UI_STATUS_PERMISSION_DENIED;goto rejected;}
    bytes=sizeof(*c)+sizeof(call_v2_t)+canonical_length+1;
    if(r->call_count==UI_CAPABILITY_MAX_CALLS||bytes>UI_CAPABILITY_CALL_BYTES-r->call_bytes||r->sequence==UINT64_MAX){s=UI_STATUS_LIMIT_EXCEEDED;goto rejected;}
    c=calloc(1,sizeof(*c));if(!c){s=UI_STATUS_OUT_OF_MEMORY;goto rejected;}c->v2=calloc(1,sizeof(*c->v2));
    if(!c->v2){free(c);c=NULL;s=UI_STATUS_OUT_OF_MEMORY;goto rejected;}
    c->id=++r->sequence;c->generation=generation;c->params=canonical;canonical=NULL;c->bytes=bytes;c->capability=cap;c->resource=res;
    c->result.state=UI_CAP_PREPARED;c->v2->modal_owner=modal_owner;c->v2->target=context.target;c->v2->binding_id=b->id;
    (void)ui_cap_hash_bytes(c->params,canonical_length,c->v2->params_hash);
    c->v2->result.size=sizeof(c->v2->result);c->v2->result.version=2;c->v2->result.state=UI_CAP_PREPARED;
    strcpy(c->v2->result.revision_before,context.target.expected_revision);++cap->references;++res->references;
    c->next=r->calls;r->calls=c;r->call_bytes+=bytes;++r->call_count;r->human_active=0;
    ++c->v2->leases;observe_v2(host,r,b,c->id,UI_CAP_UI_ADMITTED,UI_STATUS_OK,"admitted",NULL);
    s=c->result.state==UI_CAP_PREPARED?gate_v2(host,r,c,b,event,valid,valid_data):UI_STATUS_OK;
    if(c->result.state==UI_CAP_PREPARED){if(s){const char *code=s==UI_STATUS_VALIDATION_FAILED?"revision_conflict":"unavailable";
        if(b->retiring)code="binding_expired";else if(c->generation!=r->generation)code="catalog_changed";
        else if(is_closing(r)||host->dispatch_blocked)code="closing";
        else if(event!=r->human_sequence||(valid&&!valid(valid_data)))code="credential_invalid";
        finish_v2(host,r,c,s==UI_STATUS_VALIDATION_FAILED?UI_CAP2_STALE:UI_CAP2_REJECTED,UI_CAP2_EFFECT_NONE,code);}
      else{c->result.state=UI_CAP_RUNNING;c->v2->result.state=UI_CAP_RUNNING;enter(host,r);cap->v2->desc.execute(c->id,&c->v2->target,c->params,cap->v2->desc.user_data);leave(host,r);}}
    deliver_v2(host,r);--c->v2->leases;s=UI_STATUS_OK;goto done;
rejected:observe_v2(host,r,b,0,UI_CAP_UI_REJECTED,s,
    rejection?rejection:s==UI_STATUS_LIMIT_EXCEEDED?"budget_exceeded":s==UI_STATUS_VALIDATION_FAILED?"invalid_params":s==UI_STATUS_NOT_FOUND?"target_expired":s==UI_STATUS_CANCELLED?"closing":"human_source_required",NULL);
done:r->human_active=0;free(canonical);deliver_v2(host,r);r->work->used=0;--b->leases;--host->capability_ui_callbacks;--host->assistant_callbacks;ui_dispatch_leave(host);return s;
}
ui_status_t ui_capability_complete_v2(ui_host_t *host,uint64_t id,const ui_cap_completion_v2_t *in)
{
    registry_t *r;call_t *c;ui_status_t s=registry(host,&r);char rev[128],*canonical=NULL;size_t length=0,n=0;const char *code="application_failure";
    ui_cap_outcome_v2_t outcome;ui_cap_effect_v2_t effect;
    if(s)return s;c=find_call(r,id);if(!c)return UI_STATUS_NOT_FOUND;if(!c->v2)return UI_STATUS_UNSUPPORTED;
    if(c->edit)return UI_STATUS_UNSUPPORTED;
    if(c->result.state==UI_CAP_FINISHED)return UI_STATUS_ALREADY_EXISTS;if(c->completing)return UI_STATUS_PENDING;
    if(c->result.state!=UI_CAP_RUNNING||!in||in->size<sizeof(*in)||in->version!=2||in->outcome<UI_CAP2_OK||in->outcome>UI_CAP2_UNKNOWN||
      in->effect<UI_CAP2_EFFECT_NONE||in->effect>UI_CAP2_EFFECT_UNKNOWN||(in->outcome!=UI_CAP2_OK&&in->output_json))return UI_STATUS_INVALID_ARGUMENT;
    ++c->v2->leases;c->completing=1;outcome=in->outcome;effect=in->effect;
    if(outcome==UI_CAP2_OK){n=bounded_length(in->output_json,UI_CAPABILITY_OUTPUT_BYTES);
      s=n>UI_CAPABILITY_OUTPUT_BYTES?UI_STATUS_LIMIT_EXCEEDED:ui_cap_json_validate(c->capability->output,in->output_json,n);
      if(!s)s=ui_cap_json_canonicalize(in->output_json,n,&canonical,&length);
      if(s==UI_STATUS_OUT_OF_MEMORY)goto retry;if(s){outcome=UI_CAP2_UNKNOWN;effect=UI_CAP2_EFFECT_UNKNOWN;code="invalid_output";}}
    s=revision(host,r,c->resource,rev);
    if(s){outcome=UI_CAP2_UNKNOWN;effect=UI_CAP2_EFFECT_UNKNOWN;code="revision_unavailable";}
    else{strcpy(c->v2->result.revision_after,rev);
      if((effect==UI_CAP2_EFFECT_NONE&&strcmp(rev,c->v2->target.expected_revision))||
        (effect==UI_CAP2_EFFECT_APPLIED&&!strcmp(rev,c->v2->target.expected_revision))){outcome=UI_CAP2_UNKNOWN;effect=UI_CAP2_EFFECT_UNKNOWN;code="effect_revision_conflict";}}
    if(outcome==UI_CAP2_UNKNOWN||effect==UI_CAP2_EFFECT_UNKNOWN){outcome=UI_CAP2_UNKNOWN;effect=UI_CAP2_EFFECT_UNKNOWN;}
    if(outcome==UI_CAP2_OK){if(length+1>UI_CAPABILITY_CALL_BYTES-r->call_bytes){s=UI_STATUS_LIMIT_EXCEEDED;goto retry;}
      c->output=canonical;canonical=NULL;c->v2->result.output_json=c->output;c->bytes+=length+1;r->call_bytes+=length+1;code="ok";}
    c->completing=0;finish_v2(host,r,c,outcome,effect,code);--c->v2->leases;free(canonical);return UI_STATUS_OK;
retry:c->completing=0;--c->v2->leases;free(canonical);return s;
}
ui_status_t ui_capability_get_result_v2(ui_host_t *host,uint64_t id,ui_cap_result_v2_t *out)
{
    registry_t *r;call_t *c;ui_status_t s=registry(host,&r);if(s)return s;
    if(!out||out->size<sizeof(*out)||out->version!=2)return UI_STATUS_INVALID_ARGUMENT;
    c=find_call(r,id);if(!c)return UI_STATUS_NOT_FOUND;if(!c->v2)return UI_STATUS_UNSUPPORTED;if(c->edit)return UI_STATUS_UNSUPPORTED;*out=c->v2->result;return UI_STATUS_OK;
}
ui_status_t ui_capability_cancel_v2(ui_host_t *host,uint64_t id)
{
    registry_t *r;call_t *c;ui_status_t s=registry(host,&r);if(s)return s;
    c=find_call(r,id);if(!c)return UI_STATUS_NOT_FOUND;if(!c->v2)return UI_STATUS_UNSUPPORTED;
    if(c->result.state==UI_CAP_FINISHED||c->v2->result.cancel_requested)return UI_STATUS_OK;c->v2->result.cancel_requested=1;
    if(c->result.state==UI_CAP_PREPARED)finish_v2(host,r,c,UI_CAP2_CANCELLED,UI_CAP2_EFFECT_NONE,"cancelled_before_dispatch");
    else if(c->capability->v2->desc.cancel){++c->v2->leases;enter(host,r);c->capability->v2->desc.cancel(id,c->capability->v2->desc.user_data);leave(host,r);deliver_v2(host,r);--c->v2->leases;}
    return UI_STATUS_OK;
}
static ui_status_t cancel_any(ui_host_t *host,uint64_t id)
{call_t *c=find_call(host->capabilities,id);return c&&c->v2?ui_capability_cancel_v2(host,id):ui_capability_cancel(host,id);}
ui_status_t ui_capability_forget_v2(ui_host_t *host,uint64_t id)
{
    registry_t *r;call_t **p,*c;ui_status_t s=registry(host,&r);if(s)return s;
    for(p=&r->calls;*p&&(*p)->id!=id;p=&(*p)->next){}if(!*p)return UI_STATUS_NOT_FOUND;c=*p;
    if(!c->v2)return UI_STATUS_UNSUPPORTED;
    if(c->edit)return UI_STATUS_PERMISSION_DENIED;
    if(c->result.state!=UI_CAP_FINISHED||c->v2->leases||c->v2->unknown_held||r->callbacks||host->assistant_callbacks)return UI_STATUS_PENDING;
    *p=c->next;r->call_bytes-=c->bytes;--r->call_count;free(c->params);free(c->output);free(c->v2);free(c);return UI_STATUS_OK;
}
ui_status_t ui_web_semantic_action(ui_host_t *host,void *backend_view,const char *node,const ui_input_event_t *event)
{
    ui_web_view_t *v;for(v=host->web_views;v;v=v->host_next)if(v->user_data==backend_view&&v->semantic_action){
      v->private_input=*event;v->private_input.text_utf8=NULL;return v->semantic_action(v,node,v->semantic_data);}return UI_STATUS_UNSUPPORTED;
}

ui_status_t ui_capabilities_form_reserve(ui_host_t *host,size_t bytes)
{
    registry_t *r;ui_status_t s=registry(host,&r);if(s)return s;
    if(bytes>UI_CAPABILITY_CALL_BYTES-r->call_bytes)return UI_STATUS_LIMIT_EXCEEDED;r->call_bytes+=bytes;return UI_STATUS_OK;
}
void ui_capabilities_form_release(ui_host_t *host,size_t bytes){host->capabilities->call_bytes-=bytes;}
ui_status_t ui_capabilities_model_revision(ui_host_t *host,const ui_cap_resource_ref_v2_t *ref,const char *expected)
{
    registry_t *r;resource_t *res;char rev[128];ui_status_t s=registry(host,&r);if(s)return s;
    if(!ref_valid(ref)||!ui_cap_text(expected,127,0))return UI_STATUS_INVALID_ARGUMENT;
    res=find_resource(r,ref->resource_id);if(!ref_matches(r,res,ref)||res->retiring)return UI_STATUS_NOT_FOUND;
    if(is_closing(r))return UI_STATUS_CANCELLED;++res->references;s=revision(host,r,res,rev);
    if(!s&&(!ref_matches(r,res,ref)||res->retiring||is_closing(r)))s=UI_STATUS_CANCELLED;
    --res->references;if(s)return s;return strcmp(rev,expected)?UI_STATUS_VALIDATION_FAILED:UI_STATUS_OK;
}
ui_status_t ui_capabilities_control_consume(ui_host_t *host,void *owner,uint64_t *event)
{
    registry_t *r;ui_status_t s=registry(host,&r);if(s)return s;
    if(!r->human_active||r->human_owner!=owner)return UI_STATUS_PERMISSION_DENIED;
    if(is_closing(r)||host->dispatch_blocked||!host->app_active)return UI_STATUS_CANCELLED;
    if(r->ui_notifying||r->callbacks||(r->work&&r->work->used))return UI_STATUS_PENDING;
    *event=r->human_sequence;r->human_active=0;return UI_STATUS_OK;
}
int ui_capabilities_control_current(ui_host_t *host,uint64_t event)
{return host->capabilities->human_sequence==event&&!is_closing(host->capabilities)&&!host->dispatch_blocked&&(!host->capabilities->human_valid||host->capabilities->human_valid(host->capabilities->human_valid_data));}

void ui_capabilities_human_guard(ui_host_t *host,int (*valid)(void *),void *data)
{if(host&&host->capabilities){host->capabilities->human_valid=valid;host->capabilities->human_valid_data=data;}}
void ui_capabilities_human_invalidate(ui_host_t *host,void *owner)
{registry_t *r=host?host->capabilities:NULL;if(r&&r->human_owner==owner){r->human_active=0;++r->human_sequence;}}

void ui_capabilities_finalize_close(ui_host_t *host)
{
    registry_t *r=host?host->capabilities:NULL;capability_t *c;if(!r)return;
    for(c=r->capabilities;c;c=c->next)if(c->v2){r->closing=1;break;}
}
