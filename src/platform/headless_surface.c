#include "../ui_internal.h"
#include "ui_framework/opengl.h"

ui_status_t ui_platform_prepare_dpi(void)
{
    return UI_STATUS_OK;
}

uint32_t ui_platform_get_dpi(void *native_parent)
{
    (void)native_parent;
    return 96u;
}

ui_status_t ui_platform_host_handle_message(ui_host_t *host,
                                            void *hwnd,
                                            uint32_t message,
                                            uintptr_t w_param,
                                            intptr_t l_param,
                                            intptr_t *result)
{
    (void)host;
    (void)hwnd;
    (void)message;
    (void)w_param;
    (void)l_param;
    (void)result;
    return UI_STATUS_UNSUPPORTED;
}

ui_status_t ui_platform_surface_create(ui_surface_t *surface)
{
    (void)surface;
    return UI_STATUS_UNSUPPORTED;
}

ui_status_t ui_platform_surface_create_configured(
    ui_surface_t *surface,
    const ui_opengl_config_t *config)
{
    (void)surface;
    (void)config;
    return UI_STATUS_UNSUPPORTED;
}

ui_status_t ui_opengl_surface_get_info(const ui_surface_t *surface,
                                      ui_opengl_info_t *info)
{
    (void)surface;
    (void)info;
    return UI_STATUS_UNSUPPORTED;
}

ui_opengl_proc_t ui_opengl_surface_get_proc_address(
    const ui_surface_t *surface,
    const char *name)
{
    (void)surface;
    (void)name;
    return NULL;
}

void ui_platform_surface_destroy(ui_surface_t *surface)
{
    (void)surface;
}

ui_status_t ui_platform_surface_set_rect(ui_surface_t *surface,
                                         const ui_rect_t *rect)
{
    (void)surface;
    (void)rect;
    return UI_STATUS_UNSUPPORTED;
}

ui_status_t ui_platform_surface_set_visible(ui_surface_t *surface,
                                            int visible)
{
    (void)surface;
    (void)visible;
    return UI_STATUS_UNSUPPORTED;
}

ui_status_t ui_platform_surface_invalidate(ui_surface_t *surface)
{
    if (surface == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    if (surface->frame != NULL) {
        surface->frame(surface, surface->callback_user_data);
    }
    return UI_STATUS_OK;
}

ui_status_t ui_platform_surface_make_current(ui_surface_t *surface)
{
    (void)surface;
    return UI_STATUS_UNSUPPORTED;
}

ui_status_t ui_platform_surface_swap_buffers(ui_surface_t *surface)
{
    (void)surface;
    return UI_STATUS_UNSUPPORTED;
}
