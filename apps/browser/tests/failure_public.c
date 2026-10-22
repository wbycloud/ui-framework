#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ui_framework/application.h"
#include "ui_framework/package.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int failures;
#define T(x) do{if(!(x)){fprintf(stderr,"failure cleanup line%d: %s\n",__LINE__,#x);++failures;}}while(0)
static void utf8(const wchar_t *s,char out[32768]){T(WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,s,-1,out,32768,NULL,NULL)>0);}
static void write_file(const wchar_t *dir,const wchar_t *name,const void *data,size_t bytes)
{wchar_t path[32768];FILE *f=NULL;swprintf(path,32768,L"%s\\%s",dir,name);T(!_wfopen_s(&f,path,L"wb"));if(f){T(fwrite(data,1,bytes,f)==bytes);fclose(f);}}
static void asset(ui_package_t *pkg,const char *name,const wchar_t *dir,const wchar_t *wide_name)
{void *data=NULL;size_t bytes=0;T(ui_package_read(pkg,name,&data,&bytes)==UI_STATUS_OK);if(data){write_file(dir,wide_name,data,bytes);ui_package_release(data);}}
static void remove_file(const wchar_t *dir,const wchar_t *name)
{wchar_t path[32768];swprintf(path,32768,L"%s\\%s",dir,name);if(GetFileAttributesW(path)!=INVALID_FILE_ATTRIBUTES)T(DeleteFileW(path));}
static void pump(ui_workspace_t *w)
{MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}ui_workspace_poll(w);Sleep(1);}
int wmain(int argc,wchar_t **argv)
{
    char package_path[32768],source_path[32768],manifest_path[32768],out_path[32768],error[512];ui_package_t *package=NULL;wchar_t temporary[MAX_PATH],base[MAX_PATH],stage[MAX_PATH],manifest[MAX_PATH],output[MAX_PATH];HWND root;ui_workspace_config_t cfg={0};ui_workspace_t *w;ui_rect_t rect={0,0,640,480};DWORD before=0,after=0;
    if(argc<2)return 2;utf8(argv[1],package_path);T(ui_package_open(package_path,&package,error,sizeof(error))==UI_STATUS_OK);if(!package)return 1;
    T(GetTempPathW(MAX_PATH,temporary)>0);T(GetTempFileNameW(temporary,L"ubf",0,base)!=0);T(DeleteFileW(base));T(CreateDirectoryW(base,NULL));swprintf(stage,MAX_PATH,L"%s\\stage",base);T(CreateDirectoryW(stage,NULL));swprintf(manifest,MAX_PATH,L"%s\\manifest.ini",base);swprintf(output,MAX_PATH,L"%s\\fixture.uapp",base);utf8(stage,source_path);utf8(manifest,manifest_path);utf8(output,out_path);
    T(ui_framework_initialize()==UI_STATUS_OK);root=CreateWindowW(L"STATIC",L"Failure cleanup public caller",WS_OVERLAPPEDWINDOW,0,0,660,520,NULL,NULL,NULL,NULL);cfg.size=sizeof(cfg);cfg.native_parent=root;cfg.shell_mode=UI_WORKSPACE_SHELL_WEB;w=ui_workspace_create(&cfg);T(w!=NULL);T(ui_workspace_set_rect(w,&rect,96)==UI_STATUS_OK);GetProcessHandleCount(GetCurrentProcess(),&before);
    for(int mode=0;mode<5;++mode){uint64_t id=0;ui_status_t status;char text[512];
        asset(package,"browser_app.dll",stage,L"browser_app.dll");
        if(mode==0)write_file(stage,L"origins.txt","file:///C:/",11);else asset(package,"origins.txt",stage,L"origins.txt");
        if(mode!=1){if(mode==3)write_file(stage,L"chrome.html","<h1>unsupported chrome</h1>",27);else asset(package,"chrome.html",stage,L"chrome.html");}
        if(mode!=2)asset(package,"start.html",stage,L"start.html");
        snprintf(text,sizeof(text),"[application]\napp_id=org.ui-framework.browser.failure-%d\nname=Failure fixture\nversion=0.1.0\narchitecture=x64\nabi_version=1\nframework_api_version=9\nmodule=browser_app.dll\nmultiple_instances=true\n",mode);write_file(base,L"manifest.ini",text,strlen(text));
        T(ui_package_pack(manifest_path,source_path,out_path,error,sizeof(error))==UI_STATUS_OK);status=ui_workspace_open(w,out_path,&id);
        if(mode<4){ULONGLONG end=GetTickCount64()+5000;do{pump(w);}while(GetModuleHandleW(L"browser_app.dll")&&GetTickCount64()<end);}
        if(mode<4){T(status!=UI_STATUS_OK);T(!id);T(!ui_workspace_count(w));T(GetModuleHandleW(L"browser_app.dll")==NULL);}
        else{T(status==UI_STATUS_OK);T(ui_workspace_count(w)==1);T(ui_workspace_close_all(w)==UI_STATUS_OK);ULONGLONG end=GetTickCount64()+5000;while(ui_workspace_count(w)&&GetTickCount64()<end)pump(w);T(!ui_workspace_count(w));T(GetModuleHandleW(L"browser_app.dll")==NULL);}
        printf("failure point %d status=%d remaining=%zu DLL=%s\n",mode,(int)status,ui_workspace_count(w),GetModuleHandleW(L"browser_app.dll")?"loaded":"unloaded");
        remove_file(stage,L"browser_app.dll");remove_file(stage,L"origins.txt");remove_file(stage,L"chrome.html");remove_file(stage,L"start.html");remove_file(base,L"fixture.uapp");remove_file(base,L"manifest.ini");
    }
    GetProcessHandleCount(GetCurrentProcess(),&after);T(after<=before+12);T(ui_workspace_destroy(w)==UI_STATUS_OK);DestroyWindow(root);ui_package_destroy(package);T(RemoveDirectoryW(stage));T(RemoveDirectoryW(base));printf("initialization/mount failure cleanup failures=%d handles=%lu->%lu\n",failures,before,after);return failures?1:0;
}
