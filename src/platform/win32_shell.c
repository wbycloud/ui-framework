#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0603

#include <windows.h>
#include <commctrl.h>

#include <stdint.h>
#include <limits.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../ui_internal.h"
#include "ui_framework/native.h"
#include "ui_framework/shell.h"
#ifdef UI_FRAMEWORK_ENABLE_LIGHT_WEB
#include "ui_framework/light_web.h"
#endif

#pragma comment(lib, "comctl32.lib")

#define UI_NATIVE_FIRST_COMMAND 0x5000u
#define UI_WEB_PANEL_TITLE_HEIGHT 28

struct ui_shell {
    ui_native_shell_t *native;
};

struct ui_content_slot {
    ui_shell_t *shell;
    char *panel_id;
    ui_rect_t frame_rect;
    ui_rect_t rect;
    ui_rect_t pixel_rect;
    int visible;
    int floating;
    ui_surface_t *surface;
    ui_web_view_t *web_view;
    struct ui_content_slot *next;
};

typedef struct ui_native_command_binding {
    UINT native_id;
    char *command_id;
} ui_native_command_binding_t;

typedef struct ui_native_panel {
    HWND hwnd;
    HWND tag;
    char *id;
    ui_panel_kind_t kind;
    ui_layout_region_t dock_region;
    int floating;
    int manual_floating;
    int closed;
    int positioned;
    int preferred_width;
    int collapsed, order, height;
    ui_rect_t saved_float;
    uint32_t saved_dpi;
    uint32_t floating_dpi;
    HFONT floating_font;
    HWND popup;
    ui_host_t *popup_host;
    ui_web_backend_t *popup_backend;
    ui_web_view_t *popup_view;
    ui_content_slot_t *slot;
    uint64_t tab_group;
    int tab_active;
} ui_native_panel_t;

struct ui_native_shell {
    ui_shell_t shell;
    ui_content_slot_t *slots;
    int web_chrome;
    int offscreen;
    int reflowing;
    ui_host_t *host;
    HWND parent;
    HWND menu_owner;
    int managed_activation;
    int active;
    HMENU previous_menu;
    HMENU menu;
    HWND toolbar;
    HFONT font;
    uint32_t font_dpi;
    ui_native_command_binding_t *bindings;
    size_t binding_count;
    size_t binding_capacity;
    ui_native_panel_t *panels;
    size_t panel_count;
    size_t panel_capacity;
    int layout_enabled, drag_active, drag_x, drag_y;
    char drag_id[64];
    ui_layout_region_t drag_region;
    HWND drag_preview;
    int split_active;
    ui_layout_region_t split_region;
    char split_id[64];
    void *split_snapshot;
    size_t split_bytes;
    HWND gesture_focus;
    ui_layout_desc_t default_layout;
    int bottom_extent;
    uint64_t next_tab_group;
    char drag_tab[64];
};

static HFONT create_font_for_dpi(uint32_t dpi);
static RECT logical_rect_to_pixels(const ui_rect_t *logical, uint32_t dpi);
static int logical_to_pixels(int value, uint32_t dpi);
static ui_status_t web_shell_reflow(ui_native_shell_t *shell);
static void close_popup(ui_native_panel_t *panel);
static void update_native_slot(ui_content_slot_t *slot);
static ui_rect_t panel_dock_rect(ui_native_shell_t *, ui_native_panel_t *, ui_rect_t);
static void position_panel(ui_native_shell_t *, ui_native_panel_t *, HWND);
static LRESULT CALLBACK layout_subclass(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static void end_splitter(ui_native_shell_t *,int);
static void clamp_floating_windows(ui_native_shell_t *);
static void prepare_layout(ui_native_shell_t *);
static int panel_tab_visible(const ui_native_panel_t *);

static ui_content_slot_t *find_slot(ui_native_shell_t *shell,
                                     const char *panel_id)
{
    ui_content_slot_t *slot;
    for (slot = shell->slots; slot != NULL; slot = slot->next) {
        if ((panel_id == NULL && slot->panel_id == NULL) ||
            (panel_id != NULL && slot->panel_id != NULL &&
             strcmp(panel_id, slot->panel_id) == 0)) return slot;
    }
    return NULL;
}

static ui_content_slot_t *create_slot(ui_native_shell_t *shell,
                                       const char *panel_id)
{
    ui_content_slot_t *slot = find_slot(shell, panel_id);
    if (slot != NULL) return slot;
    slot = (ui_content_slot_t *)calloc(1, sizeof(*slot));
    if (slot == NULL) return NULL;
    if (panel_id != NULL) {
        slot->panel_id = ui_strdup(panel_id);
        if (slot->panel_id == NULL) { free(slot); return NULL; }
    }
    slot->shell = &shell->shell;
    slot->next = shell->slots;
    shell->slots = slot;
    return slot;
}

static void detach_slot(ui_content_slot_t *slot)
{
    if (slot == NULL) return;
    if (slot->surface != NULL && slot->panel_id != NULL) {
        HWND window = (HWND)ui_surface_native_handle(slot->surface);
        if (window != NULL && IsWindow(window))
            (void)SetParent(window, slot->shell->native->parent);
    }
    if (slot->web_view != NULL && slot->panel_id != NULL) {
        HWND window = (HWND)ui_web_view_native_handle(slot->web_view);
        if (window != NULL && IsWindow(window))
            (void)SetParent(window, slot->shell->native->parent);
    }
    slot->surface = NULL;
    slot->web_view = NULL;
    slot->visible = 0;
}

static int set_panel_dpi(ui_native_panel_t *panel, uint32_t dpi)
{
    HFONT font = create_font_for_dpi(dpi);
    if (font == NULL) return 0;
    (void)SendMessageW(panel->hwnd, WM_SETFONT, (WPARAM)font, TRUE);
    if (panel->floating_font != NULL) DeleteObject(panel->floating_font);
    panel->floating_font = font;
    panel->floating_dpi = dpi;
    return 1;
}

static ui_native_panel_t *find_panel(ui_native_shell_t *shell,
                                     const char *id)
{
    size_t index;
    for (index = 0; index < shell->panel_count; ++index) {
        if (strcmp(shell->panels[index].id, id) == 0) {
            return &shell->panels[index];
        }
    }
    return NULL;
}

static LRESULT CALLBACK panel_subclass_proc(HWND hwnd, UINT message,
                                             WPARAM w_param, LPARAM l_param,
                                             UINT_PTR subclass_id,
                                             DWORD_PTR reference)
{
    ui_native_shell_t *shell = (ui_native_shell_t *)reference;
    size_t index;
    (void)subclass_id;
    if(message==WM_LBUTTONDOWN&&shell->layout_enabled&&(short)HIWORD(l_param)<logical_to_pixels(28,shell->host->dpi)){
        for(index=0;index<shell->panel_count;++index)if(shell->panels[index].hwnd==hwnd){(void)ui_shell_begin_panel_drag(&shell->shell,shell->panels[index].id);return 0;}}
    if (message == WM_CLOSE || message == WM_DPICHANGED) {
        for (index = 0; index < shell->panel_count; ++index) {
            ui_native_panel_t *panel = &shell->panels[index];
            if (panel->hwnd == hwnd && message == WM_CLOSE) {
                shell->panels[index].closed = 1;
                ShowWindow(hwnd, SW_HIDE);
                if (shell->web_chrome && panel->popup != NULL) {
                    panel->slot->visible = 0;
                    ShowWindow(panel->popup, SW_HIDE);
                }
                return 0;
            }
            if (panel->hwnd == hwnd && message == WM_DPICHANGED && panel->floating) {
                UINT dpi = HIWORD(w_param);
                const RECT *suggested = (const RECT *)l_param;
                if (dpi == 0) dpi = LOWORD(w_param);
                if (dpi == 0) break;
                if (!set_panel_dpi(panel, dpi)) break;
                if (suggested != NULL) {
                    (void)SetWindowPos(hwnd, NULL, suggested->left, suggested->top,
                                       suggested->right - suggested->left,
                                       suggested->bottom - suggested->top,
                                       SWP_NOACTIVATE | SWP_NOZORDER);
                }
                panel->positioned = 1;
                return 0;
            }
        }
    }
    return DefSubclassProc(hwnd, message, w_param, l_param);
}

static wchar_t *utf8_to_wide(const char *value)
{
    int length;
    wchar_t *result;

    if (value == NULL) {
        return NULL;
    }

    length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                 value, -1, NULL, 0);
    if (length <= 0) {
        length = MultiByteToWideChar(CP_UTF8, 0, value, -1, NULL, 0);
    }
    if (length <= 0) {
        return NULL;
    }

    /* TB_ADDSTRING consumes a list terminated by two NUL characters. */
    result = (wchar_t *)malloc(((size_t)length + 1u) * sizeof(*result));
    if (result == NULL) {
        return NULL;
    }
    if (MultiByteToWideChar(CP_UTF8, 0, value, -1, result, length) <= 0) {
        free(result);
        return NULL;
    }
    result[length] = L'\0';
    return result;
}

static char *duplicate_string(const char *value)
{
    size_t length;
    char *result;

    if (value == NULL) {
        value = "";
    }
    length = strlen(value);
    result = (char *)malloc(length + 1u);
    if (result != NULL) {
        memcpy(result, value, length + 1u);
    }
    return result;
}

static int ensure_capacity(void **items,
                           size_t *capacity,
                           size_t count,
                           size_t item_size)
{
    size_t next_capacity;
    void *new_items;

    if (count < *capacity) {
        return 1;
    }
    next_capacity = *capacity == 0u ? 8u : *capacity * 2u;
    if (next_capacity < count + 1u ||
        next_capacity > (SIZE_MAX / item_size)) {
        return 0;
    }
    new_items = realloc(*items, next_capacity * item_size);
    if (new_items == NULL) {
        return 0;
    }
    *items = new_items;
    *capacity = next_capacity;
    return 1;
}

static int menu_entry_compare(const void *left, const void *right)
{
    const ui_menu_entry_t *const *a =
        (const ui_menu_entry_t *const *)left;
    const ui_menu_entry_t *const *b =
        (const ui_menu_entry_t *const *)right;

    if ((*a)->order < (*b)->order) {
        return -1;
    }
    if ((*a)->order > (*b)->order) {
        return 1;
    }
    return strcmp((*a)->id, (*b)->id);
}

static int toolbar_item_compare(const void *left, const void *right)
{
    const ui_toolbar_item_entry_t *const *a =
        (const ui_toolbar_item_entry_t *const *)left;
    const ui_toolbar_item_entry_t *const *b =
        (const ui_toolbar_item_entry_t *const *)right;

    if ((*a)->order < (*b)->order) {
        return -1;
    }
    if ((*a)->order > (*b)->order) {
        return 1;
    }
    return strcmp((*a)->id, (*b)->id);
}

