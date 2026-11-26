/* One confirmed operation uses one Service and successive exact Coordinator
 * packets. Only the declared readback of the last persisted P4 receipt can
 * advance the observed revision. The original finite-plan path stays separate. */
static int ao_current(ui_ai_service_t *s,ai_run_t *r)
{
    as_run_t *a=r->model->assistant;as_operation_t *op=a->operation;
    const ui_cap_target_v2_t *fixed=&a->targets[0];
    ui_app_instance_info_t app;ui_cap_target_t live={0};live.size=sizeof(live);live.version=1;
    if(ag_app(s,fixed,&app)||ui_capability_bind(app.host,fixed->resource.resource_id,&live))return 0;
    const char *revision=op->phase==AO_VERIFY?op->next_revision:op->revision;
    if(live.instance_id!=fixed->resource.instance_id||live.kind!=fixed->resource.kind||
       strcmp(live.host_epoch,fixed->resource.host_epoch)||strcmp(live.open_epoch,fixed->resource.open_epoch)||
       strcmp(live.expected_revision,revision)||ag_app(s,fixed,&app))return 0;
    yyjson_doc *d=yyjson_read(a->scope,strlen(a->scope),0);
    const char *id=ag_str(yyjson_doc_get_root(d),"id");char *current=NULL;
    int ok=*id&&!ui_cap_operation_get(app.host,id,&current)&&!strcmp(current,a->scope);
    free(current);yyjson_doc_free(d);return ok;
}

static yyjson_val *ao_find(yyjson_val *rows,const char *key,const char *id)
{
    yyjson_val *row;size_t i,n;
    yyjson_arr_foreach(rows,i,n,row)if(yyjson_equals_str(yyjson_obj_get(row,key),id))return row;
    return NULL;
}

static int ao_includes(yyjson_val *values,const char *name)
{
    yyjson_val *v;size_t i,n;
    yyjson_arr_foreach(values,i,n,v)if(yyjson_equals_str(v,name))return 1;
    return 0;
}

static int ao_source(yyjson_val *source,yyjson_val *observations,yyjson_val *fields)
{
    if(yyjson_obj_size(source)==1)return yyjson_obj_get(source,"value")!=NULL;
    return yyjson_obj_size(source)==2&&ao_find(observations,"id",ag_str(source,"from"))&&
        yyjson_obj_get(fields,ag_str(source,"field"));
}

static yyjson_val *ao_value(yyjson_val *source,yyjson_val *facts)
{
    yyjson_val *literal=yyjson_obj_get(source,"value");
    return literal?literal:yyjson_obj_get(yyjson_obj_get(facts,ag_str(source,"from")),ag_str(source,"field"));
}

static int ao_validate_plan(yyjson_val *plan,yyjson_val *spec)
{
    yyjson_val *reads=yyjson_obj_get(plan,"observations"),*writes=yyjson_obj_get(plan,"actions"),*goal=yyjson_obj_get(plan,"goal");
    yyjson_val *fields=yyjson_obj_get(yyjson_obj_get(spec,"observe"),"fields");
    yyjson_val *progress=yyjson_obj_get(goal,"progress");
    if(yyjson_obj_size(plan)!=3||!yyjson_is_arr(reads)||!yyjson_arr_size(reads)||yyjson_arr_size(reads)>4||
       !yyjson_is_arr(writes)||!yyjson_arr_size(writes)||yyjson_arr_size(writes)>8||
       yyjson_obj_size(goal)!=(progress?4u:3u)||(progress&&!yyjson_equals_str(progress,"increase"))||
       !ao_find(reads,"id",ag_str(goal,"observation"))||!yyjson_obj_get(fields,ag_str(goal,"field"))||
       !ao_source(yyjson_obj_get(goal,"equals"),reads,fields))return 0;
    size_t i,n;yyjson_val *row;
    yyjson_arr_foreach(reads,i,n,row){
        const char *id=ag_str(row,"id");yyjson_val *params=yyjson_obj_get(row,"parameters");
        if(yyjson_obj_size(row)!=2||!ai_id(id)||ao_find(reads,"id",id)!=row||
           !yyjson_is_obj(params)||yyjson_obj_size(params)>16)return 0;
    }
    yyjson_arr_foreach(writes,i,n,row){
        yyjson_val *policy=ao_find(yyjson_obj_get(spec,"actions"),"capability",ag_str(row,"capability"));
        yyjson_val *read=ao_find(reads,"id",ag_str(row,"readback"));
        yyjson_val *params=yyjson_obj_get(row,"parameters"),*fixed=yyjson_obj_get(read,"parameters");
        yyjson_val *identity=yyjson_obj_get(policy,"identity"),*sources=yyjson_obj_get(policy,"sources");
        const char *id=ag_str(row,"id");
        if(yyjson_obj_size(row)!=4||!ai_id(id)||ao_find(writes,"id",id)!=row||!policy||!read||
           yyjson_obj_size(fixed)!=yyjson_arr_size(identity)||!yyjson_is_obj(params)||yyjson_obj_size(params)>32)return 0;
        size_t j,count;yyjson_val *v,*key;
        yyjson_arr_foreach(identity,j,count,v){
            const char *name=yyjson_get_str(v);
            yyjson_val *value=yyjson_obj_get(yyjson_obj_get(params,name),"value");
            if(!value||!yyjson_equals(value,yyjson_obj_get(fixed,name)))return 0;
        }
        yyjson_obj_foreach(params,j,count,key,v){
            const char *name=yyjson_get_str(key);
            if(!ao_source(v,reads,fields)||
               !yyjson_obj_get(yyjson_obj_get(yyjson_obj_get(policy,"input_schema"),"properties"),name))return 0;
            if(!yyjson_obj_get(v,"value")&&!ao_includes(yyjson_obj_get(sources,name),ag_str(v,"field")))return 0;
        }
    }
    return 1;
}

