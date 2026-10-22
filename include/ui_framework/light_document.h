#ifndef UI_FRAMEWORK_LIGHT_DOCUMENT_H
#define UI_FRAMEWORK_LIGHT_DOCUMENT_H
#include "ui_framework/light_web.h"
#include "ui_framework/images.h"
#ifdef __cplusplus
extern "C" {
#endif
/* API9 development addition. A backend created here is permanently untrusted.
 * It never creates a JS runtime; load_html, JSON bridges and trusted loading are
 * unsupported. Separate views/DOM/styles; no script, native or package access.
 * All calls/callbacks are UI-thread only. Use matching headers/library/DLL.
 * HTML <=256KiB, 1024 nodes, depth64, each text/attribute <=4095/2047 bytes.
 * Exceeding a bound fails the document, never truncates it. */
typedef enum ui_document_event_kind {
    UI_DOCUMENT_NAVIGATE=1, UI_DOCUMENT_IMAGE=2, UI_DOCUMENT_STYLESHEET=3,
    UI_DOCUMENT_CANCEL_RESOURCE=4, UI_DOCUMENT_FOCUS_OUT=5, UI_DOCUMENT_SUBMIT=6
} ui_document_event_kind_t;
typedef struct ui_document_field { const char *name_utf8, *value_utf8; } ui_document_field_t;
typedef struct ui_document_event {
    uint32_t size;
    ui_document_event_kind_t kind;
    uint64_t navigation_id, resource_id;
    const char *url_utf8; /* Resolved URL data, never a command/script. */
    const char *method; /* GET only; NULL for resources. */
    const ui_document_field_t *fields; /* Borrowed, max64 successful controls. */
    size_t field_count;
} ui_document_event_t;
typedef void (*ui_document_event_fn)(ui_web_view_t *,const ui_document_event_t *,void *);
typedef struct ui_document_load {
    uint32_t size;
    uint64_t navigation_id; /* Nonzero, caller owns monotonic generation. */
    const char *source_url, *html_utf8; /* Copied/parsed during the call. UTF-8. */
} ui_document_load_t;
typedef struct ui_document_state {
    uint32_t size;
    uint64_t navigation_id;
    uint32_t restrictions, pending_resources, failed_resources, node_count;
    int scroll_x, scroll_y, content_width, content_height;
    uint32_t zoom_percent, script_runtime; /* script_runtime is always zero. */
    char title_utf8[4096];
    double parse_ms, layout_ms, paint_ms; /* Last document, not network time. */
    uint64_t paint_count;
} ui_document_state_t;
/* 2026-10-21 reading implementation: decimal ol start/reversed/li value;
 * bounded 64-column rowspan/colspan grid (rowspan0 ends at its row group),
 * caption/empty cells, authored basic typography/spacing/whitespace and
 * source-ordered external CSS. Hidden text is excluded from find/copy.
 * Unsupported declarations/selectors count as simplification; exceeding the
 * existing node/rule/fragment limits fails. No public structure/export change.
 * Simple GET reports successful controls in tree order, including only the
 * activated/default submitter. Unsupported critical controls refuse the form. */
UI_API ui_web_backend_t *ui_light_document_backend_create(const ui_light_web_config_t *);
/* Callback cannot reload/destroy view. Post app work and handle after returning.
 * Resource requests are emitted by flush, after load/previous callback returns.
 * Removing callback is allowed at teardown; no callbacks after destroy. */
UI_API ui_status_t ui_light_document_set_event_callback(ui_web_view_t *,ui_document_event_fn,void *);
UI_API ui_status_t ui_light_document_load(ui_web_view_t *,const ui_document_load_t *);
UI_API ui_status_t ui_light_document_get_state(ui_web_view_t *,ui_document_state_t *);
/* Image is borrowed until replacement/navigation/destroy. Zero marks failure.
 * Styles accept color/background/font subset; <=64KiB per sheet, 256 rules in
 * total. Unsupported CSS is counted as simplification. Stale IDs => NOT_FOUND.
 * App owns transport policy, redirects/cancellation and image ID lifetime. */
UI_API ui_status_t ui_light_document_resource(ui_web_view_t *,uint64_t navigation_id,
    uint64_t resource_id,ui_image_id_t image,const char *css_utf8);
/* Ends pending requests, marks them failed, emits CANCEL_RESOURCE once for each
 * issued request. The transport owner cancels them; no work runs in this view. */
UI_API ui_status_t ui_light_document_cancel_resources(ui_web_view_t *);
UI_API ui_status_t ui_light_document_scroll(ui_web_view_t *,int x,int y);
UI_API ui_status_t ui_light_document_fragment(ui_web_view_t *,const char *fragment_utf8);
UI_API ui_status_t ui_light_document_zoom(ui_web_view_t *,uint32_t percent); /* 50..200, separate from DPI */
/* UTF-8 case-insensitive Windows text search, wrap, reveal and select result.
 * reverse=0 forward,1 backward; empty clears. NOT_FOUND preserves document. */
UI_API ui_status_t ui_light_document_find(ui_web_view_t *,const char *,int reverse);
/* Copies selected Unicode text to system clipboard; no script/command bridge. */
UI_API ui_status_t ui_light_document_copy(ui_web_view_t *);
#ifdef __cplusplus
}
#endif
#endif
