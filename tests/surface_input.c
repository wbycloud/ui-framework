#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include <stdio.h>
#include <string.h>

#include "ui_framework/ui.h"

typedef struct input_state {
    ui_surface_t *surface;
    ui_input_event_t last;
    char text[16];
    int events;
    int text_events;
    int frames;
} input_state_t;

static int failures;

static void check(int condition, const char *description)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", description);
        ++failures;
    }
}

static void on_input(ui_surface_t *surface, const ui_input_event_t *event,
                       void *user_data)
{
    input_state_t *state = (input_state_t *)user_data;
    check(surface == state->surface, "input callback identifies surface");
    check(event->size == sizeof(*event), "input event size");
    state->last = *event;
    state->last.text_utf8 = NULL;
    ++state->events;
    if (event->kind == UI_INPUT_TEXT) {
        check(event->text_utf8 != NULL, "text callback has borrowed UTF-8");
        strcpy_s(state->text, sizeof(state->text), event->text_utf8);
        ++state->text_events;
    }
}

static void on_frame(ui_surface_t *surface, void *user_data)
{
    input_state_t *state = (input_state_t *)user_data;
    check(surface == state->surface, "paint callback identifies surface");
    ++state->frames;
}

static void exercise_surface(ui_host_t *host, ui_surface_kind_t kind,
                               const char *id)
{
    ui_surface_desc_t descriptor;
    ui_surface_t *surface;
    input_state_t state;
    HWND hwnd;
    POINT screen;
    BYTE keyboard[256], modified[256];
    int count;

    memset(&descriptor, 0, sizeof(descriptor));
    descriptor.size = sizeof(descriptor); descriptor.id = id;
    descriptor.kind = kind; descriptor.visible = 1;
    descriptor.rect.x = 13; descriptor.rect.y = 17;
    descriptor.rect.width = 240; descriptor.rect.height = 160;
    surface = ui_surface_create(host, &descriptor);
    check(surface != NULL, "create input-capable surface");
    if (surface == NULL) return;
    hwnd = (HWND)ui_surface_native_handle(surface);
    check(hwnd != NULL && IsWindowUnicode(hwnd), "surface uses Unicode window procedure");
    memset(&state, 0, sizeof(state)); state.surface = surface;
    check(ui_surface_set_input_callback(surface, on_input, &state) == UI_STATUS_OK,
          "install C input callback");
    check(ui_surface_set_callbacks(surface, NULL, on_frame, &state) == UI_STATUS_OK,
          "install independent frame callback");

    SendMessageW(hwnd, WM_MOUSEMOVE, MK_CONTROL | MK_SHIFT, MAKELPARAM(30, 45));
    check(state.last.kind == UI_INPUT_POINTER_MOVE && state.last.x == 20 && state.last.y == 30,
          "144 DPI child-local mouse position is logical");
    check((state.last.modifiers & (UI_INPUT_MODIFIER_CONTROL | UI_INPUT_MODIFIER_SHIFT)) ==
              (UI_INPUT_MODIFIER_CONTROL | UI_INPUT_MODIFIER_SHIFT),
          "pointer modifiers preserve CTRL and SHIFT");
    SendMessageW(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(30, 45));
    check(state.last.kind == UI_INPUT_POINTER_DOWN && state.last.pointer_button == 1,
          "left button dispatch");
    check(GetFocus() == hwnd && GetCapture() == hwnd, "click focuses and captures surface");
    SendMessageW(hwnd, WM_RBUTTONDOWN, MK_LBUTTON | MK_RBUTTON, MAKELPARAM(30, 45));
    check(state.last.pointer_button == 2, "right button dispatch");
    SendMessageW(hwnd, WM_MBUTTONDOWN, MK_LBUTTON | MK_RBUTTON | MK_MBUTTON, MAKELPARAM(30, 45));
    check(state.last.pointer_button == 3, "middle button dispatch");
    SendMessageW(hwnd, WM_LBUTTONUP, MK_RBUTTON | MK_MBUTTON, MAKELPARAM(30, 45));
    check(state.last.kind == UI_INPUT_POINTER_UP && state.last.pointer_button == 1 && GetCapture() == hwnd,
          "capture remains while another button is down");
    SendMessageW(hwnd, WM_RBUTTONUP, MK_MBUTTON, MAKELPARAM(30, 45));
    SendMessageW(hwnd, WM_MBUTTONUP, 0, MAKELPARAM(30, 45));
    check(state.last.pointer_button == 3 && GetCapture() != hwnd,
          "last button release removes capture");
    SendMessageW(hwnd, WM_MOUSEMOVE, 0, MAKELPARAM(-30, -45));
    check(state.last.x == -20 && state.last.y == -30, "captured negative positions remain signed");

    screen.x = 30; screen.y = 45;
    check(ClientToScreen(hwnd, &screen) != 0, "map wheel point to screen");
    SendMessageW(hwnd, WM_MOUSEWHEEL, MAKEWPARAM(MK_CONTROL, -120), MAKELPARAM(screen.x, screen.y));
    check(state.last.kind == UI_INPUT_WHEEL && state.last.x == 20 && state.last.y == 30 &&
              state.last.wheel_delta == -120 && (state.last.modifiers & UI_INPUT_MODIFIER_CONTROL),
          "wheel screen point maps to local logical coordinates and signed delta");

    check(GetKeyboardState(keyboard) != 0, "save thread keyboard state");
    memcpy(modified, keyboard, sizeof(modified));
    modified[VK_CONTROL] = 0x80; modified[VK_SHIFT] = 0x80; modified[VK_MENU] = 0;
    check(SetKeyboardState(modified) != 0, "set synthetic thread modifier state");
    SendMessageW(hwnd, WM_KEYDOWN, 'A', 1);
    check(state.last.kind == UI_INPUT_KEY_DOWN && state.last.key_code == 'A' &&
              state.last.modifiers == (UI_INPUT_MODIFIER_CONTROL | UI_INPUT_MODIFIER_SHIFT),
          "key down and thread modifiers dispatch");
    SendMessageW(hwnd, WM_KEYUP, 'A', (LPARAM)0xc0000001u);
    check(state.last.kind == UI_INPUT_KEY_UP && state.last.key_code == 'A', "key up dispatch");
    SendMessageW(hwnd, WM_SYSKEYDOWN, VK_F10, (LPARAM)0x20000001u);
    check(state.last.kind == UI_INPUT_KEY_DOWN && (state.last.modifiers & UI_INPUT_MODIFIER_ALT),
          "system key context preserves ALT");
    check(SetKeyboardState(keyboard) != 0, "restore thread keyboard state");

    SendMessageW(hwnd, WM_CHAR, 0x4e2d, 1);
    check(state.last.kind == UI_INPUT_TEXT && strcmp(state.text, "\xe4\xb8\xad") == 0,
          "Chinese UTF-16 input becomes UTF-8");
    count = state.text_events;
    SendMessageW(hwnd, WM_CHAR, 0xd83d, 1);
    check(state.text_events == count, "high surrogate waits for pair");
    SendMessageW(hwnd, WM_CHAR, 0xde00, 1);
    check(state.text_events == count + 1 && strcmp(state.text, "\xf0\x9f\x98\x80") == 0,
          "emoji surrogate pair becomes one UTF-8 event");
    SendMessageW(hwnd, WM_CHAR, 0xdc00, 1);
    check(strcmp(state.text, "\xef\xbf\xbd") == 0, "unpaired low surrogate is replaced");
    count = state.text_events;
    SendMessageW(hwnd, WM_CHAR, 0xd83d, 1);
    SendMessageW(hwnd, WM_CHAR, 'x', 1);
    check(state.text_events == count + 2 && strcmp(state.text, "x") == 0,
          "unpaired high surrogate is replaced before next character");
    SendMessageW(hwnd, WM_CHAR, 0xd83d, 1);
    SendMessageW(hwnd, WM_KILLFOCUS, 0, 0);
    SendMessageW(hwnd, WM_CHAR, 0xde00, 1);
    check(strcmp(state.text, "\xef\xbf\xbd") == 0, "focus loss clears partial surrogate");

    SendMessageW(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(30, 45));
    SendMessageW(hwnd, WM_CANCELMODE, 0, 0);
    check(GetCapture() != hwnd, "cancel mode releases capture");
    check(ui_surface_invalidate(surface) == UI_STATUS_OK, "invalidate surface after input");
    SendMessageW(hwnd, WM_PAINT, 0, 0);
    check(state.frames > 0, "existing frame callback still runs after input registration");
    count = state.events;
    check(ui_surface_set_input_callback(surface, NULL, NULL) == UI_STATUS_OK, "remove input callback");
    SendMessageW(hwnd, WM_MOUSEMOVE, 0, MAKELPARAM(30, 45));
    SendMessageW(hwnd, WM_CHAR, 'z', 1);
    check(state.events == count, "removed callback receives no further input");
    ui_surface_destroy(surface);
}

