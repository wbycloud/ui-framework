/* BR-2 uses the existing validation ledger/worker, never a confirmation source. */
#include "ui_framework/ai_business_validation.h"
static const char *bv_input="BR-1：读取本轮合成交接单标题、来源与版本，不导航或修改。";
static const char *bv_body_input="PB-1：读取当前合成交接单DOM正文，概括设备总数、已完成、待处理及负责人；不导航或修改。";
static const char *bv_reading_input="PB-3：阅读SQLite《Zero-Configuration》公开网页隔离副本，概括配置特性、关键事实及待办；原文没有待办时明确说明，不导航或修改。";
static const char *bv_task_input(const av_policy_t *p){return p->body_scene==2?bv_reading_input:p->body_scene?bv_body_input:bv_input;}
#include "ai_business_restore.inc"
ui_status_t ui_ai_business_validation_enable(ui_ai_service_t *s,const char *json)
{
 if(!ai_ui(s)||!s->model_config)return UI_STATUS_INVALID_ARGUMENT;
 if(s->assistant_policy)return UI_STATUS_PERMISSION_DENIED;
 char path[4096];if(snprintf(path,sizeof(path),"%s.model.json",s->path)>=4096||strcmp(path,s->model_config->path))return UI_STATUS_PERMISSION_DENIED;
 EnterCriticalSection(&s->lock);
 if(s->validation){LeaveCriticalSection(&s->lock);return UI_STATUS_ALREADY_EXISTS;}
 if(s->close_requested||(s->phase!=UI_AI_READY&&s->phase!=UI_AI_OPENING)){LeaveCriticalSection(&s->lock);return UI_STATUS_PENDING;}
 for(unsigned i=0;i<AI_RUNS;++i)if(s->runs[i].id[0]){LeaveCriticalSection(&s->lock);return UI_STATUS_PERMISSION_DENIED;}
 if(sizeof(av_policy_t)>UI_AI_MAX_BYTES-s->bytes){LeaveCriticalSection(&s->lock);return UI_STATUS_LIMIT_EXCEEDED;}
 av_policy_t *p=calloc(1,sizeof(*p));if(!p){LeaveCriticalSection(&s->lock);return UI_STATUS_OUT_OF_MEMORY;}
 p->business=1;p->job=1;strcpy_s(p->id,65,"BR-2-unverified");s->validation=p;s->bytes+=sizeof(*p);
 ui_cap_json_doc_t d={0};int valid=json&&strlen(json)<=8192&&am_json_parse(json,strlen(json),&d);yyjson_val *o=valid?yyjson_doc_get_root(d.doc):NULL;
 const char *database=yyjson_get_str(yyjson_obj_get(o,"database_path_utf8"));
 const char *id=yyjson_get_str(yyjson_obj_get(o,"validation_id")),*model=yyjson_get_str(yyjson_obj_get(o,"model")),*protocol=yyjson_get_str(yyjson_obj_get(o,"protocol")),*version=yyjson_get_str(yyjson_obj_get(o,"price_version")),*evidence=yyjson_get_str(yyjson_obj_get(o,"price_evidence_sha256"));
 yyjson_val *in=yyjson_obj_get(o,"input_nano_per_token"),*out=yyjson_obj_get(o,"output_nano_per_token"),*limit=yyjson_obj_get(o,"billable_input_token_ceiling");
 valid=valid&&database&&!strcmp(database,s->path)&&yyjson_get_uint(yyjson_obj_get(o,"version"))==1&&ai_id(id)&&ai_id(model)&&ai_id(version)&&evidence&&strlen(evidence)==64&&protocol&&!strcmp(protocol,"deepseek-responses-v1")&&yyjson_get_bool(yyjson_obj_get(o,"price_and_ceiling_verified"))&&yyjson_is_uint(in)&&yyjson_is_uint(out)&&yyjson_is_uint(limit)&&yyjson_get_uint(in)>0&&yyjson_get_uint(in)<=1000000&&yyjson_get_uint(out)>0&&yyjson_get_uint(out)<=1000000&&yyjson_get_uint(limit)>0&&yyjson_get_uint(limit)<=10000000;
 if(valid){for(unsigned i=0;i<64;++i)if(!((evidence[i]>='0'&&evidence[i]<='9')||(evidence[i]>='a'&&evidence[i]<='f')||(evidence[i]>='A'&&evidence[i]<='F')))valid=0;}
 const char *meter_profile=yyjson_get_str(yyjson_obj_get(o,"meter_profile")),*meter_evidence=yyjson_get_str(yyjson_obj_get(o,"meter_evidence_sha256"));
 yyjson_val *scene_value=yyjson_obj_get(o,"business_scene");const char *scene=yyjson_get_str(scene_value);
 int scene_kind=scene_value?(scene?av_scene_parse(scene):-1):0;
 int scene_valid=scene_kind>=0;
 p->body_scene=scene_valid?scene_kind:0;
 if(!scene_valid)av_stop_locked(s,"business_scene_invalid");
 int legacy=0;
#ifdef UI_AI_FIXTURE
 legacy=yyjson_get_bool(yyjson_obj_get(o,"fixture_legacy_meter"));
 int component_available=strlen(BR2_METER_FIXTURE_SHA256)==64;
#else
 int component_available=strlen(BR2_METER_SHA256)==64;
 if(yyjson_obj_get(o,"fixture_legacy_meter"))valid=0;
#endif
 valid=valid&&scene_valid&&(!p->body_scene||!legacy)&&(legacy||(component_available&&bm_profile_matches(meter_profile,meter_evidence)));
 if(valid&&!legacy){p->meter_required=1;strcpy_s(p->meter_profile,65,meter_profile);strcpy_s(p->meter_evidence,65,meter_evidence);}
 if(valid){strcpy_s(p->id,65,id);strcpy_s(p->business_model,65,model);strcpy_s(p->price_version,65,version);strcpy_s(p->price_evidence,65,evidence);p->input_rate=yyjson_get_uint(in);p->output_rate=yyjson_get_uint(out);p->input_ceiling=yyjson_get_uint(limit);}
 else av_stop_locked(s,"business_price_or_ceiling_unverified");
 am_json_free(&d);(void)ai_start_locked(s);LeaveCriticalSection(&s->lock);return UI_STATUS_OK;
}
static int bv_scope(const av_policy_t *p,const ui_ai_real_task_t *q)
{
 if(!q||q->size<sizeof(*q)||q->version!=1||!q->input_utf8||strcmp(q->input_utf8,bv_task_input(p))||!q->history_selection_json||strcmp(q->history_selection_json,"[]")||q->target_count!=1||!q->targets||q->targets[0].size<sizeof(ui_cap_target_t)||!memchr(q->targets[0].resource_id,0,sizeof(q->targets[0].resource_id))||strcmp(q->targets[0].resource_id,"browser.view")||!q->disclosure_projection_json)return 0;
 const ui_cap_target_t *target=q->targets;
 if(target->version!=1||!target->instance_id||target->kind!=UI_CAP_INSTANCE_VIEW||!memchr(target->host_epoch,0,sizeof(target->host_epoch))||!memchr(target->open_epoch,0,sizeof(target->open_epoch))||!memchr(target->expected_revision,0,sizeof(target->expected_revision))||!memchr(target->scope_json,0,sizeof(target->scope_json))||strcmp(target->scope_json,"{\"access\":\"read\"}"))return 0;
 ui_cap_json_doc_t d={0};int ok=am_json_parse(q->disclosure_projection_json,strlen(q->disclosure_projection_json),&d);yyjson_val *o=ok?yyjson_doc_get_root(d.doc):NULL,*caps=yyjson_obj_get(o,"capabilities"),*fields=yyjson_obj_get(o,"fields");
 const char *cap=yyjson_get_str(yyjson_arr_get(caps,0)),*a=yyjson_get_str(yyjson_arr_get(fields,0)),*b=yyjson_get_str(yyjson_arr_get(fields,1));
 const char *value=p->body_scene?"body":"state";
 ok=ok&&(!p->body_scene||yyjson_obj_size(o)==2)&&yyjson_arr_size(caps)==1&&cap&&!strcmp(cap,p->body_scene?"browser.body":"browser.snapshot")&&yyjson_arr_size(fields)==2&&a&&b&&((!strcmp(a,value)&&!strcmp(b,"revision"))||(!strcmp(b,value)&&!strcmp(a,"revision")));am_json_free(&d);return ok;
}
static int bv_admit(ui_ai_service_t *s,const ui_ai_real_task_t *q)
{
 av_policy_t *p=s->validation;if(!bv_scope(p,q)||!ai_id(q->client_request_id))return 0;
 EnterCriticalSection(&s->lock);int ok=p->ready&&!p->stopped&&!p->admitting&&!s->store_failed&&!s->close_requested&&s->model_config&&!strcmp(s->model_config->model,p->business_model)&&(!p->client[1][0]||!strcmp(p->client[1],q->client_request_id));
 if(ok&&p->client[1][0])ok=av_target_equal(&p->target,q->targets);
 if(ok){p->admitting=2;strcpy_s(p->admit_client,65,q->client_request_id);strcpy_s(p->client[1],65,q->client_request_id);p->target=q->targets[0];}
 LeaveCriticalSection(&s->lock);return ok;
}
