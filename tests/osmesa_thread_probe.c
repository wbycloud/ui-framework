/* Standalone OSMesa24.3.4 lifecycle/worker diagnostic. No framework or app DLL. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef LONG (WINAPI *query_thread_t)(HANDLE,ULONG,PVOID,ULONG,PULONG);
typedef HRESULT (WINAPI *thread_description_t)(HANDLE,PWSTR *);
typedef void * (WINAPI *create_context_t)(const int *,void *);
typedef void (WINAPI *destroy_context_t)(void *);
typedef int (WINAPI *make_current_t)(void *,void *,unsigned int,int,int);
typedef PROC (WINAPI *get_proc_t)(const char *);
static int failures, gate;
static const wchar_t *provider_path;
static int owned_loads;

static void check(int ok,const char *name)
{ if(!ok){++failures;printf("CHECK_FAIL %s error=%lu\n",name,GetLastError());} }

static void boundary(const char *phase)
{
    DWORD count=0;HANDLE snapshot;THREADENTRY32 entry;unsigned total=0,raster=0;
    query_thread_t query;thread_description_t description;
    PROC proc=GetProcAddress(GetModuleHandleW(L"ntdll.dll"),"NtQueryInformationThread");
    memcpy(&query,&proc,sizeof(query));
    proc=GetProcAddress(GetModuleHandleW(L"kernel32.dll"),"GetThreadDescription");
    memcpy(&description,&proc,sizeof(description));
    GetProcessHandleCount(GetCurrentProcess(),&count);
    printf("BOUNDARY phase=%s pid=%lu tick=%llu module=%p owned_loads=%d handles=%lu gdi=%lu user=%lu\n",
        phase,GetCurrentProcessId(),(unsigned long long)GetTickCount64(),GetModuleHandleW(provider_path),owned_loads,count,
        GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS),GetGuiResources(GetCurrentProcess(),GR_USEROBJECTS));
    snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);
    memset(&entry,0,sizeof(entry));entry.dwSize=sizeof(entry);
    if(snapshot!=INVALID_HANDLE_VALUE&&Thread32First(snapshot,&entry))do{
        if(entry.th32OwnerProcessID==GetCurrentProcessId()){
            HANDLE thread=OpenThread(THREAD_QUERY_INFORMATION,FALSE,entry.th32ThreadID);
            PWSTR wide=NULL;char name[160]="";PVOID start=NULL;LONG status=-1;
            if(thread){
                if(description&&SUCCEEDED(description(thread,&wide))&&wide){
                    WideCharToMultiByte(CP_UTF8,0,wide,-1,name,sizeof(name),NULL,NULL);LocalFree(wide);
                }
                if(query)status=query(thread,9,&start,sizeof(start),NULL);
                CloseHandle(thread);
            }
            ++total;if(strstr(name,"llvmpipe"))++raster;
            printf("THREAD phase=%s tid=%lu name=%s start=%p query=%ld\n",phase,entry.th32ThreadID,name,start,status);
        }
    }while(Thread32Next(snapshot,&entry));
    if(snapshot!=INVALID_HANDLE_VALUE)CloseHandle(snapshot);
    printf("THREAD_TOTAL phase=%s total=%u named_raster=%u\n",phase,total,raster);
    fflush(stdout);
    if(gate&&!strcmp(phase,"rendered")){printf("GATE rendered\n");fflush(stdout);check(getchar()=='\n',"parent gate");}
}

#define LOAD(variable,type,name) type variable; do{PROC p=GetProcAddress(library,name);memcpy(&variable,&p,sizeof(p));if(!variable){fprintf(stderr,"Missing %s\n",name);return 3;}}while(0)
#define LOAD_GL_PROC(variable,type,name) type variable; do{PROC p=get_proc(name);memcpy(&variable,&p,sizeof(p));if(!variable){fprintf(stderr,"Missing %s\n",name);return 3;}}while(0)
typedef void (APIENTRY *color4_t)(float,float,float,float);
typedef void (APIENTRY *clear_t)(unsigned int);
typedef void (APIENTRY *void_t)(void);
typedef void (APIENTRY *enum_t)(unsigned int);
typedef void (APIENTRY *color3_t)(float,float,float);
typedef void (APIENTRY *vertex_t)(float,float);
typedef void (APIENTRY *viewport_t)(int,int,int,int);
typedef const unsigned char *(APIENTRY *string_t)(unsigned int);
typedef unsigned int (APIENTRY *error_t)(void);

int wmain(int argc,wchar_t **argv)
{
    const int width=512,height=512,frames=120;
    int attributes[]={0x22,GL_RGBA,0x30,24,0x33,0x35,0x36,3,0x37,3,0};
    unsigned char *pixels;void *context;HMODULE library,extra;
    LARGE_INTEGER frequency,start,end;char threads[80];DWORD env_size;
    uint64_t hash=UINT64_C(14695981039346656037);int unload,cycle,cleanup;
    if(argc!=3&&argc!=5){fprintf(stderr,"Usage: probe absolute-osmesa.dll exit|unload|cycle|cleanup [manager-RVA global-RVA]\n");return 2;}
    provider_path=argv[1];unload=!wcscmp(argv[2],L"unload");cycle=!wcscmp(argv[2],L"cycle");
    cleanup=!wcscmp(argv[2],L"cleanup");
    if(cleanup!=(argc==5))return 2;
    gate=GetEnvironmentVariableA("UI_MESA_PROBE_GATE",threads,sizeof(threads))!=0;
    env_size=GetEnvironmentVariableA("LP_NUM_THREADS",threads,sizeof(threads));
    printf("START pid=%lu lp_num_threads=%s mode=%ls\n",GetCurrentProcessId(),env_size?threads:"<unset>",argv[2]);
    boundary("before-load");
    library=LoadLibraryExW(provider_path,NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    check(library!=NULL,"LoadLibraryEx");if(!library)return 3;++owned_loads;
    extra=LoadLibraryExW(provider_path,NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    check(extra==library,"second owned reference");if(extra)++owned_loads;
    check(FreeLibrary(extra)!=0,"release extra reference");--owned_loads;
    boundary("loaded");
    LOAD(create_context,create_context_t,"OSMesaCreateContextAttribs");
    LOAD(destroy_context,destroy_context_t,"OSMesaDestroyContext");
    LOAD(make_current,make_current_t,"OSMesaMakeCurrent");
    LOAD(get_proc,get_proc_t,"OSMesaGetProcAddress");
    pixels=(unsigned char *)calloc((size_t)width*height,4);if(!pixels)return 3;
    for(int pass=0;pass<(cycle?8:1);++pass){
        printf("CONTEXT_CREATE_BEGIN pass=%d\n",pass);fflush(stdout);
        context=create_context(attributes,NULL);check(context!=NULL,"context create");if(!context)return 3;
        check(make_current(context,pixels,GL_UNSIGNED_BYTE,width,height)!=0,"make current");
        boundary("context-current");
        LOAD_GL_PROC(clear_color,color4_t,"glClearColor");LOAD_GL_PROC(clear,clear_t,"glClear");
        LOAD_GL_PROC(finish,void_t,"glFinish");LOAD_GL_PROC(begin,enum_t,"glBegin");LOAD_GL_PROC(end_gl,void_t,"glEnd");
        LOAD_GL_PROC(color,color3_t,"glColor3f");LOAD_GL_PROC(vertex,vertex_t,"glVertex2f");LOAD_GL_PROC(viewport,viewport_t,"glViewport");
        LOAD_GL_PROC(get_string,string_t,"glGetString");LOAD_GL_PROC(get_error,error_t,"glGetError");
        printf("GL renderer=%s version=%s\n",get_string(GL_RENDERER),get_string(GL_VERSION));
        viewport(0,0,width,height);QueryPerformanceFrequency(&frequency);QueryPerformanceCounter(&start);
        for(int frame=0;frame<frames;++frame){
            clear_color(0.0f,0.0f,0.0f,1.0f);clear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            begin(GL_TRIANGLES);
            for(int triangle=0;triangle<64;++triangle){
                color(1.0f,0.0f,0.0f);vertex(-0.8f,-0.8f);vertex(0.8f,-0.8f);vertex(0.0f,0.8f);
            }
            end_gl();finish();
        }
        QueryPerformanceCounter(&end);
        for(size_t i=0;i<(size_t)width*height*4;++i){hash^=pixels[i];hash*=UINT64_C(1099511628211);}
        check(get_error()==GL_NO_ERROR,"GL error");
        {size_t center=((size_t)height/2*width+width/2)*4;
         check(pixels[center]==255&&pixels[center+1]==0&&pixels[center+2]==0&&pixels[center+3]==255,"triangle center readable");}
        check(pixels[0]==0&&pixels[1]==0&&pixels[2]==0&&pixels[3]==255,"background readable");
        printf("RENDER pass=%d frames=%d triangles=64 width=%d height=%d elapsed_ms=%.3f hash=%016llx center=%u,%u,%u,%u\n",
            pass,frames,width,height,1000.0*(double)(end.QuadPart-start.QuadPart)/(double)frequency.QuadPart,
            (unsigned long long)hash,pixels[((size_t)height/2*width+width/2)*4],
            pixels[((size_t)height/2*width+width/2)*4+1],pixels[((size_t)height/2*width+width/2)*4+2],
            pixels[((size_t)height/2*width+width/2)*4+3]);
        boundary("rendered");
        printf("CONTEXT_DESTROY_BEGIN pass=%d\n",pass);fflush(stdout);
        make_current(NULL,NULL,GL_UNSIGNED_BYTE,0,0);destroy_context(context);
        printf("CONTEXT_DESTROY_END pass=%d\n",pass);boundary("context-destroyed");
    }
    free(pixels);
    if(cleanup){
        /* Private, PDB-matched diagnostic only: no framework/provider API.
         * Call outside loader lock, then clear the singleton so registered
         * atexit cleanup cannot destroy the same allocation a second time. */
        uintptr_t manager_rva=(uintptr_t)wcstoull(argv[3],NULL,16);
        uintptr_t global_rva=(uintptr_t)wcstoull(argv[4],NULL,16);
        void (*destroy_manager)(void);void **global;
        PROC proc=(PROC)((uintptr_t)library+manager_rva);
        memcpy(&destroy_manager,&proc,sizeof(destroy_manager));
        global=(void **)((uintptr_t)library+global_rva);
        printf("PRIVATE_MANAGER_DESTROY_BEGIN manager_rva=%llx global_rva=%llx global=%p\n",
            (unsigned long long)manager_rva,(unsigned long long)global_rva,*global);fflush(stdout);
        destroy_manager();*global=NULL;
        printf("PRIVATE_MANAGER_DESTROY_END\n");boundary("manager-destroyed-outside-loader");
    }
    if(unload||cycle||cleanup){
        printf("FREE_LIBRARY_BEGIN owned_loads=%d\n",owned_loads);fflush(stdout);
        check(FreeLibrary(library)!=0,"release final owned reference");--owned_loads;
        printf("FREE_LIBRARY_END\n");boundary("after-free-library");
        if(cleanup)check(GetModuleHandleW(provider_path)==NULL,"true module unload after private manager cleanup");
    }
    boundary("before-main-return");
    printf("MAIN_RETURN failures=%d\n",failures);fflush(stdout);
    return failures?1:0;
}
