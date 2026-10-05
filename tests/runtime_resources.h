#ifndef UI_TEST_RUNTIME_RESOURCES_H
#define UI_TEST_RUNTIME_RESOURCES_H
#include <processsnapshot.h>
#include <tlhelp32.h>
/* Diagnostic snapshots are freed before resource assertions. They do not
 * change the lifecycle's baseline or permit additional handles. */
static void runtime_resources(const char *stage)
{
    HPSS snapshot=NULL;HPSSWALK marker=NULL;PSS_HANDLE_ENTRY entry;
    wchar_t names[32][64]={{0}};unsigned counts[32]={0},used=0;DWORD handles=0;
    if(!GetEnvironmentVariableW(L"UI_RUNTIME_RESOURCES",NULL,0))return;
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
