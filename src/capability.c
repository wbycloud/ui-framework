#include "capability_internal.h"
#include "ui_internal.h"
#include "assistant_internal.h"
#include "ui_framework/capability_entry.h"
#include "ui_framework/capability_ui.h"
#include "capability_edit_internal.h"
#include "json_ui.h"
#include "ui_framework/ai_coordinator.h"
#include "ui_framework/ai_operation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#endif

typedef struct resource resource_t;
typedef struct capability capability_t;
typedef struct call call_t;
typedef struct cap_v2 cap_v2_t;
typedef struct call_v2 call_v2_t;
typedef struct edit_call edit_call_t;
typedef struct ui_binding ui_binding_t;
typedef struct ui_work ui_work_t;
typedef struct input_binding input_binding_t;
static int input_available(ui_host_t *,input_binding_t *);
static void input_cleanup(void *);
static ui_status_t input_expected_revision(input_binding_t *,char [128]);
struct resource {
    ui_cap_resource_desc_t desc;
    char id[256], epoch[37];
    unsigned references, unknown_references; int retiring;
    resource_t *next;
};
struct capability {
    uint64_t incarnation;
    cap_v2_t *v2;
    ui_command_entry_t *entry;
    ui_cap_target_t *entry_target;
    ui_capability_desc_t desc;
    char *id, *name, *description, *category, *input, *output, *metadata;
    char hash[65]; size_t bytes;
    unsigned references, unknown_references; int retiring;
    capability_t *next;
};
struct call {
    edit_call_t *edit;
    call_v2_t *v2;
    uint64_t id, generation;
    uint64_t request_id;
    int delivered;
    int completing;
    ui_cap_target_t target;
    capability_t *capability; resource_t *resource;
    char *params, *output; size_t bytes;
    ui_cap_result_t result;
    call_t *next;
};
typedef struct failure_slot {
    ui_pending_request_t pending;
    char id[256],source[256];
} failure_slot_t;
typedef struct agent_profile {struct agent_profile *next;char *id,*json;size_t bytes;} agent_profile_t;
struct ui_cap_registry {
    agent_profile_t *profiles;size_t profile_bytes,profile_count;
    agent_profile_t *operations;unsigned operation_count;
    char epoch[37]; uint64_t instance, sequence, generation;
    size_t catalog_bytes, capability_count, resource_count, call_bytes, call_count;
    size_t entry_requests;
    ui_binding_t *bindings; ui_work_t *work;
    uint64_t binding_sequence, human_sequence;
    void *human_owner; int (*human_valid)(void *);void *human_valid_data;
    int human_active, ui_notifying, delivering;
    failure_slot_t *failure_slot;
    int failure_slot_used, failure_notifying;
    unsigned callbacks; int closing, workspace_closing;
    capability_t *capabilities; resource_t *resources; call_t *calls;
};
typedef struct ui_cap_registry registry_t;
struct cap_v2 { ui_capability_desc_v2_t desc; char *reconcile_schema; ui_cap_edit_desc_t *edit; unsigned execution_policy; char side_effect[64]; };
struct call_v2 {
    ui_cap_target_v2_t target; ui_cap_result_v2_t result;
    char params_hash[65];
    ui_component_t *modal_owner;
    uint64_t binding_id; unsigned leases; int unknown_held, notify_pending;
};
struct ui_binding {
    input_binding_t *input;
    ui_component_t *component; void (*cleanup)(void *);
    ui_cap_ui_binding_desc_t desc; uint64_t id,cap_incarnation; size_t bytes;
    unsigned leases; int retiring; ui_binding_t *next;
};
struct ui_work {
    int used;
    char params[UI_CAPABILITY_INPUT_BYTES+1u];
    char evidence[UI_CAPABILITY_OUTPUT_BYTES+1u];
    char input_value[4097];
};
static char *metadata_v2(capability_t *,int);
static void free_v2_call(call_t *);
static ui_status_t cancel_any(ui_host_t *,uint64_t);
static int unknown_cap(registry_t *,capability_t *);
static void detach_unknown(registry_t *,capability_t *,resource_t *);
static void destroy_v2(registry_t *);
static int is_closing(const registry_t *r){return r->closing||r->workspace_closing;}
static ui_status_t cancel_edit(ui_host_t *,registry_t *,call_t *);
static void free_edit(call_t *);

