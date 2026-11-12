/* RP-A/B limited closeout. Private probes are linked only to the isolated DLL. */
__declspec(dllimport) ui_status_t ui_ai_model_fixture_result(ui_ai_service_t *,const char *,const char *,char **);
static void rc_never(uint64_t id,const ui_cap_target_t *target,const char *params,void *data)
{(void)id;(void)target;(void)params;(void)data;CHECK(0);}
static yyjson_doc *rc_output(void)
{
 yyjson_doc *body=yyjson_read(s->model_config->fixture_body,strlen(s->model_config->fixture_body),0),*output=NULL;
 yyjson_val *v,*a=body?yyjson_obj_get(yyjson_doc_get_root(body),"input"):NULL;size_t i,n;unsigned found=0;
 yyjson_arr_foreach(a,i,n,v){const char *type=yyjson_get_str(yyjson_obj_get(v,"type"));if(type&&!strcmp(type,"function_call_output")){const char *text=yyjson_get_str(yyjson_obj_get(v,"output"));CHECK(text!=NULL);if(text){++found;yyjson_doc_free(output);output=yyjson_read(text,strlen(text),0);}}}
 CHECK(found==1&&output!=NULL);yyjson_doc_free(body);return output;
}
static void rc_same_field(yyjson_val *actual,yyjson_val *expected,const char *field)
{
 yyjson_val *a=yyjson_obj_get(actual,field),*e=yyjson_obj_get(expected,field);CHECK(a&&e);char *aa=a?yyjson_val_write(a,0,NULL):NULL,*ee=e?yyjson_val_write(e,0,NULL):NULL;CHECK(aa&&ee&&!strcmp(aa,ee));free(aa);free(ee);
}
static void rc_discover(void)
{
 ui_capability_desc_t d={0};d.size=sizeof(d);d.version=1;d.capability_version=7;d.id="memory.hidden";d.name_utf8="Unselected read";d.description_utf8="Must not be sent";d.category="memory";d.effect="read";d.target_kind=UI_CAP_DOCUMENT;d.input_schema_json=d.output_schema_json="{\"type\":\"object\",\"additionalProperties\":false}";d.execute=rc_never;CHECK(ui_capability_register(apps[0].host,&d)==UI_STATUS_OK);
 responses(rp_discover,rp_text);CHECK(real_submit("closeout-discover",0)==UI_STATUS_OK);authorize();CHECK(real_wait("\"terminal\":true"));CHECK(strstr(state,"completed"));CHECK(s->model_config->requests==2);
 yyjson_doc *out=rc_output();yyjson_val *root=out?yyjson_doc_get_root(out):NULL,*items=yyjson_obj_get(root,"items");CHECK(yyjson_is_arr(items)&&yyjson_arr_size(items)==1);CHECK(yyjson_get_uint(yyjson_obj_get(root,"next"))==0);
 char *description=NULL;CHECK(ui_capability_describe(apps[0].host,"memory.read",&description)==UI_STATUS_OK);yyjson_doc *expected=description?yyjson_read(description,strlen(description),0):NULL;
 yyjson_val *row=yyjson_arr_get(items,0),*e=expected?yyjson_doc_get_root(expected):NULL;
 for(unsigned i=0;i<8;++i){const char *fields[]={"id","capability_version","description","input_schema","output_schema","effect","name","descriptor_hash"};rc_same_field(row,e,fields[i]);}
 size_t i,n;yyjson_val *v;yyjson_arr_foreach(items,i,n,v){const char *id=yyjson_get_str(yyjson_obj_get(v,"id")),*effect=yyjson_get_str(yyjson_obj_get(v,"effect"));CHECK(id&&!strcmp(id,"memory.read"));CHECK(effect&&!strcmp(effect,"read"));CHECK(id&&strcmp(id,"memory.hidden")&&strcmp(id,"memory.set")&&strcmp(id,"memory.external"));}
 yyjson_doc_free(out);yyjson_doc_free(expected);ui_capability_release(description);CHECK(value(0)==10);
 /* Real upstream empty category, with the same nonempty allowed capability list. */
 char *wire=_strdup(rp_discover);CHECK(wire!=NULL);if(wire){char *p;while((p=strstr(wire,"memory"))!=NULL)memcpy(p,"absent",6);responses(wire,rp_text);free(wire);CHECK(real_submit("closeout-empty",0)==UI_STATUS_OK);authorize();CHECK(real_wait("\"terminal\":true"));CHECK(strstr(state,"completed"));out=rc_output();items=out?yyjson_obj_get(yyjson_doc_get_root(out),"items"):NULL;CHECK(yyjson_is_arr(items)&&yyjson_arr_size(items)==0);yyjson_doc_free(out);}
 cr_idle();const char *invalid[]={"{}","{\"items\":[]}","null","1","[1]","[{}]","[{\"id\":\"memory.read\",\"effect\":\"read\"}]"};
 for(unsigned j=0;j<sizeof(invalid)/sizeof(invalid[0]);++j){char *projected=NULL;CHECK(ui_ai_model_fixture_result(s,run,invalid[j],&projected)==UI_STATUS_VALIDATION_FAILED);CHECK(projected==NULL);ui_ai_release(projected);}
 char *projected=NULL;CHECK(ui_ai_model_fixture_result(s,run,"[]",&projected)==UI_STATUS_OK);out=projected?yyjson_read(projected,strlen(projected),0):NULL;CHECK(out&&yyjson_arr_size(yyjson_obj_get(yyjson_doc_get_root(out),"items"))==0);yyjson_doc_free(out);ui_ai_release(projected);
 printf("RP closeout discovery verified actual function_call_output, descriptor fields and malformed input; failures=%u\n",failures);
}
static void rc_scan(void)
{
 char candidate[65],accepted[65];int slot,refs;unsigned occupied=0;
 responses(rp_text,NULL);CHECK(ui_ai_model_fixture(s,3,"1")==UI_STATUS_OK);CHECK(real_submit("closeout-cancelled",0)==UI_STATUS_OK);authorize();DWORD begin=GetTickCount();do{pump();if(s->model_config->requests)break;}while(GetTickCount()-begin<5000);CHECK(s->model_config->requests==1);CHECK(ui_ai_cancel(s,run)==UI_STATUS_OK);CHECK(ui_ai_model_fixture(s,3,"0")==UI_STATUS_OK);CHECK(real_wait("\"terminal\":true"));CHECK(strstr(state,"cancelled"));cr_idle();strcpy_s(candidate,65,run);slot=cr_slot(candidate);CHECK(slot>=0);
 /* Fill the real 64-slot cache. No private limit reduction, direct eviction or
    forged terminal record: every other record is accepted and persisted. */
 for(unsigned i=0;i<63;++i)cr_plain(3000+i,1);
 EnterCriticalSection(&s->lock);for(int i=0;i<AI_RUNS;++i)if(s->runs[i].id[0])++occupied;CHECK(s->runs[slot].cancel&&s->runs[slot].terminal&&s->runs[slot].refs==0);CHECK(s->runs[slot].model&&s->runs[slot].model->net.finished);LeaveCriticalSection(&s->lock);CHECK(occupied==64&&AI_RUNS==64);
 CHECK(ui_ai_model_fixture(s,10,candidate)==UI_STATUS_OK);begin=GetTickCount();int entered;do{entered=WaitForSingleObject(s->model_config->fixture_scan_enter,0)==WAIT_OBJECT_0;if(entered)break;(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}while(GetTickCount()-begin<5000);CHECK(entered);
 EnterCriticalSection(&s->lock);refs=s->runs[slot].refs;LeaveCriticalSection(&s->lock);CHECK(refs==1);
 ui_ai_task_request_t q={0};q.size=sizeof(q);q.version=1;q.client_request_id="closeout-pressure-new";q.input_utf8="Force production cache eviction while cleanup borrows a cancelled run";q.stub_plan_json="{\"steps\":[]}";CHECK(ui_ai_submit(s,&q,run)==UI_STATUS_OK);strcpy_s(accepted,65,run);CHECK(cr_slot(candidate)==slot);
 printf("RP closeout scan occupied=%u refs_during=%d retained=%d\n",occupied,refs,cr_slot(candidate)==slot);fflush(stdout);
 CHECK(ui_ai_model_fixture(s,11,"")==UI_STATUS_OK);cr_idle();CHECK(s->model_config->fixture_scan_done==1&&s->model_config->fixture_scan_same==1);CHECK(cr_slot(candidate)==slot);
 if(cr_slot(candidate)==slot){EnterCriticalSection(&s->lock);CHECK(s->runs[slot].refs==0&&s->runs[slot].model->net.finished);LeaveCriticalSection(&s->lock);}
 strcpy_s(run,65,accepted);CHECK(real_wait("\"terminal\":true"));CHECK(s->model_config->requests==1);CHECK(value(0)==10);
 cr_plain(4000,1);CHECK(cr_slot(candidate)<0);CHECK(s->model_config->fixture_scan_done==1);printf("RP closeout scan released once, oldest candidate evictable again; failures=%u\n",failures);
}
