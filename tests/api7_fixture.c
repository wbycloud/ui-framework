#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui_framework/components.h"
#include "ui_framework/webview2.h"
#include "ui_framework/opengl.h"
#include "ui_framework/menus.h"
#define ROW_BASE 9007199254741000ULL
#define ROW_TOTAL 100000ULL
typedef struct fixture {
    ui_app_context_t context;ui_component_t *table,*tree,*form,*viewport,*dialog,*status;
    ui_web_backend_t *backend;ui_surface_t *gl;ui_image_id_t image;ui_component_query_t late;
    HANDLE stop,worker;CRITICAL_SECTION lock;ui_component_query_t thumbnails[128];size_t head,count;
    char column_ids[64][12];unsigned frames,commands,edits,version,gl_errors;GLuint gl_list;int runtime;
} fixture_t;
static LONG live_instances,live_workers,live_surfaces;
typedef void (APIENTRY *color_fn)(float,float,float,float);
typedef void (APIENTRY *clear_fn)(unsigned);
typedef void (APIENTRY *begin_fn)(unsigned);
typedef void (APIENTRY *end_fn)(void);
typedef void (APIENTRY *vertex_fn)(float,float);
typedef GLuint (APIENTRY *gen_list_fn)(GLsizei);
typedef void (APIENTRY *new_list_fn)(GLuint,GLenum);
typedef GLboolean (APIENTRY *is_list_fn)(GLuint);
static void frame(ui_surface_t *surface,void *data)
{
    fixture_t *s=(fixture_t *)data;color_fn color=(color_fn)ui_opengl_surface_get_proc_address(surface,"glClearColor");
    clear_fn clear=(clear_fn)ui_opengl_surface_get_proc_address(surface,"glClear");begin_fn begin=(begin_fn)ui_opengl_surface_get_proc_address(surface,"glBegin");
    end_fn end=(end_fn)ui_opengl_surface_get_proc_address(surface,"glEnd");vertex_fn vertex=(vertex_fn)ui_opengl_surface_get_proc_address(surface,"glVertex2f");
    gen_list_fn gen=(gen_list_fn)ui_opengl_surface_get_proc_address(surface,"glGenLists");new_list_fn compile=(new_list_fn)ui_opengl_surface_get_proc_address(surface,"glNewList");
    end_fn finish=(end_fn)ui_opengl_surface_get_proc_address(surface,"glEndList");is_list_fn exists=(is_list_fn)ui_opengl_surface_get_proc_address(surface,"glIsList");
    if(!s->gl_list){s->gl_list=gen(1);if(s->gl_list){compile(s->gl_list,GL_COMPILE);finish();}else ++s->gl_errors;}
    else if(!exists(s->gl_list))++s->gl_errors;
    color(0,0,0,1);clear(GL_COLOR_BUFFER_BIT);color=(color_fn)ui_opengl_surface_get_proc_address(surface,"glColor4f");color(1,1,1,1);
    begin(GL_TRIANGLES);vertex(-.83f,-.71f);vertex(.77f,-.66f);vertex(-.23f,.89f);end();++s->frames;
}
static DWORD WINAPI thumbnails(void *data)
{
    fixture_t *s=(fixture_t *)data;InterlockedIncrement(&live_workers);
    while(WaitForSingleObject(s->stop,10)==WAIT_TIMEOUT){ui_component_query_t q={0};int have=0;uint8_t pixels[16]={255,128,0,255,255,128,0,255,255,128,0,255,255,128,0,255};ui_thumbnail_result_t r={0};
        EnterCriticalSection(&s->lock);if(s->count){q=s->thumbnails[s->head];s->head=(s->head+1)%128;--s->count;have=1;}LeaveCriticalSection(&s->lock);if(!have)continue;
        r.size=sizeof(r);r.component_generation=q.component_generation;r.request_id=q.request_id;r.item_id=q.item_id;r.content_version=q.content_version;r.dpi=q.dpi;
        r.rgba=(ui_rgba_desc_t){sizeof(r.rgba),2,2,8,16,pixels};(void)ui_workspace_post_thumbnail(s->context.workspace,s->context.instance_id,"tree",&r);
    }InterlockedDecrement(&live_workers);(void)ui_workspace_post_close_complete(s->context.workspace,s->context.instance_id,UI_APP_CLOSE_ALLOW);return 0;
}
static uint64_t row_id(const ui_component_query_t *q,uint64_t index)
{return ROW_BASE+(q->sort_direction==-1?ROW_TOTAL-1-index:index);}
static void source(ui_component_t *c,const ui_component_query_t *q,void *data)
{
    fixture_t *s=(fixture_t *)data;ui_row_t rows[128]={0};ui_cell_t (*cells)[64];ui_component_batch_t b={0};uint64_t ids[512];size_t i,n;
    if(q->kind==UI_QUERY_SELECTION){for(i=0;i<q->count;++i)ids[i]=row_id(q,q->first+i);(void)ui_workspace_post_component_selection(s->context.workspace,s->context.instance_id,"table",q->component_generation,q->request_id,ids,q->count);return;}
    if(q->kind==UI_QUERY_THUMBNAIL){s->late=*q;s->late.sort_column=NULL;EnterCriticalSection(&s->lock);if(s->count<128){s->thumbnails[(s->head+s->count)%128]=s->late;++s->count;}LeaveCriticalSection(&s->lock);return;}
    if(q->kind!=UI_QUERY_ROWS)return;cells=calloc(128,sizeof(*cells));if(!cells)return;n=q->count;if(n>128)n=128;
    if(c==s->tree){if(q->parent_id){n=q->first?0:1;}else{if(q->first>=20)n=0;else if(n>20-q->first)n=(size_t)(20-q->first);}}
    else if(q->first>=ROW_TOTAL)n=0;else if(n>ROW_TOTAL-q->first)n=(size_t)(ROW_TOTAL-q->first);
    for(i=0;i<n;++i){rows[i].size=sizeof(rows[i]);rows[i].id=c==s->tree?(q->parent_id?1000+q->parent_id:1+q->first+i):row_id(q,q->first+i);
        rows[i].parent_id=q->parent_id;rows[i].title="Independent API7 row";rows[i].content_version=s->version;rows[i].has_children=c==s->tree&&!q->parent_id;
        if(c==s->table){cells[i][0].size=cells[i][1].size=sizeof(ui_cell_t);cells[i][0].column_id="name";cells[i][0].kind=UI_VALUE_TEXT;cells[i][0].text="editable";
            cells[i][1].column_id="value";cells[i][1].kind=UI_VALUE_NUMBER;cells[i][1].text="42";rows[i].cells=cells[i];for(size_t col=2;col<64;++col){cells[i][col].size=sizeof(ui_cell_t);cells[i][col].column_id=s->column_ids[col];cells[i][col].kind=UI_VALUE_TEXT;cells[i][col].text="wide cell";}rows[i].cell_count=64;}}
    b.size=sizeof(b);b.component_generation=q->component_generation;b.request_id=q->request_id;b.parent_id=q->parent_id;b.first=q->first;b.total_count=c==s->tree?(q->parent_id?1:20):ROW_TOTAL;b.rows=rows;b.row_count=n;(void)ui_component_submit(c,&b);free(cells);
}
static ui_status_t render(fixture_t *s)
{
    ui_pixel_buffer_t p={0};ui_cell_t value={0};ui_status_t status;p.size=sizeof(p);status=ui_opengl_offscreen_render(s->gl,&p);if(status!=UI_STATUS_OK){fprintf(stderr,"fixture %d status %d\n",__LINE__,status);return status;}
    p.capacity=p.stride*p.height;p.pixels=(uint8_t *)malloc(p.capacity);if(!p.pixels)return UI_STATUS_OUT_OF_MEMORY;status=ui_opengl_offscreen_render(s->gl,&p);
    if(status==UI_STATUS_OK&&(s->gl_errors||!s->gl_list))status=UI_STATUS_VALIDATION_FAILED;
    if(status==UI_STATUS_OK){ui_rgba_desc_t r={sizeof(r),p.width,p.height,p.stride,p.capacity,p.pixels};status=s->image?ui_image_update(s->context.host,s->image,&r):ui_image_create(s->context.host,&r,&s->image);}
    free(p.pixels);if(status==UI_STATUS_OK){value.size=sizeof(value);value.kind=UI_VALUE_IMAGE;value.flags=UI_VALUE_READONLY;value.image_id=s->image;status=ui_component_set_field(s->viewport,"pixels",&value);}return status;
}
static void command(ui_host_t *h,uint64_t request,const char *id,const char *params,const char *origin,void *data)
{
    fixture_t *s=(fixture_t *)data;ui_status_t status=UI_STATUS_OK;(void)params;(void)origin;++s->commands;
    if(!strcmp(id,"test.render"))status=render(s);
    else if(!strcmp(id,"test.edit"))++s->edits;
    else if(!strcmp(id,"test.submit"))status=ui_component_accept_fields(s->form);
    else if(!strcmp(id,"test.reset"))status=ui_shell_reset_layout(ui_host_get_shell(h));
    else if(!strcmp(id,"test.modal"))status=ui_component_show_dialog(s->dialog);
    else if(!strcmp(id,"test.modal.close"))status=ui_component_close_dialog(s->dialog);
    else if(!strcmp(id,"test.image.release")){status=ui_image_release(h,s->image);s->image=0;}
    else if(!strcmp(id,"test.late")){ui_thumbnail_result_t r={0};r.size=sizeof(r);r.component_generation=s->late.component_generation;r.request_id=s->late.request_id;r.item_id=s->late.item_id;r.content_version=s->late.content_version;r.dpi=s->late.dpi;r.failed=1;
        status=ui_component_set_source(s->tree,source,s);if(status==UI_STATUS_OK)status=ui_component_thumbnail(s->tree,&r);if(status==UI_STATUS_CANCELLED)status=UI_STATUS_OK;else status=UI_STATUS_VALIDATION_FAILED;}
    {char result[160];snprintf(result,sizeof(result),"{\"frames\":%u,\"commands\":%u,\"edits\":%u,\"status\":%d}",s->frames,s->commands,s->edits,status);(void)ui_component_set_text(s->status,result);(void)ui_host_reply(h,request,status==UI_STATUS_OK,result);}
}
static ui_status_t UI_APP_CALL create(const ui_app_context_t *context,void **out,ui_assistant_config_t *assistant)
{
    fixture_t *s=(fixture_t *)calloc(1,sizeof(*s));ui_component_desc_t d={0};ui_panel_desc_t panel={0};ui_column_desc_t columns[64]={0};ui_field_desc_t fields[3]={0};ui_command_desc_t cmd={0};ui_status_t status;
    const char *options[]={"low","medium","high"},*commands[]={"test.render","test.edit","test.select","test.sort","test.submit","test.reset","test.modal","test.modal.close","test.image.release","test.late"};size_t i;char backend[32];
    (void)assistant;if(!s)return UI_STATUS_OUT_OF_MEMORY;*out=s;s->context=*context;s->version=1;InitializeCriticalSection(&s->lock);InterlockedIncrement(&live_instances);
    s->runtime=GetEnvironmentVariableA("UI_API7_BACKEND",backend,sizeof(backend))&&!strcmp(backend,"webview2");
    if(s->runtime){
#ifdef UI_API7_WEBVIEW2
        ui_webview2_backend_config_t config={0};config.size=sizeof(config);config.framework_components=1;s->backend=ui_webview2_backend_create(&config,&status);if(!s->backend)return status;
#else
        return UI_STATUS_UNSUPPORTED;
#endif
    }
    {ui_layout_desc_t l={0};l.size=sizeof(l);l.left_sidebar_width=240;l.right_sidebar_width=300;l.left_sidebar_min_width=120;l.left_sidebar_max_width=400;l.right_sidebar_min_width=160;l.right_sidebar_max_width=500;l.collapsed_tag_width=28;l.main_min_width=300;status=ui_host_set_layout(context->host,&l);if(status!=UI_STATUS_OK){fprintf(stderr,"fixture %d status %d\n",__LINE__,status);return status;}}
    cmd.size=sizeof(cmd);cmd.title="API7 integration";cmd.handler=command;cmd.user_data=s;for(i=0;i<sizeof(commands)/sizeof(commands[0]);++i){cmd.id=commands[i];status=ui_host_register_command(context->host,&cmd);if(status!=UI_STATUS_OK){fprintf(stderr,"fixture %d status %d\n",__LINE__,status);return status;}}
    panel.size=sizeof(panel);panel.title="API7 panel";panel.entry_url="";panel.kind=UI_PANEL_SIDEBAR;panel.dock_region=UI_LAYOUT_REGION_LEFT_SIDEBAR;panel.id="tree";status=ui_host_register_panel(context->host,&panel);if(status!=UI_STATUS_OK){fprintf(stderr,"fixture %d status %d\n",__LINE__,status);return status;}
    panel.id="viewport";status=ui_host_register_panel(context->host,&panel);if(status!=UI_STATUS_OK){fprintf(stderr,"fixture %d status %d\n",__LINE__,status);return status;}panel.id="form";panel.dock_region=UI_LAYOUT_REGION_RIGHT_SIDEBAR;status=ui_host_register_panel(context->host,&panel);if(status!=UI_STATUS_OK){fprintf(stderr,"fixture %d status %d\n",__LINE__,status);return status;}
    columns[0].size=columns[1].size=sizeof(columns[0]);columns[0].id="name";columns[0].title="Name";columns[0].kind=UI_VALUE_TEXT;columns[0].width=200;columns[1].id="value";columns[1].title="Value";columns[1].kind=UI_VALUE_NUMBER;columns[1].width=120;
    for(i=2;i<64;++i){snprintf(s->column_ids[i],sizeof(s->column_ids[i]),"c%zu",i);columns[i].size=sizeof(columns[i]);columns[i].id=s->column_ids[i];columns[i].title=s->column_ids[i];columns[i].kind=UI_VALUE_TEXT;columns[i].width=180;}
    d.size=sizeof(d);d.web_backend=s->backend;d.id="table";d.title="Complete 100000 row source";d.kind=UI_COMPONENT_TABLE;d.columns=columns;d.column_count=64;d.source=source;d.user_data=s;d.commands.select="test.select";d.commands.edit="test.edit";d.sort_command="test.sort";d.selection_flags=UI_SELECTION_MULTIPLE;status=ui_component_register(context->host,&d,&s->table);if(status!=UI_STATUS_OK){fprintf(stderr,"fixture %d status %d\n",__LINE__,status);return status;}
    memset(&d,0,sizeof(d));d.size=sizeof(d);d.web_backend=s->backend;d.id="tree";d.title="Async tree thumbnails";d.kind=UI_COMPONENT_TREE;d.source=source;d.user_data=s;status=ui_component_register(context->host,&d,&s->tree);if(status!=UI_STATUS_OK){fprintf(stderr,"fixture %d status %d\n",__LINE__,status);return status;}
    fields[0].size=fields[1].size=fields[2].size=sizeof(fields[0]);fields[0].id="name";fields[0].title="Draft";fields[0].kind=UI_VALUE_TEXT;fields[1].id="mode";fields[1].title="Enum";fields[1].kind=UI_VALUE_ENUM;fields[1].options=options;fields[1].option_count=3;fields[2].id="color";fields[2].title="Color";fields[2].kind=UI_VALUE_COLOR;
    memset(&d,0,sizeof(d));d.size=sizeof(d);d.web_backend=s->backend;d.id="form";d.kind=UI_COMPONENT_FORM;d.fields=fields;d.field_count=3;d.commands.submit="test.submit";status=ui_component_register(context->host,&d,&s->form);if(status!=UI_STATUS_OK){fprintf(stderr,"fixture %d status %d\n",__LINE__,status);return status;}
    d.id="dialog";d.kind=UI_COMPONENT_DIALOG;d.commands.submit="test.modal.close";d.fields=fields;d.field_count=1;status=ui_component_register(context->host,&d,&s->dialog);if(status!=UI_STATUS_OK){fprintf(stderr,"fixture %d status %d\n",__LINE__,status);return status;}
    fields[0].id="pixels";fields[0].title="Actual OSMesa frame";fields[0].kind=UI_VALUE_IMAGE;fields[0].flags=UI_VALUE_READONLY;d.id="viewport";d.kind=UI_COMPONENT_FORM;d.commands.submit=NULL;status=ui_component_register(context->host,&d,&s->viewport);if(status!=UI_STATUS_OK){fprintf(stderr,"fixture %d status %d\n",__LINE__,status);return status;}
    d.id="status";d.kind=UI_COMPONENT_STATUS;d.field_count=0;status=ui_component_register(context->host,&d,&s->status);if(status!=UI_STATUS_OK){fprintf(stderr,"fixture %d status %d\n",__LINE__,status);return status;}
    return GetEnvironmentVariableA("UI_API7_FAIL_CREATE",backend,sizeof(backend))?UI_STATUS_PLATFORM_ERROR:UI_STATUS_OK;
}
static ui_status_t UI_APP_CALL mount(void *data,const ui_app_context_t *context)
{
    fixture_t *s=(fixture_t *)data;ui_status_t status;ui_shell_t *shell=ui_host_get_shell(context->host);ui_surface_desc_t desc={0};ui_opengl_config_t gl={0};ui_opengl_windowless_config_t provider={0};char path[16384];wchar_t wide[4096];void *resource=NULL;size_t bytes=0;
    s->context=*context;status=ui_app_resource_read(context,"marker.txt",&resource,&bytes);if(status!=UI_STATUS_OK){fprintf(stderr,"fixture %d status %d\n",__LINE__,status);return status;}status=bytes>=4&&!memcmp(resource,"API7",4)?UI_STATUS_OK:UI_STATUS_VALIDATION_FAILED;ui_app_resource_release(resource);if(status!=UI_STATUS_OK){fprintf(stderr,"fixture %d status %d\n",__LINE__,status);return status;}
    status=ui_component_mount(s->table,ui_shell_get_content_slot(shell,NULL));if(status!=UI_STATUS_OK){fprintf(stderr,"fixture %d status %d\n",__LINE__,status);return status;}status=ui_component_mount(s->tree,ui_shell_get_content_slot(shell,"tree"));if(status!=UI_STATUS_OK){fprintf(stderr,"fixture %d status %d\n",__LINE__,status);return status;}
    status=ui_component_mount(s->form,ui_shell_get_content_slot(shell,"form"));if(status!=UI_STATUS_OK){fprintf(stderr,"fixture %d status %d\n",__LINE__,status);return status;}status=ui_component_mount(s->viewport,ui_shell_get_content_slot(shell,"viewport"));if(status!=UI_STATUS_OK){fprintf(stderr,"fixture %d status %d\n",__LINE__,status);return status;}
    if(!GetEnvironmentVariableW(L"UI_API7_OSMESA_DLL",wide,4096)||!WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,wide,-1,path,sizeof(path),NULL,NULL))return UI_STATUS_INVALID_ARGUMENT;
    desc.size=sizeof(desc);desc.id="viewport-gl";desc.kind=UI_SURFACE_OPENGL;desc.rect=(ui_rect_t){0,0,64,64};gl.size=sizeof(gl);gl.major_version=3;gl.minor_version=3;gl.profile=UI_OPENGL_PROFILE_COMPATIBILITY;gl.samples=4;provider.size=sizeof(provider);provider.library_path_utf8=path;
    s->gl=ui_opengl_windowless_surface_create(context->host,&desc,&gl,&provider,&status);if(!s->gl)return status;InterlockedIncrement(&live_surfaces);status=ui_surface_set_callbacks(s->gl,NULL,frame,s);if(status!=UI_STATUS_OK){fprintf(stderr,"fixture %d status %d\n",__LINE__,status);return status;}
    s->stop=CreateEventW(NULL,TRUE,FALSE,NULL);if(!s->stop)return UI_STATUS_PLATFORM_ERROR;s->worker=CreateThread(NULL,0,thumbnails,s,0,NULL);if(!s->worker)return UI_STATUS_PLATFORM_ERROR;
    if(GetEnvironmentVariableA("UI_API7_FAIL_MOUNT",path,sizeof(path)))return UI_STATUS_PLATFORM_ERROR;return render(s);
}
static void UI_APP_CALL active(void *data,int enabled){(void)data;(void)enabled;}
static ui_app_close_decision_t UI_APP_CALL close_app(void *data,ui_app_close_reason_t reason)
{fixture_t *s=(fixture_t *)data;(void)reason;if(s->stop)SetEvent(s->stop);return s->worker&&WaitForSingleObject(s->worker,0)!=WAIT_OBJECT_0?UI_APP_CLOSE_WAIT:UI_APP_CLOSE_ALLOW;}
static void UI_APP_CALL unmount(void *data)
{fixture_t *s=(fixture_t *)data;if(s->stop)SetEvent(s->stop);if(s->worker){WaitForSingleObject(s->worker,INFINITE);CloseHandle(s->worker);s->worker=NULL;}if(s->stop){CloseHandle(s->stop);s->stop=NULL;}if(s->gl){ui_surface_destroy(s->gl);s->gl=NULL;InterlockedDecrement(&live_surfaces);}}
static void UI_APP_CALL destroy(void *data)
{fixture_t *s=(fixture_t *)data;
#ifdef UI_API7_WEBVIEW2
 if(s->backend)ui_webview2_backend_destroy(s->backend);
#endif
 DeleteCriticalSection(&s->lock);free(s);InterlockedDecrement(&live_instances);}
static ui_status_t UI_APP_CALL shutdown_module(void)
{return live_instances||live_workers||live_surfaces?UI_STATUS_PLATFORM_ERROR:UI_STATUS_OK;}
#ifndef UI_FIXTURE_EMBEDDED
UI_APP_EXPORT const ui_app_descriptor_t *UI_APP_CALL ui_app_query_v1(void)
{static const ui_app_descriptor_t d={sizeof(d),1,UI_FRAMEWORK_API_VERSION,create,mount,active,close_app,unmount,destroy,shutdown_module};return &d;}
#endif
