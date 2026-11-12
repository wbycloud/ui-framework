#include "ui_framework/capability_entry.h"
#include "ui_framework/assistant.h"
#include "ui_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int checks,failures;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);++failures;}}while(0)
static const char empty[]="{\"type\":\"object\",\"additionalProperties\":false}";
static const char output[]="{\"type\":\"object\",\"properties\":{\"value\":{\"type\":\"integer\"}},\"required\":[\"value\"],\"additionalProperties\":false}";
typedef struct model {
    ui_host_t *host;ui_assistant_t *assistant;
    unsigned revision,executions,results,successes,cancels,manual,views,legacy_cancels;
    int async,bad,nested,disable_on_revision,deny,result_unregister;
    uint64_t request,pending;char last[128];
} model_t;
static ui_cap_result_t result(model_t *m,uint64_t id)
{ui_cap_result_t r={0};r.size=sizeof(r);r.version=1;CHECK(ui_capability_get_result(m->host,id,&r)==UI_STATUS_OK);return r;}
static ui_status_t revision(char *p,size_t n,void *user)
{
    model_t *m=user;(void)snprintf(p,n,"%u",m->revision);
    if(m->disable_on_revision){ui_command_state_t s={0};s.size=sizeof(s);s.visible=1;CHECK(ui_host_set_command_state(m->host,"read",&s)==UI_STATUS_OK);}
    return UI_STATUS_OK;
}
static ui_status_t available(const ui_cap_target_t *t,const char *p,void *user)
{model_t *m=user;(void)t;(void)p;return m->deny?UI_STATUS_PERMISSION_DENIED:UI_STATUS_OK;}
static void completed(const ui_result_t *r,void *user)
{
    model_t *m=user;uint64_t call=0;++m->results;if(r->success)++m->successes;
    (void)snprintf(m->last,sizeof(m->last),"%s",r->result_json);
    if(ui_capability_request_call(m->host,r->request_id,&call)==UI_STATUS_OK){
        CHECK(result(m,call).state==UI_CAP_FINISHED);
        CHECK(ui_capability_forget(m->host,call)==UI_STATUS_PENDING);
        if(m->assistant&&r->request_id==m->request)CHECK(ui_assistant_forget_request(m->assistant,r->request_id)==UI_STATUS_PENDING);
        CHECK(ui_capability_dispatch(m->host,call)==UI_STATUS_OK);
        if(m->result_unregister){CHECK(ui_capability_begin_unregistration(m->host,"read")==UI_STATUS_OK);CHECK(ui_capability_finish_unregistration(m->host,"read")==UI_STATUS_PENDING);}
        ui_host_destroy(m->host); /* Callback lease must retain the host. */
        CHECK(result(m,call).state==UI_CAP_FINISHED);
    }
}
static void execute(uint64_t id,const ui_cap_target_t *t,const char *p,void *user)
{
    model_t *m=user;ui_cap_result_t r={0};uint64_t mapped=0;++m->executions;m->pending=id;
    CHECK(!strcmp(t->resource_id,"document-a")&&!strcmp(t->expected_revision,"1"));CHECK(!strcmp(p,"{}"));
    CHECK(result(m,id).state==UI_CAP_RUNNING);
    if(m->request)CHECK(ui_capability_request_call(m->host,m->request,&mapped)==UI_STATUS_OK&&mapped==id);
    if(m->nested){uint64_t request=ui_host_invoke(m->host,"read","{}","component"),call=0;CHECK(request!=0);CHECK(ui_capability_request_call(m->host,request,&call)==UI_STATUS_OK);CHECK(result(m,call).outcome==UI_CAP_CANCELLED);}
    if(m->async)return;
    r.size=sizeof(r);r.version=1;r.outcome=UI_CAP_OK;r.output_json=m->bad?"{\"value\":false}":"{\"value\":42}";
    CHECK(ui_capability_complete(m->host,id,&r)==UI_STATUS_OK);
    CHECK(ui_capability_complete(m->host,id,&r)==UI_STATUS_ALREADY_EXISTS);
    CHECK(ui_capability_forget(m->host,id)==UI_STATUS_PENDING);
    CHECK(!strcmp(t->resource_id,"document-a")&&!strcmp(p,"{}"));
}
static void cancel(uint64_t id,void *user)
{model_t *m=user;CHECK(result(m,id).state==UI_CAP_RUNNING);++m->cancels;}
static void legacy_cancel(uint64_t id,void *user)
{model_t *m=user;(void)id;++m->legacy_cancels;}
static void manual(ui_host_t *h,uint64_t id,const char *c,const char *p,const char *s,void *user)
{model_t *m=user;(void)c;(void)p;(void)s;++m->manual;CHECK(ui_host_reply(h,id,1,"{}")==UI_STATUS_OK);}
static void hold(ui_host_t *h,uint64_t id,const char *c,const char *p,const char *s,void *user)
{(void)h;(void)id;(void)c;(void)p;(void)s;(void)user;}
static ui_capability_desc_t descriptor(model_t *m,const char *id)
{ui_capability_desc_t d={0};d.size=sizeof(d);d.version=d.capability_version=1;d.id=id;d.name_utf8="Read";d.description_utf8="C1 fixture";d.category="test";d.input_schema_json=empty;d.output_schema_json=output;d.effect="read";d.target_kind=UI_CAP_DOCUMENT;d.check=available;d.execute=execute;d.cancel=cancel;d.user_data=m;return d;}
static ui_cap_target_t bind(model_t *m)
{ui_cap_target_t t={0};t.size=sizeof(t);t.version=1;CHECK(ui_capability_bind(m->host,"document-a",&t)==UI_STATUS_OK);return t;}
static void view(const ui_assistant_command_desc_t *d,void *user)
{model_t *m=user;++m->views;CHECK(!strcmp(d->id,"read"));CHECK(d->permission==UI_ASSISTANT_PERMISSION_READ&&!d->validate&&!d->user_data);CHECK(!strcmp(d->params_schema_json,"{\"additionalProperties\":false,\"type\":\"object\"}"));}
static void init(model_t *m)
{ui_host_config_t h={0};ui_cap_resource_desc_t r={0};h.size=sizeof(h);h.api_version=10;h.result_callback=completed;h.user_data=m;m->host=ui_host_create(&h);m->revision=1;CHECK(m->host!=NULL);r.size=sizeof(r);r.version=1;r.kind=UI_CAP_DOCUMENT;r.id="document-a";r.get_revision=revision;r.user_data=m;CHECK(ui_capability_register_resource(m->host,&r)==UI_STATUS_OK);}
static void publish(model_t *m,const char *id,ui_cap_target_t *t)
{ui_cap_entry_binding_t b={0};b.size=sizeof(b);b.version=1;b.capability_id=id;b.target=t;CHECK(ui_capability_publish_read_entry(m->host,&b)==UI_STATUS_OK);}
static uint64_t invoke(model_t *m,int assistant)
{uint64_t request=0,call=0;m->request=0;if(assistant){CHECK(ui_assistant_invoke(m->assistant,"read","{}",&m->request)==UI_STATUS_OK);request=m->request;}else request=ui_host_invoke(m->host,"read","{}","manual");CHECK(request!=0);CHECK(ui_capability_request_call(m->host,request,&call)==UI_STATUS_OK);return call;}
static void finish(model_t *m,uint64_t call,ui_cap_outcome_t outcome)
{ui_cap_result_t r={0};r.size=sizeof(r);r.version=1;r.outcome=outcome;r.output_json="{\"value\":42}";CHECK(ui_capability_complete(m->host,call,&r)==UI_STATUS_OK);}
static void budget_case(void)
{
    model_t m={0};ui_capability_desc_t d;ui_cap_target_t t;ui_cap_result_t r={0};
    uint64_t ids[UI_CAPABILITY_MAX_CALLS],requests[UI_CAPABILITY_MAX_CALLS],request=0,id=0;
    unsigned i,done=0,before;char *payload=malloc(UI_CAPABILITY_OUTPUT_BYTES+1u);
    init(&m);m.async=1;d=descriptor(&m,"large");d.output_schema_json="{\"type\":\"string\"}";
    CHECK(ui_capability_register(m.host,&d)==UI_STATUS_OK);t=bind(&m);CHECK(ui_capability_enable_read_entries(m.host)==UI_STATUS_OK);publish(&m,"large",&t);CHECK(payload!=NULL);
    if(payload){payload[0]='"';memset(payload+1,'x',UI_CAPABILITY_OUTPUT_BYTES-2u);payload[UI_CAPABILITY_OUTPUT_BYTES-1u]='"';payload[UI_CAPABILITY_OUTPUT_BYTES]=0;
        r.size=sizeof(r);r.version=1;r.outcome=UI_CAP_OK;r.output_json=payload;
        for(i=0;i<40;++i){ui_status_t s;request=ui_host_invoke(m.host,"large","{}","test");CHECK(request!=0);CHECK(ui_capability_request_call(m.host,request,&ids[i])==UI_STATUS_OK);before=m.results;
            s=ui_capability_complete(m.host,ids[i],&r);
            if(s==UI_STATUS_LIMIT_EXCEEDED){CHECK(result(&m,ids[i]).state==UI_CAP_RUNNING&&m.results==before);break;}
            CHECK(s==UI_STATUS_OK&&m.results==before+1);++done;
        }
        CHECK(done>0&&done<40);
        if(done>0&&done<40){CHECK(ui_capability_forget(m.host,ids[0])==UI_STATUS_OK);CHECK(ui_capability_complete(m.host,ids[done],&r)==UI_STATUS_OK);CHECK(ui_capability_complete(m.host,ids[done],&r)==UI_STATUS_ALREADY_EXISTS);for(i=1;i<=done;++i)CHECK(ui_capability_forget(m.host,ids[i])==UI_STATUS_OK);}
        free(payload);
    }
    for(i=0;i<UI_CAPABILITY_MAX_CALLS;++i){request=ui_host_invoke(m.host,"large","{}","test");CHECK(request!=0);CHECK(ui_capability_request_call(m.host,request,&ids[i])==UI_STATUS_OK);}
    before=m.executions;request=ui_host_invoke(m.host,"large","{}","test");CHECK(request&&m.executions==before);CHECK(ui_capability_request_call(m.host,request,&id)==UI_STATUS_NOT_FOUND&&strstr(m.last,"status"));
    for(i=0;i<UI_CAPABILITY_MAX_CALLS;++i){CHECK(ui_capability_cancel(m.host,ids[i])==UI_STATUS_OK);finish(&m,ids[i],UI_CAP_CANCELLED);CHECK(ui_capability_forget(m.host,ids[i])==UI_STATUS_OK);}
    m.assistant=ui_assistant_create(m.host,NULL);CHECK(m.assistant!=NULL);
    for(i=0;i<UI_CAPABILITY_MAX_CALLS;++i)CHECK(ui_assistant_invoke(m.assistant,"large","{\"unknown\":1}",&requests[i])==UI_STATUS_OK&&requests[i]);
    CHECK(ui_assistant_invoke(m.assistant,"large","{\"unknown\":1}",&request)==UI_STATUS_LIMIT_EXCEEDED&&request==0);
    {ui_assistant_t *second=ui_assistant_create(m.host,NULL);CHECK(second!=NULL);CHECK(ui_assistant_invoke(second,"large","{\"unknown\":1}",&request)==UI_STATUS_LIMIT_EXCEEDED&&request==0);ui_assistant_destroy(second);}
    for(i=0;i<UI_CAPABILITY_MAX_CALLS;++i)CHECK(ui_assistant_forget_request(m.assistant,requests[i])==UI_STATUS_OK);
    ui_assistant_destroy(m.assistant);ui_host_destroy(m.host);
}
static void lifetime_case(void)
{
    model_t m={0};ui_capability_desc_t d;ui_cap_target_t t;ui_cap_resource_desc_t rd={0};uint64_t call,request=0;unsigned before;
    init(&m);d=descriptor(&m,"read");CHECK(ui_capability_register(m.host,&d)==UI_STATUS_OK);t=bind(&m);CHECK(ui_capability_enable_read_entries(m.host)==UI_STATUS_OK);publish(&m,"read",&t);
    m.assistant=ui_assistant_create(m.host,NULL);m.async=1;call=invoke(&m,1);
    ui_assistant_destroy(m.assistant);m.assistant=NULL; /* Bridge retains no Assistant pointer. */
    finish(&m,call,UI_CAP_OK);CHECK(result(&m,call).outcome==UI_CAP_OK);CHECK(ui_capability_forget(m.host,call)==UI_STATUS_OK);
    CHECK(ui_capability_begin_resource_unregistration(m.host,"document-a")==UI_STATUS_OK);CHECK(ui_capability_finish_resource_unregistration(m.host,"document-a")==UI_STATUS_OK);
    rd.size=sizeof(rd);rd.version=1;rd.kind=UI_CAP_DOCUMENT;rd.id="document-a";rd.get_revision=revision;rd.user_data=&m;CHECK(ui_capability_register_resource(m.host,&rd)==UI_STATUS_OK);
    before=m.executions;request=ui_host_invoke(m.host,"read","{}","human");CHECK(request!=0&&m.executions==before);CHECK(ui_capability_request_call(m.host,request,&call)==UI_STATUS_NOT_FOUND);
    /* A reopened resource cannot silently rebind the entry's old open epoch. */
    CHECK(ui_capability_begin_unregistration(m.host,"read")==UI_STATUS_OK);CHECK(ui_capability_finish_unregistration(m.host,"read")==UI_STATUS_OK);
    d=descriptor(&m,"read");CHECK(ui_capability_register(m.host,&d)==UI_STATUS_OK);t=bind(&m);publish(&m,"read",&t);
    call=invoke(&m,0);CHECK(ui_capability_quiesce(m.host)==UI_STATUS_PENDING);before=m.executions;
    request=ui_host_invoke(m.host,"read","{}","component");CHECK(request&&m.executions==before);CHECK(ui_capability_request_call(m.host,request,&m.request)==UI_STATUS_NOT_FOUND);m.request=0;
    CHECK(result(&m,call).state==UI_CAP_RUNNING&&result(&m,call).cancel_requested);finish(&m,call,UI_CAP_CANCELLED);CHECK(ui_capability_forget(m.host,call)==UI_STATUS_OK);CHECK(ui_capability_quiesce(m.host)==UI_STATUS_OK);
    m.assistant=ui_assistant_create(m.host,NULL);CHECK(m.assistant!=NULL);ui_host_destroy(m.host);ui_assistant_destroy(m.assistant); /* Detach bookkeeping must not retain a freed host. */
}
static void catalog_case(void)
{
    model_t m={0};ui_capability_desc_t d;ui_cap_target_t t;ui_cap_entry_binding_t b={0};
    char id[32],*json=NULL;unsigned i,published=0;uint64_t generation=0;
    init(&m);t=bind(&m);CHECK(ui_capability_enable_read_entries(m.host)==UI_STATUS_OK);
    b.size=sizeof(b);b.version=1;b.target=&t;
    for(i=0;i<UI_CAPABILITY_MAX_COUNT;++i){ui_status_t s;(void)snprintf(id,sizeof(id),"entry.%u",i);d=descriptor(&m,id);CHECK(ui_capability_register(m.host,&d)==UI_STATUS_OK);b.capability_id=id;
        generation=0;CHECK(ui_capability_discover(m.host,NULL,0,1,&generation,&json)==UI_STATUS_OK);ui_capability_release(json);
        s=ui_capability_publish_read_entry(m.host,&b);
        if(s==UI_STATUS_LIMIT_EXCEEDED){CHECK(ui_capability_discover(m.host,NULL,0,1,&generation,&json)==UI_STATUS_OK);ui_capability_release(json);break;}
        CHECK(s==UI_STATUS_OK);++published;
    }
    CHECK(published>0&&published<UI_CAPABILITY_MAX_COUNT); /* 2MiB includes each target and UI projection. */
    ui_host_destroy(m.host);
}
int main(void)
{
    model_t m={0},other={0};ui_capability_desc_t d;ui_cap_target_t target,wrong;
    ui_cap_entry_binding_t b={0};ui_command_desc_t old={0};ui_assistant_command_desc_t ac={0};
    ui_command_state_t state={0};ui_assistant_config_t config={0};uint64_t call,request=0,gen=0;char *json=NULL;unsigned count;
    init(&m);init(&other);config.size=sizeof(config);config.max_permission=UI_ASSISTANT_PERMISSION_DESTRUCTIVE;config.source_id="human";config.cancel=legacy_cancel;config.user_data=&m;m.assistant=ui_assistant_create(m.host,&config);CHECK(m.assistant!=NULL);
    old.size=sizeof(old);old.id="manual.edit";old.title="Edit";old.params_schema_json=empty;old.handler=manual;old.user_data=&m;
    CHECK(ui_host_register_command(m.host,&old)==UI_STATUS_OK);
    ac.size=sizeof(ac);ac.id=old.id;ac.permission=UI_ASSISTANT_PERMISSION_EDIT;ac.params_schema_json=empty;CHECK(ui_assistant_register_command(m.assistant,&ac)==UI_STATUS_OK);
    d=descriptor(&m,old.id);CHECK(ui_capability_register(m.host,&d)==UI_STATUS_ALREADY_EXISTS);
    ac.id="assistant.only";ac.permission=UI_ASSISTANT_PERMISSION_READ;CHECK(ui_assistant_register_command(m.assistant,&ac)==UI_STATUS_OK);d=descriptor(&m,ac.id);CHECK(ui_capability_register(m.host,&d)==UI_STATUS_ALREADY_EXISTS);
    d=descriptor(&m,"read");CHECK(ui_capability_register(m.host,&d)==UI_STATUS_OK);CHECK(ui_capability_register(m.host,&d)==UI_STATUS_ALREADY_EXISTS);
    old.id="read";CHECK(ui_host_register_command(m.host,&old)==UI_STATUS_ALREADY_EXISTS);ac.id="read";CHECK(ui_assistant_register_command(m.assistant,&ac)==UI_STATUS_ALREADY_EXISTS);
    target=bind(&m);b.size=sizeof(b);b.version=1;b.capability_id="read";b.target=&target;
    CHECK(ui_capability_publish_read_entry(m.host,&b)==UI_STATUS_PERMISSION_DENIED);
    CHECK(ui_capability_discover(m.host,NULL,0,32,&gen,&json)==UI_STATUS_OK);ui_capability_release(json);
    old.id="legacy.wait";old.handler=hold;CHECK(ui_host_register_command(m.host,&old)==UI_STATUS_OK);request=ui_host_invoke(m.host,old.id,"{}","manual");CHECK(request!=0);CHECK(ui_capability_enable_read_entries(m.host)==UI_STATUS_PENDING);CHECK(ui_host_reply(m.host,request,1,"{}")==UI_STATUS_OK);
    CHECK(ui_capability_enable_read_entries(m.host)==UI_STATUS_OK);CHECK(ui_capability_enable_read_entries(m.host)==UI_STATUS_OK);
    wrong=bind(&other);b.target=&wrong;CHECK(ui_capability_publish_read_entry(m.host,&b)==UI_STATUS_NOT_FOUND);b.target=&target;
    publish(&m,"read",&target);CHECK(ui_capability_publish_read_entry(m.host,&b)==UI_STATUS_ALREADY_EXISTS);
    memset(&target,0,sizeof(target)); /* Publication copied caller-owned data. */
    {uint64_t next=m.host->next_request_id;m.host->next_request_id=UINT64_MAX;CHECK(ui_host_invoke(m.host,"read","{}","human")==0);m.host->next_request_id=next;}
    CHECK(ui_capability_discover(m.host,NULL,0,32,&gen,&json)==UI_STATUS_CANCELLED);
    CHECK(ui_assistant_visit_commands(m.assistant,view,&m)==UI_STATUS_OK&&m.views==1);
    CHECK(ui_host_set_title(m.host,UI_TEXT_COMMAND,"read","Localized read") == UI_STATUS_OK);
    CHECK(ui_capability_describe(m.host,"read",&json)==UI_STATUS_OK&&strstr(json,"\"name\":\"Read\""));ui_capability_release(json);
    {ui_assistant_t *late=ui_assistant_create(m.host,NULL);CHECK(late!=NULL);CHECK(ui_assistant_visit_commands(late,view,&m)==UI_STATUS_OK&&m.views==2);ui_assistant_destroy(late);}
    CHECK(ui_assistant_invoke(m.assistant,"manual.edit","{\"source\":\"human\"}",&request)==UI_STATUS_PERMISSION_DENIED&&request==0&&m.manual==0);
    CHECK(ui_assistant_invoke(m.assistant,"assistant.only","{}",&request)==UI_STATUS_NOT_FOUND);
    ac.id="new.legacy";CHECK(ui_assistant_register_command(m.assistant,&ac)==UI_STATUS_PERMISSION_DENIED);
    CHECK(ui_host_invoke(m.host,"manual.edit","{}","button")!=0&&m.manual==1);
    call=invoke(&m,1);CHECK(m.executions==1&&result(&m,call).outcome==UI_CAP_OK&&!strcmp(m.last,"{\"value\":42}"));
    CHECK(ui_capability_dispatch(m.host,call)==UI_STATUS_OK&&m.executions==1);CHECK(ui_capability_forget(m.host,call)==UI_STATUS_OK);
    CHECK(ui_assistant_cancel(m.assistant,m.request)==UI_STATUS_OK&&m.legacy_cancels==0);CHECK(ui_assistant_forget_request(m.assistant,m.request)==UI_STATUS_OK);
    CHECK(ui_capability_request_call(m.host,m.request,&call)==UI_STATUS_NOT_FOUND);m.request=0;
    count=m.executions;request=ui_host_invoke(m.host,"read","{\"unknown\":1}","component");CHECK(request&&m.executions==count&&strstr(m.last,"status"));CHECK(ui_capability_request_call(m.host,request,&call)==UI_STATUS_NOT_FOUND);
    state.size=sizeof(state);state.visible=1;state.enabled=0;CHECK(ui_host_set_command_state(m.host,"read",&state)==UI_STATUS_OK);call=invoke(&m,0);CHECK(result(&m,call).outcome==UI_CAP_REJECTED&&m.executions==count);CHECK(ui_capability_forget(m.host,call)==UI_STATUS_OK);
    state.enabled=1;state.busy=1;CHECK(ui_host_set_command_state(m.host,"read",&state)==UI_STATUS_OK);call=invoke(&m,1);CHECK(result(&m,call).outcome==UI_CAP_REJECTED&&m.executions==count);CHECK(ui_assistant_forget_request(m.assistant,m.request)==UI_STATUS_OK);m.request=0;
    state.busy=0;CHECK(ui_host_set_command_state(m.host,"read",&state)==UI_STATUS_OK);
    m.disable_on_revision=1;call=invoke(&m,0);CHECK(result(&m,call).outcome==UI_CAP_REJECTED&&m.executions==count);CHECK(ui_capability_forget(m.host,call)==UI_STATUS_OK);m.disable_on_revision=0;CHECK(ui_host_set_command_state(m.host,"read",&state)==UI_STATUS_OK);
    m.host->modal_component=(ui_component_t *)&m;CHECK(ui_host_invoke(m.host,"read","{}","manual")==0);
    request=ui_host_invoke(m.host,"read","{}","component");CHECK(request!=0);CHECK(ui_capability_request_call(m.host,request,&call)==UI_STATUS_OK);CHECK(result(&m,call).outcome==UI_CAP_REJECTED&&m.executions==count);CHECK(ui_capability_forget(m.host,call)==UI_STATUS_OK);
    m.host->modal_component=NULL;
    m.deny=1;call=invoke(&m,0);CHECK(result(&m,call).outcome==UI_CAP_REJECTED&&m.executions==count);CHECK(ui_capability_forget(m.host,call)==UI_STATUS_OK);m.deny=0;
    m.bad=1;call=invoke(&m,0);CHECK(result(&m,call).outcome==UI_CAP_FAILED&&!strcmp(m.last,"{\"code\":\"invalid_output\"}"));CHECK(ui_capability_forget(m.host,call)==UI_STATUS_OK);m.bad=0;
    m.nested=1;call=invoke(&m,0);CHECK(result(&m,call).outcome==UI_CAP_OK);CHECK(ui_capability_forget(m.host,call)==UI_STATUS_OK);m.nested=0;
    m.async=1;call=invoke(&m,1);CHECK(result(&m,call).state==UI_CAP_RUNNING);CHECK(ui_host_reply(m.host,m.request,1,"{} ")==UI_STATUS_PERMISSION_DENIED);
    CHECK(ui_assistant_cancel(m.assistant,m.request)==UI_STATUS_OK&&m.cancels==1);CHECK(ui_assistant_cancel(m.assistant,m.request)==UI_STATUS_OK&&m.cancels==1);CHECK(result(&m,call).state==UI_CAP_RUNNING&&result(&m,call).cancel_requested);
    CHECK(ui_assistant_forget_request(m.assistant,m.request)==UI_STATUS_PENDING);finish(&m,call,UI_CAP_CANCELLED);CHECK(ui_assistant_forget_request(m.assistant,m.request)==UI_STATUS_OK);m.request=0;
    count=m.executions;++m.revision;call=invoke(&m,0);CHECK(result(&m,call).outcome==UI_CAP_STALE&&m.executions==count);CHECK(ui_capability_forget(m.host,call)==UI_STATUS_OK);--m.revision;
    /* The explicit target remains document-a even if another resource becomes
     * the apparent UI selection. No active/current target lookup is involved. */
    {ui_cap_resource_desc_t rd={0};rd.size=sizeof(rd);rd.version=1;rd.id="document-b";rd.kind=UI_CAP_DOCUMENT;rd.get_revision=revision;rd.user_data=&other;CHECK(ui_capability_register_resource(m.host,&rd)==UI_STATUS_OK);}
    call=invoke(&m,1);CHECK(result(&m,call).state==UI_CAP_RUNNING);CHECK(ui_capability_begin_unregistration(m.host,"read")==UI_STATUS_OK);CHECK(ui_capability_finish_unregistration(m.host,"read")==UI_STATUS_PENDING);m.views=0;CHECK(ui_assistant_visit_commands(m.assistant,view,&m)==UI_STATUS_OK&&m.views==0);
    CHECK(ui_assistant_invoke(m.assistant,"read","{}",&request)==UI_STATUS_NOT_FOUND);finish(&m,call,UI_CAP_CANCELLED);CHECK(ui_capability_finish_unregistration(m.host,"read")==UI_STATUS_OK);CHECK(ui_assistant_forget_request(m.assistant,m.request)==UI_STATUS_OK);m.request=0;
    CHECK(ui_capability_complete(m.host,call,NULL)==UI_STATUS_NOT_FOUND);d=descriptor(&m,"read");CHECK(ui_capability_register(m.host,&d)==UI_STATUS_OK);target=bind(&m);publish(&m,"read",&target);m.async=0;m.result_unregister=1;call=invoke(&m,1);CHECK(ui_capability_finish_unregistration(m.host,"read")==UI_STATUS_OK);CHECK(ui_assistant_forget_request(m.assistant,m.request)==UI_STATUS_OK);m.result_unregister=0;
    /* Ordinary UI remains usable after capability retirement. */
    CHECK(ui_host_invoke(m.host,"manual.edit","{}","menu")!=0&&m.manual==2);
    CHECK(ui_capability_quiesce(m.host)==UI_STATUS_OK);CHECK(ui_host_invoke(m.host,"read","{}","component")==0);
    ui_assistant_destroy(m.assistant);ui_host_destroy(m.host);ui_host_destroy(other.host);
    budget_case();
    lifetime_case();
    catalog_case();
    printf("C1 entry checks=%d failures=%d\n",checks,failures);return failures?1:0;
}
