#include <stdio.h>
#include <string.h>

#include "ui_framework/ui.h"

static void on_result(const ui_result_t *result, void *user_data)
{
    int *reply_count = (int *)user_data;
    *reply_count += 1;
    printf("%s: %s\n", result->command_id, result->result_json);
}

static void add_block(ui_host_t *host,
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
    (void)ui_host_reply(host, request_id, 1, "{\"blocks\":1}");
}

int main(void)
{
    int reply_count = 0;
    ui_host_config_t config;
    ui_command_desc_t command;
    ui_toolbar_desc_t toolbar;
    ui_toolbar_item_desc_t toolbar_item;
    ui_layout_desc_t layout;
    ui_rect_t main_rect;
    ui_rect_t right_rect;
    ui_sidebar_state_t right_state;
    ui_host_t *host;

    memset(&config, 0, sizeof(config));
    config.size = sizeof(config);
    config.api_version = UI_FRAMEWORK_API_VERSION;
    config.native_parent = NULL;
    config.user_data = &reply_count;
    config.result_callback = on_result;

    host = ui_host_create(&config);
    if (host == NULL) {
        return 1;
    }

    memset(&command, 0, sizeof(command));
    command.size = sizeof(command);
    command.id = "eda.add_block";
    command.title = "Add block";
    command.params_schema_json = "{}";
    command.handler = add_block;
    if (ui_host_register_command(host, &command) != UI_STATUS_OK) {
        ui_host_destroy(host);
        return 1;
    }

    memset(&toolbar, 0, sizeof(toolbar));
    toolbar.size = sizeof(toolbar);
    toolbar.id = "eda.main_toolbar";
    toolbar.title = "EDA";
    toolbar.visible = 1;
    if (ui_host_register_toolbar(host, &toolbar) != UI_STATUS_OK) {
        ui_host_destroy(host);
        return 1;
    }

    memset(&toolbar_item, 0, sizeof(toolbar_item));
    toolbar_item.size = sizeof(toolbar_item);
    toolbar_item.id = "eda.toolbar.add_block";
    toolbar_item.toolbar_id = "eda.main_toolbar";
    toolbar_item.title = "Add block";
    toolbar_item.command_id = "eda.add_block";
    if (ui_host_register_toolbar_item(host, &toolbar_item) != UI_STATUS_OK) {
        ui_host_destroy(host);
        return 1;
    }

    memset(&layout, 0, sizeof(layout));
    layout.size = sizeof(layout);
    layout.toolbar_height = 40;
    layout.right_sidebar_width = 200;
    if (ui_host_set_layout(host, &layout) != UI_STATUS_OK ||
        ui_host_resize(host, 800, 600) != UI_STATUS_OK ||
        ui_host_get_rect(host, UI_LAYOUT_REGION_MAIN, &main_rect) !=
            UI_STATUS_OK ||
        main_rect.x != 0 || main_rect.y != 40 ||
        main_rect.width != 600 || main_rect.height != 560) {
        ui_host_destroy(host);
        return 1;
    }

    /* The appended responsive fields collapse the right rail at narrow sizes. */
    layout.right_sidebar_min_width = 160;
    layout.right_sidebar_preferred_width = 240;
    layout.right_sidebar_max_width = 360;
    layout.narrow_policy = UI_NARROW_MAIN_PRIORITY;
    if (ui_host_set_layout(host, &layout) != UI_STATUS_OK ||
        ui_host_resize(host, 200, 600) != UI_STATUS_OK ||
        ui_host_get_rect(host, UI_LAYOUT_REGION_RIGHT_SIDEBAR, &right_rect) !=
            UI_STATUS_OK || right_rect.width != 0 ||
        ui_host_get_sidebar_state(host, UI_LAYOUT_REGION_RIGHT_SIDEBAR,
                                  &right_state) != UI_STATUS_OK ||
        right_state != UI_SIDEBAR_COLLAPSED) {
        ui_host_destroy(host);
        return 1;
    }

    if (ui_host_invoke(host, "eda.add_block", "{}", "headless") == 0u ||
        reply_count != 1) {
        ui_host_destroy(host);
        return 1;
    }

    ui_host_destroy(host);
    return 0;
}
