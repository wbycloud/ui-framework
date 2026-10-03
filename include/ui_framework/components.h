#ifndef UI_FRAMEWORK_COMPONENTS_H
#define UI_FRAMEWORK_COMPONENTS_H
#include "ui_framework/application.h"
#include "ui_framework/shell.h"
#include "ui_framework/images.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct ui_component ui_component_t;
typedef enum ui_component_kind { UI_COMPONENT_TREE = 1, UI_COMPONENT_TABLE = 2,
    UI_COMPONENT_LIST = 3, UI_COMPONENT_FORM = 4, UI_COMPONENT_STATUS = 5,
    UI_COMPONENT_DIALOG = 6 } ui_component_kind_t;
typedef enum ui_value_kind { UI_VALUE_TEXT = 1, UI_VALUE_NUMBER = 2,
    UI_VALUE_BOOLEAN = 3, UI_VALUE_ENUM = 4, UI_VALUE_COLOR = 5,
    UI_VALUE_IMAGE = 6, UI_VALUE_STYLE = 7, UI_VALUE_GROUP = 8 } ui_value_kind_t;
#define UI_VALUE_READONLY 1u
#define UI_VALUE_DISABLED 2u
#define UI_VALUE_MIXED 4u
#define UI_VALUE_MODIFIED 8u
#define UI_VALUE_REQUIRED 16u
#define UI_VALUE_MULTILINE 32u
typedef struct ui_column_desc {
    uint32_t size;
    const char *id, *title;
    ui_value_kind_t kind;
    int width;
} ui_column_desc_t;
typedef struct ui_field_desc {
    uint32_t size;
    const char *id, *title, *unit, *group;
    ui_value_kind_t kind;
    uint32_t flags;
    const char *const *options;
    size_t option_count;
} ui_field_desc_t;
typedef struct ui_cell {
    uint32_t size;
    const char *column_id, *text, *error;
    ui_value_kind_t kind;
    uint32_t flags, color_rgba;
    ui_image_id_t image_id;
    ui_fill_style_t style;
} ui_cell_t;
typedef struct ui_row {
    uint32_t size;
    uint64_t id, parent_id, content_version;
    const char *title;
    int has_children;
    ui_image_id_t image_id;
    const ui_cell_t *cells;
    size_t cell_count;
} ui_row_t;
typedef enum ui_query_kind { UI_QUERY_ROWS = 1, UI_QUERY_THUMBNAIL = 2 } ui_query_kind_t;
/* Borrowed only during UI-thread source callback. Copy for async use. Supply
 * data through submit/post, not a pointer to callback-stack arrays. */
typedef struct ui_component_query {
    uint32_t size;
    ui_query_kind_t kind;
    uint64_t component_generation, request_id, parent_id, first;
    uint64_t item_id, content_version;
    size_t count, first_column, column_count;
    uint32_t pixel_width, pixel_height, dpi;
} ui_component_query_t;
typedef void (*ui_component_source_fn)(ui_component_t *, const ui_component_query_t *, void *);
typedef struct ui_component_bindings {
    const char *select, *edit, *rename, *context_menu, *submit, *cancel;
} ui_component_bindings_t;
typedef struct ui_component_desc {
    uint32_t size;
    const char *id, *title;
    ui_component_kind_t kind;
    const ui_column_desc_t *columns;
    size_t column_count;
    const ui_field_desc_t *fields;
    size_t field_count;
    ui_component_bindings_t commands;
    ui_component_source_fn source;
    void *user_data;
    int row_height; /* zero => 32 logical pixels; minimum 24 */
} ui_component_desc_t;
typedef struct ui_component_batch {
    uint32_t size;
    uint64_t component_generation, request_id, parent_id, first, total_count;
    const ui_row_t *rows;
    size_t row_count;
} ui_component_batch_t;
typedef struct ui_component_state {
    uint32_t size;
    uint64_t generation, first, total_count, selected_id;
    size_t row_count, first_column, column_count, rendered_nodes;
    int focused, dirty, visible, modal;
    size_t cached_rows, cached_bytes;
    ui_status_t presentation_status;
    size_t row_nodes_created; /* Actual row allocations reported by the template. */
} ui_component_state_t;
typedef struct ui_thumbnail_result {
    uint32_t size;
    uint64_t component_generation, request_id, item_id, content_version;
    uint32_t dpi;
    ui_rgba_desc_t rgba;
    int failed;
} ui_thumbnail_result_t;
/* UI-thread. Copied description. Host owns component; unregister invalidates
 * pointer. Unregister during callbacks is rejected; defer until return. */
