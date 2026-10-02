#include "ui_framework/ui.h"
#include "ui_framework/native.h"
#include "ui_framework/opengl.h"
#include "ui_framework/assistant.h"
#include "ui_framework/light_web.h"
#include "ui_framework/webview2.h"
int main(void)
{
    ui_host_config_t config = {0};
    ui_host_t *host;
    config.size = sizeof(config);
    config.api_version = UI_FRAMEWORK_API_VERSION;
    host = ui_host_create(&config);
    if (!host) return 1;
    ui_host_destroy(host);
    return 0;
}
