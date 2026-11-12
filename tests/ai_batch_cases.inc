/* Inspect the actual encoded production request, not a catalog-ID substring. */
#include "ui_framework/ai_validation_batch.h"
static void batch_page_contract(void)
{
 responses(rp_text,NULL);CHECK(real_submit("page-contract",0)==UI_STATUS_OK);authorize();CHECK(real_wait("\"terminal\":true"));
 yyjson_doc *d=yyjson_read(s->model_config->fixture_body,strlen(s->model_config->fixture_body),0);CHECK(d!=NULL);
 yyjson_val *tools=d?yyjson_obj_get(yyjson_doc_get_root(d),"tools"):NULL,*discover=yyjson_arr_get(tools,0),*page=yyjson_obj_get(yyjson_obj_get(yyjson_obj_get(discover,"parameters"),"properties"),"page"),*values=yyjson_obj_get(page,"enum");
 CHECK(yyjson_is_arr(values)&&yyjson_arr_size(values)==1&&yyjson_is_uint(yyjson_arr_get(values,0))&&yyjson_get_uint(yyjson_arr_get(values,0))==0);
 const char *description=yyjson_get_str(yyjson_obj_get(discover,"description"));CHECK(description&&strstr(description,"page 0"));yyjson_doc_free(d);
}
static void batch_page_reject(void)
{
 char *wire=_strdup(rp_discover),*scan=wire;unsigned changed=0;CHECK(wire!=NULL);if(!wire)return;
 while((scan=strstr(scan,"page\\\": 0"))!=NULL){scan[strlen("page\\\": ")]='1';++scan;++changed;}CHECK(changed>=3);responses(wire,NULL);free(wire);
 CHECK(real_submit("page-one-rejected",0)==UI_STATUS_OK);authorize();CHECK(real_wait("\"terminal\":true"));CHECK(strstr(state,"batch_wrapper_invalid"));CHECK(strstr(state,"\"tool_calls\":0"));CHECK(s->model_config->requests==1);CHECK(value(0)==10);close_service();CHECK(cr_sql("SELECT count(*) FROM tools")==0);
}
static char bc_state[20000];
static int bc_wait(const char *part)
{DWORD start=GetTickCount();do{char *j=NULL;pump();if(!ui_ai_validation_batch_get_state(s,&j)){int found=strstr(j,part)!=NULL;strcpy_s(bc_state,sizeof(bc_state),j);ui_ai_release(j);if(found)return 1;}}while(GetTickCount()-start<5000);fprintf(stderr,"batch wait %.1000s\n",bc_state);return 0;}
static void bc_event(const ui_ai_event_t *e,void *data)
{if(!strcmp(e->type,"received")){char *j=NULL;CHECK(!strcmp(e->client_request_id,"batch-T2")||!strcmp(e->client_request_id,"batch-T3"));CHECK(ui_ai_get_run(s,e->run_id,&j)==UI_STATUS_OK);CHECK(j&&strstr(j,e->client_request_id));ui_ai_release(j);}else model_event(e,data);}
static void bc_setup(const wchar_t *mode)
{
 close_service();CHECK(ui_capability_begin_unregistration(apps[0].host,"memory.read")==UI_STATUS_OK);CHECK(ui_capability_finish_unregistration(apps[0].host,"memory.read")==UI_STATUS_OK);
 ui_capability_desc_t read={0};read.size=sizeof(read);read.version=read.capability_version=1;read.id="memory.read";read.name_utf8="Read synthetic memory value and revision";read.description_utf8="Read-only RP-C fixture; value 10 and revision r1.";read.category="memory";read.effect="read";read.target_kind=UI_CAP_DOCUMENT;read.input_schema_json="{\"type\":\"object\",\"additionalProperties\":false}";read.output_schema_json="{\"type\":\"object\",\"properties\":{\"value\":{\"type\":\"integer\"},\"revision\":{\"type\":\"string\"}},\"required\":[\"value\",\"revision\"],\"additionalProperties\":false}";read.execute=vc_read;CHECK(ui_capability_register(apps[0].host,&read)==UI_STATUS_OK);
 GUID g;wchar_t guid[40],path[4096];CoCreateGuid(&g);StringFromGUID2(&g,guid,40);swprintf_s(path,4096,L"%s/batch-%s.sqlite",cr_directory,guid);CHECK(WideCharToMultiByte(CP_UTF8,0,path,-1,dbpath,sizeof(dbpath),NULL,NULL)>0);
 ui_ai_service_config_t config={0};config.size=sizeof(config);config.version=1;config.workspace=w;config.storage_path_utf8=dbpath;config.on_event=bc_event;CHECK(ui_ai_service_create(&config,&s)==UI_STATUS_OK);CHECK(ui_ai_service_open(s)==UI_STATUS_OK);DWORD begin=GetTickCount();ui_ai_service_state_t ss={0};ss.size=sizeof(ss);ss.version=1;do{pump();ui_ai_service_get_state(s,&ss);if(ss.phase==UI_AI_READY)break;}while(GetTickCount()-begin<5000);CHECK(ss.phase==UI_AI_READY);
 ui_cap_target_t target={0};target.size=sizeof(target);target.version=1;CHECK(ui_capability_bind(apps[0].host,"memory.document",&target)==UI_STATUS_OK);char profile[4096];snprintf(profile,sizeof(profile),"%s.validation-profile.json",dbpath);ui_ai_validation_batch_desc_t b={sizeof(b),1,profile,&target};CHECK(ui_ai_validation_batch_open(s,&b)==UI_STATUS_OK);
 /* Install memory credentials before the first UI poll can schedule job4. */
 CHECK(ui_ai_model_fixture(s,20,"1")==UI_STATUS_OK);wcscpy_s(s->model_config->fixture_credential,513,L"fixture-key-no-leak");s->model_config->fixture_batch_mode=1;if(!wcscmp(mode,L"bc-key")||!wcscmp(mode,L"bc-key-cancel"))s->model_config->fixture_read_error=ERROR_NOT_FOUND;CHECK(bc_wait(s->model_config->fixture_read_error?"\"need_key\":true":"\"ready\":true"));CHECK(!strcmp(s->model_config->ref,"ui-framework/xuanshu/deepseek/d15f7883aeeac194fe15029c233b44bdeb1f20dcd3d2f680373ed5e70803ab38"));CHECK(!strstr(s->model_config->path,".sqlite.model.json"));CHECK(s->model_config->requests==0);
}
static void bc_authorize(void)
{char *j=NULL,hash[65];CHECK(ui_ai_validation_batch_get_state(s,&j)==UI_STATUS_OK);str(j,"batch_hash",hash,65);ui_ai_release(j);CHECK(hash[0]);CHECK(ui_ai_model_fixture(s,30,hash)==UI_STATUS_OK);CHECK(ui_ai_model_fixture(s,30,hash)==UI_STATUS_PERMISSION_DENIED);}
static void bc_closed(void)
{DWORD start=GetTickCount();ui_ai_service_state_t ss={0};ss.size=sizeof(ss);ss.version=1;do{pump();ui_ai_service_get_state(s,&ss);if(ss.phase==UI_AI_CLOSED&&!ss.worker_alive)break;}while(GetTickCount()-start<5000);CHECK(ss.phase==UI_AI_CLOSED&&!ss.worker_alive);CHECK(bc_wait("\"export_ok\":true"));char path[4096];snprintf(path,sizeof(path),"%s.batch.json",dbpath);wchar_t wide[4096];CHECK(MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,path,-1,wide,4096)>0);FILE *f=_wfopen(wide,L"rb");CHECK(f!=NULL);if(f){char *text=calloc(1,512*1024+1);size_t n=fread(text,1,512*1024,f);fclose(f);yyjson_doc *d=yyjson_read(text,n,0);CHECK(d!=NULL);if(d){yyjson_val *o=yyjson_doc_get_root(d);CHECK(!strcmp(yyjson_get_str(yyjson_obj_get(o,"policy")),"RPCB-1"));CHECK(yyjson_is_arr(yyjson_obj_get(o,"events")));
 if(yyjson_get_bool(yyjson_obj_get(o,"read10_r1_proven"))){size_t i,count;yyjson_val *row;unsigned reads=0,proofs=0;
  yyjson_arr_foreach(yyjson_obj_get(o,"tools"),i,count,row){const char *id=yyjson_get_str(yyjson_obj_get(row,"capability"));if(id&&!strcmp(id,"memory.read")){const char *raw=yyjson_get_str(yyjson_obj_get(row,"result"));yyjson_doc *result=raw?yyjson_read(raw,strlen(raw),0):NULL;yyjson_val *obj=result?yyjson_doc_get_root(result):NULL,*v=yyjson_obj_get(obj,"value"),*r=yyjson_obj_get(obj,"revision");CHECK(yyjson_is_uint(v)&&yyjson_get_uint(v)==10);CHECK(yyjson_is_str(r)&&!strcmp(yyjson_get_str(r),"r1"));CHECK(yyjson_get_sint(yyjson_obj_get(row,"status"))==UI_STATUS_OK);++reads;yyjson_doc_free(result);}}
  yyjson_arr_foreach(yyjson_obj_get(o,"events"),i,count,row){const char *kind=yyjson_get_str(yyjson_obj_get(row,"kind"));if(kind&&!strcmp(kind,"rpc_batch_tool_proof")){const char *raw=yyjson_get_str(yyjson_obj_get(row,"payload"));yyjson_doc *proof=yyjson_read(raw,strlen(raw),0);yyjson_val *obj=proof?yyjson_doc_get_root(proof):NULL;const char *type=yyjson_get_str(yyjson_obj_get(obj,"kind"));if(type&&!strcmp(type,"read")){CHECK(!strcmp(yyjson_get_str(yyjson_obj_get(obj,"framework_revision")),"r1"));size_t j,total;yyjson_val *tool;int matched=0;const char *call=yyjson_get_str(yyjson_obj_get(obj,"tool_call_id"));yyjson_arr_foreach(yyjson_obj_get(o,"tools"),j,total,tool){const char *id=yyjson_get_str(yyjson_obj_get(tool,"tool_id"));if(call&&id&&!strcmp(id,call)){const char *raw_result=yyjson_get_str(yyjson_obj_get(tool,"result"));char hash[65];CHECK(raw_result&&ui_cap_json_fingerprint(raw_result,strlen(raw_result),hash)==UI_STATUS_OK);CHECK(!strcmp(yyjson_get_str(yyjson_obj_get(obj,"result_hash")),hash));matched=1;break;}}CHECK(matched);++proofs;}yyjson_doc_free(proof);}}
  CHECK(reads>=1&&proofs==reads);
 }}yyjson_doc_free(d);free(text);}}
