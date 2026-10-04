#ifndef UI_FRAMEWORK_MENUS_H
#define UI_FRAMEWORK_MENUS_H
#include "ui_framework/components.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Optional group labels/order; existing menu_path registrations remain valid. */
typedef struct ui_menu_group_desc {
    uint32_t size;
    const char *path, *title;
    int order;
    uint32_t reserved_v4; /* Retains the complete old 32-byte prefix. */
    uint32_t access_key; /* API5 optional ASCII letter/digit; missing => 0. */
} ui_menu_group_desc_t;
typedef struct ui_menu_model_entry {
    uint32_t size;
    const char *id, *path, *title, *command_id;
    int group, order;
    uint64_t image_id;
    ui_command_state_t state;
    uint32_t reserved_v4; /* Preserve the old model's 80-byte full prefix. */
    uint32_t access_key;
} ui_menu_model_entry_t;
/* Strings and entry borrowed only during the UI-thread visitor. */
typedef void (*ui_menu_visit_fn)(const ui_menu_model_entry_t *, void *);
typedef enum ui_menu_anchor_kind {
    UI_MENU_ANCHOR_HOST = 0, UI_MENU_ANCHOR_CONTENT_SLOT = 1,
    UI_MENU_ANCHOR_COMPONENT_ROW = 2
} ui_menu_anchor_kind_t;
typedef struct ui_menu_anchor {
    uint32_t size;
    ui_menu_anchor_kind_t kind;
    ui_rect_t rect; /* Logical pixels relative to host/slot; row uses its layout. */
    ui_content_slot_t *slot;
    ui_component_t *component;
    uint64_t row_id; /* Display anchor, independent of business target_id. */
} ui_menu_anchor_t;
typedef struct ui_menu_popup_desc {
    uint32_t size;
    const char *path;
    ui_menu_anchor_t anchor;
    uint64_t target_id;
} ui_menu_popup_desc_t;
#define UI_MENU_MAX_ITEMS 128u
UI_API ui_status_t ui_host_register_menu_group(ui_host_t *, const ui_menu_group_desc_t *);
/* NULL/empty path visits root groups, otherwise its direct items/subgroups. */
UI_API ui_status_t ui_host_visit_menu(ui_host_t *, const char *, ui_menu_visit_fn, void *);
UI_API ui_status_t ui_host_show_menu(ui_host_t *, const ui_menu_popup_desc_t *);
/* Ordered registered tool items, starting at first; reuses their states/commands. */
UI_API ui_status_t ui_host_show_toolbar_menu(ui_host_t *, const ui_menu_anchor_t *, size_t first);
UI_API ui_status_t ui_host_show_tooltip(ui_host_t *, const ui_menu_anchor_t *, const char *text_utf8);
UI_API ui_status_t ui_host_close_menu(ui_host_t *);
UI_API ui_status_t ui_host_hide_tooltip(ui_host_t *);
UI_API ui_status_t ui_host_menu_get_capabilities(ui_host_t *, uint64_t *);
#define UI_MENU_CAP_MODEL UINT64_C(1)
#define UI_MENU_CAP_POPUP UINT64_C(2)
#define UI_MENU_CAP_OFFSCREEN UINT64_C(4)
#define UI_MENU_CAP_ACCESS_KEYS UINT64_C(8)
/* Feed KEY_DOWN/KEY_UP before ordinary shortcuts. Bare Alt release opens root;
 * Alt+access key opens a group. Ctrl/AltGr and registered Alt shortcuts retain
 * priority. Same-level duplicates cycle focus; Enter executes once per press.
 * NOT_FOUND means not consumed. Native callers suppress translated SYSCHAR
 * when consumed and do not feed composition keys. UI thread only. */
UI_API ui_status_t ui_host_menu_dispatch_input(ui_host_t *, const ui_input_event_t *);
UI_API ui_status_t ui_host_menu_get_presentation(ui_host_t *, const char *, ui_element_presentation_t *);
UI_API ui_status_t ui_host_menu_get_item_presentation(ui_host_t *,const char *registered_id,ui_element_presentation_t *);
UI_API ui_status_t ui_host_menu_capture_rgba(ui_host_t *, ui_pixel_buffer_t *);
UI_API ui_status_t ui_host_menu_flush(ui_host_t *, uint32_t budget);
#ifdef __cplusplus
}
#endif
#endif
