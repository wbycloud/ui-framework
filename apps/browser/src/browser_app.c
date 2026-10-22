#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include "ui_framework/application.h"
#include "ui_framework/app_data.h"
#include "ui_framework/shell.h"
#include "ui_framework/light_document.h"
#include "ui_framework/theme.h"
#include "ui_framework/menus.h"
#include "browser_net.h"
#include "browser_json.h"
#include "browser_settings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ACTION_KIND 2u
#define CHROME_HEIGHT 132
#define HISTORY_LIMIT 64
typedef struct browser_action { char action[32],value[BROWSER_URL_CAP]; } browser_action_t;
typedef struct history { char url[BROWSER_URL_CAP];int x,y; } history_t;
typedef struct browser {
    ui_app_context_t context;
    ui_web_backend_t *chrome_backend,*document_backend;
    ui_web_view_t *chrome,*document;
    HWND slot;
    browser_net_t *net;
    browser_policy_t policy;
    browser_settings_t settings;
    ULONGLONG settings_poll;
    int panel;
    uint32_t last_category,last_windows,last_http,last_tls,last_phase,diagnostic_omitted;
    char diagnostics[4096],settings_feedback[512];
    uint64_t navigation;
    uint32_t zoom;
    ui_image_id_t images[40];size_t image_count;
    int closing,mounted,error_active,loading,stopping,history_index,history_count,restore,subclassed,status_dirty,accept_network;
    history_t history[HISTORY_LIMIT];
    char address[BROWSER_URL_CAP],source[BROWSER_URL_CAP],pending_fragment[BROWSER_URL_CAP],status[512],error[1024],find[4096];
    double network_ms,decode_ms;
    uint32_t resource_restrictions;
    char snapshot[49152];
} browser_t;
static volatile LONG instances;
static const char *command_names[]={"navigate","back","forward","reload","stop","home","help","address","find","find-next","find-prev","plus","minus","reset","settings","diagnostics","settings-save","settings-reset","panel-close"};
static void resize_views(browser_t *s);
static void chrome_state(browser_t *s,int address);
static void navigate(browser_t *,const char *,int);
static void action(browser_t *,const browser_action_t *);
static void failure(browser_t *,const char *);
static void settings_changed(browser_t *s);
static void position_fragment(browser_t *s);
static void diagnostic(browser_t *s,const browser_reply_t *reply)
{
    char entry[6144],origin[200]="[地址已隐藏]";browser_url_t parsed;size_t used,length;
    if(browser_url_parse(NULL,reply->url,&parsed))snprintf(origin,sizeof(origin),"%.170s%s",parsed.origin,strlen(parsed.origin)>170?"[长来源省略]":"");
    if(reply->category){s->last_category=reply->category;s->last_windows=browser_failure_system_error((browser_failure_t)reply->category,reply->error)?reply->error:0;s->last_http=reply->http_status;s->last_tls=reply->tls_flags;s->last_phase=reply->phase;}
    snprintf(entry,sizeof(entry),"%s #%llu 来源 %s\n%s\n%s",reply->kind==BROWSER_HTML?"主文档":reply->kind==BROWSER_IMAGE?"图片":"样式",(unsigned long long)reply->resource,origin,reply->message,reply->redirect_trace);
    length=strlen(entry);used=strlen(s->diagnostics);
    if(length>=sizeof(s->diagnostics)-64){snprintf(entry,sizeof(entry),"链细节超出显示门槛，已省略；最后结果：%s；重定向%u步；来源%s\n",reply->message,reply->redirects,origin);length=strlen(entry);++s->diagnostic_omitted;}
    if(used+length>=sizeof(s->diagnostics)-1){s->diagnostics[0]=0;used=0;++s->diagnostic_omitted;}
    memcpy(s->diagnostics+used,entry,length+1);
}
static void local_failure(browser_t *s,browser_failure_t kind,const char *url)
{
    browser_reply_t reply={0};reply.kind=BROWSER_HTML;reply.category=(uint32_t)kind;
    if(browser_url_parse(NULL,url,&(browser_url_t){0}))strcpy(reply.url,url);
    browser_diagnostic_format(kind,BROWSER_PHASE_NONE,reply.error,0,0,reply.message,sizeof(reply.message));diagnostic(s,&reply);failure(s,reply.message);
}
static void settings_ui(browser_t *s,int replace_text)
{
    char quoted[24578],message[3074],json[28672];
    if(!browser_json_quote(s->settings.text,quoted,sizeof(quoted))||!browser_json_quote(*s->settings_feedback?s->settings_feedback:s->settings.message,message,sizeof(message)))return;
    snprintf(json,sizeof(json),"{\"config_message\":%s%s%s}",message,replace_text?",\"origins\":" : "",replace_text?quoted:"");
    (void)ui_web_view_post_json(s->chrome,json);
}

