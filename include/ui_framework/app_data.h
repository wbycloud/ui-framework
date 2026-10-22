#ifndef UI_FRAMEWORK_APP_DATA_H
#define UI_FRAMEWORK_APP_DATA_H
#include "ui_framework/application.h"
#ifdef __cplusplus
extern "C" {
#endif
/* API9 development addition, no existing structure changes. Copied binary data
 * for an app's own UI-thread receiver. Workers use only workspace + stable ID.
 * The instance's existing 8MiB component/message budget includes these bytes.
 * Unknown/closed IDs reject posts. Payload contains data, never callbacks or
 * borrowed object pointers. Receiver arguments are borrowed during callback.
 * Delivery continues during WAIT so the receiver can finish cleanup. ALLOW
 * means workers/callbacks are already finished; remaining copies are discarded.
 * Receiver is protected by workspace dispatch depth. Do not destroy/reload a
 * current Web view inside its callback; defer from Web input callbacks first. */
typedef void (*ui_app_data_fn)(uint64_t kind,const void *data,size_t bytes,void *user);
UI_API ui_status_t ui_app_set_data_callback(const ui_app_context_t *,ui_app_data_fn,void *);
UI_API ui_status_t ui_workspace_post_app_data(ui_workspace_t *,uint64_t instance_id,
    uint64_t kind,const void *data,size_t bytes);
#ifdef __cplusplus
}
#endif
#endif
