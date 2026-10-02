#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <GL/gl.h>

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "ui_framework/application.h"
#include "ui_framework/opengl.h"

typedef struct eda_block {
    float x, y, width, height;
} eda_block_t;

typedef struct eda_state {
    ui_app_context_t context;
    ui_surface_t *canvas;
    HWND panel, properties, summary, note;
    HFONT font;
    int active;
    int pixel_width, pixel_height;
    float zoom;
    size_t block_count;
    eda_block_t blocks[64];
    char snapshot[256];
} eda_state_t;

static HINSTANCE module_instance;
static ATOM properties_class;
static const wchar_t properties_class_name[] = L"UiFramework.MinimalEda.Properties.v1";
static const char empty_schema[] =
    "{\"type\":\"object\",\"properties\":{},\"additionalProperties\":false}";

static void update_properties(eda_state_t *eda)
{
    wchar_t text[160];
    if (eda->summary == NULL) return;
    (void)swprintf(text, sizeof(text) / sizeof(text[0]),
        L"Instance %llu\nBlocks: %u   Zoom: %.2f\nA: add  Z: zoom  C: clear",
        (unsigned long long)eda->context.instance_id,
        (unsigned)eda->block_count, (double)eda->zoom);
    SetWindowTextW(eda->summary, text);
}

static void changed(eda_state_t *eda, uint64_t request_id, const char *result)
{
    update_properties(eda);
    if (eda->active && eda->canvas != NULL)
        (void)ui_surface_invalidate(eda->canvas);
    (void)ui_host_emit_event(eda->context.host, "eda.state.changed", result);
    (void)ui_host_reply(eda->context.host, request_id, 1, result);
}

static void command_add(ui_host_t *host, uint64_t request_id,
    const char *command_id, const char *params_json, const char *source, void *data)
{
    eda_state_t *eda = data;
    eda_block_t *block;
    char result[80];
    (void)command_id; (void)params_json; (void)source;
    if (eda->block_count == sizeof(eda->blocks) / sizeof(eda->blocks[0])) {
        (void)ui_host_reply(host, request_id, 0, "{\"error\":\"block_limit\"}");
        return;
    }
    block = &eda->blocks[eda->block_count];
    block->x = -0.8f + (float)(eda->block_count % 5u) * 0.35f;
    block->y = 0.6f - (float)(eda->block_count / 5u) * 0.25f;
    block->width = 0.22f;
    block->height = 0.12f;
    ++eda->block_count;
    (void)snprintf(result, sizeof(result), "{\"blocks\":%u}",
        (unsigned)eda->block_count);
    changed(eda, request_id, result);
}

static void command_zoom(ui_host_t *host, uint64_t request_id,
    const char *command_id, const char *params_json, const char *source, void *data)
{
    eda_state_t *eda = data;
    char result[80];
    (void)host; (void)command_id; (void)params_json; (void)source;
    eda->zoom *= 1.1f;
    if (eda->zoom > 4.0f) eda->zoom = 4.0f;
    (void)snprintf(result, sizeof(result), "{\"zoom\":%.3f}", (double)eda->zoom);
    changed(eda, request_id, result);
}

static void command_clear(ui_host_t *host, uint64_t request_id,
    const char *command_id, const char *params_json, const char *source, void *data)
{
    eda_state_t *eda = data;
    (void)host; (void)command_id; (void)params_json; (void)source;
    eda->block_count = 0;
    eda->zoom = 1.0f;
    changed(eda, request_id, "{\"blocks\":0,\"zoom\":1.0}");
}

static const char *snapshot(void *data)
{
    eda_state_t *eda = data;
    (void)snprintf(eda->snapshot, sizeof(eda->snapshot),
        "{\"instance_id\":%llu,\"blocks\":%u,\"zoom\":%.3f,\"active\":%s,"
        "\"framebuffer_width\":%d,\"framebuffer_height\":%d}",
        (unsigned long long)eda->context.instance_id,
        (unsigned)eda->block_count, (double)eda->zoom,
        eda->active ? "true" : "false", eda->pixel_width, eda->pixel_height);
    return eda->snapshot;
}

