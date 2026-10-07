#ifndef UI_FRAMEWORK_UI_H
#define UI_FRAMEWORK_UI_H

#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32) && defined(UI_FRAMEWORK_BUILD_SHARED)
#  if defined(UI_FRAMEWORK_BUILDING)
#    define UI_API __declspec(dllexport)
#  else
#    define UI_API __declspec(dllimport)
#  endif
#else
#  define UI_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define UI_FRAMEWORK_API_VERSION 8u
#define UI_FRAMEWORK_MIN_API_VERSION 1u

typedef struct ui_host ui_host_t;
typedef struct ui_surface ui_surface_t;
typedef struct ui_web_backend ui_web_backend_t;
typedef struct ui_web_view ui_web_view_t;

typedef struct ui_rect {
    int x;
    int y;
    int width;
    int height;
} ui_rect_t;

/* Fixed regions managed by the native layout pass. */
typedef enum ui_layout_region {
    UI_LAYOUT_REGION_NONE = 0,
    UI_LAYOUT_REGION_ROOT = 1,
    UI_LAYOUT_REGION_MENU_BAR = 2,
    UI_LAYOUT_REGION_TOOLBAR = 3,
    UI_LAYOUT_REGION_MAIN = 4,
    UI_LAYOUT_REGION_LEFT_SIDEBAR = 5,
    UI_LAYOUT_REGION_RIGHT_SIDEBAR = 6,
    UI_LAYOUT_REGION_STATUS_BAR = 7,
    UI_LAYOUT_REGION_COUNT = 8, /* Legacy ABI sentinel; 8 remains reserved. */
    UI_LAYOUT_REGION_BOTTOM = 9
} ui_layout_region_t;

typedef enum ui_narrow_window_policy {
    /* Collapse sidebars as needed so the central work area keeps priority. */
    UI_NARROW_MAIN_PRIORITY = 0,
    UI_NARROW_KEEP_SIDEBARS = 1,
    UI_NARROW_COLLAPSE_LEFT = 2,
    UI_NARROW_COLLAPSE_RIGHT = 3,
    UI_NARROW_FLOAT_SIDEBARS = 4,
    UI_NARROW_DISALLOW_SHRINK = 5
} ui_narrow_window_policy_t;

typedef enum ui_sidebar_state {
    UI_SIDEBAR_VISIBLE = 0,
    UI_SIDEBAR_COLLAPSED = 1,
    UI_SIDEBAR_FLOATING = 2
} ui_sidebar_state_t;

/* Dimensions are in logical client pixels. Zero disables that area. */
typedef struct ui_layout_desc {
    uint32_t size;
    int menu_bar_height;
    int toolbar_height;
    int left_sidebar_width;
    int right_sidebar_width;
    int status_bar_height;
    /* Optional responsive constraints appended after the original fields. */
    int left_sidebar_min_width;
    int left_sidebar_preferred_width;
    int left_sidebar_max_width;
    int right_sidebar_min_width;
    int right_sidebar_preferred_width;
    int right_sidebar_max_width;
    ui_narrow_window_policy_t narrow_policy;
    int main_min_width;          /* Zero uses 400 for responsive layouts. */
    int collapsed_tag_width;     /* Zero hides the rail; nonzero shows a tab. */
    int left_sidebar_no_collapse;
    int right_sidebar_no_collapse;
} ui_layout_desc_t;

/* Runs on the UI thread; must not mutate or destroy the host from this callback. */
typedef ui_narrow_window_policy_t (*ui_narrow_policy_fn)(
    ui_host_t *host, int logical_width, int logical_height, uint32_t dpi,
    ui_narrow_window_policy_t configured_policy, void *user_data);

