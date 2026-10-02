#include <stdio.h>
#include <string.h>

#include "ui_framework/assistant.h"

typedef struct test_state {
    int results;
    int confirms;
    int progress;
    int cancels;
    int command_calls;
    uint64_t last_transaction;
} test_state_t;

static void on_result(const ui_result_t *result, void *user_data)
{
    test_state_t *state = (test_state_t *)user_data;
    if (result != NULL && result->success) {
        state->results += 1;
    }
}

static void command_handler(ui_host_t *host,
                            uint64_t request_id,
                            const char *command_id,
                            const char *params_json,
                            const char *source,
                            void *user_data)
{
    test_state_t *state = (test_state_t *)user_data;
    (void)command_id;
    (void)params_json;
    (void)source;
    state->command_calls += 1;
    (void)ui_host_reply(host, request_id, 1, "{\"ok\":true}");
}

static void async_handler(ui_host_t *host,
                          uint64_t request_id,
                          const char *command_id,
                          const char *params_json,
                          const char *source,
                          void *user_data)
{
    (void)host;
    (void)request_id;
    (void)command_id;
    (void)params_json;
    (void)source;
    ((test_state_t *)user_data)->command_calls += 1;
}

static ui_status_t validate_ok(const char *params_json, void *user_data)
{
    (void)user_data;
    return params_json != NULL && strstr(params_json, "\"ok\":true") != NULL
               ? UI_STATUS_OK
               : UI_STATUS_INVALID_ARGUMENT;
}

static int confirm_command(const char *command_id,
                           const char *params_json,
                           ui_assistant_permission_t permission,
                           void *user_data)
{
    test_state_t *state = (test_state_t *)user_data;
    (void)command_id;
    (void)params_json;
    (void)permission;
    state->confirms += 1;
    return 1;
}

static void report_progress(uint64_t request_id,
                            int progress_percent,
                            const char *message_utf8,
                            void *user_data)
{
    test_state_t *state = (test_state_t *)user_data;
    (void)request_id;
    (void)message_utf8;
    if (progress_percent == 50) {
        state->progress += 1;
    }
}

static void report_cancel(uint64_t request_id, void *user_data)
{
    (void)request_id;
    ((test_state_t *)user_data)->cancels += 1;
}

static const char *snapshot(void *user_data)
{
    (void)user_data;
    return "{\"blocks\":2}";
}

static ui_status_t begin_transaction(const char *label,
                                     uint64_t *transaction_id,
                                     void *user_data)
{
    test_state_t *state = (test_state_t *)user_data;
    (void)label;
    state->last_transaction = 42u;
    *transaction_id = state->last_transaction;
    return UI_STATUS_OK;
}

static ui_status_t end_transaction(uint64_t transaction_id, void *user_data)
{
    test_state_t *state = (test_state_t *)user_data;
    return transaction_id == state->last_transaction ? UI_STATUS_OK
                                                       : UI_STATUS_NOT_FOUND;
}

static int check(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "assistant check failed: %s\n", message);
    }
    return condition;
}

