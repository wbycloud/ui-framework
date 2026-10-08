#ifndef UI_FRAMEWORK_LANGUAGE_H
#define UI_FRAMEWORK_LANGUAGE_H
#include "ui_framework/ui.h"
#ifdef __cplusplus
extern "C" {
#endif
/* API9. Preference and persistence belong to the application, never the host. */
typedef enum ui_language_source { UI_LANGUAGE_APPLICATION=1,
    UI_LANGUAGE_WINDOWS_DISPLAY=2, UI_LANGUAGE_PLATFORM_DEFAULT=3 } ui_language_source_t;
typedef enum ui_language_provider { UI_LANGUAGE_USER_PREFERRED_UI=1,
    UI_LANGUAGE_USER_DEFAULT_UI=2, UI_LANGUAGE_NO_PLATFORM_UI=3 } ui_language_provider_t;
typedef struct ui_language_info {
    uint32_t size;
    ui_language_source_t source;
    uint64_t generation;
    char language[16]; /* effective zh-CN or en-US, copied UTF-8 */
    char system_language[85]; /* actual first Windows UI name; empty if unavailable */
    uint32_t fallback;
    ui_language_provider_t system_provider;
} ui_language_info_t;
typedef void (*ui_language_callback_fn)(ui_host_t *,const ui_language_info_t *,void *);
/* UI-thread. Explicit tokens are exactly zh-CN/en-US; unsupported tokens fail
 * without changing any state. NULL/empty/malformed fail INVALID_ARGUMENT.
 * Unsubmitted hosts initially use Windows display UI language; reset refreshes
 * that default. No setlocale, thread/process/browser preference is changed.
 * Notification is synchronous once per effective/source change, guarded by the
 * normal dispatch scope. Query/text updates are allowed in the callback;
 * recursive language setting/reset returns CANCELLED. Info is callback-borrowed.
 * One app-owned callback per host, NULL removes it, including closing/unmount.
 * Registering during blocked dispatch fails CANCELLED. Destruction discards it.
 * Also emits ui.host.language_changed to the existing workspace observer. */
UI_API ui_status_t ui_host_set_language(ui_host_t *,const char *);
UI_API ui_status_t ui_host_reset_language(ui_host_t *);
UI_API ui_status_t ui_host_get_language(const ui_host_t *,ui_language_info_t *);
UI_API ui_status_t ui_host_get_system_language(ui_language_info_t *);
UI_API ui_status_t ui_host_set_language_callback(ui_host_t *,ui_language_callback_fn,void *);
typedef enum ui_text_target { UI_TEXT_COMMAND=1, UI_TEXT_MENU_ITEM=2,
    UI_TEXT_TOOLBAR=3, UI_TEXT_TOOLBAR_ITEM=4, UI_TEXT_PANEL=5,
    UI_TEXT_MENU_GROUP=6 } ui_text_target_t;
/* Copied UTF-8 title, max4095 bytes; stable ID/path and semantic behavior unchanged.
 * UI-thread, valid during language callback. Does not reload/re-register objects. */
UI_API ui_status_t ui_host_set_title(ui_host_t *,ui_text_target_t,const char *,const char *);
#ifdef __cplusplus
}
#endif
#endif
