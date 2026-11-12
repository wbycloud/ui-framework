#ifndef UI_FRAMEWORK_CAPABILITY_V2_H
#define UI_FRAMEWORK_CAPABILITY_V2_H
#include "capability.h"
#ifdef __cplusplus
extern "C" {
#endif
/* C2A-1; UI thread only. See docs/xuanshu-p2c2a-interface-and-acceptance.md. */
typedef enum ui_cap_outcome_v2 {
    UI_CAP2_OK = 0, UI_CAP2_REJECTED = 1, UI_CAP2_FAILED = 2,
    UI_CAP2_CANCELLED = 3, UI_CAP2_STALE = 4, UI_CAP2_UNKNOWN = 5
} ui_cap_outcome_v2_t;

typedef enum ui_cap_effect_v2 {
    UI_CAP2_EFFECT_NONE = 0, UI_CAP2_EFFECT_APPLIED = 1,
    UI_CAP2_EFFECT_UNKNOWN = 2
} ui_cap_effect_v2_t;

typedef enum ui_cap_resolution_v2 {
    UI_CAP2_UNRESOLVED = 0, UI_CAP2_RESOLVED_APPLIED = 1,
    UI_CAP2_RESOLVED_NOT_APPLIED = 2
} ui_cap_resolution_v2_t;

typedef struct ui_cap_resource_ref_v2 {
    uint32_t size, version;
    uint64_t instance_id;
    ui_cap_resource_kind_t kind;
    char host_epoch[37], resource_id[256], open_epoch[37];
} ui_cap_resource_ref_v2_t;

typedef struct ui_cap_target_v2 {
    uint32_t size, version;
    ui_cap_resource_ref_v2_t resource;
    char expected_revision[128];
    char scope_json[4097];
} ui_cap_target_v2_t;

typedef struct ui_cap_completion_v2 {
    uint32_t size, version;
    ui_cap_outcome_v2_t outcome;
    ui_cap_effect_v2_t effect;
    const char *output_json;
} ui_cap_completion_v2_t;

typedef struct ui_cap_result_v2 {
    uint32_t size, version;
    ui_cap_call_state_t state;
    ui_cap_outcome_v2_t outcome;
    ui_cap_effect_v2_t effect;
    uint32_t cancel_requested;
    char code[64];
    char revision_before[128], revision_after[128];
    ui_cap_resolution_v2_t resolution;
    char reconciled_revision[128], evidence_sha256[65];
    const char *output_json;
} ui_cap_result_v2_t;

typedef struct ui_cap_reconciliation_v2 {
    uint32_t size, version;
    ui_cap_resolution_v2_t resolution;
    char observed_revision[128];
} ui_cap_reconciliation_v2_t;

typedef struct ui_capability_desc_v2 {
    uint32_t size, version, capability_version;
    const char *id, *name_utf8, *description_utf8, *category;
    const char *input_schema_json, *output_schema_json;
    const char *reconcile_schema_json;
    ui_cap_resource_kind_t target_kind;
    const char *effect;
    ui_status_t (*check)(const ui_cap_target_v2_t *,
        const char *params_json, void *user_data);
    void (*execute)(uint64_t call_id, const ui_cap_target_v2_t *,
        const char *params_json, void *user_data);
    void (*cancel)(uint64_t call_id, void *user_data);
    ui_status_t (*reconcile)(uint64_t call_id,
        const ui_cap_target_v2_t *original_target,
        const char *original_params_json,
        ui_cap_reconciliation_v2_t *result,
        char *evidence_json, size_t capacity, size_t *length,
        void *user_data);
    void *user_data;
} ui_capability_desc_v2_t;
UI_API ui_status_t ui_capability_register_v2(
    ui_host_t *, const ui_capability_desc_v2_t *);
UI_API ui_status_t ui_capability_discover_v2(
    ui_host_t *, const char *category, size_t offset, size_t limit,
    uint64_t *generation, char **page_json);
UI_API ui_status_t ui_capability_describe_v2(
    ui_host_t *, const char *capability_id, char **json);
UI_API ui_status_t ui_capability_resource_ref_v2(
    ui_host_t *, const char *resource_id, ui_cap_resource_ref_v2_t *);
UI_API ui_status_t ui_capability_complete_v2(
    ui_host_t *, uint64_t call_id, const ui_cap_completion_v2_t *);
UI_API ui_status_t ui_capability_get_result_v2(
    ui_host_t *, uint64_t call_id, ui_cap_result_v2_t *);
UI_API ui_status_t ui_capability_cancel_v2(ui_host_t *, uint64_t call_id);
UI_API ui_status_t ui_capability_forget_v2(ui_host_t *, uint64_t call_id);

#ifdef __cplusplus
}
#endif
#endif
