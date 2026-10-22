#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include <string.h>
#include "ui_framework/ui.h"
#include "ui_framework/light_web.h"
#include "ui_framework/webview2.h"
#include "ui_framework/menus.h"

static ui_host_t *host;
static ui_web_view_t *view;
static ui_rect_t button;
static int enters,leaves,failures,real_input;
#define CHECK(x) do{if(!(x)){printf("FAIL line%d %s\n",__LINE__,#x);++failures;}}while(0)
static void pump(DWORD ms)
{ULONGLONG end=GetTickCount64()+ms;do{MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}if(ms)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}while(GetTickCount64()<end);}
static void message(ui_web_view_t *v,const char *json,void *data)
{(void)v;(void)data;printf("BOUNDARY tick=%llu message=%s\n",GetTickCount64(),json);if(strstr(json,"button-enter")){ui_menu_anchor_t a={0};a.size=sizeof(a);a.rect=button;++enters;CHECK(ui_host_show_tooltip(host,&a,"Button tooltip survives movement over its child")==UI_STATUS_OK);}if(strstr(json,"button-leave")){++leaves;CHECK(ui_host_hide_tooltip(host)==UI_STATUS_OK);}}
static ui_status_t presentation(const char *id,ui_element_presentation_t *p)
{ULONGLONG end=GetTickCount64()+10000;ui_status_t s;do{p->size=sizeof(*p);s=ui_web_view_get_presentation(view,id,p);if(s!=UI_STATUS_PENDING)return s;pump(1);}while(GetTickCount64()<end);return s;}
static void move(int x,int y,const char *name)
{printf("PHASE tick=%llu name=%s\n",GetTickCount64(),name);if(real_input){POINT p={x,y};INPUT i={0};ClientToScreen((HWND)ui_web_view_native_handle(view),&p);i.type=INPUT_MOUSE;i.mi.dx=MulDiv(p.x,65535,GetSystemMetrics(SM_CXVIRTUALSCREEN)-1);i.mi.dy=MulDiv(p.y,65535,GetSystemMetrics(SM_CYVIRTUALSCREEN)-1);i.mi.dwFlags=MOUSEEVENTF_MOVE|MOUSEEVENTF_ABSOLUTE;CHECK(SendInput(1,&i,sizeof(i))==1);}else{ui_input_event_t e={0};e.size=sizeof(e);e.kind=UI_INPUT_POINTER_MOVE;e.x=x;e.y=y;CHECK(ui_web_view_dispatch_input(view,&e)==UI_STATUS_OK);}pump(240);printf("COUNTS enters=%d leaves=%d\n",enters,leaves);}
static int tip(void)
{ui_element_presentation_t p={0};p.size=sizeof(p);return ui_host_menu_get_presentation(host,"detail",&p)==UI_STATUS_OK&&p.visible;}
int main(int argc,char **argv)
{
    int runtime=argc>1&&!strcmp(argv[1],"runtime");HWND root=NULL;ui_host_config_t hc={0};ui_light_web_config_t lc={0};ui_webview2_backend_config_t wc={0};ui_web_backend_t *backend;ui_status_t status;ui_element_presentation_t inner={0};
    const char *html="<body style='margin:0;padding:20px'><button id='button' style='width:240px;height:60px;padding:12px'><span id='inner' style='width:180px;height:28px'>Child label</span></button><script>['button','inner'].forEach(function(id){var n=document.getElementById(id);n.addEventListener('mouseenter',function(){ui.postMessage({event:id+'-enter'})});n.addEventListener('mouseleave',function(){ui.postMessage({event:id+'-leave'})})})</script></body>";
    real_input=argc>2&&!strcmp(argv[2],"--real");setvbuf(stdout,NULL,_IONBF,0);CHECK(SUCCEEDED(CoInitializeEx(NULL,COINIT_APARTMENTTHREADED)));CHECK(ui_framework_initialize()==UI_STATUS_OK);
    if(real_input||runtime){root=CreateWindowW(L"STATIC",L"Public hover boundaries",WS_OVERLAPPEDWINDOW,100,100,600,360,NULL,NULL,NULL,NULL);CHECK(root);}
    hc.size=sizeof(hc);hc.api_version=UI_FRAMEWORK_API_VERSION;hc.native_parent=root;host=ui_host_create(&hc);CHECK(host);CHECK(ui_host_resize(host,580,320)==UI_STATUS_OK);
    if(runtime){
#ifdef UI_TOOLTIP_WEBVIEW2
        wc.size=sizeof(wc);wc.parent_window=root;wc.framework_components=1;backend=ui_webview2_backend_create(&wc,&status);CHECK(backend&&status==UI_STATUS_OK);
#else
        (void)wc;(void)status;return 2;
#endif
    }else{lc.size=sizeof(lc);lc.parent_hwnd=root;backend=ui_light_web_backend_create(&lc);CHECK(backend);}
    if(!backend)return 2;view=ui_web_view_create(host,backend);CHECK(view);CHECK(ui_web_view_set_message_callback(view,message,NULL)==UI_STATUS_OK);CHECK(ui_web_view_load_html(view,html)==UI_STATUS_OK);CHECK(ui_web_view_resize(view,580,320,96)==UI_STATUS_OK);CHECK(presentation("button",&inner)==UI_STATUS_OK);button=inner.rect;CHECK(presentation("inner",&inner)==UI_STATUS_OK&&inner.visible);
    if(real_input){CHECK(ui_web_view_native_handle(view)!=NULL);ShowWindow(root,SW_SHOW);SetWindowPos(root,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);SetForegroundWindow(root);}pump(100);
    move(400,220,"outside-start");move(button.x+3,button.y+3,"button-padding");CHECK(enters==1&&leaves==0&&tip());move(inner.clip.x+inner.clip.width/2,inner.clip.y+inner.clip.height/2,"inside-child");CHECK(enters==1&&leaves==0&&tip());move(400,220,"outside-end");CHECK(enters==1&&leaves==1&&!tip());
    move(inner.clip.x+inner.clip.width/2,inner.clip.y+inner.clip.height/2,"direct-child");CHECK(enters==2&&leaves==1&&tip());move(400,220,"outside-final");CHECK(enters==2&&leaves==2&&!tip());
    ui_web_view_destroy(view);ui_host_destroy(host);
#ifdef UI_TOOLTIP_WEBVIEW2
    if(runtime)ui_webview2_backend_destroy(backend);else
#endif
    ui_light_web_backend_destroy(backend);if(root)DestroyWindow(root);pump(200);CoUninitialize();printf("RESULT failures=%d enters=%d leaves=%d\n",failures,enters,leaves);return failures?1:0;
}
