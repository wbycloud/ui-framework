#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>

#include <stdio.h>
#include <string.h>

#include "ui_framework/opengl.h"

static int failures;
static int modern_supported;

static void check(int condition, const char *description)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", description);
        ++failures;
    }
}

static void render(ui_surface_t *surface)
{
    ui_rect_t pixels;
    RECT actual;
    check(ui_surface_get_pixel_rect(surface, &pixels) == UI_STATUS_OK,
          "query framebuffer rectangle");
    check(GetClientRect((HWND)ui_surface_native_handle(surface), &actual) &&
              actual.right == pixels.width && actual.bottom == pixels.height,
          "actual drawable follows framebuffer rectangle");
    check(ui_surface_make_current(surface) == UI_STATUS_OK,
          "make rendering context current");
    glViewport(0, 0, pixels.width, pixels.height);
    glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    check(glGetError() == GL_NO_ERROR, "viewport and clear after reflow");
    check(ui_surface_swap_buffers(surface) == UI_STATUS_OK, "swap framebuffer");
}

static ui_surface_t *try_modern(ui_host_t *host, const char *id,
                                ui_opengl_profile_t profile,
                                int samples, int debug_context)
{
    ui_surface_desc_t descriptor;
    ui_opengl_config_t config;
    ui_opengl_info_t info;
    ui_surface_t *surface;
    ui_status_t status = UI_STATUS_PLATFORM_ERROR;
    HDC previous_dc = wglGetCurrentDC();
    HGLRC previous_context = wglGetCurrentContext();

    memset(&descriptor, 0, sizeof(descriptor));
    descriptor.size = sizeof(descriptor);
    descriptor.id = id;
    descriptor.kind = UI_SURFACE_OPENGL;
    descriptor.rect.width = 320;
    descriptor.rect.height = 240;
    memset(&config, 0, sizeof(config));
    config.size = sizeof(config);
    config.major_version = 3;
    config.minor_version = 3;
    config.profile = profile;
    config.samples = samples;
    config.debug_context = debug_context;
    surface = ui_opengl_surface_create(host, &descriptor, &config, &status);
    check(wglGetCurrentDC() == previous_dc &&
              wglGetCurrentContext() == previous_context,
          "modern creation restores previous current context");
    if (surface == NULL) {
        check(status == UI_STATUS_UNSUPPORTED,
              "unavailable modern configuration reports unsupported");
        printf("SKIP %s: requested OpenGL 3.3 profile=%d samples=%d debug=%d unavailable\n",
               id, (int)profile, samples, debug_context);
        return NULL;
    }
    ++modern_supported;
    check(status == UI_STATUS_OK, "modern creation success status");
    memset(&info, 0, sizeof(info));
    info.size = sizeof(info);
    check(ui_opengl_surface_get_info(surface, &info) == UI_STATUS_OK,
          "query modern GPU context info");
    check(wglGetCurrentDC() == previous_dc &&
              wglGetCurrentContext() == previous_context,
          "GPU query restores previous current context");
    check(info.major_version > 3 ||
              (info.major_version == 3 && info.minor_version >= 3),
          "modern context honors requested version");
    check(info.profile == profile && !info.legacy_context,
          "modern context honors requested profile");
    check(samples > 0 ? info.samples >= samples : info.samples == 0,
          "modern pixel format honors MSAA request");
    check(!debug_context || info.debug_context,
          "modern context honors debug request");
    check(ui_opengl_surface_get_proc_address(surface, "glCreateShader") != NULL,
          "modern extension procedure lookup");
    check(wglGetCurrentContext() == previous_context,
          "procedure lookup restores previous context");
    printf("%s: %s | %s | %s samples=%d debug=%d\n",
           id, info.vendor, info.renderer, info.version,
           info.samples, info.debug_context);
    return surface;
}

