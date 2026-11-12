/* Public-API, production-linked synthetic read-only uapp. */
#include "ui_framework/application.h"
#include "ui_framework/capability.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
typedef struct memory {ui_host_t *host;int value;char revision[8];} memory_t;
static ui_status_t revision(char *out,size_t cap,void *data)
{memory_t *m=data;return strcpy_s(out,cap,m->revision)?UI_STATUS_INVALID_ARGUMENT:UI_STATUS_OK;}
static void read_value(uint64_t id,const ui_cap_target_t *target,const char *params,void *data)
{memory_t *m=data;char json[96];ui_cap_result_t r={0};(void)target;(void)params;snprintf(json,sizeof(json),"{\"value\":%d,\"revision\":\"%s\"}",m->value,m->revision);r.size=sizeof(r);r.version=1;r.state=UI_CAP_FINISHED;r.outcome=UI_CAP_OK;r.output_json=json;(void)ui_capability_complete(m->host,id,&r);}
static ui_status_t UI_APP_CALL create(const ui_app_context_t *c,void **out,ui_assistant_config_t *assistant)
{
 (void)assistant;memory_t *m=calloc(1,sizeof(*m));if(!m)return UI_STATUS_OUT_OF_MEMORY;*out=m;m->host=c->host;m->value=10;strcpy_s(m->revision,8,"r1");
 ui_cap_resource_desc_t r={0};r.size=sizeof(r);r.version=1;r.id="memory.document";r.kind=UI_CAP_DOCUMENT;r.get_revision=revision;r.user_data=m;ui_status_t s=ui_capability_register_resource(c->host,&r);if(s)return s;
 ui_capability_desc_t d={0};d.size=sizeof(d);d.version=d.capability_version=1;d.id="memory.read";d.name_utf8="Read synthetic memory value and revision";d.description_utf8="Read-only RP-C fixture; value 10 and revision r1.";d.category="memory";d.effect="read";d.target_kind=UI_CAP_DOCUMENT;d.input_schema_json="{\"type\":\"object\",\"additionalProperties\":false}";d.output_schema_json="{\"type\":\"object\",\"properties\":{\"value\":{\"type\":\"integer\"},\"revision\":{\"type\":\"string\"}},\"required\":[\"value\",\"revision\"],\"additionalProperties\":false}";d.execute=read_value;d.user_data=m;return ui_capability_register(c->host,&d);
}
static ui_status_t UI_APP_CALL mount(void *p,const ui_app_context_t *c){(void)p;(void)c;return UI_STATUS_OK;}
static ui_app_close_decision_t UI_APP_CALL close_app(void *p,ui_app_close_reason_t reason){(void)p;(void)reason;return UI_APP_CLOSE_ALLOW;}
static void UI_APP_CALL unmount(void *p){(void)p;}
static void UI_APP_CALL destroy(void *p){free(p);}
UI_APP_EXPORT const ui_app_descriptor_t *UI_APP_CALL ui_app_query_v1(void)
{static const ui_app_descriptor_t d={sizeof(d),1,UI_FRAMEWORK_API_VERSION,create,mount,NULL,close_app,unmount,destroy,NULL};return &d;}
