#ifndef UI_FRAMEWORK_APPLICATION_H
#define UI_FRAMEWORK_APPLICATION_H

#include "ui_framework/native.h"
#include "ui_framework/assistant.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UI_APPLICATION_ABI_VERSION 1u
#define UI_WORKSPACE_WAKE_MESSAGE (0x8000u + 0x241u)
#if defined(_WIN32)
#define UI_APP_EXPORT __declspec(dllexport)
#define UI_APP_CALL __cdecl
#else
#define UI_APP_EXPORT
#define UI_APP_CALL
#endif

typedef struct ui_workspace ui_workspace_t;
typedef enum ui_workspace_shell_mode {
    UI_WORKSPACE_SHELL_NATIVE = 0,
    UI_WORKSPACE_SHELL_WEB = 1
} ui_workspace_shell_mode_t;
typedef enum ui_app_close_decision {
    UI_APP_CLOSE_ALLOW = 0,
    UI_APP_CLOSE_REFUSE = 1,
    UI_APP_CLOSE_WAIT = 2
} ui_app_close_decision_t;
typedef enum ui_app_close_reason {
    UI_APP_CLOSE_TAB = 0,
    UI_APP_CLOSE_WORKSPACE = 1
} ui_app_close_reason_t;

/* Borrowed instance context; copy it into private state if needed. The workspace
   and instance ID are the only handles permitted in worker-thread posts. */
typedef struct ui_app_context {
    uint32_t size;
    uint32_t abi_version;
    ui_workspace_t *workspace;
    uint64_t instance_id;
    ui_host_t *host;
    ui_native_shell_t *shell; /* NULL during create; valid during mount/unmount. */
    ui_assistant_t *assistant;
} ui_app_context_t;

/* Exactly one exported function: ui_app_query_v1. All callbacks run on the UI
   thread. create registers UI and fills assistant_config; mount creates content.
   On create failure, a non-NULL state is still passed to destroy.
   unmount releases GPU/native/backend resources while host is still alive.
   destroy releases private state after framework objects are destroyed.
   ALLOW means all workers and external callbacks have stopped; WAIT completes
   through ui_workspace_post_close_complete. module_shutdown runs after the last
   instance; failure retains the DLL, never forces unloading. */
typedef struct ui_app_descriptor {
    uint32_t size;
    uint32_t abi_version;
    uint32_t framework_api_version;
    ui_status_t (UI_APP_CALL *create)(const ui_app_context_t *context,
        void **state, ui_assistant_config_t *assistant_config);
    ui_status_t (UI_APP_CALL *mount)(void *state,
        const ui_app_context_t *context);
    void (UI_APP_CALL *set_active)(void *state, int active);
    ui_app_close_decision_t (UI_APP_CALL *request_close)(void *state,
        ui_app_close_reason_t reason);
    void (UI_APP_CALL *unmount)(void *state);
    void (UI_APP_CALL *destroy)(void *state);
    ui_status_t (UI_APP_CALL *module_shutdown)(void);
} ui_app_descriptor_t;
typedef const ui_app_descriptor_t *(UI_APP_CALL *ui_app_query_fn)(void);

typedef void (*ui_workspace_result_fn)(uint64_t instance_id,
    const ui_result_t *result, void *user_data);
typedef void (*ui_workspace_event_fn)(uint64_t instance_id,
    const char *event_id, const char *payload_json, void *user_data);
typedef void (*ui_workspace_progress_fn)(uint64_t instance_id,
    uint64_t request_id, int percent, const char *message_utf8, void *user_data);
typedef int (*ui_workspace_confirm_fn)(uint64_t instance_id,
    const char *command_id, const char *params_json,
    ui_assistant_permission_t permission, void *user_data);
typedef void (*ui_workspace_changed_fn)(void *user_data);

typedef struct ui_workspace_config {
    uint32_t size;
    void *native_parent; /* Borrowed top-level HWND. Initialize DPI first. */
    void *user_data;
    ui_workspace_result_fn result;
    ui_workspace_event_fn event;
    ui_workspace_progress_fn progress;
    ui_workspace_confirm_fn confirm;
    ui_workspace_changed_fn changed;
    ui_assistant_permission_t max_permission;
    /* Preserve the complete v1 structure, including its tail padding. */
    uintptr_t reserved_v1;
    /* Appended. Zero preserves native embedding; standalone uses WEB. */
    ui_workspace_shell_mode_t shell_mode;
} ui_workspace_config_t;