static yyjson_mut_val *ao_parameters(yyjson_mut_doc *d,yyjson_val *bindings,yyjson_val *facts)
{
    yyjson_mut_val *params=yyjson_mut_obj(d);if(!params)return NULL;
    size_t i,n;yyjson_val *key,*binding;
    yyjson_obj_foreach(bindings,i,n,key,binding){
        yyjson_val *value=ao_value(binding,facts);
        if(!value||!yyjson_mut_obj_add_val(d,params,yyjson_get_str(key),yyjson_val_mut_copy(d,value)))return NULL;
    }
    return params;
}

/* -1 invalid proof, 0 still needed, 1 exact match, 2 crossed a monotone goal. */
static int ao_goal(yyjson_val *plan,yyjson_val *facts)
{
    yyjson_val *goal=yyjson_obj_get(plan,"goal");
    yyjson_val *observed=yyjson_obj_get(yyjson_obj_get(facts,ag_str(goal,"observation")),ag_str(goal,"field"));
    yyjson_val *expected=ao_value(yyjson_obj_get(goal,"equals"),facts);
    if(!observed||!expected)return -1;
    if(yyjson_obj_get(goal,"progress")){
        if(!yyjson_is_uint(observed)||!yyjson_is_uint(expected))return -1;
        if(yyjson_get_uint(observed)>yyjson_get_uint(expected))return 2;
    }
    return yyjson_equals(observed,expected)?1:0;
}

static int ao_advances(yyjson_val *plan,yyjson_val *old,yyjson_val *fresh)
{
    yyjson_val *goal=yyjson_obj_get(plan,"goal");
    if(!yyjson_obj_get(goal,"progress"))return 1;
    const char *id=ag_str(goal,"observation"),*field=ag_str(goal,"field");
    return yyjson_get_uint(yyjson_obj_get(yyjson_obj_get(fresh,id),field))>
           yyjson_get_uint(yyjson_obj_get(yyjson_obj_get(old,id),field));
}

static yyjson_mut_val *ao_project(yyjson_mut_doc *d,yyjson_val *raw,yyjson_val *fields,const char *revision,int receipt)
{
    if(!yyjson_equals_str(yyjson_obj_get(raw,"revision"),revision))return NULL;
    yyjson_mut_val *object=yyjson_mut_obj(d);if(!object)return NULL;
    yyjson_val *key,*mode;size_t i,n;
    yyjson_obj_foreach(fields,i,n,key,mode){
        const char *name=yyjson_get_str(key);yyjson_val *value=yyjson_obj_get(raw,name);
        if(receipt&&!value)continue;
        if(yyjson_equals_str(mode,"count")){
            if(!yyjson_is_arr(value)||!yyjson_mut_obj_add_uint(d,object,name,yyjson_arr_size(value)))return NULL;
        }else if((!yyjson_is_str(value)&&!yyjson_is_num(value)&&!yyjson_is_bool(value))||
                !yyjson_mut_obj_add_val(d,object,name,yyjson_val_mut_copy(d,value)))return NULL;
    }
    return object;
}

