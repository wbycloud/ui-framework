#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "ui_framework/language.h"
#include "ui_framework/components.h"
_Static_assert(UI_FRAMEWORK_API_VERSION==9,"matched current SDK");
_Static_assert(sizeof(ui_language_info_t)==128,"language C ABI");
_Static_assert(offsetof(ui_field_text_t,option_labels)==48,"text C ABI");
static int notified;
static void changed(ui_host_t *host,const ui_language_info_t *info,void *data)
{(void)host;(void)data;if(info->source==UI_LANGUAGE_APPLICATION)++notified;}
int main(void)
{
 ui_host_config_t config={0};ui_language_info_t info={0};ui_column_desc_t col={0};ui_component_desc_t desc={0};ui_component_t *table=NULL;int width=0;
 config.size=sizeof(config);config.api_version=UI_FRAMEWORK_API_VERSION;if(ui_framework_initialize()!=UI_STATUS_OK)return 1;ui_host_t *host=ui_host_create(&config);if(!host)return 2;
 if(ui_host_set_language_callback(host,changed,NULL)!=UI_STATUS_OK||ui_host_set_language(host,"en-US")!=UI_STATUS_OK||notified!=1)return 3;
 info.size=sizeof(info);if(ui_host_get_language(host,&info)!=UI_STATUS_OK||strcmp(info.language,"en-US")||info.source!=UI_LANGUAGE_APPLICATION)return 4;
 uint64_t version=info.generation;if(ui_host_set_language(host,"fr-FR")!=UI_STATUS_UNSUPPORTED||ui_host_get_language(host,&info)!=UI_STATUS_OK||info.generation!=version)return 5;
 col.size=sizeof(col);col.id="stable-column";col.title="Name";col.kind=UI_VALUE_TEXT;col.width=88;desc.size=sizeof(desc);desc.id="table";desc.kind=UI_COMPONENT_TABLE;desc.columns=&col;desc.column_count=1;
 if(ui_component_register(host,&desc,&table)!=UI_STATUS_OK||ui_component_set_column_width(table,col.id,160)!=UI_STATUS_OK||ui_component_set_column_title(table,col.id,"名称")!=UI_STATUS_OK||ui_host_set_language(host,"zh-CN")!=UI_STATUS_OK)return 6;
 if(ui_component_get_column_width(table,col.id,&width)!=UI_STATUS_OK||width!=160||notified!=2)return 7;
 if(ui_host_set_language_callback(host,NULL,NULL)!=UI_STATUS_OK)return 8;ui_host_destroy(host);puts("Extracted API9 C header/import/shared DLL: language/query/notify/text/unchanged width PASS");return 0;
}
