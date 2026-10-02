#define COBJMACROS

#include "ui_framework/webview2.h"
#include "../ui_internal.h"

#include <windows.h>
#include <wchar.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <WebView2.h>

typedef struct webview2_backend webview2_backend_t;
typedef struct webview2_view webview2_view_t;
typedef struct webview2_binding webview2_binding_t;
typedef struct navigation_handler navigation_handler_t;
typedef struct message_handler message_handler_t;
typedef struct navigation_starting_handler navigation_starting_handler_t;

typedef struct environment_handler {
    ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler iface;
    LONG refs;
    webview2_view_t *view;
} environment_handler_t;

typedef struct controller_handler {
    ICoreWebView2CreateCoreWebView2ControllerCompletedHandler iface;
    LONG refs;
    webview2_view_t *view;
} controller_handler_t;

struct navigation_handler {
    ICoreWebView2NavigationCompletedEventHandler iface;
    LONG refs;
    webview2_view_t *view;
};

struct message_handler {
    ICoreWebView2WebMessageReceivedEventHandler iface;
    LONG refs;
    webview2_view_t *view;
};

struct navigation_starting_handler {
    ICoreWebView2NavigationStartingEventHandler iface;
    LONG refs;
    webview2_view_t *view;
};

typedef struct script_handler {
    ICoreWebView2ExecuteScriptCompletedHandler iface;
    LONG refs;
    webview2_view_t *view;
    ui_webview2_script_callback_fn callback;
    void *user_data;
} script_handler_t;

typedef struct bridge_handler {
    ICoreWebView2AddScriptToExecuteOnDocumentCreatedCompletedHandler iface;
    LONG refs;
    webview2_view_t *view;
} bridge_handler_t;

struct webview2_backend {
    HWND parent_window;
    wchar_t *user_data_folder;
    int com_initialized;
    LONG refs;
    DWORD thread_id;
    webview2_view_t *views;
};

struct webview2_binding {
    ui_web_backend_t *public_backend;
    webview2_backend_t *state;
    webview2_binding_t *next;
};

struct webview2_view {
    webview2_backend_t *backend;
    ui_host_t *host;
    HWND parent_window;
    ICoreWebView2Environment *environment;
    ICoreWebView2Controller *controller;
    ICoreWebView2 *webview;
    wchar_t *pending_html;
    int x;
    int y;
    int width;
    int height;
    uint32_t dpi;
    HRESULT async_error;
    LONG refs;
    int destroyed;
    int navigation_completed;
    int trusted_html;
    int bridge_installed;
    int allow_html_navigation;
    UINT64 navigation_id;
    navigation_handler_t *navigation_handler;
    EventRegistrationToken navigation_token;
    message_handler_t *message_handler;
    EventRegistrationToken message_token;
    navigation_starting_handler_t *starting_handler;
    EventRegistrationToken starting_token;
    webview2_view_t *backend_next;
};

static webview2_binding_t *bindings;

static ui_status_t hresult_status(HRESULT hr);

static const wchar_t bridge_script[] =
    L"if(window===window.top){window.ui={invoke:function(command,params){"
    L"if(typeof command!=='string'||! /^[A-Za-z0-9_.-]{1,255}$/.test(command))"
    L"throw new TypeError('invalid command');"
    L"var json=typeof params==='string'?params:JSON.stringify(params||{});"
    L"JSON.parse(json);chrome.webview.postMessage('ui.invoke\\n'+command+'\\n'+json);"
    L"},value:function(id){var e=document.getElementById(id);return e?e.value:'';}};}";

static void backend_add_ref(webview2_backend_t *backend)
{
    (void)InterlockedIncrement(&backend->refs);
}

static void backend_release(webview2_backend_t *backend)
{
    if (InterlockedDecrement(&backend->refs) != 0) {
        return;
    }
    free(backend->user_data_folder);
    if (backend->com_initialized) {
        CoUninitialize();
    }
    free(backend);
}

static void view_add_ref(webview2_view_t *view)
{
    (void)InterlockedIncrement(&view->refs);
}

static void view_release(webview2_view_t *view)
{
    webview2_view_t **it;
    if (InterlockedDecrement(&view->refs) != 0) {
        return;
    }
    it = &view->backend->views;
    while (*it != NULL && *it != view) {
        it = &(*it)->backend_next;
    }
    if (*it == view) {
        *it = view->backend_next;
    }
    if (view->webview != NULL) {
        ICoreWebView2_Release(view->webview);
    }
    if (view->controller != NULL) {
        ICoreWebView2Controller_Close(view->controller);
        ICoreWebView2Controller_Release(view->controller);
    }
    if (view->environment != NULL) {
        ICoreWebView2Environment_Release(view->environment);
    }
    free(view->pending_html);
    backend_release(view->backend);
    free(view);
}

static ui_status_t view_status(const webview2_view_t *view)
{
    if (view == NULL || view->destroyed) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    if (view->backend->thread_id != GetCurrentThreadId()) {
        return UI_STATUS_PLATFORM_ERROR;
    }
    return FAILED(view->async_error) ? UI_STATUS_PLATFORM_ERROR : UI_STATUS_OK;
}