static void post_action(browser_t *s,const char *name,const char *value)
{
    browser_action_t a={0};if(s->closing)return;
    if(strlen(name)>=sizeof(a.action)||strlen(value?value:"")>=sizeof(a.value))return;
    strcpy(a.action,name);if(value)strcpy(a.value,value);
    if(ui_workspace_post_app_data(s->context.workspace,s->context.instance_id,ACTION_KIND,&a,sizeof(a))!=UI_STATUS_OK){strcpy(s->status,"操作队列已满，请重试。");chrome_state(s,0);}
}
static int shortcut(browser_t *s,UINT message,WPARAM wp)
{
    int control=(GetKeyState(VK_CONTROL)&0x8000)!=0,alt=(GetKeyState(VK_MENU)&0x8000)!=0;const char *name=NULL;
    if(message!=WM_KEYDOWN&&message!=WM_SYSKEYDOWN)return 0;
    if(control){switch(wp){case 'L':name="address";break;case 'F':name="find";break;case 'R':name="reload";break;case VK_OEM_PLUS:case VK_ADD:name="plus";break;case VK_OEM_MINUS:case VK_SUBTRACT:name="minus";break;case '0':case VK_NUMPAD0:name="reset";break;}}
    else if(alt){if(wp==VK_LEFT)name="back";else if(wp==VK_RIGHT)name="forward";}
    else if(wp==VK_F5)name="reload";else if(wp==VK_ESCAPE)name="escape";
    if(name){if(alt)(void)ui_host_close_menu(s->context.host);post_action(s,name,"");return 1;}return 0;
}
static LRESULT CALLBACK view_proc(HWND hwnd,UINT message,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR data)
{
    browser_t *s=(browser_t *)data;LRESULT result;ui_app_callback_enter(&s->context);(void)id;
    if(shortcut(s,message,wp))result=0;else result=DefSubclassProc(hwnd,message,wp,lp);
    ui_app_callback_leave(&s->context);return result;
}
static void finish_close(browser_t *s)
{
    if(s->closing&&browser_net_finished(s->net)){
        if(ui_workspace_post_close_complete(s->context.workspace,s->context.instance_id,UI_APP_CLOSE_ALLOW)==UI_STATUS_OK)KillTimer(s->slot,0x4252);
    }
}
static LRESULT CALLBACK slot_proc(HWND hwnd,UINT message,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR data)
{
    browser_t *s=(browser_t *)data;LRESULT result;ui_app_callback_enter(&s->context);(void)id;
    result=DefSubclassProc(hwnd,message,wp,lp);
    if(message==WM_SIZE&&!s->closing)resize_views(s);
    if(message==WM_TIMER&&wp==0x4252){
        if(s->closing)finish_close(s);
        else if(s->mounted){
            if(GetTickCount64()>=s->settings_poll){s->settings_poll=GetTickCount64()+500;if(browser_settings_reload(&s->settings))settings_changed(s);}
            if(browser_net_take_error(s->net))failure(s,"后台结果投递失败（内存或队列门槛）");
            if(s->stopping&&browser_net_idle(s->net)){s->stopping=0;strcpy(s->status,"已停止");chrome_state(s,0);}
            if(s->document&&!s->error_active){ui_document_state_t doc={0};doc.size=sizeof(doc);(void)ui_web_view_flush(s->document,64);
                if(ui_light_document_get_state(s->document,&doc)==UI_STATUS_OK&&!s->loading){
                    if(*s->pending_fragment&&!doc.pending_resources)position_fragment(s);
                    if(s->restore&&!doc.pending_resources){history_t *h=&s->history[s->history_index];(void)ui_light_document_scroll(s->document,h->x,h->y);s->restore=0;}
                    if(!s->stopping&&s->status_dirty){const char *state=doc.failed_resources?"部分呈现：资源失败（诊断可查看原因）":doc.pending_resources?"加载资源":!doc.paint_count?"等待呈现":(doc.restrictions||s->resource_restrictions)?"受限呈现":"阅读就绪";
                        if(doc.paint_count)s->status_dirty=0;if(strncmp(s->status,state,strlen(state))){snprintf(s->status,sizeof(s->status),"%s · %u%%",state,s->zoom);chrome_state(s,0);}}
                }}
        }
    }
    ui_app_callback_leave(&s->context);return result;
}
static void resize_views(browser_t *s)
{
    RECT r;uint32_t dpi=96;ui_rect_t chrome,doc;if(!s->slot)return;GetClientRect(s->slot,&r);(void)ui_host_get_dpi(s->context.host,&dpi);
    chrome=(ui_rect_t){0,0,MulDiv(r.right,96,(int)dpi),s->error_active||s->panel?MulDiv(r.bottom,96,(int)dpi):CHROME_HEIGHT};
    if(chrome.height>MulDiv(r.bottom,96,(int)dpi))chrome.height=MulDiv(r.bottom,96,(int)dpi);
    doc=(ui_rect_t){0,chrome.height,chrome.width,MulDiv(r.bottom,96,(int)dpi)-chrome.height};if(doc.height<0)doc.height=0;
    if(s->chrome)(void)ui_web_view_set_rect(s->chrome,&chrome,dpi);if(s->document){(void)ui_web_view_set_rect(s->document,&doc,dpi);if(ui_web_view_native_handle(s->document))ShowWindow((HWND)ui_web_view_native_handle(s->document),s->error_active||s->panel?SW_HIDE:SW_SHOWNA);}
}
static void chrome_state(browser_t *s,int set_address)
{
    char status[3074],address[8194],error[6146],json[20480];ui_theme_info_t theme={0};int n;
    if(!s->chrome)return;theme.size=sizeof(theme);(void)ui_host_get_theme(s->context.host,&theme);
    if(!browser_json_quote(s->status,status,sizeof(status))||!browser_json_quote(s->address,address,sizeof(address))||!browser_json_quote(s->error,error,sizeof(error)))return;
    n=snprintf(json,sizeof(json),"{\"status\":%s,%s%s%s\"error\":%s,\"theme\":\"%s\",\"zoom\":%u,\"back\":%s,\"forward\":%s}",status,
        set_address?"\"address\":" : "",set_address?address:"",set_address?",":"",error,theme.effective==UI_THEME_DARK?"dark":"light",s->zoom,s->history_index>0?"true":"false",s->history_index+1<s->history_count?"true":"false");
    if(n>0&&(size_t)n<sizeof(json))(void)ui_web_view_post_json(s->chrome,json);
    {char text[24578],data[26624];if(browser_json_quote(s->diagnostics,text,sizeof(text))){snprintf(data,sizeof(data),"{\"panel\":%d,\"diagnostics\":%s,\"omitted\":%u,\"config_invalid\":%s}",s->panel,text,s->diagnostic_omitted,s->settings.valid?"false":"true");(void)ui_web_view_post_json(s->chrome,data);}}
}
static void theme_changed(ui_host_t *host,const ui_theme_info_t *t,void *data)
{browser_t *s=(browser_t *)data;(void)host;(void)t;chrome_state(s,0);resize_views(s);}
static void release_images(browser_t *s)
{for(size_t i=0;i<s->image_count;++i)(void)ui_image_release(s->context.host,s->images[i]);s->image_count=0;}
static void failure(browser_t *s,const char *text)
{
    s->pending_fragment[0]=0;s->accept_network=0;browser_net_stop(s->net);if(s->document)(void)ui_light_document_cancel_resources(s->document);
    s->loading=0;s->stopping=0;s->error_active=1;snprintf(s->error,sizeof(s->error),"加载失败\n%s\n可修改地址重试，或使用后退、首页和帮助。",text);
    snprintf(s->status,sizeof(s->status),"失败：%s",text);resize_views(s);chrome_state(s,1);
    (void)ui_workspace_set_instance_title(s->context.workspace,s->context.instance_id,"加载失败 · 浏览网页");
}
static void save_scroll(browser_t *s)
{
    ui_document_state_t d={0};if(!s->document||s->history_index<0||s->error_active)return;d.size=sizeof(d);
    if(ui_light_document_get_state(s->document,&d)==UI_STATUS_OK){s->history[s->history_index].x=d.scroll_x;s->history[s->history_index].y=d.scroll_y;}
}
static void add_history(browser_t *s,const char *url)
{
    s->history_count=s->history_index+1;if(s->history_count==HISTORY_LIMIT){memmove(s->history,s->history+1,(HISTORY_LIMIT-1)*sizeof(history_t));--s->history_count;}
    s->history_index=s->history_count++;memset(&s->history[s->history_index],0,sizeof(history_t));strcpy(s->history[s->history_index].url,url);
}
static int builtin(browser_t *s,const char *url)
{
    void *html=NULL;size_t length=0;ui_status_t status;ui_document_load_t load={0};ui_document_state_t d={0};const char *resource=!strcmp(url,"browser:help")?"help.html":"start.html";
    status=ui_app_resource_read(&s->context,resource,&html,&length);if(status!=UI_STATUS_OK){failure(s,"包内页面读取失败");return 0;}
    load.size=sizeof(load);load.navigation_id=s->navigation;load.source_url="https://browser.invalid/";load.html_utf8=(const char *)html;
    status=ui_light_document_load(s->document,&load);ui_app_resource_release(html);
    if(status!=UI_STATUS_OK){failure(s,"包内页面解析失败");return 0;}(void)ui_light_document_zoom(s->document,s->zoom);release_images(s);strcpy(s->source,load.source_url);
    s->loading=0;d.size=sizeof(d);(void)ui_light_document_get_state(s->document,&d);(void)ui_workspace_set_instance_title(s->context.workspace,s->context.instance_id,*d.title_utf8?d.title_utf8:"浏览网页");
    strcpy(s->status,!strcmp(url,"browser:help")?"包内帮助":"包内起始页");return 1;
}
static ui_status_t apply_fragment(browser_t *s,const char *fragment)
{
    char decoded[BROWSER_URL_CAP];size_t used=0;const char *at=fragment;
    while(*at&&used+1<sizeof(decoded)){if(*at=='%'&&at[1]&&at[2]){char pair[3]={at[1],at[2],0},*end;long c=strtol(pair,&end,16);if(*end||!c)break;decoded[used++]=(char)c;at+=3;}else decoded[used++]=*at++;}decoded[used]=0;
    if(!*at)return ui_light_document_fragment(s->document,decoded);return UI_STATUS_VALIDATION_FAILED;
}
static void position_fragment(browser_t *s)
{
    ui_status_t status=apply_fragment(s,s->pending_fragment);ui_document_state_t doc={0};doc.size=sizeof(doc);(void)ui_light_document_get_state(s->document,&doc);s->pending_fragment[0]=0;s->status_dirty=0;
    snprintf(s->status,sizeof(s->status),"%s%s",status==UI_STATUS_OK?"已定位页面片段":"未找到片段目标，正文仍可阅读",doc.failed_resources?"；部分呈现：资源失败（诊断可查看原因）":doc.restrictions||s->resource_restrictions?"；受限呈现":"");chrome_state(s,0);
}
static void navigate(browser_t *s,const char *input,int history_mode)
{
    browser_url_t url;browser_url_result_t result=BROWSER_URL_OK;int local=!strcmp(input,"browser:start")||!strcmp(input,"browser:help");char address[BROWSER_URL_CAP];
    if(s->closing)return;if(history_mode!=1)save_scroll(s);
    s->pending_fragment[0]=0;s->panel=0;s->diagnostics[0]=0;s->diagnostic_omitted=0;s->last_category=s->last_windows=s->last_http=s->last_tls=s->last_phase=0;
    if(local)strcpy(address,input);else if((result=browser_url_parse_ex(NULL,input,&url))!=BROWSER_URL_OK||!browser_url_allowed(&s->policy,&url)){
        ++s->navigation;if(strlen(input)<sizeof(s->address)){if(result==BROWSER_URL_OK)strcpy(s->address,url.text);else browser_url_public(input,s->address);if(!history_mode)add_history(s,s->address);}local_failure(s,result!=BROWSER_URL_OK?browser_url_failure(result,0):BROWSER_FAILURE_ORIGIN,input);return;
    }else strcpy(address,url.text);
    if(!history_mode)add_history(s,address);else if(history_mode==1)s->restore=1;
    strcpy(s->address,address);
    /* Same-document fragments need no transport or document recreation. */
    if(!local&&!s->error_active&&strchr(address,'#')){browser_url_t current_url;
        if(browser_url_parse(NULL,s->source,&current_url)&&!strcmp(current_url.origin,url.origin)&&!strcmp(current_url.path,url.path)){
            strcpy(s->pending_fragment,url.fragment);if(!*url.fragment)(void)apply_fragment(s,"");else{ui_document_state_t doc={0};doc.size=sizeof(doc);(void)ui_light_document_get_state(s->document,&doc);if(!doc.pending_resources)position_fragment(s);}chrome_state(s,1);return;}}
    browser_net_stop(s->net);++s->navigation;s->resource_restrictions=0;s->accept_network=!local;s->error_active=0;s->error[0]=0;s->stopping=0;s->loading=!local;
    resize_views(s);
    if(local)(void)builtin(s,address);
    else{strcpy(s->status,"正在加载主文档…");if(!browser_net_begin(s->net,s->navigation,address))failure(s,"无法创建请求");}
    chrome_state(s,1);
}
static void settings_changed(browser_t *s)
{
    s->policy=s->settings.policy;browser_net_set_policy(s->net,&s->policy);
    navigate(s,"browser:start",0);strcpy(s->status,s->settings.valid?"来源规则已更新；在途请求已取消，已返回包内首页。":"来源配置无效：全部网络已拒绝，请设置或恢复默认。");
    chrome_state(s,0);settings_ui(s,1);
}
static void document_event(ui_web_view_t *view,const ui_document_event_t *event,void *data)
{
    browser_t *s=(browser_t *)data;(void)view;if(s->closing)return;
    if(event->kind==UI_DOCUMENT_FOCUS_OUT){post_action(s,event->resource_id?"focus-last":"address","");return;}
    if(event->kind==UI_DOCUMENT_NAVIGATE||event->kind==UI_DOCUMENT_SUBMIT){char target[BROWSER_URL_CAP];
        if(event->kind==UI_DOCUMENT_SUBMIT){const char *names[64],*values[64];for(size_t i=0;i<event->field_count;++i){names[i]=event->fields[i].name_utf8;values[i]=event->fields[i].value_utf8;}
            if(!browser_url_form(event->url_utf8,names,values,event->field_count,target)){post_action(s,"error","GET 表单地址超限");return;}}
        else strcpy(target,event->url_utf8);post_action(s,"go",target);
    }else if(event->navigation_id==s->navigation&&(event->kind==UI_DOCUMENT_IMAGE||event->kind==UI_DOCUMENT_STYLESHEET)){
        if(!browser_net_resource(s->net,s->navigation,event->resource_id,event->kind==UI_DOCUMENT_IMAGE?BROWSER_IMAGE:BROWSER_CSS,event->url_utf8)){
            /* Delivery must occur outside the document's native callback. */
            browser_reply_t reply={0};reply.navigation=s->navigation;reply.resource=event->resource_id;reply.kind=event->kind==UI_DOCUMENT_IMAGE?BROWSER_IMAGE:BROWSER_CSS;reply.error=ERROR_ACCESS_DENIED;
            {browser_url_t url;browser_url_result_t result=browser_url_parse_ex(NULL,event->url_utf8,&url);reply.category=(uint32_t)(result!=BROWSER_URL_OK?browser_url_failure(result,0):!browser_url_allowed(&s->policy,&url)?BROWSER_FAILURE_ORIGIN:BROWSER_FAILURE_QUEUE);}
            strcpy(reply.url,event->url_utf8);browser_diagnostic_format((browser_failure_t)reply.category,BROWSER_PHASE_NONE,reply.error,0,0,reply.message,sizeof(reply.message));
            (void)ui_workspace_post_app_data(s->context.workspace,s->context.instance_id,BROWSER_REPLY_KIND,&reply,sizeof(reply));}
    }
}
static void chrome_message(ui_web_view_t *view,const char *json,void *data)
{
    browser_t *s=(browser_t *)data;browser_action_t a={0};(void)view;
    if(browser_json_get(json,"action",a.action,sizeof(a.action))&&browser_json_get(json,"value",a.value,sizeof(a.value)))post_action(s,a.action,a.value);
}
static void receive(uint64_t kind,const void *data,size_t bytes,void *user)
{
    browser_t *s=(browser_t *)user;if(s->closing)return;
    if(kind==ACTION_KIND){if(bytes==sizeof(browser_action_t))action(s,(const browser_action_t *)data);return;}
    if(kind==BROWSER_REPLY_KIND&&bytes>=sizeof(browser_reply_t)){
        const browser_reply_t *r=(const browser_reply_t *)data;if(!s->accept_network||r->navigation!=s->navigation||r->bytes>bytes-sizeof(*r))return;
        diagnostic(s,r);if(r->kind==BROWSER_HTML)s->network_ms=r->network_ms;
        if(r->kind==BROWSER_HTML){ui_document_load_t load={0};ui_document_state_t d={0};ui_status_t status;
            if(r->error){failure(s,r->message);return;}if(r->bytes==bytes-sizeof(*r)||BROWSER_REPLY_DATA(r)[r->bytes]!=0){failure(s,"文档结果不完整");return;}
            load.size=sizeof(load);load.navigation_id=s->navigation;load.source_url=r->url;load.html_utf8=(const char *)BROWSER_REPLY_DATA(r);
            status=ui_light_document_load(s->document,&load);release_images(s);
            if(status!=UI_STATUS_OK){local_failure(s,status==UI_STATUS_LIMIT_EXCEEDED?BROWSER_FAILURE_DOCUMENT_LIMIT:status==UI_STATUS_UNSUPPORTED?BROWSER_FAILURE_DOCUMENT_EMPTY:BROWSER_FAILURE_DOCUMENT,r->url);return;}
            s->loading=0;s->status_dirty=1;s->network_ms=r->network_ms;s->decode_ms=r->decode_ms;strcpy(s->source,r->url);strcpy(s->address,r->url);
            if(s->history_index>=0)strcpy(s->history[s->history_index].url,r->url);
            d.size=sizeof(d);(void)ui_light_document_zoom(s->document,s->zoom);(void)ui_light_document_get_state(s->document,&d);
            (void)ui_workspace_set_instance_title(s->context.workspace,s->context.instance_id,*d.title_utf8?d.title_utf8:r->url);
            {browser_url_t url;if(!s->restore&&browser_url_parse(NULL,r->url,&url)&&*url.fragment)strcpy(s->pending_fragment,url.fragment);}
            (void)ui_web_view_flush(s->document,64);strcpy(s->status,"主文档已解析，正在呈现…");chrome_state(s,1);
        }else{
            ui_image_id_t id=0;
            s->decode_ms+=r->decode_ms;
            s->resource_restrictions+=r->restrictions;
            if(r->kind==BROWSER_IMAGE&&!r->error){ui_rgba_desc_t rgba={0};rgba.size=sizeof(rgba);rgba.width=r->width;rgba.height=r->height;rgba.stride=(size_t)r->width*4;rgba.bytes=r->bytes;rgba.pixels=BROWSER_REPLY_DATA(r);
                if(s->image_count<40&&ui_image_create(s->context.host,&rgba,&id)==UI_STATUS_OK)s->images[s->image_count++]=id;
                else{browser_reply_t info=*r;info.category=BROWSER_FAILURE_BUDGET;info.error=ERROR_NOT_ENOUGH_MEMORY;browser_diagnostic_format(BROWSER_FAILURE_BUDGET,BROWSER_PHASE_DECODE,info.error,info.http_status,0,info.message,sizeof(info.message));diagnostic(s,&info);}}
            {ui_status_t status=ui_light_document_resource(s->document,r->navigation,r->resource,id,r->kind==BROWSER_CSS&&!r->error?(const char *)BROWSER_REPLY_DATA(r):NULL);
             if(status!=UI_STATUS_OK&&status!=UI_STATUS_NOT_FOUND){local_failure(s,status==UI_STATUS_LIMIT_EXCEEDED?BROWSER_FAILURE_DOCUMENT_LIMIT:status==UI_STATUS_UNSUPPORTED?BROWSER_FAILURE_DOCUMENT_EMPTY:BROWSER_FAILURE_DOCUMENT,r->url);return;}}
            s->status_dirty=1;chrome_state(s,0);
        }
    }
}
static void focus_chrome(browser_t *s,const char *id)
{
    char json[96];snprintf(json,sizeof(json),"{\"focus\":\"%s\"}",id);SetFocus((HWND)ui_web_view_native_handle(s->chrome));(void)ui_web_view_post_json(s->chrome,json);
    if(!strcmp(id,"address")){ui_input_event_t input={0};input.size=sizeof(input);input.kind=UI_INPUT_KEY_DOWN;input.key_code='A';input.modifiers=UI_INPUT_MODIFIER_CONTROL;(void)ui_web_view_dispatch_input(s->chrome,&input);}
}
static void action(browser_t *s,const browser_action_t *a)
{
    if(s->closing)return;
    if(!strcmp(a->action,"settings")){s->panel=1;settings_ui(s,1);resize_views(s);chrome_state(s,0);focus_chrome(s,"origins");}
    else if(!strcmp(a->action,"diagnostics")){s->panel=2;resize_views(s);chrome_state(s,0);focus_chrome(s,"diagnostic-text");}
    else if(!strcmp(a->action,"panel-close")||(!strcmp(a->action,"escape")&&s->panel)){s->panel=0;resize_views(s);chrome_state(s,0);}
    else if(!strcmp(a->action,"settings-save")||!strcmp(a->action,"settings-reset")){
        int saved=!strcmp(a->action,"settings-save")?browser_settings_save(&s->settings,a->value):browser_settings_reset(&s->settings);
        if(saved){s->settings_feedback[0]=0;settings_changed(s);s->panel=1;}
        else{strcpy(s->settings_feedback,s->settings.message);s->panel=1;settings_ui(s,0);}
        resize_views(s);chrome_state(s,0);
    }else if(!strcmp(a->action,"go"))navigate(s,a->value,0);
    else if(!strcmp(a->action,"home"))navigate(s,"browser:start",0);
    else if(!strcmp(a->action,"help"))navigate(s,"browser:help",0);
    else if(!strcmp(a->action,"reload")){char url[BROWSER_URL_CAP];strcpy(url,s->address);navigate(s,url,2);}
    else if(!strcmp(a->action,"back")||!strcmp(a->action,"forward")){int index=s->history_index+(!strcmp(a->action,"back")?-1:1);if(index>=0&&index<s->history_count){save_scroll(s);s->history_index=index;navigate(s,s->history[index].url,1);}}
    else if(!strcmp(a->action,"stop")||(!strcmp(a->action,"escape")&&(s->loading||!browser_net_idle(s->net)))){
        s->pending_fragment[0]=0;browser_net_stop(s->net);s->accept_network=0;(void)ui_light_document_cancel_resources(s->document);s->loading=0;s->stopping=1;s->status_dirty=0;strcpy(s->status,"正在停止…");chrome_state(s,0);
    }else if(!strcmp(a->action,"address"))focus_chrome(s,"address");
    else if(!strcmp(a->action,"focus-last"))focus_chrome(s,"diagnostics");
    else if(!strcmp(a->action,"find")){focus_chrome(s,"find");}
    else if(!strcmp(a->action,"escape")){s->find[0]=0;(void)ui_light_document_find(s->document,"",0);SetFocus((HWND)ui_web_view_native_handle(s->document));}
    else if(!strcmp(a->action,"find-next")||!strcmp(a->action,"find-prev")){
        ui_status_t status;s->status_dirty=0;strcpy(s->find,a->value);status=ui_light_document_find(s->document,s->find,!strcmp(a->action,"find-prev"));
        if(status==UI_STATUS_OK)SetFocus((HWND)ui_web_view_native_handle(s->document));
        strcpy(s->status,status==UI_STATUS_OK?"已定位查找结果":status==UI_STATUS_NOT_FOUND?"未找到文字":"查找失败");chrome_state(s,0);
    }else if(!strcmp(a->action,"plus")||!strcmp(a->action,"minus")||!strcmp(a->action,"reset")){
        if(!strcmp(a->action,"reset"))s->zoom=100;else if(!strcmp(a->action,"plus")&&s->zoom<200)s->zoom+=10;else if(!strcmp(a->action,"minus")&&s->zoom>50)s->zoom-=10;
        if(ui_light_document_zoom(s->document,s->zoom)!=UI_STATUS_OK){failure(s,"缩放后的文档超出呈现门槛");return;}
        snprintf(s->status,sizeof(s->status),"页面缩放 %u%%",s->zoom);chrome_state(s,0);
    }else if(!strcmp(a->action,"document")||!strcmp(a->action,"document-last")){
        ui_input_event_t input={0};SetFocus((HWND)ui_web_view_native_handle(s->document));input.size=sizeof(input);input.kind=UI_INPUT_KEY_DOWN;input.key_code=VK_TAB;
        if(!strcmp(a->action,"document-last"))input.modifiers=UI_INPUT_MODIFIER_SHIFT;(void)ui_web_view_dispatch_input(s->document,&input);
    }else if(!strcmp(a->action,"error"))failure(s,a->value);
}
static const char *snapshot(void *data)
{
    browser_t *s=(browser_t *)data;ui_document_state_t d={0};ui_image_stats_t images={0};char address[8194],status[3074],title[24578],public_url[BROWSER_URL_CAP],diagnostics[24578];int n;
    d.size=sizeof(d);images.size=sizeof(images);if(s->document)(void)ui_light_document_get_state(s->document,&d);(void)ui_image_get_stats(s->context.host,&images);
    d.restrictions+=s->resource_restrictions;
    browser_url_public(s->address,public_url);
    if(!browser_json_quote(public_url,address,sizeof(address))||!browser_json_quote(s->diagnostics,diagnostics,sizeof(diagnostics))||!browser_json_quote(s->status,status,sizeof(status))||!browser_json_quote(d.title_utf8,title,sizeof(title)))return "{\"error\":\"snapshot encoding\"}";
    n=snprintf(s->snapshot,sizeof(s->snapshot),"{\"address\":%s,\"title\":%s,\"status\":%s,\"navigation\":\"%llu\",\"document_navigation\":\"%llu\",\"loading\":%d,\"error\":%d,\"idle\":%d,\"pending\":%u,\"failed_resources\":%u,\"restrictions\":%u,\"script_runtime\":%u,\"zoom\":%u,\"scroll_x\":%d,\"scroll_y\":%d,\"content_width\":%d,\"content_height\":%d,\"images\":%zu,\"image_bytes\":%zu,\"network_ms\":%.3f,\"decode_ms\":%.3f,\"parse_ms\":%.3f,\"layout_ms\":%.3f,\"paint_ms\":%.3f,\"paint_count\":%llu,\"threads\":%ld,\"callbacks\":%ld}",
        address,title,status,(unsigned long long)s->navigation,(unsigned long long)d.navigation_id,s->loading,s->error_active,browser_net_idle(s->net),d.pending_resources,d.failed_resources,d.restrictions,d.script_runtime,s->zoom,d.scroll_x,d.scroll_y,d.content_width,d.content_height,images.count,images.bytes,s->network_ms,s->decode_ms,d.parse_ms,d.layout_ms,d.paint_ms,(unsigned long long)d.paint_count,browser_net_live_threads(),browser_net_live_callbacks());
    if(n>0&&(size_t)n+32<sizeof(s->snapshot)){int extra;s->snapshot[n-1]=0;extra=snprintf(s->snapshot+n-1,sizeof(s->snapshot)-(size_t)n+1,",\"decoding\":%d,\"failure\":\"%s\",\"windows\":%u,\"http\":%u,\"tls\":%u,\"phase\":%u,\"policy_valid\":%d,\"policy_count\":%zu,\"custom_policy\":%d,\"panel\":%d,\"diagnostic_omitted\":%u,\"diagnostics\":%s}",browser_net_decoding(s->net),browser_failure_name((browser_failure_t)s->last_category),s->last_windows,s->last_http,s->last_tls,s->last_phase,s->settings.valid,s->policy.count,s->settings.custom,s->panel,s->diagnostic_omitted,diagnostics);
        if(extra>0&&(size_t)extra<sizeof(s->snapshot)-(size_t)n+1)return s->snapshot;}return "{\"error\":\"snapshot size\"}";
}
static void command(ui_host_t *host,uint64_t id,const char *command_id,const char *params,const char *source,void *data)
{
    browser_t *s=(browser_t *)data;char value[BROWSER_URL_CAP]="";const char *name=command_id+8;(void)source;
    if(!strcmp(name,"navigate")){if(!browser_json_get(params,"url",value,sizeof(value))){(void)ui_host_reply(host,id,0,"{\"error\":\"url string required\"}");return;}name="go";}
    if(!strcmp(name,"settings-save")&&!browser_json_get(params,"origins",value,sizeof(value))){(void)ui_host_reply(host,id,0,"{\"error\":\"origins string required\"}");return;}
    if(!strcmp(name,"find-next")||!strcmp(name,"find-prev")){if(!browser_json_get(params,"text",value,sizeof(value)))strcpy(value,s->find);}
    post_action(s,name,value);(void)ui_host_reply(host,id,1,"{\"queued\":true}");
}
static ui_status_t UI_APP_CALL create(const ui_app_context_t *context,void **out,ui_assistant_config_t *assistant)
{
    const char *const *names=command_names;
    static const char *titles[]={"打开地址","后退","前进","刷新","停止","首页","帮助","地址栏","查找","下一个结果","上一个结果","放大","缩小","重置缩放","可信来源设置","访问诊断","保存来源","恢复默认来源","返回正文"};
    browser_t *s=(browser_t *)calloc(1,sizeof(*s));if(!s)return UI_STATUS_OUT_OF_MEMORY;
    *out=s;s->context=*context;s->history_index=-1;s->zoom=100;InterlockedIncrement(&instances);
    for(size_t i=0;i<sizeof(command_names)/sizeof(command_names[0]);++i){ui_command_desc_t c={0};ui_menu_item_desc_t m={0};char command_id[64],menu_id[64];ui_status_t status;
        snprintf(command_id,sizeof(command_id),"browser.%s",names[i]);c.size=sizeof(c);c.id=command_id;c.title=titles[i];c.params_schema_json=!strcmp(names[i],"navigate")?"{\"type\":\"object\",\"properties\":{\"url\":{\"type\":\"string\"}},\"required\":[\"url\"]}":"{\"type\":\"object\"}";c.handler=command;c.user_data=s;
        status=ui_host_register_command(context->host,&c);if(status!=UI_STATUS_OK)return status;
        if(!strcmp(names[i],"navigate")||!strcmp(names[i],"settings-save"))continue;
        snprintf(menu_id,sizeof(menu_id),"browser.menu.%zu",i);m.size=sizeof(m);m.id=menu_id;m.menu_path="浏览";m.title=titles[i];m.command_id=command_id;m.order=(int)i;
        status=ui_host_register_menu_item(context->host,&m);if(status!=UI_STATUS_OK)return status;}
    assistant->user_data=s;assistant->snapshot=snapshot;assistant->source_id="browser.local";assistant->max_permission=UI_ASSISTANT_PERMISSION_READ;
    return UI_STATUS_OK;
}
static ui_status_t UI_APP_CALL mount(void *data,const ui_app_context_t *context)
{
    browser_t *s=(browser_t *)data;ui_content_slot_t *slot;ui_light_web_config_t cfg={0};void *html=NULL,*policy=NULL;size_t bytes;ui_status_t status;
    s->context=*context;slot=ui_shell_get_content_slot(ui_host_get_shell(context->host),NULL);if(!slot)return UI_STATUS_UNSUPPORTED;s->slot=(HWND)ui_content_slot_native_handle(slot);if(!s->slot)return UI_STATUS_UNSUPPORTED;
    for(size_t i=0;i<sizeof(command_names)/sizeof(command_names[0]);++i){ui_assistant_command_desc_t c={0};char id[64];snprintf(id,sizeof(id),"browser.%s",command_names[i]);
        if(!strcmp(command_names[i],"settings-save")||!strcmp(command_names[i],"settings-reset"))continue; /* Local trusted UI only. */
        c.size=sizeof(c);c.id=id;c.permission=UI_ASSISTANT_PERMISSION_READ;c.params_schema_json="{\"type\":\"object\"}";status=ui_assistant_register_command(context->assistant,&c);if(status!=UI_STATUS_OK)return status;}
    status=ui_app_resource_read(context,"origins.txt",&policy,&bytes);if(status!=UI_STATUS_OK)return status;
    if(!browser_settings_init(&s->settings,(char *)policy)){ui_app_resource_release(policy);return UI_STATUS_VALIDATION_FAILED;}s->policy=s->settings.policy;ui_app_resource_release(policy);s->settings_poll=GetTickCount64()+500;
    cfg.size=sizeof(cfg);cfg.parent_hwnd=s->slot;s->chrome_backend=ui_light_web_backend_create(&cfg);s->document_backend=ui_light_document_backend_create(&cfg);if(!s->chrome_backend||!s->document_backend)return UI_STATUS_OUT_OF_MEMORY;
    s->chrome=ui_web_view_create(context->host,s->chrome_backend);s->document=ui_web_view_create(context->host,s->document_backend);if(!s->chrome||!s->document)return UI_STATUS_PLATFORM_ERROR;
    if(!SetWindowSubclass(s->slot,slot_proc,0x4252,(DWORD_PTR)s))return UI_STATUS_PLATFORM_ERROR;s->subclassed=1;
    if(!SetWindowSubclass((HWND)ui_web_view_native_handle(s->chrome),view_proc,0x4252,(DWORD_PTR)s)||!SetWindowSubclass((HWND)ui_web_view_native_handle(s->document),view_proc,0x4252,(DWORD_PTR)s))return UI_STATUS_PLATFORM_ERROR;
    status=ui_app_set_data_callback(context,receive,s);if(status!=UI_STATUS_OK)return status;
    status=ui_web_view_set_message_callback(s->chrome,chrome_message,s);if(status!=UI_STATUS_OK)return status;
    status=ui_light_document_set_event_callback(s->document,document_event,s);if(status!=UI_STATUS_OK)return status;
    status=ui_app_resource_read(context,"chrome.html",&html,&bytes);if(status!=UI_STATUS_OK)return status;
    status=ui_web_view_load_html(s->chrome,(char *)html);ui_app_resource_release(html);if(status!=UI_STATUS_OK)return status;
    (void)ui_host_set_theme_callback(context->host,theme_changed,s);s->mounted=1;
    if(!SetTimer(s->slot,0x4252,30,NULL))return UI_STATUS_PLATFORM_ERROR;
    resize_views(s);navigate(s,"browser:start",0);if(s->error_active)return UI_STATUS_VALIDATION_FAILED;
    s->net=browser_net_create(context->workspace,context->instance_id,&s->policy);return s->net?UI_STATUS_OK:UI_STATUS_OUT_OF_MEMORY;
}
static void UI_APP_CALL active(void *data,int value){(void)data;(void)value;}
static ui_app_close_decision_t UI_APP_CALL request_close(void *data,ui_app_close_reason_t reason)
{
    browser_t *s=(browser_t *)data;(void)reason;s->closing=1;browser_net_shutdown(s->net);
    if(browser_net_finished(s->net))return UI_APP_CLOSE_ALLOW;return UI_APP_CLOSE_WAIT;
}
static void UI_APP_CALL unmount(void *data)
{
    browser_t *s=(browser_t *)data;
    if(s->slot){KillTimer(s->slot,0x4252);if(s->subclassed)RemoveWindowSubclass(s->slot,slot_proc,0x4252);s->subclassed=0;}
    (void)ui_host_set_theme_callback(s->context.host,NULL,NULL);(void)ui_app_set_data_callback(&s->context,NULL,NULL);
    if(s->chrome){RemoveWindowSubclass((HWND)ui_web_view_native_handle(s->chrome),view_proc,0x4252);ui_web_view_destroy(s->chrome);s->chrome=NULL;}
    if(s->document){RemoveWindowSubclass((HWND)ui_web_view_native_handle(s->document),view_proc,0x4252);ui_web_view_destroy(s->document);s->document=NULL;}
    release_images(s);if(s->chrome_backend)ui_light_web_backend_destroy(s->chrome_backend);if(s->document_backend)ui_light_web_backend_destroy(s->document_backend);s->chrome_backend=s->document_backend=NULL;s->mounted=0;
}
static void UI_APP_CALL destroy(void *data)
{browser_t *s=(browser_t *)data;if(!s)return;browser_net_destroy(s->net);InterlockedDecrement(&instances);free(s);}
static ui_status_t UI_APP_CALL module_shutdown(void)
{return !InterlockedCompareExchange(&instances,0,0)&&!browser_net_live_threads()&&!browser_net_live_callbacks()?UI_STATUS_OK:UI_STATUS_CANCELLED;}
UI_APP_EXPORT const ui_app_descriptor_t *UI_APP_CALL ui_app_query_v1(void)
{
    static const ui_app_descriptor_t descriptor={sizeof(ui_app_descriptor_t),UI_APPLICATION_ABI_VERSION,UI_FRAMEWORK_API_VERSION,create,mount,active,request_close,unmount,destroy,module_shutdown};return &descriptor;
}
