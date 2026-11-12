#include "ui_internal.h"
#include "ui_framework/capability_ui.h"
#include "ui_framework/capability_entry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned checks,failures,observer_depth,max_observer_depth;
#define CHECK(x) do{++checks;if(!(x)){++failures;fprintf(stderr,"line %d: %s\n",__LINE__,#x);}}while(0)
static const char *input="{\"type\":\"object\",\"properties\":{\"value\":{\"type\":\"integer\"}},\"required\":[\"value\"],\"additionalProperties\":false}";
static const char *output="{\"type\":\"object\",\"properties\":{\"value\":{\"type\":\"integer\"}},\"required\":[\"value\"],\"additionalProperties\":false}";
typedef struct model {
    ui_host_t *host;unsigned revision,executions,cancels,notices,revision_calls,modal_at;
    int value,async,invalid,check_modal,admitted_modal,nested,read_in_observer,admitted_invalidate,guard_value;
    uint64_t call,binding,reconcile_binding;const char *params,*effect;void *owner;
    struct model *finish_other;unsigned terminals;
    char code[64];ui_status_t last_status;ui_cap_resolution_v2_t resolution;
} model_t;
static ui_status_t revision(char *out,size_t capacity,void *data)
{
    model_t *m=data;++m->revision_calls;
    if(m->modal_at==m->revision_calls)m->host->modal_component=(ui_component_t *)m;
    snprintf(out,capacity,"r%u",m->revision);return UI_STATUS_OK;
}
static ui_cap_result_v2_t result(model_t *m,uint64_t call)
{ui_cap_result_v2_t r={0};r.size=sizeof(r);r.version=2;CHECK(ui_capability_get_result_v2(m->host,call,&r)==UI_STATUS_OK);return r;}
static void complete(model_t *m,uint64_t call,int apply)
{
    ui_cap_completion_v2_t done={0};char json[64];
    if(apply){m->value=7;++m->revision;}
    snprintf(json,sizeof(json),"{\"value\":%d}",m->value);
    done.size=sizeof(done);done.version=2;done.outcome=UI_CAP2_OK;done.effect=apply?UI_CAP2_EFFECT_APPLIED:UI_CAP2_EFFECT_NONE;
    done.output_json=m->invalid?"{\"bad\":true}":json;
    CHECK(ui_capability_complete_v2(m->host,call,&done)==UI_STATUS_OK);
}
static ui_status_t check(const ui_cap_target_v2_t *t,const char *params,void *data)
{model_t *m=data;(void)t;(void)params;if(m->check_modal)m->host->modal_component=(ui_component_t *)m;return UI_STATUS_OK;}
static void execute(uint64_t call,const ui_cap_target_v2_t *t,const char *params,void *data)
{
    model_t *m=data;int value=0;++m->executions;CHECK(t->resource.instance_id!=0);CHECK(strstr(t->scope_json,m->effect?m->effect:"write")!=NULL);
    CHECK(sscanf_s(params,"{\"value\":%d}",&value)==1&&value==7);CHECK(result(m,call).state==UI_CAP_RUNNING);
    CHECK(m->call==call);CHECK(ui_capability_forget_v2(m->host,call)==UI_STATUS_PENDING);
    if(!m->async)complete(m,call,m->value!=value);
}
static void cancel(uint64_t call,void *data)
{model_t *m=data;(void)call;++m->cancels;}
static ui_status_t reconcile(uint64_t call,const ui_cap_target_v2_t *t,const char *params,
    ui_cap_reconciliation_v2_t *r,char *json,size_t capacity,size_t *length,void *data)
{
    model_t *m=data;CHECK(call==m->call);CHECK(strstr(params,"value")!=NULL);CHECK(*t->expected_revision!=0);
    r->resolution=m->resolution;snprintf(r->observed_revision,sizeof(r->observed_revision),"r%u",m->revision);
    *length=(size_t)snprintf(json,capacity,"{\"value\":%d}",m->value);return UI_STATUS_OK;
}
static ui_status_t collect(const ui_cap_ui_context_t *ctx,char *json,size_t capacity,size_t *length,void *data)
{model_t *m=data;CHECK(ctx->version==1&&ctx->target.version==2);*length=(size_t)snprintf(json,capacity,"%s",m->params);return UI_STATUS_OK;}
static void read_execute(uint64_t call,const ui_cap_target_t *t,const char *params,void *data)
{
    model_t *m=data;ui_cap_result_t r={0};char json[64];(void)t;(void)params;
    snprintf(json,sizeof(json),"{\"value\":%d}",m->value);r.size=sizeof(r);r.version=1;r.outcome=UI_CAP_OK;r.output_json=json;
    CHECK(ui_capability_complete(m->host,call,&r)==UI_STATUS_OK);
}
static void readback(model_t *m)
{
    ui_cap_target_t t={0};uint64_t id=0;ui_cap_result_t r={0};char expected[64];
    t.size=sizeof(t);t.version=1;r.size=sizeof(r);r.version=1;
    CHECK(ui_capability_bind(m->host,"business",&t)==UI_STATUS_OK);
    CHECK(ui_capability_prepare(m->host,"read",&t,"{}",&id)==UI_STATUS_OK);
    CHECK(ui_capability_dispatch(m->host,id)==UI_STATUS_OK);CHECK(ui_capability_get_result(m->host,id,&r)==UI_STATUS_OK);
    snprintf(expected,sizeof(expected),"{\"value\":%d}",m->value);CHECK(r.outcome==UI_CAP_OK&&r.output_json&&!strcmp(r.output_json,expected));
    CHECK(ui_capability_forget(m->host,id)==UI_STATUS_OK);
}
static void observe(const ui_cap_ui_notice_t *n,void *data)
{
    model_t *m=data;++observer_depth;if(observer_depth>max_observer_depth)max_observer_depth=observer_depth;++m->notices;m->last_status=n->status;strcpy_s(m->code,sizeof(m->code),n->code);
    if(n->stage==UI_CAP_UI_ADMITTED){m->call=n->call_id;CHECK(result(m,m->call).state==UI_CAP_PREPARED);
      CHECK(ui_capability_forget_v2(m->host,m->call)==UI_STATUS_PENDING);
      if(m->admitted_modal)m->host->modal_component=(ui_component_t *)m;
      if(m->admitted_invalidate){ui_capabilities_human_begin(m->host,m->owner);ui_capabilities_human_end(m->host);}}
    if(n->stage==UI_CAP_UI_FINISHED){++m->terminals;if(m->finish_other){model_t *other=m->finish_other;m->finish_other=NULL;complete(other,other->call,1);}
      CHECK(n->call_id==m->call);CHECK(ui_capability_forget_v2(m->host,n->call_id)==UI_STATUS_PENDING);}
    if(n->stage==UI_CAP_UI_RECONCILED){CHECK(n->evidence_json&&strstr(n->evidence_json,"value"));CHECK(ui_capability_forget_v2(m->host,n->call_id)==UI_STATUS_PENDING);}
    if(m->nested)CHECK(ui_capabilities_ui_action(m->host,"apply",m->owner)==UI_STATUS_LIMIT_EXCEEDED);
    if(m->read_in_observer)readback(m);
    --observer_depth;
}
static ui_capability_desc_v2_t descriptor(model_t *m,const char *id)
{
    ui_capability_desc_v2_t d={0};d.size=sizeof(d);d.version=2;d.capability_version=1;d.id=id;d.name_utf8=id;d.description_utf8="";
    d.category="test";d.input_schema_json=input;d.output_schema_json=output;d.reconcile_schema_json=output;
    d.target_kind=UI_CAP_DOCUMENT;d.effect=m->effect?m->effect:"write";d.check=check;d.execute=execute;d.cancel=cancel;d.reconcile=reconcile;d.user_data=m;return d;
}
static void menu(model_t *m,const char *id,const char *command)
{ui_menu_item_desc_t d={0};d.size=sizeof(d);d.id=id;d.title=id;d.command_id=command;d.menu_path="Test";CHECK(ui_host_register_menu_item(m->host,&d)==UI_STATUS_OK);}
static uint64_t binding(model_t *m,const char *item,uint64_t original)
{
    ui_cap_ui_binding_desc_t b={0};uint64_t id=0;b.size=sizeof(b);b.version=1;b.capability_id="set";
    b.resource.size=sizeof(b.resource);b.resource.version=2;CHECK(ui_capability_resource_ref_v2(m->host,"business",&b.resource)==UI_STATUS_OK);
    b.menu_item_id=item;b.purpose=original?UI_CAP_UI_RECONCILE:UI_CAP_UI_INVOKE;b.reconcile_call_id=original;
    b.collect=original?NULL:collect;b.observe=observe;b.user_data=m;CHECK(ui_capability_ui_bind(m->host,&b,&id)==UI_STATUS_OK);return id;
}
static void init(model_t *m)
{
    ui_host_config_t config={0};ui_cap_resource_desc_t r={0};ui_capability_desc_t rd={0};ui_capability_desc_v2_t d;
    config.size=sizeof(config);config.api_version=10;m->host=ui_host_create(&config);CHECK(m->host!=NULL);m->params="{\"value\":7}";m->owner=m;
    r.size=sizeof(r);r.version=1;r.kind=UI_CAP_DOCUMENT;r.id="business";r.get_revision=revision;r.user_data=m;
    CHECK(ui_capability_register_resource(m->host,&r)==UI_STATUS_OK);d=descriptor(m,"set");
    CHECK(ui_capability_register_v2(m->host,&d)==UI_STATUS_PERMISSION_DENIED);
    CHECK(ui_capability_enable_read_entries(m->host)==UI_STATUS_OK);CHECK(ui_capability_register_v2(m->host,&d)==UI_STATUS_OK);
    CHECK(ui_capability_register_v2(m->host,&d)==UI_STATUS_ALREADY_EXISTS);
    rd.size=sizeof(rd);rd.version=1;rd.capability_version=1;rd.id="read";rd.name_utf8="read";rd.description_utf8="";rd.category="test";
    rd.input_schema_json="{\"type\":\"object\",\"properties\":{},\"additionalProperties\":false}";rd.output_schema_json=output;
    rd.target_kind=UI_CAP_DOCUMENT;rd.effect="read";rd.execute=read_execute;rd.user_data=m;CHECK(ui_capability_register(m->host,&rd)==UI_STATUS_OK);
    menu(m,"apply","set");menu(m,"review","set");m->binding=binding(m,"apply",0);
}
static ui_status_t action(model_t *m,const char *item)
{ui_status_t s;ui_capabilities_human_begin(m->host,m->owner);s=ui_capabilities_ui_action(m->host,item,m->owner);ui_capabilities_human_end(m->host);return s;}
static void finish_host(model_t *m)
{m->host->modal_component=NULL;CHECK(ui_capability_quiesce(m->host)==UI_STATUS_OK);ui_host_destroy(m->host);}
static void basic_case(void)
{
    model_t m={0};ui_cap_result_t old={0};ui_cap_completion_v2_t done={0};ui_cap_target_t t={0};ui_cap_entry_binding_t entry={0};
    char *json=NULL;uint64_t generation=0,id=0;ui_assistant_t *assistant;ui_cap_result_v2_t r;
    init(&m);CHECK(ui_capability_discover(m.host,NULL,0,32,&generation,&json)==UI_STATUS_OK);CHECK(!strstr(json,"\"write\""));ui_capability_release(json);
    generation=0;CHECK(ui_capability_discover_v2(m.host,NULL,0,32,&generation,&json)==UI_STATUS_OK);CHECK(strstr(json,"\"write\""));ui_capability_release(json);
    CHECK(ui_capabilities_ui_action(m.host,"apply",m.owner)==UI_STATUS_PERMISSION_DENIED&&m.executions==0);
    assistant=ui_assistant_create(m.host,NULL);CHECK(ui_assistant_invoke(assistant,"set",m.params,&id)==UI_STATUS_PERMISSION_DENIED&&id==0);ui_assistant_destroy(assistant);
    CHECK(ui_host_invoke(m.host,"set",m.params,"human")!=0&&m.executions==0);
    t.size=sizeof(t);t.version=1;CHECK(ui_capability_bind(m.host,"business",&t)==UI_STATUS_OK);
    CHECK(ui_capability_prepare(m.host,"set",&t,m.params,&id)==UI_STATUS_PERMISSION_DENIED);
    entry.size=sizeof(entry);entry.version=1;entry.capability_id="set";entry.target=&t;CHECK(ui_capability_publish_read_entry(m.host,&entry)==UI_STATUS_UNSUPPORTED);
    m.nested=1;m.read_in_observer=1;CHECK(action(&m,"apply")==UI_STATUS_OK);m.nested=0;m.read_in_observer=0;
    r=result(&m,m.call);CHECK(m.executions==1&&m.value==7&&m.revision==1&&r.outcome==UI_CAP2_OK&&r.effect==UI_CAP2_EFFECT_APPLIED);
    CHECK(!strcmp(r.revision_before,"r0")&&!strcmp(r.revision_after,"r1"));readback(&m);
    old.size=sizeof(old);old.version=1;CHECK(ui_capability_get_result(m.host,m.call,&old)==UI_STATUS_UNSUPPORTED);
    CHECK(ui_capability_cancel(m.host,m.call)==UI_STATUS_UNSUPPORTED);CHECK(ui_capability_forget(m.host,m.call)==UI_STATUS_UNSUPPORTED);
    CHECK(ui_capability_dispatch(m.host,m.call)==UI_STATUS_PERMISSION_DENIED&&m.executions==1);
    done.size=sizeof(done);done.version=2;done.outcome=UI_CAP2_OK;done.output_json="{\"value\":7}";
    CHECK(ui_capability_complete_v2(m.host,m.call,&done)==UI_STATUS_ALREADY_EXISTS);CHECK(ui_capability_forget_v2(m.host,m.call)==UI_STATUS_OK);
    CHECK(ui_capabilities_ui_action(m.host,"apply",m.owner)==UI_STATUS_PERMISSION_DENIED&&m.executions==1);
    CHECK(action(&m,"apply")==UI_STATUS_OK);CHECK(result(&m,m.call).effect==UI_CAP2_EFFECT_NONE&&m.revision==1);
    CHECK(ui_capability_forget_v2(m.host,m.call)==UI_STATUS_OK);finish_host(&m);
}
static void rejection_case(void)
{
    model_t m={0};unsigned i;init(&m);
    m.params="{\"value\":7,\"value\":7}";CHECK(action(&m,"apply")==UI_STATUS_VALIDATION_FAILED&&m.executions==0);m.params="{\"value\":7}";
    for(i=0;i<3;++i){m.check_modal=i==0;m.admitted_modal=i==1;m.modal_at=i==2?m.revision_calls+3:0;
      CHECK(action(&m,"apply")==UI_STATUS_OK);CHECK(m.executions==0&&m.value==0&&result(&m,m.call).outcome==UI_CAP2_REJECTED);
      m.host->modal_component=NULL;CHECK(ui_capability_forget_v2(m.host,m.call)==UI_STATUS_OK);}
    m.check_modal=m.admitted_modal=0;m.modal_at=0;CHECK(action(&m,"apply")==UI_STATUS_OK);CHECK(m.executions==1&&m.value==7);
    CHECK(ui_capability_forget_v2(m.host,m.call)==UI_STATUS_OK);finish_host(&m);
}
static void async_unknown_case(void)
{
    model_t m={0};uint64_t original;ui_cap_result_v2_t r;init(&m);m.async=1;
    CHECK(action(&m,"apply")==UI_STATUS_OK);original=m.call;CHECK(result(&m,original).state==UI_CAP_RUNNING);
    CHECK(action(&m,"apply")==UI_STATUS_PERMISSION_DENIED&&m.executions==1);CHECK(!strcmp(m.code,"busy"));
    CHECK(ui_capability_cancel_v2(m.host,original)==UI_STATUS_OK&&m.cancels==1);CHECK(action(&m,"apply")==UI_STATUS_PERMISSION_DENIED);
    m.invalid=1;complete(&m,original,1);r=result(&m,original);CHECK(r.outcome==UI_CAP2_UNKNOWN&&r.effect==UI_CAP2_EFFECT_UNKNOWN);
    CHECK(ui_capability_forget_v2(m.host,original)==UI_STATUS_PENDING);CHECK(action(&m,"apply")==UI_STATUS_PERMISSION_DENIED&&m.executions==1);
    m.reconcile_binding=binding(&m,"review",original);CHECK(action(&m,"review")==UI_STATUS_PENDING);CHECK(result(&m,original).resolution==UI_CAP2_UNRESOLVED);
    m.resolution=UI_CAP2_RESOLVED_APPLIED;CHECK(action(&m,"review")==UI_STATUS_OK);r=result(&m,original);
    CHECK(r.outcome==UI_CAP2_UNKNOWN&&r.resolution==UI_CAP2_RESOLVED_APPLIED&&strlen(r.evidence_sha256)==64);
    CHECK(ui_capability_forget_v2(m.host,original)==UI_STATUS_OK);m.invalid=0;CHECK(action(&m,"apply")==UI_STATUS_OK);complete(&m,m.call,0);
    CHECK(ui_capability_forget_v2(m.host,m.call)==UI_STATUS_OK);finish_host(&m);
}
typedef struct large_result {ui_host_t *host;const char *payload;ui_status_t last;} large_result_t;
static void large_execute(uint64_t id,const ui_cap_target_t *target,const char *params,void *data)
{
    large_result_t *m=data;ui_cap_result_t r={0};(void)target;(void)params;
    r.size=sizeof(r);r.version=1;r.outcome=UI_CAP_OK;r.output_json=m->payload;m->last=ui_capability_complete(m->host,id,&r);
}
static void budget_case(int bytes)
{
    model_t m={0};ui_cap_target_t target={0};uint64_t ids[UI_CAPABILITY_MAX_CALLS],extra=0,unknown;
    unsigned n=0,i;ui_cap_result_t old={0};large_result_t large={0};char *payload=NULL;
    init(&m);m.invalid=1;CHECK(action(&m,"apply")==UI_STATUS_OK);unknown=m.call;
    CHECK(result(&m,unknown).outcome==UI_CAP2_UNKNOWN);m.reconcile_binding=binding(&m,"review",unknown);
    target.size=sizeof(target);target.version=1;CHECK(ui_capability_bind(m.host,"business",&target)==UI_STATUS_OK);
    if(bytes){ui_capability_desc_t d={0};payload=malloc(UI_CAPABILITY_OUTPUT_BYTES+1u);CHECK(payload!=NULL);
      if(!payload){finish_host(&m);return;}payload[0]='"';memset(payload+1,'x',UI_CAPABILITY_OUTPUT_BYTES-2u);payload[UI_CAPABILITY_OUTPUT_BYTES-1u]='"';payload[UI_CAPABILITY_OUTPUT_BYTES]=0;
      large.host=m.host;large.payload=payload;d.size=sizeof(d);d.version=1;d.capability_version=1;d.id="large";d.name_utf8="Large";d.description_utf8="";d.category="test";
      d.input_schema_json="{\"type\":\"object\",\"properties\":{},\"additionalProperties\":false}";d.output_schema_json="{\"type\":\"string\"}";
      d.target_kind=UI_CAP_DOCUMENT;d.effect="read";d.execute=large_execute;d.user_data=&large;CHECK(ui_capability_register(m.host,&d)==UI_STATUS_OK);}
    while(n<UI_CAPABILITY_MAX_CALLS){ui_status_t status=ui_capability_prepare(m.host,bytes?"large":"read",&target,"{}",&ids[n]);
      if(status==UI_STATUS_LIMIT_EXCEEDED)break;CHECK(status==UI_STATUS_OK);++n;
      if(bytes){CHECK(ui_capability_dispatch(m.host,ids[n-1])==UI_STATUS_OK);CHECK(large.last==UI_STATUS_OK||large.last==UI_STATUS_LIMIT_EXCEEDED);}}
    CHECK(n>0);CHECK(bytes?n<UI_CAPABILITY_MAX_CALLS:n==UI_CAPABILITY_MAX_CALLS-1);
    CHECK(ui_capability_prepare(m.host,"read",&target,"{}",&extra)==UI_STATUS_LIMIT_EXCEEDED&&extra==0);
    CHECK(ui_capability_forget_v2(m.host,unknown)==UI_STATUS_PENDING);m.resolution=UI_CAP2_RESOLVED_APPLIED;
    CHECK(action(&m,"review")==UI_STATUS_OK);CHECK(result(&m,unknown).resolution==UI_CAP2_RESOLVED_APPLIED);
    CHECK(ui_capability_forget_v2(m.host,unknown)==UI_STATUS_OK);
    old.size=sizeof(old);old.version=1;
    for(i=0;i<n;++i){CHECK(ui_capability_get_result(m.host,ids[i],&old)==UI_STATUS_OK);
      if(old.state!=UI_CAP_FINISHED){CHECK(ui_capability_cancel(m.host,ids[i])==UI_STATUS_OK);CHECK(ui_capability_get_result(m.host,ids[i],&old)==UI_STATUS_OK);
        if(old.state==UI_CAP_RUNNING){old.outcome=UI_CAP_CANCELLED;old.output_json=NULL;CHECK(ui_capability_complete(m.host,ids[i],&old)==UI_STATUS_OK);}}
      CHECK(ui_capability_forget(m.host,ids[i])==UI_STATUS_OK);}
    free(payload);m.invalid=0;CHECK(action(&m,"apply")==UI_STATUS_OK);CHECK(result(&m,m.call).outcome==UI_CAP2_OK);
    CHECK(ui_capability_forget_v2(m.host,m.call)==UI_STATUS_OK);finish_host(&m);
}
static void notification_case(void)
{
    model_t m={0},n={0};ui_capability_desc_v2_t d;ui_cap_resource_desc_t res={0};ui_cap_ui_binding_desc_t b={0};
    init(&m);m.async=1;n.host=m.host;n.async=1;n.params="{\"value\":7}";n.owner=&n;
    res.size=sizeof(res);res.version=1;res.id="other";res.kind=UI_CAP_DOCUMENT;res.get_revision=revision;res.user_data=&n;
    CHECK(ui_capability_register_resource(m.host,&res)==UI_STATUS_OK);d=descriptor(&n,"second");CHECK(ui_capability_register_v2(m.host,&d)==UI_STATUS_OK);
    menu(&m,"second-apply","second");b.size=sizeof(b);b.version=1;b.capability_id="second";b.resource.size=sizeof(b.resource);b.resource.version=2;
    CHECK(ui_capability_resource_ref_v2(m.host,"other",&b.resource)==UI_STATUS_OK);b.menu_item_id="second-apply";b.purpose=UI_CAP_UI_INVOKE;b.collect=collect;b.observe=observe;b.user_data=&n;
    CHECK(ui_capability_ui_bind(m.host,&b,&n.binding)==UI_STATUS_OK);CHECK(action(&m,"apply")==UI_STATUS_OK);CHECK(action(&n,"second-apply")==UI_STATUS_OK);
    m.finish_other=&n;complete(&m,m.call,1);CHECK(m.terminals==1&&n.terminals==1&&max_observer_depth==1);
    CHECK(ui_capability_forget_v2(m.host,m.call)==UI_STATUS_OK);CHECK(ui_capability_forget_v2(m.host,n.call)==UI_STATUS_OK);finish_host(&m);
}
static void incarnation_case(void)
{
    model_t m={0};ui_capability_desc_v2_t d;ui_cap_resource_desc_t res={0};init(&m);
    CHECK(ui_capability_begin_unregistration(m.host,"set")==UI_STATUS_OK);CHECK(ui_capability_finish_unregistration(m.host,"set")==UI_STATUS_OK);
    d=descriptor(&m,"set");CHECK(ui_capability_register_v2(m.host,&d)==UI_STATUS_OK);
    CHECK(action(&m,"apply")==UI_STATUS_NOT_FOUND&&m.executions==0);
    CHECK(ui_capability_ui_begin_unbind(m.host,m.binding)==UI_STATUS_OK);CHECK(ui_capability_ui_finish_unbind(m.host,m.binding)==UI_STATUS_OK);m.binding=binding(&m,"apply",0);
    CHECK(ui_capability_begin_resource_unregistration(m.host,"business")==UI_STATUS_OK);CHECK(ui_capability_finish_resource_unregistration(m.host,"business")==UI_STATUS_OK);
    res.size=sizeof(res);res.version=1;res.id="business";res.kind=UI_CAP_DOCUMENT;res.get_revision=revision;res.user_data=&m;
    CHECK(ui_capability_register_resource(m.host,&res)==UI_STATUS_OK);CHECK(action(&m,"apply")==UI_STATUS_NOT_FOUND&&m.executions==0);
    CHECK(ui_capability_ui_begin_unbind(m.host,m.binding)==UI_STATUS_OK);CHECK(ui_capability_ui_finish_unbind(m.host,m.binding)==UI_STATUS_OK);m.binding=binding(&m,"apply",0);
    CHECK(action(&m,"apply")==UI_STATUS_OK&&m.value==7);CHECK(ui_capability_forget_v2(m.host,m.call)==UI_STATUS_OK);finish_host(&m);
    memset(&m,0,sizeof(m));init(&m);m.invalid=1;CHECK(action(&m,"apply")==UI_STATUS_OK);CHECK(result(&m,m.call).outcome==UI_CAP2_UNKNOWN);finish_host(&m);
}
static void isolated_effect_case(void)
{
    model_t m={0},models[2]={{0}};const char *effects[]={"view","draft"};size_t i;init(&m);
    for(i=0;i<2;++i){model_t *n=&models[i];ui_cap_resource_desc_t res={0};ui_capability_desc_v2_t d;ui_cap_ui_binding_desc_t b={0};
      n->host=m.host;n->effect=effects[i];n->params="{\"value\":7}";n->owner=n;
      res.size=sizeof(res);res.version=1;res.id=effects[i];res.kind=UI_CAP_DOCUMENT;res.get_revision=revision;res.user_data=n;
      CHECK(ui_capability_register_resource(m.host,&res)==UI_STATUS_OK);d=descriptor(n,effects[i]);CHECK(ui_capability_register_v2(m.host,&d)==UI_STATUS_OK);
      menu(n,effects[i],effects[i]);b.size=sizeof(b);b.version=1;b.capability_id=effects[i];b.menu_item_id=effects[i];b.purpose=UI_CAP_UI_INVOKE;
      b.resource.size=sizeof(b.resource);b.resource.version=2;CHECK(ui_capability_resource_ref_v2(m.host,effects[i],&b.resource)==UI_STATUS_OK);
      b.collect=collect;b.observe=observe;b.user_data=n;CHECK(ui_capability_ui_bind(m.host,&b,&n->binding)==UI_STATUS_OK);
      CHECK(action(n,effects[i])==UI_STATUS_OK&&n->value==7&&n->revision==1);CHECK(m.value==0&&m.revision==0);readback(&m);
      CHECK(result(n,n->call).effect==UI_CAP2_EFFECT_APPLIED);CHECK(ui_capability_forget_v2(m.host,n->call)==UI_STATUS_OK);
    }
    finish_host(&m);
}
static int guard_valid(void *data){return ((model_t *)data)->guard_value;}
static void final_admission_case(void)
{
    model_t m={0};init(&m);m.admitted_invalidate=1;
    CHECK(action(&m,"apply")==UI_STATUS_OK);CHECK(m.executions==0&&result(&m,m.call).outcome==UI_CAP2_REJECTED);CHECK(!strcmp(result(&m,m.call).code,"credential_invalid"));
    CHECK(ui_capability_forget_v2(m.host,m.call)==UI_STATUS_OK);m.admitted_invalidate=0;m.call=0;
    ui_capabilities_human_begin(m.host,m.owner);m.params="{}";
    CHECK(ui_capabilities_ui_action(m.host,"apply",m.owner)==UI_STATUS_VALIDATION_FAILED);m.params="{\"value\":7}";
    CHECK(ui_capabilities_ui_action(m.host,"apply",m.owner)==UI_STATUS_PERMISSION_DENIED&&m.executions==0);ui_capabilities_human_end(m.host);
    if(m.call&&result(&m,m.call).state==UI_CAP_FINISHED)(void)ui_capability_forget_v2(m.host,m.call);
    m.guard_value=0;ui_capabilities_human_begin(m.host,m.owner);ui_capabilities_human_guard(m.host,guard_valid,&m);
    CHECK(ui_capabilities_ui_action(m.host,"apply",m.owner)==UI_STATUS_PERMISSION_DENIED);ui_capabilities_human_end(m.host);
    finish_host(&m);
}
static void long_discovery_case(void)
{
    model_t m={0};ui_capability_desc_v2_t d;char description[4097],*page=NULL;uint64_t generation=0;
    init(&m);memset(description,'x',sizeof(description)-1);description[sizeof(description)-1]=0;
    d=descriptor(&m,"long.metadata");d.name_utf8="A capability whose metadata exceeds a local formatting buffer";d.description_utf8=description;
    CHECK(ui_capability_register_v2(m.host,&d)==UI_STATUS_OK);
    CHECK(ui_capability_discover_v2(m.host,NULL,0,32,&generation,&page)==UI_STATUS_OK);
    CHECK(page&&strstr(page,description)&&strstr(page,"descriptor_hash")&&strstr(page,"long.metadata"));ui_capability_release(page);finish_host(&m);
}
int main(void)
{long_discovery_case();final_admission_case();basic_case();rejection_case();async_unknown_case();budget_case(0);budget_case(1);notification_case();incarnation_case();isolated_effect_case();printf("C2-A checks=%u failures=%u\n",checks,failures);return failures?1:0;}