typedef enum ui_status {
    UI_STATUS_PENDING = 1, /* API5 asynchronous rendering; pump UI and poll. */
    UI_STATUS_OK = 0,
    UI_STATUS_INVALID_ARGUMENT = -1,
    UI_STATUS_OUT_OF_MEMORY = -2,
    UI_STATUS_ALREADY_EXISTS = -3,
    UI_STATUS_NOT_FOUND = -4,
    UI_STATUS_UNSUPPORTED = -5,
    UI_STATUS_PLATFORM_ERROR = -6,
    /* Appended statuses preserve the numeric values of the original ABI. */
    UI_STATUS_PERMISSION_DENIED = -7,
    UI_STATUS_CANCELLED = -8,
    UI_STATUS_VALIDATION_FAILED = -9,
    UI_STATUS_LIMIT_EXCEEDED = -10
} ui_status_t;

typedef enum ui_surface_kind {
    UI_SURFACE_WEB = 1,
    UI_SURFACE_OPENGL = 2,
    UI_SURFACE_NATIVE = 3
} ui_surface_kind_t;

typedef enum ui_panel_kind {
    UI_PANEL_SIDEBAR = 1,
    UI_PANEL_FLOATING = 2
} ui_panel_kind_t;

typedef struct ui_result {
    uint64_t request_id;
    const char *command_id;
    const char *source;
    int success;
    const char *result_json;
} ui_result_t;

typedef void (*ui_result_callback_fn)(const ui_result_t *result,
                                      void *user_data);

typedef void (*ui_event_callback_fn)(const char *event_id,
                                     const char *payload_json,
                                     void *user_data);

typedef struct ui_host_config {
    uint32_t size;
    uint32_t api_version;
    /* Optional until an OpenGL/native surface is created. */
    void *native_parent;
    void *user_data;
    ui_result_callback_fn result_callback;
    ui_event_callback_fn event_callback;
} ui_host_config_t;

typedef void (*ui_command_handler_fn)(ui_host_t *host,
                                      uint64_t request_id,
                                      const char *command_id,
                                      const char *params_json,
                                      const char *source,
                                      void *user_data);

typedef struct ui_command_desc {
    uint32_t size;
    const char *id;
    const char *title;
    const char *params_schema_json;
    ui_command_handler_fn handler;
    void *user_data;
    uint32_t shortcut_key, shortcut_modifiers;
} ui_command_desc_t;

typedef struct ui_command_state {
    uint32_t size;
    int visible, enabled, checked, busy;
} ui_command_state_t;

typedef struct ui_menu_item_desc {
    uint32_t size;
    const char *id;
    const char *menu_path;
    const char *title;
    const char *command_id;
    int order;
    uint32_t reserved_v2;
    uint64_t image_id;
    /* API5: 0 disables; ASCII A-Z/a-z/0-9 mnemonic, independent of title. */
    uint32_t access_key;
} ui_menu_item_desc_t;

/* Toolbars are registered separately from their command-backed items. */
typedef enum ui_toolbar_display { UI_TOOLBAR_TEXT_ICONS = 0, UI_TOOLBAR_COMPACT = 1 } ui_toolbar_display_t;
typedef struct ui_toolbar_desc {
    uint32_t size;
    const char *id;
    const char *title;
    int order;
    int visible;
    ui_toolbar_display_t display; /* Appended; missing => TEXT_ICONS. */
} ui_toolbar_desc_t;

typedef struct ui_toolbar_item_desc {
    uint32_t size;
    const char *id;
    const char *toolbar_id;
    const char *title;
    const char *command_id;
    const char *icon_url;
    int order;
    uint32_t reserved_v2;
    uint64_t image_id;
} ui_toolbar_item_desc_t;

typedef struct ui_panel_desc {
    uint32_t size;
    const char *id;
    const char *title;
    ui_panel_kind_t kind;
    const char *entry_url;
    int preferred_width;
    uint32_t reserved_v1;
    /* LEFT/RIGHT/BOTTOM destination; NONE preserves the right sidebar default. */
    ui_layout_region_t dock_region;
} ui_panel_desc_t;

typedef struct ui_surface_desc {
    uint32_t size;
    const char *id;
    ui_surface_kind_t kind;
    ui_rect_t rect;
    int visible;
} ui_surface_desc_t;

