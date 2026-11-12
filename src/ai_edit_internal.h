#ifndef UI_AI_EDIT_INTERNAL_H
#define UI_AI_EDIT_INTERNAL_H
#include "ui_framework/ai_edit_ui.h"
#include "capability_edit_internal.h"
enum {AE_CHECK=1,AE_MIGRATE,AE_PROPOSE,AE_SCOPE,AE_INTENT,AE_FINAL,AE_PROOF};
enum {AE_IDLE=0,AE_SCOPE_WAIT,AE_APPROVAL_WAIT,AE_GATE,AE_INTENT_WAIT,
      AE_BEGIN,AE_EXECUTE,AE_COMMIT,AE_ROLLBACK,AE_READBACK,AE_RECORD,
      AE_DONE,AE_UNKNOWN,AE_RECONCILE,AE_BASELINE,AE_PROPOSE_WAIT};
typedef struct ai_edit_button ai_edit_button_t;
typedef struct ai_edit {
    ui_ai_service_t *service;
    int rejected,live,schema,job,busy,io_done,io_ok,phase,cancel,started,committed,rolled_back,undo,reconciling,recovered;
    int proposal_confirmed,host_status,execution_policy,risk,approval,scope,call_finished,holding,fixture_hold,fixture_fail,fixture_pause;
    uint64_t call,scope_until,approval_until,button_sequence,sequence,cancel_at;
    ui_cap_edit_stage_t stage;
    ui_cap_effect_v2_t effect;
    ui_cap_target_v2_t target;
    char id[65],client[65],capability[256],hash[65],descriptor[65],binding[65],side_effect[64];
    char transaction[65],undo_id[65],revision_after[128],code[96];
    char params[UI_CAPABILITY_INPUT_BYTES+1u],constraint[65537];
    char output[UI_CAPABILITY_OUTPUT_BYTES+1u],payload[AI_PAGE+1u];
    char before_hash[65];
    char backup[4096],storage_path[4096];
    ai_edit_button_t *buttons;
} ai_edit_t;
static void ai_edit_ui(ui_ai_service_t *);
static int ai_edit_io(ui_ai_service_t *);
static int ai_edit_pending(ui_ai_service_t *);
static void ai_edit_dispose(ui_ai_service_t *);
static void ai_edit_quiesce(ui_ai_service_t *);
#endif