static size_t bounded_length(const char *text, size_t max)
{size_t n=0;if(!text)return max+1;while(n<=max&&text[n])++n;return n;}
static int kind_ok(ui_cap_resource_kind_t kind)
{return kind>=UI_CAP_DOCUMENT&&kind<=UI_CAP_WORKSPACE;}
static int uuid_ok(const char *text)
{
    size_t i;if(!text||bounded_length(text,36)!=36)return 0;
    for(i=0;i<36;++i){char c=text[i];if(i==8||i==13||i==18||i==23){if(c!='-')return 0;}
      else if(!((c>='0'&&c<='9')||(c>='a'&&c<='f')||(c>='A'&&c<='F')))return 0;}
    return 1;
}
static int make_uuid(char out[37])
{
#ifdef _WIN32
    GUID g;if(FAILED(CoCreateGuid(&g)))return 0;
    (void)snprintf(out,37,"%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
      (unsigned long)g.Data1,g.Data2,g.Data3,g.Data4[0],g.Data4[1],g.Data4[2],g.Data4[3],g.Data4[4],g.Data4[5],g.Data4[6],g.Data4[7]);
    return 1;
#else
    (void)out;return 0;
#endif
}
#ifdef _WIN32
static INIT_ONCE epoch_once=INIT_ONCE_STATIC_INIT;
static char process_epoch[37];
static volatile LONG64 standalone_sequence;
static BOOL CALLBACK init_epoch(PINIT_ONCE once,PVOID parameter,PVOID *context)
{(void)once;(void)parameter;(void)context;return make_uuid(process_epoch)?TRUE:FALSE;}
#endif
static ui_status_t registry(ui_host_t *host,registry_t **out)
{
    registry_t *r;
    if(!ui_language_thread_ok(host))return UI_STATUS_INVALID_ARGUMENT;
    if(host->api_version<10)return UI_STATUS_UNSUPPORTED;
    r=host->capabilities;
    if(!r){
#ifdef _WIN32
        if(!InitOnceExecuteOnce(&epoch_once,init_epoch,NULL,NULL))return UI_STATUS_PLATFORM_ERROR;
        r=calloc(1,sizeof(*r));if(!r)return UI_STATUS_OUT_OF_MEMORY;
        memcpy(r->epoch,process_epoch,sizeof(r->epoch));
        r->instance=host->capability_instance?host->capability_instance:((UINT64_C(1)<<63)|(uint64_t)InterlockedIncrement64(&standalone_sequence));
        r->generation=1;host->capabilities=r;
#else
        return UI_STATUS_UNSUPPORTED;
#endif
    }
    *out=r;return UI_STATUS_OK;
}
void ui_capabilities_set_instance(ui_host_t *host,uint64_t id){host->capability_instance=id;host->capability_loading=1;}
static capability_t *find_cap(registry_t *r,const char *id)
{capability_t *c;for(c=r->capabilities;c;c=c->next)if(!strcmp(c->id,id))return c;return NULL;}
static resource_t *find_resource(registry_t *r,const char *id)
{resource_t *p;for(p=r->resources;p;p=p->next)if(!strcmp(p->id,id))return p;return NULL;}
static call_t *find_call(registry_t *r,uint64_t id)
{call_t *c;for(c=r->calls;c;c=c->next)if(c->id==id)return c;return NULL;}
int ui_capabilities_has_id(const ui_host_t *host,const char *id)
{return host&&host->capabilities&&find_cap(host->capabilities,id)!=NULL;}
int ui_capabilities_entry_active(const ui_host_t *host,const char *id)
{capability_t *c=host&&host->capabilities?find_cap(host->capabilities,id):NULL;return c&&c->entry&&!c->v2&&!c->retiring&&!is_closing(host->capabilities);}
static void enter(ui_host_t *host,registry_t *r){++r->callbacks;ui_dispatch_enter(host);}
static void leave(ui_host_t *host,registry_t *r){--r->callbacks;ui_dispatch_leave(host);}
static ui_status_t revision(ui_host_t *host,registry_t *r,resource_t *p,char out[128])
{
    ui_status_t s;memset(out,0,128);enter(host,r);
    s=p->desc.get_revision(out,128,p->desc.user_data);leave(host,r);
    if(s)return s;
    if(out[127]||!ui_cap_text(out,127,0))return UI_STATUS_VALIDATION_FAILED;
    return UI_STATUS_OK;
}
static void free_cap(capability_t *c)
{if(c->v2){free(c->v2->edit);free(c->v2->reconcile_schema);free(c->v2);}free(c->entry_target);free(c->id);free(c->name);free(c->description);free(c->category);free(c->input);free(c->output);free(c->metadata);free(c);}
static void remove_entry(ui_host_t *host,capability_t *c)
{ui_command_entry_t **p;if(!c->entry)return;for(p=&host->commands;*p&&*p!=c->entry;p=&(*p)->next){}if(*p)*p=(*p)->next;free(c->entry->title);free(c->entry);c->entry=NULL;}
static char *metadata(capability_t *c,int schemas)
{
    char *a=ui_cap_quote(c->id),*b=ui_cap_quote(c->name),*d=ui_cap_quote(c->description),*e=ui_cap_quote(c->category),*out=NULL;
    size_t size;
    if(c->v2){free(a);free(b);free(d);free(e);return metadata_v2(c,schemas);}
    if(!a||!b||!d||!e)goto done;
    size=strlen(a)+strlen(b)+strlen(d)+strlen(e)+512+(schemas?strlen(c->input)+strlen(c->output):0);
    out=malloc(size);if(!out)goto done;
    (void)snprintf(out,size,"{\"id\":%s,\"name\":%s,\"description\":%s,\"category\":%s,\"contract_version\":1,\"capability_version\":%u,\"target_kind\":%u,\"effect\":\"read\",\"cancellable\":%s%s%s%s%s}",
      a,b,d,e,c->desc.capability_version,(unsigned)c->desc.target_kind,c->desc.cancel?"true":"false",
      schemas?",\"input_schema\":" : "",schemas?c->input:"",schemas?",\"output_schema\":" : "",schemas?c->output:"");
done:free(a);free(b);free(d);free(e);return out;
}

ui_status_t ui_capability_register_resource(ui_host_t *host,const ui_cap_resource_desc_t *d)
{
    registry_t *r;resource_t *p;ui_status_t s=registry(host,&r);if(s)return s;
    if(!d||d->size<sizeof(*d)||d->version!=1||!kind_ok(d->kind)||!ui_cap_text(d->id,255,0)||!d->get_revision||
       (d->open_epoch&&!uuid_ok(d->open_epoch)))return UI_STATUS_INVALID_ARGUMENT;
    if(is_closing(r)||host->dispatch_blocked)return UI_STATUS_CANCELLED;
    if(find_resource(r,d->id))return UI_STATUS_ALREADY_EXISTS;
    if(r->resource_count==256||sizeof(*p)>UI_CAPABILITY_CATALOG_BYTES-r->catalog_bytes)return UI_STATUS_LIMIT_EXCEEDED;
    p=calloc(1,sizeof(*p));if(!p)return UI_STATUS_OUT_OF_MEMORY;
    p->desc=*d;strcpy(p->id,d->id);p->desc.id=p->id;
    if(d->open_epoch)strcpy(p->epoch,d->open_epoch);else if(!make_uuid(p->epoch)){free(p);return UI_STATUS_PLATFORM_ERROR;}
    p->desc.open_epoch=p->epoch;p->next=r->resources;r->resources=p;
    ++r->resource_count;r->catalog_bytes+=sizeof(*p);++r->generation;return UI_STATUS_OK;
}

