/* Count actual dispatched/sent paints, including invalid regions after return. */
#define UI_HOST_TEST 1
#include "../examples/framework_host/main.c"
typedef struct paint_count { HWND window; wchar_t name[80]; unsigned paints,before,after,positions; } paint_count_t;
static paint_count_t windows[64];
static unsigned window_count,refreshes,wakes,layouts,events,queued_paints,dispatched;
static HHOOK enter_hook,return_hook;
static ui_event_callback_fn original_event;
static paint_count_t *count_for(HWND window)
{
    unsigned i;for(i=0;i<window_count;++i)if(windows[i].window==window)return &windows[i];
    if(window_count==64)return NULL;i=window_count++;windows[i].window=window;GetClassNameW(window,windows[i].name,80);return &windows[i];
}
static LRESULT CALLBACK entering(int code,WPARAM wp,LPARAM lp)
{
    if(code>=0){CWPSTRUCT *m=(CWPSTRUCT *)lp;paint_count_t *c;
        if(m->message==WM_PAINT&&(c=count_for(m->hwnd))!=NULL){++c->paints;c->before+=GetUpdateRect(m->hwnd,NULL,FALSE)!=0;}
        if(m->message==WM_WINDOWPOSCHANGED&&(c=count_for(m->hwnd))!=NULL)++c->positions;
        refreshes+=m->message==HOST_REFRESH_MESSAGE;wakes+=m->message==UI_WORKSPACE_WAKE_MESSAGE;
    }return CallNextHookEx(enter_hook,code,wp,lp);
}
static LRESULT CALLBACK returning(int code,WPARAM wp,LPARAM lp)
{
    if(code>=0){CWPRETSTRUCT *m=(CWPRETSTRUCT *)lp;paint_count_t *c;
        if(m->message==WM_PAINT&&(c=count_for(m->hwnd))!=NULL)c->after+=GetUpdateRect(m->hwnd,NULL,FALSE)!=0;
    }return CallNextHookEx(return_hook,code,wp,lp);
}
static void observed_event(const char *event,const char *json,void *data)
{++events;layouts+=!strcmp(event,"ui.host.layout_changed");original_event(event,json,data);}
static void pump_until(ULONGLONG end)
{
    while(GetTickCount64()<end){MSG m;unsigned n=0;ULONGLONG start=GetTickCount64();
        while(n<3000&&GetTickCount64()-start<100&&PeekMessageW(&m,NULL,0,0,PM_REMOVE)){
            ++n;if(m.message!=WM_QUIT){paint_count_t *c=NULL;
                if(m.message==WM_PAINT){++queued_paints;c=count_for(m.hwnd);if(c){++c->paints;c->before+=GetUpdateRect(m.hwnd,NULL,FALSE)!=0;}}
                TranslateMessage(&m);DispatchMessageW(&m);++dispatched;
                if(c)c->after+=GetUpdateRect(m.hwnd,NULL,FALSE)!=0;}}
        if(!n){ULONGLONG left=end-GetTickCount64();if(left<2000)MsgWaitForMultipleObjects(0,NULL,FALSE,(DWORD)left,QS_ALLINPUT);}
    }
}
static uint64_t cpu_time(void)
{FILETIME created,exited,kernel,user;ULARGE_INTEGER k,u;GetProcessTimes(GetCurrentProcess(),&created,&exited,&kernel,&user);k.LowPart=kernel.dwLowDateTime;k.HighPart=kernel.dwHighDateTime;u.LowPart=user.dwLowDateTime;u.HighPart=user.dwHighDateTime;return k.QuadPart+u.QuadPart;}
static unsigned measure(const char *phase,DWORD duration,host_window_t *host,int width)
{
    uint64_t cpu=cpu_time();DWORD handles=0;unsigned paints=0;ULONGLONG start=GetTickCount64();
    memset(windows,0,sizeof(windows));window_count=refreshes=wakes=layouts=events=queued_paints=dispatched=0;
    if(width>0){MoveWindow(host->hwnd,40,40,width,800,TRUE);layout(host);}
    else if(width==-1)InvalidateRect(host->web_hwnd,NULL,FALSE);
    else if(width==-2){ui_app_instance_info_t info={0};if(get_instance(host,ui_workspace_active(host->workspace),&info))(void)ui_host_invoke(info.host,"test.render","{}","repaint-observer");}
    pump_until(start+duration);
    GetProcessHandleCount(GetCurrentProcess(),&handles);
    for(unsigned i=0;i<window_count;++i){paint_count_t *c=&windows[i];paints+=c->paints;printf("WINDOW %s hwnd%p class%ls paint%u invalid-before%u invalid-after%u positions%u\n",phase,(void *)c->window,c->name,c->paints,c->before,c->after,c->positions);}
    printf("MEASURE %s elapsed%llu cpu-ms%.3f paint%u queued-paint%u dispatched%u host-refresh%u wake%u layouts%u events%u handles%lu GDI%lu USER%lu\n",phase,(unsigned long long)(GetTickCount64()-start),(double)(cpu_time()-cpu)/10000.,paints,queued_paints,dispatched,refreshes,wakes,layouts,events,handles,GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS),GetGuiResources(GetCurrentProcess(),GR_USEROBJECTS));return paints;
}
int wmain(int argc,wchar_t **argv)
{
    host_window_t host={0};ui_app_instance_info_t info={0};HWND root;wchar_t provider[4096];int failed=0;
    if(argc<3)return 2;setvbuf(stdout,NULL,_IONBF,0);if(FAILED(CoInitializeEx(NULL,COINIT_APARTMENTTHREADED))||ui_framework_initialize()!=UI_STATUS_OK)return 2;
    GetFullPathNameW(argv[2],4096,provider,NULL);SetEnvironmentVariableW(L"UI_API7_OSMESA_DLL",provider);
    enter_hook=SetWindowsHookExW(WH_CALLWNDPROC,entering,NULL,GetCurrentThreadId());return_hook=SetWindowsHookExW(WH_CALLWNDPROCRET,returning,NULL,GetCurrentThreadId());if(!enter_hook||!return_hook)return 2;
    root=create_host(&host,GetModuleHandleW(NULL));if(!root)return 2;ShowWindow(root,SW_SHOW);MoveWindow(root,40,40,1200,800,TRUE);open_path(&host,argv[1]);pump_until(GetTickCount64()+1000);
    if(!get_instance(&host,ui_workspace_active(host.workspace),&info))return 2;original_event=info.host->event_callback;info.host->event_callback=observed_event;
    if(!measure("invalidate-control",200,&host,-1)||GetUpdateRect(host.web_hwnd,NULL,FALSE))failed=1;
    for(int i=0;i<3;++i){char phase[32];snprintf(phase,sizeof(phase),"idle-%d",i);if(measure(phase,2000,&host,0)>100)failed=1;}
    for(int i=0;i<3;++i)measure("resize",500,&host,1000+i*100);
    measure("explicit-GL-frame",500,&host,-2);if(!strstr(host.result_summary,"操作完成"))failed=1;
    if(measure("idle-after-GL",2000,&host,0)>100)failed=1;
    SendMessageW(root,WM_CLOSE,0,0);pump_until(GetTickCount64()+1000);failed|=IsWindow(root)!=0;
    UnhookWindowsHookEx(enter_hook);UnhookWindowsHookEx(return_hook);CoUninitialize();printf("Host idle repaint: %d failures\n",failed);return failed;
}