static LONG scaled_coordinate(int64_t value, uint32_t dpi)
{
    int64_t magnitude = value < 0 ? -value : value;
    int64_t maximum = value < 0 ? -(int64_t)LONG_MIN : LONG_MAX;
    int64_t result;
    /* Check before multiplication: two rectangle endpoints can exceed int,
       and the public DPI is uint32_t. */
    if (magnitude > maximum * 96 / dpi) {
        return value < 0 ? LONG_MIN : LONG_MAX;
    }
    result = (magnitude * dpi + 48) / 96;
    if (result > maximum) result = maximum;
    return (LONG)(value < 0 ? -result : result);
}

static HRESULT apply_bounds(webview2_view_t *view)
{
    ICoreWebView2Controller3 *controller3 = NULL;
    RECT bounds;
    HRESULT hr;

    if (view->controller == NULL) {
        return S_OK;
    }
    bounds.left = scaled_coordinate(view->x, view->dpi);
    bounds.top = scaled_coordinate(view->y, view->dpi);
    bounds.right = scaled_coordinate((int64_t)view->x + view->width, view->dpi);
    bounds.bottom = scaled_coordinate((int64_t)view->y + view->height, view->dpi);
    if (SUCCEEDED(ICoreWebView2Controller_QueryInterface(
                      view->controller,
                      &IID_ICoreWebView2Controller3,
                      (void **)&controller3))) {
        hr = ICoreWebView2Controller3_put_BoundsMode(
            controller3, COREWEBVIEW2_BOUNDS_MODE_USE_RAW_PIXELS);
        if (SUCCEEDED(hr)) {
            hr = ICoreWebView2Controller3_put_ShouldDetectMonitorScaleChanges(
                controller3, FALSE);
        }
        if (SUCCEEDED(hr)) {
            hr = ICoreWebView2Controller3_put_RasterizationScale(
                controller3, (double)view->dpi / 96.0);
        }
        ICoreWebView2Controller3_Release(controller3);
        if (FAILED(hr)) {
            return hr;
        }
    }
    return ICoreWebView2Controller_put_Bounds(view->controller, bounds);
}

static webview2_backend_t *find_binding(ui_web_backend_t *backend,
                                        webview2_binding_t **previous)
{
    webview2_binding_t *it = bindings;
    webview2_binding_t *last = NULL;
    while (it != NULL) {
        if (it->public_backend == backend) {
            if (previous != NULL) {
                *previous = last;
            }
            return it->state;
        }
        last = it;
        it = it->next;
    }
    if (previous != NULL) {
        *previous = NULL;
    }
    return NULL;
}

static wchar_t *utf8_to_wide(const char *text)
{
    int length;
    wchar_t *result;

    if (text == NULL) {
        return NULL;
    }
    length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                 text, -1, NULL, 0);
    if (length <= 0) {
        return NULL;
    }
    result = (wchar_t *)calloc((size_t)length, sizeof(*result));
    if (result == NULL) {
        return NULL;
    }
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                            text, -1, result, length) != length) {
        free(result);
        return NULL;
    }
    return result;
}

static char *wide_to_utf8(const wchar_t *text)
{
    int length;
    char *result;
    if (text == NULL) {
        return NULL;
    }
    length = WideCharToMultiByte(CP_UTF8, 0, text, -1, NULL, 0, NULL, NULL);
    if (length <= 0) {
        return NULL;
    }
    result = (char *)malloc((size_t)length);
    if (result != NULL) {
        (void)WideCharToMultiByte(CP_UTF8, 0, text, -1, result,
                                  length, NULL, NULL);
    }
    return result;
}

static webview2_view_t *find_view(ui_web_view_t *view)
{
    webview2_binding_t *binding;
    webview2_view_t *candidate;
    if (view == NULL) {
        return NULL;
    }
    for (binding = bindings; binding != NULL; binding = binding->next) {
        for (candidate = binding->state->views; candidate != NULL;
             candidate = candidate->backend_next) {
            if (((struct ui_web_view *)view)->user_data == candidate) {
                return candidate;
            }
        }
    }
    return NULL;
}

