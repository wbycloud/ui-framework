#define WIN32_LEAN_AND_MEAN
#include "browser_settings.h"
#include <stdio.h>
#include <string.h>

static int parse(const char *text,browser_policy_t *policy,char canonical[BROWSER_SETTINGS_CAP])
{
    size_t used=0;if(strlen(text)>=BROWSER_SETTINGS_CAP||!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,text,-1,NULL,0)||!browser_policy_read(policy,text))return 0;
    canonical[0]=0;
    for(size_t i=0;i<policy->count;++i){size_t length=strlen(policy->origins[i]);if(length+2>BROWSER_SETTINGS_CAP-used)return 0;memcpy(canonical+used,policy->origins[i],length);used+=length;canonical[used++]='\n';canonical[used]=0;}
    return 1;
}
int browser_settings_init(browser_settings_t *s,const char *defaults)
{
    wchar_t base[32768];DWORD length;memset(s,0,sizeof(*s));
    if(!parse(defaults,&s->policy,s->defaults))return 0;
    length=GetEnvironmentVariableW(L"LOCALAPPDATA",base,32768);
    if(length&&length<32700)swprintf(s->path,32768,L"%s\\ui-framework\\browser\\origins-v1.txt",base);
    (void)browser_settings_reload(s);return 1;
}
int browser_settings_reload(browser_settings_t *s)
{
    HANDLE file=INVALID_HANDLE_VALUE;char text[BROWSER_SETTINGS_CAP]={0},canonical[BROWSER_SETTINGS_CAP],message[256];browser_policy_t policy={0};DWORD count=0,error=0;LARGE_INTEGER size;int valid=0,custom=0,changed;
    if(!*s->path)error=ERROR_PATH_NOT_FOUND;
    else {file=CreateFileW(s->path,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);if(file==INVALID_HANDLE_VALUE)error=GetLastError();}
    if(file!=INVALID_HANDLE_VALUE){
        custom=1;
        if(!GetFileSizeEx(file,&size))error=GetLastError();
        else if(size.QuadPart>=BROWSER_SETTINGS_CAP)error=ERROR_FILE_TOO_LARGE;
        else if(!ReadFile(file,text,(DWORD)size.QuadPart,&count,NULL)||count!=(DWORD)size.QuadPart)error=ERROR_READ_FAULT;
        else if(memchr(text,0,count)||!parse(text,&policy,canonical))error=ERROR_INVALID_DATA;
        else{valid=1;strcpy(text,canonical);}
        CloseHandle(file);
    }else if(*s->path&&(error==ERROR_FILE_NOT_FOUND||error==ERROR_PATH_NOT_FOUND)){
        valid=parse(s->defaults,&policy,canonical);strcpy(text,s->defaults);error=0;
    }
    if(!valid){memset(&policy,0,sizeof(policy));text[0]=0;if(error==ERROR_INVALID_DATA)strcpy(message,"本地来源文件无效：当前拒绝全部网络来源；请修正或恢复默认。");
        else snprintf(message,sizeof(message),"来源配置不可读取（Windows %lu）：当前拒绝全部网络来源。",(unsigned long)error);}
    else strcpy(message,custom?"使用用户规则。空列表拒绝全部网络来源。":"使用包内默认规则；保存后无需重新打包。");
    changed=!s->initialized||s->valid!=valid||s->custom!=custom||strcmp(s->text,text);
    s->policy=policy;s->valid=valid;s->custom=custom;s->initialized=1;strcpy(s->text,text);strcpy(s->message,message);return changed;
}
int browser_settings_save(browser_settings_t *s,const char *text)
{
    browser_policy_t policy;char canonical[BROWSER_SETTINGS_CAP];wchar_t temp[32768],directory[32768],*separator;HANDLE file;DWORD written=0,length,error=0;int ok;
    if(!parse(text,&policy,canonical)){strcpy(s->message,"未保存：每行仅HTTP/HTTPS精确来源，不接受凭据、路径、查询或片段；最多16项、4095字节。");return 0;}
    if(!*s->path){strcpy(s->message,"未保存：LOCALAPPDATA路径不可用。");return 0;}
    wcscpy(directory,s->path);separator=wcsrchr(directory,L'\\');*separator=0;separator=wcsrchr(directory,L'\\');*separator=0;
    if(!CreateDirectoryW(directory,NULL)&&GetLastError()!=ERROR_ALREADY_EXISTS)goto failed;
    *separator=L'\\';if(!CreateDirectoryW(directory,NULL)&&GetLastError()!=ERROR_ALREADY_EXISTS)goto failed;
    swprintf(temp,32768,L"%s.%lu.tmp",s->path,(unsigned long)GetCurrentProcessId());
    file=CreateFileW(temp,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_TEMPORARY,NULL);if(file==INVALID_HANDLE_VALUE)goto failed;
    length=(DWORD)strlen(canonical);ok=WriteFile(file,canonical,length,&written,NULL)!=0;
    if(!ok)error=GetLastError();else if(written!=length){ok=0;error=ERROR_WRITE_FAULT;}else if(!FlushFileBuffers(file)){ok=0;error=GetLastError();}CloseHandle(file);
    if(ok){ok=MoveFileExW(temp,s->path,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;if(!ok)error=GetLastError();}
    if(!ok){DeleteFileW(temp);goto failed;}(void)browser_settings_reload(s);return 1;
failed:if(!error)error=GetLastError();snprintf(s->message,sizeof(s->message),"未保存：本地文件写入失败（Windows %lu），旧规则保留。",(unsigned long)error);return 0;
}
int browser_settings_reset(browser_settings_t *s)
{
    if(!*s->path){strcpy(s->message,"无法恢复：LOCALAPPDATA路径不可用。");return 0;}
    if(!DeleteFileW(s->path)){DWORD error=GetLastError();if(error!=ERROR_FILE_NOT_FOUND&&error!=ERROR_PATH_NOT_FOUND){snprintf(s->message,sizeof(s->message),"恢复失败（Windows %lu），旧规则保留。",(unsigned long)error);return 0;}}
    (void)browser_settings_reload(s);return 1;
}
