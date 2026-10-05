#include "ui_internal.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *ui_strdup(const char *value)
{
    size_t length;
    char *copy;

    if (value == NULL) {
        value = "";
    }

    length = strlen(value);
    copy = (char *)malloc(length + 1u);
    if (copy == NULL) {
        return NULL;
    }

    memcpy(copy, value, length + 1u);
    return copy;
}

static int valid_size(uint32_t size, size_t expected)
{
    return size >= expected;
}

static int valid_web_input_event(const ui_input_event_t *event)
{
    return event != NULL && valid_size(event->size, sizeof(*event)) &&
           event->kind >= UI_INPUT_POINTER_MOVE && event->kind <= UI_INPUT_CANCEL;
}

static int scale_coordinate(int64_t value, uint32_t dpi)
{
    int64_t scaled;

    if (dpi == 0u) {
        dpi = 96u;
    }

    /* Split before multiplying to avoid overflow for an endpoint x+width. */
    scaled = value < 0 ? -value : value;
    scaled = (scaled / 96) * dpi + ((scaled % 96) * dpi + 48) / 96;
    if (value < 0) scaled = -scaled;
    if (scaled > INT_MAX) {
        return INT_MAX;
    }
    if (scaled < INT_MIN) {
        return INT_MIN;
    }
    return (int)scaled;
}

static ui_rect_t logical_to_pixel_rect(const ui_rect_t *rect, uint32_t dpi)
{
    ui_rect_t result;

    result.x = scale_coordinate(rect->x, dpi);
    result.y = scale_coordinate(rect->y, dpi);
    {
        int64_t width = (int64_t)scale_coordinate((int64_t)rect->x + rect->width, dpi) - result.x;
        int64_t height = (int64_t)scale_coordinate((int64_t)rect->y + rect->height, dpi) - result.y;
        result.width = width > INT_MAX ? INT_MAX : (int)width;
        result.height = height > INT_MAX ? INT_MAX : (int)height;
    }
    return result;
}

int ui_layout_region_valid(ui_layout_region_t region)
{
    return region > UI_LAYOUT_REGION_NONE &&
           (region < UI_LAYOUT_REGION_COUNT || region == UI_LAYOUT_REGION_BOTTOM);
}

static int clamp_dimension(int value, int maximum)
{
    if (value < 0) {
        return 0;
    }
    if (value > maximum) {
        return maximum;
    }
    return value;
}

static int layout_width(int legacy,
                        int minimum,
                        int preferred,
                        int maximum,
                        int available)
{
    int width = preferred > 0 ? preferred : legacy;

    if (width < minimum) {
        width = minimum;
    }
    if (maximum > 0 && width > maximum) {
        width = maximum;
    }
    return clamp_dimension(width, available);
}

static int responsive_main_width(const ui_layout_desc_t *layout)
{
    return layout->main_min_width > 0 ? layout->main_min_width : 400;
}

static int fit_sidebar(int *width, int minimum, int shortage)
{
    int reduction = *width > minimum ? *width - minimum : 0;
    if (reduction > shortage) reduction = shortage;
    *width -= reduction;
    return shortage - reduction;
}

ui_status_t ui_layout_recompute(ui_host_t *host)
{
    ui_surface_t *surface;
    ui_web_view_t *view;
    ui_rect_t root;
    int menu_height;
    int toolbar_height;
    int status_height;
    int left_width;
    int right_width;
    int content_top;
    int content_bottom;
    int content_height;
    int content_width;
    int shortage;
    int main_minimum;
    ui_narrow_window_policy_t policy;

    if (host == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }

    root = host->host_rect;
    menu_height = clamp_dimension(host->layout.menu_bar_height, root.height);
    toolbar_height = clamp_dimension(host->layout.toolbar_height,
                                     root.height - menu_height);
    status_height = clamp_dimension(host->layout.status_bar_height,
                                    root.height - menu_height - toolbar_height);
    content_top = menu_height + toolbar_height;
    content_bottom = root.height - status_height;
    content_height = content_bottom > content_top
                         ? content_bottom - content_top
                         : 0;
    left_width = layout_width(host->layout.left_sidebar_width,
                              host->layout.left_sidebar_min_width,
                              host->layout.left_sidebar_preferred_width,
                              host->layout.left_sidebar_max_width,
                              INT_MAX);
    right_width = layout_width(host->layout.right_sidebar_width,
                               host->layout.right_sidebar_min_width,
                               host->layout.right_sidebar_preferred_width,
                               host->layout.right_sidebar_max_width,
                               INT_MAX);

    host->sidebar_states[0] = UI_SIDEBAR_VISIBLE;
    host->sidebar_states[1] = UI_SIDEBAR_VISIBLE;
    policy = host->layout.narrow_policy;
    if (host->narrow_callback != NULL) {
        ui_dispatch_enter(host);
        policy = host->narrow_callback(host, root.width, root.height,
                                       host->dpi, policy,
                                       host->narrow_user_data);
        ui_dispatch_leave(host);
        if (policy < UI_NARROW_MAIN_PRIORITY ||
            policy > UI_NARROW_DISALLOW_SHRINK) {
            return UI_STATUS_INVALID_ARGUMENT;
        }
    }
    main_minimum = responsive_main_width(&host->layout);
    /* Compute in 64 bits so extreme descriptor sizes cannot overflow. */
    {
        int64_t needed = (int64_t)left_width + right_width + main_minimum - root.width;
        shortage = needed > INT_MAX ? INT_MAX : needed > 0 ? (int)needed : 0;
    }
    if (policy != UI_NARROW_KEEP_SIDEBARS &&
        policy != UI_NARROW_DISALLOW_SHRINK && shortage > 0) {
        shortage = fit_sidebar(&right_width,
                                host->layout.right_sidebar_min_width > 0 ?
                                    host->layout.right_sidebar_min_width : right_width,
                                shortage);
        shortage = fit_sidebar(&left_width,
                                host->layout.left_sidebar_min_width > 0 ?
                                    host->layout.left_sidebar_min_width : left_width,
                                shortage);
        if (shortage > 0 && right_width > 0 &&
            policy != UI_NARROW_COLLAPSE_LEFT &&
            !host->layout.right_sidebar_no_collapse) {
            right_width = policy == UI_NARROW_FLOAT_SIDEBARS ? 0 :
                clamp_dimension(host->layout.collapsed_tag_width, right_width);
            host->sidebar_states[1] = policy == UI_NARROW_FLOAT_SIDEBARS ?
                UI_SIDEBAR_FLOATING : UI_SIDEBAR_COLLAPSED;
        }
        if ((int64_t)left_width + right_width + main_minimum > root.width &&
            left_width > 0 && policy != UI_NARROW_COLLAPSE_RIGHT &&
            !host->layout.left_sidebar_no_collapse) {
            left_width = policy == UI_NARROW_FLOAT_SIDEBARS ? 0 :
                clamp_dimension(host->layout.collapsed_tag_width, left_width);
            host->sidebar_states[0] = policy == UI_NARROW_FLOAT_SIDEBARS ?
                UI_SIDEBAR_FLOATING : UI_SIDEBAR_COLLAPSED;
        }
    }
    left_width = clamp_dimension(left_width, root.width);
    right_width = clamp_dimension(right_width, root.width - left_width);
    content_width = root.width - left_width - right_width;

    host->layout_rects[UI_LAYOUT_REGION_ROOT] = root;
    host->layout_rects[UI_LAYOUT_REGION_MENU_BAR].x = 0;
    host->layout_rects[UI_LAYOUT_REGION_MENU_BAR].y = 0;
    host->layout_rects[UI_LAYOUT_REGION_MENU_BAR].width = root.width;
    host->layout_rects[UI_LAYOUT_REGION_MENU_BAR].height = menu_height;

    host->layout_rects[UI_LAYOUT_REGION_TOOLBAR].x = 0;
    host->layout_rects[UI_LAYOUT_REGION_TOOLBAR].y = menu_height;
    host->layout_rects[UI_LAYOUT_REGION_TOOLBAR].width = root.width;
    host->layout_rects[UI_LAYOUT_REGION_TOOLBAR].height = toolbar_height;

    host->layout_rects[UI_LAYOUT_REGION_LEFT_SIDEBAR].x = 0;
    host->layout_rects[UI_LAYOUT_REGION_LEFT_SIDEBAR].y = content_top;
    host->layout_rects[UI_LAYOUT_REGION_LEFT_SIDEBAR].width = left_width;
    host->layout_rects[UI_LAYOUT_REGION_LEFT_SIDEBAR].height = content_height;

    host->layout_rects[UI_LAYOUT_REGION_MAIN].x = left_width;
    host->layout_rects[UI_LAYOUT_REGION_MAIN].y = content_top;
    host->layout_rects[UI_LAYOUT_REGION_MAIN].width = content_width;
    host->bottom_rect=(ui_rect_t){left_width,content_bottom,content_width,0};
    if(host->bottom_height>0){int height=clamp_dimension(host->bottom_height,content_height>120?content_height-120:content_height/2);host->bottom_rect.y-=height;host->bottom_rect.height=height;}
    host->layout_rects[UI_LAYOUT_REGION_MAIN].height = content_height-host->bottom_rect.height;

    host->layout_rects[UI_LAYOUT_REGION_RIGHT_SIDEBAR].x =
        left_width + content_width;
    host->layout_rects[UI_LAYOUT_REGION_RIGHT_SIDEBAR].y = content_top;
    host->layout_rects[UI_LAYOUT_REGION_RIGHT_SIDEBAR].width = right_width;
    host->layout_rects[UI_LAYOUT_REGION_RIGHT_SIDEBAR].height = content_height;

    host->layout_rects[UI_LAYOUT_REGION_STATUS_BAR].x = 0;
    host->layout_rects[UI_LAYOUT_REGION_STATUS_BAR].y = content_bottom;
    host->layout_rects[UI_LAYOUT_REGION_STATUS_BAR].width = root.width;
    host->layout_rects[UI_LAYOUT_REGION_STATUS_BAR].height = status_height;

    for (surface = host->surfaces; surface != NULL; surface = surface->next) {
        if (ui_shell_surface_bound(host, surface)) continue;
        const ui_rect_t *target_rect =
            surface->layout_region != UI_LAYOUT_REGION_NONE
                ? (surface->layout_region==UI_LAYOUT_REGION_BOTTOM?&host->bottom_rect:&host->layout_rects[surface->layout_region])
                : &surface->rect;

        if (ui_surface_set_rect(surface, target_rect) != UI_STATUS_OK) {
            return UI_STATUS_PLATFORM_ERROR;
        }
    }

    for (view = host->web_views; view != NULL; view = view->host_next) {
        if (ui_shell_web_view_bound(host, view)) continue;
        if (view->layout_region != UI_LAYOUT_REGION_NONE) {
            ui_status_t status = ui_web_view_set_rect(view,
                (view->layout_region==UI_LAYOUT_REGION_BOTTOM?&host->bottom_rect:&host->layout_rects[view->layout_region]), host->dpi);
            if (status != UI_STATUS_OK) return status;
        }
    }

    ui_shell_layout_changed(host);
    if (host->event_callback != NULL) {
        char payload[160];
        (void)snprintf(payload, sizeof(payload),
            "{\"width\":%d,\"height\":%d,\"dpi\":%u,\"left_state\":%d,\"right_state\":%d}",
            root.width, root.height, host->dpi,
            (int)host->sidebar_states[0], (int)host->sidebar_states[1]);
        host->event_callback("ui.host.layout_changed", payload, host->user_data);
    }

    return UI_STATUS_OK;
}

