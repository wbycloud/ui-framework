/* New read-only API leases cover the last application callback and outer stack. */
static int uh_callback,uh_changed,uh_destroy_pending;
static ui_status_t uh_revision(char *out,size_t capacity,void *data)
{
 (void)data;if(uh_callback){uh_callback=0;CHECK(ui_ai_service_request_close(s)==UI_STATUS_OK);DWORD start=GetTickCount();ui_ai_service_state_t service_state={0};service_state.size=sizeof(service_state);service_state.version=1;
  do{CHECK(ui_ai_service_poll(s,64)==UI_STATUS_OK);CHECK(ui_ai_service_get_state(s,&service_state)==UI_STATUS_OK);if(service_state.phase==UI_AI_CLOSED&&!service_state.worker_alive)break;(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}while(GetTickCount()-start<5000);
  CHECK(service_state.phase==UI_AI_CLOSED);ui_status_t status=ui_ai_service_destroy(s);CHECK(status==UI_STATUS_PENDING);uh_destroy_pending=status==UI_STATUS_PENDING;
 }
 return strcpy_s(out,capacity,uh_changed?"r2":"r1")?UI_STATUS_INVALID_ARGUMENT:UI_STATUS_OK;
}
static void uh_disclosure(const wchar_t *kind)
{
 ui_cap_resource_desc_t desc={0};desc.size=sizeof(desc);desc.version=1;desc.kind=UI_CAP_DOCUMENT;desc.id="view.resource";desc.get_revision=uh_revision;CHECK(ui_capability_register_resource(apps[0].host,&desc)==UI_STATUS_OK);
 ui_cap_target_t target={0};target.size=sizeof(target);target.version=1;CHECK(ui_capability_bind(apps[0].host,desc.id,&target)==UI_STATUS_OK);ui_ai_real_task_t q={0};q.size=sizeof(q);q.version=1;q.client_request_id="view-proposal";q.input_utf8="synthetic view only";q.profile_id="deepseek";q.profile_revision=1;q.targets=&target;q.target_count=1;q.history_selection_json="[]";q.disclosure_projection_json="{\"capabilities\":[],\"fields\":[]}";
 CHECK(ui_ai_real_propose(s,&q,run)==UI_STATUS_OK);CHECK(real_wait("\"phase\":3"));char *out=NULL;CHECK(ui_ai_model_get_disclosure(s,run,&out)==UI_STATUS_OK);CHECK(out&&strstr(out,"xs_read_discover")&&strstr(out,"instructions"));ui_ai_release(out);out=NULL;
 if(!wcscmp(kind,L"uh-close")){uh_callback=1;CHECK(ui_ai_model_get_disclosure(s,run,&out)!=UI_STATUS_OK);CHECK(!out&&uh_destroy_pending);CHECK(ui_ai_service_destroy(s)==UI_STATUS_OK);s=NULL;}
 else{uh_changed=1;CHECK(ui_ai_model_get_disclosure(s,run,&out)==UI_STATUS_VALIDATION_FAILED&&!out);CHECK(s->model_config->requests==0);uh_changed=0;CHECK(ui_ai_model_get_disclosure(s,run,&out)==UI_STATUS_OK);ui_ai_release(out);close_service();}
}