static char *ao_observe(ai_graph_t *g,yyjson_val *spec,const char *revision)
{
    yyjson_val *fields=yyjson_obj_get(yyjson_obj_get(spec,"observe"),"fields");
    yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *rows=d?yyjson_mut_obj(d):NULL;
    int ok=rows!=NULL;if(ok)yyjson_mut_doc_set_root(d,rows);
    for(unsigned i=0;ok&&i<g->count;++i){
        ag_node_t *n=&g->nodes[i];yyjson_doc *output=n->result?yyjson_read(n->result,strlen(n->result),0):NULL;
        yyjson_doc *params=yyjson_read(n->params,strlen(n->params),0);
        yyjson_val *data=yyjson_doc_get_root(output),*locators=yyjson_doc_get_root(params),*key,*value;size_t j,count;
        ok=!strcmp(n->outcome,"completed")&&!strcmp(n->revision,revision)&&params;
        /* If a read declares an identity as observed, verify the app returned
         * the selected object's value. No locator is taken from a model reply. */
        yyjson_obj_foreach(locators,j,count,key,value)if(yyjson_obj_get(fields,yyjson_get_str(key))&&
            !yyjson_equals(value,yyjson_obj_get(data,yyjson_get_str(key))))ok=0;
        if(ok)ok=yyjson_mut_obj_add_val(d,rows,n->id,ao_project(d,data,fields,revision,0));
        yyjson_doc_free(output);yyjson_doc_free(params);
    }
    char *raw=ok?yyjson_mut_write(d,0,NULL):NULL;yyjson_mut_doc_free(d);return raw;
}

static int ao_packet_node(yyjson_mut_doc *d,yyjson_mut_val *nodes,const char *id,const char *operation,
                          yyjson_val *cap,yyjson_val *parameters,const char *prior)
{
    yyjson_mut_val *node=yyjson_mut_obj(d),*deps=yyjson_mut_arr(d);
    return node&&deps&&(!prior||yyjson_mut_arr_add_strcpy(d,deps,prior))&&
        yyjson_mut_obj_add_strcpy(d,node,"id",id)&&yyjson_mut_obj_add_strcpy(d,node,"operation_id",operation)&&
        yyjson_mut_obj_add_strcpy(d,node,"capability",ag_str(cap,"capability"))&&yyjson_mut_obj_add_uint(d,node,"target",0)&&
        yyjson_mut_obj_add_val(d,node,"params",yyjson_val_mut_copy(d,parameters))&&
        yyjson_mut_obj_add_val(d,node,"constraint",yyjson_val_mut_copy(d,yyjson_obj_get(cap,"input_schema")))&&
        yyjson_mut_obj_add_val(d,node,"dependencies",deps)&&yyjson_mut_arr_add_val(nodes,node);
}

static ui_status_t ao_packet(ui_ai_service_t *s,ai_run_t *r,yyjson_val *spec,yyjson_val *plan,
                             yyjson_val *action,yyjson_val *parameters,int phase,const char *revision)
{
    as_run_t *a=r->model->assistant;as_operation_t *op=a->operation;ai_graph_t *old=r->graph;
    yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *o=d?yyjson_mut_obj(d):NULL,*nodes=d?yyjson_mut_arr(d):NULL;
    char id[65];int ok=o&&nodes;
    if(ok&&action){
        snprintf(id,sizeof(id),"edit-%u",op->attempted+1);
        yyjson_val *cap=ao_find(yyjson_obj_get(spec,"actions"),"capability",ag_str(action,"capability"));
        ok=ao_packet_node(d,nodes,id,id,cap,parameters,NULL);
    }else if(ok){
        yyjson_val *read;size_t i,n;const char *prior=NULL;
        yyjson_arr_foreach(yyjson_obj_get(plan,"observations"),i,n,read){
            snprintf(id,sizeof(id),"observe-%u-%zu",op->attempted,i+1);
            const char *name=ag_str(read,"id");
            if(!ao_packet_node(d,nodes,name,id,yyjson_obj_get(spec,"observe"),yyjson_obj_get(read,"parameters"),prior)){ok=0;break;}
            prior=name;
        }
    }
    if(ok){yyjson_mut_doc_set_root(d,o);ok=yyjson_mut_obj_add_val(d,o,"nodes",nodes);}
    char *raw=ok?yyjson_mut_write(d,0,NULL):NULL;yyjson_mut_doc_free(d);
    if(!raw)return UI_STATUS_OUT_OF_MEMORY;
    ui_cap_target_v2_t target=a->targets[0];strcpy_s(target.expected_revision,128,revision);
    ui_ai_graph_request_t q={0};q.size=sizeof(q);q.version=1;q.client_request_id=r->client;
    q.input_utf8=r->input;q.graph_json=raw;q.targets=&target;q.target_count=1;
    ai_graph_t *g=NULL;ui_status_t status=ag_build(s,&q,&g);free(raw);
    if(status)return status==UI_STATUS_PENDING?UI_STATUS_VALIDATION_FAILED:status;
    uint64_t version=old?old->version:0;
    char signature[256],hash[65];
    ui_cap_hash_bytes(g->json,strlen(g->json),hash);
    snprintf(signature,sizeof(signature),"%s:%s:%llu:%s",r->id,s->session,(unsigned long long)(version+1),hash);
    status=ui_cap_hash_bytes(signature,strlen(signature),g->binding);
    EnterCriticalSection(&s->lock);
    if(!status&&(r->cancel||a->stopped||a->control!=AS_CONTROL_AI||s->phase!=UI_AI_READY||
       s->store_failed||s->close_requested||s->active!=(int)(r-s->runs)||r->graph!=old))status=UI_STATUS_CANCELLED;
    if(!status&&r->refs!=1)status=UI_STATUS_PENDING;
    if(!status){
        g->admitted=1;g->version=version+1;g->persisted=version;g->confirmed=action!=NULL;g->scope_until=a->until;
        /* Keep the Coordinator's structural charge. Actual model requests
         * remain in the one root ledger and are never reserved by this packet. */
        r->graph=g;op->phase=phase;a->stage=AS_EXECUTING;
        if(action){
            ++op->attempted;
            strcpy_s(op->readback,65,ag_str(action,"readback"));
            strcpy_s(op->capability,256,ag_str(action,"capability"));
        }
        ai_graph_free(s,old);g=NULL;
    }
    ai_graph_free(s,g);LeaveCriticalSection(&s->lock);return status;
}

