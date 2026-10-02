#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui_framework/application.h"

typedef struct fixture_state {
    ui_app_context_t context;
    ui_surface_t *surface;
    HWND window, edit;
    int count, active, cancels, callback_mode;
    ui_app_close_decision_t close_mode;
    char snapshot[256];
} fixture_state_t;

static int module_identity, instance_count, shutdown_failed;
static HINSTANCE module;
static ATOM window_class;
static const wchar_t class_name[] = L"UiFrameworkLifecycleFixtureV1";
static const char *const commands[] = {
    "fixture.add", "eda.add_block", "fixture.delayed", "fixture.close_refuse",
    "fixture.close_wait", "fixture.close_allow", "fixture.shutdown_fail",
    "fixture.shutdown_ok", "fixture.destructive", "fixture.close_self",
    "fixture.arm_frame", "fixture.arm_input", "fixture.arm_external"
};

static const char *state_snapshot(void *data)
{
    fixture_state_t *s = data;
    (void)snprintf(s->snapshot, sizeof(s->snapshot),
        "{\"instance_id\":%llu,\"count\":%d,\"active\":%s,\"cancels\":%d}",
        (unsigned long long)s->context.instance_id, s->count,
        s->active ? "true" : "false", s->cancels);
    return s->snapshot;
}

static ui_status_t validate(const char *json, void *data)
{
    (void)data;
    return json && strcmp(json, "{}") == 0 ? UI_STATUS_OK : UI_STATUS_VALIDATION_FAILED;
}

static void execute(ui_host_t *host, uint64_t request, const char *command,
    const char *json, const char *source, void *data)
{
    fixture_state_t *s = data;
    (void)json; (void)source;
    if (!strcmp(command, "fixture.delayed")) return;
    if (!strcmp(command, "fixture.close_self")) {
        (void)ui_workspace_close(s->context.workspace, s->context.instance_id, UI_APP_CLOSE_TAB);
        ui_workspace_poll(s->context.workspace);
        (void)ui_host_reply(host, request, IsWindow(s->window), "{\"alive_in_callback\":true}");
        return;
    }
    if (!strcmp(command, "fixture.close_refuse")) s->close_mode = UI_APP_CLOSE_REFUSE;
    else if (!strcmp(command, "fixture.close_wait")) s->close_mode = UI_APP_CLOSE_WAIT;
    else if (!strcmp(command, "fixture.close_allow")) s->close_mode = UI_APP_CLOSE_ALLOW;
    else if (!strcmp(command, "fixture.shutdown_fail")) shutdown_failed = 1;
    else if (!strcmp(command, "fixture.shutdown_ok")) shutdown_failed = 0;
    else if (!strcmp(command, "fixture.arm_frame")) s->callback_mode = 1;
    else if (!strcmp(command, "fixture.arm_input")) s->callback_mode = 2;
    else if (!strcmp(command, "fixture.arm_external")) s->callback_mode = 3;
    else ++s->count;
    (void)ui_host_reply(host, request, 1, state_snapshot(s));
}

static void cancel(uint64_t request, void *data)
{
    fixture_state_t *s = data;
    ++s->cancels;
    (void)ui_workspace_post_result(s->context.workspace, s->context.instance_id,
        request, 0, "{\"cancelled\":true}");
}

static void callback_close(fixture_state_t *s, int mode)
{
    HWND surface_window;
    if (s->callback_mode != mode) return;
    s->callback_mode = 0;
    (void)ui_workspace_close(s->context.workspace, s->context.instance_id, UI_APP_CLOSE_TAB);
    ui_workspace_poll(s->context.workspace);
    surface_window = (HWND)ui_surface_native_handle(s->surface);
    (void)ui_host_emit_event(s->context.host, "fixture.callback.alive",
        IsWindow(s->window) && IsWindow(surface_window) ? "true" : "false");
}

static void frame(ui_surface_t *surface, void *data)
{ (void)surface; callback_close(data, 1); }

static void input(ui_surface_t *surface, const ui_input_event_t *event, void *data)
{
    (void)surface;
    if (event->kind == UI_INPUT_KEY_DOWN) callback_close(data, 2);
}

static LRESULT CALLBACK fixture_proc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp)
{
    fixture_state_t *s = (fixture_state_t *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    ui_app_context_t callback_context;
    LRESULT result;
    int guarded;
    if (message == WM_NCCREATE) {
        s = (fixture_state_t *)((CREATESTRUCTW *)lp)->lpCreateParams;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)s);
    }
    guarded = s != NULL;
    if (guarded) {
        callback_context = s->context;
        ui_app_callback_enter(&callback_context);
    }
    if (message == WM_APP + 101u && s != NULL) callback_close(s, 3);
    if (message == WM_SIZE && s != NULL && s->edit != NULL)
        MoveWindow(s->edit, 0, 0, LOWORD(lp), HIWORD(lp), TRUE);
    if (message == WM_NCDESTROY) SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
    result = DefWindowProcW(hwnd, message, wp, lp);
    if (guarded) ui_app_callback_leave(&callback_context);
    return result;
}

