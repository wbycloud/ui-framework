#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0603
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "ui_framework/application.h"
#include "ui_framework/package.h"
#include "ui_internal.h"

typedef struct app_module app_module_t;
typedef struct app_instance app_instance_t;
typedef struct posted_message posted_message_t;
struct app_module {
    ui_package_t *package;
    const ui_package_metadata_t *meta;
    HMODULE dll;
    ui_app_descriptor_t api;
    wchar_t directory[MAX_PATH];
    wchar_t dll_path[MAX_PATH];
    size_t instances;
    int cleanup_failed;
    app_module_t *next;
};
struct app_instance {
    ui_workspace_t *workspace;
    app_module_t *module;
    ui_app_context_t context;
    ui_assistant_config_t app_assistant;
    void *state;
    HWND container;
    int mounted, closing, close_ready, active;
    app_instance_t *next;
};
enum post_kind { POST_RESULT, POST_EVENT, POST_PROGRESS, POST_CLOSE };
struct posted_message {
    enum post_kind kind;
    uint64_t id, request;
    int value;
    char *name, *text;
    posted_message_t *next;
};
struct ui_workspace {
    ui_workspace_config_t config;
    HWND parent;
    app_module_t *modules;
    app_instance_t *instances;
    uint64_t active_id;
    ui_rect_t rect;
    uint32_t dpi;
    int depth, polling, deferred_poll;
    CRITICAL_SECTION queue_lock;
    posted_message_t *queue_head, *queue_tail;
    char error[512];
};
static volatile LONG64 instance_sequence;

static char *copy_text(const char *s)
{
    size_t n; char *p;
    if (s == NULL) s = "";
    n = strlen(s) + 1; p = (char *)malloc(n);
    if (p != NULL) memcpy(p, s, n);
    return p;
}
static wchar_t *wide_text(const char *s)
{
    int n; wchar_t *p;
    if (s == NULL) return NULL;
    n = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s, -1, NULL, 0);
    if (n == 0) return NULL;
    p = (wchar_t *)malloc((size_t)n * sizeof(*p));
    if (p != NULL && !MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                          s, -1, p, n)) { free(p); p = NULL; }
    return p;
}
static ui_status_t error_text(ui_workspace_t *w, ui_status_t status,
                              const char *text)
{
    (void)snprintf(w->error, sizeof(w->error), "%s", text);
    return status;
}
static ui_status_t windows_error(ui_workspace_t *w, const char *operation)
{
    DWORD code = GetLastError();
    (void)snprintf(w->error, sizeof(w->error), "%s (Windows error %lu)",
                   operation, (unsigned long)code);
    return UI_STATUS_PLATFORM_ERROR;
}
static app_instance_t *find_instance(const ui_workspace_t *w, uint64_t id)
{
    app_instance_t *p;
    for (p = w->instances; p != NULL; p = p->next)
        if (p->context.instance_id == id) return p;
    return NULL;
}
static void leave_callback(ui_workspace_t *w);
static void changed(ui_workspace_t *w)
{
    if (w->config.changed != NULL) {
        ++w->depth; w->config.changed(w->config.user_data); leave_callback(w);
    }
}
static void wake(ui_workspace_t *w)
{
    (void)PostMessageW(w->parent, UI_WORKSPACE_WAKE_MESSAGE, 0, 0);
}
static void leave_callback(ui_workspace_t *w)
{
    --w->depth;
    if (!w->depth && w->deferred_poll) {
        w->deferred_poll = 0; wake(w);
    }
}
static void host_idle(void *data)
{
    ui_workspace_t *w = (ui_workspace_t *)data;
    if (!w->depth && w->deferred_poll) { w->deferred_poll = 0; wake(w); }
}
static void result_received(const ui_result_t *result, void *data)
{
    app_instance_t *p = (app_instance_t *)data;
    ui_workspace_t *w = p->workspace;
    ++w->depth;
    if (w->config.result != NULL)
        w->config.result(p->context.instance_id, result, w->config.user_data);
    /* Request tracking belongs to the host, and is removed only after observers. */
    if (p->context.assistant != NULL)
        (void)ui_assistant_forget_request(p->context.assistant, result->request_id);
    leave_callback(w);
}
static void event_received(const char *name, const char *json, void *data)
{
    app_instance_t *p = (app_instance_t *)data;
    if (p->workspace->config.event != NULL) {
        ++p->workspace->depth;
        p->workspace->config.event(p->context.instance_id, name, json,
                                   p->workspace->config.user_data);
        leave_callback(p->workspace);
    }
}
static int confirm_command(const char *id, const char *json,
    ui_assistant_permission_t permission, void *data)
{
    app_instance_t *p = (app_instance_t *)data;
    int allow = 0;
    ++p->workspace->depth;
    if (p->workspace->config.confirm != NULL)
        allow = p->workspace->config.confirm(p->context.instance_id, id, json,
            permission, p->workspace->config.user_data);
    if (allow && p->app_assistant.confirm != NULL)
        allow = p->app_assistant.confirm(id, json, permission,
                                         p->app_assistant.user_data);
    leave_callback(p->workspace);
    return allow;
}
static void progress_received(uint64_t request, int percent,
                              const char *message, void *data)
{
    app_instance_t *p = (app_instance_t *)data;
    ++p->workspace->depth;
    if (p->app_assistant.progress != NULL)
        p->app_assistant.progress(request, percent, message,
                                  p->app_assistant.user_data);
    if (p->workspace->config.progress != NULL)
        p->workspace->config.progress(p->context.instance_id, request, percent,
            message, p->workspace->config.user_data);
    leave_callback(p->workspace);
}
static void cancel_received(uint64_t request, void *data)
{
    app_instance_t *p = (app_instance_t *)data;
    if (p->app_assistant.cancel != NULL)
        p->app_assistant.cancel(request, p->app_assistant.user_data);
}
static const char *snapshot_received(void *data)
{
    app_instance_t *p = (app_instance_t *)data;
    const char *snapshot;
    ++p->workspace->depth;
    snapshot = p->app_assistant.snapshot != NULL ?
        p->app_assistant.snapshot(p->app_assistant.user_data) : "{}";
    leave_callback(p->workspace); return snapshot;
}
static ui_status_t transaction_begin(const char *label, uint64_t *id, void *data)
{
    app_instance_t *p = (app_instance_t *)data;
    ui_status_t status;
    ++p->workspace->depth;
    status = p->app_assistant.transaction_begin != NULL ?
        p->app_assistant.transaction_begin(label, id, p->app_assistant.user_data) :
        UI_STATUS_UNSUPPORTED;
    leave_callback(p->workspace); return status;
}
#define TRANSACTION_FORWARD(name, field) \
static ui_status_t name(uint64_t id, void *data) \
{ app_instance_t *p = (app_instance_t *)data; ui_status_t status; \
  ++p->workspace->depth; status = p->app_assistant.field != NULL ? \
    p->app_assistant.field(id, p->app_assistant.user_data) : UI_STATUS_UNSUPPORTED; \
  leave_callback(p->workspace); return status; }
