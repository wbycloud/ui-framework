/* Single asynchronous request. Only the Service worker calls WinHTTP APIs. */
static void CALLBACK am_http(HINTERNET h,DWORD_PTR context,DWORD status,void *info,DWORD length)
{
 am_net_t *n=(am_net_t *)context;ui_ai_service_t *s;if(!n)return;s=n->service;(void)h;
 EnterCriticalSection(&s->lock);++n->callbacks;
#ifdef UI_AI_FIXTURE
 if(s->model_config&&s->model_config->fixture_cb_gate){HANDLE gate=s->model_config->fixture_cb_gate;SetEvent(s->model_config->fixture_cb_enter);LeaveCriticalSection(&s->lock);WaitForSingleObject(gate,INFINITE);EnterCriticalSection(&s->lock);}
#endif
 if(status==WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE)n->flags|=1;
 if(status==WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE)n->flags|=2;
 if(status==WINHTTP_CALLBACK_STATUS_READ_COMPLETE){n->received=length;n->flags|=4;}
 if(status==WINHTTP_CALLBACK_STATUS_REQUEST_ERROR){if(info&&length>=sizeof(WINHTTP_ASYNC_RESULT))n->transport_error=((WINHTTP_ASYNC_RESULT *)info)->dwError;n->error=1;}
 if(status==WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING)n->closed=1;
 --n->callbacks;(void)ai_start_locked(s);LeaveCriticalSection(&s->lock);
}
static void am_close(ui_ai_service_t *s,am_run_t *m)
{
 am_net_t *n=&m->net;HINTERNET h=NULL;
#ifdef UI_AI_FIXTURE
 if(s->model_config&&s->model_config->fixture_count&&s->model_config->fixture_hold)return;
#endif
 EnterCriticalSection(&s->lock);if(n->request&&!n->closing){n->closing=1;h=n->request;}LeaveCriticalSection(&s->lock);
 if(h){if(!n->callback_set)n->closed=1;(void)WinHttpCloseHandle(h);}
 if(!n->request)n->closed=1;
}
static int am_credential_read(am_config_t *c,wchar_t out[513],DWORD *error)
{
 wchar_t target[256];PCREDENTIALW cred=NULL;int ok=0;*error=ERROR_SUCCESS;
#ifdef UI_AI_FIXTURE
 if(c->fixture_credentials){strcpy_s(c->fixture_read_ref,128,c->ref);if(c->fixture_read_error){*error=c->fixture_read_error;return 0;}wcscpy_s(out,513,c->fixture_credential);if(!am_key_valid(out)){*error=ERROR_INVALID_DATA;return 0;}return 1;}
 if(c->fixture_count){wcscpy_s(out,513,c->fixture_key);if(!out[0])*error=ERROR_NOT_FOUND;return out[0]!=0;}
#endif
 if(!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,c->ref,-1,target,256)){*error=GetLastError();return 0;}
 if(!CredReadW(target,CRED_TYPE_GENERIC,0,&cred)){*error=GetLastError();return 0;}
 if(cred->CredentialBlobSize&&cred->CredentialBlobSize<=512*sizeof(wchar_t)&&!(cred->CredentialBlobSize%sizeof(wchar_t))){size_t z=cred->CredentialBlobSize/sizeof(wchar_t);memcpy(out,cred->CredentialBlob,cred->CredentialBlobSize);out[z]=0;ok=am_key_valid(out)&&wcslen(out)==z;}
 if(!ok)*error=ERROR_INVALID_DATA;SecureZeroMemory(cred->CredentialBlob,cred->CredentialBlobSize);CredFree(cred);return ok;
}
static int am_credential_write(am_config_t *c,DWORD *error)
{
 *error=ERROR_SUCCESS;
#ifdef UI_AI_FIXTURE
 if(c->fixture_credentials){strcpy_s(c->fixture_write_ref,128,c->ref);if(c->fixture_write_error){*error=c->fixture_write_error;return 0;}wcscpy_s(c->fixture_credential,513,c->secret);return 1;}
#endif
 wchar_t ref[256];if(!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,c->ref,-1,ref,256)){*error=GetLastError();return 0;}CREDENTIALW cred={0};cred.Type=CRED_TYPE_GENERIC;cred.TargetName=ref;cred.Persist=CRED_PERSIST_LOCAL_MACHINE;cred.CredentialBlob=(LPBYTE)c->secret;cred.CredentialBlobSize=(DWORD)(wcslen(c->secret)*sizeof(wchar_t));if(!CredWriteW(&cred,0)){*error=GetLastError();return 0;}return 1;
}
static int am_net_start(ui_ai_service_t *s,ai_run_t *r)
{
 am_run_t *m=r->model;am_net_t *n=&m->net;wchar_t secret[513]={0},header[600];DWORD disable=WINHTTP_DISABLE_REDIRECTS|WINHTTP_DISABLE_AUTHENTICATION|WINHTTP_DISABLE_COOKIES,autologon=WINHTTP_AUTOLOGON_SECURITY_LEVEL_HIGH;int ok=0;
 memset(n,0,sizeof(*n));n->service=s;n->begin=n->progress=ai_now(s);n->started=1;++r->refs;
 DWORD credential_error=ERROR_SUCCESS;int readable=am_credential_read(s->model_config,secret,&credential_error);EnterCriticalSection(&s->lock);am_credential_status_t *credential=&s->model_config->credential;credential->observed|=8;credential->read_code=credential_error;credential->readable=readable;if(readable){credential->revision=s->model_config->revision;credential->error[0]=0;strcpy_s(credential->stage,24,credential->written&&credential->config_saved?"readable":"local_readable");}if(!readable){strcpy_s(credential->stage,24,"read_failed");strcpy_s(credential->error,64,"credential_read_failed");}LeaveCriticalSection(&s->lock);
 if(!readable){strcpy_s(m->error,96,"credential_unavailable");goto done;}
 if(swprintf_s(header,600,L"Authorization: Bearer %s\r\nContent-Type: application/json\r\nAccept: text/event-stream\r\n",secret)<0)goto done;
#ifdef UI_AI_FIXTURE
 if(s->model_config->fixture_count){if(!av_before_send(s,r,1))goto done;n->sent=1;++s->model_config->requests;strcpy_s(s->model_config->fixture_body,AM_REQUEST+1,m->body);av_send_ack(s,r);SecureZeroMemory(secret,sizeof(secret));SecureZeroMemory(header,sizeof(header));return 1;}
#endif
 n->session=WinHttpOpen(L"ui-framework XuanShu Responses/1",WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,WINHTTP_NO_PROXY_NAME,WINHTTP_NO_PROXY_BYPASS,WINHTTP_FLAG_ASYNC);
 if(!n->session||!WinHttpSetTimeouts(n->session,10000,10000,30000,30000))goto done;
 {WINHTTP_FAILED_CONNECTION_RETRIES retries={0};BOOL isolated=TRUE;if(!WinHttpSetOption(n->session,WINHTTP_OPTION_FAILED_CONNECTION_RETRIES,&retries,sizeof(retries))||!WinHttpSetOption(n->session,WINHTTP_OPTION_DISABLE_GLOBAL_POOLING,&isolated,sizeof(isolated))){strcpy_s(m->error,96,"no_retry_transport_unavailable");goto done;}}
 n->connection=WinHttpConnect(n->session,L"api.deepseek.com",443,0);if(!n->connection)goto done;
 n->request=WinHttpOpenRequest(n->connection,L"POST",L"/responses",NULL,WINHTTP_NO_REFERER,WINHTTP_DEFAULT_ACCEPT_TYPES,WINHTTP_FLAG_SECURE);if(!n->request)goto done;
 {DWORD_PTR context=(DWORD_PTR)n;if(!WinHttpSetOption(n->request,WINHTTP_OPTION_CONTEXT_VALUE,&context,sizeof(context)))goto done;}
 if(WinHttpSetStatusCallback(n->request,am_http,WINHTTP_CALLBACK_FLAG_ALL_COMPLETIONS|WINHTTP_CALLBACK_FLAG_HANDLES,0)==WINHTTP_INVALID_STATUS_CALLBACK)goto done;n->callback_set=1;
 if(!WinHttpSetOption(n->request,WINHTTP_OPTION_DISABLE_FEATURE,&disable,sizeof(disable))||!WinHttpSetOption(n->request,WINHTTP_OPTION_AUTOLOGON_POLICY,&autologon,sizeof(autologon)))goto done;
 if(!av_before_send(s,r,0))goto done;n->sent=1;ok=WinHttpSendRequest(n->request,header,(DWORD)-1,m->body,(DWORD)strlen(m->body),(DWORD)strlen(m->body),(DWORD_PTR)n)!=0;av_send_ack(s,r);
done:SecureZeroMemory(secret,sizeof(secret));SecureZeroMemory(header,sizeof(header));if(!ok){if(!m->error[0])strcpy_s(m->error,96,"network_send_unknown");n->error=1;am_close(s,m);}return ok;
}
/* Returns progress; finished means HANDLE_CLOSING and all callbacks exited. */
static int am_net_poll(ui_ai_service_t *s,ai_run_t *r)
{
 am_run_t *m=r->model;am_net_t *n=&m->net;int flags,progress=0;DWORD got;
 if(!n->started||n->finished)return 0;if(ai_now(s)-n->begin>=120000||ai_now(s)-n->progress>=30000){n->error=1;strcpy_s(m->error,96,"network_timeout_unknown");}
#ifdef UI_AI_FIXTURE
 if(s->model_config->fixture_count){am_config_t *c=s->model_config;if(c->fixture_batch_mode==2&&r->model->validation_kind==3&&!r->cancel)return 0;
  if(c->fixture_error){n->error=1;n->transport_error=c->fixture_error;}if(!c->fixture_hold&&!n->closed){if(!r->cancel&&!s->store_failed&&!n->error){unsigned at=c->fixture_batch_mode&&r->model->validation_kind==3?c->fixture_count-1:m->ordinal-1;if(c->fixture_http){n->http=(DWORD)c->fixture_http;n->error=1;snprintf(m->error,96,"http_%lu",(unsigned long)n->http);}else{
   const char *wire=at<c->fixture_count?c->fixture[at]:NULL;char *observed=NULL;
   if(c->fixture_reply){size_t capacity=16384;observed=calloc(1,capacity);
    wire=observed&&c->fixture_reply(c->fixture_reply_data,m->ordinal,m->body,observed,capacity)&&memchr(observed,0,capacity)?observed:NULL;}
   if(!wire||!am_stream_feed(m->stream,wire,strlen(wire))||!am_stream_eof(m->stream)){n->error=1;strcpy_s(m->error,96,m->stream->error[0]?m->stream->error:"truncated_stream");}free(observed);}}
   n->closed=1;}else if(r->cancel||s->store_failed)strcpy_s(m->error,96,"cancellation_stalled");
 }else
#endif
 {
  EnterCriticalSection(&s->lock);flags=n->flags;n->flags=0;got=n->received;LeaveCriticalSection(&s->lock);
  if(ai_now(s)-n->begin>=120000||ai_now(s)-n->progress>=30000){n->error=1;strcpy_s(m->error,96,"network_timeout_unknown");}
  if(r->cancel||s->store_failed||n->error)am_close(s,m);
  else if(flags&1){if(!WinHttpReceiveResponse(n->request,NULL))n->error=1;progress=1;}
  else if(flags&2){DWORD size=sizeof(n->http);wchar_t content[128];DWORD length=sizeof(content);if(!WinHttpQueryHeaders(n->request,WINHTTP_QUERY_STATUS_CODE|WINHTTP_QUERY_FLAG_NUMBER,WINHTTP_HEADER_NAME_BY_INDEX,&n->http,&size,WINHTTP_NO_HEADER_INDEX)||n->http!=200){n->error=1;snprintf(m->error,96,"http_%lu",(unsigned long)n->http);}else if(!WinHttpQueryHeaders(n->request,WINHTTP_QUERY_CONTENT_TYPE,WINHTTP_HEADER_NAME_BY_INDEX,content,&length,WINHTTP_NO_HEADER_INDEX)||_wcsnicmp(content,L"text/event-stream",17)||(content[17]&&content[17]!=L';'&&content[17]!=L' '))n->error=1;else if(!WinHttpReadData(n->request,n->block,sizeof(n->block),NULL))n->error=1;progress=1;}
  else if(flags&4){if(!got){if(!am_stream_eof(m->stream))n->error=1;am_close(s,m);}else{uint64_t before=m->stream->sequence;if(!am_stream_feed(m->stream,n->block,got)){n->error=1;strcpy_s(m->error,96,m->stream->error);}else{if(before!=m->stream->sequence)n->progress=ai_now(s);if(!WinHttpReadData(n->request,n->block,sizeof(n->block),NULL))n->error=1;}}progress=1;}
  if(n->error)am_close(s,m);
 }
 if(n->error&&n->transport_error&&!m->error[0]){if(n->transport_error==ERROR_WINHTTP_SECURE_FAILURE)strcpy_s(m->error,96,"tls_failure_unknown");else if(n->transport_error==ERROR_WINHTTP_CANNOT_CONNECT||n->transport_error==ERROR_WINHTTP_LOGIN_FAILURE)strcpy_s(m->error,96,"connection_or_proxy_failure_unknown");else strcpy_s(m->error,96,"transport_failure_unknown");}
 EnterCriticalSection(&s->lock);if(n->closed&&!n->callbacks){n->response_complete=!n->error&&m->stream&&m->stream->complete;n->finished=1;n->request=NULL;--r->refs;progress=1;}LeaveCriticalSection(&s->lock);
 if(n->finished){av_network_end(s,r);if(n->connection)WinHttpCloseHandle(n->connection);if(n->session)WinHttpCloseHandle(n->session);n->connection=n->session=NULL;if(n->error&&!m->error[0])strcpy_s(m->error,96,"network_delivery_unknown");}return progress;
}

#ifdef UI_AI_FIXTURE
static DWORD WINAPI am_fixture_callback(void *data){am_http(NULL,(DWORD_PTR)data,WINHTTP_CALLBACK_STATUS_READ_COMPLETE,NULL,0);return 0;}
#endif
