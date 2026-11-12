#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "ui_framework/application.h"
#include "ui_framework/capability_entry.h"
#include "ui_framework/capability_v2.h"
static unsigned checks,failures;
#define CHECK(x) do{++checks;if(!(x)){++failures;fprintf(stderr,"line %d: %s\n",__LINE__,#x);}}while(0)
static void pump(ui_workspace_t *w)
{MSG m;unsigned n=0;while(n++<100&&PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}ui_workspace_poll(w);SwitchToThread();}
static int read_call(ui_host_t *host,const char *resource,const char *cap,const char *params,const char *expected)
{ui_cap_target_t target={0};ui_cap_result_t result={0};uint64_t call=0;ui_status_t status;target.size=sizeof(target);target.version=1;status=ui_capability_bind(host,resource,&target);CHECK(status==UI_STATUS_OK);if(status)return 0;
 status=ui_capability_prepare(host,cap,&target,params,&call);CHECK(status==UI_STATUS_OK);if(status)return 0;CHECK(ui_capability_dispatch(host,call)==UI_STATUS_OK);result.size=sizeof(result);result.version=1;
 CHECK(ui_capability_get_result(host,call,&result)==UI_STATUS_OK);CHECK(result.outcome==UI_CAP_OK&&result.output_json&&strstr(result.output_json,expected));CHECK(ui_capability_dispatch(host,call)==UI_STATUS_OK);CHECK(ui_capability_forget(host,call)==UI_STATUS_OK);return 1;}
