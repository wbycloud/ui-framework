#include "../ui_internal.h"
#include "ui_framework/native.h"

#include <stdlib.h>

struct ui_native_shell {
    ui_host_t *host;
};

ui_native_shell_t *ui_native_shell_create(
    const ui_native_shell_config_t *config)
{
    ui_native_shell_t *shell;

    if (config == NULL || config->host == NULL ||
        config->size < offsetof(ui_native_shell_config_t, menu_owner) ||
        (config->size >= offsetof(ui_native_shell_config_t, flags) + sizeof(config->flags) &&
         (config->flags & ~UI_NATIVE_SHELL_MANAGED_ACTIVATION) != 0u)) {
        return NULL;
    }
    shell = (ui_native_shell_t *)calloc(1u, sizeof(*shell));
    if (shell != NULL) {
        shell->host = config->host;
    }
    return shell;
}

void ui_native_shell_destroy(ui_native_shell_t *shell)
{
    free(shell);
}

ui_status_t ui_native_shell_set_active(ui_native_shell_t *shell, int active)
{
    (void)active;
    return shell == NULL ? UI_STATUS_INVALID_ARGUMENT : UI_STATUS_OK;
}

void *ui_native_shell_menu_handle(ui_native_shell_t *shell)
{
    (void)shell;
    return NULL;
}

ui_status_t ui_native_shell_refresh(ui_native_shell_t *shell)
{
    return shell == NULL ? UI_STATUS_INVALID_ARGUMENT : UI_STATUS_OK;
}

ui_status_t ui_native_shell_reflow(ui_native_shell_t *shell)
{
    return shell == NULL ? UI_STATUS_INVALID_ARGUMENT : UI_STATUS_OK;
}

void *ui_native_shell_panel_handle(ui_native_shell_t *shell, const char *panel_id)
{
    (void)shell;
    (void)panel_id;
    return NULL;
}

ui_status_t ui_native_shell_set_panel_floating(ui_native_shell_t *shell,
                                               const char *panel_id,
                                               int floating)
{
    (void)floating;
    return shell == NULL || panel_id == NULL
               ? UI_STATUS_INVALID_ARGUMENT : UI_STATUS_UNSUPPORTED;
}

ui_status_t ui_native_shell_handle_message(ui_native_shell_t *shell,
                                            void *hwnd,
                                            uint32_t message,
                                            uintptr_t w_param,
                                            intptr_t l_param,
                                            intptr_t *result)
{
    (void)hwnd;
    (void)message;
    (void)w_param;
    (void)l_param;
    if (result != NULL) {
        *result = 0;
    }
    return shell == NULL ? UI_STATUS_INVALID_ARGUMENT : UI_STATUS_NOT_FOUND;
}
