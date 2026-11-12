#include "ui_framework/ai_assistant.h"
#include "ui_framework/ai_business_validation.h"
#include "ui_framework/ai_model_view.h"
#include "ui_framework/ai_validation_batch.h"
#include "ui_framework/ai_validation.h"
#include "ui_framework/ai_model.h"
#include "ui_framework/ai_model_ui.h"
#include "ui_framework/ai_coordinator.h"
#include "ui_framework/ui.h"
#include "ui_framework/native.h"
#include "ui_framework/opengl.h"
#include "ui_framework/assistant.h"
#include "ui_framework/light_web.h"
#include "ui_framework/webview2.h"
#include "ui_framework/components.h"
#include "ui_framework/images.h"
#include "ui_framework/menus.h"
#include "ui_framework/theme.h"
#include "ui_framework/capability.h"
#include "ui_framework/capability_entry.h"
#include "ui_framework/capability_forms.h"
#include "ui_framework/capability_inputs.h"
#include "ui_framework/capability_component_inputs.h"
#include "ui_framework/app_poll.h"
#include "ui_framework/ai_service.h"
#include "ui_framework/ai_edit.h"
#include "ui_framework/ai_edit_ui.h"
#include "ui_framework/capability_edit_extensions.h"
static_assert(offsetof(ui_ai_edit_request_t,client_request_id)==8 && offsetof(ui_cap_edit_desc_t,capability)==8,"P4 independent prefixes");
static_assert(offsetof(ui_ai_service_config_t,workspace)==8 && offsetof(ui_ai_task_request_t,client_request_id)==8,"P3 independent size/version prefix");
static_assert(offsetof(ui_cap_component_input_desc_t,component)==8,"CI-1 independent prefix");
static_assert(sizeof(ui_cap_input_context_t)==4672 && sizeof(ui_cap_input_binding_desc_t)==472,"C2C-1 independent layout");
static_assert(sizeof(ui_cap_entry_binding_t)==24 && offsetof(ui_cap_entry_binding_t,target)==16,"C1 independent binding layout");
static_assert(UI_FRAMEWORK_API_VERSION == 10 && UI_CAPABILITY_CONTRACT_VERSION == 1, "current contract");
static_assert(sizeof(ui_cap_resource_ref_v2_t)==352 && sizeof(ui_cap_target_v2_t)==4592,"C2A-1 target layout");
static_assert(sizeof(ui_capability_desc_v2_t)==128 && sizeof(ui_cap_completion_v2_t)==24 && sizeof(ui_cap_result_v2_t)==552,"C2A-1 descriptor/completion/result layout");
static_assert(sizeof(ui_cap_reconciliation_v2_t)==140 && sizeof(ui_cap_ui_context_t)==4608 && sizeof(ui_cap_ui_notice_t)==104 && sizeof(ui_cap_ui_binding_desc_t)==416,"C2A-1 UI layout");
static_assert(sizeof(ui_cap_form_projection_t)==32 && sizeof(ui_cap_form_field_state_t)==280 && sizeof(ui_cap_form_state_t)==280 && sizeof(ui_cap_form_binding_desc_t)==400,"C2B-1 layout");
int main()
{
    ui_host_config_t config{};
    if(ui_ai_business_validation_enable(NULL,NULL)!=UI_STATUS_INVALID_ARGUMENT)return 21;
    config.size = sizeof(config);
    config.api_version = UI_FRAMEWORK_API_VERSION;
    ui_host_t *host = ui_host_create(&config);
    if (!host) return 1;
    uint64_t ci_id=0;
    if(ui_capability_component_input_bind(host,nullptr,&ci_id)!=UI_STATUS_INVALID_ARGUMENT||ui_capability_component_editor_state(host,1,nullptr,nullptr)!=UI_STATUS_INVALID_ARGUMENT)return 19;
    char *canonical = nullptr;size_t length = 0;
    if(ui_cap_json_canonicalize("{}",2,&canonical,&length)!=UI_STATUS_OK||length!=2){ui_host_destroy(host);return 2;}
    ui_capability_release(canonical);
#ifdef UI_PUBLIC_WORKSPACE
    if(ui_app_set_poll_callback(nullptr,nullptr,nullptr)!=UI_STATUS_INVALID_ARGUMENT)return 21;
    if(ui_capability_workspace_input_bind(NULL,NULL)!=UI_STATUS_INVALID_ARGUMENT||ui_capability_workspace_input_unbind(NULL)!=UI_STATUS_INVALID_ARGUMENT)return 20;
#endif
    ui_host_destroy(host);
    return 0;
}

static_assert(offsetof(ui_cap_edit_extension_t,edit)==8,"P4BC independent prefix");
