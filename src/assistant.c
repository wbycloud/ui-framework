#include "assistant_internal.h"
#include "ui_internal.h"
#include "ui_framework/capability_entry.h"

#include <stdlib.h>
#include <string.h>

static char *assistant_strdup(const char *value)
{
    size_t length;
    char *copy;

    if (value == NULL) {
        value = "";
    }
    length = strlen(value);
    copy = (char *)malloc(length + 1u);
    if (copy != NULL) {
        memcpy(copy, value, length + 1u);
    }
    return copy;
}

static int valid_size(uint32_t size, size_t expected)
{
    return size >= expected;
}

static int valid_permission(ui_assistant_permission_t permission)
{
    return permission >= UI_ASSISTANT_PERMISSION_READ &&
           permission <= UI_ASSISTANT_PERMISSION_DESTRUCTIVE;
}

/* Separate from capability callbacks: these leases protect activation and
 * host lifetime, without making the subsequent generated dispatch nested. */
static void callback_enter(ui_host_t *host)
{++host->assistant_callbacks;ui_dispatch_enter(host);}
static void callback_leave(ui_host_t *host)
{--host->assistant_callbacks;ui_dispatch_leave(host);}

static ui_status_t legacy_policy(ui_assistant_t *assistant,ui_assistant_command_t *command)
{
    if(assistant->host->capability_read_entries)
        return command->permission==UI_ASSISTANT_PERMISSION_READ?UI_STATUS_NOT_FOUND:UI_STATUS_PERMISSION_DENIED;
    return assistant->host->dispatch_blocked?UI_STATUS_NOT_FOUND:UI_STATUS_OK;
}

static ui_assistant_command_t *find_command(ui_assistant_t *assistant,
                                            const char *id)
{
    ui_assistant_command_t *command;

    for (command = assistant->commands; command != NULL;
         command = command->next) {
        if (strcmp(command->id, id) == 0) {
            return command;
        }
    }
    return NULL;
}

int ui_assistants_have_id(const ui_host_t *host,const char *id)
{
    ui_assistant_t *a;
    for(a=host->assistants;a;a=a->host_next)if(find_command(a,id))return 1;
    return 0;
}
void ui_assistants_detach_host(ui_host_t *host)
{ui_assistant_t *p;while((p=host->assistants)!=NULL){host->assistants=p->host_next;p->host=NULL;p->host_next=NULL;}}

static ui_assistant_request_t *find_request(
    ui_assistant_t *assistant,
    uint64_t request_id,
    ui_assistant_request_t **previous)
{
    ui_assistant_request_t *request;
    ui_assistant_request_t *last = NULL;

    for (request = assistant->requests; request != NULL;
         request = request->next) {
        if (request->id == request_id) {
            if (previous != NULL) {
                *previous = last;
            }
            return request;
        }
        last = request;
    }
    return NULL;
}

ui_assistant_t *ui_assistant_create(ui_host_t *host,
                                    const ui_assistant_config_t *config)
{
    ui_assistant_t *assistant;
    ui_assistant_permission_t max_permission = UI_ASSISTANT_PERMISSION_READ;
    const char *source_id = "assistant";

    if (host == NULL) {
        return NULL;
    }
    if(host->capability_read_entries&&!ui_language_thread_ok(host))return NULL;
    if (config != NULL && !valid_size(config->size, sizeof(*config))) {
        return NULL;
    }
    if (config != NULL) {
        max_permission = config->max_permission;
        source_id = config->source_id != NULL ? config->source_id : source_id;
        if (!valid_permission(max_permission)) {
            return NULL;
        }
    }

    assistant = (ui_assistant_t *)calloc(1u, sizeof(*assistant));
    if (assistant == NULL) {
        return NULL;
    }
    assistant->host = host;
    assistant->max_permission = max_permission;
    assistant->source_id = assistant_strdup(source_id);
    if (assistant->source_id == NULL) {
        free(assistant);
        return NULL;
    }
    if (config != NULL) {
        assistant->user_data = config->user_data;
        assistant->confirm = config->confirm;
        assistant->progress = config->progress;
        assistant->cancel = config->cancel;
        assistant->snapshot = config->snapshot;
        assistant->transaction_begin = config->transaction_begin;
        assistant->transaction_commit = config->transaction_commit;
        assistant->transaction_rollback = config->transaction_rollback;
        assistant->transaction_undo = config->transaction_undo;
    }
    assistant->host_next=host->assistants;host->assistants=assistant;
    return assistant;
}