static ui_command_entry_t *find_command(ui_host_t *host, const char *id)
{
    ui_command_entry_t *entry;

    for (entry = host->commands; entry != NULL; entry = entry->next) {
        if (strcmp(entry->id, id) == 0) {
            return entry;
        }
    }

    return NULL;
}

static ui_toolbar_entry_t *find_toolbar(ui_host_t *host, const char *id)
{
    ui_toolbar_entry_t *entry;

    for (entry = host->toolbars; entry != NULL; entry = entry->next) {
        if (strcmp(entry->id, id) == 0) {
            return entry;
        }
    }

    return NULL;
}

static ui_pending_request_t *find_pending(ui_host_t *host,
                                          uint64_t request_id,
                                          ui_pending_request_t **previous)
{
    ui_pending_request_t *entry;
    ui_pending_request_t *last = NULL;

    for (entry = host->pending; entry != NULL; entry = entry->next) {
        if (entry->id == request_id) {
            if (previous != NULL) {
                *previous = last;
            }
            return entry;
        }
        last = entry;
    }

    if (previous != NULL) {
        *previous = NULL;
    }
    return NULL;
}

ui_status_t ui_framework_initialize(void)
{
    return ui_platform_prepare_dpi();
}

void ui_dispatch_enter(ui_host_t *host) { ++host->dispatch_depth; }
void ui_dispatch_leave(ui_host_t *host)
{
    --host->dispatch_depth;
    if (!host->dispatch_depth && host->dispatch_idle)
        host->dispatch_idle(host->dispatch_idle_data);
}

ui_status_t ui_host_get_run_mode(const ui_host_t *host,ui_run_mode_t *mode)
{if(!host||!mode)return UI_STATUS_INVALID_ARGUMENT;*mode=host->run_mode;return UI_STATUS_OK;}

int ui_framework_supports_api(uint32_t api_version)
{
    return api_version >= 1u && api_version <= UI_FRAMEWORK_API_VERSION;
}

ui_host_t *ui_host_create(const ui_host_config_t *config)
{
    ui_host_t *host;

    if (config == NULL ||
        !valid_size(config->size, sizeof(*config)) ||
        !ui_framework_supports_api(config->api_version)) {
        return NULL;
    }

    if (ui_platform_prepare_dpi() != UI_STATUS_OK) return NULL;
    host = (ui_host_t *)calloc(1u, sizeof(*host));
    if (host == NULL) {
        return NULL;
    }

    host->native_parent = config->native_parent;
    host->api_version = config->api_version;
    host->user_data = config->user_data;
    host->result_callback = config->result_callback;
    host->event_callback = config->event_callback;
    host->next_request_id = 1u;
    host->image_limit = 32u * 1024u * 1024u;
    host->app_active = 1;
    host->dpi = ui_platform_get_dpi(config->native_parent);
    if (host->dpi == 0u) {
        host->dpi = 96u;
    }
    host->layout.size = (uint32_t)sizeof(host->layout);
    return host;
}

static void free_commands(ui_command_entry_t *entry)
{
    while (entry != NULL) {
        ui_command_entry_t *next = entry->next;
        free(entry->id);
        free(entry->title);
        free(entry->params_schema_json);
        free(entry);
        entry = next;
    }
}

static void free_menus(ui_menu_entry_t *entry)
{
    while (entry != NULL) {
        ui_menu_entry_t *next = entry->next;
        free(entry->id);
        free(entry->menu_path);
        free(entry->title);
        free(entry->command_id);
        free(entry);
        entry = next;
    }
}

static void free_toolbars(ui_toolbar_entry_t *entry)
{
    while (entry != NULL) {
        ui_toolbar_entry_t *next = entry->next;
        free(entry->id);
        free(entry->title);
        free(entry);
        entry = next;
    }
}

static void free_toolbar_items(ui_toolbar_item_entry_t *entry)
{
    while (entry != NULL) {
        ui_toolbar_item_entry_t *next = entry->next;
        free(entry->id);
        free(entry->toolbar_id);
        free(entry->title);
        free(entry->command_id);
        free(entry->icon_url);
        free(entry);
        entry = next;
    }
}

static void free_panels(ui_panel_entry_t *entry)
{
    while (entry != NULL) {
        ui_panel_entry_t *next = entry->next;
        free(entry->id);
        free(entry->title);
        free(entry->entry_url);
        free(entry);
        entry = next;
    }
}

static void free_pending(ui_pending_request_t *entry)
{
    while (entry != NULL) {
        ui_pending_request_t *next = entry->next;
        free(entry->command_id);
        free(entry->source);
        free(entry);
        entry = next;
    }
}

void ui_host_destroy(ui_host_t *host)
{
    if (host == NULL) {
        return;
    }

    ui_menus_destroy(host);
    ui_components_destroy(host);
    while (host->surfaces != NULL) {
        ui_surface_destroy(host->surfaces);
    }

    while (host->web_views != NULL) {
        ui_web_view_destroy(host->web_views);
    }

    ui_images_destroy(host);
    free_commands(host->commands);
    free_menus(host->menus);
    free_toolbar_items(host->toolbar_items);
    free_toolbars(host->toolbars);
    free_panels(host->panels);
    free_pending(host->pending);
    free(host);
}

