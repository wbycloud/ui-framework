#define WIN32_LEAN_AND_MEAN
#include "browser_net.h"
#include "browser_encoding.h"
#include "browser_image.h"
#include <winhttp.h>
#include <objbase.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct net_job { uint64_t navigation,resource;browser_resource_kind_t kind;char url[BROWSER_URL_CAP];struct net_job *next; } net_job_t;
struct browser_net {
    ui_workspace_t *workspace;uint64_t instance;
    browser_policy_t policy;
    CRITICAL_SECTION lock;
    HANDLE thread,wake,cancel,quit;
    net_job_t *head,*tail;
    uint64_t navigation;
    size_t transfer_bytes,decoded_bytes;
    unsigned resource_count;
    volatile LONG busy,closing,post_error,decoding;
};
typedef struct exchange {
    HANDLE step,closed;
    volatile LONG active;
    DWORD status,error,available,read;
    volatile LONG phase;
    DWORD tls;
} exchange_t;
static volatile LONG live_threads,live_callbacks;
long browser_net_live_threads(void){return InterlockedCompareExchange(&live_threads,0,0);}
long browser_net_live_callbacks(void){return InterlockedCompareExchange(&live_callbacks,0,0);}
static wchar_t *wide(const char *s)
{int count=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,s,-1,NULL,0);wchar_t *out=count?(wchar_t *)malloc((size_t)count*sizeof(wchar_t)):NULL;
 if(out&&!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,s,-1,out,count)){free(out);out=NULL;}return out;}
