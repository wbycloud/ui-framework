/* Public C integration caller: no framework internals, view internals or JS. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ui_framework/application.h"
#include "browser_json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static ui_workspace_t *workspace;
static int failures;
static char latest[49152];
#define T(x) do{if(!(x)){fprintf(stderr,"m01 line%d: %s\n",__LINE__,#x);++failures;}}while(0)
static void pump(unsigned ms)
{ULONGLONG end=GetTickCount64()+ms;do{MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}ui_workspace_flush(workspace,64);Sleep(1);}while(GetTickCount64()<end);}
static const char *state(uint64_t id)
{ui_app_instance_info_t info={0};const char *text="{}";info.size=sizeof(info);T(ui_workspace_get_instance(workspace,id,&info)==UI_STATUS_OK);T(ui_assistant_get_state_snapshot(info.assistant,&text)==UI_STATUS_OK);strcpy_s(latest,sizeof(latest),text);return latest;}
static int local(uint64_t id,const char *command,const char *params)
{ui_app_instance_info_t info={0};info.size=sizeof(info);T(ui_workspace_get_instance(workspace,id,&info)==UI_STATUS_OK);return ui_host_invoke(info.host,command,params,"browser.local-ui")!=0;}
static int ready(uint64_t id)
{ULONGLONG end=GetTickCount64()+8000;do{pump(10);state(id);if(strstr(latest,"\"loading\":0")&&strstr(latest,"\"idle\":1")&&(strstr(latest,"\"pending\":0")||strstr(latest,"\"error\":1")))return 1;}while(GetTickCount64()<end);fprintf(stderr,"Not ready: %s\n",latest);return 0;}
static void navigate(uint64_t id,const char *url,int wait)
{char quoted[24578],params[28672];uint64_t request;T(browser_json_quote(url,quoted,sizeof(quoted)));snprintf(params,sizeof(params),"{\"url\":%s}",quoted);T(ui_workspace_invoke(workspace,id,"browser.navigate",params,&request)==UI_STATUS_OK);if(wait)T(ready(id));else pump(20);}
static void origins(uint64_t id,const char *text)
{char quoted[24578],params[28672];T(browser_json_quote(text,quoted,sizeof(quoted)));snprintf(params,sizeof(params),"{\"origins\":%s}",quoted);T(local(id,"browser.settings-save",params));pump(650);}
static void expect(uint64_t id,const char *path,const char *category)
{char url[512],pattern[128];snprintf(url,sizeof(url),"http://127.0.0.1:8765%s",path);navigate(id,url,1);snprintf(pattern,sizeof(pattern),"\"failure\":\"%s\"",category);T(strstr(state(id),pattern)!=NULL);T(!strstr(latest,"SuperSecret"));printf("CASE %s category=%s %s\n",path,category,latest);}
int wmain(int argc,wchar_t **argv)
{
    wchar_t directory[MAX_PATH],base[MAX_PATH],old[32768],file[32768];char package[32768];HWND root;ui_workspace_config_t config={0};ui_rect_t rect={0,0,860,640};uint64_t first=0,second=0,third=0,request;HANDLE output;DWORD bytes;
    if(argc<2)return 2;setvbuf(stdout,NULL,_IONBF,0);GetEnvironmentVariableW(L"LOCALAPPDATA",old,32768);T(GetTempPathW(MAX_PATH,directory));T(GetTempFileNameW(directory,L"ubm",0,base));T(DeleteFileW(base));T(CreateDirectoryW(base,NULL));T(SetEnvironmentVariableW(L"LOCALAPPDATA",base));
    T(ui_framework_initialize()==UI_STATUS_OK);root=CreateWindowW(L"STATIC",L"M0/M1 public caller",WS_OVERLAPPEDWINDOW,0,0,900,700,NULL,NULL,NULL,NULL);T(root!=NULL);config.size=sizeof(config);config.native_parent=root;config.shell_mode=UI_WORKSPACE_SHELL_WEB;workspace=ui_workspace_create(&config);T(workspace!=NULL);T(ui_workspace_set_rect(workspace,&rect,96)==UI_STATUS_OK);
    T(WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,argv[1],-1,package,sizeof(package),NULL,NULL)>0);T(ui_workspace_open(workspace,package,&first)==UI_STATUS_OK);if(!first)return 1;T(ui_workspace_open(workspace,package,&second)==UI_STATUS_OK);pump(40);
    T(strstr(state(first),"\"policy_count\":3")!=NULL);T(local(first,"browser.settings","{}"));pump(40);
    if(argc>2&&!wcscmp(argv[2],L"--legacy-baseline"))goto close;
    if(argc>2&&!wcscmp(argv[2],L"--external")){navigate(first,"https://example.com/",1);printf("PUBLIC_OBSERVATION %s\n",state(first));T(strstr(latest,"\"error\":0")!=NULL);goto close;}
    if(argc>2&&!wcscmp(argv[2],L"--tls-only")){navigate(first,"https://localhost:8766/page",1);T(strstr(state(first),"\"failure\":\"tls\"")!=NULL);T(strstr(latest,"\"windows\":12175")!=NULL);printf("TLS_AFTER_REMOVAL %s\n",latest);goto close;}
    T(ui_workspace_invoke(workspace,first,"browser.settings-save","{\"origins\":\"\"}",&request)==UI_STATUS_NOT_FOUND);
    origins(first,"https://user:SuperSecret@example.test");T(strstr(state(first),"\"policy_count\":3")!=NULL);
    origins(first,"http://127.0.0.1:8765\nhttp://localhost:8765\n");T(strstr(state(first),"\"policy_count\":2")!=NULL);T(strstr(state(second),"\"policy_count\":2")!=NULL);T(strstr(latest,"browser:start")!=NULL);
    navigate(first,"http://127.0.0.1:8765/cross",1);T(strstr(state(first),"\"error\":0")!=NULL);T(strstr(latest,"http://localhost:8765")!=NULL);
    navigate(first,"http://127.0.0.1:8765/slow",0);origins(second,"");T(strstr(state(first),"\"policy_count\":0")!=NULL);pump(2200);T(strstr(state(first),"browser:start")!=NULL);T(strstr(latest,"\"idle\":1")!=NULL);
    T(ui_workspace_open(workspace,package,&third)==UI_STATUS_OK);T(strstr(state(third),"\"policy_count\":0")!=NULL);
    swprintf(file,32768,L"%s\\ui-framework\\browser\\origins-v1.txt",base);output=CreateFileW(file,GENERIC_WRITE,0,NULL,TRUNCATE_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);T(output!=INVALID_HANDLE_VALUE);if(output!=INVALID_HANDLE_VALUE){T(WriteFile(output,"invalid disk rule",17,&bytes,NULL));CloseHandle(output);}pump(650);T(strstr(state(first),"\"policy_valid\":0")!=NULL);T(strstr(state(second),"\"policy_count\":0")!=NULL);
    T(local(second,"browser.settings-reset","{}"));pump(650);T(strstr(state(first),"\"policy_count\":3")!=NULL);T(strstr(state(third),"\"policy_count\":3")!=NULL);
    navigate(first,"http://[invalid",1);T(strstr(state(first),"\"failure\":\"url\"")!=NULL);
    navigate(first,"javascript:alert('SuperSecret')",1);T(strstr(state(first),"\"failure\":\"protocol\"")!=NULL);T(!strstr(latest,"SuperSecret"));
    navigate(first,"https://user:SuperSecret@example.com/",1);T(strstr(state(first),"\"failure\":\"credentials\"")!=NULL);T(!strstr(latest,"SuperSecret"));
    navigate(first,"https://example.test.evil/private?token=SuperSecret",1);T(strstr(state(first),"\"failure\":\"origin\"")!=NULL);T(!strstr(latest,"SuperSecret"));
    expect(first,"/404","http");T(strstr(latest,"\"http\":404")!=NULL);expect(first,"/500","http");expect(first,"/cross","redirect-origin");expect(first,"/protocol","redirect-protocol");expect(first,"/credentials-redirect","redirect-credentials");expect(first,"/redirect-missing","redirect-url");expect(first,"/loop","redirect-limit");
    expect(first,"/disconnect","transport");expect(first,"/slow-headers","transport");T(strstr(latest,"\"windows\":12002")||strstr(latest,"应用状态码（非系统返回） 1460"));
    /* WinHTTP can still have SendRequest outstanding while the test server
       delays its response. Preserve observed API/callback phase, not a guess. */
    T(strstr(latest,"\"phase\":4")||strstr(latest,"\"phase\":5"));
    expect(first,"/wrong-type","content-type");expect(first,"/attachment","attachment");expect(first,"/invalid-encoding","encoding");expect(first,"/big","budget");expect(first,"/longtext","document-limit");expect(first,"/empty","document-empty");
    navigate(first,"http://127.0.0.1:8765/redirect-secret?token=SuperSecret",1);T(strstr(state(first),"\"error\":0")!=NULL);T(!strstr(latest,"SuperSecret"));T(strstr(latest,"1: http://127.0.0.1:8765 ->")!=NULL);
    navigate(first,"http://127.0.0.1:8765/diag-resources",1);pump(80);T(strstr(state(first),"\"error\":0")!=NULL);T(strstr(latest,"部分呈现")!=NULL);T(strstr(latest,"http")&&strstr(latest,"origin")&&strstr(latest,"image")&&strstr(latest,"budget"));T(!strstr(latest,"SuperSecret"));printf("PARTIAL %s\n",latest);
    navigate(first,"http://127.0.0.1:8765/policy-attack",1);T(strstr(state(first),"\"script_runtime\":0")!=NULL);T(strstr(latest,"\"policy_count\":3")!=NULL);T(strstr(state(second),"\"policy_count\":3")!=NULL);
    navigate(first,"https://localhost:8766/page",1);
    if(argc>2&&!wcscmp(argv[2],L"--trusted-local")){T(strstr(state(first),"\"error\":0")!=NULL);T(strstr(latest,"\"images\":1")!=NULL);}else{T(strstr(state(first),"\"failure\":\"tls\"")!=NULL);T(strstr(latest,"\"windows\":12175")!=NULL);}printf("TLS %s\n",latest);
    navigate(first,"http://127.0.0.1:8765/slow",0);
