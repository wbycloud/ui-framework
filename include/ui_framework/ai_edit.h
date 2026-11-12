#ifndef UI_FRAMEWORK_AI_EDIT_H
#define UI_FRAMEWORK_AI_EDIT_H
#include "ai_service.h"
#include "capability_edit.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef enum ui_ai_edit_action {UI_AI_EDIT_CONFIRM=1,UI_AI_EDIT_APPROVE=2,
    UI_AI_EDIT_UNDO=3,UI_AI_EDIT_RECONCILE=4,UI_AI_EDIT_MIGRATE=5} ui_ai_edit_action_t;
typedef enum ui_ai_edit_phase {
    UI_AI_EDIT_IDLE=0,UI_AI_EDIT_SCOPE_WAIT,UI_AI_EDIT_APPROVAL_WAIT,
    UI_AI_EDIT_GATE,UI_AI_EDIT_INTENT_WAIT,UI_AI_EDIT_BEGIN,
    UI_AI_EDIT_EXECUTE,UI_AI_EDIT_COMMIT,UI_AI_EDIT_ROLLBACK,
    UI_AI_EDIT_READBACK,UI_AI_EDIT_RECORD,UI_AI_EDIT_DONE,
    UI_AI_EDIT_UNKNOWN,UI_AI_EDIT_PROOF,UI_AI_EDIT_BASELINE,
    UI_AI_EDIT_RECEIVED
} ui_ai_edit_phase_t;
#define UI_AI_EDIT_SCOPE_MS UINT64_C(1800000)
#define UI_AI_EDIT_APPROVAL_MS UINT64_C(300000)
typedef struct ui_ai_edit_request {
    uint32_t size,version;
    const char *client_request_id,*capability_id;
    ui_cap_target_v2_t target;
    const char *parameter_constraint_schema,*params_json;
} ui_ai_edit_request_t;
/* Inputs are copied. OK from propose is receipt, not durable acceptance.
 * RECEIVED becomes SCOPE_WAIT only after storage confirmation. Cached duplicate
 * conflicts return ALREADY_EXISTS; database-only conflicts are asynchronously
 * rejected (code=request_id_conflict, rejected=true, storage_confirmed=false).
 * get_state returns copied JSON released with ui_ai_release; output is JSON,
 * call/instance IDs decimal strings. No method accepts approved/source flags. */
UI_API ui_status_t ui_ai_edit_enable(ui_ai_service_t *);
UI_API ui_status_t ui_ai_edit_propose(ui_ai_service_t *,const ui_ai_edit_request_t *,char id[65]);
UI_API ui_status_t ui_ai_edit_get_state(ui_ai_service_t *,char **json);
UI_API ui_status_t ui_ai_edit_cancel(ui_ai_service_t *,const char *id);
UI_API ui_status_t ui_ai_edit_forget(ui_ai_service_t *,const char *id);
/* Bind the displayed snapshot hash to a framework-controlled native button.
 * Only the private, classified input chain can consume the host action.
 * Same-process cooperation, not a malicious-module isolation guarantee. */
UI_API ui_status_t ui_ai_edit_bind_button(ui_ai_service_t *,void *native_button,
    ui_ai_edit_action_t,const char *binding_hash,uint64_t *binding_id);
UI_API ui_status_t ui_ai_edit_unbind_button(ui_ai_service_t *,uint64_t binding_id);
#ifdef __cplusplus
}
#endif
#endif
