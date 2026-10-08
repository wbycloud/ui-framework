#include "ui_internal.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif
typedef struct language_text { const char *key,*zh,*en; } language_text_t;
#define UI_TEXT(key,zh,en) {key,zh,en},
static const language_text_t texts[]={
#include "language_catalog.inc"
};
#undef UI_TEXT
size_t ui_language_catalog_count(void){return sizeof(texts)/sizeof(texts[0]);}
void ui_language_catalog_entry(const ui_host_t *host,size_t index,const char **key,const char **value)
{ui_language_catalog_language(host?host->language.language:"en-US",index,key,value);}
void ui_language_catalog_language(const char *language,size_t index,const char **key,const char **value)
{if(index>=ui_language_catalog_count()){*key=*value="";return;}*key=texts[index].key;*value=!strcmp(language,"zh-CN")?texts[index].zh:texts[index].en;}
const char *ui_language_lookup(const char *language,const char *key)
{for(size_t i=0;i<ui_language_catalog_count();++i)if(!strcmp(texts[i].key,key))return !strcmp(language,"zh-CN")?texts[i].zh:texts[i].en;return key;}
const char *ui_language_text(const ui_host_t *host,const char *key)
{
 int chinese=host&&!strcmp(host->language.language,"zh-CN");
 if(!host){ui_language_info_t info={0};info.size=sizeof(info);(void)ui_host_get_system_language(&info);chinese=!strcmp(info.language,"zh-CN");}
 for(size_t i=0;i<ui_language_catalog_count();++i)if(!strcmp(texts[i].key,key))return chinese?texts[i].zh:texts[i].en;return key;
}
const char *ui_language_map_windows(const char *name)
{return name&&strlen(name)>=2&&tolower((unsigned char)name[0])=='z'&&tolower((unsigned char)name[1])=='h'&&(name[2]=='-'||!name[2])?"zh-CN":"en-US";}
int ui_language_thread_ok(const ui_host_t *host)
{
#ifdef _WIN32
 return host&&host->language_thread==GetCurrentThreadId();
#else
 return host!=NULL;
#endif
}
ui_status_t ui_host_get_system_language(ui_language_info_t *out)
{
 ui_language_info_t info={0};if(!out||out->size<sizeof(*out))return UI_STATUS_INVALID_ARGUMENT;
 info.size=sizeof(info);info.source=UI_LANGUAGE_PLATFORM_DEFAULT;info.system_provider=UI_LANGUAGE_NO_PLATFORM_UI;info.fallback=1;
#ifdef _WIN32
 ULONG count=0,length=0;wchar_t *names=NULL;
 if(GetUserPreferredUILanguages(MUI_LANGUAGE_NAME,&count,NULL,&length)&&length>1&&length<=8192){names=calloc(length,sizeof(*names));
  if(names&&GetUserPreferredUILanguages(MUI_LANGUAGE_NAME,&count,names,&length)&&count&&*names&&WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,names,-1,info.system_language,sizeof(info.system_language),NULL,NULL)>0)info.system_provider=UI_LANGUAGE_USER_PREFERRED_UI;
  free(names);}
 if(!*info.system_language){wchar_t name[LOCALE_NAME_MAX_LENGTH]={0};LANGID id=GetUserDefaultUILanguage();
  if(LCIDToLocaleName(MAKELCID(id,SORT_DEFAULT),name,LOCALE_NAME_MAX_LENGTH,0)&&WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,name,-1,info.system_language,sizeof(info.system_language),NULL,NULL)>0)info.system_provider=UI_LANGUAGE_USER_DEFAULT_UI;}
 if(*info.system_language){info.source=UI_LANGUAGE_WINDOWS_DISPLAY;info.fallback=!(tolower((unsigned char)info.system_language[0])=='z'&&tolower((unsigned char)info.system_language[1])=='h'||tolower((unsigned char)info.system_language[0])=='e'&&tolower((unsigned char)info.system_language[1])=='n');}