/* Input events are expressed in logical client pixels. */
typedef enum ui_input_kind {
    UI_INPUT_POINTER_MOVE = 1,
    UI_INPUT_POINTER_DOWN = 2,
    UI_INPUT_POINTER_UP = 3,
    UI_INPUT_WHEEL = 4,
    UI_INPUT_KEY_DOWN = 5,
    UI_INPUT_KEY_UP = 6,
    UI_INPUT_TEXT = 7,
    UI_INPUT_CANCEL = 8 /* Capture loss/cancel, including offscreen input. */
} ui_input_kind_t;

typedef enum ui_input_modifier {
    UI_INPUT_MODIFIER_CONTROL = 1,
    UI_INPUT_MODIFIER_SHIFT = 2,
    UI_INPUT_MODIFIER_ALT = 4
} ui_input_modifier_t;

typedef struct ui_input_event {
    uint32_t size;
    ui_input_kind_t kind;
    int x;
    int y;
    int wheel_delta;
    uint32_t pointer_button;
    uint32_t key_code;
    uint32_t modifiers;
    const char *text_utf8;
} ui_input_event_t;

/*
 * A backend owns the actual Web implementation. The framework stores these
 * callbacks and never exposes a third-party engine type through the public
 * ABI. create_view and destroy_view are required; later callbacks may be
 * omitted and then return UI_STATUS_UNSUPPORTED from the corresponding API.
 */
typedef ui_status_t (*ui_web_view_create_fn)(void *backend_user_data,
                                             ui_host_t *host,
                                             void **view_user_data);
typedef void (*ui_web_view_destroy_fn)(void *backend_user_data,
                                       void *view_user_data);
typedef ui_status_t (*ui_web_view_load_html_fn)(void *backend_user_data,
                                                void *view_user_data,
                                                const char *html_utf8);
typedef ui_status_t (*ui_web_view_resize_fn)(void *backend_user_data,
                                             void *view_user_data,
                                             int logical_width,
                                             int logical_height,
                                             uint32_t dpi);
typedef ui_status_t (*ui_web_view_dispatch_input_fn)(
    void *backend_user_data,
    void *view_user_data,
    const ui_input_event_t *event);
typedef ui_status_t (*ui_web_view_invalidate_fn)(void *backend_user_data,
                                                 void *view_user_data);
typedef ui_status_t (*ui_web_view_get_element_rect_fn)(
    void *backend_user_data,
    void *view_user_data,
    const char *id_utf8,
    ui_rect_t *logical_rect);
typedef ui_status_t (*ui_web_view_set_rect_fn)(
    void *backend_user_data, void *view_user_data,
    const ui_rect_t *logical_rect, uint32_t dpi);

/* Messages are UTF-8 JSON data. Callbacks run on the UI thread and must not
 * directly destroy the view/backend/host; schedule teardown after returning. */
typedef void (*ui_web_backend_message_fn)(const char *json_utf8, void *user_data);
typedef void (*ui_web_message_fn)(ui_web_view_t *view, const char *json_utf8,
                                 void *user_data);
typedef ui_status_t (*ui_web_set_message_handler_fn)(void *backend_user_data,
    void *view_user_data, ui_web_backend_message_fn callback, void *user_data);
typedef ui_status_t (*ui_web_post_json_fn)(void *backend_user_data,
    void *view_user_data, const char *json_utf8);
typedef ui_status_t (*ui_web_get_capabilities_fn)(void *backend_user_data,
    void *view_user_data, uint64_t *capabilities);
typedef void *(*ui_web_native_handle_fn)(void *backend_user_data,
                                        void *view_user_data);

#define UI_WEB_CAP_JSON_MESSAGES UINT64_C(1)
#define UI_WEB_CAP_DYNAMIC_DOM UINT64_C(2)
#define UI_WEB_CAP_RESPONSIVE_LAYOUT UINT64_C(4)
#define UI_WEB_CAP_NATIVE_WINDOW UINT64_C(8)
#define UI_WEB_CAP_TEXT_INPUT UINT64_C(16)
#define UI_WEB_CAP_IMAGES UINT64_C(32)
#define UI_WEB_CAP_WEB_TEXT_EDIT UINT64_C(64)
#define UI_WEB_CAP_COMPONENTS UINT64_C(128)

