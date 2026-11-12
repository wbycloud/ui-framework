#ifndef UI_FRAMEWORK_AI_VALIDATION_H
#define UI_FRAMEWORK_AI_VALIDATION_H
#include "ai_model.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Restrictive RPCP-1 policy only. Does not grant disclosure or credential access.
 * UI thread; copied inputs. Enable is asynchronously persisted before ready. */
typedef enum ui_ai_validation_task { UI_AI_VALIDATION_TEXT=1, UI_AI_VALIDATION_READ=2, UI_AI_VALIDATION_CANCEL=3 } ui_ai_validation_task_t;
typedef struct ui_ai_validation_desc {
 uint32_t size,version;
 const char *validation_id;
 uint64_t input_nano_per_token,output_nano_per_token;
} ui_ai_validation_desc_t;
typedef struct ui_ai_validation_request {
 uint32_t size,version;
 ui_ai_validation_task_t task;
 const char *client_request_id;
 const ui_cap_target_t *target;
} ui_ai_validation_request_t;
UI_API ui_status_t ui_ai_validation_enable(ui_ai_service_t *,const ui_ai_validation_desc_t *);
UI_API ui_status_t ui_ai_validation_get_state(ui_ai_service_t *,char **json);
UI_API ui_status_t ui_ai_validation_propose(ui_ai_service_t *,const ui_ai_validation_request_t *,char run_id[65]);
UI_API ui_status_t ui_ai_validation_stop(ui_ai_service_t *);
#ifdef __cplusplus
}
#endif
#endif