static ui_status_t register_common(ui_host_t *host,const ui_capability_desc_t *d,const ui_capability_desc_v2_t *d2,const ui_cap_edit_desc_t *edit,const ui_cap_edit_extension_t *extension)
{
    registry_t *r;capability_t *c;ui_status_t s=registry(host,&r);char *raw=NULL;size_t n;
    if(s)return s;
    if(!d||d->size<sizeof(*d)||d->version!=1||!d->capability_version||(!d2&&!d->execute)||!kind_ok(d->target_kind)||
      !ui_cap_text(d->id,255,0)||!ui_cap_text(d->name_utf8,255,0)||!ui_cap_text(d->description_utf8,4096,1)||!ui_cap_text(d->category,127,0))return UI_STATUS_INVALID_ARGUMENT;
    if(!d2&&(!d->effect||strcmp(d->effect,"read")))return UI_STATUS_UNSUPPORTED;
    if(is_closing(r)||host->dispatch_blocked)return UI_STATUS_CANCELLED;
    if(find_cap(r,d->id)||ui_assistants_have_id(host,d->id))return UI_STATUS_ALREADY_EXISTS;
    {ui_command_entry_t *e;for(e=host->commands;e;e=e->next)if(!strcmp(e->id,d->id))return UI_STATUS_ALREADY_EXISTS;}
    if(r->capability_count==UI_CAPABILITY_MAX_COUNT)return UI_STATUS_LIMIT_EXCEEDED;
    c=calloc(1,sizeof(*c));if(!c)return UI_STATUS_OUT_OF_MEMORY;c->desc=*d;
    if(d2){c->v2=calloc(1,sizeof(*c->v2));if(!c->v2){s=UI_STATUS_OUT_OF_MEMORY;goto fail;}c->v2->desc=*d2;
      if(extension){c->v2->execution_policy=extension->execution_policy;strcpy(c->v2->side_effect,extension->side_effect);}
      if(edit){c->v2->edit=malloc(sizeof(*edit));if(!c->v2->edit){s=UI_STATUS_OUT_OF_MEMORY;goto fail;}*c->v2->edit=*edit;}
      s=ui_cap_schema_canonical(d2->reconcile_schema_json,&c->v2->reconcile_schema);if(s)goto fail;
      c->v2->desc.effect=!strcmp(d2->effect,"write")?"write":!strcmp(d2->effect,"draft")?"draft":"view";}
    s=ui_cap_schema_canonical(d->input_schema_json,&c->input);if(s)goto fail;
    s=ui_cap_schema_canonical(d->output_schema_json,&c->output);if(s)goto fail;
    c->id=ui_strdup(d->id);c->name=ui_strdup(d->name_utf8);c->description=ui_strdup(d->description_utf8);c->category=ui_strdup(d->category);
    if(!c->id||!c->name||!c->description||!c->category){s=UI_STATUS_OUT_OF_MEMORY;goto fail;}
    c->desc.id=c->id;c->desc.name_utf8=c->name;c->desc.description_utf8=c->description;c->desc.category=c->category;
    c->desc.input_schema_json=c->input;c->desc.output_schema_json=c->output;c->desc.effect=d2?c->v2->desc.effect:"read";
    raw=metadata(c,1);if(!raw){s=UI_STATUS_OUT_OF_MEMORY;goto fail;}
    s=ui_cap_json_canonicalize(raw,strlen(raw),&c->metadata,&n);free(raw);raw=NULL;if(s)goto fail;
    s=ui_cap_hash_bytes(c->metadata,n,c->hash);if(s)goto fail;
    c->bytes=sizeof(*c)+strlen(c->id)+strlen(c->name)+strlen(c->description)+strlen(c->category)+strlen(c->input)+strlen(c->output)+n+7;
    if(d2){c->v2->desc.id=c->id;c->v2->desc.name_utf8=c->name;c->v2->desc.description_utf8=c->description;c->v2->desc.category=c->category;
      c->v2->desc.input_schema_json=c->input;c->v2->desc.output_schema_json=c->output;c->v2->desc.reconcile_schema_json=c->v2->reconcile_schema;}
    if(d2)c->bytes+=sizeof(*c->v2)+strlen(c->v2->reconcile_schema)+1+sizeof(ui_command_entry_t)+4096;
    if(edit)c->bytes+=sizeof(*edit);
    if(c->bytes>UI_CAPABILITY_CATALOG_BYTES-r->catalog_bytes){s=UI_STATUS_LIMIT_EXCEEDED;goto fail;}
    if(d2){c->entry=calloc(1,sizeof(*c->entry));if(!c->entry){s=UI_STATUS_OUT_OF_MEMORY;goto fail;}
      c->entry->title=ui_strdup(c->name);if(!c->entry->title){free(c->entry);c->entry=NULL;s=UI_STATUS_OUT_OF_MEMORY;goto fail;}
      c->entry->id=c->id;c->entry->params_schema_json=c->input;c->entry->capability_generated=2;
      c->entry->state.size=sizeof(c->entry->state);c->entry->state.enabled=c->entry->state.visible=1;
      c->entry->next=host->commands;host->commands=c->entry;}
    c->incarnation=r->generation+1;c->next=r->capabilities;r->capabilities=c;r->catalog_bytes+=c->bytes;++r->capability_count;++r->generation;return UI_STATUS_OK;
fail:free(raw);free_cap(c);return s;
}

ui_status_t ui_capability_register(ui_host_t *host,const ui_capability_desc_t *d)
{return register_common(host,d,NULL,NULL,NULL);}

