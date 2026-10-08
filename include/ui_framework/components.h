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
    const char *help; /* API8 optional, copied; read only with the full field. */
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
typedef enum ui_query_kind { UI_QUERY_ROWS = 1, UI_QUERY_THUMBNAIL = 2,
    UI_QUERY_SELECTION = 3 } ui_query_kind_t;
/* Borrowed only during UI-thread source callback. Copy for async use. Supply
 * data through submit/post, not a pointer to callback-stack arrays. */
typedef struct ui_component_query {
    uint32_t size;
    ui_query_kind_t kind;
    uint64_t component_generation, request_id, parent_id, first;
    uint64_t item_id, content_version;
    size_t count, first_column, column_count;
    uint32_t pixel_width, pixel_height, dpi;
    /* API6: complete-source ordering, borrowed during callback. NONE=0,
     * ascending=1, descending=-1. Selection query requests stable IDs only. */
    const char *sort_column;
    int sort_direction;
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
    /* API5 borrowed backend, retained through unregister/host teardown.
     * NULL selects the framework-owned light backend. */
    ui_web_backend_t *web_backend;
    /* API6 optional semantic command, copied. NULL disables user sorting. */
    const char *sort_command;
    uint32_t selection_flags;
} ui_component_desc_t;
#define UI_SELECTION_MULTIPLE 1u
typedef struct ui_component_batch {
    uint32_t size;
    uint64_t component_generation, request_id, parent_id, first, total_count;
    const ui_row_t *rows;
    size_t row_count; /* Empty batch may report a shrunken total below first.
                      * Nonempty first/count must stay within total_count. */
} ui_component_batch_t;
typedef struct ui_component_state {
    uint32_t size;
    uint64_t generation, first, total_count, selected_id;
    size_t row_count, first_column, column_count, rendered_nodes;
    int focused, dirty, visible, modal;
    size_t cached_rows, cached_bytes;
    ui_status_t presentation_status;
    size_t row_nodes_created; /* Actual row allocations reported by the template. */
    /* API6 complete fields only; sort_column borrowed until sort/unregister. */
    size_t selected_count;
    const char *sort_column;
    int sort_direction;
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
/* Explicit real-template mode, no HWND. UI-thread. */
UI_API ui_status_t ui_component_mount_offscreen(ui_component_t *, int width, int height, uint32_t dpi);
UI_API ui_status_t ui_component_dispatch_input(ui_component_t *, const ui_input_event_t *);
UI_API ui_status_t ui_component_get_presentation(ui_component_t *, const char *, ui_element_presentation_t *);
UI_API ui_status_t ui_component_capture_rgba(ui_component_t *, int width, int height, uint32_t dpi, ui_pixel_buffer_t *);
UI_API ui_status_t ui_component_flush(ui_component_t *, uint32_t budget);
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
/* Stable IDs, at most512, charged to existing2MiB component cache budget.
 * Replacing source clears selection; sort/page changes preserve it; removal
 * removes IDs. Programmatic selection does not invoke semantic commands. */
UI_API ui_status_t ui_component_set_selection(ui_component_t *, const uint64_t *, size_t);
UI_API ui_status_t ui_component_get_selection(const ui_component_t *, uint64_t *, size_t, size_t *);
/* Range in current complete-source order, max512. Async source must return
 * matching generation/request; newer range, source/sort/close invalidates it. */
UI_API ui_status_t ui_component_select_range(ui_component_t *, uint64_t, size_t, int extend);
UI_API ui_status_t ui_component_submit_selection(ui_component_t *, uint64_t generation,
    uint64_t request, const uint64_t *, size_t);
/* Requeries source and invokes sort_command once with column/direction;
 * never sorts cached pages. Source owns sorting the complete dataset. */
UI_API ui_status_t ui_component_set_sort(ui_component_t *, const char *, int);
/* API8. UI-thread, TABLE only. Widths are logical pixels. Registration widths
 * (zero =>120) remain untouched until an explicit user/programmatic operation.
 * User widths24..4096. Reset NULL resets all columns; otherwise one stable ID.
 * Fit measures the title and currently cached cells only; it never fetches data
 * to measure it. Changes preserve rows/selection/drafts and requery the viewport. */
UI_API ui_status_t ui_component_get_column_width(const ui_component_t *, const char *, int *);
UI_API ui_status_t ui_component_set_column_width(ui_component_t *, const char *, int);
UI_API ui_status_t ui_component_fit_column(ui_component_t *, const char *);
UI_API ui_status_t ui_component_reset_columns(ui_component_t *, const char *);
/* Caller-owned bytes/storage, format1, max2640 bytes, stable component/column
 * IDs. Save NULL reports required size; restore validates atomically. Future
 * formats UNSUPPORTED; malformed/wrong component INVALID_ARGUMENT. Missing
 * saved columns ignored, new columns use initial widths. Load after register,
 * before or after mount. Source/sort changes keep widths, instances isolate. */
UI_API ui_status_t ui_component_save_columns(const ui_component_t *, void *, size_t, size_t *);
UI_API ui_status_t ui_component_restore_columns(ui_component_t *, const void *, size_t);
UI_API ui_status_t ui_component_set_field(ui_component_t *, const char *, const ui_cell_t *);
/* Borrowed strings until next field update/unregister. Returns the current draft. */
UI_API ui_status_t ui_component_get_field(const ui_component_t *, const char *, ui_cell_t *);
UI_API ui_status_t ui_component_set_error(ui_component_t *, const char *, const char *);
/* Accept successful submission: current drafts become committed values. */
UI_API ui_status_t ui_component_accept_fields(ui_component_t *);
UI_API ui_status_t ui_component_set_text(ui_component_t *, const char *);
/* API9. Copied display metadata only: no source query, generation change,
 * column auto-fit, registration, draft/selection reset or object recreation. */
UI_API ui_status_t ui_component_set_title(ui_component_t *,const char *);
UI_API ui_status_t ui_component_set_column_title(ui_component_t *,const char *,const char *);
typedef struct ui_field_text {
    uint32_t size;
    const char *id,*title,*unit,*group,*help;
    /* Display labels parallel registered options, which remain semantic values.
     * NULL preserves labels; otherwise count must exactly match registration. */
    const char *const *option_labels;
    size_t option_count;
} ui_field_text_t;
/* NULL strings preserve metadata, empty strings clear it. Whole update validates
 * and allocates before changing anything. Complete size required. UI-thread. */
UI_API ui_status_t ui_component_set_field_text(ui_component_t *,const ui_field_text_t *);
/* Borrowed until the status component changes or the host is destroyed. */
UI_API const char *ui_host_status_text(const ui_host_t *);
UI_API ui_status_t ui_component_thumbnail(ui_component_t *, const ui_thumbnail_result_t *);
UI_API ui_status_t ui_component_set_visible(ui_component_t *, int);
/* API9 development append: independent full-size descriptors; existing public
 * structures/ABI are unchanged. Requires headers/library from the same commit.
 * All dimensions are logical client pixels at 96 DPI. Zero selects defaults:
 * preferred width 440, preferred height measured from fields; min 240x180;
 * maximum is the monitor work area. Positive minima >=160x120; positive
 * maxima intersect that work area.
 * Set before first show; later explicit sets resize the retained dialog. Content
 * updates/reopen retain user size, drafts and focus. UI-thread, DIALOG only. */
typedef struct ui_dialog_layout {
    uint32_t size;
    int preferred_width, preferred_height;
    int min_width, min_height, max_width, max_height;
} ui_dialog_layout_t;
UI_API ui_status_t ui_component_set_dialog_layout(ui_component_t *, const ui_dialog_layout_t *);
UI_API ui_status_t ui_component_get_dialog_layout(const ui_component_t *, ui_dialog_layout_t *);
/* FORM/DIALOG multiline text only. id is read during the call; height overrides visible_rows.
 * Zero/zero restores four visible rows. Range rows 0..100, height 0..32767;
 * positive height >=42. Container scrolling and field scrolling are separate.
 * Layout changes retain value, readonly state and the content view. UI-thread. */
typedef struct ui_field_layout {
    uint32_t size;
    const char *id;
    int visible_rows, height;
} ui_field_layout_t;
UI_API ui_status_t ui_component_set_field_layout(ui_component_t *, const ui_field_layout_t *);
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
UI_API ui_status_t ui_workspace_post_component_selection(ui_workspace_t *, uint64_t,
    const char *, uint64_t generation, uint64_t request, const uint64_t *, size_t);
UI_API ui_status_t ui_workspace_set_component_queue_limit(ui_workspace_t *, uint64_t, size_t);
#ifdef __cplusplus
}
#endif
#endif
