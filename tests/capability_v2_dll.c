#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ui_internal.h"
#include "ui_framework/capability_entry.h"
#include "capability_v2_dll_control.h"
#include <stdio.h>
#include <string.h>
static unsigned checks,failures;
#define CHECK(x) do{++checks;if(!(x)){++failures;fprintf(stderr,"line %d: %s\n",__LINE__,#x);}}while(0)
static uint64_t call;
static void observe(const ui_cap_ui_notice_t *n,void *data)
{(void)data;if(n->stage==UI_CAP_UI_ADMITTED)call=n->call_id;}
static void run(const char *path,int sync)
{
    HMODULE module=LoadLibraryA(path);FARPROC symbol;c2_dll_create_fn create=NULL;c2_dll_pump_fn pump=NULL;
    c2_dll_signal_fn signal=NULL;c2_dll_state_fn state=NULL;c2_dll_destroy_fn destroy=NULL;void *fixture;ui_host_t *host;
    ui_host_config_t h={0};ui_cap_ui_binding_desc_t b={0};ui_menu_item_desc_t menu={0};uint64_t binding;ui_cap_result_v2_t result={0};ULONGLONG end;
    c2_dll_api_t api={ui_capability_register_resource,ui_capability_register_v2,ui_capability_complete_v2,ui_capability_forget_v2,
      ui_capability_begin_unregistration,ui_capability_finish_unregistration,ui_capability_quiesce,ui_host_destroy};
    CHECK(module!=NULL);if(!module)return;
#define LOAD(name,fn) symbol=GetProcAddress(module,name);memcpy(&fn,&symbol,sizeof(fn));CHECK(fn!=NULL)
    LOAD("c2_dll_create",create);LOAD("c2_dll_pump",pump);LOAD("c2_dll_signal",signal);LOAD("c2_dll_state",state);LOAD("c2_dll_destroy",destroy);
#undef LOAD
    if(!create||!pump||!signal||!state||!destroy){FreeLibrary(module);return;}
    h.size=sizeof(h);h.api_version=10;host=ui_host_create(&h);CHECK(host!=NULL);CHECK(ui_capability_enable_read_entries(host)==UI_STATUS_OK);
    fixture=create(host,&api,sync);CHECK(fixture!=NULL);if(!fixture)return;
    menu.size=sizeof(menu);menu.id="dll-action";menu.title="DLL";menu.menu_path="Test";menu.command_id="dll-write";CHECK(ui_host_register_menu_item(host,&menu)==UI_STATUS_OK);
    b.size=sizeof(b);b.version=1;b.capability_id="dll-write";b.resource.size=sizeof(b.resource);b.resource.version=2;
    CHECK(ui_capability_resource_ref_v2(host,"dll-business",&b.resource)==UI_STATUS_OK);b.menu_item_id="dll-action";b.purpose=UI_CAP_UI_INVOKE;b.observe=observe;
    CHECK(ui_capability_ui_bind(host,&b,&binding)==UI_STATUS_OK);call=0;ui_capabilities_human_begin(host,fixture);
    CHECK(ui_capabilities_ui_action(host,"dll-action",fixture)==UI_STATUS_OK);ui_capabilities_human_end(host);CHECK(call!=0);
    result.size=sizeof(result);result.version=2;
    if(sync){CHECK(state(fixture,1)==6);CHECK(ui_capability_get_result_v2(host,call,&result)==UI_STATUS_OK&&result.state==UI_CAP_FINISHED);}
    else{
      CHECK(ui_capability_quiesce(host)==UI_STATUS_PENDING);CHECK(ui_capability_begin_unregistration(host,"dll-write")==UI_STATUS_OK);
      CHECK(ui_capability_finish_unregistration(host,"dll-write")==UI_STATUS_PENDING);ui_host_destroy(host);
      CHECK(ui_capability_get_result_v2(host,call,&result)==UI_STATUS_OK&&result.state==UI_CAP_RUNNING);CHECK(state(fixture,0)==0);
      signal(fixture,0);end=GetTickCount64()+5000;
      while(!state(fixture,2)&&GetTickCount64()<end){MSG msg;while(PeekMessageW(&msg,NULL,0,0,PM_REMOVE)){TranslateMessage(&msg);DispatchMessageW(&msg);}MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}
      CHECK(state(fixture,2)&&!state(fixture,3));CHECK(pump(fixture)==UI_STATUS_PENDING);
      CHECK(ui_capability_finish_unregistration(host,"dll-write")==UI_STATUS_PENDING);
      signal(fixture,1);end=GetTickCount64()+5000;
      while(pump(fixture)==UI_STATUS_PENDING&&GetTickCount64()<end){MSG msg;while(PeekMessageW(&msg,NULL,0,0,PM_REMOVE)){TranslateMessage(&msg);DispatchMessageW(&msg);}MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}
      CHECK(state(fixture,2)&&state(fixture,3));
    }
    CHECK(ui_capability_get_result_v2(host,call,&result)==UI_STATUS_OK&&result.outcome==UI_CAP2_OK&&result.effect==UI_CAP2_EFFECT_APPLIED);
    CHECK(state(fixture,0)==7);CHECK(ui_capability_complete_v2(host,call,NULL)==UI_STATUS_ALREADY_EXISTS);
    CHECK(ui_capability_forget_v2(host,call)==UI_STATUS_OK);CHECK(ui_capability_finish_unregistration(host,"dll-write")==UI_STATUS_OK);
    CHECK(ui_capability_quiesce(host)==UI_STATUS_OK);ui_host_destroy(host);destroy(fixture);CHECK(FreeLibrary(module));
}
int main(int argc,char **argv)
{if(argc!=2)return 2;run(argv[1],0);run(argv[1],1);printf("C2 DLL checks=%u failures=%u\n",checks,failures);return failures?1:0;}