static int ao_ready(ui_ai_service_t *s,ai_run_t *r)
{
    as_run_t *a=r->model->assistant;as_operation_t *op=a->operation;
    return r->model->confirmation_saved&&!r->cancel&&!a->stopped&&a->control==AS_CONTROL_AI&&
        a->until&&ai_now(s)<a->until&&!a->fee_unknown&&as_policy_ready(&a->policy)&&
        s->model_config&&r->model->revision==s->model_config->revision&&
        (op->phase==AO_OBSERVE||(op->granted&&(op->phase==AO_APPLY||op->phase==AO_VERIFY)));
}

static char *ao_items(ai_run_t *r,yyjson_val *plan,yyjson_val *facts)
{
    as_run_t *a=r->model->assistant;as_operation_t *op=a->operation;
    yyjson_doc *spec=yyjson_read(a->scope,strlen(a->scope),0);
    yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *o=d?yyjson_mut_obj(d):NULL,*options=d?yyjson_mut_arr(d):NULL;
    int ok=spec&&o&&options&&yyjson_mut_obj_add_strcpy(d,o,"user_goal",r->input)&&yyjson_mut_obj_add_strcpy(d,o,"revision",op->revision)&&
        yyjson_mut_obj_add_val(d,o,"observations",yyjson_val_mut_copy(d,facts))&&
        yyjson_mut_obj_add_val(d,o,"goal",yyjson_val_mut_copy(d,yyjson_obj_get(plan,"goal")));
    yyjson_val *row;size_t i,n;
    yyjson_arr_foreach(yyjson_obj_get(plan,"actions"),i,n,row){
        if(!ok)break;
        yyjson_mut_val *option=yyjson_mut_obj(d),*params=ao_parameters(d,yyjson_obj_get(row,"parameters"),facts);
        yyjson_val *registered=ao_find(yyjson_obj_get(yyjson_doc_get_root(spec),"actions"),"capability",ag_str(row,"capability"));
        char *schema=registered?yyjson_val_write(yyjson_obj_get(registered,"input_schema"),0,NULL):NULL;
        char *concrete=params?yyjson_mut_val_write(params,0,NULL):NULL;
        ok=option&&schema&&concrete&&!ui_cap_json_validate(schema,concrete,strlen(concrete))&&
            yyjson_mut_obj_add_strcpy(d,option,"id",ag_str(row,"id"))&&
            yyjson_mut_obj_add_strcpy(d,option,"capability",ag_str(row,"capability"))&&
            yyjson_mut_obj_add_val(d,option,"parameters",params)&&yyjson_mut_arr_add_val(options,option);
        free(schema);free(concrete);
    }
    if(ok)ok=yyjson_mut_obj_add_val(d,o,"permitted_actions",options);
    char *context=ok?yyjson_mut_val_write(o,0,NULL):NULL;yyjson_mut_val *items=d?yyjson_mut_arr(d):NULL,*item=d?yyjson_mut_obj(d):NULL;
    ok=context&&items&&item&&yyjson_mut_obj_add_str(d,item,"role","user")&&
        yyjson_mut_obj_add_strcpy(d,item,"content",context)&&yyjson_mut_arr_add_val(items,item);
    if(ok)yyjson_mut_doc_set_root(d,items);
    char *raw=ok?yyjson_mut_write(d,0,NULL):NULL;free(context);yyjson_mut_doc_free(d);yyjson_doc_free(spec);
    if(raw&&strlen(raw)>AI_CONTEXT){free(raw);return NULL;}return raw;
}

