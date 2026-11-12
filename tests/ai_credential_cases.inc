/* No user credential access. The native form driver uses synthetic input only
 * after private fixture admission; it is not a physical input positive. */
static yyjson_doc *ck_snapshot(void)
{char *json=NULL;CHECK(ui_ai_model_get_state(s,&json)==UI_STATUS_OK);CHECK(json&&!strstr(json,"fixture-key-no-leak"));yyjson_doc *d=json?yyjson_read(json,strlen(json),0):NULL;ui_ai_release(json);return d;}
static yyjson_val *ck_credential(yyjson_doc *d)
{return d?yyjson_obj_get(yyjson_doc_get_root(d),"credential"):NULL;}
static int ck_wait(const char *stage)
{DWORD begin=GetTickCount();do{pump();yyjson_doc *d=ck_snapshot();const char *value=yyjson_get_str(yyjson_obj_get(ck_credential(d),"stage"));int found=value&&!strcmp(value,stage);yyjson_doc_free(d);if(found)return 1;}while(GetTickCount()-begin<5000);return 0;}
static void ck_case(const wchar_t *mode)
{
 yyjson_doc *d=ck_snapshot();yyjson_val *o=ck_credential(d);CHECK(yyjson_is_obj(o));const char *stage=yyjson_get_str(yyjson_obj_get(o,"stage"));CHECK(stage&&!strcmp(stage,"not_configured"));CHECK(!yyjson_get_bool(yyjson_obj_get(o,"local_readable")));CHECK(yyjson_is_null(yyjson_obj_get(o,"write_error")));yyjson_doc_free(d);
 if(!wcscmp(mode,L"ck-state"))return;
 if(!wcscmp(mode,L"ck-request-read")){responses(rp_text,NULL);CHECK(real_submit("credential-local-read",0)==UI_STATUS_OK);authorize();CHECK(real_wait("\"terminal\":true"));CHECK(strstr(state,"completed"));d=ck_snapshot();o=ck_credential(d);CHECK(yyjson_get_bool(yyjson_obj_get(o,"local_readable")));CHECK(!strcmp(yyjson_get_str(yyjson_obj_get(o,"stage")),"local_readable"));CHECK(!yyjson_get_bool(yyjson_obj_get(o,"credential_saved")));yyjson_doc_free(d);return;}

 am_config_t *c=s->model_config;uint64_t revision=c->revision;CHECK(ui_ai_model_fixture(s,20,"1")==UI_STATUS_OK);
 const char *expected="readable";int native=!wcsncmp(mode,L"ck-dialog-",10);
 if(!wcscmp(mode,L"ck-cancel")){c->fixture_prompt_result=0;expected="cancelled";}
 if(!wcscmp(mode,L"ck-prompt-fail")){c->fixture_prompt_result=-1;expected="prompt_failed";}
 if(!wcscmp(mode,L"ck-write-fail")){c->fixture_write_error=ERROR_ACCESS_DENIED;expected="write_failed";}
 if(!wcscmp(mode,L"ck-config-fail")){c->fixture_config_error=ERROR_WRITE_FAULT;expected="config_failed";}
 if(!wcscmp(mode,L"ck-read-fail")){c->fixture_read_error=ERROR_NOT_FOUND;expected="read_failed";}
 if(!wcscmp(mode,L"ck-reenter-config")){c->fixture_prompt_reenter=2;expected="stale";}
 if(!wcscmp(mode,L"ck-reenter-close")){c->fixture_prompt_reenter=1;expected="stale";}
 if(native){c->fixture_credentials=2;c->fixture_prompt_result=!wcscmp(mode,L"ck-dialog-cancel")?0:!wcscmp(mode,L"ck-dialog-close")?2:1;if(!c->fixture_prompt_result||c->fixture_prompt_result==2)expected="cancelled";}
 int queued=!strcmp(expected,"readable")||!strcmp(expected,"write_failed")||!strcmp(expected,"config_failed")||!strcmp(expected,"read_failed");
 if(queued)CHECK(ui_ai_model_fixture(s,22,"1")==UI_STATUS_OK);
 ui_status_t accepted=ui_ai_model_fixture(s,21,"");CHECK(accepted==(queued?UI_STATUS_OK:UI_STATUS_PERMISSION_DENIED));CHECK(s->methods==0);
 if(queued){d=ck_snapshot();o=ck_credential(d);CHECK(!strcmp(yyjson_get_str(yyjson_obj_get(o,"stage")),"saving"));CHECK(!yyjson_get_bool(yyjson_obj_get(o,"credential_saved")));CHECK(!yyjson_get_bool(yyjson_obj_get(o,"local_readable")));CHECK(yyjson_is_null(yyjson_obj_get(o,"read_error")));CHECK(c->revision==revision);yyjson_doc_free(d);CHECK(ui_ai_model_fixture(s,22,"0")==UI_STATUS_OK);}
 CHECK(ck_wait(expected));d=ck_snapshot();o=ck_credential(d);
 if(!strcmp(expected,"readable")){CHECK(yyjson_get_bool(yyjson_obj_get(o,"credential_saved")));CHECK(yyjson_get_bool(yyjson_obj_get(o,"config_saved")));CHECK(yyjson_get_bool(yyjson_obj_get(o,"local_readable")));CHECK(yyjson_get_uint(yyjson_obj_get(o,"saved_revision"))==revision+1);CHECK(c->revision==revision+1);CHECK(!strcmp(c->fixture_write_ref,c->ref)&&!strcmp(c->fixture_read_ref,c->ref));CHECK(yyjson_get_uint(yyjson_obj_get(o,"read_error"))==ERROR_SUCCESS);}
 else{CHECK(!yyjson_get_bool(yyjson_obj_get(o,"local_readable")));if(!strcmp(expected,"write_failed"))CHECK(yyjson_get_uint(yyjson_obj_get(o,"write_error"))==ERROR_ACCESS_DENIED);if(!strcmp(expected,"config_failed")){CHECK(yyjson_get_bool(yyjson_obj_get(o,"credential_saved")));CHECK(!yyjson_get_bool(yyjson_obj_get(o,"config_saved")));CHECK(yyjson_get_uint(yyjson_obj_get(o,"config_error"))==ERROR_WRITE_FAULT);}if(!strcmp(expected,"read_failed")){CHECK(yyjson_get_bool(yyjson_obj_get(o,"config_saved")));CHECK(yyjson_get_uint(yyjson_obj_get(o,"read_error"))==ERROR_NOT_FOUND);}if(!strcmp(expected,"prompt_failed"))CHECK(yyjson_get_uint(yyjson_obj_get(o,"prompt_code"))==ERROR_INVALID_WINDOW_HANDLE);}
 yyjson_doc_free(d);CHECK(c->secret[0]==0);if(native)CHECK(c->fixture_dialog_checks);CHECK(c->requests==0);CHECK(!secret_leaked);
 printf("credential offline %ls stage=%s; memory store and isolated form only; no Provider access\n",mode,expected);
}
