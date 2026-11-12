/* Native secret entry stays inside the existing qualified-input method lease. */
typedef struct am_key_prompt {
 ui_ai_service_t *service;wchar_t *secret;HWND edit;DWORD error;int opened,submitted;
} am_key_prompt_t;
static int am_key_valid(const wchar_t *secret)
{size_t n=wcslen(secret);if(!n||n>512)return 0;for(size_t i=0;i<n;++i)if(secret[i]<33||secret[i]>126)return 0;return 1;}
static INT_PTR CALLBACK am_key_dialog(HWND dialog,UINT message,WPARAM wp,LPARAM lp)
{
 am_key_prompt_t *p=(am_key_prompt_t *)GetWindowLongPtrW(dialog,DWLP_USER);
 if(message==WM_INITDIALOG){p=(am_key_prompt_t *)lp;SetWindowLongPtrW(dialog,DWLP_USER,(LONG_PTR)p);RECT r;GetClientRect(dialog,&r);HINSTANCE module=GetModuleHandleW(NULL);
  HWND label=CreateWindowW(L"STATIC",L"DeepSeek API Key（仅保存到本用户的隔离凭据引用）",WS_CHILD|WS_VISIBLE,16,14,r.right-32,24,dialog,NULL,module,NULL);
  if(!label){p->error=GetLastError();if(!p->error)p->error=MAXDWORD;EndDialog(dialog,-1);return TRUE;}
  p->edit=CreateWindowExW(WS_EX_CLIENTEDGE,L"EDIT",L"",WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_PASSWORD|ES_AUTOHSCROLL,16,44,r.right-32,28,dialog,(HMENU)1001,module,NULL);
  if(!p->edit){p->error=GetLastError();if(!p->error)p->error=MAXDWORD;EndDialog(dialog,-1);return TRUE;}
  HWND note=CreateWindowW(L"STATIC",L"本地可读取不代表服务端已验证。密钥不会显示在诊断中。",WS_CHILD|WS_VISIBLE,16,82,r.right-32,40,dialog,NULL,module,NULL);
  if(!note){p->error=GetLastError();if(!p->error)p->error=MAXDWORD;EndDialog(dialog,-1);return TRUE;}
  HWND save=CreateWindowW(L"BUTTON",L"保存",WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_DEFPUSHBUTTON,r.right-192,r.bottom-44,80,28,dialog,(HMENU)IDOK,module,NULL);
  if(!save){p->error=GetLastError();if(!p->error)p->error=MAXDWORD;EndDialog(dialog,-1);return TRUE;}
  HWND cancel=CreateWindowW(L"BUTTON",L"取消",WS_CHILD|WS_VISIBLE|WS_TABSTOP,r.right-96,r.bottom-44,80,28,dialog,(HMENU)IDCANCEL,module,NULL);
  if(!cancel){p->error=GetLastError();if(!p->error)p->error=MAXDWORD;EndDialog(dialog,-1);return TRUE;}
  for(HWND child=GetWindow(dialog,GW_CHILD);child;child=GetWindow(child,GW_HWNDNEXT))SendMessageW(child,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),TRUE);
  SendMessageW(p->edit,EM_SETLIMITTEXT,512,0);p->opened=1;EnterCriticalSection(&p->service->lock);p->service->model_config->credential.opened=1;LeaveCriticalSection(&p->service->lock);
  if(!SetTimer(dialog,1,50,NULL)){p->error=GetLastError();if(!p->error)p->error=MAXDWORD;EndDialog(dialog,-1);return TRUE;}
#ifdef UI_AI_FIXTURE
  if(p->service->model_config->fixture_credentials==2)PostMessageW(dialog,WM_APP+29,0,0);
#endif
  SetFocus(p->edit);return FALSE;
 }
 if(!p)return FALSE;
#ifdef UI_AI_FIXTURE
 if(message==WM_APP+29&&p->service->model_config->fixture_credentials==2){am_config_t *c=p->service->model_config;wchar_t caption[96],button[24];GetWindowTextW(dialog,caption,96);GetWindowTextW(GetDlgItem(dialog,IDOK),button,24);
  c->fixture_dialog_checks=(wcsstr(caption,L"DeepSeek API Key")!=NULL)&&!wcscmp(button,L"保存")&&(GetWindowLongPtrW(p->edit,GWL_STYLE)&ES_PASSWORD)!=0;
  if(c->fixture_prompt_result==2){(void)ui_ai_service_request_close(p->service);if(ui_ai_service_destroy(p->service)!=UI_STATUS_PENDING)abort();SendMessageW(dialog,WM_COMMAND,IDCANCEL,0);}
  else{if(c->fixture_prompt_result>0)SetWindowTextW(p->edit,L"fixture-key-no-leak");SendMessageW(dialog,WM_COMMAND,c->fixture_prompt_result>0?IDOK:IDCANCEL,0);}return TRUE;
 }