static HRESULT STDMETHODCALLTYPE navigation_query_interface(
    ICoreWebView2NavigationCompletedEventHandler *self,
    REFIID riid,
    void **object)
{
    if (object == NULL) {
        return E_POINTER;
    }
    *object = NULL;
    if (IsEqualIID(riid, &IID_IUnknown) ||
        IsEqualIID(riid, &IID_ICoreWebView2NavigationCompletedEventHandler)) {
        *object = self;
        self->lpVtbl->AddRef(self);
        return S_OK;
    }
    return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE navigation_add_ref(
    ICoreWebView2NavigationCompletedEventHandler *self)
{
    return (ULONG)InterlockedIncrement(&((navigation_handler_t *)self)->refs);
}

static ULONG STDMETHODCALLTYPE navigation_release(
    ICoreWebView2NavigationCompletedEventHandler *self)
{
    navigation_handler_t *handler = (navigation_handler_t *)self;
    LONG refs = InterlockedDecrement(&handler->refs);
    if (refs == 0) {
        free(handler);
    }
    return (ULONG)refs;
}

static HRESULT STDMETHODCALLTYPE navigation_completed(
    ICoreWebView2NavigationCompletedEventHandler *self,
    ICoreWebView2 *sender,
    ICoreWebView2NavigationCompletedEventArgs *args)
{
    navigation_handler_t *handler = (navigation_handler_t *)self;
    BOOL success = FALSE;
    UINT64 navigation_id = 0;
    (void)sender;
    if (handler->view != NULL) {
        (void)ICoreWebView2NavigationCompletedEventArgs_get_NavigationId(args, &navigation_id);
        if (navigation_id != handler->view->navigation_id) return S_OK;
        if (SUCCEEDED(ICoreWebView2NavigationCompletedEventArgs_get_IsSuccess(
                          args, &success)) && success) {
            handler->view->navigation_completed = 1;
        } else {
            COREWEBVIEW2_WEB_ERROR_STATUS web_error = COREWEBVIEW2_WEB_ERROR_STATUS_UNKNOWN;
            (void)ICoreWebView2NavigationCompletedEventArgs_get_WebErrorStatus(args, &web_error);
            if (web_error != COREWEBVIEW2_WEB_ERROR_STATUS_OPERATION_CANCELED) {
                handler->view->async_error = E_FAIL;
            }
        }
    }
    return S_OK;
}

static ICoreWebView2NavigationCompletedEventHandlerVtbl navigation_vtable = {
    navigation_query_interface,
    navigation_add_ref,
    navigation_release,
    navigation_completed
};

/* Event subscriptions hold a handler reference. The view unregisters them and
 * clears these back pointers before releasing its own reference. */
#define DEFINE_EVENT_UNKNOWN(prefix, type, iid) \
static HRESULT STDMETHODCALLTYPE prefix##_query(type *self, REFIID riid, void **object) \
{ \
    if (object == NULL) return E_POINTER; \
    *object = NULL; \
    if (IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &iid)) { \
        *object = self; self->lpVtbl->AddRef(self); return S_OK; \
    } \
    return E_NOINTERFACE; \
} \
static ULONG STDMETHODCALLTYPE prefix##_add_ref(type *self) \
{ return (ULONG)InterlockedIncrement(&((prefix##_handler_t *)self)->refs); } \
static ULONG STDMETHODCALLTYPE prefix##_release(type *self) \
{ \
    prefix##_handler_t *handler = (prefix##_handler_t *)self; \
    LONG refs = InterlockedDecrement(&handler->refs); \
    if (refs == 0) free(handler); \
    return (ULONG)refs; \
}

DEFINE_EVENT_UNKNOWN(message, ICoreWebView2WebMessageReceivedEventHandler,
                     IID_ICoreWebView2WebMessageReceivedEventHandler)
DEFINE_EVENT_UNKNOWN(navigation_starting, ICoreWebView2NavigationStartingEventHandler,
                     IID_ICoreWebView2NavigationStartingEventHandler)

static HRESULT STDMETHODCALLTYPE message_received(
    ICoreWebView2WebMessageReceivedEventHandler *self,
    ICoreWebView2 *sender,
    ICoreWebView2WebMessageReceivedEventArgs *args)
{
    webview2_view_t *view = ((message_handler_t *)self)->view;
    LPWSTR source = NULL, message = NULL;
    char *utf8 = NULL, *command, *params;
    size_t i, length;
    (void)sender;
    if (view == NULL || view->destroyed || !view->trusted_html) return S_OK;
    if (FAILED(ICoreWebView2WebMessageReceivedEventArgs_get_Source(args, &source)) ||
        source == NULL || wcscmp(source, L"about:blank") != 0) goto done;
    if (FAILED(ICoreWebView2WebMessageReceivedEventArgs_TryGetWebMessageAsString(
                   args, &message)) || message == NULL) goto done;
    utf8 = wide_to_utf8(message);
    if (utf8 == NULL || strncmp(utf8, "ui.invoke\n", 10u) != 0) goto done;
    command = utf8 + 10;
    params = strchr(command, '\n');
    if (params == NULL) goto done;
    *params++ = '\0';
    length = strlen(command);
    if (length == 0u || length > 255u) goto done;
    for (i = 0u; i < length; ++i) {
        char c = command[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '_' || c == '.' || c == '-')) goto done;
    }
    (void)ui_host_invoke(view->host, command, params, "webview2");
done:
    free(utf8);
    CoTaskMemFree(source);
    CoTaskMemFree(message);
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE navigation_starting(
    ICoreWebView2NavigationStartingEventHandler *self,
    ICoreWebView2 *sender,
    ICoreWebView2NavigationStartingEventArgs *args)
{
    webview2_view_t *view = ((navigation_starting_handler_t *)self)->view;
    LPWSTR uri = NULL;
    int allow = 0;
    (void)sender;
    if (view == NULL) return S_OK;
    if (SUCCEEDED(ICoreWebView2NavigationStartingEventArgs_get_Uri(args, &uri)) &&
        uri != NULL && view->trusted_html && view->allow_html_navigation &&
        wcsncmp(uri, L"data:text/html;charset=utf-8;base64,", 34u) == 0) {
        /* NavigateToString uses a data URI for NavigationStarting while the
         * resulting document's source/origin is about:blank. Permit exactly
         * the next navigation initiated by our load_html call. */
        allow = 1;
        view->allow_html_navigation = 0;
        (void)ICoreWebView2NavigationStartingEventArgs_get_NavigationId(
            args, &view->navigation_id);
    }
    if (!allow) {
        (void)ICoreWebView2NavigationStartingEventArgs_put_Cancel(args, TRUE);
    }
    CoTaskMemFree(uri);
    return S_OK;
}

static ICoreWebView2WebMessageReceivedEventHandlerVtbl message_vtable = {
    message_query, message_add_ref, message_release, message_received
};
static ICoreWebView2NavigationStartingEventHandlerVtbl starting_vtable = {
    navigation_starting_query, navigation_starting_add_ref,
    navigation_starting_release, navigation_starting
};

static HRESULT STDMETHODCALLTYPE bridge_query(
    ICoreWebView2AddScriptToExecuteOnDocumentCreatedCompletedHandler *self,
    REFIID riid, void **object)
{
    if (object == NULL) return E_POINTER;
    *object = NULL;
    if (IsEqualIID(riid, &IID_IUnknown) ||
        IsEqualIID(riid, &IID_ICoreWebView2AddScriptToExecuteOnDocumentCreatedCompletedHandler)) {
        *object = self;
        self->lpVtbl->AddRef(self);
        return S_OK;
    }
    return E_NOINTERFACE;
}
static ULONG STDMETHODCALLTYPE bridge_add_ref(
    ICoreWebView2AddScriptToExecuteOnDocumentCreatedCompletedHandler *self)
{
    return (ULONG)InterlockedIncrement(&((bridge_handler_t *)self)->refs);
}
static ULONG STDMETHODCALLTYPE bridge_release(
    ICoreWebView2AddScriptToExecuteOnDocumentCreatedCompletedHandler *self)
{
    bridge_handler_t *handler = (bridge_handler_t *)self;
    LONG refs = InterlockedDecrement(&handler->refs);
    if (refs == 0) {
        view_release(handler->view);
        free(handler);
    }
    return (ULONG)refs;
}
static HRESULT STDMETHODCALLTYPE bridge_completed(
    ICoreWebView2AddScriptToExecuteOnDocumentCreatedCompletedHandler *self,
    HRESULT error_code, LPCWSTR script_id)
{
    webview2_view_t *view = ((bridge_handler_t *)self)->view;
    (void)script_id;
    if (view->destroyed) return S_OK;
    view->async_error = error_code;
    if (FAILED(error_code)) return S_OK;
    view->bridge_installed = 1;
    if (view->pending_html != NULL) {
        view->allow_html_navigation = 1;
        view->async_error = ICoreWebView2_NavigateToString(view->webview,
                                                          view->pending_html);
        free(view->pending_html);
        view->pending_html = NULL;
    }
    return S_OK;
}
static ICoreWebView2AddScriptToExecuteOnDocumentCreatedCompletedHandlerVtbl bridge_vtable = {
    bridge_query, bridge_add_ref, bridge_release, bridge_completed
};

static HRESULT STDMETHODCALLTYPE script_query_interface(
    ICoreWebView2ExecuteScriptCompletedHandler *self,
    REFIID riid,
    void **object)
{
    if (object == NULL) {
        return E_POINTER;
    }
    *object = NULL;
    if (IsEqualIID(riid, &IID_IUnknown) ||
        IsEqualIID(riid, &IID_ICoreWebView2ExecuteScriptCompletedHandler)) {
        *object = self;
        self->lpVtbl->AddRef(self);
        return S_OK;
    }
    return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE script_add_ref(
    ICoreWebView2ExecuteScriptCompletedHandler *self)
{
    return (ULONG)InterlockedIncrement(&((script_handler_t *)self)->refs);
}

static ULONG STDMETHODCALLTYPE script_release(
    ICoreWebView2ExecuteScriptCompletedHandler *self)
{
    script_handler_t *handler = (script_handler_t *)self;
    LONG refs = InterlockedDecrement(&handler->refs);
    if (refs == 0) {
        view_release(handler->view);
        free(handler);
    }
    return (ULONG)refs;
}

static HRESULT STDMETHODCALLTYPE script_completed(
    ICoreWebView2ExecuteScriptCompletedHandler *self,
    HRESULT error_code,
    LPCWSTR result_json)
{
    script_handler_t *handler = (script_handler_t *)self;
    char *result = wide_to_utf8(result_json);
    if (!handler->view->destroyed && handler->callback != NULL) {
        handler->callback(hresult_status(error_code), result, handler->user_data);
    }
    free(result);
    return S_OK;
}

static ICoreWebView2ExecuteScriptCompletedHandlerVtbl script_vtable = {
    script_query_interface,
    script_add_ref,
    script_release,
    script_completed
};

static wchar_t *wide_dup(const wchar_t *text)
{
    size_t bytes;
    wchar_t *result;

    if (text == NULL) {
        return NULL;
    }
    bytes = (wcslen(text) + 1u) * sizeof(*text);
    result = (wchar_t *)malloc(bytes);
    if (result != NULL) {
        memcpy(result, text, bytes);
    }
    return result;
}

static ui_status_t hresult_status(HRESULT hr)
{
    if (SUCCEEDED(hr)) {
        return UI_STATUS_OK;
    }
    if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) ||
        hr == HRESULT_FROM_WIN32(ERROR_PRODUCT_UNINSTALLED)) {
        return UI_STATUS_UNSUPPORTED;
    }
    return UI_STATUS_PLATFORM_ERROR;
}

static HRESULT STDMETHODCALLTYPE environment_query_interface(
    ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler *self,
    REFIID riid,
    void **object)
{
    environment_handler_t *handler = (environment_handler_t *)self;

    if (object == NULL) {
        return E_POINTER;
    }
    *object = NULL;
    if (IsEqualIID(riid, &IID_IUnknown) ||
        IsEqualIID(riid,
                   &IID_ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler)) {
        *object = handler;
        handler->iface.lpVtbl->AddRef(self);
        return S_OK;
    }
    return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE environment_add_ref(
    ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler *self)
{
    environment_handler_t *handler = (environment_handler_t *)self;
    return (ULONG)InterlockedIncrement(&handler->refs);
}

static ULONG STDMETHODCALLTYPE environment_release(
    ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler *self)
{
    environment_handler_t *handler = (environment_handler_t *)self;
    LONG refs = InterlockedDecrement(&handler->refs);
    if (refs == 0) {
        view_release(handler->view);
        free(handler);
    }
    return (ULONG)refs;
}

static HRESULT STDMETHODCALLTYPE controller_query_interface(
    ICoreWebView2CreateCoreWebView2ControllerCompletedHandler *self,
    REFIID riid,
    void **object)
{
    controller_handler_t *handler = (controller_handler_t *)self;

    if (object == NULL) {
        return E_POINTER;
    }
    *object = NULL;
    if (IsEqualIID(riid, &IID_IUnknown) ||
        IsEqualIID(riid,
                   &IID_ICoreWebView2CreateCoreWebView2ControllerCompletedHandler)) {
        *object = handler;
        handler->iface.lpVtbl->AddRef(self);
        return S_OK;
    }
    return E_NOINTERFACE;
}

static ULONG STDMETHODCALLTYPE controller_add_ref(
    ICoreWebView2CreateCoreWebView2ControllerCompletedHandler *self)
{
    controller_handler_t *handler = (controller_handler_t *)self;
    return (ULONG)InterlockedIncrement(&handler->refs);
}

static ULONG STDMETHODCALLTYPE controller_release(
    ICoreWebView2CreateCoreWebView2ControllerCompletedHandler *self)
{
    controller_handler_t *handler = (controller_handler_t *)self;
    LONG refs = InterlockedDecrement(&handler->refs);
    if (refs == 0) {
        view_release(handler->view);
        free(handler);
    }
    return (ULONG)refs;
}

static HRESULT STDMETHODCALLTYPE controller_completed(
    ICoreWebView2CreateCoreWebView2ControllerCompletedHandler *self,
    HRESULT error_code,
    ICoreWebView2Controller *controller)
{
    controller_handler_t *handler = (controller_handler_t *)self;
    webview2_view_t *view = handler->view;
    bridge_handler_t *bridge;
    HRESULT hr;

    if (view->destroyed) {
        if (controller != NULL) ICoreWebView2Controller_Close(controller);
        return S_OK;
    }
    if (FAILED(error_code) || controller == NULL) {
        view->async_error = FAILED(error_code) ? error_code : E_FAIL;
        return S_OK;
    }

    view->controller = controller;
    ICoreWebView2Controller_AddRef(controller);
    hr = apply_bounds(view);
    if (FAILED(hr)) {
        view->async_error = hr;
        return S_OK;
    }
    ICoreWebView2Controller_put_IsVisible(controller, TRUE);
    hr = ICoreWebView2Controller_get_CoreWebView2(controller, &view->webview);
    if (FAILED(hr)) {
        view->async_error = hr;
        return S_OK;
    }
    view->navigation_handler = (navigation_handler_t *)calloc(
        1u, sizeof(*view->navigation_handler));
    if (view->navigation_handler == NULL) {
        view->async_error = E_OUTOFMEMORY;
        return S_OK;
    }
    view->navigation_handler->iface.lpVtbl = &navigation_vtable;
    view->navigation_handler->refs = 1;
    view->navigation_handler->view = view;
    hr = ICoreWebView2_add_NavigationCompleted(
        view->webview,
        &view->navigation_handler->iface,
        &view->navigation_token);
    if (FAILED(hr)) {
        view->async_error = hr;
        return S_OK;
    }
    view->message_handler = (message_handler_t *)calloc(1u, sizeof(*view->message_handler));
    view->starting_handler = (navigation_starting_handler_t *)calloc(
        1u, sizeof(*view->starting_handler));
    bridge = (bridge_handler_t *)calloc(1u, sizeof(*bridge));
    if (view->message_handler == NULL || view->starting_handler == NULL || bridge == NULL) {
        free(bridge);
        view->async_error = E_OUTOFMEMORY;
        return S_OK;
    }
    view->message_handler->iface.lpVtbl = &message_vtable;
    view->message_handler->refs = 1;
    view->message_handler->view = view;
    view->starting_handler->iface.lpVtbl = &starting_vtable;
    view->starting_handler->refs = 1;
    view->starting_handler->view = view;
    hr = ICoreWebView2_add_WebMessageReceived(view->webview,
        &view->message_handler->iface, &view->message_token);
    if (SUCCEEDED(hr)) hr = ICoreWebView2_add_NavigationStarting(view->webview,
        &view->starting_handler->iface, &view->starting_token);
    if (FAILED(hr)) {
        free(bridge);
        view->async_error = hr;
        return S_OK;
    }
    bridge->iface.lpVtbl = &bridge_vtable;
    bridge->refs = 1;
    bridge->view = view;
    view_add_ref(view);
    hr = ICoreWebView2_AddScriptToExecuteOnDocumentCreated(view->webview,
                                                          bridge_script, &bridge->iface);
    if (FAILED(hr)) view->async_error = hr;
    bridge->iface.lpVtbl->Release(&bridge->iface);
    return S_OK;
}

static ICoreWebView2CreateCoreWebView2ControllerCompletedHandlerVtbl
    controller_vtable = {
        controller_query_interface,
        controller_add_ref,
        controller_release,
        controller_completed
    };

static HRESULT STDMETHODCALLTYPE environment_completed(
    ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler *self,
    HRESULT error_code,
    ICoreWebView2Environment *environment)
{
    environment_handler_t *handler = (environment_handler_t *)self;
    controller_handler_t *controller_handler;
    HRESULT hr;

    if (handler->view->destroyed) return S_OK;
    if (FAILED(error_code) || environment == NULL) {
        handler->view->async_error = FAILED(error_code) ? error_code : E_FAIL;
        return S_OK;
    }

    handler->view->environment = environment;
    ICoreWebView2Environment_AddRef(environment);
    controller_handler = (controller_handler_t *)calloc(1u,
                                                         sizeof(*controller_handler));
    if (controller_handler == NULL) {
        handler->view->async_error = E_OUTOFMEMORY;
        return S_OK;
    }
    controller_handler->iface.lpVtbl = &controller_vtable;
    controller_handler->refs = 1;
    controller_handler->view = handler->view;
    view_add_ref(handler->view);
    hr = ICoreWebView2Environment_CreateCoreWebView2Controller(
        environment,
        handler->view->parent_window,
        &controller_handler->iface);
    if (FAILED(hr)) {
        handler->view->async_error = hr;
    }
    controller_handler->iface.lpVtbl->Release(&controller_handler->iface);
    return S_OK;
}

static ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandlerVtbl
    environment_vtable = {
        environment_query_interface,
        environment_add_ref,
        environment_release,
        environment_completed
    };

static ui_status_t webview2_create_view(void *backend_user_data,
                                        ui_host_t *host,
                                        void **view_user_data)
{
    webview2_backend_t *backend = (webview2_backend_t *)backend_user_data;
    environment_handler_t *handler;
    webview2_view_t *view;
    HRESULT hr;

    if (backend == NULL || host == NULL ||
        backend->thread_id != GetCurrentThreadId() ||
        view_user_data == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    *view_user_data = NULL;
    view = (webview2_view_t *)calloc(1u, sizeof(*view));
    if (view == NULL) {
        return UI_STATUS_OUT_OF_MEMORY;
    }
    view->backend = backend;
    view->host = host;
    backend_add_ref(backend);
    view->parent_window = host->native_parent != NULL ?
        (HWND)host->native_parent : backend->parent_window;
    if (!IsWindow(view->parent_window)) {
        backend_release(backend);
        free(view);
        return UI_STATUS_INVALID_ARGUMENT;
    }
    view->dpi = host->dpi;
    view->refs = 1;
    view->backend_next = backend->views;
    backend->views = view;
    handler = (environment_handler_t *)calloc(1u, sizeof(*handler));
    if (handler == NULL) {
        view_release(view);
        return UI_STATUS_OUT_OF_MEMORY;
    }
    handler->iface.lpVtbl = &environment_vtable;
    handler->refs = 1;
    handler->view = view;
    view_add_ref(view);
    hr = CreateCoreWebView2EnvironmentWithOptions(
        NULL,
        backend->user_data_folder,
        NULL,
        &handler->iface);
    handler->iface.lpVtbl->Release(&handler->iface);
    if (FAILED(hr)) {
        view_release(view);
        return hresult_status(hr);
    }
    *view_user_data = view;
    return UI_STATUS_OK;
}

static void webview2_destroy_view(void *backend_user_data, void *view_user_data)
{
    webview2_view_t *view = (webview2_view_t *)view_user_data;
    (void)backend_user_data;

    if (view == NULL) {
        return;
    }
    view->destroyed = 1;
    if (view->message_handler != NULL) {
        view->message_handler->view = NULL;
        if (view->webview != NULL) {
            ICoreWebView2_remove_WebMessageReceived(view->webview, view->message_token);
        }
        if (view->message_handler->iface.lpVtbl != NULL) {
            view->message_handler->iface.lpVtbl->Release(&view->message_handler->iface);
        } else free(view->message_handler);
        view->message_handler = NULL;
    }
    if (view->starting_handler != NULL) {
        view->starting_handler->view = NULL;
        if (view->webview != NULL) {
            ICoreWebView2_remove_NavigationStarting(view->webview, view->starting_token);
        }
        if (view->starting_handler->iface.lpVtbl != NULL) {
            view->starting_handler->iface.lpVtbl->Release(&view->starting_handler->iface);
        } else free(view->starting_handler);
        view->starting_handler = NULL;
    }
    if (view->navigation_handler != NULL) {
        view->navigation_handler->view = NULL;
        if (view->webview != NULL) {
            ICoreWebView2_remove_NavigationCompleted(view->webview,
                                                     view->navigation_token);
        }
        view->navigation_handler->iface.lpVtbl->Release(
            &view->navigation_handler->iface);
        view->navigation_handler = NULL;
    }
    if (view->controller != NULL) ICoreWebView2Controller_Close(view->controller);
    view_release(view);
}

static ui_status_t webview2_load_html(void *backend_user_data,
                                      void *view_user_data,
                                      const char *html_utf8)
{
    webview2_view_t *view = (webview2_view_t *)view_user_data;
    wchar_t *html;
    HRESULT hr;
    (void)backend_user_data;

    if (html_utf8 == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    if (view_status(view) != UI_STATUS_OK) {
        return view_status(view);
    }
    html = utf8_to_wide(html_utf8);
    if (html == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    view->trusted_html = 1;
    view->navigation_completed = 0;
    if (view->webview == NULL || !view->bridge_installed) {
        free(view->pending_html);
        view->pending_html = html;
        return UI_STATUS_OK;
    }
    view->allow_html_navigation = 1;
    hr = ICoreWebView2_NavigateToString(view->webview, html);
    free(html);
    return hresult_status(hr);
}

static ui_status_t webview2_set_rect(void *backend_user_data,
                                     void *view_user_data,
                                     const ui_rect_t *rect,
                                     uint32_t dpi)
{
    webview2_view_t *view = (webview2_view_t *)view_user_data;
    ui_rect_t old_rect;
    uint32_t old_dpi;
    HRESULT hr;
    (void)backend_user_data;
    if (rect == NULL || rect->width < 0 || rect->height < 0 || dpi == 0u) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    if (view_status(view) != UI_STATUS_OK) {
        return view_status(view);
    }
    old_rect.x = view->x; old_rect.y = view->y;
    old_rect.width = view->width; old_rect.height = view->height;
    old_dpi = view->dpi;
    view->x = rect->x; view->y = rect->y;
    view->width = rect->width;
    view->height = rect->height;
    view->dpi = dpi;
    if (view->controller == NULL) {
        return UI_STATUS_OK;
    }
    hr = apply_bounds(view);
    if (FAILED(hr)) {
        view->x = old_rect.x; view->y = old_rect.y;
        view->width = old_rect.width; view->height = old_rect.height;
        view->dpi = old_dpi;
        (void)apply_bounds(view);
    }
    return hresult_status(hr);
}

static ui_status_t webview2_resize(void *backend_user_data,
                                   void *view_user_data,
                                   int logical_width,
                                   int logical_height,
                                   uint32_t dpi)
{
    webview2_view_t *view = (webview2_view_t *)view_user_data;
    ui_rect_t rect;
    if (view == NULL) return UI_STATUS_INVALID_ARGUMENT;
    rect.x = view->x; rect.y = view->y;
    rect.width = logical_width; rect.height = logical_height;
    return webview2_set_rect(backend_user_data, view_user_data, &rect, dpi);
}

static ui_status_t webview2_dispatch_input(void *backend_user_data,
                                           void *view_user_data,
                                           const ui_input_event_t *event)
{
    (void)backend_user_data;
    (void)view_user_data;
    (void)event;
    /* WebView2 receives input through its child HWND automatically. */
    return UI_STATUS_UNSUPPORTED;
}

static ui_status_t webview2_invalidate(void *backend_user_data,
                                       void *view_user_data)
{
    webview2_view_t *view = (webview2_view_t *)view_user_data;
    (void)backend_user_data;
    if (view_status(view) != UI_STATUS_OK) {
        return view_status(view);
    }
    InvalidateRect(view->parent_window, NULL, FALSE);
    return UI_STATUS_OK;
}

static const ui_web_backend_ops_t backend_ops = {
    sizeof(ui_web_backend_ops_t),
    webview2_create_view,
    webview2_destroy_view,
    webview2_load_html,
    webview2_resize,
    webview2_dispatch_input,
    webview2_invalidate,
    NULL,
    webview2_set_rect
};

ui_status_t ui_webview2_runtime_status(void)
{
    LPWSTR version = NULL;
    HRESULT hr = GetAvailableCoreWebView2BrowserVersionString(NULL, &version);
    if (version != NULL) {
        CoTaskMemFree(version);
    }
    return hresult_status(hr);
}

ui_status_t ui_webview2_view_get_state(ui_web_view_t *view,
                                      int *ready,
                                      int *navigation_complete)
{
    webview2_view_t *state = find_view(view);
    if (state == NULL) {
        return UI_STATUS_NOT_FOUND;
    }
    if (ready != NULL) {
        *ready = state->webview != NULL && state->bridge_installed;
    }
    if (navigation_complete != NULL) {
        *navigation_complete = state->navigation_completed;
    }
    return view_status(state);
}

ui_status_t ui_webview2_view_execute_script(
    ui_web_view_t *view,
    const char *script_utf8,
    ui_webview2_script_callback_fn callback,
    void *user_data)
{
    webview2_view_t *state = find_view(view);
    script_handler_t *handler;
    wchar_t *script;
    HRESULT hr;
    if (state == NULL || script_utf8 == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    if (state->webview == NULL || view_status(state) != UI_STATUS_OK) {
        return UI_STATUS_PLATFORM_ERROR;
    }
    script = utf8_to_wide(script_utf8);
    if (script == NULL) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    handler = (script_handler_t *)calloc(1u, sizeof(*handler));
    if (handler == NULL) {
        free(script);
        return UI_STATUS_OUT_OF_MEMORY;
    }
    handler->iface.lpVtbl = &script_vtable;
    handler->refs = 1;
    handler->view = state;
    handler->callback = callback;
    handler->user_data = user_data;
    view_add_ref(state);
    hr = ICoreWebView2_ExecuteScript(state->webview,
                                     script,
                                     &handler->iface);
    free(script);
    handler->iface.lpVtbl->Release(&handler->iface);
    return hresult_status(hr);
}

ui_status_t ui_webview2_view_get_error(ui_web_view_t *view, int32_t *hresult)
{
    webview2_view_t *state = find_view(view);
    if (state == NULL || hresult == NULL) return UI_STATUS_INVALID_ARGUMENT;
    *hresult = (int32_t)state->async_error;
    return UI_STATUS_OK;
}

ui_status_t ui_webview2_view_get_bounds(ui_web_view_t *view, ui_rect_t *pixel_rect)
{
    webview2_view_t *state = find_view(view);
    RECT bounds;
    HRESULT hr;
    if (state == NULL || pixel_rect == NULL) return UI_STATUS_INVALID_ARGUMENT;
    if (view_status(state) != UI_STATUS_OK) return view_status(state);
    if (state->controller == NULL) return UI_STATUS_NOT_FOUND;
    hr = ICoreWebView2Controller_get_Bounds(state->controller, &bounds);
    if (FAILED(hr)) return hresult_status(hr);
    if ((int64_t)bounds.right - bounds.left > INT_MAX ||
        (int64_t)bounds.bottom - bounds.top > INT_MAX) return UI_STATUS_PLATFORM_ERROR;
    pixel_rect->x = bounds.left; pixel_rect->y = bounds.top;
    pixel_rect->width = bounds.right - bounds.left;
    pixel_rect->height = bounds.bottom - bounds.top;
    return UI_STATUS_OK;
}

ui_web_backend_t *ui_webview2_backend_create(
    const ui_webview2_backend_config_t *config,
    ui_status_t *status)
{
    webview2_backend_t *backend;
    ui_web_backend_desc_t desc;
    ui_web_backend_t *result;
    HRESULT hr;

    if (status != NULL) {
        *status = UI_STATUS_INVALID_ARGUMENT;
    }
    if (config == NULL || config->size < sizeof(*config)) {
        return NULL;
    }
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        if (status != NULL) {
            *status = UI_STATUS_PLATFORM_ERROR;
        }
        return NULL;
    }
    {
        ui_status_t runtime_status = ui_webview2_runtime_status();
        if (runtime_status != UI_STATUS_OK) {
            CoUninitialize();
            if (status != NULL) {
                *status = runtime_status;
            }
            return NULL;
        }
    }
    backend = (webview2_backend_t *)calloc(1u, sizeof(*backend));
    if (backend == NULL) {
        CoUninitialize();
        if (status != NULL) {
            *status = UI_STATUS_OUT_OF_MEMORY;
        }
        return NULL;
    }
    backend->parent_window = (HWND)config->parent_window;
    backend->com_initialized = hr == S_OK || hr == S_FALSE;
    backend->refs = 1;
    backend->thread_id = GetCurrentThreadId();
    backend->user_data_folder = wide_dup(config->user_data_folder);
    if (config->user_data_folder != NULL && backend->user_data_folder == NULL) {
        backend_release(backend);
        if (status != NULL) {
            *status = UI_STATUS_OUT_OF_MEMORY;
        }
        return NULL;
    }
    desc.size = sizeof(desc);
    desc.ops = &backend_ops;
    desc.user_data = backend;
    result = ui_web_backend_create(&desc);
    if (result == NULL) {
        backend_release(backend);
        if (status != NULL) {
            *status = UI_STATUS_OUT_OF_MEMORY;
        }
        return NULL;
    }
    {
        webview2_binding_t *binding = (webview2_binding_t *)calloc(1u,
                                                                    sizeof(*binding));
        if (binding == NULL) {
            ui_web_backend_destroy(result);
            backend_release(backend);
            if (status != NULL) {
                *status = UI_STATUS_OUT_OF_MEMORY;
            }
            return NULL;
        }
        binding->public_backend = result;
        binding->state = backend;
        binding->next = bindings;
        bindings = binding;
    }
    if (status != NULL) {
        *status = UI_STATUS_OK;
    }
    return result;
}

void ui_webview2_backend_destroy(ui_web_backend_t *backend)
{
    webview2_backend_t *state;
    webview2_binding_t *binding;
    webview2_binding_t *previous;
    if (backend == NULL) {
        return;
    }
    state = find_binding(backend, &previous);
    binding = NULL;
    if (state != NULL) {
        binding = previous == NULL ? bindings : previous->next;
        if (previous == NULL) {
            bindings = binding->next;
        } else {
            previous->next = binding->next;
        }
    }
    ui_web_backend_destroy(backend);
    if (state != NULL) {
        backend_release(state);
    }
    free(binding);
}