static void finish(ui_host_t *host,registry_t *r,call_t *c,ui_cap_outcome_t outcome,const char *code)
{
    c->result.state=UI_CAP_FINISHED;c->result.outcome=outcome;
    (void)snprintf(c->result.code,sizeof(c->result.code),"%s",code);
    if(c->capability){--c->capability->references;c->capability=NULL;}
    if(c->resource){--c->resource->references;c->resource=NULL;}
    if(c->request_id&&!c->delivered){char json[96];c->delivered=1;
        (void)snprintf(json,sizeof(json),"{\"code\":\"%s\"}",c->result.code);
        /* Retain the record, host and application module throughout the result
         * callback, including synchronous completion inside execute. */
        enter(host,r);(void)ui_host_reply_capability(host,c->request_id,outcome==UI_CAP_OK,
            outcome==UI_CAP_OK?c->output:json);leave(host,r);
    }
}
ui_status_t ui_capability_cancel(ui_host_t *host,uint64_t id)
{
    registry_t *r;call_t *c;ui_status_t s=registry(host,&r);if(s)return s;c=find_call(r,id);if(!c)return UI_STATUS_NOT_FOUND;
    if(c->v2)return UI_STATUS_UNSUPPORTED;
    if(c->result.state==UI_CAP_FINISHED||c->result.cancel_requested)return UI_STATUS_OK;
    c->result.cancel_requested=1;
    if(c->result.state==UI_CAP_PREPARED)finish(host,r,c,UI_CAP_CANCELLED,"cancelled_before_dispatch");
    else if(c->capability->desc.cancel){capability_t *cap=c->capability;enter(host,r);cap->desc.cancel(id,cap->desc.user_data);leave(host,r);}
    return UI_STATUS_OK;
}
ui_status_t ui_capability_begin_unregistration(ui_host_t *host,const char *id)
{
    registry_t *r;capability_t *p;call_t *c;ui_status_t s=registry(host,&r);if(s)return s;if(!ui_cap_text(id,255,0))return UI_STATUS_INVALID_ARGUMENT;
    p=find_cap(r,id);if(!p)return UI_STATUS_NOT_FOUND;if(p->retiring)return UI_STATUS_OK;
    p->retiring=1;++r->generation;
    if(p->entry)p->entry->state.enabled=0;
    for(c=r->calls;c;c=c->next)if(c->capability==p)(void)cancel_any(host,c->id);
    return UI_STATUS_OK;
}
ui_status_t ui_capability_finish_unregistration(ui_host_t *host,const char *id)
{
    registry_t *r;capability_t **p,*old;ui_status_t s=registry(host,&r);if(s)return s;if(!ui_cap_text(id,255,0))return UI_STATUS_INVALID_ARGUMENT;
    for(p=&r->capabilities;*p&&strcmp((*p)->id,id);p=&(*p)->next){}
    if(!*p)return UI_STATUS_NOT_FOUND;old=*p;
    if(!old->retiring)return UI_STATUS_INVALID_ARGUMENT;
    if(old->references||r->callbacks||host->capability_ui_callbacks||(!r->closing&&unknown_cap(r,old)))return UI_STATUS_PENDING;
    if(r->closing)detach_unknown(r,old,NULL);
    *p=old->next;r->catalog_bytes-=old->bytes;--r->capability_count;remove_entry(host,old);free_cap(old);return UI_STATUS_OK;
}
ui_status_t ui_capability_begin_resource_unregistration(ui_host_t *host,const char *id)
{
    registry_t *r;resource_t *p;call_t *c;ui_status_t s=registry(host,&r);if(s)return s;if(!ui_cap_text(id,255,0))return UI_STATUS_INVALID_ARGUMENT;
    p=find_resource(r,id);if(!p)return UI_STATUS_NOT_FOUND;if(p->retiring)return UI_STATUS_OK;
    p->retiring=1;++r->generation;
    for(c=r->calls;c;c=c->next)if(c->resource==p)(void)cancel_any(host,c->id);
    return UI_STATUS_OK;
}
ui_status_t ui_capability_finish_resource_unregistration(ui_host_t *host,const char *id)
{
    registry_t *r;resource_t **p,*old;ui_status_t s=registry(host,&r);if(s)return s;if(!ui_cap_text(id,255,0))return UI_STATUS_INVALID_ARGUMENT;
    for(p=&r->resources;*p&&strcmp((*p)->id,id);p=&(*p)->next){}
    if(!*p)return UI_STATUS_NOT_FOUND;old=*p;if(!old->retiring)return UI_STATUS_INVALID_ARGUMENT;
    if(old->references||r->callbacks||host->capability_ui_callbacks||(!r->closing&&old->unknown_references))return UI_STATUS_PENDING;
    if(r->closing)detach_unknown(r,NULL,old);
    *p=old->next;free(old);--r->resource_count;r->catalog_bytes-=sizeof(*old);return UI_STATUS_OK;
}

ui_status_t ui_capability_describe(ui_host_t *host,const char *id,char **json)
{
    registry_t *r;capability_t *c;ui_status_t s=registry(host,&r);size_t n,length;char *raw;
    if(s)return s;if(!json||!ui_cap_text(id,255,0))return UI_STATUS_INVALID_ARGUMENT;*json=NULL;
    c=find_cap(r,id);if(!c||c->v2||c->retiring||is_closing(r))return UI_STATUS_NOT_FOUND;
    n=strlen(c->metadata);raw=malloc(n+89);if(!raw)return UI_STATUS_OUT_OF_MEMORY;
    (void)snprintf(raw,n+89,"{\"descriptor_hash\":\"%s\",%s",c->hash,c->metadata+1);
    s=ui_cap_json_canonicalize(raw,strlen(raw),json,&length);free(raw);return s;
}
ui_status_t ui_capability_discover(ui_host_t *host,const char *category,size_t offset,size_t limit,uint64_t *generation,char **json)
{
    registry_t *r;capability_t *c;size_t seen=0,count=0,used=0,next=0;ui_status_t s=registry(host,&r);char *page;
    if(s)return s;if(!generation||!json||!limit||limit>32||(category&&!ui_cap_text(category,127,1)))return UI_STATUS_INVALID_ARGUMENT;
    *json=NULL;if(*generation&&*generation!=r->generation)return UI_STATUS_CANCELLED;
    page=malloc(UI_CAPABILITY_OUTPUT_BYTES+1u);if(!page)return UI_STATUS_OUT_OF_MEMORY;
    used=(size_t)snprintf(page,UI_CAPABILITY_OUTPUT_BYTES+1u,"{\"generation\":\"%llu\",\"items\":[",(unsigned long long)r->generation);
    for(c=r->capabilities;c&&!is_closing(r);c=c->next){char *entry;size_t n;
        if(c->v2||c->retiring||(category&&*category&&strcmp(category,c->category)))continue;
        if(seen++<offset)continue;if(count==limit){next=offset+count;break;}
        entry=metadata(c,0);if(!entry){free(page);return UI_STATUS_OUT_OF_MEMORY;}n=strlen(entry);
        if(n+used+100>UI_CAPABILITY_OUTPUT_BYTES){free(entry);free(page);return UI_STATUS_LIMIT_EXCEEDED;}
        if(count)page[used++]=',';
        /* Include the same identity hash in discovery and describe. */
        used+=(size_t)snprintf(page+used,UI_CAPABILITY_OUTPUT_BYTES+1u-used,"{\"descriptor_hash\":\"%s\",%s",c->hash,entry+1);free(entry);++count;
    }
    (void)snprintf(page+used,UI_CAPABILITY_OUTPUT_BYTES+1u-used,"],\"next\":%zu}",next);
    *generation=r->generation;*json=page;return UI_STATUS_OK;
}