static void ao_continue(ui_ai_service_t *s,ai_run_t *r,yyjson_val *plan,yyjson_val *facts)
{
    as_run_t *a=r->model->assistant;as_operation_t *op=a->operation;
    int goal=ao_goal(plan,facts);
    if(goal==1){
        yyjson_val *g=yyjson_obj_get(plan,"goal");
        char *value=yyjson_val_write(yyjson_obj_get(yyjson_obj_get(facts,ag_str(g,"observation")),ag_str(g,"field")),0,NULL);
        snprintf(a->final_result,sizeof(a->final_result),"已核对目标。实际读回 %s。%s",value?value:"",op->verified?"":"已有目标值，本次无需修改。");
        free(value);op->phase=AO_DONE;return;
    }
    if(goal){as_stop(s,r,"operation_goal_unverified");return;}
    if(op->attempted>=op->limit||a->used>=a->policy.requests){as_stop(s,r,"operation_limit");return;}
    char *items=ao_items(r,plan,facts);
    if(!items){as_stop(s,r,"operation_context_limit");return;}
    EnterCriticalSection(&s->lock);
    if(!r->cancel&&!a->stopped&&a->control==AS_CONTROL_AI&&!s->close_requested&&s->phase==UI_AI_READY){
        strcpy_s(a->frame_input,sizeof(a->frame_input),items);a->frame=AO_FRAME;a->node=0;
        a->response[0]=0;op->phase=AO_NEXT;a->stage=AS_FRAME_QUEUED;
        (void)ai_start_locked(s);
    }
    LeaveCriticalSection(&s->lock);free(items);
}

static int ao_single_question(as_operation_t *op,yyjson_val *reply)
{
    const char *text=ag_str(reply,"question");
    if(yyjson_obj_size(reply)!=1||strlen(text)!=yyjson_get_len(yyjson_obj_get(reply,"question"))||!ai_text(text,1024,0))return 0;
    strcpy_s(op->question,sizeof(op->question),text);return 1;
}

static int ao_settled(ai_graph_t *g)
{
    if(!g||g->dirty||g->version!=g->persisted)return 0;
    for(unsigned i=0;i<g->count;++i)if(g->nodes[i].state!=AG_COLLECTED)return 0;
    return 1;
}

static void ao_review(ui_ai_service_t *s,ai_run_t *r)
{
    as_run_t *a=r->model->assistant;as_operation_t *op=a->operation;ai_graph_t *g=r->graph;
    char plan[65],facts[65],scope[65],identity[480];
    if(ui_cap_hash_bytes(op->plan,strlen(op->plan),plan)||ui_cap_hash_bytes(op->facts,strlen(op->facts),facts)||
       ui_cap_hash_bytes(r->plan,strlen(r->plan),scope)){as_stop(s,r,"operation_identity");return;}
    snprintf(identity,sizeof(identity),"%s:%s:%s:%s:%s:%s",r->id,s->session,op->revision,plan,facts,scope);
    if(ui_cap_hash_bytes(identity,strlen(identity),g->binding)){as_stop(s,r,"operation_identity");return;}
    op->phase=AO_REVIEW;g->needs_scope=1;ag_dirty(g);
    EnterCriticalSection(&s->lock);a->stage=AS_PLAN_WAIT;LeaveCriticalSection(&s->lock);
}

