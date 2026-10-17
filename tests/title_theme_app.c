/* Diagnostic package: only public C. No host internals or injected JS. */
#include "ui_framework/application.h"
#include "ui_framework/components.h"
#include "ui_framework/theme.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct probe {ui_app_context_t context;ui_component_t *form;} probe_t;
static void record(probe_t *p,const char *phase,ui_status_t status)
{FILE *f=NULL;ui_theme_info_t t={0};char title[4096];size_t required=0;t.size=sizeof(t);ui_host_get_theme(p->context.host,&t);if(ui_workspace_get_instance_title(p->context.workspace,p->context.instance_id,title,sizeof(title),&required)!=UI_STATUS_OK)strcpy_s(title,sizeof(title),"unregistered");if(!fopen_s(&f,"public-contract-events.log","a")){fprintf(f,"%s id=%llu status=%d mode=%d effective=%d generation=%llu title=%s\n",phase,(unsigned long long)p->context.instance_id,(int)status,(int)t.mode,(int)t.effective,(unsigned long long)t.generation,title);fclose(f);}}
static void theme(ui_host_t *h,const ui_theme_info_t *t,void *data)
{(void)h;(void)t;record(data,"theme",UI_STATUS_OK);}
static void command(ui_host_t *h,uint64_t request,const char *id,const char *params,const char *source,void *data)
{probe_t *p=data;ui_status_t s;ui_theme_info_t t={0};ui_cell_t text={0};(void)params;(void)source;if(!strcmp(id,"probe.rename")){text.size=sizeof(text);s=ui_component_get_field(p->form,"title",&text);if(s==UI_STATUS_OK)s=ui_workspace_set_instance_title(p->context.workspace,p->context.instance_id,text.text);}else{t.size=sizeof(t);s=ui_host_get_theme(h,&t);if(s==UI_STATUS_OK)s=ui_host_set_theme(h,t.effective==UI_THEME_DARK?UI_THEME_LIGHT:UI_THEME_DARK);}record(p,id,s);ui_host_reply(h,request,s==UI_STATUS_OK,"{}");}
static ui_status_t UI_APP_CALL create(const ui_app_context_t *ctx,void **out,ui_assistant_config_t *assistant)
{probe_t *p=calloc(1,sizeof(*p));ui_component_desc_t d={0};ui_field_desc_t field={0};ui_command_desc_t c={0};ui_toolbar_item_desc_t tool={0};ui_toolbar_desc_t bar={0};ui_layout_desc_t layout={0};const char *ids[]={"probe.rename","probe.theme"},*names[]={"Apply title","Toggle theme"};ui_status_t s;(void)assistant;if(!p)return UI_STATUS_OUT_OF_MEMORY;p->context=*ctx;*out=p;layout.size=sizeof(layout);layout.toolbar_height=32;s=ui_host_set_layout(ctx->host,&layout);if(s!=UI_STATUS_OK)return s;bar.size=sizeof(bar);bar.id="main";bar.title="Public contract";bar.visible=1;s=ui_host_register_toolbar(ctx->host,&bar);if(s!=UI_STATUS_OK)return s;for(int i=0;i<2;++i){c.size=sizeof(c);c.id=ids[i];c.title=names[i];c.handler=command;c.user_data=p;s=ui_host_register_command(ctx->host,&c);if(s!=UI_STATUS_OK)return s;tool.size=sizeof(tool);tool.id=ids[i];tool.toolbar_id="main";tool.command_id=ids[i];tool.title=names[i];tool.order=i;s=ui_host_register_toolbar_item(ctx->host,&tool);if(s!=UI_STATUS_OK)return s;}field.size=sizeof(field);field.id="title";field.title="Title";field.kind=UI_VALUE_TEXT;d.size=sizeof(d);d.id="contract.form";d.kind=UI_COMPONENT_FORM;d.title="Public title and theme";d.fields=&field;d.field_count=1;d.commands.submit="probe.rename";s=ui_component_register(ctx->host,&d,&p->form);if(s!=UI_STATUS_OK)return s;return ui_host_set_theme_callback(ctx->host,theme,p);}
static ui_status_t UI_APP_CALL mount(void *data,const ui_app_context_t *ctx)
{probe_t *p=data;ui_cell_t value={0};ui_status_t s;p->context=*ctx;value.size=sizeof(value);value.kind=UI_VALUE_TEXT;value.text="共享标题 中文 * — Long title 012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789 TITLE_END";s=ui_component_set_field(p->form,"title",&value);if(s==UI_STATUS_OK)s=ui_component_mount(p->form,ui_shell_get_content_slot(ui_host_get_shell(ctx->host),NULL));if(s==UI_STATUS_OK)s=ui_workspace_set_instance_title(ctx->workspace,ctx->instance_id,"Shared initial title 中文");record(p,"mounted",s);return s;}
static ui_app_close_decision_t UI_APP_CALL close_app(void *data,ui_app_close_reason_t why)
{(void)data;(void)why;return UI_APP_CLOSE_ALLOW;}
static void UI_APP_CALL unmount(void *data)
{probe_t *p=data;record(p,"unmount",UI_STATUS_OK);ui_host_set_theme_callback(p->context.host,NULL,NULL);ui_component_unregister(p->form);p->form=NULL;}
static void UI_APP_CALL destroy(void *data){free(data);}
UI_APP_EXPORT const ui_app_descriptor_t *UI_APP_CALL ui_app_query_v1(void)
{static const ui_app_descriptor_t d={sizeof(d),1,UI_FRAMEWORK_API_VERSION,create,mount,NULL,close_app,unmount,destroy,NULL};return &d;}
