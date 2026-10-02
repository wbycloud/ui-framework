#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <commctrl.h>

#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include "ui_framework/ui.h"
#include "ui_framework/native.h"

static int invoked_count;

static void execute_command(ui_host_t *host,
                            uint64_t request_id,
                            const char *command_id,
                            const char *params_json,
                            const char *source,
                            void *user_data)
{
    (void)command_id;
    (void)params_json;
    (void)source;
    (void)user_data;
    invoked_count += 1;
    (void)ui_host_reply(host, request_id, 1, "{}");
}

static int check(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "native shell check failed: %s\n", message);
    }
    return condition;
}

static HWND find_child(HWND parent, const wchar_t *class_name,
                        const wchar_t *title)
{
    HWND child;

    for (child = GetWindow(parent, GW_CHILD); child != NULL;
         child = GetWindow(child, GW_HWNDNEXT)) {
        wchar_t actual_class[128];
        wchar_t actual_title[128];
        (void)GetClassNameW(child, actual_class,
                            (int)(sizeof(actual_class) /
                                  sizeof(actual_class[0])));
        (void)GetWindowTextW(child, actual_title,
                             (int)(sizeof(actual_title) /
                                   sizeof(actual_title[0])));
        if (_wcsicmp(actual_class, class_name) == 0 &&
            (title == NULL || wcscmp(actual_title, title) == 0)) {
            return child;
        }
    }
    return NULL;
}

static int scale_coordinate(int logical, uint32_t dpi)
{
    return (int)(((int64_t)logical * dpi + 48) / 96);
}

static int check_panel_rect(ui_host_t *host, HWND parent, HWND panel)
{
    ui_rect_t expected;
    RECT actual;
    uint32_t dpi;

    if (!check(ui_host_get_rect(host, UI_LAYOUT_REGION_RIGHT_SIDEBAR,
                                &expected) == UI_STATUS_OK,
               "right sidebar logical rect") ||
        !check(ui_host_get_dpi(host, &dpi) == UI_STATUS_OK,
               "host DPI") ||
        !check(GetWindowRect(panel, &actual) != FALSE,
               "sidebar device rect")) {
        return 0;
    }
    (void)MapWindowPoints(NULL, parent, (POINT *)&actual, 2);
    return check(actual.left == scale_coordinate(expected.x, dpi) &&
                 actual.top == scale_coordinate(expected.y, dpi) &&
                 actual.right ==
                     scale_coordinate(expected.x + expected.width, dpi) &&
                 actual.bottom ==
                     scale_coordinate(expected.y + expected.height, dpi),
                 "sidebar follows logical rect and DPI");
}