int main(void)
{
    static const uint32_t dpis[] = {96u, 144u, 192u};
    WNDCLASSA window_class;
    HWND window;
    ui_host_config_t host_config;
    ui_surface_desc_t descriptor;
    ui_opengl_config_t config;
    ui_opengl_info_t info;
    ui_host_t *host;
    ui_surface_t *legacy;
    ui_surface_t *core;
    ui_surface_t *compatibility;
    ui_surface_t *msaa_debug;
    ui_surface_t *invalid;
    ui_status_t status;
    ui_rect_t pixels;
    HGLRC previous_context;
    size_t index;

    check(ui_framework_initialize() == UI_STATUS_OK, "initialize framework");
    memset(&window_class, 0, sizeof(window_class));
    window_class.lpfnWndProc = DefWindowProcA;
    window_class.hInstance = GetModuleHandleA(NULL);
    window_class.lpszClassName = "UiOpenGLIntegrationWindow";
    check(RegisterClassA(&window_class) != 0, "register integration host");
    window = CreateWindowExA(0, window_class.lpszClassName, "OpenGL test",
                            WS_OVERLAPPEDWINDOW, 0, 0, 800, 600,
                            NULL, NULL, window_class.hInstance, NULL);
    if (window == NULL) {
        return 1;
    }
    memset(&host_config, 0, sizeof(host_config));
    host_config.size = sizeof(host_config);
    host_config.api_version = UI_FRAMEWORK_API_VERSION;
    host_config.native_parent = window;
    host = ui_host_create(&host_config);
    if (host == NULL) {
        DestroyWindow(window);
        return 1;
    }
    check(ui_host_set_dpi(host, 96u) == UI_STATUS_OK, "initialize test DPI");
    check(ui_host_resize(host, 800, 600) == UI_STATUS_OK, "initial host size");
    memset(&descriptor, 0, sizeof(descriptor));
    descriptor.size = sizeof(descriptor);
    descriptor.id = "gl.legacy";
    descriptor.kind = UI_SURFACE_OPENGL;
    descriptor.visible = 1;
    memset(&config, 0, sizeof(config));
    config.size = sizeof(config);
    config.legacy_context = 1;
    legacy = ui_opengl_surface_create(host, &descriptor, &config, &status);
    check(legacy != NULL && status == UI_STATUS_OK, "explicit legacy context");
    if (legacy == NULL) {
        ui_host_destroy(host);
        DestroyWindow(window);
        return 1;
    }
    check(ui_surface_set_layout_region(legacy, UI_LAYOUT_REGION_MAIN) == UI_STATUS_OK,
          "bind OpenGL surface to main layout");
    render(legacy);
    previous_context = wglGetCurrentContext();
    memset(&info, 0, sizeof(info));
    info.size = sizeof(info);
    check(ui_opengl_surface_get_info(legacy, &info) == UI_STATUS_OK &&
              info.legacy_context && info.vendor[0] && info.renderer[0],
          "legacy GPU information");
    printf("legacy: %s | %s | %s\n", info.vendor, info.renderer, info.version);
    check(wglGetCurrentContext() == previous_context, "legacy info preserves context");
    check(ui_opengl_surface_get_proc_address(legacy, "glClear") != NULL,
          "OpenGL 1.1 DLL procedure lookup");

    core = try_modern(host, "gl.core", UI_OPENGL_PROFILE_CORE, 0, 0);
    compatibility = try_modern(host, "gl.compatibility", UI_OPENGL_PROFILE_COMPATIBILITY, 0, 0);
    msaa_debug = try_modern(host, "gl.msaa_debug", UI_OPENGL_PROFILE_CORE, 4, 1);

    descriptor.id = "gl.impossible";
    config.legacy_context = 0;
    config.major_version = 99;
    invalid = ui_opengl_surface_create(host, &descriptor, &config, &status);
    check(invalid == NULL && status == UI_STATUS_UNSUPPORTED,
          "impossible OpenGL version is never downgraded");
    check(wglGetCurrentContext() == previous_context,
          "failed context creation preserves current context");
    config.legacy_context = 1;
    invalid = ui_opengl_surface_create(host, &descriptor, &config, &status);
    check(invalid == NULL && status == UI_STATUS_INVALID_ARGUMENT,
          "legacy configuration rejects modern requirements");

    for (index = 0; index < sizeof(dpis) / sizeof(dpis[0]); ++index) {
        check(ui_host_set_dpi(host, dpis[index]) == UI_STATUS_OK, "DPI reflow");
        check(ui_surface_get_pixel_rect(legacy, &pixels) == UI_STATUS_OK &&
                  pixels.width == 800 * (int)dpis[index] / 96 &&
                  pixels.height == 600 * (int)dpis[index] / 96,
              "OpenGL framebuffer follows DPI");
        render(legacy);
        if (core != NULL) {
            render(core);
        }
        if (compatibility != NULL) {
            render(compatibility);
        }
        if (msaa_debug != NULL) {
            render(msaa_debug);
        }
    }
    check(ui_host_handle_message(host, window, WM_SIZE, SIZE_MINIMIZED, 0, NULL) == UI_STATUS_OK,
          "OpenGL minimize message");
    check(ui_surface_get_pixel_rect(legacy, &pixels) == UI_STATUS_OK &&
              pixels.width == 0 && pixels.height == 0,
          "minimize produces zero framebuffer size");
    check(ui_host_handle_message(host, window, WM_SIZE, SIZE_RESTORED,
                                 (intptr_t)MAKELPARAM(1280, 960), NULL) == UI_STATUS_OK,
          "OpenGL restore message");
    render(legacy);
    if (core != NULL) {
        render(core);
    }

    check(ui_surface_make_current(legacy) == UI_STATUS_OK, "legacy context before cleanup");
    if (core != NULL) {
        ui_surface_destroy(core);
    }
    check(wglGetCurrentContext() == previous_context,
          "destroying another context preserves current context");
    ui_host_destroy(host);
    check(wglGetCurrentContext() == NULL, "destroying current surface releases current context");
    DestroyWindow(window);
    (void)UnregisterClassA(window_class.lpszClassName, window_class.hInstance);
    printf("OpenGL integration: %d failure(s), %d modern configurations supported\n",
           failures, modern_supported);
    return failures != 0;
}
