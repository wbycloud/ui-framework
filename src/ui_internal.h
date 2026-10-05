#ifndef UI_FRAMEWORK_INTERNAL_H
#define UI_FRAMEWORK_INTERNAL_H

#include "ui_framework/ui.h"
#include "ui_framework/opengl.h"
#include "ui_framework/shell.h"
#include "ui_framework/components.h"
#include "ui_framework/menus.h"

typedef struct ui_command_entry ui_command_entry_t;
typedef struct ui_menu_entry ui_menu_entry_t;
typedef struct ui_toolbar_entry ui_toolbar_entry_t;
typedef struct ui_toolbar_item_entry ui_toolbar_item_entry_t;
typedef struct ui_panel_entry ui_panel_entry_t;
typedef struct ui_pending_request ui_pending_request_t;

struct ui_surface {
    ui_host_t *host;
    char *id;
    ui_surface_kind_t kind;
    ui_rect_t rect;
    ui_rect_t pixel_rect;
    int visible;
    int offscreen;
    const char *gl_library_path; /* Borrowed only during provider creation. */
    ui_layout_region_t layout_region;
    ui_surface_resize_fn resized;
    ui_surface_frame_fn frame;
    void *callback_user_data;
    ui_surface_input_fn input;
    void *input_user_data;
    void *native_handle;
    void *platform;
    ui_surface_t *next;
};

struct ui_web_backend {
    ui_web_backend_ops_t ops;
    void *user_data;
    ui_web_view_t *views;
};

struct ui_web_view {
    ui_host_t *host;
    ui_web_backend_t *backend;
    void *user_data;
    int logical_width;
    int logical_height;
    ui_rect_t rect;
    ui_layout_region_t layout_region;
    uint32_t dpi;
    ui_web_view_t *host_next;
    ui_web_view_t *backend_next;
    ui_web_message_fn message_callback;
    void *message_user_data;
};

struct ui_host {
    int dispatch_blocked; /* Workspace close gate; legacy hosts default to zero. */
    unsigned dispatch_depth;
    void (*dispatch_idle)(void *data);
    void *dispatch_idle_data;
    void *native_parent;
    void *user_data;
    ui_result_callback_fn result_callback;
    ui_event_callback_fn event_callback;
    uint64_t next_request_id;
    uint32_t dpi;
    ui_rect_t host_rect;
    ui_layout_desc_t layout;
    ui_rect_t layout_rects[UI_LAYOUT_REGION_COUNT];
    ui_sidebar_state_t sidebar_states[2];
    ui_narrow_policy_fn narrow_callback;
    void *narrow_user_data;
    ui_command_entry_t *commands;
    ui_menu_entry_t *menus;
    void *menu_groups, *menu_popup;
    int menu_alt_pending, menu_routing;
    uint32_t menu_pressed_key;
    ui_toolbar_entry_t *toolbars;
    ui_toolbar_item_entry_t *toolbar_items;
    ui_panel_entry_t *panels;
    ui_pending_request_t *pending;
    ui_surface_t *surfaces;
    ui_web_view_t *web_views;
    ui_shell_t *shell;
    ui_component_t *components;
    struct ui_image_entry *images;
    size_t image_bytes, image_metadata_bytes, image_limit, image_evictions;
    uint64_t component_generation;
    ui_component_t *modal_component;
    ui_component_t *focused_component;
    int app_active;
    ui_run_mode_t run_mode;
    ui_host_t *image_source; /* Trusted shell chrome may display its active application's images. */
    const char *menu_open_path; /* Borrowed current root path; NULL when closed. */
    int menu_dark;
};

struct ui_command_entry {
    char *id;
    char *title;
    char *params_schema_json;
    ui_command_handler_fn handler;
    void *user_data;
    uint32_t shortcut_key, shortcut_modifiers;
    ui_command_state_t state;
    ui_command_entry_t *next;
};

struct ui_menu_entry {
    char *id;
    char *menu_path;
    char *title;
    char *command_id;
    int order;
    uint64_t image_id;
    uint32_t access_key;
    ui_command_state_t state;
    ui_menu_entry_t *next;
};

struct ui_toolbar_entry {
    char *id;
    char *title;
    int order;
    int visible;
    ui_toolbar_display_t display;
    ui_toolbar_entry_t *next;
};

