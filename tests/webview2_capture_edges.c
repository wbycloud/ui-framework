/* Public-only odd-origin / odd-size DPI capture regression, no business code. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
#include "ui_framework/webview2.h"
static int failures;
#define CHECK(x) do{if(!(x)){printf("capture edge line%d: %s\n",__LINE__,#x);++failures;}}while(0)
static void pump(void)
{MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}Sleep(1);}
int main(void)
{
    HWND root;ui_host_config_t hc={0};ui_webview2_backend_config_t config={0};ui_status_t status;ui_host_t *h;ui_web_backend_t *b;ui_web_view_t *v;wchar_t folder[4096];
    setvbuf(stdout,NULL,_IONBF,0);CHECK(SUCCEEDED(CoInitializeEx(NULL,COINIT_APARTMENTTHREADED)));CHECK(ui_framework_initialize()==UI_STATUS_OK);
    root=CreateWindowW(L"STATIC",L"Public capture edges",WS_OVERLAPPEDWINDOW,30,30,800,700,NULL,NULL,NULL,NULL);CHECK(root);
    hc.size=sizeof(hc);hc.api_version=UI_FRAMEWORK_API_VERSION;hc.native_parent=root;h=ui_host_create(&hc);CHECK(h);if(!h)return 1;
    config.size=sizeof(config);config.parent_window=root;config.framework_components=1;CHECK(GetCurrentDirectoryW(4000,folder)>0);wcscat_s(folder,4096,L"\\capture-edge-profile");config.user_data_folder=folder;b=ui_webview2_backend_create(&config,&status);CHECK(b&&status==UI_STATUS_OK);if(!b)return 1;
    v=ui_web_view_create(h,b);CHECK(v);if(!v)return 1;CHECK(ui_web_view_load_html(v,"<body style='margin:0;background:#123456'></body>")==UI_STATUS_OK);
    for(uint32_t dpi=96;dpi<=192;dpi+=48)for(int variant=0;variant<8;++variant){
        ui_rect_t rect={variant&1?5:0,variant&1?7:0,variant&2?224:223,variant&4?158:157},bounds={0};ui_pixel_buffer_t pixels={0};ULONGLONG end;
        CHECK(ui_web_view_set_rect(v,&rect,dpi)==UI_STATUS_OK);end=GetTickCount64()+10000;
        do{status=ui_web_view_flush(v,64);if(status==UI_STATUS_OK)break;pump();}while(GetTickCount64()<end);CHECK(status==UI_STATUS_OK);
        CHECK(ui_webview2_view_get_bounds(v,&bounds)==UI_STATUS_OK);pixels.size=sizeof(pixels);CHECK(ui_web_view_capture_rgba(v,rect.width,rect.height,dpi,&pixels)==UI_STATUS_OK);
        printf("EDGE dpi%u origin%d,%d native%d,%d,%d,%d query%u,%u stride%zu\n",dpi,rect.x,rect.y,bounds.x,bounds.y,bounds.width,bounds.height,pixels.width,pixels.height,pixels.stride);
        CHECK(pixels.width==(uint32_t)bounds.width&&pixels.height==(uint32_t)bounds.height);
        pixels.capacity=pixels.stride*pixels.height;pixels.pixels=malloc(pixels.capacity);CHECK(pixels.pixels);if(!pixels.pixels)continue;end=GetTickCount64()+10000;
        do{status=ui_web_view_capture_rgba(v,rect.width,rect.height,dpi,&pixels);if(status!=UI_STATUS_PENDING)break;pump();}while(GetTickCount64()<end);
        printf("CAPTURE dpi%u variant%d status%d\n",dpi,variant,(int)status);CHECK(status==UI_STATUS_OK);
        if(status==UI_STATUS_OK)for(uint32_t y=0;y<pixels.height;++y)for(uint32_t x=0;x<pixels.width;++x){size_t at=y*pixels.stride+x*4;CHECK(pixels.pixels[at]==0x12&&pixels.pixels[at+1]==0x34&&pixels.pixels[at+2]==0x56&&pixels.pixels[at+3]==255);}
        free(pixels.pixels);
    }
    ui_web_view_destroy(v);ui_host_destroy(h);ui_webview2_backend_destroy(b);DestroyWindow(root);{ULONGLONG end=GetTickCount64()+200;while(GetTickCount64()<end)pump();}CoUninitialize();printf("Public capture edges failures=%d\n",failures);return failures?1:0;
}
