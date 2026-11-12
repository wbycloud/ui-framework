#ifndef UI_FRAMEWORK_CAPABILITY_COMPONENT_INPUTS_H
#define UI_FRAMEWORK_CAPABILITY_COMPONENT_INPUTS_H
#include "capability_inputs.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef enum ui_cap_component_selector {
    UI_CAP_CELL_EDIT=1, UI_CAP_ROW_RENAME=2, UI_CAP_ROW_SELECT=3,
    UI_CAP_COLUMN_SORT=4, UI_CAP_ROW_CONTEXT=5, UI_CAP_CELL_TOGGLE=6
} ui_cap_component_selector_t;
typedef struct ui_cap_component_input_desc {
    uint32_t size,version;
    ui_component_t *component;
    ui_cap_component_selector_t selector;
    const char *capability_id;
    ui_cap_resource_ref_v2_t resource;
    ui_status_t (*collect)(const ui_cap_input_context_t *,char *,size_t,size_t *,void *);
    void (*observe)(const ui_cap_ui_notice_t *,void *);
    void *user_data;
} ui_cap_component_input_desc_t;
typedef struct ui_cap_editor_state {
    uint32_t size,version;
    uint64_t generation;
    uint32_t conflict;
    char base_revision[128];
} ui_cap_editor_state_t;
/* CI-1; same catalog/binding lifecycle and original budgets. No execution API. */
UI_API ui_status_t ui_capability_component_input_bind(ui_host_t *,
    const ui_cap_component_input_desc_t *,uint64_t *binding_id);
UI_API ui_status_t ui_capability_component_editor_state(ui_host_t *,
    uint64_t binding_id,const char *node_id,ui_cap_editor_state_t *);
#ifdef __cplusplus
}
#endif
#endif