static ui_status_t UI_APP_CALL create(const ui_app_context_t *context,
    void **state, ui_assistant_config_t *assistant)
{
    fixture_state_t *s = calloc(1, sizeof(*s));
    ui_command_desc_t command;
    ui_menu_item_desc_t menu;
    ui_toolbar_desc_t toolbar;
    ui_toolbar_item_desc_t item;
    ui_panel_desc_t panel;
    ui_layout_desc_t layout;
    size_t i;
    wchar_t setting[2];
    if (s == NULL) return UI_STATUS_OUT_OF_MEMORY;
    *state = s; s->context = *context; ++instance_count;
    if (GetEnvironmentVariableW(L"UI_FIXTURE_FAIL_CREATE", setting, 2))
        return UI_STATUS_PLATFORM_ERROR;
    memset(&command, 0, sizeof(command));
    command.size = sizeof(command); command.title = "Lifecycle command";
    command.params_schema_json = "{}"; command.handler = execute; command.user_data = s;
    for (i = 0; i < sizeof(commands) / sizeof(commands[0]); ++i) {
        ui_status_t status;
        command.id = commands[i]; status = ui_host_register_command(context->host, &command);
        if (status != UI_STATUS_OK) return status;
    }
    memset(&menu, 0, sizeof(menu)); menu.size = sizeof(menu);
    menu.id = "fixture.increment.menu"; menu.menu_path = "Fixture";
    menu.title = "Increment"; menu.command_id = commands[0];
    if (ui_host_register_menu_item(context->host, &menu) != UI_STATUS_OK) return UI_STATUS_PLATFORM_ERROR;
    memset(&toolbar, 0, sizeof(toolbar)); toolbar.size = sizeof(toolbar);
    toolbar.id = "fixture.toolbar"; toolbar.title = "Fixture"; toolbar.visible = 1;
    if (ui_host_register_toolbar(context->host, &toolbar) != UI_STATUS_OK) return UI_STATUS_PLATFORM_ERROR;
    memset(&item, 0, sizeof(item)); item.size = sizeof(item);
    item.id = "fixture.increment.tool"; item.toolbar_id = toolbar.id;
    item.title = "Increment"; item.command_id = commands[0];
    if (ui_host_register_toolbar_item(context->host, &item) != UI_STATUS_OK) return UI_STATUS_PLATFORM_ERROR;
    memset(&panel, 0, sizeof(panel)); panel.size = sizeof(panel);
    panel.id = "fixture.panel"; panel.title = "Lifecycle";
    panel.kind = UI_PANEL_SIDEBAR; panel.entry_url = "app://fixture/panel";
    panel.preferred_width = 200;
    if (ui_host_register_panel(context->host, &panel) != UI_STATUS_OK) return UI_STATUS_PLATFORM_ERROR;
    memset(&layout, 0, sizeof(layout)); layout.size = sizeof(layout);
    layout.toolbar_height = 40; layout.right_sidebar_width = 200;
    layout.right_sidebar_min_width = 200; layout.main_min_width = 400;
    layout.collapsed_tag_width = 32;
    if (ui_host_set_layout(context->host, &layout) != UI_STATUS_OK) return UI_STATUS_PLATFORM_ERROR;
    memset(assistant, 0, sizeof(*assistant)); assistant->size = sizeof(*assistant);
    assistant->max_permission = UI_ASSISTANT_PERMISSION_EDIT;
    assistant->user_data = s; assistant->snapshot = state_snapshot; assistant->cancel = cancel;
    return UI_STATUS_OK;
}

