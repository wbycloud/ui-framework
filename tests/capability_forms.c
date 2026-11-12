#include "ui_internal.h"
#include "capability_internal.h"
#include "ui_framework/capability_entry.h"
#include <stdio.h>
#include <string.h>
static unsigned checks,failures;
#define CHECK(x) do{++checks;if(!(x)){++failures;fprintf(stderr,"line %d: %s\n",__LINE__,#x);}}while(0)
typedef struct model model_t;
typedef struct view {model_t *m;int dialog,bad_read,wrong_value,wrong_revision,open_other;uint64_t binding,call;ui_status_t status;char code[64];} view_t;
struct model {ui_host_t *host;ui_component_t *form,*dialog;view_t a,b;char name[128],pending[2048];
    unsigned revision,executions,legacy_calls;int async,invalid_output,open_modal;double amount;int enabled;char mode[16],color[16];};
static void legacy(ui_host_t *host,uint64_t id,const char *command,const char *params,const char *source,void *data)
{model_t *m=data;(void)command;(void)params;(void)source;++m->legacy_calls;(void)ui_host_reply(host,id,1,"{}");}
static const char *schema="{\"type\":\"object\",\"properties\":{\"fields\":{\"type\":\"object\",\"properties\":{\"name\":{\"type\":\"string\"},\"amount\":{\"type\":\"number\"},\"enabled\":{\"type\":\"boolean\"},\"mode\":{\"type\":\"string\",\"enum\":[\"low\",\"high\"]},\"color\":{\"type\":\"string\"}},\"required\":[\"name\"],\"additionalProperties\":false}},\"required\":[\"fields\"],\"additionalProperties\":false}";
static ui_status_t revision(char *out,size_t capacity,void *data)
{model_t *m=data;snprintf(out,capacity,"r%u",m->revision);return UI_STATUS_OK;}
static ui_status_t read_model(char rev[128],char *json,size_t capacity,size_t *length,void *data)
{
    view_t *v=data;model_t *m=v->m;if(v->bad_read)return UI_STATUS_PLATFORM_ERROR;
    if(v->open_other)m->host->modal_component=m->form;
    snprintf(rev,128,"r%u",m->revision+(v->wrong_revision?1u:0u));
    if(v->dialog)*length=(size_t)snprintf(json,capacity,"{\"name\":\"%s\"}",m->name);
    else *length=(size_t)snprintf(json,capacity,"{\"name\":\"%s\",\"amount\":%.1f,\"enabled\":%s,\"mode\":\"%s\",\"color\":\"%s\"}",
      v->wrong_value?"not-the-submitted-value":m->name,m->amount,m->enabled?"true":"false",m->mode,m->color);return UI_STATUS_OK;
}
static void observe(const ui_cap_ui_notice_t *n,void *data)
{view_t *v=data;v->status=n->status;strcpy_s(v->code,sizeof(v->code),n->code);if(n->stage==UI_CAP_UI_ADMITTED)v->call=n->call_id;}
static ui_cap_form_state_t state(ui_component_t *c)
{ui_cap_form_state_t s={0};s.size=sizeof(s);s.version=1;CHECK(ui_capability_form_get_state(c,&s)==UI_STATUS_OK);return s;}
static ui_cap_form_field_state_t field_state(ui_component_t *c,const char *id)
{ui_cap_form_field_state_t s={0};s.size=sizeof(s);s.version=1;CHECK(ui_capability_form_get_field_state(c,id,&s)==UI_STATUS_OK);return s;}
static void assert_draft(ui_component_t *c,const char *id,const char *text)
{ui_cell_t cell={0};cell.size=sizeof(cell);CHECK(ui_component_get_field(c,id,&cell)==UI_STATUS_OK);CHECK(cell.text&&!strcmp(cell.text,text));}
static void apply(model_t *m,uint64_t call)
{
    ui_cap_json_doc_t doc={0};yyjson_val *fields,*value;ui_cap_completion_v2_t done={0};char before[512],after[512];
    snprintf(before,sizeof(before),"%s/%.1f/%d/%s/%s",m->name,m->amount,m->enabled,m->mode,m->color);
    CHECK(ui_cap_parse(m->pending,strlen(m->pending),UI_CAPABILITY_INPUT_BYTES,&doc)==UI_STATUS_OK);
    fields=yyjson_obj_get(yyjson_doc_get_root(doc.doc),"fields");
    value=yyjson_obj_get(fields,"name");if(value)strcpy_s(m->name,sizeof(m->name),yyjson_get_str(value));
    value=yyjson_obj_get(fields,"amount");if(value)m->amount=yyjson_get_num(value);
    value=yyjson_obj_get(fields,"enabled");if(value)m->enabled=yyjson_get_bool(value);
    value=yyjson_obj_get(fields,"mode");if(value)strcpy_s(m->mode,sizeof(m->mode),yyjson_get_str(value));
    value=yyjson_obj_get(fields,"color");if(value)strcpy_s(m->color,sizeof(m->color),yyjson_get_str(value));ui_cap_doc_free(&doc);
    snprintf(after,sizeof(after),"%s/%.1f/%d/%s/%s",m->name,m->amount,m->enabled,m->mode,m->color);
    if(strcmp(before,after))++m->revision;
    done.size=sizeof(done);done.version=2;done.outcome=UI_CAP2_OK;done.effect=strcmp(before,after)?UI_CAP2_EFFECT_APPLIED:UI_CAP2_EFFECT_NONE;
    done.output_json=m->invalid_output?"{\"invalid\":true}":"{\"saved\":true}";
    CHECK(ui_capability_complete_v2(m->host,call,&done)==UI_STATUS_OK);
}
static ui_status_t check(const ui_cap_target_v2_t *t,const char *params,void *data)
{model_t *m=data;(void)t;(void)params;if(m->open_modal)m->host->modal_component=m->dialog;return UI_STATUS_OK;}
static void execute(uint64_t call,const ui_cap_target_v2_t *t,const char *params,void *data)
{model_t *m=data;(void)t;++m->executions;strcpy_s(m->pending,sizeof(m->pending),params);if(!m->async)apply(m,call);}
static ui_status_t reconcile(uint64_t call,const ui_cap_target_v2_t *t,const char *params,ui_cap_reconciliation_v2_t *out,
    char *json,size_t capacity,size_t *length,void *data)
{model_t *m=data;(void)call;(void)t;(void)params;out->resolution=UI_CAP2_RESOLVED_APPLIED;snprintf(out->observed_revision,128,"r%u",m->revision);*length=(size_t)snprintf(json,capacity,"{}");return UI_STATUS_OK;}
static uint64_t bind_form(model_t *m,view_t *v,const char *id)
{
    ui_cap_form_binding_desc_t d={0};uint64_t binding=0;d.size=sizeof(d);d.version=1;d.component_id=id;d.capability_id="properties";
    d.resource.size=sizeof(d.resource);d.resource.version=2;CHECK(ui_capability_resource_ref_v2(m->host,"business",&d.resource)==UI_STATUS_OK);
    d.read_model=read_model;d.observe=observe;d.user_data=v;CHECK(ui_capability_form_bind(m->host,&d,&binding)==UI_STATUS_OK);return binding;
}
static void init(model_t *m)
{
    ui_host_config_t h={0};ui_component_desc_t d={0};ui_field_desc_t fields[5]={{0}};const char *options[]={"low","high"};
    ui_cap_resource_desc_t resource={0};ui_capability_desc_v2_t cap={0};ui_command_desc_t old={0};size_t i;
    h.size=sizeof(h);h.api_version=10;m->host=ui_host_create(&h);CHECK(m->host!=NULL);
    strcpy_s(m->name,sizeof(m->name),"initial");strcpy_s(m->mode,sizeof(m->mode),"low");strcpy_s(m->color,sizeof(m->color),"#112233");m->amount=1.0;
    resource.size=sizeof(resource);resource.version=1;resource.id="business";resource.kind=UI_CAP_DOCUMENT;resource.get_revision=revision;resource.user_data=m;
    CHECK(ui_capability_register_resource(m->host,&resource)==UI_STATUS_OK);CHECK(ui_capability_enable_read_entries(m->host)==UI_STATUS_OK);
    cap.size=sizeof(cap);cap.version=2;cap.capability_version=1;cap.id="properties";cap.name_utf8="Properties";cap.description_utf8="";cap.category="test";
    cap.target_kind=UI_CAP_DOCUMENT;cap.effect="write";cap.input_schema_json=schema;
    cap.output_schema_json="{\"type\":\"object\",\"properties\":{\"saved\":{\"type\":\"boolean\"}},\"required\":[\"saved\"],\"additionalProperties\":false}";
    cap.reconcile_schema_json="{\"type\":\"object\",\"properties\":{},\"additionalProperties\":false}";cap.check=check;cap.execute=execute;cap.reconcile=reconcile;cap.user_data=m;
    CHECK(ui_capability_register_v2(m->host,&cap)==UI_STATUS_OK);
    for(i=0;i<5;++i){fields[i].size=sizeof(fields[i]);fields[i].kind=(ui_value_kind_t)(i+1);}
    fields[0].id="name";fields[1].id="amount";fields[2].id="enabled";fields[3].id="mode";fields[4].id="color";
    fields[3].options=options;fields[3].option_count=2;
    old.size=sizeof(old);old.id="legacy-form";old.title="Legacy";old.handler=legacy;old.user_data=m;CHECK(ui_host_register_command(m->host,&old)==UI_STATUS_OK);
    d.size=sizeof(d);d.id="form";d.kind=UI_COMPONENT_FORM;d.fields=fields;d.field_count=5;d.commands.rename=old.id;d.commands.cancel=old.id;
    CHECK(ui_component_register(m->host,&d,&m->form)==UI_STATUS_OK);
    d.id="dialog";d.kind=UI_COMPONENT_DIALOG;d.field_count=1;CHECK(ui_component_register(m->host,&d,&m->dialog)==UI_STATUS_OK);
    m->a.m=m;m->b.m=m;m->b.dialog=1;m->a.binding=bind_form(m,&m->a,"form");m->b.binding=bind_form(m,&m->b,"dialog");
}
static ui_status_t action(model_t *m,ui_component_t *c,const char *name)
{ui_status_t s;ui_capabilities_human_begin(m->host,c);s=ui_components_form_action(c,name);ui_capabilities_human_end(m->host);return s;}
static void project(model_t *m,view_t *v,ui_component_t *c)
{char revision_text[128],json[2048];size_t length;ui_cap_form_projection_t p={0};CHECK(read_model(revision_text,json,sizeof(json),&length,v)==UI_STATUS_OK);
 p.size=sizeof(p);p.version=1;p.revision=revision_text;p.fields_json=json;p.length=length;CHECK(ui_capability_form_project(c,&p)==UI_STATUS_OK);(void)m;}