static void reopen(ui_workspace_t *w,const char *package,const char *resource,const char *read,const char *expected,const ui_cap_target_t *old)
{
 uint64_t instance=0,call=0;ui_app_instance_info_t info={0};ui_cap_target_t fresh={0};DWORD start;
 CHECK(ui_workspace_open(w,package,&instance)==UI_STATUS_OK);info.size=sizeof(info);CHECK(ui_workspace_get_instance(w,instance,&info)==UI_STATUS_OK);if(!info.host)return;
 fresh.size=sizeof(fresh);fresh.version=1;CHECK(ui_capability_bind(info.host,resource,&fresh)==UI_STATUS_OK);
 CHECK(fresh.instance_id!=old->instance_id&&!strcmp(fresh.host_epoch,old->host_epoch)&&strcmp(fresh.open_epoch,old->open_epoch));
 {ui_cap_target_t stale=fresh;strcpy_s(stale.open_epoch,sizeof(stale.open_epoch),old->open_epoch);CHECK(ui_capability_prepare(info.host,read,&stale,"{}",&call)==UI_STATUS_NOT_FOUND&&call==0);}
 CHECK(ui_capability_prepare(info.host,read,old,"{}",&call)==UI_STATUS_NOT_FOUND&&call==0);CHECK(read_call(info.host,resource,read,"{}",expected));
 printf("SIM reopen %s old=%llu new=%llu old epoch rejected\n",read,(unsigned long long)old->instance_id,(unsigned long long)instance);
 CHECK(ui_workspace_close(w,instance,UI_APP_CLOSE_TAB)==UI_STATUS_OK);start=GetTickCount();while(ui_workspace_count(w)&&GetTickCount()-start<5000)pump(w);CHECK(ui_workspace_count(w)==0);
}
static void maintained(const wchar_t *path,const char *resource,const char *read,const char *expected,const char *write,const wchar_t *module,int offscreen)
{
 ui_workspace_config_t config={0};ui_workspace_t *w;HWND parent=NULL;char package[32768];uint64_t instances[2]={0};ui_app_instance_info_t info[2]={{0}};ui_cap_target_t old={0};DWORD start;
 CHECK(WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,path,-1,package,sizeof(package),NULL,NULL)>0);
 if(!offscreen){parent=CreateWindowExW(0,L"STATIC",L"P2 hidden real workspace",WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,-16000,-16000,1100,800,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(parent!=NULL);}
 config.size=sizeof(config);config.native_parent=parent;config.run_mode=offscreen?UI_RUN_OFFSCREEN:UI_RUN_WINDOWED;w=ui_workspace_create(&config);CHECK(w!=NULL);if(!w){if(parent)DestroyWindow(parent);return;}
 CHECK(ui_workspace_set_rect(w,&(ui_rect_t){0,0,1100,800},96)==UI_STATUS_OK);
 for(unsigned i=0;i<2;++i){ui_status_t status=ui_workspace_open(w,package,&instances[i]);if(status)fprintf(stderr,"P2 DLL open %s status=%d %s\n",read,status,ui_workspace_last_error(w));CHECK(status==UI_STATUS_OK);info[i].size=sizeof(info[i]);CHECK(ui_workspace_get_instance(w,instances[i],&info[i])==UI_STATUS_OK);}
 if(info[0].host&&info[1].host){ui_cap_target_t fixed={0};uint64_t call=0;ui_cap_result_t result={0};fixed.size=sizeof(fixed);fixed.version=1;pump(w);
   CHECK(read_call(info[0].host,resource,read,"{}",expected));CHECK(ui_capability_bind(info[0].host,resource,&fixed)==UI_STATUS_OK);CHECK(ui_capability_prepare(info[0].host,read,&fixed,"{}",&call)==UI_STATUS_OK);
   CHECK(ui_workspace_activate(w,instances[1])==UI_STATUS_OK&&ui_workspace_active(w)==instances[1]);CHECK(ui_capability_dispatch(info[0].host,call)==UI_STATUS_OK);result.size=sizeof(result);result.version=1;CHECK(ui_capability_get_result(info[0].host,call,&result)==UI_STATUS_OK&&result.outcome==UI_CAP_OK);CHECK(ui_capability_forget(info[0].host,call)==UI_STATUS_OK);
   (void)ui_host_invoke(info[0].host,write,"{}","human");CHECK(read_call(info[0].host,resource,read,"{}",expected));CHECK(read_call(info[1].host,resource,read,"{}",expected));
   old=fixed;{char *catalog=NULL;uint64_t generation=0;CHECK(ui_capability_discover_v2(info[0].host,NULL,0,32,&generation,&catalog)==UI_STATUS_OK&&catalog&&strstr(catalog,write));ui_capability_release(catalog);}}
 for(unsigned i=0;i<2;++i)if(instances[i])CHECK(ui_workspace_close(w,instances[i],UI_APP_CLOSE_TAB)==UI_STATUS_OK);start=GetTickCount();while(ui_workspace_count(w)&&GetTickCount()-start<5000)pump(w);CHECK(ui_workspace_count(w)==0);pump(w);CHECK(GetModuleHandleW(module)==NULL);if(old.instance_id)reopen(w,package,resource,read,expected,&old);CHECK(ui_workspace_destroy(w)==UI_STATUS_OK);if(parent)DestroyWindow(parent);
}
int wmain(int argc,wchar_t **argv)
{
 ui_workspace_config_t config={0};ui_workspace_t *w;uint64_t ids[2]={0};ui_app_instance_info_t info[2]={{0}};ui_cap_target_t old={0};char package[32768];DWORD start;
 if(argc!=8)return 2;if(!WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,argv[1],-1,package,sizeof(package),NULL,NULL))return 2;
 SetEnvironmentVariableW(L"UI_API7_OSMESA_DLL",argv[2]);SetEnvironmentVariableW(L"UI_STATE_EXAMPLE_HOME",argv[3]);SetEnvironmentVariableW(L"UI_STATE_EXAMPLE_PROFILE",NULL);SetEnvironmentVariableW(L"UI_API7_BACKEND",L"light");
 config.size=sizeof(config);config.run_mode=UI_RUN_OFFSCREEN;w=ui_workspace_create(&config);CHECK(w!=NULL);if(!w)return 1;
 for(unsigned i=0;i<2;++i){ui_status_t status=ui_workspace_open(w,package,&ids[i]);if(status)fprintf(stderr,"stateful DLL open=%d %s\n",status,ui_workspace_last_error(w));CHECK(status==UI_STATUS_OK);info[i].size=sizeof(info[i]);CHECK(ui_workspace_get_instance(w,ids[i],&info[i])==UI_STATUS_OK);}
 if(!info[0].host||!info[1].host)return 1;
 pump(w);CHECK(ui_workspace_count(w)==2);CHECK(ui_workspace_active(w)==ids[1]);old.size=sizeof(old);old.version=1;CHECK(ui_capability_bind(info[0].host,"state.document",&old)==UI_STATUS_OK);
 CHECK(read_call(info[0].host,"state.document","state.document.read","{}","\"name\":\"Untitled\""));CHECK(read_call(info[1].host,"state.document","state.cell.read","{\"id\":\"9007199254741000\",\"column\":\"value\"}","\"text\":\"42\""));
 {char *catalog=NULL;uint64_t generation=0;CHECK(ui_capability_discover_v2(info[0].host,NULL,0,32,&generation,&catalog)==UI_STATUS_OK);CHECK(catalog&&strstr(catalog,"state.save")&&strstr(catalog,"test.late")&&strstr(catalog,"language.zh"));ui_capability_release(catalog);}
 CHECK(ui_workspace_activate(w,ids[0])==UI_STATUS_OK);CHECK(ui_workspace_active(w)==ids[0]);CHECK(read_call(info[1].host,"state.document","state.document.read","{}","\"row_count\":100000"));
 (void)ui_host_invoke(info[0].host,"test.edit","{}","human"); /* Next read proves no business effect. */
 CHECK(read_call(info[0].host,"state.document","state.document.read","{}","\"changed_cells\":0"));
 for(unsigned i=0;i<2;++i)CHECK(ui_workspace_close(w,ids[i],UI_APP_CLOSE_TAB)==UI_STATUS_OK);start=GetTickCount();while(ui_workspace_count(w)&&GetTickCount()-start<5000)pump(w);CHECK(ui_workspace_count(w)==0);CHECK(GetModuleHandleW(L"stateful_components_app.dll")==NULL);reopen(w,package,"state.document","state.document.read","\"name\":\"Untitled\"",&old);CHECK(ui_workspace_destroy(w)==UI_STATUS_OK);
 maintained(argv[4],"eda.document","eda.document.read","\"objects\":[]","eda.add_block",L"minimal_eda_app.dll",0);
 maintained(argv[5],"demo.document","demo.document.read","\"changes\":[]","demo.edit",L"generic_components_app.dll",0);
 maintained(argv[6],"demo.document","demo.document.read","\"changes\":[]","demo.edit",L"framework_features_app.dll",0);
 SetEnvironmentVariableW(L"UI_FEATURES_NO_GL",L"1");maintained(argv[6],"demo.document","demo.document.read","\"changes\":[]","demo.edit",L"framework_features_app.dll",1);SetEnvironmentVariableW(L"UI_FEATURES_NO_GL",NULL);
 maintained(argv[7],"counter.document","counter.read","\"count\":0","counter.increment",L"web_counter_app.dll",0);
 printf("P2 five real DLLs/two shared branches/fixed instances/lifecycle (not physical): checks=%u failures=%u\n",checks,failures);return failures?1:0;
}