void ui_assistant_destroy(ui_assistant_t *assistant)
{
    ui_assistant_command_t *command;
    ui_assistant_request_t *request;

    if (assistant == NULL) {
        return;
    }
    if(assistant->host&&assistant->host->app_poll_callbacks)return;
    if(assistant->host&&assistant->host->capability_read_entries&&!ui_language_thread_ok(assistant->host))return;
    if(assistant->host){ui_assistant_t **p;for(p=&assistant->host->assistants;*p&&*p!=assistant;p=&(*p)->host_next){}if(*p)*p=assistant->host_next;}
    while (assistant->commands != NULL) {
        command = assistant->commands;
        assistant->commands = command->next;
        free(command->id);
        free(command->params_schema_json);
        free(command);
    }
    while (assistant->requests != NULL) {
        request = assistant->requests;
        assistant->requests = request->next;
        if(request->generated&&assistant->host)ui_capabilities_release_request(assistant->host);
        free(request);
    }
    free(assistant->source_id);
    free(assistant);
}

ui_status_t ui_assistant_register_command(
    ui_assistant_t *assistant,
    const ui_assistant_command_desc_t *desc)
{
    ui_assistant_command_t *command;

    if (assistant == NULL || desc == NULL ||
        !valid_size(desc->size, sizeof(*desc)) || desc->id == NULL ||
        desc->id[0] == '\0' || !valid_permission(desc->permission)) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    if(assistant->host->capability_read_entries&&!ui_language_thread_ok(assistant->host))return UI_STATUS_INVALID_ARGUMENT;
    if (find_command(assistant, desc->id) != NULL || ui_capabilities_has_id(assistant->host,desc->id)) {
        return UI_STATUS_ALREADY_EXISTS;
    }
    if(assistant->host->capability_read_entries)return UI_STATUS_PERMISSION_DENIED;

    command = (ui_assistant_command_t *)calloc(1u, sizeof(*command));
    if (command == NULL) {
        return UI_STATUS_OUT_OF_MEMORY;
    }
    command->id = assistant_strdup(desc->id);
    command->params_schema_json = assistant_strdup(desc->params_schema_json);
    command->permission = desc->permission;
    command->validate = desc->validate;
    command->user_data = desc->user_data;
    if (command->id == NULL || command->params_schema_json == NULL) {
        free(command->id);
        free(command->params_schema_json);
        free(command);
        return UI_STATUS_OUT_OF_MEMORY;
    }
    command->next = assistant->commands;
    assistant->commands = command;
    return UI_STATUS_OK;
}