static void ao_ui_step(ui_ai_service_t *s,ai_run_t *r,yyjson_val *spec,yyjson_val *plan)
{
    as_run_t *a=r->model->assistant;as_operation_t *op=a->operation;ai_graph_t *g=r->graph;
    int stage=as_stage(s,a);ui_status_t status=UI_STATUS_OK;
    if((op->phase==AO_GOAL||op->phase==AO_NEXT)&&stage==AS_RESPONSE){
        char *canonical=NULL;size_t length=0;
        if(strlen(a->response)>AO_PLAN_BYTES||ui_cap_json_canonicalize(a->response,strlen(a->response),&canonical,&length)){
            as_stop(s,r,"operation_invalid_response");return;
        }
        yyjson_doc *response=yyjson_read(canonical,length,0);yyjson_val *reply=yyjson_doc_get_root(response);
        if(ao_single_question(op,reply))as_stop(s,r,"operation_needs_input");
        else if(op->phase==AO_GOAL){
            if(length>AO_PLAN_BYTES||!ao_validate_plan(reply,spec)||!as_current(s,r))status=UI_STATUS_VALIDATION_FAILED;
            else{
                strcpy_s(op->plan,sizeof(op->plan),canonical);
                status=ao_packet(s,r,spec,reply,NULL,NULL,AO_OBSERVE,op->revision);
            }
        }else{
            yyjson_val *write=ao_find(yyjson_obj_get(plan,"actions"),"id",ag_str(reply,"action"));
            yyjson_doc *evidence=yyjson_read(op->facts,strlen(op->facts),0);
            yyjson_mut_doc *params=yyjson_mut_doc_new(NULL);
            yyjson_mut_val *bound=write&&params?ao_parameters(params,yyjson_obj_get(write,"parameters"),yyjson_doc_get_root(evidence)):NULL;
            char *text=bound?yyjson_mut_val_write(bound,0,NULL):NULL;yyjson_doc *values=text?yyjson_read(text,strlen(text),0):NULL;
            yyjson_val *concrete=yyjson_obj_get(reply,"parameters");
            if(!op->granted||!ao_settled(g)||op->attempted>=op->limit||yyjson_obj_size(reply)!=3||
               !yyjson_equals_str(yyjson_obj_get(reply,"revision"),op->revision)||!yyjson_is_obj(concrete)||
               !values||!yyjson_equals(yyjson_doc_get_root(values),concrete)||!as_current(s,r))
                status=UI_STATUS_PERMISSION_DENIED;
            else status=ao_packet(s,r,spec,plan,write,concrete,AO_APPLY,op->revision);
            yyjson_doc_free(values);free(text);yyjson_mut_doc_free(params);yyjson_doc_free(evidence);
        }
        yyjson_doc_free(response);free(canonical);
        if(status&&status!=UI_STATUS_PENDING)as_stop(s,r,"operation_proposal_rejected");
        return;
    }
    if(!ao_settled(g)||op->phase==AO_NEXT||op->phase==AO_DONE)return;
    if(g->failed){as_stop(s,r,"operation_step_unconfirmed");return;}
    if(op->phase==AO_APPLY){
        ag_node_t *n=&g->nodes[0];ai_edit_t *e=n->edit;
        if(!e||e->phase!=AE_DONE||!e->committed||e->effect!=UI_CAP2_EFFECT_APPLIED||!e->transaction[0]||
           !e->undo_id[0]||!n->revision[0]||!strcmp(n->revision,op->revision)||strcmp(n->revision,e->revision_after)){
            as_stop(s,r,"operation_receipt_unverified");return;
        }
        yyjson_doc *d=n->result?yyjson_read(n->result,strlen(n->result),0):NULL;
        yyjson_mut_doc *p=yyjson_mut_doc_new(NULL);
        yyjson_mut_val *value=p?ao_project(p,yyjson_doc_get_root(d),yyjson_obj_get(yyjson_obj_get(spec,"observe"),"fields"),n->revision,1):NULL;
        char *receipt=value?yyjson_mut_val_write(value,0,NULL):NULL;
        int ok=receipt&&strlen(receipt)<=AG_RESULT;
        if(ok){strcpy_s(op->receipt_facts,sizeof(op->receipt_facts),receipt);strcpy_s(op->next_revision,128,n->revision);}
        free(receipt);yyjson_mut_doc_free(p);yyjson_doc_free(d);
        if(!ok){as_stop(s,r,"operation_receipt_unverified");return;}
        op->committed=op->attempted;
        status=ao_packet(s,r,spec,plan,NULL,NULL,AO_VERIFY,op->next_revision);
        if(status&&status!=UI_STATUS_PENDING)as_stop(s,r,"operation_readback_rejected");
        return;
    }
    if(!as_current(s,r)){as_stop(s,r,"operation_target_changed");return;}
    if(op->phase==AO_REVIEW){
        if(!g->confirmed)return;
        op->granted=1;
        yyjson_doc *d=yyjson_read(op->facts,strlen(op->facts),0);
        ao_continue(s,r,plan,yyjson_doc_get_root(d));yyjson_doc_free(d);return;
    }
    const char *revision=op->phase==AO_VERIFY?op->next_revision:op->revision;
    char *observations=ao_observe(g,spec,revision);
    yyjson_doc *fresh=observations?yyjson_read(observations,strlen(observations),0):NULL;
    yyjson_val *facts=yyjson_doc_get_root(fresh);
    int ok=fresh&&strlen(observations)<=AO_FACT_BYTES&&ao_goal(plan,facts)>=0;
    if(ok&&op->phase==AO_VERIFY){
        yyjson_doc *last=yyjson_read(op->facts,strlen(op->facts),0),*receipt=yyjson_read(op->receipt_facts,strlen(op->receipt_facts),0);
        yyjson_val *actual=yyjson_obj_get(facts,op->readback),*key,*value;size_t i,n;
        ok=receipt&&yyjson_is_obj(actual)&&ao_advances(plan,yyjson_doc_get_root(last),facts);
        yyjson_obj_foreach(yyjson_doc_get_root(receipt),i,n,key,value)
            if(!yyjson_equals(value,yyjson_obj_get(actual,yyjson_get_str(key))))ok=0;
        yyjson_doc_free(last);yyjson_doc_free(receipt);
    }
    if(ok){
        strcpy_s(op->facts,sizeof(op->facts),observations);
        if(op->phase==AO_OBSERVE)ao_review(s,r);
        else{strcpy_s(op->revision,128,op->next_revision);op->verified=op->committed;ao_continue(s,r,plan,facts);}
    }else as_stop(s,r,"operation_observation_unverified");
    yyjson_doc_free(fresh);free(observations);
}

