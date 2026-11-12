/* One qualified batch click, original production Service, no auto approval hook. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui_framework/ai_validation_batch.h"
#include "ui_framework/ai_model_ui.h"
#include "yyjson.h"
static ui_workspace_t *workspace;static ui_ai_service_t *service;
static HWND display,start_button,key_button;static uint64_t start_binding,key_binding,instance;
static int opened,failed,exit_result,closing,ticking;static wchar_t *snapshot;
static char shown_hash[65],bound_hash[65];
static int projection_error_logged;
static int startup_diagnostic(const char *,const char *,long long,int);
static const char *str(yyjson_val *o,const char *name){const char *p=yyjson_get_str(yyjson_obj_get(o,name));return p?p:"";}
static int project(const char *state,const char *model,const char *policy)
{
 int ok=0;char *pretty=NULL,*text=NULL;wchar_t *wide=NULL;yyjson_doc *batch=NULL,*parsed=NULL;char verified[65];
 if(!IsWindow(display)||closing){shown_hash[0]=0;EnableWindow(start_button,FALSE);return 0;}
 batch=yyjson_read(state,strlen(state),0);yyjson_val *root=batch?yyjson_doc_get_root(batch):NULL;const char *preview=str(root,"preview"),*hash=str(root,"batch_hash");
 if(strlen(hash)!=64||!*preview||ui_cap_json_fingerprint(preview,strlen(preview),verified)!=UI_STATUS_OK||strcmp(hash,verified))goto done;
 parsed=yyjson_read(preview,strlen(preview),0);if(!parsed||!yyjson_is_obj(yyjson_doc_get_root(parsed)))goto done;
 pretty=yyjson_write(parsed,YYJSON_WRITE_PRETTY,NULL);if(!pretty)goto done;
 size_t n=strlen(state)+strlen(model)+strlen(policy)+strlen(pretty)+1600;text=malloc(n);if(!text)goto done;
 int written=snprintf(text,n,"受控T2/T3批次：T1禁用，总POST最多4（T2≤3/T3≤1），无重试。\r\nT2：先发现memory.read，再回读整数10、r1并保存；T3：无目标合成解释，在真实在途时尝试取消。\r\n只向DeepSeek发送下列合成模板、系统指令、两个read工具定义及固定目标必要结果；history=[]，无业务文件、其他应用或AI写入。\r\n指定引用后台本地检查；请核对下面完整范围后，只点击一次“开始受控补测”。\r\n完成/失败自动保存脱敏证据并安全关闭，无需复制JSON或手动关窗。\r\nUSD0.10仅本地停止阈值，未知费用即停止；首次确认起10分钟；取消不保证远端停止或免除费用。\r\n完整批次确认范围：\r\n%s\r\n执行状态：%s\r\n模型/凭据：%s\r\n计数及费用：%s",pretty,state,model,policy);
 if(written<0||(size_t)written>=n)goto done;
 int count=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,text,-1,NULL,0);if(count<=0)goto done;wide=malloc((size_t)count*sizeof(wchar_t));if(!wide||MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,text,-1,wide,count)!=count)goto done;
 if(!snapshot||wcscmp(snapshot,wide)||strcmp(shown_hash,hash)){EnableWindow(start_button,FALSE);shown_hash[0]=0;DWORD first=0,last=0;SendMessageW(display,EM_GETSEL,(WPARAM)&first,(LPARAM)&last);LRESULT line=SendMessageW(display,EM_GETFIRSTVISIBLELINE,0,0);
  if(!SetWindowTextW(display,wide)||!IsWindow(display)||GetWindowTextLengthW(display)!=count-1||closing)goto done;
  SendMessageW(display,EM_SETSEL,first,last);SendMessageW(display,EM_LINESCROLL,0,line-SendMessageW(display,EM_GETFIRSTVISIBLELINE,0,0));free(snapshot);snapshot=wide;wide=NULL;
 }
 if(!closing&&IsWindow(display)&&GetWindowTextLengthW(display)==count-1){strcpy_s(shown_hash,65,hash);ok=1;}
done:if(!ok){shown_hash[0]=0;EnableWindow(start_button,FALSE);}free(wide);free(text);free(pretty);yyjson_doc_free(parsed);yyjson_doc_free(batch);return ok;
}
static void tick_impl(HWND window,const char *config)
{
 if(!service){if(workspace){ui_workspace_close_all(workspace);ui_workspace_poll(workspace);if(ui_workspace_destroy(workspace)!=UI_STATUS_OK)return;workspace=NULL;}DestroyWindow(window);return;}
 ui_ai_service_poll(service,64);ui_workspace_poll(workspace);ui_ai_service_state_t current={0};current.size=sizeof(current);current.version=1;ui_ai_service_get_state(service,&current);
 if(!opened&&current.phase==UI_AI_READY){ui_app_instance_info_t app={0};app.size=sizeof(app);ui_cap_target_t target={0};target.size=sizeof(target);target.version=1;ui_status_t status=ui_workspace_get_instance(workspace,instance,&app);if(!status)status=ui_capability_bind(app.host,"memory.document",&target);ui_ai_validation_batch_desc_t d={sizeof(d),1,config,&target};if(!status)status=ui_ai_validation_batch_open(service,&d);if(status){failed=1;ui_ai_service_request_close(service);}opened=1;}
 char *batch=NULL,*model=NULL,*policy=NULL;ui_ai_validation_batch_get_state(service,&batch);ui_ai_model_get_state(service,&model);ui_ai_validation_get_state(service,&policy);
 if(batch){yyjson_doc *d=yyjson_read(batch,strlen(batch),0);yyjson_val *o=d?yyjson_doc_get_root(d):NULL;int ready=yyjson_get_bool(yyjson_obj_get(o,"ready")),key=yyjson_get_bool(yyjson_obj_get(o,"need_key"));int offered_ready=ready;int projected=project(batch,model?model:"pending",policy?policy:"pending");if(offered_ready&&!projected&&!closing&&!projection_error_logged){startup_diagnostic("preview_display","launcher",0,0);projection_error_logged=1;}if(projected)projection_error_logged=0;ready=ready&&projected&&!closing&&!strcmp(shown_hash,str(o,"batch_hash"));if(start_binding&&strcmp(bound_hash,str(o,"batch_hash"))){if(ui_ai_model_unbind(service,start_binding)==UI_STATUS_OK){start_binding=0;bound_hash[0]=0;}else ready=0;}if(ready&&!start_binding){ui_ai_validation_batch_button_t binding={sizeof(binding),1,start_button,str(o,"batch_hash")};ui_status_t st=ui_ai_validation_batch_bind_button(service,&binding,&start_binding);if(st==UI_STATUS_OK)strcpy_s(bound_hash,65,str(o,"batch_hash"));if(st!=UI_STATUS_OK&&st!=UI_STATUS_PENDING){failed=1;ui_ai_validation_stop(service);}}
  EnableWindow(start_button,ready&&start_binding);SetWindowTextW(window,ready&&start_binding?L"玄枢受控T2/T3（可开始）":offered_ready&&!projected?L"玄枢受控T2/T3（预览失败，禁止开始）":key?L"玄枢受控T2/T3（需原生录入Key）":L"玄枢受控T2/T3（自动准备/执行/收尾）");ShowWindow(key_button,key?SW_SHOW:SW_HIDE);EnableWindow(key_button,key);
  if(key&&model&&!key_binding){yyjson_doc *m=yyjson_read(model,strlen(model),0);ui_ai_model_button_binding_t binding={0};binding.size=sizeof(binding);binding.version=1;binding.button=key_button;binding.binding_hash=str(m?yyjson_doc_get_root(m):NULL,"binding_hash");binding.action=UI_AI_MODEL_KEY;(void)ui_ai_model_bind_button(service,&binding,&key_binding);yyjson_doc_free(m);}
  if(yyjson_get_bool(yyjson_obj_get(o,"export_done"))){exit_result=yyjson_get_bool(yyjson_obj_get(o,"export_ok"))?0:3;if(*str(o,"error")||yyjson_get_bool(yyjson_obj_get(o,"cancel_window_missed")))failed=1;}yyjson_doc_free(d);
 }
 if(!batch){shown_hash[0]=0;EnableWindow(start_button,FALSE);}ui_ai_release(batch);ui_ai_release(model);ui_ai_release(policy);
 if(current.phase==UI_AI_UNAVAILABLE&&!opened){failed=1;ui_ai_service_request_close(service);}
 if(current.phase==UI_AI_CLOSED&&ui_ai_service_destroy(service)==UI_STATUS_OK){service=NULL;}
}
static void tick(HWND window,const char *config)
{if(ticking)return;ticking=1;tick_impl(window,config);ticking=0;}
static char profile[4096];
static LRESULT CALLBACK proc(HWND w,UINT message,WPARAM wp,LPARAM lp)
{
 if(message==WM_TIMER){tick(w,profile);return 0;}
 if(message==WM_COMMAND&&LOWORD(wp)==103&&service){closing=1;EnableWindow(start_button,FALSE);if(ui_ai_validation_stop(service)==UI_STATUS_NOT_FOUND)ui_ai_service_request_close(service);return 0;}
 /* Start/Key commands are intentionally ignored. Only framework input leases act. */
 if(message==WM_COMMAND)return 0;
 if(message==WM_CLOSE){closing=1;EnableWindow(start_button,FALSE);if(service){if(ui_ai_validation_stop(service)==UI_STATUS_NOT_FOUND)ui_ai_service_request_close(service);}else DestroyWindow(w);return 0;}
 if(message==WM_DESTROY){free(snapshot);snapshot=NULL;PostQuitMessage(failed?2:exit_result);return 0;}return DefWindowProcW(w,message,wp,lp);
}
static int startup_diagnostic(const char *stage,const char *domain,long long code,int known)
{fprintf(stderr,"{\"startup_stage\":\"%s\",\"error_domain\":\"%s\",\"code_known\":%s,\"code\":",stage,domain,known?"true":"false");if(known)fprintf(stderr,"%lld",code);else fputs("null",stderr);fputs("}\n",stderr);fflush(stderr);return 2;}
static int startup_error(const char *stage,DWORD error){return startup_diagnostic(stage,"win32",error,error!=ERROR_SUCCESS);}

