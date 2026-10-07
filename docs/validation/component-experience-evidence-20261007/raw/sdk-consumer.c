#include <stdio.h>
#include <stdlib.h>
#include "ui_framework/components.h"
_Static_assert(UI_FRAMEWORK_API_VERSION==8,"current SDK");
_Static_assert(offsetof(ui_field_desc_t,help)==64,"complete help offset");
int main(void){ui_host_config_t h={0};ui_column_desc_t col={0};ui_component_desc_t d={0};ui_component_t *c=NULL;int width=0;size_t n=0;void *data;h.size=sizeof(h);h.api_version=8;ui_host_t *host=ui_host_create(&h);col.size=sizeof(col);col.id="stable-column";col.title="SDK table";col.kind=UI_VALUE_TEXT;col.width=88;d.size=sizeof(d);d.id="sdk-table";d.kind=UI_COMPONENT_TABLE;d.columns=&col;d.column_count=1;if(!host||ui_component_register(host,&d,&c)!=UI_STATUS_OK||ui_component_set_column_width(c,col.id,160)!=UI_STATUS_OK||ui_component_save_columns(c,NULL,0,&n)!=UI_STATUS_OK)return 1;data=malloc(n);if(!data||ui_component_save_columns(c,data,n,&n)!=UI_STATUS_OK||ui_component_reset_columns(c,NULL)!=UI_STATUS_OK||ui_component_restore_columns(c,data,n)!=UI_STATUS_OK||ui_component_get_column_width(c,col.id,&width)!=UI_STATUS_OK||width!=160)return 2;free(data);ui_host_destroy(host);printf("SDK public headers/import/DLL columns 160 and UCW1 %zu bytes PASS\n",n);return 0;}
