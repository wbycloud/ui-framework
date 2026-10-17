#include "ui_framework/theme.h"
#include "ui_framework/components.h"
#include "ui_framework/language.h"
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <objbase.h>
#endif
static int failures,notices,events,no_callback;
#define CHECK(x) do{if(!(x)){printf("theme line%d: %s\n",__LINE__,#x);++failures;}}while(0)
static void notice(ui_host_t *h,const ui_theme_info_t *info,void *data)
{
    ui_theme_info_t got={0};(void)data;got.size=sizeof(got);++notices;
    CHECK(ui_host_get_theme(h,&got)==UI_STATUS_OK&&got.generation==info->generation);
    CHECK(got.effective==info->effective&&got.icon_rgba==info->text_rgba);
    CHECK(ui_host_set_theme(h,UI_THEME_LIGHT)==UI_STATUS_CANCELLED);
    CHECK(ui_host_refresh_theme(h)==UI_STATUS_CANCELLED);
    CHECK(notices==events+1);
}
static void event(const char *name,const char *json,void *data)
{(void)data;if(!strcmp(name,"ui.host.theme_changed")){CHECK(strstr(json,"generation")!=NULL);++events;CHECK(events==notices+no_callback);}}
#ifdef _WIN32
static DWORD WINAPI worker(void *data)
{
    ui_theme_info_t got={0};ui_host_config_t cfg={0};ui_host_t *own;APTTYPE type;APTTYPEQUALIFIER qualifier;int ok;
    got.size=sizeof(got);ok=ui_host_get_theme(data,&got)==UI_STATUS_INVALID_ARGUMENT&&ui_host_set_theme(data,UI_THEME_DARK)==UI_STATUS_INVALID_ARGUMENT&&ui_host_refresh_theme(data)==UI_STATUS_INVALID_ARGUMENT&&ui_host_set_theme_callback(data,NULL,NULL)==UI_STATUS_INVALID_ARGUMENT;
    if(FAILED(CoInitializeEx(NULL,COINIT_MULTITHREADED)))return 1;
    cfg.size=sizeof(cfg);cfg.api_version=UI_FRAMEWORK_API_VERSION;own=ui_host_create(&cfg);
    ok=ok&&own&&ui_host_set_theme(own,UI_THEME_SYSTEM)==UI_STATUS_OK&&SUCCEEDED(CoGetApartmentType(&type,&qualifier))&&type==APTTYPE_MTA;
    ui_host_destroy(own);CoUninitialize();return ok?0:1;
}
#endif
int main(void)
{
    ui_host_config_t cfg={0};ui_host_t *a,*b;ui_theme_info_t initial={0},got={0},other={0};ui_language_info_t language={0},after_language={0};
    ui_component_desc_t desc={0};ui_component_t *form=NULL;ui_component_state_t before={0},after={0};ui_field_desc_t field={0};ui_cell_t value={0},read={0};
    cfg.size=sizeof(cfg);cfg.api_version=UI_FRAMEWORK_API_VERSION;cfg.event_callback=event;a=ui_host_create(&cfg);cfg.event_callback=NULL;b=ui_host_create(&cfg);CHECK(a&&b);if(!a||!b)return 1;
    initial.size=got.size=other.size=sizeof(got);language.size=after_language.size=sizeof(language);
    CHECK(ui_host_get_theme(a,&initial)==UI_STATUS_OK&&initial.mode==UI_THEME_LIGHT&&initial.effective==UI_THEME_LIGHT&&initial.generation==1);
    CHECK(ui_host_get_language(a,&language)==UI_STATUS_OK);CHECK(initial.background_rgba==0xf7f7f8ff&&initial.focus_rgba==0x0078d4ff);
    field.size=sizeof(field);field.id="draft";field.kind=UI_VALUE_TEXT;desc.size=sizeof(desc);desc.id="theme.form";desc.kind=UI_COMPONENT_FORM;desc.fields=&field;desc.field_count=1;
    CHECK(ui_component_register(a,&desc,&form)==UI_STATUS_OK);value.size=sizeof(value);value.kind=UI_VALUE_TEXT;value.text="Keep draft 中文";CHECK(ui_component_set_field(form,"draft",&value)==UI_STATUS_OK);
    before.size=after.size=sizeof(before);CHECK(ui_component_get_state(form,&before)==UI_STATUS_OK);
    CHECK(ui_host_set_theme_callback(a,notice,NULL)==UI_STATUS_OK);CHECK(ui_host_set_theme(a,UI_THEME_DARK)==UI_STATUS_OK);CHECK(notices==1&&events==1);
    CHECK(ui_host_get_theme(a,&got)==UI_STATUS_OK&&got.mode==UI_THEME_DARK&&got.effective==UI_THEME_DARK&&got.generation==2);
    CHECK(got.background_rgba==0x202020ff&&got.text_rgba==0xedededff&&got.focus_rgba==0x60b7ffff&&got.selection_rgba==0x263e50ff);
    CHECK(ui_host_set_theme(a,UI_THEME_DARK)==UI_STATUS_OK&&notices==1);CHECK(ui_host_refresh_theme(a)==UI_STATUS_OK&&notices==1);
    CHECK(ui_host_get_theme(b,&other)==UI_STATUS_OK&&other.mode==UI_THEME_LIGHT&&other.generation==1);
    CHECK(ui_host_set_theme(a,(ui_theme_mode_t)99)==UI_STATUS_INVALID_ARGUMENT);CHECK(ui_host_set_theme(a,(ui_theme_mode_t)-1)==UI_STATUS_INVALID_ARGUMENT);
    got.size=sizeof(got)-1;CHECK(ui_host_get_theme(a,&got)==UI_STATUS_INVALID_ARGUMENT);got.size=sizeof(got);CHECK(ui_host_get_theme(NULL,&got)==UI_STATUS_INVALID_ARGUMENT);CHECK(ui_host_get_theme(a,NULL)==UI_STATUS_INVALID_ARGUMENT);
#ifdef _WIN32
    {APTTYPE type;APTTYPEQUALIFIER qualifier;HRESULT before=CoGetApartmentType(&type,&qualifier);CHECK(before==CO_E_NOTINITIALIZED);
     CHECK(ui_host_set_theme(a,UI_THEME_SYSTEM)==UI_STATUS_OK&&notices==2);CHECK(CoGetApartmentType(&type,&qualifier)==before);}
#else
    CHECK(ui_host_set_theme(a,UI_THEME_SYSTEM)==UI_STATUS_OK&&notices==2);
#endif
    CHECK(ui_host_get_theme(a,&got)==UI_STATUS_OK&&got.mode==UI_THEME_SYSTEM&&got.generation==3);
    printf("SYSTEM available=%u effective=%d background=%08x generation=%llu\n",got.system_theme_available,(int)got.effective,got.background_rgba,(unsigned long long)got.generation);
    CHECK(ui_host_refresh_theme(a)==UI_STATUS_OK&&notices==2);
    CHECK(ui_host_get_language(a,&after_language)==UI_STATUS_OK&&after_language.generation==language.generation&&!strcmp(after_language.language,language.language));
    CHECK(ui_component_get_state(form,&after)==UI_STATUS_OK&&before.generation==after.generation&&before.dirty==after.dirty);
    read.size=sizeof(read);CHECK(ui_component_get_field(form,"draft",&read)==UI_STATUS_OK&&!strcmp(read.text,"Keep draft 中文"));
#ifdef _WIN32
    {HANDLE thread=CreateThread(NULL,0,worker,a,0,NULL);DWORD code=1;CHECK(thread&&WaitForSingleObject(thread,10000)==WAIT_OBJECT_0&&GetExitCodeThread(thread,&code)&&code==0);if(thread)CloseHandle(thread);}
    {intptr_t result=0;CHECK(ui_host_handle_message(a,NULL,WM_THEMECHANGED,0,0,&result)==UI_STATUS_OK&&notices==2);}
#endif
    CHECK(ui_host_set_theme_callback(a,NULL,NULL)==UI_STATUS_OK);no_callback=1;CHECK(ui_host_set_theme(a,UI_THEME_LIGHT)==UI_STATUS_OK);CHECK(notices==2&&events==3);CHECK(ui_component_unregister(form)==UI_STATUS_OK);
    ui_host_destroy(a);ui_host_destroy(b);printf("Theme core failures=%d\n",failures);return failures?1:0;
}
