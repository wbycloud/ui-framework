#ifndef UI_FRAMEWORK_LIGHT_WEB_H
#define UI_FRAMEWORK_LIGHT_WEB_H

#include "ui_framework/ui.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The light backend intentionally implements a small, deterministic HTML
 * subset. It is useful for application chrome and assistant panels; it is not
 * a replacement for a general browser engine.
 */
typedef struct ui_light_web_config {
    uint32_t size;
    void *parent_hwnd;       /* HWND on Windows; may be NULL for headless use. */
    uint32_t narrow_width;   /* Reserved legacy field; HTML media rule sets breakpoint. */
    int enable_native_input;
} ui_light_web_config_t;

/* The returned pointer is the common ui_web_backend_t handle. */
UI_API ui_web_backend_t *ui_light_web_backend_create(
    const ui_light_web_config_t *config);
UI_API void ui_light_web_backend_destroy(ui_web_backend_t *backend);

/* Returns a short capability string owned by the library. */
UI_API const char *ui_light_web_backend_capabilities(void);

#ifdef __cplusplus
}
#endif

#endif
