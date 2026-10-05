#define COBJMACROS

#include "ui_framework/webview2.h"
#include "../ui_internal.h"

#include <windows.h>
#include <wchar.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <wincodec.h>
#include "../json_ui.h"

#include <WebView2.h>

typedef struct webview2_backend webview2_backend_t;
typedef struct webview2_view webview2_view_t;
typedef struct webview2_binding webview2_binding_t;
typedef struct navigation_handler navigation_handler_t;
typedef struct message_handler message_handler_t;
typedef struct navigation_starting_handler navigation_starting_handler_t;
typedef struct resource_handler resource_handler_t;
typedef struct capture_handler {ICoreWebView2CapturePreviewCompletedHandler iface;LONG refs;webview2_view_t *view;IStream *stream;uint64_t epoch;} capture_handler_t;

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
    int drain_cancelled;
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
    uint64_t epoch;
    int drain_cancelled;
} script_handler_t;

typedef struct bridge_handler {
    ICoreWebView2AddScriptToExecuteOnDocumentCreatedCompletedHandler iface;
    LONG refs;
    webview2_view_t *view;
} bridge_handler_t;
typedef struct browser_exit_handler {ICoreWebView2BrowserProcessExitedEventHandler iface;LONG refs;webview2_backend_t *backend;} browser_exit_handler_t;

struct webview2_backend {
    HWND parent_window;
    wchar_t *user_data_folder;
    int com_initialized;
    LONG refs;
    DWORD thread_id;
    webview2_view_t *views;
    int framework_components;
    ICoreWebView2Environment *environment;
    int environment_creating;
    HWND cleanup_window;
    ICoreWebView2Environment5 *exit_environment;
    EventRegistrationToken exit_token;
    UINT32 browser_pid;
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
    int creation_deferred;
    int navigation_completed;
    int drain_attempts;
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
    ui_web_backend_message_fn message_callback;
    void *message_user_data;
    HWND container;
    char *pending_json,*query_id;
    uint64_t epoch,query_epoch,capture_epoch,render_epoch;
    int query_pending,query_ready,capture_pending,capture_ready,render_pending;
    ui_status_t query_status,capture_status;
    ui_element_presentation_t presentation;
    uint8_t *capture_pixels;
    uint32_t capture_width,capture_height;
    resource_handler_t *resource_handler;
    EventRegistrationToken resource_token;
};
struct resource_handler {ICoreWebView2WebResourceRequestedEventHandler iface;LONG refs;webview2_view_t *view;};
static void invalidate_render(webview2_view_t *v)
{++v->epoch;v->query_ready=v->capture_ready=0;if(v->capture_pending==1)v->capture_pending=0;free(v->capture_pixels);v->capture_pixels=NULL;}
static HRESULT begin_capture(webview2_view_t *);
static ui_status_t webview2_post_json(void *,void *,const char *);
static ui_status_t start_script(webview2_view_t *,const char *,ui_webview2_script_callback_fn,void *);
static ui_status_t start_view_creation(webview2_view_t *);
static const char drain_check_script[]="document.documentElement.id==='ui-runtime-drain'";

static webview2_binding_t *bindings;

static ui_status_t hresult_status(HRESULT hr);

#include "webview2_bridge.h"

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
    if(backend->cleanup_window)DestroyWindow(backend->cleanup_window);
    if(backend->environment)ICoreWebView2Environment_Release(backend->environment);
    if (backend->com_initialized) {
        CoUninitialize();
    }
    free(backend);
}

static void view_add_ref(webview2_view_t *view)
{
    (void)InterlockedIncrement(&view->refs);
}

static void free_view(webview2_view_t *view)
{
    webview2_view_t **it;
    if(GetEnvironmentVariableW(L"UI_RUNTIME_RESOURCES",NULL,0))fprintf(stderr,"Free Runtime view controller=%d bridge=%d navigation=%d id=%llu\n",view->controller!=NULL,view->bridge_installed,view->navigation_completed,(unsigned long long)view->navigation_id);
    it = &view->backend->views;
    while (*it != NULL && *it != view) {
        it = &(*it)->backend_next;
    }
    if (*it == view) {
        *it = view->backend_next;
    }
    /* A cancelled creation installs only an internal navigation observer. */
    if(view->starting_handler){view->starting_handler->view=NULL;
        if(view->webview)ICoreWebView2_remove_NavigationStarting(view->webview,view->starting_token);
        view->starting_handler->iface.lpVtbl->Release(&view->starting_handler->iface);view->starting_handler=NULL;}
    if(view->webview)ICoreWebView2_Stop(view->webview);
    if (view->controller != NULL) {
        ICoreWebView2Controller_Close(view->controller);
        ICoreWebView2Controller_Release(view->controller);
    }
    if (view->webview != NULL) {
        ICoreWebView2_Release(view->webview);
    }
    if (view->environment != NULL) {
        ICoreWebView2Environment_Release(view->environment);
    }
    free(view->pending_html);
    free(view->pending_json);free(view->query_id);free(view->capture_pixels);
    if(view->container)DestroyWindow(view->container);
    backend_release(view->backend);
    free(view);
}
static void view_release(webview2_view_t *view)
{
    if(InterlockedDecrement(&view->refs)!=0)return;
    /* Close outside an SDK completion/release stack. The backend reference
       keeps this UI-thread cleanup window alive until the inert view is gone. */
    if(!PostMessageW(view->backend->cleanup_window,WM_APP+51,0,(LPARAM)view))free_view(view);
}
static LRESULT CALLBACK cleanup_proc(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    if(message==WM_APP+51){free_view((webview2_view_t *)lp);return 0;}
    if(message==WM_APP+53){backend_release((webview2_backend_t *)lp);return 0;}
    if(message==WM_APP+54){webview2_view_t *view=(webview2_view_t *)lp;
        if(!view->destroyed){
            if(view->host->dispatch_blocked)view->creation_deferred=1;
            else{ui_status_t status=start_view_creation(view);if(status!=UI_STATUS_OK)view->async_error=E_FAIL;}
        }
        view_release(view);return 0;}
    if(message==WM_APP+52){webview2_view_t *view=(webview2_view_t *)lp;
        if(!view->destroyed&&view->pending_json){char *json=view->pending_json;view->pending_json=NULL;(void)webview2_post_json(NULL,view,json);free(json);}
        view_release(view);return 0;}
    return DefWindowProcW(window,message,wp,lp);
}