typedef struct ui_app_instance_info {
    uint32_t size;
    uint64_t instance_id;
    const char *app_id;
    const char *name_utf8;
    const char *version;
    int active;
    int closing;
    ui_host_t *host;
    ui_native_shell_t *shell;
    ui_assistant_t *assistant;
    void *native_container;
} ui_app_instance_info_t;

UI_API ui_workspace_t *ui_workspace_create(const ui_workspace_config_t *config);
/* Only succeeds after all instances have closed and all modules unloaded.
   On failure the workspace remains valid, including the copied-message queue. */
UI_API ui_status_t ui_workspace_destroy(ui_workspace_t *workspace);
UI_API ui_status_t ui_workspace_open(ui_workspace_t *workspace,
    const char *package_path_utf8, uint64_t *instance_id);
UI_API ui_status_t ui_workspace_activate(ui_workspace_t *workspace, uint64_t id);
UI_API ui_status_t ui_workspace_close(ui_workspace_t *workspace, uint64_t id,
    ui_app_close_reason_t reason);
UI_API ui_status_t ui_workspace_close_all(ui_workspace_t *workspace);
UI_API size_t ui_workspace_count(const ui_workspace_t *workspace);
UI_API uint64_t ui_workspace_instance_at(const ui_workspace_t *workspace,
    size_t index);
UI_API uint64_t ui_workspace_active(const ui_workspace_t *workspace);
UI_API ui_status_t ui_workspace_get_instance(const ui_workspace_t *workspace,
    uint64_t id, ui_app_instance_info_t *info);
/* Logical rect relative to native_parent client, without the native HMENU. */
UI_API ui_status_t ui_workspace_set_rect(ui_workspace_t *workspace,
    const ui_rect_t *rect, uint32_t dpi);
UI_API ui_status_t ui_workspace_invoke(ui_workspace_t *workspace, uint64_t id,
    const char *command_id, const char *params_json, uint64_t *request_id);
UI_API ui_status_t ui_workspace_cancel(ui_workspace_t *workspace, uint64_t id,
    uint64_t request_id);
UI_API ui_status_t ui_workspace_handle_message(ui_workspace_t *workspace,
    void *hwnd, uint32_t message, uintptr_t w_param, intptr_t l_param,
    intptr_t *result);
/* Poll on UI thread after WAKE_MESSAGE. Never destroys a current callback. */
UI_API void ui_workspace_poll(ui_workspace_t *workspace);
UI_API const char *ui_workspace_last_error(const ui_workspace_t *workspace);

/* Thread-safe copied delivery. Workspace remains alive until workers stop.
   Posts never carry app callbacks or host pointers. Closed IDs are discarded. */
UI_API ui_status_t ui_workspace_post_result(ui_workspace_t *workspace,
    uint64_t id, uint64_t request_id, int success, const char *result_json);
UI_API ui_status_t ui_workspace_post_event(ui_workspace_t *workspace,
    uint64_t id, const char *event_id, const char *payload_json);
UI_API ui_status_t ui_workspace_post_progress(ui_workspace_t *workspace,
    uint64_t id, uint64_t request_id, int percent, const char *message_utf8);
UI_API ui_status_t ui_workspace_post_close_complete(ui_workspace_t *workspace,
    uint64_t id, ui_app_close_decision_t decision);

/* UI-thread resource read. Data is a framework allocation, size bytes plus a
   trailing zero for text; release with this function, not application free(). */
UI_API ui_status_t ui_app_resource_read(const ui_app_context_t *context,
    const char *name_utf8, void **data, size_t *size);
UI_API void ui_app_resource_release(void *data);
/* UI-thread scope for application-owned HWND/subclass/external UI callbacks.
   Pair around the entire callback, including DefWindowProc/modal message pumps.
   Framework command/surface callbacks are guarded automatically. */
UI_API void ui_app_callback_enter(const ui_app_context_t *context);
UI_API void ui_app_callback_leave(const ui_app_context_t *context);

#ifdef __cplusplus
}
#endif
#endif
