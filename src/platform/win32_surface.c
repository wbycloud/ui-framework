#include "../ui_internal.h"
#include "ui_framework/opengl.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <GL/gl.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_SUPPORT_OPENGL_ARB 0x2010
#define WGL_DOUBLE_BUFFER_ARB 0x2011
#define WGL_PIXEL_TYPE_ARB 0x2013
#define WGL_COLOR_BITS_ARB 0x2014
#define WGL_DEPTH_BITS_ARB 0x2022
#define WGL_STENCIL_BITS_ARB 0x2023
#define WGL_TYPE_RGBA_ARB 0x202B
#define WGL_SAMPLE_BUFFERS_ARB 0x2041
#define WGL_SAMPLES_ARB 0x2042
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_FLAGS_ARB 0x2094
#define WGL_CONTEXT_DEBUG_BIT_ARB 0x0001
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x0001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x0002
#define GL_SHADING_LANGUAGE_VERSION_VALUE 0x8B8C
#define GL_CONTEXT_FLAGS_VALUE 0x821E
#define GL_CONTEXT_PROFILE_MASK_VALUE 0x9126
#define GL_CONTEXT_DEBUG_BIT_VALUE 0x0002
#define GL_CONTEXT_CORE_PROFILE_BIT_VALUE 0x0001
#define GL_CONTEXT_COMPATIBILITY_PROFILE_BIT_VALUE 0x0002

typedef BOOL (WINAPI *choose_pixel_format_arb_fn)(
    HDC, const int *, const FLOAT *, UINT, int *, UINT *);
typedef HGLRC (WINAPI *create_context_attribs_arb_fn)(HDC, HGLRC, const int *);
typedef BOOL (WINAPI *get_pixel_format_attrib_arb_fn)(
    HDC, int, int, UINT, const int *, int *);
typedef const char *(WINAPI *get_extensions_string_arb_fn)(HDC);

typedef struct ui_win32_surface {
    HWND hwnd;
    HDC dc;
    HGLRC glrc;
    int is_opengl;
    int legacy_context;
    int samples;
    uint32_t captured_buttons;
    uint16_t pending_high_surrogate;
} ui_win32_surface_t;

static const wchar_t *surface_class_name = L"UiFrameworkSurface";
static ATOM surface_class_atom;
static int dpi_prepared;
static ui_status_t dpi_prepare_status;