static void bc_fill_notices(void)
{
 unsigned admitted=0;DWORD begin=GetTickCount();int full=0;
 do{EnterCriticalSection(&s->lock);full=s->event_count==AI_EVENTS;CHECK(s->event_count<=AI_EVENTS&&s->event_bytes<=UI_AI_EVENT_BYTES);LeaveCriticalSection(&s->lock);if(full)break;char id[65];snprintf(id,65,"batch-pressure-%u",admitted);ui_status_t st=ui_ai_history(s,id,UI_AI_MESSAGES,0,1);CHECK(st==UI_STATUS_OK||st==UI_STATUS_LIMIT_EXCEEDED);if(!st)++admitted;SwitchToThread();}while(GetTickCount()-begin<5000);
 CHECK(full&&admitted>=AI_EVENTS);CHECK(s->model_config->requests==0);
}
static void bc_restart(void)
{
 close_service();ui_ai_service_config_t config={0};config.size=sizeof(config);config.version=1;config.workspace=w;config.storage_path_utf8=dbpath;config.on_event=bc_event;CHECK(ui_ai_service_create(&config,&s)==UI_STATUS_OK);CHECK(ui_ai_service_open(s)==UI_STATUS_OK);cr_idle();
 ui_cap_target_t target={0};target.size=sizeof(target);target.version=1;CHECK(ui_capability_bind(apps[0].host,"memory.document",&target)==UI_STATUS_OK);char profile[4096];snprintf(profile,sizeof(profile),"%s.validation-profile.json",dbpath);ui_ai_validation_batch_desc_t d={sizeof(d),1,profile,&target};CHECK(ui_ai_validation_batch_open(s,&d)==UI_STATUS_OK);CHECK(ui_ai_model_fixture(s,20,"1")==UI_STATUS_OK);wcscpy_s(s->model_config->fixture_credential,513,L"fixture-key-no-leak");CHECK(bc_wait("\"export_done\":true"));CHECK(strstr(bc_state,"\"accepted\":false"));CHECK(strstr(bc_state,"\"export_ok\":false"));char *policy=NULL;CHECK(ui_ai_validation_get_state(s,&policy)==UI_STATUS_OK);CHECK(policy&&strstr(policy,"\"prior_session\":true")&&strstr(policy,"\"stopped\":true")&&strstr(policy,"\"reserved\":4"));ui_ai_release(policy);CHECK(s->model_config->requests==0);CHECK(ui_ai_model_fixture(s,30,"closed-batch")==UI_STATUS_PERMISSION_DENIED);close_service();CHECK(cr_sql("SELECT count(*) FROM events WHERE kind='rpc_reserved'")==4);CHECK(cr_sql("SELECT count(*) FROM events WHERE kind='rpc_batch_confirm'")==1);
}
static char bc_nested_hash[65];static unsigned bc_nested_calls;
static void bc_nested(const char *id,const char *payload,void *data)
{(void)payload;(void)data;if(!strcmp(id,"edit.reenter")){++bc_nested_calls;CHECK(ui_ai_model_fixture(s,30,bc_nested_hash)==UI_STATUS_PERMISSION_DENIED);CHECK(s->methods>0);CHECK(ui_ai_service_destroy(s)==UI_STATUS_PENDING);}}
static void bc_case(const wchar_t *mode)
{
 bc_setup(mode);vc_wires(rp_discover,rp_read,rp_text,NULL);
 if(!wcscmp(mode,L"bc-key")||!wcscmp(mode,L"bc-key-cancel")){CHECK(s->model_config->requests==0);s->model_config->fixture_read_error=0;if(!wcscmp(mode,L"bc-key-cancel"))s->model_config->fixture_prompt_result=0;ui_status_t st=ui_ai_model_fixture(s,21,"");CHECK(st==(!wcscmp(mode,L"bc-key-cancel")?UI_STATUS_PERMISSION_DENIED:UI_STATUS_OK));if(!wcscmp(mode,L"bc-key-cancel")){bc_closed();CHECK(s->model_config->requests==0);return;}CHECK(bc_wait("\"ready\":true"));CHECK(!strcmp(s->model_config->fixture_write_ref,s->model_config->ref));}
 if(!wcscmp(mode,L"bc-bad-value"))vc_value=100;
 if(!wcscmp(mode,L"bc-target")){CHECK(ui_workspace_activate(w,apps[1].instance_id)==UI_STATUS_OK);CHECK(ui_workspace_close(w,apps[0].instance_id,UI_APP_CLOSE_TAB)==UI_STATUS_OK);bc_closed();CHECK(s->model_config->requests==0);return;}
 if(!wcscmp(mode,L"bc-unconfirmed")){
  CHECK(ui_ai_model_fixture(s,30,"invalid")==UI_STATUS_PERMISSION_DENIED);CHECK(vc_submit(1,"forbidden-T1")==UI_STATUS_PERMISSION_DENIED);CHECK(vc_submit(2,"external-submit")==UI_STATUS_PERMISSION_DENIED);CHECK(real_submit("old-real-bypass",0)==UI_STATUS_PERMISSION_DENIED);
  char *j=NULL,hash[65];CHECK(ui_ai_validation_batch_get_state(s,&j)==UI_STATUS_OK);str(j,"batch_hash",hash,65);ui_ai_release(j);HWND parent=CreateWindowW(L"STATIC",L"batch",WS_OVERLAPPED,-16000,-16000,200,100,NULL,NULL,GetModuleHandleW(NULL),NULL);HWND button=CreateWindowW(L"BUTTON",L"Start",WS_CHILD|WS_VISIBLE,0,0,100,30,parent,NULL,GetModuleHandleW(NULL),NULL);ui_ai_validation_batch_button_t d={sizeof(d),1,button,hash};uint64_t id=0;CHECK(ui_ai_validation_batch_bind_button(s,&d,&id)==UI_STATUS_OK);SendMessageW(button,BM_CLICK,0,0);SendMessageW(button,WM_LBUTTONDOWN,MK_LBUTTON,0);SendMessageW(button,WM_LBUTTONUP,0,0);pump();CHECK(bc_wait("\"accepted\":false"));CHECK(s->model_config->requests==0);CHECK(ui_ai_model_unbind(s,id)==UI_STATUS_OK);DestroyWindow(parent);CHECK(ui_ai_validation_stop(s)==UI_STATUS_OK);bc_closed();return;
 }
 if(!wcscmp(mode,L"bc-change")){ui_ai_model_profile_t v={sizeof(v),1,"deepseek","deepseek-responses-v1","https://api.deepseek.com","deepseek-flash",s->model_config->ref,s->model_config->revision+1};CHECK(ui_ai_model_configure(s,&v,"batch-change")==UI_STATUS_OK);bc_closed();CHECK(s->model_config->requests==0);return;}
 if(!wcscmp(mode,L"bc-store"))s->fixture_fail_write=1;
 if(!wcscmp(mode,L"bc-checkpoint"))InterlockedExchange(&s->fixture_checkpoint,SQLITE_IOERR_WRITE);
 if(!wcscmp(mode,L"bc-limit")){char *unique=vc_unique_read();vc_wires(rp_discover,rp_read,unique,rp_text);free(unique);}
 if(!wcscmp(mode,L"bc-cancel"))s->model_config->fixture_batch_mode=2;
 if(!wcscmp(mode,L"bc-backpressure"))bc_fill_notices();CHECK(ui_workspace_activate(w,apps[1].instance_id)==UI_STATUS_OK);if(!wcscmp(mode,L"bc-reentry")){char *state_json=NULL;CHECK(ui_ai_validation_batch_get_state(s,&state_json)==UI_STATUS_OK);str(state_json,"batch_hash",bc_nested_hash,65);ui_ai_release(state_json);ui_event_callback_fn old=apps[0].host->event_callback;apps[0].host->event_callback=bc_nested;control(0,AE_MODE_REENTER_PROPOSE);bc_authorize();apps[0].host->event_callback=old;CHECK(bc_nested_calls==1);}else bc_authorize();if(!wcscmp(mode,L"bc-expire"))s->time_bias+=600001;
 bc_closed();CHECK(!secret_leaked);CHECK(s->model_config->requests<=4);CHECK(vc_submit(1,"never-T1")==UI_STATUS_PERMISSION_DENIED);
 if(!wcscmp(mode,L"bc-store")||!wcscmp(mode,L"bc-expire")){CHECK(s->model_config->requests==0);}
 else if(!wcscmp(mode,L"bc-bad-value")){CHECK(s->model_config->requests==2);CHECK(!strstr(bc_state,"\"read10_r1_proven\":true"));}
 else if(!wcscmp(mode,L"bc-limit")){CHECK(s->model_config->requests==3);CHECK(!strstr(bc_state,"\"T3_run\":\"batch"));}
 else if(!wcscmp(mode,L"bc-checkpoint")){CHECK(s->store_failed);CHECK(s->model_config->requests<=3);}
 else{CHECK(strstr(bc_state,"\"discovery_proven\":true"));CHECK(strstr(bc_state,"\"read10_r1_proven\":true"));CHECK(s->model_config->requests==4);if(!wcscmp(mode,L"bc-cancel"))CHECK(strstr(bc_state,"\"cancel_requested\":true"));else CHECK(strstr(bc_state,"\"cancel_window_missed\":true")||strstr(bc_state,"\"cancel_requested\":true"));}
 if(!wcscmp(mode,L"bc-restart")){bc_restart();return;}
 close_service();CHECK(cr_sql("SELECT count(*) FROM runs WHERE client='batch-T1'")==0);CHECK(cr_sql("SELECT count(*) FROM events WHERE kind='rpc_batch_confirm'")==(!wcscmp(mode,L"bc-store")?0:1));
}
