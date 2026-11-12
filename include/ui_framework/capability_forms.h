#ifndef UI_FRAMEWORK_CAPABILITY_FORMS_H
#define UI_FRAMEWORK_CAPABILITY_FORMS_H
#include "capability_ui.h"
#include "components.h"
#ifdef __cplusplus
extern "C" {
#endif
/* C2B-1: UI thread, copied binding, borrowed callback buffers. */
typedef struct ui_cap_form_projection {
    uint32_t size, version;
    const char *revision;
    const char *fields_json;
    size_t length;
} ui_cap_form_projection_t;

typedef struct ui_cap_form_field_state {
    uint32_t size, version;
    uint64_t generation;
    uint32_t modified, conflict;
    char base_revision[128], model_revision[128];
} ui_cap_form_field_state_t;

typedef struct ui_cap_form_state {
    uint32_t size, version;
    uint64_t pending_call;
    uint32_t dirty_fields, conflict_fields;
    char base_revision[128], model_revision[128];
} ui_cap_form_state_t;

typedef struct ui_cap_form_binding_desc {
    uint32_t size, version;
    const char *component_id, *capability_id;
    ui_cap_resource_ref_v2_t resource;
    ui_status_t (*read_model)(char revision[128], char *fields_json,
        size_t capacity, size_t *length, void *user_data);
    void (*observe)(const ui_cap_ui_notice_t *, void *user_data);
    void *user_data;
} ui_cap_form_binding_desc_t;

UI_API ui_status_t ui_capability_form_bind(ui_host_t *,
    const ui_cap_form_binding_desc_t *, uint64_t *binding_id);
UI_API ui_status_t ui_capability_form_project(ui_component_t *,
    const ui_cap_form_projection_t *);
UI_API ui_status_t ui_capability_form_get_field_state(ui_component_t *,
    const char *field_id, ui_cap_form_field_state_t *);
UI_API ui_status_t ui_capability_form_get_state(ui_component_t *,
    ui_cap_form_state_t *);

#ifdef __cplusplus
}
#endif
#endif
