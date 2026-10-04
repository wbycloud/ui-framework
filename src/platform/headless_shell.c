#include "../ui_internal.h"
#include "ui_framework/native.h"
#include "ui_framework/shell.h"

#include <stdlib.h>
#include <string.h>

struct ui_shell { ui_native_shell_t *native; };
struct ui_content_slot {
    ui_shell_t *shell;
    char *panel_id;
    ui_surface_t *surface;
    ui_web_view_t *view;
    struct ui_content_slot *next;
};

struct ui_native_shell {
    ui_host_t *host;
    ui_shell_t shell;
    ui_content_slot_t *slots;
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
        shell->shell.native = shell;
        config->host->shell = &shell->shell;
        if (ui_shell_refresh(&shell->shell) != UI_STATUS_OK) {
            ui_native_shell_destroy(shell);
            return NULL;
        }
    }
    return shell;
}

void ui_native_shell_destroy(ui_native_shell_t *shell)
{
    if (shell != NULL) {
        ui_content_slot_t *slot;
        if (shell->host->shell == &shell->shell) shell->host->shell = NULL;
        while ((slot = shell->slots) != NULL) {
            shell->slots = slot->next;
            free(slot->panel_id);
            free(slot);
        }
    }
    free(shell);
}

ui_status_t ui_native_shell_set_active(ui_native_shell_t *shell, int active)
{
    if (shell != NULL) ui_components_active(shell->host, active);
    return shell == NULL ? UI_STATUS_INVALID_ARGUMENT : UI_STATUS_OK;
}

void *ui_native_shell_menu_handle(ui_native_shell_t *shell)
{
    (void)shell;
    return NULL;
}

