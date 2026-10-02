#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "ui_framework/shell.h"
#include "../src/ui_internal.h"

static int failed;
static int invoked;

static void check(int condition, const char *message)
{
    if (!condition) { fprintf(stderr, "Web shell slots: %s\n", message); ++failed; }
}

static void command(ui_host_t *host, uint64_t request_id, const char *id,
                      const char *params, const char *source, void *data)
{
    (void)id; (void)params; (void)source; (void)data;
    ++invoked;
    (void)ui_host_reply(host, request_id, 1, "{}");
}

static void register_panel(ui_host_t *host, const char *id)
{
    ui_panel_desc_t panel;
    memset(&panel, 0, sizeof(panel));
    panel.size = sizeof(panel); panel.id = id; panel.title = "Properties <&>";
    panel.kind = UI_PANEL_SIDEBAR; panel.preferred_width = 220;
    panel.entry_url = "app://fixture/panel";
    panel.dock_region = UI_LAYOUT_REGION_RIGHT_SIDEBAR;
    check(ui_host_register_panel(host, &panel) == UI_STATUS_OK, "register panel");
}

int main(void)
{
    HWND root, container, panel, app_child, surface_window;
    ui_host_t *host;
    ui_host_config_t host_config;
    ui_native_shell_config_t config;
    ui_native_shell_t *native;
    ui_shell_t *shell;
    ui_content_slot_t *main_slot, *panel_slot;
    ui_surface_t *surface;
    ui_surface_desc_t desc;
    ui_layout_desc_t layout;
    ui_command_desc_t cmd;
    ui_menu_item_desc_t menu;
    ui_toolbar_desc_t toolbar;
    ui_toolbar_item_desc_t item;
    ui_rect_t frame = {0}, content = {0}, pixels = {0}, actual_surface = {0};
    RECT actual;
    int visible = 0, floating = 0;
    size_t i, j;
    const int sizes[][2] = {{1920,1080}, {1280,720}, {800,600}, {640,480}};
    const uint32_t dpis[] = {96,144,192};
    check(ui_framework_initialize() == UI_STATUS_OK, "initialize DPI");
    root = CreateWindowExW(0, L"STATIC", L"Web shell test", WS_OVERLAPPEDWINDOW,
        0, 0, 1000, 700, NULL, NULL, GetModuleHandleW(NULL), NULL);
    container = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_CLIPCHILDREN,
        0, 0, 1000, 700, root, NULL, GetModuleHandleW(NULL), NULL);
    if (root == NULL || container == NULL) return 1;
    memset(&host_config, 0, sizeof(host_config));
    host_config.size = sizeof(host_config); host_config.api_version = 1;
    host_config.native_parent = container;
    host = ui_host_create(&host_config);
    if (host == NULL) return 1;
    memset(&layout, 0, sizeof(layout));
    layout.size = sizeof(layout); layout.menu_bar_height = 28; layout.toolbar_height = 34;
    layout.right_sidebar_width = 220; layout.right_sidebar_min_width = 160;
    layout.right_sidebar_preferred_width = 220; layout.right_sidebar_max_width = 280;
    layout.main_min_width = 500; layout.collapsed_tag_width = 28;
    check(ui_host_set_layout(host, &layout) == UI_STATUS_OK, "set layout");
    check(ui_host_resize(host, 1000, 700) == UI_STATUS_OK, "initial logical resize");
    memset(&cmd, 0, sizeof(cmd));
    cmd.size = sizeof(cmd); cmd.id = "fixture.action"; cmd.title = "Action";
    cmd.params_schema_json = "{}"; cmd.handler = command;
    check(ui_host_register_command(host, &cmd) == UI_STATUS_OK, "register command");
    memset(&menu, 0, sizeof(menu));
    menu.size = sizeof(menu); menu.id = "fixture.menu"; menu.menu_path = "File";
    menu.title = "Action"; menu.command_id = cmd.id;
    check(ui_host_register_menu_item(host, &menu) == UI_STATUS_OK, "register menu");
    memset(&toolbar, 0, sizeof(toolbar)); toolbar.size = sizeof(toolbar);
    toolbar.id = "fixture.toolbar"; toolbar.title = "Tools"; toolbar.visible = 1;
    check(ui_host_register_toolbar(host, &toolbar) == UI_STATUS_OK, "register toolbar");
    memset(&item, 0, sizeof(item)); item.size = sizeof(item); item.id = "fixture.button";
    item.toolbar_id = toolbar.id; item.command_id = cmd.id; item.title = "Action";
    check(ui_host_register_toolbar_item(host, &item) == UI_STATUS_OK, "register toolbar item");
    register_panel(host, "fixture.panel");
    memset(&config, 0, sizeof(config)); config.size = sizeof(config); config.host = host;
    config.native_parent = container; config.menu_owner = root;
    config.flags = UI_NATIVE_SHELL_MANAGED_ACTIVATION;
    native = ui_native_shell_create_web(&config);
    check(native != NULL, "create Web compatibility facade");
    if (native == NULL) return 1;
    shell = ui_host_get_shell(host); main_slot = ui_shell_get_content_slot(shell, NULL);
    panel_slot = ui_shell_get_content_slot(shell, "fixture.panel");
    check(shell != NULL && main_slot != NULL && panel_slot != NULL, "borrow shell and content slots");
    check(ui_content_slot_native_handle(main_slot) == container, "main container is legacy host parent");
    panel = (HWND)ui_content_slot_native_handle(panel_slot);
    check(panel == ui_native_shell_panel_handle(native, "fixture.panel"), "old and new panel handles agree");
    check(GetMenu(root) == NULL && ui_native_shell_menu_handle(native) != NULL, "menu mirror stays detached");
    check(FindWindowExW(container, NULL, L"ToolbarWindow32", NULL) == NULL &&
          FindWindowExW(container, NULL, L"BUTTON", NULL) == NULL, "no native toolbar or collapsed tag");
    check(ui_native_shell_set_active(native, 1) == UI_STATUS_OK && GetMenu(root) == NULL,
          "activation retains Web menu ownership");
    check(ui_native_shell_handle_message(native, root, WM_COMMAND, 0x5000, 0, NULL) == UI_STATUS_OK &&
          invoked == 1, "legacy menu mirror routes registered command");
    app_child = CreateWindowExW(0, L"EDIT", L"application text", WS_CHILD | WS_VISIBLE,
        0, 0, 100, 24, panel, NULL, GetModuleHandleW(NULL), NULL);
    memset(&desc, 0, sizeof(desc)); desc.size = sizeof(desc); desc.id = "fixture.native";
    desc.kind = UI_SURFACE_NATIVE; desc.visible = 1;
    surface = ui_surface_create(host, &desc);
    check(surface != NULL, "application chooses non-OpenGL surface");
    if (surface == NULL) return 1;
    surface_window = (HWND)ui_surface_native_handle(surface);
    check(ui_content_slot_attach_surface(panel_slot, surface) == UI_STATUS_OK &&
          GetParent(surface_window) == panel, "bind surface to actual panel content window");
    check(ui_content_slot_attach_surface(main_slot, surface) == UI_STATUS_ALREADY_EXISTS,
          "a surface cannot bind two slots");
    for (i = 0; i < sizeof(dpis) / sizeof(dpis[0]); ++i) {
        check(ui_host_set_dpi(host, dpis[i]) == UI_STATUS_OK, "set test DPI");
        for (j = 0; j < sizeof(sizes) / sizeof(sizes[0]); ++j) {
            check(ui_host_resize(host, sizes[j][0], sizes[j][1]) == UI_STATUS_OK, "resize layout matrix");
            check(ui_content_slot_get_rect(panel_slot, &content) == UI_STATUS_OK &&
                ui_content_slot_get_pixel_rect(panel_slot, &pixels) == UI_STATUS_OK, "slot geometry query");
            check(ui_shell_get_slot_state(shell, "fixture.panel", &frame, NULL, &visible, &floating) ==
                UI_STATUS_OK && !floating, "docked state query");
            if (visible) {
                GetClientRect(surface_window, &actual);
                check(actual.right == pixels.width && actual.bottom == pixels.height,
                      "framebuffer matches slot physical extent");
                check(ui_surface_get_rect(surface, &actual_surface) == UI_STATUS_OK &&
                    memcmp(&actual_surface, &content, sizeof(content)) == 0,
                    "surface API preserves host logical coordinates while docked");
                check(content.y == frame.y + 28, "Web panel title reserves logical space");
            } else check(content.width == 0 && content.height == 0, "collapsed panel hides content");
        }
    }
    check(ui_host_set_dpi(host, 96) == UI_STATUS_OK && ui_host_resize(host, 1000, 700) == UI_STATUS_OK,
          "restore docked geometry");
    check(ui_native_shell_set_panel_floating(native, "fixture.panel", 1) == UI_STATUS_OK,
          "float panel with Web title");
    check(ui_content_slot_native_handle(panel_slot) == panel && GetParent(panel) != container,
          "floating preserves borrowed content HWND");
    check((GetWindowLongPtrW(GetParent(panel), GWL_STYLE) & WS_CAPTION) == 0,
          "floating title is Web rather than native caption");
    check(ui_content_slot_get_rect(panel_slot, &content) == UI_STATUS_OK && content.x == 0 && content.y == 0,
          "floating slot reports local content dimensions");
    check(ui_native_shell_set_active(native, 0) == UI_STATUS_OK &&
          !(GetWindowLongPtrW(GetParent(panel), GWL_STYLE) & WS_VISIBLE), "background hides owned popup");
    check(ui_native_shell_set_active(native, 1) == UI_STATUS_OK &&
          ui_native_shell_set_panel_floating(native, "fixture.panel", 0) == UI_STATUS_OK &&
          GetParent(panel) == container && IsWindow(app_child), "activation and redock preserve application children");
    register_panel(host, "fixture.second");
    check(ui_shell_refresh(shell) == UI_STATUS_OK && ui_content_slot_native_handle(panel_slot) == panel &&
          IsWindow(app_child) && GetParent(surface_window) == panel, "incremental refresh preserves content");
    check(ui_shell_get_content_slot(shell, "fixture.second") != NULL, "incremental refresh adds new slot");
    ui_surface_destroy(surface);
    check(ui_host_resize(host, 900, 600) == UI_STATUS_OK, "destroyed content binding cannot dangle");
    check(ui_native_shell_refresh(native) == UI_STATUS_OK && !IsWindow(app_child),
          "legacy refresh explicitly destroys old panel children");
    check(ui_shell_get_content_slot(shell, "fixture.panel") == panel_slot,
          "slot identity remains stable after legacy container rebuild");
    ui_native_shell_destroy(native);
    check(ui_host_get_shell(host) == NULL, "destroy clears borrowed shell");
    ui_host_destroy(host); DestroyWindow(root);
    if (failed) return 1;
    puts("Web shell content slots and legacy facade passed");
    return 0;
}
