#include "ui_framework/light_web.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
static char message[8192];static int failed;
#define CHECK(x) do{if(!(x)){printf("FAIL %d: %s\n",__LINE__,#x);++failed;}}while(0)
static void received(ui_web_view_t *v,const char *json,void *u){(void)v;(void)u;strcpy_s(message,sizeof(message),json);}
static void key(ui_web_view_t *v,uint32_t k,uint32_t mods)
{ui_input_event_t e={0};e.size=sizeof(e);e.kind=UI_INPUT_KEY_DOWN;e.key_code=k;e.modifiers=mods;CHECK(ui_web_view_dispatch_input(v,&e)==UI_STATUS_OK);}
static void insert(ui_web_view_t *v,const char *t)
{ui_input_event_t e={0};e.size=sizeof(e);e.kind=UI_INPUT_TEXT;e.text_utf8=t;CHECK(ui_web_view_dispatch_input(v,&e)==UI_STATUS_OK);}
static void snap(ui_web_view_t *v){CHECK(ui_web_view_post_json(v,"{}") == UI_STATUS_OK);}
int main(void)
{
    HWND parent,hwnd;ui_host_config_t hc={0};ui_light_web_config_t cfg={0};ui_host_t *h;ui_web_backend_t *backend;ui_web_view_t *v;ui_input_event_t e={0};ui_rect_t r;
    const char *page="<body><input id='single' value='seed'><textarea id='multi'>alpha\nbeta</textarea><input id='readonly' readonly value='fixed'><script>var inputs=0;document.getElementById('single').addEventListener('input',function(){inputs++});ui.onmessage=function(){ui.postMessage({single:document.getElementById('single').value,multi:document.getElementById('multi').value,inputs:inputs,focus:document.activeElement?document.activeElement.id:''})};</script></body>";
    CHECK(ui_framework_initialize()==UI_STATUS_OK);parent=CreateWindowExW(0,L"STATIC",L"Web text test",WS_OVERLAPPEDWINDOW,0,0,800,600,NULL,NULL,GetModuleHandleW(NULL),NULL);
    hc.size=sizeof(hc);hc.api_version=UI_FRAMEWORK_API_VERSION;hc.native_parent=parent;h=ui_host_create(&hc);cfg.size=sizeof(cfg);cfg.parent_hwnd=parent;cfg.enable_native_input=0;
    backend=ui_light_web_backend_create(&cfg);v=ui_web_view_create(h,backend);CHECK(v!=NULL);CHECK(ui_web_view_set_message_callback(v,received,NULL)==UI_STATUS_OK);CHECK(ui_web_view_load_html(v,page)==UI_STATUS_OK);CHECK(ui_web_view_resize(v,800,600,96)==UI_STATUS_OK);
    hwnd=(HWND)ui_web_view_native_handle(v);CHECK(FindWindowExW(hwnd,NULL,L"EDIT",NULL)==NULL);ShowWindow(parent,SW_SHOW);SetFocus(hwnd);
    CHECK(ui_web_view_get_element_rect(v,"single",&r)==UI_STATUS_OK);e.size=sizeof(e);e.kind=UI_INPUT_POINTER_DOWN;e.pointer_button=1;e.x=r.x+5;e.y=r.y+5;CHECK(ui_web_view_dispatch_input(v,&e)==UI_STATUS_OK);
    key(v,'A',UI_INPUT_MODIFIER_CONTROL);insert(v,"A😀é中文");key(v,VK_END,0);key(v,VK_LEFT,0);key(v,VK_LEFT,0);key(v,VK_BACK,0);snap(v);CHECK(strstr(message,"A😀中文")!=NULL); /* erase e + combining accent as a grapheme */
    key(v,'Z',UI_INPUT_MODIFIER_CONTROL);snap(v);CHECK(strstr(message,"A😀é中文")!=NULL);key(v,'Y',UI_INPUT_MODIFIER_CONTROL);snap(v);CHECK(strstr(message,"A😀中文")!=NULL);
    key(v,'A',UI_INPUT_MODIFIER_CONTROL);key(v,'C',UI_INPUT_MODIFIER_CONTROL);key(v,VK_END,0);key(v,'V',UI_INPUT_MODIFIER_CONTROL);snap(v);if(!strstr(message,"A😀中文A😀中文")){DWORD owner_pid=0;HWND owner=GetOpenClipboardWindow();GetWindowThreadProcessId(owner,&owner_pid);BOOL available=OpenClipboard(hwnd);DWORD error=available?0:GetLastError();printf("Clipboard diagnostic: open=%d error=%lu owner=%p owner_pid=%lu state=%s\n",available,error,(void *)owner,owner_pid,message);if(available)CloseClipboard();}CHECK(strstr(message,"A😀中文A😀中文")!=NULL);
    SendMessageW(hwnd,WM_CHAR,0x6d4b,0);SendMessageW(hwnd,WM_CHAR,0x8bd5,0);snap(v);CHECK(strstr(message,"测试")!=NULL); /* WM_CHAR is transport, not real IME evidence. */
    /* A valid in-bounds caret alone does not show that End exposed the tail. */
    key(v,'A',UI_INPUT_MODIFIER_CONTROL);insert(v,"START abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz TAIL");
    key(v,VK_END,0);CHECK(ui_web_view_get_element_rect(v,"single",&r)==UI_STATUS_OK);{POINT caret;CHECK(GetCaretPos(&caret));CHECK(caret.x>=r.x+r.width-20&&caret.x<r.x+r.width);}
    key(v,VK_HOME,0);{POINT caret;CHECK(GetCaretPos(&caret));CHECK(caret.x>=r.x&&caret.x<r.x+20);}
    key(v,VK_TAB,0);key(v,VK_END,UI_INPUT_MODIFIER_CONTROL);key(v,VK_RETURN,0);insert(v,"新行");snap(v);CHECK(strstr(message,"beta\\n新行")!=NULL);
    key(v,VK_TAB,0);e.kind=UI_INPUT_TEXT;e.text_utf8="ignored";CHECK(ui_web_view_dispatch_input(v,&e)==UI_STATUS_NOT_FOUND);snap(v);CHECK(strstr(message,"\"focus\":\"readonly\"")!=NULL);
    /* Reader scrolling must survive later hover/layout; compare actual text pixels. */
    CHECK(ui_web_view_load_html(v,"<body><textarea id='reader' readonly style='height:100px;width:220px;overflow:auto'>LINE 01\nLINE 02\nLINE 03\nLINE 04\nLINE 05\nLINE 06\nLINE 07\nLINE 08\nLINE 09\nFINAL LINE 10</textarea></body>")==UI_STATUS_OK);
    CHECK(ui_web_view_resize(v,240,160,96)==UI_STATUS_OK);CHECK(ui_web_view_get_element_rect(v,"reader",&r)==UI_STATUS_OK);
    e.kind=UI_INPUT_POINTER_DOWN;e.x=r.x+10;e.y=r.y+10;CHECK(ui_web_view_dispatch_input(v,&e)==UI_STATUS_OK);e.kind=UI_INPUT_POINTER_UP;CHECK(ui_web_view_dispatch_input(v,&e)==UI_STATUS_OK);key(v,VK_HOME,UI_INPUT_MODIFIER_CONTROL);
    {ui_pixel_buffer_t b={0};unsigned char *initial,*scrolled;int different=0;size_t bytes;
        b.size=sizeof(b);CHECK(ui_web_view_capture_rgba(v,240,160,96,&b)==UI_STATUS_OK);bytes=b.stride*b.height;b.capacity=bytes;b.pixels=malloc(bytes);initial=malloc(bytes);scrolled=malloc(bytes);CHECK(b.pixels&&initial&&scrolled);
        if(b.pixels&&initial&&scrolled){CHECK(ui_web_view_capture_rgba(v,240,160,96,&b)==UI_STATUS_OK);memcpy(initial,b.pixels,bytes);
            e.kind=UI_INPUT_WHEEL;e.wheel_delta=-1200;CHECK(ui_web_view_dispatch_input(v,&e)==UI_STATUS_OK);CHECK(ui_web_view_capture_rgba(v,240,160,96,&b)==UI_STATUS_OK);memcpy(scrolled,b.pixels,bytes);
            e.kind=UI_INPUT_POINTER_MOVE;e.x=235;e.y=150;CHECK(ui_web_view_dispatch_input(v,&e)==UI_STATUS_OK);CHECK(ui_web_view_capture_rgba(v,240,160,96,&b)==UI_STATUS_OK);
            printf("Reader viewport=%d,%d,%d,%d capture=%u,%u\n",r.x,r.y,r.width,r.height,b.width,b.height);
            for(int y=r.y+8;y<r.y+r.height-20&&y<(int)b.height;++y){size_t at=(size_t)y*b.stride+(size_t)(r.x+8)*4,size=(size_t)(r.width-40)*4;if(memcmp(initial+at,scrolled+at,size))different=1;CHECK(!memcmp(scrolled+at,b.pixels+at,size));}CHECK(different);
        }free(initial);free(scrolled);free(b.pixels);
    }
    CHECK(ui_web_view_resize(v,500,400,192)==UI_STATUS_OK);CHECK(GetFocus()==hwnd);ui_web_view_destroy(v);ui_light_web_backend_destroy(backend);ui_host_destroy(h);DestroyWindow(parent);
    printf("Web editor: %d failures. Physical Chinese IME remains unverified.\n",failed);return failed?1:0;
}
