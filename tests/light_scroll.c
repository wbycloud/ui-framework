#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ui_framework/light_web.h"
static int failures;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"overflow line%d: %s\n",__LINE__,#x);++failures;}}while(0)
static void input(ui_web_view_t *v,ui_input_kind_t kind,int x,int y,unsigned key){ui_input_event_t e={0};e.size=sizeof(e);e.kind=kind;e.x=x;e.y=y;e.pointer_button=1;e.key_code=key;CHECK(ui_web_view_dispatch_input(v,&e)==UI_STATUS_OK);}
static void run(int windowed){ui_host_config_t hc={0};ui_light_web_config_t config={0};ui_host_t *h;ui_web_backend_t *backend;ui_web_view_t *v;ui_element_presentation_t p={0};ui_pixel_buffer_t pixels={0};char html[8192];
    HWND parent=windowed?CreateWindowW(L"STATIC",L"Overflow capture",WS_OVERLAPPEDWINDOW,0,0,400,300,NULL,NULL,NULL,NULL):NULL;config.parent_hwnd=parent;
    hc.size=sizeof(hc);hc.api_version=UI_FRAMEWORK_API_VERSION;h=ui_host_create(&hc);config.size=sizeof(config);backend=ui_light_web_backend_create(&config);v=ui_web_view_create(h,backend);CHECK(v!=NULL);if(!v)return;
    strcpy_s(html,sizeof(html),"<body style='padding:0'><div id='area' style='width:200px;height:120px;overflow:auto;background:#151b26;display:flex;flex-direction:column'>");
    for(int i=0;i<20;++i){char row[256];snprintf(row,sizeof(row),"<div id='r%d' style='height:30px;width:800px;position:relative'><div id='last%d' style='position:absolute;left:748px;width:40px;height:24px'>%d</div></div>",i,i,i);strcat_s(html,sizeof(html),row);}strcat_s(html,sizeof(html),"</div><div style='height:1px'></div></body>");
    CHECK(ui_web_view_load_html(v,html)==UI_STATUS_OK);CHECK(ui_web_view_resize(v,240,160,96)==UI_STATUS_OK);CHECK((ui_web_view_native_handle(v)!=NULL)==windowed);p.size=sizeof(p);CHECK(ui_web_view_get_presentation(v,"r19",&p)==UI_STATUS_OK&&!p.visible);
    input(v,UI_INPUT_POINTER_DOWN,194,10,0);input(v,UI_INPUT_POINTER_MOVE,194,300,0);CHECK(ui_web_view_get_presentation(v,"r19",&p)==UI_STATUS_OK&&p.visible);input(v,UI_INPUT_KEY_DOWN,0,0,VK_ESCAPE);CHECK(ui_web_view_get_presentation(v,"r0",&p)==UI_STATUS_OK&&p.visible&&p.rect.y==0);
    if(windowed){input(v,UI_INPUT_POINTER_DOWN,194,10,0);input(v,UI_INPUT_POINTER_MOVE,194,300,0);CHECK(GetCapture()==(HWND)ui_web_view_native_handle(v));ReleaseCapture();CHECK(ui_web_view_get_presentation(v,"r0",&p)==UI_STATUS_OK&&p.visible&&p.rect.y==0);}
    input(v,UI_INPUT_POINTER_DOWN,194,10,0);input(v,UI_INPUT_POINTER_MOVE,194,300,0);input(v,UI_INPUT_POINTER_UP,194,300,0);CHECK(ui_web_view_get_presentation(v,"r19",&p)==UI_STATUS_OK&&p.visible);
    input(v,UI_INPUT_POINTER_DOWN,12,114,0);input(v,UI_INPUT_POINTER_MOVE,400,114,0);input(v,UI_INPUT_POINTER_UP,400,114,0);CHECK(ui_web_view_get_presentation(v,"last19",&p)==UI_STATUS_OK&&p.visible&&p.clip.width>0);

    input(v,UI_INPUT_KEY_DOWN,0,0,VK_HOME);CHECK(ui_web_view_get_presentation(v,"last19",&p)==UI_STATUS_OK&&!p.visible);
    input(v,UI_INPUT_KEY_DOWN,0,0,VK_END);CHECK(ui_web_view_get_presentation(v,"last19",&p)==UI_STATUS_OK&&p.visible);
    input(v,UI_INPUT_POINTER_DOWN,194,10,0);input(v,UI_INPUT_POINTER_UP,194,10,0);input(v,UI_INPUT_KEY_DOWN,0,0,VK_HOME);CHECK(ui_web_view_get_presentation(v,"r0",&p)==UI_STATUS_OK&&p.visible);
    input(v,UI_INPUT_POINTER_DOWN,194,10,0);input(v,UI_INPUT_POINTER_MOVE,194,300,0);input(v,UI_INPUT_CANCEL,0,0,0);CHECK(ui_web_view_get_presentation(v,"r0",&p)==UI_STATUS_OK&&p.visible&&p.rect.y==0);
    input(v,UI_INPUT_POINTER_DOWN,194,10,0);input(v,UI_INPUT_POINTER_MOVE,194,300,0);input(v,UI_INPUT_POINTER_UP,194,300,0);
    pixels.size=sizeof(pixels);CHECK(ui_web_view_capture_rgba(v,240,160,144,&pixels)==UI_STATUS_OK);pixels.capacity=pixels.stride*pixels.height;pixels.pixels=malloc(pixels.capacity);CHECK(pixels.pixels!=NULL);if(pixels.pixels){CHECK(ui_web_view_capture_rgba(v,240,160,144,&pixels)==UI_STATUS_OK);free(pixels.pixels);}CHECK(ui_web_view_resize(v,240,160,192)==UI_STATUS_OK);CHECK(ui_web_view_get_presentation(v,"last19",&p)==UI_STATUS_OK&&p.visible);
    CHECK(ui_web_view_load_html(v,"<body><div style='width:200px;height:120px;overflow:auto'><button id='only'>Only</button></div></body>")==UI_STATUS_OK);CHECK(ui_web_view_get_presentation(v,"only",&p)==UI_STATUS_OK&&p.visible);ui_web_view_destroy(v);ui_light_web_backend_destroy(backend);ui_host_destroy(h);if(parent)DestroyWindow(parent);}
int main(void){run(0);run(1);printf("visible overflow bars windowed/windowless: %d failures\n",failures);return failures?1:0;}
