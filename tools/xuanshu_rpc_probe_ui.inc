/* One redacted UI snapshot. Clipboard copying never consumes an input binding. */
static wchar_t *diagnostic_text;
static ui_status_t key_bind_status=UI_STATUS_PENDING,confirm_bind_status=UI_STATUS_PENDING;
static HWND task_buttons[3];
#ifdef UI_RPC_PROBE_TEST
static wchar_t *probe_test_clipboard;
#endif
static int probe_credential_ready(yyjson_val *o)
{
 yyjson_val *c=yyjson_obj_get(o,"credential");return yyjson_get_bool(yyjson_obj_get(o,"ready"))&&!yyjson_get_bool(yyjson_obj_get(o,"pending"))&&yyjson_get_bool(yyjson_obj_get(c,"credential_saved"))&&yyjson_get_bool(yyjson_obj_get(c,"config_saved"))&&yyjson_get_bool(yyjson_obj_get(c,"local_readable"))&&yyjson_get_uint(yyjson_obj_get(c,"saved_revision"))==yyjson_get_uint(yyjson_obj_get(o,"revision"));
}
static void probe_display(const wchar_t *text)
{
 if(diagnostic_text&&!wcscmp(diagnostic_text,text))return;wchar_t *copy=_wcsdup(text);if(!copy)return;
 DWORD first=0,last=0;SendMessageW(display,EM_GETSEL,(WPARAM)&first,(LPARAM)&last);LRESULT line=SendMessageW(display,EM_GETFIRSTVISIBLELINE,0,0);
 SendMessageW(display,WM_SETREDRAW,FALSE,0);SetWindowTextW(display,copy);SendMessageW(display,EM_SETSEL,first,last);SendMessageW(display,EM_LINESCROLL,0,line-SendMessageW(display,EM_GETFIRSTVISIBLELINE,0,0));SendMessageW(display,WM_SETREDRAW,TRUE,0);InvalidateRect(display,NULL,TRUE);free(diagnostic_text);diagnostic_text=copy;
}
static ui_status_t probe_copy(HWND owner)
{
 if(!diagnostic_text)return UI_STATUS_NOT_FOUND;
#ifdef UI_RPC_PROBE_TEST
 (void)owner;free(probe_test_clipboard);probe_test_clipboard=_wcsdup(diagnostic_text);return probe_test_clipboard?UI_STATUS_OK:UI_STATUS_OUT_OF_MEMORY;
#else
 SIZE_T bytes=(wcslen(diagnostic_text)+1)*sizeof(wchar_t);HGLOBAL memory=GlobalAlloc(GMEM_MOVEABLE,bytes);if(!memory)return UI_STATUS_OUT_OF_MEMORY;void *p=GlobalLock(memory);if(!p){GlobalFree(memory);return UI_STATUS_PLATFORM_ERROR;}memcpy(p,diagnostic_text,bytes);GlobalUnlock(memory);
 if(!OpenClipboard(owner)){GlobalFree(memory);return UI_STATUS_PLATFORM_ERROR;}if(!EmptyClipboard()||!SetClipboardData(CF_UNICODETEXT,memory)){CloseClipboard();GlobalFree(memory);return UI_STATUS_PLATFORM_ERROR;}CloseClipboard();return UI_STATUS_OK;
#endif
}
static const char *probe_credential_label(const char *stage)
{
 if(!stage||!strcmp(stage,"not_configured"))return "未配置（模型配置ready不代表Key可用）";
 if(!strcmp(stage,"prompting"))return "原生API Key窗口处理中";
 if(!strcmp(stage,"saving"))return "正在保存，尚未确认";
 if(!strcmp(stage,"readable"))return "保存成功，本地可读取；服务端未验证";
 if(!strcmp(stage,"local_readable"))return "本地可读取；本次没有保存确认，服务端未验证";
 if(!strcmp(stage,"cancelled"))return "已取消录入";
 if(!strcmp(stage,"prompt_failed"))return "原生窗口失败，请查看prompt_code";
 if(!strcmp(stage,"write_failed"))return "Windows凭据保存失败";
 if(!strcmp(stage,"config_failed"))return "模型配置保存失败";
 if(!strcmp(stage,"read_failed"))return "本地凭据读取失败";
 return "配置条件已变化，请查看credential诊断";
}
