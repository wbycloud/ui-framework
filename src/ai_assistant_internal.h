#ifndef UI_AI_ASSISTANT_INTERNAL_H
#define UI_AI_ASSISTANT_INTERNAL_H
#include "ui_framework/ai_assistant.h"

#define AS_ATTACHMENTS 4
#define AS_ATTACHMENT_BYTES 16384u
#define AS_REQUESTS 16
enum { AS_PLANNING = 1, AS_RESPONSE, AS_PLAN_WAIT, AS_EXECUTING,
       AS_FRAME_QUEUED, AS_MODEL_RUNNING, AS_NODE_RESPONSE, AS_FINAL_RESPONSE, AS_DONE,
       AS_PREPARING, AS_NODE_RESULT_RESPONSE };
enum { AS_CONTROL_AI = 0, AS_CONTROL_DRAIN, AS_CONTROL_MANUAL, AS_CONTROL_INVALID };
enum { AS_NODE_NEW = 0, AS_NODE_MODEL, AS_NODE_READY, AS_NODE_QUESTION };
enum { AO_GOAL, AO_OBSERVE, AO_REVIEW, AO_NEXT, AO_APPLY, AO_VERIFY, AO_DONE };
#define AO_FRAME 4u
#define AO_PLAN_BYTES 8192u
#define AO_FACT_BYTES 32768u

typedef struct as_policy {
    unsigned requests, deadline_ms;
    uint64_t budget, input_rate, output_rate;
    char price_version[65], price_evidence[65], meter_profile[65], meter_evidence[65];
} as_policy_t;

typedef struct as_ticket {
    char request[65], hash[65];
    uint64_t estimate, input;
    unsigned ordinal,frame,node;
    int entered, ended, priced, unknown, simulated;
} as_ticket_t;

typedef struct as_operation {
    int phase,granted;
    unsigned limit,attempted,committed,verified;
    char plan[AO_PLAN_BYTES+1],facts[AO_FACT_BYTES+1],receipt_facts[AG_RESULT+1];
    char revision[128],next_revision[128],question[1025],readback[65];
    char capability[256];
} as_operation_t;

typedef struct as_run {
    as_policy_t policy;
    ui_cap_target_v2_t targets[16];
    size_t target_count, bytes;
    char *scope, *attachments;
    as_operation_t *operation;
    char candidate[65537], frame_input[AI_CONTEXT + 1], response[65537], final_result[65537];
    char node_summary[AG_NODES][1025];
    unsigned node_state[AG_NODES], used, frame, node;
    int node_analyzed[AG_NODES];
    int stage, control, stopped, fee_unknown, finish_ready;
    char error[96];
    uint64_t until, known, estimated;
    as_ticket_t tickets[AS_REQUESTS];
} as_run_t;

#ifdef UI_AI_ASSISTANT_IMPLEMENTATION
static void as_free(ui_ai_service_t *, as_run_t *);
static int as_worker(ui_ai_service_t *, ai_run_t *);
static void as_ui(ui_ai_service_t *);
static int as_node_ready(ui_ai_service_t *, ai_run_t *, unsigned);
static int as_collect(ui_ai_service_t *, ai_run_t *);
static int as_frame_complete(ui_ai_service_t *, ai_run_t *);
static int as_current(ui_ai_service_t *, ai_run_t *);
static int ao_current(ui_ai_service_t *,ai_run_t *);
static int ao_ready(ui_ai_service_t *,ai_run_t *);
static int ao_collect(ui_ai_service_t *,ai_run_t *);
static void ao_snapshot(ui_ai_service_t *,ai_run_t *,yyjson_mut_doc *,yyjson_mut_val *);
static int as_read_allowed(ai_run_t *,unsigned,const char *);
static char *as_initial_scope(am_run_t *);
static char *as_policy_json(as_run_t *);
static const char *as_instructions(am_run_t *,unsigned);
static int as_reserve(ui_ai_service_t *, ai_run_t *);
static int as_before_send(ui_ai_service_t *, ai_run_t *, int);
static void as_send_ack(ui_ai_service_t *, ai_run_t *);
static int as_price(ui_ai_service_t *, ai_run_t *);
static void as_network_end(ui_ai_service_t *, ai_run_t *);
static void as_confirm(ui_ai_service_t *, ai_run_t *);
static int as_policy_ready(as_policy_t *);
static ui_status_t as_undo_action(ui_ai_service_t *,ag_binding_t *);
static ui_status_t ag_build(ui_ai_service_t *, const ui_ai_graph_request_t *, ai_graph_t **);
static void ag_dirty(ai_graph_t *);
#endif
#endif