static ui_status_t validate_empty_object(const char *json, void *data)
{
    const unsigned char *cursor = (const unsigned char *)json;
    (void)data;
    if (cursor == NULL) return UI_STATUS_INVALID_ARGUMENT;
    while (isspace(*cursor)) ++cursor;
    if (*cursor++ != '{') return UI_STATUS_INVALID_ARGUMENT;
    while (isspace(*cursor)) ++cursor;
    if (*cursor++ != '}') return UI_STATUS_INVALID_ARGUMENT;
    while (isspace(*cursor)) ++cursor;
    return *cursor == 0 ? UI_STATUS_OK : UI_STATUS_INVALID_ARGUMENT;
}

static void canvas_resize(ui_surface_t *surface, const ui_rect_t *logical,
    const ui_rect_t *pixels, uint32_t dpi, void *data)
{
    eda_state_t *eda = data;
    (void)surface; (void)logical; (void)dpi;
    eda->pixel_width = pixels->width;
    eda->pixel_height = pixels->height;
}

static void canvas_frame(ui_surface_t *surface, void *data)
{
    eda_state_t *eda = data;
    double aspect;
    size_t index;
    int grid;
    if (!eda->active || eda->pixel_width <= 0 || eda->pixel_height <= 0 ||
        ui_surface_make_current(surface) != UI_STATUS_OK) return;
    glViewport(0, 0, eda->pixel_width, eda->pixel_height);
    glClearColor(0.055f, 0.065f, 0.08f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    aspect = (double)eda->pixel_width / eda->pixel_height;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-aspect / eda->zoom, aspect / eda->zoom,
        -1.0 / eda->zoom, 1.0 / eda->zoom, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor3f(0.18f, 0.22f, 0.28f);
    glBegin(GL_LINES);
    for (grid = -10; grid <= 10; ++grid) {
        float position = (float)grid * 0.1f;
        glVertex2f(position, -1.0f); glVertex2f(position, 1.0f);
        glVertex2f(-1.0f, position); glVertex2f(1.0f, position);
    }
    glEnd();
    glColor3f(0.25f, 0.72f, 0.95f);
    glBegin(GL_QUADS);
    for (index = 0; index < eda->block_count; ++index) {
        const eda_block_t *block = &eda->blocks[index];
        glVertex2f(block->x, block->y);
        glVertex2f(block->x + block->width, block->y);
        glVertex2f(block->x + block->width, block->y + block->height);
        glVertex2f(block->x, block->y + block->height);
    }
    glEnd();
    (void)ui_surface_swap_buffers(surface);
}

static void canvas_input(ui_surface_t *surface, const ui_input_event_t *event,
    void *data)
{
    eda_state_t *eda = data;
    const char *id;
    (void)surface;
    if (!eda->active || event->kind != UI_INPUT_KEY_DOWN) return;
    id = event->key_code == 'A' ? "eda.add_block" :
         event->key_code == 'Z' ? "eda.zoom_in" :
         event->key_code == 'C' ? "eda.clear" : NULL;
    if (id != NULL)
        (void)ui_host_invoke(eda->context.host, id, "{}", "keyboard");
}

static void properties_layout(eda_state_t *eda)
{
    RECT rect;
    uint32_t dpi = GetDpiForWindow(eda->properties);
    int margin, summary_height, edit_y;
    HFONT font;
    if (dpi == 0) dpi = 96;
    GetClientRect(eda->properties, &rect);
    margin = MulDiv(8, (int)dpi, 96);
    summary_height = MulDiv(66, (int)dpi, 96);
    edit_y = summary_height + 2 * margin;
    font = CreateFontW(-MulDiv(14, (int)dpi, 96), 0, 0, 0,
        FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH, L"Segoe UI");
    if (font != NULL) {
        SendMessageW(eda->summary, WM_SETFONT, (WPARAM)font, TRUE);
        SendMessageW(eda->note, WM_SETFONT, (WPARAM)font, TRUE);
        if (eda->font != NULL) DeleteObject(eda->font);
        eda->font = font;
    }
    MoveWindow(eda->summary, margin, margin,
        rect.right > 2 * margin ? rect.right - 2 * margin : 0, summary_height, TRUE);
    MoveWindow(eda->note, margin, edit_y,
        rect.right > 2 * margin ? rect.right - 2 * margin : 0,
        rect.bottom > edit_y + margin ? rect.bottom - edit_y - margin : 0, TRUE);
}

static LRESULT CALLBACK properties_proc(HWND hwnd, UINT message,
    WPARAM w_param, LPARAM l_param)
{
    eda_state_t *eda = (eda_state_t *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    LRESULT result;
    if (message == WM_NCCREATE) {
        eda = ((CREATESTRUCTW *)l_param)->lpCreateParams;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)eda);
        eda->properties = hwnd;
    }
    if (eda) ui_app_callback_enter(&eda->context);
    if ((message == WM_SIZE || message == WM_DPICHANGED_AFTERPARENT) &&
        eda != NULL && eda->summary != NULL && eda->note != NULL) {
        properties_layout(eda);
        ui_app_callback_leave(&eda->context);
        return 0;
    }
    if (message == WM_NCDESTROY) SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
    result = DefWindowProcW(hwnd, message, w_param, l_param);
    if (eda) ui_app_callback_leave(&eda->context);
    return result;
}

