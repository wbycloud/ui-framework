#ifndef UI_TEST_RUNTIME_DIAGNOSTICS_H
#define UI_TEST_RUNTIME_DIAGNOSTICS_H
#include <wchar.h>
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
