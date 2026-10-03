#ifndef UI_FRAMEWORK_WEBVIEW2_H
#define UI_FRAMEWORK_WEBVIEW2_H

#include <stdint.h>
#include <wchar.h>

#include "ui_framework/ui.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque configuration; no WebView2 or COM interface is exposed here. */
typedef struct ui_webview2_backend_config {
    uint32_t size;
    /* Fallback HWND; a host's native_parent takes precedence for each view. */
    void *parent_window;
    const wchar_t *user_data_folder;
} ui_webview2_backend_config_t;

/*
 * Creates a backend backed by the WebView2 Runtime installed on Windows.
 * The optional status receives UI_STATUS_UNSUPPORTED when the runtime or
 * loader is unavailable, and UI_STATUS_PLATFORM_ERROR for COM/initialization
 * failures. The caller owns the returned backend and must use the matching
 * destroy function after all views have been destroyed.
 */
UI_API ui_web_backend_t *ui_webview2_backend_create(
    const ui_webview2_backend_config_t *config,
    ui_status_t *status);
UI_API void ui_webview2_backend_destroy(ui_web_backend_t *backend);

/* Performs a cheap installed-runtime check without creating a view. */
UI_API ui_status_t ui_webview2_runtime_status(void);

/* The UI thread must pump its Win32 message loop for asynchronous creation.
 * ready includes completion of the trusted ui.invoke / ui.value bridge.
 * load_html is for trusted application HTML. External navigation is blocked.
 * ui.invoke(command, params) posts a semantic command asynchronously to the
 * host; params may be a JSON string or a JavaScript object. It returns void.
 * Script results are borrowed UTF-8 JSON strings, valid only in the callback.
 */
typedef void (*ui_webview2_script_callback_fn)(ui_status_t status,
                                               const char *result_json_utf8,
                                               void *user_data);
UI_API ui_status_t ui_webview2_view_get_state(ui_web_view_t *view,
                                               int *ready,
                                               int *navigation_completed);
UI_API ui_status_t ui_webview2_view_execute_script(
    ui_web_view_t *view,
    const char *script_utf8,
    ui_webview2_script_callback_fn callback,
    void *user_data);
/* Last asynchronous HRESULT as a platform-independent 32-bit diagnostic. */
UI_API ui_status_t ui_webview2_view_get_error(ui_web_view_t *view,
                                             int32_t *hresult);
/* Queries the actual controller bounds in parent-relative device pixels.
 * Returns NOT_FOUND while the controller is still being created. */
UI_API ui_status_t ui_webview2_view_get_bounds(ui_web_view_t *view,
                                               ui_rect_t *pixel_rect);

#ifdef __cplusplus
}
#endif

#endif