ui_status_t ui_assistant_invoke(ui_assistant_t *assistant,
                                const char *command_id,
                                const char *params_json,
                                uint64_t *request_id)
{
    ui_assistant_command_t *command;
    ui_assistant_request_t *request;
    ui_status_t validation_status;
    uint64_t host_request_id;
    const char *params = params_json != NULL ? params_json : "{}";
    int confirmation_required;
    int generated;

    if (request_id != NULL) {
        *request_id = 0u;
    }
    if (assistant == NULL || command_id == NULL || command_id[0] == '\0' ||
        request_id == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    if(assistant->host->capability_read_entries&&!ui_language_thread_ok(assistant->host))return UI_STATUS_INVALID_ARGUMENT;
    if(ui_capabilities_nonread(assistant->host,command_id))return UI_STATUS_PERMISSION_DENIED;
    command = find_command(assistant, command_id);
    generated=assistant->host->capability_read_entries&&ui_capabilities_entry_active(assistant->host,command_id);
    if(assistant->host->capability_read_entries&&!generated){
        return command&&command->permission!=UI_ASSISTANT_PERMISSION_READ?UI_STATUS_PERMISSION_DENIED:UI_STATUS_NOT_FOUND;
    }
    if (command == NULL && !generated) {
        return UI_STATUS_NOT_FOUND;
    }
    if (!generated && command->validate != NULL) {
        callback_enter(assistant->host);
        validation_status = command->validate(params, command->user_data);
        callback_leave(assistant->host);
        if (validation_status != UI_STATUS_OK) {
            return validation_status == UI_STATUS_INVALID_ARGUMENT
                       ? UI_STATUS_VALIDATION_FAILED
                       : validation_status;
        }
    }

    if(!generated){validation_status=legacy_policy(assistant,command);if(validation_status)return validation_status;}
    confirmation_required = !generated && (command->permission > assistant->max_permission ||
                            command->permission ==
                                UI_ASSISTANT_PERMISSION_DESTRUCTIVE);
    if (confirmation_required) {
        int approved;
        if(assistant->confirm==NULL)return UI_STATUS_PERMISSION_DENIED;
        callback_enter(assistant->host);
        approved=assistant->confirm(command->id,params,command->permission,assistant->user_data);
        callback_leave(assistant->host);
        if(!approved)return UI_STATUS_PERMISSION_DENIED;
    }

    /* Callbacks may reenter the host. Never dispatch using a cached policy. */
    if(!generated){validation_status=legacy_policy(assistant,command);if(validation_status)return validation_status;}

    if(generated){ui_status_t s=ui_capabilities_reserve_request(assistant->host);if(s)return s;}
    request = (ui_assistant_request_t *)calloc(1u, sizeof(*request));
    if (request == NULL) {
        if(generated)ui_capabilities_release_request(assistant->host);
        return UI_STATUS_OUT_OF_MEMORY;
    }
    /* UI-thread dispatch is synchronous. Track before dispatch so progress,
       cancellation and a synchronous result callback see the request. */
    host_request_id = assistant->host->next_request_id;
    request->id = host_request_id;
    request->generated=generated;
    request->next = assistant->requests;
    assistant->requests = request;
    *request_id = host_request_id;
    if (ui_host_invoke(assistant->host, command_id, params,
                       assistant->source_id) == 0u) {
        (void)ui_assistant_forget_request(assistant, host_request_id);
        *request_id = 0u;
        return UI_STATUS_NOT_FOUND;
    }
    return UI_STATUS_OK;
}

ui_status_t ui_assistant_visit_commands(const ui_assistant_t *assistant,
                                         ui_assistant_command_visitor_fn visitor,
                                         void *user_data)
{
    ui_assistant_command_t *command;
    if (assistant == NULL || visitor == NULL) return UI_STATUS_INVALID_ARGUMENT;
    if(assistant->host->capability_read_entries)return ui_capabilities_visit_entries(assistant->host,visitor,user_data);
    for (command = assistant->commands; command != NULL; command = command->next) {
        ui_assistant_command_desc_t desc;
        memset(&desc, 0, sizeof(desc));
        desc.size = sizeof(desc);
        desc.id = command->id;
        desc.permission = command->permission;
        desc.params_schema_json = command->params_schema_json;
        desc.validate = command->validate;
        desc.user_data = command->user_data;
        callback_enter(assistant->host);
        visitor(&desc, user_data);
        callback_leave(assistant->host);
    }
    return UI_STATUS_OK;
}

ui_status_t ui_assistant_report_progress(ui_assistant_t *assistant,
                                          uint64_t request_id,
                                          int progress_percent,
                                          const char *message_utf8)
{
    ui_assistant_request_t *request;

    if (assistant == NULL || request_id == 0u || progress_percent < 0 ||
        progress_percent > 100) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    if(assistant->host->capability_read_entries&&!ui_language_thread_ok(assistant->host))return UI_STATUS_INVALID_ARGUMENT;
    request = find_request(assistant, request_id, NULL);
    if (request == NULL) {
        return UI_STATUS_NOT_FOUND;
    }
    if (request->cancelled) {
        return UI_STATUS_CANCELLED;
    }
    if (assistant->progress == NULL) {
        return UI_STATUS_UNSUPPORTED;
    }
    callback_enter(assistant->host);
    assistant->progress(request_id, progress_percent,
                        message_utf8 != NULL ? message_utf8 : "",
                        assistant->user_data);
    callback_leave(assistant->host);
    return UI_STATUS_OK;
}

ui_status_t ui_assistant_cancel(ui_assistant_t *assistant,
                                uint64_t request_id)
{
    ui_assistant_request_t *request;

    if (assistant == NULL || request_id == 0u) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    if(assistant->host->capability_read_entries&&!ui_language_thread_ok(assistant->host))return UI_STATUS_INVALID_ARGUMENT;
    request = find_request(assistant, request_id, NULL);
    if (request == NULL) {
        return UI_STATUS_NOT_FOUND;
    }
    if (request->cancelled) {
        return UI_STATUS_OK;
    }
    request->cancelled = 1;
    if(request->generated){uint64_t call=0;ui_status_t s=ui_capability_request_call(assistant->host,request_id,&call);
        return s==UI_STATUS_OK?ui_capability_cancel(assistant->host,call):s==UI_STATUS_NOT_FOUND?UI_STATUS_OK:s;
    }
    if (assistant->cancel != NULL) {
        callback_enter(assistant->host);
        assistant->cancel(request_id, assistant->user_data);
        callback_leave(assistant->host);
    }
    return UI_STATUS_OK;
}

ui_status_t ui_assistant_is_cancelled(const ui_assistant_t *assistant,
                                      uint64_t request_id,
                                      int *cancelled)
{
    ui_assistant_request_t *request;

    if (assistant == NULL || request_id == 0u || cancelled == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    if(assistant->host->capability_read_entries&&!ui_language_thread_ok(assistant->host))return UI_STATUS_INVALID_ARGUMENT;
    request = find_request((ui_assistant_t *)assistant, request_id, NULL);
    if (request == NULL) {
        return UI_STATUS_NOT_FOUND;
    }
    *cancelled = request->cancelled;
    if(request->generated){uint64_t call=0;ui_cap_result_t r={0};r.size=sizeof(r);r.version=1;
        if(ui_capability_request_call(assistant->host,request_id,&call)==UI_STATUS_OK&&ui_capability_get_result(assistant->host,call,&r)==UI_STATUS_OK)
            *cancelled=*cancelled||r.cancel_requested;
    }
    return UI_STATUS_OK;
}

ui_status_t ui_assistant_forget_request(ui_assistant_t *assistant,
                                         uint64_t request_id)
{
    ui_assistant_request_t *previous;
    ui_assistant_request_t *request;

    if (assistant == NULL || request_id == 0u) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    if(assistant->host->capability_read_entries&&!ui_language_thread_ok(assistant->host))return UI_STATUS_INVALID_ARGUMENT;
    request = find_request(assistant, request_id, &previous);
    if (request == NULL) {
        return UI_STATUS_NOT_FOUND;
    }
    {uint64_t call=0;if(assistant->host->capabilities&&ui_capability_request_call(assistant->host,request_id,&call)==UI_STATUS_OK){ui_status_t s=ui_capability_forget(assistant->host,call);if(s)return s;}}
    if (previous == NULL) {
        assistant->requests = request->next;
    } else {
        previous->next = request->next;
    }
    if(request->generated)ui_capabilities_release_request(assistant->host);
    free(request);
    return UI_STATUS_OK;
}

/* Workspace owns its Assistant request tracking. Reap terminal generated
 * requests only at a poll boundary, after all execute/result frames returned.
 * Standalone host callers retain explicit query/forget ownership. */
void ui_assistant_collect_completed(ui_assistant_t *assistant)
{
    ui_assistant_request_t *p,*next;
    if(!assistant||!assistant->host->capability_read_entries)return;
    for(p=assistant->requests;p;p=next){uint64_t call=0;ui_cap_result_t r={0};next=p->next;
        r.size=sizeof(r);r.version=1;
        if(p->generated&&(ui_capability_request_call(assistant->host,p->id,&call)==UI_STATUS_NOT_FOUND||
           (ui_capability_get_result(assistant->host,call,&r)==UI_STATUS_OK&&r.state==UI_CAP_FINISHED)))
            (void)ui_assistant_forget_request(assistant,p->id);
    }
}

ui_status_t ui_assistant_get_state_snapshot(const ui_assistant_t *assistant,
                                            const char **snapshot_utf8)
{
    const char *snapshot;

    if (assistant == NULL || snapshot_utf8 == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    if (assistant->snapshot == NULL) {
        return UI_STATUS_UNSUPPORTED;
    }
    callback_enter(assistant->host);
    snapshot = assistant->snapshot(assistant->user_data);
    callback_leave(assistant->host);
    *snapshot_utf8 = snapshot != NULL ? snapshot : "";
    return UI_STATUS_OK;
}

ui_status_t ui_assistant_begin_transaction(ui_assistant_t *assistant,
                                           const char *label_utf8,
                                           uint64_t *transaction_id)
{
    if (assistant == NULL || transaction_id == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    *transaction_id = 0u;
    if(assistant->host->capability_read_entries)return ui_language_thread_ok(assistant->host)?UI_STATUS_PERMISSION_DENIED:UI_STATUS_INVALID_ARGUMENT;
    if (assistant->transaction_begin == NULL) {
        return UI_STATUS_UNSUPPORTED;
    }
    {ui_status_t s;callback_enter(assistant->host);
     s=assistant->transaction_begin(label_utf8 != NULL ? label_utf8 : "",transaction_id,assistant->user_data);
     callback_leave(assistant->host);return s;}
}

ui_status_t ui_assistant_commit_transaction(ui_assistant_t *assistant,
                                            uint64_t transaction_id)
{
    if (assistant == NULL || transaction_id == 0u) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    if(assistant->host->capability_read_entries)return ui_language_thread_ok(assistant->host)?UI_STATUS_PERMISSION_DENIED:UI_STATUS_INVALID_ARGUMENT;
    if (assistant->transaction_commit == NULL) {
        return UI_STATUS_UNSUPPORTED;
    }
    {ui_status_t s;callback_enter(assistant->host);s=assistant->transaction_commit(transaction_id,assistant->user_data);callback_leave(assistant->host);return s;}
}

ui_status_t ui_assistant_rollback_transaction(ui_assistant_t *assistant,
                                              uint64_t transaction_id)
{
    if (assistant == NULL || transaction_id == 0u) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    if(assistant->host->capability_read_entries)return ui_language_thread_ok(assistant->host)?UI_STATUS_PERMISSION_DENIED:UI_STATUS_INVALID_ARGUMENT;
    if (assistant->transaction_rollback == NULL) {
        return UI_STATUS_UNSUPPORTED;
    }
    {ui_status_t s;callback_enter(assistant->host);s=assistant->transaction_rollback(transaction_id,assistant->user_data);callback_leave(assistant->host);return s;}
}

ui_status_t ui_assistant_undo_transaction(ui_assistant_t *assistant,
                                           uint64_t transaction_id)
{
    if (assistant == NULL || transaction_id == 0u) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    if(assistant->host->capability_read_entries)return ui_language_thread_ok(assistant->host)?UI_STATUS_PERMISSION_DENIED:UI_STATUS_INVALID_ARGUMENT;
    if (assistant->transaction_undo == NULL) {
        return UI_STATUS_UNSUPPORTED;
    }
    {ui_status_t s;callback_enter(assistant->host);s=assistant->transaction_undo(transaction_id,assistant->user_data);callback_leave(assistant->host);return s;}
}
