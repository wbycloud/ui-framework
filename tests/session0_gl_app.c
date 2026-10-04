#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui_framework/application.h"
#include "ui_framework/opengl.h"

/* A real ABI1/API5 application. The runner uses only public workspace APIs;
 * there are no test exports and no renderer in the runner. */
typedef struct fixture {
    ui_app_context_t context;
    ui_surface_t *gl;
    ui_pixel_buffer_t pixels;
    unsigned frames, inputs, errors, partial, white, hash;
    int samples, actual_samples, input_status;
    char snapshot[512];
} fixture_t;
static unsigned live_instances, live_surfaces;
typedef void (APIENTRY *color_fn)(float,float,float,float);
typedef void (APIENTRY *clear_fn)(unsigned);
typedef void (APIENTRY *begin_fn)(unsigned);
typedef void (APIENTRY *end_fn)(void);
typedef void (APIENTRY *vertex_fn)(float,float);
typedef void (APIENTRY *integer_fn)(unsigned,int *);
#define EXPECT(s,x) do { if (!(x)) { ++(s)->errors; fprintf(stderr,"App line %d: %s\n",__LINE__,#x); } } while (0)

static void frame(ui_surface_t *surface,void *data)
{
    fixture_t *s=(fixture_t *)data;
    color_fn clear_color=(color_fn)ui_opengl_surface_get_proc_address(surface,"glClearColor");
    color_fn color=(color_fn)ui_opengl_surface_get_proc_address(surface,"glColor4f");
    clear_fn clear=(clear_fn)ui_opengl_surface_get_proc_address(surface,"glClear");
    begin_fn begin=(begin_fn)ui_opengl_surface_get_proc_address(surface,"glBegin");
    end_fn end=(end_fn)ui_opengl_surface_get_proc_address(surface,"glEnd");
    vertex_fn vertex=(vertex_fn)ui_opengl_surface_get_proc_address(surface,"glVertex2f");
    integer_fn get=(integer_fn)ui_opengl_surface_get_proc_address(surface,"glGetIntegerv");
    float shift=s->inputs?.13f:0;
    ++s->frames;
    EXPECT(s,clear_color&&color&&clear&&begin&&end&&vertex&&get);
    if(!clear_color||!color||!clear||!begin||!end||!vertex||!get)return;
    get(0x80a9,&s->actual_samples); /* GL_SAMPLES on the actual draw attachment. */
    clear_color(0,0,0,1);clear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    color(1,1,1,1);begin(GL_TRIANGLES);
    vertex(-.83f+shift,-.77f);vertex(.81f+shift,-.71f);vertex(-.21f+shift,.87f);end();
    EXPECT(s,ui_surface_swap_buffers(surface)==UI_STATUS_OK);
}
static void input(ui_surface_t *surface,const ui_input_event_t *event,void *data)
{fixture_t *s=(fixture_t *)data;(void)surface;EXPECT(s,event->kind==UI_INPUT_POINTER_DOWN&&event->x==7);++s->inputs;}
static const char *snapshot(void *data)
{
    fixture_t *s=(fixture_t *)data;
    snprintf(s->snapshot,sizeof(s->snapshot),"{\"frames\":%u,\"inputs\":%u,\"errors\":%u,\"width\":%u,\"height\":%u,\"samples\":%d,\"actual_samples\":%d,\"partial\":%u,\"white\":%u,\"hash\":%u,\"input_status\":%d}",
        s->frames,s->inputs,s->errors,s->pixels.width,s->pixels.height,s->samples,s->actual_samples,s->partial,s->white,s->hash,s->input_status);
    return s->snapshot;
}
static void render(fixture_t *s,int save)
{
    ui_pixel_buffer_t query={0};unsigned before=s->frames;query.size=sizeof(query);
    EXPECT(s,ui_opengl_offscreen_render(s->gl,&query)==UI_STATUS_OK&&s->frames==before);
    if(query.stride*query.height>s->pixels.capacity){
        free(s->pixels.pixels);s->pixels.pixels=(uint8_t *)malloc(query.stride*query.height);
        s->pixels.capacity=query.stride*query.height;
    }
    EXPECT(s,s->pixels.pixels!=NULL);if(!s->pixels.pixels)return;
    s->pixels.stride=0;
    EXPECT(s,ui_opengl_offscreen_render(s->gl,&s->pixels)==UI_STATUS_OK&&s->frames==before+1);
    s->partial=s->white=0;s->hash=2166136261u;
    for(unsigned y=0;y<s->pixels.height;++y)for(unsigned x=0;x<s->pixels.width;++x){
        const uint8_t *p=s->pixels.pixels+y*s->pixels.stride+x*4;
        if(p[0]>0&&p[0]<255)++s->partial;if(p[0]==255)++s->white;
        EXPECT(s,p[0]==p[1]&&p[1]==p[2]&&p[3]==255);
        for(unsigned k=0;k<4;++k)s->hash=(s->hash^p[k])*16777619u;
    }
    EXPECT(s,s->actual_samples==s->samples&&s->white>100);
    EXPECT(s,s->samples?s->partial>20:s->partial==0);
    if(save){
        wchar_t dir[32768],path[32768];FILE *file=NULL;
        DWORD length=GetEnvironmentVariableW(L"UI_SESSION0_OUTPUT",dir,32768);
        EXPECT(s,length&&length<32768);if(!length||length>=32768)return;
        EXPECT(s,swprintf_s(path,32768,L"%s/frame-%llu-samples%d-%u.ppm",dir,(unsigned long long)s->context.instance_id,s->samples,s->frames)>0);
        EXPECT(s,_wfopen_s(&file,path,L"wb")==0&&file!=NULL);if(!file)return;
        fprintf(file,"P6\n%u %u\n255\n",s->pixels.width,s->pixels.height);
        for(unsigned y=0;y<s->pixels.height;++y)for(unsigned x=0;x<s->pixels.width;++x)
            EXPECT(s,fwrite(s->pixels.pixels+y*s->pixels.stride+x*4,1,3,file)==3);
        EXPECT(s,fclose(file)==0);
    }
}
static void command(ui_host_t *host,uint64_t request,const char *id,const char *params,const char *origin,void *data)
{
    fixture_t *s=(fixture_t *)data;(void)origin;
    if(!strcmp(id,"test.render"))render(s,!strcmp(params,"{\"save\":true}"));
    else if(!strcmp(id,"test.input")){
        ui_input_event_t event={0};event.size=sizeof(event);event.kind=UI_INPUT_POINTER_DOWN;event.x=7;
        s->input_status=ui_surface_dispatch_input(s->gl,&event);
        EXPECT(s,s->input_status==UI_STATUS_OK||s->input_status==UI_STATUS_CANCELLED);
    }else if(!strcmp(id,"test.resize")){
        ui_rect_t rect={0,0,80,50};EXPECT(s,ui_surface_set_rect(s->gl,&rect)==UI_STATUS_OK);
    }else if(!strcmp(id,"test.budget")){
        ui_rect_t rect={0,0,16384,16384};EXPECT(s,ui_surface_set_rect(s->gl,&rect)==UI_STATUS_LIMIT_EXCEEDED);
    }
    (void)ui_host_reply(host,request,s->errors==0,snapshot(s));
}
static ui_status_t UI_APP_CALL create(const ui_app_context_t *context,void **state,ui_assistant_config_t *assistant)
{
    fixture_t *s=(fixture_t *)calloc(1,sizeof(*s));ui_command_desc_t cmd={0};
    const char *ids[]={"test.render","test.input","test.resize","test.budget"};
    if(!s)return UI_STATUS_OUT_OF_MEMORY;*state=s;++live_instances;s->context=*context;
    s->pixels.size=sizeof(s->pixels);s->actual_samples=-1;
    assistant->user_data=s;assistant->snapshot=snapshot;
    cmd.size=sizeof(cmd);cmd.title="Session0 GL validation";cmd.handler=command;cmd.user_data=s;
    for(size_t i=0;i<sizeof(ids)/sizeof(ids[0]);++i){ui_status_t st;cmd.id=ids[i];st=ui_host_register_command(context->host,&cmd);if(st!=UI_STATUS_OK)return st;}
    return UI_STATUS_OK;
}
static ui_status_t UI_APP_CALL mount(void *data,const ui_app_context_t *context)
{
    fixture_t *s=(fixture_t *)data;void *resource=NULL;size_t bytes=0;ui_status_t st;
    ui_surface_desc_t desc={0};ui_opengl_config_t gl={0};ui_opengl_windowless_config_t cfg={0};
    ui_opengl_info_t info={0};ui_opengl_window_dependency_t dependency=UI_OPENGL_VISIBLE_WINDOW;ui_run_mode_t mode;
    wchar_t path[32768],samples[16];char utf8[131072];DWORD length;
    s->context=*context;
    {ui_assistant_command_desc_t cmd={0};const char *ids[]={"test.render","test.input","test.resize","test.budget"};cmd.size=sizeof(cmd);
     for(size_t i=0;i<sizeof(ids)/sizeof(ids[0]);++i){cmd.id=ids[i];st=ui_assistant_register_command(context->assistant,&cmd);if(st!=UI_STATUS_OK)return st;}}
    EXPECT(s,ui_host_get_run_mode(context->host,&mode)==UI_STATUS_OK&&mode==UI_RUN_OFFSCREEN);
    st=ui_app_resource_read(context,"marker.txt",&resource,&bytes);if(st!=UI_STATUS_OK)return st;
    EXPECT(s,bytes==strlen("Session0 real application resource\n")&&!memcmp(resource,"Session0 real application resource\n",bytes));
    ui_app_resource_release(resource);
    length=GetEnvironmentVariableW(L"UI_SESSION0_OSMESA_DLL",path,32768);
    if(!length||length>=32768||!WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,path,-1,utf8,sizeof(utf8),NULL,NULL))return UI_STATUS_INVALID_ARGUMENT;
    length=GetEnvironmentVariableW(L"UI_SESSION0_SAMPLES",samples,16);
    if(!length||length>=16)return UI_STATUS_INVALID_ARGUMENT;
    s->samples=_wtoi(samples);if(s->samples!=0&&s->samples!=4)return UI_STATUS_INVALID_ARGUMENT;
    desc.size=sizeof(desc);desc.id="application-gl";desc.kind=UI_SURFACE_OPENGL;desc.rect=(ui_rect_t){0,0,64,64};
    gl.size=sizeof(gl);gl.major_version=3;gl.minor_version=3;gl.profile=UI_OPENGL_PROFILE_COMPATIBILITY;gl.samples=s->samples;
    cfg.size=sizeof(cfg);cfg.library_path_utf8=utf8;
    s->gl=ui_opengl_windowless_surface_create(context->host,&desc,&gl,&cfg,&st);if(!s->gl)return st;++live_surfaces;
    info.size=sizeof(info);EXPECT(s,ui_opengl_surface_get_info(s->gl,&info)==UI_STATUS_OK&&info.samples==s->samples);
    EXPECT(s,ui_opengl_surface_get_window_dependency(s->gl,&dependency)==UI_STATUS_OK&&dependency==UI_OPENGL_NO_WINDOW);
    EXPECT(s,ui_surface_native_handle(s->gl)==NULL);
    printf("Application %llu GL %s / %s / %s requested=%d actual=%d HWND=NULL dependency=%d\n",(unsigned long long)context->instance_id,info.vendor,info.renderer,info.version,s->samples,info.samples,dependency);
    EXPECT(s,ui_surface_set_callbacks(s->gl,NULL,frame,s)==UI_STATUS_OK);
    EXPECT(s,ui_surface_set_input_callback(s->gl,input,s)==UI_STATUS_OK);
    desc.id="unsupported";gl.samples=9999;
    {ui_surface_t *bad=ui_opengl_windowless_surface_create(context->host,&desc,&gl,&cfg,&st);EXPECT(s,!bad&&st==UI_STATUS_UNSUPPORTED);if(bad)ui_surface_destroy(bad);}
    return s->errors?UI_STATUS_PLATFORM_ERROR:UI_STATUS_OK;
}
static void UI_APP_CALL active(void *data,int value){(void)data;(void)value;}
static ui_app_close_decision_t UI_APP_CALL close_app(void *data,ui_app_close_reason_t reason){(void)data;(void)reason;return UI_APP_CLOSE_ALLOW;}
static void UI_APP_CALL unmount(void *data){fixture_t *s=(fixture_t *)data;if(s->gl){ui_surface_destroy(s->gl);s->gl=NULL;--live_surfaces;}}
static void UI_APP_CALL destroy(void *data){fixture_t *s=(fixture_t *)data;free(s->pixels.pixels);free(s);--live_instances;}
static ui_status_t UI_APP_CALL shutdown_module(void)
{printf("Application module_shutdown live_instances=%u live_surfaces=%u\n",live_instances,live_surfaces);return live_instances||live_surfaces?UI_STATUS_PLATFORM_ERROR:UI_STATUS_OK;}
UI_APP_EXPORT const ui_app_descriptor_t *UI_APP_CALL ui_app_query_v1(void)
{static const ui_app_descriptor_t descriptor={sizeof(descriptor),1,5,create,mount,active,close_app,unmount,destroy,shutdown_module};return &descriptor;}
