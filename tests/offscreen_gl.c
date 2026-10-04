#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include "ui_framework/opengl.h"
static int failures,frames,inputs,visible;
static GLuint pixel_buffer;
typedef void (APIENTRY *buffer_bind_fn)(GLenum,GLuint);
typedef void (APIENTRY *buffer_gen_fn)(GLsizei,GLuint *);
typedef void (APIENTRY *buffer_delete_fn)(GLsizei,const GLuint *);
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);++failures;}}while(0)
static BOOL CALLBACK count_visible(HWND w,LPARAM p){(void)p;if(IsWindowVisible(w))++visible;return TRUE;}
static void frame(ui_surface_t *s,void *data)
{
    ui_rect_t r;GLuint texture;unsigned char rgba[4]={0,255,0,255};(void)data;++frames;
    CHECK(ui_surface_make_current(s)==UI_STATUS_OK);CHECK(ui_surface_get_pixel_rect(s,&r)==UI_STATUS_OK);
    glDisable(GL_SCISSOR_TEST);glClearColor(1,0,0,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST);glScissor(0,r.height/2,r.width,r.height-r.height/2);glClearColor(0,0,1,1);glClear(GL_COLOR_BUFFER_BIT);glDisable(GL_SCISSOR_TEST);
    glMatrixMode(GL_PROJECTION);glLoadIdentity();glMatrixMode(GL_MODELVIEW);glLoadIdentity();
    glGenTextures(1,&texture);glBindTexture(GL_TEXTURE_2D,texture);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,1,1,0,GL_RGBA,GL_UNSIGNED_BYTE,rgba);
    glEnable(GL_TEXTURE_2D);glColor4f(1,1,1,1);glBegin(GL_QUADS);
    glTexCoord2f(0,0);glVertex2f(-.3f,-.3f);glTexCoord2f(1,0);glVertex2f(.3f,-.3f);
    glTexCoord2f(1,1);glVertex2f(.3f,.3f);glTexCoord2f(0,1);glVertex2f(-.3f,.3f);glEnd();glDisable(GL_TEXTURE_2D);glDeleteTextures(1,&texture);
    CHECK(ui_surface_swap_buffers(s)==UI_STATUS_OK);
    /* Renderer pixel-store/PBO state must not redirect or overflow CPU readback. */
    glPixelStorei(GL_PACK_SKIP_ROWS,5);glPixelStorei(GL_PACK_SKIP_PIXELS,6);
    {buffer_bind_fn bind=(buffer_bind_fn)ui_opengl_surface_get_proc_address(s,"glBindBuffer");buffer_gen_fn gen=(buffer_gen_fn)ui_opengl_surface_get_proc_address(s,"glGenBuffers");CHECK(bind&&gen);
     if(bind&&gen){if(!pixel_buffer)gen(1,&pixel_buffer);bind(0x88eb,pixel_buffer);}}
}
static void input(ui_surface_t *s,const ui_input_event_t *e,void *data){(void)s;(void)data;CHECK(e->x==12&&e->kind==UI_INPUT_POINTER_DOWN);++inputs;}
int main(void)
{
    ui_host_config_t hc={0};ui_host_t *host;ui_surface_desc_t d={0};ui_opengl_config_t gl={0};ui_surface_t *a,*b;
    ui_status_t status;ui_opengl_info_t info={0};ui_opengl_window_dependency_t dependency;ui_pixel_buffer_t p={0};ui_input_event_t e={0};
    HGLRC previous;GLint viewport[4],skip;ui_rect_t rect={0,0,20,16};
    hc.size=sizeof(hc);hc.api_version=4;host=ui_host_create(&hc);CHECK(host!=NULL);CHECK(ui_host_set_dpi(host,144)==UI_STATUS_OK);
    d.size=sizeof(d);d.id="a";d.kind=UI_SURFACE_OPENGL;d.rect=(ui_rect_t){0,0,32,24};d.visible=1;
    gl.size=sizeof(gl);gl.major_version=3;gl.minor_version=3;gl.profile=UI_OPENGL_PROFILE_COMPATIBILITY;
    a=ui_opengl_offscreen_surface_create(host,&d,&gl,&status);if(!a){printf("Strict WGL 3.3 compatibility unavailable, status=%d\n",status);ui_host_destroy(host);return status==UI_STATUS_UNSUPPORTED?77:1;}
    info.size=sizeof(info);CHECK(ui_opengl_surface_get_info(a,&info)==UI_STATUS_OK);CHECK(info.major_version>=3&&info.profile==UI_OPENGL_PROFILE_COMPATIBILITY&&!info.legacy_context);
    printf("Offscreen GPU: %s / %s / %s; hidden HWND dependency; hardware class unknown\n",info.vendor,info.renderer,info.version);
    CHECK(ui_opengl_surface_get_window_dependency(a,&dependency)==UI_STATUS_OK&&dependency==UI_OPENGL_HIDDEN_WINDOW);
    CHECK(ui_surface_set_callbacks(a,NULL,frame,NULL)==UI_STATUS_OK);CHECK(ui_surface_set_input_callback(a,input,NULL)==UI_STATUS_OK);
    CHECK(ui_surface_make_current(a)==UI_STATUS_OK);glPixelStorei(GL_PACK_SKIP_ROWS,3);glPixelStorei(GL_PACK_SKIP_PIXELS,4);
    e.size=sizeof(e);e.kind=UI_INPUT_POINTER_DOWN;e.x=12;CHECK(ui_surface_dispatch_input(a,&e)==UI_STATUS_OK&&inputs==1);
    d.id="b";b=ui_opengl_offscreen_surface_create(host,&d,&gl,&status);CHECK(b!=NULL);
    CHECK(ui_surface_make_current(b)==UI_STATUS_OK);previous=wglGetCurrentContext();glViewport(2,3,7,9);
    p.size=sizeof(p);CHECK(ui_opengl_offscreen_render(a,&p)==UI_STATUS_OK&&p.width==48&&p.height==36&&frames==0);
    p.capacity=p.stride*p.height;p.pixels=(uint8_t *)malloc(p.capacity);CHECK(p.pixels!=NULL);
    CHECK(ui_opengl_offscreen_render(a,&p)==UI_STATUS_OK&&frames==1);CHECK(wglGetCurrentContext()==previous);
    glGetIntegerv(GL_VIEWPORT,viewport);CHECK(viewport[0]==2&&viewport[1]==3&&viewport[2]==7&&viewport[3]==9);
    CHECK(p.pixels[2]==255&&p.pixels[(p.height-1)*p.stride]==255);
    CHECK(p.pixels[(p.height/2)*p.stride+(p.width/2)*4+1]==255);
    CHECK(ui_surface_make_current(a)==UI_STATUS_OK);glGetIntegerv(GL_PACK_SKIP_ROWS,&skip);CHECK(skip==3);glGetIntegerv(GL_PACK_SKIP_PIXELS,&skip);CHECK(skip==4);
    glGetIntegerv(0x88ed,&skip);CHECK(skip==0);CHECK(ui_surface_make_current(b)==UI_STATUS_OK);
    CHECK(ui_surface_set_rect(a,&rect)==UI_STATUS_OK);p.stride=0;CHECK(ui_opengl_offscreen_render(a,&p)==UI_STATUS_OK&&p.width==30&&p.height==24&&frames==2);
    p.stride=SIZE_MAX;CHECK(ui_opengl_offscreen_render(a,&p)==UI_STATUS_INVALID_ARGUMENT);free(p.pixels);
    d.id="bad";gl.profile=UI_OPENGL_PROFILE_CORE;CHECK(!ui_opengl_offscreen_surface_create(host,&d,&gl,&status)&&status==UI_STATUS_UNSUPPORTED);
    gl.profile=UI_OPENGL_PROFILE_COMPATIBILITY;d.rect.width=100000;CHECK(!ui_opengl_offscreen_surface_create(host,&d,&gl,&status)&&status==UI_STATUS_LIMIT_EXCEEDED);
    CHECK(ui_surface_make_current(a)==UI_STATUS_OK);{buffer_delete_fn del=(buffer_delete_fn)ui_opengl_surface_get_proc_address(a,"glDeleteBuffers");CHECK(del!=NULL);if(del)del(1,&pixel_buffer);}CHECK(ui_surface_make_current(b)==UI_STATUS_OK);
    EnumThreadWindows(GetCurrentThreadId(),count_visible,0);CHECK(visible==0);ui_host_destroy(host);
    CHECK(wglGetCurrentContext()==NULL);printf("Hidden WGL/FBO/frame/input/readback: %d failures\n",failures);return failures?1:0;
}
