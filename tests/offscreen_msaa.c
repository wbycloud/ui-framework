#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui_framework/opengl.h"
static int failures,frames,inputs,windows,actual_samples,created_windows,process_windows;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);++failures;}}while(0)
typedef void (APIENTRY *clear_color_fn)(float,float,float,float);
typedef void (APIENTRY *clear_fn)(unsigned);
typedef void (APIENTRY *begin_fn)(unsigned);
typedef void (APIENTRY *end_fn)(void);
typedef void (APIENTRY *vertex_fn)(float,float);
typedef void (APIENTRY *integer_fn)(unsigned,int *);
typedef void (APIENTRY *viewport_fn)(int,int,int,int);
typedef void (APIENTRY *pixel_store_fn)(unsigned,int);
typedef unsigned char (APIENTRY *enabled_fn)(unsigned);
static BOOL CALLBACK count_window(HWND w,LPARAM p){(void)w;(void)p;++windows;return TRUE;}
static BOOL CALLBACK count_process_child(HWND w,LPARAM p){DWORD pid=0;(void)p;GetWindowThreadProcessId(w,&pid);if(pid==GetCurrentProcessId())++process_windows;return TRUE;}
static BOOL CALLBACK count_process_root(HWND w,LPARAM p){(void)count_process_child(w,p);EnumChildWindows(w,count_process_child,p);return TRUE;}
static int process_window_count(void){process_windows=0;EnumWindows(count_process_root,0);return process_windows;}
static LRESULT CALLBACK window_hook(int code,WPARAM w,LPARAM p){if(code==HCBT_CREATEWND)++created_windows;return CallNextHookEx(NULL,code,w,p);}
static void frame(ui_surface_t *s,void *u)
{
 clear_color_fn color=(clear_color_fn)ui_opengl_surface_get_proc_address(s,"glClearColor");clear_fn clear=(clear_fn)ui_opengl_surface_get_proc_address(s,"glClear");
 begin_fn begin=(begin_fn)ui_opengl_surface_get_proc_address(s,"glBegin");end_fn end=(end_fn)ui_opengl_surface_get_proc_address(s,"glEnd");vertex_fn vertex=(vertex_fn)ui_opengl_surface_get_proc_address(s,"glVertex2f");integer_fn get=(integer_fn)ui_opengl_surface_get_proc_address(s,"glGetIntegerv");
 (void)u;++frames;CHECK(color&&clear&&begin&&end&&vertex&&get);if(!color||!clear||!begin||!end||!vertex||!get)return;
 get(0x80a9,&actual_samples);color(0,0,0,1);clear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);begin(GL_TRIANGLES);vertex(-.83f,-.77f);vertex(.81f,-.71f);vertex(-.21f,.87f);end();
 /* A frame may leave scissor enabled. Resolve must still copy the complete
  * image, and the caller's enable state must be restored. */
 {clear_fn enable=(clear_fn)ui_opengl_surface_get_proc_address(s,"glEnable");viewport_fn scissor=(viewport_fn)ui_opengl_surface_get_proc_address(s,"glScissor");CHECK(enable&&scissor);if(enable&&scissor){scissor(0,0,1,1);enable(GL_SCISSOR_TEST);}}
 CHECK(ui_surface_swap_buffers(s)==UI_STATUS_OK);
}
static void input(ui_surface_t *s,const ui_input_event_t *e,void *u){(void)s;(void)u;CHECK(e->x==7);++inputs;}
int wmain(int argc,wchar_t **argv)
{
 ui_host_config_t hc={0};ui_host_t *h;ui_surface_desc_t d={0};ui_opengl_config_t gl={0};ui_opengl_windowless_config_t cfg={0};ui_surface_t *s,*other;ui_status_t st;ui_opengl_info_t info={0};ui_opengl_window_dependency_t dep;ui_pixel_buffer_t p={0};ui_input_event_t e={0};int partial=0,base_windows,base_process_windows=process_window_count();ui_rect_t rect={0,0,80,50};
 HHOOK hook=NULL;hc.size=sizeof(hc);hc.api_version=UI_FRAMEWORK_API_VERSION;h=ui_host_create(&hc);CHECK(h!=NULL);d.size=sizeof(d);d.id="test";d.kind=UI_SURFACE_OPENGL;d.rect=(ui_rect_t){0,0,64,64};gl.size=sizeof(gl);gl.major_version=3;gl.minor_version=3;gl.profile=UI_OPENGL_PROFILE_COMPATIBILITY;gl.samples=4;
 hook=SetWindowsHookExW(WH_CBT,window_hook,NULL,GetCurrentThreadId());CHECK(hook!=NULL);
 {static char path[4096];if(argc>1)CHECK(WideCharToMultiByte(CP_UTF8,0,argv[1],-1,path,sizeof(path),NULL,NULL)>0);cfg.library_path_utf8=argc>1?path:NULL;}
 EnumThreadWindows(GetCurrentThreadId(),count_window,0);base_windows=windows;cfg.size=sizeof(cfg);
 s=argc>1?ui_opengl_windowless_surface_create(h,&d,&gl,&cfg,&st):ui_opengl_offscreen_surface_create(h,&d,&gl,&st);CHECK(s!=NULL);if(!s){printf("creation status=%d; not acceptance\n",st);ui_host_destroy(h);return 1;}
 info.size=sizeof(info);CHECK(ui_opengl_surface_get_info(s,&info)==UI_STATUS_OK);CHECK(info.samples==4);printf("Actual GL %s / %s / %s samples=%d\n",info.vendor,info.renderer,info.version,info.samples);
 CHECK(ui_opengl_surface_get_window_dependency(s,&dep)==UI_STATUS_OK&&dep==(argc>1?UI_OPENGL_NO_WINDOW:UI_OPENGL_HIDDEN_WINDOW));
 if(argc>1){CHECK(strstr(info.version,"Mesa")!=NULL);CHECK(ui_surface_native_handle(s)==NULL);windows=0;EnumThreadWindows(GetCurrentThreadId(),count_window,0);CHECK(windows==base_windows);CHECK(process_window_count()==base_process_windows);}
 CHECK(ui_surface_set_callbacks(s,NULL,frame,NULL)==UI_STATUS_OK);CHECK(ui_surface_set_input_callback(s,input,NULL)==UI_STATUS_OK);e.size=sizeof(e);e.kind=UI_INPUT_POINTER_DOWN;e.x=7;CHECK(ui_surface_dispatch_input(s,&e)==UI_STATUS_OK&&inputs==1);
 d.id="other";gl.samples=0;other=argc>1?ui_opengl_windowless_surface_create(h,&d,&gl,&cfg,&st):ui_opengl_offscreen_surface_create(h,&d,&gl,&st);CHECK(other!=NULL);CHECK(ui_surface_make_current(other)==UI_STATUS_OK);
 p.size=sizeof(p);CHECK(ui_opengl_offscreen_render(s,&p)==UI_STATUS_OK&&frames==0);p.capacity=65536;p.pixels=(uint8_t *)malloc(p.capacity);CHECK(p.pixels!=NULL);CHECK(ui_opengl_offscreen_render(s,&p)==UI_STATUS_OK&&actual_samples==4&&frames==1);
 for(unsigned y=0;y<p.height;++y)for(unsigned x=0;x<p.width;++x){unsigned v=p.pixels[y*p.stride+x*4];if(v>0&&v<255)++partial;}
 CHECK(partial>20);printf("Actual triangle antialias partial pixels=%d\n",partial);
 {FILE *file=NULL;if(!fopen_s(&file,argc>1?"windowless-msaa.ppm":"wgl-msaa.ppm","wb")){fprintf(file,"P6\n%u %u\n255\n",p.width,p.height);for(unsigned y=0;y<p.height;++y)for(unsigned x=0;x<p.width;++x)fwrite(p.pixels+y*p.stride+x*4,1,3,file);fclose(file);}}
 /* Both current-provider restoration and the surface's own pack/viewport state. */
 {integer_fn get=(integer_fn)ui_opengl_surface_get_proc_address(other,"glGetIntegerv");viewport_fn viewport=(viewport_fn)ui_opengl_surface_get_proc_address(other,"glViewport");int state[4];
  viewport(3,4,17,19);CHECK(ui_opengl_offscreen_render(s,&p)==UI_STATUS_OK);get(GL_VIEWPORT,state);CHECK(state[0]==3&&state[1]==4&&state[2]==17&&state[3]==19);
  CHECK(ui_surface_make_current(s)==UI_STATUS_OK);get=(integer_fn)ui_opengl_surface_get_proc_address(s,"glGetIntegerv");viewport=(viewport_fn)ui_opengl_surface_get_proc_address(s,"glViewport");pixel_store_fn store=(pixel_store_fn)ui_opengl_surface_get_proc_address(s,"glPixelStorei");
  viewport(5,6,21,23);store(GL_PACK_ALIGNMENT,1);store(GL_PACK_ROW_LENGTH,93);store(GL_PACK_SKIP_ROWS,2);store(GL_PACK_SKIP_PIXELS,3);CHECK(ui_opengl_offscreen_render(s,&p)==UI_STATUS_OK);get(GL_VIEWPORT,state);CHECK(state[0]==5&&state[1]==6&&state[2]==21&&state[3]==23);get(GL_PACK_ALIGNMENT,state);CHECK(state[0]==1);get(GL_PACK_ROW_LENGTH,state);CHECK(state[0]==93);get(GL_PACK_SKIP_ROWS,state);CHECK(state[0]==2);get(GL_PACK_SKIP_PIXELS,state);CHECK(state[0]==3);CHECK(!((enabled_fn)ui_opengl_surface_get_proc_address(s,"glIsEnabled"))(GL_SCISSOR_TEST));}
 CHECK(ui_surface_set_rect(s,&rect)==UI_STATUS_OK);p.stride=0;CHECK(ui_opengl_offscreen_render(s,&p)==UI_STATUS_OK&&p.width==80&&p.height==50&&frames==4);free(p.pixels);
 rect.width=rect.height=16384;CHECK(ui_surface_set_rect(s,&rect)==UI_STATUS_LIMIT_EXCEEDED);
 p.pixels=NULL;p.capacity=0;p.stride=0;CHECK(ui_opengl_offscreen_render(s,&p)==UI_STATUS_OK&&p.width==80&&p.height==50);
 CHECK(ui_surface_set_callbacks(other,NULL,frame,NULL)==UI_STATUS_OK);p.stride=0;CHECK(ui_opengl_offscreen_render(other,&p)==UI_STATUS_OK);p.capacity=p.stride*p.height;p.pixels=(uint8_t *)malloc(p.capacity);CHECK(ui_opengl_offscreen_render(other,&p)==UI_STATUS_OK&&actual_samples==0);partial=0;for(size_t i=0;i<p.capacity;i+=4)if(p.pixels[i]>0&&p.pixels[i]<255)++partial;CHECK(partial==0);printf("Non-MSAA actual samples=0 partial=%d\n",partial);free(p.pixels);
 d.id="unsupported";gl.samples=9999;CHECK(!(argc>1?ui_opengl_windowless_surface_create(h,&d,&gl,&cfg,&st):ui_opengl_offscreen_surface_create(h,&d,&gl,&st))&&st==UI_STATUS_UNSUPPORTED);
 if(argc==1){d.id="api4-budget-boundary";d.rect.width=d.rect.height=2048;gl.samples=0;s=ui_opengl_offscreen_surface_create(h,&d,&gl,&st);CHECK(s&&st==UI_STATUS_OK);if(s)ui_surface_destroy(s);}
 ui_host_destroy(h);windows=0;EnumThreadWindows(GetCurrentThreadId(),count_window,0);CHECK(windows==base_windows);
 if(argc>1){DWORD first=0,last=0;GetProcessHandleCount(GetCurrentProcess(),&first);for(int i=0;i<16;++i){h=ui_host_create(&hc);gl.samples=4;s=ui_opengl_windowless_surface_create(h,&d,&gl,&cfg,&st);CHECK(s!=NULL);ui_host_destroy(h);GetProcessHandleCount(GetCurrentProcess(),&last);printf("OSMesa reopen %d handles=%lu loaded=%d\n",i+1,last,GetModuleHandleW(L"osmesa.dll")!=NULL);}CHECK(last<=first+2);}
 CHECK(process_window_count()==base_process_windows);if(hook){CHECK(argc>1?created_windows==0:created_windows>0);UnhookWindowsHookEx(hook);}printf("GL provider/MSAA: %d failures, frame=%d, windows=%d, created HWNDs=%d, process HWNDs=%d, HWND dependency=%d\n",failures,frames,windows,created_windows,process_windows,dep);return failures?1:0;
}