TRANSACTION_FORWARD(transaction_commit, transaction_commit)
TRANSACTION_FORWARD(transaction_rollback, transaction_rollback)
TRANSACTION_FORWARD(transaction_undo, transaction_undo)

static LRESULT CALLBACK container_proc(HWND hwnd, UINT message,
                                       WPARAM wp, LPARAM lp)
{
    app_instance_t *p = (app_instance_t *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    if (message == WM_NCCREATE) {
        p = (app_instance_t *)((CREATESTRUCTW *)lp)->lpCreateParams;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)p);
    }
    if (p != NULL && p->context.host != NULL && message == WM_SIZE) {
        intptr_t result;
        ++p->workspace->depth;
        (void)ui_host_handle_message(p->context.host, hwnd, message,
                                     (uintptr_t)wp, (intptr_t)lp, &result);
        if (p->context.shell != NULL)
            (void)ui_native_shell_reflow(p->context.shell);
        leave_callback(p->workspace);
    }
    if (p != NULL && p->context.shell != NULL && p->active && !p->closing &&
        message == WM_COMMAND) {
        intptr_t result; ui_status_t status;
        ++p->workspace->depth;
        status = ui_native_shell_handle_message(p->context.shell, hwnd, message,
            (uintptr_t)wp, (intptr_t)lp, &result);
        leave_callback(p->workspace);
        if (status == UI_STATUS_OK) return (LRESULT)result;
    }
    return DefWindowProcW(hwnd, message, wp, lp);
}
static int create_container_class(void)
{
    WNDCLASSW wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc = container_proc;
    wc.hInstance = GetModuleHandleW(L"ui_framework.dll");
    wc.lpszClassName = L"UiFrameworkApplicationContainerV1";
    wc.hCursor = LoadCursorW(NULL, MAKEINTRESOURCEW(32512));
    return RegisterClassW(&wc) != 0 || GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}