close:
    T(ui_workspace_close_all(workspace)==UI_STATUS_OK);{ULONGLONG end=GetTickCount64()+10000;while(ui_workspace_count(workspace)&&GetTickCount64()<end)pump(10);}T(!ui_workspace_count(workspace));T(GetModuleHandleW(L"browser_app.dll")==NULL);T(ui_workspace_destroy(workspace)==UI_STATUS_OK);workspace=NULL;DestroyWindow(root);
    swprintf(file,32768,L"%s\\ui-framework\\browser\\origins-v1.txt",base);if(GetFileAttributesW(file)!=INVALID_FILE_ATTRIBUTES)T(DeleteFileW(file));swprintf(file,32768,L"%s\\ui-framework\\browser",base);if(GetFileAttributesW(file)!=INVALID_FILE_ATTRIBUTES)T(RemoveDirectoryW(file));swprintf(file,32768,L"%s\\ui-framework",base);if(GetFileAttributesW(file)!=INVALID_FILE_ATTRIBUTES)T(RemoveDirectoryW(file));T(RemoveDirectoryW(base));T(SetEnvironmentVariableW(L"LOCALAPPDATA",*old?old:NULL));
    printf("M0/M1 public integration failures=%d DLL_unloaded=%d\n",failures,GetModuleHandleW(L"browser_app.dll")==NULL);return failures?1:0;
}