int main(void)
{
    WNDCLASSW window_class;
    HWND window;
    ui_host_config_t config;
    ui_host_t *host;
    check(ui_framework_initialize() == UI_STATUS_OK, "initialize DPI awareness");
    memset(&window_class, 0, sizeof(window_class));
    window_class.lpfnWndProc = DefWindowProcW;
    window_class.hInstance = GetModuleHandleW(NULL);
    window_class.lpszClassName = L"UiSurfaceInputTestHost";
    check(RegisterClassW(&window_class) != 0, "register input test host");
    window = CreateWindowExW(0, window_class.lpszClassName, L"Surface input test",
        WS_OVERLAPPEDWINDOW, 0, 0, 800, 600, NULL, NULL, window_class.hInstance, NULL);
    if (!window) return 1;
    memset(&config, 0, sizeof(config)); config.size = sizeof(config);
    config.api_version = UI_FRAMEWORK_API_VERSION; config.native_parent = window;
    host = ui_host_create(&config);
    if (!host) { DestroyWindow(window); return 1; }
    check(ui_host_set_dpi(host, 144) == UI_STATUS_OK, "set 144 DPI");
    check(ui_surface_set_input_callback(NULL, on_input, NULL) == UI_STATUS_INVALID_ARGUMENT,
          "null surface callback registration rejected");
    exercise_surface(host, UI_SURFACE_NATIVE, "input.native");
    exercise_surface(host, UI_SURFACE_OPENGL, "input.opengl");
    ui_host_destroy(host); DestroyWindow(window);
    UnregisterClassW(window_class.lpszClassName, window_class.hInstance);
    printf("Surface input integration: %d failure(s)\n", failures);
    return failures != 0;
}
