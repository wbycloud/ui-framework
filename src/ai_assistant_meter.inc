/* All root and application model frames pass these hooks from the original
 * av_reserve / av_before_send / transport / av_price call sites. */
#include "ai_assistant_price.h"

static int as_policy_ready(as_policy_t *p)
{
    return p->input_rate&&p->output_rate&&p->price_version[0]&&ai_assistant_price_current(p->price_version)&&
           as_hash(p->price_evidence)&&bm_profile_matches(p->meter_profile,p->meter_evidence);
}

static void as_confirm(ui_ai_service_t *s,ai_run_t *r)
{
    as_run_t *a=r->model->assistant;
    if(!a->until)a->until=ai_now(s)+a->policy.deadline_ms;
}

static int as_send_allowed(ui_ai_service_t *s,ai_run_t *r)
{
    as_run_t *a=r->model->assistant;
    EnterCriticalSection(&s->lock);
    int allowed=r->model->confirmed&&!r->model->recovered&&!r->terminal&&!r->cancel&&
           s->phase==UI_AI_READY&&!s->store_failed&&!s->close_requested&&
           !a->stopped&&!a->fee_unknown&&a->until&&ai_now(s)<a->until&&
           a->control==AS_CONTROL_AI&&as_policy_ready(&a->policy)&&
           !strcmp(r->model->model,"deepseek-flash");
    LeaveCriticalSection(&s->lock);return allowed;
}

static int as_event(ui_ai_service_t *s,ai_run_t *r,const char *kind,const char *payload)
{
    char operation[96];snprintf(operation,sizeof(operation),"assistant:%u:%s",r->model->ordinal,kind);
    return am_record(s,r,operation,kind,payload);
}

static int as_reserve(ui_ai_service_t *s,ai_run_t *r)
{
    as_run_t *a=r->model->assistant;am_run_t *m=r->model;
    if(!as_send_allowed(s,r)){as_stop(s,r,"assistant_send_conditions");return 0;}
    for(unsigned i=0;i<a->used;++i)if(!strcmp(a->tickets[i].request,m->request_id)){
        char hash[65];ui_cap_hash_bytes(m->body,strlen(m->body),hash);
        return !a->tickets[i].entered&&!strcmp(hash,a->tickets[i].hash);
    }
    if(a->used>=a->policy.requests){as_stop(s,r,"assistant_request_limit");return 0;}
    av_policy_t meter={0};uint64_t input=0;
    meter.meter_required=1;meter.input_ceiling=AM_REQUEST;
    strcpy_s(meter.meter_profile,65,a->policy.meter_profile);
    strcpy_s(meter.meter_evidence,65,a->policy.meter_evidence);
    if(!bm_measure_bound(s,r,&meter,&input)){as_stop(s,r,meter.error[0]?meter.error:"assistant_meter_unavailable");return 0;}
    /* Policy bounds were copied at admission. Multiplication remains checked
       even when the official profile's current limits are smaller. */
    if(input>UINT64_MAX/a->policy.input_rate||4096>UINT64_MAX/a->policy.output_rate){as_stop(s,r,"assistant_cost_overflow");return 0;}
    uint64_t output=4096*a->policy.output_rate,estimate=input*a->policy.input_rate;
    if(estimate>UINT64_MAX-output){as_stop(s,r,"assistant_cost_overflow");return 0;}estimate+=output;
    EnterCriticalSection(&s->lock);
    int allowed=as_send_allowed(s,r)&&a->used<a->policy.requests&&
       a->known<=a->policy.budget&&a->estimated<=a->policy.budget-a->known&&
       estimate<=a->policy.budget-a->known-a->estimated;
    LeaveCriticalSection(&s->lock);
    if(!allowed){as_stop(s,r,"assistant_fee_limit");return 0;}
    char hash[65];ui_cap_hash_bytes(m->body,strlen(m->body),hash);
    yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *o=d?yyjson_mut_obj(d):NULL;
    if(!o){yyjson_mut_doc_free(d);as_stop(s,r,"assistant_record_limit");return 0;}yyjson_mut_doc_set_root(d,o);
    yyjson_mut_obj_add_strcpy(d,o,"root",r->id);yyjson_mut_obj_add_strcpy(d,o,"request_id",m->request_id);
    yyjson_mut_obj_add_uint(d,o,"ordinal",m->ordinal);yyjson_mut_obj_add_uint(d,o,"frame",a->frame);
    yyjson_mut_obj_add_strcpy(d,o,"body_hash",hash);yyjson_mut_obj_add_uint(d,o,"input_tokens",input);
    yyjson_mut_obj_add_uint(d,o,"estimated_nano_usd",estimate);yyjson_mut_obj_add_strcpy(d,o,"price_version",a->policy.price_version);
    if((a->frame==1||a->frame==3)&&r->graph&&a->node<r->graph->count){
        ag_node_t *node=&r->graph->nodes[a->node];
        yyjson_mut_obj_add_strcpy(d,o,"node",node->id);yyjson_mut_obj_add_strcpy(d,o,"operation",node->operation);
        yyjson_mut_obj_add_strcpy(d,o,"profile",node->profile);yyjson_mut_obj_add_uint(d,o,"target",node->target);
        yyjson_mut_obj_add_strcpy(d,o,"question_id",node->question_id);
    }
    char *record=yyjson_mut_write(d,0,NULL);yyjson_mut_doc_free(d);
    int saved=record&&as_event(s,r,"assistant_reserved",record);free(record);if(!saved)return 0;
    EnterCriticalSection(&s->lock);
    as_ticket_t *t=&a->tickets[a->used++];strcpy_s(t->request,65,m->request_id);
    strcpy_s(t->hash,65,hash);t->ordinal=m->ordinal;t->frame=a->frame;t->node=a->node;t->estimate=estimate;t->input=input;
    a->estimated+=estimate;
    LeaveCriticalSection(&s->lock);
    return 1;
}