#endif
 strcpy(info.language,ui_language_map_windows(info.system_language));*out=info;return UI_STATUS_OK;
}
void ui_language_initialize(ui_host_t *host)
{
 host->language.size=sizeof(host->language);(void)ui_host_get_system_language(&host->language);host->language.generation=1;
#ifdef _WIN32
 host->language_thread=GetCurrentThreadId();
#endif
}
ui_status_t ui_host_get_language(const ui_host_t *host,ui_language_info_t *out)
{if(!ui_language_thread_ok(host)||!out||out->size<sizeof(*out))return UI_STATUS_INVALID_ARGUMENT;*out=host->language;return UI_STATUS_OK;}
ui_status_t ui_host_set_language_callback(ui_host_t *host,ui_language_callback_fn callback,void *data)
{if(!ui_language_thread_ok(host))return UI_STATUS_INVALID_ARGUMENT;if(host->dispatch_blocked)return UI_STATUS_CANCELLED;host->language_callback=callback;host->language_data=data;return UI_STATUS_OK;}
static ui_status_t apply_language(ui_host_t *host,const ui_language_info_t *value)
{
 char json[160];if(host->language_changing||host->dispatch_blocked)return UI_STATUS_CANCELLED;
 if(host->language.source==value->source&&host->language.fallback==value->fallback&&host->language.system_provider==value->system_provider&&!strcmp(host->language.language,value->language)&&!strcmp(host->language.system_language,value->system_language))return UI_STATUS_OK;
 ui_dispatch_enter(host);host->language_changing=1;uint64_t version=host->language.generation+1;host->language=*value;host->language.generation=version;
 (void)ui_host_close_menu(host);
 if(host->language_callback)host->language_callback(host,&host->language,host->language_data);
 ui_components_language_changed(host);ui_shell_refresh_titles(host);
 snprintf(json,sizeof(json),"{\"language\":\"%s\",\"source\":%d,\"generation\":\"%llu\"}",host->language.language,host->language.source,(unsigned long long)version);
 (void)ui_host_emit_event(host,"ui.host.language_changed",json);host->language_changing=0;ui_dispatch_leave(host);return UI_STATUS_OK;
}
ui_status_t ui_host_set_language(ui_host_t *host,const char *language)
{
 if(!ui_language_thread_ok(host)||!language||!*language||strlen(language)>=16)return UI_STATUS_INVALID_ARGUMENT;
 if(!isalpha((unsigned char)language[0])||language[strlen(language)-1]=='-')return UI_STATUS_INVALID_ARGUMENT;
 for(const unsigned char *p=(const unsigned char *)language;*p;++p)if(!(*p>='a'&&*p<='z'||*p>='A'&&*p<='Z'||*p>='0'&&*p<='9'||*p=='-'))return UI_STATUS_INVALID_ARGUMENT;
 if(strstr(language,"--"))return UI_STATUS_INVALID_ARGUMENT;
 if(strcmp(language,"zh-CN")&&strcmp(language,"en-US"))return UI_STATUS_UNSUPPORTED;
 ui_language_info_t info=host->language;info.source=UI_LANGUAGE_APPLICATION;info.fallback=0;strcpy(info.language,language);return apply_language(host,&info);
}
ui_status_t ui_host_reset_language(ui_host_t *host)
{ui_language_info_t info={0};if(!ui_language_thread_ok(host))return UI_STATUS_INVALID_ARGUMENT;info.size=sizeof(info);ui_status_t status=ui_host_get_system_language(&info);return status==UI_STATUS_OK?apply_language(host,&info):status;}
ui_status_t ui_host_set_title(ui_host_t *host,ui_text_target_t target,const char *id,const char *title)
{
 char **field=NULL,*copy;if(!ui_language_thread_ok(host)||!id||!*id||!title||strlen(title)>4095)return UI_STATUS_INVALID_ARGUMENT;
 if(host->dispatch_blocked)return UI_STATUS_CANCELLED;
 if(target==UI_TEXT_MENU_GROUP)return ui_menus_set_title(host,id,title);
 if(target==UI_TEXT_COMMAND){for(ui_command_entry_t *p=host->commands;p;p=p->next)if(!strcmp(p->id,id)){field=&p->title;break;}}
 else if(target==UI_TEXT_MENU_ITEM){for(ui_menu_entry_t *p=host->menus;p;p=p->next)if(!strcmp(p->id,id)){field=&p->title;break;}}
 else if(target==UI_TEXT_TOOLBAR){for(ui_toolbar_entry_t *p=host->toolbars;p;p=p->next)if(!strcmp(p->id,id)){field=&p->title;break;}}
 else if(target==UI_TEXT_TOOLBAR_ITEM){for(ui_toolbar_item_entry_t *p=host->toolbar_items;p;p=p->next)if(!strcmp(p->id,id)){field=&p->title;break;}}
 else if(target==UI_TEXT_PANEL){for(ui_panel_entry_t *p=host->panels;p;p=p->next)if(!strcmp(p->id,id)){field=&p->title;break;}}
 else return UI_STATUS_INVALID_ARGUMENT;
 if(!field)return UI_STATUS_NOT_FOUND;copy=ui_strdup(title);if(!copy)return UI_STATUS_OUT_OF_MEMORY;free(*field);*field=copy;
 if(!host->language_changing)ui_shell_refresh_titles(host);return ui_host_emit_event(host,"ui.commands.changed","{}");
}
