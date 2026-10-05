#ifndef UI_TEST_RUNTIME_RESOURCES_H
#define UI_TEST_RUNTIME_RESOURCES_H
#include <processsnapshot.h>
#include <tlhelp32.h>
static HHOOK runtime_inventory_hook;
static LRESULT CALLBACK runtime_inventory_create(int code,WPARAM window,LPARAM data)
{
    if(code==HCBT_CREATEWND){wchar_t buffer[128]={0};const wchar_t *name=((CBT_CREATEWNDW *)data)->lpcs->lpszClass;
        if((ULONG_PTR)name<=65535){GetClassNameW((HWND)window,buffer,128);name=buffer;}
        if(!wcscmp(name,L"SystemUserAdapterWindowClass")){
            void *frames[24]={0};USHORT count=CaptureStackBackTrace(0,24,frames,NULL);
            fprintf(stderr,"SystemUserAdapter creation stack\n");
            for(USHORT i=0;i<count;++i){HMODULE module=NULL;wchar_t path[MAX_PATH]={0};GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)frames[i],&module);GetModuleFileNameW(module,path,MAX_PATH);fprintf(stderr," %ls+%llx\n",path,(unsigned long long)((ULONG_PTR)frames[i]-(ULONG_PTR)module));}
        }
    }
    return CallNextHookEx(runtime_inventory_hook,code,window,data);
}
static BOOL CALLBACK runtime_inventory_window(HWND window,LPARAM unused)
{
    DWORD pid=0;wchar_t name[128]={0};(void)unused;
    GetWindowThreadProcessId(window,&pid);if(pid!=GetCurrentProcessId())return TRUE;
    GetClassNameW(window,name,128);printf(" Window=%p class=%ls thread=%lu\n",window,name,GetWindowThreadProcessId(window,NULL));return TRUE;
}
static void runtime_inventory(const char *stage)
{
    HANDLE modules;MODULEENTRY32W m={0};HWND window=NULL;
    if(!GetEnvironmentVariableW(L"UI_RUNTIME_INVENTORY",NULL,0))return;
    if(strstr(stage,"baseline")&&!runtime_inventory_hook)runtime_inventory_hook=SetWindowsHookExW(WH_CBT,runtime_inventory_create,NULL,GetCurrentThreadId());
    if(strstr(stage,"settled")&&runtime_inventory_hook){UnhookWindowsHookEx(runtime_inventory_hook);runtime_inventory_hook=NULL;}
    printf("Inventory %s\n",stage);EnumWindows(runtime_inventory_window,0);
    while((window=FindWindowExW(HWND_MESSAGE,window,NULL,NULL))!=NULL)runtime_inventory_window(window,0);
    modules=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,GetCurrentProcessId());
    if(modules!=INVALID_HANDLE_VALUE){m.dwSize=sizeof(m);if(Module32FirstW(modules,&m))do{printf(" Module=%ls\n",m.szModule);}while(Module32NextW(modules,&m));CloseHandle(modules);}
}
/* Diagnostic snapshots are freed before resource assertions. They do not
 * change the lifecycle's baseline or permit additional handles. */
static void runtime_resources(const char *stage)
{
    HPSS snapshot=NULL;HPSSWALK marker=NULL;PSS_HANDLE_ENTRY entry;
    wchar_t names[32][64]={{0}};unsigned counts[32]={0},used=0;DWORD handles=0;
    runtime_inventory(stage);
    if(!GetEnvironmentVariableW(L"UI_RUNTIME_RESOURCES",NULL,0))return;
    if(GetEnvironmentVariableW(L"UI_RUNTIME_ONLY_SETTLED",NULL,0)&&!strstr(stage,"settled"))return;
    GetProcessHandleCount(GetCurrentProcess(),&handles);
    printf("Resources %s pid=%lu handles=%lu",stage,GetCurrentProcessId(),handles);
    if(PssCaptureSnapshot(GetCurrentProcess(),PSS_CAPTURE_HANDLES|PSS_CAPTURE_HANDLE_NAME_INFORMATION|PSS_CAPTURE_HANDLE_TYPE_SPECIFIC_INFORMATION,0,&snapshot)==ERROR_SUCCESS){
        if(PssWalkMarkerCreate(NULL,&marker)==ERROR_SUCCESS){
            while(PssWalkSnapshot(snapshot,PSS_WALK_HANDLES,marker,&entry,sizeof(entry))==ERROR_SUCCESS){
                unsigned i;const wchar_t *name=entry.TypeName?entry.TypeName:L"unknown";
                for(i=0;i<used&&wcscmp(names[i],name);++i){}
                if(i==used&&used<32){wcsncpy_s(names[used],64,name,_TRUNCATE);++used;}
                if(i<32)++counts[i];
                if(!strcmp(stage,"settled")&&entry.ObjectType==PSS_OBJECT_TYPE_PROCESS)printf("\n Retained process handle=%p pid=%lu exit=%lu",entry.Handle,entry.TypeSpecificInformation.Process.ProcessId,entry.TypeSpecificInformation.Process.ExitStatus);
            }
            PssWalkMarkerFree(marker);
        }
        PssFreeSnapshot(GetCurrentProcess(),snapshot);
    }
    for(unsigned i=0;i<used;++i)printf(" %ls=%u",names[i],counts[i]);puts("");
    {HANDLE processes=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);PROCESSENTRY32W p={0};DWORD parents[64]={GetCurrentProcessId()};size_t count=1;
        if(processes!=INVALID_HANDLE_VALUE){for(size_t level=0;level<count;++level){p.dwSize=sizeof(p);if(Process32FirstW(processes,&p))do{
            if(p.th32ParentProcessID==parents[level]&&!_wcsicmp(p.szExeFile,L"msedgewebview2.exe")){
                HANDLE child=OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,p.th32ProcessID);DWORD n=0;if(child){GetProcessHandleCount(child,&n);CloseHandle(child);}
                printf(" Runtime process pid=%lu parent=%lu handles=%lu\n",p.th32ProcessID,p.th32ParentProcessID,n);if(count<64)parents[count++]=p.th32ProcessID;
            }}while(Process32NextW(processes,&p));}CloseHandle(processes);}}
}
#endif
