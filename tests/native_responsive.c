#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

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

static int visible_style(HWND hwnd)
{
    return (GetWindowLongPtrW(hwnd, GWL_STYLE) & WS_VISIBLE) != 0;
}

static int is_floating(HWND hwnd, HWND owner)
{
    LONG_PTR style = GetWindowLongPtrW(hwnd, GWL_STYLE);
    return (style & WS_POPUP) && !(style & WS_CHILD) &&
            GetWindow(hwnd, GW_OWNER) == owner;
}

static void execute(ui_host_t *host, uint64_t request_id,
                     const char *command_id, const char *params_json,
                     const char *source, void *user_data)
{
    (void)command_id;
    (void)params_json;
    (void)source;
    (void)user_data;
    (void)ui_host_reply(host, request_id, 1, "{}");
}

static void register_toolbar(ui_host_t *host, const char *id, int order,
                               int visible, const char *item_id,
                               const char *title, int item_order)
{
    ui_toolbar_desc_t toolbar;
    ui_toolbar_item_desc_t item;
    memset(&toolbar, 0, sizeof(toolbar));
    toolbar.size = sizeof(toolbar);
    toolbar.id = id;
    toolbar.title = id;
    toolbar.order = order;
    toolbar.visible = visible;
    check(ui_host_register_toolbar(host, &toolbar) == UI_STATUS_OK, "register toolbar");
    memset(&item, 0, sizeof(item));
    item.size = sizeof(item);
    item.id = item_id;
    item.toolbar_id = id;
    item.title = title;
    item.command_id = "test.execute";
    item.order = item_order;
    check(ui_host_register_toolbar_item(host, &item) == UI_STATUS_OK, "register toolbar item");
}

static void check_toolbar_title(HWND toolbar, int index, const wchar_t *expected)
{
    TBBUTTON button;
    wchar_t title[128];
    memset(&button, 0, sizeof(button));
    check(SendMessageW(toolbar, TB_GETBUTTON, (WPARAM)index, (LPARAM)&button) == TRUE,
          "get merged toolbar button");
    memset(title, 0, sizeof(title));
    check(SendMessageW(toolbar, TB_GETBUTTONTEXTW, (WPARAM)button.idCommand,
                       (LPARAM)title) >= 0 && wcscmp(title, expected) == 0,
          "merged toolbar order preserves toolbar then item ordering");
}

static void check_dock_rect(ui_host_t *host, HWND parent, HWND panel,
                              ui_layout_region_t region)
{
    ui_rect_t logical;
    RECT actual;
    uint32_t dpi;
    check(ui_host_get_rect(host, region, &logical) == UI_STATUS_OK, "query dock region");
    check(ui_host_get_dpi(host, &dpi) == UI_STATUS_OK, "query dock DPI");
    check(GetWindowRect(panel, &actual), "query native dock rectangle");
    (void)MapWindowPoints(NULL, parent, (POINT *)&actual, 2);
    check(actual.left == (int)(((int64_t)logical.x * dpi + 48) / 96) &&
              actual.top == (int)(((int64_t)logical.y * dpi + 48) / 96) &&
              actual.right - actual.left == (int)(((int64_t)logical.width * dpi + 48) / 96) &&
              actual.bottom - actual.top == (int)(((int64_t)logical.height * dpi + 48) / 96),
          "native dock rectangle follows logical size and DPI");
}

static void check_floating_dpi(HWND panel, const RECT *expected, int font_height)
{
    RECT actual;
    LOGFONTW font;
    HFONT native_font = (HFONT)SendMessageW(panel, WM_GETFONT, 0, 0);
    check(GetWindowRect(panel, &actual) && actual.left == expected->left &&
              actual.top == expected->top && actual.right == expected->right &&
              actual.bottom == expected->bottom,
          "floating panel preserves its own DPI suggested rectangle");
    memset(&font, 0, sizeof(font));
    check(native_font != NULL && GetObjectW(native_font, sizeof(font), &font) &&
              font.lfHeight == font_height,
          "floating panel preserves its own DPI font");
}

