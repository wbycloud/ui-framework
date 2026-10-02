#include "ui_framework/application.h"
#include "ui_framework/shell.h"
#include "ui_framework/light_web.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct counter {
    ui_app_context_t context;
    ui_web_backend_t *backend;
    ui_web_view_t *view;
    unsigned count;
    char snapshot_json[64];
} counter_t;

static const char page[] =
    "<html><head><style>body{background:#f4f6fa;color:#17243b;padding:24px;}"
    "button{background:#2867dc;color:#ffffff;padding:12px;border-radius:8px;}"
    "p{font-size:18px;margin:12px;}</style></head><body>"
    "<p>Web Counter / API 2</p><p id='count'>0</p>"
    "<button id='add' onclick=\"ui.postMessage({action:'increment'})\">+1</button>"
    "<script>ui.onmessage=function(data){document.getElementById('count').textContent="
    "String(data.count);};</script></body></html>";

static void increment(ui_host_t *host, uint64_t request, const char *command,
    const char *params, const char *source, void *data)
{
    counter_t *state = (counter_t *)data;
    char json[64];
    (void)command; (void)params; (void)source;
    ++state->count;
    (void)snprintf(json, sizeof(json), "{\"count\":%u}", state->count);
    if (state->view) (void)ui_web_view_post_json(state->view, json);
    (void)ui_host_reply(host, request, 1, json);
}

static const char *snapshot(void *data)
{
    counter_t *state = (counter_t *)data;
    (void)snprintf(state->snapshot_json, sizeof(state->snapshot_json),
                   "{\"count\":%u}", state->count);
    return state->snapshot_json;
}

static void message(ui_web_view_t *view, const char *json, void *data)
{
    counter_t *state = (counter_t *)data;
    (void)view;
    if (!strcmp(json, "{\"action\":\"increment\"}"))
        (void)ui_host_invoke(state->context.host, "counter.increment", "{}", "web-ui");
}

static ui_status_t UI_APP_CALL create(const ui_app_context_t *context,
    void **state_out, ui_assistant_config_t *assistant)
{
    counter_t *state = (counter_t *)calloc(1, sizeof(*state));
    ui_command_desc_t command = {0};
    ui_menu_item_desc_t menu = {0};
    ui_status_t status;
    if (!state) return UI_STATUS_OUT_OF_MEMORY;
    state->context = *context; *state_out = state;
    command.size = sizeof(command); command.id = "counter.increment";
    command.title = "Increment"; command.params_schema_json = "{\"type\":\"object\"}";
    command.handler = increment; command.user_data = state;
    status = ui_host_register_command(context->host, &command);
    if (status != UI_STATUS_OK) return status;
    menu.size = sizeof(menu); menu.id = "counter.menu.increment";
    menu.menu_path = "Counter"; menu.title = "Increment";
    menu.command_id = command.id;
    status = ui_host_register_menu_item(context->host, &menu);
    if (status != UI_STATUS_OK) return status;
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
    ui_assistant_command_desc_t command = {0};
    state->context = *context;
    if (!slot) return UI_STATUS_UNSUPPORTED;
    command.size = sizeof(command); command.id = "counter.increment";
    command.permission = UI_ASSISTANT_PERMISSION_EDIT;
    command.params_schema_json = "{\"type\":\"object\"}";
    status = ui_assistant_register_command(context->assistant, &command);
    if (status != UI_STATUS_OK) return status;
    config.size = sizeof(config); config.parent_hwnd = ui_content_slot_native_handle(slot);
    config.enable_native_input = 1;
    state->backend = ui_light_web_backend_create(&config);
    if (!state->backend) return UI_STATUS_OUT_OF_MEMORY;
    state->view = ui_web_view_create(context->host, state->backend);
    if (!state->view) return UI_STATUS_PLATFORM_ERROR;
    status = ui_web_view_set_message_callback(state->view, message, state);
    if (status == UI_STATUS_OK) status = ui_web_view_load_html(state->view, page);
    if (status == UI_STATUS_OK) status = ui_content_slot_attach_web_view(slot, state->view);
    return status;
}

static void UI_APP_CALL active(void *data, int value) { (void)data; (void)value; }
static ui_app_close_decision_t UI_APP_CALL close_app(void *data, ui_app_close_reason_t reason)
{ (void)data; (void)reason; return UI_APP_CLOSE_ALLOW; }
static void UI_APP_CALL unmount(void *data)
{
    counter_t *state = (counter_t *)data;
    if (state->view) ui_web_view_destroy(state->view);
    state->view = NULL;
    if (state->backend) ui_light_web_backend_destroy(state->backend);
    state->backend = NULL;
}
static void UI_APP_CALL destroy(void *data) { free(data); }
static ui_status_t UI_APP_CALL shutdown_module(void) { return UI_STATUS_OK; }

UI_APP_EXPORT const ui_app_descriptor_t *UI_APP_CALL ui_app_query_v1(void)
{
    static const ui_app_descriptor_t descriptor = {
        sizeof(ui_app_descriptor_t), UI_APPLICATION_ABI_VERSION, UI_FRAMEWORK_API_VERSION,
        create, mount, active, close_app, unmount, destroy, shutdown_module
    };
    return &descriptor;
}
