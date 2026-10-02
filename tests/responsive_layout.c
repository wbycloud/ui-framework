#include "ui_framework/ui.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>

static int failures, events;
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x); ++failures; } } while (0)
static void event(const char *id, const char *json, void *data)
{
    (void)json; (void)data;
    if (strcmp(id, "ui.host.layout_changed") == 0) ++events;
}
static ui_narrow_window_policy_t policy(ui_host_t *h, int w, int y,
    uint32_t d, ui_narrow_window_policy_t p, void *data)
{
    (void)h; (void)w; (void)y; (void)d; (void)p;
    return *(ui_narrow_window_policy_t *)data;
}
int main(void)
{
    static const int sizes[][2] = {{1920,1080},{1280,720},{800,600},{640,480}};
    static const uint32_t dpis[] = {96,144,192};
    ui_host_config_t config = {0};
    ui_layout_desc_t layout = {0};
    ui_host_t *host;
    ui_rect_t main_rect, side, top, bottom;
    ui_sidebar_state_t state;
    ui_narrow_window_policy_t chosen;
    int min_width, min_height;
    size_t i, j;
    config.size = sizeof(config); config.api_version = UI_FRAMEWORK_API_VERSION;
    config.event_callback = event;
    host = ui_host_create(&config); CHECK(host != NULL); if (!host) return 1;
    layout.size = sizeof(layout); layout.toolbar_height = 36; layout.status_bar_height = 24;
    layout.right_sidebar_preferred_width = 320; layout.right_sidebar_min_width = 260;
    layout.right_sidebar_max_width = 360; layout.main_min_width = 400;
    layout.collapsed_tag_width = 32;
    CHECK(ui_host_set_layout(host, &layout) == UI_STATUS_OK);
    for (i = 0; i < sizeof(sizes)/sizeof(sizes[0]); ++i) {
        for (j = 0; j < sizeof(dpis)/sizeof(dpis[0]); ++j) {
            CHECK(ui_host_set_dpi(host, dpis[j]) == UI_STATUS_OK);
            CHECK(ui_host_resize(host, sizes[i][0], sizes[i][1]) == UI_STATUS_OK);
            CHECK(ui_host_get_rect(host, UI_LAYOUT_REGION_MAIN, &main_rect) == UI_STATUS_OK);
            CHECK(ui_host_get_rect(host, UI_LAYOUT_REGION_RIGHT_SIDEBAR, &side) == UI_STATUS_OK);
            CHECK(ui_host_get_rect(host, UI_LAYOUT_REGION_TOOLBAR, &top) == UI_STATUS_OK);
            CHECK(ui_host_get_rect(host, UI_LAYOUT_REGION_STATUS_BAR, &bottom) == UI_STATUS_OK);
            CHECK(main_rect.x == 0 && main_rect.y == 36);
            CHECK(main_rect.width + side.width == sizes[i][0]);
            CHECK(main_rect.height == sizes[i][1] - 60);
            CHECK(side.x == main_rect.width && side.y == main_rect.y);
            CHECK(top.height == 36 && bottom.y == sizes[i][1] - 24);
            CHECK(ui_host_get_sidebar_state(host, UI_LAYOUT_REGION_RIGHT_SIDEBAR, &state) == UI_STATUS_OK);
            CHECK(sizes[i][0] == 640 ? (state == UI_SIDEBAR_COLLAPSED && side.width == 32) :
                                      (state == UI_SIDEBAR_VISIBLE && side.width == 320));
        }
    }
    chosen = UI_NARROW_FLOAT_SIDEBARS;
    CHECK(ui_host_set_narrow_policy_callback(host, policy, &chosen) == UI_STATUS_OK);
    CHECK(ui_host_get_sidebar_state(host, UI_LAYOUT_REGION_RIGHT_SIDEBAR, &state) == UI_STATUS_OK && state == UI_SIDEBAR_FLOATING);
    chosen = UI_NARROW_KEEP_SIDEBARS;
    CHECK(ui_host_resize(host, 640, 480) == UI_STATUS_OK);
    CHECK(ui_host_get_rect(host, UI_LAYOUT_REGION_MAIN, &main_rect) == UI_STATUS_OK && main_rect.width == 320);
    chosen = UI_NARROW_DISALLOW_SHRINK;
    CHECK(ui_host_get_min_size(host, &min_width, &min_height) == UI_STATUS_OK && min_width == 720 && min_height == 60);
    CHECK(ui_host_set_narrow_policy_callback(host, NULL, NULL) == UI_STATUS_OK);
    layout.right_sidebar_no_collapse = 1;
    CHECK(ui_host_set_layout(host, &layout) == UI_STATUS_OK);
    CHECK(ui_host_get_rect(host, UI_LAYOUT_REGION_RIGHT_SIDEBAR, &side) == UI_STATUS_OK && side.width == 260);
    CHECK(ui_host_resize(host, 0, 0) == UI_STATUS_OK);
    CHECK(ui_host_get_rect(host, UI_LAYOUT_REGION_MAIN, &main_rect) == UI_STATUS_OK && main_rect.width == 0 && main_rect.height == 0);
    CHECK(ui_host_resize(host, -1, 480) == UI_STATUS_INVALID_ARGUMENT);
    CHECK(ui_host_set_dpi(host, 0) == UI_STATUS_INVALID_ARGUMENT);
    layout.right_sidebar_max_width = 100;
    CHECK(ui_host_set_layout(host, &layout) == UI_STATUS_INVALID_ARGUMENT);
    layout.right_sidebar_max_width = 360;
    /* Original six-field layout descriptor remains accepted. */
    layout.size = 6u * sizeof(int); layout.right_sidebar_width = 300;
    CHECK(ui_host_set_layout(host, &layout) == UI_STATUS_OK);
    CHECK(ui_host_resize(host, INT_MAX, INT_MAX) == UI_STATUS_OK);
    CHECK(ui_host_get_rect(host, UI_LAYOUT_REGION_MAIN, &main_rect) == UI_STATUS_OK && main_rect.width == INT_MAX - 300);
    CHECK(events > 12);
    ui_host_destroy(host);
    printf("responsive layout matrix: %d failures\n", failures);
    return failures != 0;
}
