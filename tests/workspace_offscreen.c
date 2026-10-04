#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui_framework/application.h"
#include "ui_framework/components.h"
#include "ui_framework/menus.h"
static int failures,windows,events;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);++failures;}}while(0)
static BOOL CALLBACK count_window(HWND w,LPARAM p){(void)w;(void)p;++windows;return TRUE;}
static void event(uint64_t id,const char *name,const char *json,void *data){(void)id;(void)json;(void)data;if(!strcmp(name,"test.event"))++events;}
static int click(ui_component_t *c,const char *id)
{ui_element_presentation_t p={0};ui_input_event_t e={0};p.size=sizeof(p);if(ui_component_get_presentation(c,id,&p)!=UI_STATUS_OK||!p.visible||!p.enabled)return 0;
 e.size=sizeof(e);e.kind=UI_INPUT_POINTER_DOWN;e.pointer_button=1;e.x=p.clip.x+3;e.y=p.clip.y+3;if(ui_component_dispatch_input(c,&e)!=UI_STATUS_OK)return 0;e.kind=UI_INPUT_POINTER_UP;return ui_component_dispatch_input(c,&e)==UI_STATUS_OK;}
typedef struct worker_input {ui_workspace_t *workspace;uint64_t id;} worker_input_t;
static DWORD WINAPI post_worker(void *data)
{worker_input_t *info=(worker_input_t *)data;
 CHECK(ui_workspace_post_event(info->workspace,info->id,"test.event","{\"copied\":true}")==UI_STATUS_OK);return 0;}