/* API4 diagnostic output. Rectangles use view-local logical pixels. */
typedef struct ui_element_presentation {
    uint32_t size;
    ui_rect_t rect, clip;
    int visible, enabled, focused, text_overflow;
    char text_utf8[4096];
} ui_element_presentation_t;
/* Caller-owned top-down RGBA8. Web capture is straight RGBA with alpha=255;
 * OpenGL readback retains the framebuffer's application-defined alpha.
 * pixels=NULL queries width/height/required stride without allocating. */
typedef struct ui_pixel_buffer {
    uint32_t size, width, height;
    size_t stride, capacity;
    uint8_t *pixels;
} ui_pixel_buffer_t;
#define UI_WEB_CAP_OFFSCREEN_CAPTURE UINT64_C(256)
#define UI_WEB_CAP_PRESENTATION_QUERY UINT64_C(512)
#define UI_WEB_CAP_ASYNC_RENDER UINT64_C(1024)

typedef struct ui_web_backend_ops {
    uint32_t size;
    ui_web_view_create_fn create_view;
    ui_web_view_destroy_fn destroy_view;
    ui_web_view_load_html_fn load_html;
    ui_web_view_resize_fn resize;
    ui_web_view_dispatch_input_fn dispatch_input;
    ui_web_view_invalidate_fn invalidate;
    ui_web_view_get_element_rect_fn get_element_rect;
    ui_web_view_set_rect_fn set_rect;
    ui_web_set_message_handler_fn set_message_handler;
    ui_web_post_json_fn post_json;
    ui_web_get_capabilities_fn get_capabilities;
    ui_web_native_handle_fn native_handle;
    void (*image_changed)(void *backend_user_data, void *view_user_data, uint64_t image_id);
    ui_status_t (*get_presentation)(void *, void *, const char *, ui_element_presentation_t *);
    ui_status_t (*capture_rgba)(void *, void *, ui_pixel_buffer_t *);
    ui_status_t (*flush)(void *, void *, uint32_t budget);

} ui_web_backend_ops_t;

typedef struct ui_web_backend_desc {
    uint32_t size;
    const ui_web_backend_ops_t *ops;
    void *user_data;
} ui_web_backend_desc_t;

/* UI-thread. Async backends return PENDING; pump normally then retry the same
 * query/capture. No caller buffer is retained. A completed result is consumed
 * once; navigation/resize/input/data changes invalidate previous results. */
UI_API ui_status_t ui_web_view_get_presentation(ui_web_view_t *, const char *, ui_element_presentation_t *);
UI_API ui_status_t ui_web_view_capture_rgba(ui_web_view_t *, int width, int height,
    uint32_t dpi, ui_pixel_buffer_t *);
/* UI-thread; bounded pending JS jobs/layout, not OS input or workspace messages. */
UI_API ui_status_t ui_web_view_flush(ui_web_view_t *, uint32_t budget);

UI_API ui_host_t *ui_host_create(const ui_host_config_t *config);
UI_API void ui_host_destroy(ui_host_t *host);
/* Call before creating the application's first Win32 window. */
typedef enum ui_run_mode { UI_RUN_WINDOWED=0, UI_RUN_OFFSCREEN=1 } ui_run_mode_t;
/* Explicit workspace mode. NULL parent alone does not select this mode. */
UI_API ui_status_t ui_host_get_run_mode(const ui_host_t *,ui_run_mode_t *);
UI_API ui_status_t ui_framework_initialize(void);
UI_API int ui_framework_supports_api(uint32_t api_version);
/* Width and height are logical client pixels. */
UI_API ui_status_t ui_host_resize(ui_host_t *host, int width, int height);
/* DPI is expressed as Windows effective DPI (96 means 100%). */
UI_API ui_status_t ui_host_set_dpi(ui_host_t *host, uint32_t dpi);
UI_API ui_status_t ui_host_get_dpi(const ui_host_t *host, uint32_t *dpi);
/*
 * Forward a native window message to the framework. The host handles layout
 * messages such as WM_SIZE and WM_DPICHANGED. A handled message returns OK
 * and writes its native result when result is non-NULL; other messages return
 * UI_STATUS_NOT_FOUND so the application's window procedure can dispatch it.
 */
