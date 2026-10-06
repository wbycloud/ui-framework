#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui_framework/webview2.h"
#include "ui_framework/images.h"
#include "runtime_diagnostics.h"
static int failures,shortcuts;
static int runtime_cleanup_pending(void)
{HWND w=NULL;int count=0;while((w=FindWindowExW(HWND_MESSAGE,w,L"UIFrameworkWebView2Cleanup5",NULL))!=NULL)if(GetWindowThreadProcessId(w,NULL)==GetCurrentThreadId())++count;return count;}
static void shortcut(ui_host_t *h,uint64_t request,const char *id,const char *params,const char *origin,void *data)
{(void)id;(void)params;(void)origin;(void)data;++shortcuts;(void)ui_host_reply(h,request,1,"{}");}
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);++failures;}}while(0)
static void pump(void){MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}Sleep(1);}
static void ready(ui_web_view_t *v){ULONGLONG start=GetTickCount64();int r=0,n=0;while(GetTickCount64()-start<15000){pump();CHECK(ui_webview2_view_get_state(v,&r,&n)==UI_STATUS_OK);if(r&&n)return;}CHECK(r&&n);}
static ui_status_t query(ui_web_view_t *v,const char *id,ui_element_presentation_t *p)
{ULONGLONG start=GetTickCount64();ui_status_t s;do{s=ui_web_view_get_presentation(v,id,p);if(s!=UI_STATUS_PENDING)return s;pump();}while(GetTickCount64()-start<5000);return s;}
static ui_status_t capture(ui_web_view_t *v,ui_pixel_buffer_t *p)
{ULONGLONG start=GetTickCount64();ui_status_t s;do{s=ui_web_view_capture_rgba(v,200,100,144,p);if(s!=UI_STATUS_PENDING)return s;pump();}while(GetTickCount64()-start<5000);return s;}
int main(void)
{
 HWND root;ui_host_config_t hc={0};ui_webview2_backend_config_t cfg={0};ui_host_t *h;ui_web_backend_t *b;ui_web_view_t *v;ui_status_t s;ui_element_presentation_t p={0};ui_pixel_buffer_t pixels={0};ui_input_event_t input={0};ui_rgba_desc_t rgba={0};ui_image_id_t image;uint8_t color[4]={255,0,0,255};char html[1000];
 setvbuf(stdout,NULL,_IONBF,0);setvbuf(stderr,NULL,_IONBF,0);SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
 runtime_diagnostics_initialize();
 puts("Runtime render: initialize");CHECK(SUCCEEDED(CoInitializeEx(NULL,COINIT_APARTMENTTHREADED)));CHECK(ui_framework_initialize()==UI_STATUS_OK);CHECK(ui_webview2_runtime_status()==UI_STATUS_OK);
 root=CreateWindowW(L"STATIC",L"Real Runtime render",WS_OVERLAPPEDWINDOW,0,0,500,300,NULL,NULL,GetModuleHandleW(NULL),NULL);hc.size=sizeof(hc);hc.api_version=UI_FRAMEWORK_API_VERSION;hc.native_parent=root;h=ui_host_create(&hc);CHECK(h!=NULL);
 cfg.size=sizeof(cfg);cfg.framework_components=1;b=ui_webview2_backend_create(&cfg,&s);CHECK(b&&s==UI_STATUS_OK);v=ui_web_view_create(h,b);CHECK(v!=NULL);CHECK(ui_web_view_resize(v,200,100,144)==UI_STATUS_OK);
 rgba.size=sizeof(rgba);rgba.width=rgba.height=1;rgba.stride=rgba.bytes=4;rgba.pixels=color;CHECK(ui_image_create(h,&rgba,&image)==UI_STATUS_OK);
 snprintf(html,sizeof(html),"<style>body{margin:0;background:#102030}#box{width:80px;height:24px;background:#f00000;overflow:hidden;white-space:nowrap}</style><div id='box'>REAL Runtime 中文 visible text overflow long</div><input id='edit' value='draft 中文'><img id='pic' style='position:absolute;left:150px;top:50px;width:20px;height:20px' src='%llu'>",(unsigned long long)image);
 CHECK(ui_web_view_load_html(v,html)==UI_STATUS_OK);ShowWindow(root,SW_SHOW);ready(v);puts("Runtime render: ready");
 p.size=sizeof(p);CHECK(query(v,"box",&p)==UI_STATUS_OK&&p.visible&&p.rect.width==80&&p.text_overflow&&strstr(p.text_utf8,"REAL Runtime"));
 pixels.size=sizeof(pixels);CHECK(capture(v,&pixels)==UI_STATUS_OK&&pixels.width==300&&pixels.height==150);pixels.capacity=pixels.stride*pixels.height;pixels.pixels=(uint8_t *)malloc(pixels.capacity);CHECK(pixels.pixels!=NULL);
 CHECK(capture(v,&pixels)==UI_STATUS_OK);if(pixels.pixels&&pixels.capacity>16)CHECK(pixels.pixels[0]>200&&pixels.pixels[1]<30&&pixels.pixels[3]==255);
 {size_t at=80*pixels.stride+230*4;CHECK(pixels.pixels[at]==255&&pixels.pixels[at+1]==0);
  color[0]=0;color[1]=255;CHECK(ui_image_update(h,image,&rgba)==UI_STATUS_OK);CHECK(capture(v,&pixels)==UI_STATUS_OK);CHECK(pixels.pixels[at]==0&&pixels.pixels[at+1]==255);
  CHECK(ui_image_release(h,image)==UI_STATUS_OK);CHECK(capture(v,&pixels)==UI_STATUS_OK);CHECK(pixels.pixels[at+1]!=255);}
 puts("Runtime render: capture/images complete");CHECK(query(v,"edit",&p)==UI_STATUS_OK);input.size=sizeof(input);input.kind=UI_INPUT_POINTER_DOWN;input.pointer_button=1;input.x=p.clip.x+4;input.y=p.clip.y+4;CHECK(ui_web_view_dispatch_input(v,&input)==UI_STATUS_OK);input.kind=UI_INPUT_POINTER_UP;CHECK(ui_web_view_dispatch_input(v,&input)==UI_STATUS_OK);
 input.kind=UI_INPUT_KEY_DOWN;input.key_code='A';input.modifiers=UI_INPUT_MODIFIER_CONTROL;CHECK(ui_web_view_dispatch_input(v,&input)==UI_STATUS_OK);input.kind=UI_INPUT_TEXT;input.modifiers=0;input.text_utf8="C bridge 中文😀";CHECK(ui_web_view_dispatch_input(v,&input)==UI_STATUS_OK);
 CHECK(query(v,"edit",&p)==UI_STATUS_OK&&p.focused&&!strcmp(p.text_utf8,"C bridge 中文😀"));
 {ui_command_desc_t command={0};command.size=sizeof(command);command.id="test.open";command.title="Open";command.handler=shortcut;command.shortcut_key='O';command.shortcut_modifiers=UI_INPUT_MODIFIER_CONTROL;CHECK(ui_host_register_command(h,&command)==UI_STATUS_OK);input.kind=UI_INPUT_KEY_DOWN;input.key_code='O';input.modifiers=UI_INPUT_MODIFIER_CONTROL;CHECK(ui_web_view_dispatch_input(v,&input)==UI_STATUS_OK);CHECK(query(v,"edit",&p)==UI_STATUS_OK);CHECK(shortcuts==1);command.id="test.select";command.shortcut_key='A';CHECK(ui_host_register_command(h,&command)==UI_STATUS_OK);input.key_code='A';CHECK(ui_web_view_dispatch_input(v,&input)==UI_STATUS_OK);CHECK(query(v,"edit",&p)==UI_STATUS_OK&&shortcuts==1);}
 CHECK(ui_web_view_get_presentation(v,"box",&p)==UI_STATUS_PENDING);CHECK(ui_web_view_load_html(v,"<div id='box'>REPLACED</div>")==UI_STATUS_OK);ready(v);CHECK(query(v,"box",&p)==UI_STATUS_OK&&!strcmp(p.text_utf8,"REPLACED"));
 CHECK(ui_web_view_get_presentation(v,"box",&p)==UI_STATUS_PENDING);puts("Runtime render: close");free(pixels.pixels);ui_web_view_destroy(v);ui_webview2_backend_destroy(b);for(int i=0;i<100;++i)pump();
 {DWORD handles=0,last=0,users;ULONGLONG drain_start=GetTickCount64();while(runtime_cleanup_pending()&&GetTickCount64()-drain_start<15000)pump();CHECK(!runtime_cleanup_pending());
  users=GetGuiResources(GetCurrentProcess(),GR_USEROBJECTS);GetProcessHandleCount(GetCurrentProcess(),&handles);cfg.framework_components=1;
  for(int i=0;i<12;++i){b=ui_webview2_backend_create(&cfg,&s);CHECK(b&&s==UI_STATUS_OK);v=ui_web_view_create(h,b);CHECK(v!=NULL);CHECK(ui_web_view_load_html(v,"<div>cancel before creation</div>")==UI_STATUS_OK);ui_web_view_destroy(v);ui_webview2_backend_destroy(b);ULONGLONG start=GetTickCount64();do{pump();}while(GetTickCount64()-start<500);}
  {ULONGLONG start=GetTickCount64();do{GetProcessHandleCount(GetCurrentProcess(),&last);if(last<=handles+12&&GetGuiResources(GetCurrentProcess(),GR_USEROBJECTS)<=users+2&&!runtime_cleanup_pending())break;pump();}while(GetTickCount64()-start<15000);}
  CHECK(last<=handles+12&&GetGuiResources(GetCurrentProcess(),GR_USEROBJECTS)<=users+2);CHECK(!runtime_cleanup_pending());printf("Runtime close before creation x12 handles %lu->%lu USER %lu->%lu pending cleanup=%d\n",handles,last,users,GetGuiResources(GetCurrentProcess(),GR_USEROBJECTS),runtime_cleanup_pending());}
 ui_host_destroy(h);DestroyWindow(root);CoUninitialize();printf("Real WebView2 presentation/capture/images/input/stale/close: %d failures\n",failures);return failures?1:0;
}
