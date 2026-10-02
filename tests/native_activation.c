#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <GL/gl.h>

#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include "ui_framework/native.h"

static int failures;

static void check(int condition, const char *description)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", description);
        ++failures;
    }
}

static int shown(HWND hwnd)
{
    return hwnd != NULL && (GetWindowLongPtrW(hwnd, GWL_STYLE) & WS_VISIBLE) != 0;
}

static void execute(ui_host_t *host, uint64_t request_id,
    const char *command_id, const char *params_json,
    const char *source, void *user_data)
{
    (void)command_id;
    (void)params_json;
    (void)source;
    ++*(int *)user_data;
    (void)ui_host_reply(host, request_id, 1, "{}");
}

static ui_host_t *create_host(HWND parent, int *counter)
{
    ui_host_config_t config;
    ui_command_desc_t command;
    ui_menu_item_desc_t menu;
    ui_toolbar_desc_t toolbar;
    ui_toolbar_item_desc_t item;
    ui_panel_desc_t panel;
    ui_layout_desc_t layout;
    ui_host_t *host;
    memset(&config, 0, sizeof(config));
    config.size = sizeof(config);
    config.api_version = UI_FRAMEWORK_API_VERSION;
    config.native_parent = parent;
    host = ui_host_create(&config);
    if (host == NULL) return NULL;
    memset(&command, 0, sizeof(command));
    command.size = sizeof(command);
    command.id = "same.command";
    command.title = "Execute";
    command.handler = execute;
    command.user_data = counter;
    check(ui_host_register_command(host, &command) == UI_STATUS_OK, "register per-instance command");
    memset(&menu, 0, sizeof(menu));
    menu.size = sizeof(menu);
    menu.id = "same.menu";
    menu.menu_path = "Application";
    menu.title = "Execute";
    menu.command_id = command.id;
    check(ui_host_register_menu_item(host, &menu) == UI_STATUS_OK, "register per-instance menu");
    memset(&toolbar, 0, sizeof(toolbar));
    toolbar.size = sizeof(toolbar);
    toolbar.id = "same.toolbar";
    toolbar.title = "Tools";
    toolbar.visible = 1;
    check(ui_host_register_toolbar(host, &toolbar) == UI_STATUS_OK, "register toolbar");
    memset(&item, 0, sizeof(item));
    item.size = sizeof(item);
    item.id = "same.tool";
    item.toolbar_id = toolbar.id;
    item.title = "Execute";
    item.command_id = command.id;
    check(ui_host_register_toolbar_item(host, &item) == UI_STATUS_OK, "register toolbar item");
    memset(&panel, 0, sizeof(panel));
    panel.size = sizeof(panel);
    panel.id = "sidebar";
    panel.title = "Inspector";
    panel.kind = UI_PANEL_SIDEBAR;
    panel.entry_url = "app://activation/panel";
    panel.preferred_width = 200;
    panel.dock_region = UI_LAYOUT_REGION_RIGHT_SIDEBAR;
    check(ui_host_register_panel(host, &panel) == UI_STATUS_OK, "register sidebar");
    panel.id = "floating";
    panel.title = "Floating";
    panel.kind = UI_PANEL_FLOATING;
    check(ui_host_register_panel(host, &panel) == UI_STATUS_OK, "register floating panel");
    memset(&layout, 0, sizeof(layout));
    layout.size = sizeof(layout);
    layout.toolbar_height = 40;
    layout.right_sidebar_width = 200;
    layout.right_sidebar_min_width = 200;
    layout.main_min_width = 400;
    layout.collapsed_tag_width = 32;
    check(ui_host_set_layout(host, &layout) == UI_STATUS_OK &&
        ui_host_set_dpi(host, 96) == UI_STATUS_OK &&
        ui_host_resize(host, 1000, 700) == UI_STATUS_OK, "initialize app layout");
    return host;
}

