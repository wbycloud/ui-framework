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
typedef struct fixture {ui_app_context_t context;ui_web_backend_t *backend;ui_component_t *form,*dialog,*tree,*status;ui_surface_t *gl;ui_image_id_t image;unsigned frames;} fixture_t;
typedef void (APIENTRY *color_fn)(float,float,float,float);
typedef void (APIENTRY *clear_fn)(unsigned);
typedef void (APIENTRY *begin_fn)(unsigned);
typedef void (APIENTRY *end_fn)(void);
typedef void (APIENTRY *vertex_fn)(float,float);
static void frame(ui_surface_t *surface,void *data)
{
 fixture_t *s=(fixture_t *)data;char text[80];color_fn color=(color_fn)ui_opengl_surface_get_proc_address(surface,"glClearColor");clear_fn clear=(clear_fn)ui_opengl_surface_get_proc_address(surface,"glClear");
 begin_fn begin=(begin_fn)ui_opengl_surface_get_proc_address(surface,"glBegin");end_fn end=(end_fn)ui_opengl_surface_get_proc_address(surface,"glEnd");vertex_fn vertex=(vertex_fn)ui_opengl_surface_get_proc_address(surface,"glVertex2f");
 if(!color||!clear||!begin||!end||!vertex)return;color(.05f,.1f,.2f,1);clear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);begin(GL_TRIANGLES);vertex(-.8f,-.7f);vertex(.8f,-.7f);vertex(-.2f,.8f);end();++s->frames;
 snprintf(text,sizeof(text),"Application DLL real GL frames=%u",s->frames);(void)ui_component_set_text(s->status,text);
}
static void source(ui_component_t *c,const ui_component_query_t *q,void *data)
{fixture_t *s=(fixture_t *)data;ui_row_t row={0};ui_component_batch_t b={0};if(q->kind!=UI_QUERY_ROWS)return;
 row.size=sizeof(row);row.id=q->parent_id?2:1;row.parent_id=q->parent_id;row.title=q->parent_id?"Nested DLL row":"C source root";row.has_children=!q->parent_id;row.content_version=1;row.image_id=s->image;
 b.size=sizeof(b);b.component_generation=q->component_generation;b.request_id=q->request_id;b.parent_id=q->parent_id;b.first=q->first;b.total_count=1;b.rows=&row;b.row_count=1;(void)ui_component_submit(c,&b);}
static void command(ui_host_t *h,uint64_t request,const char *id,const char *params,const char *origin,void *data)
{fixture_t *s=(fixture_t *)data;ui_cell_t value={0};uint8_t pixel[4]={0,255,0,255};ui_rgba_desc_t rgba={sizeof(rgba),1,1,4,4,pixel};(void)params;(void)origin;
 if(!strcmp(id,"test.submit")){(void)ui_component_accept_fields(s->form);(void)ui_component_set_text(s->status,"submitted");}
 else if(!strcmp(id,"test.image.update"))(void)ui_image_update(h,s->image,&rgba);
 else if(!strcmp(id,"test.image.release")){(void)ui_image_release(h,s->image);s->image=0;}
 else if(!strcmp(id,"test.dialog"))(void)ui_component_show_dialog(s->dialog);
 else if(!strcmp(id,"test.dialog.close"))(void)ui_component_close_dialog(s->dialog);
 else if(!strcmp(id,"test.field")){value.size=sizeof(value);value.kind=UI_VALUE_TEXT;value.text="DLL updated";(void)ui_component_set_field(s->form,"name",&value);}
 (void)ui_host_reply(h,request,1,"{}");}