ui_status_t ui_platform_prepare_dpi(void)
{
    BOOL prepared;

    if (!dpi_prepared) {
        prepared = SetProcessDpiAwarenessContext(
            DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        dpi_prepared = 1;
        if (!prepared) {
            if (GetLastError() != ERROR_ACCESS_DENIED) {
                dpi_prepare_status = UI_STATUS_PLATFORM_ERROR;
            } else if (GetAwarenessFromDpiAwarenessContext(
                           GetThreadDpiAwarenessContext()) != DPI_AWARENESS_PER_MONITOR_AWARE) {
                dpi_prepare_status = UI_STATUS_UNSUPPORTED;
            }
        }
    }
    return dpi_prepare_status;
}

uint32_t ui_platform_get_dpi(void *native_parent)
{
    HWND parent = (HWND)native_parent;
    UINT dpi;

    if (parent == NULL) {
        return 96u;
    }

    dpi = GetDpiForWindow(parent);
    return dpi != 0u ? (uint32_t)dpi : 96u;
}

static int input_pixels_to_logical(int pixels, uint32_t dpi)
{
    int64_t scaled;
    if (dpi == 0u) dpi = 96u;
    scaled = (int64_t)pixels * 96;
    return (int)((scaled + (scaled < 0 ? -(int64_t)dpi / 2 : (int64_t)dpi / 2)) / dpi);
}

static uint32_t input_modifiers(void)
{
    uint32_t modifiers = 0;
    if (GetKeyState(VK_CONTROL) & 0x8000) modifiers |= UI_INPUT_MODIFIER_CONTROL;
    if (GetKeyState(VK_SHIFT) & 0x8000) modifiers |= UI_INPUT_MODIFIER_SHIFT;
    if (GetKeyState(VK_MENU) & 0x8000) modifiers |= UI_INPUT_MODIFIER_ALT;
    return modifiers;
}

static void surface_emit_codepoint(ui_surface_t *surface, uint32_t codepoint,
                                    uint32_t modifiers)
{
    ui_input_event_t event;
    char text[5];
    if (codepoint == 0u) return;
    if (codepoint <= 0x7fu) {
        text[0] = (char)codepoint; text[1] = '\0';
    } else if (codepoint <= 0x7ffu) {
        text[0] = (char)(0xc0u | (codepoint >> 6));
        text[1] = (char)(0x80u | (codepoint & 0x3fu)); text[2] = '\0';
    } else if (codepoint <= 0xffffu) {
        text[0] = (char)(0xe0u | (codepoint >> 12));
        text[1] = (char)(0x80u | ((codepoint >> 6) & 0x3fu));
        text[2] = (char)(0x80u | (codepoint & 0x3fu)); text[3] = '\0';
    } else {
        text[0] = (char)(0xf0u | (codepoint >> 18));
        text[1] = (char)(0x80u | ((codepoint >> 12) & 0x3fu));
        text[2] = (char)(0x80u | ((codepoint >> 6) & 0x3fu));
        text[3] = (char)(0x80u | (codepoint & 0x3fu)); text[4] = '\0';
    }
    ZeroMemory(&event, sizeof(event));
    event.size = sizeof(event); event.kind = UI_INPUT_TEXT;
    event.modifiers = modifiers; event.text_utf8 = text;
    ui_dispatch_enter(surface->host);
    surface->input(surface, &event, surface->input_user_data);
    ui_dispatch_leave(surface->host);
}

static int surface_dispatch_native_input(ui_surface_t *surface, HWND hwnd,
                                          UINT message, WPARAM w_param,
                                          LPARAM l_param)
{
    ui_win32_surface_t *platform;
    ui_input_event_t event;
    if (surface == NULL || surface->platform == NULL) return 0;
    platform = (ui_win32_surface_t *)surface->platform;
    if (message == WM_CAPTURECHANGED) {
        if ((HWND)l_param != hwnd) platform->captured_buttons = 0;
        return 0;
    }
    if (message == WM_KILLFOCUS || message == WM_CANCELMODE) {
        platform->pending_high_surrogate = 0;
        platform->captured_buttons = 0;
        if (GetCapture() == hwnd) ReleaseCapture();
        return 0;
    }
    if (surface->input == NULL || surface->host->dispatch_blocked) {
        platform->pending_high_surrogate = 0;
        return 0;
    }
    ZeroMemory(&event, sizeof(event));
    event.size = sizeof(event);
    event.modifiers = input_modifiers();
    switch (message) {
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN: case WM_LBUTTONUP:
    case WM_RBUTTONDOWN: case WM_RBUTTONUP:
    case WM_MBUTTONDOWN: case WM_MBUTTONUP: {
        event.x = input_pixels_to_logical(GET_X_LPARAM(l_param), surface->host->dpi);
        event.y = input_pixels_to_logical(GET_Y_LPARAM(l_param), surface->host->dpi);
        if (w_param & MK_CONTROL) event.modifiers |= UI_INPUT_MODIFIER_CONTROL;
        if (w_param & MK_SHIFT) event.modifiers |= UI_INPUT_MODIFIER_SHIFT;
        if (message == WM_MOUSEMOVE) event.kind = UI_INPUT_POINTER_MOVE;
        else {
            int down = message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN ||
                       message == WM_MBUTTONDOWN;
            uint32_t mask;
            event.kind = down ? UI_INPUT_POINTER_DOWN : UI_INPUT_POINTER_UP;
            event.pointer_button = (message == WM_LBUTTONDOWN || message == WM_LBUTTONUP) ? 1u :
                                   (message == WM_RBUTTONDOWN || message == WM_RBUTTONUP) ? 2u : 3u;
            mask = 1u << (event.pointer_button - 1u);
            if (down) {
                if (GetFocus() != hwnd) SetFocus(hwnd);
                platform->captured_buttons |= mask;
                if (GetCapture() != hwnd) SetCapture(hwnd);
            } else {
                platform->captured_buttons &= ~mask;
                if (!platform->captured_buttons && GetCapture() == hwnd) ReleaseCapture();
            }
        }
        break;
    }
    case WM_MOUSEWHEEL: {
        POINT point = {GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param)};
        if (!ScreenToClient(hwnd, &point)) return 0;
        event.kind = UI_INPUT_WHEEL;
        event.x = input_pixels_to_logical(point.x, surface->host->dpi);
        event.y = input_pixels_to_logical(point.y, surface->host->dpi);
        event.wheel_delta = GET_WHEEL_DELTA_WPARAM(w_param);
        if (GET_KEYSTATE_WPARAM(w_param) & MK_CONTROL) event.modifiers |= UI_INPUT_MODIFIER_CONTROL;
        if (GET_KEYSTATE_WPARAM(w_param) & MK_SHIFT) event.modifiers |= UI_INPUT_MODIFIER_SHIFT;
        break;
    }
    case WM_KEYDOWN: case WM_KEYUP:
    case WM_SYSKEYDOWN: case WM_SYSKEYUP:
        event.kind = (message == WM_KEYDOWN || message == WM_SYSKEYDOWN)
                         ? UI_INPUT_KEY_DOWN : UI_INPUT_KEY_UP;
        event.key_code = (uint32_t)w_param;
        if ((uintptr_t)l_param & ((uintptr_t)1u << 29)) event.modifiers |= UI_INPUT_MODIFIER_ALT;
        break;
    case WM_CHAR: case WM_SYSCHAR: {
        uint32_t value = (uint16_t)w_param;
        uint16_t previous = platform->pending_high_surrogate;
        if ((uintptr_t)l_param & ((uintptr_t)1u << 29)) event.modifiers |= UI_INPUT_MODIFIER_ALT;
        platform->pending_high_surrogate = 0;
        if (value >= 0xd800u && value <= 0xdbffu) {
            if (previous) surface_emit_codepoint(surface, 0xfffdu, event.modifiers);
            platform->pending_high_surrogate = (uint16_t)value;
            return 1;
        }
        if (value >= 0xdc00u && value <= 0xdfffu) {
            value = previous ? 0x10000u + (((uint32_t)previous - 0xd800u) << 10) + value - 0xdc00u
                             : 0xfffdu;
        } else if (previous) surface_emit_codepoint(surface, 0xfffdu, event.modifiers);
        surface_emit_codepoint(surface, value, event.modifiers);
        return 1;
    }
    default:
        return 0;
    }
    ui_dispatch_enter(surface->host);
    surface->input(surface, &event, surface->input_user_data);
    ui_dispatch_leave(surface->host);
    return 1;
}

