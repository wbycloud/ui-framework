#define WIN32_LEAN_AND_MEAN
#define SECURITY_WIN32
#include <windows.h>
#include <ntsecapi.h>
#include <wtsapi32.h>
#include <psapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui_framework/application.h"

static int failures,created_windows,result_count,result_ok;
static int allow_interactive,require_no_login,service_started;
static DWORD logged_session_count;
static int session_inventory_valid;
static wchar_t **arguments;
static wchar_t *registered_service_name;
static SERVICE_STATUS_HANDLE service_handle;
static SERVICE_STATUS service_status;
static volatile LONG stopped;
#define CHECK(x) do { if(!(x)){fprintf(stderr,"Runner line %d: %s\n",__LINE__,#x);++failures;} } while(0)
static void result(uint64_t id,const ui_result_t *value,void *data)
{(void)data;++result_count;result_ok=value->success;printf("Result instance=%llu command=%s success=%d %s\n",(unsigned long long)id,value->command_id,value->success,value->result_json);}
static void invoke(ui_workspace_t *w,uint64_t id,const char *command,int save)
{uint64_t request=0;int before=result_count;CHECK(ui_workspace_invoke(w,id,command,save?"{\"save\":true}":"{}",&request)==UI_STATUS_OK);CHECK(result_count==before+1&&result_ok);}
static unsigned field(ui_assistant_t *assistant,const char *name)
{const char *json=NULL,*p;char key[80];CHECK(ui_assistant_get_state_snapshot(assistant,&json)==UI_STATUS_OK&&json!=NULL);if(!json)return 0;snprintf(key,sizeof(key),"\"%s\":",name);p=strstr(json,key);CHECK(p!=NULL);return p?(unsigned)_strtoui64(p+strlen(key),NULL,10):0;}
static BOOL CALLBACK count_child(HWND hwnd,LPARAM data)
{DWORD pid=0;GetWindowThreadProcessId(hwnd,&pid);if(pid==GetCurrentProcessId())++*(unsigned *)data;return TRUE;}
static BOOL CALLBACK count_root(HWND hwnd,LPARAM data)
{count_child(hwnd,data);EnumChildWindows(hwnd,count_child,data);return TRUE;}
static unsigned window_count(void)
{
    unsigned count=0;HWND message=NULL;CHECK(EnumWindows(count_root,(LPARAM)&count));
    while((message=FindWindowExW(HWND_MESSAGE,message,NULL,NULL))!=NULL)count_child(message,(LPARAM)&count);
    return count;
}
static LRESULT CALLBACK window_hook(int code,WPARAM w,LPARAM p)
{if(code==HCBT_CREATEWND)++created_windows;return CallNextHookEx(NULL,code,w,p);}
static int environment(void)
{
    DWORD session=MAXDWORD,returned=0;HANDLE token=NULL;TOKEN_STATISTICS stats={0};
    PSECURITY_LOGON_SESSION_DATA logon=NULL;WTS_SESSION_INFOW *sessions=NULL;DWORD count=0,logged=0;
    wchar_t station[256]=L"",desktop[256]=L"";BOOL system=FALSE;BYTE sid[SECURITY_MAX_SID_SIZE];DWORD sid_bytes=sizeof(sid);
    CHECK(ProcessIdToSessionId(GetCurrentProcessId(),&session));
    CHECK(OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&token));if(!token)return 0;
    CHECK(GetTokenInformation(token,TokenStatistics,&stats,sizeof(stats),&returned));
    CHECK(LsaGetLogonSessionData(&stats.AuthenticationId,&logon)==0&&logon!=NULL);
    CHECK(CreateWellKnownSid(WinLocalSystemSid,NULL,sid,&sid_bytes));CHECK(CheckTokenMembership(NULL,sid,&system));
    CHECK(GetUserObjectInformationW(GetProcessWindowStation(),UOI_NAME,station,sizeof(station),&returned));
    CHECK(GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desktop,sizeof(desktop),&returned));
    session_inventory_valid=WTSEnumerateSessionsW(WTS_CURRENT_SERVER_HANDLE,0,1,&sessions,&count)!=0;
    CHECK(session_inventory_valid);
    for(DWORD i=0;i<count;++i){wchar_t *user=NULL;DWORD bytes=0;
        BOOL queried=WTSQuerySessionInformationW(WTS_CURRENT_SERVER_HANDLE,sessions[i].SessionId,WTSUserName,&user,&bytes);
        CHECK(queried);if(!queried)session_inventory_valid=0;
        if(user&&user[0]){++logged;printf("Logged session=%lu state=%d user=%ls\n",sessions[i].SessionId,sessions[i].State,user);}
        if(user)WTSFreeMemory(user);
    }
    if(sessions)WTSFreeMemory(sessions);
    printf("Environment PID=%lu SessionId=%lu LogonType=%lu LocalSystem=%d ServiceMain=%d WindowStation=%ls Desktop=%ls logged_sessions=%lu mode=%s require_no_login=%d\n",
        GetCurrentProcessId(),session,logon?logon->LogonType:MAXDWORD,system,service_started,station,desktop,logged,allow_interactive?"INTERACTIVE_CONTROL":"SESSION0",require_no_login);
    if(!allow_interactive){
        CHECK(session==0);
        /* SYSTEM's well-known authentication session can report Undefined(0).
         * In that case require SYSTEM SID AND an actual SCM ServiceMain entry. */
        CHECK(logon&&(logon->LogonType==Batch||logon->LogonType==Service||
            (logon->LogonType==UndefinedLogonType&&system&&service_started)));
    }
    logged_session_count=logged;
    if(logon)LsaFreeReturnBuffer(logon);CloseHandle(token);
    return failures==0;
}
typedef struct resources {DWORD handles,gdi,user;SIZE_T private_bytes;unsigned windows;} resources_t;
static resources_t sample(FILE *csv,unsigned cycle)
{
    resources_t r={0};PROCESS_MEMORY_COUNTERS_EX memory={0};memory.cb=sizeof(memory);
    CHECK(GetProcessHandleCount(GetCurrentProcess(),&r.handles));
    CHECK(GetProcessMemoryInfo(GetCurrentProcess(),(PROCESS_MEMORY_COUNTERS *)&memory,sizeof(memory)));
    r.gdi=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);r.user=GetGuiResources(GetCurrentProcess(),GR_USEROBJECTS);
    r.private_bytes=memory.PrivateUsage;r.windows=window_count();
    fprintf(csv,"%u,%lu,%lu,%lu,%llu,%u,%d,%d\n",cycle,r.handles,r.gdi,r.user,(unsigned long long)r.private_bytes,r.windows,created_windows,GetModuleHandleW(L"ui_session0_gl_app.dll")!=NULL);fflush(csv);
    CHECK(r.windows==0&&created_windows==0);return r;
}
static void close_all(ui_workspace_t *w)
{
    HMODULE provider;typedef void *(APIENTRY *current_fn)(void);current_fn current;
    CHECK(ui_workspace_close_all(w)==UI_STATUS_OK);for(int n=0;n<1024&&ui_workspace_count(w);++n)ui_workspace_poll(w);
    CHECK(ui_workspace_count(w)==0);CHECK(GetModuleHandleW(L"ui_session0_gl_app.dll")==NULL);
    provider=GetModuleHandleW(L"osmesa.dll");current=provider?(current_fn)GetProcAddress(provider,"OSMesaGetCurrentContext"):NULL;
    CHECK(current&&current()==NULL);
}
static void cycle(ui_workspace_t *w,const char *package,int samples,int save)
{
    uint64_t first=0,second=0;ui_app_instance_info_t a={0},b={0};unsigned hash_a,hash_b;ui_run_mode_t mode;
    CHECK(SetEnvironmentVariableW(L"UI_SESSION0_SAMPLES",samples?L"4":L"0"));
    CHECK(ui_workspace_open(w,package,&first)==UI_STATUS_OK);CHECK(ui_workspace_open(w,package,&second)==UI_STATUS_OK&&first!=second);
    if(!first||!second){fprintf(stderr,"Application open failure: %s\n",ui_workspace_last_error(w));close_all(w);return;}
    a.size=b.size=sizeof(a);CHECK(ui_workspace_get_instance(w,first,&a)==UI_STATUS_OK);CHECK(ui_workspace_get_instance(w,second,&b)==UI_STATUS_OK);
    CHECK(!a.native_container&&!b.native_container&&GetModuleHandleW(L"ui_session0_gl_app.dll")!=NULL);
    CHECK(ui_host_get_run_mode(a.host,&mode)==UI_STATUS_OK&&mode==UI_RUN_OFFSCREEN);
    CHECK(field(a.assistant,"frames")==0&&field(b.assistant,"frames")==0);
    CHECK(ui_workspace_activate(w,first)==UI_STATUS_OK);invoke(w,first,"test.render",save);hash_a=field(a.assistant,"hash");
    CHECK(field(a.assistant,"width")==64&&field(a.assistant,"height")==64&&field(a.assistant,"actual_samples")== (unsigned)samples);
    CHECK(field(a.assistant,"frames")==1&&hash_a!=0);
    invoke(w,second,"test.input",0);CHECK(field(b.assistant,"inputs")==0&&field(b.assistant,"input_status")==UI_STATUS_CANCELLED);
    invoke(w,first,"test.input",0);CHECK(field(a.assistant,"inputs")==1&&field(a.assistant,"input_status")==UI_STATUS_OK);
    invoke(w,first,"test.render",save);CHECK(field(a.assistant,"frames")==2&&field(a.assistant,"hash")!=hash_a);
    CHECK(ui_workspace_activate(w,second)==UI_STATUS_OK);invoke(w,second,"test.render",save);hash_b=field(b.assistant,"hash");CHECK(hash_b==hash_a);
    invoke(w,second,"test.render",0);CHECK(field(b.assistant,"frames")==2&&field(b.assistant,"hash")==hash_b);
    CHECK(ui_workspace_activate(w,first)==UI_STATUS_OK);invoke(w,first,"test.resize",0);invoke(w,first,"test.render",save);
    CHECK(field(a.assistant,"width")==80&&field(a.assistant,"height")==50&&field(a.assistant,"frames")==3);
    invoke(w,first,"test.budget",0);invoke(w,first,"test.render",0);
    CHECK(field(a.assistant,"width")==80&&field(a.assistant,"height")==50&&field(a.assistant,"frames")==4);
    CHECK(field(a.assistant,"errors")==0&&field(b.assistant,"errors")==0&&field(b.assistant,"inputs")==0);
    CHECK(window_count()==0&&created_windows==0);close_all(w);
}
static int run(void)
{
    char package[131072];wchar_t file_path[32768];FILE *csv=NULL,*done=NULL;
    ui_workspace_config_t config={0};ui_workspace_t *w=NULL;ui_rect_t rect={0,0,128,96};HHOOK hook=NULL;resources_t base={0},last={0},peak={0};
    if(!environment())goto complete;
    CHECK(WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,arguments[1],-1,package,sizeof(package),NULL,NULL)>0);
    CHECK(SetEnvironmentVariableW(L"UI_SESSION0_OSMESA_DLL",arguments[2]));CHECK(SetEnvironmentVariableW(L"UI_SESSION0_OUTPUT",arguments[3]));
    hook=SetWindowsHookExW(WH_CBT,window_hook,NULL,GetCurrentThreadId());CHECK(hook!=NULL);
    config.size=sizeof(config);config.run_mode=UI_RUN_OFFSCREEN;config.result=result;config.max_permission=UI_ASSISTANT_PERMISSION_EDIT;
    w=ui_workspace_create(&config);CHECK(w!=NULL);if(!w)goto complete;
    CHECK(ui_workspace_set_rect(w,&rect,96)==UI_STATUS_OK);
    swprintf_s(file_path,32768,L"%s/resources.csv",arguments[3]);CHECK(_wfopen_s(&csv,file_path,L"wb")==0&&csv!=NULL);if(!csv)goto complete;
    fprintf(csv,"cycle,handles,gdi,user,private_bytes,process_hwnds,created_hwnds,app_dll_loaded\n");
    for(unsigned i=0;i<4&&!failures;++i){cycle(w,package,i%2?4:0,i<2);last=sample(csv,i);}
    base=peak=last;
    /* Mesa/LLVM and Windows heaps retain process caches. Private commitment is
     * bounded by a declared whole-process budget, not required to return to the
     * four-cycle warm-up value. Keep every sample, including transient peaks. */
    for(unsigned i=4;i<200&&!failures&&!stopped;++i){
        cycle(w,package,i%2?4:0,0);last=sample(csv,i);
        if(last.handles>peak.handles)peak.handles=last.handles;if(last.private_bytes>peak.private_bytes)peak.private_bytes=last.private_bytes;
        CHECK(last.handles<=base.handles+2&&last.gdi==base.gdi&&last.user==base.user);
        CHECK(last.private_bytes<=128u*1024u*1024u);
    }
    CHECK(!stopped&&result_count==200*10);
    CHECK(ui_workspace_destroy(w)==UI_STATUS_OK);w=NULL;
    last=sample(csv,200);CHECK(last.handles<=base.handles+2&&last.gdi==base.gdi&&last.user==base.user);
    CHECK(last.private_bytes<=128u*1024u*1024u);
    printf("Resources baseline/final/peak handles=%lu/%lu/%lu GDI=%lu/%lu USER=%lu/%lu private=%llu/%llu/%llu; provider_loaded=%d (process cache)\n",base.handles,last.handles,peak.handles,base.gdi,last.gdi,base.user,last.user,(unsigned long long)base.private_bytes,(unsigned long long)last.private_bytes,(unsigned long long)peak.private_bytes,GetModuleHandleW(L"osmesa.dll")!=NULL);