int wmain(int argc,wchar_t **argv)
{
 int check=argc==5&&!wcscmp(argv[4],L"--startup-check");if(argc!=4&&!check)return startup_error("arguments",ERROR_INVALID_PARAMETER);
 wchar_t root[3500],dbwide[3600];DWORD length=GetFullPathNameW(argv[1],3500,root,NULL);if(!length)return startup_error("resolve_session",GetLastError());if(length>=3500)return startup_error("session_path_length",ERROR_FILENAME_EXCED_RANGE);if(!CreateDirectoryW(root,NULL))return startup_error("create_session",GetLastError());swprintf_s(dbwide,3600,L"%s/conversation.sqlite",root);char db[4096],package[4096];if(!WideCharToMultiByte(CP_UTF8,0,dbwide,-1,db,4096,NULL,NULL)||!WideCharToMultiByte(CP_UTF8,0,argv[2],-1,profile,4096,NULL,NULL)||!WideCharToMultiByte(CP_UTF8,0,argv[3],-1,package,4096,NULL,NULL))return startup_error("utf8_paths",GetLastError());
 WNDCLASSW c={0};c.lpfnWndProc=proc;c.hInstance=GetModuleHandleW(NULL);c.lpszClassName=L"XuanShuRpcBatch";c.hCursor=LoadCursorW(NULL,MAKEINTRESOURCEW(32512));if(!RegisterClassW(&c))return startup_error("register_window",GetLastError());HWND window=CreateWindowW(c.lpszClassName,L"玄枢：一次确认的受控T2/T3补测",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,1050,800,NULL,NULL,c.hInstance,NULL);if(!window)return startup_error("create_window",GetLastError());
 start_button=CreateWindowW(L"BUTTON",L"开始受控补测",WS_CHILD|WS_VISIBLE|WS_TABSTOP,12,12,180,36,window,(HMENU)101,c.hInstance,NULL);if(!start_button){DWORD e=GetLastError();startup_error("control_start",e);DestroyWindow(window);return 2;}key_button=CreateWindowW(L"BUTTON",L"设置DeepSeek API Key",WS_CHILD|WS_TABSTOP,210,12,220,36,window,(HMENU)102,c.hInstance,NULL);if(!key_button){DWORD e=GetLastError();startup_error("control_key",e);DestroyWindow(window);return 2;}HWND stop_button=CreateWindowW(L"BUTTON",L"停止并安全退出",WS_CHILD|WS_VISIBLE,450,12,180,36,window,(HMENU)103,c.hInstance,NULL);if(!stop_button){DWORD e=GetLastError();startup_error("control_stop",e);DestroyWindow(window);return 2;}display=CreateWindowW(L"EDIT",L"正在后台检查指定凭据，请稍候。",WS_CHILD|WS_VISIBLE|WS_VSCROLL|ES_MULTILINE|ES_READONLY,12,60,1000,690,window,NULL,c.hInstance,NULL);if(!display){DWORD e=GetLastError();startup_error("control_preview",e);DestroyWindow(window);return 2;}EnableWindow(start_button,FALSE);
 if(check){ui_status_t initialized=ui_framework_initialize();if(initialized){DestroyWindow(window);return startup_diagnostic("framework_initialize","ui_status",(long long)initialized,1);}DestroyWindow(window);printf("{\"startup_check\":true,\"credential_reads\":0,\"service_created\":false,\"sends\":0}\n");return 0;}
 if(!SetTimer(window,1,15,NULL)){DWORD e=GetLastError();startup_error("timer",e);DestroyWindow(window);return 2;}ui_status_t initialized=ui_framework_initialize();if(initialized){startup_diagnostic("framework_initialize","ui_status",(long long)initialized,1);DestroyWindow(window);return 2;}ui_workspace_config_t w={0};w.size=sizeof(w);w.run_mode=UI_RUN_OFFSCREEN;workspace=ui_workspace_create(&w);if(!workspace){startup_diagnostic("workspace_create","ui_status",0,0);failed=1;}else{ui_status_t status=ui_workspace_open(workspace,package,&instance);if(status){startup_diagnostic("workspace_open","ui_status",(long long)status,1);failed=1;}}ui_ai_service_config_t s={0};s.size=sizeof(s);s.version=1;s.workspace=workspace;s.storage_path_utf8=db;if(!failed){ui_status_t status=ui_ai_service_create(&s,&service);if(status)startup_diagnostic("service_create","ui_status",(long long)status,1);else{status=ui_ai_service_open(service);if(status)startup_diagnostic("service_open","ui_status",(long long)status,1);}if(status){failed=1;if(service)ui_ai_service_request_close(service);}}
 ShowWindow(window,SW_SHOW);MSG m;while(GetMessageW(&m,NULL,0,0)>0){TranslateMessage(&m);DispatchMessageW(&m);}printf("{\"service_destroyed\":%s,\"workspace_destroyed\":%s,\"result\":%d}\n",service?"false":"true",workspace?"false":"true",(int)m.wParam);return (int)m.wParam;
}
