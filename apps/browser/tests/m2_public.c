/* Actual app via public workspace/assistant C ABI; no view or JS internals. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ui_framework/application.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "browser_json.h"
static const char *origin="http://127.0.0.1:8765";
static ui_workspace_t *workspace;static int failures;static char latest[49152];
#define T(x) do{if(!(x)){fprintf(stderr,"M2 app line%d: %s\n",__LINE__,#x);++failures;}}while(0)
static void pump(unsigned ms)
{ULONGLONG end=GetTickCount64()+ms;do{MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}ui_workspace_flush(workspace,64);Sleep(1);}while(GetTickCount64()<end);}
static const char *state(uint64_t id)
{ui_app_instance_info_t info={0};const char *text="{}";info.size=sizeof(info);T(ui_workspace_get_instance(workspace,id,&info)==UI_STATUS_OK);T(ui_assistant_get_state_snapshot(info.assistant,&text)==UI_STATUS_OK);strcpy_s(latest,sizeof(latest),text);return latest;}
static void invoke(uint64_t id,const char *cmd,const char *json)
{uint64_t request;T(ui_workspace_invoke(workspace,id,cmd,json?json:"{}",&request)==UI_STATUS_OK);pump(20);}
static void navigate(uint64_t id,const char *path)
{char url[4096],quoted[24578],json[28672];ULONGLONG end;snprintf(url,sizeof(url),"%s%s",origin,path);T(browser_json_quote(url,quoted,sizeof(quoted)));snprintf(json,sizeof(json),"{\"url\":%s}",quoted);invoke(id,"browser.navigate",json);end=GetTickCount64()+8000;
 do{pump(10);state(id);if(strstr(latest,"\"loading\":0")&&strstr(latest,"\"idle\":1")&&(strstr(latest,"\"pending\":0")||strstr(latest,"\"error\":1")))break;}while(GetTickCount64()<end);T(GetTickCount64()<end);pump(80);state(id);}
static long number(const char *name)
{char key[96],*end;const char *at;long value;snprintf(key,sizeof(key),"\"%s\":",name);at=strstr(latest,key);T(at!=NULL);if(!at)return -1;at+=strlen(key);value=strtol(at,&end,10);T(end!=at&&(*end==','||*end=='}'));return value;}
int wmain(int argc,wchar_t **argv)
{
    wchar_t old[32768],base[MAX_PATH],folder[MAX_PATH],file[32768];char package[32768];HWND root;ui_workspace_config_t config={0};ui_rect_t r={0,0,860,640};uint64_t first=0,second=0;
    if(argc<2)return 2;setvbuf(stdout,NULL,_IONBF,0);GetEnvironmentVariableW(L"LOCALAPPDATA",old,32768);T(GetTempPathW(MAX_PATH,folder));T(GetTempFileNameW(folder,L"ub2",0,base));T(DeleteFileW(base));T(CreateDirectoryW(base,NULL));T(SetEnvironmentVariableW(L"LOCALAPPDATA",base));
    T(ui_framework_initialize()==UI_STATUS_OK);root=CreateWindowW(L"STATIC",L"M2 public app verification",WS_OVERLAPPEDWINDOW,0,0,900,700,NULL,NULL,NULL,NULL);T(root!=NULL);config.size=sizeof(config);config.native_parent=root;config.shell_mode=UI_WORKSPACE_SHELL_WEB;workspace=ui_workspace_create(&config);T(workspace!=NULL);T(ui_workspace_set_rect(workspace,&r,96)==UI_STATUS_OK);
    T(WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,argv[1],-1,package,sizeof(package),NULL,NULL)>0);T(ui_workspace_open(workspace,package,&first)==UI_STATUS_OK);T(ui_workspace_open(workspace,package,&second)==UI_STATUS_OK);if(!first||!second)return 1;
    if(argc>2&&!wcscmp(argv[2],L"--trusted-local"))origin="https://localhost:8766";
    if(argc>2&&!wcscmp(argv[2],L"--tls-only")){origin="https://localhost:8766";navigate(first,"/m2-reading");T(strstr(latest,"\"failure\":\"tls\"")&&strstr(latest,"\"windows\":12175"));printf("TLS_AFTER_REMOVAL %s\n",latest);goto close;}
    navigate(first,"/m2-reading");T(strstr(latest,"M2 阅读兼容固定集")!=NULL);T(number("error")==0&&number("images")==2&&number("script_runtime")==0);T(strstr(latest,"图片8×12")&&strstr(latest,"EXIF方向6已应用")&&strstr(latest,"动画仅呈现第一帧"));T(strstr(state(second),"browser:start")!=NULL);
    invoke(first,"browser.find-next","{\"text\":\"12. 编号十二\"}");T(strstr(state(first),"已定位")!=NULL);
    invoke(first,"browser.find-next","{\"text\":\"HIDDEN_SENTINEL_M2\"}");T(strstr(state(first),"未找到")!=NULL);
    navigate(first,"/m2-reading?anchor=1#定位");T(number("scroll_y")>0);T(number("content_height")-number("scroll_y")<600);printf("M2_READING %s\n",latest);
    navigate(first,"/m2-bom-conflict");T(strstr(latest,"BOM中文优先")&&number("error")==0);
    navigate(first,"/m2-http-conflict");T(strstr(latest,"HTTP中文优先")&&number("error")==0);
    navigate(first,"/m2-meta-utf16");T(strstr(latest,"Meta中文UTF8")&&number("error")==0);
    navigate(first,"/m2-unknown");T(strstr(latest,"\"failure\":\"encoding\"")&&strstr(latest,"未知或不支持的声明"));
    navigate(first,"/m2-odd-utf16");T(strstr(latest,"\"failure\":\"encoding\"")&&strstr(latest,"损坏字节"));
    navigate(first,"/m2-converted-budget");T(strstr(latest,"\"failure\":\"budget\"")&&strstr(latest,"转换后UTF-8超过门槛"));
    navigate(first,"/m2-table-limit");T(strstr(latest,"\"failure\":\"document-limit\"")!=NULL);
    navigate(first,"/m2-css-limit");T(strstr(latest,"\"failure\":\"document-limit\"")!=NULL);
    navigate(first,"/m2-form?q=%E4%B8%AD%E6%96%87+%26+%2B+English&same=first&same=second&empty=&submitter=%E6%8F%90%E4%BA%A4#结果");T(strstr(latest,"M2 GET Passed")&&number("error")==0);T(!strstr(latest,"?q="));
close:
    T(ui_workspace_close_all(workspace)==UI_STATUS_OK);{ULONGLONG end=GetTickCount64()+10000;while(ui_workspace_count(workspace)&&GetTickCount64()<end)pump(10);}T(!ui_workspace_count(workspace));T(GetModuleHandleW(L"browser_app.dll")==NULL);T(ui_workspace_destroy(workspace)==UI_STATUS_OK);DestroyWindow(root);
    swprintf(file,32768,L"%s\\ui-framework\\browser\\origins-v1.txt",base);if(GetFileAttributesW(file)!=INVALID_FILE_ATTRIBUTES)T(DeleteFileW(file));swprintf(file,32768,L"%s\\ui-framework\\browser",base);if(GetFileAttributesW(file)!=INVALID_FILE_ATTRIBUTES)T(RemoveDirectoryW(file));swprintf(file,32768,L"%s\\ui-framework",base);if(GetFileAttributesW(file)!=INVALID_FILE_ATTRIBUTES)T(RemoveDirectoryW(file));T(RemoveDirectoryW(base));T(SetEnvironmentVariableW(L"LOCALAPPDATA",*old?old:NULL));
    printf("M2 app compatibility failures=%d actual_DLL_unloaded=%d\n",failures,GetModuleHandleW(L"browser_app.dll")==NULL);return failures?1:0;
}
