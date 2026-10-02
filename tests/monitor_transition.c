#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ui_framework/ui.h"

typedef struct monitor_entry {
    HMONITOR handle;
    RECT work;
    UINT observed_dpi;
} monitor_entry_t;

typedef struct monitor_list {
    monitor_entry_t *entries;
    size_t count;
    int allocation_failed;
} monitor_list_t;

typedef struct test_state {
    ui_host_t *host;
    int dpi_notifications;
    int size_notifications;
    int message_failure;
} test_state_t;

static BOOL CALLBACK collect_monitor(HMONITOR monitor, HDC dc, LPRECT rect,
                                       LPARAM parameter)
{
    monitor_list_t *list = (monitor_list_t *)parameter;
    MONITORINFO info;
    monitor_entry_t *entries;
    (void)dc; (void)rect;
    memset(&info, 0, sizeof(info)); info.cbSize = sizeof(info);
    if (!GetMonitorInfoW(monitor, &info)) return FALSE;
    entries = (monitor_entry_t *)realloc(list->entries,
        (list->count + 1) * sizeof(*entries));
    if (!entries) { list->allocation_failed = 1; return FALSE; }
    list->entries = entries;
    entries[list->count].handle = monitor;
    entries[list->count].work = info.rcWork;
    entries[list->count].observed_dpi = 0;
    ++list->count;
    return TRUE;
}

static LRESULT CALLBACK test_window_proc(HWND hwnd, UINT message,
                                          WPARAM wp, LPARAM lp)
{
    test_state_t *state = (test_state_t *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    if (message == WM_NCCREATE) {
        const CREATESTRUCTW *create = (const CREATESTRUCTW *)lp;
        state = (test_state_t *)create->lpCreateParams;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)state);
    }
    if (state && state->host && (message == WM_SIZE || message == WM_DPICHANGED)) {
        intptr_t result = 0;
        if (ui_host_handle_message(state->host, hwnd, message,
            (uintptr_t)wp, (intptr_t)lp, &result) != UI_STATUS_OK) state->message_failure = 1;
        if (message == WM_SIZE) ++state->size_notifications;
        else {
            const RECT *suggested = (const RECT *)lp;
            ++state->dpi_notifications;
            if (!suggested || !SetWindowPos(hwnd, NULL, suggested->left,
                suggested->top, suggested->right - suggested->left,
                suggested->bottom - suggested->top,
                SWP_NOACTIVATE | SWP_NOZORDER)) state->message_failure = 1;
        }
        return (LRESULT)result;
    }
    return DefWindowProcW(hwnd, message, wp, lp);
}

static void pump_window_messages(void)
{
    ULONGLONG deadline = GetTickCount64() + 200;
    MSG message;
    do {
        while (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message); DispatchMessageW(&message);
        }
        if (GetTickCount64() < deadline)
            MsgWaitForMultipleObjects(0, NULL, FALSE, 20, QS_ALLINPUT);
    } while (GetTickCount64() < deadline);
}

static int logical_size(int pixels, UINT dpi)
{
    return (int)(((int64_t)pixels * 96 + dpi / 2) / dpi);
}

static int verify_current_layout(test_state_t *state, HWND window)
{
    ui_rect_t root, main;
    RECT client;
    uint32_t host_dpi = 0;
    UINT actual_dpi = GetDpiForWindow(window);
    if (!actual_dpi || !GetClientRect(window, &client) ||
        ui_host_get_dpi(state->host, &host_dpi) != UI_STATUS_OK || host_dpi != actual_dpi ||
        ui_host_get_rect(state->host, UI_LAYOUT_REGION_ROOT, &root) != UI_STATUS_OK ||
        ui_host_get_rect(state->host, UI_LAYOUT_REGION_MAIN, &main) != UI_STATUS_OK ||
        root.width != logical_size(client.right, actual_dpi) ||
        root.height != logical_size(client.bottom, actual_dpi) ||
        main.x < 0 || main.y < 0 || main.width < 0 || main.height < 0 ||
        (int64_t)main.x + main.width > root.width ||
        (int64_t)main.y + main.height > root.height) {
        fprintf(stderr, "FAIL: host DPI/client/layout differs from actual window\n");
        return 0;
    }
    return !state->message_failure;
}

