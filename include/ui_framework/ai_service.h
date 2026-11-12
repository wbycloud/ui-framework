#ifndef UI_FRAMEWORK_AI_SERVICE_H
#define UI_FRAMEWORK_AI_SERVICE_H
#include "application.h"
#include "capability.h"
#ifdef __cplusplus
extern "C" {
#endif
/* P3-1. Workspace-owned, UI-thread API. All input data is copied before return.
 * No real Provider, AI write, approval or agent-spawn interface is exposed. */
typedef struct ui_ai_service ui_ai_service_t;
typedef enum ui_ai_service_phase {UI_AI_CREATED=0,UI_AI_OPENING,UI_AI_READY,
    UI_AI_QUIESCING,UI_AI_CLOSING,UI_AI_CLOSED,UI_AI_UNAVAILABLE} ui_ai_service_phase_t;
typedef enum ui_ai_run_phase {UI_AI_RECEIVED=0,UI_AI_QUEUED,UI_AI_RUNNING,
    UI_AI_WAITING_INPUT,UI_AI_WAITING_TOOL,UI_AI_CANCEL_REQUESTED,
    UI_AI_CANCELLATION_STALLED,UI_AI_FINISHED,UI_AI_NEEDS_REVIEW,UI_AI_UNCONFIRMED} ui_ai_run_phase_t;
typedef enum ui_ai_history_kind {UI_AI_MESSAGES=0,UI_AI_RUNS=1,UI_AI_EVENTS=2} ui_ai_history_kind_t;
typedef struct ui_ai_event {
    uint32_t size,version;
    const char *type,*conversation_id,*client_request_id,*run_id,*query_id;
    uint64_t sequence; /* 0: receipt/display/query snapshot, no new event sequence. */
    const char *payload; /* delta: valid UTF-8 text; other types: JSON. */
    int committed;
} ui_ai_event_t;
typedef void (*ui_ai_event_fn)(const ui_ai_event_t *,void *);
typedef struct ui_ai_service_config {
    uint32_t size,version;
    ui_workspace_t *workspace;
    const char *storage_path_utf8;
    ui_ai_event_fn on_event;
    void *user_data;
} ui_ai_service_config_t;
typedef struct ui_ai_task_request {
    uint32_t size,version;
    const char *client_request_id,*input_utf8,*stub_plan_json;
    const ui_cap_target_t *targets;
    size_t target_count;
} ui_ai_task_request_t;
typedef struct ui_ai_service_state {
    uint32_t size,version;
    ui_ai_service_phase_t phase;
    uint32_t queued,worker_alive;
    size_t bytes,event_bytes;
    char conversation_id[37],active_run_id[65],error[256];
} ui_ai_service_state_t;
#define UI_AI_MAX_QUEUED 16u
#define UI_AI_MAX_INPUT (16u*1024u)
#define UI_AI_MAX_PLAN (64u*1024u)
#define UI_AI_MAX_REPLY (64u*1024u)
#define UI_AI_MAX_BYTES (8u*1024u*1024u)
#define UI_AI_EVENT_BYTES (2u*1024u*1024u)
UI_API ui_status_t ui_ai_service_create(const ui_ai_service_config_t *,ui_ai_service_t **);
UI_API ui_status_t ui_ai_service_open(ui_ai_service_t *);
UI_API ui_status_t ui_ai_service_get_state(ui_ai_service_t *,ui_ai_service_state_t *);
/* At most 1..256 notices; may also advance one UI-side read operation.
 * Event payloads are borrowed until callback return. Nested poll is PENDING. */
UI_API ui_status_t ui_ai_service_poll(ui_ai_service_t *,uint32_t);
UI_API ui_status_t ui_ai_service_quiesce(ui_ai_service_t *);
UI_API ui_status_t ui_ai_service_resume(ui_ai_service_t *);
UI_API ui_status_t ui_ai_service_request_close(ui_ai_service_t *);
UI_API ui_status_t ui_ai_service_destroy(ui_ai_service_t *);
/* Writes run_id and associates it BEFORE the synchronous received callback.
 * OK is receipt only; accepted/existing committed event confirms storage.
 * Any failure retains caller input. Ordinary calls from callbacks backpressure. */
UI_API ui_status_t ui_ai_submit(ui_ai_service_t *,const ui_ai_task_request_t *,char run_id[65]);
/* Copied cache projection; NOT_FOUND can mean evicted. query_run accesses store. */
UI_API ui_status_t ui_ai_get_run(ui_ai_service_t *,const char *,char **json);
UI_API ui_status_t ui_ai_query_run(ui_ai_service_t *,const char *query_id,const char *run_id);
UI_API ui_status_t ui_ai_cancel(ui_ai_service_t *,const char *run_id);
UI_API ui_status_t ui_ai_answer(ui_ai_service_t *,const char *run_id,const char *question_id,
    const char *operation_id,const char *answer_utf8);
UI_API ui_status_t ui_ai_history(ui_ai_service_t *,const char *query_id,
    ui_ai_history_kind_t,uint64_t after,size_t limit);
/* New linked run, explicit fresh target bindings; never replay the old tools. */
UI_API ui_status_t ui_ai_continue(ui_ai_service_t *,const char *old_run_id,
    const ui_ai_task_request_t *,char new_run_id[65]);
UI_API void ui_ai_release(void *);
#ifdef __cplusplus
}
#endif
#endif
