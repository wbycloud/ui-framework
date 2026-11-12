#ifndef UI_FRAMEWORK_APP_POLL_H
#define UI_FRAMEWORK_APP_POLL_H
#include "application.h"
#ifdef __cplusplus
extern "C" {
#endif
/* AP-1, opt-in UI-thread callback. Context is borrowed until return. Called at
 * safe workspace polls (also while closing), never after unmount. No business
 * admission, worker waiting or automatic thread tracking is provided. Clearing
 * prevents future calls; in-flight data remains borrowed until callback return. */
typedef void (*ui_app_poll_fn)(const ui_app_context_t *,void *);
UI_API ui_status_t ui_app_set_poll_callback(const ui_app_context_t *,ui_app_poll_fn,void *);
#ifdef __cplusplus
}
#endif
#endif
