#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <commctrl.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "ui_framework/application.h"
#include "ui_framework/package.h"

static int failures;
typedef struct observations {
    ui_workspace_t *workspace;
    unsigned results, events, progress, confirmations, unmounts;
    int approve, result_success;
    uint64_t last_instance, last_request, progress_instance, event_instance, confirm_instance;
    char result[512], event[128], progress_text[128], event_payload[128];
} observations_t;
typedef struct staging {
    wchar_t root[MAX_PATH], directory[MAX_PATH], module[MAX_PATH], resource[MAX_PATH];
    wchar_t manifest[8][MAX_PATH], packages[8][MAX_PATH];
    char paths[8][MAX_PATH * 3];
} staging_t;

static void check(int condition, const char *description)
{
    if (!condition) { fprintf(stderr, "FAIL: %s\n", description); ++failures; }
}
static int utf8(const wchar_t *wide, char *text, int capacity)
{ return WideCharToMultiByte(CP_UTF8, 0, wide, -1, text, capacity, NULL, NULL) != 0; }
static LRESULT CALLBACK root_proc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp)
{
    ui_workspace_t *w = (ui_workspace_t *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    if (w && message == WM_COMMAND) {
        intptr_t result;
        if (ui_workspace_handle_message(w, hwnd, message, wp, lp, &result) == UI_STATUS_OK)
            return (LRESULT)result;
    }
    return DefWindowProcW(hwnd, message, wp, lp);
}
static void result_received(uint64_t id, const ui_result_t *result, void *data)
{
    observations_t *o = data; ++o->results;
    o->last_instance = id; o->last_request = result->request_id;
    o->result_success = result->success;
    (void)snprintf(o->result, sizeof(o->result), "%s", result->result_json);
}
static void event_received(uint64_t id, const char *event, const char *json, void *data)
{
    observations_t *o = data; ++o->events; o->event_instance = id;
    if (!strcmp(event, "fixture.unmount")) ++o->unmounts;
    (void)snprintf(o->event, sizeof(o->event), "%s", event);
    (void)snprintf(o->event_payload, sizeof(o->event_payload), "%s", json);
}
static void progress_received(uint64_t id, uint64_t request, int percent,
    const char *message, void *data)
{
    observations_t *o = data; (void)request; (void)percent;
    ++o->progress; o->progress_instance = id;
    (void)snprintf(o->progress_text, sizeof(o->progress_text), "%s", message);
}
static int confirm(uint64_t id, const char *command, const char *json,
    ui_assistant_permission_t permission, void *data)
{
    observations_t *o = data;
    (void)command; (void)json; (void)permission;
    ++o->confirmations; o->confirm_instance = id; return o->approve;
}
static int get_info(ui_workspace_t *w, uint64_t id, ui_app_instance_info_t *info)
{
    memset(info, 0, sizeof(*info)); info->size = sizeof(*info);
    return ui_workspace_get_instance(w, id, info) == UI_STATUS_OK;
}
static int state_has(ui_workspace_t *w, uint64_t id, const char *token)
{
    ui_app_instance_info_t info; const char *snapshot;
    return get_info(w, id, &info) &&
        ui_assistant_get_state_snapshot(info.assistant, &snapshot) == UI_STATUS_OK &&
        strstr(snapshot, token) != NULL;
}
static HWND descendant(HWND parent, const wchar_t *class_name)
{
    HWND child;
    if (!parent) return NULL;
    for (child = GetWindow(parent, GW_CHILD); child; child = GetWindow(child, GW_HWNDNEXT)) {
        wchar_t name[128]; HWND found;
        GetClassNameW(child, name, 128);
        if (!_wcsicmp(name, class_name)) return child;
        found = descendant(child, class_name);
        if (found) return found;
    }
    return NULL;
}
static int module_path(HWND private_window, wchar_t *path)
{
    HMODULE dll = (HMODULE)GetWindowLongPtrW(private_window, GWLP_HINSTANCE);
    return dll && GetModuleFileNameW(dll, path, MAX_PATH) != 0;
}
static void assert_unloaded(const wchar_t *path)
{
    wchar_t directory[MAX_PATH], *separator;
    check(GetModuleHandleW(path) == NULL, "actual module unloaded after last instance closes");
    check(GetFileAttributesW(path) == INVALID_FILE_ATTRIBUTES, "extracted DLL deleted after unload");
    (void)wcscpy(directory, path); separator = wcsrchr(directory, L'\\');
    if (separator) { *separator = 0;
        check(GetFileAttributesW(directory) == INVALID_FILE_ATTRIBUTES, "module temporary directory removed"); }
}
static int invoke(ui_workspace_t *w, uint64_t id, const char *command, uint64_t *request)
{
    ui_status_t status = ui_workspace_invoke(w, id, command, "{}", request);
    if (status != UI_STATUS_OK)
        fprintf(stderr, "invoke %s: status %d (%s)\n", command, (int)status, ui_workspace_last_error(w));
    return status == UI_STATUS_OK;
}
static int open_app(ui_workspace_t *w, const char *path, uint64_t *id)
{
    ui_status_t status = ui_workspace_open(w, path, id);
    if (status != UI_STATUS_OK)
        fprintf(stderr, "open app: status %d (%s)\n", (int)status, ui_workspace_last_error(w));
    return status == UI_STATUS_OK;
}
static int stage_create(staging_t *s, const wchar_t *fixture)
{
    wchar_t temp[MAX_PATH]; FILE *out;
    memset(s, 0, sizeof(*s));
    if (!GetTempPathW(MAX_PATH, temp) || !GetTempFileNameW(temp, L"uwt", 0, s->root) ||
        !DeleteFileW(s->root) || !CreateDirectoryW(s->root, NULL)) return 0;
    (void)swprintf(s->directory, MAX_PATH, L"%ls\\stage", s->root);
    (void)swprintf(s->module, MAX_PATH, L"%ls\\fixture.dll", s->directory);
    (void)swprintf(s->resource, MAX_PATH, L"%ls\\fixture.txt", s->directory);
    if (!CreateDirectoryW(s->directory, NULL) || !CopyFileW(fixture, s->module, TRUE)) return 0;
    out = _wfopen(s->resource, L"wb"); if (!out) return 0;
    if (fwrite("fixture resource", 1, 16, out) != 16) { fclose(out); return 0; }
    return fclose(out) == 0;
}
static int pack(staging_t *s, size_t index, const char *app_id, const char *version, int multi)
{
    char manifest[MAX_PATH * 3], directory[MAX_PATH * 3], error[256]; FILE *out;
    (void)swprintf(s->manifest[index], MAX_PATH, L"%ls\\manifest%u.ini", s->root, (unsigned)index);
    (void)swprintf(s->packages[index], MAX_PATH, L"%ls\\app%u.uapp", s->root, (unsigned)index);
    if (!utf8(s->manifest[index], manifest, sizeof(manifest)) ||
        !utf8(s->directory, directory, sizeof(directory)) ||
        !utf8(s->packages[index], s->paths[index], sizeof(s->paths[index]))) return 0;
    out = _wfopen(s->manifest[index], L"wb"); if (!out) return 0;
    fprintf(out, "[application]\napp_id=%s\nname=Lifecycle fixture\nversion=%s\n"
        "architecture=x64\nabi_version=1\nframework_api_version=7\nmodule=fixture.dll\n"
        "multiple_instances=%s\n", app_id, version, multi ? "true" : "false");
    if (fclose(out)) return 0;
    if (ui_package_pack(manifest, directory, s->paths[index], error, sizeof(error)) != UI_STATUS_OK) {
        fprintf(stderr, "pack: %s\n", error); return 0; }
    return 1;
}
static int altered_copy(staging_t *s, size_t index, const char *before, const char *after)
{
    FILE *file; unsigned char *bytes; long length; size_t i, n = strlen(before); int found = 0;
    (void)swprintf(s->packages[index], MAX_PATH, L"%ls\\app%u.uapp", s->root, (unsigned)index);
    if (strlen(after) != n || !CopyFileW(s->packages[0], s->packages[index], TRUE) ||
        !utf8(s->packages[index], s->paths[index], sizeof(s->paths[index]))) return 0;
    file = _wfopen(s->packages[index], L"r+b"); if (!file) return 0;
    if (fseek(file, 0, SEEK_END) || (length = ftell(file)) < 0 || fseek(file, 0, SEEK_SET)) {
        fclose(file); return 0; }
    bytes = malloc((size_t)length); if (!bytes) { fclose(file); return 0; }
    if (fread(bytes, 1, (size_t)length, file) != (size_t)length) { free(bytes); fclose(file); return 0; }
    for (i = 0; i + n <= (size_t)length; ++i) if (!memcmp(bytes + i, before, n)) {
        found = 1; if (fseek(file, (long)i, SEEK_SET) || fwrite(after, 1, n, file) != n) found = 0;
        break;
    }
    free(bytes); if (fclose(file)) found = 0; return found;
}
static void stage_destroy(staging_t *s)
{
    size_t i;
    if (!s->root[0]) return;
    /* Delete only the explicitly created files; no recursive shell traversal. */
    for (i = 0; i < 8; ++i) {
        if (s->manifest[i][0]) (void)DeleteFileW(s->manifest[i]);
        if (s->packages[i][0]) (void)DeleteFileW(s->packages[i]);
    }
    if (s->resource[0]) (void)DeleteFileW(s->resource);
    if (s->module[0]) (void)DeleteFileW(s->module);
    if (s->directory[0]) (void)RemoveDirectoryW(s->directory);
    (void)RemoveDirectoryW(s->root);
}

typedef struct worker_post {
    ui_workspace_t *workspace;
    uint64_t id, request;
    int succeeded;
} worker_post_t;
static DWORD WINAPI worker(void *data)
{
    worker_post_t *p = data;
    char json[] = "{\"from_worker\":true}";
    char event[] = "worker.event", message[] = "worker progress";
    p->succeeded = ui_workspace_post_progress(p->workspace, p->id, p->request, 50, message) == UI_STATUS_OK &&
        ui_workspace_post_event(p->workspace, p->id, event, json) == UI_STATUS_OK &&
        ui_workspace_post_result(p->workspace, p->id, p->request, 1, json) == UI_STATUS_OK;
    memset(json, 'x', sizeof(json)); memset(event, 'x', sizeof(event)); memset(message, 'x', sizeof(message));
    return 0;
}
static void copied_delivery(ui_workspace_t *w, observations_t *o, uint64_t target, uint64_t foreground)
{
    worker_post_t p; HANDLE thread; unsigned results, events, progress;
    memset(&p, 0, sizeof(p)); p.workspace = w; p.id = target;
    check(invoke(w, target, "fixture.delayed", &p.request), "submit delayed semantic request");
    check(ui_workspace_activate(w, foreground) == UI_STATUS_OK, "switch foreground while request pending");
    results = o->results; events = o->events; progress = o->progress;
    thread = CreateThread(NULL, 0, worker, &p, 0, NULL);
    check(thread != NULL, "create actual posting worker");
    if (!thread) return;
    check(WaitForSingleObject(thread, 5000) == WAIT_OBJECT_0, "worker posting finishes");
    CloseHandle(thread); check(p.succeeded, "all worker posts accepted");
    ui_workspace_poll(w);
    check(o->results == results + 1 && o->last_instance == target && o->last_request == p.request &&
        !strcmp(o->result, "{\"from_worker\":true}"), "copied result reaches original background instance");
    check(o->events == events + 1 && o->event_instance == target && !strcmp(o->event, "worker.event"),
        "copied event reaches original instance");
    check(o->progress == progress + 1 && o->progress_instance == target &&
        !strcmp(o->progress_text, "worker progress"), "copied progress reaches original request");
}

static int failure_mode(ui_workspace_t *w, staging_t *stage, const wchar_t *report)
{
    ui_app_instance_info_t info; uint64_t id, request, duplicate; HWND private_window;
    wchar_t path[MAX_PATH]; char path_utf8[MAX_PATH * 3]; FILE *out;
    if (!pack(stage, 0, "org.ui.lifecycle.failure", "1.0.0", 1) || !open_app(w, stage->paths[0], &id) ||
        !get_info(w, id, &info)) { check(0, "load cleanup-failure fixture"); return 0; }
    private_window = descendant(info.native_container, L"UiFrameworkLifecycleFixtureV1");
    if (!module_path(private_window, path)) { check(0, "capture retained module path"); return 0; }
    check(invoke(w, id, "fixture.shutdown_fail", &request), "request fixture cleanup failure");
    check(ui_workspace_close(w, id, UI_APP_CLOSE_TAB) == UI_STATUS_OK, "close fixture before failed shutdown");
    ui_workspace_poll(w);
    check(ui_workspace_count(w) == 0 && GetModuleHandleW(path) != NULL &&
        GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES, "failed module cleanup retains DLL after instance teardown");
    check(strstr(ui_workspace_last_error(w), "cleanup failed") != NULL, "cleanup failure is reported");
    check(ui_workspace_open(w, stage->paths[0], &duplicate) == UI_STATUS_PLATFORM_ERROR &&
        ui_workspace_count(w) == 0, "retained failed module cannot reopen");
    check(ui_workspace_destroy(w) == UI_STATUS_PLATFORM_ERROR, "workspace stays alive when module cannot unload");
    if (!utf8(path, path_utf8, sizeof(path_utf8))) return 0;
    out = _wfopen(report, L"wb"); if (!out) return 0;
    fprintf(out, "%s", path_utf8); return fclose(out) == 0;
}

static void cleanup_failure_process(const wchar_t *fixture, const wchar_t *report)
{
    wchar_t exe[MAX_PATH], command[MAX_PATH * 4], path[MAX_PATH], temp[MAX_PATH], directory[MAX_PATH];
    char bytes[MAX_PATH * 3]; STARTUPINFOW startup; PROCESS_INFORMATION process;
    FILE *file; size_t length; DWORD exit_code = 1; wchar_t *separator;
    check(GetModuleFileNameW(NULL, exe, MAX_PATH) != 0, "locate integration executable");
    (void)swprintf(command, sizeof(command) / sizeof(command[0]),
        L"\"%ls\" --shutdown-failure \"%ls\" \"%ls\"", exe, fixture, report);
    memset(&startup, 0, sizeof(startup)); startup.cb = sizeof(startup);
    memset(&process, 0, sizeof(process));
    check(CreateProcessW(exe, command, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL,
        &startup, &process), "start isolated cleanup-failure validation");
    if (!process.hProcess) return;
    check(WaitForSingleObject(process.hProcess, 15000) == WAIT_OBJECT_0, "cleanup-failure validation exits");
    GetExitCodeProcess(process.hProcess, &exit_code);
    CloseHandle(process.hThread); CloseHandle(process.hProcess);
    check(exit_code == 0, "isolated cleanup-failure assertions pass");
    file = _wfopen(report, L"rb"); check(file != NULL, "child reports retained DLL for cleanup");
    if (!file) return;
    length = fread(bytes, 1, sizeof(bytes) - 1, file); fclose(file); bytes[length] = 0;
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, bytes, -1, path, MAX_PATH) ||
        !GetTempPathW(MAX_PATH, temp)) { check(0, "decode retained temporary path"); return; }
    (void)wcscpy(directory, path); separator = wcsrchr(directory, L'\\');
    /* Validate the reported absolute target beneath %TEMP%, with the expected
       workspace directory prefix and the exact file we packaged. */
    if (!separator || wcscmp(separator + 1, L"fixture.dll") ||
        _wcsnicmp(path, temp, wcslen(temp)) != 0 ||
        _wcsnicmp(path + wcslen(temp), L"uap", 3) != 0 || wcschr(path + wcslen(temp), L'/')) {
        check(0, "retained path remains inside known temporary extraction directory"); return;
    }
    *separator = 0;
    check(DeleteFileW(path) && RemoveDirectoryW(directory), "clean retained files after child process exits");
    (void)DeleteFileW(report);
}

