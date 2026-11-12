#ifndef UI_FRAMEWORK_AI_VALIDATION_BATCH_H
#define UI_FRAMEWORK_AI_VALIDATION_BATCH_H
#include "ai_validation.h"
#ifdef __cplusplus
extern "C" {
#endif
/* RPCB-1 only: fixed synthetic T2/T3, designated credential reference, no T1.
 * UI thread; copied inputs. Open/OK is not authorization or persistence. */
typedef struct ui_ai_validation_batch_desc {
 uint32_t size,version;
 const char *model_config_path_utf8;
 const ui_cap_target_t *target;
} ui_ai_validation_batch_desc_t;
typedef struct ui_ai_validation_batch_button {
 uint32_t size,version;
 void *button;
 const char *batch_hash;
} ui_ai_validation_batch_button_t;
UI_API ui_status_t ui_ai_validation_batch_open(ui_ai_service_t *,const ui_ai_validation_batch_desc_t *);
UI_API ui_status_t ui_ai_validation_batch_bind_button(ui_ai_service_t *,const ui_ai_validation_batch_button_t *,uint64_t *);
/* Copied redacted preview/state. Release with ui_ai_release. No confirm API. */
UI_API ui_status_t ui_ai_validation_batch_get_state(ui_ai_service_t *,char **);
#ifdef __cplusplus
}
#endif
#endif
