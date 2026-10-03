#include "ui_framework/light_web.h"

#include <windows.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures, g_clicks, g_inputs, g_loads;
static char g_last_params[4096];

#ifndef UI_WEB_PARITY_PAGE_PATH
#define UI_WEB_PARITY_PAGE_PATH "examples/web_common/assistant.html"
#endif

static char *read_parity_page(void)
{
    wchar_t path[2048];
    FILE *file;
    long length;
    char *content;
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
            UI_WEB_PARITY_PAGE_PATH, -1, path, 2048)) return NULL;
    if (_wfopen_s(&file, path, L"rb") != 0) return NULL;
    if (fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) < 0 ||
        fseek(file, 0, SEEK_SET) != 0) { fclose(file); return NULL; }
    content = (char *)malloc((size_t)length + 1);
    if (content && fread(content, 1, (size_t)length, file) == (size_t)length)
        content[length] = 0;
    else { free(content); content = NULL; }
    fclose(file); return content;
}

#define CHECK(condition) do { if (!(condition)) { \
    printf("FAIL line %d: %s\n", __LINE__, #condition); ++g_failures; \
} } while (0)

static void on_command(ui_host_t *host, uint64_t request, const char *id,
                       const char *params, const char *source, void *data)
{
    (void)data;
    CHECK(strcmp(source, "light-web") == 0);
    if (strcmp(id, "probe.clicked") == 0) ++g_clicks;
    if (strcmp(id, "probe.input") == 0) ++g_inputs;
    if (strcmp(id, "probe.loaded") == 0) ++g_loads;
    strcpy_s(g_last_params, sizeof(g_last_params), params);
    CHECK(ui_host_reply(host, request, 1, "{}") == UI_STATUS_OK);
}

static LRESULT CALLBACK parent_proc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp)
{
    return DefWindowProcW(hwnd, message, wp, lp);
}

static ui_status_t send_input(ui_web_view_t *view, ui_input_kind_t kind,
                               int x, int y, const char *text, int wheel)
{
    ui_input_event_t event = {0};
    event.size = sizeof(event); event.kind = kind; event.x = x; event.y = y;
    event.pointer_button = 1; event.text_utf8 = text; event.wheel_delta = wheel;
    return ui_web_view_dispatch_input(view, &event);
}

static void check_child_placement(HWND parent, HWND child, int x, int y,
                                    int width, int height)
{
    RECT actual;
    POINT origin;
    CHECK(GetWindowRect(child, &actual) != 0);
    origin.x = actual.left; origin.y = actual.top;
    CHECK(ScreenToClient(parent, &origin) != 0);
    CHECK(origin.x == x && origin.y == y);
    CHECK(actual.right - actual.left == width && actual.bottom - actual.top == height);
}

