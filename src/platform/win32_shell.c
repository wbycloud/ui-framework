#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0603

#include <windows.h>
#include <commctrl.h>

#include <stdint.h>
#include <limits.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>

#include "../ui_internal.h"
#include "ui_framework/native.h"

#pragma comment(lib, "comctl32.lib")

#define UI_NATIVE_FIRST_COMMAND 0x5000u

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
    uint32_t floating_dpi;
    HFONT floating_font;
} ui_native_panel_t;

struct ui_native_shell {
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
};

static HFONT create_font_for_dpi(uint32_t dpi);

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
    if (message == WM_CLOSE || message == WM_DPICHANGED) {
        for (index = 0; index < shell->panel_count; ++index) {
            ui_native_panel_t *panel = &shell->panels[index];
            if (panel->hwnd == hwnd && message == WM_CLOSE) {
                shell->panels[index].closed = 1;
                ShowWindow(hwnd, SW_HIDE);
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
        if (shell->panels[index].hwnd != NULL) {
            DestroyWindow(shell->panels[index].hwnd);
        }
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

    if (shell->active) {
        if (!SetMenu(shell->menu_owner, menu)) {
            DestroyMenu(menu);
            return UI_STATUS_PLATFORM_ERROR;
        }
        (void)DrawMenuBar(shell->menu_owner);
    } else if (shell->menu != NULL && GetMenu(shell->menu_owner) == shell->menu) {
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

static ui_status_t refresh_panels(ui_native_shell_t *shell)
{
    ui_panel_entry_t *entry;

    free_panels(shell);
    for (entry = shell->host->panels; entry != NULL; entry = entry->next) {
        wchar_t *title;
        HWND panel_window;
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
        title = utf8_to_wide(entry->title);
        if (title == NULL) {
            return UI_STATUS_OUT_OF_MEMORY;
        }
        if (entry->kind == UI_PANEL_FLOATING) {
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
                                           title,
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
        if (entry->kind == UI_PANEL_SIDEBAR) {
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
        panel = &shell->panels[shell->panel_count];
        memset(panel, 0, sizeof(*panel));
        panel->hwnd = panel_window;
        panel->tag = tag;
        panel->id = duplicate_string(entry->id);
        panel->kind = entry->kind;
        panel->dock_region = entry->dock_region;
        panel->preferred_width = entry->preferred_width;
        panel->floating = entry->kind == UI_PANEL_FLOATING;
        if (panel->id == NULL || !SetWindowSubclass(panel_window, panel_subclass_proc,
                                                    1, (DWORD_PTR)shell)) {
            ui_status_t error = panel->id == NULL ? UI_STATUS_OUT_OF_MEMORY : UI_STATUS_PLATFORM_ERROR;
            DestroyWindow(panel_window);
            if (tag != NULL) DestroyWindow(tag);
            free(panel->id);
            return error;
        }
        shell->panel_count += 1u;
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

ui_status_t ui_native_shell_reflow(ui_native_shell_t *shell)
{
    ui_rect_t logical;
    RECT pixel;
    ui_rect_t sidebar_rects[2];
    RECT sidebar_pixels[2];
    ui_sidebar_state_t states[2];
    uint32_t dpi = 96u;
    size_t index;
    size_t counts[2] = {0, 0};
    size_t dock_counts[2] = {0, 0};
    size_t tag_indices[2] = {0, 0};
    size_t dock_indices[2] = {0, 0};

    if (shell == NULL || shell->host == NULL || shell->parent == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
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

    for (index = 0; index < 2; ++index) {
        ui_layout_region_t region = index == 0 ? UI_LAYOUT_REGION_LEFT_SIDEBAR : UI_LAYOUT_REGION_RIGHT_SIDEBAR;
        if (ui_host_get_rect(shell->host, region, &sidebar_rects[index]) != UI_STATUS_OK ||
            ui_host_get_sidebar_state(shell->host, region, &states[index]) != UI_STATUS_OK) {
            return UI_STATUS_PLATFORM_ERROR;
        }
        sidebar_pixels[index] = logical_rect_to_pixels(&sidebar_rects[index], dpi);
    }
    for (index = 0; index < shell->panel_count; ++index) {
        ui_native_panel_t *panel = &shell->panels[index];
        if (panel->kind == UI_PANEL_SIDEBAR) {
            size_t side = panel->dock_region == UI_LAYOUT_REGION_LEFT_SIDEBAR ? 0 : 1;
            ++counts[side];
            if (!panel->manual_floating && states[side] == UI_SIDEBAR_VISIBLE)
                ++dock_counts[side];
        }
    }
    for (index = 0u; index < shell->panel_count; ++index) {
        ui_native_panel_t *panel = &shell->panels[index];
        size_t side = panel->dock_region == UI_LAYOUT_REGION_LEFT_SIDEBAR ? 0 : 1;
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
            ShowWindow(panel->hwnd, shell->active && !panel->closed && shell->host->host_rect.width > 0 &&
                         shell->host->host_rect.height > 0 ? SW_SHOWNOACTIVATE : SW_HIDE);
        } else if (states[side] == UI_SIDEBAR_VISIBLE && dock_counts[side] > 0) {
            RECT dock_rect = sidebar_pixels[side];
            int height = (dock_rect.bottom - dock_rect.top) / (int)dock_counts[side];
            dock_rect.top += height * (int)dock_indices[side];
            dock_rect.bottom = ++dock_indices[side] == dock_counts[side]
                                 ? sidebar_pixels[side].bottom : dock_rect.top + height;
            (void)MoveWindow(panel->hwnd, dock_rect.left, dock_rect.top,
                             dock_rect.right - dock_rect.left, dock_rect.bottom - dock_rect.top, TRUE);
            ShowWindow(panel->hwnd, shell->active && sidebar_rects[side].width > 0 &&
                         sidebar_rects[side].height > 0 ? SW_SHOW : SW_HIDE);
        } else {
            ShowWindow(panel->hwnd, SW_HIDE);
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

ui_native_shell_t *ui_native_shell_create(
    const ui_native_shell_config_t *config)
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
    if (parent == NULL || !IsWindow(parent) || !IsWindow(menu_owner) ||
        (GetWindowLongPtrW(menu_owner, GWL_STYLE) & WS_CHILD) != 0 ||
        (flags & ~UI_NATIVE_SHELL_MANAGED_ACTIVATION) != 0u) {
        return NULL;
    }
    shell = (ui_native_shell_t *)calloc(1u, sizeof(*shell));
    if (shell == NULL) {
        return NULL;
    }
    shell->host = config->host;
    shell->parent = parent;
    shell->menu_owner = menu_owner;
    shell->managed_activation = (flags & UI_NATIVE_SHELL_MANAGED_ACTIVATION) != 0u;
    shell->active = !shell->managed_activation;
    shell->previous_menu = shell->managed_activation ? NULL : GetMenu(menu_owner);
    if (shell->managed_activation) ShowWindow(parent, SW_HIDE);
    if (ui_native_shell_refresh(shell) != UI_STATUS_OK) {
        ui_native_shell_destroy(shell);
        return NULL;
    }
    return shell;
}

void ui_native_shell_destroy(ui_native_shell_t *shell)
{
    if (shell == NULL) {
        return;
    }
    free_panels(shell);
    if (shell->toolbar != NULL) {
        DestroyWindow(shell->toolbar);
    }
    if (shell->menu != NULL) {
        if (GetMenu(shell->menu_owner) == shell->menu) {
            (void)SetMenu(shell->menu_owner, shell->previous_menu);
            (void)DrawMenuBar(shell->menu_owner);
        }
        DestroyMenu(shell->menu);
    }
    free_bindings(shell);
    if (shell->font != NULL) DeleteObject(shell->font);
    free(shell);
}

ui_status_t ui_native_shell_set_active(ui_native_shell_t *shell, int active)
{
    HMENU replacement;
    if (shell == NULL) return UI_STATUS_INVALID_ARGUMENT;
    active = active != 0;
    if (active || GetMenu(shell->menu_owner) == shell->menu) {
        replacement = active ? shell->menu : shell->previous_menu;
        if (!SetMenu(shell->menu_owner, replacement)) return UI_STATUS_PLATFORM_ERROR;
        (void)DrawMenuBar(shell->menu_owner);
    }
    shell->active = active;
    if (shell->managed_activation) ShowWindow(shell->parent, active ? SW_SHOWNOACTIVATE : SW_HIDE);
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
    if (shell == NULL || panel_id == NULL) return UI_STATUS_INVALID_ARGUMENT;
    panel = find_panel(shell, panel_id);
    if (panel == NULL) return UI_STATUS_NOT_FOUND;
    if (panel->kind != UI_PANEL_SIDEBAR) return UI_STATUS_UNSUPPORTED;
    panel->manual_floating = floating != 0;
    panel->closed = 0;
    return ui_native_shell_reflow(shell);
}

ui_status_t ui_native_shell_refresh(ui_native_shell_t *shell)
{
    ui_status_t status;

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
                shell->panels[index].manual_floating = 1;
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
