/* PB-1 public production DLL/read proof; no model or private input. */
#define wmain page_body_prior_main
#include "browser_capability.c"
#undef wmain
#include "yyjson.h"
#ifndef PB_PUBLIC_TEXT
#define PB_PUBLIC_TEXT "设备巡检交接单\n设备总数：12台。\n已完成：9台；待处理：3台。\n负责人：林青 & 周明。\n下一步\n复核三台设备。"
#define PB_PUBLIC_TITLE "PB-1 合成交接单"
#define PB_PUBLIC_BODY_MARKER "设备总数"
#endif
static const char expected_body[]=PB_PUBLIC_TEXT;
int wmain(int argc,wchar_t **argv)
{
 if(argc!=3&&argc!=4)return 2;int empty=argc==4&&!wcscmp(argv[3],L"empty"),over=argc==4&&!wcscmp(argv[3],L"over");const char *expected=empty?"":expected_body;wchar_t settings[4096];CHECK(CreateDirectoryW(argv[2],NULL)||GetLastError()==ERROR_ALREADY_EXISTS);
 _snwprintf_s(settings,4096,_TRUNCATE,L"%s/body-%lu.txt",argv[2],GetCurrentProcessId());HANDLE f=CreateFileW(settings,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);CHECK(f!=INVALID_HANDLE_VALUE);if(f!=INVALID_HANDLE_VALUE)CloseHandle(f);
 CHECK(SetEnvironmentVariableW(L"UI_BROWSER_SETTINGS_PATH",settings));CHECK(ui_framework_initialize()==UI_STATUS_OK);
 HWND parent=CreateWindowExW(0,L"STATIC",L"PB-1 public",WS_OVERLAPPEDWINDOW,-16000,-16000,1000,800,NULL,NULL,GetModuleHandleW(NULL),NULL);
 ui_workspace_config_t c={0};c.size=sizeof(c);c.run_mode=UI_RUN_WINDOWED;c.native_parent=parent;c.max_permission=UI_ASSISTANT_PERMISSION_READ;w=ui_workspace_create(&c);CHECK(w!=NULL);ui_rect_t rect={0,0,1000,800};CHECK(ui_workspace_set_rect(w,&rect,96)==UI_STATUS_OK);
 char path[4096];CHECK(WideCharToMultiByte(CP_UTF8,0,argv[1],-1,path,sizeof(path),NULL,NULL)>0);uint64_t first=0;ui_status_t opened=ui_workspace_open(w,path,&first);CHECK(opened==UI_STATUS_OK);if(opened){fprintf(stderr,"body public open status=%d detail=%s\n",opened,ui_workspace_last_error(w));CHECK(ui_workspace_destroy(w)==UI_STATUS_OK);DestroyWindow(parent);SetEnvironmentVariableW(L"UI_BROWSER_SETTINGS_PATH",NULL);return 1;}a.size=sizeof(a);CHECK(ui_workspace_get_instance(w,first,&a)==UI_STATUS_OK);CHECK(read_state(&a,PB_PUBLIC_TITLE));
 ui_cap_target_t t={0};t.size=sizeof(t);t.version=1;CHECK(ui_capability_bind(a.host,"browser.view",&t)==UI_STATUS_OK);uint64_t call=0;ui_status_t status=ui_capability_prepare(a.host,"browser.body",&t,"{}",&call);CHECK(status==UI_STATUS_OK);
 if(!status){CHECK(ui_capability_dispatch(a.host,call)==UI_STATUS_OK);ui_cap_result_t r={0};r.size=sizeof(r);r.version=1;CHECK(ui_capability_get_result(a.host,call,&r)==UI_STATUS_OK&&r.outcome==(over?UI_CAP_REJECTED:UI_CAP_OK));if(over){CHECK(!r.output_json&&!strcmp(r.code,"unavailable"));}else{yyjson_doc *d=r.output_json?yyjson_read(r.output_json,strlen(r.output_json),0):NULL;yyjson_val *o=yyjson_doc_get_root(d),*body=yyjson_obj_get(o,"body");const char *text=yyjson_get_str(yyjson_obj_get(body,"text"));CHECK(text&&!strcmp(text,expected));CHECK(yyjson_get_uint(yyjson_obj_get(body,"bytes"))==strlen(expected));CHECK(yyjson_is_true(yyjson_obj_get(body,"complete")));const char *semantics=yyjson_get_str(yyjson_obj_get(body,"semantics"));CHECK(semantics&&!strcmp(semantics,"dom-body-v1"));const char *revision=yyjson_get_str(yyjson_obj_get(o,"revision"));CHECK(revision&&!strcmp(revision,t.expected_revision));CHECK(!strstr(state,PB_PUBLIC_BODY_MARKER));fprintf(stderr,"public body %s\n",r.output_json?r.output_json:"");yyjson_doc_free(d);}CHECK(ui_capability_forget(a.host,call)==UI_STATUS_OK);}
 ui_cap_target_t wrong=t;uint64_t denied=0;strcpy_s(wrong.scope_json,sizeof(wrong.scope_json),"{\"access\":\"write\"}");CHECK(ui_capability_prepare(a.host,"browser.body",&wrong,"{}",&denied)==UI_STATUS_PERMISSION_DENIED&&!denied);wrong=t;wrong.host_epoch[0]=wrong.host_epoch[0]=='a'?'b':'a';CHECK(ui_capability_prepare(a.host,"browser.body",&wrong,"{}",&denied)==UI_STATUS_NOT_FOUND&&!denied);wrong=t;wrong.open_epoch[0]=wrong.open_epoch[0]=='a'?'b':'a';CHECK(ui_capability_prepare(a.host,"browser.body",&wrong,"{}",&denied)==UI_STATUS_NOT_FOUND&&!denied);CHECK(ui_capability_prepare(a.host,"browser.body",&t,"{\"other_page\":1}",&denied)==UI_STATUS_VALIDATION_FAILED&&!denied);
 CHECK(ui_workspace_close_all(w)==UI_STATUS_OK);DWORD begin=GetTickCount();while(ui_workspace_count(w)&&GetTickCount()-begin<5000)pump();CHECK(!ui_workspace_count(w));CHECK(ui_workspace_destroy(w)==UI_STATUS_OK);DestroyWindow(parent);SetEnvironmentVariableW(L"UI_BROWSER_SETTINGS_PATH",NULL);
 printf("PB-1 public checks=%u failures=%u\n",checks,failures);return failures?1:0;
}