static void CALLBACK http_status(HINTERNET request,DWORD_PTR context,DWORD status,void *info,DWORD size)
{
    exchange_t *e=(exchange_t *)context;(void)request;
    if(!e)return;InterlockedIncrement(&e->active);InterlockedIncrement(&live_callbacks);
    if(status==WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING)SetEvent(e->closed);
    else{
        if(InterlockedCompareExchange(&e->phase,0,0)<BROWSER_PHASE_HEADERS){
            if(status==WINHTTP_CALLBACK_STATUS_RESOLVING_NAME)InterlockedExchange(&e->phase,BROWSER_PHASE_DNS);
            if(status==WINHTTP_CALLBACK_STATUS_CONNECTING_TO_SERVER)InterlockedExchange(&e->phase,BROWSER_PHASE_CONNECT);
            if(status==WINHTTP_CALLBACK_STATUS_SENDING_REQUEST)InterlockedExchange(&e->phase,BROWSER_PHASE_SEND);}
        if(status==WINHTTP_CALLBACK_STATUS_SECURE_FAILURE&&size>=sizeof(DWORD)){e->tls=*(DWORD *)info;InterlockedExchange(&e->phase,BROWSER_PHASE_CONNECT);}
        if(status==WINHTTP_CALLBACK_STATUS_REQUEST_ERROR&&size>=sizeof(WINHTTP_ASYNC_RESULT))e->error=((WINHTTP_ASYNC_RESULT *)info)->dwError;
        if(status==WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE&&size>=sizeof(DWORD))e->available=*(DWORD *)info;
        if(status==WINHTTP_CALLBACK_STATUS_READ_COMPLETE)e->read=size;
        if(status==WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE||status==WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE||status==WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE||status==WINHTTP_CALLBACK_STATUS_READ_COMPLETE||status==WINHTTP_CALLBACK_STATUS_REQUEST_ERROR){e->status=status;SetEvent(e->step);}
    }
    InterlockedDecrement(&live_callbacks);InterlockedDecrement(&e->active);
}
static int wait_step(browser_net_t *n,exchange_t *e,DWORD expected,ULONGLONG deadline)
{
    ULONGLONG now=GetTickCount64();DWORD remaining=now>=deadline?0:(DWORD)(deadline-now);HANDLE handles[]={n->cancel,e->step};DWORD result=WaitForMultipleObjects(2,handles,FALSE,remaining<3000?remaining:3000);
    if(result!=WAIT_OBJECT_0+1){SetLastError(result==WAIT_OBJECT_0?ERROR_CANCELLED:ERROR_TIMEOUT);return 0;}
    if(e->error){SetLastError(e->error);return 0;}if(e->status!=expected){SetLastError(ERROR_INVALID_DATA);return 0;}return 1;
}
static void close_exchange(HINTERNET request,exchange_t *e)
{
    if(request){(void)WinHttpCloseHandle(request);(void)WaitForSingleObject(e->closed,INFINITE);
        while(InterlockedCompareExchange(&e->active,0,0))SwitchToThread();}
    if(e->step)CloseHandle(e->step);if(e->closed)CloseHandle(e->closed);
}
static int header_utf8(HINTERNET request,DWORD query,char *out,size_t capacity)
{
    wchar_t buffer[4096];DWORD bytes=sizeof(buffer);out[0]=0;
    if(!WinHttpQueryHeaders(request,query,WINHTTP_HEADER_NAME_BY_INDEX,buffer,&bytes,WINHTTP_NO_HEADER_INDEX))return GetLastError()==ERROR_WINHTTP_HEADER_NOT_FOUND;
    return WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,buffer,-1,out,(int)capacity,NULL,NULL)>0;
}
static int current(browser_net_t *n,uint64_t generation)
{int ok;EnterCriticalSection(&n->lock);ok=!n->closing&&n->navigation==generation;LeaveCriticalSection(&n->lock);return ok;}
static int allowed(browser_net_t *n,const browser_url_t *url)
{int ok;EnterCriticalSection(&n->lock);ok=browser_url_allowed(&n->policy,url);LeaveCriticalSection(&n->lock);return ok;}
static void redirect_trace(browser_reply_t *reply,const browser_url_t *from,const browser_url_t *to)
{
    size_t used=strlen(reply->redirect_trace);
    snprintf(reply->redirect_trace+used,sizeof(reply->redirect_trace)-used,"%u: %.170s%s -> %.170s%s\n",reply->redirects,
        from->origin,strlen(from->origin)>170?"[长来源省略]":"",to?to->origin:"[无效、特殊协议或含凭据的目标已隐藏]",to&&strlen(to->origin)>170?"[长来源省略]":"");
}
static int fetch(browser_net_t *n,const net_job_t *job,browser_reply_t *reply,unsigned char **body,size_t *length)
{
    browser_url_t url;char target[BROWSER_URL_CAP];unsigned redirects;int success=0;ULONGLONG deadline=GetTickCount64()+15000;
    strcpy(target,job->url);*body=NULL;*length=0;
    for(redirects=0;redirects<=8&&current(n,job->navigation);++redirects){
        HINTERNET session=NULL,connection=NULL,request=NULL;wchar_t *hostname=NULL,*path=NULL;exchange_t e={0};DWORD flags,status_size=sizeof(DWORD),disable,autologon;
        char location[BROWSER_URL_CAP],content_type[256],disposition[256];unsigned char *data=NULL;size_t used=0,capacity=0;
        DWORD error=ERROR_INVALID_DATA;int again=0,callback_installed=0;
        if(GetTickCount64()>deadline){reply->error=ERROR_TIMEOUT;break;}
        {browser_url_result_t result=browser_url_parse_ex(NULL,target,&url);
         if(result!=BROWSER_URL_OK||!allowed(n,&url)){reply->error=ERROR_ACCESS_DENIED;reply->category=(uint32_t)(result!=BROWSER_URL_OK?browser_url_failure(result,0):BROWSER_FAILURE_ORIGIN);break;}}
        strcpy(reply->url,url.text);hostname=wide(url.host);path=wide(url.path);if(!hostname||!path)goto done;
        /* Lexbor serializes IPv6 hosts with brackets; WinHttpConnect takes the address. */
        if(hostname[0]==L'['){size_t len=wcslen(hostname);memmove(hostname,hostname+1,len*sizeof(wchar_t));if(len>1)hostname[len-2]=0;}
        e.step=CreateEventW(NULL,FALSE,FALSE,NULL);e.closed=CreateEventW(NULL,TRUE,FALSE,NULL);if(!e.step||!e.closed)goto done;
        e.phase=BROWSER_PHASE_SESSION;
        session=WinHttpOpen(L"ui-framework restricted browser/0.2",WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,WINHTTP_NO_PROXY_NAME,WINHTTP_NO_PROXY_BYPASS,WINHTTP_FLAG_ASYNC);
        if(!session||!WinHttpSetTimeouts(session,3000,3000,3000,3000))goto done;
        connection=WinHttpConnect(session,hostname,url.port,0);if(!connection)goto done;
        flags=url.secure?WINHTTP_FLAG_SECURE:0;request=WinHttpOpenRequest(connection,L"GET",path,NULL,WINHTTP_NO_REFERER,WINHTTP_DEFAULT_ACCEPT_TYPES,flags);if(!request)goto done;
        {DWORD_PTR context=(DWORD_PTR)&e;if(!WinHttpSetOption(request,WINHTTP_OPTION_CONTEXT_VALUE,&context,sizeof(context)))goto done;}
        if(WinHttpSetStatusCallback(request,http_status,WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS,0)==WINHTTP_INVALID_STATUS_CALLBACK)goto done;
        callback_installed=1;
        disable=WINHTTP_DISABLE_REDIRECTS|WINHTTP_DISABLE_COOKIES|WINHTTP_DISABLE_AUTHENTICATION;
        autologon=WINHTTP_AUTOLOGON_SECURITY_LEVEL_HIGH;
        if(!WinHttpSetOption(request,WINHTTP_OPTION_DISABLE_FEATURE,&disable,sizeof(disable))||!WinHttpSetOption(request,WINHTTP_OPTION_AUTOLOGON_POLICY,&autologon,sizeof(autologon)))goto done;
        {DWORD compression=WINHTTP_DECOMPRESSION_FLAG_GZIP|WINHTTP_DECOMPRESSION_FLAG_DEFLATE;if(!WinHttpSetOption(request,WINHTTP_OPTION_DECOMPRESSION,&compression,sizeof(compression)))goto done;}
        if(!current(n,job->navigation)){error=ERROR_CANCELLED;goto done;}e.phase=BROWSER_PHASE_SEND;
        if(!WinHttpSendRequest(request,L"Accept: text/html, text/css, image/png, image/jpeg, image/gif, image/bmp\r\n",(DWORD)-1,WINHTTP_NO_REQUEST_DATA,0,0,(DWORD_PTR)&e)||!wait_step(n,&e,WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE,deadline))goto done;
        e.phase=BROWSER_PHASE_HEADERS;
        if(!WinHttpReceiveResponse(request,NULL)||!wait_step(n,&e,WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE,deadline))goto done;
        if(!WinHttpQueryHeaders(request,WINHTTP_QUERY_STATUS_CODE|WINHTTP_QUERY_FLAG_NUMBER,WINHTTP_HEADER_NAME_BY_INDEX,&reply->http_status,&status_size,WINHTTP_NO_HEADER_INDEX))goto done;
        if(reply->http_status==301||reply->http_status==302||reply->http_status==303||reply->http_status==307||reply->http_status==308){
            browser_url_t next;browser_url_result_t result;++reply->redirects;
            if(!header_utf8(request,WINHTTP_QUERY_LOCATION,location,sizeof(location))||!*location){reply->category=BROWSER_FAILURE_REDIRECT_URL;error=ERROR_WINHTTP_INVALID_SERVER_RESPONSE;goto done;}
            result=browser_url_parse_ex(url.text,location,&next);redirect_trace(reply,&url,result==BROWSER_URL_OK?&next:NULL);
            if(redirects==8){reply->category=BROWSER_FAILURE_REDIRECT_LIMIT;error=ERROR_WINHTTP_REDIRECT_FAILED;goto done;}
            if(result!=BROWSER_URL_OK||!allowed(n,&next)){reply->category=(uint32_t)(result!=BROWSER_URL_OK?browser_url_failure(result,1):BROWSER_FAILURE_REDIRECT_ORIGIN);error=ERROR_ACCESS_DENIED;goto done;}
            strcpy(target,next.text);again=1;goto done;}
        if(reply->http_status<200||reply->http_status>=300){reply->category=BROWSER_FAILURE_HTTP;error=ERROR_WINHTTP_INVALID_SERVER_RESPONSE;goto done;}
        if(!header_utf8(request,WINHTTP_QUERY_CONTENT_TYPE,content_type,sizeof(content_type))||!header_utf8(request,WINHTTP_QUERY_CONTENT_DISPOSITION,disposition,sizeof(disposition)))goto done;
        if(*disposition&&!_strnicmp(disposition,"attachment",10)){reply->category=BROWSER_FAILURE_ATTACHMENT;error=ERROR_ACCESS_DENIED;goto done;}
        if((job->kind==BROWSER_HTML&&_strnicmp(content_type,"text/html",9))||(job->kind==BROWSER_CSS&&_strnicmp(content_type,"text/css",8))||(job->kind==BROWSER_IMAGE&&_strnicmp(content_type,"image/",6))){reply->category=BROWSER_FAILURE_TYPE;error=ERROR_UNSUPPORTED_TYPE;goto done;}
        for(;;){DWORD available,read;size_t limit=job->kind==BROWSER_HTML?BROWSER_HTML_LIMIT:job->kind==BROWSER_CSS?65536u:2u*1024u*1024u;
            if(GetTickCount64()>deadline){error=ERROR_TIMEOUT;goto done;}
            if(!current(n,job->navigation)){error=ERROR_CANCELLED;goto done;}
            e.phase=BROWSER_PHASE_READ;
            if(!WinHttpQueryDataAvailable(request,NULL)||!wait_step(n,&e,WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE,deadline))goto done;
            available=e.available;if(!available)break;if(available>limit-used){error=ERROR_FILE_TOO_LARGE;goto done;}
            if(used+available+1>capacity){unsigned char *p=(unsigned char *)realloc(data,used+available+1);if(!p){error=ERROR_OUTOFMEMORY;goto done;}data=p;capacity=used+available+1;}
            if(!WinHttpReadData(request,data+used,available,NULL)||!wait_step(n,&e,WINHTTP_CALLBACK_STATUS_READ_COMPLETE,deadline))goto done;read=e.read;
            if(read>available){error=ERROR_INVALID_DATA;goto done;}used+=read;
            if(!read)break;
        }
        if(!data){data=(unsigned char *)calloc(1,1);if(!data){error=ERROR_OUTOFMEMORY;goto done;}}data[used]=0;
        if(n->transfer_bytes>BROWSER_TRANSFER_LIMIT-used){error=ERROR_FILE_TOO_LARGE;goto done;}n->transfer_bytes+=used;
        if(job->kind!=BROWSER_IMAGE){unsigned char *utf8=NULL;size_t utf8_size=0;
            e.phase=BROWSER_PHASE_ENCODING;
            reply->encoding_status=(uint32_t)browser_encoding_convert(data,used,content_type,job->kind==BROWSER_HTML,job->kind==BROWSER_HTML?BROWSER_HTML_LIMIT:65536,&utf8,&utf8_size);
            if(reply->encoding_status!=BROWSER_ENCODING_OK){reply->category=reply->encoding_status==BROWSER_ENCODING_LIMIT||reply->encoding_status==BROWSER_ENCODING_MEMORY?BROWSER_FAILURE_BUDGET:BROWSER_FAILURE_ENCODING;error=reply->encoding_status==BROWSER_ENCODING_LIMIT?ERROR_FILE_TOO_LARGE:reply->encoding_status==BROWSER_ENCODING_MEMORY?ERROR_NOT_ENOUGH_MEMORY:ERROR_NO_UNICODE_TRANSLATION;goto done;}
            free(data);data=utf8;used=utf8_size;}
        *body=data;*length=used;data=NULL;success=1;error=0;
done:
        if(!success&&!again&&error==ERROR_INVALID_DATA){DWORD last=GetLastError();if(last)error=last;}
        if(request){/* Once callback/context are installed, HANDLE_CLOSING is the final notification. */
            if(callback_installed)close_exchange(request,&e);
            else{WinHttpCloseHandle(request);if(e.step)CloseHandle(e.step);if(e.closed)CloseHandle(e.closed);}}
        else{if(e.step)CloseHandle(e.step);if(e.closed)CloseHandle(e.closed);}
        if(connection)WinHttpCloseHandle(connection);if(session)WinHttpCloseHandle(session);free(hostname);free(path);free(data);
        if(success)break;if(!again){reply->error=error;reply->phase=(uint32_t)e.phase;reply->tls_flags=e.tls;break;}
    }
    if(!success&&!reply->error)reply->error=ERROR_CANCELLED;return success;
}
static int decode(browser_net_t *n,browser_reply_t *reply,const unsigned char *data,size_t bytes,unsigned char **pixels)
{
    if(!current(n,reply->navigation)){reply->error=ERROR_CANCELLED;return 0;}
    if(!browser_image_decode(data,bytes,32u*1024u*1024u-n->decoded_bytes,reply,pixels))return 0;
    if(!current(n,reply->navigation)){free(*pixels);*pixels=NULL;reply->error=ERROR_CANCELLED;return 0;}
    n->decoded_bytes+=reply->bytes;return 1;
}
static unsigned __stdcall run(void *data)
{
    browser_net_t *n=(browser_net_t *)data;HANDLE handles[]={n->quit,n->wake};HRESULT initialized=CoInitializeEx(NULL,COINIT_MULTITHREADED);LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);InterlockedIncrement(&live_threads);
    for(;;){net_job_t *job;browser_reply_t header={0},*reply;unsigned char *body=NULL,*pixels=NULL;size_t length=0;LARGE_INTEGER a,b,c;
        if(WaitForMultipleObjects(2,handles,FALSE,INFINITE)==WAIT_OBJECT_0)break;
        EnterCriticalSection(&n->lock);job=n->head;if(job){n->head=job->next;if(!n->head)n->tail=NULL;
            if(job->navigation==n->navigation)ResetEvent(n->cancel);InterlockedExchange(&n->busy,1);}if(!n->head)ResetEvent(n->wake);LeaveCriticalSection(&n->lock);
        if(!job)continue;if(!current(n,job->navigation)){free(job);InterlockedExchange(&n->busy,0);continue;}
        if(job->kind==BROWSER_HTML){n->transfer_bytes=0;n->decoded_bytes=0;}
        header.navigation=job->navigation;header.resource=job->resource;header.kind=(uint32_t)job->kind;strcpy(header.url,job->url);QueryPerformanceCounter(&a);
        {int fetched=fetch(n,job,&header,&body,&length);QueryPerformanceCounter(&b);header.network_ms=(double)(b.QuadPart-a.QuadPart)*1000.0/frequency.QuadPart;
        if(fetched){
            if(job->kind==BROWSER_IMAGE){header.phase=BROWSER_PHASE_DECODE;InterlockedExchange(&n->decoding,1);if(decode(n,&header,body,length,&pixels)){free(body);body=pixels;pixels=NULL;length=header.bytes;}else length=0;InterlockedExchange(&n->decoding,0);}
            else if(memchr(body,0,length)){header.error=ERROR_NO_UNICODE_TRANSLATION;length=0;}
            else{wchar_t *valid=wide((char *)body);if(!valid){header.error=ERROR_NO_UNICODE_TRANSLATION;length=0;}free(valid);}
            QueryPerformanceCounter(&c);header.decode_ms=(double)(c.QuadPart-b.QuadPart)*1000.0/frequency.QuadPart;
        }}
        if(header.error&&!header.category){header.category=header.error==ERROR_CANCELLED?BROWSER_FAILURE_CANCELLED:header.error==ERROR_FILE_TOO_LARGE||header.error==ERROR_NOT_ENOUGH_MEMORY?BROWSER_FAILURE_BUDGET:header.error==ERROR_NO_UNICODE_TRANSLATION?BROWSER_FAILURE_ENCODING:header.error==ERROR_WINHTTP_SECURE_FAILURE||header.tls_flags?BROWSER_FAILURE_TLS:job->kind==BROWSER_IMAGE&&header.phase==BROWSER_PHASE_DECODE?BROWSER_FAILURE_IMAGE:BROWSER_FAILURE_TRANSPORT;}
        browser_diagnostic_format((browser_failure_t)header.category,(browser_phase_t)header.phase,header.error,header.http_status,header.tls_flags,header.message,sizeof(header.message));
        if(header.encoding_status){size_t used=strlen(header.message);snprintf(header.message+used,sizeof(header.message)-used,"；编码：%s",header.encoding_status==BROWSER_ENCODING_UNKNOWN?"未知或不支持的声明":header.encoding_status==BROWSER_ENCODING_LIMIT?"转换后UTF-8超过门槛":header.encoding_status==BROWSER_ENCODING_MEMORY?"转换内存不足":"损坏字节或不允许的NUL");}
        if(header.kind==BROWSER_IMAGE&&!header.error){size_t used=strlen(header.message);snprintf(header.message+used,sizeof(header.message)-used,"；图片%u×%u；EXIF方向%u%s%s",header.width,header.height,header.orientation,header.orientation>1?"已应用":header.orientation==0?"无效，按原图简化":"",header.frames>1?"；动画仅呈现第一帧":"");}
        if(header.codec_status){size_t used=strlen(header.message);snprintf(header.message+used,sizeof(header.message)-used,"；WIC HRESULT 0x%08x",header.codec_status);}
        header.bytes=header.error?0:length;reply=(browser_reply_t *)malloc(sizeof(header)+header.bytes+1);
        if(reply){memcpy(reply,&header,sizeof(header));if(header.bytes)memcpy(BROWSER_REPLY_DATA(reply),body,header.bytes);BROWSER_REPLY_DATA(reply)[header.bytes]=0;
            if(current(n,job->navigation)){ui_status_t status;do{status=ui_workspace_post_app_data(n->workspace,n->instance,BROWSER_REPLY_KIND,reply,sizeof(header)+header.bytes+1);
                if(status!=UI_STATUS_LIMIT_EXCEEDED)break;}while(WaitForSingleObject(n->cancel,10)==WAIT_TIMEOUT&&current(n,job->navigation));
                if(status!=UI_STATUS_OK&&status!=UI_STATUS_CANCELLED)InterlockedExchange(&n->post_error,(LONG)status);}
            free(reply);}
        else InterlockedExchange(&n->post_error,UI_STATUS_OUT_OF_MEMORY);
        free(body);free(job);InterlockedExchange(&n->busy,0);
    }
    if(SUCCEEDED(initialized))CoUninitialize();InterlockedDecrement(&live_threads);return 0;
}
static void clear_jobs(browser_net_t *n)
{net_job_t *job=n->head;while(job){net_job_t *next=job->next;free(job);job=next;}n->head=n->tail=NULL;}
browser_net_t *browser_net_create(ui_workspace_t *w,uint64_t id,const browser_policy_t *policy)
{
    browser_net_t *n=(browser_net_t *)calloc(1,sizeof(*n));if(!n)return NULL;n->workspace=w;n->instance=id;n->policy=*policy;InitializeCriticalSection(&n->lock);
    n->wake=CreateEventW(NULL,TRUE,FALSE,NULL);n->cancel=CreateEventW(NULL,TRUE,FALSE,NULL);n->quit=CreateEventW(NULL,TRUE,FALSE,NULL);
    if(n->wake&&n->cancel&&n->quit)n->thread=(HANDLE)_beginthreadex(NULL,0,run,n,0,NULL);
    if(!n->thread){browser_net_destroy(n);return NULL;}return n;
}
int browser_net_resource(browser_net_t *n,uint64_t navigation,uint64_t resource,browser_resource_kind_t kind,const char *url)
{
    net_job_t *job;browser_url_t parsed;
    if(!browser_url_parse(NULL,url,&parsed))return 0;
    job=(net_job_t *)calloc(1,sizeof(*job));if(!job)return 0;job->navigation=navigation;job->resource=resource;job->kind=kind;strcpy(job->url,parsed.text);
    EnterCriticalSection(&n->lock);if(n->closing||n->navigation!=navigation||n->resource_count>=40||!browser_url_allowed(&n->policy,&parsed)){LeaveCriticalSection(&n->lock);free(job);return 0;}
    if(kind!=BROWSER_HTML)++n->resource_count;if(n->tail)n->tail->next=job;else n->head=job;n->tail=job;SetEvent(n->wake);LeaveCriticalSection(&n->lock);return 1;
}
int browser_net_begin(browser_net_t *n,uint64_t generation,const char *url)
{EnterCriticalSection(&n->lock);n->navigation=generation;n->resource_count=0;clear_jobs(n);SetEvent(n->cancel);LeaveCriticalSection(&n->lock);return browser_net_resource(n,generation,0,BROWSER_HTML,url);}
void browser_net_stop(browser_net_t *n)
{if(!n)return;EnterCriticalSection(&n->lock);n->navigation=0;clear_jobs(n);SetEvent(n->cancel);LeaveCriticalSection(&n->lock);}
void browser_net_shutdown(browser_net_t *n)
{if(!n)return;InterlockedExchange(&n->closing,1);browser_net_stop(n);SetEvent(n->quit);}
void browser_net_set_policy(browser_net_t *n,const browser_policy_t *policy)
{if(!n)return;browser_net_stop(n);EnterCriticalSection(&n->lock);n->policy=*policy;LeaveCriticalSection(&n->lock);}
int browser_net_finished(browser_net_t *n){return !n||!n->thread||WaitForSingleObject(n->thread,0)==WAIT_OBJECT_0;}
int browser_net_idle(browser_net_t *n)
{int idle;if(!n)return 1;EnterCriticalSection(&n->lock);idle=!n->head&&!InterlockedCompareExchange(&n->busy,0,0);LeaveCriticalSection(&n->lock);return idle;}
long browser_net_take_error(browser_net_t *n){return n?InterlockedExchange(&n->post_error,0):0;}
int browser_net_decoding(browser_net_t *n){return n?(int)InterlockedCompareExchange(&n->decoding,0,0):0;}
void browser_net_destroy(browser_net_t *n)
{if(!n)return;if(n->thread)CloseHandle(n->thread);if(n->wake)CloseHandle(n->wake);if(n->cancel)CloseHandle(n->cancel);if(n->quit)CloseHandle(n->quit);clear_jobs(n);DeleteCriticalSection(&n->lock);free(n);}
