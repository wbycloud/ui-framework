#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui_framework/light_web.h"
#include "ui_framework/menus.h"
static int failures,calls,windows;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);++failures;}}while(0)
static BOOL CALLBACK count_window(HWND w,LPARAM p){(void)w;(void)p;++windows;return TRUE;}
static void run(ui_host_t *host,uint64_t id,const char *command,const char *params,void *user)
{(void)command;(void)user;++calls;CHECK(strstr(params,"\"42\"")!=NULL);(void)ui_host_reply(host,id,1,"{}");}
static void source(ui_component_t *c,const ui_component_query_t *q,void *data)
{
    ui_row_t rows[2]={0};ui_component_batch_t batch={0};(void)data;if(q->kind!=UI_QUERY_ROWS)return;
    rows[0].size=rows[1].size=sizeof(rows[0]);rows[0].id=1;rows[0].title="TOP";rows[0].has_children=1;rows[0].content_version=1;
    rows[1].id=2;rows[1].title="CHILD";rows[1].content_version=1;
    batch.size=sizeof(batch);batch.component_generation=q->component_generation;batch.request_id=q->request_id;
    batch.parent_id=q->parent_id;batch.first=q->first;batch.total_count=2;batch.rows=rows;batch.row_count=2;
    CHECK(ui_component_submit(c,&batch)==UI_STATUS_OK);
}
static int menu_click(ui_host_t *host,const char *id)
{
    ui_element_presentation_t p={0};ui_input_event_t e={0};p.size=sizeof(p);
    if(ui_host_menu_get_presentation(host,id,&p)!=UI_STATUS_OK||!p.visible||!p.enabled)return 0;
    e.size=sizeof(e);e.x=p.clip.x+2;e.y=p.clip.y+2;e.pointer_button=1;e.kind=UI_INPUT_POINTER_DOWN;
    if(ui_host_menu_dispatch_input(host,&e)!=UI_STATUS_OK)return 0;e.kind=UI_INPUT_POINTER_UP;
    return ui_host_menu_dispatch_input(host,&e)==UI_STATUS_OK;
}
static void count_group(const ui_menu_model_entry_t *entry,void *data)
{size_t *n=(size_t *)data;CHECK(entry->group&&entry->state.visible);++*n;}
int main(void)
{
    ui_host_config_t hc={0};ui_host_t *a,*b;ui_command_desc_t command={0};ui_menu_item_desc_t item={0};
    ui_menu_popup_desc_t popup={0};ui_component_desc_t desc={0};ui_component_t *tree,*form,*status;
    ui_element_presentation_t p={0};ui_pixel_buffer_t pixels={0};ui_command_state_t state={0};ui_input_event_t input={0};
    ui_field_desc_t field={0};char id[40],title[96];size_t n=0;uint64_t caps=0;int i;
    hc.size=sizeof(hc);hc.api_version=4;a=ui_host_create(&hc);b=ui_host_create(&hc);CHECK(a&&b);
    CHECK(ui_host_resize(a,400,300)==UI_STATUS_OK);CHECK(ui_host_menu_get_capabilities(a,&caps)==UI_STATUS_OK&&(caps&UI_MENU_CAP_OFFSCREEN));
    command.size=sizeof(command);command.id="test.action";command.title="Action";command.handler=run;CHECK(ui_host_register_command(a,&command)==UI_STATUS_OK);
    item.size=sizeof(item);item.menu_path="File";item.command_id=command.id;
    for(i=0;i<32;++i){snprintf(id,sizeof(id),"item-%02d",i);snprintf(title,sizeof(title),"Menu %02d 中文长标题",i);
        item.id=id;item.title=title;item.order=i;CHECK(ui_host_register_menu_item(a,&item)==UI_STATUS_OK);}
    item.id="nested";item.title="Nested";item.menu_path="File/Nested";CHECK(ui_host_register_menu_item(a,&item)==UI_STATUS_OK);
    CHECK(ui_host_visit_menu(a,"",count_group,&n)==UI_STATUS_OK&&n==1);
    popup.size=sizeof(popup);popup.path="File";popup.anchor.size=sizeof(popup.anchor);popup.target_id=42;
    CHECK(ui_host_show_menu(a,&popup)==UI_STATUS_OK);CHECK(ui_host_menu_flush(a,64)==UI_STATUS_OK);
    CHECK(ui_host_menu_get_presentation(b,"menu-item-item-00",&p)==UI_STATUS_NOT_FOUND);
    CHECK(menu_click(a,"menu-item-item-00")&&calls==1);
    CHECK(ui_host_show_menu(a,&popup)==UI_STATUS_OK);
    p.size=sizeof(p);CHECK(ui_host_menu_get_presentation(a,"menu-item-item-00",&p)==UI_STATUS_OK&&p.visible&&p.clip.height>0);
    state.size=sizeof(state);state.visible=1;state.enabled=0;CHECK(ui_host_set_command_state(a,command.id,&state)==UI_STATUS_OK);
    CHECK(!menu_click(a,"menu-item-item-00")&&calls==1);state.enabled=1;CHECK(ui_host_set_command_state(a,command.id,&state)==UI_STATUS_OK);
    for(i=0;i<23;++i)CHECK(menu_click(a,"down"));CHECK(menu_click(a,"menu-item-item-31")&&calls==2);
    CHECK(ui_host_show_menu(a,&popup)==UI_STATUS_OK);input.size=sizeof(input);input.kind=UI_INPUT_KEY_DOWN;input.key_code=27;
    CHECK(ui_host_menu_dispatch_input(a,&input)==UI_STATUS_OK);CHECK(ui_host_menu_get_presentation(a,"close",&p)==UI_STATUS_NOT_FOUND&&calls==2);
    desc.size=sizeof(desc);desc.id="tree";desc.title="Tree";desc.kind=UI_COMPONENT_TREE;desc.source=source;
    CHECK(ui_component_register(a,&desc,&tree)==UI_STATUS_OK);CHECK(ui_component_mount_offscreen(tree,230,240,96)==UI_STATUS_OK);
    CHECK(ui_component_flush(tree,64)==UI_STATUS_OK);p.size=sizeof(p);
    CHECK(ui_component_get_presentation(tree,"tree-label-1",&p)==UI_STATUS_OK&&p.visible&&!p.text_overflow&&p.rect.width>150);
    CHECK(ui_component_get_presentation(tree,"tree-label-2",&p)==UI_STATUS_OK&&p.visible&&!p.text_overflow&&strstr(p.text_utf8,"CHILD"));
    CHECK(ui_component_show_menu(tree,1,"File")==UI_STATUS_OK);{uint64_t deleted=1;CHECK(ui_component_remove_rows(tree,&deleted,1)==UI_STATUS_OK);}
    CHECK(ui_host_menu_get_presentation(a,"close",&p)==UI_STATUS_NOT_FOUND);
    pixels.size=sizeof(pixels);CHECK(ui_component_capture_rgba(tree,230,240,144,&pixels)==UI_STATUS_OK&&pixels.width==345&&pixels.height==360);
    pixels.capacity=pixels.stride*pixels.height;pixels.pixels=(uint8_t *)malloc(pixels.capacity);CHECK(pixels.pixels!=NULL);
    CHECK(ui_component_capture_rgba(tree,230,240,144,&pixels)==UI_STATUS_OK);
    CHECK(pixels.pixels[3]==255&&pixels.pixels[pixels.capacity-1]==255);
    free(pixels.pixels);pixels.pixels=NULL;pixels.stride=SIZE_MAX;CHECK(ui_component_capture_rgba(tree,230,240,144,&pixels)==UI_STATUS_INVALID_ARGUMENT);
    field.size=sizeof(field);field.id="name";field.title="名称";field.kind=UI_VALUE_TEXT;field.flags=UI_VALUE_REQUIRED;
    memset(&desc,0,sizeof(desc));desc.size=sizeof(desc);desc.id="form";desc.title="Form";desc.kind=UI_COMPONENT_FORM;desc.fields=&field;desc.field_count=1;
    CHECK(ui_component_register(a,&desc,&form)==UI_STATUS_OK);CHECK(ui_component_mount_offscreen(form,300,240,96)==UI_STATUS_OK);
    CHECK(ui_component_get_presentation(form,"field-name",&p)==UI_STATUS_OK&&p.enabled);
    input.kind=UI_INPUT_POINTER_DOWN;input.pointer_button=1;input.x=p.clip.x+5;input.y=p.clip.y+5;
    CHECK(ui_component_dispatch_input(form,&input)==UI_STATUS_OK);input.kind=UI_INPUT_TEXT;input.text_utf8="draft 文本";
    CHECK(ui_component_dispatch_input(form,&input)==UI_STATUS_OK);CHECK(ui_component_get_presentation(form,"field-name",&p)==UI_STATUS_OK&&p.focused&&!strcmp(p.text_utf8,"draft 文本"));
    memset(&desc,0,sizeof(desc));desc.size=sizeof(desc);desc.id="status";desc.title="状态";desc.kind=UI_COMPONENT_STATUS;
    CHECK(ui_component_register(a,&desc,&status)==UI_STATUS_OK);CHECK(ui_component_mount_offscreen(status,230,60,96)==UI_STATUS_OK);
    CHECK(ui_component_set_text(status,"ready 中文")==UI_STATUS_OK);CHECK(ui_component_get_presentation(status,"title",&p)==UI_STATUS_OK&&p.visible&&strstr(p.text_utf8,"ready"));
    {char legacy_path[2048];memset(legacy_path,'x',sizeof(legacy_path)-1);legacy_path[sizeof(legacy_path)-1]=0;
     item.id="long-path";item.title="Legacy path";item.menu_path=legacy_path;CHECK(ui_host_register_menu_item(a,&item)==UI_STATUS_OK);
     popup.path=legacy_path;CHECK(ui_host_show_menu(a,&popup)==UI_STATUS_OK);CHECK(menu_click(a,"menu-item-long-path")&&calls==3);}
    EnumThreadWindows(GetCurrentThreadId(),count_window,0);CHECK(windows==0);
    ui_host_destroy(a);ui_host_destroy(b);
    printf("Real light/template offscreen: %d failures; thread windows=%d; no physical IME/DPI claim\n",failures,windows);return failures?1:0;
}
