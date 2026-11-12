#ifndef UI_FRAMEWORK_AI_COORDINATOR_H
#define UI_FRAMEWORK_AI_COORDINATOR_H
#include "ai_edit_ui.h"
#ifdef __cplusplus
extern "C" {
#endif
/* P5-1, UI-thread, deep-copy input. Pure role data NEVER grants authority. */
typedef struct ui_ai_agent_profile {
 uint32_t size,version;
 const char *id,*instructions_utf8,*allowed_capability_ids_json;
} ui_ai_agent_profile_t;
UI_API ui_status_t ui_ai_agent_profile_register(ui_host_t *,const ui_ai_agent_profile_t *);
UI_API ui_status_t ui_ai_agent_profile_unregister(ui_host_t *,const char *);
UI_API ui_status_t ui_ai_agent_profile_get(ui_host_t *,const char *,char **json);
/* Copied profile metadata for the host's bounded application task chooser. */
UI_API ui_status_t ui_ai_agent_profile_list(ui_host_t *,char **json);
typedef struct ui_ai_graph_request {
 uint32_t size,version;
 const char *client_request_id,*input_utf8,*graph_json;
 const ui_cap_target_v2_t *targets;size_t target_count;
} ui_ai_graph_request_t;
typedef enum ui_ai_graph_action {UI_AI_GRAPH_CONFIRM=1,UI_AI_GRAPH_APPROVE,
 UI_AI_GRAPH_RECONCILE,UI_AI_GRAPH_MIGRATE} ui_ai_graph_action_t;
typedef struct ui_ai_graph_light_binding {
 uint32_t size,version;ui_web_view_t *view;
 const char *control_id,*run_id,*node_id,*binding_hash;
 ui_ai_graph_action_t action;
} ui_ai_graph_light_binding_t;
/* enable checks storage asynchronously; schema migration requires a fresh
 * classified host action. No method accepts an approved/source boolean. */
UI_API ui_status_t ui_ai_coordinator_enable(ui_ai_service_t *);
UI_API ui_status_t ui_ai_coordinator_get_state(ui_ai_service_t *,char **json);
UI_API ui_status_t ui_ai_graph_submit(ui_ai_service_t *,const ui_ai_graph_request_t *,char run_id[65]);
UI_API ui_status_t ui_ai_graph_get(ui_ai_service_t *,const char *run_id,char **json);
UI_API ui_status_t ui_ai_graph_cancel(ui_ai_service_t *,const char *run_id);
UI_API ui_status_t ui_ai_graph_answer(ui_ai_service_t *,const char *run_id,const char *node_id,
 const char *question_id,const char *operation_id,const char *answer_utf8);
UI_API ui_status_t ui_ai_graph_continue(ui_ai_service_t *,const char *old_run_id,
 const ui_ai_graph_request_t *,char new_run_id[65]);
UI_API ui_status_t ui_ai_graph_bind_light(ui_ai_service_t *,const ui_ai_graph_light_binding_t *,uint64_t *);
UI_API ui_status_t ui_ai_graph_unbind(ui_ai_service_t *,uint64_t);
#ifdef __cplusplus
}
#endif
#endif
