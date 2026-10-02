#include "ui_framework/ui.h"
#include <stdio.h>
#include <string.h>

typedef struct mock { int fail, attempts; uint32_t dpi; } mock_t;
static ui_status_t create(void *data, ui_host_t *host, void **view)
{ (void)host; *view = data; return UI_STATUS_OK; }
static void destroy(void *data, void *view) { (void)data; (void)view; }
static ui_status_t resize(void *data, void *view, int w, int h, uint32_t dpi)
{
    mock_t *mock = data; (void)view; (void)w; (void)h; ++mock->attempts;
    if (mock->fail) return UI_STATUS_PLATFORM_ERROR;
    mock->dpi = dpi; return UI_STATUS_OK;
}
int main(void)
{
    mock_t mock = {0}; ui_host_config_t hc = {0}; ui_web_backend_ops_t ops = {0};
    ui_web_backend_desc_t desc = {0}; ui_host_t *host; ui_web_backend_t *backend;
    ui_web_view_t *view; uint32_t dpi; int failures = 0;
    hc.size = sizeof(hc); hc.api_version = UI_FRAMEWORK_API_VERSION;
    host = ui_host_create(&hc); if (!host) return 1;
    ops.size = sizeof(ops); ops.create_view = create; ops.destroy_view = destroy; ops.resize = resize;
    desc.size = sizeof(desc); desc.ops = &ops; desc.user_data = &mock;
    backend = ui_web_backend_create(&desc); view = ui_web_view_create(host, backend);
    if (!view || ui_web_view_resize(view, 800, 600, 96) != UI_STATUS_OK) return 1;
    mock.fail = 1;
    if (ui_host_set_dpi(host, 144) != UI_STATUS_PLATFORM_ERROR || mock.dpi != 96) ++failures;
    if (ui_host_get_dpi(host, &dpi) != UI_STATUS_OK || dpi != 144) ++failures;
    mock.fail = 0;
    if (ui_host_set_dpi(host, 144) != UI_STATUS_OK || mock.dpi != 144 || mock.attempts != 3) ++failures;
    ui_host_destroy(host); ui_web_backend_destroy(backend);
    printf("failed Web DPI sync and retry: %d failures\n", failures);
    return failures != 0;
}