ui_status_t ui_capability_bind(ui_host_t *host,const char *id,ui_cap_target_t *target)
{
    registry_t *r;resource_t *p;ui_cap_target_t t;uint64_t generation;ui_status_t s=registry(host,&r);
    if(s)return s;if(!ui_cap_text(id,255,0)||!target||target->size<sizeof(*target)||target->version!=1)return UI_STATUS_INVALID_ARGUMENT;
    p=find_resource(r,id);if(!p||p->retiring)return UI_STATUS_NOT_FOUND;
    if(is_closing(r)||host->dispatch_blocked)return UI_STATUS_CANCELLED;
    memset(&t,0,sizeof(t));t.size=sizeof(t);t.version=1;t.kind=p->desc.kind;t.instance_id=r->instance;
    strcpy(t.host_epoch,r->epoch);strcpy(t.resource_id,p->id);strcpy(t.open_epoch,p->epoch);strcpy(t.scope_json,"{\"access\":\"read\"}");
    generation=r->generation;++p->references;s=revision(host,r,p,t.expected_revision);--p->references;
    if(s)return s;if(generation!=r->generation||is_closing(r))return UI_STATUS_CANCELLED;*target=t;return UI_STATUS_OK;
}
ui_status_t ui_capability_resources(ui_host_t *host,size_t offset,ui_cap_target_t *targets,size_t capacity,size_t *count,size_t *next,uint64_t *generation)
{
    registry_t *r;resource_t *p;ui_cap_target_t *temp;size_t seen=0,n=0,more=0;uint64_t gen;ui_status_t s=registry(host,&r);
    if(s)return s;if(!targets||!count||!next||!generation||!capacity||capacity>32)return UI_STATUS_INVALID_ARGUMENT;
    *count=0;*next=0;gen=r->generation;if(*generation&&*generation!=gen)return UI_STATUS_CANCELLED;
    temp=calloc(capacity,sizeof(*temp));if(!temp)return UI_STATUS_OUT_OF_MEMORY;
    for(p=r->resources;p&&!is_closing(r);p=p->next){
        if(p->retiring)continue;if(seen++<offset)continue;if(n==capacity){more=offset+n;break;}
        temp[n].size=sizeof(*temp);temp[n].version=1;s=ui_capability_bind(host,p->id,&temp[n]);if(s)break;++n;
    }
    if(!s&&gen!=r->generation)s=UI_STATUS_CANCELLED;
    if(!s){memcpy(targets,temp,n*sizeof(*temp));*count=n;*next=more;*generation=gen;}free(temp);return s;
}