static LRESULT CALLBACK panel_resize_proc(HWND hwnd, UINT message,
    WPARAM w_param, LPARAM l_param, UINT_PTR subclass_id, DWORD_PTR reference)
{
    eda_state_t *eda = (eda_state_t *)reference;
    LRESULT result;
    ui_app_callback_enter(&eda->context);
    result = DefSubclassProc(hwnd, message, w_param, l_param);
    (void)subclass_id;
    if ((message == WM_SIZE || message == WM_DPICHANGED ||
        message == WM_DPICHANGED_AFTERPARENT) && eda->properties != NULL) {
        RECT rect;
        GetClientRect(hwnd, &rect);
        MoveWindow(eda->properties, 0, 0, rect.right, rect.bottom, TRUE);
        properties_layout(eda);
    }
    ui_app_callback_leave(&eda->context);
    return result;
}

static ui_status_t register_properties_class(void)
{
    WNDCLASSW window_class;
    if (properties_class != 0) return UI_STATUS_OK;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCWSTR)(const void *)&module_instance, &module_instance))
        return UI_STATUS_PLATFORM_ERROR;
    ZeroMemory(&window_class, sizeof(window_class));
    window_class.lpfnWndProc = properties_proc;
    window_class.hInstance = module_instance;
    window_class.hCursor = LoadCursorW(NULL, MAKEINTRESOURCEW(32512));
    window_class.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    window_class.lpszClassName = properties_class_name;
    properties_class = RegisterClassW(&window_class);
    return properties_class != 0 ? UI_STATUS_OK : UI_STATUS_PLATFORM_ERROR;
}

static ui_status_t register_ui(eda_state_t *eda)
{
    static const char *ids[] = { "eda.add_block", "eda.zoom_in", "eda.clear" };
    static const char *titles[] = { "Add block (A)", "Zoom in (Z)", "Clear (C)" };
    static const ui_command_handler_fn handlers[] = {
        command_add, command_zoom, command_clear
    };
    ui_command_desc_t command;
    ui_menu_item_desc_t menu;
    ui_toolbar_desc_t toolbar;
    ui_toolbar_item_desc_t item;
    ui_panel_desc_t panel;
    ui_layout_desc_t layout;
    ui_status_t status;
    size_t i;
    ZeroMemory(&layout, sizeof(layout));
    layout.size = sizeof(layout);
    layout.toolbar_height = 36;
    layout.right_sidebar_preferred_width = 260;
    layout.right_sidebar_min_width = 220;
    layout.right_sidebar_max_width = 320;
    layout.collapsed_tag_width = 32;
    status = ui_host_set_layout(eda->context.host, &layout);
    if (status != UI_STATUS_OK) return status;
    ZeroMemory(&toolbar, sizeof(toolbar));
    toolbar.size = sizeof(toolbar);
    toolbar.id = "eda.main_toolbar";
    toolbar.title = "EDA";
    toolbar.order = 10;
    toolbar.visible = 1;
    status = ui_host_register_toolbar(eda->context.host, &toolbar);
    if (status != UI_STATUS_OK) return status;
    for (i = 0; i < 3; ++i) {
        ZeroMemory(&command, sizeof(command));
        command.size = sizeof(command);
        command.id = ids[i]; command.title = titles[i];
        command.params_schema_json = empty_schema;
        command.handler = handlers[i]; command.user_data = eda;
        status = ui_host_register_command(eda->context.host, &command);
        if (status != UI_STATUS_OK) return status;
        ZeroMemory(&menu, sizeof(menu));
        menu.size = sizeof(menu); menu.id = ids[i]; menu.menu_path = "EDA";
        menu.title = titles[i]; menu.command_id = ids[i]; menu.order = (int)i * 10;
        status = ui_host_register_menu_item(eda->context.host, &menu);
        if (status != UI_STATUS_OK) return status;
        ZeroMemory(&item, sizeof(item));
        item.size = sizeof(item); item.id = ids[i];
        item.toolbar_id = toolbar.id; item.title = titles[i];
        item.command_id = ids[i]; item.order = (int)i * 10;
        status = ui_host_register_toolbar_item(eda->context.host, &item);
        if (status != UI_STATUS_OK) return status;
    }
    ZeroMemory(&panel, sizeof(panel));
    panel.size = sizeof(panel); panel.id = "eda.properties";
    panel.title = "Properties and notes"; panel.kind = UI_PANEL_SIDEBAR;
    panel.entry_url = "";
    panel.preferred_width = 260; panel.dock_region = UI_LAYOUT_REGION_RIGHT_SIDEBAR;
    return ui_host_register_panel(eda->context.host, &panel);
}

