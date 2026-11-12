#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ui_internal.h"
#include "../examples/p2_support.h"
static unsigned checks,failures;
#define CHECK(x) do{++checks;if(!(x)){++failures;fprintf(stderr,"line %d: %s\n",__LINE__,#x);}}while(0)
typedef struct model {unsigned value;p2_app_t app;} model_t;
static ui_status_t revision(void *data,unsigned resource,char out[128])
{model_t *m=data;(void)resource;snprintf(out,128,"r%u",m->value);return UI_STATUS_OK;}
static ui_status_t describe(void *data,unsigned resource,yyjson_mut_doc *doc,yyjson_mut_val *root)
{model_t *m=data;char rev[128];(void)revision(m,resource,rev);yyjson_mut_obj_add_uint(doc,root,"value",m->value);yyjson_mut_obj_add_strcpy(doc,root,"revision",rev);return UI_STATUS_OK;}
static ui_status_t run(void *data,const char *id,yyjson_val *params,yyjson_mut_doc *doc,yyjson_mut_val *root,uint64_t call)
{model_t *m=data;(void)id;(void)params;(void)call;++m->value;return describe(m,0,doc,root);}
static const char *parameters="{}";
static ui_status_t collect(const ui_cap_ui_context_t *context,char *out,size_t capacity,size_t *length,void *data)
{(void)context;(void)data;*length=strlen(parameters);if(*length>=capacity)return UI_STATUS_LIMIT_EXCEEDED;memcpy(out,parameters,*length+1);return UI_STATUS_OK;}
int main(void)
{
 static const char output[]="{\"type\":\"object\",\"properties\":{\"value\":{\"type\":\"integer\"},\"revision\":{\"type\":\"string\"},\"proof\":{\"type\":\"boolean\"}},\"required\":[\"value\",\"revision\",\"proof\"],\"additionalProperties\":false}";
 static const p2_spec_t specs[]={{"fixture.write","Controlled invalid output","write","{\"type\":\"object\",\"properties\":{\"text\":{\"type\":\"string\"}},\"additionalProperties\":false}",output,"fixture",0}};
 const char *resources[]={"fixture.document"};ui_cap_resource_kind_t kinds[]={UI_CAP_DOCUMENT};model_t m={0};ui_app_context_t context={0};ui_host_config_t config={0};ui_panel_desc_t panel={0};ui_menu_item_desc_t item={0};ui_native_shell_config_t native={0};HWND window,keeper;uint64_t call;ui_cap_result_v2_t result={0};
 window=CreateWindowExW(0,L"STATIC",L"P2 native content lifetime",WS_OVERLAPPEDWINDOW,-16000,-16000,600,400,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(window!=NULL);
 config.size=sizeof(config);config.api_version=10;config.native_parent=window;context.size=sizeof(context);context.host=ui_host_create(&config);CHECK(context.host!=NULL);if(!context.host)return 1;m.app.run=run;m.app.revision=revision;m.app.describe=describe;CHECK(p2_register(&m.app,&context,&m,specs,1,resources,kinds,1)==UI_STATUS_OK);
 panel.size=sizeof(panel);panel.id="content";panel.title="Keep content";panel.entry_url="";panel.kind=UI_PANEL_SIDEBAR;panel.dock_region=UI_LAYOUT_REGION_LEFT_SIDEBAR;CHECK(ui_host_register_panel(context.host,&panel)==UI_STATUS_OK);
 item.size=sizeof(item);item.id=item.command_id="fixture.write";item.title="Apply";item.menu_path="Fixture";CHECK(ui_host_register_menu_item(context.host,&item)==UI_STATUS_OK);CHECK(p2_bind_menu(&m.app,item.id,item.command_id,collect)==UI_STATUS_OK);
 native.size=sizeof(native);native.host=context.host;native.native_parent=window;context.shell=ui_native_shell_create(&native);CHECK(context.shell!=NULL);m.app.context=context;keeper=CreateWindowExW(0,L"STATIC",L"must survive menu projection",WS_CHILD,0,0,100,30,(HWND)ui_native_shell_panel_handle(context.shell,"content"),NULL,GetModuleHandleW(NULL),NULL);CHECK(IsWindow(keeper));
 parameters="{\"text\":\"prefix\\u0000suffix\"}";ui_capabilities_human_begin(context.host,&m);CHECK(ui_capabilities_ui_action(context.host,item.id,&m)==UI_STATUS_OK);ui_capabilities_human_end(context.host);call=m.app.resources[0].last_call;result.size=sizeof(result);result.version=2;CHECK(call!=0&&ui_capability_get_result_v2(context.host,call,&result)==UI_STATUS_OK&&result.outcome==UI_CAP2_FAILED&&m.value==0);CHECK(ui_capability_forget_v2(context.host,call)==UI_STATUS_OK);
 parameters="{}";ui_capabilities_human_begin(context.host,&m);CHECK(ui_capabilities_ui_action(context.host,item.id,&m)==UI_STATUS_OK);ui_capabilities_human_end(context.host);call=m.app.resources[0].last_call;CHECK(call!=0&&m.value==1);result.size=sizeof(result);result.version=2;CHECK(ui_capability_get_result_v2(context.host,call,&result)==UI_STATUS_OK&&result.outcome==UI_CAP2_UNKNOWN&&!strcmp(result.code,"invalid_output"));
 p2_poll(&context,&m.app);CHECK(m.app.caps[0].verify_binding!=0);CHECK(IsWindow(keeper));
 CHECK(ui_capability_cancel_v2(context.host,call)==UI_STATUS_OK);CHECK(ui_capability_forget_v2(context.host,call)==UI_STATUS_PENDING);ui_capabilities_human_begin(context.host,&m);CHECK(ui_capabilities_ui_action(context.host,item.id,&m)==UI_STATUS_PERMISSION_DENIED);ui_capabilities_human_end(context.host);CHECK(m.value==1);
 ui_native_shell_destroy(context.shell);ui_host_destroy(context.host);p2_dispose(&m.app);DestroyWindow(window);printf("P2 native UNKNOWN menu/content (not physical): checks=%u failures=%u\n",checks,failures);return failures?1:0;
}
