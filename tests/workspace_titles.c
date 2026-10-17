/* Stable-instance title contract, public C only. Uses the maintained Web Counter. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ui_framework/application.h"
static int failures,notices,reentrant;
static ui_workspace_t *workspace;
static uint64_t changing_id;
#define CHECK(x) do{if(!(x)){printf("title line%d: %s\n",__LINE__,#x);++failures;}}while(0)
static void changed(void *data)
{(void)data;++notices;if(reentrant)CHECK(ui_workspace_set_instance_title(workspace,changing_id,"recursive")==UI_STATUS_CANCELLED);}
static void pump(void)
{MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}ui_workspace_poll(workspace);Sleep(1);}
static DWORD WINAPI worker(void *data)
{char out[32];size_t need=0;(void)data;return ui_workspace_set_instance_title(workspace,changing_id,"wrong-thread")==UI_STATUS_INVALID_ARGUMENT&&ui_workspace_get_instance_title(workspace,changing_id,out,sizeof(out),&need)==UI_STATUS_INVALID_ARGUMENT?0:1;}
int wmain(int argc,wchar_t **argv)
{
    HWND root;ui_workspace_config_t config={0};uint64_t first=0,second=0;ui_app_instance_info_t before={0},after={0};
    char title[4097],out[4097],original[512],*path;size_t required=0;int count,bytes;ui_rect_t rect={0,0,600,400};
    setvbuf(stdout,NULL,_IONBF,0);if(argc!=2)return 2;bytes=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,argv[1],-1,NULL,0,NULL,NULL);if(!bytes)return 2;path=malloc((size_t)bytes);if(!path)return 2;if(!WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,argv[1],-1,path,bytes,NULL,NULL)){free(path);return 2;}
    CHECK(ui_framework_initialize()==UI_STATUS_OK);root=CreateWindowW(L"STATIC",L"Public title contract",WS_OVERLAPPEDWINDOW,20,20,640,480,NULL,NULL,NULL,NULL);CHECK(root);
    config.size=sizeof(config);config.native_parent=root;config.shell_mode=UI_WORKSPACE_SHELL_WEB;config.changed=changed;workspace=ui_workspace_create(&config);CHECK(workspace);if(!workspace)return 1;
    CHECK(ui_workspace_set_rect(workspace,&rect,96)==UI_STATUS_OK);CHECK(ui_workspace_open(workspace,path,&first)==UI_STATUS_OK);CHECK(ui_workspace_open(workspace,path,&second)==UI_STATUS_OK);free(path);CHECK(first&&second&&first!=second);if(!first||!second){(void)ui_workspace_close_all(workspace);for(int i=0;i<1000&&ui_workspace_count(workspace);++i)pump();(void)ui_workspace_destroy(workspace);DestroyWindow(root);return 1;}CHECK(ui_workspace_activate(workspace,first)==UI_STATUS_OK);
    before.size=after.size=sizeof(before);CHECK(ui_workspace_get_instance(workspace,second,&before)==UI_STATUS_OK);strcpy_s(original,sizeof(original),before.name_utf8);
    CHECK(ui_workspace_get_instance_title(workspace,second,NULL,0,&required)==UI_STATUS_OK&&required==strlen(original)+1);
    strcpy_s(title,sizeof(title),"Shared title 中文 *");changing_id=second;reentrant=1;count=notices;
    CHECK(ui_workspace_set_instance_title(workspace,second,title)==UI_STATUS_OK&&notices==count+1);reentrant=0;title[0]='X';
    CHECK(ui_workspace_get_instance_title(workspace,second,out,sizeof(out),&required)==UI_STATUS_OK&&!strcmp(out,"Shared title 中文 *"));
    CHECK(ui_workspace_active(workspace)==first);CHECK(ui_workspace_get_instance(workspace,second,&after)==UI_STATUS_OK);
    CHECK(before.host==after.host&&before.shell==after.shell&&before.native_container==after.native_container&&!strcmp(after.name_utf8,original));
    count=notices;CHECK(ui_workspace_set_instance_title(workspace,second,"Shared title 中文 *")==UI_STATUS_OK&&notices==count);
    CHECK(ui_workspace_set_instance_title(workspace,first,"Shared title 中文 *")==UI_STATUS_OK);CHECK(ui_workspace_active(workspace)==first);
    for(int i=0;i<20;++i){snprintf(title,sizeof(title),"改名 %d",i);CHECK(ui_workspace_set_instance_title(workspace,second,title)==UI_STATUS_OK);CHECK(ui_workspace_get_instance_title(workspace,second,out,sizeof(out),&required)==UI_STATUS_OK&&!strcmp(title,out));}
    memset(title,'T',4095);memcpy(title,"长",3);title[4095]=0;CHECK(ui_workspace_set_instance_title(workspace,second,title)==UI_STATUS_OK);
    CHECK(ui_workspace_get_instance_title(workspace,second,NULL,0,&required)==UI_STATUS_OK&&required==4096);strcpy_s(out,sizeof(out),"unchanged");CHECK(ui_workspace_get_instance_title(workspace,second,out,8,&required)==UI_STATUS_LIMIT_EXCEEDED&&!strcmp(out,"unchanged"));
    CHECK(ui_workspace_get_instance_title(workspace,second,out,sizeof(out),&required)==UI_STATUS_OK&&!strcmp(out,title));title[4095]='T';title[4096]=0;CHECK(ui_workspace_set_instance_title(workspace,second,title)==UI_STATUS_INVALID_ARGUMENT);
    CHECK(ui_workspace_set_instance_title(workspace,second,"")==UI_STATUS_INVALID_ARGUMENT);CHECK(ui_workspace_set_instance_title(workspace,second,"\xc0\xaf")==UI_STATUS_INVALID_ARGUMENT);CHECK(ui_workspace_set_instance_title(workspace,UINT64_MAX,"gone")==UI_STATUS_NOT_FOUND);
    {HANDLE thread=CreateThread(NULL,0,worker,NULL,0,NULL);DWORD code=1;CHECK(thread&&WaitForSingleObject(thread,10000)==WAIT_OBJECT_0&&GetExitCodeThread(thread,&code)&&code==0);if(thread)CloseHandle(thread);}
    CHECK(ui_workspace_set_instance_title(workspace,second,NULL)==UI_STATUS_OK);CHECK(ui_workspace_get_instance_title(workspace,second,out,sizeof(out),&required)==UI_STATUS_OK&&!strcmp(out,original));
    CHECK(ui_workspace_close(workspace,second,UI_APP_CLOSE_TAB)==UI_STATUS_OK);CHECK(ui_workspace_set_instance_title(workspace,second,"closing")==UI_STATUS_CANCELLED);
    {ULONGLONG end=GetTickCount64()+10000;while(ui_workspace_count(workspace)>1&&GetTickCount64()<end)pump();}CHECK(ui_workspace_count(workspace)==1);CHECK(ui_workspace_set_instance_title(workspace,second,"gone")==UI_STATUS_NOT_FOUND);CHECK(ui_workspace_get_instance_title(workspace,second,out,sizeof(out),&required)==UI_STATUS_NOT_FOUND);
    CHECK(ui_workspace_close_all(workspace)==UI_STATUS_OK);{ULONGLONG end=GetTickCount64()+10000;while(ui_workspace_count(workspace)&&GetTickCount64()<end)pump();}CHECK(!ui_workspace_count(workspace));CHECK(ui_workspace_destroy(workspace)==UI_STATUS_OK);workspace=NULL;DestroyWindow(root);
    printf("Workspace title failures=%d changed-notices=%d\n",failures,notices);return failures?1:0;
}
