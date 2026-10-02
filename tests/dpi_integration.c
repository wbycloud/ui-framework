#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdio.h>
#include <string.h>

#include "ui_framework/ui.h"

static int failures;

static void check(int condition, const char *description)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", description);
        ++failures;
    }
}

static void check_child_rect(HWND parent, HWND child,
                              const ui_rect_t *expected)
{
    RECT actual;
    POINT origin;

    check(GetWindowRect(child, &actual) != 0, "GetWindowRect child");
    origin.x = actual.left;
    origin.y = actual.top;
    check(ScreenToClient(parent, &origin) != 0, "ScreenToClient child");
    check(origin.x == expected->x && origin.y == expected->y &&
              actual.right - actual.left == expected->width &&
              actual.bottom - actual.top == expected->height,
          "native child rectangle matches pixel rectangle");
}

int main(void)
{
    static const uint32_t dpis[] = {96u, 144u, 192u};
    static const ui_rect_t manual_expected[] = {
        {13,17,101,77}, {20,26,151,115}, {26,34,202,154}
    };
    WNDCLASSA window_class;
    HWND window;
    ui_host_config_t config;
    ui_layout_desc_t layout;
    ui_surface_desc_t surface_desc;
    ui_host_t *host;
    ui_surface_t *bound_surface;
    ui_surface_t *manual_surface;
    ui_rect_t logical_rect;
    ui_rect_t pixel_rect;
    ui_rect_t manual_pixel_rect;
    uint32_t dpi;
    intptr_t result;
    size_t index;

    check(ui_framework_initialize() == UI_STATUS_OK, "DPI initialization");
    memset(&window_class, 0, sizeof(window_class));
    window_class.lpfnWndProc = DefWindowProcA;
    window_class.hInstance = GetModuleHandleA(NULL);
    window_class.lpszClassName = "UiDpiIntegrationWindow";
    check(RegisterClassA(&window_class) != 0, "register host window class");
    window = CreateWindowExA(0, window_class.lpszClassName, "DPI test",
                              WS_OVERLAPPEDWINDOW, 0, 0, 800, 600,
                              NULL, NULL, window_class.hInstance, NULL);
    if (window == NULL) {
        fprintf(stderr, "FAIL: create host window\n");
        return 1;
    }

    memset(&config, 0, sizeof(config));
    config.size = sizeof(config);
    config.api_version = UI_FRAMEWORK_API_VERSION;
    config.native_parent = (void *)window;
    host = ui_host_create(&config);
    if (host == NULL) {
        DestroyWindow(window);
        return 1;
    }

    memset(&layout, 0, sizeof(layout));
    layout.size = sizeof(layout);
    layout.toolbar_height = 40;
    layout.right_sidebar_width = 200;
    check(ui_host_set_layout(host, &layout) == UI_STATUS_OK,
          "set logical layout");
    check(ui_host_resize(host, 800, 600) == UI_STATUS_OK,
          "resize logical host");

    memset(&surface_desc, 0, sizeof(surface_desc));
    surface_desc.size = sizeof(surface_desc);
    surface_desc.id = "dpi.bound";
    surface_desc.kind = UI_SURFACE_NATIVE;
    surface_desc.visible = 1;
    bound_surface = ui_surface_create(host, &surface_desc);
    check(bound_surface != NULL, "create bound native surface");
    if (bound_surface == NULL) {
        ui_host_destroy(host);
        DestroyWindow(window);
        return 1;
    }
    check(ui_surface_set_layout_region(bound_surface, UI_LAYOUT_REGION_MAIN) ==
              UI_STATUS_OK,
          "bind surface to main region");

    surface_desc.id = "dpi.manual";
    surface_desc.rect.x = 13;
    surface_desc.rect.y = 17;
    surface_desc.rect.width = 101;
    surface_desc.rect.height = 77;
    manual_surface = ui_surface_create(host, &surface_desc);
    check(manual_surface != NULL, "create manual native surface");
    if (manual_surface == NULL) {
        ui_host_destroy(host);
        DestroyWindow(window);
        return 1;
    }

    for (index = 0u; index < sizeof(dpis) / sizeof(dpis[0]); ++index) {
        int scale = (int)dpis[index];
        check(ui_host_set_dpi(host, dpis[index]) == UI_STATUS_OK,
              "set DPI");
        check(ui_host_get_dpi(host, &dpi) == UI_STATUS_OK &&
                  dpi == dpis[index],
              "get DPI");
        check(ui_host_get_rect(host, UI_LAYOUT_REGION_MAIN, &logical_rect) ==
                  UI_STATUS_OK && logical_rect.x == 0 && logical_rect.y == 40 &&
                  logical_rect.width == 600 && logical_rect.height == 560,
              "DPI changes preserve logical layout");
        check(ui_surface_get_pixel_rect(bound_surface, &pixel_rect) ==
                  UI_STATUS_OK && pixel_rect.x == 0 &&
                  pixel_rect.y == (40 * scale + 48) / 96 &&
                  pixel_rect.width == (600 * scale + 48) / 96 &&
                  pixel_rect.height == (560 * scale + 48) / 96,
              "bound surface DPI scale");
        check_child_rect(window,
                         (HWND)ui_surface_native_handle(bound_surface),
                         &pixel_rect);
        check(ui_surface_get_pixel_rect(manual_surface, &manual_pixel_rect) ==
                  UI_STATUS_OK &&
                  memcmp(&manual_pixel_rect, &manual_expected[index],
                         sizeof(manual_pixel_rect)) == 0,
              "manual surface rounded pixel edges");
        check_child_rect(window,
                         (HWND)ui_surface_native_handle(manual_surface),
                         &manual_pixel_rect);
    }

    /* Odd adjacent logical edges must conserve the root pixel width. */
    layout.left_sidebar_width = 261;
    layout.right_sidebar_width = 0;
    layout.narrow_policy = UI_NARROW_KEEP_SIDEBARS;
    check(ui_host_set_layout(host, &layout) == UI_STATUS_OK &&
          ui_host_resize(host, 800, 600) == UI_STATUS_OK &&
          ui_host_set_dpi(host, 144) == UI_STATUS_OK,
          "odd-width adjacent surface setup");
    check(ui_surface_get_pixel_rect(bound_surface, &pixel_rect) == UI_STATUS_OK &&
          pixel_rect.x == 392 && pixel_rect.width == 808 &&
          pixel_rect.x + pixel_rect.width == 1200,
          "144 DPI adjacent edges conserve root width");
    check_child_rect(window, (HWND)ui_surface_native_handle(bound_surface), &pixel_rect);
    check(ui_host_set_dpi(host, 192) == UI_STATUS_OK, "restore DPI for size conversion");

    result = -1;
    check(ui_host_handle_message(host, (void *)window, WM_SIZE,
                                  SIZE_RESTORED, (intptr_t)MAKELPARAM(1600, 1200),
                                  &result) == UI_STATUS_OK && result == 0,
          "WM_SIZE forwarding");
    check(ui_host_get_rect(host, UI_LAYOUT_REGION_ROOT, &logical_rect) ==
              UI_STATUS_OK && logical_rect.width == 800 &&
              logical_rect.height == 600,
          "WM_SIZE converts physical dimensions to logical");
    check(ui_host_handle_message(host, (void *)window, WM_DPICHANGED,
                                  (uintptr_t)MAKELONG(144, 144), 0,
                                  &result) == UI_STATUS_OK,
          "WM_DPICHANGED forwarding");
    check(ui_host_get_dpi(host, &dpi) == UI_STATUS_OK && dpi == 144u,
          "WM_DPICHANGED updates DPI");
    check(ui_host_handle_message(host, (void *)window, WM_SIZE,
                                  SIZE_MINIMIZED, 0, &result) == UI_STATUS_OK,
          "minimize message");
    check(ui_host_get_rect(host, UI_LAYOUT_REGION_ROOT, &logical_rect) ==
              UI_STATUS_OK && logical_rect.width == 0 && logical_rect.height == 0,
          "minimized root has zero dimensions");
    check(ui_host_handle_message(host, (void *)window, WM_SIZE,
                                  SIZE_RESTORED, (intptr_t)MAKELPARAM(1200, 900),
                                  &result) == UI_STATUS_OK,
          "restore message");
    check(ui_host_get_rect(host, UI_LAYOUT_REGION_ROOT, &logical_rect) ==
              UI_STATUS_OK && logical_rect.width == 800 &&
              logical_rect.height == 600,
          "restored dimensions remain logical");
    check(ui_host_handle_message(host, (void *)window, WM_USER, 0, 0,
                                  &result) == UI_STATUS_NOT_FOUND,
          "unhandled native message");
    check(ui_host_set_dpi(host, 0u) == UI_STATUS_INVALID_ARGUMENT,
          "invalid zero DPI rejected");

    ui_host_destroy(host);
    DestroyWindow(window);
    (void)UnregisterClassA(window_class.lpszClassName, window_class.hInstance);
    printf("DPI integration: %d failure(s)\n", failures);
    return failures != 0;
}
