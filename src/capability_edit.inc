/* P4A-1. A continuation of the same registry, not a second executor. */
struct edit_call {
    ui_cap_edit_context_t context;
    ui_cap_edit_completion_t completion;
    uint64_t completed;
    int waiting,ready,held,proof_refs;
};
static void free_edit(call_t *c){free(c->edit);}
static ui_status_t register_edit_common(ui_host_t *host,const ui_cap_edit_desc_t *d,const ui_cap_edit_extension_t *extension)
{
    ui_capability_desc_t n={0};const ui_capability_desc_v2_t *b;registry_t *r;ui_status_t s=registry(host,&r);if(s)return s;
    if(!d||d->size<sizeof(*d)||d->version!=1||d->risk<UI_EDIT_REVERSIBLE||d->risk>UI_EDIT_IRREVERSIBLE||(!d->transaction&&(!extension||extension->execution_policy!=UI_EDIT_POLICY_ONLY)))return UI_STATUS_INVALID_ARGUMENT;
    b=&d->capability;if(b->size<sizeof(*b)||b->version!=2||!b->execute||!b->reconcile||!b->effect||strcmp(b->effect,"write"))return UI_STATUS_INVALID_ARGUMENT;
    if(!host->capability_read_entries)return UI_STATUS_PERMISSION_DENIED;
    s=ensure_work(r);if(s)return s;n.size=sizeof(n);n.version=1;n.capability_version=b->capability_version;n.id=b->id;n.name_utf8=b->name_utf8;n.description_utf8=b->description_utf8;n.category=b->category;
    n.input_schema_json=b->input_schema_json;n.output_schema_json=b->output_schema_json;n.target_kind=b->target_kind;n.effect="write";
    return register_common(host,&n,b,d,extension);
}
ui_status_t ui_capability_register_edit(ui_host_t *host,const ui_cap_edit_desc_t *d)
{return register_edit_common(host,d,NULL);}
ui_status_t ui_capability_register_edit_extension(ui_host_t *host,const ui_cap_edit_extension_t *d)
{
 if(!d||d->size<sizeof(*d)||d->version!=1||!ui_cap_text(d->side_effect,63,0))return UI_STATUS_INVALID_ARGUMENT;
 if(d->execution_policy==UI_EDIT_APPROVED_DESTRUCTIVE){if(d->edit.risk!=UI_EDIT_DESTRUCTIVE||!d->edit.transaction)return UI_STATUS_INVALID_ARGUMENT;}
 else if(d->execution_policy==UI_EDIT_POLICY_ONLY){if(d->edit.risk!=UI_EDIT_EXTERNAL&&d->edit.risk!=UI_EDIT_IRREVERSIBLE)return UI_STATUS_INVALID_ARGUMENT;}
 else return UI_STATUS_INVALID_ARGUMENT;
 return register_edit_common(host,&d->edit,d);
}
static ui_status_t edit_gate(ui_host_t *host,registry_t *r,call_t *c)
{
    char rev[128];ui_status_t s;
    if(!c->capability||!c->resource||c->result.state==UI_CAP_FINISHED)return UI_STATUS_CANCELLED;
    if(is_closing(r)||host->dispatch_blocked||c->capability->retiring||c->resource->retiring||c->generation!=r->generation||c->v2->result.cancel_requested)return UI_STATUS_CANCELLED;
    if(host->modal_component||!c->capability->entry->state.enabled||!c->capability->entry->state.visible||c->capability->entry->state.busy)return UI_STATUS_PERMISSION_DENIED;
    s=revision(host,r,c->resource,rev);if(s)return s;if(strcmp(rev,c->v2->target.expected_revision))return UI_STATUS_VALIDATION_FAILED;
    if(c->capability->v2->desc.check){enter(host,r);s=c->capability->v2->desc.check(&c->v2->target,c->params,c->capability->v2->desc.user_data);leave(host,r);if(s)return s;}
    s=revision(host,r,c->resource,rev);if(s)return s;if(strcmp(rev,c->v2->target.expected_revision))return UI_STATUS_VALIDATION_FAILED;
    if(is_closing(r)||host->dispatch_blocked||c->capability->retiring||c->resource->retiring||c->generation!=r->generation||c->v2->result.cancel_requested)return UI_STATUS_CANCELLED;
    if(host->modal_component||!c->capability->entry->state.enabled||!c->capability->entry->state.visible||c->capability->entry->state.busy)return UI_STATUS_PERMISSION_DENIED;
    return UI_STATUS_OK;
}
ui_status_t ui_cap_edit_prepare(ui_host_t *host,const char *id,const ui_cap_target_v2_t *t,const char *params,const char *hash,const char *operation,uint64_t *out)
{
    registry_t *r;capability_t *cap;resource_t *res;call_t *c;char *canonical=NULL;size_t n,bytes;ui_status_t s=registry(host,&r);if(out)*out=0;if(s)return s;
    if(!out||!t||t->size<sizeof(*t)||t->version!=2||!ref_valid(&t->resource)||!memchr(t->expected_revision,0,128)||!memchr(t->scope_json,0,sizeof(t->scope_json))||!ui_cap_text(id,255,0)||!ui_cap_text(hash,64,0)||!ui_cap_text(operation,64,0))return UI_STATUS_INVALID_ARGUMENT;
    if(strcmp(t->scope_json,"{\"access\":\"write\"}"))return UI_STATUS_PERMISSION_DENIED;
    cap=find_cap(r,id);res=find_resource(r,t->resource.resource_id);
    if(!cap||!cap->v2||!cap->v2->edit||!ref_matches(r,res,&t->resource)||cap->desc.target_kind!=t->resource.kind)return UI_STATUS_NOT_FOUND;
    if(strcmp(hash,cap->hash))return UI_STATUS_VALIDATION_FAILED;
    if(is_closing(r)||cap->retiring||res->retiring||host->dispatch_blocked)return UI_STATUS_CANCELLED;
    if(resource_busy(r,res))return UI_STATUS_PERMISSION_DENIED;
    n=bounded_length(params,UI_CAPABILITY_INPUT_BYTES);if(n>UI_CAPABILITY_INPUT_BYTES)return UI_STATUS_LIMIT_EXCEEDED;
    s=ui_cap_json_validate(cap->input,params,n);if(s)return s;s=ui_cap_json_canonicalize(params,n,&canonical,&n);if(s)return s;
    bytes=sizeof(*c)+sizeof(call_v2_t)+sizeof(edit_call_t)+n+1;
    if(r->call_count==UI_CAPABILITY_MAX_CALLS||bytes>UI_CAPABILITY_CALL_BYTES-r->call_bytes||r->sequence==UINT64_MAX){free(canonical);return UI_STATUS_LIMIT_EXCEEDED;}
    c=calloc(1,sizeof(*c));if(!c){free(canonical);return UI_STATUS_OUT_OF_MEMORY;}c->v2=calloc(1,sizeof(*c->v2));c->edit=calloc(1,sizeof(*c->edit));
    if(!c->v2||!c->edit){free(c->v2);free(c->edit);free(c);free(canonical);return UI_STATUS_OUT_OF_MEMORY;}
    c->id=++r->sequence;c->generation=r->generation;c->params=canonical;c->bytes=bytes;c->capability=cap;c->resource=res;c->result.state=UI_CAP_PREPARED;
    c->v2->target=*t;c->v2->result.size=sizeof(c->v2->result);c->v2->result.version=2;c->v2->result.state=UI_CAP_PREPARED;c->v2->leases=1;c->edit->held=1;
    strcpy(c->v2->result.revision_before,t->expected_revision);c->edit->context.size=sizeof(c->edit->context);c->edit->context.version=1;c->edit->context.call_id=c->id;c->edit->context.target=*t;strcpy(c->edit->context.operation_id,operation);
    ++cap->references;++res->references;c->next=r->calls;r->calls=c;++r->call_count;r->call_bytes+=bytes;*out=c->id;return UI_STATUS_OK;
}
ui_status_t ui_capability_edit_get_stage(ui_host_t *host,uint64_t id,ui_cap_edit_context_t *out)
{
    registry_t *r;call_t *c;ui_status_t s=registry(host,&r);if(s)return s;
    if(!out||out->size<sizeof(*out)||out->version!=1)return UI_STATUS_INVALID_ARGUMENT;c=find_call(r,id);if(!c||!c->edit)return UI_STATUS_NOT_FOUND;*out=c->edit->context;return UI_STATUS_OK;
}
ui_status_t ui_cap_edit_undo_receipt(ui_host_t *host,uint64_t id,const char *receipt)
{registry_t *r;call_t *c;ui_status_t s=registry(host,&r);if(s)return s;c=find_call(r,id);if(!c||!c->edit)return UI_STATUS_NOT_FOUND;if(!ui_cap_text(receipt,64,0))return UI_STATUS_INVALID_ARGUMENT;strcpy(c->edit->context.undo_id,receipt);return UI_STATUS_OK;}
ui_status_t ui_capability_edit_complete(ui_host_t *host,uint64_t id,const ui_cap_edit_completion_t *in)
{
    registry_t *r;call_t *c;edit_call_t *e;char *json=NULL,rev[128];size_t n=0,old;int invalid=0;ui_status_t s=registry(host,&r);if(s)return s;c=find_call(r,id);if(!c||!c->edit)return UI_STATUS_NOT_FOUND;e=c->edit;
    if(!in||in->size<sizeof(*in)||in->version!=1||!in->stage_id||!memchr(in->transaction_id,0,65)||!memchr(in->undo_id,0,65)||(in->effect<UI_CAP2_EFFECT_NONE||in->effect>UI_CAP2_EFFECT_UNKNOWN))return UI_STATUS_INVALID_ARGUMENT;
    if(in->stage_id==e->completed)return UI_STATUS_ALREADY_EXISTS;
    if(!e->waiting||in->stage_id!=e->context.stage_id||c->completing)return UI_STATUS_NOT_FOUND;
    if(!in->executor_exited)return UI_STATUS_PENDING;
    if(in->status==UI_STATUS_OK){
      if(e->context.stage==UI_EDIT_BEGIN&&!ui_cap_text(in->transaction_id,64,0))invalid=1;
      if(e->context.transaction_id[0]&&in->transaction_id[0]&&strcmp(e->context.transaction_id,in->transaction_id))invalid=1;
      if(e->context.stage==UI_EDIT_COMMIT&&!ui_cap_text(in->undo_id,64,0))invalid=1;
      if(e->context.stage==UI_EDIT_RECONCILE&&e->context.transaction_id[0]&&strcmp(e->context.transaction_id,in->transaction_id))invalid=1;
    }
    if(in->output_json){size_t len=bounded_length(in->output_json,UI_CAPABILITY_OUTPUT_BYTES);if(len>UI_CAPABILITY_OUTPUT_BYTES)return UI_STATUS_LIMIT_EXCEEDED;
      s=ui_cap_json_validate(c->capability->output,in->output_json,len);if(!s)s=ui_cap_json_canonicalize(in->output_json,len,&json,&n);if(s==UI_STATUS_OUT_OF_MEMORY||s==UI_STATUS_LIMIT_EXCEEDED)return s;if(s)invalid=1;}
    old=c->output?strlen(c->output)+1:0;if(n+(json?1u:0u)>UI_CAPABILITY_CALL_BYTES-r->call_bytes+old){free(json);return UI_STATUS_LIMIT_EXCEEDED;}
    c->completing=1;++c->v2->leases;s=revision(host,r,c->resource,rev);--c->v2->leases;c->completing=0;if(s){free(json);return s;}
    free(c->output);r->call_bytes-=old;c->bytes-=old;c->output=json;c->bytes+=json?n+1:0;r->call_bytes+=json?n+1:0;
    e->completion=*in;if(invalid){e->completion.status=UI_STATUS_VALIDATION_FAILED;e->completion.effect=UI_CAP2_EFFECT_UNKNOWN;}e->completion.output_json=c->output;e->completed=in->stage_id;e->waiting=0;e->ready=1;
    strcpy(c->v2->result.revision_after,rev);if(!invalid&&e->context.stage==UI_EDIT_BEGIN&&in->transaction_id[0])strcpy(e->context.transaction_id,in->transaction_id);if(!invalid&&(e->context.stage==UI_EDIT_COMMIT||e->context.stage==UI_EDIT_RECONCILE)&&in->undo_id[0])strcpy(e->context.undo_id,in->undo_id);return UI_STATUS_OK;
}
ui_status_t ui_cap_edit_invoke(ui_host_t *host,uint64_t id,ui_cap_edit_stage_t stage,int (*valid)(void *),void *data)
{
    registry_t *r;call_t *c;edit_call_t *e;ui_status_t s=registry(host,&r);if(s)return s;c=find_call(r,id);if(!c||!c->edit||!c->capability)return UI_STATUS_NOT_FOUND;e=c->edit;
    if(r->callbacks||e->waiting)return UI_STATUS_PENDING;if(stage!=UI_EDIT_EXECUTE&&!c->capability->v2->edit->transaction)return UI_STATUS_UNSUPPORTED;if(e->context.stage_id==UINT64_MAX)return UI_STATUS_LIMIT_EXCEEDED;
    if(stage==UI_EDIT_BEGIN||stage==UI_EDIT_EXECUTE||stage==UI_EDIT_UNDO){s=edit_gate(host,r,c);if(s)return s;}
    if(valid&&!valid(data))return UI_STATUS_PERMISSION_DENIED;
    if(stage==UI_EDIT_RECONCILE&&!e->proof_refs){++c->capability->references;++c->resource->references;e->proof_refs=1;if(!e->held){++c->v2->leases;e->held=1;}}
    e->context.stage=stage;++e->context.stage_id;e->waiting=1;e->ready=0;
    if(stage!=UI_EDIT_RECONCILE){c->result.state=UI_CAP_RUNNING;c->v2->result.state=UI_CAP_RUNNING;}
    enter(host,r);if(stage==UI_EDIT_EXECUTE)c->capability->v2->desc.execute(id,&c->v2->target,c->params,c->capability->v2->desc.user_data);
    else c->capability->v2->edit->transaction(&e->context,c->capability->v2->desc.user_data);leave(host,r);return UI_STATUS_OK;
}
ui_status_t ui_cap_edit_result(ui_host_t *host,uint64_t id,ui_cap_edit_completion_t *out,char rev[128])
{
    registry_t *r;call_t *c;ui_status_t s=registry(host,&r);if(s)return s;c=find_call(r,id);if(!c||!c->edit)return UI_STATUS_NOT_FOUND;if(r->callbacks||!c->edit->ready)return UI_STATUS_PENDING;
    *out=c->edit->completion;strcpy(rev,c->v2->result.revision_after);return UI_STATUS_OK;
}
static ui_status_t cancel_edit(ui_host_t *host,registry_t *r,call_t *c)
{
    if(c->v2->result.cancel_requested)return UI_STATUS_OK;c->v2->result.cancel_requested=1;
    if(c->edit->waiting&&c->capability&&c->capability->v2->desc.cancel){enter(host,r);c->capability->v2->desc.cancel(c->id,c->capability->v2->desc.user_data);leave(host,r);}return UI_STATUS_OK;
}
int ui_cap_edit_cancelled(ui_host_t *host,uint64_t id)
{registry_t *r;call_t *c;if(registry(host,&r))return 1;c=find_call(r,id);return !c||!c->v2||c->v2->result.cancel_requested;}
ui_status_t ui_cap_edit_finish(ui_host_t *host,uint64_t id,ui_cap_outcome_v2_t outcome,ui_cap_effect_v2_t effect,const char *code)
{
    registry_t *r;call_t *c;ui_status_t s=registry(host,&r);if(s)return s;c=find_call(r,id);if(!c||!c->edit)return UI_STATUS_NOT_FOUND;
    if(r->callbacks||c->edit->waiting)return UI_STATUS_PENDING;if(c->result.state==UI_CAP_FINISHED)return UI_STATUS_ALREADY_EXISTS;
    finish_v2(host,r,c,outcome,effect,code);return UI_STATUS_OK;
}
ui_status_t ui_cap_edit_release(ui_host_t *host,uint64_t id,int reconciled)
{
    registry_t *r;call_t **p,*c;ui_status_t s=registry(host,&r);if(s)return s;for(p=&r->calls;*p&&(*p)->id!=id;p=&(*p)->next){}if(!*p)return UI_STATUS_NOT_FOUND;c=*p;
    if(!c->edit)return UI_STATUS_UNSUPPORTED;if(r->callbacks||c->edit->waiting)return UI_STATUS_PENDING;
    if(c->edit->proof_refs){--c->capability->references;--c->resource->references;c->edit->proof_refs=0;}
    if(c->edit->held){--c->v2->leases;c->edit->held=0;}
    if(c->v2->unknown_held&&!reconciled)return UI_STATUS_PENDING;
    if(c->result.state!=UI_CAP_FINISHED||c->v2->leases)return UI_STATUS_PENDING;
    if(c->v2->unknown_held)release_unknown(c);*p=c->next;r->call_bytes-=c->bytes;--r->call_count;free(c->output);free(c->params);free_v2_call(c);free(c);return UI_STATUS_OK;
}