static void ao_ui(ui_ai_service_t *s,ai_run_t *r)
{
    as_run_t *a=r->model->assistant;
    if(r->terminal||r->cancel||a->stopped||a->control!=AS_CONTROL_AI||!r->model->confirmed||s->store_failed)return;
    if(a->until&&ai_now(s)>=a->until){as_stop(s,r,"operation_deadline");return;}
    if(a->fee_unknown||!as_policy_ready(&a->policy)){as_stop(s,r,"operation_billing_unavailable");return;}
    if(!s->model_config||r->model->revision!=s->model_config->revision){as_stop(s,r,"operation_model_changed");return;}
    EnterCriticalSection(&s->lock);
    int eligible=!r->refs&&s->active==(int)(r-s->runs);
    if(eligible)++r->refs;LeaveCriticalSection(&s->lock);
    if(!eligible)return;
    yyjson_doc *spec=yyjson_read(a->scope,strlen(a->scope),0);
    yyjson_doc *plan=yyjson_read(a->operation->plan,strlen(a->operation->plan),0);
    if(spec)ao_ui_step(s,r,yyjson_doc_get_root(spec),yyjson_doc_get_root(plan));else as_stop(s,r,"operation_scope_unavailable");
    yyjson_doc_free(plan);yyjson_doc_free(spec);
    EnterCriticalSection(&s->lock);--r->refs;LeaveCriticalSection(&s->lock);
}

static void ao_snapshot(ui_ai_service_t *s,ai_run_t *r,yyjson_mut_doc *d,yyjson_mut_val *root)
{
    as_run_t *a=r->model->assistant;as_operation_t *op=a->operation;
    yyjson_mut_val *o=yyjson_mut_obj(d);if(!o)return;
    unsigned committed=op->committed;
    if(r->graph&&op->phase==AO_APPLY&&r->graph->nodes[0].edit&&
       r->graph->nodes[0].edit->committed&&r->graph->nodes[0].edit->phase==AE_DONE)committed=op->attempted;
    yyjson_mut_obj_add_int(d,o,"phase",op->phase);yyjson_mut_obj_add_bool(d,o,"granted",op->granted!=0);
    yyjson_mut_obj_add_strcpy(d,o,"plan",op->plan);yyjson_mut_obj_add_strcpy(d,o,"facts",op->facts);
    yyjson_mut_obj_add_strcpy(d,o,"revision",op->revision);yyjson_mut_obj_add_strcpy(d,o,"candidate_revision",op->next_revision);
    yyjson_mut_obj_add_strcpy(d,o,"action",op->capability);yyjson_mut_obj_add_strcpy(d,o,"question",op->question);
    yyjson_mut_obj_add_uint(d,o,"attempted",op->attempted);yyjson_mut_obj_add_uint(d,o,"committed",committed);
    yyjson_mut_obj_add_uint(d,o,"verified",op->verified);yyjson_mut_obj_add_uint(d,o,"limit",op->limit);
    yyjson_mut_obj_add_uint(d,o,"request_limit",a->policy.requests);yyjson_mut_obj_add_uint(d,o,"budget_nano_usd",a->policy.budget);
    yyjson_mut_obj_add_uint(d,o,"deadline_ms",a->policy.deadline_ms);
    (void)s;
    yyjson_mut_obj_add_val(d,root,"operation_task",o);
}

static int ao_collect(ui_ai_service_t *s,ai_run_t *r)
{
    as_run_t *a=r->model->assistant;as_operation_t *op=a->operation;ai_graph_t *g=r->graph;
    if(!r->cancel&&!a->stopped&&!g->unknown&&op->phase!=AO_DONE)return 0;
    EnterCriticalSection(&s->lock);
    int inflight=r->model->net.started&&!r->model->net.finished;
    LeaveCriticalSection(&s->lock);
    if(inflight)return 0;
    int takeover=r->cancel&&(a->control==AS_CONTROL_DRAIN||a->control==AS_CONTROL_MANUAL);
    if(r->cancel||a->stopped||g->unknown){
        unsigned committed=op->committed;
        if(op->phase==AO_APPLY&&g->nodes[0].edit&&g->nodes[0].edit->committed&&g->nodes[0].edit->phase==AE_DONE)committed=op->attempted;
        const char *reason=g->unknown?"业务回执尚未确定。保留当前状态供核对。":
            takeover&&a->fee_unknown?"已停止后续操作。已发请求的远端结果和费用尚不确定。":
            takeover?"已停止后续操作。":
            op->question[0]?op->question:!strcmp(a->error,"operation_limit")?"已达到本次执行限额。":
            r->cancel&&!a->stopped?"已停止后续动作。":"当前范围、版本或执行结果需要重新核对。";
        snprintf(a->final_result,sizeof(a->final_result),"%s\n已确认提交 %u 项，独立读回核实 %u 项。",reason,committed,op->verified);
    }
    strcpy_s(r->reply,UI_AI_MAX_REPLY+1,a->final_result);r->reply_length=strlen(r->reply);
    strcpy_s(r->outcome,32,g->unknown?"unknown":takeover?"cancelled":a->stopped?"failed":r->cancel?"cancelled":"completed");
    a->stage=AS_DONE;return 1;
}

