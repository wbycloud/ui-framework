#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <wchar.h>

enum { ID_OPEN=0x1001, ID_CLOSE, ID_EXIT, ID_NEXT, ID_PREVIOUS,
       ID_TARGET=0x1101, ID_COMMAND, ID_INVOKE, ID_CANCEL, ID_SNAPSHOT,
       ID_ASSISTANT_TOGGLE, ID_PARAMS, ID_BEGIN, ID_COMMIT, ID_ROLLBACK, ID_UNDO };

typedef struct frontend {
    PROCESS_INFORMATION process;
    HWND root, tabs, target, command, params, log;
    void *remote;
} frontend_t;
typedef struct window_query {
    DWORD process;
    const wchar_t *class_name;
    int id;
    HWND found;
} window_query_t;

static int failures;
static void check(int value, const char *message)
{
    if (!value) { fprintf(stderr, "FAIL: %s (Windows error %lu)\n", message, (unsigned long)GetLastError()); ++failures; }
}
static LRESULT send(HWND hwnd, UINT message, WPARAM wp, LPARAM lp)
{
    DWORD_PTR result = 0;
    if (!SendMessageTimeoutW(hwnd, message, wp, lp, SMTO_ABORTIFHUNG, 5000, &result)) {
        check(0, "frontend control message timed out");
        return 0;
    }
    return (LRESULT)result;
}
static BOOL CALLBACK find_window(HWND hwnd, LPARAM argument)
{
    window_query_t *query = (window_query_t *)argument;
    wchar_t class_name[128];
    DWORD process;
    GetWindowThreadProcessId(hwnd, &process);
    if (query->process && process != query->process) return TRUE;
    if (query->id && GetDlgCtrlID(hwnd) != query->id) return TRUE;
    if (query->class_name) {
        GetClassNameW(hwnd, class_name, 128);
        if (wcscmp(class_name, query->class_name)) return TRUE;
    }
    query->found = hwnd;
    return FALSE;
}
static HWND child(HWND parent, const wchar_t *class_name, int id)
{
    window_query_t query = {0};
    query.class_name = class_name; query.id = id;
    EnumChildWindows(parent, find_window, (LPARAM)&query);
    return query.found;
}
static BOOL CALLBACK find_log(HWND hwnd, LPARAM argument)
{
    wchar_t class_name[64];
    GetClassNameW(hwnd, class_name, 64);
    if (!wcscmp(class_name, L"Edit") && (GetWindowLongPtrW(hwnd, GWL_STYLE) & ES_AUTOVSCROLL)) {
        *(HWND *)argument = hwnd;
        return FALSE;
    }
    return TRUE;
}
static uint64_t tab_id(frontend_t *test, int index)
{
    TCITEMW item;
    SIZE_T copied;
    ZeroMemory(&item, sizeof(item)); item.mask = TCIF_PARAM;
    if (!WriteProcessMemory(test->process.hProcess, test->remote, &item, sizeof(item), &copied)) return 0;
    if (!send(test->tabs, TCM_GETITEMW, (WPARAM)index, (LPARAM)test->remote)) return 0;
    if (!ReadProcessMemory(test->process.hProcess, test->remote, &item, sizeof(item), &copied)) return 0;
    return (uint64_t)item.lParam;
}
static void select_tab(frontend_t *test, int index)
{
    RECT rect;
    SIZE_T copied;
    ZeroMemory(&rect, sizeof(rect));
    check(WriteProcessMemory(test->process.hProcess, test->remote, &rect, sizeof(rect), &copied), "initialize remote tab rectangle");
    check(send(test->tabs, TCM_GETITEMRECT, (WPARAM)index, (LPARAM)test->remote) != 0, "query actual tab rectangle");
    check(ReadProcessMemory(test->process.hProcess, test->remote, &rect, sizeof(rect), &copied), "read tab rectangle");
    /* WM_NOTIFY cannot cross process boundaries. The native tab control emits
     * its own TCN_SELCHANGE when this real click changes the selection. */
    send(test->tabs, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM((rect.left+rect.right)/2, (rect.top+rect.bottom)/2));
    send(test->tabs, WM_LBUTTONUP, 0, MAKELPARAM((rect.left+rect.right)/2, (rect.top+rect.bottom)/2));
}
static void select_target(frontend_t *test, int index)
{
    send(test->target, CB_SETCURSEL, (WPARAM)index, 0);
    send(GetParent(test->target), WM_COMMAND, MAKEWPARAM(ID_TARGET, CBN_SELCHANGE), (LPARAM)test->target);
}
static void select_command(frontend_t *test, const wchar_t *name)
{
    LRESULT index = send(test->command, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)name);
    check(index != CB_ERR, "assistant command exists");
    send(test->command, CB_SETCURSEL, (WPARAM)index, 0);
    send(GetParent(test->command), WM_COMMAND, MAKEWPARAM(ID_COMMAND, CBN_SELCHANGE), (LPARAM)test->command);
}
static void get_text(HWND hwnd, wchar_t *text, size_t capacity)
{
    text[0] = 0;
    send(hwnd, WM_GETTEXT, (WPARAM)capacity, (LPARAM)text);
}
static UINT menu_command(HMENU menu, const wchar_t *title)
{
    int count = GetMenuItemCount(menu), i;
    for (i = 0; i < count; ++i) {
        wchar_t text[256];
        HMENU submenu = GetSubMenu(menu, i);
        UINT id;
        if (submenu) { id = menu_command(submenu, title); if (id) return id; }
        GetMenuStringW(menu, (UINT)i, text, 256, MF_BYPOSITION);
        if (wcsstr(text, title)) {
            id = GetMenuItemID(menu, i);
            if (id != (UINT)-1) return id;
        }
    }
    return 0;
}
static HWND properties(frontend_t *test, uint64_t id)
{
    HWND last = NULL, parent;
    wchar_t text[256];
    /* EDA properties windows are children of distinct native sidebar panels. */
    for (parent = FindWindowExW(test->root, NULL, L"UiFrameworkApplicationContainerV1", NULL);
         parent; parent = FindWindowExW(test->root, parent, L"UiFrameworkApplicationContainerV1", NULL)) {
        last = child(parent, L"UiFramework.MinimalEda.Properties.v1", 0);
        if (last) {
            unsigned long long found = 0;
            get_text(GetDlgItem(last, 1), text, 256);
            if (swscanf(text, L"Instance %llu", &found) == 1 && found == id) return last;
        }
    }
    return NULL;
}
static unsigned blocks(frontend_t *test, uint64_t id)
{
    HWND panel = properties(test, id);
    wchar_t text[256], *position;
    unsigned count = (unsigned)-1;
    check(panel != NULL, "EDA instance property panel remains alive");
    if (!panel) return count;
    get_text(GetDlgItem(panel, 1), text, 256);
    position = wcsstr(text, L"Blocks:");
    if (position) (void)swscanf(position, L"Blocks: %u", &count);
    return count;
}
static HWND container_of(HWND root, HWND hwnd)
{
    while (hwnd && GetParent(hwnd) != root) hwnd = GetParent(hwnd);
    return hwnd;
}
static int start(frontend_t *test, const wchar_t *exe, const wchar_t *package,
                   const wchar_t *cpp_package, int count)
{
    STARTUPINFOW startup;
    wchar_t command[32768];
    ULONGLONG deadline;
    window_query_t query = {0};
    ZeroMemory(test, sizeof(*test)); ZeroMemory(&startup, sizeof(startup));
    startup.cb = sizeof(startup); startup.dwFlags = STARTF_USESHOWWINDOW; startup.wShowWindow = SW_HIDE;
    if (count == 0) (void)swprintf(command, 32768, L"\"%ls\"", exe);
    else if (count == 3) (void)swprintf(command, 32768, L"\"%ls\" \"%ls\" \"%ls\" \"%ls\"", exe, package, cpp_package, cpp_package);
    else (void)swprintf(command, 32768, L"\"%ls\" \"%ls\" \"%ls\"", exe, package, package);
    if (!CreateProcessW(exe, command, NULL, NULL, FALSE, 0, NULL, NULL, &startup, &test->process)) {
        check(0, "start standalone host"); return 0;
    }
    query.process = test->process.dwProcessId; query.class_name = L"UiFrameworkStandaloneHostV1";
    deadline = GetTickCount64() + 15000;
    while (GetTickCount64() < deadline && WaitForSingleObject(test->process.hProcess, 0) == WAIT_TIMEOUT) {
        EnumWindows(find_window, (LPARAM)&query);
        if (query.found) {
            HWND tabs = child(query.found, WC_TABCONTROLW, 0);
            if (tabs && send(tabs, TCM_GETITEMCOUNT, 0, 0) == count) { test->root = query.found; test->tabs = tabs; break; }
        }
        Sleep(20);
    }
    check(test->root != NULL, "standalone host opens requested tab count");
    if (!test->root) return 0;
    test->remote = VirtualAllocEx(test->process.hProcess, NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    check(test->remote != NULL, "allocate cross-process control buffer");
    test->target = child(test->root, L"ComboBox", ID_TARGET);
    test->command = child(test->root, L"ComboBox", ID_COMMAND);
    test->params = child(test->root, L"Edit", ID_PARAMS);
    EnumChildWindows(GetParent(test->target), find_log, (LPARAM)&test->log);
    check(test->target && test->command && test->params && test->log, "global assistant controls exist");
    return test->remote && test->target && test->command && test->params && test->log;
}
static void stop(frontend_t *test)
{
    DWORD result = (DWORD)-1;
    if (test->root) PostMessageW(test->root, WM_CLOSE, 0, 0);
    if (test->process.hProcess) {
        DWORD waited = WaitForSingleObject(test->process.hProcess, 10000);
        check(waited == WAIT_OBJECT_0, "host exits after normal WM_CLOSE");
        if (waited != WAIT_OBJECT_0) TerminateProcess(test->process.hProcess, 2);
        else { GetExitCodeProcess(test->process.hProcess, &result); check(result == 0, "host exits successfully"); }
        if (test->remote) VirtualFreeEx(test->process.hProcess, test->remote, 0, MEM_RELEASE);
        CloseHandle(test->process.hThread); CloseHandle(test->process.hProcess);
    }
}
static void wait_tabs(frontend_t *test, int count)
{
    ULONGLONG deadline = GetTickCount64() + 5000;
    while (GetTickCount64() < deadline && send(test->tabs, TCM_GETITEMCOUNT, 0, 0) != count) Sleep(10);
    check(send(test->tabs, TCM_GETITEMCOUNT, 0, 0) == count, "tab close completes in frontend");
}
static void decline_confirmation(frontend_t *test)
{
    window_query_t query = {0};
    ULONGLONG deadline = GetTickCount64() + 5000;
    query.process = test->process.dwProcessId; query.class_name = L"#32770";
    check(PostMessageW(child(test->root, L"Button", ID_INVOKE), BM_CLICK, 0, 0), "start command requiring confirmation");
    while (GetTickCount64() < deadline) {
        EnumWindows(find_window, (LPARAM)&query);
        if (query.found) break;
        Sleep(10);
    }
    check(query.found != NULL, "destructive assistant command asks for confirmation");
    if (query.found) {
        ShowWindow(query.found, SW_HIDE);
        send(GetDlgItem(query.found, IDNO), BM_CLICK, 0, 0);
        send(test->root, WM_NULL, 0, 0);
    }
}
static void resize_matrix(frontend_t *test, HWND canvas)
{
    static const int sizes[][2] = {{1920,1080},{1280,720},{800,600},{640,480}};
    size_t i;
    for (i = 0; i < sizeof(sizes) / sizeof(sizes[0]); ++i) {
        RECT outside = {0, 0, sizes[i][0], sizes[i][1]}, client, tabs, panel, content;
        UINT dpi = GetDpiForWindow(test->root);
        if (!dpi) dpi = 96;
        check(AdjustWindowRectExForDpi(&outside, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, TRUE, 0, dpi), "calculate client-size resize");
        check(SetWindowPos(test->root, NULL, 20, 20, outside.right-outside.left, outside.bottom-outside.top,
                           SWP_NOACTIVATE | SWP_NOZORDER), "resize frontend window");
        send(test->root, WM_NULL, 0, 0);
        GetClientRect(test->root, &client); GetWindowRect(test->tabs, &tabs);
        GetWindowRect(GetParent(test->target), &panel); GetWindowRect(canvas, &content);
        check(client.right == sizes[i][0] && client.bottom == sizes[i][1], "requested client dimensions applied");
        check(tabs.right-tabs.left >= 0 && content.right-content.left > 0 && content.bottom-content.top > 0,
              "tabs and OpenGL main area retain usable geometry");
        if (MulDiv(client.right, 96, (int)dpi) < 800) {
            check(panel.right-panel.left == MulDiv(32, (int)dpi, 96), "assistant collapses to a DPI-scaled rail at narrow width");
            check((GetWindowLongPtrW(test->target, GWL_STYLE) & WS_VISIBLE) == 0, "narrow window hides assistant target content");
        } else check((GetWindowLongPtrW(test->target, GWL_STYLE) & WS_VISIBLE) != 0, "wide window restores assistant controls");
    }
    ShowWindow(test->root, SW_MINIMIZE); ShowWindow(test->root, SW_RESTORE); ShowWindow(test->root, SW_HIDE);
    send(test->root, WM_NULL, 0, 0);
    check(IsWindow(canvas), "EDA canvas survives minimize and restore");
}
static void dpi_matrix(frontend_t *test, HWND canvas)
{
    static const UINT dpis[] = {96, 144, 192};
    size_t i;
    for (i = 0; i < sizeof(dpis) / sizeof(dpis[0]); ++i) {
        RECT suggested, tabs, pixels;
        wchar_t log[32768], expected[128];
        UINT dpi = dpis[i];
        GetWindowRect(test->root, &suggested);
        send(test->root, WM_DPICHANGED, MAKEWPARAM(dpi, dpi), (LPARAM)&suggested);
        GetWindowRect(test->tabs, &tabs); GetWindowRect(canvas, &pixels);
        check(tabs.bottom-tabs.top == MulDiv(34, (int)dpi, 96), "WM_DPICHANGED rescales standalone tab geometry");
        check(pixels.right-pixels.left > 0 && pixels.bottom-pixels.top > 0, "DPI change preserves nonempty framebuffer");
        send(test->log, WM_SETTEXT, 0, (LPARAM)L"");
        send(child(test->root, L"Button", ID_SNAPSHOT), BM_CLICK, 0, 0);
        get_text(test->log, log, 32768);
        (void)swprintf(expected, 128, L"\"framebuffer_width\":%ld,\"framebuffer_height\":%ld",
                       pixels.right-pixels.left, pixels.bottom-pixels.top);
        check(wcsstr(log, expected) != NULL, "EDA reports actual framebuffer pixels after DPI change");
    }
}

static void click_action(frontend_t *test, int id)
{
    send(child(test->root, L"Button", id), BM_CLICK, 0, 0);
}
static void cpp_state(frontend_t *test, unsigned expected_count, int expected_open)
{
    wchar_t log[32768], *count_text, *transaction_text;
    unsigned count = (unsigned)-1;
    unsigned long long transaction = (unsigned long long)-1;
    send(test->log, WM_SETTEXT, 0, (LPARAM)L"");
    click_action(test, ID_SNAPSHOT);
    get_text(test->log, log, 32768);
    count_text = wcsstr(log, L"\"count\":"); transaction_text = wcsstr(log, L"\"transaction_id\":");
    if (count_text) (void)swscanf(count_text, L"\"count\":%u", &count);
    if (transaction_text) (void)swscanf(transaction_text, L"\"transaction_id\":%llu", &transaction);
    check(count == expected_count, "C++ transaction snapshot has expected count");
    check(transaction != (unsigned long long)-1 && (transaction != 0) == expected_open,
          "C++ transaction snapshot has expected open/closed state");
}
static void transaction_frontend(const wchar_t *exe, const wchar_t *eda_package,
                                   const wchar_t *cpp_package)
{
    frontend_t test;
    uint64_t first_cpp, second_cpp;
    if (!start(&test, exe, eda_package, cpp_package, 3)) { stop(&test); return; }
    first_cpp = tab_id(&test, 1); second_cpp = tab_id(&test, 2);
    check(first_cpp && second_cpp && first_cpp != second_cpp, "frontend loads two independent C++ transaction instances");
    select_target(&test, 1); select_command(&test, L"fixture.cpp.count");
    cpp_state(&test, 0, 0);
    click_action(&test, ID_BEGIN); cpp_state(&test, 0, 1);
    click_action(&test, ID_INVOKE); cpp_state(&test, 1, 1);
    click_action(&test, ID_COMMIT); cpp_state(&test, 1, 0);
    /* A committed transaction must no longer block another Begin. */
    click_action(&test, ID_BEGIN); cpp_state(&test, 1, 1);
    click_action(&test, ID_INVOKE); cpp_state(&test, 2, 1);
    click_action(&test, ID_ROLLBACK); cpp_state(&test, 1, 0);
    click_action(&test, ID_UNDO); cpp_state(&test, 0, 0);
    click_action(&test, ID_BEGIN); click_action(&test, ID_INVOKE);
    click_action(&test, ID_COMMIT); cpp_state(&test, 1, 0);
    click_action(&test, ID_UNDO); cpp_state(&test, 0, 0);
    click_action(&test, ID_BEGIN); cpp_state(&test, 0, 1);
    select_tab(&test, 1); send(test.root, WM_COMMAND, ID_CLOSE, 0); wait_tabs(&test, 2);
    check(tab_id(&test, 1) == second_cpp, "closing transaction owner retains the other C++ instance");
    select_target(&test, 1); select_command(&test, L"fixture.cpp.count");
    cpp_state(&test, 0, 0);
    /* A closed owner cannot leave a stale global transaction lock. */
    click_action(&test, ID_BEGIN); cpp_state(&test, 0, 1);
    click_action(&test, ID_INVOKE); click_action(&test, ID_COMMIT); cpp_state(&test, 1, 0);
    click_action(&test, ID_UNDO); cpp_state(&test, 0, 0);
    stop(&test);
}

int wmain(int argc, wchar_t **argv)
{
    frontend_t test;
    wchar_t exe[32768], package[32768], cpp_package[32768], log[32768], note[256];
    uint64_t first, second;
    HWND first_properties, second_properties, first_canvas, second_canvas, note_edit, toolbar;
    HWND first_container, second_container;
    UINT add_id;
    TBBUTTON button;
    SIZE_T copied;
    if (argc != 3 && argc != 4) { fprintf(stderr, "Usage: host_frontend framework_host.exe minimal_eda.uapp [cpp_fixture.uapp]\n"); return 2; }
    if (!GetFullPathNameW(argv[1], 32768, exe, NULL) || !GetFullPathNameW(argv[2], 32768, package, NULL)) return 2;
    if (argc == 4 && !GetFullPathNameW(argv[3], 32768, cpp_package, NULL)) return 2;
    (void)SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    if (start(&test, exe, package, NULL, 0)) {
        check(GetMenu(test.root) != NULL && GetMenuState(GetMenu(test.root), ID_OPEN, MF_BYCOMMAND) != (UINT)-1,
              "empty host exposes common Open application menu");
        check(send(test.target, CB_GETCOUNT, 0, 0) == 0, "empty host has no assistant targets");
    }
    stop(&test);
    if (!start(&test, exe, package, NULL, 2)) { stop(&test); return 1; }
    first = tab_id(&test, 0); second = tab_id(&test, 1);
    check(first != 0 && second != 0 && first != second, "tab items retain distinct immutable instance IDs");
    check(send(test.tabs, TCM_GETCURSEL, 0, 0) == 1, "last opened application is active");
    first_properties = properties(&test, first); second_properties = properties(&test, second);
    first_container = container_of(test.root, first_properties); second_container = container_of(test.root, second_properties);
    first_canvas = child(first_container, L"UiFrameworkSurface", 0); second_canvas = child(second_container, L"UiFrameworkSurface", 0);
    check(first_canvas && second_canvas && first_canvas != second_canvas, "both instances own distinct native OpenGL canvases");
    check(blocks(&test, first) == 0 && blocks(&test, second) == 0, "EDA instances start with separate empty models");
    add_id = menu_command(GetMenu(test.root), L"Add block");
    check(add_id != 0 && GetMenuState(GetMenu(test.root), ID_OPEN, MF_BYCOMMAND) != (UINT)-1,
          "active app menu includes app commands and framework commands");
    send(test.root, WM_COMMAND, add_id, 0);
    check(blocks(&test, second) == 1 && blocks(&test, first) == 0, "menu command reaches the active instance only");
    toolbar = child(second_container, TOOLBARCLASSNAMEW, 0);
    check(toolbar != NULL, "active application toolbar exists");
    ZeroMemory(&button, sizeof(button));
    check(WriteProcessMemory(test.process.hProcess, test.remote, &button, sizeof(button), &copied), "initialize remote toolbar query");
    check(send(toolbar, TB_GETBUTTON, 0, (LPARAM)test.remote) != 0, "query actual native toolbar command");
    check(ReadProcessMemory(test.process.hProcess, test.remote, &button, sizeof(button), &copied), "read actual toolbar binding");
    send(second_container, WM_COMMAND, MAKEWPARAM(button.idCommand, 0), (LPARAM)toolbar);
    check(blocks(&test, second) == 2 && blocks(&test, first) == 0, "toolbar command reaches its active application");
    select_target(&test, 0); select_command(&test, L"eda.add_block");
    send(test.params, WM_SETTEXT, 0, (LPARAM)L"{}");
    send(child(test.root, L"Button", ID_INVOKE), BM_CLICK, 0, 0);
    check(blocks(&test, first) == 1 && blocks(&test, second) == 2, "global assistant mutates explicitly selected background instance");
    get_text(test.log, log, 32768);
    { wchar_t expected[128]; (void)swprintf(expected, 128, L"[#%llu / request", (unsigned long long)first);
      check(wcsstr(log, expected) && wcsstr(log, L"eda.add_block: OK"), "assistant result identifies original target instance"); }
    check(!IsWindowEnabled(child(test.root, L"Button", ID_CANCEL)), "synchronous assistant reply does not leave cancellation enabled");
    send(child(test.root, L"Button", ID_SNAPSHOT), BM_CLICK, 0, 0); get_text(test.log, log, 32768);
    check(wcsstr(log, L"\"blocks\":1") && wcsstr(log, L"\"active\":false"), "snapshot reads the selected background instance");
    select_command(&test, L"eda.clear"); decline_confirmation(&test);
    check(blocks(&test, first) == 1, "declining permission confirmation prevents destructive assistant clear");
    select_command(&test, L"eda.add_block"); send(test.params, WM_SETTEXT, 0, (LPARAM)L"{\"unexpected\":1}");
    send(child(test.root, L"Button", ID_INVOKE), BM_CLICK, 0, 0);
    check(blocks(&test, first) == 1, "application parameter validation prevents invalid assistant input");
    send(test.params, WM_SETTEXT, 0, (LPARAM)L"{}");
    note_edit = GetDlgItem(first_properties, 2); send(note_edit, WM_SETTEXT, 0, (LPARAM)L"note persists across tabs");
    select_tab(&test, 0);
    check(send(test.tabs, TCM_GETCURSEL, 0, 0) == 0, "TCN_SELCHANGE switches the active application");
    send(test.root, WM_COMMAND, menu_command(GetMenu(test.root), L"Add block"), 0);
    check(blocks(&test, first) == 2 && blocks(&test, second) == 2, "switched menu operates first instance");
    send(test.root, WM_COMMAND, ID_NEXT, 0); check(send(test.tabs, TCM_GETCURSEL, 0, 0) == 1, "next-tab action selects second instance");
    send(test.root, WM_COMMAND, ID_PREVIOUS, 0); check(send(test.tabs, TCM_GETCURSEL, 0, 0) == 0, "previous-tab action restores first instance");
    get_text(note_edit, note, 256);
    check(!wcscmp(note, L"note persists across tabs") && IsWindow(first_canvas) && IsWindow(second_canvas),
          "tab switching preserves editable content and native OpenGL window identities");
    dpi_matrix(&test, first_canvas);
    /* Restore the OS DPI before physical client-size checks. */
    { RECT current; UINT dpi = GetDpiForWindow(test.root); if (!dpi) dpi = 96;
      GetWindowRect(test.root, &current); send(test.root, WM_DPICHANGED, MAKEWPARAM(dpi,dpi), (LPARAM)&current); }
    resize_matrix(&test, first_canvas);
    send(test.root, WM_COMMAND, ID_CLOSE, 0); wait_tabs(&test, 1);
    check(!IsWindow(first_canvas) && IsWindow(second_canvas), "closing one tab removes only its canvas");
    check(GetMenuState(GetMenu(test.root), ID_OPEN, MF_BYCOMMAND) != (UINT)-1 && menu_command(GetMenu(test.root), L"Add block"),
          "remaining tab retains both common and application menu");
    send(test.root, WM_COMMAND, menu_command(GetMenu(test.root), L"Add block"), 0);
    check(blocks(&test, second) == 3, "remaining app still accepts menu commands after close");
    send(test.root, WM_COMMAND, ID_CLOSE, 0); wait_tabs(&test, 0);
    check(GetMenuState(GetMenu(test.root), ID_OPEN, MF_BYCOMMAND) != (UINT)-1 && !menu_command(GetMenu(test.root), L"Add block"),
          "last close restores empty framework menu");
    check(send(test.target, CB_GETCOUNT, 0, 0) == 0 && !IsWindow(second_canvas), "last close clears assistant targets and application canvas");
    stop(&test);
    if (argc == 4) transaction_frontend(exe, package, cpp_package);
    printf("standalone host frontend: %d failures\n", failures);
    return failures != 0;
}