static LRESULT CALLBACK surface_window_proc(HWND hwnd,
                                            UINT message,
                                            WPARAM w_param,
                                            LPARAM l_param)
{
    ui_surface_t *surface = (ui_surface_t *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    if (message == WM_NCCREATE) {
        const CREATESTRUCTW *create = (const CREATESTRUCTW *)l_param;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)create->lpCreateParams);
    }

    if (message == WM_ERASEBKGND) {
        return 1;
    }
    if (message == WM_PAINT) {
        PAINTSTRUCT paint;
        BeginPaint(hwnd, &paint);
        EndPaint(hwnd, &paint);
        if (surface != NULL && !surface->host->dispatch_blocked && surface->frame != NULL &&
            surface->pixel_rect.width > 0 && surface->pixel_rect.height > 0) {
            ui_dispatch_enter(surface->host);
            surface->frame(surface, surface->callback_user_data);
            ui_dispatch_leave(surface->host);
        }
        return 0;
    }

    if (surface_dispatch_native_input(surface, hwnd, message, w_param, l_param)) return 0;
    if (message == WM_NCDESTROY) SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);

    return DefWindowProcW(hwnd, message, w_param, l_param);
}

static int ensure_surface_class(void)
{
    WNDCLASSW window_class;

    if (surface_class_atom != 0) {
        return 1;
    }

    ZeroMemory(&window_class, sizeof(window_class));
    window_class.style = CS_OWNDC;
    window_class.lpfnWndProc = surface_window_proc;
    window_class.hInstance = GetModuleHandleW(NULL);
    window_class.hCursor = LoadCursorW(NULL, MAKEINTRESOURCEW(32512));
    window_class.lpszClassName = surface_class_name;
    surface_class_atom = RegisterClassW(&window_class);
    return surface_class_atom != 0 || GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

static int setup_legacy_opengl(ui_win32_surface_t *platform)
{
    PIXELFORMATDESCRIPTOR descriptor;
    int pixel_format;

    ZeroMemory(&descriptor, sizeof(descriptor));
    descriptor.nSize = sizeof(descriptor);
    descriptor.nVersion = 1;
    descriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    descriptor.iPixelType = PFD_TYPE_RGBA;
    descriptor.cColorBits = 32;
    descriptor.cDepthBits = 24;
    descriptor.cStencilBits = 8;

    pixel_format = ChoosePixelFormat(platform->dc, &descriptor);
    if (pixel_format == 0 || !SetPixelFormat(platform->dc, pixel_format, &descriptor)) {
        return 0;
    }

    platform->glrc = wglCreateContext(platform->dc);
    if (platform->glrc == NULL) {
        return 0;
    }

    platform->is_opengl = 1;
    platform->legacy_context = 1;
    return 1;
}

static PROC get_wgl_proc(const char *name)
{
    PROC proc = wglGetProcAddress(name);
    if (proc == NULL || proc == (PROC)(intptr_t)1 ||
        proc == (PROC)(intptr_t)2 || proc == (PROC)(intptr_t)3 ||
        proc == (PROC)(intptr_t)-1) {
        return NULL;
    }
    return proc;
}

static int has_extension(const char *extensions, const char *name)
{
    const char *match;
    size_t length = strlen(name);
    if (extensions == NULL) {
        return 0;
    }
    match = extensions;
    while ((match = strstr(match, name)) != NULL) {
        if ((match == extensions || match[-1] == ' ') &&
            (match[length] == '\0' || match[length] == ' ')) {
            return 1;
        }
        match += length;
    }
    return 0;
}

static void get_context_version(int *major, int *minor)
{
    const char *version = (const char *)glGetString(GL_VERSION);
    char *end;
    long parsed;

    *major = 0;
    *minor = 0;
    if (version == NULL) {
        return;
    }
    parsed = strtol(version, &end, 10);
    if (parsed > 0 && parsed <= INT_MAX && *end == '.') {
        *major = (int)parsed;
        parsed = strtol(end + 1, &end, 10);
        if (parsed >= 0 && parsed <= INT_MAX) {
            *minor = (int)parsed;
        }
    }
}

static ui_opengl_profile_t get_context_profile(int major, int minor)
{
    GLint mask = 0;
    if (major > 3 || (major == 3 && minor >= 2)) {
        glGetIntegerv(GL_CONTEXT_PROFILE_MASK_VALUE, &mask);
        if (mask & GL_CONTEXT_CORE_PROFILE_BIT_VALUE) {
            return UI_OPENGL_PROFILE_CORE;
        }
        if (mask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT_VALUE) {
            return UI_OPENGL_PROFILE_COMPATIBILITY;
        }
    }
    return UI_OPENGL_PROFILE_ANY;
}

static ui_status_t validate_opengl_config(const ui_opengl_config_t *config)
{
    if (config == NULL) {
        return UI_STATUS_OK;
    }
    if (config->size < sizeof(*config) || config->major_version < 0 ||
        config->minor_version < 0 || config->samples < 0 ||
        config->profile < UI_OPENGL_PROFILE_ANY ||
        config->profile > UI_OPENGL_PROFILE_COMPATIBILITY) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    if (config->legacy_context) {
        return (config->major_version == 0 && config->minor_version == 0 &&
                config->profile == UI_OPENGL_PROFILE_ANY &&
                config->samples == 0 && config->debug_context == 0)
                   ? UI_STATUS_OK : UI_STATUS_INVALID_ARGUMENT;
    }
    if (config->major_version == 0 ||
        (config->profile != UI_OPENGL_PROFILE_ANY &&
         (config->major_version < 3 ||
          (config->major_version == 3 && config->minor_version < 2)))) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    return UI_STATUS_OK;
}

static ui_status_t setup_modern_opengl(ui_win32_surface_t *platform,
                                       const ui_opengl_config_t *config)
{
    ui_win32_surface_t bootstrap;
    HDC previous_dc = wglGetCurrentDC();
    HGLRC previous_context = wglGetCurrentContext();
    choose_pixel_format_arb_fn choose_format;
    create_context_attribs_arb_fn create_context;
    get_pixel_format_attrib_arb_fn get_format;
    get_extensions_string_arb_fn get_extensions;
    int has_multisample;
    PIXELFORMATDESCRIPTOR descriptor;
    int pixel_attributes[23];
    int context_attributes[11];
    int pixel_index = 0;
    int context_index = 0;
    int pixel_format = 0;
    UINT format_count = 0;
    int actual_major;
    int actual_minor;
    GLint flags = 0;
    RECT target_rect;
    ui_status_t status = UI_STATUS_UNSUPPORTED;

    ZeroMemory(&bootstrap, sizeof(bootstrap));
    ZeroMemory(&target_rect, sizeof(target_rect));
    GetWindowRect(platform->hwnd, &target_rect);
    bootstrap.hwnd = CreateWindowExW(0, surface_class_name, L"", WS_POPUP,
                                      target_rect.left, target_rect.top, 1, 1, NULL, NULL,
                                      GetModuleHandleW(NULL), NULL);
    if (bootstrap.hwnd == NULL) {
        return UI_STATUS_PLATFORM_ERROR;
    }
    bootstrap.dc = GetDC(bootstrap.hwnd);
    if (bootstrap.dc == NULL || !setup_legacy_opengl(&bootstrap) ||
        !wglMakeCurrent(bootstrap.dc, bootstrap.glrc)) {
        status = UI_STATUS_PLATFORM_ERROR;
        goto cleanup;
    }
    choose_format = (choose_pixel_format_arb_fn)get_wgl_proc("wglChoosePixelFormatARB");
    create_context = (create_context_attribs_arb_fn)get_wgl_proc("wglCreateContextAttribsARB");
    get_format = (get_pixel_format_attrib_arb_fn)get_wgl_proc("wglGetPixelFormatAttribivARB");
    get_extensions = (get_extensions_string_arb_fn)get_wgl_proc("wglGetExtensionsStringARB");
    if (choose_format == NULL || create_context == NULL ||
        get_format == NULL || get_extensions == NULL) {
        goto cleanup;
    }
    has_multisample = has_extension(get_extensions(bootstrap.dc), "WGL_ARB_multisample");
    if (config->samples > 0 && !has_multisample) {
        goto cleanup;
    }

#define ADD_PIXEL_ATTRIBUTE(key, value) \
    do { pixel_attributes[pixel_index++] = (key); \
         pixel_attributes[pixel_index++] = (value); } while (0)
    ADD_PIXEL_ATTRIBUTE(WGL_DRAW_TO_WINDOW_ARB, TRUE);
    ADD_PIXEL_ATTRIBUTE(WGL_SUPPORT_OPENGL_ARB, TRUE);
    ADD_PIXEL_ATTRIBUTE(WGL_DOUBLE_BUFFER_ARB, TRUE);
    ADD_PIXEL_ATTRIBUTE(WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB);
    ADD_PIXEL_ATTRIBUTE(WGL_COLOR_BITS_ARB, 32);
    ADD_PIXEL_ATTRIBUTE(WGL_DEPTH_BITS_ARB, 24);
    ADD_PIXEL_ATTRIBUTE(WGL_STENCIL_BITS_ARB, 8);
    if (has_multisample) {
        ADD_PIXEL_ATTRIBUTE(WGL_SAMPLE_BUFFERS_ARB, config->samples > 0 ? 1 : 0);
        ADD_PIXEL_ATTRIBUTE(WGL_SAMPLES_ARB, config->samples);
    }
    pixel_attributes[pixel_index] = 0;
#undef ADD_PIXEL_ATTRIBUTE
    if (!choose_format(platform->dc, pixel_attributes, NULL, 1,
                       &pixel_format, &format_count) || format_count == 0) {
        goto cleanup;
    }
    if (has_multisample) {
        int attribute = WGL_SAMPLES_ARB;
        int samples = 0;
        if (!get_format(platform->dc, pixel_format, 0, 1, &attribute, &samples)) {
            goto cleanup;
        }
        platform->samples = samples;
    }
    if ((config->samples > 0 && platform->samples < config->samples) ||
        (config->samples == 0 && platform->samples != 0)) {
        goto cleanup;
    }
    ZeroMemory(&descriptor, sizeof(descriptor));
    if (!DescribePixelFormat(platform->dc, pixel_format,
                             sizeof(descriptor), &descriptor) ||
        !SetPixelFormat(platform->dc, pixel_format, &descriptor)) {
        status = UI_STATUS_PLATFORM_ERROR;
        goto cleanup;
    }
#define ADD_CONTEXT_ATTRIBUTE(key, value) \
    do { context_attributes[context_index++] = (key); \
         context_attributes[context_index++] = (value); } while (0)
    ADD_CONTEXT_ATTRIBUTE(WGL_CONTEXT_MAJOR_VERSION_ARB, config->major_version);
    ADD_CONTEXT_ATTRIBUTE(WGL_CONTEXT_MINOR_VERSION_ARB, config->minor_version);
    ADD_CONTEXT_ATTRIBUTE(WGL_CONTEXT_FLAGS_ARB,
                          config->debug_context ? WGL_CONTEXT_DEBUG_BIT_ARB : 0);
    if (config->profile != UI_OPENGL_PROFILE_ANY) {
        ADD_CONTEXT_ATTRIBUTE(WGL_CONTEXT_PROFILE_MASK_ARB,
            config->profile == UI_OPENGL_PROFILE_CORE
                ? WGL_CONTEXT_CORE_PROFILE_BIT_ARB
                : WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB);
    }
    context_attributes[context_index] = 0;
#undef ADD_CONTEXT_ATTRIBUTE
    platform->glrc = create_context(platform->dc, NULL, context_attributes);
    if (platform->glrc == NULL) {
        goto cleanup;
    }
    if (!wglMakeCurrent(platform->dc, platform->glrc)) {
        status = UI_STATUS_PLATFORM_ERROR;
        goto cleanup;
    }
    get_context_version(&actual_major, &actual_minor);
    if (actual_major < config->major_version ||
        (actual_major == config->major_version && actual_minor < config->minor_version) ||
        (config->profile != UI_OPENGL_PROFILE_ANY &&
         get_context_profile(actual_major, actual_minor) != config->profile)) {
        goto cleanup;
    }
    if (actual_major >= 3) {
        glGetIntegerv(GL_CONTEXT_FLAGS_VALUE, &flags);
    }
    if (config->debug_context && !(flags & GL_CONTEXT_DEBUG_BIT_VALUE)) {
        goto cleanup;
    }
    platform->is_opengl = 1;
    platform->legacy_context = 0;
    status = UI_STATUS_OK;

cleanup:
    if (!wglMakeCurrent(previous_dc, previous_context)) {
        wglMakeCurrent(NULL, NULL);
        status = UI_STATUS_PLATFORM_ERROR;
    }
    if (status != UI_STATUS_OK && platform->glrc != NULL) {
        wglDeleteContext(platform->glrc);
        platform->glrc = NULL;
    }
    if (bootstrap.glrc != NULL) {
        wglDeleteContext(bootstrap.glrc);
    }
    if (bootstrap.dc != NULL) {
        ReleaseDC(bootstrap.hwnd, bootstrap.dc);
    }
    DestroyWindow(bootstrap.hwnd);
    return status;
}

ui_status_t ui_platform_surface_create_configured(
    ui_surface_t *surface,
    const ui_opengl_config_t *config)
{
    ui_win32_surface_t *platform;
    HWND parent;
    ui_status_t status;

    status = validate_opengl_config(config);
    if (status != UI_STATUS_OK) {
        return status;
    }

    if (surface == NULL || surface->host == NULL ||
        !ensure_surface_class()) {
        return UI_STATUS_PLATFORM_ERROR;
    }

    parent = (HWND)surface->host->native_parent;
    if (parent == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }

    platform = (ui_win32_surface_t *)HeapAlloc(
        GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*platform));
    if (platform == NULL) {
        return UI_STATUS_OUT_OF_MEMORY;
    }

    platform->hwnd = CreateWindowExW(
        0,
        surface_class_name,
        L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        surface->pixel_rect.x,
        surface->pixel_rect.y,
        surface->pixel_rect.width,
        surface->pixel_rect.height,
        parent,
        NULL,
        GetModuleHandleW(NULL),
        surface);
    if (platform->hwnd == NULL) {
        HeapFree(GetProcessHeap(), 0, platform);
        return UI_STATUS_PLATFORM_ERROR;
    }

    platform->dc = GetDC(platform->hwnd);
    if (platform->dc == NULL) {
        DestroyWindow(platform->hwnd);
        HeapFree(GetProcessHeap(), 0, platform);
        return UI_STATUS_PLATFORM_ERROR;
    }
    status = UI_STATUS_OK;
    if (surface->kind == UI_SURFACE_OPENGL) {
        if (config == NULL || config->legacy_context) {
            status = setup_legacy_opengl(platform)
                       ? UI_STATUS_OK : UI_STATUS_PLATFORM_ERROR;
        } else {
            status = setup_modern_opengl(platform, config);
        }
    }
    if (status != UI_STATUS_OK) {
        ReleaseDC(platform->hwnd, platform->dc);
        DestroyWindow(platform->hwnd);
        HeapFree(GetProcessHeap(), 0, platform);
        return status;
    }

    surface->platform = platform;
    surface->native_handle = (void *)platform->hwnd;
    ShowWindow(platform->hwnd, surface->visible ? SW_SHOW : SW_HIDE);
    return UI_STATUS_OK;
}

