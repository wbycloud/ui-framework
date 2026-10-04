#ifndef UI_TEST_RUNTIME_DIAGNOSTICS_H
#define UI_TEST_RUNTIME_DIAGNOSTICS_H
#include <processsnapshot.h>

/* Inspect only this test's handles. Never print object names or user paths. */
static void runtime_handle_types(const char *phase)
{
    HPSS snapshot=NULL;HPSSWALK marker=NULL;PSS_HANDLE_ENTRY entry;unsigned counts[8]={0},live=0,exited=0,unknown=0;DWORD status;
    status=PssCaptureSnapshot(GetCurrentProcess(),PSS_CAPTURE_HANDLES|PSS_CAPTURE_HANDLE_BASIC_INFORMATION|PSS_CAPTURE_HANDLE_TYPE_SPECIFIC_INFORMATION,0,&snapshot);
    if(status==ERROR_SUCCESS)status=PssWalkMarkerCreate(NULL,&marker);
    if(status==ERROR_SUCCESS)while(PssWalkSnapshot(snapshot,PSS_WALK_HANDLES,marker,&entry,sizeof(entry))==ERROR_SUCCESS){
        if((unsigned)entry.ObjectType<8)++counts[entry.ObjectType];
        if(entry.ObjectType==PSS_OBJECT_TYPE_PROCESS){
            if(!(entry.Flags&PSS_HANDLE_HAVE_TYPE_SPECIFIC_INFORMATION))++unknown;else if(entry.TypeSpecificInformation.Process.ExitStatus==STILL_ACTIVE)++live;else ++exited;
        }
    }
    if(marker)PssWalkMarkerFree(marker);if(snapshot)PssFreeSnapshot(GetCurrentProcess(),snapshot);
    printf("Runtime handles %s snapshot=%lu other=%u process=%u(live=%u exited=%u unknown=%u) thread=%u mutant=%u event=%u section=%u semaphore=%u\n",phase,status,counts[0],counts[1],live,exited,unknown,counts[2],counts[3],counts[4],counts[5],counts[6]);
}
static LONG WINAPI runtime_test_exception(EXCEPTION_POINTERS *error)
{
    HMODULE module=NULL;wchar_t path[MAX_PATH]={0};const wchar_t *name=L"unknown";
    void *address=error->ExceptionRecord->ExceptionAddress;
    if(GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&module)&&GetModuleFileNameW(module,path,MAX_PATH)){
        const wchar_t *last=wcsrchr(path,L'\\');name=last?last+1:path;
    }
    fprintf(stderr,"Unhandled Runtime test exception code=%08lx module=%ls offset=%llu\n",error->ExceptionRecord->ExceptionCode,name,(unsigned long long)((uintptr_t)address-(uintptr_t)module));
    {void *frames[24];USHORT count=CaptureStackBackTrace(0,24,frames,NULL);for(USHORT i=0;i<count;++i){
        module=NULL;path[0]=0;name=L"unknown";if(GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)frames[i],&module)&&GetModuleFileNameW(module,path,MAX_PATH)){const wchar_t *last=wcsrchr(path,L'\\');name=last?last+1:path;}
        fprintf(stderr," Runtime exception stack %u %ls+%llu\n",i,name,(unsigned long long)((uintptr_t)frames[i]-(uintptr_t)module));}}
    return EXCEPTION_CONTINUE_SEARCH;
}
static void runtime_test_exit(void){puts("Runtime test: normal CRT exit callback reached");}
static void runtime_diagnostics_initialize(void)
{SetUnhandledExceptionFilter(runtime_test_exception);atexit(runtime_test_exit);}
#endif