UI_API ui_status_t ui_host_handle_message(ui_host_t *host,
                                          void *hwnd,
                                          uint32_t message,
                                          uintptr_t w_param,
                                          intptr_t l_param,
                                          intptr_t *result);
UI_API ui_status_t ui_host_set_layout(ui_host_t *host,
                                      const ui_layout_desc_t *desc);
UI_API ui_status_t ui_host_get_rect(const ui_host_t *host,
                                    ui_layout_region_t region,
                                    ui_rect_t *rect);
UI_API ui_status_t ui_host_get_sidebar_state(
    const ui_host_t *host,
    ui_layout_region_t region,
    ui_sidebar_state_t *state);
UI_API ui_status_t ui_host_set_narrow_policy_callback(ui_host_t *host,
                                                       ui_narrow_policy_fn callback,
                                                       void *user_data);
UI_API ui_status_t ui_host_get_min_size(const ui_host_t *host,
                                         int *logical_width,
                                         int *logical_height);

UI_API ui_status_t ui_host_register_command(ui_host_t *host,
                                             const ui_command_desc_t *desc);
UI_API ui_status_t ui_host_register_menu_item(
    ui_host_t *host,
    const ui_menu_item_desc_t *desc);
UI_API ui_status_t ui_host_register_toolbar(ui_host_t *host,
                                             const ui_toolbar_desc_t *desc);
UI_API ui_status_t ui_host_register_toolbar_item(
    ui_host_t *host,
    const ui_toolbar_item_desc_t *desc);
UI_API ui_status_t ui_host_register_panel(ui_host_t *host,
                                           const ui_panel_desc_t *desc);
UI_API ui_status_t ui_host_set_command_state(ui_host_t *, const char *, const ui_command_state_t *);
UI_API ui_status_t ui_host_get_command_state(const ui_host_t *, const char *, ui_command_state_t *);
UI_API ui_status_t ui_host_set_item_state(ui_host_t *, const char *, const ui_command_state_t *);
/* Updates all registered menu/toolbar items with this ID. Zero clears. */
UI_API ui_status_t ui_host_set_item_image(ui_host_t *, const char *, uint64_t image_id);
UI_API uint64_t ui_host_dispatch_shortcut(ui_host_t *, uint32_t key,
    uint32_t modifiers, int text_editing);

/* Returns a non-zero request ID. The handler may reply immediately or later. */
UI_API uint64_t ui_host_invoke(ui_host_t *host,
                               const char *command_id,
                               const char *params_json,
                               const char *source);
/* The caller must marshal replies to the host/UI thread in this initial slice. */
UI_API ui_status_t ui_host_reply(ui_host_t *host,
                                 uint64_t request_id,
                                 int success,
                                 const char *result_json);
UI_API ui_status_t ui_host_emit_event(ui_host_t *host,
                                      const char *event_id,
                                      const char *payload_json);

/*
 * Backends are created explicitly. The standalone host uses the controlled
 * light backend; applications may select it, WebView2, or a custom backend.
 */
UI_API ui_web_backend_t *ui_web_backend_create(
    const ui_web_backend_desc_t *desc);
UI_API void ui_web_backend_destroy(ui_web_backend_t *backend);
UI_API ui_web_view_t *ui_web_view_create(ui_host_t *host,
                                          ui_web_backend_t *backend);
UI_API void ui_web_view_destroy(ui_web_view_t *view);
UI_API ui_status_t ui_web_view_load_html(ui_web_view_t *view,
                                          const char *html_utf8);
UI_API ui_status_t ui_web_view_resize(ui_web_view_t *view,
                                       int logical_width,
                                       int logical_height,
                                       uint32_t dpi);
