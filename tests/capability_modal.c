#include "ui_framework/capability.h"
#include "ui_framework/components.h"
#include <stdio.h>
#include <string.h>
static int failures,checks;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);++failures;}}while(0)
typedef struct model {ui_host_t *host;ui_component_t *dialog;int mode,revisions,executions;} model_t;
static ui_status_t revision(char *buffer,size_t capacity,void *data)
{
    model_t *m=data;++m->revisions;
    if((m->mode==2&&m->revisions==1)||(m->mode==3&&m->revisions==2))CHECK(ui_component_show_dialog(m->dialog)==UI_STATUS_OK);
    (void)snprintf(buffer,capacity,"1");return UI_STATUS_OK;
}
static ui_status_t check(const ui_cap_target_t *target,const char *params,void *data)
{model_t *m=data;(void)target;(void)params;if(m->mode==1)CHECK(ui_component_show_dialog(m->dialog)==UI_STATUS_OK);return UI_STATUS_OK;}
static void execute(uint64_t id,const ui_cap_target_t *target,const char *params,void *data)
{model_t *m=data;ui_cap_result_t r={0};(void)target;(void)params;++m->executions;r.size=sizeof(r);r.version=1;r.outcome=UI_CAP_OK;r.output_json="{}";CHECK(ui_capability_complete(m->host,id,&r)==UI_STATUS_OK);}
int main(void)
{
    ui_host_config_t hc={0};ui_component_desc_t dialog={0};ui_field_desc_t field={0};ui_cap_resource_desc_t resource={0};ui_capability_desc_t cap={0};model_t m={0};int mode;
    hc.size=sizeof(hc);hc.api_version=UI_FRAMEWORK_API_VERSION;m.host=ui_host_create(&hc);CHECK(m.host!=NULL);if(!m.host)return 1;
    field.size=sizeof(field);field.id="name";field.title="Name";field.kind=UI_VALUE_TEXT;
    dialog.size=sizeof(dialog);dialog.id="modal";dialog.title="Modal execution gate";dialog.kind=UI_COMPONENT_DIALOG;dialog.fields=&field;dialog.field_count=1;
    CHECK(ui_component_register(m.host,&dialog,&m.dialog)==UI_STATUS_OK);if(!m.dialog)return 1;
    CHECK(ui_component_mount_offscreen(m.dialog,320,240,96)==UI_STATUS_OK);
    resource.size=sizeof(resource);resource.version=1;resource.id="model";resource.kind=UI_CAP_DOCUMENT;resource.get_revision=revision;resource.user_data=&m;
    CHECK(ui_capability_register_resource(m.host,&resource)==UI_STATUS_OK);
    cap.size=sizeof(cap);cap.version=cap.capability_version=1;cap.id="read";cap.name_utf8="Read";cap.description_utf8="Modal callback regression";cap.category="test";
    cap.input_schema_json=cap.output_schema_json="{\"type\":\"object\",\"additionalProperties\":false}";
    cap.target_kind=UI_CAP_DOCUMENT;cap.effect="read";cap.check=check;cap.execute=execute;cap.user_data=&m;CHECK(ui_capability_register(m.host,&cap)==UI_STATUS_OK);
    for(mode=1;mode<=3;++mode){
        ui_cap_target_t target={0};ui_cap_result_t result={0};ui_component_state_t state={0};uint64_t id=0;int before=m.executions;
        target.size=sizeof(target);target.version=1;m.mode=0;CHECK(ui_capability_bind(m.host,"model",&target)==UI_STATUS_OK);
        CHECK(ui_capability_prepare(m.host,"read",&target,"{}",&id)==UI_STATUS_OK);m.mode=mode;m.revisions=0;
        CHECK(ui_capability_dispatch(m.host,id)==UI_STATUS_OK);state.size=sizeof(state);CHECK(ui_component_get_state(m.dialog,&state)==UI_STATUS_OK&&state.modal);
        result.size=sizeof(result);result.version=1;CHECK(ui_capability_get_result(m.host,id,&result)==UI_STATUS_OK);
        CHECK(result.state==UI_CAP_FINISHED&&result.outcome==UI_CAP_REJECTED&&!strcmp(result.code,"unavailable"));CHECK(m.executions==before);
        CHECK(ui_capability_dispatch(m.host,id)==UI_STATUS_OK&&m.executions==before);CHECK(ui_capability_forget(m.host,id)==UI_STATUS_OK);
        CHECK(ui_component_close_dialog(m.dialog)==UI_STATUS_OK);m.mode=0;
        CHECK(ui_capability_prepare(m.host,"read",&target,"{}",&id)==UI_STATUS_OK);CHECK(ui_capability_dispatch(m.host,id)==UI_STATUS_OK);
        CHECK(ui_capability_get_result(m.host,id,&result)==UI_STATUS_OK&&result.state==UI_CAP_FINISHED&&result.outcome==UI_CAP_OK);CHECK(m.executions==before+1);
        CHECK(ui_capability_dispatch(m.host,id)==UI_STATUS_OK&&m.executions==before+1);CHECK(ui_capability_forget(m.host,id)==UI_STATUS_OK);
    }
    ui_host_destroy(m.host);printf("capability modal: %d checks, %d failures\n",checks,failures);return failures?1:0;
}
