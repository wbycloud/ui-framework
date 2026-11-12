#ifndef UI_FRAMEWORK_CAPABILITY_EDIT_EXTENSIONS_H
#define UI_FRAMEWORK_CAPABILITY_EDIT_EXTENSIONS_H
#include "capability_edit.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef enum ui_cap_edit_execution_policy {
 UI_EDIT_APPROVED_DESTRUCTIVE=1, UI_EDIT_POLICY_ONLY=2
} ui_cap_edit_execution_policy_t;
typedef struct ui_cap_edit_extension {
 uint32_t size,version;
 ui_cap_edit_desc_t edit;
 ui_cap_edit_execution_policy_t execution_policy;
 const char *side_effect;
} ui_cap_edit_extension_t;
/* Same directory/handler. Policy-only preserves pre-existing trusted manual
 * UI effects; AI approvals are consumed but never execute an external effect. */
UI_API ui_status_t ui_capability_register_edit_extension(ui_host_t *,const ui_cap_edit_extension_t *);
#ifdef __cplusplus
}
#endif
#endif
