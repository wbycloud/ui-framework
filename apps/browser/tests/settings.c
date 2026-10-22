#define WIN32_LEAN_AND_MEAN
#include "browser_settings.h"
#include "browser_diagnostic.h"
#include <stdio.h>
#include <string.h>
static int failures;
#define T(x) do{if(!(x)){fprintf(stderr,"settings line%d: %s\n",__LINE__,#x);++failures;}}while(0)
int main(void)
{
    wchar_t directory[MAX_PATH],base[MAX_PATH],old[32768],file[32768];browser_settings_t a,b;browser_url_t url;HANDLE handle;DWORD count;char message[512],public_url[BROWSER_URL_CAP];
    T(GetTempPathW(MAX_PATH,directory));T(GetTempFileNameW(directory,L"ubs",0,base));T(DeleteFileW(base));T(CreateDirectoryW(base,NULL));GetEnvironmentVariableW(L"LOCALAPPDATA",old,32768);T(SetEnvironmentVariableW(L"LOCALAPPDATA",base));
    T(browser_settings_init(&a,"https://example.test\n"));T(a.valid&&!a.custom&&a.policy.count==1);
    T(!browser_settings_save(&a,"https://user:secret@example.test"));T(a.policy.count==1);
    T(browser_settings_save(&a,"https://EXAMPLE.test:443\nhttp://127.0.0.1:8765\n"));T(a.custom&&a.policy.count==2);
    T(browser_settings_init(&b,"https://example.test\n"));T(b.policy.count==2);
    handle=CreateFileW(a.path,GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);T(handle!=INVALID_HANDLE_VALUE);
    if(handle!=INVALID_HANDLE_VALUE){T(browser_settings_reload(&b));T(!b.valid&&!b.policy.count);T(!browser_settings_save(&a,""));T(a.policy.count==2);printf("locked-file actual: %s\n",a.message);T(strstr(a.message,"Windows 32")||strstr(a.message,"Windows 5"));CloseHandle(handle);}
    T(browser_settings_reload(&b));T(b.valid&&b.policy.count==2);
    T(browser_settings_save(&a,""));T(!a.policy.count&&a.valid);T(browser_settings_reload(&b));T(!b.policy.count);
    handle=CreateFileW(a.path,GENERIC_WRITE,0,NULL,TRUNCATE_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);T(handle!=INVALID_HANDLE_VALUE);if(handle!=INVALID_HANDLE_VALUE){T(WriteFile(handle,"file:///C:/",11,&count,NULL));CloseHandle(handle);}
    T(browser_settings_reload(&b));T(!b.valid&&!b.policy.count);T(strstr(b.message,"拒绝全部")!=NULL);
    T(browser_settings_reset(&a));T(a.valid&&!a.custom&&a.policy.count==1);T(browser_settings_reload(&b));T(b.policy.count==1);
    T(browser_url_parse_ex(NULL,"javascript:alert(1)",&url)==BROWSER_URL_SCHEME);
    T(browser_url_parse_ex(NULL,"https://user:secret@example.test/?token=secret",&url)==BROWSER_URL_CREDENTIALS);
    T(browser_url_parse_ex(NULL,"http://[invalid",&url)==BROWSER_URL_INVALID);
    browser_url_public("https://example.test/page?token=secret#private",public_url);T(!strcmp(public_url,"https://example.test/page"));
    browser_diagnostic_format(BROWSER_FAILURE_REDIRECT_ORIGIN,BROWSER_PHASE_HEADERS,5,302,0,message,sizeof(message));T(strstr(message,"redirect-origin")&&strstr(message,"302"));T(!strstr(message,"secret"));
    swprintf(file,32768,L"%s\\ui-framework\\browser",base);T(RemoveDirectoryW(file));swprintf(file,32768,L"%s\\ui-framework",base);T(RemoveDirectoryW(file));T(RemoveDirectoryW(base));T(SetEnvironmentVariableW(L"LOCALAPPDATA",*old?old:NULL));
    printf("settings atomic/persist/invalid/reset/classification/privacy failures=%d\n",failures);return failures?1:0;
}
