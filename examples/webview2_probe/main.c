#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ui_framework/ui.h"
#include "ui_framework/webview2.h"

#ifndef UI_WEB_PARITY_PAGE_PATH
#define UI_WEB_PARITY_PAGE_PATH "examples/web_common/assistant.html"
#endif

typedef struct probe_state {
    const char *expected;
    int script_result;
    int click_calls;
    int input_calls;
    int command_results;
    int failed;
} probe_state_t;

typedef struct viewport_case {
    int width;
    uint32_t dpi;
    const char *expected;
} viewport_case_t;

static const viewport_case_t viewport_cases[] = {
    {800, 96u, "[800,240,false]"},
    {500, 96u, "[500,0,true]"},
    {800, 144u, "[800,240,false]"},
    {500, 144u, "[500,0,true]"}
};

static LRESULT CALLBACK probe_window_proc(HWND window, UINT message,
                                          WPARAM w_param, LPARAM l_param)
{
    if (message == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(window, message, w_param, l_param);
}

static HWND create_probe_window(void)
{
    WNDCLASSW klass;
    memset(&klass, 0, sizeof(klass));
    klass.lpfnWndProc = probe_window_proc;
    klass.hInstance = GetModuleHandleW(NULL);
    klass.lpszClassName = L"UiFrameworkWebView2Probe";
    if (RegisterClassW(&klass) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        return NULL;
    }
    /* A real parent HWND is required; the integration test stays hidden. */
    return CreateWindowExW(0, klass.lpszClassName, L"WebView2 probe",
                            WS_OVERLAPPEDWINDOW, 0, 0, 1280, 800,
                            NULL, NULL, klass.hInstance, NULL);
}

static char *read_page(void)
{
    FILE *file = fopen(UI_WEB_PARITY_PAGE_PATH, "rb");
    char *html;
    long length;
    if (file == NULL) return NULL;
    if (fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) < 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    html = (char *)malloc((size_t)length + 1u);
    if (html == NULL || fread(html, 1u, (size_t)length, file) != (size_t)length) {
        free(html);
        fclose(file);
        return NULL;
    }
    html[length] = '\0';
    fclose(file);
    return html;
}

static void command_handler(ui_host_t *host, uint64_t request_id,
                             const char *command_id, const char *params_json,
                             const char *source, void *user_data)
{
    probe_state_t *state = (probe_state_t *)user_data;
    int valid = strcmp(source, "webview2") == 0;
    if (strcmp(command_id, "probe.clicked") == 0 && strcmp(params_json, "{}") == 0) {
        ++state->click_calls;
    } else if (strcmp(command_id, "probe.input") == 0 &&
               strcmp(params_json, "{\"value\":\"EDA updated\"}") == 0) {
        ++state->input_calls;
    } else valid = 0;
    if (!valid) state->failed = 1;
    (void)ui_host_reply(host, request_id, valid, "{}");
}

static void result_callback(const ui_result_t *result, void *user_data)
{
    probe_state_t *state = (probe_state_t *)user_data;
    if (!result->success) state->failed = 1;
    ++state->command_results;
}

static void script_completed(ui_status_t status, const char *result_json_utf8,
                              void *user_data)
{
    probe_state_t *state = (probe_state_t *)user_data;
    if (status != UI_STATUS_OK || result_json_utf8 == NULL) {
        state->script_result = -1;
        return;
    }
    state->script_result = strcmp(result_json_utf8, state->expected) == 0 ? 1 : 2;
    if (state->script_result == 1) {
        printf("javascript_verified=%s\n", result_json_utf8);
    }
}

static int same_rect(const ui_rect_t *a, const ui_rect_t *b)
{
    return a->x == b->x && a->y == b->y &&
        a->width == b->width && a->height == b->height;
}

static int verify_composition(ui_host_t *host, ui_web_view_t *view)
{
    ui_rect_t logical = {120, 40, 500, 400};
    ui_rect_t expected_pixels = {180, 60, 750, 600};
    ui_rect_t reported, pixels;
    ui_layout_desc_t layout = {0};
    if (ui_web_view_set_rect(view, &logical, 144u) != UI_STATUS_OK ||
        ui_web_view_get_rect(view, &reported) != UI_STATUS_OK ||
        !same_rect(&reported, &logical) ||
        ui_webview2_view_get_bounds(view, &pixels) != UI_STATUS_OK ||
        !same_rect(&pixels, &expected_pixels)) return 0;
    logical.width = 600; logical.height = 420;
    expected_pixels.width = 900; expected_pixels.height = 630;
    if (ui_web_view_resize(view, 600, 420, 144u) != UI_STATUS_OK ||
        ui_web_view_get_rect(view, &reported) != UI_STATUS_OK ||
        !same_rect(&reported, &logical) ||
        ui_webview2_view_get_bounds(view, &pixels) != UI_STATUS_OK ||
        !same_rect(&pixels, &expected_pixels)) return 0;
    layout.size = sizeof(layout); layout.toolbar_height = 36;
    layout.right_sidebar_width = 320;
    if (ui_host_set_layout(host, &layout) != UI_STATUS_OK ||
        ui_host_set_dpi(host, 144u) != UI_STATUS_OK ||
        ui_host_resize(host, 800, 600) != UI_STATUS_OK ||
        ui_web_view_set_layout_region(view, UI_LAYOUT_REGION_RIGHT_SIDEBAR) != UI_STATUS_OK) return 0;
    logical.x = 480; logical.y = 36; logical.width = 320; logical.height = 564;
    expected_pixels.x = 720; expected_pixels.y = 54;
    expected_pixels.width = 480; expected_pixels.height = 846;
    if (ui_web_view_get_rect(view, &reported) != UI_STATUS_OK ||
        !same_rect(&reported, &logical) ||
        ui_webview2_view_get_bounds(view, &pixels) != UI_STATUS_OK ||
        !same_rect(&pixels, &expected_pixels) ||
        ui_host_resize(host, 1000, 600) != UI_STATUS_OK) return 0;
    logical.x = 680; expected_pixels.x = 1020;
    if (ui_web_view_get_rect(view, &reported) != UI_STATUS_OK ||
        !same_rect(&reported, &logical) ||
        ui_webview2_view_get_bounds(view, &pixels) != UI_STATUS_OK ||
        !same_rect(&pixels, &expected_pixels)) return 0;
    puts("composition_verified=set_rect,resize_preserves_origin,sidebar_binding,host_reflow");
    return 1;
}

int main(void)
{
    HWND window;
    ui_host_config_t host_config;
    ui_webview2_backend_config_t backend_config;
    ui_command_desc_t command;
    ui_host_t *host;
    ui_web_backend_t *backend;
    ui_web_view_t *view;
    ui_status_t status, state_status = UI_STATUS_OK;
    ULONGLONG start;
    MSG message;
    probe_state_t state = {0};
    unsigned int viewport_index = 0;
    int script_pending = 0, viewport_applied = 0, commands_requested = 0;
    int composition_verified = 0;
    int ready = 0, navigation_completed = 0, passed;
    int32_t async_error = 0;
    char *html;
    const char *rect_script =
        "[window.innerWidth,Math.round(document.getElementById('assistant')"
        ".getBoundingClientRect().width),getComputedStyle(document.getElementById"
        "('assistant')).display==='none']";

    (void)ui_framework_initialize();
    status = ui_webview2_runtime_status();
    if (status != UI_STATUS_OK) {
        printf("webview2_runtime_status=%d\n", (int)status);
        /* CTest skips only a genuinely unavailable installed runtime. */
        return status == UI_STATUS_UNSUPPORTED ? 77 : 1;
    }
    html = read_page();
    if (html == NULL) {
        fprintf(stderr, "Unable to read shared page: %s\n", UI_WEB_PARITY_PAGE_PATH);
        return 2;
    }
    window = create_probe_window();
    if (window == NULL) {
        free(html);
        return 3;
    }
    memset(&host_config, 0, sizeof(host_config));
    host_config.size = sizeof(host_config);
    host_config.api_version = UI_FRAMEWORK_API_VERSION;
    host_config.native_parent = window;
    host_config.user_data = &state;
    host_config.result_callback = result_callback;
    host = ui_host_create(&host_config);
    if (host == NULL) {
        free(html);
        DestroyWindow(window);
        return 4;
    }
    memset(&command, 0, sizeof(command));
    command.size = sizeof(command);
    command.id = "probe.clicked";
    command.title = "Add";
    command.params_schema_json = "{\"type\":\"object\"}";
    command.handler = command_handler;
    command.user_data = &state;
    status = ui_host_register_command(host, &command);
    command.id = "probe.input";
    if (status != UI_STATUS_OK || ui_host_register_command(host, &command) != UI_STATUS_OK) {
        free(html);
        ui_host_destroy(host);
        DestroyWindow(window);
        return 5;
    }
    memset(&backend_config, 0, sizeof(backend_config));
    backend_config.size = sizeof(backend_config);
    /* No fallback: the backend must embed into this host's native_parent. */
    backend = ui_webview2_backend_create(&backend_config, &status);
    if (backend == NULL) {
        free(html);
        printf("webview2_status=%d\n", (int)status);
        ui_host_destroy(host);
        DestroyWindow(window);
        return status == UI_STATUS_UNSUPPORTED ? 77 : 6;
    }
    view = ui_web_view_create(host, backend);
    if (view == NULL || ui_web_view_resize(view, 800, 300, 96u) != UI_STATUS_OK ||
        ui_web_view_load_html(view, html) != UI_STATUS_OK) {
        free(html);
        ui_webview2_backend_destroy(backend);
        ui_host_destroy(host);
        DestroyWindow(window);
        return 7;
    }
    free(html);
    start = GetTickCount64();
    /* Bound first-run runtime/profile startup and all asynchronous checks. */
    while (GetTickCount64() - start < 15000u) {
        while (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE) != 0) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
        state_status = ui_webview2_view_get_state(view, &ready, &navigation_completed);
        if (state_status != UI_STATUS_OK || state.failed || state.script_result < 0) break;
        if (ready && navigation_completed) {
            if (viewport_index < sizeof(viewport_cases) / sizeof(viewport_cases[0])) {
                if (!viewport_applied) {
                    const viewport_case_t *test = &viewport_cases[viewport_index];
                    if (ui_web_view_resize(view, test->width, 300, test->dpi) != UI_STATUS_OK) {
                        state.failed = 1;
                        break;
                    }
                    state.expected = test->expected;
                    state.script_result = 0;
                    viewport_applied = 1;
                }
                if (script_pending && state.script_result != 0) {
                    script_pending = 0;
                    if (state.script_result == 1) {
                        ++viewport_index;
                        viewport_applied = 0;
                    }
                    state.script_result = 0;
                }
                if (viewport_applied && !script_pending) {
                    script_pending = 1;
                    if (ui_webview2_view_execute_script(view, rect_script,
                            script_completed, &state) != UI_STATUS_OK) {
                        state.failed = 1;
                    }
                }
            } else if (!composition_verified) {
                composition_verified = verify_composition(host, view);
                if (!composition_verified) state.failed = 1;
            } else if (!commands_requested) {
                commands_requested = 1;
                state.expected = "true";
                state.script_result = 0;
                if (ui_webview2_view_execute_script(view,
                        "document.getElementById('add').click();"
                        "var e=document.getElementById('name');e.value='EDA updated';"
                        "e.dispatchEvent(new Event('input',{bubbles:true}));"
                        "typeof ui.invoke==='function'",
                        script_completed, &state) != UI_STATUS_OK) state.failed = 1;
            } else if (state.script_result == 1 && state.click_calls == 1 &&
                       state.input_calls == 1 && state.command_results == 2) break;
        }
        Sleep(10);
    }
    (void)ui_webview2_view_get_error(view, &async_error);
    passed = state_status == UI_STATUS_OK && ready && navigation_completed && !state.failed &&
        viewport_index == 4u && composition_verified && state.script_result == 1 && state.click_calls == 1 &&
        state.input_calls == 1 && state.command_results == 2;
    printf("state=%d ready=%d html_navigated=%d viewport_cases=%u composition=%d "
           "clicks=%d inputs=%d results=%d hresult=0x%08lx elapsed_ms=%llu\n",
           (int)state_status, ready, navigation_completed, viewport_index, composition_verified,
           state.click_calls, state.input_calls, state.command_results,
           (unsigned long)async_error, GetTickCount64() - start);
    ui_webview2_backend_destroy(backend);
    ui_host_destroy(host);
    DestroyWindow(window);
    return passed ? 0 : 8;
}