ui_status_t ui_host_resize(ui_host_t *host, int width, int height)
{
    if (host == NULL || width < 0 || height < 0) {
        return UI_STATUS_INVALID_ARGUMENT;
    }

    if(host->host_rect.width!=width||host->host_rect.height!=height)(void)ui_host_close_menu(host);
    host->host_rect.x = 0;
    host->host_rect.y = 0;
    host->host_rect.width = width;
    host->host_rect.height = height;
    return ui_layout_recompute(host);
}

ui_status_t ui_host_set_dpi(ui_host_t *host, uint32_t dpi)
{
    char payload[64];
    ui_status_t status;
    ui_web_view_t *view;

    if (host == NULL || dpi == 0u) {
        return UI_STATUS_INVALID_ARGUMENT;
    }

    status = UI_STATUS_OK;
    if (host->dpi != dpi) {
        (void)ui_host_close_menu(host);
        host->dpi = dpi;
        status = ui_layout_recompute(host);
    }

    for (view = host->web_views; view != NULL; view = view->host_next) {
        if (ui_shell_web_view_bound(host, view)) continue;
        if (view->dpi != dpi) {
            const ui_rect_t *target = view->layout_region == UI_LAYOUT_REGION_NONE ?
                &view->rect : (view->layout_region==UI_LAYOUT_REGION_BOTTOM?&host->bottom_rect:&host->layout_rects[view->layout_region]);
            ui_status_t view_status = ui_web_view_set_rect(view, target, dpi);
            if (status == UI_STATUS_OK) status = view_status;
        }
    }

    if (host->event_callback != NULL) {
        (void)snprintf(payload, sizeof(payload), "{\"dpi\":%u}", dpi);
        host->event_callback("ui.host.dpi_changed", payload,
                             host->user_data);
    }
    return status;
}

ui_status_t ui_host_get_dpi(const ui_host_t *host, uint32_t *dpi)
{
    if (host == NULL || dpi == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }

    *dpi = host->dpi;
    return UI_STATUS_OK;
}

ui_status_t ui_host_handle_message(ui_host_t *host,
                                   void *hwnd,
                                   uint32_t message,
                                   uintptr_t w_param,
                                   intptr_t l_param,
                                   intptr_t *result)
{
    if (host == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }

    if (result != NULL) {
        *result = 0;
    }
    return ui_platform_host_handle_message(host, hwnd, message, w_param,
                                           l_param, result);
}

ui_status_t ui_host_set_layout(ui_host_t *host,
                               const ui_layout_desc_t *desc)
{
    ui_layout_desc_t next;
    size_t copy_size;

    if (host == NULL || desc == NULL ||
        desc->size < offsetof(ui_layout_desc_t, status_bar_height) +
                         sizeof(desc->status_bar_height) ||
        desc->size % sizeof(int) != 0u) {
        return UI_STATUS_INVALID_ARGUMENT;
    }

    memset(&next, 0, sizeof(next));
    next.size = (uint32_t)sizeof(next);
    copy_size = desc->size < sizeof(next) ? desc->size : sizeof(next);
    memcpy(&next, desc, copy_size);
    next.size = (uint32_t)sizeof(next);
    if (next.menu_bar_height < 0 || next.toolbar_height < 0 ||
        next.left_sidebar_width < 0 || next.right_sidebar_width < 0 ||
        next.status_bar_height < 0 || next.left_sidebar_min_width < 0 ||
        next.left_sidebar_preferred_width < 0 || next.left_sidebar_max_width < 0 ||
        next.right_sidebar_min_width < 0 || next.right_sidebar_preferred_width < 0 ||
        next.right_sidebar_max_width < 0 || next.main_min_width < 0 ||
        next.collapsed_tag_width < 0 ||
        (next.left_sidebar_max_width > 0 &&
         next.left_sidebar_min_width > next.left_sidebar_max_width) ||
        (next.right_sidebar_max_width > 0 &&
         next.right_sidebar_min_width > next.right_sidebar_max_width) ||
        next.narrow_policy < UI_NARROW_MAIN_PRIORITY ||
        next.narrow_policy > UI_NARROW_DISALLOW_SHRINK) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    host->layout = next;
    return ui_layout_recompute(host);
}

ui_status_t ui_host_set_narrow_policy_callback(ui_host_t *host,
                                               ui_narrow_policy_fn callback,
                                               void *user_data)
{
    if (host == NULL) return UI_STATUS_INVALID_ARGUMENT;
    host->narrow_callback = callback;
    host->narrow_user_data = user_data;
    return ui_layout_recompute(host);
}

ui_status_t ui_host_get_min_size(const ui_host_t *host,
                                 int *logical_width, int *logical_height)
{
    int64_t width;
    int64_t height;
    ui_narrow_window_policy_t policy;
    if (host == NULL || logical_width == NULL || logical_height == NULL)
        return UI_STATUS_INVALID_ARGUMENT;
    width = responsive_main_width(&host->layout);
    policy = host->layout.narrow_policy;
    if (host->narrow_callback != NULL) {
        ui_dispatch_enter((ui_host_t *)host);
        policy = host->narrow_callback((ui_host_t *)host, host->host_rect.width,
            host->host_rect.height, host->dpi, policy, host->narrow_user_data);
        ui_dispatch_leave((ui_host_t *)host);
    }
    if (policy == UI_NARROW_DISALLOW_SHRINK) {
        width += layout_width(host->layout.left_sidebar_width,
            host->layout.left_sidebar_min_width, host->layout.left_sidebar_preferred_width,
            host->layout.left_sidebar_max_width, INT_MAX);
        width += layout_width(host->layout.right_sidebar_width,
            host->layout.right_sidebar_min_width, host->layout.right_sidebar_preferred_width,
            host->layout.right_sidebar_max_width, INT_MAX);
    }
    height = (int64_t)host->layout.menu_bar_height + host->layout.toolbar_height +
             host->layout.status_bar_height;
    *logical_width = width > INT_MAX ? INT_MAX : (int)width;
    *logical_height = height > INT_MAX ? INT_MAX : (int)height;
    return UI_STATUS_OK;
}

ui_status_t ui_host_get_rect(const ui_host_t *host,
                             ui_layout_region_t region,
                             ui_rect_t *rect)
{
    if (host == NULL || rect == NULL || !ui_layout_region_valid(region)) {
        return UI_STATUS_INVALID_ARGUMENT;
    }

    *rect = region==UI_LAYOUT_REGION_BOTTOM?host->bottom_rect:host->layout_rects[region];

    return UI_STATUS_OK;
}