complete:
    if(w){close_all(w);CHECK(ui_workspace_destroy(w)==UI_STATUS_OK);}if(hook)UnhookWindowsHookEx(hook);if(csv)fclose(csv);
    if(!allow_interactive)printf("SESSION0 GL %s: failures=%d commands=%d\n",failures?"FAIL":"PASS",failures,result_count);
    /* Keep independent Session0 GL evidence if the machine has a logged-in
     * session. The combined no-login run still fails; never log users off. */
    if(require_no_login){printf("NO_LOGIN %s: logged_sessions=%lu inventory_valid=%d\n",!session_inventory_valid?"UNVERIFIED":logged_session_count?"FAIL":"PASS",logged_session_count,session_inventory_valid);CHECK(session_inventory_valid&&logged_session_count==0);}
    printf("%s %s: failures=%d commands=%d created_HWNDs=%d process_HWNDs=%u\n",allow_interactive?"INTERACTIVE_CONTROL":"SESSION0",failures?"FAIL":"PASS",failures,result_count,created_windows,window_count());fflush(stdout);fflush(stderr);
    swprintf_s(file_path,32768,L"%s/result.txt",arguments[3]);if(!_wfopen_s(&done,file_path,L"wb")){fprintf(done,"%d\n",failures?1:0);fclose(done);}else ++failures;
    return failures?1:0;
}
static void WINAPI control(DWORD code)
{if(code==SERVICE_CONTROL_STOP)InterlockedExchange(&stopped,1);}
static void WINAPI service_main(DWORD argc,wchar_t **argv)
{
    int exit_code;(void)argc;(void)argv;service_handle=RegisterServiceCtrlHandlerW(registered_service_name,control);
    if(!service_handle)return;service_started=1;service_status.dwServiceType=SERVICE_WIN32_OWN_PROCESS;
    service_status.dwCurrentState=SERVICE_RUNNING;service_status.dwControlsAccepted=SERVICE_ACCEPT_STOP;
    SetServiceStatus(service_handle,&service_status);exit_code=run();service_status.dwCurrentState=SERVICE_STOPPED;
    service_status.dwWin32ExitCode=exit_code?ERROR_SERVICE_SPECIFIC_ERROR:0;service_status.dwServiceSpecificExitCode=(DWORD)exit_code;SetServiceStatus(service_handle,&service_status);
}
int wmain(int argc,wchar_t **argv)
{
    wchar_t path[32768];FILE *log=NULL;wchar_t *service_name=NULL;
    if(argc<4){fprintf(stderr,"usage: runner package osmesa output [--allow-interactive] [--require-no-login] [--service name]\n");return 2;}
    arguments=argv;
    for(int i=4;i<argc;++i){if(!wcscmp(argv[i],L"--allow-interactive"))allow_interactive=1;else if(!wcscmp(argv[i],L"--require-no-login"))require_no_login=1;
        else if(!wcscmp(argv[i],L"--service")&&i+1<argc)service_name=argv[++i];else return 2;}
    swprintf_s(path,32768,L"%s/run.log",argv[3]);if(_wfreopen_s(&log,path,L"wb",stdout))return 2;
    /* SCM/DETACHED_PROCESS has no inherited stderr descriptor (-2). Reopen
     * each stream directly; do not dup to that invalid descriptor. */
    swprintf_s(path,32768,L"%s/errors.log",argv[3]);if(_wfreopen_s(&log,path,L"wb",stderr))return 2;
    setvbuf(stdout,NULL,_IONBF,0);setvbuf(stderr,NULL,_IONBF,0);
    if(service_name){SERVICE_TABLE_ENTRYW table[2]={{service_name,service_main},{NULL,NULL}};
        registered_service_name=service_name;
        if(!StartServiceCtrlDispatcherW(table)){fprintf(stderr,"SCM dispatch error=%lu\n",GetLastError());return 1;}return service_status.dwServiceSpecificExitCode?1:0;
    }
    return run();
}
