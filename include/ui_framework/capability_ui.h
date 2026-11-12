#ifndef UI_FRAMEWORK_CAPABILITY_UI_H
#define UI_FRAMEWORK_CAPABILITY_UI_H
#include "capability_v2.h"
#ifdef __cplusplus
extern "C" {
#endif
/* C2A-1; UI thread only. See docs/xuanshu-p2c2a-interface-and-acceptance.md. */
typedef enum ui_cap_ui_purpose {
    UI_CAP_UI_INVOKE = 1, UI_CAP_UI_RECONCILE = 2
} ui_cap_ui_purpose_t;

typedef enum ui_cap_ui_stage {
    UI_CAP_UI_ADMITTED = 1, UI_CAP_UI_REJECTED = 2,
    UI_CAP_UI_FINISHED = 3, UI_CAP_UI_RECONCILED = 4
} ui_cap_ui_stage_t;

typedef struct ui_cap_ui_context {
    uint32_t size, version;
    uint64_t binding_id;
    ui_cap_target_v2_t target;
} ui_cap_ui_context_t;

typedef struct ui_cap_ui_notice {
    uint32_t size, version;
    uint64_t binding_id, call_id;
    ui_cap_ui_stage_t stage;
    ui_status_t status;
    char code[64];
    const char *evidence_json;
} ui_cap_ui_notice_t;

typedef struct ui_cap_ui_binding_desc {
    uint32_t size, version;
    const char *capability_id;
    ui_cap_resource_ref_v2_t resource;
    const char *menu_item_id;
    ui_cap_ui_purpose_t purpose;
    uint64_t reconcile_call_id;
    ui_status_t (*collect)(const ui_cap_ui_context_t *,
        char *params_json, size_t capacity, size_t *length,
        void *user_data);
    void (*observe)(const ui_cap_ui_notice_t *, void *user_data);
    void *user_data;
} ui_cap_ui_binding_desc_t;
UI_API ui_status_t ui_capability_ui_bind(
    ui_host_t *, const ui_cap_ui_binding_desc_t *, uint64_t *binding_id);
UI_API ui_status_t ui_capability_ui_begin_unbind(ui_host_t *, uint64_t binding_id);
UI_API ui_status_t ui_capability_ui_finish_unbind(ui_host_t *, uint64_t binding_id);

#ifdef __cplusplus
}
#endif
#endif
