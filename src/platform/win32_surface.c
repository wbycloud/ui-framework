#include "../ui_internal.h"
#include "ui_framework/opengl.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <GL/gl.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

typedef void (APIENTRY *fbo_gen_fn)(GLsizei,GLuint *);
typedef void (APIENTRY *fbo_delete_fn)(GLsizei,const GLuint *);
typedef void (APIENTRY *fbo_bind_fn)(GLenum,GLuint);
typedef void (APIENTRY *fbo_texture_fn)(GLenum,GLenum,GLenum,GLuint,GLint);
typedef GLenum (APIENTRY *fbo_status_fn)(GLenum);
typedef void (APIENTRY *rb_storage_fn)(GLenum,GLenum,GLsizei,GLsizei);
typedef void (APIENTRY *fbo_rb_fn)(GLenum,GLenum,GLenum,GLuint);
typedef void (APIENTRY *rb_msaa_fn)(GLenum,GLsizei,GLenum,GLsizei,GLsizei);
typedef void (APIENTRY *blit_fn)(GLint,GLint,GLint,GLint,GLint,GLint,GLint,GLint,GLbitfield,GLenum);
typedef struct ui_gl_api {
    void (APIENTRY *GetIntegerv)(GLenum,GLint *);
    void (APIENTRY *GenTextures)(GLsizei,GLuint *);
    void (APIENTRY *DeleteTextures)(GLsizei,const GLuint *);
    void (APIENTRY *BindTexture)(GLenum,GLuint);
    void (APIENTRY *TexParameteri)(GLenum,GLenum,GLint);
    void (APIENTRY *TexImage2D)(GLenum,GLint,GLint,GLsizei,GLsizei,GLint,GLenum,GLenum,const void *);
    void (APIENTRY *Viewport)(GLint,GLint,GLsizei,GLsizei);
    void (APIENTRY *ReadBuffer)(GLenum);
    void (APIENTRY *PixelStorei)(GLenum,GLint);
    void (APIENTRY *ReadPixels)(GLint,GLint,GLsizei,GLsizei,GLenum,GLenum,void *);
    GLboolean (APIENTRY *IsEnabled)(GLenum);
    void (APIENTRY *Enable)(GLenum);
    void (APIENTRY *Disable)(GLenum);
    void (APIENTRY *Flush)(void);
} ui_gl_api_t;
typedef struct ui_win32_surface {
    HWND hwnd;
    DWORD owner_thread;
    GLuint fbo, color, depth, msaa_fbo, msaa_color;
    int fbo_width,fbo_height;
    fbo_gen_fn gen_fbo,gen_rb;
    fbo_delete_fn delete_fbo,delete_rb;
    fbo_bind_fn bind_fbo,bind_rb,bind_buffer;
    fbo_texture_fn attach_texture;
    fbo_status_fn check_fbo;
    rb_storage_fn storage_rb;
    fbo_rb_fn attach_rb;
    rb_msaa_fn storage_msaa;
    blit_fn blit;
    void (APIENTRY *get_rb_parameter)(GLenum,GLenum,GLint *);
    ui_gl_api_t gl;
    HMODULE os_library;
    void *os_context;
    unsigned char os_buffer[4];
    void *(APIENTRY *os_create)(const int *,void *);
    void (APIENTRY *os_destroy)(void *);
    GLboolean (APIENTRY *os_current)(void *,void *,GLenum,GLsizei,GLsizei);
    void *(APIENTRY *os_get_current)(void);
    GLboolean (APIENTRY *os_get_buffer)(void *,GLint *,GLint *,GLint *,void **);
    ui_opengl_proc_t (APIENTRY *os_get_proc)(const char *);
    HDC dc;
    HGLRC glrc;
    int is_opengl;
    int legacy_context;
    int samples;
    uint32_t captured_buttons;
    uint16_t pending_high_surrogate;
} ui_win32_surface_t;
typedef struct ui_gl_binding {HDC dc;HGLRC context;void *os,*buffer;int width,height,format;} ui_gl_binding_t;
static void save_binding(ui_win32_surface_t *p,ui_gl_binding_t *old)
{memset(old,0,sizeof(*old));if(p->os_library){old->os=p->os_get_current();if(old->os)p->os_get_buffer(old->os,&old->width,&old->height,&old->format,&old->buffer);}else{old->dc=wglGetCurrentDC();old->context=wglGetCurrentContext();}}
static int provider_current(ui_win32_surface_t *p)
{return p->os_library?p->os_current(p->os_context,p->os_buffer,GL_UNSIGNED_BYTE,1,1):wglMakeCurrent(p->dc,p->glrc);}
static int restore_binding(ui_win32_surface_t *p,const ui_gl_binding_t *old)
{return p->os_library?p->os_current(old->os,old->buffer,GL_UNSIGNED_BYTE,old->width,old->height):wglMakeCurrent(old->dc,old->context);}
static PROC provider_proc(ui_win32_surface_t *,const char *);
static void release_fbo(ui_win32_surface_t *p)
{if(p->fbo){p->delete_fbo(1,&p->fbo);p->delete_rb(1,&p->depth);p->gl.DeleteTextures(1,&p->color);p->fbo=p->depth=p->color=0;}
 if(p->msaa_fbo){p->delete_fbo(1,&p->msaa_fbo);p->delete_rb(1,&p->msaa_color);p->msaa_fbo=p->msaa_color=0;}p->fbo_width=p->fbo_height=0;}

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
    if(message==WM_KEYDOWN||message==WM_SYSKEYDOWN||message==WM_KEYUP||message==WM_SYSKEYUP){ui_input_event_t key={0};key.size=sizeof(key);key.key_code=(uint32_t)w_param;key.modifiers=input_modifiers();
        key.kind=(message==WM_KEYUP||message==WM_SYSKEYUP)?UI_INPUT_KEY_UP:UI_INPUT_KEY_DOWN;
        if(ui_menus_route_input(surface->host,NULL,&key,0)==UI_STATUS_OK)return 1;}
    if (!surface->host->dispatch_blocked && (message == WM_KEYDOWN || message == WM_SYSKEYDOWN)) {
        uint32_t modifiers = input_modifiers();
        if ((uintptr_t)l_param & ((uintptr_t)1u << 29)) modifiers |= UI_INPUT_MODIFIER_ALT;
        if (ui_host_dispatch_shortcut(surface->host, (uint32_t)w_param, modifiers, 0) != 0) return 1;
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
        event.x = input_pixels_to_logical(GET_X_LPARAM(l_param), ui_shell_surface_dpi(surface->host, surface));
        event.y = input_pixels_to_logical(GET_Y_LPARAM(l_param), ui_shell_surface_dpi(surface->host, surface));
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
        event.x = input_pixels_to_logical(point.x, ui_shell_surface_dpi(surface->host, surface));
        event.y = input_pixels_to_logical(point.y, ui_shell_surface_dpi(surface->host, surface));
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
        if (surface != NULL && !surface->offscreen && !surface->host->dispatch_blocked && surface->frame != NULL &&
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

static PROC provider_proc(ui_win32_surface_t *p,const char *name)
{PROC proc;if(p->os_library)return (PROC)p->os_get_proc(name);proc=get_wgl_proc(name);if(!proc)proc=GetProcAddress(GetModuleHandleW(L"opengl32.dll"),name);return proc;}
static ui_status_t setup_osmesa(ui_win32_surface_t *p,const ui_surface_t *surface,const ui_opengl_config_t *config)
{
    wchar_t *path;int length;ui_gl_binding_t old;ui_status_t status=UI_STATUS_UNSUPPORTED;
    int attributes[]={0x22,GL_RGBA,0x30,24,0x31,8,0x33,0x35,0x36,config->major_version,0x37,config->minor_version,0};
    const char *name=surface->gl_library_path;
    if(!name||!((strlen(name)>2&&name[1]==':'&&(name[2]=='\\'||name[2]=='/'))||(name[0]=='\\'&&name[1]=='\\')))return UI_STATUS_INVALID_ARGUMENT;
    length=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,name,-1,NULL,0);if(!length)return UI_STATUS_INVALID_ARGUMENT;
    path=(wchar_t *)malloc((size_t)length*sizeof(*path));if(!path)return UI_STATUS_OUT_OF_MEMORY;
    MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,name,-1,path,length);
    {DWORD count=GetFullPathNameW(path,0,NULL,NULL);wchar_t *absolute=count?(wchar_t *)malloc((size_t)count*sizeof(*absolute)):NULL;
     if(!absolute){free(path);return count?UI_STATUS_OUT_OF_MEMORY:UI_STATUS_INVALID_ARGUMENT;}
     if(GetFullPathNameW(path,count,absolute,NULL))p->os_library=LoadLibraryExW(absolute,NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
     free(absolute);free(path);if(!p->os_library)return status;}
#define OS_LOAD(member,name) {PROC proc=GetProcAddress(p->os_library,name);memcpy(&p->member,&proc,sizeof(proc));if(!proc)goto fail;}
    OS_LOAD(os_create,"OSMesaCreateContextAttribs");OS_LOAD(os_destroy,"OSMesaDestroyContext");OS_LOAD(os_current,"OSMesaMakeCurrent");
    OS_LOAD(os_get_current,"OSMesaGetCurrentContext");OS_LOAD(os_get_buffer,"OSMesaGetColorBuffer");OS_LOAD(os_get_proc,"OSMesaGetProcAddress");
#undef OS_LOAD
    p->os_context=p->os_create(attributes,NULL);if(!p->os_context)goto fail;
    save_binding(p,&old);if(!provider_current(p))goto fail;
    {typedef const GLubyte *(APIENTRY *string_fn)(GLenum);string_fn get=(string_fn)provider_proc(p,"glGetString");const char *version=get?(const char *)get(GL_VERSION):NULL;int major=0,minor=0;
     if(version)sscanf(version,"%d.%d",&major,&minor);if(major>config->major_version||(major==config->major_version&&minor>=config->minor_version))status=UI_STATUS_OK;}
    if(!restore_binding(p,&old))status=UI_STATUS_PLATFORM_ERROR;
    if(status==UI_STATUS_OK){p->is_opengl=1;p->samples=config->samples;return status;}
fail:
    if(p->os_context&&p->os_destroy)p->os_destroy(p->os_context);p->os_context=NULL;FreeLibrary(p->os_library);p->os_library=NULL;return status;
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
    if(surface&&surface->offscreen==2){
        platform=(ui_win32_surface_t *)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*platform));if(!platform)return UI_STATUS_OUT_OF_MEMORY;
        platform->owner_thread=GetCurrentThreadId();status=setup_osmesa(platform,surface,config);
        if(status!=UI_STATUS_OK){HeapFree(GetProcessHeap(),0,platform);return status;}surface->platform=platform;return UI_STATUS_OK;
    }

    if (surface == NULL || surface->host == NULL ||
        !ensure_surface_class()) {
        return UI_STATUS_PLATFORM_ERROR;
    }

    parent = (HWND)surface->host->native_parent;
    if (parent == NULL&&!surface->offscreen) {
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
        surface->offscreen?WS_POPUP:WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        surface->pixel_rect.x,
        surface->pixel_rect.y,
        surface->pixel_rect.width,
        surface->pixel_rect.height,
        surface->offscreen?NULL:parent,
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
            ui_opengl_config_t actual=*config;if(surface->offscreen)actual.samples=0;
            status = setup_modern_opengl(platform, &actual);if(status==UI_STATUS_OK&&surface->offscreen)platform->samples=config->samples;
        }
    }
    if (status != UI_STATUS_OK) {
        ReleaseDC(platform->hwnd, platform->dc);
        DestroyWindow(platform->hwnd);
        HeapFree(GetProcessHeap(), 0, platform);
        return status;
    }

    platform->owner_thread=GetCurrentThreadId();
    surface->platform = platform;
    surface->native_handle = (void *)platform->hwnd;
    if(!surface->offscreen)ShowWindow(platform->hwnd, surface->visible ? SW_SHOW : SW_HIDE);
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
    if(platform->os_library){ui_gl_binding_t old;save_binding(platform,&old);if(provider_current(platform))release_fbo(platform);
        if(old.os!=platform->os_context)(void)restore_binding(platform,&old);else (void)platform->os_current(NULL,NULL,GL_UNSIGNED_BYTE,0,0);
        platform->os_destroy(platform->os_context);FreeLibrary(platform->os_library);HeapFree(GetProcessHeap(),0,platform);surface->platform=NULL;return;}
    if (platform->glrc != NULL) {
        if(platform->fbo){HDC old_dc=wglGetCurrentDC();HGLRC old_ctx=wglGetCurrentContext();
            if(wglMakeCurrent(platform->dc,platform->glrc))release_fbo(platform);
            if(old_ctx!=platform->glrc)(void)wglMakeCurrent(old_dc,old_ctx);}
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
    if(surface->offscreen){uint64_t bytes=(uint64_t)surface->pixel_rect.width*(uint64_t)surface->pixel_rect.height*(platform->samples?8u+8u*(uint64_t)platform->samples:platform->os_library?12u:8u)+(platform->os_library?4u:0u);
        if(platform->owner_thread!=GetCurrentThreadId())return UI_STATUS_INVALID_ARGUMENT;
        if(surface->pixel_rect.width>16384||surface->pixel_rect.height>16384||bytes>32u*1024u*1024u)return UI_STATUS_LIMIT_EXCEEDED;
        if(platform->os_library)return UI_STATUS_OK;}
    {
        RECT target = { surface->pixel_rect.x, surface->pixel_rect.y,
                        surface->pixel_rect.x + surface->pixel_rect.width,
                        surface->pixel_rect.y + surface->pixel_rect.height };
        HWND parent = GetParent(platform->hwnd);
        if (parent != (HWND)surface->host->native_parent)
            MapWindowPoints((HWND)surface->host->native_parent, parent,
                            (POINT *)&target, 2);
        if (!MoveWindow(platform->hwnd, target.left, target.top,
                        target.right - target.left, target.bottom - target.top,
                        TRUE)) return UI_STATUS_PLATFORM_ERROR;
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
    if(!surface->offscreen)ShowWindow(platform->hwnd, visible ? SW_SHOW : SW_HIDE);
    return UI_STATUS_OK;
}

ui_status_t ui_platform_surface_invalidate(ui_surface_t *surface)
{
    ui_win32_surface_t *platform;
    if (surface == NULL || surface->platform == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    platform = (ui_win32_surface_t *)surface->platform;
    if(surface->offscreen)return UI_STATUS_OK;
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
    if(platform->owner_thread!=GetCurrentThreadId())return UI_STATUS_INVALID_ARGUMENT;
    if (!platform->is_opengl || !provider_current(platform)) {
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
    if(surface->offscreen){if(platform->owner_thread!=GetCurrentThreadId())return UI_STATUS_INVALID_ARGUMENT;
        if(platform->os_library){if(platform->os_get_current()!=platform->os_context)return UI_STATUS_INVALID_ARGUMENT;platform->gl.Flush();}
        else {if(wglGetCurrentContext()!=platform->glrc)return UI_STATUS_INVALID_ARGUMENT;glFlush();}return UI_STATUS_OK;}
    if (!platform->is_opengl || !SwapBuffers(platform->dc)) {
        return UI_STATUS_UNSUPPORTED;
    }
    return UI_STATUS_OK;
}

ui_status_t ui_opengl_surface_get_info(const ui_surface_t *surface,ui_opengl_info_t *info)
{
    ui_win32_surface_t *p;ui_gl_binding_t old;GLint mask=0,flags=0;
    typedef const GLubyte *(APIENTRY *string_fn)(GLenum);string_fn get;
    typedef void (APIENTRY *integer_fn)(GLenum,GLint *);integer_fn integer;
    if(!surface||!info||info->size<sizeof(*info))return UI_STATUS_INVALID_ARGUMENT;
    p=(ui_win32_surface_t *)surface->platform;if(!p||surface->kind!=UI_SURFACE_OPENGL)return UI_STATUS_UNSUPPORTED;
    if(p->owner_thread!=GetCurrentThreadId())return UI_STATUS_INVALID_ARGUMENT;save_binding(p,&old);if(!provider_current(p))return UI_STATUS_PLATFORM_ERROR;
    get=(string_fn)provider_proc(p,"glGetString");integer=(integer_fn)provider_proc(p,"glGetIntegerv");
    memset(info,0,sizeof(*info));info->size=sizeof(*info);
#define COPY_GL(field,name) {const char *value=(const char *)get(name);if(value)snprintf(info->field,sizeof(info->field),"%s",value);}
    COPY_GL(vendor,GL_VENDOR);COPY_GL(renderer,GL_RENDERER);COPY_GL(version,GL_VERSION);
    if(info->version[0])sscanf(info->version,"%d.%d",&info->major_version,&info->minor_version);
    if(info->major_version>=2){COPY_GL(shading_language_version,GL_SHADING_LANGUAGE_VERSION_VALUE);}
#undef COPY_GL
    if(info->major_version>=3)integer(GL_CONTEXT_FLAGS_VALUE,&flags);
    if(info->major_version>3||(info->major_version==3&&info->minor_version>=2))integer(GL_CONTEXT_PROFILE_MASK_VALUE,&mask);
    info->profile=(mask&GL_CONTEXT_CORE_PROFILE_BIT_VALUE)?UI_OPENGL_PROFILE_CORE:(mask&GL_CONTEXT_COMPATIBILITY_PROFILE_BIT_VALUE)?UI_OPENGL_PROFILE_COMPATIBILITY:UI_OPENGL_PROFILE_ANY;
    info->debug_context=(flags&GL_CONTEXT_DEBUG_BIT_VALUE)!=0;info->samples=p->samples;info->legacy_context=p->legacy_context;
    if(!restore_binding(p,&old))return UI_STATUS_PLATFORM_ERROR;return info->version[0]?UI_STATUS_OK:UI_STATUS_PLATFORM_ERROR;
}
ui_opengl_proc_t ui_opengl_surface_get_proc_address(const ui_surface_t *surface,const char *name)
{
    ui_win32_surface_t *p;ui_gl_binding_t old;PROC proc;
    if(!surface||surface->kind!=UI_SURFACE_OPENGL||!surface->platform||!name||!*name)return NULL;
    p=(ui_win32_surface_t *)surface->platform;if(p->owner_thread!=GetCurrentThreadId())return NULL;
    save_binding(p,&old);if(!provider_current(p))return NULL;proc=provider_proc(p,name);if(!restore_binding(p,&old))return NULL;return (ui_opengl_proc_t)proc;
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

ui_status_t ui_platform_offscreen_render(ui_surface_t *surface,ui_pixel_buffer_t *out)
{
    ui_win32_surface_t *p=(ui_win32_surface_t *)surface->platform;ui_gl_binding_t old;
    int width=surface->pixel_rect.width,height=surface->pixel_rect.height;size_t stride,bytes;uint8_t *pixels=NULL;
    GLint viewport[4],draw,read,texture,rb,pack,row_length,skip_rows,skip_pixels,pack_buffer,unpack_buffer,max_texture,max_samples=0,actual=0;
    GLboolean multisample,scissor;ui_status_t status=UI_STATUS_OK;
    if(!p||p->owner_thread!=GetCurrentThreadId())return UI_STATUS_INVALID_ARGUMENT;
    if(width<=0||height<=0||width>16384||height>16384)return UI_STATUS_LIMIT_EXCEEDED;
    stride=out->stride?out->stride:(size_t)width*4;if(stride<(size_t)width*4||stride>SIZE_MAX/(size_t)height)return UI_STATUS_INVALID_ARGUMENT;
    bytes=stride*(height-1)+(size_t)width*4;out->width=(uint32_t)width;out->height=(uint32_t)height;out->stride=stride;
    if(out->pixels&&bytes>out->capacity)return UI_STATUS_LIMIT_EXCEEDED;
    save_binding(p,&old);if(!provider_current(p))return UI_STATUS_PLATFORM_ERROR;
#define LOAD_MEMBER(member,type,name) p->member=(type)provider_proc(p,name)
    if(!p->bind_fbo){LOAD_MEMBER(gen_fbo,fbo_gen_fn,"glGenFramebuffers");LOAD_MEMBER(delete_fbo,fbo_delete_fn,"glDeleteFramebuffers");
        LOAD_MEMBER(bind_fbo,fbo_bind_fn,"glBindFramebuffer");LOAD_MEMBER(attach_texture,fbo_texture_fn,"glFramebufferTexture2D");
        LOAD_MEMBER(check_fbo,fbo_status_fn,"glCheckFramebufferStatus");LOAD_MEMBER(gen_rb,fbo_gen_fn,"glGenRenderbuffers");
        LOAD_MEMBER(delete_rb,fbo_delete_fn,"glDeleteRenderbuffers");LOAD_MEMBER(bind_rb,fbo_bind_fn,"glBindRenderbuffer");
        LOAD_MEMBER(storage_rb,rb_storage_fn,"glRenderbufferStorage");LOAD_MEMBER(attach_rb,fbo_rb_fn,"glFramebufferRenderbuffer");LOAD_MEMBER(bind_buffer,fbo_bind_fn,"glBindBuffer");
        LOAD_MEMBER(storage_msaa,rb_msaa_fn,"glRenderbufferStorageMultisample");LOAD_MEMBER(blit,blit_fn,"glBlitFramebuffer");
        {PROC proc=provider_proc(p,"glGetRenderbufferParameteriv");memcpy(&p->get_rb_parameter,&proc,sizeof(proc));}
#define UI_GL_LOAD(member) {PROC proc=provider_proc(p,"gl" #member);memcpy(&p->gl.member,&proc,sizeof(proc));if(!proc){status=UI_STATUS_UNSUPPORTED;goto restore_context;}}
        UI_GL_LOAD(GetIntegerv);UI_GL_LOAD(GenTextures);UI_GL_LOAD(DeleteTextures);UI_GL_LOAD(BindTexture);UI_GL_LOAD(TexParameteri);UI_GL_LOAD(TexImage2D);
        UI_GL_LOAD(Viewport);UI_GL_LOAD(ReadBuffer);UI_GL_LOAD(PixelStorei);UI_GL_LOAD(ReadPixels);UI_GL_LOAD(IsEnabled);UI_GL_LOAD(Enable);UI_GL_LOAD(Disable);UI_GL_LOAD(Flush);
#undef UI_GL_LOAD
    }
#undef LOAD_MEMBER
    if(!p->gen_fbo||!p->delete_fbo||!p->bind_fbo||!p->attach_texture||!p->check_fbo||!p->gen_rb||!p->delete_rb||!p->bind_rb||!p->storage_rb||!p->attach_rb||!p->bind_buffer||!p->storage_msaa||!p->blit||!p->get_rb_parameter){status=UI_STATUS_UNSUPPORTED;goto restore_context;}
#define glGetIntegerv p->gl.GetIntegerv
#define glGenTextures p->gl.GenTextures
#define glDeleteTextures p->gl.DeleteTextures
#define glBindTexture p->gl.BindTexture
#define glTexParameteri p->gl.TexParameteri
#define glTexImage2D p->gl.TexImage2D
#define glViewport p->gl.Viewport
#define glReadBuffer p->gl.ReadBuffer
#define glPixelStorei p->gl.PixelStorei
#define glReadPixels p->gl.ReadPixels
    glGetIntegerv(GL_VIEWPORT,viewport);glGetIntegerv(0x8ca6,&draw);glGetIntegerv(0x8caa,&read);
    glGetIntegerv(GL_TEXTURE_BINDING_2D,&texture);glGetIntegerv(0x8ca7,&rb);glGetIntegerv(GL_PACK_ALIGNMENT,&pack);glGetIntegerv(GL_PACK_ROW_LENGTH,&row_length);
    glGetIntegerv(GL_PACK_SKIP_ROWS,&skip_rows);glGetIntegerv(GL_PACK_SKIP_PIXELS,&skip_pixels);glGetIntegerv(0x88ed,&pack_buffer);glGetIntegerv(0x88ef,&unpack_buffer);
    multisample=p->gl.IsEnabled(0x809d);scissor=p->gl.IsEnabled(GL_SCISSOR_TEST);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE,&max_texture);glGetIntegerv(0x8d57,&max_samples);
    if(width>max_texture||height>max_texture||p->samples>max_samples){status=UI_STATUS_UNSUPPORTED;goto restore_state;}
    if((uint64_t)width*(uint64_t)height*(p->samples?8u+8u*(uint64_t)p->samples:p->os_library?12u:8u)+(p->os_library?4u:0u)>32u*1024u*1024u){status=UI_STATUS_LIMIT_EXCEEDED;goto restore_state;}
    if(p->fbo_width!=width||p->fbo_height!=height){
        release_fbo(p);p->gen_fbo(1,&p->fbo);p->bind_fbo(0x8d40,p->fbo);glGenTextures(1,&p->color);glBindTexture(GL_TEXTURE_2D,p->color);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        p->bind_buffer(0x88ec,0);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);p->bind_buffer(0x88ec,(GLuint)unpack_buffer);p->attach_texture(0x8d40,0x8ce0,GL_TEXTURE_2D,p->color,0);
        if(p->check_fbo(0x8d40)!=0x8cd5){status=UI_STATUS_PLATFORM_ERROR;goto restore_state;}
        if(p->samples){
            p->gen_fbo(1,&p->msaa_fbo);p->bind_fbo(0x8d40,p->msaa_fbo);p->gen_rb(1,&p->msaa_color);p->bind_rb(0x8d41,p->msaa_color);
            p->storage_msaa(0x8d41,p->samples,GL_RGBA8,width,height);p->get_rb_parameter(0x8d41,0x8cab,&actual);
            if(actual!=p->samples){status=UI_STATUS_UNSUPPORTED;goto restore_state;}p->attach_rb(0x8d40,0x8ce0,0x8d41,p->msaa_color);
        }
        p->gen_rb(1,&p->depth);p->bind_rb(0x8d41,p->depth);
        if(p->samples){p->storage_msaa(0x8d41,p->samples,0x88f0,width,height);p->get_rb_parameter(0x8d41,0x8cab,&actual);if(actual!=p->samples){status=UI_STATUS_UNSUPPORTED;goto restore_state;}}
        else p->storage_rb(0x8d41,0x88f0,width,height);
        p->attach_rb(0x8d40,0x821a,0x8d41,p->depth);if(p->check_fbo(0x8d40)!=0x8cd5){status=UI_STATUS_PLATFORM_ERROR;goto restore_state;}p->fbo_width=width;p->fbo_height=height;
    }
    if(!out->pixels)goto restore_state;
    pixels=(uint8_t *)malloc((size_t)width*height*4);if(!pixels){status=UI_STATUS_OUT_OF_MEMORY;goto restore_state;}
    glBindTexture(GL_TEXTURE_2D,(GLuint)texture);p->bind_rb(0x8d41,(GLuint)rb);p->bind_fbo(0x8d40,p->samples?p->msaa_fbo:p->fbo);glViewport(0,0,width,height);
    if(p->samples)p->gl.Enable(0x809d);else p->gl.Disable(0x809d);
    if(surface->frame)surface->frame(surface,surface->callback_user_data);
    if(!provider_current(p)){status=UI_STATUS_PLATFORM_ERROR;goto restore_context;}
    if(p->samples){p->gl.Disable(GL_SCISSOR_TEST);p->bind_fbo(0x8ca8,p->msaa_fbo);p->bind_fbo(0x8ca9,p->fbo);p->blit(0,0,width,height,0,0,width,height,GL_COLOR_BUFFER_BIT,GL_NEAREST);}
    p->bind_fbo(0x8ca8,p->fbo);glReadBuffer(0x8ce0);glPixelStorei(GL_PACK_ALIGNMENT,4);glPixelStorei(GL_PACK_ROW_LENGTH,0);
    glPixelStorei(GL_PACK_SKIP_ROWS,0);glPixelStorei(GL_PACK_SKIP_PIXELS,0);p->bind_buffer(0x88eb,0);glReadPixels(0,0,width,height,GL_RGBA,GL_UNSIGNED_BYTE,pixels);
    for(int y=0;y<height;++y)memcpy(out->pixels+(size_t)y*stride,pixels+(size_t)(height-1-y)*width*4,(size_t)width*4);
restore_state:
    p->bind_fbo(0x8ca9,(GLuint)draw);p->bind_fbo(0x8ca8,(GLuint)read);glViewport(viewport[0],viewport[1],viewport[2],viewport[3]);
    glBindTexture(GL_TEXTURE_2D,(GLuint)texture);p->bind_rb(0x8d41,(GLuint)rb);glPixelStorei(GL_PACK_ALIGNMENT,pack);glPixelStorei(GL_PACK_ROW_LENGTH,row_length);
    glPixelStorei(GL_PACK_SKIP_ROWS,skip_rows);glPixelStorei(GL_PACK_SKIP_PIXELS,skip_pixels);p->bind_buffer(0x88eb,(GLuint)pack_buffer);p->bind_buffer(0x88ec,(GLuint)unpack_buffer);
    if(multisample)p->gl.Enable(0x809d);else p->gl.Disable(0x809d);if(scissor)p->gl.Enable(GL_SCISSOR_TEST);else p->gl.Disable(GL_SCISSOR_TEST);
restore_context:
    if(!restore_binding(p,&old))status=UI_STATUS_PLATFORM_ERROR;free(pixels);return status;
#undef glGetIntegerv
#undef glGenTextures
#undef glDeleteTextures
#undef glBindTexture
#undef glTexParameteri
#undef glTexImage2D
#undef glViewport
#undef glReadBuffer
#undef glPixelStorei
#undef glReadPixels
}