static ui_status_t ao_disclosure(ui_ai_service_t *s,ai_run_t *r,char **out)
{
    as_run_t *a=r->model->assistant;as_operation_t *op=a->operation;
    yyjson_doc *plan=yyjson_read(op->plan,strlen(op->plan),0),*facts=yyjson_read(op->facts,strlen(op->facts),0);
    int goal=ao_goal(yyjson_doc_get_root(plan),yyjson_doc_get_root(facts));
    char *items=goal==0?ao_items(r,yyjson_doc_get_root(plan),yyjson_doc_get_root(facts)):NULL;
    am_run_t projection={0};projection.assistant=a;projection.items=items;strcpy_s(projection.model,128,r->model->model);
    char *wire=items?am_body_for(&projection,AO_FRAME):NULL;
    yyjson_doc *request=wire?yyjson_read(wire,strlen(wire),0):NULL;
    yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *o=d?yyjson_mut_obj(d):NULL;
    int ok=o&&plan&&facts&&(goal>0||request);
    if(ok){
        yyjson_mut_doc_set_root(d,o);
        ok=yyjson_mut_obj_add_uint(d,o,"version",1)&&yyjson_mut_obj_add_strcpy(d,o,"run_id",r->id)&&
            yyjson_mut_obj_add_strcpy(d,o,"binding_hash",r->graph->binding)&&yyjson_mut_obj_add_strcpy(d,o,"fixed_targets",r->targets_json)&&
            yyjson_mut_obj_add_strcpy(d,o,"scope",r->plan)&&yyjson_mut_obj_add_bool(d,o,"operation_review",true)&&
            yyjson_mut_obj_add_strcpy(d,o,"user_goal",r->input)&&
            yyjson_mut_obj_add_val(d,o,"approved_plan",yyjson_val_mut_copy(d,yyjson_doc_get_root(plan)))&&
            yyjson_mut_obj_add_val(d,o,"observed",yyjson_val_mut_copy(d,yyjson_doc_get_root(facts)))&&
            yyjson_mut_obj_add_str(d,o,"stop","同一目标内按核实版本顺序操作。遇到范围变化、结果不明或限额即停。可随时接管。");
        if(ok&&request)ok=yyjson_mut_obj_add_val(d,o,"request",yyjson_val_mut_copy(d,yyjson_doc_get_root(request)));
        if(ok&&!request)ok=yyjson_mut_obj_add_str(d,o,"next_request","目标已达到或不符合继续条件，当前没有后续发送。");
        if(ok){
            EnterCriticalSection(&s->lock);
            unsigned requests_left=a->used<a->policy.requests?a->policy.requests-a->used:0;
            uint64_t fees_left=a->known<a->policy.budget?a->policy.budget-a->known:0;
            fees_left=a->estimated<fees_left?fees_left-a->estimated:0;
            int fee_unknown=a->fee_unknown;
            LeaveCriticalSection(&s->lock);
            ok=yyjson_mut_obj_add_uint(d,o,"actions_limit",op->limit)&&
                yyjson_mut_obj_add_uint(d,o,"requests_limit",a->policy.requests)&&
                yyjson_mut_obj_add_uint(d,o,"requests_remaining",requests_left)&&
                yyjson_mut_obj_add_uint(d,o,"budget_nano_usd",a->policy.budget)&&
                (fee_unknown?yyjson_mut_obj_add_null(d,o,"budget_remaining_nano_usd"):
                    yyjson_mut_obj_add_uint(d,o,"budget_remaining_nano_usd",fees_left))&&
                yyjson_mut_obj_add_uint(d,o,"deadline_ms",a->policy.deadline_ms);
            if(ok)*out=yyjson_mut_write(d,YYJSON_WRITE_PRETTY,NULL);
        }
    }
    (void)s;yyjson_mut_doc_free(d);yyjson_doc_free(request);yyjson_doc_free(plan);yyjson_doc_free(facts);am_delete(wire);free(items);
    return *out?UI_STATUS_OK:UI_STATUS_VALIDATION_FAILED;
}
