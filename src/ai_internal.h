#ifndef UI_AI_INTERNAL_H
#define UI_AI_INTERNAL_H
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ui_framework/ai_service.h"
#include "sqlite3.h"
#include "yyjson.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct ai_edit ai_edit_t;
typedef struct ai_graph ai_graph_t;
typedef struct av_policy av_policy_t;
typedef struct am_run am_run_t;
typedef struct am_config am_config_t;
typedef struct ai_coordinator ai_coordinator_t;
typedef struct as_policy as_policy_t;
typedef struct as_run as_run_t;
#define AI_RUNS 64
#define AI_COMMANDS 32
#define AI_EVENTS 64
#define AI_PAGE (512u*1024u)
#define AI_CONTEXT (128u*1024u)
#define AI_CONTROL (256u*1024u)
typedef struct ai_run {
    ai_graph_t *graph;am_run_t *model;
    char id[65],client[65],hash[65],parent[65],question[65],answer_op[65],answer_hash[65];
    char *input,*plan,*request,*targets_json,*reply,*answer,*question_text;
    ui_cap_target_t *targets;size_t target_count,bytes,reply_length;
    ui_ai_run_phase_t phase;int committed,terminal,cancel,stalled,cancel_recorded,answer_ready,refs;
    uint64_t order,cancel_at;
    unsigned step,model_calls,tool_calls,steps;
    char outcome[32],error[256],tool_ids[64][65];
    unsigned tool_id_count;
} ai_run_t;
typedef struct ai_command {int kind,run;char query[65],id[65];uint64_t after;size_t limit;ui_ai_history_kind_t history;} ai_command_t;
typedef struct ai_notice {ui_ai_event_t event;char conversation[37],client[65],run[65],query[65],type[40];char *payload;size_t bytes;} ai_notice_t;
enum {AI_TOOL_NONE=0,AI_TOOL_PREPARE,AI_TOOL_PREPARED,AI_TOOL_DISPATCH,AI_TOOL_RUNNING,AI_TOOL_RESULT};
typedef struct ai_tool {
    int state,run,discover,cancel_sent;unsigned target_index;uint64_t call;
    char id[65],capability[256],category[128],result_revision[128];
    char *params,*result,*target_json;ui_status_t status;ui_cap_target_t target;
} ai_tool_t;
struct ui_ai_service {
    ui_ai_service_config_t config;DWORD thread;CRITICAL_SECTION lock;CONDITION_VARIABLE changed;
    HANDLE worker;sqlite3 *db;char *path;void *control;
    ui_ai_service_phase_t phase;char conversation[37],session[37],error[256];
    int opened,close_requested,callbacks,polling,methods,active,answer_owner;uint64_t order,time_bias;
    size_t bytes,event_bytes;unsigned queued;
    ai_run_t runs[AI_RUNS];ai_command_t commands[AI_COMMANDS];unsigned cmd_head,cmd_count;
    ai_notice_t *events[AI_EVENTS];unsigned event_head,event_count;
    ai_tool_t tool;
    ai_edit_t *edit;am_config_t *model_config;av_policy_t *validation;
    ai_coordinator_t *coordinator;
    as_policy_t *assistant_policy;
    unsigned store_commits,store_checkpoints;
    volatile LONG fixture_crash,fixture_checkpoint;
    unsigned fixture_worker_yields;int fixture_worker_yield;
    int fixture_fail_write,fixture_hold,store_failed;
#ifdef UI_AI_FIXTURE
    int fixture_tool_pause,fixture_tool_seen;
#endif
};
ui_status_t ui_workspace_ai_claim(ui_workspace_t *,ui_ai_service_t *);
void ui_workspace_ai_release(ui_workspace_t *,ui_ai_service_t *);
static void ai_graph_cancelled(ui_ai_service_t *,ai_graph_t *);
static int ai_graph_continue_valid(ui_ai_service_t *,ai_run_t *);
static ui_status_t ai_graph_request_hash(const char *,const char *,const char *,const char *,char [65]);
static char *ai_graph_targets_json(ai_graph_t *);
static int ai_graph_io(ui_ai_service_t *);
static void ai_graph_ui(ui_ai_service_t *);
static int ai_graph_pending(ui_ai_service_t *);
static int ai_graph_jobs(ui_ai_service_t *);
static void ai_graph_free(ui_ai_service_t *,ai_graph_t *);
static int ai_graph_retain(ai_graph_t *);
static void ai_graph_dispose(ui_ai_service_t *);
static int ai_emit(ui_ai_service_t *,const char *,ai_run_t *,const char *,uint64_t,int,const char *,int);
static void ai_store_error(ui_ai_service_t *,ai_run_t *,const char *);
static int am_config_io(ui_ai_service_t *);
static int am_tick(ui_ai_service_t *,ai_run_t *);
static void am_ui(ui_ai_service_t *);
static int am_pending(ui_ai_service_t *);
static void am_free(ui_ai_service_t *,ai_run_t *);
static void am_dispose(ui_ai_service_t *);
#endif
