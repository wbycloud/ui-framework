#include "ui_framework/capability_entry.h"
#include "ui_framework/assistant.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned checks,failures;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);++failures;}}while(0)
typedef struct fixture {
    ui_host_t *host;ui_assistant_t *assistant;
    unsigned executions,manual,results,transactions,confirmations,depth,max_depth;
    int enable_callback,validate_fail,confirm_fail,nest,async,bad_output;
    int legacy_observer;
    int read_in_legacy_result;
    unsigned activation_attempts;
    uint64_t generated_request;
    ui_status_t activation,nested_status;uint64_t nested_request;
    char last[96];
} fixture_t;
static const char schema[]="{\"type\":\"object\",\"additionalProperties\":false}";
static void activate(fixture_t *f)
{if(f->enable_callback)f->activation=ui_capability_enable_read_entries(f->host);}
static ui_status_t validate(const char *p,void *data)
{fixture_t *f=data;(void)p;activate(f);return f->validate_fail?UI_STATUS_INVALID_ARGUMENT:UI_STATUS_OK;}
static int confirm(const char *id,const char *p,ui_assistant_permission_t permission,void *data)
{fixture_t *f=data;(void)id;(void)p;(void)permission;++f->confirmations;activate(f);return !f->confirm_fail;}
static const char *snapshot(void *data){activate(data);return "{}";}
static void progress(uint64_t id,int percent,const char *message,void *data)
{(void)id;(void)percent;(void)message;activate(data);}
static void cancel(uint64_t id,void *data){(void)id;activate(data);}
static ui_status_t begin(const char *label,uint64_t *id,void *data)
{fixture_t *f=data;(void)label;++f->transactions;activate(f);*id=7;return UI_STATUS_OK;}
static ui_status_t end(uint64_t id,void *data)
{fixture_t *f=data;(void)id;++f->transactions;activate(f);return UI_STATUS_OK;}
static void visitor(const ui_assistant_command_desc_t *d,void *data)
{(void)d;activate(data);}
static void manual(ui_host_t *h,uint64_t id,const char *c,const char *p,const char *s,void *data)
{fixture_t *f=data;(void)c;(void)p;(void)s;++f->manual;CHECK(ui_host_reply(h,id,1,"{}")==UI_STATUS_OK);}
static void observed(const ui_result_t *r,void *data)
{
    fixture_t *f=data;++f->results;++f->depth;if(f->depth>f->max_depth)f->max_depth=f->depth;
    (void)snprintf(f->last,sizeof(f->last),"%s",r->result_json);
    if(f->legacy_observer){
        CHECK(!strcmp(r->command_id,"legacy.async")&&!strcmp(r->source,"human"));
        f->activation=ui_capability_enable_read_entries(f->host);++f->activation_attempts;
        CHECK(f->activation==UI_STATUS_PENDING);
        CHECK(ui_assistant_forget_request(f->assistant,r->request_id)==UI_STATUS_OK);
        f->activation=ui_capability_enable_read_entries(f->host);++f->activation_attempts;
        CHECK(f->activation==UI_STATUS_PENDING);
        CHECK(!strcmp(r->command_id,"legacy.async")&&!strcmp(r->source,"human")&&!strcmp(r->result_json,"{}"));
    }
    if(f->read_in_legacy_result&&!strcmp(r->command_id,"manual")){
        unsigned before=f->executions;f->read_in_legacy_result=0;
        CHECK(ui_assistant_invoke(f->assistant,"read","{}",&f->generated_request)==UI_STATUS_OK&&f->generated_request!=0);
        CHECK(f->executions==before+1); /* The legacy lease is not a cap-dispatch lease. */
    }
    /* Bounded even on the old implementation: no crash/exhaustion experiment. */
    if(f->nest&&!r->success&&f->depth<4){
        char command[256],source[256];
        (void)snprintf(command,sizeof(command),"%s",r->command_id);
        (void)snprintf(source,sizeof(source),"%s",r->source);
        CHECK(ui_host_invoke(f->host,"read","{\"invalid\":1}","observer")==0);
        f->nested_request=99;
        f->nested_status=ui_assistant_invoke(f->assistant,"read","{\"invalid\":1}",&f->nested_request);
        CHECK(f->nested_status==UI_STATUS_LIMIT_EXCEEDED&&f->nested_request==0);
        CHECK(!strcmp(command,r->command_id)&&!strcmp(source,r->source));
    }
    --f->depth;
}
static void init(fixture_t *f)
{
    ui_host_config_t h={0};ui_assistant_config_t a={0};ui_command_desc_t c={0};
    h.size=sizeof(h);h.api_version=10;h.result_callback=observed;h.user_data=f;
    f->host=ui_host_create(&h);CHECK(f->host!=NULL);
    a.size=sizeof(a);a.max_permission=UI_ASSISTANT_PERMISSION_DESTRUCTIVE;a.source_id="human";a.user_data=f;
    a.confirm=confirm;a.progress=progress;a.cancel=cancel;a.snapshot=snapshot;
    a.transaction_begin=begin;a.transaction_commit=end;a.transaction_rollback=end;a.transaction_undo=end;
    f->assistant=ui_assistant_create(f->host,&a);CHECK(f->assistant!=NULL);
    c.size=sizeof(c);c.id="manual";c.title="Manual";c.handler=manual;c.user_data=f;
    CHECK(ui_host_register_command(f->host,&c)==UI_STATUS_OK);
}
static void dispose(fixture_t *f){ui_assistant_destroy(f->assistant);ui_host_destroy(f->host);}
static void legacy(fixture_t *f,int with_validate)
{
    ui_command_desc_t c={0};ui_assistant_command_desc_t a={0};
    c.size=sizeof(c);c.id="edit";c.title="Edit";c.handler=manual;c.user_data=f;
    CHECK(ui_host_register_command(f->host,&c)==UI_STATUS_OK);
    a.size=sizeof(a);a.id="edit";a.permission=UI_ASSISTANT_PERMISSION_DESTRUCTIVE;
    a.params_schema_json=schema;a.validate=with_validate?validate:NULL;a.user_data=f;
    CHECK(ui_assistant_register_command(f->assistant,&a)==UI_STATUS_OK);
}
static void activation_case(void)
{
    int variant;for(variant=0;variant<10;++variant){
        fixture_t f={0};uint64_t id=0,tx=0;const char *text=NULL;
        init(&f);legacy(&f,variant==0);f.enable_callback=1;
        if(variant<2){CHECK(ui_assistant_invoke(f.assistant,"edit","{}",&id)==UI_STATUS_OK);CHECK(f.manual==1);CHECK(ui_assistant_forget_request(f.assistant,id)==UI_STATUS_OK);}
        else if(variant==2)CHECK(ui_assistant_visit_commands(f.assistant,visitor,&f)==UI_STATUS_OK);
        else if(variant==3)CHECK(ui_assistant_get_state_snapshot(f.assistant,&text)==UI_STATUS_OK);
        else if(variant==4)CHECK(ui_assistant_begin_transaction(f.assistant,"human",&tx)==UI_STATUS_OK);
        else if(variant==7)CHECK(ui_assistant_commit_transaction(f.assistant,7)==UI_STATUS_OK);
        else if(variant==8)CHECK(ui_assistant_rollback_transaction(f.assistant,7)==UI_STATUS_OK);
        else if(variant==9)CHECK(ui_assistant_undo_transaction(f.assistant,7)==UI_STATUS_OK);
        else {
            f.enable_callback=0;CHECK(ui_assistant_invoke(f.assistant,"edit","{}",&id)==UI_STATUS_OK);
            f.enable_callback=1;
            if(variant==5)CHECK(ui_assistant_report_progress(f.assistant,id,50,"half")==UI_STATUS_OK);
            else CHECK(ui_assistant_cancel(f.assistant,id)==UI_STATUS_OK);
            CHECK(ui_assistant_forget_request(f.assistant,id)==UI_STATUS_OK);
        }
        CHECK(f.activation==UI_STATUS_PENDING);f.enable_callback=0;
        CHECK(ui_capability_enable_read_entries(f.host)==UI_STATUS_OK);
        {unsigned before=f.manual;CHECK(ui_assistant_invoke(f.assistant,"edit","{\"source\":\"human\"}",&id)==UI_STATUS_PERMISSION_DENIED&&id==0);CHECK(f.manual==before);}
        CHECK(ui_host_invoke(f.host,"manual","{}","human")!=0);dispose(&f);
    }
    {fixture_t f={0};uint64_t id=0;init(&f);legacy(&f,1);f.enable_callback=f.validate_fail=1;
        CHECK(ui_assistant_invoke(f.assistant,"edit","{}",&id)==UI_STATUS_VALIDATION_FAILED&&id==0);
        CHECK(f.activation==UI_STATUS_PENDING&&f.manual==0);f.enable_callback=0;
        CHECK(ui_capability_enable_read_entries(f.host)==UI_STATUS_OK);
        CHECK(ui_assistant_invoke(f.assistant,"edit","{}",&id)==UI_STATUS_PERMISSION_DENIED&&f.manual==0);dispose(&f);}
    {fixture_t f={0};uint64_t id=0;init(&f);legacy(&f,0);f.enable_callback=f.confirm_fail=1;
        CHECK(ui_assistant_invoke(f.assistant,"edit","{}",&id)==UI_STATUS_PERMISSION_DENIED&&id==0);
        CHECK(f.activation==UI_STATUS_PENDING&&f.manual==0);f.enable_callback=0;
        CHECK(ui_capability_enable_read_entries(f.host)==UI_STATUS_OK);
        CHECK(ui_assistant_invoke(f.assistant,"edit","{}",&id)==UI_STATUS_PERMISSION_DENIED&&f.manual==0);dispose(&f);}
}
static void transaction_case(void)
{
    fixture_t f={0};uint64_t id=0;unsigned before;init(&f);
    CHECK(ui_assistant_begin_transaction(f.assistant,"human",&id)==UI_STATUS_OK&&id==7);
    CHECK(ui_assistant_commit_transaction(f.assistant,id)==UI_STATUS_OK);
    CHECK(ui_assistant_rollback_transaction(f.assistant,id)==UI_STATUS_OK);
    CHECK(ui_assistant_undo_transaction(f.assistant,id)==UI_STATUS_OK&&f.transactions==4);
    CHECK(ui_capability_enable_read_entries(f.host)==UI_STATUS_OK);before=f.transactions;
    CHECK(ui_assistant_begin_transaction(f.assistant,"{\"source\":\"human\",\"confirm\":true}",&id)==UI_STATUS_PERMISSION_DENIED&&id==0);
    CHECK(ui_assistant_commit_transaction(f.assistant,7)==UI_STATUS_PERMISSION_DENIED);
    CHECK(ui_assistant_rollback_transaction(f.assistant,7)==UI_STATUS_PERMISSION_DENIED);
    CHECK(ui_assistant_undo_transaction(f.assistant,7)==UI_STATUS_PERMISSION_DENIED&&f.transactions==before);
    CHECK(ui_host_invoke(f.host,"manual","{}","human")!=0&&f.manual==1);dispose(&f);
}
static ui_status_t revision(char *p,size_t n,void *data){(void)data;(void)snprintf(p,n,"1");return UI_STATUS_OK;}
static void execute(uint64_t id,const ui_cap_target_t *t,const char *p,void *data)
{
    fixture_t *f=data;ui_cap_result_t r={0};(void)t;(void)p;++f->executions;
    if(f->async)return;r.size=sizeof(r);r.version=1;r.outcome=UI_CAP_OK;r.output_json=f->bad_output?"false":"{}";
    CHECK(ui_capability_complete(f->host,id,&r)==UI_STATUS_OK);
    CHECK(ui_capability_complete(f->host,id,&r)==UI_STATUS_ALREADY_EXISTS);
}
static ui_cap_target_t setup(fixture_t *f,int string_output)
{
    ui_cap_resource_desc_t r={0};ui_capability_desc_t d={0};ui_cap_target_t t={0};ui_cap_entry_binding_t b={0};
    r.size=sizeof(r);r.version=1;r.id="document";r.kind=UI_CAP_DOCUMENT;r.get_revision=revision;
    CHECK(ui_capability_register_resource(f->host,&r)==UI_STATUS_OK);
    d.size=sizeof(d);d.version=d.capability_version=1;d.id="read";d.name_utf8="Read";d.description_utf8="Closeout";
    d.category="test";d.input_schema_json=schema;d.output_schema_json=string_output?"{\"type\":\"string\"}":schema;
    d.target_kind=UI_CAP_DOCUMENT;d.effect="read";d.execute=execute;d.user_data=f;
    CHECK(ui_capability_register(f->host,&d)==UI_STATUS_OK);t.size=sizeof(t);t.version=1;
    CHECK(ui_capability_bind(f->host,"document",&t)==UI_STATUS_OK);
    CHECK(ui_capability_enable_read_entries(f->host)==UI_STATUS_OK);
    b.size=sizeof(b);b.version=1;b.capability_id="read";b.target=&t;
    CHECK(ui_capability_publish_read_entry(f->host,&b)==UI_STATUS_OK);return t;
}
static void recover(fixture_t *f)
{
    uint64_t request=0,call=0;unsigned before=f->executions;f->nest=f->async=0;
    CHECK(ui_assistant_invoke(f->assistant,"read","{}",&request)==UI_STATUS_OK&&request!=0);
    CHECK(ui_capability_request_call(f->host,request,&call)==UI_STATUS_OK);
    CHECK(ui_capability_dispatch(f->host,call)==UI_STATUS_OK&&f->executions==before+1);
    CHECK(ui_assistant_forget_request(f->assistant,request)==UI_STATUS_OK);
}
static void legacy_hold(ui_host_t *h,uint64_t id,const char *c,const char *p,const char *s,void *data)
{fixture_t *f=data;(void)h;(void)id;(void)c;(void)p;(void)s;++f->manual;}
static void legacy_result_case(void)
{
    int success;for(success=0;success<2;++success){
        fixture_t f={0};ui_command_desc_t c={0};ui_assistant_command_desc_t a={0};uint64_t id=0;unsigned before;
        init(&f);c.size=sizeof(c);c.id="legacy.async";c.title="Legacy async";c.handler=legacy_hold;c.user_data=&f;
        CHECK(ui_host_register_command(f.host,&c)==UI_STATUS_OK);
        a.size=sizeof(a);a.id=c.id;a.permission=UI_ASSISTANT_PERMISSION_EDIT;a.params_schema_json=schema;
        CHECK(ui_assistant_register_command(f.assistant,&a)==UI_STATUS_OK);
        CHECK(ui_assistant_invoke(f.assistant,c.id,"{}",&id)==UI_STATUS_OK&&id!=0&&f.manual==1);
        CHECK(f.results==0&&ui_capability_enable_read_entries(f.host)==UI_STATUS_PENDING);
        /* Complete on the UI thread after invoke/handler have fully returned. */
        f.legacy_observer=1;CHECK(ui_host_reply(f.host,id,success,"{}")==UI_STATUS_OK);f.legacy_observer=0;
        CHECK(f.results==1&&f.activation_attempts==2&&f.activation==UI_STATUS_PENDING);
        CHECK(ui_host_reply(f.host,id,success,"{}")==UI_STATUS_NOT_FOUND&&f.results==1);
        CHECK(ui_capability_enable_read_entries(f.host)==UI_STATUS_OK);
        before=f.manual;CHECK(ui_assistant_invoke(f.assistant,c.id,"{\"source\":\"human\"}",&id)==UI_STATUS_PERMISSION_DENIED&&id==0&&f.manual==before);
        id=ui_host_invoke(f.host,c.id,"{}","human");CHECK(id!=0&&f.manual==before+1);
        CHECK(ui_host_reply(f.host,id,1,"{}")==UI_STATUS_OK);
        (void)setup(&f,0);recover(&f);
        f.read_in_legacy_result=1;CHECK(ui_host_invoke(f.host,"manual","{}","human")!=0);
        {uint64_t call=0;before=f.executions;CHECK(ui_capability_request_call(f.host,f.generated_request,&call)==UI_STATUS_OK);
            CHECK(ui_capability_dispatch(f.host,call)==UI_STATUS_OK&&f.executions==before);
            CHECK(ui_assistant_forget_request(f.assistant,f.generated_request)==UI_STATUS_OK);}
        dispose(&f);
    }
}
static void failure_case(int count_limit)
{
    fixture_t f={0};ui_cap_target_t t;uint64_t ids[UI_CAPABILITY_MAX_CALLS],request=0,id=99,retained=0;unsigned i;
    init(&f);t=setup(&f,0);f.nest=1;
    if(count_limit){f.async=1;retained=ui_host_invoke(f.host,"read","{}","retained");CHECK(retained!=0);
        CHECK(ui_capability_request_call(f.host,retained,&ids[0])==UI_STATUS_OK);
        for(i=1;i<UI_CAPABILITY_MAX_CALLS;++i)CHECK(ui_capability_prepare(f.host,"read",&t,"{}",&ids[i])==UI_STATUS_OK);}
    request=ui_host_invoke(f.host,"read",count_limit?"{}":"{\"invalid\":1}","source");
    CHECK(request!=0&&f.max_depth==1&&f.results==1&&f.executions==(count_limit?1u:0u));
    CHECK(ui_capability_request_call(f.host,request,&id)==UI_STATUS_NOT_FOUND&&id==0);
    {char expected[64];(void)snprintf(expected,sizeof(expected),"{\"status\":%d}",count_limit?UI_STATUS_LIMIT_EXCEEDED:UI_STATUS_VALIDATION_FAILED);CHECK(!strcmp(f.last,expected));}
    f.nest=0;
    if(count_limit){ui_cap_result_t r={0};CHECK(ui_capability_request_call(f.host,retained,&id)==UI_STATUS_OK&&id==ids[0]);
        r.size=sizeof(r);r.version=1;r.outcome=UI_CAP_CANCELLED;CHECK(ui_capability_complete(f.host,ids[0],&r)==UI_STATUS_OK);
        CHECK(ui_capability_forget(f.host,ids[0])==UI_STATUS_OK);
        for(i=1;i<UI_CAPABILITY_MAX_CALLS;++i){CHECK(ui_capability_cancel(f.host,ids[i])==UI_STATUS_OK);CHECK(ui_capability_forget(f.host,ids[i])==UI_STATUS_OK);}}
    recover(&f);dispose(&f);
}
static void shared_case(void)
{
    fixture_t f={0};ui_assistant_t *second;uint64_t id=0;unsigned i;init(&f);(void)setup(&f,0);
    second=ui_assistant_create(f.host,NULL);CHECK(second!=NULL);
    for(i=0;i<512;++i){CHECK(ui_assistant_invoke(f.assistant,"read","{\"invalid\":1}",&id)==UI_STATUS_OK&&id);CHECK(ui_assistant_invoke(second,"read","{\"invalid\":1}",&id)==UI_STATUS_OK&&id);}
    CHECK(ui_assistant_invoke(second,"read","{}",&id)==UI_STATUS_LIMIT_EXCEEDED&&id==0);
    ui_assistant_destroy(f.assistant);f.assistant=NULL;
    for(i=0;i<512;++i)CHECK(ui_assistant_invoke(second,"read","{\"invalid\":1}",&id)==UI_STATUS_OK&&id);
    CHECK(ui_assistant_invoke(second,"read","{}",&id)==UI_STATUS_LIMIT_EXCEEDED&&id==0);
    ui_assistant_destroy(second);f.assistant=ui_assistant_create(f.host,NULL);CHECK(f.assistant!=NULL);
    recover(&f);dispose(&f);
}
static void output_failure_case(void)
{
    fixture_t f={0};uint64_t request=0,call=0;init(&f);(void)setup(&f,0);f.nest=f.bad_output=1;
    request=ui_host_invoke(f.host,"read","{}","output-failure");CHECK(request!=0);
    CHECK(f.results==1&&f.max_depth==1&&f.executions==1&&!strcmp(f.last,"{\"code\":\"invalid_output\"}"));
    CHECK(ui_capability_request_call(f.host,request,&call)==UI_STATUS_OK);
    CHECK(ui_capability_forget(f.host,call)==UI_STATUS_OK);f.bad_output=0;recover(&f);dispose(&f);
}
static void byte_case(void)
{
    fixture_t f={0};ui_cap_target_t t;ui_cap_result_t r={0};uint64_t ids[64],request=0,id=0;
    unsigned i,used=0;size_t n=UI_CAPABILITY_OUTPUT_BYTES;char *payload=malloc(n+1);int full=0;
    init(&f);t=setup(&f,1);f.async=1;CHECK(payload!=NULL);if(!payload){dispose(&f);return;}
    for(i=0;i<64;++i){CHECK(ui_capability_prepare(f.host,"read",&t,"{}",&ids[i])==UI_STATUS_OK);CHECK(ui_capability_dispatch(f.host,ids[i])==UI_STATUS_OK);}
    r.size=sizeof(r);r.version=1;r.outcome=UI_CAP_OK;
    for(i=0;i<64;++i){ui_status_t s;
        for(;;){payload[0]='"';memset(payload+1,'x',n-2);payload[n-1]='"';payload[n]=0;r.output_json=payload;
            s=ui_capability_complete(f.host,ids[i],&r);if(s!=UI_STATUS_LIMIT_EXCEEDED)break;
            if(n==2){full=1;break;}n/=2;if(n<2)n=2;
        }
        if(full)break;CHECK(s==UI_STATUS_OK);++used;
    }
    CHECK(full&&used>0&&used<64);f.nest=1;
    /* No calls/results released between attempts: each must reuse the fixed
     * slot after unwind, while nested attempts cannot borrow its live data. */
    for(i=0;i<3;++i){uint64_t previous=request;char source[32],expected[64];
        (void)snprintf(source,sizeof(source),"byte-budget-%u",i);
        request=ui_host_invoke(f.host,"read","{}",source);
        CHECK(request>previous&&f.max_depth==1&&f.results==i+1&&f.executions==64);
        CHECK(ui_capability_request_call(f.host,request,&id)==UI_STATUS_NOT_FOUND);
        (void)snprintf(expected,sizeof(expected),"{\"status\":%d}",UI_STATUS_LIMIT_EXCEEDED);CHECK(!strcmp(f.last,expected));
    }
    for(i=0;i<64;++i){ui_cap_result_t current={0};current.size=sizeof(current);current.version=1;
        CHECK(ui_capability_get_result(f.host,ids[i],&current)==UI_STATUS_OK);
        if(i>=used){CHECK(current.state==UI_CAP_RUNNING);r.outcome=UI_CAP_CANCELLED;r.output_json=NULL;CHECK(ui_capability_complete(f.host,ids[i],&r)==UI_STATUS_OK);}
        else CHECK(current.state==UI_CAP_FINISHED&&current.outcome==UI_CAP_OK);
        CHECK(ui_capability_forget(f.host,ids[i])==UI_STATUS_OK);
    }
    /* String capability remains usable after draining. */
    f.nest=0;request=ui_host_invoke(f.host,"read","{}","drained");CHECK(request!=0);
    CHECK(ui_capability_request_call(f.host,request,&id)==UI_STATUS_OK);r.outcome=UI_CAP_OK;r.output_json="\"ok\"";
    CHECK(ui_capability_complete(f.host,id,&r)==UI_STATUS_OK);CHECK(ui_capability_forget(f.host,id)==UI_STATUS_OK);
    free(payload);dispose(&f);
}
int main(void)
{
    activation_case();transaction_case();legacy_result_case();failure_case(0);failure_case(1);shared_case();output_failure_case();byte_case();
    printf("C1 closeout: %u checks, %u failures\n",checks,failures);return failures?1:0;
}
