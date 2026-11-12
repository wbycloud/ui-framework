/* Repeated real host/panel lifecycle, isolated acceptance after trusted gate. */
#define UI_P6_PANEL 1
#include "ai_coordinator_panel.c"
#include <psapi.h>
#include <tlhelp32.h>
static DWORD panel_threads(void)
{DWORD count=0;HANDLE h=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);THREADENTRY32 e={0};e.dwSize=sizeof(e);if(h!=INVALID_HANDLE_VALUE){if(Thread32First(h,&e))do{if(e.th32OwnerProcessID==GetCurrentProcessId())++count;}while(Thread32Next(h,&e));CloseHandle(h);}return count;}
int wmain(int argc,wchar_t **argv)
{
 if(argc!=3)return 2;FILE *csv=NULL;wchar_t path[4096];CHECK(CreateDirectoryW(argv[2],NULL)||GetLastError()==ERROR_ALREADY_EXISTS);_snwprintf_s(path,4096,_TRUNCATE,L"%s/panel-resources-%lu.csv",argv[2],GetCurrentProcessId());csv=_wfopen(path,L"w");if(csv)fputs("cycle,handles,threads,private_bytes\n",csv);DWORD base_h=0,base_t=0;SIZE_T base_b=0;
 for(unsigned i=0;i<60;++i){CHECK(graph_panel_fixture_run(argc,argv)==0);DWORD h=0,t=panel_threads();PROCESS_MEMORY_COUNTERS_EX m={0};m.cb=sizeof(m);CHECK(GetProcessHandleCount(GetCurrentProcess(),&h));CHECK(GetProcessMemoryInfo(GetCurrentProcess(),(PROCESS_MEMORY_COUNTERS *)&m,sizeof(m)));if(csv){fprintf(csv,"%u,%lu,%lu,%zu\n",i,h,t,m.PrivateUsage);fflush(csv);}if(i==15){base_h=h;base_t=t;base_b=m.PrivateUsage;}if(i>15){CHECK(h<=base_h+8);CHECK(t<=base_t+2);CHECK(m.PrivateUsage<=base_b+32u*1024u*1024u);}CHECK(GetModuleHandleW(L"web_counter_app.dll")==NULL);if(failures)break;}
 if(csv)fclose(csv);printf("P6 repeated formal panels checks=%u failures=%u; isolated positive, not physical\n",checks,failures);return failures?1:0;
}
