#ifndef UI_FRAMEWORK_AI_BUSINESS_VALIDATION_H
#define UI_FRAMEWORK_AI_BUSINESS_VALIDATION_H
#include "ai_validation.h"
#ifdef __cplusplus
extern "C" {
#endif
/* BR-2 restrictive policy, UI thread, before any task. Copied JSON, no grant.
 * NULL/invalid/unverified policy installs a stopped guard, never ordinary mode.
 * State/stop use ui_ai_validation_get_state/stop. Same DB cannot rearm on restart. */
UI_API ui_status_t ui_ai_business_validation_enable(ui_ai_service_t *,const char *policy_json);
#ifdef __cplusplus
}
#endif
#endif