static void release_call(model_t *m,view_t *v)
{if(v->call){CHECK(ui_capability_forget_v2(m->host,v->call)==UI_STATUS_OK);v->call=0;}}
static void finish(model_t *m)
{
    m->host->modal_component=NULL;CHECK(ui_capability_ui_begin_unbind(m->host,m->a.binding)==UI_STATUS_OK);
    CHECK(ui_capability_ui_finish_unbind(m->host,m->a.binding)==UI_STATUS_OK);CHECK(ui_capability_ui_begin_unbind(m->host,m->b.binding)==UI_STATUS_OK);
    CHECK(ui_capability_ui_finish_unbind(m->host,m->b.binding)==UI_STATUS_OK);CHECK(ui_component_unregister(m->form)==UI_STATUS_OK);
    CHECK(ui_component_unregister(m->dialog)==UI_STATUS_OK);CHECK(ui_capability_quiesce(m->host)==UI_STATUS_OK);ui_host_destroy(m->host);
}
static void typed_shared_case(void)
{
    model_t m={0};init(&m);assert_draft(m.form,"name","initial");
    {ui_cap_input_binding_desc_t input={0};uint64_t duplicate=0;input.size=sizeof(input);input.version=1;input.kind=UI_CAP_INPUT_COMPONENT;input.component=m.form;
     input.capability_id="properties";input.resource.size=sizeof(input.resource);input.resource.version=2;input.purpose=UI_CAP_UI_INVOKE;
     CHECK(ui_capability_resource_ref_v2(m.host,"business",&input.resource)==UI_STATUS_OK);input.control_id="submit";input.trigger=UI_INPUT_POINTER_UP;input.pointer_button=1;CHECK(ui_capability_input_bind(m.host,&input,&duplicate)==UI_STATUS_ALREADY_EXISTS&&duplicate==0);}
    CHECK(ui_components_form_action(m.form,"submit")==UI_STATUS_PERMISSION_DENIED&&m.executions==0);
    ui_components_form_draft(m.form,"name","form-name");ui_components_form_draft(m.form,"amount","2.5");
    ui_components_form_draft(m.form,"enabled","true");ui_components_form_draft(m.form,"mode","high");ui_components_form_draft(m.form,"color","#445566");
    CHECK(state(m.form).dirty_fields==5);CHECK(action(&m,m.form,"submit")==UI_STATUS_OK);
    CHECK(m.executions==1&&!strcmp(m.name,"form-name")&&m.amount==2.5&&m.enabled&&!strcmp(m.mode,"high")&&!strcmp(m.color,"#445566"));
    CHECK(state(m.form).dirty_fields==0&&m.a.status==UI_STATUS_OK);release_call(&m,&m.a);
    project(&m,&m.b,m.dialog);assert_draft(m.dialog,"name","form-name");ui_components_form_draft(m.dialog,"name","dialog-name");
    m.host->modal_component=m.dialog;CHECK(action(&m,m.form,"submit")==UI_STATUS_PERMISSION_DENIED&&m.executions==1);
    CHECK(action(&m,m.dialog,"submit")==UI_STATUS_OK&&m.executions==2&&!strcmp(m.name,"dialog-name"));release_call(&m,&m.b);
    m.host->modal_component=NULL;project(&m,&m.a,m.form);assert_draft(m.form,"name","dialog-name");CHECK(state(m.dialog).dirty_fields==0);finish(&m);
}
static void conflict_snapshot_case(void)
{
    model_t m={0};unsigned before;uint64_t generation;init(&m);
    ui_components_form_draft(m.form,"name","private");generation=field_state(m.form,"name").generation;
    strcpy_s(m.name,sizeof(m.name),"external");++m.revision;project(&m,&m.a,m.form);
    assert_draft(m.form,"name","private");CHECK(field_state(m.form,"name").conflict&&field_state(m.form,"name").generation==generation);
    before=m.executions;CHECK(action(&m,m.form,"submit")==UI_STATUS_VALIDATION_FAILED&&m.executions==before);
    CHECK(ui_components_form_action(m.form,"conflict-keep")==UI_STATUS_PERMISSION_DENIED);CHECK(field_state(m.form,"name").conflict);
    CHECK(action(&m,m.form,"conflict-keep")==UI_STATUS_OK&&m.executions==before);CHECK(!field_state(m.form,"name").conflict);
    CHECK(action(&m,m.form,"submit")==UI_STATUS_OK&&!strcmp(m.name,"private"));release_call(&m,&m.a);
    m.async=1;ui_components_form_draft(m.form,"name","submitted");CHECK(action(&m,m.form,"submit")==UI_STATUS_OK);
    ui_components_form_draft(m.form,"name","newer-draft");apply(&m,m.a.call);
    assert_draft(m.form,"name","newer-draft");CHECK(!strcmp(m.name,"submitted")&&field_state(m.form,"name").modified&&field_state(m.form,"name").conflict);
    release_call(&m,&m.a);CHECK(action(&m,m.form,"conflict-reload")==UI_STATUS_OK);assert_draft(m.form,"name","submitted");CHECK(state(m.form).dirty_fields==0);
    ui_components_form_draft(m.form,"name","cancelled-draft");CHECK(action(&m,m.form,"cancel")==UI_STATUS_OK);assert_draft(m.form,"name","submitted");finish(&m);
}
static void invalid_readback_case(void)
{
    model_t m={0};init(&m);ui_components_form_draft(m.form,"amount","NaN");CHECK(action(&m,m.form,"submit")==UI_STATUS_VALIDATION_FAILED&&m.executions==0);
    CHECK(action(&m,m.form,"cancel")==UI_STATUS_OK);ui_components_form_draft(m.form,"name","saved-but-ui-failed");m.a.bad_read=1;
    CHECK(action(&m,m.form,"submit")==UI_STATUS_OK&&m.executions==1);CHECK(m.a.status==UI_STATUS_PLATFORM_ERROR&&!strcmp(m.a.code,"business_readback_failed"));
    CHECK(field_state(m.form,"name").modified&&field_state(m.form,"name").conflict);m.a.bad_read=0;release_call(&m,&m.a);
    CHECK(action(&m,m.form,"conflict-reload")==UI_STATUS_OK);CHECK(ui_component_unregister(m.form)==UI_STATUS_PENDING);finish(&m);
}
static void completion_edge_case(void)
{
    model_t m={0};ui_cap_result_v2_t result={0};uint64_t call;unsigned before;init(&m);
    ui_components_form_draft(m.form,"name","stale-draft");strcpy_s(m.name,sizeof(m.name),"unprojected-update");++m.revision;
    before=m.executions;CHECK(action(&m,m.form,"submit")==UI_STATUS_OK);CHECK(m.executions==before);
    CHECK(field_state(m.form,"name").modified&&field_state(m.form,"name").conflict);assert_draft(m.form,"name","stale-draft");release_call(&m,&m.a);
    CHECK(action(&m,m.form,"conflict-reload")==UI_STATUS_OK);ui_components_form_draft(m.form,"name","mismatch");m.a.wrong_value=1;
    CHECK(action(&m,m.form,"submit")==UI_STATUS_OK);CHECK(m.a.status==UI_STATUS_VALIDATION_FAILED&&!strcmp(m.a.code,"business_readback_failed"));
    CHECK(field_state(m.form,"name").modified&&field_state(m.form,"name").conflict);m.a.wrong_value=0;release_call(&m,&m.a);
    CHECK(action(&m,m.form,"conflict-reload")==UI_STATUS_OK);ui_components_form_draft(m.form,"name","bad-version");m.a.wrong_revision=1;
    CHECK(action(&m,m.form,"submit")==UI_STATUS_OK);CHECK(m.a.status==UI_STATUS_VALIDATION_FAILED&&field_state(m.form,"name").modified);
    m.a.wrong_revision=0;release_call(&m,&m.a);CHECK(action(&m,m.form,"conflict-reload")==UI_STATUS_OK);
    ui_components_form_draft(m.form,"name","unknown-applied");m.invalid_output=1;CHECK(action(&m,m.form,"submit")==UI_STATUS_OK);call=m.a.call;
    result.size=sizeof(result);result.version=2;CHECK(ui_capability_get_result_v2(m.host,call,&result)==UI_STATUS_OK&&result.outcome==UI_CAP2_UNKNOWN);
    CHECK(field_state(m.form,"name").modified&&field_state(m.form,"name").conflict);CHECK(ui_capability_forget_v2(m.host,call)==UI_STATUS_PENDING);
    before=m.executions;CHECK(action(&m,m.form,"submit")==UI_STATUS_PERMISSION_DENIED&&m.executions==before);finish(&m);
}
static void unbind_pending_case(void)
{
    model_t m={0};uint64_t call;ui_cap_result_v2_t result={0};init(&m);m.async=1;ui_components_form_draft(m.form,"name","survives-unbind");
    CHECK(action(&m,m.form,"submit")==UI_STATUS_OK);call=m.a.call;
    CHECK(ui_capability_ui_begin_unbind(m.host,m.a.binding)==UI_STATUS_OK);CHECK(ui_capability_ui_finish_unbind(m.host,m.a.binding)==UI_STATUS_OK);
    CHECK(ui_capability_form_get_state(m.form,&(ui_cap_form_state_t){sizeof(ui_cap_form_state_t),1})==UI_STATUS_NOT_FOUND);
    apply(&m,call);result.size=sizeof(result);result.version=2;CHECK(ui_capability_get_result_v2(m.host,call,&result)==UI_STATUS_OK&&result.outcome==UI_CAP2_OK);
    CHECK(!strcmp(m.name,"survives-unbind"));CHECK(ui_capability_forget_v2(m.host,call)==UI_STATUS_OK);
    m.a.call=0;m.a.binding=bind_form(&m,&m.a,"form");assert_draft(m.form,"name","survives-unbind");finish(&m);
}
#ifdef UI_CAP_FORMS_LIGHT
static ui_web_view_t *find_view(model_t *m,ui_component_t *c)
{ui_web_view_t *v;for(v=m->host->web_views;v;v=v->host_next)if(v->semantic_data==c)return v;return NULL;}
static void light_case(void)
{
    model_t m={0};ui_element_presentation_t presentation={0};ui_input_event_t event={0};ui_web_view_t *view;unsigned before;
    init(&m);CHECK(ui_component_mount_offscreen(m.form,480,420,96)==UI_STATUS_OK);CHECK(ui_component_mount_offscreen(m.dialog,340,200,96)==UI_STATUS_OK);
    view=find_view(&m,m.form);CHECK(view!=NULL);if(!view){finish(&m);return;}
    view->message_callback(view,"{\"action\":\"rename\"}",view->message_user_data);
    view->message_callback(view,"{\"action\":\"cancel\"}",view->message_user_data);CHECK(m.legacy_calls==0);
    ui_components_form_draft(m.form,"name","light-submit");presentation.size=sizeof(presentation);
    CHECK(ui_component_get_presentation(m.form,"submit",&presentation)==UI_STATUS_OK&&presentation.visible);
    event.size=sizeof(event);event.kind=UI_INPUT_POINTER_DOWN;event.pointer_button=1;
    event.x=presentation.rect.x+presentation.rect.width/2;event.y=presentation.rect.y+presentation.rect.height/2;
    (void)ui_component_dispatch_input(m.form,&event);event.kind=UI_INPUT_POINTER_UP;(void)ui_component_dispatch_input(m.form,&event);CHECK(m.executions==0);
    ui_capabilities_human_begin(m.host,view->user_data);CHECK(view->semantic_action(view,"submit",m.form)==UI_STATUS_OK);ui_capabilities_human_end(m.host);
    CHECK(m.executions==1&&!strcmp(m.name,"light-submit")&&state(m.form).dirty_fields==0);release_call(&m,&m.a);
    CHECK(ui_component_show_dialog(m.dialog)==UI_STATUS_OK);view=find_view(&m,m.dialog);ui_components_form_draft(m.dialog,"name","modal-submit");
    ui_capabilities_human_begin(m.host,view->user_data);CHECK(view->semantic_action(view,"@key:13:0:field-name",m.dialog)==UI_STATUS_OK);ui_capabilities_human_end(m.host);
    CHECK(m.executions==2&&!strcmp(m.name,"modal-submit"));release_call(&m,&m.b);
    ui_components_form_draft(m.dialog,"name","retained-on-reopen");CHECK(ui_component_close_dialog(m.dialog)==UI_STATUS_OK);
    strcpy_s(m.name,sizeof(m.name),"changed-while-closed");++m.revision;
    CHECK(ui_component_show_dialog(m.dialog)==UI_STATUS_OK);assert_draft(m.dialog,"name","retained-on-reopen");
    CHECK(field_state(m.dialog,"name").conflict);ui_capabilities_human_begin(m.host,view->user_data);
    CHECK(view->semantic_action(view,"submit",m.dialog)==UI_STATUS_VALIDATION_FAILED);ui_capabilities_human_end(m.host);CHECK(m.executions==2);
    before=m.executions;CHECK(ui_component_close_dialog(m.dialog)==UI_STATUS_OK);m.b.open_other=1;
    CHECK(ui_component_show_dialog(m.dialog)==UI_STATUS_ALREADY_EXISTS&&m.host->modal_component==m.form);m.b.open_other=0;m.host->modal_component=NULL;
    project(&m,&m.a,m.form);ui_components_form_draft(m.form,"name","blocked");m.open_modal=1;
    view=find_view(&m,m.form);ui_capabilities_human_begin(m.host,view->user_data);CHECK(view->semantic_action(view,"submit",m.form)==UI_STATUS_OK);ui_capabilities_human_end(m.host);
    CHECK(m.executions==before);m.open_modal=0;CHECK(ui_component_close_dialog(m.dialog)==UI_STATUS_OK);release_call(&m,&m.a);finish(&m);
}
#endif
int main(void)
{typed_shared_case();conflict_snapshot_case();invalid_readback_case();completion_edge_case();unbind_pending_case();
#ifdef UI_CAP_FORMS_LIGHT
light_case();
#endif
printf("C2-B checks=%u failures=%u\n",checks,failures);return failures?1:0;}