int main(void)
{
    monitor_list_t monitors = {0};
    test_state_t state = {0};
    ui_host_config_t config = {0};
    ui_layout_desc_t layout = {0};
    WNDCLASSW window_class = {0};
    HWND window = NULL;
    RECT client, outer;
    UINT previous_dpi;
    size_t index;
    int failures = 0, differing_dpi = 0, exit_code;
    if (ui_framework_initialize() != UI_STATUS_OK) return 1;
    if (!EnumDisplayMonitors(NULL, NULL, collect_monitor, (LPARAM)&monitors) ||
        monitors.allocation_failed || !monitors.count) {
        fprintf(stderr, "FAIL: cannot enumerate monitor work areas\n");
        free(monitors.entries); return 1;
    }
    printf("Monitor transition environment: %zu monitor(s)\n", monitors.count);
    if (monitors.count == 1) {
        printf("SKIP: one monitor; a real cross-monitor DPI transition is unavailable\n");
        free(monitors.entries); return 77;
    }
    window_class.lpfnWndProc = test_window_proc;
    window_class.hInstance = GetModuleHandleW(NULL);
    window_class.lpszClassName = L"UiMonitorTransitionTestWindow";
    if (!RegisterClassW(&window_class)) { free(monitors.entries); return 1; }
    window = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        window_class.lpszClassName, L"UI framework monitor transition test",
        WS_OVERLAPPEDWINDOW, monitors.entries[0].work.left + 20,
        monitors.entries[0].work.top + 20, 400, 300, NULL, NULL,
        window_class.hInstance, &state);
    if (!window) { free(monitors.entries); return 1; }
    if (!AreDpiAwarenessContextsEqual(GetWindowDpiAwarenessContext(window),
                                      DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
        fprintf(stderr, "FAIL: temporary window is not Per-Monitor DPI aware V2\n");
        failures = 1; goto cleanup;
    }
    config.size = sizeof(config); config.api_version = UI_FRAMEWORK_API_VERSION;
    config.native_parent = window;
    state.host = ui_host_create(&config);
    if (!state.host) { failures = 1; goto cleanup; }
    layout.size = sizeof(layout); layout.toolbar_height = 32;
    layout.status_bar_height = 24; layout.right_sidebar_width = 160;
    if (ui_host_set_layout(state.host, &layout) != UI_STATUS_OK ||
        !GetClientRect(window, &client)) { failures = 1; goto cleanup; }
    previous_dpi = GetDpiForWindow(window);
    if (!previous_dpi || ui_host_resize(state.host,
        logical_size(client.right, previous_dpi),
        logical_size(client.bottom, previous_dpi)) != UI_STATUS_OK) {
        failures = 1; goto cleanup;
    }
    ShowWindow(window, SW_SHOWNOACTIVATE);
    pump_window_messages();
    state.dpi_notifications = state.size_notifications = 0;
    previous_dpi = GetDpiForWindow(window);
    for (index = 0; index < monitors.count; ++index) {
        RECT work = monitors.entries[index].work;
        int before = state.dpi_notifications;
        UINT current_dpi;
        if (!GetWindowRect(window, &outer) || !SetWindowPos(window, NULL,
            work.left + ((work.right - work.left) - (outer.right - outer.left)) / 2,
            work.top + ((work.bottom - work.top) - (outer.bottom - outer.top)) / 2,
            0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER)) {
            fprintf(stderr, "FAIL: move temporary window to monitor %zu\n", index + 1);
            ++failures; break;
        }
        pump_window_messages();
        current_dpi = GetDpiForWindow(window);
        monitors.entries[index].observed_dpi = current_dpi;
        printf("Monitor %zu: DPI=%u, OS WM_DPICHANGED=%d, WM_SIZE=%d\n", index + 1,
            current_dpi, state.dpi_notifications, state.size_notifications);
        if (MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST) != monitors.entries[index].handle) {
            fprintf(stderr, "FAIL: window did not reach requested monitor\n"); ++failures;
        }
        if (!verify_current_layout(&state, window)) ++failures;
        if (current_dpi != previous_dpi) {
            differing_dpi = 1;
            if (state.dpi_notifications == before) {
                fprintf(stderr, "FAIL: DPI changed without an actual OS WM_DPICHANGED\n");
                ++failures;
            }
        }
        previous_dpi = current_dpi;
    }

cleanup:
    if (state.host) ui_host_destroy(state.host);
    state.host = NULL;
    DestroyWindow(window);
    UnregisterClassW(window_class.lpszClassName, window_class.hInstance);
    free(monitors.entries);
    exit_code = failures ? 1 : differing_dpi ? 0 : 77;
    if (exit_code == 77)
        printf("SKIP: all observed monitors have the same DPI; movement/layout verified, cross-DPI notification untested\n");
    else printf("Monitor transition integration: %d failure(s), real DPI notifications=%d\n",
                failures, state.dpi_notifications);
    return exit_code;
}
