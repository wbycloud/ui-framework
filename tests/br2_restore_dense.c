/* Ordinary admission and reopening at the existing container/node boundaries. */
#define BR1_BUSINESS_MAIN dense_previous_main
#include "business_read.c"

int wmain(int argc,wchar_t **argv)
{
 if(argc!=2)return 2;
 CHECK(CreateDirectoryW(argv[1],NULL)||GetLastError()==ERROR_ALREADY_EXISTS);
 GUID id;wchar_t name[40],database[4096];CHECK(CoCreateGuid(&id)==S_OK);StringFromGUID2(&id,name,40);
 _snwprintf_s(database,4096,_TRUNCATE,L"%s/dense-%s.db",argv[1],name);
 CHECK(SetEnvironmentVariableW(L"UI_AI_DB_PATH",database));CHECK(SetEnvironmentVariableW(L"UI_AI_BR2_POLICY",NULL));
 CHECK(ui_framework_initialize()==UI_STATUS_OK);
 yyjson_mut_doc *doc=yyjson_mut_doc_new(NULL);CHECK(doc!=NULL);if(!doc)return 1;
 yyjson_mut_val *root=yyjson_mut_obj(doc),*steps=yyjson_mut_arr(doc),*step=yyjson_mut_obj(doc),*extra=yyjson_mut_arr(doc);
 yyjson_mut_doc_set_root(doc,root);yyjson_mut_obj_add_val(doc,root,"steps",steps);yyjson_mut_arr_add_val(steps,step);
 yyjson_mut_obj_add_strcpy(doc,step,"type","text");yyjson_mut_obj_add_strcpy(doc,step,"text","ordinary dense saved result");yyjson_mut_obj_add_val(doc,step,"extra",extra);
 for(unsigned i=0;i<7;++i){yyjson_mut_val *items=yyjson_mut_arr(doc);yyjson_mut_arr_add_val(extra,items);for(unsigned j=0;j<512;++j)yyjson_mut_arr_add_uint(doc,items,12345678);}
 char *plan=yyjson_mut_write(doc,0,NULL);CHECK(plan&&strlen(plan)<=UI_AI_MAX_PLAN);
 char run[65]={0};
 for(unsigned attempt=0;attempt<2;++attempt){
  host_window_t host={0};CHECK(create_host(&host,GetModuleHandleW(NULL))!=NULL);if(!host.ai)break;ShowWindow(host.hwnd,SW_HIDE);
  DWORD begin=GetTickCount();ui_ai_service_state_t state={0};state.size=sizeof(state);state.version=1;
  do{pump(&host);CHECK(ui_ai_service_get_state(host.ai,&state)==UI_STATUS_OK);}while(state.phase==UI_AI_OPENING&&GetTickCount()-begin<5000);
  CHECK(state.phase==UI_AI_READY);char *guard=NULL;CHECK(ui_ai_validation_get_state(host.ai,&guard)==UI_STATUS_NOT_FOUND);ui_ai_release(guard);
  if(!attempt){
   ui_ai_task_request_t request={0};request.size=sizeof(request);request.version=1;request.client_request_id="ordinary-dense";request.input_utf8="普通库合法高节点计划";request.stub_plan_json=plan;
   CHECK(ui_ai_submit(host.ai,&request,run)==UI_STATUS_OK);int done=0;begin=GetTickCount();
   do{char *task=NULL;pump(&host);if(!ui_ai_get_run(host.ai,run,&task))done=strstr(task,"\"terminal\":true")!=NULL;ui_ai_release(task);}while(!done&&GetTickCount()-begin<5000);CHECK(done);
  }else{
   host_action_t action={0};action.action="ai-select";action.id=run;process_action(&host,&action);begin=GetTickCount();
   do{pump(&host);}while((host.ai_query_pending||!host.ai_stored_run)&&GetTickCount()-begin<5000);
   CHECK(host.ai_stored_run&&strstr(host.ai_stored_run,run)&&strstr(host.ai_stored_run,"ordinary dense saved result")&&strstr(host.ai_stored_run,"completed"));
  }
  CHECK(host.ai->model_config->requests==0&&host.ai->methods==0&&host.ai->callbacks==0);CHECK(close_host(&host));
 }
 printf("ordinary dense plan bytes=%zu arrays=7*512 checks=%u failures=%u\n",plan?strlen(plan):0,checks,failures);
 free(plan);yyjson_mut_doc_free(doc);SetEnvironmentVariableW(L"UI_AI_DB_PATH",NULL);return failures?1:0;
}
