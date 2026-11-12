#ifndef UI_FRAMEWORK_AI_MODEL_UI_H
#define UI_FRAMEWORK_AI_MODEL_UI_H
#include "ai_model.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef enum ui_ai_model_action {UI_AI_MODEL_CONFIRM=1,UI_AI_MODEL_KEY=2} ui_ai_model_action_t;
typedef struct ui_ai_model_light_binding {
 uint32_t size,version;
 ui_web_view_t *view;
 const char *control_id,*run_id,*binding_hash;
 ui_ai_model_action_t action;
} ui_ai_model_light_binding_t;
typedef struct ui_ai_model_button_binding {
 uint32_t size,version;
 void *button;
 const char *run_id,*binding_hash;
 ui_ai_model_action_t action;
} ui_ai_model_button_binding_t;
/* Only a matching framework-qualified input can consume a binding.
 * view borrowed until unbind/detach; strings copied; 16 bindings maximum. */
UI_API ui_status_t ui_ai_model_bind_light(ui_ai_service_t *,const ui_ai_model_light_binding_t *,uint64_t *);
UI_API ui_status_t ui_ai_model_unbind(ui_ai_service_t *,uint64_t);
UI_API ui_status_t ui_ai_model_bind_button(ui_ai_service_t *,const ui_ai_model_button_binding_t *,uint64_t *);
#ifdef __cplusplus
}
#endif
#endif
