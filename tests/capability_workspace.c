#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ui_framework/application.h"
#include "ui_framework/app_data.h"
#include "ui_framework/capability.h"
#ifdef UI_CAP_ENTRY_WORKSPACE
#include "ui_framework/capability_entry.h"
static uint64_t entry_call(ui_host_t *host,const char *id,ui_cap_target_t *target,const char *params)
{
    ui_cap_entry_binding_t b={0};uint64_t request,call=0;
    b.size=sizeof(b);b.version=1;b.capability_id=id;b.target=target;
    if(ui_capability_enable_read_entries(host)!=UI_STATUS_OK||ui_capability_publish_read_entry(host,&b)!=UI_STATUS_OK)return 0;
    request=ui_host_invoke(host,id,params,"C1-fixture");
    if(!request||ui_capability_request_call(host,request,&call)!=UI_STATUS_OK)return 0;
    return call;
}
#endif
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "capability_fixture_control.h"
static int failures,checks,events,sync_events,drain_worker,drain_callback;
static uint64_t drain_instance,late_call;
static ui_workspace_t *late_workspace;
static ui_status_t late_status;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);++failures;}}while(0)
#ifdef UI_CAP_ENTRY_WORKSPACE
static ui_workspace_t *entry_workspace;
static int entry_results;
static void entry_result(uint64_t instance,const ui_result_t *r,void *user)
{
    ui_app_instance_info_t info={0};uint64_t call=0;(void)user;info.size=sizeof(info);
    CHECK(ui_workspace_get_instance(entry_workspace,instance,&info)==UI_STATUS_OK);
    CHECK(ui_capability_request_call(info.host,r->request_id,&call)==UI_STATUS_OK&&call!=0);
    CHECK(ui_capability_forget(info.host,call)==UI_STATUS_PENDING);
    ++entry_results;ui_workspace_poll(entry_workspace); /* Never releases this callback's host. */
    CHECK(ui_workspace_get_instance(entry_workspace,instance,&info)==UI_STATUS_OK);
}
#endif
typedef struct post {ui_workspace_t *workspace;uint64_t instance,call;ui_status_t status;} post_t;
static DWORD WINAPI worker(void *data)
{post_t *p=data;p->status=ui_workspace_post_app_data(p->workspace,p->instance,1,&p->call,sizeof(p->call));return 0;}
static void event(uint64_t id,const char *name,const char *json,void *user)
{
    (void)user;
    if(!strcmp(name,"cap.complete")){++events;if(late_workspace)late_status=ui_workspace_post_app_data(late_workspace,id,CAP_FIXTURE_RESULT,&late_call,sizeof(late_call));}
    if(!strcmp(name,"cap.sync"))++sync_events;
    if(!strcmp(name,"cap.drain")){drain_instance=id;CHECK(sscanf_s(json,"{\"worker_done\":%d,\"callback_done\":%d}",&drain_worker,&drain_callback)==2);}
}
/* Only zero-time observations. The test UI keeps processing messages; no join
 * or blocking wait is performed on the UI thread, including the old EXE case. */
