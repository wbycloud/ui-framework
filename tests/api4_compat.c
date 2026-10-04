#include "ui_framework/menus.h"
#include "ui_framework/webview2.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
_Static_assert(UI_FRAMEWORK_API_VERSION==4,"frozen SDK4");
_Static_assert(sizeof(ui_menu_group_desc_t)==32,"old group including padding");
_Static_assert(offsetof(ui_menu_group_desc_t,order)==24,"group offset");
_Static_assert(sizeof(ui_menu_item_desc_t)==56,"old item");
_Static_assert(sizeof(ui_webview2_backend_config_t)==24,"old WebView2 config");
static int failures,visited;
static void command_handler(ui_host_t *h,uint64_t id,const char *command,const char *params,const char *source,void *data)
{(void)command;(void)params;(void)source;(void)data;(void)ui_host_reply(h,id,1,"{}");}
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);++failures;}}while(0)
static void visit(const ui_menu_model_entry_t *e,void *data)
{(void)data;CHECK(e->group&&!strcmp(e->path,"File"));++visited;}
int main(void)
{
 ui_host_config_t config={0};ui_host_t *h;ui_menu_group_desc_t group;ui_menu_item_desc_t item={0};ui_command_desc_t command={0};ui_component_desc_t desc={0};ui_component_t *component=NULL;
 config.size=sizeof(config);config.api_version=4;h=ui_host_create(&config);CHECK(h!=NULL);
 /* Deliberately nonzero old tail padding must not become an access key. */
 memset(&group,0xa5,sizeof(group));group.size=sizeof(group);group.path="File";group.title="File";group.order=0;
 CHECK(ui_host_register_menu_group(h,&group)==UI_STATUS_OK);
 command.size=sizeof(command);command.id="old-command";command.title="Old command";command.handler=command_handler;CHECK(ui_host_register_command(h,&command)==UI_STATUS_OK);
 item.size=sizeof(item);item.id="old";item.title="Old item";item.menu_path="File";item.command_id=command.id;CHECK(ui_host_register_menu_item(h,&item)==UI_STATUS_OK);
 CHECK(ui_host_visit_menu(h,"",visit,NULL)==UI_STATUS_OK&&visited==1);
 desc.size=sizeof(desc);desc.id="old-component";desc.kind=UI_COMPONENT_STATUS;CHECK(ui_component_register(h,&desc,&component)==UI_STATUS_OK);
 CHECK(ui_component_unregister(component)==UI_STATUS_OK);ui_host_destroy(h);
 printf("Frozen SDK4 prefixes/dirty padding/component: %d failures, component_size=%zu\n",failures,sizeof(desc));return failures?1:0;
}
