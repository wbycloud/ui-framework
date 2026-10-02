#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include "ui_framework/ui.h"
#include "ui_framework/shell.h"
#include "ui_framework/light_web.h"

static int failures;
static void check(int condition, const char *text)
{
    if (!condition) { fprintf(stderr, "Floating Web DPI: %s\n", text); ++failures; }
}

int main(void)
{
    HWND root, popup, container, window;
    RECT popup_rect, actual;
    ui_host_config_t hc = {0};
    ui_layout_desc_t layout = {0};
    ui_panel_desc_t panel = {0};
    ui_native_shell_config_t shell_config = {0};
    ui_light_web_config_t config = {0};
    ui_host_t *host;
    ui_native_shell_t *shell;
    ui_content_slot_t *slot;
    ui_web_backend_t *backend;
    ui_web_view_t *view;
    ui_rect_t expected;

    check(ui_framework_initialize() == UI_STATUS_OK, "initialize");
    root = CreateWindowW(L"STATIC", L"DPI test", WS_OVERLAPPEDWINDOW,
        0, 0, 800, 600, NULL, NULL, GetModuleHandleW(NULL), NULL);
    if (root == NULL) return 1;
    hc.size = sizeof(hc); hc.api_version = 2; hc.native_parent = root;
    host = ui_host_create(&hc);
    if (host == NULL) return 1;
    layout.size = sizeof(layout);
    check(ui_host_set_layout(host, &layout) == UI_STATUS_OK &&
        ui_host_resize(host, 800, 600) == UI_STATUS_OK, "host layout");
    panel.size = sizeof(panel); panel.id = "floating"; panel.title = "Floating";
    panel.kind = UI_PANEL_FLOATING; panel.entry_url = ""; panel.preferred_width = 320;
    check(ui_host_register_panel(host, &panel) == UI_STATUS_OK, "register panel");
    shell_config.size = sizeof(shell_config); shell_config.host = host;
    shell_config.native_parent = root;
    shell = ui_native_shell_create_web(&shell_config);
    if (shell == NULL) return 1;
    slot = ui_shell_get_content_slot(ui_host_get_shell(host), "floating");
    container = (HWND)ui_content_slot_native_handle(slot); popup = GetParent(container);
    config.size = sizeof(config); config.parent_hwnd = container; config.enable_native_input = 1;
    backend = ui_light_web_backend_create(&config); view = ui_web_view_create(host, backend);
    if (view == NULL) return 1;
    check(ui_web_view_load_html(view, "<button>Floating Web</button>") == UI_STATUS_OK &&
        ui_content_slot_attach_web_view(slot, view) == UI_STATUS_OK, "attach view");
    window = (HWND)ui_web_view_native_handle(view);
    GetWindowRect(popup, &popup_rect);
    SendMessageW(popup, WM_DPICHANGED, MAKEWPARAM(144, 144), (LPARAM)&popup_rect);
    check(ui_content_slot_get_pixel_rect(slot, &expected) == UI_STATUS_OK, "floating pixel rect");
    GetClientRect(window, &actual);
    check(actual.right == expected.width && actual.bottom == expected.height,
        "floating view follows its window DPI");
    check(ui_host_set_dpi(host, 192) == UI_STATUS_OK, "move main host to another DPI");
    check(ui_content_slot_get_pixel_rect(slot, &expected) == UI_STATUS_OK, "retained pixel rect");
    GetClientRect(window, &actual);
    check(actual.right == expected.width && actual.bottom == expected.height,
        "main host DPI does not override floating view DPI");
    check(ui_host_resize(host, 640, 480) == UI_STATUS_OK, "resize main host");
    GetClientRect(window, &actual);
    check(actual.right == expected.width && actual.bottom == expected.height,
        "main layout does not override floating view geometry");
    ui_web_view_destroy(view); ui_light_web_backend_destroy(backend);
    ui_native_shell_destroy(shell); ui_host_destroy(host); DestroyWindow(root);
    return failures ? 1 : 0;
}
