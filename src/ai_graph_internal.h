#ifndef UI_AI_GRAPH_INTERNAL_H
#define UI_AI_GRAPH_INTERNAL_H
#include "ui_framework/ai_coordinator.h"
#define AG_NODES 32
#define AG_RESULT 16384u
#define AG_CONTEXT 131072u
#define AG_RESULTS 65536u
enum {AG_RESERVED=0,AG_DEPENDENCY,AG_INPUT,AG_APPROVAL,AG_RUNNING,AG_COLLECTED};
typedef struct ag_node {
 char capability[256],id[65],operation[65],parent[65],profile[65],descriptor[65],profile_hash[65],context_hash[65];
 char question_id[65],answer_op[65],answer_hash[65],outcome[32],code[96],revision[128];
 char *params,*constraint,*instruction,*context,*question,*answer,*result,*delegation;
 size_t input_bytes;unsigned target,depth;uint32_t dependencies;int parent_index,state,write,risk,charged,settled,cancel_sent,fail,tool_phase;
 uint64_t call,last_call;ai_edit_t *edit;
 int undo_accepting,undo_running,undo_finished;
} ag_node_t;
struct ai_graph {
 ui_cap_target_v2_t *targets;size_t target_count,bytes,context_bytes,result_bytes;
 char *json;char binding[65];
 ag_node_t nodes[AG_NODES];unsigned count,agents,models,tools,steps,charged_agents,charged_models,charged_tools,charged_steps,active,peak;
 uint64_t version,persisted,scope_until;int admitted,record_only,confirmed,needs_scope,finishing,finished,failed,unknown,cancel,dirty,suspended;
};
typedef struct ag_binding {struct ag_binding *next;ui_ai_graph_light_binding_t d;uint64_t id;int used;char control[128],run[65],node[65],hash[65];
 ui_status_t (*display_check)(void *,const char *,const char *,uint64_t);
 void *display_data;uint64_t display_generation;int node_undo;
} ag_binding_t;
struct ai_coordinator {
 int schema,job,busy,done,ok,slot,failed,recovered,fixture_hold,fixture_fail;
 uint64_t sequence;char binding[65],backup[4096],error[256];
 char lookup_id[65],lookup_hash[65],lookup_client[65];
 /* Queue -> worker -> pending result -> UI owns exactly one run reference. */
 char run_id[65];int run_ref;
 char payload[AI_PAGE+1];ag_binding_t *bindings;
};
#endif
