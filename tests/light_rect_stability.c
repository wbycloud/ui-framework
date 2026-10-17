/* Public geometry contract: repeated placement must not send native moves;
 * actual HWND geometry, size/DPI changes and document reload remain effective. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include "ui_framework/light_web.h"
static HWND observed;
static unsigned changing,failures;
#define CHECK(x) do { if (!(x)) { ++failures; printf("rect line%d: %s\n",__LINE__,#x); } } while (0)
static LRESULT CALLBACK position_hook(int code,WPARAM wp,LPARAM lp)
{
    if(code>=0){CWPSTRUCT *m=(CWPSTRUCT *)lp;
        if(m->hwnd==observed&&m->message==WM_WINDOWPOSCHANGING)++changing;}
    return CallNextHookEx(NULL,code,wp,lp);
}
static void native_rect(ui_rect_t rect,uint32_t dpi)
{
    RECT actual;CHECK(GetWindowRect(observed,&actual));
    MapWindowPoints(NULL,GetParent(observed),(POINT *)&actual,2);
    CHECK(actual.left==MulDiv(rect.x,(int)dpi,96));
    CHECK(actual.top==MulDiv(rect.y,(int)dpi,96));
    CHECK(actual.right==MulDiv(rect.x+rect.width,(int)dpi,96));
    CHECK(actual.bottom==MulDiv(rect.y+rect.height,(int)dpi,96));
}
int main(void)
{
    ui_host_config_t hc={0};ui_light_web_config_t bc={0};ui_host_t *host;
    ui_web_backend_t *backend;ui_web_view_t *view;HHOOK hook;HWND root;
    CHECK(ui_framework_initialize()==UI_STATUS_OK);
    root=CreateWindowW(L"STATIC",L"Public geometry",WS_OVERLAPPEDWINDOW,20,20,800,600,NULL,NULL,NULL,NULL);CHECK(root);
    hc.size=sizeof(hc);hc.api_version=UI_FRAMEWORK_API_VERSION;hc.native_parent=root;host=ui_host_create(&hc);
    bc.size=sizeof(bc);bc.parent_hwnd=root;backend=ui_light_web_backend_create(&bc);view=ui_web_view_create(host,backend);
    CHECK(view);if(!view)return 1;
    CHECK(ui_web_view_load_html(view,"<body><input id='edit' value='0.002'><button id='next'>Next</button></body>")==UI_STATUS_OK);
    observed=(HWND)ui_web_view_native_handle(view);CHECK(observed);
    hook=SetWindowsHookExW(WH_CALLWNDPROC,position_hook,NULL,GetCurrentThreadId());CHECK(hook);
    for(uint32_t dpi=96;dpi<=192;dpi+=48){
        ui_rect_t rect={9,11,223,157};ui_element_presentation_t before={0},after={0};
        CHECK(ui_web_view_set_rect(view,&rect,dpi)==UI_STATUS_OK);native_rect(rect,dpi);
        before.size=sizeof(before);CHECK(ui_web_view_get_presentation(view,"edit",&before)==UI_STATUS_OK);
        changing=0;
        for(int i=0;i<100;++i)CHECK(ui_web_view_set_rect(view,&rect,dpi)==UI_STATUS_OK);
        printf("SAME_RECT dpi%u calls100 native-changing%u\n",dpi,changing);CHECK(changing==0);
        after.size=sizeof(after);CHECK(ui_web_view_get_presentation(view,"edit",&after)==UI_STATUS_OK);
        CHECK(before.rect.x==after.rect.x&&before.rect.y==after.rect.y&&before.rect.width==after.rect.width&&before.rect.height==after.rect.height);
        rect.x=30;rect.y=40;CHECK(ui_web_view_set_rect(view,&rect,dpi)==UI_STATUS_OK);native_rect(rect,dpi);
        CHECK(SetWindowPos(observed,NULL,1,2,10,20,SWP_NOZORDER|SWP_NOACTIVATE));
        CHECK(ui_web_view_set_rect(view,&rect,dpi)==UI_STATUS_OK);native_rect(rect,dpi);
        rect.width=320;CHECK(ui_web_view_set_rect(view,&rect,dpi)==UI_STATUS_OK);native_rect(rect,dpi);
    }
    CHECK(ui_web_view_load_html(view,"<body><input id='updated' value='new document'></body>")==UI_STATUS_OK);
    {ui_element_presentation_t p={0};p.size=sizeof(p);CHECK(ui_web_view_get_presentation(view,"updated",&p)==UI_STATUS_OK&&p.visible);}
    UnhookWindowsHookEx(hook);ui_web_view_destroy(view);ui_light_web_backend_destroy(backend);ui_host_destroy(host);DestroyWindow(root);
    printf("Light geometry stability: %u failures\n",failures);return failures?1:0;
}