static void pump(void)
{MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}SwitchToThread();}
static void observe_signal(HANDLE handle)
{ULONGLONG end=GetTickCount64()+5000;while(WaitForSingleObject(handle,0)==WAIT_TIMEOUT&&GetTickCount64()<end)pump();CHECK(WaitForSingleObject(handle,0)==WAIT_OBJECT_0);}
static ui_cap_result_t read_result(ui_host_t *host,uint64_t id)
{ui_cap_result_t r={0};r.size=sizeof(r);r.version=1;CHECK(ui_capability_get_result(host,id,&r)==UI_STATUS_OK);return r;}
static void poll_call(ui_workspace_t *w,uint64_t instance,uint64_t call)
{CHECK(ui_workspace_post_app_data(w,instance,CAP_FIXTURE_POLL,&call,sizeof(call))==UI_STATUS_OK);ui_workspace_poll(w);pump();}
static void dll_worker_case(ui_workspace_t *w,const char *path,int fill)
{
    uint64_t instance=0,call=0,reopened=0,new_call=0;ui_app_instance_info_t info={0};ui_cap_target_t target={0};HANDLE gates[CAP_FIXTURE_GATE_COUNT]={0};int i,before=events;ULONGLONG end;wchar_t name[128];
    CHECK(ui_workspace_open(w,path,&instance)==UI_STATUS_OK);info.size=sizeof(info);CHECK(ui_workspace_get_instance(w,instance,&info)==UI_STATUS_OK);
    target.size=sizeof(target);target.version=1;CHECK(ui_capability_bind(info.host,"fixture.document",&target)==UI_STATUS_OK);
#ifdef UI_CAP_ENTRY_WORKSPACE
    call=entry_call(info.host,"fixture.worker",&target,fill?"{\"fill\":true}":"{\"fill\":false}");CHECK(call!=0);
    CHECK(ui_capability_dispatch(info.host,call)==UI_STATUS_OK); /* Duplicate never restarts worker. */
#else
    CHECK(ui_capability_prepare(info.host,"fixture.worker",&target,fill?"{\"fill\":true}":"{\"fill\":false}",&call)==UI_STATUS_OK);
    CHECK(ui_capability_dispatch(info.host,call)==UI_STATUS_OK);
#endif
    for(i=0;i<CAP_FIXTURE_GATE_COUNT;++i){cap_fixture_gate_name(name,instance,i);gates[i]=OpenEventW(EVENT_MODIFY_STATE|SYNCHRONIZE,FALSE,name);CHECK(gates[i]!=NULL);}
    observe_signal(gates[CAP_FIXTURE_WORKER_STARTED]);observe_signal(gates[CAP_FIXTURE_CALLBACK_STARTED]);
    if(fill)observe_signal(gates[CAP_FIXTURE_POST_LIMIT]);
    CHECK(read_result(info.host,call).state==UI_CAP_RUNNING);
    CHECK(ui_workspace_close(w,instance,UI_APP_CLOSE_TAB)==UI_STATUS_OK);
    if(fill){ui_status_t status=ui_workspace_flush(w,1);CHECK(status==UI_STATUS_OK||status==UI_STATUS_CANCELLED);}else ui_workspace_poll(w);
    CHECK(ui_workspace_count(w)==1&&GetModuleHandleW(L"ui_capability_fixture.dll")!=NULL);
    CHECK(read_result(info.host,call).state==UI_CAP_RUNNING);
    if(!fill){CHECK(SetEvent(gates[CAP_FIXTURE_POST]));observe_signal(gates[CAP_FIXTURE_POST_OK]);ui_workspace_poll(w);CHECK(read_result(info.host,call).state==UI_CAP_RUNNING);}
    else ui_workspace_poll(w); /* Drain the full queue, retaining the call. */
    CHECK(SetEvent(gates[CAP_FIXTURE_WORKER_EXIT]));drain_instance=0;drain_worker=drain_callback=0;end=GetTickCount64()+5000;
    do{poll_call(w,instance,call);}while((drain_instance!=instance||!drain_worker)&&GetTickCount64()<end);
    CHECK(drain_instance==instance&&drain_worker&&!drain_callback);CHECK(read_result(info.host,call).state==UI_CAP_RUNNING);
    CHECK(ui_workspace_count(w)==1&&GetModuleHandleW(L"ui_capability_fixture.dll")!=NULL);
    late_workspace=w;late_call=call;late_status=UI_STATUS_PENDING;
    CHECK(SetEvent(gates[CAP_FIXTURE_CALLBACK_EXIT]));end=GetTickCount64()+5000;
    do{poll_call(w,instance,call);}while(ui_workspace_count(w)&&!drain_callback&&GetTickCount64()<end);
    CHECK(drain_worker&&drain_callback);
    if(fill){
        CHECK(ui_workspace_count(w)==1&&read_result(info.host,call).state==UI_CAP_RUNNING);
        CHECK(ui_workspace_post_app_data(w,instance,CAP_FIXTURE_RETRY,&call,sizeof(call))==UI_STATUS_OK);ui_workspace_poll(w);
        CHECK(ui_workspace_count(w)==1&&read_result(info.host,call).state==UI_CAP_RUNNING);ui_workspace_poll(w);
    }
    late_workspace=NULL;
    CHECK(ui_workspace_count(w)==0&&events==before+1&&GetModuleHandleW(L"ui_capability_fixture.dll")==NULL);CHECK(late_status==UI_STATUS_OK);
    for(i=0;i<CAP_FIXTURE_GATE_COUNT;++i)if(gates[i])CloseHandle(gates[i]);
    CHECK(ui_workspace_post_app_data(w,instance,CAP_FIXTURE_RESULT,&call,sizeof(call))==UI_STATUS_CANCELLED);
    CHECK(ui_workspace_open(w,path,&reopened)==UI_STATUS_OK&&reopened!=instance);info.size=sizeof(info);CHECK(ui_workspace_get_instance(w,reopened,&info)==UI_STATUS_OK);
    target.size=sizeof(target);target.version=1;CHECK(ui_capability_bind(info.host,"fixture.document",&target)==UI_STATUS_OK);
    CHECK(ui_capability_prepare(info.host,"fixture.read",&target,"{}",&new_call)==UI_STATUS_OK&&new_call==call);
    CHECK(ui_workspace_post_app_data(w,instance,CAP_FIXTURE_RESULT,&call,sizeof(call))==UI_STATUS_CANCELLED);
    ui_workspace_poll(w);CHECK(read_result(info.host,new_call).state==UI_CAP_PREPARED&&events==before+1);
    CHECK(ui_capability_cancel(info.host,new_call)==UI_STATUS_OK);CHECK(ui_capability_forget(info.host,new_call)==UI_STATUS_OK);
    CHECK(ui_workspace_close_all(w)==UI_STATUS_OK);ui_workspace_poll(w);CHECK(!ui_workspace_count(w)&&GetModuleHandleW(L"ui_capability_fixture.dll")==NULL);
    printf("DLL worker/callback close and copied-queue case fill=%d checked\n",fill);
}
static void dll_sync_case(ui_workspace_t *w,const char *path)
{
    uint64_t instance=0,call=0;ui_app_instance_info_t info={0};ui_cap_target_t target={0};int before=sync_events;
    CHECK(ui_workspace_open(w,path,&instance)==UI_STATUS_OK);info.size=sizeof(info);CHECK(ui_workspace_get_instance(w,instance,&info)==UI_STATUS_OK);
    target.size=sizeof(target);target.version=1;CHECK(ui_capability_bind(info.host,"fixture.document",&target)==UI_STATUS_OK);
#ifdef UI_CAP_ENTRY_WORKSPACE
    call=entry_call(info.host,"fixture.sync",&target,"{}");CHECK(call!=0);
#else
    CHECK(ui_capability_prepare(info.host,"fixture.sync",&target,"{}",&call)==UI_STATUS_OK);CHECK(ui_capability_dispatch(info.host,call)==UI_STATUS_OK);
#endif
    CHECK(sync_events==before+1&&ui_workspace_count(w)==1&&read_result(info.host,call).state==UI_CAP_FINISHED);
    CHECK(ui_capability_finish_unregistration(info.host,"fixture.sync")==UI_STATUS_OK);
    CHECK(ui_capability_finish_resource_unregistration(info.host,"fixture.document")==UI_STATUS_OK);
    CHECK(ui_capability_forget(info.host,call)==UI_STATUS_OK);ui_workspace_poll(w);
    CHECK(ui_workspace_count(w)==0&&GetModuleHandleW(L"ui_capability_fixture.dll")==NULL);
}
int wmain(int argc,wchar_t **argv)
{
    ui_workspace_config_t config={0};ui_workspace_t *w;ui_app_instance_info_t a={0},b={0};
    HWND parent;char path[32768];uint64_t first=0,second=0,reopened=0,id=0;ui_cap_target_t target={0},old;
    ui_cap_result_t result={0};post_t post;HANDLE thread;DWORD start;
    if(argc!=2||!WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,argv[1],-1,path,sizeof(path),NULL,NULL))return 2;
    parent=CreateWindowExW(0,L"STATIC",L"Capability lifecycle",WS_OVERLAPPED,0,0,640,480,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(parent!=NULL);
    config.size=sizeof(config);config.native_parent=parent;config.event=event;
#ifdef UI_CAP_ENTRY_WORKSPACE
    config.result=entry_result;
#endif
    w=ui_workspace_create(&config);CHECK(w!=NULL);if(!w)return 1;
#ifdef UI_CAP_ENTRY_WORKSPACE
    entry_workspace=w;
#endif
    CHECK(ui_workspace_open(w,path,&first)==UI_STATUS_OK);CHECK(ui_workspace_open(w,path,&second)==UI_STATUS_OK);
    a.size=b.size=sizeof(a);CHECK(ui_workspace_get_instance(w,first,&a)==UI_STATUS_OK);CHECK(ui_workspace_get_instance(w,second,&b)==UI_STATUS_OK);if(!a.host||!b.host)return 1;
    target.size=sizeof(target);target.version=1;CHECK(ui_capability_bind(a.host,"fixture.document",&target)==UI_STATUS_OK&&target.instance_id==first);old=target;
    CHECK(ui_capability_prepare(b.host,"fixture.read",&target,"{}",&id)==UI_STATUS_NOT_FOUND);
#ifdef UI_CAP_ENTRY_WORKSPACE
    {ui_cap_entry_binding_t binding={0};uint64_t request=0;
     binding.size=sizeof(binding);binding.version=1;binding.capability_id="fixture.read";binding.target=&target;
     CHECK(ui_capability_enable_read_entries(a.host)==UI_STATUS_OK);CHECK(ui_capability_publish_read_entry(a.host,&binding)==UI_STATUS_OK);
     CHECK(ui_workspace_activate(w,second)==UI_STATUS_OK);
     CHECK(ui_assistant_invoke(a.assistant,"fixture.read","{}",&request)==UI_STATUS_OK);
     CHECK(ui_capability_request_call(a.host,request,&id)==UI_STATUS_OK);}
#else
    CHECK(ui_capability_prepare(a.host,"fixture.read",&target,"{}",&id)==UI_STATUS_OK);CHECK(ui_workspace_activate(w,second)==UI_STATUS_OK);
#endif
    CHECK(ui_capability_dispatch(a.host,id)==UI_STATUS_OK);post.workspace=w;post.instance=first;post.call=id;post.status=UI_STATUS_PENDING;
    thread=CreateThread(NULL,0,worker,&post,0,NULL);CHECK(thread!=NULL);if(thread){observe_signal(thread);CloseHandle(thread);}CHECK(post.status==UI_STATUS_OK);
    ui_workspace_poll(w);result.size=sizeof(result);result.version=1;CHECK(ui_capability_get_result(a.host,id,&result)==UI_STATUS_OK&&result.state==UI_CAP_FINISHED&&result.outcome==UI_CAP_OK);CHECK(events==1);
#ifdef UI_CAP_ENTRY_WORKSPACE
    CHECK(entry_results==1);ui_workspace_poll(w);CHECK(ui_capability_get_result(a.host,id,&result)==UI_STATUS_NOT_FOUND);
#else
    CHECK(ui_capability_forget(a.host,id)==UI_STATUS_OK);
#endif
    /* WAIT followed by REFUSE reopens admission, without resurrecting calls. */
    CHECK(SetEnvironmentVariableA("UI_CAP_FIXTURE_CLOSE_WAIT","1"));
    CHECK(ui_workspace_close(w,first,UI_APP_CLOSE_TAB)==UI_STATUS_OK);
    CHECK(ui_workspace_post_close_complete(w,first,UI_APP_CLOSE_REFUSE)==UI_STATUS_OK);
    ui_workspace_poll(w);CHECK(ui_workspace_count(w)==2);
    CHECK(SetEnvironmentVariableA("UI_CAP_FIXTURE_CLOSE_WAIT",NULL));
    CHECK(ui_capability_bind(a.host,"fixture.document",&target)==UI_STATUS_OK);
    /* Close ALLOW is still gated by the framework's outstanding callback lease. */
    CHECK(ui_capability_prepare(a.host,"fixture.read",&target,"{}",&id)==UI_STATUS_OK);CHECK(ui_capability_dispatch(a.host,id)==UI_STATUS_OK);
    CHECK(ui_workspace_close(w,first,UI_APP_CLOSE_TAB)==UI_STATUS_OK);ui_workspace_poll(w);CHECK(ui_workspace_count(w)==2);
    post.call=id;thread=CreateThread(NULL,0,worker,&post,0,NULL);CHECK(thread!=NULL);if(thread){observe_signal(thread);CloseHandle(thread);}CHECK(post.status==UI_STATUS_OK);
    ui_workspace_poll(w);CHECK(ui_workspace_count(w)==1&&events==2);
    CHECK(ui_workspace_close(w,second,UI_APP_CLOSE_TAB)==UI_STATUS_OK);ui_workspace_poll(w);CHECK(ui_workspace_count(w)==0);CHECK(GetModuleHandleW(L"ui_capability_fixture.dll")==NULL);
    CHECK(ui_workspace_post_app_data(w,first,1,&id,sizeof(id))==UI_STATUS_CANCELLED);
    CHECK(ui_workspace_open(w,path,&reopened)==UI_STATUS_OK&&reopened>second);a.size=sizeof(a);CHECK(ui_workspace_get_instance(w,reopened,&a)==UI_STATUS_OK);
    CHECK(ui_capability_prepare(a.host,"fixture.read",&old,"{}",&id)==UI_STATUS_NOT_FOUND);
    CHECK(ui_workspace_close_all(w)==UI_STATUS_OK);start=GetTickCount();while(ui_workspace_count(w)&&GetTickCount()-start<5000)ui_workspace_poll(w);
    dll_worker_case(w,path,0);dll_worker_case(w,path,1);dll_sync_case(w,path);
    CHECK(ui_workspace_destroy(w)==UI_STATUS_OK);DestroyWindow(parent);CHECK(GetModuleHandleW(L"ui_capability_fixture.dll")==NULL);
    printf("capability workspace: %d checks, %d failures\n",checks,failures);return failures?1:0;
}