struct ui_toolbar_item_entry {
    char *id;
    char *toolbar_id;
    char *title;
    char *command_id;
    char *icon_url;
    int order;
    uint64_t image_id;
    ui_command_state_t state;
    ui_toolbar_item_entry_t *next;
};

struct ui_panel_entry {
    char *id;
    char *title;
    ui_panel_kind_t kind;
    char *entry_url;
    int preferred_width;
    ui_layout_region_t dock_region;
    ui_panel_entry_t *next;
};

struct ui_pending_request {
    uint64_t id;
    char *command_id;
    char *source;
    ui_pending_request_t *next;
};

char *ui_strdup(const char *value);
ui_native_shell_t *ui_native_shell_create_offscreen(const ui_native_shell_config_t *);
int ui_content_slot_belongs_to(const ui_content_slot_t *,const ui_host_t *);
void ui_dispatch_enter(ui_host_t *host);
void ui_dispatch_leave(ui_host_t *host);
void ui_components_destroy(ui_host_t *host);
void ui_menus_destroy(ui_host_t *);
void ui_menus_commands_changed(ui_host_t *);
void ui_menus_hide_tooltip(ui_host_t *);
ui_status_t ui_menus_route_input(ui_host_t *,const void *,const ui_input_event_t *,int composing);
void ui_menus_component_invalidated(ui_host_t *,ui_component_t *,uint64_t);
int ui_component_menu_target_valid(ui_component_t *,uint64_t,uint64_t);
ui_status_t ui_component_menu_anchor(ui_component_t *,ui_host_t *,uint64_t,ui_rect_t *,void **,uint64_t *);
void ui_components_layout(ui_host_t *host);
void ui_components_active(ui_host_t *host, int active);
int ui_components_input_allowed(const ui_host_t *host, const void *view_data);
void ui_images_destroy(ui_host_t *host);
ui_status_t ui_image_png_stream(const ui_host_t *,uint64_t,void **);
ui_status_t ui_images_reserve_metadata(ui_host_t *host,size_t bytes);
void ui_images_release_metadata(ui_host_t *host,size_t bytes);
void ui_image_changed(ui_host_t *host, uint64_t id);
ui_status_t ui_image_set_evictable(ui_host_t *host, uint64_t id);
/* Private GDI rendering adapter; draw accepts an HDC/RECT on Windows. */
int ui_image_draw(const ui_host_t *host, uint64_t id, void *dc, const void *rect, int disabled);
void ui_web_image_changed(ui_web_view_t *, uint64_t);
ui_component_batch_t *ui_component_batch_copy(const ui_component_batch_t *, size_t *bytes);
void ui_component_batch_free(ui_component_batch_t *);

ui_status_t ui_platform_surface_create(ui_surface_t *surface);
ui_status_t ui_platform_surface_create_configured(
    ui_surface_t *surface, const ui_opengl_config_t *config);
ui_status_t ui_platform_offscreen_render(ui_surface_t *,ui_pixel_buffer_t *);
ui_status_t ui_platform_surface_invalidate(ui_surface_t *surface);
void ui_platform_surface_destroy(ui_surface_t *surface);
ui_status_t ui_platform_surface_set_rect(ui_surface_t *surface,
                                         const ui_rect_t *rect);
ui_status_t ui_platform_surface_set_visible(ui_surface_t *surface,
                                            int visible);
ui_status_t ui_platform_surface_make_current(ui_surface_t *surface);
ui_status_t ui_platform_surface_swap_buffers(ui_surface_t *surface);
ui_status_t ui_platform_host_handle_message(ui_host_t *host,
                                            void *hwnd,
                                            uint32_t message,
                                            uintptr_t w_param,
                                            intptr_t l_param,
                                            intptr_t *result);
uint32_t ui_platform_get_dpi(void *native_parent);
ui_status_t ui_platform_prepare_dpi(void);

ui_status_t ui_layout_recompute(ui_host_t *host);
int ui_layout_region_valid(ui_layout_region_t region);
void ui_shell_layout_changed(ui_host_t *host);
int ui_shell_surface_bound(ui_host_t *host, const ui_surface_t *surface);
int ui_shell_web_view_bound(ui_host_t *host, const ui_web_view_t *view);
uint32_t ui_shell_surface_dpi(ui_host_t *host, const ui_surface_t *surface);
void ui_shell_surface_destroyed(ui_host_t *host, ui_surface_t *surface);
void ui_shell_web_view_destroyed(ui_host_t *host, ui_web_view_t *view);

#endif
