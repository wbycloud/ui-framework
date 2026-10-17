#ifndef UI_FRAMEWORK_THEME_H
#define UI_FRAMEWORK_THEME_H
#include "ui_framework/ui.h"
#ifdef __cplusplus
extern "C" {
#endif
/* API9 development addition; independent full-size structure, ABI/package1.
 * Per host, initially LIGHT for compatibility. SYSTEM is explicit opt-in and
 * resolves Windows UISettings foreground luminance, falling back to LIGHT if unavailable.
 * All colors are opaque 0xRRGGBBAA, matching ui_fill_style_t. */
typedef enum ui_theme_mode { UI_THEME_LIGHT=0, UI_THEME_DARK=1,
    UI_THEME_SYSTEM=2 } ui_theme_mode_t;
typedef struct ui_theme_info {
    uint32_t size;
    ui_theme_mode_t mode;
    uint64_t generation;
    ui_theme_mode_t effective; /* Always LIGHT or DARK. */
    uint32_t system_theme_available;
    uint32_t background_rgba, surface_rgba, text_rgba, muted_text_rgba;
    uint32_t border_rgba, focus_rgba, selection_rgba, selection_text_rgba;
    uint32_t hover_rgba, pressed_rgba, disabled_background_rgba, disabled_text_rgba;
    uint32_t progress_rgba, icon_rgba, accent_rgba, error_rgba;
} ui_theme_info_t;
typedef void (*ui_theme_callback_fn)(ui_host_t *,const ui_theme_info_t *,void *);
/* UI-thread and complete size required. Setter copies the mode; getter copies
 * the complete palette. Notification is synchronous once per mode/effective
 * change, under the existing dispatch guard. Components/shell are updated
 * before callback, then ui.host.theme_changed is emitted to workspace observers.
 * Runtime rendering remains asynchronous; query presentation normally.
 * Query and resource updates are allowed in callback; recursive set/refresh is
 * CANCELLED. Callback info is borrowed only for its call. NULL removes it even
 * during close; host destruction discards it. A destroyed host pointer is invalid.
 * Caller-owned native/Web/GL drawing and icons must apply the returned palette
 * themselves; no injected script, resource replacement, or font/layout reset. */
UI_API ui_status_t ui_host_set_theme(ui_host_t *,ui_theme_mode_t);
UI_API ui_status_t ui_host_get_theme(const ui_host_t *,ui_theme_info_t *);
UI_API ui_status_t ui_host_set_theme_callback(ui_host_t *,ui_theme_callback_fn,void *);
/* Explicit system resample; no registry writes. Forward WM_SETTINGCHANGE /
 * WM_THEMECHANGED through ui_host_handle_message, or call this from an owner
 * whose nested/hidden hosts do not receive those messages. Explicit modes no-op. */
UI_API ui_status_t ui_host_refresh_theme(ui_host_t *);
#ifdef __cplusplus
}
#endif
#endif
