#ifndef UI_FRAMEWORK_AI_OPERATION_H
#define UI_FRAMEWORK_AI_OPERATION_H
#include "capability_edit.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Optional application-owned task declaration. Register after the resource,
 * observation and reversible P4 actions. Input is copied on the UI thread.
 * Registration grants no authority. The host registry owns it until teardown.
 * See docs/xuanshu-operation-tasks-contract.md for the version 1 JSON format. */
UI_API ui_status_t ui_ai_operation_register(ui_host_t *,const char *declaration_json);
/* Active declarations as a copied JSON array, released with ui_capability_release. */
UI_API ui_status_t ui_ai_operation_list(ui_host_t *,char **json);

#ifdef __cplusplus
}
#endif
#endif