int main(void)
{
    WNDCLASSW window_class;
    HWND parent;
    HWND left;
    HWND right;
    HWND right_second;
    HWND floating;
    HWND content;
    HWND tag;
    HWND toolbar;
    ui_host_config_t host_config;
    ui_native_shell_config_t shell_config;
    ui_command_desc_t command;
    ui_toolbar_item_desc_t extra_item;
    ui_panel_desc_t panel;
    ui_layout_desc_t layout;
    ui_host_t *host;
    ui_native_shell_t *shell;
    ui_sidebar_state_t state;
    LOGFONTW font;
    HFONT native_font;
    MINMAXINFO limits;
    RECT button_at_96;
    RECT button_at_192;
    RECT floating_at_144 = {120, 80, 480, 350};
    RECT floating_at_192 = {120, 80, 600, 440};
    intptr_t result;

    check(ui_framework_initialize() == UI_STATUS_OK, "initialize framework");
    memset(&window_class, 0, sizeof(window_class));
    window_class.lpfnWndProc = DefWindowProcW;
    window_class.hInstance = GetModuleHandleW(NULL);
    window_class.lpszClassName = L"UiNativeResponsiveTest";
    check(RegisterClassW(&window_class) != 0, "register host class");
    parent = CreateWindowExW(0, window_class.lpszClassName, L"Responsive test",
                             WS_OVERLAPPEDWINDOW, 0, 0, 1280, 720,
                             NULL, NULL, window_class.hInstance, NULL);
    if (parent == NULL) return 1;
    memset(&host_config, 0, sizeof(host_config));
    host_config.size = sizeof(host_config);
    host_config.api_version = UI_FRAMEWORK_API_VERSION;
    host_config.native_parent = parent;
    host = ui_host_create(&host_config);
    if (host == NULL) {
        DestroyWindow(parent);
        return 1;
    }
    memset(&command, 0, sizeof(command));
    command.size = sizeof(command);
    command.id = "test.execute";
    command.title = "Execute";
    command.params_schema_json = "{}";
    command.handler = execute;
    check(ui_host_register_command(host, &command) == UI_STATUS_OK, "register command");
    register_toolbar(host, "late", 50, 1, "late.item", "Late", 0);
    register_toolbar(host, "first", 10, 1, "first.second", "Second", 20);
    register_toolbar(host, "hidden", 0, 0, "hidden.item", "Hidden", 0);
    memset(&extra_item, 0, sizeof(extra_item));
    extra_item.size = sizeof(extra_item);
    extra_item.id = "first.first";
    extra_item.toolbar_id = "first";
    extra_item.title = "First";
    extra_item.command_id = command.id;
    extra_item.order = 10;
    check(ui_host_register_toolbar_item(host, &extra_item) == UI_STATUS_OK, "add ordered item");
    memset(&panel, 0, sizeof(panel));
    panel.size = sizeof(panel);
    panel.kind = UI_PANEL_SIDEBAR;
    panel.entry_url = "app://test/panel";
    panel.preferred_width = 200;
    panel.id = "left";
    panel.title = "Left";
    panel.dock_region = UI_LAYOUT_REGION_LEFT_SIDEBAR;
    check(ui_host_register_panel(host, &panel) == UI_STATUS_OK, "register left panel");
    panel.id = "right";
    panel.title = "Right";
    panel.dock_region = UI_LAYOUT_REGION_RIGHT_SIDEBAR;
    check(ui_host_register_panel(host, &panel) == UI_STATUS_OK, "register right panel");
    panel.id = "right.second";
    panel.title = "Right Second";
    check(ui_host_register_panel(host, &panel) == UI_STATUS_OK, "register second right panel");
    panel.id = "floating";
    panel.title = "Floating";
    panel.kind = UI_PANEL_FLOATING;
    check(ui_host_register_panel(host, &panel) == UI_STATUS_OK, "register floating panel");
    memset(&layout, 0, sizeof(layout));
    layout.size = sizeof(layout);
    layout.toolbar_height = 40;
    layout.status_bar_height = 24;
    layout.left_sidebar_width = 180;
    layout.right_sidebar_width = 220;
    layout.left_sidebar_min_width = 180;
    layout.right_sidebar_min_width = 220;
    layout.main_min_width = 400;
    layout.collapsed_tag_width = 32;
    check(ui_host_set_layout(host, &layout) == UI_STATUS_OK, "set responsive layout");
    check(ui_host_set_dpi(host, 96u) == UI_STATUS_OK, "set initial DPI");
    check(ui_host_resize(host, 1280, 720) == UI_STATUS_OK, "set wide host size");
    memset(&shell_config, 0, sizeof(shell_config));
    shell_config.size = sizeof(shell_config);
    shell_config.host = host;
    shell_config.native_parent = parent;
    shell = ui_native_shell_create(&shell_config);
    if (shell == NULL) {
        ui_host_destroy(host);
        DestroyWindow(parent);
        return 1;
    }
    left = (HWND)ui_native_shell_panel_handle(shell, "left");
    right = (HWND)ui_native_shell_panel_handle(shell, "right");
    right_second = (HWND)ui_native_shell_panel_handle(shell, "right.second");
    floating = (HWND)ui_native_shell_panel_handle(shell, "floating");
    check(left && right && right_second && floating, "borrow panel containers by stable ID");
    check(ui_native_shell_panel_handle(shell, "missing") == NULL, "missing panel handle");
    check(GetParent(left) == parent && GetParent(right) == parent, "left and right panels are docked");
    check_dock_rect(host, parent, left, UI_LAYOUT_REGION_LEFT_SIDEBAR);
    check(is_floating(floating, parent), "registered floating panel is owned tool window");
    content = CreateWindowExW(0, L"EDIT", L"Application content", WS_CHILD | WS_VISIBLE,
                              0, 24, 100, 24, right, NULL, window_class.hInstance, NULL);
    check(content != NULL, "application embeds content child in borrowed container");
    toolbar = FindWindowExW(parent, NULL, TOOLBARCLASSNAMEW, NULL);
    check(toolbar && SendMessageW(toolbar, TB_BUTTONCOUNT, 0, 0) == 3,
          "all visible toolbars merge and hidden toolbar is excluded");
    check_toolbar_title(toolbar, 0, L"First");
    check_toolbar_title(toolbar, 1, L"Second");
    check_toolbar_title(toolbar, 2, L"Late");
    check(SendMessageW(toolbar, TB_GETITEMRECT, 0, (LPARAM)&button_at_96) == TRUE,
          "initial native toolbar button rectangle");

    check(ui_host_resize(host, 640, 480) == UI_STATUS_OK &&
              ui_native_shell_reflow(shell) == UI_STATUS_OK, "collapse narrow window");
    check(ui_host_get_sidebar_state(host, UI_LAYOUT_REGION_RIGHT_SIDEBAR, &state) == UI_STATUS_OK &&
              state == UI_SIDEBAR_COLLAPSED, "right sidebar has collapsed state");
    check(!visible_style(right) && !visible_style(right_second), "collapsed content windows hide");
    tag = FindWindowExW(parent, NULL, L"BUTTON", L"Right");
    check(tag != NULL && visible_style(tag), "collapsed sidebar shows clickable tag");
    check(ui_native_shell_handle_message(shell, parent, WM_COMMAND,
                                         MAKEWPARAM(0, BN_CLICKED), (intptr_t)tag,
                                         &result) == UI_STATUS_OK,
          "tag click opens panel");
    check(is_floating(right, parent) && visible_style(right), "tag floats selected panel");
    check(GetParent(content) == right && IsWindow(content), "application children survive floating");
    (void)SendMessageW(right, WM_CLOSE, 0, 0);
    check(IsWindow(right) && !visible_style(right), "closing floating panel hides rather than destroys");
    check(ui_native_shell_reflow(shell) == UI_STATUS_OK && !visible_style(right), "closed panel stays hidden on reflow");
    check(ui_native_shell_set_panel_floating(shell, "right", 1) == UI_STATUS_OK && visible_style(right),
          "explicit floating call reopens panel");
    check(ui_native_shell_set_panel_floating(shell, "right", 0) == UI_STATUS_OK &&
              GetParent(right) == parent && !visible_style(right), "return to collapsed dock");

    layout.narrow_policy = UI_NARROW_FLOAT_SIDEBARS;
    check(ui_host_set_layout(host, &layout) == UI_STATUS_OK &&
              ui_native_shell_reflow(shell) == UI_STATUS_OK, "enable automatic floating strategy");
    check(ui_host_get_sidebar_state(host, UI_LAYOUT_REGION_RIGHT_SIDEBAR, &state) == UI_STATUS_OK &&
              state == UI_SIDEBAR_FLOATING && is_floating(right, parent) && is_floating(right_second, parent),
          "automatic floating materializes both right panels");
    check(ui_host_resize(host, 1280, 720) == UI_STATUS_OK &&
              ui_native_shell_reflow(shell) == UI_STATUS_OK, "restore wide layout");
    check(GetParent(right) == parent && GetParent(right_second) == parent && visible_style(right),
          "automatic floating panels return to dock");
    check(GetParent(content) == right && IsWindow(content), "children survive automatic dock restoration");
    check(ui_native_shell_panel_handle(shell, "right") == right, "borrowed container remains stable through transitions");
    check(ui_native_shell_set_panel_floating(shell, "right", 1) == UI_STATUS_OK, "manual floating in wide layout");
    check_dock_rect(host, parent, right_second, UI_LAYOUT_REGION_RIGHT_SIDEBAR);
    (void)SendMessageW(right, WM_DPICHANGED, MAKEWPARAM(144, 144), (LPARAM)&floating_at_144);
    check_floating_dpi(right, &floating_at_144, -18);
    check(ui_host_set_dpi(host, 192u) == UI_STATUS_OK &&
              ui_native_shell_reflow(shell) == UI_STATUS_OK,
          "host DPI changes independently of floating panel");
    check_floating_dpi(right, &floating_at_144, -18);
    (void)SendMessageW(right, WM_DPICHANGED, MAKEWPARAM(192, 192), (LPARAM)&floating_at_192);
    check_floating_dpi(right, &floating_at_192, -24);
    check(ui_host_set_dpi(host, 96u) == UI_STATUS_OK &&
              ui_native_shell_reflow(shell) == UI_STATUS_OK,
          "host reflow retains floating panel monitor DPI");
    check_floating_dpi(right, &floating_at_192, -24);
    check(ui_native_shell_set_panel_floating(shell, "right", 0) == UI_STATUS_OK, "manual redock");
    native_font = (HFONT)SendMessageW(right, WM_GETFONT, 0, 0);
    check(native_font != NULL && GetObjectW(native_font, sizeof(font), &font) &&
              font.lfHeight == -12, "redock restores host DPI font");
    check(GetParent(content) == right && IsWindow(content), "application child survives floating DPI and redock");

    check(ui_host_set_dpi(host, 192u) == UI_STATUS_OK &&
              ui_native_shell_reflow(shell) == UI_STATUS_OK, "DPI 192 reflow");
    check_dock_rect(host, parent, left, UI_LAYOUT_REGION_LEFT_SIDEBAR);
    native_font = (HFONT)SendMessageW(left, WM_GETFONT, 0, 0);
    memset(&font, 0, sizeof(font));
    check(native_font != NULL && GetObjectW(native_font, sizeof(font), &font) != 0 &&
              font.lfHeight == -24, "DPI reflow scales native panel font");
    native_font = (HFONT)SendMessageW(toolbar, WM_GETFONT, 0, 0);
    check(native_font != NULL && GetObjectW(native_font, sizeof(font), &font) != 0 &&
              font.lfHeight == -24, "DPI reflow scales toolbar font");
    check(SendMessageW(toolbar, TB_GETITEMRECT, 0, (LPARAM)&button_at_192) == TRUE &&
              button_at_192.bottom - button_at_192.top ==
                  2 * (button_at_96.bottom - button_at_96.top),
          "DPI reflow scales toolbar button height");
    layout.narrow_policy = UI_NARROW_DISALLOW_SHRINK;
    check(ui_host_set_layout(host, &layout) == UI_STATUS_OK, "set minimum tracking policy");
    memset(&limits, 0, sizeof(limits));
    check(ui_host_handle_message(host, parent, WM_GETMINMAXINFO, 0,
                                  (intptr_t)&limits, &result) == UI_STATUS_OK &&
              limits.ptMinTrackSize.x >= 1600 && limits.ptMinTrackSize.y >= 128,
          "minimum tracking size includes scaled content and window chrome");
    (void)SendMessageW(floating, WM_CLOSE, 0, 0);
    check(IsWindow(floating) && !visible_style(floating) &&
              ui_native_shell_reflow(shell) == UI_STATUS_OK && !visible_style(floating),
          "registered floating panel close is safe across reflow");
    check(ui_native_shell_set_panel_floating(shell, "missing", 1) == UI_STATUS_NOT_FOUND,
          "missing panel reports not found");
    check(ui_native_shell_set_panel_floating(shell, "floating", 0) == UI_STATUS_UNSUPPORTED,
          "fixed floating registration cannot be docked as sidebar");

    ui_native_shell_destroy(shell);
    check(!IsWindow(content), "shell destruction destroys application content containers");
    ui_host_destroy(host);
    DestroyWindow(parent);
    (void)UnregisterClassW(window_class.lpszClassName, window_class.hInstance);
    printf("Native responsive integration: %d failure(s)\n", failures);
    return failures != 0;
}
