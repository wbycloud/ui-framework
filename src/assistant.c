#include "assistant_internal.h"
#include "ui_internal.h"

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
    return assistant;
}

void ui_assistant_destroy(ui_assistant_t *assistant)
{
    ui_assistant_command_t *command;
    ui_assistant_request_t *request;

    if (assistant == NULL) {
        return;
    }
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
    if (find_command(assistant, desc->id) != NULL) {
        return UI_STATUS_ALREADY_EXISTS;
    }

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

    if (request_id != NULL) {
        *request_id = 0u;
    }
    if (assistant == NULL || command_id == NULL || command_id[0] == '\0' ||
        request_id == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    command = find_command(assistant, command_id);
    if (command == NULL) {
        return UI_STATUS_NOT_FOUND;
    }
    if (command->validate != NULL) {
        validation_status = command->validate(params, command->user_data);
        if (validation_status != UI_STATUS_OK) {
            return validation_status == UI_STATUS_INVALID_ARGUMENT
                       ? UI_STATUS_VALIDATION_FAILED
                       : validation_status;
        }
    }

    confirmation_required = command->permission > assistant->max_permission ||
                            command->permission ==
                                UI_ASSISTANT_PERMISSION_DESTRUCTIVE;
    if (confirmation_required &&
        (assistant->confirm == NULL ||
         !assistant->confirm(command->id, params, command->permission,
                             assistant->user_data))) {
        return UI_STATUS_PERMISSION_DENIED;
    }

    request = (ui_assistant_request_t *)calloc(1u, sizeof(*request));
    if (request == NULL) {
        return UI_STATUS_OUT_OF_MEMORY;
    }
    /* UI-thread dispatch is synchronous. Track before dispatch so progress,
       cancellation and a synchronous result callback see the request. */
    host_request_id = assistant->host->next_request_id;
    request->id = host_request_id;
    request->next = assistant->requests;
    assistant->requests = request;
    *request_id = host_request_id;
    if (ui_host_invoke(assistant->host, command->id, params,
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
    for (command = assistant->commands; command != NULL; command = command->next) {
        ui_assistant_command_desc_t desc;
        memset(&desc, 0, sizeof(desc));
        desc.size = sizeof(desc);
        desc.id = command->id;
        desc.permission = command->permission;
        desc.params_schema_json = command->params_schema_json;
        desc.validate = command->validate;
        desc.user_data = command->user_data;
        visitor(&desc, user_data);
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
    assistant->progress(request_id, progress_percent,
                        message_utf8 != NULL ? message_utf8 : "",
                        assistant->user_data);
    return UI_STATUS_OK;
}

ui_status_t ui_assistant_cancel(ui_assistant_t *assistant,
                                uint64_t request_id)
{
    ui_assistant_request_t *request;

    if (assistant == NULL || request_id == 0u) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    request = find_request(assistant, request_id, NULL);
    if (request == NULL) {
        return UI_STATUS_NOT_FOUND;
    }
    if (request->cancelled) {
        return UI_STATUS_OK;
    }
    request->cancelled = 1;
    if (assistant->cancel != NULL) {
        assistant->cancel(request_id, assistant->user_data);
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
    request = find_request((ui_assistant_t *)assistant, request_id, NULL);
    if (request == NULL) {
        return UI_STATUS_NOT_FOUND;
    }
    *cancelled = request->cancelled;
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
    request = find_request(assistant, request_id, &previous);
    if (request == NULL) {
        return UI_STATUS_NOT_FOUND;
    }
    if (previous == NULL) {
        assistant->requests = request->next;
    } else {
        previous->next = request->next;
    }
    free(request);
    return UI_STATUS_OK;
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
    snapshot = assistant->snapshot(assistant->user_data);
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
    if (assistant->transaction_begin == NULL) {
        return UI_STATUS_UNSUPPORTED;
    }
    return assistant->transaction_begin(
        label_utf8 != NULL ? label_utf8 : "", transaction_id,
        assistant->user_data);
}

ui_status_t ui_assistant_commit_transaction(ui_assistant_t *assistant,
                                            uint64_t transaction_id)
{
    if (assistant == NULL || transaction_id == 0u) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    if (assistant->transaction_commit == NULL) {
        return UI_STATUS_UNSUPPORTED;
    }
    return assistant->transaction_commit(transaction_id, assistant->user_data);
}

ui_status_t ui_assistant_rollback_transaction(ui_assistant_t *assistant,
                                              uint64_t transaction_id)
{
    if (assistant == NULL || transaction_id == 0u) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    if (assistant->transaction_rollback == NULL) {
        return UI_STATUS_UNSUPPORTED;
    }
    return assistant->transaction_rollback(transaction_id,
                                           assistant->user_data);
}

ui_status_t ui_assistant_undo_transaction(ui_assistant_t *assistant,
                                           uint64_t transaction_id)
{
    if (assistant == NULL || transaction_id == 0u) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    if (assistant->transaction_undo == NULL) {
        return UI_STATUS_UNSUPPORTED;
    }
    return assistant->transaction_undo(transaction_id, assistant->user_data);
}
