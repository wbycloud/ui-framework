#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ui_framework/ui.h"
#include "ui_framework/native.h"
#include "ui_framework/opengl.h"

typedef struct eda_block {
    float x;
    float y;
    float width;
    float height;
} eda_block_t;

typedef struct eda_state {
    ui_host_t *host;
    ui_surface_t *canvas;
    ui_native_shell_t *shell;
    float zoom;
    int width;
    int height;
    int pixel_width;
    int pixel_height;
    size_t block_count;
    eda_block_t blocks[64];
} eda_state_t;

static eda_state_t *g_eda;

static void result_callback(const ui_result_t *result, void *user_data)
{
    char message[512];

    (void)user_data;
    snprintf(message, sizeof(message),
             "command=%s request=%llu source=%s ok=%d result=%s\n",
             result->command_id,
             (unsigned long long)result->request_id,
             result->source,
             result->success,
             result->result_json);
    OutputDebugStringA(message);
}

static void event_callback(const char *event_id,
                           const char *payload_json,
                           void *user_data)
{
    char message[512];

    (void)user_data;
    snprintf(message, sizeof(message), "event=%s payload=%s\n",
             event_id, payload_json);
    OutputDebugStringA(message);
}

static void reply_json(ui_host_t *host,
                       uint64_t request_id,
                       const char *json)
{
    (void)ui_host_reply(host, request_id, 1, json);
}

static void command_add_block(ui_host_t *host,
                              uint64_t request_id,
                              const char *command_id,
                              const char *params_json,
                              const char *source,
                              void *user_data)
{
    eda_state_t *eda = (eda_state_t *)user_data;
    char result[128];

    (void)command_id;
    (void)params_json;
    (void)source;

    if (eda->block_count >= sizeof(eda->blocks) / sizeof(eda->blocks[0])) {
        (void)ui_host_reply(host, request_id, 0,
                            "{\"error\":\"block_limit\"}");
        return;
    }

    eda->blocks[eda->block_count].x = -0.8f + (float)(eda->block_count % 5u) * 0.35f;
    eda->blocks[eda->block_count].y = 0.6f - (float)(eda->block_count / 5u) * 0.25f;
    eda->blocks[eda->block_count].width = 0.22f;
    eda->blocks[eda->block_count].height = 0.12f;
    eda->block_count += 1u;

    snprintf(result, sizeof(result), "{\"blocks\":%u}",
             (unsigned)eda->block_count);
    (void)ui_host_emit_event(host, "eda.state.changed", result);
    reply_json(host, request_id, result);
}

static void command_zoom_in(ui_host_t *host,
                            uint64_t request_id,
                            const char *command_id,
                            const char *params_json,
                            const char *source,
                            void *user_data)
{
    eda_state_t *eda = (eda_state_t *)user_data;
    char result[128];

    (void)command_id;
    (void)params_json;
    (void)source;

    eda->zoom *= 1.1f;
    if (eda->zoom > 4.0f) {
        eda->zoom = 4.0f;
    }
    snprintf(result, sizeof(result), "{\"zoom\":%.3f}", eda->zoom);
    (void)ui_host_emit_event(host, "eda.state.changed", result);
    reply_json(host, request_id, result);
}

static void command_clear(ui_host_t *host,
                          uint64_t request_id,
                          const char *command_id,
                          const char *params_json,
                          const char *source,
                          void *user_data)
{
    eda_state_t *eda = (eda_state_t *)user_data;

    (void)command_id;
    (void)params_json;
    (void)source;

    eda->block_count = 0u;
    eda->zoom = 1.0f;
    (void)ui_host_emit_event(host, "eda.state.changed",
                             "{\"blocks\":0,\"zoom\":1.0}");
    reply_json(host, request_id, "{\"cleared\":true}");
}