static ui_status_t view_status(webview2_view_t *view)
{
    if (view == NULL || view->destroyed) {
        return UI_STATUS_INVALID_ARGUMENT;
    }
    if (view->backend->thread_id != GetCurrentThreadId()) {
        return UI_STATUS_PLATFORM_ERROR;
    }
    if(view->creation_deferred&&!view->host->dispatch_blocked){
        view->creation_deferred=0;view_add_ref(view);
        if(!PostMessageW(view->backend->cleanup_window,WM_APP+54,0,(LPARAM)view)){view_release(view);view->async_error=E_FAIL;}
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
    if(view->container){if(!MoveWindow(view->container,bounds.left,bounds.top,bounds.right-bounds.left,bounds.bottom-bounds.top,TRUE))return E_FAIL;
        bounds.right-=bounds.left;bounds.bottom-=bounds.top;bounds.left=bounds.top=0;}
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
        if(handler->drain_cancelled)view_release(handler->view);
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
    if(handler->drain_cancelled){
        webview2_view_t *view=handler->view;
        /* Ignore the controller's initial about:blank completion. Only the
         * inert navigation we started proves that renderer startup drained. */
        if(view->navigation_handler!=handler||!view->navigation_id||FAILED(ICoreWebView2NavigationCompletedEventArgs_get_NavigationId(args,&navigation_id))||navigation_id!=view->navigation_id)return S_OK;
        (void)ICoreWebView2NavigationCompletedEventArgs_get_IsSuccess(args,&success);
        if(GetEnvironmentVariableW(L"UI_RUNTIME_RESOURCES",NULL,0))fprintf(stderr,"Drain navigation id=%llu success=%d\n",(unsigned long long)navigation_id,success);
        view->navigation_handler=NULL;
        ICoreWebView2_remove_NavigationCompleted(sender,view->navigation_token);
        /* A renderer acknowledgement on the completed inert document keeps
         * Close behind startup IPC, without entering unloaded app code. */
        (void)start_script(view,drain_check_script,NULL,NULL);
        self->lpVtbl->Release(self);
        return S_OK;
    }
    if (handler->view != NULL) {
        (void)ICoreWebView2NavigationCompletedEventArgs_get_NavigationId(args, &navigation_id);
        if (navigation_id != handler->view->navigation_id) return S_OK;
        if (SUCCEEDED(ICoreWebView2NavigationCompletedEventArgs_get_IsSuccess(
                          args, &success)) && success) {
            handler->view->navigation_completed = 1;
            /* A modal can focus its container before async controller creation.
             * Transfer that still-current focus once the document is ready. */
            if(handler->view->container&&GetFocus()==handler->view->container&&
               handler->view->host->app_active&&!handler->view->host->dispatch_blocked&&
               ui_components_input_allowed(handler->view->host,handler->view))
                ICoreWebView2Controller_MoveFocus(handler->view->controller,COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
            if(handler->view->pending_json){handler->view->render_pending=1;view_add_ref(handler->view);if(!PostMessageW(handler->view->backend->cleanup_window,WM_APP+52,0,(LPARAM)handler->view)){handler->view->async_error=HRESULT_FROM_WIN32(GetLastError());view_release(handler->view);}}
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
static void drain_cancelled_view(webview2_view_t *view);

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
DEFINE_EVENT_UNKNOWN(resource, ICoreWebView2WebResourceRequestedEventHandler,
                     IID_ICoreWebView2WebResourceRequestedEventHandler)
DEFINE_EVENT_UNKNOWN(browser_exit,ICoreWebView2BrowserProcessExitedEventHandler,IID_ICoreWebView2BrowserProcessExitedEventHandler)
static HRESULT STDMETHODCALLTYPE browser_exited(ICoreWebView2BrowserProcessExitedEventHandler *self,ICoreWebView2Environment *sender,ICoreWebView2BrowserProcessExitedEventArgs *args)
{
 browser_exit_handler_t *handler=(browser_exit_handler_t *)self;webview2_backend_t *backend=handler->backend;ICoreWebView2Environment5 *env;
 UINT32 pid=0;(void)sender;if(!backend)return S_OK;ICoreWebView2BrowserProcessExitedEventArgs_get_BrowserProcessId(args,&pid);
 if(backend->browser_pid&&pid!=backend->browser_pid)return S_OK;
 handler->backend=NULL;env=backend->exit_environment;backend->exit_environment=NULL;
 ICoreWebView2Environment5_remove_BrowserProcessExited(env,backend->exit_token);ICoreWebView2Environment5_Release(env);
 if(!PostMessageW(backend->cleanup_window,WM_APP+53,0,(LPARAM)backend))backend_release(backend);return S_OK;
}
static ICoreWebView2BrowserProcessExitedEventHandlerVtbl browser_exit_vtable={browser_exit_query,browser_exit_add_ref,browser_exit_release,browser_exited};
static void observe_browser_exit(webview2_backend_t *backend)
{
 browser_exit_handler_t *handler;HRESULT hr;
 if(FAILED(ICoreWebView2Environment_QueryInterface(backend->environment,&IID_ICoreWebView2Environment5,(void **)&backend->exit_environment)))return;
 handler=(browser_exit_handler_t *)calloc(1,sizeof(*handler));if(!handler){ICoreWebView2Environment5_Release(backend->exit_environment);backend->exit_environment=NULL;return;}
 handler->iface.lpVtbl=&browser_exit_vtable;handler->refs=1;handler->backend=backend;backend_add_ref(backend);
 hr=ICoreWebView2Environment5_add_BrowserProcessExited(backend->exit_environment,&handler->iface,&backend->exit_token);
 handler->iface.lpVtbl->Release(&handler->iface);
 if(FAILED(hr)){ICoreWebView2Environment5_Release(backend->exit_environment);backend->exit_environment=NULL;backend_release(backend);}
}

static HRESULT STDMETHODCALLTYPE resource_requested(ICoreWebView2WebResourceRequestedEventHandler *self,ICoreWebView2 *sender,ICoreWebView2WebResourceRequestedEventArgs *args)
{
 webview2_view_t *v=((resource_handler_t *)self)->view;ICoreWebView2WebResourceRequest *request=NULL;ICoreWebView2WebResourceResponse *response=NULL;LPWSTR uri=NULL;IStream *stream=NULL;HRESULT hr;ui_status_t status=UI_STATUS_NOT_FOUND;
 const wchar_t prefix[]=L"https://ui.framework.invalid/image/";(void)sender;
 if(!v||v->destroyed||!v->trusted_html)return S_OK;
 hr=ICoreWebView2WebResourceRequestedEventArgs_get_Request(args,&request);if(SUCCEEDED(hr))hr=ICoreWebView2WebResourceRequest_get_Uri(request,&uri);
 if(SUCCEEDED(hr)&&uri&&!wcsncmp(uri,prefix,wcslen(prefix))){wchar_t *end;uint64_t id=wcstoull(uri+wcslen(prefix),&end,10);if(*end==0||*end=='?')status=ui_image_png_stream(v->host,id,(void **)&stream);}
 hr=ICoreWebView2Environment_CreateWebResourceResponse(v->environment,stream,status==UI_STATUS_OK?200:404,status==UI_STATUS_OK?L"OK":L"Not Found",L"Content-Type: image/png\r\nCache-Control: no-store",&response);
 if(SUCCEEDED(hr))hr=ICoreWebView2WebResourceRequestedEventArgs_put_Response(args,response);
 if(response)ICoreWebView2WebResourceResponse_Release(response);if(stream)IStream_Release(stream);if(request)ICoreWebView2WebResourceRequest_Release(request);CoTaskMemFree(uri);return S_OK;
}
static ICoreWebView2WebResourceRequestedEventHandlerVtbl resource_vtable={resource_query,resource_add_ref,resource_release,resource_requested};

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
    if (utf8 == NULL) goto done;
    if(!strncmp(utf8,"ui.rendered\n",12)){if(strtoull(utf8+12,NULL,10)==view->render_epoch)view->render_pending=0;goto done;}
    if(!strncmp(utf8,"ui.capture\n",11)){uint64_t epoch=strtoull(utf8+11,NULL,10);if(view->capture_pending==1&&epoch==view->capture_epoch&&epoch==view->epoch){HRESULT hr=begin_capture(view);if(FAILED(hr)){view->capture_pending=0;view->capture_status=hresult_status(hr);view->capture_ready=1;}}goto done;}
    if(!strncmp(utf8,"ui.shortcut\n",12)){unsigned key,mods,editing;
        if(ui_components_input_allowed(view->host,view)&&view->host->app_active&&!view->host->dispatch_blocked&&sscanf(utf8+12,"%u %u %u",&key,&mods,&editing)==3&&key<=255&&mods<=7&&editing<=1)
            (void)ui_host_dispatch_shortcut(view->host,key,mods,(int)editing);
        goto done;}
    if (strncmp(utf8, "ui.json\n", 8u) == 0) {
        if(!ui_components_input_allowed(view->host,view)||view->host->dispatch_blocked||!view->host->app_active)goto done;
        invalidate_render(view);
        if (view->message_callback && strlen(utf8 + 8) <= 1048576u) {
            ui_dispatch_enter(view->host);
            view->message_callback(utf8 + 8, view->message_user_data);
            ui_dispatch_leave(view->host);
        }
        goto done;
    }
    if (strncmp(utf8, "ui.invoke\n", 10u) != 0) goto done;
    if(!ui_components_input_allowed(view->host,view)||view->host->dispatch_blocked||!view->host->app_active)goto done;
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
/* Cancellation can race either controller creation or the first navigation.
 * Both paths must finish renderer startup before closing the SDK object. */
static void drain_cancelled_view(webview2_view_t *view)
{
    navigation_handler_t *completion;HRESULT hr;
    if(!view->webview||view->navigation_completed)return;
    if(view->drain_attempts++>=3){view->async_error=E_FAIL;return;}
    if(view->starting_handler){
        ICoreWebView2_remove_NavigationStarting(view->webview,view->starting_token);
        view->starting_handler->iface.lpVtbl->Release(&view->starting_handler->iface);view->starting_handler=NULL;
    }
    completion=(navigation_handler_t *)calloc(1,sizeof(*completion));
    if(!completion)return;
    completion->iface.lpVtbl=&navigation_vtable;completion->refs=1;completion->view=view;completion->drain_cancelled=1;
    view_add_ref(view);view->navigation_handler=completion;view->navigation_id=0;
    hr=ICoreWebView2_add_NavigationCompleted(view->webview,&completion->iface,&view->navigation_token);
    if(SUCCEEDED(hr)){
        view->starting_handler=(navigation_starting_handler_t *)calloc(1,sizeof(*view->starting_handler));
        if(!view->starting_handler)hr=E_OUTOFMEMORY;
        else{view->starting_handler->iface.lpVtbl=&starting_vtable;view->starting_handler->refs=1;view->starting_handler->view=view;
            view->trusted_html=view->allow_html_navigation=1;
            hr=ICoreWebView2_add_NavigationStarting(view->webview,&view->starting_handler->iface,&view->starting_token);}
    }
    if(SUCCEEDED(hr)){ICoreWebView2_Stop(view->webview);hr=ICoreWebView2_NavigateToString(view->webview,L"<!doctype html><html id='ui-runtime-drain'></html>");}
    if(FAILED(hr)){
        ICoreWebView2_remove_NavigationCompleted(view->webview,view->navigation_token);
        view->navigation_handler=NULL;completion->iface.lpVtbl->Release(&completion->iface);
    }
}

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
    if(handler->drain_cancelled){
        if(SUCCEEDED(error_code)&&result&&!strcmp(result,"true"))handler->view->navigation_completed=1;
        else drain_cancelled_view(handler->view);
        if(GetEnvironmentVariableW(L"UI_RUNTIME_RESOURCES",NULL,0))fprintf(stderr,"Drain renderer document confirmed=%d attempts=%d hr=%08lx result=%s\n",handler->view->navigation_completed,handler->view->drain_attempts,error_code,result?result:"null");
    }
    if (!handler->view->destroyed && handler->callback != NULL) {
        handler->callback(handler->epoch==handler->view->epoch?hresult_status(error_code):UI_STATUS_CANCELLED,
                          handler->epoch==handler->view->epoch?result:NULL, handler->user_data);
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
    if(controller&&SUCCEEDED(error_code)){ICoreWebView2 *core=NULL;
        if(SUCCEEDED(ICoreWebView2Controller_get_CoreWebView2(controller,&core))){ICoreWebView2_get_BrowserProcessId(core,&view->backend->browser_pid);ICoreWebView2_Release(core);}
        if(!view->backend->exit_environment)observe_browser_exit(view->backend);}

    if (view->destroyed) {
        /* Creation may finish after the application has already unloaded.
         * Retain only the SDK object; deferred view cleanup closes it after
         * this completion stack has returned, without touching the host. */
        if (controller != NULL) {
            view->controller=controller;ICoreWebView2Controller_AddRef(controller);
            /* Runtime creation can complete before its initial renderer is
             * ready. Complete an inert navigation before deferred Close;
             * ExecuteScript on the initial document can leave startup work
             * retained. No application callback or document is installed. */
            if(SUCCEEDED(ICoreWebView2Controller_get_CoreWebView2(controller,&view->webview)))drain_cancelled_view(view);
        }
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
    if(SUCCEEDED(hr)){
        view->resource_handler=(resource_handler_t *)calloc(1,sizeof(*view->resource_handler));
        if(!view->resource_handler)hr=E_OUTOFMEMORY;
        else{view->resource_handler->iface.lpVtbl=&resource_vtable;view->resource_handler->refs=1;view->resource_handler->view=view;
            hr=ICoreWebView2_AddWebResourceRequestedFilter(view->webview,L"https://ui.framework.invalid/image/*",COREWEBVIEW2_WEB_RESOURCE_CONTEXT_IMAGE);
            if(SUCCEEDED(hr))hr=ICoreWebView2_add_WebResourceRequested(view->webview,&view->resource_handler->iface,&view->resource_token);}
    }
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

static HRESULT create_controller(webview2_view_t *view,ICoreWebView2Environment *environment)
{
    controller_handler_t *controller_handler;
    ICoreWebView2Environment10 *environment10=NULL;
    ICoreWebView2ControllerOptions *options=NULL;
    ICoreWebView2ControllerOptions4 *options4=NULL;
    HRESULT hr;
    view->environment = environment;
    ICoreWebView2Environment_AddRef(environment);
    controller_handler = (controller_handler_t *)calloc(1u,
                                                         sizeof(*controller_handler));
    if (controller_handler == NULL) {
        view->async_error = E_OUTOFMEMORY;
        return S_OK;
    }
    controller_handler->iface.lpVtbl = &controller_vtable;
    controller_handler->refs = 1;
    controller_handler->view = view;
    view_add_ref(view);
    if(view->backend->framework_components){
        hr=ICoreWebView2Environment_QueryInterface(environment,&IID_ICoreWebView2Environment10,(void **)&environment10);
        if(SUCCEEDED(hr))hr=ICoreWebView2Environment10_CreateCoreWebView2ControllerOptions(environment10,&options);
        if(SUCCEEDED(hr))hr=ICoreWebView2ControllerOptions_QueryInterface(options,&IID_ICoreWebView2ControllerOptions4,(void **)&options4);
        if(SUCCEEDED(hr))hr=ICoreWebView2ControllerOptions4_put_AllowHostInputProcessing(options4,TRUE);
        if(SUCCEEDED(hr))hr=ICoreWebView2Environment10_CreateCoreWebView2ControllerWithOptions(environment10,view->parent_window,options,&controller_handler->iface);
        if(options4)ICoreWebView2ControllerOptions4_Release(options4);
        if(options)ICoreWebView2ControllerOptions_Release(options);
        if(environment10)ICoreWebView2Environment10_Release(environment10);
    }else hr=ICoreWebView2Environment_CreateCoreWebView2Controller(environment,view->parent_window,&controller_handler->iface);
    if (FAILED(hr)) {
        view->async_error = hr;
    }
    controller_handler->iface.lpVtbl->Release(&controller_handler->iface);
    return S_OK;
}
static HRESULT STDMETHODCALLTYPE environment_completed(ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler *self,HRESULT error_code,ICoreWebView2Environment *environment)
{
    webview2_backend_t *backend=((environment_handler_t *)self)->view->backend;webview2_view_t *view;int creating_controller=0;
    backend->environment_creating=0;
    if(SUCCEEDED(error_code)&&environment){backend->environment=environment;ICoreWebView2Environment_AddRef(environment);}
    for(view=backend->views;view;view=view->backend_next)if(!view->destroyed&&!view->environment){
        if(backend->environment){(void)create_controller(view,backend->environment);creating_controller=1;}else view->async_error=FAILED(error_code)?error_code:E_FAIL;}
    /* A started environment still needs a controller Close to finish its
     * browser lifecycle when every view was cancelled during environment
     * creation. Only this handler's view has a guaranteed live reference.
     * Its completion sees destroyed and never enters application code. */
    if(backend->environment&&!creating_controller)(void)create_controller(((environment_handler_t *)self)->view,backend->environment);
    return S_OK;
}

static ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandlerVtbl
    environment_vtable = {
        environment_query_interface,
        environment_add_ref,
        environment_release,
        environment_completed
    };

static LRESULT CALLBACK container_proc(HWND hwnd,UINT message,WPARAM wp,LPARAM lp)
{webview2_view_t *v=(webview2_view_t *)GetWindowLongPtrW(hwnd,GWLP_USERDATA);
 if(message==WM_NCCREATE){v=(webview2_view_t *)((CREATESTRUCTW *)lp)->lpCreateParams;SetWindowLongPtrW(hwnd,GWLP_USERDATA,(LONG_PTR)v);}
 if(v&&!v->destroyed&&message==WM_SETFOCUS&&v->controller){ICoreWebView2Controller_MoveFocus(v->controller,COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);return 0;}
 if(message==WM_NCDESTROY)SetWindowLongPtrW(hwnd,GWLP_USERDATA,0);return DefWindowProcW(hwnd,message,wp,lp);}
static ui_status_t webview2_create_view(void *backend_user_data,
                                        ui_host_t *host,
                                        void **view_user_data)
{
    webview2_backend_t *backend = (webview2_backend_t *)backend_user_data;
    webview2_view_t *view;

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
    if(backend->framework_components){WNDCLASSW wc={0};wc.lpfnWndProc=container_proc;wc.lpszClassName=L"UIFrameworkWebView2Container5";wc.hInstance=GetModuleHandleW(NULL);
        if(!RegisterClassW(&wc)&&GetLastError()!=ERROR_CLASS_ALREADY_EXISTS){backend_release(backend);free(view);return UI_STATUS_PLATFORM_ERROR;}
        view->container=CreateWindowExW(0,wc.lpszClassName,L"",WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,0,0,1,1,view->parent_window,NULL,wc.hInstance,view);
        if(!view->container){backend_release(backend);free(view);return UI_STATUS_PLATFORM_ERROR;}view->parent_window=view->container;
    }
    view->dpi = host->dpi;
    view->refs = 1;
    view->backend_next = backend->views;
    backend->views = view;
    view_add_ref(view);
    if(!PostMessageW(backend->cleanup_window,WM_APP+54,0,(LPARAM)view)){
        view_release(view);view_release(view);return UI_STATUS_PLATFORM_ERROR;
    }
    *view_user_data=view;return UI_STATUS_OK;
}
/* Begin SDK work on the next STA turn. An application cancelled before that
 * turn needs no Runtime startup or synthetic controller solely for teardown. */
static ui_status_t start_view_creation(webview2_view_t *view)
{
    webview2_backend_t *backend=view->backend;
    environment_handler_t *handler;HRESULT hr;
    if(view->environment)return UI_STATUS_OK;
    if(backend->environment){(void)create_controller(view,backend->environment);return UI_STATUS_OK;}
    if(backend->environment_creating){return UI_STATUS_OK;}
    handler = (environment_handler_t *)calloc(1u, sizeof(*handler));
    if (handler == NULL) {
        return UI_STATUS_OUT_OF_MEMORY;
    }
    handler->iface.lpVtbl = &environment_vtable;
    handler->refs = 1;
    handler->view = view;
    view_add_ref(view);
    backend->environment_creating=1;
    hr = CreateCoreWebView2EnvironmentWithOptions(
        NULL,
        backend->user_data_folder,
        NULL,
        &handler->iface);
    handler->iface.lpVtbl->Release(&handler->iface);
    if (FAILED(hr)) {
        backend->environment_creating=0;
        return hresult_status(hr);
    }
    return UI_STATUS_OK;
}

static void webview2_destroy_view(void *backend_user_data, void *view_user_data)
{
    webview2_view_t *view = (webview2_view_t *)view_user_data;
    (void)backend_user_data;

    if (view == NULL) {
        return;
    }
    if(GetEnvironmentVariableW(L"UI_RUNTIME_RESOURCES",NULL,0))fprintf(stderr,"Destroy Runtime view controller=%d bridge=%d navigation=%d id=%llu\n",view->controller!=NULL,view->bridge_installed,view->navigation_completed,(unsigned long long)view->navigation_id);
    view->destroyed = 1;
    invalidate_render(view);
    if(view->resource_handler){view->resource_handler->view=NULL;if(view->webview)ICoreWebView2_remove_WebResourceRequested(view->webview,view->resource_token);view->resource_handler->iface.lpVtbl->Release(&view->resource_handler->iface);view->resource_handler=NULL;}
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
    drain_cancelled_view(view);
    /* Let in-flight SDK operations release their inert view before Close.
       Its app container may disappear immediately after this call. */
    if(view->container){HWND root=GetAncestor(view->container,GA_ROOT);ShowWindow(view->container,SW_HIDE);if(root&&root!=view->container)(void)SetParent(view->container,root);}
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
    invalidate_render(view);view->render_pending=0;
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
    if(rect->x==view->x&&rect->y==view->y&&rect->width==view->width&&rect->height==view->height&&dpi==view->dpi)return UI_STATUS_OK;
    invalidate_render(view);
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

static ui_status_t start_script(webview2_view_t *v,const char *script,ui_webview2_script_callback_fn callback,void *user)
{
 script_handler_t *h;wchar_t *wide;HRESULT hr;
 int drain=v->destroyed&&!strcmp(script,drain_check_script);
 if(!v->webview||(!v->navigation_completed&&!drain))return UI_STATUS_PENDING;
 wide=utf8_to_wide(script);if(!wide)return UI_STATUS_INVALID_ARGUMENT;h=(script_handler_t *)calloc(1,sizeof(*h));if(!h){free(wide);return UI_STATUS_OUT_OF_MEMORY;}
 h->iface.lpVtbl=&script_vtable;h->refs=1;h->view=v;h->epoch=v->epoch;h->callback=callback;h->user_data=user;h->drain_cancelled=drain;view_add_ref(v);
 hr=ICoreWebView2_ExecuteScript(v->webview,wide,&h->iface);free(wide);h->iface.lpVtbl->Release(&h->iface);return hresult_status(hr);
}
static void presentation_completed(ui_status_t status,const char *json,void *data)
{
 webview2_view_t *v=(webview2_view_t *)data;ui_element_presentation_t *p=&v->presentation;char value[40];v->query_pending=0;
 if(v->query_epoch!=v->epoch)return;v->query_status=status;v->query_ready=1;if(status!=UI_STATUS_OK)return;
 if(!json||!strcmp(json,"null")){v->query_status=UI_STATUS_NOT_FOUND;return;}memset(p,0,sizeof(*p));p->size=sizeof(*p);
#define NUMBER(key,field) if(!uj_get(json,key,value,sizeof(value))){v->query_status=UI_STATUS_VALIDATION_FAILED;return;}p->field=atoi(value)
 NUMBER("x",rect.x);NUMBER("y",rect.y);NUMBER("w",rect.width);NUMBER("h",rect.height);NUMBER("cx",clip.x);NUMBER("cy",clip.y);NUMBER("cw",clip.width);NUMBER("ch",clip.height);
 NUMBER("visible",visible);NUMBER("enabled",enabled);NUMBER("focused",focused);NUMBER("overflow",text_overflow);
#undef NUMBER
 if(!uj_get(json,"text",p->text_utf8,sizeof(p->text_utf8)))v->query_status=UI_STATUS_LIMIT_EXCEEDED;
}
static ui_status_t webview2_presentation(void *backend,void *data,const char *id,ui_element_presentation_t *out)
{
 webview2_view_t *v=(webview2_view_t *)data;ui_json_t script={0};ui_status_t status;(void)backend;
 status=view_status(v);if(status!=UI_STATUS_OK)return status;
 if(v->query_ready&&v->query_id&&!strcmp(id,v->query_id)){v->query_ready=0;if(v->query_status==UI_STATUS_OK)*out=v->presentation;return v->query_status;}
 if(v->query_pending||v->render_pending||!v->navigation_completed)return UI_STATUS_PENDING;
 free(v->query_id);v->query_id=ui_strdup(id);if(!v->query_id)return UI_STATUS_OUT_OF_MEMORY;
 uj_add(&script,"(function(){var e=document.getElementById(");uj_string(&script,id);
 uj_add(&script,");if(!e)return null;var r=e.getBoundingClientRect(),c={l:Math.max(0,r.left),t:Math.max(0,r.top),r:Math.min(innerWidth,r.right),b:Math.min(innerHeight,r.bottom)},visible=true,enabled=!e.disabled;for(var n=e;n;n=n.parentElement){var s=getComputedStyle(n),a=n.getBoundingClientRect();if(s.display==='none'||s.visibility==='hidden')visible=false;if(n.inert||n.getAttribute('aria-disabled')==='true')enabled=false;if(n!==e){if(s.overflowX!=='visible'){c.l=Math.max(c.l,a.left);c.r=Math.min(c.r,a.right);}if(s.overflowY!=='visible'){c.t=Math.max(c.t,a.top);c.b=Math.min(c.b,a.bottom);}}}return {x:Math.round(r.left),y:Math.round(r.top),w:Math.round(r.width),h:Math.round(r.height),cx:Math.ceil(c.l),cy:Math.ceil(c.t),cw:Math.max(0,Math.floor(c.r)-Math.ceil(c.l)),ch:Math.max(0,Math.floor(c.b)-Math.ceil(c.t)),visible:visible&&c.r>c.l&&c.b>c.t?1:0,enabled:enabled?1:0,focused:document.activeElement===e?1:0,overflow:e.scrollWidth>e.clientWidth||e.scrollHeight>e.clientHeight?1:0,text:/^(INPUT|TEXTAREA|SELECT)$/.test(e.tagName)?String(e.value):e.textContent};})()");
 if(script.failed){free(script.data);return UI_STATUS_LIMIT_EXCEEDED;}v->query_epoch=v->epoch;v->query_pending=1;status=start_script(v,script.data,presentation_completed,v);free(script.data);if(status!=UI_STATUS_OK){v->query_pending=0;return status;}return UI_STATUS_PENDING;
}
static HRESULT decode_capture(IStream *stream,webview2_view_t *v)
{
 IWICImagingFactory *factory=NULL;IWICBitmapDecoder *decoder=NULL;IWICBitmapFrameDecode *frame=NULL;IWICFormatConverter *converter=NULL;HRESULT hr;UINT width=0,height=0;uint8_t *pixels=NULL;LARGE_INTEGER zero={0};
 hr=IStream_Seek(stream,zero,STREAM_SEEK_SET,NULL);if(SUCCEEDED(hr))hr=CoCreateInstance(&CLSID_WICImagingFactory,NULL,CLSCTX_INPROC_SERVER,&IID_IWICImagingFactory,(void **)&factory);
 if(SUCCEEDED(hr))hr=IWICImagingFactory_CreateDecoderFromStream(factory,stream,NULL,WICDecodeMetadataCacheOnLoad,&decoder);
 if(SUCCEEDED(hr))hr=IWICBitmapDecoder_GetFrame(decoder,0,&frame);if(SUCCEEDED(hr))hr=IWICBitmapFrameDecode_GetSize(frame,&width,&height);
 if(SUCCEEDED(hr)&&(width!=v->capture_width||height!=v->capture_height||(uint64_t)width*height*4>32u*1024u*1024u))hr=E_FAIL;
 if(SUCCEEDED(hr))hr=IWICImagingFactory_CreateFormatConverter(factory,&converter);
 if(SUCCEEDED(hr))hr=IWICFormatConverter_Initialize(converter,(IWICBitmapSource *)frame,&GUID_WICPixelFormat32bppRGBA,WICBitmapDitherTypeNone,NULL,0,WICBitmapPaletteTypeCustom);
 if(SUCCEEDED(hr)){pixels=(uint8_t *)malloc((size_t)width*height*4);if(!pixels)hr=E_OUTOFMEMORY;}
 if(SUCCEEDED(hr))hr=IWICFormatConverter_CopyPixels(converter,NULL,width*4,width*height*4,pixels);
 if(SUCCEEDED(hr)){for(size_t i=3;i<(size_t)width*height*4;i+=4)pixels[i]=255;free(v->capture_pixels);v->capture_pixels=pixels;pixels=NULL;}
 free(pixels);if(converter)IWICFormatConverter_Release(converter);if(frame)IWICBitmapFrameDecode_Release(frame);if(decoder)IWICBitmapDecoder_Release(decoder);if(factory)IWICImagingFactory_Release(factory);return hr;
}
static HRESULT STDMETHODCALLTYPE capture_query(ICoreWebView2CapturePreviewCompletedHandler *self,REFIID iid,void **out)
{if(!out)return E_POINTER;*out=NULL;if(IsEqualIID(iid,&IID_IUnknown)||IsEqualIID(iid,&IID_ICoreWebView2CapturePreviewCompletedHandler)){*out=self;self->lpVtbl->AddRef(self);return S_OK;}return E_NOINTERFACE;}
static ULONG STDMETHODCALLTYPE capture_add_ref(ICoreWebView2CapturePreviewCompletedHandler *self){return (ULONG)InterlockedIncrement(&((capture_handler_t *)self)->refs);}
static ULONG STDMETHODCALLTYPE capture_release(ICoreWebView2CapturePreviewCompletedHandler *self)
{capture_handler_t *h=(capture_handler_t *)self;LONG refs=InterlockedDecrement(&h->refs);if(!refs){IStream_Release(h->stream);view_release(h->view);free(h);}return (ULONG)refs;}
static HRESULT STDMETHODCALLTYPE capture_completed(ICoreWebView2CapturePreviewCompletedHandler *self,HRESULT error)
{capture_handler_t *h=(capture_handler_t *)self;webview2_view_t *v=h->view;v->capture_pending=0;if(!v->destroyed&&v->epoch==h->epoch){v->capture_status=hresult_status(FAILED(error)?error:decode_capture(h->stream,v));v->capture_ready=1;}return S_OK;}
static ICoreWebView2CapturePreviewCompletedHandlerVtbl capture_vtable={capture_query,capture_add_ref,capture_release,capture_completed};
static HRESULT begin_capture(webview2_view_t *v)
{capture_handler_t *h=(capture_handler_t *)calloc(1,sizeof(*h));HRESULT hr;if(!h)return E_OUTOFMEMORY;hr=CreateStreamOnHGlobal(NULL,TRUE,&h->stream);if(FAILED(hr)){free(h);return hr;}
 h->iface.lpVtbl=&capture_vtable;h->refs=1;h->view=v;h->epoch=v->epoch;view_add_ref(v);v->capture_pending=2;hr=ICoreWebView2_CapturePreview(v->webview,COREWEBVIEW2_CAPTURE_PREVIEW_IMAGE_FORMAT_PNG,h->stream,&h->iface);h->iface.lpVtbl->Release(&h->iface);return hr;}
static ui_status_t webview2_capture(void *backend,void *data,ui_pixel_buffer_t *out)
{
 webview2_view_t *v=(webview2_view_t *)data;uint64_t width=((uint64_t)v->width*v->dpi+48)/96,height=((uint64_t)v->height*v->dpi+48)/96;size_t stride,bytes;char script[100];ui_status_t status;(void)backend;
 status=view_status(v);if(status!=UI_STATUS_OK)return status;if(!width||!height||width>16384||height>16384||width*height*4>32u*1024u*1024u)return UI_STATUS_LIMIT_EXCEEDED;
 stride=out->stride?out->stride:(size_t)width*4;if(stride<width*4||stride>SIZE_MAX/height)return UI_STATUS_INVALID_ARGUMENT;bytes=stride*(height-1)+(size_t)width*4;
 out->width=(uint32_t)width;out->height=(uint32_t)height;out->stride=stride;if(!out->pixels)return UI_STATUS_OK;if(out->capacity<bytes)return UI_STATUS_LIMIT_EXCEEDED;
 if(v->capture_ready){v->capture_ready=0;if(v->capture_status!=UI_STATUS_OK)return v->capture_status;for(size_t y=0;y<height;++y)memcpy(out->pixels+y*stride,v->capture_pixels+y*width*4,(size_t)width*4);free(v->capture_pixels);v->capture_pixels=NULL;return UI_STATUS_OK;}
 if(v->capture_pending||v->render_pending||!v->navigation_completed)return UI_STATUS_PENDING;
 v->capture_width=(uint32_t)width;v->capture_height=(uint32_t)height;v->capture_epoch=v->epoch;v->capture_pending=1;
 snprintf(script,sizeof(script),"ui.capture('%llu')",(unsigned long long)v->epoch);status=start_script(v,script,NULL,NULL);if(status!=UI_STATUS_OK){v->capture_pending=0;return status;}return UI_STATUS_PENDING;
}
static ui_status_t webview2_flush(void *backend,void *data,uint32_t budget)
{webview2_view_t *v=(webview2_view_t *)data;ui_status_t status=view_status(v);(void)backend;if(!budget)return UI_STATUS_INVALID_ARGUMENT;if(status!=UI_STATUS_OK)return status;return v->navigation_completed&&!v->render_pending&&!v->capture_pending&&!v->query_pending?UI_STATUS_OK:UI_STATUS_PENDING;}
static void webview2_image_changed(void *backend,void *data,uint64_t id)
{webview2_view_t *v=(webview2_view_t *)data;char script[160];(void)backend;if(view_status(v)!=UI_STATUS_OK)return;invalidate_render(v);snprintf(script,sizeof(script),"ui.refreshImage('%llu','%llu')",(unsigned long long)id,(unsigned long long)v->epoch);(void)start_script(v,script,NULL,NULL);}

static ui_status_t webview2_dispatch_input(void *backend_user_data,
                                           void *view_user_data,
                                           const ui_input_event_t *event)
{
    webview2_view_t *v=(webview2_view_t *)view_user_data;ui_json_t script={0};ui_status_t status;(void)backend_user_data;
    status=view_status(v);if(status!=UI_STATUS_OK)return status;
    if(!event||event->size<sizeof(*event)||event->kind<UI_INPUT_POINTER_MOVE||event->kind>UI_INPUT_CANCEL||(event->kind==UI_INPUT_TEXT&&!event->text_utf8))return UI_STATUS_INVALID_ARGUMENT;
    if(event->kind!=UI_INPUT_CANCEL&&(!ui_components_input_allowed(v->host,v)||v->host->dispatch_blocked||!v->host->app_active))return UI_STATUS_CANCELLED;
    if(!v->navigation_completed)return UI_STATUS_PENDING;
    if(ui_menus_route_input(v->host,v,event,0)==UI_STATUS_OK)return UI_STATUS_OK;
    invalidate_render(v);
    uj_fmt(&script,"ui.dispatchInput({kind:%d,x:%d,y:%d,key:%u,mods:%u,button:%u,delta:%d,epoch:'%llu',text:",event->kind,event->x,event->y,event->key_code,event->modifiers,event->pointer_button,event->wheel_delta,(unsigned long long)v->epoch);
    uj_string(&script,event->text_utf8?event->text_utf8:"");uj_add(&script,"})");
    status=script.failed?UI_STATUS_LIMIT_EXCEEDED:start_script(v,script.data,NULL,NULL);if(status==UI_STATUS_OK){v->render_pending=1;v->render_epoch=v->epoch;}free(script.data);return status;
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

static ui_status_t webview2_set_message_handler(void *backend_data, void *view_data,
    ui_web_backend_message_fn callback, void *user_data)
{
    webview2_view_t *view = (webview2_view_t *)view_data;
    (void)backend_data;
    if (!view || view->destroyed) return UI_STATUS_INVALID_ARGUMENT;
    view->message_callback = callback;
    view->message_user_data = user_data;
    return UI_STATUS_OK;
}

static ui_status_t webview2_post_json(void *backend_data, void *view_data,
                                     const char *json)
{
    webview2_view_t *view = (webview2_view_t *)view_data;
    wchar_t *wide;
    HRESULT hr;
    ui_json_t packet={0};
    (void)backend_data;
    if (!json || !view || view->destroyed) return UI_STATUS_INVALID_ARGUMENT;
    if(strlen(json)>1048576u)return UI_STATUS_LIMIT_EXCEEDED;
    if (view_status(view) != UI_STATUS_OK) return view_status(view);
    invalidate_render(view);
    if (!view->webview || !view->navigation_completed){char *copy=ui_strdup(json);if(!copy)return UI_STATUS_OUT_OF_MEMORY;free(view->pending_json);view->pending_json=copy;return UI_STATUS_OK;}
    uj_fmt(&packet,"{\"__ui_epoch\":\"%llu\",\"__ui_payload\":",(unsigned long long)view->epoch);uj_add(&packet,json);uj_add(&packet,"}");
    wide = packet.failed?NULL:utf8_to_wide(packet.data);free(packet.data);
    if (!wide) return UI_STATUS_INVALID_ARGUMENT;
    hr = ICoreWebView2_PostWebMessageAsJson(view->webview, wide);
    if(SUCCEEDED(hr)){view->render_pending=1;view->render_epoch=view->epoch;}
    free(wide);
    return hresult_status(hr);
}

static ui_status_t webview2_capabilities(void *backend_data, void *view_data,
                                        uint64_t *caps)
{
    (void)backend_data;
    if (!view_data || !caps) return UI_STATUS_INVALID_ARGUMENT;
    *caps = UI_WEB_CAP_JSON_MESSAGES | UI_WEB_CAP_DYNAMIC_DOM |
            UI_WEB_CAP_RESPONSIVE_LAYOUT | UI_WEB_CAP_TEXT_INPUT | UI_WEB_CAP_IMAGES |
            UI_WEB_CAP_WEB_TEXT_EDIT | UI_WEB_CAP_PRESENTATION_QUERY | UI_WEB_CAP_OFFSCREEN_CAPTURE | UI_WEB_CAP_ASYNC_RENDER;
    if(((webview2_view_t *)view_data)->backend->framework_components)*caps|=UI_WEB_CAP_NATIVE_WINDOW|UI_WEB_CAP_COMPONENTS;
    return UI_STATUS_OK;
}

static void *webview2_native_handle(void *backend,void *data){(void)backend;return ((webview2_view_t *)data)->container;}
static ui_status_t webview2_element_rect(void *backend,void *data,const char *id,ui_rect_t *rect)
{ui_element_presentation_t p={0};ui_status_t status;p.size=sizeof(p);status=webview2_presentation(backend,data,id,&p);if(status==UI_STATUS_OK)*rect=p.rect;return status;}
static const ui_web_backend_ops_t backend_ops = {
    sizeof(ui_web_backend_ops_t),
    webview2_create_view,
    webview2_destroy_view,
    webview2_load_html,
    webview2_resize,
    webview2_dispatch_input,
    webview2_invalidate,
    webview2_element_rect,
    webview2_set_rect,
    webview2_set_message_handler,
    webview2_post_json,
    webview2_capabilities,
    webview2_native_handle,
    webview2_image_changed,
    webview2_presentation,
    webview2_capture,
    webview2_flush
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
    handler->epoch = state->epoch;
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
    pixel_rect->x = state->container?MulDiv(state->x,(int)state->dpi,96):bounds.left; pixel_rect->y = state->container?MulDiv(state->y,(int)state->dpi,96):bounds.top;
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
    if (config == NULL || config->size < offsetof(ui_webview2_backend_config_t,framework_components)) {
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
    if(config->size>=offsetof(ui_webview2_backend_config_t,framework_components)+sizeof(config->framework_components))backend->framework_components=config->framework_components!=0;
    backend->com_initialized = hr == S_OK || hr == S_FALSE;
    backend->refs = 1;
    backend->thread_id = GetCurrentThreadId();
    {WNDCLASSW wc={0};wc.lpfnWndProc=cleanup_proc;wc.hInstance=GetModuleHandleW(NULL);wc.lpszClassName=L"UIFrameworkWebView2Cleanup5";
     if(!RegisterClassW(&wc)&&GetLastError()!=ERROR_CLASS_ALREADY_EXISTS){backend_release(backend);if(status)*status=UI_STATUS_PLATFORM_ERROR;return NULL;}
     backend->cleanup_window=CreateWindowW(wc.lpszClassName,L"",0,0,0,0,0,HWND_MESSAGE,NULL,wc.hInstance,NULL);
     if(!backend->cleanup_window){backend_release(backend);if(status)*status=UI_STATUS_PLATFORM_ERROR;return NULL;}}
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