int wmain(int argc, wchar_t **argv)
{
    static const int sizes[][2] = {{1920,1080}, {1280,720}, {800,600}, {640,480}};
    static const uint32_t dpis[] = {96,144,192};
    WNDCLASSW wc; HWND root = NULL, canvas[2], note[2], properties[2];
    HGLRC gl_context[2]; ui_workspace_t *w = NULL;
    ui_workspace_config_t config; ui_app_instance_info_t info[2], fixture_info;
    observations_t o; staging_t stage; ui_rect_t rect = {10,30,1000,700};
    char eda_path[MAX_PATH * 3]; wchar_t eda_dll[MAX_PATH], fixture_dll[MAX_PATH], report[MAX_PATH];
    uint64_t eda[2], fixtures[2], single, repeated, request, reopened, rejected;
    size_t i, j, k; intptr_t native_result; int failure_child;
    memset(&o, 0, sizeof(o)); memset(&stage, 0, sizeof(stage));
    failure_child = argc == 4 && !wcscmp(argv[1], L"--shutdown-failure");
    if ((!failure_child && argc != 3 && argc != 4) || (failure_child && argc != 4)) {
        fprintf(stderr, "usage: workspace_integration minimal_eda.uapp fixture.dll [cpp_fixture.dll]\n"); return 2; }
    check(ui_framework_initialize() == UI_STATUS_OK, "initialize PMv2 before first HWND");
    memset(&wc, 0, sizeof(wc)); wc.hInstance = GetModuleHandleW(NULL);
    wc.lpfnWndProc = root_proc; wc.lpszClassName = L"UiWorkspaceIntegrationV1";
    check(RegisterClassW(&wc) != 0, "register integration root");
    root = CreateWindowExW(0, wc.lpszClassName, L"Workspace integration", WS_OVERLAPPEDWINDOW,
        0, 0, 1280, 900, NULL, NULL, wc.hInstance, NULL);
    if (!root) { check(0, "create actual native root"); goto cleanup; }
    memset(&config, 0, sizeof(config)); config.size = sizeof(config); config.native_parent = root;
    config.user_data = &o; config.result = result_received; config.event = event_received;
    config.progress = progress_received; config.confirm = confirm;
    config.max_permission = UI_ASSISTANT_PERMISSION_EDIT;
    w = ui_workspace_create(&config); o.workspace = w;
    if (!w) { check(0, "create workspace"); goto cleanup; }
    SetWindowLongPtrW(root, GWLP_USERDATA, (LONG_PTR)w);
    check(ui_workspace_count(w) == 0 && ui_workspace_active(w) == 0, "standalone workspace starts empty");
    check(ui_workspace_set_rect(w, &rect, 96) == UI_STATUS_OK, "set workspace client viewport");
    if (!stage_create(&stage, argv[2])) { check(0, "stage actual fixture DLL and resource"); goto cleanup; }
    if (failure_child) {
        check(failure_mode(w, &stage, argv[3]), "write cleanup-failure report");
        /* Cleanup failure deliberately retains the valid workspace and DLL until
           process exit. The parent removes its recorded extraction afterwards. */
        w = NULL; SetWindowLongPtrW(root, GWLP_USERDATA, 0); goto cleanup;
    }
    if (!utf8(argv[1], eda_path, sizeof(eda_path)) ||
        !pack(&stage, 0, "org.ui.lifecycle", "1.0.0", 1) ||
        !pack(&stage, 1, "org.ui.lifecycle.single", "1.0.0", 0) ||
        !pack(&stage, 2, "org.ui.lifecycle", "2.0.0", 1)) {
        check(0, "create valid multi/single/version-conflict packages"); goto cleanup; }
    if (!open_app(w, eda_path, &eda[0]) || !open_app(w, eda_path, &eda[1])) {
        check(0, "load two real EDA application instances"); goto cleanup; }
    check(eda[0] != eda[1] && ui_workspace_count(w) == 2, "EDA package declares independent multiple instances");
    for (i = 0; i < 2; ++i) {
        check(get_info(w, eda[i], &info[i]), "query actual EDA instance");
        canvas[i] = descendant(info[i].native_container, L"UiFrameworkSurface");
        properties[i] = descendant(info[i].native_container, L"UiFramework.MinimalEda.Properties.v1");
        note[i] = descendant(properties[i], L"EDIT");
        if (!canvas[i] || !properties[i] || !note[i]) {
            fprintf(stderr, "EDA instance %llu: canvas=%p properties=%p note=%p container=%p\n",
                (unsigned long long)eda[i], (void *)canvas[i], (void *)properties[i],
                (void *)note[i], info[i].native_container);
            check(0, "EDA provides GL canvas and editable panel"); goto cleanup; }
        SetWindowTextW(note[i], i == 0 ? L"first instance note" : L"second instance note");
        check(ui_workspace_activate(w, eda[i]) == UI_STATUS_OK, "activate EDA tab");
        (void)SendMessageW(canvas[i], WM_PAINT, 0, 0);
        gl_context[i] = wglGetCurrentContext();
        check(gl_context[i] != NULL, "actual EDA paint uses a GL context");
    }
    check(module_path(properties[0], eda_dll), "capture extracted real EDA DLL path");
    check(gl_context[0] != gl_context[1], "EDA tabs have independent GL contexts");
    check(invoke(w, eda[0], "eda.add_block", &request) &&
        invoke(w, eda[1], "eda.add_block", &request) &&
        invoke(w, eda[1], "eda.add_block", &request), "invoke same semantic IDs for separate EDA instances");
    check(state_has(w, eda[0], "\"blocks\":1") && state_has(w, eda[1], "\"blocks\":2"),
        "EDA model mutations remain independent");
    for (i = 0; i < sizeof(dpis) / sizeof(dpis[0]); ++i) {
        for (j = 0; j < sizeof(sizes) / sizeof(sizes[0]); ++j) {
            rect.width = sizes[j][0]; rect.height = sizes[j][1];
            check(ui_workspace_set_rect(w, &rect, dpis[i]) == UI_STATUS_OK, "resize all tab hosts at current DPI");
            for (k = 0; k < 2; ++k) {
                ui_rect_t main; RECT actual; GLint viewport[4]; wchar_t text[64];
                check(ui_workspace_activate(w, eda[k]) == UI_STATUS_OK, "switch tab after resize and DPI");
                (void)SendMessageW(canvas[k], WM_PAINT, 0, 0);
                check(IsWindow(canvas[k]) && IsWindow(note[k]) &&
                    descendant(info[k].native_container, L"UiFrameworkSurface") == canvas[k] &&
                    wglGetCurrentContext() == gl_context[k], "tab activation preserves HWND and actual GL context");
                GetWindowTextW(note[k], text, 64);
                check(!wcscmp(text, k == 0 ? L"first instance note" : L"second instance note"), "tab edit content survives switching");
                check(ui_host_get_rect(info[k].host, UI_LAYOUT_REGION_MAIN, &main) == UI_STATUS_OK &&
                    GetWindowRect(canvas[k], &actual), "query real drawable and logical viewport");
                (void)MapWindowPoints(NULL, info[k].native_container, (POINT *)&actual, 2);
                check(actual.left == MulDiv(main.x, (int)dpis[i], 96) &&
                    actual.right == MulDiv(main.x + main.width, (int)dpis[i], 96) &&
                    actual.top == MulDiv(main.y, (int)dpis[i], 96) &&
                    actual.bottom == MulDiv(main.y + main.height, (int)dpis[i], 96),
                    "actual EDA framebuffer matches logical viewport at 96/144/192 DPI");
                glGetIntegerv(GL_VIEWPORT, viewport);
                check(viewport[2] == actual.right - actual.left && viewport[3] == actual.bottom - actual.top &&
                    glGetError() == GL_NO_ERROR, "EDA renders with correct actual viewport after switching");
            }
        }
    }
    rect.width = rect.height = 0;
    check(ui_workspace_set_rect(w, &rect, 192) == UI_STATUS_OK, "zero-size/minimized workspace");
    rect.width = 1000; rect.height = 700;
    check(ui_workspace_set_rect(w, &rect, 96) == UI_STATUS_OK, "restore minimized workspace");
    (void)SendMessageW(canvas[1], WM_PAINT, 0, 0);
    check(wglGetCurrentContext() == gl_context[1] && glGetError() == GL_NO_ERROR, "EDA continues rendering after restore");

    if (!open_app(w, stage.paths[0], &fixtures[0]) || !open_app(w, stage.paths[0], &fixtures[1]) ||
        !get_info(w, fixtures[0], &fixture_info)) { check(0, "load lifecycle fixtures"); goto cleanup; }
    check(module_path(descendant(fixture_info.native_container, L"UiFrameworkLifecycleFixtureV1"), fixture_dll),
        "capture exact fixture DLL path");
    check(invoke(w, fixtures[0], "fixture.add", &request) && state_has(w, fixtures[0], "\"count\":1") &&
        state_has(w, fixtures[1], "\"count\":0"), "same fixture IDs mutate only targeted instance");
    {
        ui_app_instance_info_t active; HWND toolbar, tag, sidebar; TBBUTTON button;
        check(get_info(w, fixtures[1], &active), "query foreground fixture controls");
        check(ui_workspace_handle_message(w, root, WM_COMMAND,
            GetMenuItemID(GetSubMenu(GetMenu(root), 0), 0), 0, &native_result) == UI_STATUS_OK &&
            state_has(w, fixtures[1], "\"count\":1"), "root menu dispatch invokes foreground fixture");
        toolbar = FindWindowExW(active.native_container, NULL, TOOLBARCLASSNAMEW, NULL);
        memset(&button, 0, sizeof(button));
        check(toolbar && SendMessageW(toolbar, TB_GETBUTTON, 0, (LPARAM)&button), "get actual toolbar command");
        (void)SendMessageW(active.native_container, WM_COMMAND, (WPARAM)button.idCommand, (LPARAM)toolbar);
        check(state_has(w, fixtures[1], "\"count\":2"), "actual child-container toolbar notification reaches active application");
        rect.width = 480;
        check(ui_workspace_set_rect(w, &rect, 96) == UI_STATUS_OK, "narrow workspace collapses app sidebar");
        tag = FindWindowExW(active.native_container, NULL, L"BUTTON", L"Lifecycle");
        sidebar = ui_native_shell_panel_handle(active.shell, "fixture.panel");
        check(tag && (GetWindowLongPtrW(tag, GWL_STYLE) & WS_VISIBLE), "narrow active fixture has clickable sidebar tag");
        (void)SendMessageW(tag, BM_CLICK, 0, 0);
        check(GetWindow(sidebar, GW_OWNER) == root && (GetWindowLongPtrW(sidebar, GWL_STYLE) & WS_VISIBLE),
            "actual collapsed-tag click reaches shell and floats content");
        check(ui_native_shell_set_panel_floating(active.shell, "fixture.panel", 0) == UI_STATUS_OK, "redock fixture sidebar");
        rect.width = 1000;
        check(ui_workspace_set_rect(w, &rect, 96) == UI_STATUS_OK, "restore workspace after actual controls probe");
    }
    check(ui_workspace_invoke(w, fixtures[0], "fixture.add", "{\"bad\":1}", &request) == UI_STATUS_VALIDATION_FAILED,
        "command validator blocks invalid arguments");
    check(ui_workspace_invoke(w, fixtures[0], "fixture.destructive", "{}", &request) == UI_STATUS_PERMISSION_DENIED &&
        o.confirmations == 1 && o.confirm_instance == fixtures[0], "destructive assistant operation confirms original target");
    o.approve = 1;
    check(invoke(w, fixtures[0], "fixture.destructive", &request) && o.confirmations == 2,
        "confirmed destructive command succeeds");
    copied_delivery(w, &o, fixtures[0], fixtures[1]);
    check(invoke(w, fixtures[0], "fixture.delayed", &request) &&
        ui_workspace_cancel(w, fixtures[0], request) == UI_STATUS_OK, "cooperative cancellation targets pending instance request");
    ui_workspace_poll(w);
    check(o.last_instance == fixtures[0] && !strcmp(o.result, "{\"cancelled\":true}") &&
        state_has(w, fixtures[0], "\"cancels\":1"), "application cancellation callback returns copied result");
    check(open_app(w, stage.paths[1], &single) && open_app(w, stage.paths[1], &repeated) && single == repeated,
        "single-instance reopening activates existing tab");
    check(ui_workspace_count(w) == 5 && ui_workspace_active(w) == single, "single-instance package does not add duplicate tab");
    check(ui_workspace_open(w, stage.paths[2], &rejected) == UI_STATUS_ALREADY_EXISTS &&
        ui_workspace_count(w) == 5 && ui_workspace_active(w) == single && rejected == 0,
        "same app ID conflicting version creates no partial tab");
    check(altered_copy(&stage, 3, "architecture=x64", "architecture=x86") &&
        altered_copy(&stage, 4, "abi_version=1", "abi_version=9") &&
        altered_copy(&stage, 5, "fixture.txt", "../evil.txt") &&
        altered_copy(&stage, 6, "fixture.txt", "fixture.dll"), "prepare invalid package fixtures");
    for (i = 3; i <= 6; ++i) {
        ui_status_t expected = i < 5 ? UI_STATUS_UNSUPPORTED : UI_STATUS_VALIDATION_FAILED;
        check(ui_workspace_open(w, stage.paths[i], &rejected) == expected && rejected == 0 &&
            ui_workspace_count(w) == 5 && ui_workspace_last_error(w)[0] != 0,
            "unsupported/malformed package has error and no half-created tab");
    }
    check(invoke(w, fixtures[0], "fixture.close_refuse", &request) &&
        ui_workspace_close(w, fixtures[0], UI_APP_CLOSE_TAB) == UI_STATUS_CANCELLED,
        "application can refuse tab closure");
    check(get_info(w, fixtures[0], &fixture_info) && !fixture_info.closing && ui_workspace_count(w) == 5,
        "refused close retains usable instance");
    check(invoke(w, fixtures[0], "fixture.close_wait", &request) &&
        ui_workspace_close(w, fixtures[0], UI_APP_CLOSE_TAB) == UI_STATUS_OK, "application can wait for shutdown");
    ui_workspace_poll(w);
    check(get_info(w, fixtures[0], &fixture_info) && fixture_info.closing && ui_workspace_count(w) == 5 &&
        ui_workspace_invoke(w, fixtures[0], "fixture.add", "{}", &request) == UI_STATUS_CANCELLED,
        "waiting close retains resources and rejects new commands");
    check(ui_host_invoke(fixture_info.host, "fixture.add", "{}", "native") == 0,
        "closing host rejects direct native/JS command entry points too");
    check(ui_workspace_activate(w, fixtures[0]) == UI_STATUS_CANCELLED && ui_workspace_active(w) == single,
        "closing background instance cannot re-expose its disabled controls or floating panels");
    check(ui_workspace_destroy(w) == UI_STATUS_CANCELLED, "live instances prevent workspace destruction");
    check(ui_workspace_post_close_complete(w, fixtures[0], UI_APP_CLOSE_REFUSE) == UI_STATUS_OK,
        "copy close refusal after asynchronous save negotiation");
    ui_workspace_poll(w);
    check(get_info(w, fixtures[0], &fixture_info) && !fixture_info.closing, "asynchronous refusal resumes instance");
    check(ui_workspace_close(w, fixtures[0], UI_APP_CLOSE_TAB) == UI_STATUS_OK &&
        ui_workspace_post_close_complete(w, fixtures[0], UI_APP_CLOSE_ALLOW) == UI_STATUS_OK,
        "copy quiescent acknowledgement for waiting close");
    {
        HMENU menu = GetMenu(root); unsigned results, events, progress;
        ui_workspace_poll(w);
        check(ui_workspace_count(w) == 4 && GetMenu(root) == menu && ui_workspace_active(w) == single,
            "background close preserves foreground menu and active instance");
        check(GetModuleHandleW(fixture_dll) != NULL, "shared app module stays loaded while another instance remains");
        results = o.results; events = o.events; progress = o.progress;
        check(ui_workspace_post_result(w, fixtures[0], 1, 1, "{\"stale\":true}") == UI_STATUS_OK &&
            ui_workspace_post_event(w, fixtures[0], "stale.event", "{}") == UI_STATUS_OK &&
            ui_workspace_post_progress(w, fixtures[0], 1, 99, "stale progress") == UI_STATUS_OK,
            "late worker deliveries can safely use a closed ID");
        ui_workspace_poll(w);
        check(o.results == results && o.events == events && o.progress == progress,
            "closed-instance results/events/progress are discarded");
    }
    check(ui_workspace_close(w, fixtures[1], UI_APP_CLOSE_TAB) == UI_STATUS_OK, "close final multi-instance fixture");
    ui_workspace_poll(w); assert_unloaded(fixture_dll);
    check(open_app(w, stage.paths[0], &reopened) && reopened > fixtures[1] && reopened != fixtures[0] &&
        state_has(w, reopened, "\"count\":0"), "reopen reloads module with fresh non-reused instance identity");
    {
        unsigned results = o.results, events = o.events, progress = o.progress;
        check(invoke(w, reopened, "fixture.delayed", &request), "new instance has a fresh pending request after reload");
        check(ui_workspace_post_result(w, fixtures[0], request, 1, "{\"stale\":true}") == UI_STATUS_OK &&
            ui_workspace_post_event(w, fixtures[0], "stale.reload", "{}") == UI_STATUS_OK &&
            ui_workspace_post_progress(w, fixtures[0], request, 90, "stale reload") == UI_STATUS_OK,
            "enqueue closed instance messages while replacement is alive");
        ui_workspace_poll(w);
        check(o.results == results && o.events == events && o.progress == progress &&
            state_has(w, reopened, "\"count\":0"), "reopened app never receives old instance messages with colliding request IDs");
        check(ui_workspace_post_result(w, reopened, request, 1, "{\"fresh\":true}") == UI_STATUS_OK,
            "replacement instance can receive its own pending result");
        ui_workspace_poll(w);
        check(o.last_instance == reopened && !strcmp(o.result, "{\"fresh\":true}"), "fresh result reaches replacement instance only");
    }
    check(ui_workspace_close(w, reopened, UI_APP_CLOSE_TAB) == UI_STATUS_OK &&
        ui_workspace_close(w, single, UI_APP_CLOSE_TAB) == UI_STATUS_OK, "close reloaded and singleton fixture modules");
    ui_workspace_poll(w);
    check(o.unmounts >= 4, "unmount callbacks run while host remains alive");
    check(ui_workspace_close(w, eda[0], UI_APP_CLOSE_TAB) == UI_STATUS_OK, "close background EDA tab");
    ui_workspace_poll(w);
    check(GetModuleHandleW(eda_dll) != NULL && IsWindow(canvas[1]) && IsWindow(note[1]),
        "remaining EDA instance retains shared DLL and own windows");
    check(ui_workspace_close(w, eda[1], UI_APP_CLOSE_TAB) == UI_STATUS_OK, "close last EDA tab");
    ui_workspace_poll(w); assert_unloaded(eda_dll);
    check(ui_workspace_count(w) == 0 && ui_workspace_active(w) == 0, "all tabs close to empty workspace");
    check(open_app(w, eda_path, &reopened) && reopened > eda[1], "EDA window class/module can reload after full unload");
    check(ui_workspace_close_all(w) == UI_STATUS_OK, "close all reopened apps"); ui_workspace_poll(w);
    {
        const wchar_t *variables[] = {L"UI_FIXTURE_BAD_ABI", L"UI_FIXTURE_FAIL_CREATE", L"UI_FIXTURE_FAIL_MOUNT"};
        for (i = 0; i < sizeof(variables) / sizeof(variables[0]); ++i) {
            SetEnvironmentVariableW(variables[i], L"1");
            check(ui_workspace_open(w, stage.paths[0], &rejected) != UI_STATUS_OK && ui_workspace_count(w) == 0,
                "DLL ABI/create/mount failure leaves no instance");
            SetEnvironmentVariableW(variables[i], NULL); ui_workspace_poll(w);
            check(open_app(w, stage.paths[0], &reopened), "failed module load is cleaned and can reopen normally");
            check(ui_workspace_close_all(w) == UI_STATUS_OK, "close after failed-load recovery"); ui_workspace_poll(w);
        }
    }
    check(open_app(w, stage.paths[0], &reopened) && get_info(w, reopened, &fixture_info),
        "load fixture for closure from its current command callback");
    check(module_path(descendant(fixture_info.native_container, L"UiFrameworkLifecycleFixtureV1"), fixture_dll),
        "capture self-closing module path");
    request = ui_host_invoke(fixture_info.host, "fixture.close_self", "{}", "native");
    check(request != 0 &&
        get_info(w, reopened, &fixture_info) && fixture_info.closing &&
        GetModuleHandleW(fixture_dll) != NULL && o.result_success && !strcmp(o.result, "{\"alive_in_callback\":true}"),
        "direct host command callback cannot poll-destroy its current state, host or module");
    ui_workspace_poll(w); assert_unloaded(fixture_dll);
    {
        const char *arms[] = {"fixture.arm_frame", "fixture.arm_input", "fixture.arm_external"};
        for (i = 0; i < sizeof(arms) / sizeof(arms[0]); ++i) {
            HWND private_window, surface_window; unsigned events;
            check(open_app(w, stage.paths[0], &reopened) && get_info(w, reopened, &fixture_info),
                "open fixture for surface/external callback closure");
            private_window = descendant(fixture_info.native_container, L"UiFrameworkLifecycleFixtureV1");
            surface_window = descendant(fixture_info.native_container, L"UiFrameworkSurface");
            check(private_window && surface_window && module_path(private_window, fixture_dll),
                "fixture has actual application WndProc and native surface");
            check(invoke(w, reopened, arms[i], &request), "arm specific callback close path");
            events = o.events;
            if (i == 0) (void)SendMessageW(surface_window, WM_PAINT, 0, 0);
            else if (i == 1) (void)SendMessageW(surface_window, WM_KEYDOWN, 'A', 0);
            else (void)SendMessageW(private_window, WM_APP + 101u, 0, 0);
            check(o.events == events + 1 && o.event_instance == reopened &&
                !strcmp(o.event, "fixture.callback.alive") && !strcmp(o.event_payload, "true") &&
                get_info(w, reopened, &fixture_info) && fixture_info.closing &&
                IsWindow(private_window) && IsWindow(surface_window) && GetModuleHandleW(fixture_dll) != NULL,
                "nested poll inside frame/input/application WndProc defers unmount and DLL unload");
            ui_workspace_poll(w); assert_unloaded(fixture_dll);
        }
    }
    if (argc == 4) {
        uint64_t cpp[2]; HMODULE cpp_module; wchar_t cpp_path[MAX_PATH];
        check(CopyFileW(argv[3], stage.module, FALSE) &&
            pack(&stage, 7, "org.ui.lifecycle.cpp", "1.0.0", 1), "package actual C++ SDK consumer");
        check(open_app(w, stage.paths[7], &cpp[0]) && open_app(w, stage.paths[7], &cpp[1]) &&
            cpp[0] != cpp[1], "load two native C++ modules through the same C ABI");
        check(invoke(w, cpp[0], "fixture.cpp.count", &request) &&
            invoke(w, cpp[0], "fixture.cpp.count", &request) &&
            invoke(w, cpp[1], "fixture.cpp.count", &request) &&
            state_has(w, cpp[0], "\"count\":2") && state_has(w, cpp[1], "\"count\":1"),
            "C++ module instances maintain independent state via semantic commands");
        cpp_module = GetModuleHandleW(L"fixture.dll");
        check(cpp_module && GetModuleFileNameW(cpp_module, cpp_path, MAX_PATH), "capture C++ module extraction path");
        check(ui_workspace_close(w, cpp[0], UI_APP_CLOSE_TAB) == UI_STATUS_OK, "close background C++ instance");
        ui_workspace_poll(w);
        check(GetModuleHandleW(cpp_path) == cpp_module && state_has(w, cpp[1], "\"count\":1"),
            "background C++ instance destruction preserves remaining module and state");
        check(ui_workspace_close(w, cpp[1], UI_APP_CLOSE_TAB) == UI_STATUS_OK, "close final C++ instance");
        ui_workspace_poll(w); assert_unloaded(cpp_path);
    }
    (void)swprintf(report, MAX_PATH, L"%ls\\retained.txt", stage.root);
    cleanup_failure_process(argv[2], report);
    check(ui_workspace_destroy(w) == UI_STATUS_OK, "empty workspace destroys after all DLLs unload");
    w = NULL; SetWindowLongPtrW(root, GWLP_USERDATA, 0);

cleanup:
    if (w) { (void)ui_workspace_close_all(w); ui_workspace_poll(w); (void)ui_workspace_destroy(w); }
    stage_destroy(&stage);
    if (root) { SetWindowLongPtrW(root, GWLP_USERDATA, 0); DestroyWindow(root); }
    (void)UnregisterClassW(wc.lpszClassName, wc.hInstance);
    printf("Workspace/module lifecycle: %d failure(s)\n", failures);
    return failures != 0;
}
