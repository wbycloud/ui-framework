#ifndef UI_FRAMEWORK_NATIVE_H
#define UI_FRAMEWORK_NATIVE_H

#include <stdint.h>

#include "ui_framework/ui.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_native_shell ui_native_shell_t;

#define UI_NATIVE_SHELL_MANAGED_ACTIVATION 1u

/*
 * The native shell materializes the registrations held by a ui_host as
 * Win32 controls. native_parent owns the content controls; menu_owner is the
 * top-level menu window, defaulting to native_parent. Neither is owned by the
 * shell. The original descriptor size remains accepted, with legacy behavior.
 */
typedef struct ui_native_shell_config {
    uint32_t size;
    ui_host_t *host;
    void *native_parent;
    void *menu_owner;
    uint32_t flags;
} ui_native_shell_config_t;

UI_API ui_native_shell_t *ui_native_shell_create(
    const ui_native_shell_config_t *config);
UI_API void ui_native_shell_destroy(ui_native_shell_t *shell);

/* Managed shells start inactive. Activation preserves all content windows and
 * installs the shell menu. Deactivation hides controls, floating panels and the
 * content parent, and detaches its menu only if still attached. The owner must
 * install any fallback menu. Legacy shells start active and restore their
 * previous menu when detached. Call on the UI thread. */
UI_API ui_status_t ui_native_shell_set_active(ui_native_shell_t *shell,
                                               int active);

/* Borrowed HMENU, valid until refresh/destroy. Do not destroy it. */
UI_API void *ui_native_shell_menu_handle(ui_native_shell_t *shell);

/* Rebuild menus, toolbars, and registered panels from the host registry. */
UI_API ui_status_t ui_native_shell_refresh(ui_native_shell_t *shell);

/* Apply the host's logical layout rectangles to native child windows. */
UI_API ui_status_t ui_native_shell_reflow(ui_native_shell_t *shell);

/*
 * Borrowed native panel container. Applications may create content children
 * inside it. It survives docking/floating changes, but refresh and destroy
 * invalidate it and its children. No HWND type crosses the public C ABI.
 */
UI_API void *ui_native_shell_panel_handle(ui_native_shell_t *shell,
                                          const char *panel_id);
/* Float a sidebar or return it to the dock when its current layout permits. */
UI_API ui_status_t ui_native_shell_set_panel_floating(ui_native_shell_t *shell,
                                                       const char *panel_id,
                                                       int floating);

/* Forward commands from the content parent or menu owner. Size/DPI messages
 * apply only to the content parent. Inactive managed shells return NOT_FOUND. */
UI_API ui_status_t ui_native_shell_handle_message(
    ui_native_shell_t *shell,
    void *hwnd,
    uint32_t message,
    uintptr_t w_param,
    intptr_t l_param,
    intptr_t *result);

#ifdef __cplusplus
}
#endif

#endif
