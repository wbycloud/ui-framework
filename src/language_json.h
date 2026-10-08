#ifndef UI_LANGUAGE_JSON_H
#define UI_LANGUAGE_JSON_H
#include "json_ui.h"
static void ui_language_json(ui_host_t *host,ui_json_t *json)
{
 uj_fmt(json,",\"languageGeneration\":\"%llu\",\"language\":",(unsigned long long)host->language.generation);uj_string(json,host->language.language);uj_add(json,",\"texts\":{");
 for(size_t i=0;i<ui_language_catalog_count();++i){const char *key,*value;ui_language_catalog_entry(host,i,&key,&value);if(i)uj_add(json,",");uj_string(json,key);uj_add(json,":");uj_string(json,value);}uj_add(json,"}");
}
#endif
