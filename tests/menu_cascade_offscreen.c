#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "ui_framework/menus.h"
static int failures,calls,windows;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);++failures;}}while(0)
static void run(ui_host_t *h,uint64_t r,const char *c,const char *p,const char *s,void *u)
{(void)c;(void)s;(void)u;CHECK(strstr(p,"\"77\"")!=NULL);++calls;(void)ui_host_reply(h,r,1,"{}");}
static BOOL CALLBACK window(HWND w,LPARAM u){wchar_t name[80];(void)u;GetClassNameW(w,name,80);if(!wcscmp(name,L"UIFrameworkRegisteredMenu4"))++windows;return TRUE;}
static int input(ui_host_t *h,const char *id,int move,int generic)
{ui_element_presentation_t p={0};ui_input_event_t e={0};p.size=sizeof(p);if((generic?ui_host_menu_get_presentation(h,id,&p):ui_host_menu_get_item_presentation(h,id,&p))!=UI_STATUS_OK||!p.visible||!p.enabled)return 0;e.size=sizeof(e);e.x=p.clip.x+6;e.y=p.clip.y+8;e.kind=move?UI_INPUT_POINTER_MOVE:UI_INPUT_POINTER_DOWN;e.pointer_button=1;if(ui_host_menu_dispatch_input(h,&e)!=UI_STATUS_OK)return 0;if(move)return 1;e.kind=UI_INPUT_POINTER_UP;return ui_host_menu_dispatch_input(h,&e)==UI_STATUS_OK;}
int main(void)
{
 ui_host_config_t config={0};ui_host_t *h;ui_command_desc_t c={0};ui_menu_item_desc_t item={0};ui_menu_popup_desc_t menu={0};ui_element_presentation_t p={0};char id[40],path[1024]="Deep",next[1024];int pages=0,depth=0;
 config.size=sizeof(config);config.api_version=6;h=ui_host_create(&config);CHECK(h!=NULL);if(!h)return 1;CHECK(ui_host_resize(h,400,300)==UI_STATUS_OK);
 c.size=sizeof(c);c.id="probe";c.title="Probe";c.handler=run;CHECK(ui_host_register_command(h,&c)==UI_STATUS_OK);item.size=sizeof(item);item.title="Paged command 中文";item.command_id=c.id;item.menu_path="Many";
 for(int i=0;i<128;i++){snprintf(id,sizeof(id),"paged.%03d",i);item.id=id;item.order=i;CHECK(ui_host_register_menu_item(h,&item)==UI_STATUS_OK);}
 menu.size=sizeof(menu);menu.path="Many";menu.target_id=77;menu.anchor.size=sizeof(menu.anchor);CHECK(ui_host_show_menu(h,&menu)==UI_STATUS_OK);
 while(pages<128&&input(h,"down",0,1))++pages;p.size=sizeof(p);CHECK(ui_host_menu_get_item_presentation(h,"paged.127",&p)==UI_STATUS_OK&&p.visible&&p.enabled);CHECK(input(h,"paged.127",0,0)&&calls==1);CHECK(pages>0&&pages<128);
 item.id="paged.128";CHECK(ui_host_register_menu_item(h,&item)==UI_STATUS_OK);CHECK(ui_host_show_menu(h,&menu)==UI_STATUS_LIMIT_EXCEEDED);
 for(int i=1;i<=20;i++){snprintf(next,sizeof(next),"%s/L%d",path,i);snprintf(path,sizeof(path),"%s",next);}item.id="deep.leaf";item.menu_path=path;CHECK(ui_host_register_menu_item(h,&item)==UI_STATUS_OK);menu.path="Deep";CHECK(ui_host_show_menu(h,&menu)==UI_STATUS_OK);{ui_pixel_buffer_t pixels={0};pixels.size=sizeof(pixels);CHECK(ui_host_menu_capture_rgba(h,&pixels)==UI_STATUS_OK&&pixels.height<=36);}snprintf(path,sizeof(path),"Deep");
 for(int i=1;i<=20;i++){snprintf(next,sizeof(next),"%s/L%d",path,i);if(!input(h,next,1,0))break;snprintf(path,sizeof(path),"%s",next);++depth;}
 CHECK(depth>2&&depth<20);CHECK(ui_host_menu_get_presentation(h,"heading",&p)==UI_STATUS_NOT_FOUND);CHECK(calls==1);
 menu.path="Deep";CHECK(ui_host_show_menu(h,&menu)==UI_STATUS_OK);{ui_input_event_t e={0};e.size=sizeof(e);e.kind=UI_INPUT_POINTER_DOWN;e.x=-1;e.y=-1;e.pointer_button=1;CHECK(ui_host_menu_dispatch_input(h,&e)==UI_STATUS_OK);}CHECK(ui_host_menu_get_presentation(h,"heading",&p)==UI_STATUS_NOT_FOUND);
 ui_host_destroy(h);EnumThreadWindows(GetCurrentThreadId(),window,0);CHECK(windows==0);printf("128 items, %d page steps; shared cascade budget boundary at %d; command once; no menu HWND: %d failures\n",pages,depth,failures);return failures?1:0;
}