int main(void)
{
    static const int sizes[][2] = {{1920, 1080}, {1280, 720}, {800, 600}, {640, 480}};
    static const uint32_t dpis[] = {96, 144, 192};
    WNDCLASSW window_class;
    HWND root = NULL;
    HWND parents[2] = {NULL, NULL};
    HWND sidebars[2];
    HWND floating[2];
    HWND toolbars[2];
    HWND contents[2];
    HWND gl_windows[2];
    HMENU menus[2];
    HMENU fallback = NULL;
    HGLRC contexts[2];
    ui_host_t *hosts[2] = {NULL, NULL};
    ui_native_shell_t *shells[2] = {NULL, NULL};
    ui_surface_t *surfaces[2];
    ui_surface_desc_t surface_desc;
    ui_native_shell_config_t config;
    int counters[2] = {0, 0};
    UINT native_commands[2];
    intptr_t result;
    size_t index, size_index, dpi_index;
    wchar_t text[32];

    check(ui_framework_initialize() == UI_STATUS_OK, "initialize framework");
    memset(&window_class, 0, sizeof(window_class));
    window_class.lpfnWndProc = DefWindowProcW;
    window_class.hInstance = GetModuleHandleW(NULL);
    window_class.lpszClassName = L"UiManagedShellActivationTest";
    check(RegisterClassW(&window_class) != 0, "register root class");
    root = CreateWindowExW(0, window_class.lpszClassName, L"Tab host",
        WS_OVERLAPPEDWINDOW, 0, 0, 1200, 900, NULL, NULL, window_class.hInstance, NULL);
    if (root == NULL) { check(0, "create root window"); goto cleanup; }
    fallback = CreateMenu();
    check(fallback != NULL && SetMenu(root, fallback), "install host fallback menu");
    for (index = 0; index < 2; ++index) {
        parents[index] = CreateWindowExW(0, L"STATIC", L"App container",
            WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, 0, 30, 1000, 700, root,
            NULL, window_class.hInstance, NULL);
        hosts[index] = create_host(parents[index], &counters[index]);
        if (hosts[index] == NULL) { check(0, "create instance host"); goto cleanup; }
        memset(&config, 0, sizeof(config));
        config.size = sizeof(config);
        config.host = hosts[index];
        config.native_parent = parents[index];
        config.menu_owner = root;
        config.flags = UI_NATIVE_SHELL_MANAGED_ACTIVATION;
        shells[index] = ui_native_shell_create(&config);
        if (shells[index] == NULL) { check(0, "create managed shell"); goto cleanup; }
        menus[index] = (HMENU)ui_native_shell_menu_handle(shells[index]);
        check(menus[index] != NULL && GetMenu(root) == fallback,
            "inactive creation leaves shared root menu intact");
        check(!shown(parents[index]), "managed content parent starts hidden");
        native_commands[index] = GetMenuItemID(GetSubMenu(menus[index], 0), 0);
        check(native_commands[index] >= 0x5000u && native_commands[index] < 0xF000u,
            "application native IDs use reserved range");
        check(AppendMenuW(menus[index], MF_STRING, 0x1000u, L"Host command"), "decorate borrowed menu");
        sidebars[index] = (HWND)ui_native_shell_panel_handle(shells[index], "sidebar");
        floating[index] = (HWND)ui_native_shell_panel_handle(shells[index], "floating");
        toolbars[index] = FindWindowExW(parents[index], NULL, TOOLBARCLASSNAMEW, NULL);
        contents[index] = CreateWindowExW(0, L"EDIT", index == 0 ? L"First state" : L"Second state",
            WS_CHILD | WS_VISIBLE, 0, 20, 150, 30, sidebars[index], NULL,
            window_class.hInstance, NULL);
        check(sidebars[index] && floating[index] && toolbars[index] && contents[index],
            "materialize stable panel and content windows");
        check(!shown(floating[index]) && !shown(toolbars[index]), "inactive floating panels and toolbar hide");
        check(GetWindow(floating[index], GW_OWNER) == root, "floating tool window belongs to root");
        memset(&surface_desc, 0, sizeof(surface_desc));
        surface_desc.size = sizeof(surface_desc);
        surface_desc.id = "same.canvas";
        surface_desc.kind = UI_SURFACE_OPENGL;
        surface_desc.visible = 1;
        surface_desc.rect.width = 100;
        surface_desc.rect.height = 100;
        surfaces[index] = ui_surface_create(hosts[index], &surface_desc);
        if (surfaces[index] == NULL) { check(0, "create actual OpenGL surface"); goto cleanup; }
        check(ui_surface_set_layout_region(surfaces[index], UI_LAYOUT_REGION_MAIN) == UI_STATUS_OK,
            "bind actual canvas to main region");
        gl_windows[index] = (HWND)ui_surface_native_handle(surfaces[index]);
        check(ui_surface_make_current(surfaces[index]) == UI_STATUS_OK, "make OpenGL context current");
        contexts[index] = wglGetCurrentContext();
        check(contexts[index] != NULL, "capture actual GL context");
    }
    check(contexts[0] != contexts[1], "each tab owns independent GL context");
    check(ui_native_shell_handle_message(shells[0], root, WM_COMMAND,
        native_commands[0], 0, &result) == UI_STATUS_NOT_FOUND && counters[0] == 0,
        "inactive managed shell refuses root commands");
    check(ui_native_shell_set_active(NULL, 1) == UI_STATUS_INVALID_ARGUMENT &&
        ui_native_shell_menu_handle(NULL) == NULL, "null shell API errors");
    check(ui_native_shell_set_active(shells[0], 1) == UI_STATUS_OK &&
        GetMenu(root) == menus[0] && shown(parents[0]) && shown(floating[0]), "activate first tab");
    check(ui_native_shell_set_panel_floating(shells[0], "sidebar", 1) == UI_STATUS_OK &&
        shown(sidebars[0]), "manually float sidebar");
    (void)SendMessageW(floating[0], WM_CLOSE, 0, 0);
    check(!shown(floating[0]), "user closes floating window without destruction");
    check(ui_native_shell_handle_message(shells[0], root, WM_COMMAND,
        native_commands[0], 0, &result) == UI_STATUS_OK && counters[0] == 1 && counters[1] == 0,
        "root menu targets first instance only");
    check(ui_native_shell_handle_message(shells[0], root, WM_COMMAND, 0x1000u, 0,
        &result) == UI_STATUS_NOT_FOUND, "host command IDs remain unclaimed");
    check(ui_native_shell_handle_message(shells[0], root, WM_SIZE, 0,
        MAKELPARAM(99, 99), &result) == UI_STATUS_NOT_FOUND, "root resize does not resize app container");
    check(ui_native_shell_set_active(shells[0], 0) == UI_STATUS_OK, "hide initial active tab before transition matrix");

    for (dpi_index = 0; dpi_index < sizeof(dpis) / sizeof(dpis[0]); ++dpi_index) {
        for (size_index = 0; size_index < sizeof(sizes) / sizeof(sizes[0]); ++size_index) {
            for (index = 0; index < 2; ++index) {
                ui_rect_t pixels;
                RECT drawable;
                HWND tag;
                check(ui_native_shell_set_active(shells[1 - index], 0) == UI_STATUS_OK,
                    "deactivate prior tab");
                check(ui_host_set_dpi(hosts[index], dpis[dpi_index]) == UI_STATUS_OK &&
                    ui_host_resize(hosts[index], sizes[size_index][0], sizes[size_index][1]) == UI_STATUS_OK &&
                    ui_native_shell_reflow(shells[index]) == UI_STATUS_OK, "reflow inactive tab without exposing windows");
                tag = FindWindowExW(parents[index], NULL, L"BUTTON", L"Inspector");
                check(!shown(parents[index]) && !shown(floating[index]) && !shown(toolbars[index]) && !shown(tag),
                    "inactive reflow keeps parent, toolbar, tag and float hidden");
                check(ui_native_shell_set_active(shells[index], 1) == UI_STATUS_OK &&
                    GetMenu(root) == menus[index] && shown(parents[index]), "activate next tab without rebuild");
                check(ui_native_shell_panel_handle(shells[index], "sidebar") == sidebars[index] &&
                    IsWindow(contents[index]) && GetParent(contents[index]) == sidebars[index] &&
                    ui_surface_native_handle(surfaces[index]) == gl_windows[index], "tab switching preserves all HWNDs");
                (void)GetWindowTextW(contents[index], text, 32);
                check(wcscmp(text, index == 0 ? L"First state" : L"Second state") == 0, "per-tab content survives switching");
                check(ui_surface_make_current(surfaces[index]) == UI_STATUS_OK &&
                    wglGetCurrentContext() == contexts[index], "original GL context survives switching");
                check(ui_surface_get_pixel_rect(surfaces[index], &pixels) == UI_STATUS_OK &&
                    GetClientRect(gl_windows[index], &drawable) &&
                    drawable.right == pixels.width && drawable.bottom == pixels.height,
                    "actual drawable follows tab DPI and logical layout");
                glViewport(0, 0, pixels.width, pixels.height);
                glClear(GL_COLOR_BUFFER_BIT);
                check(glGetError() == GL_NO_ERROR && ui_surface_swap_buffers(surfaces[index]) == UI_STATUS_OK,
                    "GL still renders after activation and DPI/resize");
                if (index == 0) {
                    check(!shown(floating[0]) && GetWindow(sidebars[0], GW_OWNER) == root,
                        "activation preserves closed float and manually floating sidebar");
                }
            }
        }
    }
    check(ui_native_shell_handle_message(shells[1], parents[1], WM_COMMAND,
        native_commands[1], 0, &result) == UI_STATUS_OK && counters[1] == 1 && counters[0] == 1,
        "identical IDs route to active app container");
    {
        HWND tag = FindWindowExW(parents[1], NULL, L"BUTTON", L"Inspector");
        check(ui_host_resize(hosts[1], 480, 480) == UI_STATUS_OK &&
            ui_native_shell_reflow(shells[1]) == UI_STATUS_OK && shown(tag) && !shown(sidebars[1]),
            "active narrow tab shows collapsed sidebar tag");
        check(ui_native_shell_set_active(shells[1], 0) == UI_STATUS_OK && !shown(tag),
            "deactivate hides collapsed tag without destruction");
        check(ui_native_shell_set_active(shells[0], 1) == UI_STATUS_OK &&
            ui_native_shell_set_active(shells[1], 1) == UI_STATUS_OK &&
            ui_native_shell_set_active(shells[0], 0) == UI_STATUS_OK && GetMenu(root) == menus[1],
            "late previous-tab deactivation never detaches new active menu");
        check(shown(tag) && ui_native_shell_handle_message(shells[1], parents[1], WM_COMMAND,
            MAKEWPARAM(0, BN_CLICKED), (intptr_t)tag, &result) == UI_STATUS_OK &&
            shown(sidebars[1]) && GetWindow(sidebars[1], GW_OWNER) == root,
            "restored collapsed tag can float existing sidebar content");
        check(IsWindow(contents[1]) && GetParent(contents[1]) == sidebars[1],
            "collapsed tag action preserves application content");
    }
    ui_native_shell_destroy(shells[0]);
    shells[0] = NULL;
    check(GetMenu(root) == menus[1] && IsWindow(contents[1]) && !IsWindow(contents[0]),
        "closing background shell preserves foreground menu and content");
    ui_host_destroy(hosts[0]);
    hosts[0] = NULL;
    check(ui_surface_make_current(surfaces[1]) == UI_STATUS_OK &&
        wglGetCurrentContext() == contexts[1], "background cleanup preserves foreground GL context");
    ui_native_shell_destroy(shells[1]);
    shells[1] = NULL;
    check(GetMenu(root) == NULL, "closing active managed shell safely detaches menu");
    check(SetMenu(root, fallback), "host reinstalls fallback after last app closes");

    /* Pass a physically smaller original descriptor, rather than merely using
       a new descriptor with its size shortened, to exercise binary compatibility. */
    {
        struct legacy_config { uint32_t size; ui_host_t *host; void *native_parent; } legacy;
        memset(&legacy, 0, sizeof(legacy));
        legacy.size = sizeof(legacy);
        legacy.host = hosts[1];
        legacy.native_parent = root;
        shells[1] = ui_native_shell_create((const ui_native_shell_config_t *)&legacy);
        check(shells[1] != NULL && GetMenu(root) == ui_native_shell_menu_handle(shells[1]),
            "original binary descriptor size still creates active legacy shell");
        ui_native_shell_destroy(shells[1]);
        shells[1] = NULL;
        check(GetMenu(root) == fallback, "legacy shell restores previous root menu");
    }

cleanup:
    for (index = 0; index < 2; ++index) {
        ui_native_shell_destroy(shells[index]);
        ui_host_destroy(hosts[index]);
    }
    if (root != NULL) { (void)SetMenu(root, NULL); DestroyWindow(root); }
    if (fallback != NULL) DestroyMenu(fallback);
    (void)UnregisterClassW(window_class.lpszClassName, window_class.hInstance);
    printf("Native managed activation: %d failure(s)\n", failures);
    return failures != 0;
}
