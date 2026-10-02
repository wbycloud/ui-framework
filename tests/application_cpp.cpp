#include "ui_framework/application.h"

#include <cstdio>
#include <new>

struct cpp_state {
    ui_app_context_t context;
    unsigned count;
    int active;
    uint64_t next_transaction, transaction_id, undo_id;
    unsigned begin_count, undo_count;
    char snapshot[192];
};

static unsigned live_instances;

static const char *cpp_snapshot(void *data)
{
    cpp_state *state = static_cast<cpp_state *>(data);
    std::snprintf(state->snapshot, sizeof(state->snapshot),
        "{\"instance_id\":%llu,\"count\":%u,\"active\":%s,\"transaction_id\":%llu}",
        static_cast<unsigned long long>(state->context.instance_id), state->count,
        state->active ? "true" : "false",
        static_cast<unsigned long long>(state->transaction_id));
    return state->snapshot;
}

static void cpp_command(ui_host_t *host, uint64_t request, const char *command,
    const char *params, const char *source, void *data)
{
    cpp_state *state = static_cast<cpp_state *>(data);
    (void)command; (void)params; (void)source;
    ++state->count;
    (void)ui_host_reply(host, request, 1, cpp_snapshot(state));
}

static ui_status_t cpp_begin(const char *label, uint64_t *id, void *data)
{
    cpp_state *state = static_cast<cpp_state *>(data);
    (void)label;
    if (state->transaction_id) return UI_STATUS_ALREADY_EXISTS;
    state->begin_count = state->count;
    state->transaction_id = ++state->next_transaction;
    *id = state->transaction_id;
    return UI_STATUS_OK;
}
static ui_status_t cpp_commit(uint64_t id, void *data)
{
    cpp_state *state = static_cast<cpp_state *>(data);
    if (id != state->transaction_id) return UI_STATUS_NOT_FOUND;
    state->undo_count = state->begin_count;
    state->undo_id = id;
    state->transaction_id = 0;
    return UI_STATUS_OK;
}
static ui_status_t cpp_rollback(uint64_t id, void *data)
{
    cpp_state *state = static_cast<cpp_state *>(data);
    if (id != state->transaction_id) return UI_STATUS_NOT_FOUND;
    state->count = state->begin_count;
    state->transaction_id = 0;
    return UI_STATUS_OK;
}
static ui_status_t cpp_undo(uint64_t id, void *data)
{
    cpp_state *state = static_cast<cpp_state *>(data);
    if (state->transaction_id || id != state->undo_id) return UI_STATUS_NOT_FOUND;
    state->count = state->undo_count;
    state->undo_id = 0;
    return UI_STATUS_OK;
}

static ui_status_t UI_APP_CALL cpp_create(const ui_app_context_t *context,
    void **out, ui_assistant_config_t *assistant)
{
    cpp_state *state = new (std::nothrow) cpp_state{};
    ui_command_desc_t command = {};
    ui_menu_item_desc_t menu = {};
    ui_status_t status;
    if (!state) return UI_STATUS_OUT_OF_MEMORY;
    state->context = *context;
    *out = state;
    ++live_instances;
    *assistant = ui_assistant_config_t{};
    assistant->size = sizeof(*assistant);
    assistant->max_permission = UI_ASSISTANT_PERMISSION_EDIT;
    assistant->user_data = state;
    assistant->snapshot = cpp_snapshot;
    assistant->transaction_begin = cpp_begin;
    assistant->transaction_commit = cpp_commit;
    assistant->transaction_rollback = cpp_rollback;
    assistant->transaction_undo = cpp_undo;
    command.size = sizeof(command);
    command.id = "fixture.cpp.count";
    command.title = "C++ counter";
    command.handler = cpp_command;
    command.user_data = state;
    status = ui_host_register_command(context->host, &command);
    if (status != UI_STATUS_OK) return status;
    menu.size = sizeof(menu);
    menu.id = "fixture.cpp.menu";
    menu.menu_path = "C++ fixture";
    menu.title = "Count";
    menu.command_id = command.id;
    return ui_host_register_menu_item(context->host, &menu);
}

static ui_status_t UI_APP_CALL cpp_mount(void *data, const ui_app_context_t *context)
{
    cpp_state *state = static_cast<cpp_state *>(data);
    ui_assistant_command_desc_t command = {};
    state->context = *context;
    command.size = sizeof(command);
    command.id = "fixture.cpp.count";
    command.permission = UI_ASSISTANT_PERMISSION_EDIT;
    command.params_schema_json = "{\"type\":\"object\"}";
    return ui_assistant_register_command(context->assistant, &command);
}

static void UI_APP_CALL cpp_active(void *data, int active)
{
    static_cast<cpp_state *>(data)->active = active != 0;
}
static ui_app_close_decision_t UI_APP_CALL cpp_close(void *data, ui_app_close_reason_t reason)
{
    (void)data; (void)reason;
    return UI_APP_CLOSE_ALLOW;
}
static void UI_APP_CALL cpp_unmount(void *data) { (void)data; }
static void UI_APP_CALL cpp_destroy(void *data)
{
    delete static_cast<cpp_state *>(data);
    --live_instances;
}
static ui_status_t UI_APP_CALL cpp_shutdown(void)
{
    return live_instances == 0 ? UI_STATUS_OK : UI_STATUS_PLATFORM_ERROR;
}

extern "C" UI_APP_EXPORT const ui_app_descriptor_t *UI_APP_CALL ui_app_query_v1(void)
{
    static const ui_app_descriptor_t descriptor = {
        sizeof(ui_app_descriptor_t), UI_APPLICATION_ABI_VERSION, UI_FRAMEWORK_API_VERSION,
        cpp_create, cpp_mount, cpp_active, cpp_close, cpp_unmount, cpp_destroy, cpp_shutdown
    };
    return &descriptor;
}