static ui_status_t UI_APP_CALL eda_create(const ui_app_context_t *context,
    void **state, ui_assistant_config_t *assistant_config)
{
    eda_state_t *eda = calloc(1, sizeof(*eda));
    if (eda == NULL) return UI_STATUS_OUT_OF_MEMORY;
    *state = eda;
    eda->context = *context;
    eda->zoom = 1.0f;
    ZeroMemory(assistant_config, sizeof(*assistant_config));
    assistant_config->size = sizeof(*assistant_config);
    assistant_config->max_permission = UI_ASSISTANT_PERMISSION_EDIT;
    assistant_config->source_id = "workspace.assistant";
    assistant_config->user_data = eda;
    assistant_config->snapshot = snapshot;
    return register_ui(eda);
}

static ui_status_t UI_APP_CALL eda_mount(void *state, const ui_app_context_t *context)
{
    eda_state_t *eda = state;
    ui_surface_desc_t surface;
    ui_opengl_config_t gl_config;
    ui_assistant_command_desc_t command;
    ui_status_t status;
    HWND panel;
    RECT rect;
    void *resource = NULL;
    size_t resource_size = 0;
    wchar_t *resource_wide;
    int characters;
    size_t i;
    static const char *ids[] = { "eda.add_block", "eda.zoom_in", "eda.clear" };
    eda->context = *context;
    for (i = 0; i < 3; ++i) {
        ZeroMemory(&command, sizeof(command));
        command.size = sizeof(command); command.id = ids[i];
        command.permission = i == 2 ? UI_ASSISTANT_PERMISSION_DESTRUCTIVE :
            UI_ASSISTANT_PERMISSION_EDIT;
        command.params_schema_json = empty_schema;
        command.validate = validate_empty_object;
        status = ui_assistant_register_command(context->assistant, &command);
        if (status != UI_STATUS_OK) return status;
    }
    ZeroMemory(&surface, sizeof(surface));
    surface.size = sizeof(surface); surface.id = "eda.canvas";
    surface.kind = UI_SURFACE_OPENGL; surface.visible = 1;
    ZeroMemory(&gl_config, sizeof(gl_config));
    gl_config.size = sizeof(gl_config);
    gl_config.major_version = 3; gl_config.minor_version = 3;
    gl_config.profile = UI_OPENGL_PROFILE_COMPATIBILITY;
    eda->canvas = ui_opengl_surface_create(context->host, &surface, &gl_config, &status);
    if (eda->canvas == NULL) return status;
    status = ui_surface_set_callbacks(eda->canvas, canvas_resize, canvas_frame, eda);
    if (status != UI_STATUS_OK) return status;
    status = ui_surface_set_input_callback(eda->canvas, canvas_input, eda);
    if (status != UI_STATUS_OK) return status;
    status = ui_surface_set_layout_region(eda->canvas, UI_LAYOUT_REGION_MAIN);
    if (status != UI_STATUS_OK) return status;
    panel = ui_native_shell_panel_handle(context->shell, "eda.properties");
    if (panel == NULL) return UI_STATUS_NOT_FOUND;
    status = register_properties_class();
    if (status != UI_STATUS_OK) return status;
    GetClientRect(panel, &rect);
    eda->properties = CreateWindowExW(0, properties_class_name, L"EDA properties",
        WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, 0, 0, rect.right, rect.bottom,
        panel, NULL, module_instance, eda);
    if (eda->properties == NULL) return UI_STATUS_PLATFORM_ERROR;
    eda->summary = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE,
        0, 0, 0, 0, eda->properties, (HMENU)(uintptr_t)1, module_instance, NULL);
    eda->note = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL |
        ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
        0, 0, 0, 0, eda->properties, (HMENU)(uintptr_t)2, module_instance, NULL);
    if (eda->summary == NULL || eda->note == NULL) return UI_STATUS_PLATFORM_ERROR;
    if (!SetWindowSubclass(panel, panel_resize_proc, 1, (DWORD_PTR)eda))
        return UI_STATUS_PLATFORM_ERROR;
    eda->panel = panel;
    status = ui_app_resource_read(context, "readme.txt", &resource, &resource_size);
    if (status != UI_STATUS_OK) return status;
    if (resource_size > INT_MAX) {
        ui_app_resource_release(resource);
        return UI_STATUS_VALIDATION_FAILED;
    }
    characters = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
        resource, (int)resource_size, NULL, 0);
    if (characters <= 0) {
        ui_app_resource_release(resource);
        return UI_STATUS_VALIDATION_FAILED;
    }
    resource_wide = calloc((size_t)characters + 1, sizeof(*resource_wide));
    if (resource_wide == NULL) {
        ui_app_resource_release(resource);
        return UI_STATUS_OUT_OF_MEMORY;
    }
    (void)MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, resource,
        (int)resource_size, resource_wide, characters);
    SetWindowTextW(eda->note, resource_wide);
    free(resource_wide);
    ui_app_resource_release(resource);
    properties_layout(eda);
    update_properties(eda);
    return UI_STATUS_OK;
}