static int toolbar_compare(const void *left, const void *right)
{
    const ui_toolbar_entry_t *const *a = (const ui_toolbar_entry_t *const *)left;
    const ui_toolbar_entry_t *const *b = (const ui_toolbar_entry_t *const *)right;
    if ((*a)->order != (*b)->order) {
        return (*a)->order < (*b)->order ? -1 : 1;
    }
    return strcmp((*a)->id, (*b)->id);
}

static int collect_menu_entries(ui_native_shell_t *shell,
                                ui_menu_entry_t ***entries,
                                size_t *count)
{
    ui_menu_entry_t *entry;
    ui_menu_entry_t **items = NULL;
    size_t item_count = 0u;
    size_t capacity = 0u;

    for (entry = shell->host->menus; entry != NULL; entry = entry->next) {
        if (!ensure_capacity((void **)&items, &capacity, item_count,
                             sizeof(*items))) {
            free(items);
            return 0;
        }
        items[item_count] = entry;
        item_count += 1u;
    }
    qsort(items, item_count, sizeof(*items), menu_entry_compare);
    *entries = items;
    *count = item_count;
    return 1;
}

static int collect_toolbar_items(ui_native_shell_t *shell,
                                 const char *toolbar_id,
                                 ui_toolbar_item_entry_t ***entries,
                                 size_t *count)
{
    ui_toolbar_item_entry_t *entry;
    ui_toolbar_item_entry_t **items = NULL;
    size_t item_count = 0u;
    size_t capacity = 0u;

    for (entry = shell->host->toolbar_items; entry != NULL;
         entry = entry->next) {
        if (strcmp(entry->toolbar_id, toolbar_id) != 0) {
            continue;
        }
        if (!ensure_capacity((void **)&items, &capacity, item_count,
                             sizeof(*items))) {
            free(items);
            return 0;
        }
        items[item_count] = entry;
        item_count += 1u;
    }
    qsort(items, item_count, sizeof(*items), toolbar_item_compare);
    *entries = items;
    *count = item_count;
    return 1;
}

static void free_bindings(ui_native_shell_t *shell)
{
    size_t index;

    for (index = 0u; index < shell->binding_count; ++index) {
        free(shell->bindings[index].command_id);
    }
    free(shell->bindings);
    shell->bindings = NULL;
    shell->binding_count = 0u;
    shell->binding_capacity = 0u;
}

static void free_panels(ui_native_shell_t *shell)
{
    size_t index;

    for (index = 0u; index < shell->panel_count; ++index) {
        detach_slot(shell->panels[index].slot);
        if (shell->panels[index].hwnd != NULL) {
            DestroyWindow(shell->panels[index].hwnd);
        }
        close_popup(&shell->panels[index]);
        if (shell->panels[index].tag != NULL) {
            DestroyWindow(shell->panels[index].tag);
        }
        if (shell->panels[index].floating_font != NULL) {
            DeleteObject(shell->panels[index].floating_font);
        }
        free(shell->panels[index].id);
    }
    free(shell->panels);
    shell->panels = NULL;
    shell->panel_count = 0u;
    shell->panel_capacity = 0u;
}

static int add_binding(ui_native_shell_t *shell,
                       UINT native_id,
                       const char *command_id)
{
    ui_native_command_binding_t *binding;

    if (native_id < UI_NATIVE_FIRST_COMMAND || native_id >= 0xF000u) {
        return 0;
    }
    if (!ensure_capacity((void **)&shell->bindings,
                         &shell->binding_capacity,
                         shell->binding_count,
                         sizeof(*shell->bindings))) {
        return 0;
    }
    binding = &shell->bindings[shell->binding_count];
    binding->native_id = native_id;
    binding->command_id = duplicate_string(command_id);
    if (binding->command_id == NULL) {
        return 0;
    }
    shell->binding_count += 1u;
    return 1;
}

static HMENU find_submenu(HMENU menu, const wchar_t *title)
{
    int count;
    int index;

    count = GetMenuItemCount(menu);
    for (index = 0; index < count; ++index) {
        int length;
        wchar_t *existing;
        HMENU submenu;

        submenu = GetSubMenu(menu, index);
        if (submenu == NULL) {
            continue;
        }
        length = GetMenuStringW(menu, (UINT)index, NULL, 0,
                                MF_BYPOSITION);
        if (length <= 0) {
            continue;
        }
        existing = (wchar_t *)malloc((size_t)(length + 1) *
                                     sizeof(*existing));
        if (existing == NULL) {
            return NULL;
        }
        (void)GetMenuStringW(menu, (UINT)index, existing, length + 1,
                             MF_BYPOSITION);
        if (wcscmp(existing, title) == 0) {
            free(existing);
            return submenu;
        }
        free(existing);
    }
    return NULL;
}

static HMENU get_or_create_menu_path(HMENU root, const char *path)
{
    char *copy;
    char *cursor;
    HMENU current = root;

    copy = duplicate_string(path);
    if (copy == NULL) {
        return NULL;
    }
    cursor = copy;
    while (*cursor != '\0') {
        char *separator = strchr(cursor, '/');
        wchar_t *title;
        HMENU child;

        if (separator != NULL) {
            *separator = '\0';
        }
        if (*cursor == '\0') {
            free(copy);
            return NULL;
        }
        title = utf8_to_wide(cursor);
        if (title == NULL) {
            free(copy);
            return NULL;
        }
        child = find_submenu(current, title);
        if (child == NULL) {
            child = CreatePopupMenu();
            if (child == NULL ||
                !AppendMenuW(current, MF_POPUP, (UINT_PTR)child, title)) {
                if (child != NULL) {
                    DestroyMenu(child);
                }
                free(title);
                free(copy);
                return NULL;
            }
        }
        free(title);
        current = child;
        if (separator == NULL) {
            break;
        }
        cursor = separator + 1;
    }
    free(copy);
    return current;
}

static ui_status_t refresh_menu(ui_native_shell_t *shell)
{
    HMENU menu;
    ui_menu_entry_t **entries = NULL;
    size_t count = 0u;
    size_t index;

    if (!collect_menu_entries(shell, &entries, &count)) {
        return UI_STATUS_OUT_OF_MEMORY;
    }
    menu = CreateMenu();
    if (menu == NULL) {
        free(entries);
        return UI_STATUS_PLATFORM_ERROR;
    }

    for (index = 0u; index < count; ++index) {
        wchar_t *title = utf8_to_wide(entries[index]->title);
        HMENU target;
        UINT native_id;

        if (title == NULL) {
            DestroyMenu(menu);
            free(entries);
            return UI_STATUS_OUT_OF_MEMORY;
        }
        target = get_or_create_menu_path(menu, entries[index]->menu_path);
        if (shell->binding_count >= 0xF000u - UI_NATIVE_FIRST_COMMAND) {
            free(title);
            DestroyMenu(menu);
            free(entries);
            return UI_STATUS_OUT_OF_MEMORY;
        }
        native_id = UI_NATIVE_FIRST_COMMAND + (UINT)shell->binding_count;
        if (target == NULL || native_id >= 0xF000u ||
            !add_binding(shell, native_id, entries[index]->command_id) ||
            !AppendMenuW(target, MF_STRING, native_id, title)) {
            free(title);
            DestroyMenu(menu);
            free(entries);
            return UI_STATUS_OUT_OF_MEMORY;
        }
        free(title);
    }
    free(entries);

    if (!shell->web_chrome && shell->active) {
        if (!SetMenu(shell->menu_owner, menu)) {
            DestroyMenu(menu);
            return UI_STATUS_PLATFORM_ERROR;
        }
        (void)DrawMenuBar(shell->menu_owner);
    } else if (!shell->web_chrome && shell->menu != NULL && GetMenu(shell->menu_owner) == shell->menu) {
        if (!SetMenu(shell->menu_owner, shell->previous_menu)) {
            DestroyMenu(menu);
            return UI_STATUS_PLATFORM_ERROR;
        }
        (void)DrawMenuBar(shell->menu_owner);
    }
    if (shell->menu != NULL) DestroyMenu(shell->menu);
    shell->menu = menu;
    return UI_STATUS_OK;
}

static ui_status_t refresh_toolbar(ui_native_shell_t *shell)
{
    ui_toolbar_entry_t *toolbar_entry;
    ui_toolbar_entry_t **toolbars = NULL;
    size_t toolbar_count = 0;
    size_t toolbar_capacity = 0;
    size_t toolbar_index;
    ui_toolbar_item_entry_t **items = NULL;
    size_t item_count = 0u;
    size_t index;
    INITCOMMONCONTROLSEX common_controls;
    HWND toolbar;

    if (shell->web_chrome) {
        ui_toolbar_item_entry_t *item;
        for (item = shell->host->toolbar_items; item != NULL; item = item->next) {
            if (!add_binding(shell, UI_NATIVE_FIRST_COMMAND +
                              (UINT)shell->binding_count, item->command_id))
                return UI_STATUS_OUT_OF_MEMORY;
        }
        return UI_STATUS_OK;
    }

    ZeroMemory(&common_controls, sizeof(common_controls));
    common_controls.dwSize = sizeof(common_controls);
    common_controls.dwICC = ICC_BAR_CLASSES;
    if (!InitCommonControlsEx(&common_controls)) {
        return UI_STATUS_PLATFORM_ERROR;
    }

    for (toolbar_entry = shell->host->toolbars;
         toolbar_entry != NULL;
         toolbar_entry = toolbar_entry->next) {
        if (toolbar_entry->visible) {
            if (!ensure_capacity((void **)&toolbars, &toolbar_capacity,
                                  toolbar_count, sizeof(*toolbars))) {
                free(toolbars);
                return UI_STATUS_OUT_OF_MEMORY;
            }
            toolbars[toolbar_count++] = toolbar_entry;
        }
    }
    qsort(toolbars, toolbar_count, sizeof(*toolbars), toolbar_compare);

    if (shell->toolbar != NULL) {
        DestroyWindow(shell->toolbar);
        shell->toolbar = NULL;
    }
    if (toolbar_count == 0) {
        free(toolbars);
        return UI_STATUS_OK;
    }

    toolbar = CreateWindowExW(0,
                              TOOLBARCLASSNAMEW,
                              NULL,
                              WS_CHILD | (shell->active ? WS_VISIBLE : 0) | CCS_TOP |
                                  CCS_NORESIZE | CCS_NOPARENTALIGN |
                                  TBSTYLE_FLAT | TBSTYLE_LIST |
                                  TBSTYLE_TOOLTIPS,
                              0, 0, 0, 0,
                              shell->parent,
                              NULL,
                              GetModuleHandleW(NULL),
                              NULL);
    if (toolbar == NULL) {
        free(toolbars);
        return UI_STATUS_PLATFORM_ERROR;
    }
    (void)SendMessageW(toolbar, TB_BUTTONSTRUCTSIZE,
                       (WPARAM)sizeof(TBBUTTON), 0);
    (void)SendMessageW(toolbar, TB_SETEXTENDEDSTYLE, 0,
                       (LPARAM)TBSTYLE_EX_MIXEDBUTTONS);

    for (toolbar_index = 0; toolbar_index < toolbar_count; ++toolbar_index) {
      if (!collect_toolbar_items(shell, toolbars[toolbar_index]->id, &items, &item_count)) {
          DestroyWindow(toolbar);
          free(toolbars);
          return UI_STATUS_OUT_OF_MEMORY;
      }
      for (index = 0u; index < item_count; ++index) {
        TBBUTTON button;
        wchar_t *title = utf8_to_wide(items[index]->title);
        int string_index;
        UINT native_id = UI_NATIVE_FIRST_COMMAND +
                         (UINT)shell->binding_count;

        if (title == NULL || shell->binding_count >= 0xF000u - UI_NATIVE_FIRST_COMMAND ||
            !add_binding(shell, native_id, items[index]->command_id)) {
            free(title);
            DestroyWindow(toolbar);
            free(items);
            free(toolbars);
            return UI_STATUS_OUT_OF_MEMORY;
        }
        string_index = (int)SendMessageW(toolbar, TB_ADDSTRINGW, 0,
                                         (LPARAM)title);
        free(title);
        if (string_index < 0) {
            DestroyWindow(toolbar);
            free(items);
            free(toolbars);
            return UI_STATUS_PLATFORM_ERROR;
        }
        ZeroMemory(&button, sizeof(button));
        button.iBitmap = I_IMAGENONE;
        button.idCommand = native_id;
        button.fsState = TBSTATE_ENABLED;
        button.fsStyle = BTNS_BUTTON | BTNS_AUTOSIZE | BTNS_SHOWTEXT;
        button.iString = string_index;
        if (SendMessageW(toolbar, TB_ADDBUTTONSW, 1,
                         (LPARAM)&button) != TRUE) {
            DestroyWindow(toolbar);
            free(items);
            free(toolbars);
            return UI_STATUS_PLATFORM_ERROR;
        }
      }
      free(items);
      items = NULL;
    }
    free(toolbars);
    shell->toolbar = toolbar;
    return UI_STATUS_OK;
}