static ui_status_t target_check(ui_host_t *host,registry_t *r,call_t *c,int *conflict)
{
    char rev[128];ui_status_t s;*conflict=0;
    if(is_closing(r)||host->dispatch_blocked||c->resource->retiring||c->capability->retiring)return UI_STATUS_CANCELLED;
    if(host->modal_component)return UI_STATUS_PERMISSION_DENIED;
    if(c->generation!=r->generation)return UI_STATUS_CANCELLED;
    s=revision(host,r,c->resource,rev);if(s)return s;
    if(c->result.state!=UI_CAP_PREPARED)return UI_STATUS_CANCELLED;
    if(strcmp(rev,c->target.expected_revision)){*conflict=1;return UI_STATUS_VALIDATION_FAILED;}
    if(c->capability->desc.check){enter(host,r);s=c->capability->desc.check(&c->target,c->params,c->capability->desc.user_data);leave(host,r);if(s)return s;}
    if(c->result.state!=UI_CAP_PREPARED)return UI_STATUS_CANCELLED;
    /* check/get_revision may reenter and retire a capability or change revision. */
    if(is_closing(r)||c->resource->retiring||c->capability->retiring||c->generation!=r->generation)return UI_STATUS_CANCELLED;
    s=revision(host,r,c->resource,rev);if(s)return s;
    if(c->result.state!=UI_CAP_PREPARED)return UI_STATUS_CANCELLED;
    if(strcmp(rev,c->target.expected_revision)){*conflict=1;return UI_STATUS_VALIDATION_FAILED;}
    /* Every application callback may reenter, including the last revision read.
     * Recheck admission after it returns, with no callbacks before execute. */
    if(is_closing(r)||host->dispatch_blocked||c->resource->retiring||c->capability->retiring||c->generation!=r->generation)return UI_STATUS_CANCELLED;
    if(host->modal_component)return UI_STATUS_PERMISSION_DENIED;
    if(c->capability->entry&&(!c->capability->entry->state.enabled||c->capability->entry->state.busy||!c->capability->entry->state.visible))return UI_STATUS_PERMISSION_DENIED;
    return UI_STATUS_OK;
}
ui_status_t ui_capability_prepare(ui_host_t *host,const char *id,const ui_cap_target_t *t,const char *params,uint64_t *out)
{
    registry_t *r;capability_t *cap;resource_t *res;call_t *c;ui_status_t s=registry(host,&r);size_t n,canonical_n,bytes;char *canonical=NULL;
    if(s)return s;if(out)*out=0;
    if(!out||!ui_cap_text(id,255,0)||!t||t->size<sizeof(*t)||t->version!=1||
       !memchr(t->host_epoch,0,sizeof(t->host_epoch))||!memchr(t->resource_id,0,sizeof(t->resource_id))||
       !memchr(t->open_epoch,0,sizeof(t->open_epoch))||!memchr(t->expected_revision,0,sizeof(t->expected_revision))||
       !memchr(t->scope_json,0,sizeof(t->scope_json)))return UI_STATUS_INVALID_ARGUMENT;
    if(is_closing(r)||host->dispatch_blocked)return UI_STATUS_CANCELLED;
    cap=find_cap(r,id);res=find_resource(r,t->resource_id);
    if(cap&&cap->v2)return UI_STATUS_PERMISSION_DENIED;
    if(!cap||cap->retiring||!res||res->retiring)return UI_STATUS_NOT_FOUND;
    if(strcmp(t->host_epoch,r->epoch)||t->instance_id!=r->instance||strcmp(t->open_epoch,res->epoch)||t->kind!=res->desc.kind||t->kind!=cap->desc.target_kind)return UI_STATUS_NOT_FOUND;
    if(strcmp(t->scope_json,"{\"access\":\"read\"}"))return UI_STATUS_PERMISSION_DENIED;
    if(!ui_cap_text(t->expected_revision,127,0))return UI_STATUS_INVALID_ARGUMENT;
    if(host->capability_loading)return UI_STATUS_CANCELLED;
    n=bounded_length(params,UI_CAPABILITY_INPUT_BYTES);if(n>UI_CAPABILITY_INPUT_BYTES)return params?UI_STATUS_LIMIT_EXCEEDED:UI_STATUS_INVALID_ARGUMENT;
    s=ui_cap_json_validate(cap->input,params,n);if(s)return s;
    s=ui_cap_json_canonicalize(params,n,&canonical,&canonical_n);if(s)return s;
    if(canonical_n>UI_CAPABILITY_INPUT_BYTES){free(canonical);return UI_STATUS_LIMIT_EXCEEDED;}
    bytes=sizeof(*c)+canonical_n+1;
    if(r->call_count==UI_CAPABILITY_MAX_CALLS||bytes>UI_CAPABILITY_CALL_BYTES-r->call_bytes||r->sequence==UINT64_MAX){free(canonical);return UI_STATUS_LIMIT_EXCEEDED;}
    c=calloc(1,sizeof(*c));if(!c){free(canonical);return UI_STATUS_OUT_OF_MEMORY;}
    c->id=++r->sequence;c->generation=r->generation;c->target=*t;c->params=canonical;c->capability=cap;c->resource=res;
    c->bytes=bytes;c->result.size=sizeof(c->result);c->result.version=1;c->result.state=UI_CAP_PREPARED;
    strcpy(c->result.revision,t->expected_revision);++cap->references;++res->references;
    c->next=r->calls;r->calls=c;r->call_bytes+=c->bytes;++r->call_count;*out=c->id;return UI_STATUS_OK;
}
ui_status_t ui_capability_dispatch(ui_host_t *host,uint64_t id)
{
    registry_t *r;call_t *c;capability_t *cap;int conflict=0;ui_status_t s=registry(host,&r);if(s)return s;
    c=find_call(r,id);if(!c)return UI_STATUS_NOT_FOUND;if(c->v2)return UI_STATUS_PERMISSION_DENIED;if(c->result.state!=UI_CAP_PREPARED)return UI_STATUS_OK;
    /* No nested dispatch inside check/execute/cancel. Do not pump UI here. */
    if(r->callbacks)return UI_STATUS_PENDING;
    s=target_check(host,r,c,&conflict);
    if(c->result.state!=UI_CAP_PREPARED)return UI_STATUS_OK;
    if(s){finish(host,r,c,conflict?UI_CAP_STALE:UI_CAP_REJECTED,conflict?"revision_conflict":"unavailable");return UI_STATUS_OK;}
    c->result.state=UI_CAP_RUNNING;cap=c->capability;enter(host,r);
    cap->desc.execute(id,&c->target,c->params,cap->desc.user_data);leave(host,r);return UI_STATUS_OK;
}
ui_status_t ui_capability_complete(ui_host_t *host,uint64_t id,const ui_cap_result_t *result)
{
    registry_t *r;call_t *c;ui_status_t s=registry(host,&r);char *canonical=NULL,rev[128];size_t n,len=0;
    if(s)return s;c=find_call(r,id);if(!c)return UI_STATUS_NOT_FOUND;
    if(c->v2)return UI_STATUS_UNSUPPORTED;
    if(c->result.state==UI_CAP_FINISHED)return UI_STATUS_ALREADY_EXISTS;
    if(c->completing)return UI_STATUS_PENDING;
    if(c->result.state!=UI_CAP_RUNNING||!result||result->size<sizeof(*result)||result->version!=1||result->outcome<UI_CAP_OK||result->outcome>UI_CAP_STALE)return UI_STATUS_INVALID_ARGUMENT;
    if(result->outcome==UI_CAP_OK){
        n=bounded_length(result->output_json,UI_CAPABILITY_OUTPUT_BYTES);
        s=n>UI_CAPABILITY_OUTPUT_BYTES?UI_STATUS_LIMIT_EXCEEDED:ui_cap_json_validate(c->capability->output,result->output_json,n);
        if(!s)s=ui_cap_json_canonicalize(result->output_json,n,&canonical,&len);
        if(s==UI_STATUS_OUT_OF_MEMORY)return s;
        if(s){finish(host,r,c,UI_CAP_FAILED,"invalid_output");return UI_STATUS_OK;}
        c->completing=1;s=revision(host,r,c->resource,rev);c->completing=0;
        if(s){free(canonical);finish(host,r,c,UI_CAP_FAILED,"revision_unavailable");return UI_STATUS_OK;}
        if(strcmp(rev,c->target.expected_revision)){free(canonical);finish(host,r,c,UI_CAP_STALE,"revision_conflict");return UI_STATUS_OK;}
        if(len+1>UI_CAPABILITY_CALL_BYTES-r->call_bytes){free(canonical);return UI_STATUS_LIMIT_EXCEEDED;}
        c->output=canonical;c->result.output_json=canonical;c->bytes+=len+1;r->call_bytes+=len+1;
    }
    finish(host,r,c,result->outcome,result->outcome==UI_CAP_OK?"ok":result->outcome==UI_CAP_CANCELLED?"cancelled":"application_failure");return UI_STATUS_OK;
}
ui_status_t ui_capability_get_result(ui_host_t *host,uint64_t id,ui_cap_result_t *out)
{
    registry_t *r;call_t *c;ui_status_t s=registry(host,&r);if(s)return s;
    if(!out||out->size<sizeof(*out)||out->version!=1)return UI_STATUS_INVALID_ARGUMENT;
    c=find_call(r,id);if(!c)return UI_STATUS_NOT_FOUND;if(c->v2)return UI_STATUS_UNSUPPORTED;*out=c->result;return UI_STATUS_OK;
}
ui_status_t ui_capability_forget(ui_host_t *host,uint64_t id)
{
    registry_t *r;call_t **p,*old;ui_status_t s=registry(host,&r);if(s)return s;
    for(p=&r->calls;*p&&(*p)->id!=id;p=&(*p)->next){}
    if(!*p)return UI_STATUS_NOT_FOUND;old=*p;if(old->v2)return UI_STATUS_UNSUPPORTED;if(old->result.state!=UI_CAP_FINISHED||r->callbacks)return UI_STATUS_PENDING;
    *p=old->next;r->call_bytes-=old->bytes;--r->call_count;free(old->params);free(old->output);free(old);return UI_STATUS_OK;
}
ui_status_t ui_capability_quiesce(ui_host_t *host)
{
    registry_t *r;call_t *c;ui_status_t s=registry(host,&r);if(s)return s;r->closing=1;
    for(c=r->calls;c;c=c->next)if(c->result.state!=UI_CAP_FINISHED)(void)cancel_any(host,c->id);
    return ui_capabilities_busy(host)?UI_STATUS_PENDING:UI_STATUS_OK;
}
void ui_capabilities_workspace_close(ui_host_t *host,int closing)
{
    registry_t *r=host->capabilities;call_t *c;if(!r)return;
    r->workspace_closing=closing;
    if(closing)for(c=r->calls;c;c=c->next)if(c->result.state!=UI_CAP_FINISHED)(void)cancel_any(host,c->id);
}
int ui_capabilities_busy(const ui_host_t *host)
{
    registry_t *r=host?host->capabilities:NULL;call_t *c;
    if(host&&(host->assistant_callbacks||host->app_poll_callbacks))return 1;
    if(!r)return 0;if(r->callbacks||r->delivering)return 1;
    {resource_t *p;for(p=r->resources;p;p=p->next)if(p->references)return 1;}
    for(c=r->calls;c;c=c->next)if(c->result.state!=UI_CAP_FINISHED||(c->v2&&c->v2->leases))return 1;return 0;
}
void ui_capabilities_destroy(ui_host_t *host)
{
    registry_t *r=host->capabilities;call_t *c;capability_t *cap;resource_t *res;if(!r)return;
    destroy_v2(r);
    while((c=r->calls)!=NULL){r->calls=c->next;free(c->params);free(c->output);free_v2_call(c);free(c);}
    while((cap=r->capabilities)!=NULL){r->capabilities=cap->next;remove_entry(host,cap);free_cap(cap);}
    while((res=r->resources)!=NULL){r->resources=res->next;free(res);}
    while(r->profiles){agent_profile_t *p=r->profiles;r->profiles=p->next;free(p->id);free(p->json);free(p);}
    while(r->operations){agent_profile_t *p=r->operations;r->operations=p->next;free(p->id);free(p->json);free(p);}
    free(r->failure_slot);free(r);host->capabilities=NULL;
}