int main(void)
{
    WNDCLASSW window_class;
    HWND parent = NULL;
    HWND toolbar;
    HWND sidebar;
    HMENU previous_menu = NULL;
    HMENU root_menu;
    HMENU file_menu;
    HMENU actions_menu;
    UINT command_id;
    ui_host_t *host = NULL;
    ui_native_shell_t *shell = NULL;
    ui_host_config_t host_config;
    ui_native_shell_config_t shell_config;
    ui_command_desc_t command;
    ui_menu_item_desc_t menu_item;
    ui_toolbar_desc_t toolbar_desc;
    ui_toolbar_item_desc_t toolbar_item;
    ui_panel_desc_t panel;
    ui_layout_desc_t layout;
    intptr_t native_result = 0;
    int success = 0;

    if (!check(ui_framework_initialize() == UI_STATUS_OK,
               "initialize DPI mode")) {
        return 1;
    }
    ZeroMemory(&window_class, sizeof(window_class));
    window_class.lpfnWndProc = DefWindowProcW;
    window_class.hInstance = GetModuleHandleW(NULL);
    window_class.lpszClassName = L"UiNativeShellIntegrationTest";
    if (!check(RegisterClassW(&window_class) != 0,
               "test parent class")) {
        return 1;
    }
    parent = CreateWindowExW(0, window_class.lpszClassName,
                              L"Native shell integration", WS_OVERLAPPEDWINDOW,
                              0, 0, 1000, 700, NULL, NULL,
                              window_class.hInstance, NULL);
    if (!check(parent != NULL, "test parent")) {
        goto cleanup;
    }
    previous_menu = CreateMenu();
    if (!check(previous_menu != NULL && SetMenu(parent, previous_menu),
               "existing application menu")) {
        goto cleanup;
    }

    ZeroMemory(&host_config, sizeof(host_config));
    host_config.size = sizeof(host_config);
    host_config.api_version = UI_FRAMEWORK_API_VERSION;
    host_config.native_parent = parent;
    host = ui_host_create(&host_config);
    if (!check(host != NULL, "host creation")) {
        goto cleanup;
    }
    ZeroMemory(&command, sizeof(command));
    command.size = sizeof(command);
    command.id = "test.execute";
    command.title = "Execute";
    command.params_schema_json = "{}";
    command.handler = execute_command;
    if (!check(ui_host_register_command(host, &command) == UI_STATUS_OK,
               "command registration")) {
        goto cleanup;
    }

    ZeroMemory(&menu_item, sizeof(menu_item));
    menu_item.size = sizeof(menu_item);
    menu_item.id = "test.menu.execute";
    menu_item.menu_path = "File/Actions";
    menu_item.title = "Execute";
    menu_item.command_id = command.id;
    if (!check(ui_host_register_menu_item(host, &menu_item) == UI_STATUS_OK,
               "menu registration")) {
        goto cleanup;
    }
    ZeroMemory(&toolbar_desc, sizeof(toolbar_desc));
    toolbar_desc.size = sizeof(toolbar_desc);
    toolbar_desc.id = "test.toolbar";
    toolbar_desc.title = "Test";
    toolbar_desc.visible = 1;
    if (!check(ui_host_register_toolbar(host, &toolbar_desc) == UI_STATUS_OK,
               "toolbar registration")) {
        goto cleanup;
    }
    ZeroMemory(&toolbar_item, sizeof(toolbar_item));
    toolbar_item.size = sizeof(toolbar_item);
    toolbar_item.id = "test.toolbar.execute";
    toolbar_item.toolbar_id = toolbar_desc.id;
    toolbar_item.title = "Execute";
    toolbar_item.command_id = command.id;
    if (!check(ui_host_register_toolbar_item(host, &toolbar_item) ==
                   UI_STATUS_OK,
               "toolbar item registration")) {
        goto cleanup;
    }

    ZeroMemory(&panel, sizeof(panel));
    panel.size = sizeof(panel);
    panel.id = "test.inspector";
    panel.title = "Inspector";
    panel.kind = UI_PANEL_SIDEBAR;
    panel.entry_url = "app://test/inspector";
    panel.preferred_width = 200;
    if (!check(ui_host_register_panel(host, &panel) == UI_STATUS_OK,
               "sidebar registration")) {
        goto cleanup;
    }
    panel.id = "test.floating";
    panel.title = "Floating";
    panel.kind = UI_PANEL_FLOATING;
    panel.entry_url = "app://test/floating";
    if (!check(ui_host_register_panel(host, &panel) == UI_STATUS_OK,
               "floating panel registration")) {
        goto cleanup;
    }
    ZeroMemory(&layout, sizeof(layout));
    layout.size = sizeof(layout);
    layout.toolbar_height = 40;
    layout.right_sidebar_width = 200;
    layout.status_bar_height = 24;
    if (!check(ui_host_set_layout(host, &layout) == UI_STATUS_OK &&
                 ui_host_resize(host, 800, 600) == UI_STATUS_OK,
               "initial layout")) {
        goto cleanup;
    }
    ZeroMemory(&shell_config, sizeof(shell_config));
    shell_config.size = sizeof(shell_config);
    shell_config.host = host;
    shell_config.native_parent = parent;
    shell = ui_native_shell_create(&shell_config);
    if (!check(shell != NULL, "native shell creation")) {
        goto cleanup;
    }

    root_menu = GetMenu(parent);
    file_menu = GetSubMenu(root_menu, 0);
    actions_menu = GetSubMenu(file_menu, 0);
    if (!check(root_menu != NULL && GetMenuItemCount(root_menu) == 1 &&
                 file_menu != NULL && GetMenuItemCount(file_menu) == 1 &&
                 actions_menu != NULL && GetMenuItemCount(actions_menu) == 1,
               "nested menu path preserves hierarchy")) {
        goto cleanup;
    }
    command_id = GetMenuItemID(actions_menu, 0);
    if (!check(command_id != (UINT)-1 &&
                 ui_native_shell_handle_message(shell, parent, WM_COMMAND,
                                                  command_id, 0,
                                                  &native_result) ==
                     UI_STATUS_OK && invoked_count == 1,
               "menu command invokes host handler")) {
        goto cleanup;
    }
    toolbar = find_child(parent, TOOLBARCLASSNAMEW, NULL);
    sidebar = find_child(parent, L"STATIC", L"Inspector");
    if (!check(toolbar != NULL &&
                 SendMessageW(toolbar, TB_BUTTONCOUNT, 0, 0) == 1,
               "toolbar contains registered item") ||
        !check(sidebar != NULL, "sidebar child created") ||
        !check_panel_rect(host, parent, sidebar)) {
        goto cleanup;
    }
    if (!check(ui_native_shell_handle_message(
                    shell, parent, WM_SIZE, 0, MAKELPARAM(640, 480),
                    &native_result) == UI_STATUS_OK,
               "resize message") ||
        !check_panel_rect(host, parent, sidebar)) {
        goto cleanup;
    }
    if (!check(ui_native_shell_handle_message(
                    shell, parent, WM_DPICHANGED, MAKEWPARAM(144, 144), 0,
                    &native_result) == UI_STATUS_OK,
               "DPI change message") ||
        !check_panel_rect(host, parent, sidebar)) {
        goto cleanup;
    }
    if (!check(ui_native_shell_refresh(shell) == UI_STATUS_OK,
               "refresh registrations")) {
        goto cleanup;
    }
    toolbar = find_child(parent, TOOLBARCLASSNAMEW, NULL);
    if (!check(toolbar != NULL &&
                 SendMessageW(toolbar, TB_BUTTONCOUNT, 0, 0) == 1,
               "refresh keeps single toolbar item")) {
        goto cleanup;
    }
    ui_native_shell_destroy(shell);
    shell = NULL;
    if (!check(GetMenu(parent) == previous_menu,
               "shell destruction restores original menu")) {
        goto cleanup;
    }
    success = 1;
    puts("native shell integration checks passed");

cleanup:
    ui_native_shell_destroy(shell);
    ui_host_destroy(host);
    if (parent != NULL) {
        (void)SetMenu(parent, NULL);
        DestroyWindow(parent);
    }
    if (previous_menu != NULL) {
        DestroyMenu(previous_menu);
    }
    (void)UnregisterClassW(window_class.lpszClassName,
                           window_class.hInstance);
    return success ? 0 : 1;
}
