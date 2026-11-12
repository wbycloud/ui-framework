/* No automatic requests. Production runtime; native Key/confirmation bindings. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
#include "ui_framework/ai_validation.h"
#include "ui_framework/ai_model_ui.h"
#include "yyjson.h"
static ui_workspace_t *workspace;static ui_ai_service_t *service;static HWND display,key_button,confirm_button;static uint64_t key_binding,confirm_binding,instance;static char run[65],key_hash[65],confirm_hash[65];static int enabled,configured,closing;static unsigned sequence;static char action_status[96];
#include "xuanshu_rpc_probe_ui.inc"
static const char *field(yyjson_doc *d,const char *name){return d?yyjson_get_str(yyjson_obj_get(yyjson_doc_get_root(d),name)):NULL;}
static ui_status_t bind_button(HWND button,int action,uint64_t *binding,char old[65],const char *hash,const char *id)
{if(!hash)return UI_STATUS_PENDING;if(*binding&&!strcmp(old,hash))return UI_STATUS_OK;if(*binding){ui_status_t st=ui_ai_model_unbind(service,*binding);if(st!=UI_STATUS_OK&&st!=UI_STATUS_NOT_FOUND)return st;*binding=0;}ui_ai_model_button_binding_t b={0};b.size=sizeof(b);b.version=1;b.button=button;b.action=(ui_ai_model_action_t)action;b.run_id=id;b.binding_hash=hash;ui_status_t st=ui_ai_model_bind_button(service,&b,binding);if(st==UI_STATUS_OK)strcpy_s(old,65,hash);return st;}
static void tick(HWND window)
{
 if(closing&&!service){if(!workspace){DestroyWindow(window);return;}(void)ui_workspace_close_all(workspace);ui_workspace_poll(workspace);if(ui_workspace_destroy(workspace)==UI_STATUS_OK){workspace=NULL;DestroyWindow(window);}return;}
 int credential_ready=0,active=0,model_idle=0,confirm_available=0;const char *credential_label="未配置";char *model=NULL,*policy=NULL,*task=NULL;ui_ai_service_state_t state={0};state.size=sizeof(state);state.version=1;(void)ui_ai_service_poll(service,64);ui_workspace_poll(workspace);ui_ai_service_get_state(service,&state);
 if(closing){if(ui_ai_service_destroy(service)==UI_STATUS_OK){service=NULL;(void)ui_workspace_close_all(workspace);ui_workspace_poll(workspace);if(ui_workspace_destroy(workspace)==UI_STATUS_OK){workspace=NULL;DestroyWindow(window);}}return;}
 if(ui_ai_model_get_state(service,&model)==UI_STATUS_OK){yyjson_doc *d=yyjson_read(model,strlen(model),0);yyjson_val *o=d?yyjson_doc_get_root(d):NULL;int pending=yyjson_get_bool(yyjson_obj_get(o,"pending"));
  if(!pending&&!enabled){ui_ai_validation_desc_t v={0};v.size=sizeof(v);v.version=1;v.validation_id="rpc-first-service";v.input_nano_per_token=300;v.output_nano_per_token=1200;enabled=ui_ai_validation_enable(service,&v)==UI_STATUS_OK;}
  if(enabled&&!pending&&!configured){ui_ai_model_profile_t p={0};p.size=sizeof(p);p.version=1;p.profile_id="deepseek";p.protocol_profile="deepseek-responses-v1";p.origin="https://api.deepseek.com";p.model_id="deepseek-flash";p.credential_ref=field(d,"credential_ref");p.revision=1;configured=ui_ai_model_configure(service,&p,"rpc-model")==UI_STATUS_OK;}
  if(!pending&&configured&&yyjson_get_bool(yyjson_obj_get(o,"ready")))key_bind_status=bind_button(key_button,UI_AI_MODEL_KEY,&key_binding,key_hash,field(d,"binding_hash"),NULL);
  model_idle=!pending&&yyjson_get_bool(yyjson_obj_get(o,"ready"))&&!yyjson_get_bool(yyjson_obj_get(yyjson_obj_get(o,"credential"),"dialog_open"));credential_ready=probe_credential_ready(o);credential_label=probe_credential_label(yyjson_get_str(yyjson_obj_get(yyjson_obj_get(o,"credential"),"stage")));yyjson_doc_free(d);
 }
 (void)ui_ai_validation_get_state(service,&policy);if(run[0]&&!ui_ai_get_run(service,run,&task)){yyjson_doc *d=yyjson_read(task,strlen(task),0);yyjson_val *o=d?yyjson_doc_get_root(d):NULL;active=!yyjson_get_bool(yyjson_obj_get(o,"terminal"));if(yyjson_get_int(yyjson_obj_get(o,"phase"))==UI_AI_WAITING_INPUT&&!yyjson_get_bool(yyjson_obj_get(o,"disclosure_confirmed"))){confirm_bind_status=bind_button(confirm_button,UI_AI_MODEL_CONFIRM,&confirm_binding,confirm_hash,field(d,"binding_hash"),run);confirm_available=confirm_bind_status==UI_STATUS_OK;}yyjson_doc_free(d);}
 yyjson_doc *pd=policy?yyjson_read(policy,strlen(policy),0):NULL;yyjson_val *po=pd?yyjson_doc_get_root(pd):NULL;int allowed=credential_ready&&!active&&yyjson_get_bool(yyjson_obj_get(po,"ready"))&&!yyjson_get_bool(yyjson_obj_get(po,"stopped"));yyjson_val *used=yyjson_obj_get(po,"task_used");for(unsigned i=0;i<3;++i)if(task_buttons[i])EnableWindow(task_buttons[i],allowed&&yyjson_get_uint(yyjson_arr_get(used,i))==0);yyjson_doc_free(pd);
 EnableWindow(key_button,model_idle&&!active&&key_binding&&key_bind_status==UI_STATUS_OK);EnableWindow(confirm_button,confirm_available&&confirm_binding);
 size_t n=(model?strlen(model):0)+(policy?strlen(policy):0)+(task?strlen(task):0)+1600;char *utf=malloc(n);if(utf){snprintf(utf,n,"玄枢 RP-C受控验证：仅本轮授权的三项合成任务，逐任务确认后才发送。\r\n不自动重试；USD0.10为本地停止阈值，不是服务端硬扣费保证。\r\n凭据状态：%s\r\n入口绑定：Key=%d，外发确认=%d（0=成功，未就绪时不可用）\r\n最近操作：%s\r\n模型/凭据诊断：%s\r\nPolicy（M/R/S/C/U及费用）：%s\r\n任务/准确外发预览：%s",credential_label,key_bind_status,confirm_bind_status,action_status,model?model:"pending",policy?policy:"pending",task?task:"none");int count=MultiByteToWideChar(CP_UTF8,0,utf,-1,NULL,0);wchar_t *wide=count>0?malloc((size_t)count*sizeof(wchar_t)):NULL;if(wide){MultiByteToWideChar(CP_UTF8,0,utf,-1,wide,count);probe_display(wide);free(wide);}free(utf);}ui_ai_release(model);ui_ai_release(policy);ui_ai_release(task);
}
static LRESULT CALLBACK proc(HWND w,UINT msg,WPARAM wp,LPARAM lp)
{
 if(msg==WM_TIMER&&(service||closing)){tick(w);return 0;}
 if(msg==WM_COMMAND&&service){unsigned command=LOWORD(wp);if(command==108){ui_status_t copied=probe_copy(w);snprintf(action_status,sizeof(action_status),"复制诊断 status=%d（仅复制当前脱敏快照，无授权或请求）",copied);return 0;}
  if(command>=101&&command<=103){char *j=NULL;int ready=0;if(ui_ai_model_get_state(service,&j)==UI_STATUS_OK){yyjson_doc *d=yyjson_read(j,strlen(j),0);ready=d&&probe_credential_ready(yyjson_doc_get_root(d));yyjson_doc_free(d);}ui_ai_release(j);if(!ready){strcpy_s(action_status,sizeof(action_status),"尚无凭据保存及本地读取确认；未提交任务、未消费配额。");return 0;}char client[65];ui_cap_target_t t={0};ui_app_instance_info_t app={0};ui_ai_validation_request_t q={0};q.size=sizeof(q);q.version=1;q.task=(ui_ai_validation_task_t)(command-100);snprintf(client,65,"rpc-task-%u-%u",command,++sequence);q.client_request_id=client;if(q.task==UI_AI_VALIDATION_READ){app.size=sizeof(app);if(ui_workspace_get_instance(workspace,instance,&app)!=UI_STATUS_OK)return 0;t.size=sizeof(t);t.version=1;if(ui_capability_bind(app.host,"memory.document",&t)!=UI_STATUS_OK)return 0;q.target=&t;}char proposed[65];ui_status_t status=ui_ai_validation_propose(service,&q,proposed);snprintf(action_status,sizeof(action_status),"Task proposal status=%d (OK=received, not saved)",status);if(!status)strcpy_s(run,65,proposed);}
  if(command==104&&run[0])(void)ui_ai_cancel(service,run);if(command==105)(void)ui_ai_validation_stop(service);return 0;}
 if(msg==WM_CLOSE){closing=1;if(service)(void)ui_ai_service_request_close(service);return 0;}if(msg==WM_DESTROY){free(diagnostic_text);diagnostic_text=NULL;
#ifdef UI_RPC_PROBE_TEST
 free(probe_test_clipboard);probe_test_clipboard=NULL;
#endif
 PostQuitMessage(0);return 0;}return DefWindowProcW(w,msg,wp,lp);
}
int wmain(int argc,wchar_t **argv)
{
 if(argc!=3){fwprintf(stderr,L"Usage (requires explicit validation authorization): xuanshu_rpc_probe.exe NEW_SESSION_DIRECTORY rpc_memory.uapp\n");return 2;}
 wchar_t root[3500];DWORD root_length=GetFullPathNameW(argv[1],3500,root,NULL);if(!root_length||root_length>=3500||!CreateDirectoryW(root,NULL)){fwprintf(stderr,L"Refuse existing/uncreatable session directory.\n");return 2;}
 char db[4096],config[4096],package[4096];wchar_t path[3600];swprintf_s(path,3600,L"%s/conversation.sqlite",root);if(!WideCharToMultiByte(CP_UTF8,0,path,-1,db,4096,NULL,NULL)||!WideCharToMultiByte(CP_UTF8,0,argv[2],-1,package,4096,NULL,NULL))return 2;snprintf(config,4096,"%s.model.json",db);
 WNDCLASSW c={0};c.lpfnWndProc=proc;c.hInstance=GetModuleHandleW(NULL);c.lpszClassName=L"XuanShuRpcProbe";c.hCursor=LoadCursorW(NULL,MAKEINTRESOURCEW(32512));RegisterClassW(&c);HWND window=CreateWindowW(c.lpszClassName,L"玄枢 RP-C受控验证（仅三项合成任务）",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,1000,760,NULL,NULL,c.hInstance,NULL);if(!window)return 2;
 display=CreateWindowW(L"EDIT",L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL|ES_MULTILINE|ES_READONLY,10,60,960,650,window,NULL,c.hInstance,NULL);
 const wchar_t *titles[]={L"T1 文本",L"T2 只读",L"T3 取消测试",L"取消任务",L"停止本轮",L"设置DeepSeek API Key",L"确认外发",L"复制诊断"};int widths[]={100,100,110,80,100,190,100,120},x=10;for(unsigned i=0;i<8;++i){HWND button=CreateWindowW(L"BUTTON",titles[i],WS_CHILD|WS_VISIBLE|WS_TABSTOP,x,10,widths[i],35,window,(HMENU)(INT_PTR)(101+i),c.hInstance,NULL);x+=widths[i]+5;if(i<3)task_buttons[i]=button;if(i==5)key_button=button;if(i==6)confirm_button=button;}
 if(!display||!key_button||!confirm_button){DestroyWindow(window);return 2;}
 if(ui_framework_initialize()!=UI_STATUS_OK){DestroyWindow(window);return 2;}ui_workspace_config_t wc={0};wc.size=sizeof(wc);wc.run_mode=UI_RUN_OFFSCREEN;workspace=ui_workspace_create(&wc);if(!workspace||ui_workspace_open(workspace,package,&instance)!=UI_STATUS_OK)closing=1;
 ui_ai_service_config_t sc={0};sc.size=sizeof(sc);sc.version=1;sc.workspace=workspace;sc.storage_path_utf8=db;if(!closing&&(ui_ai_service_create(&sc,&service)||ui_ai_service_open(service)||ui_ai_model_open(service,config)))closing=1;if(closing&&service)(void)ui_ai_service_request_close(service);
 SetTimer(window,1,15,NULL);ShowWindow(window,SW_SHOW);MSG m;while(GetMessageW(&m,NULL,0,0)>0){TranslateMessage(&m);DispatchMessageW(&m);}return 0;
}
