#include "ui_framework/application.h"
#include "ui_framework/shell.h"
#include "ui_framework/light_web.h"
#include "../p2_support.h"
#include <limits.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct counter {
    ui_app_context_t context;
    ui_web_backend_t *backend;
    ui_web_view_t *view;
    unsigned count;uint64_t revision;p2_app_t capabilities;
    char snapshot_json[64];
} counter_t;

static const char page[] =
    "<html><head><style>body{background:#f4f6fa;color:#17243b;padding:24px;}"
    "button{background:#2867dc;color:#ffffff;padding:12px;border-radius:8px;}"
    "p{font-size:18px;margin:12px;}</style></head><body>"
    "<p>Web Counter / API 10</p><p id='count'>0</p>"
    "<button id='add'>+1</button>"
    "<script>ui.onmessage=function(data){document.getElementById('count').textContent="
    "String(data.count);};</script></body></html>";

static const char counter_output[]="{\"type\":\"object\",\"properties\":{\"count\":{\"type\":\"integer\",\"minimum\":0},\"revision\":{\"type\":\"string\"}},\"required\":[\"count\",\"revision\"],\"additionalProperties\":false}";
static const p2_spec_t counter_specs[]={
 {"counter.read","Read actual counter","read",p2_empty_schema,counter_output,"document",0},
 {"counter.increment","Increment","write",p2_empty_schema,counter_output,"document",0}
};
static ui_status_t counter_revision(void *data,unsigned resource,char out[128])
{counter_t *s=data;(void)resource;snprintf(out,128,"counter-%llu",(unsigned long long)s->revision);return UI_STATUS_OK;}
static ui_status_t counter_describe(void *data,unsigned resource,yyjson_mut_doc *doc,yyjson_mut_val *root)
{counter_t *s=data;char revision[128];(void)counter_revision(s,resource,revision);yyjson_mut_obj_add_uint(doc,root,"count",s->count);yyjson_mut_obj_add_strcpy(doc,root,"revision",revision);return UI_STATUS_OK;}
static ui_status_t counter_run(void *data,const char *id,yyjson_val *params,yyjson_mut_doc *doc,yyjson_mut_val *root,uint64_t call)
{counter_t *s=data;(void)params;(void)call;if(!strcmp(id,"counter.increment")){char json[64];if(s->count==UINT_MAX||s->revision==UINT64_MAX)return UI_STATUS_LIMIT_EXCEEDED;++s->count;++s->revision;snprintf(json,sizeof(json),"{\"count\":%u}",s->count);if(s->view&&!s->capabilities.staging){ui_status_t status=ui_web_view_post_json(s->view,json);if(status)return status;}}return counter_describe(s,0,doc,root);}
typedef struct counter_business {unsigned count;uint64_t revision;} counter_business_t;
static ui_status_t counter_capture(void *data,void **out,size_t *bytes)
{counter_t *s=data;counter_business_t *b=calloc(1,sizeof(*b));if(!b)return UI_STATUS_OUT_OF_MEMORY;b->count=s->count;b->revision=s->revision;*out=b;*bytes=sizeof(*b);return UI_STATUS_OK;}
static ui_status_t counter_restore(void *data,const void *copy,size_t bytes,int publish)
{counter_t *s=data;const counter_business_t *b=copy;uint64_t revision=s->revision;if(!b||bytes!=sizeof(*b))return UI_STATUS_INVALID_ARGUMENT;if(publish&&revision==UINT64_MAX)return UI_STATUS_LIMIT_EXCEEDED;s->count=b->count;s->revision=publish?revision+1:b->revision;return UI_STATUS_OK;}
static ui_status_t counter_publish(void *data)
{counter_t *s=data;char json[64];snprintf(json,sizeof(json),"{\"count\":%u}",s->count);return s->view?ui_web_view_post_json(s->view,json):UI_STATUS_OK;}
static const char *snapshot(void *data)
{
    counter_t *state = (counter_t *)data;
    (void)snprintf(state->snapshot_json, sizeof(state->snapshot_json),
                   "{\"count\":%u}", state->count);
    return state->snapshot_json;
}

