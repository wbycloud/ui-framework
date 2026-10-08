#include "ui_framework/language.h"
#include "ui_framework/components.h"
#include "../src/ui_internal.h"
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif
_Static_assert(UI_FRAMEWORK_API_VERSION==9,"language contract is API9");
static int failures,notices,reentrant;
static int queries;
static ui_component_query_t late;
static void source(ui_component_t *c,const ui_component_query_t *q,void *data)
{(void)c;(void)data;++queries;late=*q;}
#ifdef _WIN32
static DWORD WINAPI wrong_thread(void *host)
{ui_language_info_t info={0};info.size=sizeof(info);return ui_host_set_language(host,"zh-CN")==UI_STATUS_INVALID_ARGUMENT&&ui_host_get_language(host,&info)==UI_STATUS_INVALID_ARGUMENT&&ui_host_set_language_callback(host,NULL,NULL)==UI_STATUS_INVALID_ARGUMENT?0:1;}
#endif
#define CHECK(x) do{if(!(x)){fprintf(stderr,"language line%d: %s\n",__LINE__,#x);++failures;}}while(0)
static void notice(ui_host_t *host,const ui_language_info_t *info,void *data)
{ui_language_info_t query={0};(void)data;query.size=sizeof(query);++notices;CHECK(ui_host_get_language(host,&query)==UI_STATUS_OK&&query.generation==info->generation);reentrant=ui_host_set_language(host,"zh-CN");}
static void UI_APP_CALL unused_handler(ui_host_t *h,uint64_t n,const char *id,const char *p,const char *s,void *d)
{(void)h;(void)n;(void)id;(void)p;(void)s;(void)d;}
int main(void)
{
 ui_host_config_t config={0};ui_language_info_t initial={0},a={0},b={0};ui_component_t *c=NULL;ui_column_desc_t column={0};ui_component_desc_t desc={0};ui_component_state_t before={0},after={0};int width=0;
 config.size=sizeof(config);config.api_version=UI_FRAMEWORK_API_VERSION;ui_host_t *first=ui_host_create(&config),*second=ui_host_create(&config);CHECK(first&&second);if(!first||!second)return 1;
#ifdef _WIN32
 HANDLE worker=CreateThread(NULL,0,wrong_thread,first,0,NULL);DWORD exit_code=1;CHECK(worker&&WaitForSingleObject(worker,10000)==WAIT_OBJECT_0&&GetExitCodeThread(worker,&exit_code)&&exit_code==0);if(worker)CloseHandle(worker);
#endif
 initial.size=a.size=b.size=sizeof(a);CHECK(ui_host_get_system_language(&initial)==UI_STATUS_OK);CHECK(ui_host_get_language(first,&a)==UI_STATUS_OK&&a.source!=UI_LANGUAGE_APPLICATION&&!strcmp(initial.language,a.language));
 printf("REAL_WINDOWS_UI first=%s effective=%s provider=%u fallback=%u\n",initial.system_language,initial.language,initial.system_provider,initial.fallback);
 CHECK(!strcmp(ui_language_map_windows("zh-Hant-TW"),"zh-CN"));CHECK(!strcmp(ui_language_map_windows("ZH-hk"),"zh-CN"));CHECK(!strcmp(ui_language_map_windows("en-GB"),"en-US"));CHECK(!strcmp(ui_language_map_windows("fr-FR"),"en-US"));
 CHECK(ui_host_set_language_callback(first,notice,NULL)==UI_STATUS_OK);CHECK(ui_host_set_language(first,"en-US")==UI_STATUS_OK&&notices==1&&reentrant==UI_STATUS_CANCELLED);CHECK(ui_host_set_language(first,"en-US")==UI_STATUS_OK&&notices==1);
 CHECK(ui_host_set_language(second,"zh-CN")==UI_STATUS_OK);CHECK(ui_host_get_language(first,&a)==UI_STATUS_OK&&a.source==UI_LANGUAGE_APPLICATION&&!strcmp(a.language,"en-US"));CHECK(ui_host_get_language(second,&b)==UI_STATUS_OK&&!strcmp(b.language,"zh-CN"));
 uint64_t version=a.generation;CHECK(ui_host_set_language(first,"fr-FR")==UI_STATUS_UNSUPPORTED);CHECK(ui_host_set_language(first,"")==UI_STATUS_INVALID_ARGUMENT);CHECK(ui_host_set_language(first,NULL)==UI_STATUS_INVALID_ARGUMENT);CHECK(ui_host_set_language(first,"zh_CN")==UI_STATUS_INVALID_ARGUMENT);CHECK(ui_host_get_language(first,&a)==UI_STATUS_OK&&a.generation==version&&!strcmp(a.language,"en-US")&&notices==1);
 a.size=sizeof(a)-1;CHECK(ui_host_get_language(first,&a)==UI_STATUS_INVALID_ARGUMENT);a.size=sizeof(a);
 column=(ui_column_desc_t){sizeof(column),"name","Name",UI_VALUE_TEXT,200};desc.size=sizeof(desc);desc.id="table";desc.title="Table";desc.kind=UI_COMPONENT_TABLE;desc.columns=&column;desc.column_count=1;desc.source=source;CHECK(ui_component_register(first,&desc,&c)==UI_STATUS_OK);CHECK(ui_component_set_column_width(c,"name",280)==UI_STATUS_OK);CHECK(ui_component_query(c,0,0,1,0,1)==UI_STATUS_OK&&queries==1);before.size=after.size=sizeof(before);CHECK(ui_component_get_state(c,&before)==UI_STATUS_OK);
 CHECK(ui_component_set_title(c,"表格")==UI_STATUS_OK);CHECK(ui_component_set_column_title(c,"name","名称")==UI_STATUS_OK);CHECK(ui_component_get_column_width(c,"name",&width)==UI_STATUS_OK&&width==280);CHECK(ui_component_get_state(c,&after)==UI_STATUS_OK&&before.generation==after.generation&&before.first==after.first);
 CHECK(ui_host_set_language(first,"zh-CN")==UI_STATUS_OK&&queries==1&&notices==2);ui_row_t row={0};row.size=sizeof(row);row.id=9007199254741001ULL;row.title="business 用户数据";ui_component_batch_t batch={0};batch.size=sizeof(batch);batch.component_generation=late.component_generation;batch.request_id=late.request_id;batch.first=late.first;batch.rows=&row;batch.row_count=1;batch.total_count=1;CHECK(ui_component_submit(c,&batch)==UI_STATUS_OK&&queries==1);CHECK(ui_component_get_state(c,&after)==UI_STATUS_OK&&after.generation==before.generation&&after.total_count==1);
 ui_component_t *form=NULL;ui_field_desc_t field={0};const char *options[]={"low","high"},*labels[]={"低","高"};field.size=sizeof(field);field.id="mode";field.title="Mode";field.kind=UI_VALUE_ENUM;field.options=options;field.option_count=2;ui_component_desc_t fd={0};fd.size=sizeof(fd);fd.id="form";fd.kind=UI_COMPONENT_FORM;fd.fields=&field;fd.field_count=1;CHECK(ui_component_register(first,&fd,&form)==UI_STATUS_OK);ui_cell_t value={0};value.size=sizeof(value);value.kind=UI_VALUE_ENUM;value.text="low";value.flags=UI_VALUE_MODIFIED;CHECK(ui_component_set_field(form,"mode",&value)==UI_STATUS_OK);ui_field_text_t text={0};text.size=sizeof(text);text.id="mode";text.title="模式";text.option_labels=labels;text.option_count=2;CHECK(ui_component_set_field_text(form,&text)==UI_STATUS_OK);text.option_count=1;CHECK(ui_component_set_field_text(form,&text)==UI_STATUS_INVALID_ARGUMENT);text.size=sizeof(text)-1;CHECK(ui_component_set_field_text(form,&text)==UI_STATUS_INVALID_ARGUMENT);CHECK(ui_component_get_field(form,"mode",&value)==UI_STATUS_OK&&!strcmp(value.text,"low")&&(value.flags&UI_VALUE_MODIFIED));
 ui_command_desc_t cmd={0};cmd.size=sizeof(cmd);cmd.id="stable";cmd.title="Before";cmd.handler=unused_handler;CHECK(ui_host_register_command(first,&cmd)==UI_STATUS_OK);CHECK(ui_host_set_title(first,UI_TEXT_COMMAND,"stable","After")==UI_STATUS_OK);CHECK(ui_host_set_title(first,UI_TEXT_COMMAND,"missing","After")==UI_STATUS_NOT_FOUND);
 CHECK(ui_host_reset_language(first)==UI_STATUS_OK&&notices==3);CHECK(ui_host_get_language(first,&a)==UI_STATUS_OK&&a.source!=UI_LANGUAGE_APPLICATION&&!strcmp(a.language,initial.language));
 ui_host_destroy(second);ui_host_destroy(first);printf("Language core %d failures; synthetic mapping is separate from real system query\n",failures);return failures?1:0;
}
