#define WIN32_LEAN_AND_MEAN
#include "browser_settings.h"
#include <stdio.h>
#include <string.h>
#include <wchar.h>
static int failures;
#define T(x) do{if(!(x)){fprintf(stderr,"settings line %d: %s\n",__LINE__,#x);++failures;}}while(0)
int wmain(int argc,wchar_t **argv)
{if(argc!=2)return 2;browser_settings_t s,t;wchar_t path[4096];T(CreateDirectoryW(argv[1],NULL)||GetLastError()==ERROR_ALREADY_EXISTS);_snwprintf_s(path,4096,_TRUNCATE,L"%s/origins-%lu.txt",argv[1],GetCurrentProcessId());T(SetEnvironmentVariableW(L"UI_BROWSER_SETTINGS_PATH",path));T(browser_settings_init(&s,""));T(s.valid&&!s.policy.count);T(browser_settings_save(&s,""));T(browser_settings_init(&t,"https://example.test\n"));T(t.valid&&t.custom&&!t.policy.count);T(!browser_settings_save(&s,"https://user:password@example.test"));T(!s.policy.count);T(SetEnvironmentVariableW(L"UI_BROWSER_SETTINGS_PATH",L"relative.txt"));T(!browser_settings_init(&t,""));wchar_t longpath[32700];wmemset(longpath,L'x',32699);longpath[0]=L'C';longpath[1]=L':';longpath[2]=L'\\';longpath[32699]=0;T(SetEnvironmentVariableW(L"UI_BROWSER_SETTINGS_PATH",longpath));T(!browser_settings_init(&t,""));T(SetEnvironmentVariableW(L"UI_BROWSER_SETTINGS_PATH",path));T(browser_settings_init(&t,""));T(!wcscmp(s.path,t.path));T(SetEnvironmentVariableW(L"UI_BROWSER_SETTINGS_PATH",NULL));printf("BR-1 settings failures=%d; own file only\n",failures);return failures?1:0;}
