#ifndef UI_FRAMEWORK_OPENGL_H
#define UI_FRAMEWORK_OPENGL_H

#include "ui.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ui_opengl_profile {
    UI_OPENGL_PROFILE_ANY = 0,
    UI_OPENGL_PROFILE_CORE = 1,
    UI_OPENGL_PROFILE_COMPATIBILITY = 2
} ui_opengl_profile_t;

typedef struct ui_opengl_config {
    uint32_t size;
    int major_version;
    int minor_version;
    ui_opengl_profile_t profile;
    /* Zero disables MSAA. Windowed: requests at least this many samples.
     * Offscreen/windowless: exact positive count, or UNSUPPORTED. */
    int samples;
    int debug_context;
    /* Explicit legacy path: all version/profile/sample/debug fields must be zero. */
    int legacy_context;
} ui_opengl_config_t;

typedef struct ui_opengl_info {
    uint32_t size;
    char vendor[128];
    char renderer[256];
    char version[128];
    char shading_language_version[128];
    int major_version;
    int minor_version;
    ui_opengl_profile_t profile;
    int samples;
    int debug_context;
    int legacy_context;
} ui_opengl_info_t;

typedef void (*ui_opengl_proc_t)(void);
typedef enum ui_opengl_window_dependency { UI_OPENGL_VISIBLE_WINDOW=0, UI_OPENGL_HIDDEN_WINDOW=1, UI_OPENGL_NO_WINDOW=2 } ui_opengl_window_dependency_t;
typedef struct ui_opengl_windowless_config {
    uint32_t size;
    /* Absolute UTF-8 path to an x64 OSMesa DLL and adjacent dependencies.
     * Explicit software desktop GL provider. No HWND/DC is created or used.
     * All frame GL calls MUST use this surface's get_proc_address, including
     * GL1.1 calls. opengl32 imports belong to WGL, a different provider.
     * Contexts and attachments are released on destruction. OSMesa's process
     * screen/worker cache keeps the provider module loaded until process exit. */
    const char *library_path_utf8;
} ui_opengl_windowless_config_t;
UI_API ui_surface_t *ui_opengl_windowless_surface_create(ui_host_t *,const ui_surface_desc_t *,const ui_opengl_config_t *,const ui_opengl_windowless_config_t *,ui_status_t *);
/* Windows hidden WGL drawable + private FBO; requires >=3.3 compatibility.
 * Exact FBO samples (0 => single sample), no legacy fallback.
 * UI/context owner thread only. Not Session0 support. */
UI_API ui_surface_t *ui_opengl_offscreen_surface_create(ui_host_t *,const ui_surface_desc_t *,const ui_opengl_config_t *,ui_status_t *);
UI_API ui_status_t ui_opengl_surface_get_window_dependency(const ui_surface_t *,ui_opengl_window_dependency_t *);
/* NULL pixels queries size without executing frame. Otherwise invokes existing
 * frame once and synchronously reads RGBA8, top-down, caller-owned positive
 * stride/capacity. Alpha is the framebuffer alpha, unmodified. Restores previous
 * WGL context, framebuffer bindings and viewport. Does not present to desktop. */
UI_API ui_status_t ui_opengl_offscreen_render(ui_surface_t *,ui_pixel_buffer_t *);
UI_API ui_status_t ui_surface_dispatch_input(ui_surface_t *,const ui_input_event_t *);

/*
 * Creates an OpenGL surface with an explicit configuration. Modern requests
 * are never silently downgraded: unsupported version/profile/MSAA/debug
 * requirements return NULL and UI_STATUS_UNSUPPORTED through status.
 * Set legacy_context=1 with other fields zero for the compatibility test path.
 * ui_surface_create continues to use the original legacy context path.
 */
UI_API ui_surface_t *ui_opengl_surface_create(
    ui_host_t *host,
    const ui_surface_desc_t *desc,
    const ui_opengl_config_t *config,
    ui_status_t *status);

/* Copies driver strings and actual context properties; restores prior context. */
UI_API ui_status_t ui_opengl_surface_get_info(const ui_surface_t *surface,
                                              ui_opengl_info_t *info);
/* The returned function pointer belongs to this context; make it current to call. */
UI_API ui_opengl_proc_t ui_opengl_surface_get_proc_address(
    const ui_surface_t *surface,
    const char *name);

#ifdef __cplusplus
}
#endif

#endif
