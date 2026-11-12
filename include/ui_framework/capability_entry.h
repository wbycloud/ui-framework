#ifndef UI_FRAMEWORK_CAPABILITY_ENTRY_H
#define UI_FRAMEWORK_CAPABILITY_ENTRY_H

#include "capability.h"

#ifdef __cplusplus
extern "C" {
#endif

/* API10 C1 additive contract. This is a projection of the v1 read registry,
 * not a second directory or a manual UI authorization interface. */
#define UI_CAPABILITY_ENTRY_VERSION 1u
typedef struct ui_cap_entry_binding {
    uint32_t size, version;
    const char *capability_id;
    const ui_cap_target_t *target;
} ui_cap_entry_binding_t;

/* UI thread only. Irreversible for this host. Assistant discovery/invocation
 * uses generated read entries only; legacy EDIT/DESTRUCTIVE invoke is denied,
 * legacy READ is hidden; Assistant begin/commit/rollback/undo are denied.
 * Ordinary legacy UI commands remain available.
 * source strings never grant authority. Unmigrated hosts are unchanged.
 * Initial activation returns PENDING while a legacy request/callback is live. */
UI_API ui_status_t ui_capability_enable_read_entries(ui_host_t *host);
/* Copies the complete target snapshot. One entry per capability, same ID,
 * name and Schema, no alias/handler/permission override. Refresh requires
 * two-phase capability unregistration and registration (never active-tab
 * rebinding). Publication advances the catalog generation. Retiring entries
 * disappear from the Assistant view and reject new calls. */
UI_API ui_status_t ui_capability_publish_read_entry(ui_host_t *host,
    const ui_cap_entry_binding_t *binding);
/* Available before execute and in synchronous result callbacks. Retained
 * with the bounded capability call record, until ui_capability_forget or host
 * teardown. Use the returned ID for result/query/cancel/dispatch/forget.
 * Assistant forget also releases its terminal capability record; inside a
 * callback it returns PENDING. No observer or application pointer is retained.
 * Workspace-owned Assistant requests are automatically collected at the next
 * safe poll boundary; copy observer data before then. Standalone hosts and
 * ordinary host-invoke callers explicitly release records. Generated source
 * IDs are at most255 bytes. Tracking is bounded by1024 requests and the same
 * 8MiB call budget; entry/target/UI title storage uses the2MiB catalog budget.
 * Host results project validated output on success, {"code":...} on failure.
 * Admission failures before prepare project {"status":...}, with no call ID.
 * Pending/source copies stay charged until the result callback returns. One
 * fixed failure slot is reserved within8MiB. While notifying a generated
 * failure, generated submissions are refused before acceptance (host invoke
 * returns0; Assistant invoke returns LIMIT_EXCEEDED with request ID0), without
 * a recursive failure notification. Retry only after returning/draining.
 * ui_host_reply cannot bypass validation for a generated request. */
UI_API ui_status_t ui_capability_request_call(ui_host_t *host,
    uint64_t request_id, uint64_t *call_id);

#ifdef __cplusplus
}
#endif
#endif
