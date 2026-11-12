/* Probe projection test. No native credential prompt or real request. */
#define wmain rpc_probe_program_main
#include "../tools/xuanshu_rpc_probe.c"
#undef wmain
static unsigned checks,failures;
#define CHECK(x) do{++checks;if(!(x)){++failures;fprintf(stderr,"line %d: %s\n",__LINE__,#x);}}while(0)
int wmain(int argc,wchar_t **argv)
{
 if(argc!=3)return 2;CHECK(ui_framework_initialize()==UI_STATUS_OK);CHECK(CreateDirectoryW(argv[2],NULL)||GetLastError()==ERROR_ALREADY_EXISTS);
 GUID g;wchar_t guid[40],path[4096];char db[4096],config[4096],package[4096];CoCreateGuid(&g);StringFromGUID2(&g,guid,40);swprintf_s(path,4096,L"%s/%s.sqlite",argv[2],guid);CHECK(WideCharToMultiByte(CP_UTF8,0,path,-1,db,4096,NULL,NULL)>0);snprintf(config,sizeof(config),"%s.model.json",db);CHECK(WideCharToMultiByte(CP_UTF8,0,argv[1],-1,package,4096,NULL,NULL)>0);
 WNDCLASSW wc={0};wc.lpfnWndProc=proc;wc.hInstance=GetModuleHandleW(NULL);wc.lpszClassName=L"RpcProbeProjectionTest";CHECK(RegisterClassW(&wc)!=0);HWND window=CreateWindowW(wc.lpszClassName,L"Offline",WS_OVERLAPPEDWINDOW,-16000,-16000,1000,760,NULL,NULL,wc.hInstance,NULL);CHECK(window!=NULL);
 display=CreateWindowW(L"EDIT",L"",WS_CHILD|WS_VSCROLL|ES_MULTILINE|ES_READONLY,0,50,950,650,window,NULL,wc.hInstance,NULL);key_button=CreateWindowW(L"BUTTON",L"Key",WS_CHILD|WS_VISIBLE,0,0,130,30,window,(HMENU)106,wc.hInstance,NULL);confirm_button=CreateWindowW(L"BUTTON",L"Confirm",WS_CHILD|WS_VISIBLE,140,0,130,30,window,(HMENU)107,wc.hInstance,NULL);CHECK(display&&key_button&&confirm_button);ShowWindow(window,SW_SHOWNOACTIVATE);CHECK(IsWindowVisible(key_button));
 ui_workspace_config_t w={0};w.size=sizeof(w);w.run_mode=UI_RUN_OFFSCREEN;workspace=ui_workspace_create(&w);CHECK(workspace!=NULL);CHECK(ui_workspace_open(workspace,package,&instance)==UI_STATUS_OK);
 ui_ai_service_config_t sc={0};sc.size=sizeof(sc);sc.version=1;sc.workspace=workspace;sc.storage_path_utf8=db;CHECK(ui_ai_service_create(&sc,&service)==UI_STATUS_OK);CHECK(ui_ai_service_open(service)==UI_STATUS_OK);CHECK(ui_ai_model_open(service,config)==UI_STATUS_OK);
 DWORD begin=GetTickCount();do{tick(window);if(key_binding)break;(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}while(GetTickCount()-begin<5000);CHECK(key_binding!=0);CHECK(enabled&&configured);tick(window);tick(window);
 char *model=NULL;CHECK(ui_ai_model_get_state(service,&model)==UI_STATUS_OK);yyjson_doc *d=model?yyjson_read(model,strlen(model),0):NULL;yyjson_val *credential=d?yyjson_obj_get(yyjson_doc_get_root(d),"credential"):NULL;CHECK(yyjson_is_obj(credential));CHECK(!yyjson_get_bool(yyjson_obj_get(credential,"local_readable")));yyjson_doc_free(d);ui_ai_release(model);
 SendMessageW(display,EM_SETSEL,4,24);DWORD first=0,last=0;SendMessageW(display,EM_GETSEL,(WPARAM)&first,(LPARAM)&last);CHECK(first==4&&last==24);tick(window);tick(window);SendMessageW(display,EM_GETSEL,(WPARAM)&first,(LPARAM)&last);CHECK(first==4&&last==24);
 strcpy_s(action_status,sizeof(action_status),"controlled state change");tick(window);SendMessageW(display,EM_GETSEL,(WPARAM)&first,(LPARAM)&last);CHECK(first==4&&last==24);SendMessageW(window,WM_COMMAND,108,0);CHECK(probe_test_clipboard&&wcsstr(probe_test_clipboard,L"credential")&&wcsstr(probe_test_clipboard,L"manifest"));CHECK(!wcsstr(probe_test_clipboard,L"fixture-key-no-leak"));
 SendMessageW(key_button,BM_CLICK,0,0);SendMessageW(key_button,WM_LBUTTONDOWN,MK_LBUTTON,0);CHECK(GetCapture()==key_button);SendMessageW(key_button,WM_LBUTTONUP,0,0);CHECK(GetCapture()!=key_button);SendMessageW(window,WM_COMMAND,106,0);tick(window);
 model=NULL;CHECK(ui_ai_model_get_state(service,&model)==UI_STATUS_OK);d=yyjson_read(model,strlen(model),0);credential=yyjson_obj_get(yyjson_doc_get_root(d),"credential");CHECK(!yyjson_get_bool(yyjson_obj_get(credential,"window_opened")));CHECK(!strcmp(yyjson_get_str(yyjson_obj_get(credential,"last_entry")),"input_not_qualified"));yyjson_doc_free(d);ui_ai_release(model);
 SendMessageW(window,WM_COMMAND,101,0);CHECK(!run[0]);
 char *policy=NULL;CHECK(ui_ai_validation_get_state(service,&policy)==UI_STATUS_OK);CHECK(policy&&strstr(policy,"\"reserved\":0")&&strstr(policy,"\"send_entries\":0"));ui_ai_release(policy);
 CHECK(ui_ai_service_request_close(service)==UI_STATUS_OK);begin=GetTickCount();ui_status_t status=UI_STATUS_PENDING;do{ui_ai_service_poll(service,64);ui_workspace_poll(workspace);status=ui_ai_service_destroy(service);if(status==UI_STATUS_OK)break;(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}while(GetTickCount()-begin<5000);CHECK(status==UI_STATUS_OK);service=NULL;
 CHECK(ui_workspace_close_all(workspace)==UI_STATUS_OK);begin=GetTickCount();while(ui_workspace_count(workspace)&&GetTickCount()-begin<5000)ui_workspace_poll(workspace);CHECK(ui_workspace_destroy(workspace)==UI_STATUS_OK);workspace=NULL;DestroyWindow(window);
 printf("probe projection checks=%u failures=%u; no physical source or Provider claim\n",checks,failures);return failures?1:0;
}
