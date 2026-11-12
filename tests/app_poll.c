#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "ui_framework/application.h"
#include "ui_framework/app_poll.h"
#include "ui_framework/app_data.h"
#include "ui_framework/capability_entry.h"
static unsigned checks,failures;
#define CHECK(x) do{++checks;if(!(x)){++failures;fprintf(stderr,"line %d: %s\n",__LINE__,#x);}}while(0)
static unsigned snapshot(ui_assistant_t *assistant,const char **json)
{unsigned n=0;CHECK(ui_assistant_get_state_snapshot(assistant,json)==UI_STATUS_OK);if(*json)sscanf_s(*json,"{\"polls\":%u",&n);return n;}
int wmain(int argc,wchar_t **argv)
{
    ui_workspace_config_t config={0};ui_workspace_t *w;uint64_t id,second;ui_app_instance_info_t info={0};ui_app_context_t context={0};char path[32768];const char *json=NULL;unsigned count;DWORD start;
    if(argc!=2||!WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,argv[1],-1,path,sizeof(path),NULL,NULL))return 2;
    config.size=sizeof(config);config.run_mode=UI_RUN_OFFSCREEN;w=ui_workspace_create(&config);CHECK(w!=NULL);if(!w)return 1;
    CHECK(ui_workspace_open(w,path,&id)==UI_STATUS_OK);info.size=sizeof(info);CHECK(ui_workspace_get_instance(w,id,&info)==UI_STATUS_OK);
    if(!info.host){ui_workspace_destroy(w);return 1;}
    context.size=sizeof(context);context.workspace=w;context.host=info.host;context.instance_id=id;
    CHECK(ui_app_set_poll_callback(NULL,NULL,NULL)==UI_STATUS_INVALID_ARGUMENT);context.instance_id=UINT64_MAX;CHECK(ui_app_set_poll_callback(&context,NULL,NULL)==UI_STATUS_NOT_FOUND);context.instance_id=id;
    ui_workspace_poll(w);CHECK(snapshot(info.assistant,&json)==1);CHECK(json&&strstr(json,"\"in_poll\":0")&&strstr(json,"\"enable\":1"));
    CHECK(json&&strstr(json,"\"collect\":0"));
    CHECK(json&&strstr(json,"\"retire\":0"));
    CHECK(ui_capability_enable_read_entries(info.host)==UI_STATUS_OK);
    CHECK(ui_workspace_post_app_data(w,id,3,NULL,0)==UI_STATUS_OK);ui_workspace_poll(w);count=snapshot(info.assistant,&json);ui_workspace_poll(w);CHECK(snapshot(info.assistant,&json)==count);
    CHECK(ui_workspace_post_app_data(w,id,4,NULL,0)==UI_STATUS_OK);ui_workspace_poll(w);ui_workspace_poll(w);CHECK(snapshot(info.assistant,&json)==count+1);
    CHECK(ui_workspace_post_app_data(w,id,1,NULL,0)==UI_STATUS_OK);ui_workspace_poll(w);CHECK(snapshot(info.assistant,&json)>count&&json&&strstr(json,"\"worker\":true"));
    CHECK(ui_workspace_close(w,id,UI_APP_CLOSE_TAB)==UI_STATUS_OK);ui_workspace_poll(w);CHECK(ui_workspace_count(w)==1&&GetModuleHandleW(L"ui_app_poll_fixture.dll")!=NULL);
    CHECK(ui_workspace_post_app_data(w,id,2,NULL,0)==UI_STATUS_OK);start=GetTickCount();while(ui_workspace_count(w)&&GetTickCount()-start<3000){ui_workspace_poll(w);Sleep(1);}
    CHECK(ui_workspace_count(w)==0);CHECK(GetModuleHandleW(L"ui_app_poll_fixture.dll")==NULL);CHECK(ui_app_set_poll_callback(&context,NULL,NULL)==UI_STATUS_NOT_FOUND);
    CHECK(ui_workspace_open(w,path,&second)==UI_STATUS_OK&&second!=id);CHECK(ui_workspace_post_app_data(w,id,4,NULL,0)==UI_STATUS_CANCELLED);
    CHECK(ui_workspace_close(w,second,UI_APP_CLOSE_TAB)==UI_STATUS_OK);ui_workspace_poll(w);CHECK(ui_workspace_count(w)==0);CHECK(ui_workspace_destroy(w)==UI_STATUS_OK);
    printf("AP-1 real DLL: checks=%u failures=%u\n",checks,failures);return failures?1:0;
}
