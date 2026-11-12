#ifndef UI_FRAMEWORK_AI_MODEL_VIEW_H
#define UI_FRAMEWORK_AI_MODEL_VIEW_H
#include "ai_model_ui.h"
#ifdef __cplusplus
extern "C" {
#endif
/* UH-A version 1 copied disclosure JSON, UI thread, release with ui_ai_release.
 * Only a saved, current, unconfirmed proposal can be projected. No authority,
 * I/O or secret is obtained. PENDING means the context is not yet available. */
UI_API ui_status_t ui_ai_model_get_disclosure(ui_ai_service_t *,const char *run_id,char **json);
/* Restriction only: never supplies human authority. UI callback is borrowed until
 * safe unbind/detach and runs without the Service lock, before and after target
 * callbacks. The original binding still requires qualified human input. */
typedef ui_status_t (*ui_ai_model_display_check_fn)(void *,const char *run_id,const char *binding_hash,uint64_t generation);
typedef struct ui_ai_model_display_binding {
 uint32_t size,version;
 ui_ai_model_light_binding_t binding;
 uint64_t generation;
 ui_ai_model_display_check_fn check;
 void *user_data;
} ui_ai_model_display_binding_t;
UI_API ui_status_t ui_ai_model_bind_light_display(ui_ai_service_t *,const ui_ai_model_display_binding_t *,uint64_t *);
#ifdef __cplusplus
}
#endif
#endif