static int remove_directory(const wchar_t *directory)
{
    wchar_t pattern[MAX_PATH], path[MAX_PATH]; WIN32_FIND_DATAW entry;
    HANDLE search; int ok = 1;
    if (!directory[0] || wcslen(directory) + 3 >= MAX_PATH) return 0;
    (void)swprintf(pattern, MAX_PATH, L"%ls\\*", directory);
    search = FindFirstFileW(pattern, &entry);
    if (search != INVALID_HANDLE_VALUE) {
        do {
            if (wcscmp(entry.cFileName, L".") == 0 ||
                wcscmp(entry.cFileName, L"..") == 0) continue;
            if (wcslen(directory) + wcslen(entry.cFileName) + 2 >= MAX_PATH) {
                ok = 0; continue;
            }
            (void)swprintf(path, MAX_PATH, L"%ls\\%ls", directory, entry.cFileName);
            if (entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (entry.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
                    if (!RemoveDirectoryW(path)) ok = 0;
                } else if (!remove_directory(path)) ok = 0;
            } else if (!DeleteFileW(path)) ok = 0;
        } while (FindNextFileW(search, &entry));
        FindClose(search);
    }
    if (!RemoveDirectoryW(directory)) ok = 0;
    return ok;
}
static int extraction_path(app_module_t *m, const char *name, wchar_t *path)
{
    wchar_t *wide = wide_text(name); size_t i, root_length = wcslen(m->directory);
    if (wide == NULL) return 0;
    if (root_length + wcslen(wide) + 2 >= MAX_PATH) { free(wide); return 0; }
    (void)swprintf(path, MAX_PATH, L"%ls\\%ls", m->directory, wide);
    free(wide);
    for (i = root_length + 1; path[i]; ++i) {
        if (path[i] == L'/') path[i] = L'\\';
        if (path[i] == L'\\') {
            path[i] = 0;
            if (!CreateDirectoryW(path, NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
                return 0;
            path[i] = L'\\';
        }
    }
    return 1;
}
static ui_status_t extract_module(ui_workspace_t *w, app_module_t *m)
{
    wchar_t temp[MAX_PATH], path[MAX_PATH]; size_t i;
    if (!GetTempPathW(MAX_PATH, temp) ||
        !GetTempFileNameW(temp, L"uap", 0, m->directory))
        return windows_error(w, "Cannot create application temporary directory");
    if (!DeleteFileW(m->directory) || !CreateDirectoryW(m->directory, NULL))
        return windows_error(w, "Cannot create application temporary directory");
    for (i = 0; i < ui_package_entry_count(m->package); ++i) {
        ui_package_entry_t entry; size_t n, length; void *bytes; FILE *out;
        (void)ui_package_get_entry(m->package, i, &entry);
        n = strlen(entry.name);
        /* Resources stay in the immutable package; only native DLLs are extracted. */
        if (n < 4 || _stricmp(entry.name + n - 4, ".dll") != 0) continue;
        if (_stricmp(entry.name, "ui_framework.dll") == 0 ||
            (n > 17 && _stricmp(entry.name + n - 17, "/ui_framework.dll") == 0))
            return error_text(w, UI_STATUS_VALIDATION_FAILED,
                               "Application package cannot replace ui_framework.dll");
        if (!extraction_path(m, entry.name, path))
            return error_text(w, UI_STATUS_PLATFORM_ERROR, "DLL extraction path is too long or unavailable");
        if (ui_package_read(m->package, entry.name, &bytes, &length) != UI_STATUS_OK)
            return error_text(w, UI_STATUS_PLATFORM_ERROR, "Cannot read packaged DLL");
        out = _wfopen(path, L"wb");
        if (out == NULL) { ui_package_release(bytes); return windows_error(w, "Cannot write packaged DLL"); }
        n = fwrite(bytes, 1, length, out); ui_package_release(bytes);
        if (fclose(out) != 0 || n != length)
            return error_text(w, UI_STATUS_PLATFORM_ERROR, "Cannot finish packaged DLL extraction");
    }
    if (!extraction_path(m, m->meta->module, m->dll_path))
        return error_text(w, UI_STATUS_PLATFORM_ERROR, "Application module path unavailable");
    m->dll = LoadLibraryExW(m->dll_path, NULL,
        LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if (m->dll == NULL) return windows_error(w, "Cannot load application DLL or its dependencies");
    {
        FARPROC proc = GetProcAddress(m->dll, "ui_app_query_v1");
        ui_app_query_fn query = NULL; const ui_app_descriptor_t *api;
        if (proc == NULL) return error_text(w, UI_STATUS_UNSUPPORTED, "Application DLL lacks ui_app_query_v1");
        memcpy(&query, &proc, sizeof(query));
        ++w->depth; api = query(); leave_callback(w);
        if (api == NULL || api->size < sizeof(*api) ||
            api->abi_version != UI_APPLICATION_ABI_VERSION ||
            api->framework_api_version != UI_FRAMEWORK_API_VERSION)
            return error_text(w, UI_STATUS_UNSUPPORTED, "Application DLL ABI does not match the manifest/runtime");
        if (!api->create || !api->mount || !api->request_close || !api->unmount || !api->destroy)
            return error_text(w, UI_STATUS_VALIDATION_FAILED, "Application DLL lacks required lifecycle callbacks");
        m->api = *api;
    }
    return UI_STATUS_OK;
}
static void unmount_instance(app_instance_t *p)
{
    ui_workspace_t *w = p->workspace;
    ++w->depth;
    if (p->mounted) { p->module->api.unmount(p->state); p->mounted = 0; }
    if (p->context.shell != NULL) {
        (void)ui_native_shell_set_active(p->context.shell, 0);
        ui_native_shell_destroy(p->context.shell); p->context.shell = NULL;
    }
    if (p->context.assistant != NULL) {
        ui_assistant_destroy(p->context.assistant); p->context.assistant = NULL;
    }
    if (p->context.host != NULL) { ui_host_destroy(p->context.host); p->context.host = NULL; }
    if (p->container != NULL) { DestroyWindow(p->container); p->container = NULL; }
    if (p->state != NULL) { p->module->api.destroy(p->state); p->state = NULL; }
    leave_callback(w);
}
static ui_status_t create_instance(ui_workspace_t *w, app_module_t *m,
                                  app_instance_t **out)
{
    app_instance_t *p = (app_instance_t *)calloc(1, sizeof(*p));
    ui_host_config_t host; ui_native_shell_config_t shell;
    ui_assistant_config_t assistant; ui_status_t status;
    if (p == NULL) return UI_STATUS_OUT_OF_MEMORY;
    p->workspace = w; p->module = m;
    p->context.size = sizeof(p->context);
    p->context.abi_version = UI_APPLICATION_ABI_VERSION;
    p->context.workspace = w;
    p->context.instance_id = (uint64_t)InterlockedIncrement64(&instance_sequence);
    p->container = CreateWindowExW(0, L"UiFrameworkApplicationContainerV1", L"",
        WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, 0, 1, 1,
        w->parent, NULL, GetModuleHandleW(L"ui_framework.dll"), p);
    if (p->container == NULL) { free(p); return windows_error(w, "Cannot create application container"); }
    ZeroMemory(&host, sizeof(host)); host.size = sizeof(host);
    host.api_version = UI_FRAMEWORK_API_VERSION; host.native_parent = p->container;
    host.user_data = p; host.result_callback = result_received; host.event_callback = event_received;
    p->context.host = ui_host_create(&host);
    if (!p->context.host) { DestroyWindow(p->container); free(p); return error_text(w, UI_STATUS_PLATFORM_ERROR, "Cannot create application host"); }
    p->context.host->dispatch_idle = host_idle;
    p->context.host->dispatch_idle_data = w;
    /* Establish host geometry before application create/mount can start work.
       Opening one application must not resize unrelated existing instances. */
    status = ui_host_set_dpi(p->context.host, w->dpi);
    if (status != UI_STATUS_OK) goto fail;
    (void)SetWindowPos(p->container, NULL, MulDiv(w->rect.x,(int)w->dpi,96),
        MulDiv(w->rect.y,(int)w->dpi,96), MulDiv(w->rect.width,(int)w->dpi,96),
        MulDiv(w->rect.height,(int)w->dpi,96), SWP_NOZORDER|SWP_NOACTIVATE);
    status = ui_host_resize(p->context.host,w->rect.width,w->rect.height);
    if (status != UI_STATUS_OK) goto fail;
    ZeroMemory(&p->app_assistant, sizeof(p->app_assistant));
    p->app_assistant.size = sizeof(p->app_assistant);
    ++w->depth;
    status = m->api.create(&p->context, &p->state, &p->app_assistant);
    leave_callback(w);
    if (status != UI_STATUS_OK) goto fail;
    assistant = p->app_assistant;
    assistant.size = sizeof(assistant); assistant.user_data = p;
    if (assistant.max_permission > w->config.max_permission)
        assistant.max_permission = w->config.max_permission;
    assistant.confirm = confirm_command; assistant.progress = progress_received;
    assistant.cancel = cancel_received; assistant.snapshot = snapshot_received;
    assistant.transaction_begin = p->app_assistant.transaction_begin ? transaction_begin : NULL;
    assistant.transaction_commit = p->app_assistant.transaction_commit ? transaction_commit : NULL;
    assistant.transaction_rollback = p->app_assistant.transaction_rollback ? transaction_rollback : NULL;
    assistant.transaction_undo = p->app_assistant.transaction_undo ? transaction_undo : NULL;
    p->context.assistant = ui_assistant_create(p->context.host, &assistant);
    if (!p->context.assistant) { status = UI_STATUS_OUT_OF_MEMORY; goto fail; }
    ZeroMemory(&shell, sizeof(shell)); shell.size = sizeof(shell);
    shell.host = p->context.host; shell.native_parent = p->container;
    shell.menu_owner = w->parent; shell.flags = UI_NATIVE_SHELL_MANAGED_ACTIVATION;
    p->context.shell = ui_native_shell_create(&shell);
    if (!p->context.shell) { status = UI_STATUS_PLATFORM_ERROR; goto fail; }
    /* Add before mount so resource reads and callbacks can resolve the instance. */
    { app_instance_t **tail = &w->instances;
      while (*tail) tail = &(*tail)->next;
      *tail = p; }
    ++m->instances;
    p->mounted = 1;
    ++w->depth; status = m->api.mount(p->state, &p->context); leave_callback(w);
    if (status != UI_STATUS_OK) {
        app_instance_t **link = &w->instances;
        while (*link != p) link = &(*link)->next;
        *link = p->next; --m->instances; goto fail;
    }
    *out = p; return UI_STATUS_OK;
fail:
    unmount_instance(p); free(p);
    (void)snprintf(w->error, sizeof(w->error), "Application create/mount failed (status %d)", (int)status);
    return status;
}
static void set_closing(app_instance_t *p, int closing)
{
    ++p->workspace->depth;
    p->closing = closing;
    p->context.host->dispatch_blocked = closing;
    EnableWindow(p->container, !closing);
    if (closing) {
        if(p->active && p->module->api.set_active) p->module->api.set_active(p->state,0);
        (void)ui_native_shell_set_active(p->context.shell, 0);
    } else if (p->active) {
        (void)ui_native_shell_set_active(p->context.shell, 1);
        if(p->module->api.set_active) p->module->api.set_active(p->state,1);
    }
    leave_callback(p->workspace);
}

ui_workspace_t *ui_workspace_create(const ui_workspace_config_t *config)
{
    ui_workspace_t *w;
    if (!config || config->size < sizeof(*config) ||
        !IsWindow((HWND)config->native_parent) || !create_container_class()) return NULL;
    w = (ui_workspace_t *)calloc(1, sizeof(*w));
    if (w != NULL) {
        w->config = *config; w->parent = (HWND)config->native_parent;
        w->dpi = 96; InitializeCriticalSection(&w->queue_lock);
    }
    return w;
}
ui_status_t ui_workspace_open(ui_workspace_t *w, const char *path, uint64_t *id)
{
    ui_package_t *package = NULL; const ui_package_metadata_t *meta;
    app_module_t *m; app_instance_t *p; ui_status_t status;
    if (!w || !path || !id) return UI_STATUS_INVALID_ARGUMENT;
    if (w->depth || w->polling) return error_text(w, UI_STATUS_CANCELLED, "Defer application opening until the current callback returns");
    *id = 0; w->error[0] = 0;
    status = ui_package_open(path, &package, w->error, sizeof(w->error));
    if (status != UI_STATUS_OK) return status;
    meta = ui_package_get_metadata(package);
    for (m = w->modules; m; m = m->next) {
        if (strcmp(m->meta->app_id, meta->app_id) == 0) break;
    }
    if (m != NULL) {
        if (strcmp(m->meta->version, meta->version) != 0) {
            ui_package_destroy(package);
            return error_text(w, UI_STATUS_ALREADY_EXISTS, "Another version of this application is already loaded");
        }
        ui_package_destroy(package);
        if (m->cleanup_failed) return error_text(w, UI_STATUS_PLATFORM_ERROR, "Module cleanup failed; restart the host before reopening this application");
        if (!m->meta->multiple_instances) {
            for (p = w->instances; p; p = p->next) if (p->module == m) {
                if (p->closing) return error_text(w, UI_STATUS_CANCELLED, "Application is still closing");
                *id = p->context.instance_id; return ui_workspace_activate(w, *id);
            }
        }
    } else {
        m = (app_module_t *)calloc(1, sizeof(*m));
        if (!m) { ui_package_destroy(package); return UI_STATUS_OUT_OF_MEMORY; }
        m->package = package; m->meta = meta;
        status = extract_module(w, m);
        if (status != UI_STATUS_OK) {
            if (m->dll) FreeLibrary(m->dll);
            if (m->directory[0]) (void)remove_directory(m->directory);
            ui_package_destroy(package); free(m); return status;
        }
        m->next = w->modules; w->modules = m;
    }
    status = create_instance(w, m, &p);
    if (status != UI_STATUS_OK) { wake(w); return status; }
    *id = p->context.instance_id;
    return ui_workspace_activate(w, *id);
}
ui_status_t ui_workspace_activate(ui_workspace_t *w, uint64_t id)
{
    app_instance_t *p, *old;
    if (!w) return UI_STATUS_INVALID_ARGUMENT;
    p = find_instance(w, id);
    if (!p) return UI_STATUS_NOT_FOUND;
    if (p->closing) return UI_STATUS_CANCELLED;
    if (w->active_id == id) return UI_STATUS_OK;
    ++w->depth;
    old = find_instance(w, w->active_id);
    if (old) {
        old->active = 0;
        if (old->module->api.set_active) old->module->api.set_active(old->state, 0);
        (void)ui_native_shell_set_active(old->context.shell, 0);
        ShowWindow(old->container, SW_HIDE);
    }
    w->active_id = id; p->active = 1;
    ShowWindow(p->container, SW_SHOW);
    (void)ui_native_shell_set_active(p->context.shell, 1);
    if (p->module->api.set_active) p->module->api.set_active(p->state, 1);
    leave_callback(w); changed(w);
    return UI_STATUS_OK;
}
ui_status_t ui_workspace_close(ui_workspace_t *w, uint64_t id,
                               ui_app_close_reason_t reason)
{
    app_instance_t *p; ui_app_close_decision_t decision;
    if (!w) return UI_STATUS_INVALID_ARGUMENT;
    p = find_instance(w, id); if (!p) return UI_STATUS_NOT_FOUND;
    if (p->closing) return UI_STATUS_OK;
    p->closing = 1; ++w->depth;
    decision = p->module->api.request_close(p->state, reason);
    leave_callback(w);
    if (decision == UI_APP_CLOSE_REFUSE) { set_closing(p, 0); changed(w); return UI_STATUS_CANCELLED; }
    if (decision != UI_APP_CLOSE_ALLOW && decision != UI_APP_CLOSE_WAIT) {
        set_closing(p, 0); return error_text(w, UI_STATUS_INVALID_ARGUMENT, "Application returned an invalid close decision");
    }
    set_closing(p, 1);
    p->close_ready = decision == UI_APP_CLOSE_ALLOW;
    changed(w); wake(w); return UI_STATUS_OK;
}
ui_status_t ui_workspace_close_all(ui_workspace_t *w)
{
    app_instance_t *p; ui_status_t status = UI_STATUS_OK;
    if (!w) return UI_STATUS_INVALID_ARGUMENT;
    for (p = w->instances; p; p = p->next) {
        ui_status_t s = ui_workspace_close(w, p->context.instance_id, UI_APP_CLOSE_WORKSPACE);
        if (s != UI_STATUS_OK) status = s;
    }
    return status;
}
size_t ui_workspace_count(const ui_workspace_t *w)
{
    size_t n = 0; app_instance_t *p;
    if (w) for (p = w->instances; p; p = p->next) ++n;
    return n;
}
uint64_t ui_workspace_instance_at(const ui_workspace_t *w, size_t index)
{
    app_instance_t *p;
    if (w) for (p = w->instances; p; p = p->next) if (index-- == 0) return p->context.instance_id;
    return 0;
}
uint64_t ui_workspace_active(const ui_workspace_t *w) { return w ? w->active_id : 0; }
ui_status_t ui_workspace_get_instance(const ui_workspace_t *w, uint64_t id,
                                     ui_app_instance_info_t *info)
{
    app_instance_t *p;
    if (!w || !info || info->size < sizeof(*info)) return UI_STATUS_INVALID_ARGUMENT;
    p = find_instance(w, id); if (!p) return UI_STATUS_NOT_FOUND;
    info->instance_id = id; info->app_id = p->module->meta->app_id;
    info->name_utf8 = p->module->meta->name; info->version = p->module->meta->version;
    info->active = p->active; info->closing = p->closing;
    info->host = p->context.host; info->shell = p->context.shell;
    info->assistant = p->context.assistant; info->native_container = p->container;
    return UI_STATUS_OK;
}
ui_status_t ui_workspace_set_rect(ui_workspace_t *w, const ui_rect_t *rect, uint32_t dpi)
{
    app_instance_t *p; ui_status_t result = UI_STATUS_OK;
    int x, y, right, bottom;
    if (!w || !rect || rect->width < 0 || rect->height < 0 || !dpi || dpi > 768)
        return UI_STATUS_INVALID_ARGUMENT;
    w->rect = *rect; w->dpi = dpi;
    x = MulDiv(rect->x, (int)dpi, 96); y = MulDiv(rect->y, (int)dpi, 96);
    right = MulDiv(rect->x + rect->width, (int)dpi, 96);
    bottom = MulDiv(rect->y + rect->height, (int)dpi, 96);
    ++w->depth;
    for (p = w->instances; p; p = p->next) {
        ui_status_t s = ui_host_set_dpi(p->context.host, dpi);
        if (s != UI_STATUS_OK) result = s;
        (void)SetWindowPos(p->container, NULL, x, y, right-x, bottom-y,
                           SWP_NOZORDER | SWP_NOACTIVATE);
        s = ui_host_resize(p->context.host, rect->width, rect->height);
        if (s != UI_STATUS_OK) result = s;
        s = ui_native_shell_reflow(p->context.shell);
        if (s != UI_STATUS_OK) result = s;
    }
    leave_callback(w); return result;
}
ui_status_t ui_workspace_invoke(ui_workspace_t *w, uint64_t id,
    const char *command, const char *json, uint64_t *request)
{
    app_instance_t *p; ui_status_t status;
    if (!w || !command || !json || !request) return UI_STATUS_INVALID_ARGUMENT;
    p = find_instance(w, id); if (!p) return UI_STATUS_NOT_FOUND;
    if (p->closing) return UI_STATUS_CANCELLED;
    ++w->depth;
    status = ui_assistant_invoke(p->context.assistant, command, json, request);
    leave_callback(w); return status;
}
ui_status_t ui_workspace_cancel(ui_workspace_t *w, uint64_t id, uint64_t request)
{
    app_instance_t *p; ui_status_t status;
    if (!w) return UI_STATUS_INVALID_ARGUMENT;
    p = find_instance(w, id); if (!p) return UI_STATUS_NOT_FOUND;
    ++w->depth; status = ui_assistant_cancel(p->context.assistant, request); leave_callback(w);
    return status;
}
ui_status_t ui_workspace_handle_message(ui_workspace_t *w, void *hwnd,
    uint32_t message, uintptr_t wp, intptr_t lp, intptr_t *result)
{
    app_instance_t *p; ui_status_t status;
    if (!w || !result) return UI_STATUS_INVALID_ARGUMENT;
    if (message == UI_WORKSPACE_WAKE_MESSAGE) { ui_workspace_poll(w); *result = 0; return UI_STATUS_OK; }
    p = find_instance(w, w->active_id);
    if (!p || p->closing) return UI_STATUS_NOT_FOUND;
    ++w->depth;
    status = ui_native_shell_handle_message(p->context.shell, hwnd, message, wp, lp, result);
    leave_callback(w); return status;
}
const char *ui_workspace_last_error(const ui_workspace_t *w) { return w ? w->error : "Invalid workspace"; }
static ui_status_t post_copy(ui_workspace_t *w, enum post_kind kind, uint64_t id,
    uint64_t request, int value, const char *name, const char *text)
{
    posted_message_t *p;
    if (!w || !id) return UI_STATUS_INVALID_ARGUMENT;
    p = (posted_message_t *)calloc(1, sizeof(*p));
    if (!p) return UI_STATUS_OUT_OF_MEMORY;
    p->kind = kind; p->id = id; p->request = request; p->value = value;
    p->name = copy_text(name); p->text = copy_text(text);
    if (!p->name || !p->text) { free(p->name); free(p->text); free(p); return UI_STATUS_OUT_OF_MEMORY; }
    EnterCriticalSection(&w->queue_lock);
    if (w->queue_tail) w->queue_tail->next = p; else w->queue_head = p;
    w->queue_tail = p;
    LeaveCriticalSection(&w->queue_lock); wake(w); return UI_STATUS_OK;
}
ui_status_t ui_workspace_post_result(ui_workspace_t *w, uint64_t id,
    uint64_t request, int success, const char *json)
{ return json && request ? post_copy(w, POST_RESULT, id, request, success, NULL, json) : UI_STATUS_INVALID_ARGUMENT; }
ui_status_t ui_workspace_post_event(ui_workspace_t *w, uint64_t id,
    const char *event, const char *json)
{ return event && json ? post_copy(w, POST_EVENT, id, 0, 0, event, json) : UI_STATUS_INVALID_ARGUMENT; }
ui_status_t ui_workspace_post_progress(ui_workspace_t *w, uint64_t id,
    uint64_t request, int percent, const char *message)
{ return message && request && percent >= 0 && percent <= 100 ? post_copy(w, POST_PROGRESS, id, request, percent, NULL, message) : UI_STATUS_INVALID_ARGUMENT; }
ui_status_t ui_workspace_post_close_complete(ui_workspace_t *w, uint64_t id,
    ui_app_close_decision_t decision)
{ return decision == UI_APP_CLOSE_ALLOW || decision == UI_APP_CLOSE_REFUSE ? post_copy(w, POST_CLOSE, id, 0, (int)decision, NULL, NULL) : UI_STATUS_INVALID_ARGUMENT; }
void ui_workspace_poll(ui_workspace_t *w)
{
    posted_message_t *messages, *next; app_instance_t **link;
    app_module_t **module_link; int did_change = 0;
    if (!w) return;
    if (w->depth || w->polling) { w->deferred_poll = 1; return; }
    { app_instance_t *p;
      for (p=w->instances;p;p=p->next) if (p->context.host->dispatch_depth) {
          w->deferred_poll=1;return;
      } }
    w->polling = 1;
    EnterCriticalSection(&w->queue_lock);
    messages = w->queue_head; w->queue_head = w->queue_tail = NULL;
    LeaveCriticalSection(&w->queue_lock);
    while (messages) {
        app_instance_t *p = find_instance(w, messages->id);
        next = messages->next;
        if (p && !p->close_ready) {
            ++w->depth;
            switch (messages->kind) {
            case POST_RESULT: (void)ui_host_reply(p->context.host, messages->request, messages->value, messages->text); break;
            case POST_EVENT: (void)ui_host_emit_event(p->context.host, messages->name, messages->text); break;
            case POST_PROGRESS: (void)ui_assistant_report_progress(p->context.assistant, messages->request, messages->value, messages->text); break;
            case POST_CLOSE:
                if (p->closing) {
                    p->close_ready = messages->value == UI_APP_CLOSE_ALLOW;
                    if (!p->close_ready) set_closing(p, 0);
                    did_change = 1;
                }
                break;
            }
            leave_callback(w);
        }
        free(messages->name); free(messages->text); free(messages); messages = next;
    }
    link = &w->instances;
    while (*link) {
        app_instance_t *p = *link;
        if (!p->close_ready) { link = &p->next; continue; }
        if (p->active) {
            ++w->depth;
            if (p->module->api.set_active) p->module->api.set_active(p->state, 0);
            (void)ui_native_shell_set_active(p->context.shell, 0);
            leave_callback(w); w->active_id = 0;
        }
        *link = p->next;
        unmount_instance(p); --p->module->instances; free(p); did_change = 1;
    }
    if (!w->active_id) {
        app_instance_t *p;
        for(p=w->instances;p;p=p->next) if(!p->closing){
            (void)ui_workspace_activate(w,p->context.instance_id);break;
        }
    }
    /* No module function pointer remains in a posted message. All instance
       framework callbacks/private windows have been destroyed before shutdown. */
    module_link = &w->modules;
    while (*module_link) {
        app_module_t *m = *module_link; ui_status_t status = UI_STATUS_OK;
        if (m->instances || m->cleanup_failed) { module_link = &m->next; continue; }
        ++w->depth;
        if (m->dll && m->api.module_shutdown) status = m->api.module_shutdown();
        leave_callback(w);
        if (status != UI_STATUS_OK) {
            m->cleanup_failed = 1;
            (void)snprintf(w->error, sizeof(w->error), "Module cleanup failed for %s; DLL retained", m->meta->app_id);
            module_link = &m->next; did_change = 1; continue;
        }
        ZeroMemory(&m->api, sizeof(m->api));
        if (m->dll) {
            if (!FreeLibrary(m->dll)) {
                m->cleanup_failed = 1; (void)windows_error(w, "Cannot unload application DLL");
                module_link = &m->next; continue;
            }
            m->dll = NULL;
        }
        if (m->directory[0] && !remove_directory(m->directory)) {
            m->cleanup_failed = 1; (void)windows_error(w, "DLL unloaded but temporary application files could not be removed");
            module_link = &m->next; continue;
        }
        *module_link = m->next; ui_package_destroy(m->package); free(m);
    }
    w->polling = 0;
    if (did_change) changed(w);
}
ui_status_t ui_workspace_destroy(ui_workspace_t *w)
{
    posted_message_t *p;
    if (!w) return UI_STATUS_INVALID_ARGUMENT;
    if (w->depth || w->polling || w->instances) return UI_STATUS_CANCELLED;
    ui_workspace_poll(w);
    if (w->modules) return UI_STATUS_PLATFORM_ERROR;
    EnterCriticalSection(&w->queue_lock); p = w->queue_head; w->queue_head = w->queue_tail = NULL;
    LeaveCriticalSection(&w->queue_lock);
    while (p) { posted_message_t *next = p->next; free(p->name); free(p->text); free(p); p = next; }
    DeleteCriticalSection(&w->queue_lock); free(w); return UI_STATUS_OK;
}
ui_status_t ui_app_resource_read(const ui_app_context_t *context,
    const char *name, void **bytes, size_t *size)
{
    app_instance_t *p;
    if (!context || !context->workspace || !name || !bytes || !size) return UI_STATUS_INVALID_ARGUMENT;
    p = find_instance(context->workspace, context->instance_id);
    if (!p) return UI_STATUS_NOT_FOUND;
    return ui_package_read(p->module->package, name, bytes, size);
}
void ui_app_resource_release(void *data) { ui_package_release(data); }
void ui_app_callback_enter(const ui_app_context_t *context)
{ if(context && context->host) ui_dispatch_enter(context->host); }
void ui_app_callback_leave(const ui_app_context_t *context)
{ if(context && context->host) ui_dispatch_leave(context->host); }