ui_status_t ui_native_shell_refresh(ui_native_shell_t *shell)
{
    return shell == NULL ? UI_STATUS_INVALID_ARGUMENT : ui_shell_refresh(&shell->shell);
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

ui_native_shell_t *ui_native_shell_create_web(const ui_native_shell_config_t *config)
{
    (void)config;
    return NULL;
}

ui_shell_t *ui_host_get_shell(ui_host_t *host)
{
    return host != NULL ? host->shell : NULL;
}

ui_content_slot_t *ui_shell_get_content_slot(ui_shell_t *shell, const char *panel_id)
{
    ui_content_slot_t *slot;
    if (shell == NULL) return NULL;
    for (slot = shell->native->slots; slot != NULL; slot = slot->next)
        if ((panel_id == NULL && slot->panel_id == NULL) ||
            (panel_id != NULL && slot->panel_id != NULL && strcmp(panel_id, slot->panel_id) == 0))
            return slot;
    return NULL;
}

static ui_status_t add_slot(ui_shell_t *shell, const char *id)
{
    ui_content_slot_t *slot;
    if (ui_shell_get_content_slot(shell, id) != NULL) return UI_STATUS_OK;
    slot = (ui_content_slot_t *)calloc(1, sizeof(*slot));
    if (slot == NULL) return UI_STATUS_OUT_OF_MEMORY;
    if (id != NULL && (slot->panel_id = ui_strdup(id)) == NULL) {
        free(slot); return UI_STATUS_OUT_OF_MEMORY;
    }
    slot->shell = shell; slot->next = shell->native->slots;
    shell->native->slots = slot;
    return UI_STATUS_OK;
}

ui_status_t ui_shell_refresh(ui_shell_t *shell)
{
    ui_panel_entry_t *panel;
    ui_status_t status;
    if (shell == NULL) return UI_STATUS_INVALID_ARGUMENT;
    status = add_slot(shell, NULL);
    for (panel = shell->native->host->panels; panel != NULL && status == UI_STATUS_OK; panel = panel->next)
        status = add_slot(shell, panel->id);
    return status;
}

ui_status_t ui_content_slot_get_rect(const ui_content_slot_t *slot, ui_rect_t *rect)
{
    ui_panel_entry_t *panel;
    if (slot == NULL || rect == NULL) return UI_STATUS_INVALID_ARGUMENT;
    if (slot->panel_id == NULL)
        return ui_host_get_rect(slot->shell->native->host, UI_LAYOUT_REGION_MAIN, rect);
    for (panel = slot->shell->native->host->panels; panel != NULL; panel = panel->next) {
        if (strcmp(panel->id, slot->panel_id) == 0) {
            if (panel->kind == UI_PANEL_FLOATING) {
                *rect = (ui_rect_t){0, 0, panel->preferred_width > 0 ? panel->preferred_width : 320, 240};
                return UI_STATUS_OK;
            }
            return ui_host_get_rect(slot->shell->native->host, panel->dock_region, rect);
        }
    }
    return UI_STATUS_NOT_FOUND;
}

ui_status_t ui_content_slot_get_pixel_rect(const ui_content_slot_t *slot, ui_rect_t *rect)
{
    ui_rect_t logical;
    uint32_t dpi;
    ui_status_t status = ui_content_slot_get_rect(slot, &logical);
    if (status != UI_STATUS_OK || rect == NULL)
        return rect == NULL ? UI_STATUS_INVALID_ARGUMENT : status;
    dpi = slot->shell->native->host->dpi;
    rect->x = (int)(((int64_t)logical.x * dpi + 48) / 96);
    rect->y = (int)(((int64_t)logical.y * dpi + 48) / 96);
    rect->width = (int)(((int64_t)(logical.x + logical.width) * dpi + 48) / 96) - rect->x;
    rect->height = (int)(((int64_t)(logical.y + logical.height) * dpi + 48) / 96) - rect->y;
    return UI_STATUS_OK;
}

void *ui_content_slot_native_handle(const ui_content_slot_t *slot)
{
    (void)slot;
    return NULL;
}

ui_status_t ui_content_slot_attach_surface(ui_content_slot_t *slot, ui_surface_t *surface)
{
    if (slot == NULL || (surface != NULL && surface->host != slot->shell->native->host))
        return UI_STATUS_INVALID_ARGUMENT;
    if (surface != NULL) return UI_STATUS_UNSUPPORTED;
    slot->surface = NULL;
    return UI_STATUS_OK;
}

ui_status_t ui_content_slot_attach_web_view(ui_content_slot_t *slot, ui_web_view_t *view)
{
    ui_rect_t rect;
    if (slot == NULL || (view != NULL && view->host != slot->shell->native->host))
        return UI_STATUS_INVALID_ARGUMENT;
    slot->view = view;
    if (view == NULL) return UI_STATUS_OK;
    if (ui_content_slot_get_rect(slot, &rect) != UI_STATUS_OK) return UI_STATUS_NOT_FOUND;
    if (slot->panel_id != NULL) rect.x = rect.y = 0;
    return ui_web_view_set_rect(view, &rect, slot->shell->native->host->dpi);
}

ui_status_t ui_shell_get_slot_state(ui_shell_t *shell, const char *panel_id,
                                    ui_rect_t *frame, ui_rect_t *content,
                                    int *visible, int *floating)
{
    ui_content_slot_t *slot = ui_shell_get_content_slot(shell, panel_id);
    ui_rect_t rect;
    ui_status_t status;
    if (slot == NULL) return shell == NULL ? UI_STATUS_INVALID_ARGUMENT : UI_STATUS_NOT_FOUND;
    status = ui_content_slot_get_rect(slot, &rect);
    if (status != UI_STATUS_OK) return status;
    if (frame != NULL) *frame = rect;
    if (content != NULL) *content = rect;
    if (visible != NULL) *visible = rect.width > 0 && rect.height > 0;
    if (floating != NULL) *floating = 0;
    return UI_STATUS_OK;
}

void ui_shell_layout_changed(ui_host_t *host) { if(host)ui_components_layout(host); }
int ui_shell_surface_bound(ui_host_t *host, const ui_surface_t *surface)
{
    (void)host; (void)surface; return 0;
}
int ui_shell_web_view_bound(ui_host_t *host, const ui_web_view_t *view)
{
    (void)host; (void)view; return 0;
}
uint32_t ui_shell_surface_dpi(ui_host_t *host, const ui_surface_t *surface)
{
    (void)surface; return host != NULL ? host->dpi : 96u;
}
void ui_shell_surface_destroyed(ui_host_t *host, ui_surface_t *surface)
{
    ui_content_slot_t *slot;
    if (host == NULL || host->shell == NULL) return;
    for (slot = host->shell->native->slots; slot != NULL; slot = slot->next)
        if (slot->surface == surface) slot->surface = NULL;
}
void ui_shell_web_view_destroyed(ui_host_t *host, ui_web_view_t *view)
{
    ui_content_slot_t *slot;
    if (host == NULL || host->shell == NULL) return;
    for (slot = host->shell->native->slots; slot != NULL; slot = slot->next)
        if (slot->view == view) slot->view = NULL;
}

ui_native_shell_t *ui_native_shell_create_offscreen(const ui_native_shell_config_t *config)
{return ui_native_shell_create(config);}
int ui_content_slot_belongs_to(const ui_content_slot_t *slot,const ui_host_t *host)
{return slot&&slot->shell&&slot->shell->native->host==host;}
#include "headless_layout.inc"
