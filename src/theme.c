#include "ui_internal.h"
#include <string.h>
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#include <roapi.h>
#include <winstring.h>
#include <windows.ui.viewmanagement.h>
#include <wchar.h>
#endif
static int system_dark(int *available)
{
    *available=0;
#ifdef _WIN32
    /* Documented UISettings contract, via its C ABI. Balance only our own
       successful initialization; never change a caller's existing apartment. */
    static const IID settings_iid={0x03021be4,0x5254,0x4781,{0x81,0x94,0x51,0x68,0xf7,0xd0,0x6d,0x7b}};
    IInspectable *instance=NULL;__x_ABI_CWindows_CUI_CViewManagement_CIUISettings3 *settings=NULL;
    struct __x_ABI_CWindows_CUI_CColor color={0};HSTRING name=NULL;int dark=0;
    HRESULT initialized=RoInitialize(RO_INIT_SINGLETHREADED),hr;
    if(FAILED(initialized)&&initialized!=RPC_E_CHANGED_MODE)return 0;
    hr=WindowsCreateString(RuntimeClass_Windows_UI_ViewManagement_UISettings,
        (UINT32)wcslen(RuntimeClass_Windows_UI_ViewManagement_UISettings),&name);
    if(SUCCEEDED(hr))hr=RoActivateInstance(name,&instance);
    if(SUCCEEDED(hr))hr=instance->lpVtbl->QueryInterface(instance,&settings_iid,(void **)&settings);
    if(SUCCEEDED(hr))hr=settings->lpVtbl->GetColorValue(settings,UIColorType_Foreground,&color);
    if(SUCCEEDED(hr)){*available=1;dark=(5u*color.G+2u*color.R+color.B)>8u*128u;}
    if(settings)settings->lpVtbl->Release(settings);if(instance)instance->lpVtbl->Release(instance);
    if(name)WindowsDeleteString(name);if(SUCCEEDED(initialized))RoUninitialize();return dark;
#else
    return 0;
#endif
}
ui_status_t ui_host_get_theme(const ui_host_t *host,ui_theme_info_t *out)
{
    static const uint32_t colors[2][16]={
        {0xf7f7f8ff,0xffffffff,0x242424ff,0x646468ff,0xc8c8cbff,0x0078d4ff,0xdcecffFF,0x242424ff,
         0xeeeeefff,0xdedee0ff,0xf1f1f2ff,0x919191ff,0x0067b8ff,0x242424ff,0x0078d4ff,0xc42b1cff},
        {0x202020ff,0x2b2b2bff,0xedededff,0xb3b3b7ff,0x555555ff,0x60b7ffff,0x263e50ff,0xedededff,
         0x383838ff,0x454545ff,0x292929ff,0x999999ff,0x0067b8ff,0xedededff,0x60b7ffff,0xff9a8fff}};
    ui_theme_info_t info={0};
    if(!ui_language_thread_ok(host)||!out||out->size<sizeof(*out))return UI_STATUS_INVALID_ARGUMENT;
    info.size=sizeof(info);info.mode=host->theme_mode;info.effective=host->menu_dark?UI_THEME_DARK:UI_THEME_LIGHT;
    info.generation=host->theme_generation?host->theme_generation:1;info.system_theme_available=host->theme_system_available;
    memcpy((char *)&info+offsetof(ui_theme_info_t,background_rgba),colors[host->menu_dark!=0],sizeof(colors[0]));
    *out=info;return UI_STATUS_OK;
}
ui_status_t ui_host_set_theme_callback(ui_host_t *host,ui_theme_callback_fn callback,void *data)
{
    if(!ui_language_thread_ok(host))return UI_STATUS_INVALID_ARGUMENT;
    if(host->dispatch_blocked&&callback)return UI_STATUS_CANCELLED;
    host->theme_callback=callback;host->theme_data=callback?data:NULL;return UI_STATUS_OK;
}
ui_status_t ui_host_set_theme(ui_host_t *host,ui_theme_mode_t mode)
{
    int available=0,dark;ui_theme_info_t info={0};char json[160];
    if(!ui_language_thread_ok(host)||mode<UI_THEME_LIGHT||mode>UI_THEME_SYSTEM)return UI_STATUS_INVALID_ARGUMENT;
    if(host->theme_changing||host->dispatch_blocked)return UI_STATUS_CANCELLED;
    dark=mode==UI_THEME_SYSTEM?system_dark(&available):mode==UI_THEME_DARK;
    if(host->theme_mode==mode&&host->menu_dark==dark&&host->theme_system_available==(uint32_t)available)return UI_STATUS_OK;
    ui_dispatch_enter(host);host->theme_changing=1;host->theme_mode=mode;host->theme_system_available=(uint32_t)available;
    host->theme_generation=(host->theme_generation?host->theme_generation:1)+1;
    (void)ui_host_hide_tooltip(host);(void)ui_host_close_menu(host);ui_components_set_theme(host,dark);
    info.size=sizeof(info);(void)ui_host_get_theme(host,&info);
    if(host->theme_callback)host->theme_callback(host,&info,host->theme_data);
    snprintf(json,sizeof(json),"{\"mode\":%d,\"effective\":%d,\"generation\":\"%llu\"}",(int)mode,(int)info.effective,(unsigned long long)info.generation);
    (void)ui_host_emit_event(host,"ui.host.theme_changed",json);host->theme_changing=0;ui_dispatch_leave(host);return UI_STATUS_OK;
}
ui_status_t ui_host_refresh_theme(ui_host_t *host)
{
    if(!ui_language_thread_ok(host))return UI_STATUS_INVALID_ARGUMENT;
    if(host->theme_changing||host->dispatch_blocked)return UI_STATUS_CANCELLED;
    return host->theme_mode==UI_THEME_SYSTEM?ui_host_set_theme(host,UI_THEME_SYSTEM):UI_STATUS_OK;
}