ui_status_t ui_host_get_sidebar_state(const ui_host_t *host,
                                      ui_layout_region_t region,
                                      ui_sidebar_state_t *state)
{
    if (host == NULL || state == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    if (region == UI_LAYOUT_REGION_LEFT_SIDEBAR) {
        *state = host->sidebar_states[0];
        return UI_STATUS_OK;
    }
    if (region == UI_LAYOUT_REGION_RIGHT_SIDEBAR) {
        *state = host->sidebar_states[1];
        return UI_STATUS_OK;
    }
    return UI_STATUS_INVALID_ARGUMENT;
}

ui_status_t ui_host_register_command(ui_host_t *host,
                                     const ui_command_desc_t *desc)
{
    ui_command_entry_t *entry;

    if (host == NULL || desc == NULL ||
        !valid_size(desc->size, offsetof(ui_command_desc_t, shortcut_key)) ||
        desc->id == NULL || desc->id[0] == '\0' ||
        desc->handler == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }

    if (find_command(host, desc->id) != NULL) {
        return UI_STATUS_ALREADY_EXISTS;
    }

    entry = (ui_command_entry_t *)calloc(1u, sizeof(*entry));
    if (entry == NULL) {
        return UI_STATUS_OUT_OF_MEMORY;
    }

    entry->id = ui_strdup(desc->id);
    entry->title = ui_strdup(desc->title);
    entry->params_schema_json = ui_strdup(desc->params_schema_json);
    entry->handler = desc->handler;
    entry->user_data = desc->user_data;
    entry->state.size = sizeof(entry->state);
    entry->state.visible = entry->state.enabled = 1;
    if (desc->size >= sizeof(*desc)) {
        entry->shortcut_key = desc->shortcut_key;
        entry->shortcut_modifiers = desc->shortcut_modifiers;
    }
    if (entry->id == NULL || entry->title == NULL ||
        entry->params_schema_json == NULL) {
        free(entry->id);
        free(entry->title);
        free(entry->params_schema_json);
        free(entry);
        return UI_STATUS_OUT_OF_MEMORY;
    }

    entry->next = host->commands;
    host->commands = entry;
    return UI_STATUS_OK;
}

ui_status_t ui_host_register_menu_item(ui_host_t *host,
                                       const ui_menu_item_desc_t *desc)
{
    ui_menu_entry_t *entry;
    ui_menu_entry_t *it;

    if (host == NULL || desc == NULL ||
        !valid_size(desc->size, offsetof(ui_menu_item_desc_t, reserved_v2)) ||
        desc->id == NULL || desc->id[0] == '\0' ||
        desc->menu_path == NULL || desc->title == NULL ||
        desc->command_id == NULL || find_command(host, desc->command_id) == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }

    for (it = host->menus; it != NULL; it = it->next) {
        if (strcmp(it->id, desc->id) == 0) {
            return UI_STATUS_ALREADY_EXISTS;
        }
    }

    entry = (ui_menu_entry_t *)calloc(1u, sizeof(*entry));
    if (entry == NULL) {
        return UI_STATUS_OUT_OF_MEMORY;
    }

    entry->id = ui_strdup(desc->id);
    entry->menu_path = ui_strdup(desc->menu_path);
    entry->title = ui_strdup(desc->title);
    entry->command_id = ui_strdup(desc->command_id);
    entry->order = desc->order;
    entry->state.size = sizeof(entry->state);
    entry->state.visible = entry->state.enabled = 1;
    if (desc->size >= offsetof(ui_menu_item_desc_t,image_id)+sizeof(desc->image_id)) entry->image_id = desc->image_id;
    if (desc->size >= offsetof(ui_menu_item_desc_t,access_key)+sizeof(desc->access_key)) {
        uint32_t key=desc->access_key;if(key>='a'&&key<='z')key-=32;
        if(key&&!(key>='A'&&key<='Z')&&!(key>='0'&&key<='9')){free(entry->id);free(entry->menu_path);free(entry->title);free(entry->command_id);free(entry);return UI_STATUS_INVALID_ARGUMENT;}
        entry->access_key=key;
    }
    if (entry->id == NULL || entry->menu_path == NULL ||
        entry->title == NULL || entry->command_id == NULL) {
        free(entry->id);
        free(entry->menu_path);
        free(entry->title);
        free(entry->command_id);
        free(entry);
        return UI_STATUS_OUT_OF_MEMORY;
    }

    entry->next = host->menus;
    host->menus = entry;
    return UI_STATUS_OK;
}

ui_status_t ui_host_register_toolbar(ui_host_t *host,
                                     const ui_toolbar_desc_t *desc)
{
    ui_toolbar_entry_t *entry;

    if (host == NULL || desc == NULL ||
        !valid_size(desc->size, offsetof(ui_toolbar_desc_t, display)) ||
        desc->id == NULL || desc->id[0] == '\0' ||
        desc->title == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }

    if (find_toolbar(host, desc->id) != NULL) {
        return UI_STATUS_ALREADY_EXISTS;
    }

    entry = (ui_toolbar_entry_t *)calloc(1u, sizeof(*entry));
    if (entry == NULL) {
        return UI_STATUS_OUT_OF_MEMORY;
    }

    if(desc->size>=sizeof(*desc))entry->display=desc->display;
    if(entry->display!=UI_TOOLBAR_TEXT_ICONS&&entry->display!=UI_TOOLBAR_COMPACT){free(entry);return UI_STATUS_INVALID_ARGUMENT;}
    entry->id = ui_strdup(desc->id);
    entry->title = ui_strdup(desc->title);
    entry->order = desc->order;
    entry->visible = desc->visible ? 1 : 0;
    if (entry->id == NULL || entry->title == NULL) {
        free(entry->id);
        free(entry->title);
        free(entry);
        return UI_STATUS_OUT_OF_MEMORY;
    }

    entry->next = host->toolbars;
    host->toolbars = entry;
    return UI_STATUS_OK;
}

ui_status_t ui_host_register_toolbar_item(
    ui_host_t *host,
    const ui_toolbar_item_desc_t *desc)
{
    ui_toolbar_item_entry_t *entry;
    ui_toolbar_item_entry_t *it;

    if (host == NULL || desc == NULL ||
        !valid_size(desc->size, offsetof(ui_toolbar_item_desc_t, reserved_v2)) ||
        desc->id == NULL || desc->id[0] == '\0' ||
        desc->toolbar_id == NULL || desc->toolbar_id[0] == '\0' ||
        desc->title == NULL || desc->command_id == NULL ||
        find_toolbar(host, desc->toolbar_id) == NULL ||
        find_command(host, desc->command_id) == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }

    for (it = host->toolbar_items; it != NULL; it = it->next) {
        if (strcmp(it->id, desc->id) == 0) {
            return UI_STATUS_ALREADY_EXISTS;
        }
    }

    entry = (ui_toolbar_item_entry_t *)calloc(1u, sizeof(*entry));
    if (entry == NULL) {
        return UI_STATUS_OUT_OF_MEMORY;
    }

    entry->id = ui_strdup(desc->id);
    entry->toolbar_id = ui_strdup(desc->toolbar_id);
    entry->title = ui_strdup(desc->title);
    entry->command_id = ui_strdup(desc->command_id);
    entry->icon_url = ui_strdup(desc->icon_url);
    entry->order = desc->order;
    entry->state.size = sizeof(entry->state);
    entry->state.visible = entry->state.enabled = 1;
    if (desc->size >= sizeof(*desc)) entry->image_id = desc->image_id;
    if (entry->id == NULL || entry->toolbar_id == NULL ||
        entry->title == NULL || entry->command_id == NULL ||
        entry->icon_url == NULL) {
        free(entry->id);
        free(entry->toolbar_id);
        free(entry->title);
        free(entry->command_id);
        free(entry->icon_url);
        free(entry);
        return UI_STATUS_OUT_OF_MEMORY;
    }

    entry->next = host->toolbar_items;
    host->toolbar_items = entry;
    return UI_STATUS_OK;
}

ui_status_t ui_host_register_panel(ui_host_t *host,
                                   const ui_panel_desc_t *desc)
{
    ui_panel_entry_t *entry;
    ui_panel_entry_t *it;
    ui_layout_region_t region = UI_LAYOUT_REGION_RIGHT_SIDEBAR;

    if (host == NULL || desc == NULL ||
        desc->size < offsetof(ui_panel_desc_t, preferred_width) +
                     sizeof(desc->preferred_width) ||
        desc->id == NULL || desc->id[0] == '\0' ||
        desc->title == NULL || desc->entry_url == NULL ||
        desc->preferred_width < 0 ||
        (desc->kind != UI_PANEL_SIDEBAR && desc->kind != UI_PANEL_FLOATING)) {
        return UI_STATUS_INVALID_ARGUMENT;
    }

    if (desc->size >= offsetof(ui_panel_desc_t, dock_region) + sizeof(desc->dock_region) &&
        desc->dock_region != UI_LAYOUT_REGION_NONE)
        region = desc->dock_region;
    if (region != UI_LAYOUT_REGION_LEFT_SIDEBAR &&
        region != UI_LAYOUT_REGION_RIGHT_SIDEBAR &&
        region != UI_LAYOUT_REGION_BOTTOM) return UI_STATUS_INVALID_ARGUMENT;

    for (it = host->panels; it != NULL; it = it->next) {
        if (strcmp(it->id, desc->id) == 0) {
            return UI_STATUS_ALREADY_EXISTS;
        }
    }

    entry = (ui_panel_entry_t *)calloc(1u, sizeof(*entry));
    if (entry == NULL) {
        return UI_STATUS_OUT_OF_MEMORY;
    }

    entry->id = ui_strdup(desc->id);
    entry->title = ui_strdup(desc->title);
    entry->entry_url = ui_strdup(desc->entry_url);
    entry->kind = desc->kind;
    entry->preferred_width = desc->preferred_width;
    entry->dock_region = region;
    if (entry->id == NULL || entry->title == NULL || entry->entry_url == NULL) {
        free(entry->id);
        free(entry->title);
        free(entry->entry_url);
        free(entry);
        return UI_STATUS_OUT_OF_MEMORY;
    }

    entry->next = host->panels;
    host->panels = entry;
    return UI_STATUS_OK;
}

ui_status_t ui_host_set_command_state(ui_host_t *host, const char *id,
    const ui_command_state_t *state)
{
    ui_command_entry_t *entry;
    if (!host || !id || !state || state->size < sizeof(*state)) return UI_STATUS_INVALID_ARGUMENT;
    entry = find_command(host, id); if (!entry) return UI_STATUS_NOT_FOUND;
    entry->state = *state;
    ui_menus_commands_changed(host);
    return ui_host_emit_event(host, "ui.commands.changed", "{}");
}
ui_status_t ui_host_get_command_state(const ui_host_t *host, const char *id,
    ui_command_state_t *state)
{
    ui_command_entry_t *entry;
    if (!host || !id || !state || state->size < sizeof(*state)) return UI_STATUS_INVALID_ARGUMENT;
    entry = find_command((ui_host_t *)host, id); if (!entry) return UI_STATUS_NOT_FOUND;
    *state = entry->state; return UI_STATUS_OK;
}
ui_status_t ui_host_set_item_state(ui_host_t *host, const char *id,
    const ui_command_state_t *state)
{
    ui_menu_entry_t *menu; ui_toolbar_item_entry_t *tool;int found=0;
    if (!host || !id || !state || state->size < sizeof(*state)) return UI_STATUS_INVALID_ARGUMENT;
    for (menu=host->menus; menu; menu=menu->next) if (!strcmp(menu->id,id)) {
        menu->state=*state; found=1;
    }
    for (tool=host->toolbar_items; tool; tool=tool->next) if (!strcmp(tool->id,id)) {
        tool->state=*state; found=1;
    }
    if(found)ui_menus_commands_changed(host);
    return found?ui_host_emit_event(host,"ui.commands.changed","{}"):UI_STATUS_NOT_FOUND;
}
ui_status_t ui_host_set_item_image(ui_host_t *host,const char *id,uint64_t image)
{
    ui_menu_entry_t *m;ui_toolbar_item_entry_t *t;ui_image_info_t info={0};int found=0;
    if(!host||!id)return UI_STATUS_INVALID_ARGUMENT;info.size=sizeof(info);
    if(image&&ui_image_get_info(host,image,&info)!=UI_STATUS_OK)return UI_STATUS_NOT_FOUND;
    for(m=host->menus;m;m=m->next)if(!strcmp(m->id,id)){m->image_id=image;found=1;}
    for(t=host->toolbar_items;t;t=t->next)if(!strcmp(t->id,id)){t->image_id=image;found=1;}
    if(found)ui_menus_commands_changed(host);
    return found?ui_host_emit_event(host,"ui.commands.changed","{}"):UI_STATUS_NOT_FOUND;
}
uint64_t ui_host_dispatch_shortcut(ui_host_t *host, uint32_t key,
    uint32_t modifiers, int text_editing)
{
    ui_command_entry_t *command;
    if (!host) return 0;
    /* Edit gestures remain with the focused Web/native editor. */
    if (text_editing && (key==8 || key==46 || key==37 || key==38 || key==39 || key==40 ||
        key==36 || key==35 || ((modifiers&UI_INPUT_MODIFIER_CONTROL) &&
        (key=='A'||key=='C'||key=='V'||key=='X'||key=='Z'||key=='Y')))) return 0;
    for (command=host->commands; command; command=command->next)
        if (command->shortcut_key==key && command->shortcut_modifiers==modifiers &&
            command->state.visible && command->state.enabled && !command->state.busy)
            return ui_host_invoke(host,command->id,"{}","shortcut");
    return 0;
}

uint64_t ui_host_invoke(ui_host_t *host,
                        const char *command_id,
                        const char *params_json,
                        const char *source)
{
    ui_command_entry_t *command;
    ui_pending_request_t *pending;
    uint64_t request_id;

    if (host == NULL || host->dispatch_blocked || command_id == NULL || command_id[0] == '\0') {
        return 0u;
    }
    if (host->modal_component && (!source || strcmp(source,"component") != 0)) return 0u;

    command = find_command(host, command_id);
    if (command == NULL) {
        return 0u;
    }

    pending = (ui_pending_request_t *)calloc(1u, sizeof(*pending));
    if (pending == NULL) {
        return 0u;
    }

    request_id = host->next_request_id++;
    pending->id = request_id;
    pending->command_id = ui_strdup(command_id);
    pending->source = ui_strdup(source);
    if (pending->command_id == NULL || pending->source == NULL) {
        free(pending->command_id);
        free(pending->source);
        free(pending);
        return 0u;
    }

    pending->next = host->pending;
    host->pending = pending;
    ui_dispatch_enter(host);
    command->handler(host, request_id, command->id,
                     params_json != NULL ? params_json : "{}",
                     source != NULL ? source : "",
                     command->user_data);
    ui_dispatch_leave(host);
    return request_id;
}

ui_status_t ui_host_reply(ui_host_t *host,
                          uint64_t request_id,
                          int success,
                          const char *result_json)
{
    ui_pending_request_t *previous;
    ui_pending_request_t *pending;
    ui_result_t result;

    if (host == NULL || request_id == 0u) {
        return UI_STATUS_INVALID_ARGUMENT;
    }

    pending = find_pending(host, request_id, &previous);
    if (pending == NULL) {
        return UI_STATUS_NOT_FOUND;
    }

    if (previous == NULL) {
        host->pending = pending->next;
    } else {
        previous->next = pending->next;
    }

    result.request_id = pending->id;
    result.command_id = pending->command_id;
    result.source = pending->source;
    result.success = success ? 1 : 0;
    result.result_json = result_json != NULL ? result_json : "{}";
    if (host->result_callback != NULL) {
        host->result_callback(&result, host->user_data);
    }

    free(pending->command_id);
    free(pending->source);
    free(pending);
    return UI_STATUS_OK;
}

ui_status_t ui_host_emit_event(ui_host_t *host,
                               const char *event_id,
                               const char *payload_json)
{
    if (host == NULL || event_id == NULL || event_id[0] == '\0') {
        return UI_STATUS_INVALID_ARGUMENT;
    }

    if (host->event_callback != NULL) {
        ui_dispatch_enter(host);
        host->event_callback(event_id,
                             payload_json != NULL ? payload_json : "{}",
                             host->user_data);
        ui_dispatch_leave(host);
    }
    return UI_STATUS_OK;
}

ui_web_backend_t *ui_web_backend_create(const ui_web_backend_desc_t *desc)
{
    ui_web_backend_t *backend;

    if (desc == NULL || !valid_size(desc->size, sizeof(*desc)) ||
        desc->ops == NULL ||
        desc->ops->size < offsetof(ui_web_backend_ops_t, get_element_rect) ||
        desc->ops->create_view == NULL ||
        desc->ops->destroy_view == NULL) {
        return NULL;
    }

    backend = (ui_web_backend_t *)calloc(1u, sizeof(*backend));
    if (backend == NULL) {
        return NULL;
    }

    memcpy(&backend->ops, desc->ops, offsetof(ui_web_backend_ops_t, get_element_rect));
    if (desc->ops->size >= offsetof(ui_web_backend_ops_t, get_element_rect) +
                          sizeof(desc->ops->get_element_rect))
        backend->ops.get_element_rect = desc->ops->get_element_rect;
    if (desc->ops->size >= offsetof(ui_web_backend_ops_t, set_rect) +
                          sizeof(desc->ops->set_rect))
        backend->ops.set_rect = desc->ops->set_rect;
    if (desc->ops->size >= offsetof(ui_web_backend_ops_t, set_message_handler) +
                          sizeof(desc->ops->set_message_handler))
        backend->ops.set_message_handler = desc->ops->set_message_handler;
    if (desc->ops->size >= offsetof(ui_web_backend_ops_t, post_json) +
                          sizeof(desc->ops->post_json))
        backend->ops.post_json = desc->ops->post_json;
    if (desc->ops->size >= offsetof(ui_web_backend_ops_t, get_capabilities) +
                          sizeof(desc->ops->get_capabilities))
        backend->ops.get_capabilities = desc->ops->get_capabilities;
    if (desc->ops->size >= offsetof(ui_web_backend_ops_t, native_handle) +
                          sizeof(desc->ops->native_handle))
        backend->ops.native_handle = desc->ops->native_handle;
    if (desc->ops->size >= offsetof(ui_web_backend_ops_t, image_changed) + sizeof(desc->ops->image_changed))
        backend->ops.image_changed = desc->ops->image_changed;
    if (desc->ops->size >= offsetof(ui_web_backend_ops_t, get_presentation)+sizeof(desc->ops->get_presentation)) backend->ops.get_presentation=desc->ops->get_presentation;
    if (desc->ops->size >= offsetof(ui_web_backend_ops_t, capture_rgba)+sizeof(desc->ops->capture_rgba)) backend->ops.capture_rgba=desc->ops->capture_rgba;
    if (desc->ops->size >= offsetof(ui_web_backend_ops_t, flush)+sizeof(desc->ops->flush)) backend->ops.flush=desc->ops->flush;
    backend->user_data = desc->user_data;
    return backend;
}

void ui_web_backend_destroy(ui_web_backend_t *backend)
{
    if (backend == NULL) {
        return;
    }

    while (backend->views != NULL) {
        ui_web_view_destroy(backend->views);
    }
    free(backend);
}

ui_web_view_t *ui_web_view_create(ui_host_t *host,
                                  ui_web_backend_t *backend)
{
    ui_web_view_t *view;
    ui_status_t status;

    if (host == NULL || backend == NULL || backend->ops.create_view == NULL) {
        return NULL;
    }

    view = (ui_web_view_t *)calloc(1u, sizeof(*view));
    if (view == NULL) {
        return NULL;
    }

    view->host = host;
    view->backend = backend;
    view->dpi = host->dpi;
    ui_dispatch_enter(host);
    status = backend->ops.create_view(backend->user_data,
                                      host,
                                      &view->user_data);
    ui_dispatch_leave(host);
    if (status != UI_STATUS_OK) {
        free(view);
        return NULL;
    }

    view->host_next = host->web_views;
    host->web_views = view;
    view->backend_next = backend->views;
    backend->views = view;
    return view;
}

void ui_web_view_destroy(ui_web_view_t *view)
{
    ui_web_view_t **it;

    if (view == NULL) {
        return;
    }

    ui_shell_web_view_destroyed(view->host, view);
    if (view->host != NULL) {
        it = &view->host->web_views;
        while (*it != NULL && *it != view) {
            it = &(*it)->host_next;
        }
        if (*it == view) {
            *it = view->host_next;
        }
    }

    if (view->backend != NULL) {
        it = &view->backend->views;
        while (*it != NULL && *it != view) {
            it = &(*it)->backend_next;
        }
        if (*it == view) {
            *it = view->backend_next;
        }
        if (view->backend->ops.destroy_view != NULL) {
            ui_dispatch_enter(view->host);
            view->backend->ops.destroy_view(view->backend->user_data,
                                            view->user_data);
            ui_dispatch_leave(view->host);
        }
    }

    free(view);
}

ui_status_t ui_web_view_load_html(ui_web_view_t *view,
                                  const char *html_utf8)
{
    ui_status_t status;
    if (view == NULL || html_utf8 == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    if (view->backend->ops.load_html == NULL) {
        return UI_STATUS_UNSUPPORTED;
    }
    ui_dispatch_enter(view->host);
    status = view->backend->ops.load_html(view->backend->user_data,
                                        view->user_data,
                                        html_utf8);
    ui_dispatch_leave(view->host); return status;
}

ui_status_t ui_web_view_resize(ui_web_view_t *view,
                               int logical_width,
                               int logical_height,
                               uint32_t dpi)
{
    ui_rect_t rect;
    if (view == NULL) return UI_STATUS_INVALID_ARGUMENT;
    rect = view->rect;
    rect.width = logical_width;
    rect.height = logical_height;
    return ui_web_view_set_rect(view, &rect, dpi);
}

ui_status_t ui_web_view_set_rect(ui_web_view_t *view,
                                 const ui_rect_t *logical_rect, uint32_t dpi)
{
    ui_status_t status;

    if (view == NULL || logical_rect == NULL || logical_rect->width < 0 ||
        logical_rect->height < 0 || dpi == 0u) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    if (view->backend->ops.set_rect != NULL) {
        ui_dispatch_enter(view->host);
        status = view->backend->ops.set_rect(view->backend->user_data,
                                             view->user_data, logical_rect, dpi);
        ui_dispatch_leave(view->host);
    } else if (view->backend->ops.resize != NULL &&
               logical_rect->x == 0 && logical_rect->y == 0) {
        ui_dispatch_enter(view->host);
        status = view->backend->ops.resize(view->backend->user_data,
            view->user_data, logical_rect->width, logical_rect->height, dpi);
        ui_dispatch_leave(view->host);
    } else {
        return UI_STATUS_UNSUPPORTED;
    }
    if (status == UI_STATUS_OK) {
        view->rect = *logical_rect;
        view->logical_width = logical_rect->width;
        view->logical_height = logical_rect->height;
        view->dpi = dpi;
    }
    return status;
}

ui_status_t ui_web_view_get_rect(const ui_web_view_t *view, ui_rect_t *logical_rect)
{
    if (view == NULL || logical_rect == NULL) return UI_STATUS_INVALID_ARGUMENT;
    *logical_rect = view->rect;
    return UI_STATUS_OK;
}

ui_status_t ui_web_view_set_layout_region(ui_web_view_t *view,
                                          ui_layout_region_t region)
{
    ui_status_t status;
    if (view == NULL || (region != UI_LAYOUT_REGION_NONE &&
                         !ui_layout_region_valid(region)))
        return UI_STATUS_INVALID_ARGUMENT;
    if (region != UI_LAYOUT_REGION_NONE) {
        status = ui_web_view_set_rect(view, (region==UI_LAYOUT_REGION_BOTTOM?&view->host->bottom_rect:&view->host->layout_rects[region]),
                                      view->host->dpi);
        if (status != UI_STATUS_OK) return status;
    }
    view->layout_region = region;
    return UI_STATUS_OK;
}

ui_status_t ui_web_view_dispatch_input(ui_web_view_t *view,
                                       const ui_input_event_t *event)
{
    ui_status_t status;
    if (view == NULL || !valid_web_input_event(event)) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    if (view->backend->ops.dispatch_input == NULL) {
        return UI_STATUS_UNSUPPORTED;
    }
    ui_dispatch_enter(view->host);
    status = view->backend->ops.dispatch_input(view->backend->user_data,
                                             view->user_data,
                                             event);
    ui_dispatch_leave(view->host); return status;
}

ui_status_t ui_web_view_invalidate(ui_web_view_t *view)
{
    ui_status_t status;
    if (view == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    if (view->backend->ops.invalidate == NULL) {
        return UI_STATUS_UNSUPPORTED;
    }
    ui_dispatch_enter(view->host);
    status = view->backend->ops.invalidate(view->backend->user_data,
                                         view->user_data);
    ui_dispatch_leave(view->host); return status;
}

ui_status_t ui_web_view_get_element_rect(ui_web_view_t *view,
                                         const char *id_utf8,
                                         ui_rect_t *logical_rect)
{
    ui_status_t status;
    if (view == NULL || id_utf8 == NULL || id_utf8[0] == '\0' ||
        logical_rect == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    if (view->backend->ops.get_element_rect == NULL) {
        return UI_STATUS_UNSUPPORTED;
    }
    ui_dispatch_enter(view->host);
    status = view->backend->ops.get_element_rect(view->backend->user_data,
                                               view->user_data, id_utf8,
                                               logical_rect);
    ui_dispatch_leave(view->host); return status;
}

static void forward_web_message(const char *json, void *data)
{
    ui_web_view_t *view = (ui_web_view_t *)data;
    ui_host_t *host = view->host;
    ui_web_message_fn callback = view->message_callback;
    void *user_data = view->message_user_data;
    char *copy;
    if (!callback || !json || host->dispatch_blocked || strlen(json) > 1048576u)
        return;
    copy = ui_strdup(json);
    if (!copy) return;
    ui_dispatch_enter(host);
    callback(view, copy, user_data);
    free(copy);
    ui_dispatch_leave(host);
}

ui_status_t ui_web_view_set_message_callback(ui_web_view_t *view,
    ui_web_message_fn callback, void *user_data)
{
    ui_status_t status;
    if (!view) return UI_STATUS_INVALID_ARGUMENT;
    if (!view->backend->ops.set_message_handler) return UI_STATUS_UNSUPPORTED;
    ui_dispatch_enter(view->host);
    status = view->backend->ops.set_message_handler(view->backend->user_data,
        view->user_data, callback ? forward_web_message : NULL, view);
    if (status == UI_STATUS_OK) {
        view->message_callback = callback;
        view->message_user_data = user_data;
    }
    ui_dispatch_leave(view->host);
    return status;
}

ui_status_t ui_web_view_post_json(ui_web_view_t *view, const char *json)
{
    ui_status_t status;
    if (!view || !json || strlen(json) > 1048576u)
        return UI_STATUS_INVALID_ARGUMENT;
    if (!view->backend->ops.post_json) return UI_STATUS_UNSUPPORTED;
    ui_dispatch_enter(view->host);
    status = view->backend->ops.post_json(view->backend->user_data,
                                        view->user_data, json);
    ui_dispatch_leave(view->host);
    return status;
}
void ui_web_image_changed(ui_web_view_t *view,uint64_t id)
{
    if(view->backend->ops.image_changed)view->backend->ops.image_changed(view->backend->user_data,view->user_data,id);
    else (void)ui_web_view_invalidate(view);
}

ui_status_t ui_web_view_get_capabilities(ui_web_view_t *view, uint64_t *caps)
{
    ui_status_t status;
    if (!view || !caps) return UI_STATUS_INVALID_ARGUMENT;
    *caps = 0;
    if (!view->backend->ops.get_capabilities) return UI_STATUS_UNSUPPORTED;
    ui_dispatch_enter(view->host);
    status = view->backend->ops.get_capabilities(view->backend->user_data,
                                              view->user_data, caps);
    ui_dispatch_leave(view->host);
    return status;
}

void *ui_web_view_native_handle(ui_web_view_t *view)
{
    void *handle;
    if (!view || !view->backend->ops.native_handle) return NULL;
    ui_dispatch_enter(view->host);
    handle = view->backend->ops.native_handle(view->backend->user_data,
                                            view->user_data);
    ui_dispatch_leave(view->host);
    return handle;
}

static ui_surface_t *create_surface(ui_host_t *host,
                                    const ui_surface_desc_t *desc,
                                    const ui_opengl_config_t *config,
                                    ui_status_t *status,int offscreen,const char *gl_library)
{
    ui_surface_t *surface;
    ui_surface_t *it;
    ui_status_t result;

    if (status != NULL) *status = UI_STATUS_INVALID_ARGUMENT;

    if (host == NULL || desc == NULL ||
        !valid_size(desc->size, sizeof(*desc)) ||
        desc->id == NULL || desc->id[0] == '\0' ||
        desc->rect.width < 0 || desc->rect.height < 0) {
        return NULL;
    }

    if (desc->kind != UI_SURFACE_OPENGL && desc->kind != UI_SURFACE_NATIVE) {
        return NULL;
    }
    for (it = host->surfaces; it != NULL; it = it->next) {
        if (strcmp(it->id, desc->id) == 0) {
            if (status != NULL) *status = UI_STATUS_ALREADY_EXISTS;
            return NULL;
        }
    }

    surface = (ui_surface_t *)calloc(1u, sizeof(*surface));
    if (surface == NULL) {
        if (status != NULL) *status = UI_STATUS_OUT_OF_MEMORY;
        return NULL;
    }

    surface->host = host;
    surface->offscreen=offscreen;
    surface->gl_library_path=gl_library;
    surface->id = ui_strdup(desc->id);
    surface->kind = desc->kind;
    surface->rect = desc->rect;
    surface->pixel_rect = logical_to_pixel_rect(&desc->rect, host->dpi);
    surface->visible = desc->visible ? 1 : 0;
    surface->layout_region = UI_LAYOUT_REGION_NONE;
    result = surface->id == NULL ? UI_STATUS_OUT_OF_MEMORY :
        config != NULL ? ui_platform_surface_create_configured(surface, config) :
                         ui_platform_surface_create(surface);
    if(result==UI_STATUS_OK&&offscreen){ui_pixel_buffer_t query={0};query.size=sizeof(query);result=ui_platform_offscreen_render(surface,&query);if(result!=UI_STATUS_OK)ui_platform_surface_destroy(surface);}
    surface->gl_library_path=NULL;
    if (result != UI_STATUS_OK) {
        if (status != NULL) *status = result;
        free(surface->id);
        free(surface);
        return NULL;
    }

    surface->next = host->surfaces;
    host->surfaces = surface;
    if (status != NULL) *status = UI_STATUS_OK;
    return surface;
}

ui_surface_t *ui_surface_create(ui_host_t *host,
                                const ui_surface_desc_t *desc)
{
    return create_surface(host, desc, NULL, NULL,0,NULL);
}

ui_surface_t *ui_opengl_surface_create(ui_host_t *host,
                                       const ui_surface_desc_t *desc,
                                       const ui_opengl_config_t *config,
                                       ui_status_t *status)
{
    if (config == NULL || desc == NULL || desc->kind != UI_SURFACE_OPENGL) {
        if (status != NULL) *status = UI_STATUS_INVALID_ARGUMENT;
        return NULL;
    }
    return create_surface(host, desc, config, status,0,NULL);
}

ui_surface_t *ui_opengl_offscreen_surface_create(ui_host_t *host,const ui_surface_desc_t *desc,const ui_opengl_config_t *config,ui_status_t *status)
{
    if(!host||!desc||!config||config->size<sizeof(*config)||desc->kind!=UI_SURFACE_OPENGL){if(status)*status=UI_STATUS_INVALID_ARGUMENT;return NULL;}
    if(config->legacy_context||config->samples<0||config->profile!=UI_OPENGL_PROFILE_COMPATIBILITY||config->major_version<3||
       (config->major_version==3&&config->minor_version<3)){if(status)*status=UI_STATUS_UNSUPPORTED;return NULL;}
    if(desc->rect.width<=0||desc->rect.height<=0){if(status)*status=UI_STATUS_INVALID_ARGUMENT;return NULL;}
    {uint64_t width=((uint64_t)desc->rect.width*host->dpi+48)/96,height=((uint64_t)desc->rect.height*host->dpi+48)/96;
     if(width>16384||height>16384||width*height>32u*1024u*1024u/8){if(status)*status=UI_STATUS_LIMIT_EXCEEDED;return NULL;}}
    return create_surface(host,desc,config,status,1,NULL);
}
ui_surface_t *ui_opengl_windowless_surface_create(ui_host_t *host,const ui_surface_desc_t *desc,const ui_opengl_config_t *config,const ui_opengl_windowless_config_t *provider,ui_status_t *status)
{
 if(status)*status=UI_STATUS_INVALID_ARGUMENT;
 if(!host||!desc||!config||config->size<sizeof(*config)||!provider||provider->size<sizeof(*provider)||!provider->library_path_utf8||desc->kind!=UI_SURFACE_OPENGL)return NULL;
 if(config->legacy_context||config->debug_context||config->samples<0||config->profile!=UI_OPENGL_PROFILE_COMPATIBILITY||config->major_version<3||(config->major_version==3&&config->minor_version<3)){if(status)*status=UI_STATUS_UNSUPPORTED;return NULL;}
 if(desc->rect.width<=0||desc->rect.height<=0)return NULL;
 return create_surface(host,desc,config,status,2,provider->library_path_utf8);
}
ui_status_t ui_opengl_surface_get_window_dependency(const ui_surface_t *surface,ui_opengl_window_dependency_t *dependency)
{if(!surface||!dependency||surface->kind!=UI_SURFACE_OPENGL)return UI_STATUS_INVALID_ARGUMENT;
 *dependency=surface->offscreen==2?UI_OPENGL_NO_WINDOW:surface->offscreen?UI_OPENGL_HIDDEN_WINDOW:UI_OPENGL_VISIBLE_WINDOW;return UI_STATUS_OK;}
ui_status_t ui_opengl_offscreen_render(ui_surface_t *surface,ui_pixel_buffer_t *pixels)
{ui_status_t status;if(!surface||!surface->offscreen||!pixels||pixels->size<sizeof(*pixels))return UI_STATUS_INVALID_ARGUMENT;
 if(surface->host->dispatch_blocked)return UI_STATUS_CANCELLED;ui_dispatch_enter(surface->host);status=ui_platform_offscreen_render(surface,pixels);ui_dispatch_leave(surface->host);return status;}
ui_status_t ui_surface_dispatch_input(ui_surface_t *surface,const ui_input_event_t *event)
{if(!surface||!event||event->size<sizeof(*event)||event->kind<UI_INPUT_POINTER_MOVE||event->kind>UI_INPUT_TEXT)return UI_STATUS_INVALID_ARGUMENT;
 if(surface->host->dispatch_blocked||!surface->host->app_active||surface->host->modal_component)return UI_STATUS_CANCELLED;
 if(event->kind==UI_INPUT_TEXT&&!event->text_utf8)return UI_STATUS_INVALID_ARGUMENT;
 if(ui_menus_route_input(surface->host,NULL,event,0)==UI_STATUS_OK)return UI_STATUS_OK;
 if(event->kind==UI_INPUT_KEY_DOWN&&ui_host_dispatch_shortcut(surface->host,event->key_code,event->modifiers,0))return UI_STATUS_OK;
 if(!surface->input)return UI_STATUS_NOT_FOUND;ui_dispatch_enter(surface->host);surface->input(surface,event,surface->input_user_data);ui_dispatch_leave(surface->host);return UI_STATUS_OK;}

void ui_surface_destroy(ui_surface_t *surface)
{
    ui_surface_t **it;

    if (surface == NULL) {
        return;
    }

    ui_shell_surface_destroyed(surface->host, surface);
    it = &surface->host->surfaces;
    while (*it != NULL && *it != surface) {
        it = &(*it)->next;
    }
    if (*it == surface) {
        *it = surface->next;
    }

    ui_platform_surface_destroy(surface);
    free(surface->id);
    free(surface);
}

void *ui_surface_native_handle(const ui_surface_t *surface)
{
    return surface != NULL ? surface->native_handle : NULL;
}

ui_status_t ui_surface_get_pixel_rect(const ui_surface_t *surface,
                                      ui_rect_t *rect)
{
    if (surface == NULL || rect == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }

    *rect = surface->pixel_rect;
    return UI_STATUS_OK;
}

ui_status_t ui_surface_get_rect(const ui_surface_t *surface, ui_rect_t *rect)
{
    if (surface == NULL || rect == NULL) return UI_STATUS_INVALID_ARGUMENT;
    *rect = surface->rect;
    return UI_STATUS_OK;
}

ui_status_t ui_surface_set_callbacks(ui_surface_t *surface,
                                      ui_surface_resize_fn resized,
                                      ui_surface_frame_fn frame,
                                      void *user_data)
{
    if (surface == NULL) return UI_STATUS_INVALID_ARGUMENT;
    surface->resized = resized;
    surface->frame = frame;
    surface->callback_user_data = user_data;
    if (resized != NULL) {
        ui_dispatch_enter(surface->host);
        resized(surface, &surface->rect, &surface->pixel_rect,
                surface->host->dpi, user_data);
        ui_dispatch_leave(surface->host);
    }
    return UI_STATUS_OK;
}

ui_status_t ui_surface_invalidate(ui_surface_t *surface)
{
    if (surface == NULL) return UI_STATUS_INVALID_ARGUMENT;
    return ui_platform_surface_invalidate(surface);
}

ui_status_t ui_surface_set_input_callback(ui_surface_t *surface,
                                          ui_surface_input_fn input,
                                          void *user_data)
{
    if (surface == NULL) return UI_STATUS_INVALID_ARGUMENT;
    surface->input = input;
    surface->input_user_data = user_data;
    return UI_STATUS_OK;
}

ui_status_t ui_surface_set_rect(ui_surface_t *surface,
                                const ui_rect_t *rect)
{
    ui_rect_t old_rect;
    ui_rect_t old_pixel_rect;

    if (surface == NULL || rect == NULL || rect->width < 0 || rect->height < 0) {
        return UI_STATUS_INVALID_ARGUMENT;
    }

    old_rect = surface->rect;
    old_pixel_rect = surface->pixel_rect;
    surface->rect = *rect;
    surface->pixel_rect = logical_to_pixel_rect(rect, surface->host->dpi);
    {ui_status_t status=ui_platform_surface_set_rect(surface,rect);if(status!=UI_STATUS_OK){
        surface->rect = old_rect;
        surface->pixel_rect = old_pixel_rect;
        return status;
    }}

    if (memcmp(&old_rect, &surface->rect, sizeof(old_rect)) != 0 ||
        memcmp(&old_pixel_rect, &surface->pixel_rect, sizeof(old_pixel_rect)) != 0) {
        if (surface->resized != NULL) {
            ui_dispatch_enter(surface->host);
            surface->resized(surface, &surface->rect, &surface->pixel_rect,
                             surface->host->dpi, surface->callback_user_data);
            ui_dispatch_leave(surface->host);
        }
        (void)ui_surface_invalidate(surface);
    }

    return UI_STATUS_OK;
}

ui_status_t ui_surface_set_layout_region(ui_surface_t *surface,
                                         ui_layout_region_t region)
{
    if (surface == NULL ||
        (region != UI_LAYOUT_REGION_NONE && !ui_layout_region_valid(region))) {
        return UI_STATUS_INVALID_ARGUMENT;
    }

    if (region != UI_LAYOUT_REGION_NONE &&
        ui_surface_set_rect(surface, (region==UI_LAYOUT_REGION_BOTTOM?&surface->host->bottom_rect:&surface->host->layout_rects[region])) !=
            UI_STATUS_OK) {
        return UI_STATUS_PLATFORM_ERROR;
    }

    surface->layout_region = region;
    return UI_STATUS_OK;
}

ui_layout_region_t ui_surface_layout_region(const ui_surface_t *surface)
{
    return surface != NULL ? surface->layout_region : UI_LAYOUT_REGION_NONE;
}

ui_status_t ui_surface_set_visible(ui_surface_t *surface, int visible)
{
    if (surface == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }

    if (ui_platform_surface_set_visible(surface, visible ? 1 : 0) != UI_STATUS_OK) {
        return UI_STATUS_PLATFORM_ERROR;
    }

    surface->visible = visible ? 1 : 0;
    return UI_STATUS_OK;
}

ui_status_t ui_surface_make_current(ui_surface_t *surface)
{
    if (surface == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    return ui_platform_surface_make_current(surface);
}

ui_status_t ui_surface_swap_buffers(ui_surface_t *surface)
{
    if (surface == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    return ui_platform_surface_swap_buffers(surface);
}

ui_status_t ui_web_view_get_presentation(ui_web_view_t *v,const char *id,ui_element_presentation_t *out)
{
    ui_status_t status;
    if(!v||!id||!out||out->size<sizeof(*out))return UI_STATUS_INVALID_ARGUMENT;
    if(!v->backend->ops.get_presentation)return UI_STATUS_UNSUPPORTED;
    ui_dispatch_enter(v->host);status=v->backend->ops.get_presentation(v->backend->user_data,v->user_data,id,out);ui_dispatch_leave(v->host);return status;
}
ui_status_t ui_web_view_capture_rgba(ui_web_view_t *v,int width,int height,uint32_t dpi,ui_pixel_buffer_t *out)
{
    ui_status_t status;
    if(!v||!out||out->size<sizeof(*out))return UI_STATUS_INVALID_ARGUMENT;
    if(!v->backend->ops.capture_rgba)return UI_STATUS_UNSUPPORTED;
    status=ui_web_view_resize(v,width,height,dpi);if(status!=UI_STATUS_OK)return status;
    ui_dispatch_enter(v->host);status=v->backend->ops.capture_rgba(v->backend->user_data,v->user_data,out);ui_dispatch_leave(v->host);return status;
}
ui_status_t ui_web_view_flush(ui_web_view_t *v,uint32_t budget)
{
    ui_status_t status;if(!v||!budget||budget>1024)return UI_STATUS_INVALID_ARGUMENT;
    if(!v->backend->ops.flush)return UI_STATUS_UNSUPPORTED;
    ui_dispatch_enter(v->host);status=v->backend->ops.flush(v->backend->user_data,v->user_data,budget);ui_dispatch_leave(v->host);return status;
}