UI_API ui_status_t ui_web_view_set_rect(ui_web_view_t *view,
                                         const ui_rect_t *logical_rect,
                                         uint32_t dpi);
UI_API ui_status_t ui_web_view_get_rect(const ui_web_view_t *view,
                                         ui_rect_t *logical_rect);
/* Bound views follow host layout and DPI; NONE keeps the current rectangle. */
UI_API ui_status_t ui_web_view_set_layout_region(ui_web_view_t *view,
                                                  ui_layout_region_t region);
UI_API ui_status_t ui_web_view_dispatch_input(
    ui_web_view_t *view,
    const ui_input_event_t *event);
UI_API ui_status_t ui_web_view_invalidate(ui_web_view_t *view);
/* Hidden elements return a zero rect; absent elements return NOT_FOUND. */
UI_API ui_status_t ui_web_view_get_element_rect(ui_web_view_t *view,
                                                 const char *id_utf8,
                                         ui_rect_t *logical_rect);

/* UI-thread calls. Message arguments are borrowed only for the duration of the
 * call. The backend parses JSON as data; it never concatenates it into script. */
UI_API ui_status_t ui_web_view_set_message_callback(ui_web_view_t *view,
    ui_web_message_fn callback, void *user_data);
UI_API ui_status_t ui_web_view_post_json(ui_web_view_t *view,
                                         const char *json_utf8);
UI_API ui_status_t ui_web_view_get_capabilities(ui_web_view_t *view,
                                                uint64_t *capabilities);
/* Borrowed native presentation window, or NULL when unsupported. */
UI_API void *ui_web_view_native_handle(ui_web_view_t *view);

/* Native/OpenGL surfaces. Web views use ui_web_view_create with a backend. */
UI_API ui_surface_t *ui_surface_create(ui_host_t *host,
                                       const ui_surface_desc_t *desc);
UI_API void ui_surface_destroy(ui_surface_t *surface);
UI_API void *ui_surface_native_handle(const ui_surface_t *surface);
/* Returns the surface rectangle in device pixels for its current DPI. */
UI_API ui_status_t ui_surface_get_pixel_rect(const ui_surface_t *surface,
                                             ui_rect_t *rect);
UI_API ui_status_t ui_surface_get_rect(const ui_surface_t *surface,
                                       ui_rect_t *rect);
typedef void (*ui_surface_resize_fn)(ui_surface_t *surface,
                                      const ui_rect_t *logical_rect,
                                      const ui_rect_t *pixel_rect,
                                      uint32_t dpi, void *user_data);
typedef void (*ui_surface_frame_fn)(ui_surface_t *surface, void *user_data);
typedef void (*ui_surface_input_fn)(ui_surface_t *surface,
                                     const ui_input_event_t *event,
                                     void *user_data);
/* Input/text pointers are borrowed only during this UI-thread callback. */
UI_API ui_status_t ui_surface_set_input_callback(ui_surface_t *surface,
                                                  ui_surface_input_fn input,
                                                  void *user_data);
/* Callbacks run on the UI thread and must not destroy/reconfigure the surface. */
UI_API ui_status_t ui_surface_set_callbacks(ui_surface_t *surface,
                                              ui_surface_resize_fn resized,
                                              ui_surface_frame_fn frame,
                                              void *user_data);
UI_API ui_status_t ui_surface_invalidate(ui_surface_t *surface);
UI_API ui_status_t ui_surface_set_rect(ui_surface_t *surface,
                                       const ui_rect_t *rect);
UI_API ui_status_t ui_surface_set_layout_region(ui_surface_t *surface,
                                                ui_layout_region_t region);
UI_API ui_layout_region_t ui_surface_layout_region(
    const ui_surface_t *surface);
UI_API ui_status_t ui_surface_set_visible(ui_surface_t *surface, int visible);
UI_API ui_status_t ui_surface_make_current(ui_surface_t *surface);
UI_API ui_status_t ui_surface_swap_buffers(ui_surface_t *surface);

#ifdef __cplusplus
}
#endif

#endif