int main(void)
{
    test_state_t state;
    ui_host_config_t host_config;
    ui_command_desc_t command;
    ui_assistant_config_t assistant_config;
    ui_assistant_command_desc_t assistant_command;
    ui_host_t *host;
    ui_assistant_t *assistant;
    ui_assistant_t *read_only;
    uint64_t request_id = 0u;
    uint64_t transaction_id = 0u;
    const char *state_json = NULL;
    int cancelled = 0;
    int failures = 0;

    memset(&state, 0, sizeof(state));
    memset(&host_config, 0, sizeof(host_config));
    host_config.size = sizeof(host_config);
    host_config.api_version = UI_FRAMEWORK_API_VERSION;
    host_config.user_data = &state;
    host_config.result_callback = on_result;
    host = ui_host_create(&host_config);
    failures += !check(host != NULL, "host create");
    if (host == NULL) {
        return 1;
    }

    memset(&command, 0, sizeof(command));
    command.size = sizeof(command);
    command.id = "test.edit";
    command.title = "Edit";
    command.params_schema_json = "{}";
    command.handler = command_handler;
    command.user_data = &state;
    failures += !check(ui_host_register_command(host, &command) == UI_STATUS_OK,
                       "edit command registration");
    command.id = "test.async";
    command.title = "Async";
    command.handler = async_handler;
    failures += !check(ui_host_register_command(host, &command) == UI_STATUS_OK,
                       "async command registration");

    memset(&assistant_config, 0, sizeof(assistant_config));
    assistant_config.size = sizeof(assistant_config);
    assistant_config.max_permission = UI_ASSISTANT_PERMISSION_READ;
    assistant_config.user_data = &state;
    assistant_config.confirm = confirm_command;
    assistant_config.progress = report_progress;
    assistant_config.cancel = report_cancel;
    assistant_config.snapshot = snapshot;
    assistant_config.transaction_begin = begin_transaction;
    assistant_config.transaction_commit = end_transaction;
    assistant_config.transaction_rollback = end_transaction;
    assistant_config.transaction_undo =
        (ui_assistant_transaction_undo_fn)end_transaction;
    assistant = ui_assistant_create(host, &assistant_config);
    failures += !check(assistant != NULL, "assistant create");
    if (assistant == NULL) {
        ui_host_destroy(host);
        return 1;
    }

    memset(&assistant_command, 0, sizeof(assistant_command));
    assistant_command.size = sizeof(assistant_command);
    assistant_command.id = "test.edit";
    assistant_command.permission = UI_ASSISTANT_PERMISSION_EDIT;
    assistant_command.params_schema_json = "{\"type\":\"object\"}";
    assistant_command.validate = validate_ok;
    failures += !check(ui_assistant_register_command(assistant,
                                                     &assistant_command) ==
                           UI_STATUS_OK,
                       "assistant command registration");

    failures += !check(ui_assistant_invoke(assistant, "test.edit", "{}",
                                           &request_id) ==
                           UI_STATUS_VALIDATION_FAILED,
                       "validation failure");
    failures += !check(ui_assistant_invoke(assistant, "test.edit",
                                           "{\"ok\":true}", &request_id) ==
                           UI_STATUS_OK &&
                       state.confirms == 1 && state.results == 1,
                       "confirmed edit invocation");
    failures += !check(ui_assistant_forget_request(assistant, request_id) ==
                           UI_STATUS_OK,
                       "forget completed request");

    memset(&assistant_command, 0, sizeof(assistant_command));
    assistant_command.size = sizeof(assistant_command);
    assistant_command.id = "test.async";
    assistant_command.permission = UI_ASSISTANT_PERMISSION_READ;
    assistant_command.params_schema_json = "{}";
    failures += !check(ui_assistant_register_command(assistant,
                                                     &assistant_command) ==
                           UI_STATUS_OK,
                       "async assistant registration");
    failures += !check(ui_assistant_invoke(assistant, "test.async", "{}",
                                           &request_id) == UI_STATUS_OK,
                       "async invocation");
    failures += !check(ui_assistant_report_progress(assistant, request_id, 50,
                                                    "working") == UI_STATUS_OK &&
                       state.progress == 1,
                       "progress callback");
    failures += !check(ui_assistant_cancel(assistant, request_id) ==
                           UI_STATUS_OK &&
                       ui_assistant_is_cancelled(assistant, request_id,
                                                 &cancelled) == UI_STATUS_OK &&
                       cancelled && state.cancels == 1,
                       "cooperative cancellation");
    failures += !check(ui_assistant_report_progress(assistant, request_id, 80,
                                                    "cancelled") ==
                           UI_STATUS_CANCELLED,
                       "cancelled progress rejected");
    failures += !check(ui_host_reply(host, request_id, 1, "{}") ==
                           UI_STATUS_OK,
                       "complete async command");
    failures += !check(ui_assistant_forget_request(assistant, request_id) ==
                           UI_STATUS_OK,
                       "forget async request");

    failures += !check(ui_assistant_get_state_snapshot(assistant, &state_json) ==
                           UI_STATUS_OK && state_json != NULL &&
                       strcmp(state_json, "{\"blocks\":2}") == 0,
                       "state snapshot");
    failures += !check(ui_assistant_begin_transaction(assistant, "edit",
                                                      &transaction_id) ==
                           UI_STATUS_OK && transaction_id == 42u,
                       "begin transaction");
    failures += !check(ui_assistant_commit_transaction(assistant,
                                                       transaction_id) ==
                           UI_STATUS_OK &&
                       ui_assistant_rollback_transaction(assistant,
                                                         transaction_id) ==
                           UI_STATUS_OK &&
                       ui_assistant_undo_transaction(assistant,
                                                     transaction_id) ==
                           UI_STATUS_OK,
                       "transaction callbacks");

    read_only = ui_assistant_create(host, NULL);
    failures += !check(read_only != NULL, "default read-only assistant");
    if (read_only != NULL) {
        memset(&assistant_command, 0, sizeof(assistant_command));
        assistant_command.size = sizeof(assistant_command);
        assistant_command.id = "test.edit";
        assistant_command.permission = UI_ASSISTANT_PERMISSION_EDIT;
        failures += !check(ui_assistant_register_command(read_only,
                                                         &assistant_command) ==
                               UI_STATUS_OK,
                           "read-only registration");
        failures += !check(ui_assistant_invoke(read_only, "test.edit", "{}",
                                               &request_id) ==
                               UI_STATUS_PERMISSION_DENIED,
                           "permission denial");
        ui_assistant_destroy(read_only);
    }

    ui_assistant_destroy(assistant);
    ui_host_destroy(host);
    printf("assistant protocol: %d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
