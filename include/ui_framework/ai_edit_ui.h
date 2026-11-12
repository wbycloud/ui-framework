#ifndef UI_FRAMEWORK_AI_EDIT_UI_H
#define UI_FRAMEWORK_AI_EDIT_UI_H
#include "ai_edit.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct ui_ai_edit_light_binding {
 uint32_t size,version;
 ui_web_view_t *view;
 const char *control_id;
 ui_ai_edit_action_t action;
 const char *binding_hash;
} ui_ai_edit_light_binding_t;
/* Binds a displayed Service snapshot to the existing workspace Light origin
 * chain. No public function accepts a human/source/approved assertion.
 * Control/hash are copied; view is borrowed until unbind or framework detach.
 * Binding IDs share the original 16-entry budget and unbind_button lifecycle. */
UI_API ui_status_t ui_ai_edit_bind_target(ui_ai_service_t *,uint64_t instance_id,const char *resource_id,ui_cap_target_v2_t *);
UI_API ui_status_t ui_ai_edit_bind_light(ui_ai_service_t *,const ui_ai_edit_light_binding_t *,uint64_t *binding_id);
#ifdef __cplusplus
}
#endif
#endif
