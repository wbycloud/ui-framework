#include "ui_framework/ui.h"
#include "ui_framework/native.h"
#include "ui_framework/opengl.h"
#include "ui_framework/assistant.h"
#include "ui_framework/light_web.h"
#include "ui_framework/webview2.h"
#include "ui_framework/components.h"
#include "ui_framework/images.h"
#include "ui_framework/menus.h"
#include <stddef.h>
_Static_assert(UI_FRAMEWORK_API_VERSION == 8, "current SDK");
_Static_assert(offsetof(ui_field_desc_t, help) == 64, "appended complete help field");
_Static_assert(sizeof(ui_field_desc_t) == 72, "Windows x64 field description");
_Static_assert(offsetof(ui_column_desc_t, width) == 28, "column width offset unchanged");
int main(void)
{
    ui_host_config_t config = {0};
    ui_host_t *host;
    config.size = sizeof(config);
    config.api_version = UI_FRAMEWORK_API_VERSION;
    host = ui_host_create(&config);
    if (!host) return 1;
    {
        ui_field_desc_t field = {0};ui_component_desc_t desc = {0};ui_component_t *form;
        field.size = 68;field.id = "partial";field.kind = UI_VALUE_TEXT;
        field.help = (const char *)(uintptr_t)1; /* Incomplete field must not be read. */
        desc.size = sizeof(desc);desc.id = "complete-fields";desc.kind = UI_COMPONENT_FORM;
        desc.fields = &field;desc.field_count = 1;
        if(ui_component_register(host,&desc,&form)!=UI_STATUS_OK)return 2;
    }
    ui_host_destroy(host);
    return 0;
}