static ui_status_t UI_APP_CALL create(const ui_app_context_t *context,void **state,ui_assistant_config_t *assistant)
{
 fixture_t *s=(fixture_t *)calloc(1,sizeof(*s));ui_webview2_backend_config_t config={0};ui_status_t status;ui_component_desc_t desc={0};ui_field_desc_t fields[2]={{0}};ui_command_desc_t cmd={0};ui_menu_group_desc_t group={0};ui_menu_item_desc_t item={0};ui_panel_desc_t panel={0};const char *ids[]={"test.submit","test.image.update","test.image.release","test.dialog","test.dialog.close","test.field"};size_t i;
 (void)assistant;if(!s)return UI_STATUS_OUT_OF_MEMORY;*state=s;s->context=*context;config.size=sizeof(config);config.framework_components=1;s->backend=ui_webview2_backend_create(&config,&status);if(!s->backend)return status;
 {ui_layout_desc_t layout={0};layout.size=sizeof(layout);layout.menu_bar_height=28;layout.status_bar_height=24;layout.left_sidebar_preferred_width=layout.left_sidebar_min_width=layout.left_sidebar_max_width=220;layout.collapsed_tag_width=32;status=ui_host_set_layout(context->host,&layout);if(status!=UI_STATUS_OK)return status;}
 cmd.size=sizeof(cmd);cmd.title="API5 fixture";cmd.handler=command;cmd.user_data=s;for(i=0;i<sizeof(ids)/sizeof(ids[0]);++i){cmd.id=ids[i];cmd.shortcut_key=i==5?'O':0;cmd.shortcut_modifiers=i==5?UI_INPUT_MODIFIER_CONTROL:0;status=ui_host_register_command(context->host,&cmd);if(status!=UI_STATUS_OK)return status;}
 fields[0].size=fields[1].size=sizeof(fields[0]);fields[0].id="name";fields[0].title="Name";fields[0].kind=UI_VALUE_TEXT;fields[1].id="image";fields[1].title="C image";fields[1].kind=UI_VALUE_IMAGE;fields[1].flags=UI_VALUE_READONLY;
 desc.size=sizeof(desc);desc.id="form";desc.title="Runtime DLL form";desc.kind=UI_COMPONENT_FORM;desc.fields=fields;desc.field_count=2;desc.web_backend=s->backend;desc.commands.submit="test.submit";
 status=ui_component_register(context->host,&desc,&s->form);if(status!=UI_STATUS_OK)return status;
 desc.id="dialog";desc.title="Runtime DLL dialog";desc.kind=UI_COMPONENT_DIALOG;desc.commands.submit="test.dialog.close";status=ui_component_register(context->host,&desc,&s->dialog);if(status!=UI_STATUS_OK)return status;
 desc.id="tree";desc.kind=UI_COMPONENT_TREE;desc.fields=NULL;desc.field_count=0;desc.source=source;desc.user_data=s;status=ui_component_register(context->host,&desc,&s->tree);if(status!=UI_STATUS_OK)return status;
 panel.size=sizeof(panel);panel.id="tree";panel.title="Runtime tree";panel.entry_url="";panel.dock_region=UI_LAYOUT_REGION_LEFT_SIDEBAR;panel.kind=UI_PANEL_SIDEBAR;panel.preferred_width=220;status=ui_host_register_panel(context->host,&panel);if(status!=UI_STATUS_OK)return status;
 desc.id="status";desc.kind=UI_COMPONENT_STATUS;desc.source=NULL;status=ui_component_register(context->host,&desc,&s->status);if(status!=UI_STATUS_OK)return status;
 group.size=sizeof(group);group.path="File";group.title="File";group.access_key='F';status=ui_host_register_menu_group(context->host,&group);if(status!=UI_STATUS_OK)return status;
 item.size=sizeof(item);item.id="submit";item.menu_path="File";item.title="Submit";item.access_key='S';item.command_id="test.submit";return ui_host_register_menu_item(context->host,&item);
}
static ui_status_t UI_APP_CALL mount(void *data,const ui_app_context_t *context)
{
 fixture_t *s=(fixture_t *)data;ui_status_t status;ui_cell_t value={0};ui_rgba_desc_t rgba={0};uint8_t pixel[4]={255,0,0,255};ui_surface_desc_t desc={0};ui_opengl_config_t gl={0};ui_opengl_windowless_config_t provider={0};wchar_t path[4096];char utf8[16384];
 s->context=*context;rgba.size=sizeof(rgba);rgba.width=rgba.height=1;rgba.stride=rgba.bytes=4;rgba.pixels=pixel;status=ui_image_create(context->host,&rgba,&s->image);if(status!=UI_STATUS_OK)return status;
 value.size=sizeof(value);value.kind=UI_VALUE_IMAGE;value.image_id=s->image;status=ui_component_set_field(s->form,"image",&value);if(status!=UI_STATUS_OK)return status;
 status=ui_component_mount(s->form,ui_shell_get_content_slot(ui_host_get_shell(context->host),NULL));if(status!=UI_STATUS_OK)return status;
 status=ui_component_mount(s->tree,ui_shell_get_content_slot(ui_host_get_shell(context->host),"tree"));if(status!=UI_STATUS_OK)return status;
 if(!GetEnvironmentVariableW(L"UI_API5_OSMESA_DLL",path,4096)||!WideCharToMultiByte(CP_UTF8,0,path,-1,utf8,sizeof(utf8),NULL,NULL))return UI_STATUS_INVALID_ARGUMENT;
 desc.size=sizeof(desc);desc.id="actual-dll-gl";desc.kind=UI_SURFACE_OPENGL;desc.rect=(ui_rect_t){0,0,64,64};gl.size=sizeof(gl);gl.major_version=3;gl.minor_version=3;gl.profile=UI_OPENGL_PROFILE_COMPATIBILITY;gl.samples=4;provider.size=sizeof(provider);provider.library_path_utf8=utf8;
 s->gl=ui_opengl_windowless_surface_create(context->host,&desc,&gl,&provider,&status);if(!s->gl)return status;return ui_surface_set_callbacks(s->gl,NULL,frame,s);
}
static void UI_APP_CALL active(void *data,int value){(void)data;(void)value;}
static ui_app_close_decision_t UI_APP_CALL close_app(void *data,ui_app_close_reason_t reason){(void)data;(void)reason;return UI_APP_CLOSE_ALLOW;}
static void UI_APP_CALL unmount(void *data){fixture_t *s=(fixture_t *)data;if(s->gl){ui_surface_destroy(s->gl);s->gl=NULL;}}
static void UI_APP_CALL destroy(void *data){fixture_t *s=(fixture_t *)data;if(s->backend)ui_webview2_backend_destroy(s->backend);free(s);}
static ui_status_t UI_APP_CALL shutdown_module(void){return UI_STATUS_OK;}
UI_APP_EXPORT const ui_app_descriptor_t *UI_APP_CALL ui_app_query_v1(void)
{static const ui_app_descriptor_t descriptor={sizeof(descriptor),1,5,create,mount,active,close_app,unmount,destroy,shutdown_module};return &descriptor;}
