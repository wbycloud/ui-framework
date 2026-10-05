#ifndef UI_FRAMEWORK_SHELL_H
#define UI_FRAMEWORK_SHELL_H

#include "ui_framework/ui.h"
#include "ui_framework/native.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_shell ui_shell_t;
typedef struct ui_content_slot ui_content_slot_t;

/* Creates the compatibility shell used by the Web workspace: real content
 * containers and a detached menu mirror, without native toolbar/tag chrome. */
UI_API ui_native_shell_t *ui_native_shell_create_web(
    const ui_native_shell_config_t *config);

/* Borrowed UI-thread objects. Slots remain valid until their shell is destroyed.
 * A NULL panel_id selects the main content slot. Registered panel IDs select
 * their content containers. A host without a shell returns NULL. */
UI_API ui_shell_t *ui_host_get_shell(ui_host_t *host);
UI_API ui_content_slot_t *ui_shell_get_content_slot(ui_shell_t *shell,
                                                   const char *panel_id);

/* Rectangles are relative to the application's host, in logical and physical
 * pixels respectively. Floating slots report their content size at origin 0,0.
 * The borrowed native container must not be destroyed by the application. */
UI_API ui_status_t ui_content_slot_get_rect(const ui_content_slot_t *slot,
                                            ui_rect_t *rect);
UI_API ui_status_t ui_content_slot_get_pixel_rect(const ui_content_slot_t *slot,
                                                  ui_rect_t *rect);
UI_API void *ui_content_slot_native_handle(const ui_content_slot_t *slot);

/* Bind one borrowed surface or Web view; NULL detaches that content type.
 * Binding never transfers ownership. Destroy contents during unmount.
 * Surfaces must belong to this slot's host. A panel Web backend must use the
 * slot's native container as its parent; its view receives local coordinates.
 * Main-slot Web backends use the host's native parent and host coordinates. */
UI_API ui_status_t ui_content_slot_attach_surface(ui_content_slot_t *slot,
                                                  ui_surface_t *surface);
UI_API ui_status_t ui_content_slot_attach_web_view(ui_content_slot_t *slot,
                                                   ui_web_view_t *view);

/* Update registered chrome without destroying existing content containers.
 * This differs from the legacy ui_native_shell_refresh() rebuild operation. */
UI_API ui_status_t ui_shell_refresh(ui_shell_t *shell);

/* Optional outputs. Docked frame/content rectangles use host coordinates;
 * floating content is local to its owned popup. Called on the UI thread. */
UI_API ui_status_t ui_shell_get_slot_state(ui_shell_t *shell, const char *panel_id,
    ui_rect_t *frame_rect, ui_rect_t *content_rect, int *visible, int *floating);

/* API6. UI-thread, stable registered panel IDs. Logical dimensions; floating
 * origins are screen coordinates scaled by the recorded DPI. No ownership
 * transfer and no remount. Offscreen floating is a logical rectangle. */
typedef struct ui_panel_layout {
    uint32_t size;
    ui_layout_region_t dock_region;
    int floating, collapsed, closed, order, height;
    ui_rect_t floating_rect;
    uint32_t dpi;
} ui_panel_layout_t;
UI_API ui_status_t ui_shell_get_panel_layout(ui_shell_t *, const char *, ui_panel_layout_t *);
UI_API ui_status_t ui_shell_set_panel_layout(ui_shell_t *, const char *, const ui_panel_layout_t *);
/* Caller owns bytes/storage. NULL buffer queries required size. Binary format
 * v1, little endian, bounded 64 KiB; validates entire input before applying.
 * Restore after registration/mount; unknown IDs skipped, new panels unchanged. */
UI_API ui_status_t ui_shell_save_layout(ui_shell_t *, void *, size_t, size_t *);
UI_API ui_status_t ui_shell_restore_layout(ui_shell_t *, const void *, size_t);
UI_API ui_status_t ui_shell_reset_layout(ui_shell_t *);
/* NULL panel: sidebar width; panel: height of that and the next docked panel.
 * Clamps to existing sidebar limits and available space. */
UI_API ui_status_t ui_shell_resize_splitter(ui_shell_t *, ui_layout_region_t, const char *, int);
/* Windowed pointer gesture; Esc/capture loss cancels and restores initial sizes. */
UI_API ui_status_t ui_shell_begin_splitter_drag(ui_shell_t *, ui_layout_region_t, const char *);
/* Host-relative logical pointer coordinates. Preview borrows no objects.
 * Commit reparents existing content; cancel preserves the original layout. */
UI_API ui_status_t ui_shell_begin_panel_drag(ui_shell_t *, const char *);
UI_API ui_status_t ui_shell_update_panel_drag(ui_shell_t *, int, int, ui_rect_t *, ui_layout_region_t *);
UI_API ui_status_t ui_shell_end_panel_drag(ui_shell_t *, int);

#ifdef __cplusplus
}
#endif

#endif