static ui_status_t append_panel(ui_native_shell_t *shell,
                                const ui_panel_entry_t *entry)
{
        wchar_t *title;
        HWND panel_window = NULL;
        HWND tag = NULL;
        ui_native_panel_t *panel;
        DWORD style;
        DWORD extended_style = 0;

        if (!ensure_capacity((void **)&shell->panels,
                             &shell->panel_capacity,
                             shell->panel_count,
                             sizeof(*shell->panels))) {
            return UI_STATUS_OUT_OF_MEMORY;
        }
        if (shell->offscreen) goto initialize;
        title = utf8_to_wide(entry->title);
        if (title == NULL) {
            return UI_STATUS_OUT_OF_MEMORY;
        }
        if (entry->kind == UI_PANEL_FLOATING && !shell->web_chrome) {
            style = WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN;
            extended_style = WS_EX_TOOLWINDOW;
            panel_window = CreateWindowExW(extended_style,
                                           L"STATIC",
                                           title,
                                           style,
                                           0, 0, 320, 240,
                                           shell->managed_activation ? shell->menu_owner : shell->parent,
                                           NULL,
                                           GetModuleHandleW(NULL),
                                           NULL);
        } else {
            style = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
                    SS_LEFT | SS_NOPREFIX;
            panel_window = CreateWindowExW(0,
                                           L"STATIC",
                                           shell->web_chrome ? L"" : title,
                                           style,
                                           0, 0, 0, 0,
                                           shell->parent,
                                           NULL,
                                           GetModuleHandleW(NULL),
                                           NULL);
        }
        if (panel_window == NULL) {
            free(title);
            return UI_STATUS_PLATFORM_ERROR;
        }
        if (entry->kind == UI_PANEL_SIDEBAR && !shell->web_chrome) {
            tag = CreateWindowExW(0, L"BUTTON", title,
                                    WS_CHILD | BS_MULTILINE,
                                    0, 0, 0, 0, shell->parent, NULL,
                                    GetModuleHandleW(NULL), NULL);
            if (tag == NULL) {
                DestroyWindow(panel_window);
                free(title);
                return UI_STATUS_PLATFORM_ERROR;
            }
        }
        free(title);
initialize:
        panel = &shell->panels[shell->panel_count];
        memset(panel, 0, sizeof(*panel));
        panel->hwnd = panel_window;
        panel->tag = tag;
        panel->id = duplicate_string(entry->id);
        panel->kind = entry->kind;
        panel->dock_region = entry->dock_region;
        panel->preferred_width = entry->preferred_width;
        panel->order = (int)shell->panel_count;
        panel->floating = entry->kind == UI_PANEL_FLOATING && !shell->web_chrome;
        panel->slot = create_slot(shell, entry->id);
        if (panel->id == NULL || panel->slot == NULL || (!shell->offscreen && !SetWindowSubclass(panel_window, panel_subclass_proc,
                                                    1, (DWORD_PTR)shell))) {
            ui_status_t error = panel->id == NULL || panel->slot == NULL ? UI_STATUS_OUT_OF_MEMORY : UI_STATUS_PLATFORM_ERROR;
            DestroyWindow(panel_window);
            if (tag != NULL) DestroyWindow(tag);
            free(panel->id);
            return error;
        }
        shell->panel_count += 1u;
    return UI_STATUS_OK;
}

static ui_status_t refresh_panels(ui_native_shell_t *shell)
{
    ui_panel_entry_t *entry;
    free_panels(shell);
    for (entry = shell->host->panels; entry != NULL; entry = entry->next) {
        ui_status_t status = append_panel(shell, entry);
        if (status != UI_STATUS_OK) return status;
    }
    return UI_STATUS_OK;
}

static int logical_to_pixels(int value, uint32_t dpi)
{
    int64_t scaled;

    if (dpi == 0u) {
        dpi = 96u;
    }
    scaled = ((int64_t)value * (int64_t)dpi);
    if (scaled >= 0) {
        scaled = (scaled + 48) / 96;
    } else {
        scaled = -(((-scaled) + 48) / 96);
    }
    if (scaled > INT_MAX) {
        return INT_MAX;
    }
    if (scaled < INT_MIN) {
        return INT_MIN;
    }
    return (int)scaled;
}

static RECT logical_rect_to_pixels(const ui_rect_t *logical, uint32_t dpi)
{
    RECT result;
    result.left = logical_to_pixels(logical->x, dpi);
    result.top = logical_to_pixels(logical->y, dpi);
    result.right = logical_to_pixels(logical->x + logical->width, dpi);
    result.bottom = logical_to_pixels(logical->y + logical->height, dpi);
    return result;
}

static HFONT create_font_for_dpi(uint32_t dpi)
{
    LOGFONTW font_description;
    memset(&font_description, 0, sizeof(font_description));
    font_description.lfHeight = -MulDiv(9, (int)dpi, 72);
    font_description.lfWeight = FW_NORMAL;
    font_description.lfCharSet = DEFAULT_CHARSET;
    (void)lstrcpyW(font_description.lfFaceName, L"Segoe UI");
    return CreateFontIndirectW(&font_description);
}

static int update_font(ui_native_shell_t *shell, uint32_t dpi)
{
    size_t index;
    HFONT old_font = NULL;
    for (index = 0; index < shell->panel_count; ++index) {
        ui_native_panel_t *panel = &shell->panels[index];
        if (panel->floating && panel->floating_font == NULL &&
            !set_panel_dpi(panel, panel->floating_dpi != 0 ? panel->floating_dpi : dpi))
            return 0;
    }
    if (shell->font == NULL || shell->font_dpi != dpi) {
        HFONT new_font = create_font_for_dpi(dpi);
        if (new_font == NULL) return 0;
        old_font = shell->font;
        shell->font = new_font;
        shell->font_dpi = dpi;
    }
    if (shell->toolbar != NULL) {
        int button_size = logical_to_pixels(28, dpi);
        (void)SendMessageW(shell->toolbar, WM_SETFONT, (WPARAM)shell->font, FALSE);
        (void)SendMessageW(shell->toolbar, TB_SETBUTTONSIZE, 0,
                           MAKELPARAM(button_size, button_size));
        (void)SendMessageW(shell->toolbar, TB_SETPADDING, 0,
                           MAKELPARAM(logical_to_pixels(8, dpi), logical_to_pixels(6, dpi)));
        (void)SendMessageW(shell->toolbar, TB_AUTOSIZE, 0, 0);
    }
    for (index = 0; index < shell->panel_count; ++index) {
        ui_native_panel_t *panel = &shell->panels[index];
        HFONT font = panel->floating ? panel->floating_font : shell->font;
        (void)SendMessageW(panel->hwnd, WM_SETFONT, (WPARAM)font, TRUE);
        if (shell->panels[index].tag != NULL)
            (void)SendMessageW(shell->panels[index].tag, WM_SETFONT, (WPARAM)shell->font, TRUE);
    }
    if (old_font != NULL) DeleteObject(old_font);
    return 1;
}