static void UI_APP_CALL eda_set_active(void *state, int active)
{
    eda_state_t *eda = state;
    eda->active = active != 0;
    if (eda->active && eda->canvas != NULL) (void)ui_surface_invalidate(eda->canvas);
}

static ui_app_close_decision_t UI_APP_CALL eda_request_close(void *state,
    ui_app_close_reason_t reason)
{
    (void)state; (void)reason;
    /* This example has no worker, timer, file persistence, or external callback. */
    return UI_APP_CLOSE_ALLOW;
}

static void UI_APP_CALL eda_unmount(void *state)
{
    eda_state_t *eda = state;
    eda->active = 0;
    if (eda->panel != NULL) RemoveWindowSubclass(eda->panel, panel_resize_proc, 1);
    eda->panel = NULL;
    if (eda->properties != NULL) DestroyWindow(eda->properties);
    eda->properties = eda->summary = eda->note = NULL;
    if (eda->font != NULL) DeleteObject(eda->font);
    eda->font = NULL;
    if (eda->canvas != NULL) {
        (void)ui_surface_set_callbacks(eda->canvas, NULL, NULL, NULL);
        (void)ui_surface_set_input_callback(eda->canvas, NULL, NULL);
        ui_surface_destroy(eda->canvas);
        eda->canvas = NULL;
    }
}

static void UI_APP_CALL eda_destroy(void *state)
{
    free(state);
}

static ui_status_t UI_APP_CALL eda_module_shutdown(void)
{
    if (properties_class != 0 && !UnregisterClassW(properties_class_name, module_instance))
        return UI_STATUS_PLATFORM_ERROR;
    properties_class = 0;
    module_instance = NULL;
    return UI_STATUS_OK;
}

static const ui_app_descriptor_t eda_descriptor = {
    sizeof(ui_app_descriptor_t), UI_APPLICATION_ABI_VERSION, UI_FRAMEWORK_API_VERSION,
    eda_create, eda_mount, eda_set_active, eda_request_close,
    eda_unmount, eda_destroy, eda_module_shutdown
};

#ifdef __cplusplus
extern "C"
#endif
UI_APP_EXPORT const ui_app_descriptor_t *UI_APP_CALL ui_app_query_v1(void)
{
    return &eda_descriptor;
}
