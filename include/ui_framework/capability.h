#ifndef UI_FRAMEWORK_CAPABILITY_H
#define UI_FRAMEWORK_CAPABILITY_H

#include "ui.h"

#ifdef __cplusplus
extern "C" {
#endif

/* API10, independent complete structures. No AI Service or write contract.
 * All host APIs run on its creating UI thread, including complete(). Workers
 * marshal copied data and stable IDs through workspace app_data, never host or
 * callback pointers. Registration copies metadata; callback/user_data remain
 * application-owned until finish_unregistration succeeds. */
#define UI_CAPABILITY_CONTRACT_VERSION 1u
#define UI_CAPABILITY_MAX_COUNT 256u
#define UI_CAPABILITY_CATALOG_BYTES (2u * 1024u * 1024u)
#define UI_CAPABILITY_INPUT_BYTES (64u * 1024u)
#define UI_CAPABILITY_OUTPUT_BYTES (256u * 1024u)
#define UI_CAPABILITY_MAX_CALLS 1024u
#define UI_CAPABILITY_CALL_BYTES (8u * 1024u * 1024u)

typedef enum ui_cap_resource_kind {
    UI_CAP_DOCUMENT = 1, UI_CAP_INSTANCE_VIEW = 2,
    UI_CAP_USER_SETTINGS = 3, UI_CAP_WORKSPACE = 4
} ui_cap_resource_kind_t;

typedef enum ui_cap_call_state {
    UI_CAP_PREPARED = 1, UI_CAP_RUNNING = 2, UI_CAP_FINISHED = 3
} ui_cap_call_state_t;

typedef enum ui_cap_outcome {
    UI_CAP_OK = 0, UI_CAP_REJECTED = 1, UI_CAP_FAILED = 2,
    UI_CAP_CANCELLED = 3, UI_CAP_STALE = 4
} ui_cap_outcome_t;

typedef struct ui_cap_target {
    uint32_t size, version;
    uint64_t instance_id;
    ui_cap_resource_kind_t kind;
    char host_epoch[37], resource_id[256], open_epoch[37];
    char expected_revision[128];
    /* v1 bind produces exactly {"access":"read"}. Not an ACL or approval. */
    char scope_json[4097];
} ui_cap_target_t;

typedef struct ui_cap_resource_desc {
    uint32_t size, version;
    ui_cap_resource_kind_t kind;
    const char *id;
    /* NULL creates a new UUID. A shared document may supply its opening UUID. */
    const char *open_epoch;
    ui_status_t (*get_revision)(char *buffer, size_t capacity, void *user_data);
    void *user_data;
} ui_cap_resource_desc_t;

typedef struct ui_capability_desc {
    uint32_t size, version, capability_version;
    const char *id, *name_utf8, *description_utf8, *category;
    const char *input_schema_json, *output_schema_json;
    ui_cap_resource_kind_t target_kind;
    /* Must be "read". Other effects return UNSUPPORTED before registration. */
    const char *effect;
    ui_status_t (*check)(const ui_cap_target_t *, const char *params_json,
        void *user_data);
    void (*execute)(uint64_t call_id, const ui_cap_target_t *,
        const char *params_json, void *user_data);
    void (*cancel)(uint64_t call_id, void *user_data);
    void *user_data;
} ui_capability_desc_t;

typedef struct ui_cap_result {
    /* complete consumes size/version/outcome/output_json only. All other fields
     * are framework output: revision is checked through the registered resource. */
    uint32_t size, version;
    ui_cap_call_state_t state;
    ui_cap_outcome_t outcome;
    int cancel_requested;
    char code[64];
    char revision[128];
    /* complete borrows until return and copies; get_result borrows until forget
     * or host destruction. Non-OK outcomes have NULL output. Always effect=none. */
    const char *output_json;
} ui_cap_result_t;

/* Returned JSON allocations belong to this DLL: release, never application free.
 * JSON functions are host-independent and reentrant. Canonicalize accepts up to
 * OUTPUT_BYTES; validate data up to OUTPUT_BYTES, schema up to INPUT_BYTES.
 * Strict UTF-8/JSON, no duplicate keys; depth16/nodes4096/array512/object128.
 * Canonicalization follows RFC8785, SHA256 is lowercase hex. */
UI_API void ui_capability_release(void *allocation);
UI_API ui_status_t ui_cap_json_canonicalize(const char *json, size_t length,
    char **canonical, size_t *canonical_length);
UI_API ui_status_t ui_cap_json_fingerprint(const char *json, size_t length,
    char sha256_hex[65]);
UI_API ui_status_t ui_cap_json_validate(const char *schema_json,
    const char *json, size_t length);

UI_API ui_status_t ui_capability_register_resource(ui_host_t *, const ui_cap_resource_desc_t *);
UI_API ui_status_t ui_capability_begin_resource_unregistration(ui_host_t *, const char *id);
UI_API ui_status_t ui_capability_finish_resource_unregistration(ui_host_t *, const char *id);
UI_API ui_status_t ui_capability_register(ui_host_t *, const ui_capability_desc_t *);
UI_API ui_status_t ui_capability_begin_unregistration(ui_host_t *, const char *id);
UI_API ui_status_t ui_capability_finish_unregistration(ui_host_t *, const char *id);

/* Pagination: category NULL/empty means all, otherwise exact match; offset is
 * within the filtered items and starts at zero; limit 1..32. generation=0 starts a scan;
 * otherwise must match. A mismatch returns CANCELLED without a page. Discover
 * returns {items:[summary...],next:offset,generation:"decimal"}; next=0 ends.
 * describe returns canonical metadata including both schemas and fingerprint. */
UI_API ui_status_t ui_capability_discover(ui_host_t *, const char *category,
    size_t offset, size_t limit, uint64_t *generation, char **page_json);
UI_API ui_status_t ui_capability_describe(ui_host_t *, const char *id, char **json);
UI_API ui_status_t ui_capability_resources(ui_host_t *, size_t offset,
    ui_cap_target_t *targets, size_t capacity, size_t *count, size_t *next,
    uint64_t *generation);
UI_API ui_status_t ui_capability_bind(ui_host_t *, const char *resource_id,
    ui_cap_target_t *target);

/* prepare copies everything, establishes the result record, and does not execute.
 * dispatch rechecks revision/availability/modal/close; duplicate dispatch never
 * executes twice. OK means accepted, get_result supplies the business outcome.
 * complete uses a framework-generated non-reused ID, including synchronous
 * completion inside execute. A duplicate terminal complete returns ALREADY_EXISTS;
 * forgotten IDs return NOT_FOUND. Neither invokes business code again. */
UI_API ui_status_t ui_capability_prepare(ui_host_t *, const char *capability_id,
    const ui_cap_target_t *, const char *params_json, uint64_t *call_id);
UI_API ui_status_t ui_capability_dispatch(ui_host_t *, uint64_t call_id);
/* Application completion certifies that its workers/external callbacks for this
 * call have actually stopped. Posting a result is not proof of worker exit.
 * The framework tracks call/UI-callback leases, not arbitrary app thread handles;
 * poll readiness without blocking the UI and retain the call until drained. */
UI_API ui_status_t ui_capability_complete(ui_host_t *, uint64_t call_id, const ui_cap_result_t *);
UI_API ui_status_t ui_capability_cancel(ui_host_t *, uint64_t call_id);
UI_API ui_status_t ui_capability_get_result(ui_host_t *, uint64_t call_id, ui_cap_result_t *);
/* Only terminal records, outside callbacks. Pending returns PENDING. */
UI_API ui_status_t ui_capability_forget(ui_host_t *, uint64_t call_id);
/* Irreversible close gate, cancels prepared calls and requests running cancellation.
 * Running calls stay alive until real completion, including ignored cancellation.
 * destroy(host) is a no-op while callbacks/calls remain; retry after draining.
 * Workspace uses its own reversible close gate: WAIT then REFUSE reopens new
 * admission but does not resurrect cancelled calls. Explicit quiesce is not
 * undone. Standalone hosts use quiesce first. */
UI_API ui_status_t ui_capability_quiesce(ui_host_t *);

#ifdef __cplusplus
}
#endif
#endif
