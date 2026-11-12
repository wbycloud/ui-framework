#ifndef UI_FRAMEWORK_AI_MODEL_H
#define UI_FRAMEWORK_AI_MODEL_H
#include "ai_service.h"
#ifdef __cplusplus
extern "C" {
#endif
/* RPAB-1. UI-thread methods. Inputs copied; OK means received, not saved.
 * No secret input, arbitrary endpoint, authority assertion or Provider registry. */
typedef struct ui_ai_model_profile {
 uint32_t size,version;
 const char *profile_id,*protocol_profile,*origin,*model_id,*credential_ref;
 uint64_t revision;
} ui_ai_model_profile_t;
typedef struct ui_ai_real_task {
 uint32_t size,version;
 const char *client_request_id,*input_utf8,*profile_id;
 uint64_t profile_revision;
 const ui_cap_target_t *targets;
 size_t target_count;
 const char *history_selection_json,*disclosure_projection_json;
} ui_ai_real_task_t;
/* NULL path selects the user configuration; embedded hosts may isolate it. */
UI_API ui_status_t ui_ai_model_open(ui_ai_service_t *,const char *path_utf8);
UI_API ui_status_t ui_ai_model_configure(ui_ai_service_t *,const ui_ai_model_profile_t *,const char *operation_id);
UI_API ui_status_t ui_ai_model_get_state(ui_ai_service_t *,char **sanitized_json);
UI_API ui_status_t ui_ai_real_propose(ui_ai_service_t *,const ui_ai_real_task_t *,char run_id[65]);
UI_API ui_status_t ui_ai_real_continue(ui_ai_service_t *,const char *old_run_id,const ui_ai_real_task_t *,char run_id[65]);
#ifdef __cplusplus
}
#endif
#endif
