#include "ui_framework/ai_assistant.h"
#include "ui_framework/ai_operation.h"
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
#include "ui_framework/language.h"
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
_Static_assert(offsetof(ui_ai_edit_request_t,client_request_id)==8 && offsetof(ui_cap_edit_desc_t,capability)==8,"P4 independent prefixes");
_Static_assert(offsetof(ui_ai_service_config_t,workspace)==8 && offsetof(ui_ai_task_request_t,client_request_id)==8,"P3 independent size/version prefix");
_Static_assert(offsetof(ui_cap_component_input_desc_t,component)==8,"CI-1 independent prefix");
_Static_assert(offsetof(ui_cap_editor_state_t,generation)==8,"CI-1 editor prefix");
_Static_assert(sizeof(ui_cap_input_context_t)==4672 && sizeof(ui_cap_input_binding_desc_t)==472,"C2C-1 independent layout");
_Static_assert(sizeof(ui_cap_entry_binding_t)==24 && offsetof(ui_cap_entry_binding_t,target)==16,"C1 independent binding layout");
#include <stddef.h>
_Static_assert(UI_FRAMEWORK_API_VERSION == 10, "current SDK");
_Static_assert(UI_CAPABILITY_CONTRACT_VERSION == 1, "independent capability contract");
_Static_assert(sizeof(ui_cap_target_t)==4576 && offsetof(ui_cap_target_t,scope_json)==478,"API10 target Windows x64 layout");
_Static_assert(sizeof(ui_cap_resource_desc_t)==48 && sizeof(ui_capability_desc_t)==112 && sizeof(ui_cap_result_t)==224,"API10 independent complete structures");
_Static_assert(sizeof(ui_theme_info_t)==88 && offsetof(ui_theme_info_t,background_rgba)==24 && offsetof(ui_theme_info_t,error_rgba)==84,"independent theme info ABI");
_Static_assert(sizeof(ui_language_info_t)==128 && offsetof(ui_language_info_t,generation)==8 && offsetof(ui_language_info_t,system_language)==32 && offsetof(ui_language_info_t,fallback)==120,"API9 language Windows x64 layout");
_Static_assert(sizeof(ui_field_text_t)==64 && offsetof(ui_field_text_t,option_labels)==48,"API9 text Windows x64 layout");
_Static_assert(offsetof(ui_field_desc_t, help) == 64, "appended complete help field");
_Static_assert(sizeof(ui_field_desc_t) == 72, "Windows x64 field description");
_Static_assert(offsetof(ui_column_desc_t, width) == 28, "column width offset unchanged");
_Static_assert(sizeof(ui_dialog_layout_t)==28 && offsetof(ui_dialog_layout_t,max_height)==24,"independent dialog layout ABI");
_Static_assert(sizeof(ui_field_layout_t)==24 && offsetof(ui_field_layout_t,visible_rows)==16,"independent field layout ABI");
_Static_assert(sizeof(ui_cap_resource_ref_v2_t)==352 && sizeof(ui_cap_target_v2_t)==4592,"C2A-1 target layout");
_Static_assert(sizeof(ui_capability_desc_v2_t)==128 && sizeof(ui_cap_completion_v2_t)==24 && sizeof(ui_cap_result_v2_t)==552,"C2A-1 descriptor/completion/result layout");
_Static_assert(sizeof(ui_cap_reconciliation_v2_t)==140 && sizeof(ui_cap_ui_context_t)==4608 && sizeof(ui_cap_ui_notice_t)==104 && sizeof(ui_cap_ui_binding_desc_t)==416,"C2A-1 UI layout");
_Static_assert(sizeof(ui_cap_form_projection_t)==32 && sizeof(ui_cap_form_field_state_t)==280 && sizeof(ui_cap_form_state_t)==280 && sizeof(ui_cap_form_binding_desc_t)==400,"C2B-1 layout");
int main(void)
{
    ui_host_config_t config = {0};
    ui_host_t *host;
    if(ui_ai_business_validation_enable(NULL,NULL)!=UI_STATUS_INVALID_ARGUMENT)return 21;
    config.size = sizeof(config);
    config.api_version = UI_FRAMEWORK_API_VERSION;
    host = ui_host_create(&config);
    if (!host) return 1;
    {
        ui_field_desc_t field = {0};ui_component_desc_t desc = {0};ui_component_t *form;
        field.size = 68;field.id = "partial";field.kind = UI_VALUE_TEXT;
        field.help = (const char *)(uintptr_t)1; /* Incomplete field must not be read. */
        desc.size = sizeof(desc);desc.id = "complete-fields";desc.kind = UI_COMPONENT_FORM;
        desc.fields = &field;desc.field_count = 1;
        if(ui_component_register(host,&desc,&form)!=UI_STATUS_OK)return 2;
    }
    {
        char *canonical = NULL;size_t length = 0;
        if(ui_cap_json_canonicalize("{}",2,&canonical,&length)!=UI_STATUS_OK||length!=2){ui_host_destroy(host);return 3;}
        ui_capability_release(canonical);
    }
    {
        uint64_t id=0,generation=0;char *json=NULL;ui_cap_resource_ref_v2_t ref={0};
        ref.size=sizeof(ref);ref.version=2;
        if(ui_capability_register_v2(host,NULL)!=UI_STATUS_INVALID_ARGUMENT)return 4;
        if(ui_capability_discover_v2(host,NULL,0,1,&generation,&json)!=UI_STATUS_OK)return 5;
        ui_capability_release(json);
        if(ui_capability_describe_v2(host,"missing",&json)!=UI_STATUS_NOT_FOUND)return 6;
        if(ui_capability_resource_ref_v2(host,"missing",&ref)!=UI_STATUS_NOT_FOUND)return 7;
        if(ui_capability_complete_v2(host,1,NULL)!=UI_STATUS_NOT_FOUND)return 8;
        if(ui_capability_get_result_v2(host,1,NULL)!=UI_STATUS_INVALID_ARGUMENT)return 9;
        if(ui_capability_cancel_v2(host,1)!=UI_STATUS_NOT_FOUND||ui_capability_forget_v2(host,1)!=UI_STATUS_NOT_FOUND)return 10;
        if(ui_capability_ui_bind(host,NULL,&id)!=UI_STATUS_INVALID_ARGUMENT)return 11;
        if(ui_capability_ui_begin_unbind(host,1)!=UI_STATUS_NOT_FOUND||ui_capability_ui_finish_unbind(host,1)!=UI_STATUS_NOT_FOUND)return 12;
        if(ui_capability_input_bind(host,NULL,&id)!=UI_STATUS_INVALID_ARGUMENT)return 16;
        if(ui_capability_component_input_bind(host,NULL,&id)!=UI_STATUS_INVALID_ARGUMENT)return 17;
        if(ui_capability_component_editor_state(host,1,NULL,NULL)!=UI_STATUS_INVALID_ARGUMENT)return 18;
        if(ui_capability_form_bind(host,NULL,&id)!=UI_STATUS_INVALID_ARGUMENT)return 13;
        if(ui_capability_form_project(NULL,NULL)!=UI_STATUS_INVALID_ARGUMENT)return 14;
        if(ui_capability_form_get_field_state(NULL,NULL,NULL)!=UI_STATUS_INVALID_ARGUMENT||ui_capability_form_get_state(NULL,NULL)!=UI_STATUS_INVALID_ARGUMENT)return 15;
    }
#ifdef UI_PUBLIC_WORKSPACE
    if(ui_app_set_poll_callback(NULL,NULL,NULL)!=UI_STATUS_INVALID_ARGUMENT)return 21;
    if(ui_capability_workspace_input_bind(NULL,NULL)!=UI_STATUS_INVALID_ARGUMENT||ui_capability_workspace_input_unbind(NULL)!=UI_STATUS_INVALID_ARGUMENT)return 20;
#endif
    ui_host_destroy(host);
    return 0;
}

_Static_assert(offsetof(ui_cap_edit_extension_t,edit)==8,"P4BC independent prefix");
