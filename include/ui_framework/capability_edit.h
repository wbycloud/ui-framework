#ifndef UI_FRAMEWORK_CAPABILITY_EDIT_H
#define UI_FRAMEWORK_CAPABILITY_EDIT_H
#include "capability_v2.h"
#ifdef __cplusplus
extern "C" {
#endif
/* P4A-1. Independent, complete version=1 structures; UI-thread API. */
typedef enum ui_cap_edit_risk {UI_EDIT_REVERSIBLE=0,UI_EDIT_DESTRUCTIVE=1,
    UI_EDIT_EXTERNAL=2,UI_EDIT_IRREVERSIBLE=3} ui_cap_edit_risk_t;
typedef enum ui_cap_edit_stage {UI_EDIT_BEGIN=1,UI_EDIT_EXECUTE=2,
    UI_EDIT_COMMIT=3,UI_EDIT_ROLLBACK=4,UI_EDIT_READBACK=5,
    UI_EDIT_UNDO=6,UI_EDIT_RECONCILE=7} ui_cap_edit_stage_t;
typedef struct ui_cap_edit_context {
    uint32_t size,version;
    uint64_t call_id,stage_id;
    ui_cap_edit_stage_t stage;
    ui_cap_target_v2_t target;
    char operation_id[65],transaction_id[65],undo_id[65];
} ui_cap_edit_context_t;
typedef struct ui_cap_edit_completion {
    uint32_t size,version;
    uint64_t stage_id;
    ui_status_t status;
    ui_cap_effect_v2_t effect;
    uint32_t executor_exited;
    char transaction_id[65],undo_id[65];
    const char *output_json;
} ui_cap_edit_completion_t;
typedef struct ui_cap_edit_desc {
    uint32_t size,version;
    ui_capability_desc_v2_t capability; /* the only business execute */
    ui_cap_edit_risk_t risk;
    void (*transaction)(const ui_cap_edit_context_t *,void *user_data);
} ui_cap_edit_desc_t;
/* One atomic registration in the existing directory; duplicate IDs reject. */
UI_API ui_status_t ui_capability_register_edit(ui_host_t *,const ui_cap_edit_desc_t *);
UI_API ui_status_t ui_capability_edit_get_stage(ui_host_t *,uint64_t,ui_cap_edit_context_t *);
/* Copies output/receipts. Callers keep workers/external callbacks alive until
 * they have actually exited; executor_exited is a cooperative application proof. */
UI_API ui_status_t ui_capability_edit_complete(ui_host_t *,uint64_t,const ui_cap_edit_completion_t *);
#ifdef __cplusplus
}
#endif
#endif