int main(void)
{
    const char *html =
        "<style>@media (max-width: 600px) { #assistant { display: none; } }</style>"
        "<div id='app' style='display:flex;flex-direction:row;width:100%;height:100%'>"
        "<div id='main' style='flex:1;background:#202020'>"
        "<button id='add' onclick='ui.invoke(\"probe.clicked\",{kind:\"block\"})'>Add</button>"
        "<input id='name' oninput='ui.invoke(\"probe.input\",{id:event.target.id,value:event.target.value,via:ui.value(\"name\")})'>"
        "</div><aside id='assistant' style='width:30%;background:#303840'>Assistant</aside></div>"
        "<script>ui.invoke('probe.loaded',{previous:typeof marker});var marker=1;</script>";
    WNDCLASSW wc = {0};
    HWND parent, child;
    ui_host_config_t hc = {0};
    ui_host_t *host;
    ui_command_desc_t command = {0};
    ui_light_web_config_t config = {0};
    ui_web_backend_t *backend;
    ui_web_view_t *view;
    ui_rect_t rect;
    ui_rect_t placement, bound;
    ui_layout_desc_t layout = {0};
    RECT pixels;

    ULONGLONG started;
    char *large;
    char *shared;
    const char *ids[] = {"probe.clicked", "probe.input", "probe.loaded"};
    int i;
    CHECK(ui_framework_initialize() == UI_STATUS_OK);
    wc.lpfnWndProc = parent_proc; wc.hInstance = GetModuleHandleW(NULL);
    wc.lpszClassName = L"LightProbeParent";
    RegisterClassW(&wc);
    parent = CreateWindowExW(0, wc.lpszClassName, L"light probe", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 900, 600, NULL, NULL, wc.hInstance, NULL);
    if (!parent) return 1;
    hc.size = sizeof(hc); hc.api_version = UI_FRAMEWORK_API_VERSION; hc.native_parent = parent;
    host = ui_host_create(&hc); if (!host) return 1;
    for (i = 0; i < 3; ++i) {
        command.size = sizeof(command); command.id = ids[i]; command.title = ids[i];
        command.handler = on_command;
        CHECK(ui_host_register_command(host, &command) == UI_STATUS_OK);
    }
    config.size = sizeof(config); config.parent_hwnd = parent; config.enable_native_input = 1;
    backend = ui_light_web_backend_create(&config); if (!backend) return 1;
    view = ui_web_view_create(host, backend); if (!view) return 1;
    shared = read_parity_page(); if (!shared) { printf("Cannot read shared parity page\n"); return 1; }
    CHECK(ui_web_view_load_html(view, shared) == UI_STATUS_OK); free(shared);
    CHECK(ui_web_view_resize(view, 800, 400, 96) == UI_STATUS_OK);
    CHECK(ui_web_view_get_element_rect(view, "assistant", &rect) == UI_STATUS_OK && rect.width == 240);
    CHECK(send_input(view, UI_INPUT_POINTER_DOWN, 20, 16, NULL, 0) == UI_STATUS_OK);
    CHECK(send_input(view, UI_INPUT_POINTER_UP, 20, 16, NULL, 0) == UI_STATUS_OK);
    CHECK(g_clicks == 1 && strcmp(g_last_params, "{}") == 0);
    CHECK(send_input(view, UI_INPUT_POINTER_DOWN, 20, 48, NULL, 0) == UI_STATUS_OK);
    CHECK(send_input(view, UI_INPUT_TEXT, 0, 0, "\xe4\xb8\xad\xe6\x96\x87\"\\\n", 0) == UI_STATUS_OK);
    CHECK(g_inputs == 1 && strstr(g_last_params, "\xe4\xb8\xad\xe6\x96\x87") != NULL);
    CHECK(ui_web_view_resize(view, 500, 400, 144) == UI_STATUS_OK);
    CHECK(ui_web_view_get_element_rect(view, "assistant", &rect) == UI_STATUS_OK && rect.width == 0);
    g_clicks = g_inputs = 0;
    CHECK(ui_web_view_load_html(view, html) == UI_STATUS_OK);
    CHECK(g_loads == 1 && strstr(g_last_params, "\"previous\":\"undefined\"") != NULL);
    CHECK(ui_web_view_resize(view, 800, 400, 96) == UI_STATUS_OK);
    CHECK(ui_web_view_get_element_rect(view, "assistant", &rect) == UI_STATUS_OK);
    CHECK(rect.x == 560 && rect.width == 240 && rect.height == 400);
    CHECK(ui_web_view_get_element_rect(view, "add", &rect) == UI_STATUS_OK);
    CHECK(rect.width == 560 && rect.height == 32);
    CHECK(send_input(view, UI_INPUT_POINTER_DOWN, 20, 16, NULL, 0) == UI_STATUS_OK);
    CHECK(send_input(view, UI_INPUT_POINTER_UP, 20, 16, NULL, 0) == UI_STATUS_OK);
    CHECK(g_clicks == 1 && strcmp(g_last_params, "{\"kind\":\"block\"}") == 0);
    CHECK(send_input(view, UI_INPUT_POINTER_DOWN, 20, 48, NULL, 0) == UI_STATUS_OK);
    CHECK(send_input(view, UI_INPUT_TEXT, 0, 0, "\xe4\xb8\xad\xe6\x96\x87\"\\\n", 0) == UI_STATUS_OK);
    CHECK(g_inputs == 1);
    CHECK(strstr(g_last_params, "\xe4\xb8\xad\xe6\x96\x87\\\"\\\\") != NULL);
    child = FindWindowExW(parent, NULL, L"UIFrameworkLightWeb", NULL);
    CHECK(child != NULL);
    CHECK(FindWindowExW(child,NULL,L"EDIT",NULL)==NULL);
    CHECK(send_input(view,UI_INPUT_TEXT,0,0,"测试",0)==UI_STATUS_OK);
    CHECK(g_inputs == 2 && strstr(g_last_params,"测试")!=NULL);
    CHECK(ui_web_view_resize(view, 500, 400, 144) == UI_STATUS_OK);
    CHECK(ui_web_view_get_element_rect(view, "assistant", &rect) == UI_STATUS_OK);
    CHECK(rect.width == 0 && rect.height == 0);
    GetClientRect(child, &pixels); CHECK(pixels.right == 750 && pixels.bottom == 600);
    placement.x = 120; placement.y = 40; placement.width = 500; placement.height = 400;
    CHECK(ui_web_view_set_rect(view, &placement, 144) == UI_STATUS_OK);
    CHECK(ui_web_view_get_rect(view, &rect) == UI_STATUS_OK && memcmp(&rect, &placement, sizeof(rect)) == 0);
    check_child_placement(parent, child, 180, 60, 750, 600);
    placement.x = 121; placement.y = 41; placement.width = 501; placement.height = 401;
    CHECK(ui_web_view_set_rect(view, &placement, 144) == UI_STATUS_OK);
    check_child_placement(parent, child, 182, 62, 751, 601);
    CHECK(ui_web_view_resize(view, 500, 400, 144) == UI_STATUS_OK);
    check_child_placement(parent, child, 182, 62, 750, 600);
    placement.x = placement.y = 0; placement.width = 800; placement.height = 400;
    CHECK(ui_web_view_set_rect(view, &placement, 192) == UI_STATUS_OK);
    CHECK(ui_web_view_resize(view, 800, 400, 192) == UI_STATUS_OK);
    GetClientRect(child, &pixels); CHECK(pixels.right == 1600 && pixels.bottom == 800);
    CHECK(ui_web_view_get_element_rect(view, "assistant", &rect) == UI_STATUS_OK && rect.width == 240);
    SendMessageW(child, WM_LBUTTONDOWN, 0, MAKELPARAM(40, 32));
    SendMessageW(child, WM_LBUTTONUP, 0, MAKELPARAM(40, 32)); CHECK(g_clicks == 2);
    CHECK(ui_web_view_load_html(view, html) == UI_STATUS_OK);
    CHECK(g_loads == 2 && strstr(g_last_params, "\"previous\":\"undefined\"") != NULL);
    CHECK(ui_web_view_load_html(view,
        "<div id='scroll' style='overflow-y:auto;height:100%;width:100%'>"
        "<div id='one' style='height:100px'>One</div><div style='height:100px'>Two</div>"
        "<div style='height:100px'>Three</div></div>") == UI_STATUS_OK);
    CHECK(ui_web_view_resize(view, 200, 100, 96) == UI_STATUS_OK);
    CHECK(send_input(view, UI_INPUT_WHEEL, 20, 20, NULL, -120) == UI_STATUS_OK);
    CHECK(ui_web_view_get_element_rect(view, "one", &rect) == UI_STATUS_OK && rect.y == -48);
    CHECK(ui_web_view_load_html(view, "<div id='assistant'>Visible after reload</div>") == UI_STATUS_OK);
    CHECK(ui_web_view_resize(view, 500, 100, 96) == UI_STATUS_OK);
    CHECK(ui_web_view_get_element_rect(view, "assistant", &rect) == UI_STATUS_OK && rect.width == 500);
    CHECK(ui_web_view_get_element_rect(view, "missing", &rect) == UI_STATUS_NOT_FOUND);
    CHECK(ui_web_view_load_html(view, "<canvas></canvas>") == UI_STATUS_UNSUPPORTED);
    CHECK(ui_web_view_load_html(view, "<div style='display:grid'>Bad</div>") == UI_STATUS_UNSUPPORTED);
    CHECK(ui_web_view_load_html(view, "<input type='number'>") == UI_STATUS_UNSUPPORTED);
    CHECK(ui_web_view_load_html(view, "<div class='ignored'>Bad</div>") == UI_STATUS_OK);
    CHECK(ui_web_view_load_html(view, "<script src='https://example.com/a.js'></script>") == UI_STATUS_UNSUPPORTED);
    started = GetTickCount64();
    CHECK(ui_web_view_load_html(view, "<div>Bad script</div><script>while(true){}</script>") == UI_STATUS_VALIDATION_FAILED);
    CHECK(GetTickCount64() - started < 1000);
    CHECK(ui_web_view_load_html(view, "<div>Memory limit</div><script>var huge='a'.repeat(16000000);</script>") == UI_STATUS_VALIDATION_FAILED);
    CHECK(ui_web_view_load_html(view, html) == UI_STATUS_OK);
    large = (char *)malloc(40000);
    if (!large) return 1;
    strcpy_s(large, 40000, "<div>");
    for (i = 0; i < 1024; ++i) strcat_s(large, 40000, "<span>item</span>");
    strcat_s(large, 40000, "</div>");
    CHECK(ui_web_view_load_html(view, large) == UI_STATUS_VALIDATION_FAILED); free(large);
    large = (char *)malloc(270000);
    if (!large) return 1;
    memset(large, 'x', 269999); large[269999] = 0;
    CHECK(ui_web_view_load_html(view, large) == UI_STATUS_VALIDATION_FAILED); free(large);
    CHECK(ui_web_view_resize(view, INT_MAX, 1, 96) == UI_STATUS_INVALID_ARGUMENT);
    CHECK(ui_web_view_load_html(view, html) == UI_STATUS_OK);
    layout.size = sizeof(layout); layout.toolbar_height = 40; layout.right_sidebar_width = 200;
    CHECK(ui_host_set_layout(host, &layout) == UI_STATUS_OK);
    CHECK(ui_host_resize(host, 800, 600) == UI_STATUS_OK);
    CHECK(ui_host_set_dpi(host, 144) == UI_STATUS_OK);
    CHECK(ui_web_view_set_layout_region(view, UI_LAYOUT_REGION_MAIN) == UI_STATUS_OK);
    CHECK(ui_host_get_rect(host, UI_LAYOUT_REGION_MAIN, &bound) == UI_STATUS_OK);
    CHECK(ui_web_view_get_rect(view, &rect) == UI_STATUS_OK && memcmp(&rect, &bound, sizeof(rect)) == 0);
    check_child_placement(parent, child, 0, 60, 900, 840);
    CHECK(ui_host_resize(host, 1280, 720) == UI_STATUS_OK);
    CHECK(ui_host_get_rect(host, UI_LAYOUT_REGION_MAIN, &bound) == UI_STATUS_OK);
    CHECK(ui_web_view_get_rect(view, &rect) == UI_STATUS_OK && memcmp(&rect, &bound, sizeof(rect)) == 0);
    check_child_placement(parent, child, 0, 60, 1620, 1020);
    CHECK(ui_host_resize(host, 900, 600) == UI_STATUS_OK);
    CHECK(ui_web_view_set_layout_region(view, UI_LAYOUT_REGION_RIGHT_SIDEBAR) == UI_STATUS_OK);
    CHECK(ui_host_get_rect(host, UI_LAYOUT_REGION_RIGHT_SIDEBAR, &bound) == UI_STATUS_OK);
    CHECK(ui_web_view_get_rect(view, &rect) == UI_STATUS_OK && memcmp(&rect, &bound, sizeof(rect)) == 0);
    check_child_placement(parent, child, 1050, 60, 300, 840);
    i = g_clicks;
    CHECK(send_input(view, UI_INPUT_POINTER_DOWN, 20, 16, NULL, 0) == UI_STATUS_OK);
    CHECK(send_input(view, UI_INPUT_POINTER_UP, 20, 16, NULL, 0) == UI_STATUS_OK);
    CHECK(g_clicks == i + 1);
    CHECK(ui_host_set_dpi(host, 192) == UI_STATUS_OK);
    check_child_placement(parent, child, 1400, 80, 400, 1120);
    ui_web_view_destroy(view); ui_light_web_backend_destroy(backend); ui_host_destroy(host);
    DestroyWindow(parent);
    printf("Light Web probe: %d failure(s), clicks=%d, inputs=%d, reloads=%d\n", g_failures, g_clicks, g_inputs, g_loads);
    return g_failures ? 1 : 0;
}
