#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui_framework/ui.h"
#include "ui_framework/components.h"
#include "ui_framework/menus.h"
#include "ui_framework/shell.h"
#include "ui_framework/webview2.h"
#include "ui_framework/theme.h"

/* Public C reproducer. Real SendInput is explicit; no business code or script. */
static ui_host_t *host;
static HWND root;
static unsigned dispatches;
static int failures;
static const char *phase="init";
#define CHECK(x) do{if(!(x)){printf("FAIL tick=%llu phase=%s line=%d %s\n",GetTickCount64(),phase,__LINE__,#x);++failures;}}while(0)
static LRESULT CALLBACK owner(HWND w,UINT m,WPARAM a,LPARAM b)
{intptr_t result;if(host&&ui_host_handle_message(host,w,m,a,b,&result)==UI_STATUS_OK)return result;return DefWindowProcW(w,m,a,b);}
static void pump(DWORD ms)
{ULONGLONG end=GetTickCount64()+ms;do{MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){++dispatches;TranslateMessage(&m);DispatchMessageW(&m);}if(ms)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}while(GetTickCount64()<end);}
static void mark(const char *name)
{POINT at;GetCursorPos(&at);phase=name;printf("PHASE tick=%llu name=%s cursor=%ld,%ld hit=%p focus=%p capture=%p dispatch=%u\n",GetTickCount64(),name,at.x,at.y,(void*)WindowFromPoint(at),(void*)GetFocus(),(void*)GetCapture(),dispatches);}
static void event(const char *id,const char *json,void *user)
{(void)user;printf("EVENT tick=%llu phase=%s id=%s json=%s\n",GetTickCount64(),phase,id,json);}
static void move_to(POINT p)
{INPUT input={0};input.type=INPUT_MOUSE;input.mi.dx=MulDiv(p.x-GetSystemMetrics(SM_XVIRTUALSCREEN),65535,GetSystemMetrics(SM_CXVIRTUALSCREEN)-1);input.mi.dy=MulDiv(p.y-GetSystemMetrics(SM_YVIRTUALSCREEN),65535,GetSystemMetrics(SM_CYVIRTUALSCREEN)-1);input.mi.dwFlags=MOUSEEVENTF_MOVE|MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_VIRTUALDESK;CHECK(SendInput(1,&input,sizeof(input))==1);}
static ui_status_t presentation(ui_component_t *c,const char *id,ui_element_presentation_t *p)
{ULONGLONG end=GetTickCount64()+10000;ui_status_t s;do{p->size=sizeof(*p);s=ui_component_get_presentation(c,id,p);if(s!=UI_STATUS_PENDING)return s;pump(1);}while(GetTickCount64()<end);return s;}
static int visible_tip(const char *label)
{ui_element_presentation_t p={0};ui_status_t s;POINT at;unsigned before=dispatches;p.size=sizeof(p);s=ui_host_menu_get_presentation(host,"detail",&p);GetCursorPos(&at);printf("TIP tick=%llu phase=%s check=%s status=%d visible=%d text=%s cursor=%ld,%ld focus=%p capture=%p dispatch=%u queryDispatch=%u\n",GetTickCount64(),phase,label,s,s==UI_STATUS_OK?p.visible:0,s==UI_STATUS_OK?p.text_utf8:"",at.x,at.y,(void*)GetFocus(),(void*)GetCapture(),dispatches,dispatches-before);return s==UI_STATUS_OK&&p.visible;}
static void hover_element(ui_component_t *form,const char *id)
{ui_element_presentation_t p={0};ui_rect_t rect={0};void *native;POINT at={0};ui_shell_t *shell=ui_host_get_shell(host);ui_content_slot_t *slot=ui_shell_get_content_slot(shell,NULL);CHECK(presentation(form,id,&p)==UI_STATUS_OK&&p.visible);CHECK(ui_content_slot_get_rect(slot,&rect)==UI_STATUS_OK);native=ui_content_slot_native_handle(slot);if(native){at.x=p.clip.x+p.clip.width/2;at.y=p.clip.y+p.clip.height/2;ClientToScreen((HWND)native,&at);}else{at.x=rect.x+p.clip.x+p.clip.width/2;at.y=rect.y+p.clip.y+p.clip.height/2;ClientToScreen(root,&at);}move_to((POINT){at.x,at.y+100});pump(60);move_to(at);pump(350);}
static void hover(ui_component_t *form)
{hover_element(form,"label-name");}
static LRESULT CALLBACK messages(int code,WPARAM wp,LPARAM lp)
{if(code>=0){CWPSTRUCT *m=(CWPSTRUCT*)lp;if(m->message==WM_SHOWWINDOW||m->message==WM_DESTROY||m->message==WM_MOUSELEAVE||m->message==WM_SETFOCUS||m->message==WM_KILLFOCUS){wchar_t cls[100];GetClassNameW(m->hwnd,cls,100);printf("MESSAGE tick=%llu phase=%s hwnd=%p class=%ls msg=%x wp=%llu lp=%lld\n",GetTickCount64(),phase,(void*)m->hwnd,cls,m->message,(unsigned long long)m->wParam,(long long)m->lParam);}}return CallNextHookEx(NULL,code,wp,lp);}
int main(int argc,char **argv)
{
    int runtime=argc>1&&!strcmp(argv[1],"runtime"),dark=argc>2&&!strcmp(argv[2],"dark");
    ui_host_config_t hc={0};ui_native_shell_config_t nc={0};ui_native_shell_t *native;
    ui_component_desc_t desc={0};ui_component_t *form,*status,*other;
    ui_field_desc_t field={0};ui_web_backend_t *backend=NULL;ui_status_t s;
    ui_webview2_backend_config_t wc={0};WNDCLASSW cls={0};HHOOK hook;
    setvbuf(stdout,NULL,_IONBF,0);CHECK(SUCCEEDED(CoInitializeEx(NULL,COINIT_APARTMENTTHREADED)));CHECK(ui_framework_initialize()==UI_STATUS_OK);
    cls.lpfnWndProc=owner;cls.hInstance=GetModuleHandleW(NULL);cls.lpszClassName=L"PublicTooltipLifecycle";cls.hCursor=LoadCursorW(NULL,IDC_ARROW);RegisterClassW(&cls);
    root=CreateWindowW(cls.lpszClassName,L"Public tooltip lifecycle",WS_OVERLAPPEDWINDOW,100,100,900,600,NULL,NULL,cls.hInstance,NULL);CHECK(root);
    hc.size=sizeof(hc);hc.api_version=UI_FRAMEWORK_API_VERSION;hc.native_parent=root;hc.event_callback=event;host=ui_host_create(&hc);CHECK(host);if(!host)return 2;
    CHECK(ui_host_resize(host,880,560)==UI_STATUS_OK);CHECK(ui_host_set_theme(host,dark?UI_THEME_DARK:UI_THEME_LIGHT)==UI_STATUS_OK);
    nc.size=sizeof(nc);nc.host=host;nc.native_parent=root;nc.menu_owner=root;native=ui_native_shell_create_web(&nc);CHECK(native);if(!native)return 2;
    if(runtime){
#ifdef UI_TOOLTIP_WEBVIEW2
        wc.size=sizeof(wc);wc.framework_components=1;backend=ui_webview2_backend_create(&wc,&s);CHECK(backend&&s==UI_STATUS_OK);
#else
        (void)wc;(void)s;return 2;
#endif
    }
    field.size=sizeof(field);field.id="name";field.title="Hover target label";field.kind=UI_VALUE_TEXT;field.help="Target help remains visible during unrelated updates";
    desc.size=sizeof(desc);desc.id="owner";desc.title="Owner";desc.kind=UI_COMPONENT_FORM;desc.fields=&field;desc.field_count=1;desc.web_backend=backend;CHECK(ui_component_register(host,&desc,&form)==UI_STATUS_OK);CHECK(ui_component_mount(form,ui_shell_get_content_slot(ui_host_get_shell(host),NULL))==UI_STATUS_OK);
    memset(&desc,0,sizeof(desc));desc.size=sizeof(desc);desc.id="status";desc.title="Ready";desc.kind=UI_COMPONENT_STATUS;CHECK(ui_component_register(host,&desc,&status)==UI_STATUS_OK);desc.id="other";CHECK(ui_component_register(host,&desc,&other)==UI_STATUS_OK);
    ShowWindow(root,SW_SHOW);SetWindowPos(root,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);SetForegroundWindow(root);hook=SetWindowsHookExW(WH_CALLWNDPROC,messages,NULL,GetCurrentThreadId());pump(200);
    mark("idle-hover");hover(form);CHECK(visible_tip("entered"));pump(600);CHECK(visible_tip("idle600"));
    mark("unrelated-identical");CHECK(ui_component_set_text(status,"Ready")==UI_STATUS_OK);CHECK(visible_tip("same-immediate"));pump(300);CHECK(visible_tip("same-dispatched"));
    mark("unrelated-different");hover(form);CHECK(visible_tip("entered"));CHECK(ui_component_set_text(status,"Busy")==UI_STATUS_OK);CHECK(visible_tip("different-immediate"));pump(300);CHECK(visible_tip("different-dispatched"));
    mark("unrelated-destroy");hover(form);CHECK(visible_tip("entered"));CHECK(ui_component_unregister(other)==UI_STATUS_OK);CHECK(visible_tip("unregister-immediate"));pump(300);CHECK(visible_tip("unregister-dispatched"));
    mark("owner-update");hover(form);CHECK(visible_tip("entered"));{ui_field_text_t t={0};t.size=sizeof(t);t.id="name";t.title="Updated owner label";CHECK(ui_component_set_field_text(form,&t)==UI_STATUS_OK);}CHECK(!visible_tip("owner-invalidated"));pump(300);
    mark("owner-error-update");CHECK(ui_component_set_error(form,"name","Original error")==UI_STATUS_OK);hover_element(form,"error-name");CHECK(visible_tip("original-error"));CHECK(ui_component_set_error(form,"name","Updated error")==UI_STATUS_OK);CHECK(!visible_tip("error-invalidated"));pump(300);hover_element(form,"error-name");{ui_element_presentation_t p={0};p.size=sizeof(p);CHECK(ui_host_menu_get_presentation(host,"detail",&p)==UI_STATUS_OK&&p.visible&&!strcmp(p.text_utf8,"Updated error"));}
    mark("pointer-leave");hover(form);CHECK(visible_tip("entered"));move_to((POINT){1010,740});pump(300);CHECK(!visible_tip("leave-invalidated"));
    mark("owner-destroy");hover(form);CHECK(visible_tip("entered"));CHECK(ui_component_unregister(form)==UI_STATUS_OK);CHECK(!visible_tip("owner-gone"));pump(300);
    mark("public-host-tip");{ui_menu_anchor_t a={0};a.size=sizeof(a);a.rect=(ui_rect_t){20,20,180,30};CHECK(ui_host_show_tooltip(host,&a,"Host-owned toolbar-style tip")==UI_STATUS_OK);CHECK(visible_tip("public-shown"));CHECK(ui_component_set_text(status,"Working")==UI_STATUS_OK);CHECK(visible_tip("public-unrelated"));CHECK(ui_host_hide_tooltip(host)==UI_STATUS_OK);CHECK(!visible_tip("public-hidden"));}
    mark("window-close");{ui_menu_anchor_t a={0};a.size=sizeof(a);a.rect=(ui_rect_t){20,20,180,30};CHECK(ui_host_show_tooltip(host,&a,"Close owner")==UI_STATUS_OK);CHECK(visible_tip("pre-close"));ShowWindow(root,SW_HIDE);pump(300);CHECK(!visible_tip("hidden-owner"));}
    if(hook)UnhookWindowsHookEx(hook);ui_native_shell_destroy(native);ui_host_destroy(host);host=NULL;
#ifdef UI_TOOLTIP_WEBVIEW2
    if(backend)ui_webview2_backend_destroy(backend);
#endif
    DestroyWindow(root);pump(200);CoUninitialize();printf("RESULT failures=%d dispatch=%u\n",failures,dispatches);return failures?1:0;
}
