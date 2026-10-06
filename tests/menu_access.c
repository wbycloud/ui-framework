#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "ui_framework/menus.h"
#include "ui_framework/components.h"
_Static_assert(sizeof(ui_menu_group_desc_t)==40&&offsetof(ui_menu_group_desc_t,access_key)==32,"group prefix");
_Static_assert(sizeof(ui_menu_item_desc_t)==64&&offsetof(ui_menu_item_desc_t,access_key)==56,"item prefix");
_Static_assert(sizeof(ui_menu_model_entry_t)==88&&offsetof(ui_menu_model_entry_t,access_key)==80,"model prefix");
_Static_assert(sizeof(ui_component_desc_t)==160&&offsetof(ui_component_desc_t,web_backend)==136&&offsetof(ui_component_desc_t,sort_command)==144&&offsetof(ui_component_desc_t,selection_flags)==152,"component prefix");
static int failures,calls;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);++failures;}}while(0)
static void run(ui_host_t *h,uint64_t r,const char *c,const char *p,const char *s,void *u)
{(void)c;(void)p;(void)s;(void)u;++calls;(void)ui_host_reply(h,r,1,"{}");}
static ui_status_t key(ui_host_t *h,unsigned k,unsigned mods,int up)
{ui_input_event_t e={0};e.size=sizeof(e);e.kind=up?UI_INPUT_KEY_UP:UI_INPUT_KEY_DOWN;e.key_code=k;e.modifiers=mods;return ui_host_menu_dispatch_input(h,&e);}
static void press(ui_host_t *h,unsigned k,unsigned m)
{CHECK(key(h,k,m,0)==UI_STATUS_OK);CHECK(key(h,k,m,1)==UI_STATUS_OK);}
static int focused(ui_host_t *h,const char *id)
{ui_element_presentation_t p={0};p.size=sizeof(p);return ui_host_menu_get_item_presentation(h,id,&p)==UI_STATUS_OK&&p.focused;}
int main(void)
{
 ui_host_config_t hc={0};ui_host_t *a,*b;ui_command_desc_t cmd={0};ui_menu_item_desc_t item={0};ui_menu_group_desc_t g={0};ui_command_state_t state={0};
 hc.size=sizeof(hc);hc.api_version=UI_FRAMEWORK_API_VERSION;a=ui_host_create(&hc);b=ui_host_create(&hc);CHECK(a&&b);CHECK(ui_host_resize(a,180,220)==UI_STATUS_OK);
 cmd.size=sizeof(cmd);cmd.id="test.run";cmd.title="Run";cmd.handler=run;CHECK(ui_host_register_command(a,&cmd)==UI_STATUS_OK);
 item.size=sizeof(item);item.id="one";item.title="One";item.menu_path="File";item.command_id=cmd.id;item.order=1;item.access_key='R';CHECK(ui_host_register_menu_item(a,&item)==UI_STATUS_OK);
 item.id="two";item.title="Two";item.order=2;CHECK(ui_host_register_menu_item(a,&item)==UI_STATUS_OK);
 item.id="child";item.title="Child";item.access_key='C';item.menu_path="File/Nested";CHECK(ui_host_register_menu_item(a,&item)==UI_STATUS_OK);
 g.size=sizeof(g);g.path="File";g.title="File";g.access_key='F';CHECK(ui_host_register_menu_group(a,&g)==UI_STATUS_OK);g.path="File/Nested";g.title="Nested";g.order=3;g.access_key='N';CHECK(ui_host_register_menu_group(a,&g)==UI_STATUS_OK);
 /* API4 public entry already exists; baseline fails Alt activation here. */
 {ui_menu_anchor_t anchor={0};anchor.size=sizeof(anchor);CHECK(ui_host_show_tooltip(a,&anchor,"Visible tooltip before Alt")==UI_STATUS_OK);press(a,VK_MENU,0);CHECK(focused(a,"File"));press(a,VK_ESCAPE,0);}
 press(a,VK_MENU,0);CHECK(focused(a,"File"));press(a,VK_RETURN,0);CHECK(focused(a,"one"));
 press(a,VK_DOWN,0);CHECK(focused(a,"two"));press(a,VK_RETURN,0);CHECK(calls==1);
 CHECK(key(b,VK_RETURN,0,0)==UI_STATUS_NOT_FOUND&&calls==1);
 press(a,VK_MENU,0);press(a,VK_ESCAPE,0);CHECK(key(a,VK_RETURN,0,0)==UI_STATUS_NOT_FOUND);
 press(a,VK_MENU,0);CHECK(key(a,VK_MENU,0,0)==UI_STATUS_OK);CHECK(key(a,VK_MENU,0,0)==UI_STATUS_OK);CHECK(key(a,VK_MENU,0,1)==UI_STATUS_OK);{ui_element_presentation_t p={0};p.size=sizeof(p);CHECK(ui_host_menu_get_presentation(a,"close",&p)==UI_STATUS_NOT_FOUND);}
 CHECK(key(a,'F',UI_INPUT_MODIFIER_ALT|UI_INPUT_MODIFIER_CONTROL,0)==UI_STATUS_NOT_FOUND);
 press(a,'F',UI_INPUT_MODIFIER_ALT);CHECK(focused(a,"one"));press(a,'R',0);CHECK(focused(a,"two")&&calls==1);press(a,'R',0);CHECK(focused(a,"one")&&calls==1);
 press(a,'N',0);CHECK(focused(a,"child"));press(a,VK_LEFT,0);CHECK(focused(a,"one"));press(a,'N',0);
 CHECK(key(a,VK_RETURN,0,0)==UI_STATUS_OK&&calls==2);CHECK(key(a,VK_RETURN,0,0)==UI_STATUS_OK&&calls==2);CHECK(key(a,VK_RETURN,0,1)==UI_STATUS_OK);
 state.size=sizeof(state);state.visible=1;CHECK(ui_host_set_command_state(a,cmd.id,&state)==UI_STATUS_OK);
 press(a,'F',UI_INPUT_MODIFIER_ALT);CHECK(key(a,'R',0,0)==UI_STATUS_NOT_FOUND);(void)key(a,'R',0,1);CHECK(calls==2);press(a,VK_ESCAPE,0);
 state.enabled=1;CHECK(ui_host_set_command_state(a,cmd.id,&state)==UI_STATUS_OK);
 {char id[32],title[32];for(int i=0;i<40;++i){snprintf(id,sizeof(id),"overflow.%d",i);snprintf(title,sizeof(title),"Overflow %d",i);item.id=id;item.title=title;item.menu_path="File";item.order=10+i;item.access_key=i==39?'Z':0;CHECK(ui_host_register_menu_item(a,&item)==UI_STATUS_OK);}}
 press(a,'F',UI_INPUT_MODIFIER_ALT);press(a,'Z',0);CHECK(calls==3);
 item.id="invalid";item.access_key='&';CHECK(ui_host_register_menu_item(a,&item)==UI_STATUS_INVALID_ARGUMENT);
 {ui_component_desc_t partial={0};ui_component_t *component=NULL;partial.size=offsetof(ui_component_desc_t,web_backend)+1;partial.id="partial-backend";partial.kind=UI_COMPONENT_STATUS;partial.web_backend=(ui_web_backend_t *)(uintptr_t)UINTPTR_MAX;
  CHECK(ui_component_register(a,&partial,&component)==UI_STATUS_OK);CHECK(ui_component_mount_offscreen(component,180,60,96)==UI_STATUS_OK);CHECK(ui_component_unregister(component)==UI_STATUS_OK);}
 ui_host_destroy(a);ui_host_destroy(b);printf("Menu Alt real template: %d failures, calls=%d\n",failures,calls);return failures?1:0;
}
