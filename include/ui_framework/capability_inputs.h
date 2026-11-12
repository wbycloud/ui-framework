#ifndef UI_FRAMEWORK_CAPABILITY_INPUTS_H
#define UI_FRAMEWORK_CAPABILITY_INPUTS_H
#include "ui_framework/capability_ui.h"
#include "ui_framework/components.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef enum ui_cap_input_kind {
    UI_CAP_INPUT_SHORTCUT=1, UI_CAP_INPUT_TOOLBAR=2,
    UI_CAP_INPUT_LIGHT_CONTROL=3, UI_CAP_INPUT_COMPONENT=4,
    UI_CAP_INPUT_SURFACE=5, UI_CAP_INPUT_NATIVE_BUTTON=6
} ui_cap_input_kind_t;
typedef struct ui_cap_input_context {
    uint32_t size, version;
    ui_cap_ui_context_t invocation;
    ui_input_event_t event;
    const char *control_id, *value_utf8;
} ui_cap_input_context_t;
typedef struct ui_cap_input_binding_desc {
    uint32_t size, version;
    ui_cap_input_kind_t kind;
    ui_web_view_t *view;
    ui_component_t *component;
    ui_surface_t *surface;
    void *native_button;
    const char *control_id;
    ui_input_kind_t trigger;
    uint32_t key, modifiers, pointer_button;
    const char *capability_id;
    ui_cap_resource_ref_v2_t resource;
    ui_cap_ui_purpose_t purpose;
    uint64_t reconcile_call_id;
    ui_status_t (*collect)(const ui_cap_input_context_t *, char *, size_t,
        size_t *, void *);
    void (*observe)(const ui_cap_ui_notice_t *, void *);
    void *user_data;
} ui_cap_input_binding_desc_t;
typedef struct ui_workspace ui_workspace_t;
UI_API ui_status_t ui_capability_workspace_input_bind(ui_workspace_t *,ui_web_view_t *);
UI_API ui_status_t ui_capability_workspace_input_unbind(ui_web_view_t *);
UI_API ui_status_t ui_capability_input_bind(ui_host_t *,
    const ui_cap_input_binding_desc_t *, uint64_t *binding_id);

#ifdef __cplusplus
}
#endif
#endif