UI_API ui_status_t ui_component_register(ui_host_t *, const ui_component_desc_t *, ui_component_t **);
UI_API ui_status_t ui_component_unregister(ui_component_t *);
UI_API ui_component_t *ui_component_find(ui_host_t *, const char *);
UI_API ui_status_t ui_component_mount(ui_component_t *, ui_content_slot_t *);
UI_API ui_status_t ui_component_get_state(const ui_component_t *, ui_component_state_t *);
UI_API ui_status_t ui_component_set_source(ui_component_t *, ui_component_source_fn, void *);
UI_API ui_status_t ui_component_query(ui_component_t *, uint64_t parent, uint64_t first,
    size_t count, size_t first_column, size_t column_count);
UI_API ui_status_t ui_component_submit(ui_component_t *, const ui_component_batch_t *);
UI_API ui_status_t ui_component_update_rows(ui_component_t *, const ui_row_t *, size_t);
UI_API ui_status_t ui_component_insert_rows(ui_component_t *, uint64_t parent,
    uint64_t index, const ui_row_t *, size_t);
UI_API ui_status_t ui_component_remove_rows(ui_component_t *, const uint64_t *, size_t);
UI_API ui_status_t ui_component_expand(ui_component_t *, uint64_t id, int expanded);
UI_API ui_status_t ui_component_select(ui_component_t *, uint64_t id);
UI_API ui_status_t ui_component_set_field(ui_component_t *, const char *, const ui_cell_t *);
/* Borrowed strings until next field update/unregister. Returns the current draft. */
UI_API ui_status_t ui_component_get_field(const ui_component_t *, const char *, ui_cell_t *);
UI_API ui_status_t ui_component_set_error(ui_component_t *, const char *, const char *);
/* Accept successful submission: current drafts become committed values. */
UI_API ui_status_t ui_component_accept_fields(ui_component_t *);
UI_API ui_status_t ui_component_set_text(ui_component_t *, const char *);
/* Borrowed until the status component changes or the host is destroyed. */
UI_API const char *ui_host_status_text(const ui_host_t *);
UI_API ui_status_t ui_component_thumbnail(ui_component_t *, const ui_thumbnail_result_t *);
UI_API ui_status_t ui_component_set_visible(ui_component_t *, int);
UI_API ui_status_t ui_component_show_dialog(ui_component_t *);
UI_API ui_status_t ui_component_close_dialog(ui_component_t *);
/* Uses existing menu registrations matching menu_path, with item identity. */
UI_API ui_status_t ui_component_show_menu(ui_component_t *, uint64_t item_id, const char *menu_path);
/* Thread-safe copied posts, no callbacks/host pointers. Workspace must remain
 * alive until all workers stop. Closed/obsolete identities are discarded. */
UI_API ui_status_t ui_workspace_post_component_batch(ui_workspace_t *, uint64_t,
    const char *, const ui_component_batch_t *);
UI_API ui_status_t ui_workspace_post_thumbnail(ui_workspace_t *, uint64_t,
    const char *, const ui_thumbnail_result_t *);
UI_API ui_status_t ui_workspace_set_component_queue_limit(ui_workspace_t *, uint64_t, size_t);
#ifdef __cplusplus
}
#endif
#endif