static ui_status_t UI_APP_CALL create(const ui_app_context_t *context,
    void **state_out, ui_assistant_config_t *assistant)
{
    counter_t *state = (counter_t *)calloc(1, sizeof(*state));
    ui_menu_item_desc_t menu = {0};
    ui_status_t status;
    if (!state) return UI_STATUS_OUT_OF_MEMORY;
    state->context = *context; *state_out = state;
    {const char *resources[]={"counter.document"};ui_cap_resource_kind_t kinds[]={UI_CAP_DOCUMENT};state->revision=1;state->capabilities.capture_business=counter_capture;state->capabilities.restore_business=counter_restore;state->capabilities.project_business=counter_publish;state->capabilities.run=counter_run;state->capabilities.revision=counter_revision;state->capabilities.describe=counter_describe;status=p2_register(&state->capabilities,context,state,counter_specs,2,resources,kinds,1);if(status)return status;}
    menu.size = sizeof(menu); menu.id = "counter.menu.increment";
    menu.menu_path = "Counter"; menu.title = "Increment";
    menu.command_id = "counter.increment";
    status = ui_host_register_menu_item(context->host, &menu);
    if (status != UI_STATUS_OK) return status;
    status=p2_bind_menu(&state->capabilities,"counter.menu.increment","counter.increment",NULL);if(status)return status;
    assistant->user_data = state; assistant->snapshot = snapshot;
    assistant->max_permission = UI_ASSISTANT_PERMISSION_EDIT;
    assistant->source_id = "counter.assistant";
    return UI_STATUS_OK;
}

static ui_status_t UI_APP_CALL mount(void *data, const ui_app_context_t *context)
{
    counter_t *state = (counter_t *)data;
    ui_content_slot_t *slot = ui_shell_get_content_slot(ui_host_get_shell(context->host), NULL);
    ui_light_web_config_t config = {0};
    ui_status_t status;
    state->context = *context;
    if (!slot) return UI_STATUS_UNSUPPORTED;
    config.size = sizeof(config); config.parent_hwnd = ui_content_slot_native_handle(slot);
    config.enable_native_input = 1;
    state->backend = ui_light_web_backend_create(&config);
    if (!state->backend) return UI_STATUS_OUT_OF_MEMORY;
    state->view = ui_web_view_create(context->host, state->backend);
    if (!state->view) return UI_STATUS_PLATFORM_ERROR;
    status = ui_web_view_load_html(state->view, page);
    if (status == UI_STATUS_OK) status = ui_content_slot_attach_web_view(slot, state->view);
    if(status)return status;status=p2_mount(&state->capabilities,context);if(status)return status;
    {ui_cap_input_binding_desc_t d={0};uint64_t binding;d.size=sizeof(d);d.version=1;d.kind=UI_CAP_INPUT_LIGHT_CONTROL;d.view=state->view;d.control_id="add";d.trigger=UI_INPUT_POINTER_UP;d.pointer_button=1;d.capability_id="counter.increment";d.resource=state->capabilities.resources[0].ref;d.purpose=UI_CAP_UI_INVOKE;d.observe=p2_notice;d.user_data=p2_find(&state->capabilities,"counter.increment");return ui_capability_input_bind(context->host,&d,&binding);}
}

static void UI_APP_CALL active(void *data, int value) { (void)data; (void)value; }
static ui_app_close_decision_t UI_APP_CALL close_app(void *data, ui_app_close_reason_t reason)
{ ((counter_t *)data)->capabilities.closing=1;(void)reason; return UI_APP_CLOSE_ALLOW; }
static void UI_APP_CALL unmount(void *data)
{
    counter_t *state = (counter_t *)data;
    if (state->view) ui_web_view_destroy(state->view);
    state->view = NULL;
    if (state->backend) ui_light_web_backend_destroy(state->backend);
    state->backend = NULL;
}
static void UI_APP_CALL destroy(void *data) {counter_t *s=data;p2_dispose(&s->capabilities);free(data);}
static ui_status_t UI_APP_CALL shutdown_module(void) { return UI_STATUS_OK; }

UI_APP_EXPORT const ui_app_descriptor_t *UI_APP_CALL ui_app_query_v1(void)
{
    static const ui_app_descriptor_t descriptor = {
        sizeof(ui_app_descriptor_t), UI_APPLICATION_ABI_VERSION, UI_FRAMEWORK_API_VERSION,
        create, mount, active, close_app, unmount, destroy, shutdown_module
    };
    return &descriptor;
}