int wmain(int argc,wchar_t **argv)
{
    char path[32768];ui_workspace_config_t config={0};ui_workspace_t *w;ui_app_instance_info_t a={0},b={0};
    uint64_t first=0,second=0,reopen=0,request=0;ui_run_mode_t mode;ui_rect_t rect={0,0,1280,720};ui_component_t *form,*dialog,*table,*tree;
    ui_element_presentation_t p={0};ui_input_event_t e={0};ui_component_state_t state={0};HANDLE worker;
    if(argc!=2||!WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,argv[1],-1,path,sizeof(path),NULL,NULL))return 2;
    config.size=sizeof(config);config.shell_mode=UI_WORKSPACE_SHELL_WEB;config.max_permission=UI_ASSISTANT_PERMISSION_EDIT;config.event=event;
    CHECK(ui_workspace_create(&config)==NULL);config.run_mode=(ui_run_mode_t)99;CHECK(ui_workspace_create(&config)==NULL);config.run_mode=UI_RUN_OFFSCREEN;
    SetEnvironmentVariableA("UI_FEATURES_NO_GL","1");w=ui_workspace_create(&config);CHECK(w!=NULL);if(!w)return 1;
    CHECK(ui_workspace_get_run_mode(w,&mode)==UI_STATUS_OK&&mode==UI_RUN_OFFSCREEN);CHECK(ui_workspace_set_rect(w,&rect,144)==UI_STATUS_OK);
    SetEnvironmentVariableA("UI_FEATURES_FAIL_CREATE","1");CHECK(ui_workspace_open(w,path,&first)==UI_STATUS_PLATFORM_ERROR&&first==0&&ui_workspace_count(w)==0);SetEnvironmentVariableA("UI_FEATURES_FAIL_CREATE",NULL);
    SetEnvironmentVariableA("UI_FEATURES_FAIL_MOUNT","1");CHECK(ui_workspace_open(w,path,&first)==UI_STATUS_PLATFORM_ERROR&&first==0&&ui_workspace_count(w)==0);SetEnvironmentVariableA("UI_FEATURES_FAIL_MOUNT",NULL);
    ui_workspace_poll(w);CHECK(GetModuleHandleW(L"framework_features_app.dll")==NULL);
    CHECK(ui_workspace_open(w,path,&first)==UI_STATUS_OK);CHECK(ui_workspace_open(w,path,&second)==UI_STATUS_OK&&first!=second);
    if(!first||!second){printf("open: %s\n",ui_workspace_last_error(w));return 1;}
    a.size=b.size=sizeof(a);CHECK(ui_workspace_get_instance(w,first,&a)==UI_STATUS_OK);CHECK(ui_workspace_get_instance(w,second,&b)==UI_STATUS_OK);
    CHECK(GetModuleHandleW(L"framework_features_app.dll")!=NULL);
    CHECK(!a.native_container&&!b.native_container);CHECK(ui_host_get_run_mode(a.host,&mode)==UI_STATUS_OK&&mode==UI_RUN_OFFSCREEN);
    CHECK(ui_workspace_activate(w,first)==UI_STATUS_OK);form=ui_component_find(a.host,"form");table=ui_component_find(a.host,"table");dialog=ui_component_find(a.host,"dialog");CHECK(form&&table&&dialog);
    CHECK(click(form,"field-name"));e.size=sizeof(e);e.kind=UI_INPUT_TEXT;e.text_utf8="headless 草稿";CHECK(ui_component_dispatch_input(form,&e)==UI_STATUS_OK);
    CHECK(ui_workspace_activate(w,second)==UI_STATUS_OK);CHECK(ui_workspace_activate(w,first)==UI_STATUS_OK);
    p.size=sizeof(p);CHECK(ui_component_get_presentation(form,"field-name",&p)==UI_STATUS_OK&&p.focused&&strstr(p.text_utf8,"草稿"));
    tree=ui_component_find(a.host,"tree");CHECK(tree!=NULL);
    CHECK(ui_component_get_presentation(tree,"tree-label-1",&p)==UI_STATUS_OK&&p.visible);
    e.kind=UI_INPUT_POINTER_UP;e.pointer_button=2;e.x=p.clip.x+3;e.y=p.clip.y+3;
    CHECK(ui_component_dispatch_input(tree,&e)==UI_STATUS_OK);
    CHECK(ui_host_menu_get_item_presentation(a.host,"node.dialog",&p)==UI_STATUS_OK&&p.visible);
    CHECK(ui_host_close_menu(a.host)==UI_STATUS_OK);e.kind=UI_INPUT_TEXT;
    CHECK(ui_workspace_invoke(w,first,"demo.dialog","{}",&request)==UI_STATUS_OK);CHECK(ui_component_get_presentation(dialog,"field-name",&p)==UI_STATUS_OK&&p.focused);
    CHECK(ui_component_dispatch_input(form,&e)==UI_STATUS_CANCELLED);CHECK(click(dialog,"field-name"));CHECK(ui_component_dispatch_input(dialog,&e)==UI_STATUS_OK);
    CHECK(click(dialog,"submit"));CHECK(ui_component_get_state(dialog,&state)==UI_STATUS_INVALID_ARGUMENT);state.size=sizeof(state);
    CHECK(ui_component_get_state(dialog,&state)==UI_STATUS_OK&&!state.modal);
    CHECK(ui_component_query(table,0,99900,12,12,4)==UI_STATUS_OK);CHECK(ui_component_get_state(table,&state)==UI_STATUS_OK&&state.total_count==100000&&state.rendered_nodes<1024);
    printf("Offscreen actual DOM=%zu cached rows=%zu columns=%zu\n",state.rendered_nodes,state.row_count,state.column_count);
    /* A copied background event uses the normal queue and UI-thread poll. */
    {worker_input_t post={w,first};worker=CreateThread(NULL,0,post_worker,&post,0,NULL);CHECK(worker!=NULL);WaitForSingleObject(worker,INFINITE);CloseHandle(worker);{ui_status_t status=ui_workspace_flush(w,1);CHECK(status==UI_STATUS_OK||status==UI_STATUS_CANCELLED);
     for(int n=0;!events&&n<1024;++n){status=ui_workspace_flush(w,1);CHECK(status==UI_STATUS_OK||status==UI_STATUS_CANCELLED);}}
     CHECK(events==1);}
    CHECK(ui_workspace_invoke(w,first,"demo.close_refuse","{}",&request)==UI_STATUS_OK);CHECK(ui_workspace_close(w,first,UI_APP_CLOSE_TAB)==UI_STATUS_CANCELLED);CHECK(ui_workspace_count(w)==2);
    CHECK(ui_workspace_invoke(w,first,"demo.close_allow","{}",&request)==UI_STATUS_OK);CHECK(ui_workspace_close_all(w)==UI_STATUS_OK);
    {ULONGLONG end=GetTickCount64()+5000;while(ui_workspace_count(w)&&GetTickCount64()<end){ui_workspace_poll(w);SwitchToThread();}}
    CHECK(ui_workspace_count(w)==0&&GetModuleHandleW(L"framework_features_app.dll")==NULL);CHECK(ui_workspace_post_event(w,first,"test.event","{}") ==UI_STATUS_OK);ui_workspace_poll(w);CHECK(events==1);
    CHECK(ui_workspace_open(w,path,&reopen)==UI_STATUS_OK&&reopen>second);CHECK(ui_workspace_close_all(w)==UI_STATUS_OK);
    {ULONGLONG end=GetTickCount64()+5000;while(ui_workspace_count(w)&&GetTickCount64()<end){ui_workspace_poll(w);SwitchToThread();}}
    CHECK(ui_workspace_destroy(w)==UI_STATUS_OK);SetEnvironmentVariableA("UI_FEATURES_NO_GL",NULL);
    EnumThreadWindows(GetCurrentThreadId(),count_window,0);CHECK(windows==0);printf("Workspace/shared DLL/real templates: %d failures, HWNDs=%d\n",failures,windows);return failures?1:0;
}
