#ifndef UI_FRAMEWORK_AI_ASSISTANT_H
#define UI_FRAMEWORK_AI_ASSISTANT_H
#include "ai_model_view.h"
#include "ai_coordinator.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Independent v1 extension. Existing model, graph and Service layouts stay intact.
 * Enable before opening the same Service. Only a new database starts at the
 * existing schema 3. Existing databases retain their explicit migration gates. */
typedef struct ui_ai_assistant_policy {
    uint32_t size, version;
    uint32_t request_limit, deadline_ms;
    uint64_t budget_nano_usd, input_rate_nano_usd, output_rate_nano_usd;
    const char *price_version, *price_evidence_sha256;
    const char *meter_profile, *meter_evidence_sha256;
} ui_ai_assistant_policy_t;

typedef struct ui_ai_assistant_attachment {
    uint32_t size, version;
    const char *name_utf8, *media_type;
    const void *bytes;
    size_t byte_count;
} ui_ai_assistant_attachment_t;

typedef struct ui_ai_assistant_request {
    uint32_t size, version;
    ui_ai_real_task_t task;
    const ui_cap_target_v2_t *targets;
    size_t target_count;
    /* Array in target order. Each item has capabilities and profiles arrays,
     * with optional per-target fields and capability-keyed parameters fixed by
     * a completed form. Omitted fields retain the task's read projection.
     * Per-target fields restrict read-tool results as well as specialist evidence. */
    const char *permissions_json;
    const ui_ai_assistant_attachment_t *attachments;
    size_t attachment_count;
    const char *parent_run_id;
} ui_ai_assistant_request_t;

typedef enum ui_ai_assistant_control {
    UI_AI_ASSISTANT_TAKEOVER = 1,
    UI_AI_ASSISTANT_RETURN = 2
} ui_ai_assistant_control_t;

typedef struct ui_ai_assistant_display_binding {
    uint32_t size, version;
    ui_ai_graph_light_binding_t binding;
    uint64_t generation;
    ui_ai_model_display_check_fn check;
    void *user_data;
} ui_ai_assistant_display_binding_t;

typedef struct ui_ai_assistant_undo_binding {
    uint32_t size, version;
    ui_web_view_t *view;
    const char *control_id, *run_id, *node_id, *binding_hash;
    uint64_t generation;
    ui_ai_model_display_check_fn check;
    void *user_data;
} ui_ai_assistant_undo_binding_t;

UI_API ui_status_t ui_ai_assistant_enable(ui_ai_service_t *, const ui_ai_assistant_policy_t *);
UI_API ui_status_t ui_ai_assistant_catalog(ui_ai_service_t *, uint64_t instance_id, char **json);
/* Pure bounded identity check. Caller owns any durable local blob/preview. PNG
 * stays local-only. Text bytes are copied into the admitted task snapshot. */
UI_API ui_status_t ui_ai_assistant_attachment_hash(const ui_ai_assistant_attachment_t *, char sha256[65]);
UI_API ui_status_t ui_ai_assistant_propose(ui_ai_service_t *, const ui_ai_assistant_request_t *, char run_id[65]);
UI_API ui_status_t ui_ai_assistant_get(ui_ai_service_t *, const char *run_id, char **json);
UI_API ui_status_t ui_ai_assistant_get_disclosure(ui_ai_service_t *, const char *run_id, char **json);
UI_API ui_status_t ui_ai_assistant_control(ui_ai_service_t *, const char *run_id, ui_ai_assistant_control_t);
UI_API ui_status_t ui_ai_assistant_bind_light_display(ui_ai_service_t *, const ui_ai_assistant_display_binding_t *, uint64_t *);
/* New explicit local P4 undo, with its own trusted source and displayed receipt.
 * It neither resumes a root task nor grants model authority. */
UI_API ui_status_t ui_ai_assistant_bind_node_undo(ui_ai_service_t *, const ui_ai_assistant_undo_binding_t *, uint64_t *);

/* Stable, explicitly named local account identity, independent of a task DB.
 * NULL path selects the daily location. No legacy configuration is migrated.
 * The isolated runtime requires an explicit path. No secret is returned. */
UI_API ui_status_t ui_ai_model_open_profile(ui_ai_service_t *, const char *path_utf8, const char *account_id);

#ifdef __cplusplus
}
#endif
#endif