static ui_status_t UI_APP_CALL mount(void *state, const ui_app_context_t *context)
{
    fixture_state_t *s = state;
    ui_assistant_command_desc_t command;
    ui_surface_desc_t surface;
    WNDCLASSW wc;
    HWND panel;
    RECT rect;
    void *bytes = NULL;
    size_t size, i;
    wchar_t setting[2];
    s->context = *context;
    memset(&command, 0, sizeof(command)); command.size = sizeof(command);
    command.permission = UI_ASSISTANT_PERMISSION_EDIT;
    command.params_schema_json = "{}"; command.validate = validate;
    for (i = 0; i < sizeof(commands) / sizeof(commands[0]); ++i) {
        ui_status_t status;
        command.id = commands[i];
        command.permission = !strcmp(command.id, "fixture.destructive") ?
            UI_ASSISTANT_PERMISSION_DESTRUCTIVE : UI_ASSISTANT_PERMISSION_EDIT;
        status = ui_assistant_register_command(context->assistant, &command);
        if (status != UI_STATUS_OK) return status;
    }
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)&module_identity, &module))
        return UI_STATUS_PLATFORM_ERROR;
    if (!window_class) {
        memset(&wc, 0, sizeof(wc)); wc.hInstance = module;
        wc.lpfnWndProc = fixture_proc; wc.lpszClassName = class_name;
        window_class = RegisterClassW(&wc);
        if (!window_class) return UI_STATUS_PLATFORM_ERROR;
    }
    panel = ui_native_shell_panel_handle(context->shell, "fixture.panel");
    if (!panel || !GetClientRect(panel, &rect)) return UI_STATUS_PLATFORM_ERROR;
    s->window = CreateWindowExW(0, class_name, L"Lifecycle fixture",
        WS_CHILD | WS_VISIBLE, 0, 0, rect.right, rect.bottom, panel, NULL, module, s);
    if (!s->window) return UI_STATUS_PLATFORM_ERROR;
    s->edit = CreateWindowExW(0, L"EDIT", L"fixture text", WS_CHILD | WS_VISIBLE,
        0, 0, 200, 100, s->window, NULL, module, NULL);
    if (!s->edit) return UI_STATUS_PLATFORM_ERROR;
    if (ui_app_resource_read(context, "fixture.txt", &bytes, &size) != UI_STATUS_OK)
        return UI_STATUS_VALIDATION_FAILED;
    if (size != 16 || memcmp(bytes, "fixture resource", 16) != 0) {
        ui_app_resource_release(bytes); return UI_STATUS_VALIDATION_FAILED;
    }
    ui_app_resource_release(bytes);
    memset(&surface, 0, sizeof(surface)); surface.size = sizeof(surface);
    surface.id = "fixture.native.surface"; surface.kind = UI_SURFACE_NATIVE;
    surface.visible = 1; surface.rect.width = 100; surface.rect.height = 100;
    s->surface = ui_surface_create(context->host, &surface);
    if (!s->surface || ui_surface_set_callbacks(s->surface, NULL, frame, s) != UI_STATUS_OK ||
        ui_surface_set_input_callback(s->surface, input, s) != UI_STATUS_OK ||
        ui_surface_set_layout_region(s->surface, UI_LAYOUT_REGION_MAIN) != UI_STATUS_OK)
        return UI_STATUS_PLATFORM_ERROR;
    if (GetEnvironmentVariableW(L"UI_FIXTURE_FAIL_MOUNT", setting, 2))
        return UI_STATUS_PLATFORM_ERROR;
    return UI_STATUS_OK;
}

static void UI_APP_CALL set_active(void *state, int active)
{ ((fixture_state_t *)state)->active = active != 0; }

static ui_app_close_decision_t UI_APP_CALL request_close(void *state, ui_app_close_reason_t reason)
{ (void)reason; return ((fixture_state_t *)state)->close_mode; }

static void UI_APP_CALL unmount(void *state)
{
    fixture_state_t *s = state;
    uint32_t dpi;
    if (ui_host_get_dpi(s->context.host, &dpi) == UI_STATUS_OK)
        (void)ui_host_emit_event(s->context.host, "fixture.unmount", "{\"host_alive\":true}");
    if (s->window) DestroyWindow(s->window);
    s->window = s->edit = NULL;
    if (s->surface) {
        (void)ui_surface_set_callbacks(s->surface, NULL, NULL, NULL);
        (void)ui_surface_set_input_callback(s->surface, NULL, NULL);
        ui_surface_destroy(s->surface); s->surface = NULL;
    }
}

static void UI_APP_CALL destroy(void *state)
{ --instance_count; free(state); }

static ui_status_t UI_APP_CALL module_shutdown(void)
{
    if (instance_count || shutdown_failed) return UI_STATUS_PLATFORM_ERROR;
    if (window_class && !UnregisterClassW(class_name, module)) return UI_STATUS_PLATFORM_ERROR;
    window_class = 0; module = NULL; return UI_STATUS_OK;
}

UI_APP_EXPORT const ui_app_descriptor_t *UI_APP_CALL ui_app_query_v1(void)
{
    static ui_app_descriptor_t descriptor = {
        sizeof(ui_app_descriptor_t), UI_APPLICATION_ABI_VERSION, UI_FRAMEWORK_API_VERSION,
        create, mount, set_active, request_close, unmount, destroy, module_shutdown
    };
    wchar_t setting[2];
    descriptor.abi_version = GetEnvironmentVariableW(L"UI_FIXTURE_BAD_ABI", setting, 2) ?
        99u : UI_APPLICATION_ABI_VERSION;
    return &descriptor;
}