ui_status_t ui_platform_surface_create(ui_surface_t *surface)
{
    return ui_platform_surface_create_configured(surface, NULL);
}

void ui_platform_surface_destroy(ui_surface_t *surface)
{
    ui_win32_surface_t *platform;

    if (surface == NULL || surface->platform == NULL) {
        return;
    }

    platform = (ui_win32_surface_t *)surface->platform;
    if (platform->glrc != NULL) {
        if (wglGetCurrentContext() == platform->glrc) {
            wglMakeCurrent(NULL, NULL);
        }
        wglDeleteContext(platform->glrc);
    }
    if (platform->dc != NULL && platform->hwnd != NULL) {
        ReleaseDC(platform->hwnd, platform->dc);
    }
    if (platform->hwnd != NULL) {
        DestroyWindow(platform->hwnd);
    }

    HeapFree(GetProcessHeap(), 0, platform);
    surface->platform = NULL;
    surface->native_handle = NULL;
}

ui_status_t ui_platform_surface_set_rect(ui_surface_t *surface,
                                         const ui_rect_t *rect)
{
    ui_win32_surface_t *platform;

    if (surface == NULL || rect == NULL || surface->platform == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }

    platform = (ui_win32_surface_t *)surface->platform;
    (void)rect;
    if (!MoveWindow(platform->hwnd,
                    surface->pixel_rect.x,
                    surface->pixel_rect.y,
                    surface->pixel_rect.width,
                    surface->pixel_rect.height,
                    TRUE)) {
        return UI_STATUS_PLATFORM_ERROR;
    }
    return UI_STATUS_OK;
}

