#include "ui_framework/capability.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int failures,checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);++failures;}}while(0)
typedef struct model {ui_host_t *host;unsigned revision,executions,cancels;int async,bad,deny,reenter;uint64_t pending;} model_t;
static const char empty[]="{\"type\":\"object\",\"additionalProperties\":false}";
static const char output[]="{\"type\":\"object\",\"properties\":{\"value\":{\"type\":\"integer\"}},\"required\":[\"value\"],\"additionalProperties\":false}";
static ui_status_t rev(char *b,size_t n,void *data)
{model_t *m=data;(void)snprintf(b,n,"%u",m->revision);return UI_STATUS_OK;}
static ui_status_t check(const ui_cap_target_t *t,const char *p,void *data)
{model_t *m=data;(void)t;(void)p;return m->deny==2?UI_STATUS_VALIDATION_FAILED:m->deny?UI_STATUS_PERMISSION_DENIED:UI_STATUS_OK;}
static void execute(uint64_t id,const ui_cap_target_t *t,const char *params,void *data)
{
    model_t *m=data;ui_cap_result_t r={0},before={0};++m->executions;m->pending=id;
    CHECK(!strcmp(params,"{}"));CHECK(!strcmp(t->resource_id,"model"));
    before.size=sizeof(before);before.version=1;CHECK(ui_capability_get_result(m->host,id,&before)==UI_STATUS_OK&&before.state==UI_CAP_RUNNING);
    CHECK(ui_capability_forget(m->host,id)==UI_STATUS_PENDING);
    if(m->reenter){CHECK(ui_capability_begin_unregistration(m->host,"read")==UI_STATUS_OK);CHECK(ui_capability_finish_unregistration(m->host,"read")==UI_STATUS_PENDING);}
    if(m->async)return;
    r.size=sizeof(r);r.version=1;r.outcome=UI_CAP_OK;r.output_json=m->bad?"{\"value\":\"bad\"}":"{\"value\":42}";
    CHECK(ui_capability_complete(m->host,id,&r)==UI_STATUS_OK);
    CHECK(ui_capability_complete(m->host,id,&r)==UI_STATUS_ALREADY_EXISTS);
}
static void cancel(uint64_t id,void *data){model_t *m=data;CHECK(id==m->pending);++m->cancels;}
static ui_capability_desc_t desc(model_t *m,const char *id)
{ui_capability_desc_t d={0};d.size=sizeof(d);d.version=d.capability_version=1;d.id=id;d.name_utf8="Read model";d.description_utf8="Read only fixture";d.category="model";d.input_schema_json=empty;d.output_schema_json=output;d.target_kind=UI_CAP_DOCUMENT;d.effect="read";d.check=check;d.execute=execute;d.cancel=cancel;d.user_data=m;return d;}
static ui_cap_target_t bind(model_t *m)
{ui_cap_target_t t={0};t.size=sizeof(t);t.version=1;CHECK(ui_capability_bind(m->host,"model",&t)==UI_STATUS_OK);return t;}
static uint64_t prepare(model_t *m,ui_cap_target_t *t)
{uint64_t id=0;CHECK(ui_capability_prepare(m->host,"read",t,"{}",&id)==UI_STATUS_OK&&id);return id;}
static ui_cap_result_t result(model_t *m,uint64_t id)
{ui_cap_result_t r={0};r.size=sizeof(r);r.version=1;CHECK(ui_capability_get_result(m->host,id,&r)==UI_STATUS_OK);return r;}
static void complete(model_t *m,uint64_t id,ui_cap_outcome_t outcome)
{ui_cap_result_t r={0};r.size=sizeof(r);r.version=1;r.outcome=outcome;r.output_json="{\"value\":42}";CHECK(ui_capability_complete(m->host,id,&r)==UI_STATUS_OK);}
int main(void)
{
    ui_host_config_t hc={0};model_t m={0};ui_cap_resource_desc_t rd={0};ui_capability_desc_t d;
    ui_cap_target_t t,old;uint64_t id,previous,generation=0;char *json=NULL;size_t count,next;unsigned executions;
    hc.size=sizeof(hc);hc.api_version=UI_FRAMEWORK_API_VERSION;m.host=ui_host_create(&hc);m.revision=1;CHECK(m.host!=NULL);if(!m.host)return 1;
#ifndef _WIN32
    d=desc(&m,"read");CHECK(ui_capability_register(m.host,&d)==UI_STATUS_UNSUPPORTED);ui_host_destroy(m.host);return failures?1:0;
#endif
    rd.size=sizeof(rd);rd.version=1;rd.kind=UI_CAP_DOCUMENT;rd.id="model";rd.get_revision=rev;rd.user_data=&m;
    CHECK(ui_capability_register_resource(m.host,&rd)==UI_STATUS_OK);CHECK(ui_capability_register_resource(m.host,&rd)==UI_STATUS_ALREADY_EXISTS);
    d=desc(&m,"read");CHECK(ui_capability_register(m.host,&d)==UI_STATUS_OK);CHECK(ui_capability_register(m.host,&d)==UI_STATUS_ALREADY_EXISTS);
    d.effect="write";d.id="write";CHECK(ui_capability_register(m.host,&d)==UI_STATUS_UNSUPPORTED);
    d=desc(&m,"invalid");d.input_schema_json="{\"type\":\"object\"}";CHECK(ui_capability_register(m.host,&d)==UI_STATUS_VALIDATION_FAILED);
    /* JCS may expand exponent notation: stored schemas also obey 64KiB. */
    {char *schema=malloc(32768);size_t n=0,i,j;CHECK(schema!=NULL);if(schema){
      n+=(size_t)snprintf(schema,32768,"{\"type\":\"object\",\"additionalProperties\":false,\"examples\":[");
      for(i=0;i<7;++i){if(i)schema[n++]=',';schema[n++]='[';for(j=0;j<512;++j)n+=(size_t)snprintf(schema+n,32768-n,"%s1e20",j?",":"");schema[n++]=']';}
      schema[n++]=']';schema[n++]='}';schema[n]=0;d=desc(&m,"expanded-schema");d.input_schema_json=schema;CHECK(ui_capability_register(m.host,&d)==UI_STATUS_LIMIT_EXCEEDED);free(schema);}}
    CHECK(ui_capability_discover(m.host,"model",0,1,&generation,&json)==UI_STATUS_OK);CHECK(json&&strstr(json,"descriptor_hash")&&strstr(json,"read"));ui_capability_release(json);
    CHECK(ui_capability_describe(m.host,"read",&json)==UI_STATUS_OK);CHECK(json&&strstr(json,"input_schema")&&strstr(json,"descriptor_hash"));
    if(json){char *canonical=NULL;size_t n=0;CHECK(ui_cap_json_canonicalize(json,strlen(json),&canonical,&n)==UI_STATUS_OK);CHECK(canonical&&!strcmp(canonical,json));ui_capability_release(canonical);}ui_capability_release(json);
    {ui_cap_target_t targets[2];uint64_t gen=0;CHECK(ui_capability_resources(m.host,0,targets,2,&count,&next,&gen)==UI_STATUS_OK&&count==1&&next==0);}
    t=bind(&m);CHECK(t.instance_id&&strlen(t.host_epoch)==36&&strlen(t.open_epoch)==36);
    old=t;(void)snprintf(old.scope_json,sizeof(old.scope_json),"{\"access\":\"write\"}");CHECK(ui_capability_prepare(m.host,"read",&old,"{}",&id)==UI_STATUS_PERMISSION_DENIED);
    old=t;old.instance_id++;CHECK(ui_capability_prepare(m.host,"read",&old,"{}",&id)==UI_STATUS_NOT_FOUND);
    CHECK(ui_capability_prepare(m.host,"read",&t,"{\"extra\":1}",&id)==UI_STATUS_VALIDATION_FAILED);
    id=prepare(&m,&t);previous=id;CHECK(result(&m,id).state==UI_CAP_PREPARED);
    CHECK(ui_capability_dispatch(m.host,id)==UI_STATUS_OK);CHECK(result(&m,id).state==UI_CAP_FINISHED);CHECK(result(&m,id).outcome==UI_CAP_OK);
    CHECK(!strcmp(result(&m,id).output_json,"{\"value\":42}"));executions=m.executions;
    CHECK(ui_capability_dispatch(m.host,id)==UI_STATUS_OK&&m.executions==executions);CHECK(ui_capability_forget(m.host,id)==UI_STATUS_OK);
    CHECK(ui_capability_dispatch(m.host,id)==UI_STATUS_NOT_FOUND);
    id=prepare(&m,&t);CHECK(id>previous);CHECK(ui_capability_cancel(m.host,id)==UI_STATUS_OK);CHECK(result(&m,id).outcome==UI_CAP_CANCELLED);CHECK(ui_capability_dispatch(m.host,id)==UI_STATUS_OK&&m.executions==executions);CHECK(ui_capability_forget(m.host,id)==UI_STATUS_OK);
    id=prepare(&m,&t);++m.revision;CHECK(ui_capability_dispatch(m.host,id)==UI_STATUS_OK);CHECK(result(&m,id).outcome==UI_CAP_STALE&&m.executions==executions);CHECK(ui_capability_forget(m.host,id)==UI_STATUS_OK);
    t=bind(&m);m.deny=1;id=prepare(&m,&t);CHECK(ui_capability_dispatch(m.host,id)==UI_STATUS_OK);CHECK(result(&m,id).outcome==UI_CAP_REJECTED);CHECK(ui_capability_forget(m.host,id)==UI_STATUS_OK);m.deny=0;
    m.deny=2;id=prepare(&m,&t);CHECK(ui_capability_dispatch(m.host,id)==UI_STATUS_OK);CHECK(result(&m,id).outcome==UI_CAP_REJECTED&&!strcmp(result(&m,id).code,"unavailable"));CHECK(ui_capability_forget(m.host,id)==UI_STATUS_OK);m.deny=0;
    m.bad=1;id=prepare(&m,&t);CHECK(ui_capability_dispatch(m.host,id)==UI_STATUS_OK);CHECK(result(&m,id).outcome==UI_CAP_FAILED&&!strcmp(result(&m,id).code,"invalid_output"));CHECK(ui_capability_forget(m.host,id)==UI_STATUS_OK);m.bad=0;
    m.async=1;id=prepare(&m,&t);CHECK(ui_capability_dispatch(m.host,id)==UI_STATUS_OK);CHECK(result(&m,id).state==UI_CAP_RUNNING);
    CHECK(ui_capability_cancel(m.host,id)==UI_STATUS_OK&&m.cancels==1);CHECK(result(&m,id).state==UI_CAP_RUNNING&&result(&m,id).cancel_requested);complete(&m,id,UI_CAP_OK);CHECK(result(&m,id).outcome==UI_CAP_OK);CHECK(ui_capability_forget(m.host,id)==UI_STATUS_OK);
    id=prepare(&m,&t);CHECK(ui_capability_dispatch(m.host,id)==UI_STATUS_OK);++m.revision;complete(&m,id,UI_CAP_OK);CHECK(result(&m,id).outcome==UI_CAP_STALE);CHECK(ui_capability_forget(m.host,id)==UI_STATUS_OK);t=bind(&m);
    id=prepare(&m,&t);d=desc(&m,"second");CHECK(ui_capability_register(m.host,&d)==UI_STATUS_OK);CHECK(ui_capability_discover(m.host,NULL,0,32,&generation,&json)==UI_STATUS_CANCELLED);
    CHECK(ui_capability_dispatch(m.host,id)==UI_STATUS_OK&&result(&m,id).outcome==UI_CAP_REJECTED);CHECK(ui_capability_forget(m.host,id)==UI_STATUS_OK);
    t=bind(&m);id=prepare(&m,&t);CHECK(ui_capability_dispatch(m.host,id)==UI_STATUS_OK);CHECK(ui_capability_begin_resource_unregistration(m.host,"model")==UI_STATUS_OK);CHECK(ui_capability_finish_resource_unregistration(m.host,"model")==UI_STATUS_PENDING);
    complete(&m,id,UI_CAP_CANCELLED);CHECK(ui_capability_finish_resource_unregistration(m.host,"model")==UI_STATUS_OK);CHECK(ui_capability_forget(m.host,id)==UI_STATUS_OK);
    CHECK(ui_capability_register_resource(m.host,&rd)==UI_STATUS_OK);CHECK(ui_capability_prepare(m.host,"read",&t,"{}",&id)==UI_STATUS_NOT_FOUND);
    t=bind(&m);m.reenter=1;id=prepare(&m,&t);CHECK(ui_capability_dispatch(m.host,id)==UI_STATUS_OK);CHECK(ui_capability_finish_unregistration(m.host,"read")==UI_STATUS_PENDING);complete(&m,id,UI_CAP_CANCELLED);CHECK(ui_capability_finish_unregistration(m.host,"read")==UI_STATUS_OK);CHECK(ui_capability_forget(m.host,id)==UI_STATUS_OK);m.reenter=0;
    d=desc(&m,"read");CHECK(ui_capability_register(m.host,&d)==UI_STATUS_OK);
    /* Pagination and the capability-count limit are real independent registrations. */
    {unsigned i;char name[40];for(i=0;i<254;++i){(void)snprintf(name,sizeof(name),"extra.%u",i);d=desc(&m,name);CHECK(ui_capability_register(m.host,&d)==UI_STATUS_OK);}d=desc(&m,"over-limit");CHECK(ui_capability_register(m.host,&d)==UI_STATUS_LIMIT_EXCEEDED);generation=0;CHECK(ui_capability_discover(m.host,"model",0,32,&generation,&json)==UI_STATUS_OK&&strstr(json,"\"next\":32"));ui_capability_release(json);}
    /* Retained-call capacity is independent of the descriptor budget. */
    {uint64_t ids[UI_CAPABILITY_MAX_CALLS];unsigned i;for(i=0;i<UI_CAPABILITY_MAX_CALLS;++i)ids[i]=prepare(&m,&t);CHECK(ui_capability_prepare(m.host,"read",&t,"{}",&id)==UI_STATUS_LIMIT_EXCEEDED);for(i=0;i<UI_CAPABILITY_MAX_CALLS;++i){CHECK(ui_capability_cancel(m.host,ids[i])==UI_STATUS_OK);CHECK(ui_capability_forget(m.host,ids[i])==UI_STATUS_OK);}}
    id=prepare(&m,&t);CHECK(ui_capability_dispatch(m.host,id)==UI_STATUS_OK);CHECK(ui_capability_quiesce(m.host)==UI_STATUS_PENDING);
    ui_host_destroy(m.host);CHECK(result(&m,id).state==UI_CAP_RUNNING);CHECK(ui_capability_prepare(m.host,"read",&t,"{}",&previous)==UI_STATUS_CANCELLED);
    complete(&m,id,UI_CAP_CANCELLED);CHECK(ui_capability_quiesce(m.host)==UI_STATUS_OK);ui_host_destroy(m.host);
    /* Completion backpressure retains the running call and copied producer data
     * can be resubmitted after a terminal record is released. */
    {
        model_t large_model={0};char *payload=malloc(UI_CAPABILITY_OUTPUT_BYTES+1u);uint64_t ids[40];unsigned done=0,i;ui_cap_result_t r={0};
        large_model.host=ui_host_create(&hc);large_model.revision=1;large_model.async=1;CHECK(large_model.host&&payload);
        if(large_model.host&&payload){
            rd.user_data=&large_model;CHECK(ui_capability_register_resource(large_model.host,&rd)==UI_STATUS_OK);d=desc(&large_model,"read");d.output_schema_json="{\"type\":\"string\"}";CHECK(ui_capability_register(large_model.host,&d)==UI_STATUS_OK);t=bind(&large_model);
            payload[0]='\"';memset(payload+1,'x',UI_CAPABILITY_OUTPUT_BYTES-2u);payload[UI_CAPABILITY_OUTPUT_BYTES-1u]='\"';payload[UI_CAPABILITY_OUTPUT_BYTES]=0;
            r.size=sizeof(r);r.version=1;r.outcome=UI_CAP_OK;r.output_json=payload;
            for(i=0;i<40;++i){ui_status_t s;ids[i]=prepare(&large_model,&t);CHECK(ui_capability_dispatch(large_model.host,ids[i])==UI_STATUS_OK);s=ui_capability_complete(large_model.host,ids[i],&r);if(s==UI_STATUS_LIMIT_EXCEEDED){CHECK(result(&large_model,ids[i]).state==UI_CAP_RUNNING);break;}CHECK(s==UI_STATUS_OK);++done;}
            CHECK(done>0&&done<40);if(done&&done<40){CHECK(ui_capability_forget(large_model.host,ids[0])==UI_STATUS_OK);CHECK(ui_capability_complete(large_model.host,ids[done],&r)==UI_STATUS_OK);for(i=1;i<=done;++i)CHECK(ui_capability_forget(large_model.host,ids[i])==UI_STATUS_OK);}
            ui_host_destroy(large_model.host);
        }free(payload);
    }
    /* Directory bytes, not only the 256-entry count, limit copied descriptions. */
    {
        model_t catalog={0};char description[4097],name[40];unsigned i;ui_status_t s=UI_STATUS_OK;
        catalog.host=ui_host_create(&hc);CHECK(catalog.host!=NULL);memset(description,'a',sizeof(description)-1);description[4096]=0;
        if(catalog.host){for(i=0;i<256;++i){(void)snprintf(name,sizeof(name),"large.%u",i);d=desc(&catalog,name);d.description_utf8=description;s=ui_capability_register(catalog.host,&d);if(s)break;}CHECK(s==UI_STATUS_LIMIT_EXCEEDED&&i<256);ui_host_destroy(catalog.host);}
    }
    printf("capability core: %d checks, %d failures\n",checks,failures);return failures?1:0;
}
