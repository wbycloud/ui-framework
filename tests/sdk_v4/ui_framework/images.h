#ifndef UI_FRAMEWORK_IMAGES_H
#define UI_FRAMEWORK_IMAGES_H
#include "ui_framework/ui.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef uint64_t ui_image_id_t;
/* Top-down straight-alpha RGBA8; positive stride >= width*4, bytes covers
 * all rows. UI-thread create/update copy; caller frees input after return. */
typedef struct ui_rgba_desc {
    uint32_t size, width, height;
    size_t stride, bytes;
    const uint8_t *pixels;
} ui_rgba_desc_t;
typedef struct ui_image_info {
    uint32_t size, width, height;
    uint64_t version;
    size_t bytes;
} ui_image_info_t;
typedef struct ui_image_stats {
    uint32_t size;
    size_t bytes, limit, count, evictions; /* pixels + resource/thumbnail metadata */
} ui_image_stats_t;
UI_API ui_status_t ui_image_create(ui_host_t *, const ui_rgba_desc_t *, ui_image_id_t *);
UI_API ui_status_t ui_image_update(ui_host_t *, ui_image_id_t, const ui_rgba_desc_t *);
UI_API ui_status_t ui_image_release(ui_host_t *, ui_image_id_t);
UI_API ui_status_t ui_image_get_info(ui_host_t *, ui_image_id_t, ui_image_info_t *);
UI_API ui_status_t ui_image_get_stats(const ui_host_t *, ui_image_stats_t *);
UI_API ui_status_t ui_image_set_limit(ui_host_t *, size_t bytes);
/* WIC decode on Windows. ID owned by framework; release with image_release. */
UI_API ui_status_t ui_image_load_png(ui_host_t *, const void *, size_t, ui_image_id_t *);
struct ui_app_context;
UI_API ui_status_t ui_image_load_resource(const struct ui_app_context *, const char *, ui_image_id_t *);
typedef enum ui_line_style { UI_LINE_SOLID = 0, UI_LINE_DASH = 1, UI_LINE_DOT = 2 } ui_line_style_t;
/* Colors 0xRRGGBBAA. Zero spacing disables dots; nonzero image overrides fill. */
typedef struct ui_fill_style {
    uint32_t size, fill_rgba, border_rgba;
    int border_width;
    ui_line_style_t line_style;
    uint32_t dot_rgba;
    int dot_spacing, dot_radius;
    ui_image_id_t image_id;
} ui_fill_style_t;
/* A nonzero style.image_id returns that existing borrowed ID. Otherwise a new
 * resource is created. Release the original image only when all uses end. */
UI_API ui_status_t ui_image_create_preview(ui_host_t *, const ui_fill_style_t *,
    uint32_t width, uint32_t height, ui_image_id_t *);
#ifdef __cplusplus
}
#endif
#endif