ui_status_t ui_platform_surface_set_visible(ui_surface_t *surface,
                                            int visible)
{
    ui_win32_surface_t *platform;

    if (surface == NULL || surface->platform == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }

    platform = (ui_win32_surface_t *)surface->platform;
    ShowWindow(platform->hwnd, visible ? SW_SHOW : SW_HIDE);
    return UI_STATUS_OK;
}

ui_status_t ui_platform_surface_invalidate(ui_surface_t *surface)
{
    ui_win32_surface_t *platform;
    if (surface == NULL || surface->platform == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    platform = (ui_win32_surface_t *)surface->platform;
    return InvalidateRect(platform->hwnd, NULL, FALSE)
               ? UI_STATUS_OK : UI_STATUS_PLATFORM_ERROR;
}

ui_status_t ui_platform_surface_make_current(ui_surface_t *surface)
{
    ui_win32_surface_t *platform;

    if (surface == NULL || surface->platform == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }

    platform = (ui_win32_surface_t *)surface->platform;
    if (!platform->is_opengl || !wglMakeCurrent(platform->dc, platform->glrc)) {
        return UI_STATUS_UNSUPPORTED;
    }
    return UI_STATUS_OK;
}

ui_status_t ui_platform_surface_swap_buffers(ui_surface_t *surface)
{
    ui_win32_surface_t *platform;

    if (surface == NULL || surface->platform == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }

    platform = (ui_win32_surface_t *)surface->platform;
    if (!platform->is_opengl || !SwapBuffers(platform->dc)) {
        return UI_STATUS_UNSUPPORTED;
    }
    return UI_STATUS_OK;
}

static void copy_gl_string(char *destination, size_t capacity, GLenum name)
{
    const char *value = (const char *)glGetString(name);
    size_t length;

    if (value == NULL) {
        destination[0] = '\0';
        return;
    }
    length = strlen(value);
    if (length >= capacity) {
        length = capacity - 1;
    }
    memcpy(destination, value, length);
    destination[length] = '\0';
}

ui_status_t ui_opengl_surface_get_info(const ui_surface_t *surface,
                                      ui_opengl_info_t *info)
{
    ui_win32_surface_t *platform;
    HDC previous_dc;
    HGLRC previous_context;
    GLint flags = 0;

    if (surface == NULL || info == NULL || info->size < sizeof(*info)) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    if (surface->kind != UI_SURFACE_OPENGL || surface->platform == NULL) {
        return UI_STATUS_UNSUPPORTED;
    }
    platform = (ui_win32_surface_t *)surface->platform;
    previous_dc = wglGetCurrentDC();
    previous_context = wglGetCurrentContext();
    if (!wglMakeCurrent(platform->dc, platform->glrc)) {
        return UI_STATUS_PLATFORM_ERROR;
    }
    memset(info, 0, sizeof(*info));
    info->size = sizeof(*info);
    copy_gl_string(info->vendor, sizeof(info->vendor), GL_VENDOR);
    copy_gl_string(info->renderer, sizeof(info->renderer), GL_RENDERER);
    copy_gl_string(info->version, sizeof(info->version), GL_VERSION);
    get_context_version(&info->major_version, &info->minor_version);
    if (info->major_version >= 2) {
        copy_gl_string(info->shading_language_version,
                       sizeof(info->shading_language_version),
                       GL_SHADING_LANGUAGE_VERSION_VALUE);
    }
    info->profile = get_context_profile(info->major_version, info->minor_version);
    if (info->major_version >= 3) {
        glGetIntegerv(GL_CONTEXT_FLAGS_VALUE, &flags);
    }
    info->debug_context = (flags & GL_CONTEXT_DEBUG_BIT_VALUE) != 0;
    info->samples = platform->samples;
    info->legacy_context = platform->legacy_context;
    if (!wglMakeCurrent(previous_dc, previous_context)) {
        wglMakeCurrent(NULL, NULL);
        return UI_STATUS_PLATFORM_ERROR;
    }
    return info->version[0] != '\0' ? UI_STATUS_OK : UI_STATUS_PLATFORM_ERROR;
}

ui_opengl_proc_t ui_opengl_surface_get_proc_address(
    const ui_surface_t *surface,
    const char *name)
{
    ui_win32_surface_t *platform;
    HDC previous_dc;
    HGLRC previous_context;
    PROC proc;

    if (surface == NULL || surface->kind != UI_SURFACE_OPENGL ||
        surface->platform == NULL || name == NULL || name[0] == '\0') {
        return NULL;
    }
    platform = (ui_win32_surface_t *)surface->platform;
    previous_dc = wglGetCurrentDC();
    previous_context = wglGetCurrentContext();
    if (!wglMakeCurrent(platform->dc, platform->glrc)) {
        return NULL;
    }
    proc = get_wgl_proc(name);
    if (proc == NULL) {
        HMODULE opengl_module = GetModuleHandleA("opengl32.dll");
        if (opengl_module != NULL) {
            proc = GetProcAddress(opengl_module, name);
        }
    }
    if (!wglMakeCurrent(previous_dc, previous_context)) {
        wglMakeCurrent(NULL, NULL);
        return NULL;
    }
    return (ui_opengl_proc_t)proc;
}

static int pixels_to_logical(int pixels, uint32_t dpi)
{
    int64_t logical;

    if (dpi == 0u) {
        dpi = 96u;
    }
    logical = ((int64_t)pixels * 96 + (int64_t)dpi / 2) / (int64_t)dpi;
    if (logical > INT_MAX) {
        return INT_MAX;
    }
    if (logical < 0) {
        return 0;
    }
    return (int)logical;
}

static LONG logical_to_pixels(int logical, uint32_t dpi)
{
    int64_t pixels = ((int64_t)logical * dpi + 48) / 96;
    return pixels > LONG_MAX ? LONG_MAX : (LONG)pixels;
}

ui_status_t ui_platform_host_handle_message(ui_host_t *host,
                                            void *hwnd_value,
                                            uint32_t message,
                                            uintptr_t w_param,
                                            intptr_t l_param,
                                            intptr_t *result)
{
    HWND hwnd = (HWND)hwnd_value;
    UINT dpi;

    if (host == NULL || hwnd == NULL ||
        (host->native_parent != NULL &&
         hwnd != (HWND)host->native_parent)) {
        return UI_STATUS_INVALID_ARGUMENT;
    }

    if (message == WM_SIZE) {
        int width = (int)(unsigned short)LOWORD((LPARAM)l_param);
        int height = (int)(unsigned short)HIWORD((LPARAM)l_param);
        ui_status_t status = ui_host_resize(host,
                                             pixels_to_logical(width,
                                                               host->dpi),
                                             pixels_to_logical(height,
                                                               host->dpi));
        if (result != NULL) {
            *result = 0;
        }
        return status;
    }

    if (message == WM_DPICHANGED) {
        dpi = HIWORD((WPARAM)w_param);
        if (dpi == 0u) {
            dpi = LOWORD((WPARAM)w_param);
        }
        if (dpi == 0u) {
            return UI_STATUS_INVALID_ARGUMENT;
        }
        if (result != NULL) {
            *result = 0;
        }
        return ui_host_set_dpi(host, (uint32_t)dpi);
    }

    if (message == WM_GETMINMAXINFO) {
        typedef BOOL (WINAPI *adjust_window_rect_for_dpi_fn)(
            LPRECT, DWORD, BOOL, DWORD, UINT);
        adjust_window_rect_for_dpi_fn adjust_for_dpi;
        ui_narrow_window_policy_t policy = host->layout.narrow_policy;
        MINMAXINFO *limits = (MINMAXINFO *)l_param;
        RECT minimum;
        int logical_width;
        int logical_height;
        DWORD style = (DWORD)GetWindowLongPtrA(hwnd, GWL_STYLE);
        DWORD extended_style = (DWORD)GetWindowLongPtrA(hwnd, GWL_EXSTYLE);
        BOOL has_menu = GetMenu(hwnd) != NULL;
        if (host->narrow_callback != NULL) {
            ui_dispatch_enter(host);
            policy = host->narrow_callback(host, host->host_rect.width,
                                           host->host_rect.height, host->dpi,
                                           policy, host->narrow_user_data);
            ui_dispatch_leave(host);
        }
        if (policy != UI_NARROW_DISALLOW_SHRINK) {
            return UI_STATUS_NOT_FOUND;
        }
        if (limits == NULL || ui_host_get_min_size(host, &logical_width,
                                                  &logical_height) != UI_STATUS_OK) {
            return UI_STATUS_INVALID_ARGUMENT;
        }
        minimum.left = 0;
        minimum.top = 0;
        minimum.right = logical_to_pixels(logical_width, host->dpi);
        minimum.bottom = logical_to_pixels(logical_height, host->dpi);
        adjust_for_dpi = (adjust_window_rect_for_dpi_fn)GetProcAddress(
            GetModuleHandleA("user32.dll"), "AdjustWindowRectExForDpi");
        if (adjust_for_dpi != NULL) {
            if (!adjust_for_dpi(&minimum, style, has_menu, extended_style, host->dpi)) {
                return UI_STATUS_PLATFORM_ERROR;
            }
        } else if (!AdjustWindowRectEx(&minimum, style, has_menu, extended_style)) {
            return UI_STATUS_PLATFORM_ERROR;
        }
        if (limits->ptMinTrackSize.x < minimum.right - minimum.left) {
            limits->ptMinTrackSize.x = minimum.right - minimum.left;
        }
        if (limits->ptMinTrackSize.y < minimum.bottom - minimum.top) {
            limits->ptMinTrackSize.y = minimum.bottom - minimum.top;
        }
        if (result != NULL) {
            *result = 0;
        }
        return UI_STATUS_OK;
    }

    return UI_STATUS_NOT_FOUND;
}