#endif
 if(message==WM_TIMER&&p->service->close_requested){SendMessageW(dialog,WM_COMMAND,IDCANCEL,0);return TRUE;}
 if(message==WM_CLOSE||(message==WM_COMMAND&&LOWORD(wp)==IDCANCEL)){SetWindowTextW(p->edit,L"");SecureZeroMemory(p->secret,513*sizeof(wchar_t));EndDialog(dialog,0);return TRUE;}
 if(message==WM_COMMAND&&LOWORD(wp)==IDOK){if(GetWindowTextW(p->edit,p->secret,513)<=0||!am_key_valid(p->secret)){SecureZeroMemory(p->secret,513*sizeof(wchar_t));MessageBoxW(dialog,L"请输入1至512个非空格ASCII字符的API Key。",L"API Key格式",MB_OK|MB_ICONWARNING);return TRUE;}p->submitted=1;SetWindowTextW(p->edit,L"");EndDialog(dialog,1);return TRUE;}
 return FALSE;
}
static int am_key_prompt(ui_ai_service_t *s,HWND owner,wchar_t secret[513],DWORD *code)
{
 am_key_prompt_t p={0};p.service=s;p.secret=secret;
#ifdef UI_AI_FIXTURE
 am_config_t *c=s->model_config;if(c->fixture_credentials==1){
  EnterCriticalSection(&s->lock);c->credential.opened=c->fixture_prompt_result>=0;LeaveCriticalSection(&s->lock);
  if(c->fixture_prompt_reenter==1){(void)ui_ai_service_request_close(s);if(ui_ai_service_destroy(s)!=UI_STATUS_PENDING)abort();}
  if(c->fixture_prompt_reenter==2){EnterCriticalSection(&s->lock);++c->revision;am_config_hash(c);LeaveCriticalSection(&s->lock);}
  if(c->fixture_prompt_result>0){wcscpy_s(secret,513,L"fixture-key-no-leak");*code=ERROR_SUCCESS;return 1;}
  *code=c->fixture_prompt_result<0?ERROR_INVALID_WINDOW_HANDLE:ERROR_CANCELLED;return c->fixture_prompt_result<0?-1:0;
 }
#endif
 DWORD memory[128]={0};DLGTEMPLATE *t=(DLGTEMPLATE *)memory;t->style=WS_POPUP|WS_CAPTION|WS_SYSMENU|DS_MODALFRAME|DS_CENTER;t->cx=320;t->cy=112;
 wchar_t *tail=(wchar_t *)((BYTE *)t+sizeof(*t));*tail++=0;*tail++=0;wcscpy_s(tail,64,L"设置DeepSeek API Key");
 SetLastError(ERROR_SUCCESS);INT_PTR result=DialogBoxIndirectParamW(GetModuleHandleW(NULL),t,owner,am_key_dialog,(LPARAM)&p);DWORD error=GetLastError();
 *code=result<0?(p.error?p.error:(error?error:MAXDWORD)):result?ERROR_SUCCESS:ERROR_CANCELLED;return result<0?-1:p.submitted?1:0;
}
static void am_credential_json(yyjson_mut_doc *d,yyjson_mut_val *parent,const am_credential_status_t *c,uint64_t revision)
{
 yyjson_mut_val *o=yyjson_mut_obj(d);if(!o)return;yyjson_mut_obj_add_val(d,parent,"credential",o);
 yyjson_mut_obj_add_strcpy(d,o,"stage",c->readable&&c->revision!=revision?"stale":c->stage[0]?c->stage:"not_configured");yyjson_mut_obj_add_strcpy(d,o,"error",c->error);yyjson_mut_obj_add_strcpy(d,o,"last_entry",c->entry[0]?c->entry:"not_received");
 yyjson_mut_obj_add_bool(d,o,"dialog_open",c->dialog!=0);yyjson_mut_obj_add_bool(d,o,"window_opened",c->opened!=0);yyjson_mut_obj_add_bool(d,o,"submitted",c->submitted!=0);
 yyjson_mut_obj_add_bool(d,o,"credential_saved",c->written!=0);yyjson_mut_obj_add_bool(d,o,"config_saved",c->config_saved!=0);yyjson_mut_obj_add_bool(d,o,"local_readable",c->readable&&c->revision==revision);yyjson_mut_obj_add_uint(d,o,"saved_revision",c->revision);
 const char *names[]={"prompt_code","write_error","config_error","read_error"};DWORD codes[]={c->prompt_code,c->write_code,c->config_code,c->read_code};for(unsigned i=0;i<4;++i){if(c->observed&(1u<<i))yyjson_mut_obj_add_uint(d,o,names[i],codes[i]);else yyjson_mut_obj_add_null(d,o,names[i]);}
}
