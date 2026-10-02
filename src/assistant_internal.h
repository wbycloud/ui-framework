#ifndef UI_FRAMEWORK_ASSISTANT_INTERNAL_H
#define UI_FRAMEWORK_ASSISTANT_INTERNAL_H

#include "ui_framework/assistant.h"

typedef struct ui_assistant_command ui_assistant_command_t;
typedef struct ui_assistant_request ui_assistant_request_t;

struct ui_assistant_command {
    char *id;
    char *params_schema_json;
    ui_assistant_permission_t permission;
    ui_assistant_validate_fn validate;
    void *user_data;
    ui_assistant_command_t *next;
};

struct ui_assistant_request {
    uint64_t id;
    int cancelled;
    ui_assistant_request_t *next;
};

struct ui_assistant {
    ui_host_t *host;
    ui_assistant_permission_t max_permission;
    char *source_id;
    void *user_data;
    ui_assistant_confirm_fn confirm;
    ui_assistant_progress_fn progress;
    ui_assistant_cancel_fn cancel;
    ui_assistant_snapshot_fn snapshot;
    ui_assistant_transaction_begin_fn transaction_begin;
    ui_assistant_transaction_end_fn transaction_commit;
    ui_assistant_transaction_end_fn transaction_rollback;
    ui_assistant_transaction_undo_fn transaction_undo;
    ui_assistant_command_t *commands;
    ui_assistant_request_t *requests;
};

#endif
