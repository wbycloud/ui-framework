#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "ui_framework/webview2.h"

static int echoed;
static void receive(ui_web_view_t *view, const char *json, void *data)
{
    (void)view; (void)data;
    if (!strcmp(json, "{\"echo\":\"中文\\\"\\\\\\n<safe>\"}")) echoed = 1;
}

int main(void)
{
    HWND root;
    ui_host_config_t hc = {0};
    ui_webview2_backend_config_t config = {0};
    ui_host_t *host;
    ui_web_backend_t *backend;
    ui_web_view_t *view;
    ui_status_t status;
    uint64_t capabilities = 0;
    ULONGLONG start;
    MSG message;
    int ready = 0, navigated = 0, posted = 0;
    if (ui_webview2_runtime_status() == UI_STATUS_UNSUPPORTED) return 77;
    if (ui_framework_initialize() != UI_STATUS_OK) return 1;
    root = CreateWindowW(L"STATIC", L"WebView2 JSON", WS_OVERLAPPEDWINDOW,
        0, 0, 800, 600, NULL, NULL, GetModuleHandleW(NULL), NULL);
    if (root == NULL) return 1;
    hc.size = sizeof(hc); hc.api_version=UI_FRAMEWORK_API_VERSION; hc.native_parent = root;
    host = ui_host_create(&hc); if (host == NULL) return 1;
    config.size = sizeof(config);
    backend = ui_webview2_backend_create(&config, &status); if (backend == NULL) return 1;
    view = ui_web_view_create(host, backend); if (view == NULL) return 1;
    if (ui_web_view_get_capabilities(view, &capabilities) != UI_STATUS_OK ||
        !(capabilities & UI_WEB_CAP_JSON_MESSAGES) ||
        (capabilities & UI_WEB_CAP_NATIVE_WINDOW) || ui_web_view_native_handle(view) != NULL)
        return 1;
    if (ui_web_view_set_message_callback(view, receive, NULL) != UI_STATUS_OK ||
        ui_web_view_resize(view, 800, 600, 96) != UI_STATUS_OK ||
        ui_web_view_load_html(view, "<script>ui.onmessage=function(data){ui.postMessage({echo:data.text});};</script>") != UI_STATUS_OK)
        return 1;
    start = GetTickCount64();
    while (!echoed && GetTickCount64() - start < 15000) {
        while (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message); DispatchMessageW(&message);
        }
        status = ui_webview2_view_get_state(view, &ready, &navigated);
        if (status != UI_STATUS_OK) break;
        if (ready && navigated && !posted) {
            status = ui_web_view_post_json(view, "{\"text\":\"中文\\\"\\\\\\n<safe>\"}");
            if (status != UI_STATUS_OK) break;
            posted = 1;
        }
        Sleep(1);
    }
    ui_web_view_destroy(view); ui_webview2_backend_destroy(backend);
    ui_host_destroy(host); DestroyWindow(root);
    printf("WebView2 JSON: ready=%d navigated=%d posted=%d echoed=%d status=%d\n",
        ready, navigated, posted, echoed, status);
    return echoed ? 0 : 1;
}