static void draw_rect(float x, float y, float width, float height)
{
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

static void eda_render(eda_state_t *eda)
{
    size_t index;

    if (eda->canvas == NULL || ui_surface_make_current(eda->canvas) != UI_STATUS_OK) {
        return;
    }

    glViewport(0, 0, eda->pixel_width, eda->pixel_height);
    glClearColor(0.055f, 0.065f, 0.08f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    /* Document units stay square when the viewport changes aspect ratio. */
    double aspect = eda->pixel_height > 0 ?
        (double)eda->pixel_width / eda->pixel_height : 1.0;
    glOrtho(-aspect / eda->zoom, aspect / eda->zoom,
            -1.0 / eda->zoom, 1.0 / eda->zoom, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(0.18f, 0.22f, 0.28f);
    glBegin(GL_LINES);
    for (int i = -10; i <= 10; ++i) {
        float position = (float)i * 0.1f;
        glVertex2f(position, -1.0f);
        glVertex2f(position, 1.0f);
        glVertex2f(-1.0f, position);
        glVertex2f(1.0f, position);
    }
    glEnd();

    glColor3f(0.25f, 0.72f, 0.95f);
    for (index = 0u; index < eda->block_count; ++index) {
        const eda_block_t *block = &eda->blocks[index];
        draw_rect(block->x, block->y, block->width, block->height);
    }

    (void)ui_surface_swap_buffers(eda->canvas);
}

static void canvas_resized(ui_surface_t *surface, const ui_rect_t *logical,
                            const ui_rect_t *pixels, uint32_t dpi, void *data)
{
    eda_state_t *eda = data;
    (void)surface; (void)dpi;
    eda->width = logical->width; eda->height = logical->height;
    eda->pixel_width = pixels->width; eda->pixel_height = pixels->height;
}

static void canvas_frame(ui_surface_t *surface, void *data)
{
    (void)surface;
    eda_render((eda_state_t *)data);
}

static void invoke_key(eda_state_t *eda, uint32_t key)
{
    const char *id = key == 'A' ? "eda.add_block" :
                     key == 'Z' ? "eda.zoom_in" :
                     key == 'C' ? "eda.clear" : NULL;
    if (id != NULL) (void)ui_host_invoke(eda->host, id, "{}", "keyboard");
}

static void canvas_input(ui_surface_t *surface, const ui_input_event_t *event,
                          void *data)
{
    if (event->kind == UI_INPUT_KEY_DOWN) {
        invoke_key((eda_state_t *)data, event->key_code);
        (void)ui_surface_invalidate(surface);
    }
}

static void update_canvas_size(eda_state_t *eda)
{
    ui_rect_t rect;

    if (ui_host_get_rect(eda->host, UI_LAYOUT_REGION_MAIN, &rect) ==
        UI_STATUS_OK) {
        eda->width = rect.width;
        eda->height = rect.height;
    }
    if (ui_surface_get_pixel_rect(eda->canvas, &rect) == UI_STATUS_OK) {
        eda->pixel_width = rect.width;
        eda->pixel_height = rect.height;
    } else {
        eda->pixel_width = eda->width;
        eda->pixel_height = eda->height;
    }
}

static void register_eda_contract(eda_state_t *eda)
{
    static const ui_command_desc_t add_block = {
        sizeof(ui_command_desc_t), "eda.add_block", "Add block", "{}",
        command_add_block, NULL
    };
    static const ui_command_desc_t zoom_in = {
        sizeof(ui_command_desc_t), "eda.zoom_in", "Zoom in", "{}",
        command_zoom_in, NULL
    };
    static const ui_command_desc_t clear = {
        sizeof(ui_command_desc_t), "eda.clear", "Clear", "{}",
        command_clear, NULL
    };
    ui_command_desc_t command;
    ui_menu_item_desc_t menu;
    ui_toolbar_desc_t toolbar;
    ui_toolbar_item_desc_t toolbar_item;
    ui_panel_desc_t panel;
    ui_layout_desc_t layout;

    command = add_block;
    command.user_data = eda;
    (void)ui_host_register_command(eda->host, &command);

    ZeroMemory(&layout, sizeof(layout));
    layout.size = sizeof(layout);
    /* HMENU occupies the non-client area; reserve no client menu row. */
    layout.menu_bar_height = 0;
    layout.toolbar_height = 36;
    layout.right_sidebar_width = 320;
    layout.right_sidebar_min_width = 260;
    layout.right_sidebar_preferred_width = 320;
    layout.right_sidebar_max_width = 360;
    layout.collapsed_tag_width = 32;
    (void)ui_host_set_layout(eda->host, &layout);
    (void)ui_host_resize(eda->host, 960, 640);
    command = zoom_in;
    command.user_data = eda;
    (void)ui_host_register_command(eda->host, &command);
    command = clear;
    command.user_data = eda;
    (void)ui_host_register_command(eda->host, &command);

    toolbar.size = sizeof(toolbar);
    toolbar.id = "eda.main_toolbar";
    toolbar.title = "EDA";
    toolbar.order = 10;
    toolbar.visible = 1;
    (void)ui_host_register_toolbar(eda->host, &toolbar);

    toolbar_item.size = sizeof(toolbar_item);
    toolbar_item.id = "eda.toolbar.add_block";
    toolbar_item.toolbar_id = "eda.main_toolbar";
    toolbar_item.title = "Add block";
    toolbar_item.command_id = "eda.add_block";
    toolbar_item.icon_url = NULL;
    toolbar_item.order = 10;
    (void)ui_host_register_toolbar_item(eda->host, &toolbar_item);

    toolbar_item.id = "eda.toolbar.zoom_in";
    toolbar_item.title = "Zoom in";
    toolbar_item.command_id = "eda.zoom_in";
    toolbar_item.order = 20;
    (void)ui_host_register_toolbar_item(eda->host, &toolbar_item);

    menu.size = sizeof(menu);
    menu.id = "eda.menu.add_block";
    menu.menu_path = "EDA";
    menu.title = "Add block";
    menu.command_id = "eda.add_block";
    menu.order = 10;
    (void)ui_host_register_menu_item(eda->host, &menu);

    menu.id = "eda.menu.zoom_in";
    menu.title = "Zoom in";
    menu.command_id = "eda.zoom_in";
    menu.order = 20;
    (void)ui_host_register_menu_item(eda->host, &menu);

    ZeroMemory(&panel, sizeof(panel));
    panel.size = sizeof(panel);
    panel.id = "eda.properties";
    panel.title = "Properties";
    panel.kind = UI_PANEL_SIDEBAR;
    panel.entry_url = "app://eda/properties.html";
    panel.preferred_width = 280;
    (void)ui_host_register_panel(eda->host, &panel);

    panel.id = "assistant";
    panel.title = "Assistant";
    panel.kind = UI_PANEL_SIDEBAR;
    panel.entry_url = "app://assistant/index.html";
    panel.preferred_width = 360;
    (void)ui_host_register_panel(eda->host, &panel);
}

static LRESULT CALLBACK main_window_proc(HWND hwnd,
                                         UINT message,
                                         WPARAM w_param,
                                         LPARAM l_param)
{
    if (message == WM_COMMAND && g_eda != NULL && g_eda->shell != NULL) {
        intptr_t result = 0;
        if (ui_native_shell_handle_message(g_eda->shell, hwnd, message,
                                           w_param, l_param, &result) == UI_STATUS_OK) {
            eda_render(g_eda);
            return (LRESULT)result;
        }
    }
    switch (message) {
    case WM_GETMINMAXINFO:
        if (g_eda != NULL) {
            intptr_t result = 0;
            if (ui_host_handle_message(g_eda->host, hwnd, message, w_param,
                                        l_param, &result) == UI_STATUS_OK)
                return (LRESULT)result;
        }
        return DefWindowProcA(hwnd, message, w_param, l_param);
    case WM_SIZE:
        if (g_eda != NULL) {
            intptr_t native_result = 0;
            (void)ui_host_handle_message(g_eda->host, (void *)hwnd,
                                         message, (uintptr_t)w_param,
                                         (intptr_t)l_param, &native_result);
            if (g_eda->shell != NULL) {
                (void)ui_native_shell_reflow(g_eda->shell);
            }
            update_canvas_size(g_eda);
            eda_render(g_eda);
        }
        return 0;
    case WM_DPICHANGED:
        if (g_eda != NULL) {
            intptr_t native_result = 0;
            RECT *suggested = (RECT *)l_param;

            (void)ui_host_handle_message(g_eda->host, (void *)hwnd,
                                         message, (uintptr_t)w_param,
                                         (intptr_t)l_param, &native_result);
            if (suggested != NULL) {
                SetWindowPos(hwnd, NULL,
                             suggested->left,
                             suggested->top,
                             suggested->right - suggested->left,
                             suggested->bottom - suggested->top,
                             SWP_NOZORDER | SWP_NOACTIVATE);
            }
            if (g_eda->shell != NULL) {
                (void)ui_native_shell_reflow(g_eda->shell);
            }
            update_canvas_size(g_eda);
            eda_render(g_eda);
        }
        return 0;
    case WM_KEYDOWN:
        if (g_eda != NULL) {
            invoke_key(g_eda, (uint32_t)w_param);
            eda_render(g_eda);
        }
        return 0;
    case WM_TIMER:
        if (g_eda != NULL) {
            (void)ui_surface_invalidate(g_eda->canvas);
        }
        return 0;
    case WM_CLOSE:
        if (g_eda != NULL && g_eda->host != NULL) {
            KillTimer(hwnd, 1u);
            ui_native_shell_destroy(g_eda->shell);
            g_eda->shell = NULL;
            ui_host_destroy(g_eda->host);
            g_eda->host = NULL;
            g_eda->canvas = NULL;
        }
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        KillTimer(hwnd, 1u);
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcA(hwnd, message, w_param, l_param);
    }
}

int WINAPI WinMain(HINSTANCE instance,
                   HINSTANCE previous_instance,
                   LPSTR command_line,
                   int show_command)
{
    WNDCLASSA window_class;
    HWND window;
    MSG message;
    ui_host_config_t config;
    ui_surface_desc_t surface_desc;
    ui_native_shell_config_t shell_config;
    ui_opengl_config_t gl_config;
    ui_status_t gl_status;
    eda_state_t eda;

    (void)previous_instance;

    if (ui_framework_initialize() != UI_STATUS_OK) {
        return 1;
    }

    ZeroMemory(&window_class, sizeof(window_class));
    window_class.lpfnWndProc = main_window_proc;
    window_class.hInstance = instance;
    window_class.hCursor = LoadCursorA(NULL, IDC_ARROW);
    window_class.lpszClassName = "MinimalEdaWindow";
    if (RegisterClassA(&window_class) == 0) {
        return 1;
    }

    window = CreateWindowExA(
        0,
        window_class.lpszClassName,
        "Minimal EDA - UI Framework validation",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        960,
        640,
        NULL,
        NULL,
        instance,
        NULL);
    if (window == NULL) {
        return 1;
    }

    ZeroMemory(&config, sizeof(config));
    config.size = sizeof(config);
    config.api_version = UI_FRAMEWORK_API_VERSION;
    config.native_parent = (void *)window;
    config.result_callback = result_callback;
    config.event_callback = event_callback;

    ZeroMemory(&eda, sizeof(eda));
    eda.zoom = 1.0f;
    eda.width = 960;
    eda.height = 640;
    eda.pixel_width = 960;
    eda.pixel_height = 640;
    eda.host = ui_host_create(&config);
    if (eda.host == NULL) {
        DestroyWindow(window);
        return 1;
    }
    g_eda = &eda;
    register_eda_contract(&eda);

    ZeroMemory(&shell_config, sizeof(shell_config));
    shell_config.size = sizeof(shell_config);
    shell_config.host = eda.host;
    shell_config.native_parent = window;
    eda.shell = ui_native_shell_create(&shell_config);
    if (eda.shell == NULL) {
        ui_host_destroy(eda.host);
        eda.host = NULL;
        g_eda = NULL;
        DestroyWindow(window);
        return 1;
    }

    ZeroMemory(&surface_desc, sizeof(surface_desc));
    surface_desc.size = sizeof(surface_desc);
    surface_desc.id = "eda.canvas";
    surface_desc.kind = UI_SURFACE_OPENGL;
    surface_desc.rect.x = 0;
    surface_desc.rect.y = 0;
    surface_desc.rect.width = eda.width;
    surface_desc.rect.height = eda.height;
    surface_desc.visible = 1;
    ZeroMemory(&gl_config, sizeof(gl_config));
    gl_config.size = sizeof(gl_config);
    if (strstr(command_line, "--legacy") != NULL) {
        gl_config.legacy_context = 1;
    } else {
        gl_config.major_version = 3;
        gl_config.minor_version = 3;
        gl_config.profile = UI_OPENGL_PROFILE_COMPATIBILITY;
    }
    eda.canvas = ui_opengl_surface_create(eda.host, &surface_desc, &gl_config,
                                          &gl_status);
    if (eda.canvas == NULL) {
        MessageBoxA(window, "OpenGL configuration unavailable. Try --legacy.",
                     "Minimal EDA", MB_OK | MB_ICONERROR);
        ui_native_shell_destroy(eda.shell);
        ui_host_destroy(eda.host);
        eda.shell = NULL;
        eda.host = NULL;
        g_eda = NULL;
        DestroyWindow(window);
        return 1;
    }
    (void)ui_surface_set_callbacks(eda.canvas, canvas_resized, canvas_frame, &eda);
    (void)ui_surface_set_input_callback(eda.canvas, canvas_input, &eda);
    (void)ui_surface_set_layout_region(eda.canvas, UI_LAYOUT_REGION_MAIN);
    update_canvas_size(&eda);

    ShowWindow(window, show_command);
    UpdateWindow(window);
    SetTimer(window, 1u, 16u, NULL);
    eda_render(&eda);

    while (GetMessageA(&message, NULL, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    if (eda.host != NULL) {
        ui_native_shell_destroy(eda.shell);
        ui_host_destroy(eda.host);
    }
    g_eda = NULL;
    return (int)message.wParam;
}
