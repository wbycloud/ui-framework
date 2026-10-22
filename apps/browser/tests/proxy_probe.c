/* Read-only WinHTTP configuration and identical bounded requests. No settings
 * are changed and no proxy host, credentials or query string are printed. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winhttp.h>
#include <stdio.h>
typedef struct observation { HANDLE closed;volatile LONG phase,resolved,connected;DWORD tls; } observation_t;
static void CALLBACK observe(HINTERNET handle,DWORD_PTR context,DWORD status,void *data,DWORD bytes)
{
    observation_t *o=(observation_t *)context;(void)handle;if(!o)return;
    if(status==WINHTTP_CALLBACK_STATUS_RESOLVING_NAME)o->phase=1;
    if(status==WINHTTP_CALLBACK_STATUS_NAME_RESOLVED)o->resolved=1;
    if(status==WINHTTP_CALLBACK_STATUS_CONNECTING_TO_SERVER)o->phase=2;
    if(status==WINHTTP_CALLBACK_STATUS_CONNECTED_TO_SERVER)o->connected=1;
    if(status==WINHTTP_CALLBACK_STATUS_SENDING_REQUEST)o->phase=3;
    if(status==WINHTTP_CALLBACK_STATUS_RECEIVING_RESPONSE)o->phase=4;
    if(status==WINHTTP_CALLBACK_STATUS_SECURE_FAILURE&&bytes>=sizeof(DWORD))o->tls=*(DWORD *)data;
    if(status==WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING)SetEvent(o->closed);
}
static void probe(DWORD access,const char *label)
{
    HINTERNET s=WinHttpOpen(L"browser-m01-probe",access,WINHTTP_NO_PROXY_NAME,WINHTTP_NO_PROXY_BYPASS,0),c=NULL,r=NULL;
    DWORD error=0,status=0,bytes=sizeof(status);ULONGLONG start=GetTickCount64();const char *phase="session";observation_t observed={0};int callback_set=0;observed.closed=CreateEventW(NULL,TRUE,FALSE,NULL);
    if(!s){error=GetLastError();goto done;}
    WinHttpSetTimeouts(s,3000,3000,3000,3000);phase="connect";
    c=WinHttpConnect(s,L"example.com",443,0);if(!c){error=GetLastError();goto done;}
    phase="send";r=WinHttpOpenRequest(c,L"GET",L"/",NULL,WINHTTP_NO_REFERER,WINHTTP_DEFAULT_ACCEPT_TYPES,WINHTTP_FLAG_SECURE);
    if(!r){error=GetLastError();goto done;}
    {DWORD_PTR context=(DWORD_PTR)&observed;if(!WinHttpSetOption(r,WINHTTP_OPTION_CONTEXT_VALUE,&context,sizeof(context))||WinHttpSetStatusCallback(r,observe,WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS,0)==WINHTTP_INVALID_STATUS_CALLBACK){error=GetLastError();goto done;}callback_set=1;}
    if(!WinHttpSendRequest(r,WINHTTP_NO_ADDITIONAL_HEADERS,0,NULL,0,0,(DWORD_PTR)&observed)){error=GetLastError();goto done;}
    phase="headers";if(!WinHttpReceiveResponse(r,NULL)){error=GetLastError();goto done;}
    if(!WinHttpQueryHeaders(r,WINHTTP_QUERY_STATUS_CODE|WINHTTP_QUERY_FLAG_NUMBER,WINHTTP_HEADER_NAME_BY_INDEX,&status,&bytes,WINHTTP_NO_HEADER_INDEX))error=GetLastError();
done:
    if(r){WinHttpCloseHandle(r);if(callback_set)WaitForSingleObject(observed.closed,INFINITE);}
    printf("mode=%s origin=https://example.com api=%s windows=%lu http=%lu elapsed_ms=%llu transport_stage=%ld dns_resolved=%ld tcp_connected=%ld tls_flags=%lu\n",label,phase,(unsigned long)error,(unsigned long)status,(unsigned long long)(GetTickCount64()-start),observed.phase,observed.resolved,observed.connected,(unsigned long)observed.tls);
    if(c)WinHttpCloseHandle(c);if(s)WinHttpCloseHandle(s);CloseHandle(observed.closed);
}
int main(void)
{
    WINHTTP_PROXY_INFO p={0};WINHTTP_CURRENT_USER_IE_PROXY_CONFIG u={0};
    if(WinHttpGetDefaultProxyConfiguration(&p)){printf("machine_access=%lu machine_proxy_configured=%d\n",(unsigned long)p.dwAccessType,p.lpszProxy!=NULL);GlobalFree(p.lpszProxy);GlobalFree(p.lpszProxyBypass);}
    else printf("machine_query_error=%lu\n",(unsigned long)GetLastError());
    if(WinHttpGetIEProxyConfigForCurrentUser(&u)){printf("user_auto_detect=%d user_pac_configured=%d user_proxy_configured=%d\n",u.fAutoDetect,u.lpszAutoConfigUrl!=NULL,u.lpszProxy!=NULL);GlobalFree(u.lpszAutoConfigUrl);GlobalFree(u.lpszProxy);GlobalFree(u.lpszProxyBypass);}
    else printf("user_query_error=%lu\n",(unsigned long)GetLastError());
    probe(WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,"legacy-default");probe(WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,"system-user-automatic");return 0;
}