static as_ticket_t *as_ticket(ai_run_t *r)
{
    as_run_t *a=r->model->assistant;
    for(unsigned i=0;i<a->used;++i)if(!strcmp(a->tickets[i].request,r->model->request_id))return &a->tickets[i];
    return NULL;
}

static int as_before_send(ui_ai_service_t *s,ai_run_t *r,int simulated)
{
    as_ticket_t *t=as_ticket(r);char hash[65],payload[192];
    ui_cap_hash_bytes(r->model->body,strlen(r->model->body),hash);
    if(!t||t->entered||!as_send_allowed(s,r)||strcmp(hash,t->hash))return 0;
    snprintf(payload,sizeof(payload),"{\"request_id\":\"%s\",\"simulated\":%s}",t->request,simulated?"true":"false");
    if(!as_event(s,r,"assistant_send_intent",payload)||!as_send_allowed(s,r))return 0;
    EnterCriticalSection(&s->lock);t->entered=1;t->simulated=simulated;LeaveCriticalSection(&s->lock);
    return 1;
}

static void as_send_ack(ui_ai_service_t *s,ai_run_t *r)
{
    as_ticket_t *t=as_ticket(r);if(!t||t->entered!=1)return;
    char payload[192];snprintf(payload,sizeof(payload),"{\"request_id\":\"%s\",\"simulated\":%s}",t->request,t->simulated?"true":"false");
    EnterCriticalSection(&s->lock);t->entered=2;LeaveCriticalSection(&s->lock);
    if(!as_event(s,r,"assistant_transport_entry",payload))as_stop(s,r,"assistant_send_record_unconfirmed");
}

static void as_network_end(ui_ai_service_t *s,ai_run_t *r)
{
    as_ticket_t *t=as_ticket(r);if(!t||t->ended)return;
    int complete=r->model->net.response_complete,unknown=r->model->net.sent&&!complete;
    char payload[256];snprintf(payload,sizeof(payload),"{\"request_id\":\"%s\",\"complete\":%s,\"unknown\":%s}",t->request,complete?"true":"false",unknown?"true":"false");
    EnterCriticalSection(&s->lock);t->ended=1;t->unknown=unknown;
    if(unknown)r->model->assistant->fee_unknown=1;LeaveCriticalSection(&s->lock);
    if(ai_store_available(s))(void)as_event(s,r,"assistant_network_end",payload);
    if(!complete)as_stop(s,r,"assistant_network_or_delivery_unknown");
}

static int as_price(ui_ai_service_t *s,ai_run_t *r)
{
    as_run_t *a=r->model->assistant;as_ticket_t *t=as_ticket(r);
    if(!t||t->priced)return t!=NULL;
    ui_cap_json_doc_t d={0};int valid=am_json_parse(r->model->batch,strlen(r->model->batch),&d);
    yyjson_val *usage=valid?yyjson_obj_get(yyjson_doc_get_root(d.doc),"usage"):NULL;
    yyjson_val *input=yyjson_obj_get(usage,"input_tokens"),*output=yyjson_obj_get(usage,"output_tokens");
    valid=yyjson_is_uint(input)&&yyjson_is_uint(output)&&yyjson_get_uint(input)<=t->input&&yyjson_get_uint(output)<=4096;
    uint64_t cost=valid?yyjson_get_uint(input)*a->policy.input_rate+yyjson_get_uint(output)*a->policy.output_rate:0;
    valid=valid&&cost<=t->estimate&&a->estimated>=t->estimate&&cost<=UINT64_MAX-a->known;
    am_json_free(&d);
    char payload[256],amount[32];
    if(valid)snprintf(amount,sizeof(amount),"%llu",(unsigned long long)cost);else strcpy_s(amount,sizeof(amount),"null");
    snprintf(payload,sizeof(payload),"{\"request_id\":\"%s\",\"fee_status\":\"%s\",\"known_nano_usd\":%s}",t->request,valid?"known":"unknown",amount);
    if(!as_event(s,r,"assistant_cost",payload))return 0;
    EnterCriticalSection(&s->lock);t->priced=1;
    if(valid){a->estimated-=t->estimate;a->known+=cost;}else a->fee_unknown=1;
    LeaveCriticalSection(&s->lock);
    if(!valid)as_stop(s,r,"assistant_usage_unknown");
    return valid;
}