ui_status_t ui_capability_enable_read_entries(ui_host_t *host)
{registry_t *r;ui_status_t s=registry(host,&r);if(s)return s;if(is_closing(r)||host->dispatch_blocked)return UI_STATUS_CANCELLED;
 if(!host->capability_read_entries&&(host->pending||host->dispatch_depth||host->assistant_callbacks||r->callbacks))return UI_STATUS_PENDING;
 if(!r->failure_slot){
    if(sizeof(*r->failure_slot)>UI_CAPABILITY_CALL_BYTES-r->call_bytes)return UI_STATUS_LIMIT_EXCEEDED;
    r->failure_slot=calloc(1,sizeof(*r->failure_slot));if(!r->failure_slot)return UI_STATUS_OUT_OF_MEMORY;
    r->call_bytes+=sizeof(*r->failure_slot);
 }
 host->capability_read_entries=1;return UI_STATUS_OK;}

/* A single precharged slot preserves the first observable failure even when
 * the byte budget is full. While notifying that failure, new generated work
 * is rejected before acceptance; it cannot recursively notify another error. */
ui_status_t ui_capabilities_acquire_pending(ui_host_t *host,const char *id,const char *source,ui_pending_request_t **out)
{
    registry_t *r=host->capabilities;ui_pending_request_t *p;size_t id_n=strlen(id)+1,source_n=strlen(source?source:"")+1;
    size_t bytes=sizeof(*p)+id_n+source_n;
    *out=NULL;
    if(r->failure_notifying)return UI_STATUS_LIMIT_EXCEEDED;
    if(bytes>UI_CAPABILITY_CALL_BYTES-r->call_bytes){
        if(r->failure_slot_used)return UI_STATUS_LIMIT_EXCEEDED;
        p=&r->failure_slot->pending;memset(p,0,sizeof(*p));p->capability_limited=1;
        p->command_id=r->failure_slot->id;p->source=r->failure_slot->source;r->failure_slot_used=1;
    }else{
        p=calloc(1,bytes);if(!p)return UI_STATUS_OUT_OF_MEMORY;
        p->command_id=(char *)(p+1);p->source=p->command_id+id_n;
        p->capability_bytes=bytes;r->call_bytes+=bytes;
    }
    memcpy(p->command_id,id,id_n);memcpy(p->source,source?source:"",source_n);
    p->capability_generated=1;*out=p;return UI_STATUS_OK;
}
void ui_capabilities_release_pending(ui_host_t *host,ui_pending_request_t *p)
{
    registry_t *r=host->capabilities;
    if(p==&r->failure_slot->pending){r->failure_slot_used=0;return;}
    r->call_bytes-=p->capability_bytes;free(p);
}
void ui_capabilities_reply_failure(ui_host_t *host,uint64_t request,ui_status_t status)
{
    registry_t *r=host->capabilities;char json[64];
    (void)snprintf(json,sizeof(json),"{\"status\":%d}",(int)status);
    enter(host,r);
    (void)ui_host_reply_capability(host,request,0,json);
    leave(host,r);
}
void ui_capabilities_failure_enter(ui_host_t *host){++host->capabilities->failure_notifying;}
void ui_capabilities_failure_leave(ui_host_t *host){--host->capabilities->failure_notifying;}
static void entry_execute(ui_host_t *host,uint64_t request,const char *id,const char *params,const char *source,void *data)
{
    capability_t *cap=data;registry_t *r=host->capabilities;uint64_t call=0;ui_status_t s;
    (void)source;
    s=ui_capability_prepare(host,id,cap->entry_target,params,&call);
    if(s){ui_capabilities_reply_failure(host,request,s);return;}
    find_call(r,call)->request_id=request; /* Association precedes any execute. */
    s=ui_capability_dispatch(host,call);
    /* Nested invocation never pumps or executes recursively. Keep a visible
     * terminal rejection rather than stranding an undispatched legacy request. */
    if(s==UI_STATUS_PENDING)(void)ui_capability_cancel(host,call);
}
ui_status_t ui_capability_publish_read_entry(ui_host_t *host,const ui_cap_entry_binding_t *b)
{
    registry_t *r;capability_t *cap;resource_t *res;ui_command_entry_t *e;
    ui_cap_target_t *t;ui_status_t s=registry(host,&r);size_t bytes=sizeof(*e)+sizeof(*t)+4096;
    if(s)return s;
    if(!b||b->size<sizeof(*b)||b->version!=1||!ui_cap_text(b->capability_id,255,0)||!b->target||b->target->size<sizeof(*t)||b->target->version!=1)return UI_STATUS_INVALID_ARGUMENT;
    if(!host->capability_read_entries)return UI_STATUS_PERMISSION_DENIED;
    if(is_closing(r)||host->dispatch_blocked)return UI_STATUS_CANCELLED;
    cap=find_cap(r,b->capability_id);if(!cap||cap->retiring)return UI_STATUS_NOT_FOUND;
    if(cap->v2)return UI_STATUS_UNSUPPORTED;
    if(cap->entry)return UI_STATUS_ALREADY_EXISTS;
    /* prepare validates the same target without invoking application code.
     * Publication must not allocate an observable call or consume a call ID. */
    t=(ui_cap_target_t *)b->target;
    if(!memchr(t->host_epoch,0,sizeof(t->host_epoch))||!memchr(t->resource_id,0,sizeof(t->resource_id))||!memchr(t->open_epoch,0,sizeof(t->open_epoch))||!memchr(t->expected_revision,0,sizeof(t->expected_revision))||!memchr(t->scope_json,0,sizeof(t->scope_json)))return UI_STATUS_INVALID_ARGUMENT;
    res=find_resource(r,t->resource_id);
    if(!res||res->retiring||t->instance_id!=r->instance||strcmp(t->host_epoch,r->epoch)||strcmp(t->open_epoch,res->epoch)||t->kind!=res->desc.kind||t->kind!=cap->desc.target_kind)return UI_STATUS_NOT_FOUND;
    if(strcmp(t->scope_json,"{\"access\":\"read\"}"))return UI_STATUS_PERMISSION_DENIED;
    if(!ui_cap_text(t->expected_revision,127,0))return UI_STATUS_INVALID_ARGUMENT;
    if(bytes>UI_CAPABILITY_CATALOG_BYTES-r->catalog_bytes)return UI_STATUS_LIMIT_EXCEEDED;
    e=calloc(1,sizeof(*e));t=malloc(sizeof(*t));if(!e||!t){free(e);free(t);return UI_STATUS_OUT_OF_MEMORY;}
    e->title=ui_strdup(cap->name);if(!e->title){free(e);free(t);return UI_STATUS_OUT_OF_MEMORY;}
    *t=*b->target;cap->entry_target=t;cap->entry=e;cap->bytes+=bytes;r->catalog_bytes+=bytes;++r->generation;
    /* UI_TEXT_COMMAND may replace this title (up to4095 bytes). Reserve its
     * maximum within the catalog; presentation never changes cap metadata. */
    e->capability_generated=1;e->id=cap->id;e->params_schema_json=cap->input;
    e->handler=entry_execute;e->user_data=cap;e->state.size=sizeof(e->state);e->state.enabled=e->state.visible=1;
    e->next=host->commands;host->commands=e;return UI_STATUS_OK;
}
ui_status_t ui_capability_request_call(ui_host_t *host,uint64_t request,uint64_t *id)
{
    registry_t *r;call_t *c;ui_status_t s=registry(host,&r);if(s)return s;
    if(!request||!id)return UI_STATUS_INVALID_ARGUMENT;*id=0;
    for(c=r->calls;c;c=c->next)if(c->request_id==request){*id=c->id;return UI_STATUS_OK;}
    return UI_STATUS_NOT_FOUND;
}
ui_status_t ui_capabilities_reserve_request(ui_host_t *host)
{
    registry_t *r;ui_status_t s=registry(host,&r);if(s)return s;
    if(r->failure_notifying||r->entry_requests==UI_CAPABILITY_MAX_CALLS||sizeof(ui_assistant_request_t)>UI_CAPABILITY_CALL_BYTES-r->call_bytes)return UI_STATUS_LIMIT_EXCEEDED;
    ++r->entry_requests;r->call_bytes+=sizeof(ui_assistant_request_t);return UI_STATUS_OK;
}
void ui_capabilities_release_request(ui_host_t *host)
{registry_t *r=host->capabilities;--r->entry_requests;r->call_bytes-=sizeof(ui_assistant_request_t);}
ui_status_t ui_capabilities_visit_entries(ui_host_t *host,ui_assistant_command_visitor_fn visitor,void *data)
{
    registry_t *r;capability_t *c;ui_status_t s=registry(host,&r);if(s)return s;
    enter(host,r);
    for(c=r->capabilities;c;c=c->next)if(c->entry&&!c->v2&&!c->retiring&&!is_closing(r)){
        ui_assistant_command_desc_t d={0};d.size=sizeof(d);d.id=c->id;d.permission=UI_ASSISTANT_PERMISSION_READ;d.params_schema_json=c->input;
        visitor(&d,data);
    }
    leave(host,r);return UI_STATUS_OK;
}

#include "capability_v2.inc"
#include "capability_edit.inc"

#include "capability_inputs.inc"
#include "capability_component_inputs.inc"

#include "ai_profiles.inc"
#include "ai_operation_catalog.inc"
