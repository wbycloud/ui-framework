#ifndef UI_FRAMEWORK_ASSISTANT_H
#define UI_FRAMEWORK_ASSISTANT_H

#include "ui_framework/ui.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The assistant protocol is an optional, host-bound C ABI layer. */
#define UI_ASSISTANT_API_VERSION 1u

typedef struct ui_assistant ui_assistant_t;

typedef enum ui_assistant_permission {
    UI_ASSISTANT_PERMISSION_READ = 0,
    UI_ASSISTANT_PERMISSION_EDIT = 1,
    UI_ASSISTANT_PERMISSION_DESTRUCTIVE = 2
} ui_assistant_permission_t;

/* Return UI_STATUS_OK when params_json is valid for the command. */
typedef ui_status_t (*ui_assistant_validate_fn)(
    const char *params_json,
    void *user_data);

/* Return non-zero to approve an invocation that requires confirmation. */
typedef int (*ui_assistant_confirm_fn)(
    const char *command_id,
    const char *params_json,
    ui_assistant_permission_t permission,
    void *user_data);

typedef void (*ui_assistant_progress_fn)(
    uint64_t request_id,
    int progress_percent,
    const char *message_utf8,
    void *user_data);

/* Cancellation is cooperative: the command handler must observe its state. */
typedef void (*ui_assistant_cancel_fn)(uint64_t request_id,
                                       void *user_data);

/* The returned UTF-8 string is borrowed from the application. */
typedef const char *(*ui_assistant_snapshot_fn)(void *user_data);

typedef ui_status_t (*ui_assistant_transaction_begin_fn)(
    const char *label_utf8,
    uint64_t *transaction_id,
    void *user_data);
typedef ui_status_t (*ui_assistant_transaction_end_fn)(
    uint64_t transaction_id,
    void *user_data);
typedef ui_status_t (*ui_assistant_transaction_undo_fn)(
    uint64_t transaction_id,
    void *user_data);

typedef struct ui_assistant_config {
    uint32_t size;
    ui_assistant_permission_t max_permission;
    const char *source_id;
    void *user_data;
    ui_assistant_confirm_fn confirm;
    ui_assistant_progress_fn progress;
    ui_assistant_cancel_fn cancel;
    ui_assistant_snapshot_fn snapshot;
    ui_assistant_transaction_begin_fn transaction_begin;
    ui_assistant_transaction_end_fn transaction_commit;
    ui_assistant_transaction_end_fn transaction_rollback;
    ui_assistant_transaction_undo_fn transaction_undo;
} ui_assistant_config_t;

typedef struct ui_assistant_command_desc {
    uint32_t size;
    const char *id;
    ui_assistant_permission_t permission;
    const char *params_schema_json;
    ui_assistant_validate_fn validate;
    void *user_data;
} ui_assistant_command_desc_t;

/* Descriptor and strings are borrowed for the duration of the call.
   A visitor must not register commands or destroy the assistant. */
typedef void (*ui_assistant_command_visitor_fn)(
    const ui_assistant_command_desc_t *command, void *user_data);

UI_API ui_assistant_t *ui_assistant_create(
    ui_host_t *host,
    const ui_assistant_config_t *config);
UI_API void ui_assistant_destroy(ui_assistant_t *assistant);

UI_API ui_status_t ui_assistant_register_command(
    ui_assistant_t *assistant,
    const ui_assistant_command_desc_t *desc);
UI_API ui_status_t ui_assistant_visit_commands(
    const ui_assistant_t *assistant,
    ui_assistant_command_visitor_fn visitor,
    void *user_data);

/* Invokes a previously registered semantic command. */
UI_API ui_status_t ui_assistant_invoke(
    ui_assistant_t *assistant,
    const char *command_id,
    const char *params_json,
    uint64_t *request_id);

UI_API ui_status_t ui_assistant_report_progress(
    ui_assistant_t *assistant,
    uint64_t request_id,
    int progress_percent,
    const char *message_utf8);
UI_API ui_status_t ui_assistant_cancel(ui_assistant_t *assistant,
                                       uint64_t request_id);
UI_API ui_status_t ui_assistant_is_cancelled(
    const ui_assistant_t *assistant,
    uint64_t request_id,
    int *cancelled);
/* Call after the host result callback has observed completion. */
UI_API ui_status_t ui_assistant_forget_request(ui_assistant_t *assistant,
                                               uint64_t request_id);

/* Snapshot data is borrowed and must not be freed by the caller. */
UI_API ui_status_t ui_assistant_get_state_snapshot(
    const ui_assistant_t *assistant,
    const char **snapshot_utf8);

UI_API ui_status_t ui_assistant_begin_transaction(
    ui_assistant_t *assistant,
    const char *label_utf8,
    uint64_t *transaction_id);
UI_API ui_status_t ui_assistant_commit_transaction(
    ui_assistant_t *assistant,
    uint64_t transaction_id);
UI_API ui_status_t ui_assistant_rollback_transaction(
    ui_assistant_t *assistant,
    uint64_t transaction_id);
UI_API ui_status_t ui_assistant_undo_transaction(
    ui_assistant_t *assistant,
    uint64_t transaction_id);

#ifdef __cplusplus
}
#endif

#endif