static void set_floating(ui_native_shell_t *shell, ui_native_panel_t *panel,
                          int floating)
{
    if (panel->floating == floating) return;
    ShowWindow(panel->hwnd, SW_HIDE);
    if (floating) {
        SetWindowLongPtrW(panel->hwnd, GWL_STYLE,
                          WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME |
                          WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
        SetParent(panel->hwnd, NULL);
        SetWindowLongPtrW(panel->hwnd, GWLP_HWNDPARENT,
                            (LONG_PTR)(shell->managed_activation ? shell->menu_owner : shell->parent));
        SetWindowLongPtrW(panel->hwnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
        (void)set_panel_dpi(panel, shell->host->dpi);
    } else {
        SetWindowLongPtrW(panel->hwnd, GWL_STYLE,
                          WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SS_NOPREFIX);
        SetParent(panel->hwnd, shell->parent);
        SetWindowLongPtrW(panel->hwnd, GWL_EXSTYLE, 0);
        panel->closed = 0;
        (void)SendMessageW(panel->hwnd, WM_SETFONT, (WPARAM)shell->font, TRUE);
        if (panel->floating_font != NULL) DeleteObject(panel->floating_font);
        panel->floating_font = NULL;
        panel->floating_dpi = 0;
    }
    (void)SetWindowPos(panel->hwnd, NULL, 0, 0, 0, 0,
                       SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                       SWP_NOACTIVATE | SWP_FRAMECHANGED);
    panel->floating = floating;
    panel->positioned = 0;
}

static ui_native_panel_t *find_popup(ui_native_shell_t *shell, HWND window)
{
    size_t index;
    for (index = 0; index < shell->panel_count; ++index)
        if (shell->panels[index].popup == window) return &shell->panels[index];
    return NULL;
}

static void update_popup(ui_native_shell_t *shell, ui_native_panel_t *panel)
{
    RECT client;
    int title_height;
    uint32_t dpi = panel->floating_dpi != 0 ? panel->floating_dpi : shell->host->dpi;
    if (panel->popup == NULL || !GetClientRect(panel->popup, &client)) return;
    title_height = logical_to_pixels(UI_WEB_PANEL_TITLE_HEIGHT, dpi);
    if (title_height > client.bottom) title_height = client.bottom;
    (void)MoveWindow(panel->hwnd, 1, title_height,
                     client.right > 2 ? client.right - 2 : 0,
                     client.bottom > title_height + 1 ? client.bottom - title_height - 1 : 0,
                     TRUE);
    if (panel->popup_view != NULL) {
        ui_rect_t title = {0, 0, MulDiv(client.right, 96, (int)dpi),
                           UI_WEB_PANEL_TITLE_HEIGHT};
        (void)ui_web_view_set_rect(panel->popup_view, &title, dpi);
    }
    panel->slot->frame_rect = (ui_rect_t){0, 0,
        MulDiv(client.right, 96, (int)dpi), MulDiv(client.bottom, 96, (int)dpi)};
    panel->slot->rect = (ui_rect_t){0, 0,
        MulDiv(client.right > 2 ? client.right - 2 : 0, 96, (int)dpi),
        MulDiv(client.bottom > title_height + 1 ? client.bottom - title_height - 1 : 0,
               96, (int)dpi)};
    panel->slot->pixel_rect = (ui_rect_t){0, 0,
        client.right > 2 ? client.right - 2 : 0,
        client.bottom > title_height + 1 ? client.bottom - title_height - 1 : 0};
}

#ifdef UI_FRAMEWORK_ENABLE_LIGHT_WEB
static LRESULT CALLBACK popup_view_subclass_proc(HWND window, UINT message,
                                                  WPARAM w_param, LPARAM l_param,
                                                  UINT_PTR id, DWORD_PTR data)
{
    ui_native_shell_t *shell = (ui_native_shell_t *)data;
    ui_host_t *host = shell->host;
    LRESULT result;
    (void)id;
    ui_dispatch_enter(host);
    result = DefSubclassProc(window, message, w_param, l_param);
    ui_dispatch_leave(host);
    return result;
}

static LRESULT popup_window_message(HWND window, UINT message,
                                           WPARAM w_param, LPARAM l_param)
{
    ui_native_shell_t *shell = (ui_native_shell_t *)GetWindowLongPtrW(window, GWLP_USERDATA);
    ui_native_panel_t *panel = shell != NULL ? find_popup(shell, window) : NULL;
    if (message == WM_NCCREATE) {
        const CREATESTRUCTW *create = (const CREATESTRUCTW *)l_param;
        SetWindowLongPtrW(window, GWLP_USERDATA, (LONG_PTR)create->lpCreateParams);
    } else if (panel != NULL) {
        if (message == WM_CLOSE) {
            panel->closed = 1;
            panel->slot->visible = 0;
            ShowWindow(window, SW_HIDE);
            return 0;
        }
        if (message == WM_SIZE) {
            update_popup(shell, panel);
            if (!shell->reflowing) (void)web_shell_reflow(shell);
            return 0;
        }
        if (message == WM_DPICHANGED) {
            const RECT *suggested = (const RECT *)l_param;
            uint32_t dpi = HIWORD(w_param);
            if (dpi == 0) dpi = LOWORD(w_param);
            if (dpi != 0) (void)set_panel_dpi(panel, dpi);
            if (suggested != NULL)
                (void)SetWindowPos(window, NULL, suggested->left, suggested->top,
                    suggested->right - suggested->left,
                    suggested->bottom - suggested->top, SWP_NOACTIVATE | SWP_NOZORDER);
            update_popup(shell, panel);
            if (!shell->reflowing) (void)web_shell_reflow(shell);
            return 0;
        }
        if (message == WM_NCHITTEST) {
            RECT rect;
            int x = (short)LOWORD(l_param), y = (short)HIWORD(l_param);
            int border = logical_to_pixels(5, panel->floating_dpi);
            GetWindowRect(window, &rect);
            if (y < rect.top + border)
                return x < rect.left + border ? HTTOPLEFT :
                       x >= rect.right - border ? HTTOPRIGHT : HTTOP;
            if (y >= rect.bottom - border)
                return x < rect.left + border ? HTBOTTOMLEFT :
                       x >= rect.right - border ? HTBOTTOMRIGHT : HTBOTTOM;
            if (x < rect.left + border) return HTLEFT;
            if (x >= rect.right - border) return HTRIGHT;
            return HTCLIENT;
        }
        if (message == WM_ERASEBKGND) {
            RECT rect;
            HBRUSH background = CreateSolidBrush(RGB(221, 226, 234));
            GetClientRect(window, &rect);
            FillRect((HDC)w_param, &rect, background);
            DeleteObject(background);
            return 1;
        }
    }
    return DefWindowProcW(window, message, w_param, l_param);
}

static LRESULT CALLBACK popup_window_proc(HWND window, UINT message,
                                           WPARAM w_param, LPARAM l_param)
{
    ui_native_shell_t *shell = (ui_native_shell_t *)GetWindowLongPtrW(window, GWLP_USERDATA);
    ui_host_t *host;
    LRESULT result;
    if (message == WM_NCCREATE)
        shell = (ui_native_shell_t *)((const CREATESTRUCTW *)l_param)->lpCreateParams;
    host = shell != NULL ? shell->host : NULL;
    if (host != NULL) ui_dispatch_enter(host);
    result = popup_window_message(window, message, w_param, l_param);
    if (host != NULL) ui_dispatch_leave(host);
    return result;
}

static void popup_command(ui_host_t *host, uint64_t request_id,
                           const char *command_id, const char *params,
                           const char *source, void *data)
{
    ui_native_shell_t *shell = (ui_native_shell_t *)data;
    ui_host_t *app_host = shell->host;
    size_t index;
    (void)params; (void)source;
    ui_dispatch_enter(app_host);
    for (index = 0; index < shell->panel_count; ++index) {
        ui_native_panel_t *panel = &shell->panels[index];
        if (panel->popup_host != host) continue;
        if (strcmp(command_id, "framework.panel.close") == 0) {
            panel->closed = 1;
            panel->slot->visible = 0;
            ShowWindow(panel->popup, SW_HIDE);
            (void)ui_host_emit_event(app_host, "ui.shell.changed", "{}");
        } else if (strcmp(command_id, "framework.panel.dock") == 0) {
            if (panel->kind == UI_PANEL_SIDEBAR) {
                panel->manual_floating = 0;
                panel->closed = 0;
                (void)ui_native_shell_reflow(shell);
                (void)ui_host_emit_event(app_host, "ui.shell.changed", "{}");
            }
        } else {
            (void)ui_shell_begin_panel_drag(&shell->shell,panel->id);
        }
        break;
    }
    (void)ui_host_reply(host, request_id, 1, "{}");
    ui_dispatch_leave(app_host);
}

static char *popup_html(const ui_panel_entry_t *entry)
{
    static const char prefix[] =
        "<html><body style='margin:0;background:#edf0f5;color:#263143'>"
        "<div style='display:flex;flex-direction:row;height:28px;align-items:center;padding:0 6px'>"
        "<span style='flex:1;font-size:13px;font-weight:600' "
        "onmousedown=\"ui.invoke('framework.panel.drag')\">";
    static const char dock[] =
        "<button style='width:46px;height:24px;border:0;border-radius:4px' "
        "onclick=\"ui.invoke('framework.panel.dock')\">Dock</button>";
    static const char suffix[] =
        "<button style='width:26px;height:24px;border:0;border-radius:4px' "
        "onclick=\"ui.invoke('framework.panel.close')\">&#215;</button></div></body></html>";
    size_t length = strlen(entry->title), i;
    char *html, *cursor;
    if (length > (SIZE_MAX - sizeof(prefix) - sizeof(dock) - sizeof(suffix) - 8) / 6)
        return NULL;
    html = (char *)malloc(length * 6 + sizeof(prefix) + sizeof(dock) + sizeof(suffix) + 8);
    if (html == NULL) return NULL;
    strcpy(html, prefix); cursor = html + strlen(html);
    for (i = 0; i < length; ++i) {
        const char *replacement = NULL;
        if (entry->title[i] == '&') replacement = "&amp;";
        else if (entry->title[i] == '<') replacement = "&lt;";
        else if (entry->title[i] == '>') replacement = "&gt;";
        if (replacement != NULL) {
            size_t count = strlen(replacement);
            memcpy(cursor, replacement, count); cursor += count;
        } else *cursor++ = entry->title[i];
    }
    *cursor = 0;
    strcat(html, "</span>");
    if (entry->kind == UI_PANEL_SIDEBAR) strcat(html, dock);
    strcat(html, suffix);
    return html;
}
#endif

static void close_popup(ui_native_panel_t *panel)
{
#ifdef UI_FRAMEWORK_ENABLE_LIGHT_WEB
    if (panel->popup_view != NULL) ui_web_view_destroy(panel->popup_view);
    if (panel->popup_backend != NULL) ui_light_web_backend_destroy(panel->popup_backend);
#endif
    if (panel->popup_host != NULL) ui_host_destroy(panel->popup_host);
    if (panel->popup != NULL) DestroyWindow(panel->popup);
    panel->popup_view = NULL; panel->popup_backend = NULL;
    panel->popup_host = NULL; panel->popup = NULL;
}

static ui_status_t create_popup(ui_native_shell_t *shell,
                                 ui_native_panel_t *panel)
{
#ifdef UI_FRAMEWORK_ENABLE_LIGHT_WEB
    WNDCLASSW window_class;
    ui_host_config_t host_config;
    ui_light_web_config_t web_config;
    ui_command_desc_t command;
    ui_panel_entry_t *entry;
    char *html;
    size_t i;
    static const char *commands[] = {"framework.panel.close", "framework.panel.dock", "framework.panel.drag"};
    if (panel->popup != NULL) return UI_STATUS_OK;
    memset(&window_class, 0, sizeof(window_class));
    window_class.lpfnWndProc = popup_window_proc;
    window_class.hInstance = GetModuleHandleW(NULL);
    window_class.hCursor = LoadCursorW(NULL, MAKEINTRESOURCEW(32512));
    window_class.lpszClassName = L"UiFrameworkWebPanelPopupV2";
    if (RegisterClassW(&window_class) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        return UI_STATUS_PLATFORM_ERROR;
    panel->popup = CreateWindowExW(WS_EX_TOOLWINDOW, window_class.lpszClassName,
        L"", WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        0, 0, 320, 240, shell->menu_owner, NULL, window_class.hInstance, shell);
    if (panel->popup == NULL) return UI_STATUS_PLATFORM_ERROR;
    memset(&host_config, 0, sizeof(host_config));
    host_config.size = sizeof(host_config); host_config.api_version = UI_FRAMEWORK_API_VERSION;
    host_config.native_parent = panel->popup;
    panel->popup_host = ui_host_create(&host_config);
    if (panel->popup_host == NULL) { close_popup(panel); return UI_STATUS_OUT_OF_MEMORY; }
    memset(&command, 0, sizeof(command));
    command.size = sizeof(command); command.title = "Panel"; command.params_schema_json = "{}";
    command.handler = popup_command; command.user_data = shell;
    for (i = 0; i < sizeof(commands) / sizeof(commands[0]); ++i) {
        command.id = commands[i];
        if (ui_host_register_command(panel->popup_host, &command) != UI_STATUS_OK) {
            close_popup(panel); return UI_STATUS_OUT_OF_MEMORY;
        }
    }
    memset(&web_config, 0, sizeof(web_config));
    web_config.size = sizeof(web_config); web_config.parent_hwnd = panel->popup;
    panel->popup_backend = ui_light_web_backend_create(&web_config);
    if (panel->popup_backend == NULL) { close_popup(panel); return UI_STATUS_OUT_OF_MEMORY; }
    panel->popup_view = ui_web_view_create(panel->popup_host, panel->popup_backend);
    for (entry = shell->host->panels; entry != NULL; entry = entry->next)
        if (strcmp(entry->id, panel->id) == 0) break;
    html = entry != NULL ? popup_html(entry) : NULL;
    if (panel->popup_view == NULL || html == NULL) {
        free(html); close_popup(panel); return UI_STATUS_OUT_OF_MEMORY;
    }
    {
        HWND window = (HWND)ui_web_view_native_handle(panel->popup_view);
        if (window == NULL || !SetWindowSubclass(window, popup_view_subclass_proc,
                                                1, (DWORD_PTR)shell)) {
            free(html); close_popup(panel); return UI_STATUS_PLATFORM_ERROR;
        }
    }
    if (ui_web_view_load_html(panel->popup_view, html) != UI_STATUS_OK) {
        free(html); close_popup(panel); return UI_STATUS_PLATFORM_ERROR;
    }
    free(html);
    (void)set_panel_dpi(panel, shell->host->dpi);
    return UI_STATUS_OK;
#else
    (void)shell; (void)panel;
    return UI_STATUS_UNSUPPORTED;
#endif
}

static ui_status_t web_set_floating(ui_native_shell_t *shell,
                                    ui_native_panel_t *panel, int floating)
{
    if (panel->floating == floating) return UI_STATUS_OK;
    if (floating) {
        ui_status_t status = create_popup(shell, panel);
        if (status != UI_STATUS_OK) return status;
        (void)SetParent(panel->hwnd, panel->popup);
    } else {
        (void)SetParent(panel->hwnd, shell->parent);
        ShowWindow(panel->popup, SW_HIDE);
        panel->closed = 0;
    }
    panel->floating = floating;
    panel->positioned = 0;
    return UI_STATUS_OK;
}

static ui_status_t apply_slot(ui_content_slot_t *slot)
{
    ui_native_shell_t *shell = slot->shell->native;
    ui_status_t status = UI_STATUS_OK;
    if (slot->surface != NULL) {
        ui_rect_t rect = slot->rect;
        if (slot->floating) {
            HWND window = (HWND)ui_surface_native_handle(slot->surface);
            ui_rect_t old_rect = slot->surface->rect;
            ui_rect_t old_pixels = slot->surface->pixel_rect;
            if (window != NULL) (void)MoveWindow(window, 0, 0,
                slot->pixel_rect.width, slot->pixel_rect.height, TRUE);
            /* Floating content has its own DPI and local framebuffer extent. */
            slot->surface->rect = rect;
            slot->surface->pixel_rect = slot->pixel_rect;
            if (slot->surface->resized != NULL &&
                (memcmp(&old_rect, &rect, sizeof(rect)) != 0 ||
                 memcmp(&old_pixels, &slot->pixel_rect, sizeof(old_pixels)) != 0)) {
                ui_dispatch_enter(shell->host);
                slot->surface->resized(slot->surface, &rect, &slot->pixel_rect,
                    ui_shell_surface_dpi(shell->host, slot->surface),
                    slot->surface->callback_user_data);
                ui_dispatch_leave(shell->host);
            }
        } else status = ui_surface_set_rect(slot->surface, &rect);
        if (status == UI_STATUS_OK) status = ui_surface_set_visible(slot->surface, slot->visible);
    }
    if (slot->web_view != NULL) {
        ui_rect_t rect = slot->rect;
        uint32_t dpi = shell->host->dpi;
        if (slot->panel_id != NULL) { rect.x = 0; rect.y = 0; }
        if (!slot->visible) { rect.width = 0; rect.height = 0; }
        if (slot->floating) {
            ui_native_panel_t *panel = find_panel(shell, slot->panel_id);
            if (panel != NULL && panel->floating_dpi != 0) dpi = panel->floating_dpi;
        }
        status = ui_web_view_set_rect(slot->web_view, &rect, dpi);
    }
    return status;
}

static ui_status_t web_shell_reflow(ui_native_shell_t *shell)
{
    ui_rect_t side_rects[3];
    ui_sidebar_state_t states[3];
    size_t dock_count[3] = {0, 0, 0}, dock_index[3] = {0, 0, 0}, i;
    uint32_t dpi = shell->host->dpi;
    ui_status_t status = UI_STATUS_OK;
    ui_content_slot_t *main_slot = find_slot(shell, NULL);
    if (shell->reflowing) return UI_STATUS_OK;
    shell->reflowing = 1;
    if (main_slot != NULL) {
        RECT pixels;
        (void)ui_host_get_rect(shell->host, UI_LAYOUT_REGION_MAIN, &main_slot->rect);
        main_slot->frame_rect = main_slot->rect;
        pixels = logical_rect_to_pixels(&main_slot->rect, dpi);
        main_slot->pixel_rect = (ui_rect_t){pixels.left, pixels.top,
            pixels.right - pixels.left, pixels.bottom - pixels.top};
        main_slot->visible = shell->active && main_slot->rect.width > 0 && main_slot->rect.height > 0;
        status = apply_slot(main_slot);
    }
    for (i = 0; i < 3; ++i) {
        ui_layout_region_t region = i == 0 ? UI_LAYOUT_REGION_LEFT_SIDEBAR : i == 1 ? UI_LAYOUT_REGION_RIGHT_SIDEBAR : UI_LAYOUT_REGION_BOTTOM;
        (void)ui_host_get_rect(shell->host, region, &side_rects[i]);
        if(i==2)states[i]=UI_SIDEBAR_VISIBLE;else (void)ui_host_get_sidebar_state(shell->host, region, &states[i]);
    }
    for (i = 0; i < shell->panel_count; ++i) {
        ui_native_panel_t *panel = &shell->panels[i];
        size_t side = panel->dock_region == UI_LAYOUT_REGION_LEFT_SIDEBAR ? 0 : panel->dock_region == UI_LAYOUT_REGION_BOTTOM ? 2 : 1;
        if (panel->kind == UI_PANEL_SIDEBAR && !panel->manual_floating && states[side] == UI_SIDEBAR_VISIBLE)
            ++dock_count[side];
    }
    for (i = 0; i < shell->panel_count && status == UI_STATUS_OK; ++i) {
        ui_native_panel_t *panel = &shell->panels[i];
        ui_content_slot_t *slot = panel->slot;
        size_t side = panel->dock_region == UI_LAYOUT_REGION_LEFT_SIDEBAR ? 0 : panel->dock_region == UI_LAYOUT_REGION_BOTTOM ? 2 : 1;
        int floating = panel->kind == UI_PANEL_FLOATING || panel->manual_floating ||
                       states[side] == UI_SIDEBAR_FLOATING;
        status = web_set_floating(shell, panel, floating);
        if (status != UI_STATUS_OK) break;
        slot->floating = floating;
        slot->visible = shell->active && !panel->closed && panel_tab_visible(panel) && shell->host->host_rect.width > 0 &&
            shell->host->host_rect.height > 0 && (floating || states[side] == UI_SIDEBAR_VISIBLE);
        if (floating) {
            if (!panel->positioned) {
                if (shell->layout_enabled) position_panel(shell, panel, panel->popup);
                else {
                POINT position = {logical_to_pixels(shell->host->host_rect.width - 340, dpi),
                                  logical_to_pixels(48 + (int)i * 24, dpi)};
                ClientToScreen(shell->parent, &position);
                (void)SetWindowPos(panel->popup, NULL, position.x, position.y,
                    logical_to_pixels(panel->preferred_width > 0 ? panel->preferred_width : 320, panel->floating_dpi),
                    logical_to_pixels(240, panel->floating_dpi), SWP_NOACTIVATE | SWP_NOZORDER);
                panel->positioned = 1;
                }
            }
            update_popup(shell, panel);
            ShowWindow(panel->popup, slot->visible ? SW_SHOWNOACTIVATE : SW_HIDE);
        } else {
            RECT pixels;
            slot->frame_rect = side_rects[side];
            if (shell->layout_enabled) slot->frame_rect = panel_dock_rect(shell,panel,side_rects[side]);
            else if (dock_count[side] > 0) {
                int height = side_rects[side].height / (int)dock_count[side];
                slot->frame_rect.y += height * (int)dock_index[side];
                slot->frame_rect.height = ++dock_index[side] == dock_count[side]
                    ? side_rects[side].height - height * (int)(dock_index[side] - 1) : height;
            }
            slot->rect = slot->frame_rect;
            if (states[side] == UI_SIDEBAR_VISIBLE) {
                int title = slot->rect.height < UI_WEB_PANEL_TITLE_HEIGHT ? slot->rect.height : UI_WEB_PANEL_TITLE_HEIGHT;
                slot->rect.y += title; slot->rect.height -= title;
            } else slot->rect.width = slot->rect.height = 0;
            if(shell->layout_enabled){if(side==2){slot->rect.y+=5;slot->rect.height-=slot->rect.height>5?5:slot->rect.height;}else{if(side==1)slot->rect.x+=5;slot->rect.width-=slot->rect.width>5?5:slot->rect.width;}}
            if (panel->collapsed) slot->rect.height = 0;
            pixels = logical_rect_to_pixels(&slot->rect, dpi);
            slot->pixel_rect = (ui_rect_t){pixels.left, pixels.top,
                pixels.right - pixels.left, pixels.bottom - pixels.top};
            (void)MoveWindow(panel->hwnd, pixels.left, pixels.top,
                             pixels.right - pixels.left, pixels.bottom - pixels.top, TRUE);
        }
        ShowWindow(panel->hwnd, slot->visible && !panel->collapsed ? SW_SHOW : SW_HIDE);
        if (status == UI_STATUS_OK) status = apply_slot(slot);
    }
    shell->reflowing = 0;
    return status;
}

static ui_status_t offscreen_reflow(ui_native_shell_t *shell)
{
    ui_content_slot_t *slot;ui_status_t status=UI_STATUS_OK;
    for(slot=shell->slots;slot;slot=slot->next){ui_panel_entry_t *entry=NULL;RECT pixels;
        ui_native_panel_t *panel=slot->panel_id?find_panel(shell,slot->panel_id):NULL;
        if(slot->panel_id)for(entry=shell->host->panels;entry&&strcmp(entry->id,slot->panel_id);entry=entry->next){}
        if(!slot->panel_id)(void)ui_host_get_rect(shell->host,UI_LAYOUT_REGION_MAIN,&slot->rect);
        else if(!entry)return UI_STATUS_NOT_FOUND;
        else if(panel&&shell->layout_enabled){
            if(panel->kind==UI_PANEL_FLOATING||panel->manual_floating)slot->rect=(ui_rect_t){0,0,panel->saved_float.width?panel->saved_float.width:320,panel->saved_float.height?panel->saved_float.height:240};
            else{(void)ui_host_get_rect(shell->host,panel->dock_region,&slot->rect);slot->rect=panel_dock_rect(shell,panel,slot->rect);}}
        else if(entry->kind==UI_PANEL_FLOATING)slot->rect=(ui_rect_t){0,0,entry->preferred_width>0?entry->preferred_width:320,240};
        else{size_t count=0,index=0;ui_panel_entry_t *p;ui_rect_t side={0};
            for(p=shell->host->panels;p;p=p->next)if(p->dock_region==entry->dock_region&&p->kind==entry->kind){if(!strcmp(p->id,entry->id))index=count;++count;}
            (void)ui_host_get_rect(shell->host,entry->dock_region,&side);slot->rect=side;
            if(count){slot->rect.y+=side.height*(int)index/(int)count;slot->rect.height=side.height*(int)(index+1)/(int)count-side.height*(int)index/(int)count;}}
        slot->frame_rect=slot->rect;slot->floating=panel?(panel->kind==UI_PANEL_FLOATING||panel->manual_floating):entry&&entry->kind==UI_PANEL_FLOATING;
        if(panel&&panel->collapsed)slot->rect.height=0;
        slot->visible=shell->active&&(!panel||(!panel->closed&&panel_tab_visible(panel)))&&slot->rect.width>0&&slot->rect.height>0;
        pixels=logical_rect_to_pixels(&slot->rect,shell->host->dpi);slot->pixel_rect=(ui_rect_t){pixels.left,pixels.top,pixels.right-pixels.left,pixels.bottom-pixels.top};
        if((slot->web_view||slot->surface)&&(status=apply_slot(slot))!=UI_STATUS_OK)return status;
    }ui_components_layout(shell->host);return status;
}

ui_status_t ui_native_shell_reflow(ui_native_shell_t *shell)
{
    ui_rect_t logical;
    RECT pixel;
    ui_rect_t sidebar_rects[3];
    RECT sidebar_pixels[3];
    ui_sidebar_state_t states[3];
    uint32_t dpi = 96u;
    size_t index;
    size_t counts[3] = {0, 0, 0};
    size_t dock_counts[3] = {0, 0, 0};
    size_t tag_indices[3] = {0, 0, 0};
    size_t dock_indices[3] = {0, 0, 0};

    if(shell)prepare_layout(shell);
    if(shell&&shell->offscreen)return offscreen_reflow(shell);
    if (shell == NULL || shell->host == NULL || shell->parent == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    if (shell->web_chrome) {ui_status_t status=web_shell_reflow(shell);ui_components_layout(shell->host);return status;}
    (void)ui_host_get_dpi(shell->host, &dpi);
    if (!update_font(shell, dpi)) return UI_STATUS_PLATFORM_ERROR;

    if (shell->toolbar != NULL &&
        ui_host_get_rect(shell->host, UI_LAYOUT_REGION_TOOLBAR,
                         &logical) == UI_STATUS_OK) {
        pixel = logical_rect_to_pixels(&logical, dpi);
        (void)MoveWindow(shell->toolbar,
                         pixel.left, pixel.top,
                         pixel.right - pixel.left,
                         pixel.bottom - pixel.top,
                         TRUE);
        ShowWindow(shell->toolbar,
                   shell->active && logical.width > 0 && logical.height > 0 ? SW_SHOW : SW_HIDE);
    }

    for (index = 0; index < 3; ++index) {
        ui_layout_region_t region = index == 0 ? UI_LAYOUT_REGION_LEFT_SIDEBAR : index == 1 ? UI_LAYOUT_REGION_RIGHT_SIDEBAR : UI_LAYOUT_REGION_BOTTOM;
        if (ui_host_get_rect(shell->host, region, &sidebar_rects[index]) != UI_STATUS_OK ||
            (index<2 && ui_host_get_sidebar_state(shell->host, region, &states[index]) != UI_STATUS_OK)) {
            return UI_STATUS_PLATFORM_ERROR;
        }
        if(index==2)states[index]=UI_SIDEBAR_VISIBLE;
        sidebar_pixels[index] = logical_rect_to_pixels(&sidebar_rects[index], dpi);
    }
    for (index = 0; index < shell->panel_count; ++index) {
        ui_native_panel_t *panel = &shell->panels[index];
        if (panel->kind == UI_PANEL_SIDEBAR) {
            size_t side = panel->dock_region == UI_LAYOUT_REGION_LEFT_SIDEBAR ? 0 : panel->dock_region == UI_LAYOUT_REGION_BOTTOM ? 2 : 1;
            ++counts[side];
            if (!panel->manual_floating && states[side] == UI_SIDEBAR_VISIBLE)
                ++dock_counts[side];
        }
    }
    for (index = 0u; index < shell->panel_count; ++index) {
        ui_native_panel_t *panel = &shell->panels[index];
        size_t side = panel->dock_region == UI_LAYOUT_REGION_LEFT_SIDEBAR ? 0 : panel->dock_region == UI_LAYOUT_REGION_BOTTOM ? 2 : 1;
        int floating = panel->kind == UI_PANEL_FLOATING || panel->manual_floating ||
                        states[side] == UI_SIDEBAR_FLOATING;
        set_floating(shell, panel, floating);
        if (panel->tag != NULL) {
            RECT tag_rect = sidebar_pixels[side];
            int height = (tag_rect.bottom - tag_rect.top) / (int)counts[side];
            tag_rect.top += height * (int)tag_indices[side];
            tag_rect.bottom = ++tag_indices[side] == counts[side]
                                ? sidebar_pixels[side].bottom : tag_rect.top + height;
            (void)MoveWindow(panel->tag, tag_rect.left, tag_rect.top,
                             tag_rect.right - tag_rect.left, tag_rect.bottom - tag_rect.top, TRUE);
            ShowWindow(panel->tag, shell->active && states[side] == UI_SIDEBAR_COLLAPSED &&
                         sidebar_rects[side].width > 0 && sidebar_rects[side].height > 0
                             ? SW_SHOW : SW_HIDE);
        }
        if (floating) {
            if (!panel->positioned) {
                if (shell->layout_enabled) position_panel(shell,panel,panel->hwnd);
                else {
                POINT position;
                ui_rect_t root;
                (void)ui_host_get_rect(shell->host, UI_LAYOUT_REGION_ROOT, &root);
                position.x = logical_to_pixels(root.width - 340, dpi);
                position.y = logical_to_pixels(48 + (int)index * 24, dpi);
                (void)ClientToScreen(shell->parent, &position);
                (void)SetWindowPos(panel->hwnd, NULL, position.x, position.y,
                                   logical_to_pixels(panel->preferred_width > 0 ? panel->preferred_width : 320, panel->floating_dpi),
                                   logical_to_pixels(240, panel->floating_dpi), SWP_NOACTIVATE | SWP_NOZORDER);
                panel->positioned = 1;
                }
            }
            ShowWindow(panel->hwnd, shell->active && !panel->closed && panel_tab_visible(panel) && shell->host->host_rect.width > 0 &&
                         shell->host->host_rect.height > 0 ? SW_SHOWNOACTIVATE : SW_HIDE);
        } else if (states[side] == UI_SIDEBAR_VISIBLE && dock_counts[side] > 0) {
            RECT dock_rect = sidebar_pixels[side];
            int height = (dock_rect.bottom - dock_rect.top) / (int)dock_counts[side];
            dock_rect.top += height * (int)dock_indices[side];
            dock_rect.bottom = ++dock_indices[side] == dock_counts[side]
                                 ? sidebar_pixels[side].bottom : dock_rect.top + height;
            if(shell->layout_enabled){ui_rect_t r=panel_dock_rect(shell,panel,sidebar_rects[side]);dock_rect=logical_rect_to_pixels(&r,dpi);}
            if(panel->tab_group&&panel->tag){size_t j,count=0,ordinal=0,active=0;int tab_width;
                for(j=0;j<shell->panel_count;++j)if(shell->panels[j].tab_group==panel->tab_group){if(j<index)++ordinal;if(shell->panels[j].tab_active)active=count;++count;}
                tab_width=(dock_rect.right-dock_rect.left)/4;MoveWindow(panel->tag,dock_rect.left+(int)(ordinal%4)*tab_width,dock_rect.top,tab_width,logical_to_pixels(28,dpi),TRUE);
                ShowWindow(panel->tag,shell->active&&ordinal/4==active/4?SW_SHOW:SW_HIDE);SetWindowPos(panel->tag,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
            }else if(panel->collapsed&&panel->tag){MoveWindow(panel->tag,dock_rect.left,dock_rect.top,dock_rect.right-dock_rect.left,logical_to_pixels(28,dpi),TRUE);ShowWindow(panel->tag,shell->active&&!panel->closed?SW_SHOW:SW_HIDE);}

            if(panel->tab_group){int title=logical_to_pixels(28,dpi);dock_rect.top+=dock_rect.bottom-dock_rect.top<title?dock_rect.bottom-dock_rect.top:title;}
            (void)MoveWindow(panel->hwnd, dock_rect.left, dock_rect.top,
                             dock_rect.right - dock_rect.left, dock_rect.bottom - dock_rect.top, TRUE);
            ShowWindow(panel->hwnd, shell->active && !panel->collapsed && !panel->closed && panel_tab_visible(panel) && sidebar_rects[side].width > 0 &&
                         sidebar_rects[side].height > 0 ? SW_SHOW : SW_HIDE);
        } else {
            ShowWindow(panel->hwnd, SW_HIDE);
        }
    }
    {
        ui_content_slot_t *slot;
        for (slot = shell->slots; slot != NULL; slot = slot->next) {
            update_native_slot(slot);
            if ((slot->surface != NULL || slot->web_view != NULL) && apply_slot(slot) != UI_STATUS_OK)
                return UI_STATUS_PLATFORM_ERROR;
        }
    }
    return UI_STATUS_OK;
}

static const char *find_command_binding(const ui_native_shell_t *shell,
                                        UINT native_id)
{
    size_t index;

    for (index = 0u; index < shell->binding_count; ++index) {
        if (shell->bindings[index].native_id == native_id) {
            return shell->bindings[index].command_id;
        }
    }
    return NULL;
}

static ui_native_shell_t *create_shell(
    const ui_native_shell_config_t *config, int web_chrome)
{
    ui_native_shell_t *shell;
    HWND parent;
    HWND menu_owner;
    uint32_t flags = 0u;

    if (config == NULL || config->host == NULL ||
        config->size < offsetof(ui_native_shell_config_t, menu_owner)) {
        return NULL;
    }
    parent = (HWND)(config->native_parent != NULL
                        ? config->native_parent
                        : config->host->native_parent);
    menu_owner = parent;
    if (config->size >= offsetof(ui_native_shell_config_t, menu_owner) + sizeof(config->menu_owner) &&
        config->menu_owner != NULL) menu_owner = (HWND)config->menu_owner;
    if (config->size >= offsetof(ui_native_shell_config_t, flags) + sizeof(config->flags))
        flags = config->flags;
    if ((web_chrome!=2&&(parent==NULL||!IsWindow(parent)||!IsWindow(menu_owner)||
         (GetWindowLongPtrW(menu_owner,GWL_STYLE)&WS_CHILD)!=0))||
        (web_chrome==2&&(parent||config->host->native_parent))||
        (flags & ~UI_NATIVE_SHELL_MANAGED_ACTIVATION) != 0u) {
        return NULL;
    }
    shell = (ui_native_shell_t *)calloc(1u, sizeof(*shell));
    if (shell == NULL) {
        return NULL;
    }
    shell->host = config->host;
    shell->shell.native = shell;
    shell->web_chrome = web_chrome!=0;
    shell->offscreen = web_chrome==2;
    shell->parent = parent;
    shell->menu_owner = menu_owner;
    shell->managed_activation = (flags & UI_NATIVE_SHELL_MANAGED_ACTIVATION) != 0u;
    shell->active = !shell->managed_activation;
    shell->previous_menu = shell->managed_activation || web_chrome ? NULL : GetMenu(menu_owner);
    if (create_slot(shell, NULL) == NULL) { free(shell); return NULL; }
    if (shell->managed_activation&&parent) ShowWindow(parent, SW_HIDE);
    if (ui_native_shell_refresh(shell) != UI_STATUS_OK) {
        ui_native_shell_destroy(shell);
        return NULL;
    }
    shell->host->shell = &shell->shell;
    shell->default_layout = shell->host->layout;
    if(parent&&!SetWindowSubclass(parent,layout_subclass,2,(DWORD_PTR)shell)){ui_native_shell_destroy(shell);return NULL;}
    return shell;
}

ui_native_shell_t *ui_native_shell_create(const ui_native_shell_config_t *config)
{
    return create_shell(config, 0);
}

ui_native_shell_t *ui_native_shell_create_offscreen(const ui_native_shell_config_t *config)
{return create_shell(config,2);}

ui_native_shell_t *ui_native_shell_create_web(const ui_native_shell_config_t *config)
{
#ifdef UI_FRAMEWORK_ENABLE_LIGHT_WEB
    return create_shell(config, 1);
#else
    (void)config;
    return NULL;
#endif
}

void ui_native_shell_destroy(ui_native_shell_t *shell)
{
    ui_content_slot_t *slot;
    if (shell == NULL) {
        return;
    }
    if(shell->drag_active)(void)ui_shell_end_panel_drag(&shell->shell,0);
    if(shell->split_active)end_splitter(shell,1);
    if(shell->parent)RemoveWindowSubclass(shell->parent,layout_subclass,2);
    free(shell->split_snapshot);
    (void)ui_host_close_menu(shell->host);
    if (shell->host->shell == &shell->shell) shell->host->shell = NULL;
    free_panels(shell);
    if (shell->toolbar != NULL) {
        DestroyWindow(shell->toolbar);
    }
    if (shell->menu != NULL) {
        if (!shell->web_chrome && GetMenu(shell->menu_owner) == shell->menu) {
            (void)SetMenu(shell->menu_owner, shell->previous_menu);
            (void)DrawMenuBar(shell->menu_owner);
        }
        DestroyMenu(shell->menu);
    }
    free_bindings(shell);
    if (shell->font != NULL) DeleteObject(shell->font);
    while ((slot = shell->slots) != NULL) {
        shell->slots = slot->next;
        detach_slot(slot);
        free(slot->panel_id);
        free(slot);
    }
    free(shell);
}

ui_status_t ui_native_shell_set_active(ui_native_shell_t *shell, int active)
{
    HMENU replacement;
    if (shell == NULL) return UI_STATUS_INVALID_ARGUMENT;
    active = active != 0;
    if(!active){if(shell->drag_active)(void)ui_shell_end_panel_drag(&shell->shell,0);if(shell->split_active)end_splitter(shell,0);}
    if (!shell->web_chrome && (active || GetMenu(shell->menu_owner) == shell->menu)) {
        replacement = active ? shell->menu : shell->previous_menu;
        if (!SetMenu(shell->menu_owner, replacement)) return UI_STATUS_PLATFORM_ERROR;
        (void)DrawMenuBar(shell->menu_owner);
    }
    shell->active = active;
    ui_components_active(shell->host, active);
    if (shell->managed_activation&&shell->parent) ShowWindow(shell->parent, active ? SW_SHOWNOACTIVATE : SW_HIDE);
    return ui_native_shell_reflow(shell);
}

void *ui_native_shell_menu_handle(ui_native_shell_t *shell)
{
    return shell != NULL ? (void *)shell->menu : NULL;
}

void *ui_native_shell_panel_handle(ui_native_shell_t *shell, const char *panel_id)
{
    ui_native_panel_t *panel;
    if (shell == NULL || panel_id == NULL) return NULL;
    panel = find_panel(shell, panel_id);
    return panel != NULL ? (void *)panel->hwnd : NULL;
}

ui_status_t ui_native_shell_set_panel_floating(ui_native_shell_t *shell,
                                               const char *panel_id,
                                               int floating)
{
    ui_native_panel_t *panel;
    HWND focus;
    ui_status_t status;
    if (shell == NULL || panel_id == NULL) return UI_STATUS_INVALID_ARGUMENT;
    if (shell->offscreen) return UI_STATUS_UNSUPPORTED;
    panel = find_panel(shell, panel_id);
    if (panel == NULL) return UI_STATUS_NOT_FOUND;
    if (panel->kind != UI_PANEL_SIDEBAR) return UI_STATUS_UNSUPPORTED;
    focus = GetFocus();
    if (focus != panel->hwnd && !IsChild(panel->hwnd, focus)) focus = NULL;
    panel->manual_floating = floating != 0;if(floating){panel->tab_group=0;panel->tab_active=0;}
    panel->closed = 0;
    status = ui_native_shell_reflow(shell);
    if (status == UI_STATUS_OK && shell->active && !shell->host->modal_component &&
        focus && IsWindowVisible(focus) && IsWindowEnabled(focus)) SetFocus(focus);
    return status;
}

ui_status_t ui_native_shell_refresh(ui_native_shell_t *shell)
{
    ui_status_t status;

    if(shell&&shell->offscreen)return ui_shell_refresh(&shell->shell);
    if (shell == NULL || shell->host == NULL || shell->parent == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }

    free_bindings(shell);
    status = refresh_menu(shell);
    if (status != UI_STATUS_OK) {
        return status;
    }
    status = refresh_toolbar(shell);
    if (status != UI_STATUS_OK) {
        return status;
    }
    status = refresh_panels(shell);
    if (status != UI_STATUS_OK) {
        return status;
    }
    return ui_native_shell_reflow(shell);
}

ui_status_t ui_native_shell_handle_message(ui_native_shell_t *shell,
                                            void *hwnd_value,
                                            uint32_t message,
                                            uintptr_t w_param,
                                            intptr_t l_param,
                                            intptr_t *result)
{
    HWND hwnd = (HWND)hwnd_value;
    const char *command_id;
    size_t index;

    if (shell == NULL || hwnd == NULL ||
        (hwnd != shell->parent && hwnd != shell->menu_owner)) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    if (result != NULL) {
        *result = 0;
    }
    if (shell->managed_activation && !shell->active) return UI_STATUS_NOT_FOUND;

    if (message == WM_SIZE || message == WM_DPICHANGED) {
        if (hwnd != shell->parent) return UI_STATUS_NOT_FOUND;
        ui_status_t status = ui_host_handle_message(shell->host,
                                                    hwnd_value,
                                                    message,
                                                    w_param,
                                                    l_param,
                                                    result);
        if (status != UI_STATUS_OK) {
            return status;
        }
        return ui_native_shell_reflow(shell);
    }
    if (message != WM_COMMAND) {
        return UI_STATUS_NOT_FOUND;
    }

    if (HIWORD(w_param) == BN_CLICKED && l_param != 0) {
        for (index = 0; index < shell->panel_count; ++index) {
            if (shell->panels[index].tag == (HWND)l_param) {
                if(shell->panels[index].tab_group&&!shell->panels[index].tab_active)return ui_shell_activate_panel(&shell->shell,shell->panels[index].id);
                if(shell->panels[index].tab_group){size_t j;for(j=1;j<=shell->panel_count;++j){ui_native_panel_t *next=&shell->panels[(index+j)%shell->panel_count];if(next->tab_group==shell->panels[index].tab_group)return ui_shell_activate_panel(&shell->shell,next->id);}}
                if(shell->panels[index].collapsed)shell->panels[index].collapsed=0;else shell->panels[index].manual_floating = 1;
                shell->panels[index].closed = 0;
                return ui_native_shell_reflow(shell);
            }
        }
    }

    command_id = find_command_binding(shell, (UINT)LOWORD(w_param));
    if (command_id == NULL) {
        return UI_STATUS_NOT_FOUND;
    }
    (void)ui_host_invoke(shell->host, command_id, "{}", "native");
    return UI_STATUS_OK;
}

ui_shell_t *ui_host_get_shell(ui_host_t *host)
{
    return host != NULL ? host->shell : NULL;
}

ui_content_slot_t *ui_shell_get_content_slot(ui_shell_t *shell,
                                             const char *panel_id)
{
    if (shell == NULL || shell->native == NULL) return NULL;
    return find_slot(shell->native, panel_id);
}

void *ui_content_slot_native_handle(const ui_content_slot_t *slot)
{
    ui_native_shell_t *shell;
    ui_native_panel_t *panel;
    if (slot == NULL || slot->shell == NULL) return NULL;
    shell = slot->shell->native;
    if (slot->panel_id == NULL) return shell->parent;
    panel = find_panel(shell, slot->panel_id);
    return panel != NULL ? panel->hwnd : NULL;
}

static void update_native_slot(ui_content_slot_t *slot)
{
    ui_native_shell_t *shell = slot->shell->native;
    ui_native_panel_t *panel=NULL;
    RECT pixels;
    uint32_t dpi = shell->host->dpi;
    if (shell->web_chrome) return;
    if (slot->panel_id == NULL) {
        (void)ui_host_get_rect(shell->host, UI_LAYOUT_REGION_MAIN, &slot->rect);
        pixels = logical_rect_to_pixels(&slot->rect, dpi);
        slot->pixel_rect = (ui_rect_t){pixels.left, pixels.top,
            pixels.right - pixels.left, pixels.bottom - pixels.top};
        slot->floating = 0;
        slot->visible = shell->active && slot->rect.width > 0 && slot->rect.height > 0;
    } else {
        panel = find_panel(shell, slot->panel_id);
        if (panel == NULL || !GetClientRect(panel->hwnd, &pixels)) {
            slot->rect = (ui_rect_t){0, 0, 0, 0};
            slot->pixel_rect = slot->rect; slot->visible = 0;
            return;
        }
        slot->floating = panel->floating;
        if (panel->floating) dpi = panel->floating_dpi != 0 ? panel->floating_dpi : dpi;
        else MapWindowPoints(panel->hwnd, shell->parent, (POINT *)&pixels, 2);
        slot->pixel_rect = (ui_rect_t){pixels.left, pixels.top,
            pixels.right - pixels.left, pixels.bottom - pixels.top};
        slot->rect = (ui_rect_t){MulDiv(pixels.left, 96, (int)dpi),
            MulDiv(pixels.top, 96, (int)dpi),
            MulDiv(pixels.right - pixels.left, 96, (int)dpi),
            MulDiv(pixels.bottom - pixels.top, 96, (int)dpi)};
        slot->visible = shell->active && (GetWindowLongPtrW(panel->hwnd, GWL_STYLE) & WS_VISIBLE) != 0;
    }
    slot->frame_rect = slot->rect;
    if(panel&&panel->tab_group&&!panel->floating){slot->frame_rect.y-=28;slot->frame_rect.height+=28;}
}

ui_status_t ui_content_slot_get_rect(const ui_content_slot_t *slot,
                                      ui_rect_t *rect)
{
    if (slot == NULL || rect == NULL) return UI_STATUS_INVALID_ARGUMENT;
    update_native_slot((ui_content_slot_t *)slot);
    *rect = slot->rect;
    return UI_STATUS_OK;
}

ui_status_t ui_content_slot_get_pixel_rect(const ui_content_slot_t *slot,
                                            ui_rect_t *rect)
{
    if (slot == NULL || rect == NULL) return UI_STATUS_INVALID_ARGUMENT;
    update_native_slot((ui_content_slot_t *)slot);
    *rect = slot->pixel_rect;
    return UI_STATUS_OK;
}

ui_status_t ui_shell_get_slot_state(ui_shell_t *shell, const char *panel_id,
                                    ui_rect_t *frame_rect, ui_rect_t *content_rect,
                                    int *visible, int *floating)
{
    ui_content_slot_t *slot = ui_shell_get_content_slot(shell, panel_id);
    if (slot == NULL) return shell == NULL ? UI_STATUS_INVALID_ARGUMENT : UI_STATUS_NOT_FOUND;
    update_native_slot(slot);
    if (frame_rect != NULL) *frame_rect = slot->frame_rect;
    if (content_rect != NULL) *content_rect = slot->rect;
    if (visible != NULL) *visible = slot->visible;
    if (floating != NULL) *floating = slot->floating;
    return UI_STATUS_OK;
}

ui_status_t ui_content_slot_attach_surface(ui_content_slot_t *slot,
                                            ui_surface_t *surface)
{
    ui_native_shell_t *shell;
    ui_status_t status;
    ui_content_slot_t *other;
    if (slot == NULL) return UI_STATUS_INVALID_ARGUMENT;
    shell = slot->shell->native;
    if (surface != NULL && surface->host != shell->host) return UI_STATUS_INVALID_ARGUMENT;
    if (surface != NULL && slot->web_view != NULL) return UI_STATUS_ALREADY_EXISTS;
    for (other = shell->slots; surface != NULL && other != NULL; other = other->next)
        if (other != slot && other->surface == surface) return UI_STATUS_ALREADY_EXISTS;
    if (slot->surface != NULL && slot->surface != surface) {
        HWND old_window = (HWND)ui_surface_native_handle(slot->surface);
        if (old_window != NULL && slot->panel_id != NULL) (void)SetParent(old_window, shell->parent);
    }
    slot->surface = surface;
    if (surface == NULL) return UI_STATUS_OK;
    (void)ui_surface_set_layout_region(surface, UI_LAYOUT_REGION_NONE);
    if (slot->panel_id != NULL&&!shell->offscreen) {
        HWND parent = (HWND)ui_content_slot_native_handle(slot);
        HWND window = (HWND)ui_surface_native_handle(surface);
        if (parent == NULL || window == NULL) { slot->surface = NULL; return UI_STATUS_PLATFORM_ERROR; }
        SetLastError(0);
        if (SetParent(window, parent) == NULL && GetLastError() != 0) {
            slot->surface = NULL; return UI_STATUS_PLATFORM_ERROR;
        }
    }
    update_native_slot(slot);
    status = apply_slot(slot);
    if (status != UI_STATUS_OK) detach_slot(slot);
    return status;
}

ui_status_t ui_content_slot_attach_web_view(ui_content_slot_t *slot,
                                             ui_web_view_t *view)
{
    ui_content_slot_t *other;
    if (slot == NULL) return UI_STATUS_INVALID_ARGUMENT;
    if (view != NULL && view->host != slot->shell->native->host) return UI_STATUS_INVALID_ARGUMENT;
    if (view != NULL && slot->surface != NULL) return UI_STATUS_ALREADY_EXISTS;
    for (other = slot->shell->native->slots; view != NULL && other != NULL; other = other->next)
        if (other != slot && other->web_view == view) return UI_STATUS_ALREADY_EXISTS;
    slot->web_view = view;
    if (view == NULL) return UI_STATUS_OK;
    (void)ui_web_view_set_layout_region(view, UI_LAYOUT_REGION_NONE);
    update_native_slot(slot);
    return apply_slot(slot);
}

ui_status_t ui_shell_refresh(ui_shell_t *shell)
{
    ui_native_shell_t *native;
    ui_panel_entry_t *entry;
    ui_status_t status;
    if (shell == NULL || shell->native == NULL) return UI_STATUS_INVALID_ARGUMENT;
    native = shell->native;
    if(native->offscreen){for(entry=native->host->panels;entry;entry=entry->next)if(!find_panel(native,entry->id)&&append_panel(native,entry)!=UI_STATUS_OK)return UI_STATUS_OUT_OF_MEMORY;
        return offscreen_reflow(native);}
    free_bindings(native);
    status = refresh_menu(native);
    if (status != UI_STATUS_OK) return status;
    status = refresh_toolbar(native);
    if (status != UI_STATUS_OK) return status;
    for (entry = native->host->panels; entry != NULL; entry = entry->next) {
        if (find_panel(native, entry->id) == NULL) {
            status = append_panel(native, entry);
            if (status != UI_STATUS_OK) return status;
        }
    }
    status = ui_native_shell_reflow(native);
    if (status == UI_STATUS_OK && native->web_chrome)
        (void)ui_host_emit_event(native->host, "ui.shell.changed", "{}");
    return status;
}

void ui_shell_layout_changed(ui_host_t *host)
{
    if(host&&host->shell)clamp_floating_windows(host->shell->native);
    if(host&&host->shell&&host->shell->native->offscreen){(void)offscreen_reflow(host->shell->native);return;}
    if (host != NULL && host->shell != NULL && host->shell->native->web_chrome)
        (void)web_shell_reflow(host->shell->native);
    if (host != NULL) ui_components_layout(host);
}

int ui_shell_surface_bound(ui_host_t *host, const ui_surface_t *surface)
{
    ui_content_slot_t *slot;
    if (host == NULL || host->shell == NULL || !host->shell->native->web_chrome) return 0;
    for (slot = host->shell->native->slots; slot != NULL; slot = slot->next)
        if (slot->surface == surface) return 1;
    return 0;
}

int ui_shell_web_view_bound(ui_host_t *host, const ui_web_view_t *view)
{
    ui_content_slot_t *slot;
    if (host == NULL || host->shell == NULL || !host->shell->native->web_chrome) return 0;
    for (slot = host->shell->native->slots; slot != NULL; slot = slot->next)
        if (slot->web_view == view) return 1;
    return 0;
}

uint32_t ui_shell_surface_dpi(ui_host_t *host, const ui_surface_t *surface)
{
    ui_content_slot_t *slot;
    if (host == NULL) return 96u;
    if (host->shell == NULL || !host->shell->native->web_chrome) return host->dpi;
    for (slot = host->shell->native->slots; slot != NULL; slot = slot->next) {
        if (slot->surface == surface && slot->floating && slot->panel_id != NULL) {
            ui_native_panel_t *panel = find_panel(host->shell->native, slot->panel_id);
            if (panel != NULL && panel->floating_dpi != 0) return panel->floating_dpi;
        }
    }
    return host->dpi;
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
        if (slot->web_view == view) slot->web_view = NULL;
}

int ui_content_slot_belongs_to(const ui_content_slot_t *slot,const ui_host_t *host)
{return slot&&slot->shell&&slot->shell->native->host==host;}

#include "shell_layout.inc"
