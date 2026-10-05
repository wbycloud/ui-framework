#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <imm.h>
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include "ui_framework/application.h"
#include "ui_framework/light_web.h"
#include "ui_framework/shell.h"
/* The framework shell consumes registrations; SDK applications never use this header. */
#include "../../src/ui_internal.h"

enum { ID_OPEN=0x1001, ID_CLOSE, ID_EXIT, ID_NEXT, ID_PREVIOUS };
#define HOST_ACTION_MESSAGE (WM_APP+0x351)
#define HOST_REFRESH_MESSAGE (WM_APP+0x352)
#define HOST_PAGE_RESOURCE 101
#define HOST_MESSAGE_LIMIT 65536u
#define HOST_LOG_LIMIT 32000u
#define HOST_DISPLAY_TEXT_LIMIT 4095u

typedef struct json_buffer { char *text; size_t length,capacity; int failed; } json_buffer_t;
typedef struct host_action {
    char *action,*id,*command,*params,*panel,*token,*answer,*menu,*key,*modifiers,*editing;
    uint64_t popup_token;
} host_action_t;
typedef struct host_window {
    HWND hwnd,web_hwnd,popup_hwnd;
    ui_host_t *chrome_host;
    ui_web_backend_t *backend,*popup_backend;
    ui_web_view_t *view,*popup_view;
    ui_workspace_t *workspace;
    uint32_t dpi;
    ui_rect_t workspace_rect;
    uint64_t target,last_id,last_request,completed_id,completed_request;
    uint64_t transaction_id,transaction_instance,undo_id,undo_instance;
    uint64_t popup_token;
    char *command,*params,*log;
    int refreshing,refresh_pending,exiting,dark,assistant_expanded;
    int popup_kind,confirm_answer,confirm_waiting;
    char reported_error[512];
    HHOOK menu_hook;
    RECT menu_anchor;
    HWND menu_previous_focus;
} host_window_t;

static void refresh_workspace(void *data);
static void layout(host_window_t *s);
static void close_popup(host_window_t *s);
static void show_error(host_window_t *s,const char *text);
__declspec(thread) static host_window_t *pointer_menu;

static LRESULT CALLBACK host_menu_messages(int code,WPARAM wp,LPARAM lp)
{
    host_window_t *s=pointer_menu;
    if(code>=0&&wp==PM_REMOVE&&s&&s->popup_kind==1){MSG *m=(MSG *)lp;
        if((m->message==WM_LBUTTONDOWN||m->message==WM_RBUTTONDOWN||m->message==WM_MBUTTONDOWN||m->message==WM_NCLBUTTONDOWN)&&
            m->hwnd!=s->popup_hwnd&&!IsChild(s->popup_hwnd,m->hwnd)){
            POINT pt={(short)LOWORD(m->lParam),(short)HIWORD(m->lParam)};
            if(m->message!=WM_NCLBUTTONDOWN)ClientToScreen(m->hwnd,&pt);
            if(!PtInRect(&s->menu_anchor,pt))close_popup(s);
        }
    }
    return CallNextHookEx(NULL,code,wp,lp);
}

static char *copy_text(const char *text)
{
    size_t length;char *copy;
    if(!text)text="";length=strlen(text);copy=(char *)malloc(length+1);
    if(copy)memcpy(copy,text,length+1);return copy;
}
static void replace_text(char **destination,const char *text)
{char *copy=copy_text(text);if(copy){free(*destination);*destination=copy;}}
static char *to_utf8(const wchar_t *text)
{
    int n=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,text,-1,NULL,0,NULL,NULL);char *utf8;
    if(!n)return NULL;utf8=(char *)malloc((size_t)n);
    if(utf8)WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,text,-1,utf8,n,NULL,NULL);return utf8;
}
static void json_append(json_buffer_t *json,const char *text)
{
    size_t added=strlen(text),capacity;char *memory;
    if(json->failed)return;
    if(added>SIZE_MAX-json->length-1){json->failed=1;return;}
    if(json->length+added+1>json->capacity){
        capacity=json->capacity?json->capacity:512;
        while(capacity<json->length+added+1){if(capacity>SIZE_MAX/2){json->failed=1;return;}capacity*=2;}
        memory=(char *)realloc(json->text,capacity);if(!memory){json->failed=1;return;}
        json->text=memory;json->capacity=capacity;
    }
    memcpy(json->text+json->length,text,added+1);json->length+=added;
}
static void json_format(json_buffer_t *json,const char *format,...)
{
    char text[512];int length;va_list args;va_start(args,format);
    length=vsnprintf(text,sizeof(text),format,args);va_end(args);
    if(length<0||(size_t)length>=sizeof(text)){json->failed=1;return;}json_append(json,text);
}
static void json_string(json_buffer_t *json,const char *text)
{
    const unsigned char *position=(const unsigned char *)(text?text:"");char escaped[8];
    json_append(json,"\"");
    for(;*position;++position){
        if(*position=='"')json_append(json,"\\\"");
        else if(*position=='\\')json_append(json,"\\\\");
        else if(*position<32){(void)snprintf(escaped,sizeof(escaped),"\\u%04x",(unsigned)*position);json_append(json,escaped);}
        else{escaped[0]=(char)*position;escaped[1]=0;json_append(json,escaped);}
    }
    json_append(json,"\"");
}
/* Read-only previews stay within the controlled backend's text capacity.
 * The command/schema/log storage is not changed by display truncation. */
static void json_preview(json_buffer_t *json,const char *text,int tail)
{
    const char *notice="\n…（显示内容已省略）\n";char preview[HOST_DISPLAY_TEXT_LIMIT+1];
    size_t length=text?strlen(text):0,notice_length=strlen(notice),keep=HOST_DISPLAY_TEXT_LIMIT-notice_length;
    if(length<=HOST_DISPLAY_TEXT_LIMIT){json_string(json,text);return;}
    if(tail){const char *start=text+length-keep;
        while(((unsigned char)*start&0xc0)==0x80)++start;
        memcpy(preview,notice,notice_length);memcpy(preview+notice_length,start,strlen(start)+1);
    }else{
        while(((unsigned char)text[keep]&0xc0)==0x80)--keep;
        memcpy(preview,text,keep);memcpy(preview+keep,notice,notice_length+1);
    }
    json_string(json,preview);
}
/* Confirmation details are displayed in full, split into bounded text nodes. */
static void json_text_chunks(json_buffer_t *json,const char *text)
{
    size_t remaining=text?strlen(text):0;int count=0;char chunk[HOST_DISPLAY_TEXT_LIMIT+1];
    json_append(json,"[");
    while(remaining){size_t length=remaining>HOST_DISPLAY_TEXT_LIMIT?HOST_DISPLAY_TEXT_LIMIT:remaining;
        if(length<remaining)while(((unsigned char)text[length]&0xc0)==0x80)--length;
        memcpy(chunk,text,length);chunk[length]=0;if(count++)json_append(json,",");json_string(json,chunk);
        text+=length;remaining-=length;
    }
    json_append(json,"]");
}
static void json_id(json_buffer_t *json,uint64_t id)
{json_format(json,"\"%llu\"",(unsigned long long)id);}
static void json_rect(json_buffer_t *json,const ui_rect_t *rect)
{json_format(json,"{\"x\":%d,\"y\":%d,\"width\":%d,\"height\":%d}",rect->x,rect->y,rect->width,rect->height);}
static int post_json(ui_web_view_t *view,json_buffer_t *json)
{
    int ok=!json->failed&&json->text&&ui_web_view_post_json(view,json->text)==UI_STATUS_OK;
    free(json->text);memset(json,0,sizeof(*json));return ok;
}

/* Flat string-valued messages. Decimal strings preserve all uint64_t IDs. */
static void skip_space(const char **position)
{while(**position==' '||**position=='\t'||**position=='\r'||**position=='\n')++*position;}
static int hex4(const char **position,uint32_t *value)
{
    int i;uint32_t digit,result=0;
    for(i=0;i<4;++i){unsigned char c=(unsigned char)**position;if(!c)return 0;++*position;
        if(c>='0'&&c<='9')digit=(uint32_t)(c-'0');else if(c>='a'&&c<='f')digit=(uint32_t)(c-'a'+10);
        else if(c>='A'&&c<='F')digit=(uint32_t)(c-'A'+10);else return 0;result=(result<<4)|digit;}
    *value=result;return 1;
}
static char *read_string(const char **position)
{
    char *result,*out;uint32_t code,low;size_t available=strlen(*position);
    if(**position!='"')return NULL;++*position;
    result=(char *)malloc(available+1);if(!result)return NULL;out=result;
    while(**position&&**position!='"'){
        unsigned char c=(unsigned char)*(*position)++;if(c<32)goto invalid;
        if(c!='\\'){*out++=(char)c;continue;}c=(unsigned char)*(*position)++;if(!c)goto invalid;
        switch(c){
        case '"':case '\\':case '/':*out++=(char)c;break;
        case 'b':*out++='\b';break;case 'f':*out++='\f';break;case 'n':*out++='\n';break;
        case 'r':*out++='\r';break;case 't':*out++='\t';break;
        case 'u':
            if(!hex4(position,&code)||!code)goto invalid;
            if(code>=0xd800&&code<=0xdbff){
                if((*position)[0]!='\\'||(*position)[1]!='u')goto invalid;*position+=2;
                if(!hex4(position,&low)||low<0xdc00||low>0xdfff)goto invalid;
                code=0x10000+((code-0xd800)<<10)+(low-0xdc00);
            }else if(code>=0xdc00&&code<=0xdfff)goto invalid;
            if(code<0x80)*out++=(char)code;
            else if(code<0x800){*out++=(char)(0xc0|(code>>6));*out++=(char)(0x80|(code&63));}
            else if(code<0x10000){*out++=(char)(0xe0|(code>>12));*out++=(char)(0x80|((code>>6)&63));*out++=(char)(0x80|(code&63));}
            else{*out++=(char)(0xf0|(code>>18));*out++=(char)(0x80|((code>>12)&63));*out++=(char)(0x80|((code>>6)&63));*out++=(char)(0x80|(code&63));}
            break;
        default:goto invalid;
        }
    }
    if(**position!='"')goto invalid;++*position;*out=0;return result;
invalid:free(result);return NULL;
}
static void action_free(host_action_t *action)
{
    if(!action)return;free(action->action);free(action->id);free(action->command);free(action->params);
    free(action->panel);free(action->token);free(action->answer);free(action->menu);free(action->key);free(action->modifiers);free(action->editing);free(action);
}
static host_action_t *parse_action(const char *text)
{
    const char *position=text;host_action_t *action;int entries=0;
    if(!text||strlen(text)>HOST_MESSAGE_LIMIT)return NULL;
    action=(host_action_t *)calloc(1,sizeof(*action));if(!action)return NULL;
    skip_space(&position);if(*position++!='{')goto invalid;skip_space(&position);
    while(*position&&*position!='}'){
        char *key=read_string(&position),*value;char **field=NULL;
        if(!key||++entries>7){free(key);goto invalid;}skip_space(&position);
        if(*position++!=':'){free(key);goto invalid;}skip_space(&position);value=read_string(&position);
        if(!value){free(key);goto invalid;}
        if(!strcmp(key,"action"))field=&action->action;else if(!strcmp(key,"id"))field=&action->id;
        else if(!strcmp(key,"command"))field=&action->command;else if(!strcmp(key,"params"))field=&action->params;
        else if(!strcmp(key,"menu"))field=&action->menu;else if(!strcmp(key,"panel"))field=&action->panel;else if(!strcmp(key,"token"))field=&action->token;
        else if(!strcmp(key,"answer"))field=&action->answer;
        else if(!strcmp(key,"key"))field=&action->key;else if(!strcmp(key,"modifiers"))field=&action->modifiers;else if(!strcmp(key,"editing"))field=&action->editing;
        free(key);if(!field||*field){free(value);goto invalid;}*field=value;skip_space(&position);
        if(*position=='}')break;if(*position++!=',')goto invalid;skip_space(&position);if(*position=='}')goto invalid;
    }
    if(*position++!='}')goto invalid;skip_space(&position);
    if(*position||!action->action||strlen(action->action)>40)goto invalid;return action;
invalid:action_free(action);return NULL;
}
static uint64_t read_id(const char *text)
{
    uint64_t result=0;if(!text||!*text)return 0;
    while(*text){unsigned digit=(unsigned)(*text++-'0');if(digit>9||result>(UINT64_MAX-digit)/10)return 0;result=result*10+digit;}
    return result;
}
static int get_instance(host_window_t *s,uint64_t id,ui_app_instance_info_t *info)
{
    memset(info,0,sizeof(*info));info->size=sizeof(*info);
    return s->workspace&&ui_workspace_get_instance(s->workspace,id,info)==UI_STATUS_OK;
}
static void schedule_refresh(host_window_t *s)
{if(!s->refresh_pending&&s->hwnd){s->refresh_pending=1;PostMessageW(s->hwnd,HOST_REFRESH_MESSAGE,0,0);}}
static void append_log(host_window_t *s,const char *text)
{
    size_t old=s->log?strlen(s->log):0,added=text?strlen(text):0;char *memory;
    if(added>HOST_LOG_LIMIT){text="Message exceeds the host log limit.";added=strlen(text);}
    if(old+added+2>HOST_LOG_LIMIT)old=0;
    memory=(char *)realloc(s->log,old+added+2);if(!memory)return;s->log=memory;
    memcpy(memory+old,text?text:"",added);memory[old+added]='\n';memory[old+added+1]=0;schedule_refresh(s);
}
static char *load_page(void)
{
    HRSRC resource=FindResourceW(GetModuleHandleW(NULL),MAKEINTRESOURCEW(HOST_PAGE_RESOURCE),MAKEINTRESOURCEW(10));
    HGLOBAL memory;DWORD size;const void *bytes;char *page;
    if(!resource)return NULL;size=SizeofResource(GetModuleHandleW(NULL),resource);
    memory=LoadResource(GetModuleHandleW(NULL),resource);if(!memory)return NULL;bytes=LockResource(memory);
    if(!bytes)return NULL;page=(char *)malloc((size_t)size+1);if(page){memcpy(page,bytes,size);page[size]=0;}return page;
}
static void on_web_message(ui_web_view_t *view,const char *json,void *data)
{
    host_window_t *s=(host_window_t *)data;host_action_t *action=parse_action(json);if(!action)return;
    action->popup_token=view==s->popup_view?s->popup_token:0;
    if(!PostMessageW(s->hwnd,HOST_ACTION_MESSAGE,0,(LPARAM)action))action_free(action);
}
static ui_web_view_t *create_view(host_window_t *s,HWND parent,ui_web_backend_t **backend)
{
    ui_light_web_config_t config;ui_web_view_t *view;char *page;uint64_t capabilities=0;
    memset(&config,0,sizeof(config));config.size=sizeof(config);config.parent_hwnd=parent;config.enable_native_input=1;
    *backend=ui_light_web_backend_create(&config);if(!*backend)return NULL;
    view=ui_web_view_create(s->chrome_host,*backend);if(!view)goto failed;
    if(ui_web_view_get_capabilities(view,&capabilities)!=UI_STATUS_OK||
       (capabilities&(UI_WEB_CAP_JSON_MESSAGES|UI_WEB_CAP_DYNAMIC_DOM|UI_WEB_CAP_NATIVE_WINDOW))!=
       (UI_WEB_CAP_JSON_MESSAGES|UI_WEB_CAP_DYNAMIC_DOM|UI_WEB_CAP_NATIVE_WINDOW))goto failed;
    if(ui_web_view_set_message_callback(view,on_web_message,s)!=UI_STATUS_OK)goto failed;
    page=load_page();if(!page)goto failed;
    if(ui_web_view_load_html(view,page)!=UI_STATUS_OK){free(page);goto failed;}free(page);return view;
failed:if(view)ui_web_view_destroy(view);ui_light_web_backend_destroy(*backend);*backend=NULL;return NULL;
}
static LRESULT CALLBACK popup_proc(HWND hwnd,UINT message,WPARAM wp,LPARAM lp)
{
    host_window_t *s=(host_window_t *)GetWindowLongPtrW(hwnd,GWLP_USERDATA);
    if(message==WM_NCCREATE){s=(host_window_t *)((CREATESTRUCTW *)lp)->lpCreateParams;SetWindowLongPtrW(hwnd,GWLP_USERDATA,(LONG_PTR)s);}
    if(!s)return DefWindowProcW(hwnd,message,wp,lp);
    if(message==WM_ACTIVATE&&LOWORD(wp)==WA_INACTIVE&&s->popup_kind==1&&(HWND)lp!=s->hwnd){s->menu_previous_focus=NULL;SendMessageW(hwnd,WM_CLOSE,0,0);return 0;}
    if(message==WM_CLOSE){
        if(s->confirm_waiting)s->confirm_answer=-1;
        else{host_action_t *action=(host_action_t *)calloc(1,sizeof(*action));
            if(action){action->action=copy_text("dismiss");action->popup_token=s->popup_token;
                if(!action->action||!PostMessageW(s->hwnd,HOST_ACTION_MESSAGE,0,(LPARAM)action))action_free(action);}}
        return 0;
    }
    if(message==WM_SIZE&&s->popup_hwnd==hwnd&&s->popup_view){RECT rect;GetClientRect(hwnd,&rect);
        (void)ui_web_view_resize(s->popup_view,MulDiv(rect.right,96,(int)s->dpi),MulDiv(rect.bottom,96,(int)s->dpi),s->dpi);return 0;}
    if(message==WM_KEYDOWN&&wp==VK_ESCAPE){SendMessageW(hwnd,WM_CLOSE,0,0);return 0;}
    return DefWindowProcW(hwnd,message,wp,lp);
}
static void close_popup(host_window_t *s)
{
    if(s->confirm_waiting){s->confirm_answer=-1;return;}
    if(s->menu_hook){UnhookWindowsHookEx(s->menu_hook);s->menu_hook=NULL;if(pointer_menu==s)pointer_menu=NULL;}
    if(s->popup_hwnd||s->popup_view)++s->popup_token;
    if(s->popup_view){ui_web_view_destroy(s->popup_view);s->popup_view=NULL;}
    if(s->popup_backend){ui_light_web_backend_destroy(s->popup_backend);s->popup_backend=NULL;}
    if(s->popup_hwnd){HWND window=s->popup_hwnd;s->popup_hwnd=NULL;DestroyWindow(window);}s->popup_kind=0;
    if(s->menu_previous_focus&&IsWindow(s->menu_previous_focus)&&IsWindowVisible(s->menu_previous_focus)&&IsWindowEnabled(s->menu_previous_focus))SetFocus(s->menu_previous_focus);s->menu_previous_focus=NULL;
}
static int create_popup(host_window_t *s,int kind,int width,int height)
{
    WNDCLASSW wc;RECT owner,client;int x,y;ui_rect_t view_rect;
    if(s->confirm_waiting)return 0;close_popup(s);memset(&wc,0,sizeof(wc));
    wc.lpfnWndProc=popup_proc;wc.hInstance=GetModuleHandleW(NULL);wc.lpszClassName=L"UiFramework.WebPopup.v2";
    wc.hCursor=LoadCursorW(NULL,MAKEINTRESOURCEW(32512));
    if(!RegisterClassW(&wc)&&GetLastError()!=ERROR_CLASS_ALREADY_EXISTS)return 0;
    GetWindowRect(s->hwnd,&owner);GetClientRect(s->hwnd,&client);
    if(width>MulDiv(client.right,96,(int)s->dpi)-16)width=MulDiv(client.right,96,(int)s->dpi)-16;
    if(width<180)width=180;if(height>MulDiv(client.bottom,96,(int)s->dpi)-16)height=MulDiv(client.bottom,96,(int)s->dpi)-16;
    if(height<80)height=80;
    x=kind==3?owner.left+(owner.right-owner.left-MulDiv(width,(int)s->dpi,96))/2:owner.left+12;
    y=kind==3?owner.top+(owner.bottom-owner.top-MulDiv(height,(int)s->dpi,96))/2:owner.top+MulDiv(76,(int)s->dpi,96);
    s->popup_hwnd=CreateWindowExW(WS_EX_TOOLWINDOW,wc.lpszClassName,L"",WS_POPUP|WS_BORDER|WS_CLIPCHILDREN,
        x,y,MulDiv(width,(int)s->dpi,96),MulDiv(height,(int)s->dpi,96),s->hwnd,NULL,wc.hInstance,s);
    if(!s->popup_hwnd)return 0;s->popup_kind=kind;++s->popup_token;
    s->popup_view=create_view(s,s->popup_hwnd,&s->popup_backend);if(!s->popup_view){close_popup(s);return 0;}
    GetClientRect(s->popup_hwnd,&client);view_rect.x=view_rect.y=0;
    view_rect.width=MulDiv(client.right,96,(int)s->dpi);view_rect.height=MulDiv(client.bottom,96,(int)s->dpi);
    (void)ui_web_view_set_rect(s->popup_view,&view_rect,s->dpi);(void)ui_web_view_resize(s->popup_view,view_rect.width,view_rect.height,s->dpi);
    if(kind==1){ui_rect_t at={0};POINT origin={0,0};if(pointer_menu&&pointer_menu!=s)close_popup(pointer_menu);
        s->menu_previous_focus=GetFocus();(void)ui_web_view_get_element_rect(s->view,"menu",&at);ClientToScreen(s->web_hwnd,&origin);
        s->menu_anchor.left=origin.x+MulDiv(at.x,(int)s->dpi,96);s->menu_anchor.top=origin.y+MulDiv(at.y,(int)s->dpi,96);
        s->menu_anchor.right=s->menu_anchor.left+MulDiv(at.width,(int)s->dpi,96);s->menu_anchor.bottom=s->menu_anchor.top+MulDiv(at.height,(int)s->dpi,96);
        pointer_menu=s;s->menu_hook=SetWindowsHookExW(WH_GETMESSAGE,host_menu_messages,NULL,GetCurrentThreadId());if(!s->menu_hook){close_popup(s);return 0;}}
    ShowWindow(s->popup_hwnd,kind==5?SW_SHOWNOACTIVATE:SW_SHOW);if(kind==1&&IsWindowVisible(s->hwnd))SetFocus((HWND)ui_web_view_native_handle(s->popup_view));return 1;
}
static void popup_start(host_window_t *s,json_buffer_t *json,const char *title,const char *detail,int confirm)
{
    json_append(json,"{\"type\":\"popup\",\"token\":");json_id(json,s->popup_token);
    json_format(json,",\"dark\":%s,\"menu\":%s,\"confirm\":%s,\"title\":",s->dark?"true":"false",s->popup_kind==1?"true":"false",confirm?"true":"false");
    json_preview(json,title,0);json_append(json,",\"detailParts\":");json_text_chunks(json,detail);json_append(json,",\"items\":[");
}
static void popup_item(json_buffer_t *json,int *count,const char *title,const char *action,uint64_t id,const char *command)
{
    if((*count)++)json_append(json,",");json_append(json,"{\"title\":");json_string(json,title);
    json_append(json,",\"action\":");json_string(json,action);json_append(json,",\"id\":");json_id(json,id);
    json_append(json,",\"command\":");json_string(json,command);json_append(json,"}");
}
static void popup_finish(host_window_t *s,json_buffer_t *json)
{json_append(json,"]}");(void)post_json(s->popup_view,json);}
static void show_error(host_window_t *s,const char *text)
{
    json_buffer_t json={0};append_log(s,text);(void)snprintf(s->reported_error,sizeof(s->reported_error),"%s",text);
    if(create_popup(s,4,500,240)){popup_start(s,&json,"操作未完成",text,0);popup_finish(s,&json);}
}

typedef struct command_query { const char *id; char *first,*schema; int found; } command_query_t;
static void visit_command(const ui_assistant_command_desc_t *command,void *data)
{
    command_query_t *query=(command_query_t *)data;if(!query->first)query->first=copy_text(command->id);
    if(query->id&&!strcmp(query->id,command->id)){query->found=1;query->schema=copy_text(command->params_schema_json);}
}
static char *refresh_command(host_window_t *s,ui_app_instance_info_t *target)
{
    command_query_t query={0};query.id=s->command;
    if(!get_instance(s,s->target,target)){s->target=ui_workspace_active(s->workspace);
        if(!get_instance(s,s->target,target)){replace_text(&s->command,"");return copy_text("");}}
    (void)ui_assistant_visit_commands(target->assistant,visit_command,&query);
    if(!query.found){replace_text(&s->command,query.first?query.first:"");free(query.schema);query.schema=NULL;
        query.id=s->command;query.found=0;(void)ui_assistant_visit_commands(target->assistant,visit_command,&query);}
    free(query.first);return query.schema?query.schema:copy_text("");
}
static int tool_compare(void *context,const void *a,const void *b)
{
    const ui_toolbar_item_entry_t *left=*(ui_toolbar_item_entry_t *const *)a,*right=*(ui_toolbar_item_entry_t *const *)b;
    const ui_host_t *host=(const ui_host_t *)context;ui_toolbar_entry_t *toolbar;int lo=0,ro=0,group;
    for(toolbar=host->toolbars;toolbar;toolbar=toolbar->next){if(!strcmp(toolbar->id,left->toolbar_id))lo=toolbar->order;if(!strcmp(toolbar->id,right->toolbar_id))ro=toolbar->order;}
    if(lo!=ro)return lo<ro?-1:1;group=strcmp(left->toolbar_id,right->toolbar_id);if(group)return group;
    if(left->order!=right->order)return left->order<right->order?-1:1;return strcmp(left->id,right->id);
}
static int toolbar_visible(const ui_host_t *host,const char *id)
{
    ui_toolbar_entry_t *toolbar;for(toolbar=host->toolbars;toolbar;toolbar=toolbar->next)
        if(!strcmp(toolbar->id,id))return toolbar->visible;return 0;
}
static int tool_visible(const ui_host_t *host,const ui_toolbar_item_entry_t *item)
{ui_command_state_t state={0};state.size=sizeof(state);
 return item->state.visible&&toolbar_visible(host,item->toolbar_id)&&
     ui_host_get_command_state(host,item->command_id,&state)==UI_STATUS_OK&&state.visible;}
typedef struct menu_state_writer { json_buffer_t *json;size_t count; } menu_state_writer_t;
static void state_menu_group(const ui_menu_model_entry_t *entry,void *data)
{
    menu_state_writer_t *writer=(menu_state_writer_t *)data;json_buffer_t *json=writer->json;uint64_t hash=UINT64_C(1469598103934665603);const unsigned char *cursor=(const unsigned char *)entry->path;
    while(*cursor){hash^=*cursor++;hash*=UINT64_C(1099511628211);}if(writer->count++)json_append(json,",");
    json_format(json,"{\"key\":\"%llu\",\"path\":",(unsigned long long)hash);json_string(json,entry->path);json_append(json,",\"title\":");json_string(json,entry->title);json_append(json,"}");
}
static void state_tools(json_buffer_t *json,const ui_host_t *host)
{
    ui_toolbar_item_entry_t *item,**items;size_t count=0,index=0;
    for(item=host->toolbar_items;item;item=item->next)if(tool_visible(host,item))++count;
    items=count?(ui_toolbar_item_entry_t **)malloc(count*sizeof(*items)):NULL;if(count&&!items){json->failed=1;return;}
    for(item=host->toolbar_items;item;item=item->next)if(tool_visible(host,item))items[index++]=item;
    if(count)qsort_s(items,count,sizeof(*items),tool_compare,(void *)host);
    for(index=0;index<count;++index){if(index)json_append(json,",");json_append(json,"{\"id\":");json_string(json,items[index]->id);json_append(json,",\"title\":");json_string(json,items[index]->title);
        json_append(json,",\"group\":");json_string(json,items[index]->toolbar_id);
        {ui_toolbar_entry_t *bar;for(bar=host->toolbars;bar&&strcmp(bar->id,items[index]->toolbar_id);bar=bar->next){}
            json_format(json,",\"compact\":%s",bar&&bar->display==UI_TOOLBAR_COMPACT?"true":"false");}
        json_append(json,",\"command\":");json_string(json,items[index]->command_id);{ui_command_state_t state={0};state.size=sizeof(state);(void)ui_host_get_command_state(host,items[index]->command_id,&state);
        json_format(json,",\"image\":\"%llu\",\"enabled\":%s,\"checked\":%s,\"busy\":%s}",(unsigned long long)items[index]->image_id,
            state.enabled&&items[index]->state.enabled?"true":"false",state.checked||items[index]->state.checked?"true":"false",state.busy||items[index]->state.busy?"true":"false");}}free(items);
}
static void state_panels(json_buffer_t *json,ui_app_instance_info_t *info)
{
    ui_panel_entry_t *panel;int count=0;ui_shell_t *shell=ui_host_get_shell(info->host);
    for(panel=info->host->panels;panel;panel=panel->next){ui_rect_t frame={0},content={0};int visible=0,floating=0;ui_panel_layout_t layout={0};layout.size=sizeof(layout);
        if(ui_shell_get_slot_state(shell,panel->id,&frame,&content,&visible,&floating)!=UI_STATUS_OK)continue;
        if(!floating&&(!frame.width||!frame.height))continue;if(count++)json_append(json,",");
        json_append(json,"{\"id\":");json_string(json,panel->id);json_append(json,",\"title\":");json_string(json,panel->title);
        (void)ui_shell_get_panel_layout(shell,panel->id,&layout);
        json_format(json,",\"region\":%d,\"floating\":%s,\"collapsed\":%s,\"frame\":",layout.dock_region,floating?"true":"false",layout.collapsed?"true":"false");
        json_rect(json,&frame);json_append(json,"}");}
}
static int assistant_visible(host_window_t *s,int width)
{return s->assistant_expanded>0||(s->assistant_expanded==0&&width>=800);}
static void send_state(host_window_t *s)
{
    json_buffer_t json={0};ui_app_instance_info_t info,target;size_t i,count;
    char *schema,label[512];uint64_t active;RECT client;int width;
    if(!s->view||!s->workspace)return;count=ui_workspace_count(s->workspace);active=ui_workspace_active(s->workspace);
    GetClientRect(s->hwnd,&client);width=MulDiv(client.right,96,(int)s->dpi);schema=refresh_command(s,&target);
    if(!get_instance(s,s->transaction_instance,&info))s->transaction_id=s->transaction_instance=0;
    if(!get_instance(s,s->undo_instance,&info))s->undo_id=s->undo_instance=0;
    if(!get_instance(s,s->last_id,&info))s->last_request=0;
    json_append(&json,"{\"type\":\"state\",\"active\":");if(active)json_id(&json,active);else json_string(&json,"");
    json_format(&json,",\"dpi\":%u,\"dark\":%s,\"assistant\":%s,\"tabs\":[",s->dpi,s->dark?"true":"false",assistant_visible(s,width)?"true":"false");
    for(i=0;i<count;++i){uint64_t id=ui_workspace_instance_at(s->workspace,i);if(!get_instance(s,id,&info))continue;
        if(i)json_append(&json,",");json_append(&json,"{\"id\":");json_id(&json,id);
        (void)snprintf(label,sizeof(label),"%s #%llu%s",info.name_utf8,(unsigned long long)id,info.closing?" · 关闭中":"");
        json_append(&json,",\"title\":");json_string(&json,label);json_format(&json,",\"active\":%s,\"closing\":%s}",info.active?"true":"false",info.closing?"true":"false");}
    json_append(&json,"],\"targetLabel\":");label[0]=0;
    if(target.instance_id)(void)snprintf(label,sizeof(label),"%s #%llu%s",target.name_utf8,(unsigned long long)target.instance_id,target.closing?" · 关闭中":"");
    json_string(&json,label);json_append(&json,",\"command\":");json_string(&json,s->command);
    json_append(&json,",\"schema\":");json_preview(&json,schema,0);free(schema);
    json_append(&json,",\"params\":");json_string(&json,s->params?s->params:"{}");json_append(&json,",\"log\":");json_preview(&json,s->log,1);
    json_format(&json,",\"canInvoke\":%s,\"canCancel\":%s",target.instance_id&&!target.closing&&s->command&&s->command[0]?"true":"false",s->last_request?"true":"false");
    s->chrome_host->image_source=NULL;
    if(get_instance(s,active,&info)){ui_rect_t rect;info.host->menu_dark=s->dark;json_append(&json,",\"menuOpen\":");if(info.host->menu_open_path)json_string(&json,info.host->menu_open_path);else json_append(&json,"null");s->chrome_host->image_source=info.host;json_append(&json,",\"activeLabel\":");json_string(&json,info.name_utf8);
        (void)ui_host_get_rect(info.host,UI_LAYOUT_REGION_MENU_BAR,&rect);json_append(&json,",\"menuRect\":");json_rect(&json,&rect);
        (void)ui_host_get_rect(info.host,UI_LAYOUT_REGION_TOOLBAR,&rect);json_append(&json,",\"toolbarRect\":");json_rect(&json,&rect);
        (void)ui_host_get_rect(info.host,UI_LAYOUT_REGION_STATUS_BAR,&rect);json_append(&json,",\"statusRect\":");json_rect(&json,&rect);
        json_append(&json,",\"menuGroups\":[");{menu_state_writer_t writer={&json,0};if(ui_host_visit_menu(info.host,"",state_menu_group,&writer)!=UI_STATUS_OK)json.failed=1;}json_append(&json,"]");
        json_append(&json,",\"tools\":[");state_tools(&json,info.host);json_append(&json,"],\"panels\":[");state_panels(&json,&info);json_append(&json,"],\"status\":");json_string(&json,ui_host_status_text(info.host));
    }else json_append(&json,",\"activeLabel\":\"\",\"menuGroups\":[],\"tools\":[],\"panels\":[]");
    json_append(&json,"}");(void)post_json(s->view,&json);
}
static void cut_region(HRGN full,const ui_rect_t *rect,uint32_t dpi,int offset_x,int offset_y)
{
    HRGN hole;int left,top,right,bottom;if(rect->width<=0||rect->height<=0)return;
    left=MulDiv(rect->x+offset_x,(int)dpi,96);top=MulDiv(rect->y+offset_y,(int)dpi,96);
    right=MulDiv(rect->x+rect->width+offset_x,(int)dpi,96);bottom=MulDiv(rect->y+rect->height+offset_y,(int)dpi,96);
    hole=CreateRectRgn(left,top,right,bottom);if(hole){CombineRgn(full,full,hole,RGN_DIFF);DeleteObject(hole);}
}
static void update_region(host_window_t *s)
{
    RECT client;HRGN region;ui_app_instance_info_t info;
    if(!s->web_hwnd)return;GetClientRect(s->hwnd,&client);region=CreateRectRgn(0,0,client.right,client.bottom);if(!region)return;
    if(get_instance(s,ui_workspace_active(s->workspace),&info)&&!info.closing){ui_rect_t main;ui_panel_entry_t *panel;ui_shell_t *shell=ui_host_get_shell(info.host);
        if(ui_host_get_rect(info.host,UI_LAYOUT_REGION_MAIN,&main)==UI_STATUS_OK)cut_region(region,&main,s->dpi,s->workspace_rect.x,s->workspace_rect.y);
        for(panel=info.host->panels;panel;panel=panel->next){ui_rect_t frame={0},content={0};int visible=0,floating=0;
            if(ui_shell_get_slot_state(shell,panel->id,&frame,&content,&visible,&floating)==UI_STATUS_OK&&visible&&!floating)
                cut_region(region,&content,s->dpi,s->workspace_rect.x,s->workspace_rect.y);}}
    if(!SetWindowRgn(s->web_hwnd,region,TRUE))DeleteObject(region);
    SetWindowPos(s->web_hwnd,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
}
static void layout(host_window_t *s)
{
    RECT client;int width,height;ui_rect_t rect;
    if(!s->view||!s->workspace||s->refreshing)return;s->refreshing=1;GetClientRect(s->hwnd,&client);
    width=MulDiv(client.right,96,(int)s->dpi);height=MulDiv(client.bottom,96,(int)s->dpi);
    (void)ui_host_set_dpi(s->chrome_host,s->dpi);(void)ui_host_resize(s->chrome_host,width,height);
    (void)ui_web_view_resize(s->view,width,height,s->dpi);send_state(s);
    if(ui_web_view_get_element_rect(s->view,"workspace",&rect)==UI_STATUS_OK){
        if(rect.width<0)rect.width=0;if(rect.height<0)rect.height=0;s->workspace_rect=rect;(void)ui_workspace_set_rect(s->workspace,&rect,s->dpi);}
    send_state(s);update_region(s);s->refreshing=0;
}
static void refresh_workspace(void *data)
{schedule_refresh((host_window_t *)data);}
static void on_result(uint64_t id,const ui_result_t *result,void *data)
{
    host_window_t *s=(host_window_t *)data;json_buffer_t line={0};
    json_format(&line,"[#%llu / request %llu] ",(unsigned long long)id,(unsigned long long)result->request_id);
    json_append(&line,result->command_id);json_append(&line,result->success?" · OK: ":" · FAILED: ");json_append(&line,result->result_json);
    if(!line.failed)append_log(s,line.text);free(line.text);s->completed_id=id;s->completed_request=result->request_id;
    if(s->last_id==id&&s->last_request==result->request_id)s->last_request=0;schedule_refresh(s);
}
static void refresh_menu_states(host_window_t *,uint64_t);
static void on_event(uint64_t id,const char *event,const char *json,void *data)
{
    host_window_t *s=(host_window_t *)data;json_buffer_t line={0};
    if(!strcmp(event,"ui.commands.changed"))refresh_menu_states(s,id);
    if(!strncmp(event,"ui.host.",8)||!strncmp(event,"ui.commands.",12)||!strncmp(event,"ui.image.",9)||!strncmp(event,"ui.components.",14)){if(!s->refreshing)schedule_refresh(s);return;}
    json_format(&line,"[#%llu] ",(unsigned long long)id);json_append(&line,event);json_append(&line," ");json_append(&line,json);
    if(!line.failed)append_log(s,line.text);free(line.text);
}
static void on_progress(uint64_t id,uint64_t request,int percent,const char *text,void *data)
{
    json_buffer_t line={0};json_format(&line,"[#%llu / request %llu] %d%% ",(unsigned long long)id,(unsigned long long)request,percent);
    json_append(&line,text);if(!line.failed)append_log((host_window_t *)data,line.text);free(line.text);
}
static int on_confirm(uint64_t id,const char *command,const char *params,ui_assistant_permission_t permission,void *data)
{
    host_window_t *s=(host_window_t *)data;json_buffer_t json={0},detail={0};MSG message={0};int answer=0,quit=0;
    (void)permission;if(!create_popup(s,3,560,300))return 0;
    json_format(&detail,"目标实例 #%llu\n命令：",(unsigned long long)id);json_append(&detail,command);json_append(&detail,"\n参数：");json_append(&detail,params);
    popup_start(s,&json,"允许助手执行此操作？",detail.text?detail.text:"",1);popup_finish(s,&json);free(detail.text);
    s->confirm_answer=0;s->confirm_waiting=1;EnableWindow(s->hwnd,FALSE);
    /* The application dispatch scope protects its module during this modal loop. */
    while(!s->confirm_answer){int received=GetMessageW(&message,NULL,0,0);
        if(received<=0){quit=received==0;break;}TranslateMessage(&message);DispatchMessageW(&message);}
    answer=s->confirm_answer>0;s->confirm_waiting=0;EnableWindow(s->hwnd,TRUE);close_popup(s);SetActiveWindow(s->hwnd);
    if(quit)PostQuitMessage((int)message.wParam);return answer;
}
static void open_path(host_window_t *s,const wchar_t *path)
{
    char *utf8=to_utf8(path);uint64_t id=0;ui_status_t status;if(!utf8)return;
    status=ui_workspace_open(s->workspace,utf8,&id);free(utf8);
    if(status!=UI_STATUS_OK)show_error(s,ui_workspace_last_error(s->workspace));else schedule_refresh(s);
}
static void open_dialog(host_window_t *s)
{
    OPENFILENAMEW file;wchar_t path[32768]=L"";memset(&file,0,sizeof(file));file.lStructSize=sizeof(file);file.hwndOwner=s->hwnd;
    file.lpstrFilter=L"应用包 (*.uapp)\0*.uapp\0\0";file.lpstrFile=path;file.nMaxFile=32768;
    file.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;if(GetOpenFileNameW(&file))open_path(s,path);
}
static void switch_tab(host_window_t *s,int direction)
{
    size_t count=ui_workspace_count(s->workspace),i,attempt;if(!count)return;
    for(i=0;i<count;++i)if(ui_workspace_instance_at(s->workspace,i)==ui_workspace_active(s->workspace))break;
    for(attempt=0;attempt<count;++attempt){i=direction>0?(i+1)%count:(i+count-1)%count;
        if(ui_workspace_activate(s->workspace,ui_workspace_instance_at(s->workspace,i))==UI_STATUS_OK)break;}schedule_refresh(s);
}
static void invoke_selected(host_window_t *s)
{
    uint64_t id=s->target,request=0;ui_status_t status;if(!s->command||!s->command[0])return;
    status=ui_workspace_invoke(s->workspace,id,s->command,s->params?s->params:"{}",&request);
    if(status!=UI_STATUS_OK){char line[128];(void)snprintf(line,sizeof(line),"Command failed (status %d)",(int)status);append_log(s,line);return;}
    if(s->completed_id!=id||s->completed_request!=request){s->last_id=id;s->last_request=request;}schedule_refresh(s);
}
static void transaction_action(host_window_t *s,const char *action)
{
    ui_app_instance_info_t info;ui_status_t status;uint64_t id=s->target;if(!get_instance(s,id,&info)||info.closing)return;
    if(!strcmp(action,"begin")){
        if(s->transaction_id){append_log(s,"Finish the existing transaction first.");return;}
        status=ui_assistant_begin_transaction(info.assistant,"Assistant edit",&s->transaction_id);if(status==UI_STATUS_OK)s->transaction_instance=id;
    }else if(!strcmp(action,"undo")){
        if(!s->undo_id||s->undo_instance!=id){append_log(s,"Select the committed transaction's original application instance.");return;}
        status=ui_assistant_undo_transaction(info.assistant,s->undo_id);if(status==UI_STATUS_OK)s->undo_id=s->undo_instance=0;
    }else{
        if(!s->transaction_id||s->transaction_instance!=id){append_log(s,"Select the transaction's original application instance.");return;}
        if(!strcmp(action,"commit"))status=ui_assistant_commit_transaction(info.assistant,s->transaction_id);
        else status=ui_assistant_rollback_transaction(info.assistant,s->transaction_id);
        if(status==UI_STATUS_OK){if(!strcmp(action,"commit")){s->undo_id=s->transaction_id;s->undo_instance=id;}
            s->transaction_id=s->transaction_instance=0;}}
    {char line[96];(void)snprintf(line,sizeof(line),"Transaction status: %d",(int)status);append_log(s,line);}
}
static void menu_popup(host_window_t *s)
{
    json_buffer_t json={0};int count=0;if(!create_popup(s,1,360,156))return;popup_start(s,&json,"框架菜单","应用菜单位于工作区上方",0);
    popup_item(&json,&count,"打开应用包… · Ctrl+Shift+O","open",0,NULL);
    popup_item(&json,&count,"关闭当前标签 · Ctrl+W","close",ui_workspace_active(s->workspace),NULL);
    popup_item(&json,&count,"下一个标签 · Ctrl+Tab","next",0,NULL);popup_item(&json,&count,"上一个标签 · Ctrl+Shift+Tab","previous",0,NULL);
    popup_item(&json,&count,"退出","exit",0,NULL);popup_finish(s,&json);
}
static void refresh_menu_states(host_window_t *s,uint64_t id)
{
    ui_app_instance_info_t info;ui_menu_entry_t *m;json_buffer_t json={0};int count=0;
    if(s->popup_kind!=1||!s->popup_view||!get_instance(s,id,&info))return;
    json_append(&json,"{\"type\":\"command-states\",\"items\":[");
    for(m=info.host->menus;m;m=m->next){ui_command_state_t state={0};state.size=sizeof(state);(void)ui_host_get_command_state(info.host,m->command_id,&state);
        if(count++)json_append(&json,",");json_append(&json,"{\"command\":");json_string(&json,m->command_id);
        json_format(&json,",\"visible\":%s,\"enabled\":%s,\"checked\":%s,\"busy\":%s,\"image\":\"%llu\"}",
            state.visible&&m->state.visible?"true":"false",state.enabled&&m->state.enabled?"true":"false",state.checked||m->state.checked?"true":"false",state.busy||m->state.busy?"true":"false",(unsigned long long)m->image_id);
    }json_append(&json,"]}");(void)post_json(s->popup_view,&json);
}
typedef struct command_popup { json_buffer_t *json;int count;uint64_t target; } command_popup_t;
static void command_popup_item(const ui_assistant_command_desc_t *command,void *data)
{command_popup_t *popup=(command_popup_t *)data;popup_item(popup->json,&popup->count,command->id,"select-command",popup->target,command->id);}
static void selector_popup(host_window_t *s,int commands)
{
    json_buffer_t json={0};ui_app_instance_info_t info;int count=0;size_t i;
    if(!create_popup(s,2,460,440))return;popup_start(s,&json,commands?"选择语义命令":"选择助手目标实例","目标不会随标签切换自动改变。",0);
    if(commands){if(get_instance(s,s->target,&info)&&!info.closing){command_popup_t query={&json,0,s->target};
            (void)ui_assistant_visit_commands(info.assistant,command_popup_item,&query);}}
    else for(i=0;i<ui_workspace_count(s->workspace);++i){uint64_t id=ui_workspace_instance_at(s->workspace,i);char label[512];
        if(get_instance(s,id,&info)&&!info.closing){(void)snprintf(label,sizeof(label),"%s #%llu",info.name_utf8,(unsigned long long)id);
            popup_item(&json,&count,label,"select-target",id,NULL);}}popup_finish(s,&json);
}
static void tooltip_popup(host_window_t *s,const char *id)
{
    const char *text=NULL;json_buffer_t json={0};RECT origin;ui_rect_t rect;ui_app_instance_info_t active;if(get_instance(s,ui_workspace_active(s->workspace),&active)&&active.host->menu_open_path)return;if(s->popup_hwnd&&s->popup_kind!=5)return;
    if(!strcmp(id?id:"","menu"))text="框架菜单";
    else if(!strcmp(id?id:"","open"))text="打开 .uapp 应用包 · Ctrl+Shift+O";
    else if(!strcmp(id?id:"","theme"))text="切换浅色 / 深色主题";
    else if(!strcmp(id?id:"","assistant-toggle"))text="显示或收起助手工作台";
    if(!text||!create_popup(s,5,300,100))return;
    if(ui_web_view_get_element_rect(s->view,id,&rect)==UI_STATUS_OK){POINT point={0,0};ClientToScreen(s->hwnd,&point);GetWindowRect(s->popup_hwnd,&origin);
        SetWindowPos(s->popup_hwnd,NULL,point.x+MulDiv(rect.x,(int)s->dpi,96),point.y+MulDiv(rect.y+rect.height+6,(int)s->dpi,96),
            origin.right-origin.left,origin.bottom-origin.top,SWP_NOZORDER|SWP_NOACTIVATE);}
    popup_start(s,&json,text,"",0);popup_finish(s,&json);
}
static int registered_command(const ui_host_t *host,const char *id)
{ui_command_entry_t *entry;for(entry=host->commands;entry;entry=entry->next)if(!strcmp(entry->id,id))return entry->state.visible&&entry->state.enabled&&!entry->state.busy;return 0;}
static void process_action(host_window_t *s,host_action_t *action)
{
    const char *name=action->action;uint64_t id=read_id(action->id);ui_app_instance_info_t info;
    if(action->popup_token){
        if(action->popup_token!=s->popup_token)return;if(action->token&&read_id(action->token)!=s->popup_token)return;
        if(!strcmp(name,"ready"))return;
        if(s->confirm_waiting){if(!strcmp(name,"confirm"))s->confirm_answer=action->answer&&!strcmp(action->answer,"yes")?1:-1;
            else if(!strcmp(name,"dismiss"))s->confirm_answer=-1;return;}
        if(strcmp(name,"tooltip-hide"))close_popup(s);
    }else if(s->confirm_waiting)return;
    if(!strcmp(name,"ready")){schedule_refresh(s);return;}
    if(!strcmp(name,"open")){close_popup(s);open_dialog(s);}
    else if(!strcmp(name,"close")){if(!id)id=ui_workspace_active(s->workspace);if(id)(void)ui_workspace_close(s->workspace,id,UI_APP_CLOSE_TAB);}
    else if(!strcmp(name,"activate")){if(id)(void)ui_workspace_activate(s->workspace,id);}
    else if(!strcmp(name,"next"))switch_tab(s,1);else if(!strcmp(name,"previous"))switch_tab(s,-1);
    else if(!strcmp(name,"exit"))PostMessageW(s->hwnd,WM_CLOSE,0,0);else if(!strcmp(name,"theme"))s->dark=!s->dark;
    else if(!strcmp(name,"assistant")){RECT client;GetClientRect(s->hwnd,&client);s->assistant_expanded=assistant_visible(s,MulDiv(client.right,96,(int)s->dpi))?-1:1;}
    else if(!strcmp(name,"menu")){if(s->popup_kind==1)close_popup(s);else menu_popup(s);return;}else if(!strcmp(name,"targets")){selector_popup(s,0);return;}
    else if(!strcmp(name,"app-shortcut")){
        uint64_t key=read_id(action->key),modifiers=read_id(action->modifiers);
        if(key&&key<=UINT32_MAX&&modifiers<=7&&get_instance(s,id,&info)&&info.active&&!info.closing)
            (void)ui_host_dispatch_shortcut(info.host,(uint32_t)key,(uint32_t)modifiers,action->editing&&!strcmp(action->editing,"1"));return;}
    else if(!strcmp(name,"app-menu")||!strcmp(name,"app-menu-hover")||!strcmp(name,"menu-more")||!strcmp(name,"tool-more")||!strcmp(name,"tool-tip")){
        ui_menu_popup_desc_t popup={0};ui_rect_t anchor={0};char element[160];
        if(!get_instance(s,id,&info)||!info.active||info.closing)return;
        snprintf(element,sizeof(element),"%s",action->panel?action->panel:"menu-more");
        if(ui_web_view_get_element_rect(s->view,element,&anchor)!=UI_STATUS_OK)return;
        anchor.x-=s->workspace_rect.x;anchor.y-=s->workspace_rect.y;popup.size=sizeof(popup);popup.anchor.size=sizeof(popup.anchor);popup.anchor.rect=anchor;
        if(!strcmp(name,"tool-more"))(void)ui_host_show_toolbar_menu(info.host,&popup.anchor,0);
        else if(!strcmp(name,"tool-tip")){ui_toolbar_item_entry_t *tool;const char *tool_id=action->panel&&strlen(action->panel)>5?action->panel+5:"";
            for(tool=info.host->toolbar_items;tool&&strcmp(tool->id,tool_id);tool=tool->next){}if(tool)(void)ui_host_show_tooltip(info.host,&popup.anchor,tool->title);}
        else{popup.path=action->menu?action->menu:"";if(!strcmp(name,"app-menu-hover")){if(!info.host->menu_open_path||!strcmp(info.host->menu_open_path,popup.path))return;}else if(info.host->menu_open_path&&!strcmp(info.host->menu_open_path,popup.path)){(void)ui_host_close_menu(info.host);return;}close_popup(s);info.host->menu_dark=s->dark;(void)ui_host_show_menu(info.host,&popup);}return;}
    else if(!strcmp(name,"tool-tip-hide")){if(get_instance(s,id,&info))(void)ui_host_hide_tooltip(info.host);return;}
    else if(!strcmp(name,"commands")){selector_popup(s,1);return;}
    else if(!strcmp(name,"select-target")){if(get_instance(s,id,&info)&&!info.closing)s->target=id;}
    else if(!strcmp(name,"select-command")){if(id==s->target&&action->command)replace_text(&s->command,action->command);}
    else if(!strcmp(name,"params")){
        if(action->params&&strlen(action->params)<=HOST_DISPLAY_TEXT_LIMIT)replace_text(&s->params,action->params);
        else show_error(s,"参数超过当前输入框的 4095 UTF-8 字节上限。");return;}
    else if(!strcmp(name,"invoke"))invoke_selected(s);
    else if(!strcmp(name,"cancel")){if(s->last_request)(void)ui_workspace_cancel(s->workspace,s->last_id,s->last_request);}
    else if(!strcmp(name,"snapshot")){const char *snapshot;if(get_instance(s,s->target,&info)&&!info.closing&&
        ui_assistant_get_state_snapshot(info.assistant,&snapshot)==UI_STATUS_OK)append_log(s,snapshot);}
    else if(!strcmp(name,"begin")||!strcmp(name,"commit")||!strcmp(name,"rollback")||!strcmp(name,"undo"))transaction_action(s,name);
    else if(!strcmp(name,"app-command")){if(action->command&&get_instance(s,id,&info)&&info.active&&!info.closing&&registered_command(info.host,action->command))
        (void)ui_host_invoke(info.host,action->command,"{}","web.user");}
    else if(!strcmp(name,"panel-float")){if(action->panel&&get_instance(s,id,&info)&&info.active&&!info.closing)
        (void)ui_native_shell_set_panel_floating(info.shell,action->panel,1);}
    else if(!strcmp(name,"panel-drag")||!strcmp(name,"panel-collapse")||!strcmp(name,"panel-split")||!strcmp(name,"side-split")||!strcmp(name,"layout-reset")){
        if(get_instance(s,id,&info)&&info.active&&!info.closing){ui_shell_t *shell=ui_host_get_shell(info.host);ui_panel_layout_t layout={0};layout.size=sizeof(layout);
            if(!strcmp(name,"layout-reset"))(void)ui_shell_reset_layout(shell);
            else if(!strcmp(name,"side-split"))(void)ui_shell_begin_splitter_drag(shell,action->panel&&!strcmp(action->panel,"left")?UI_LAYOUT_REGION_LEFT_SIDEBAR:UI_LAYOUT_REGION_RIGHT_SIDEBAR,NULL);
            else if(action->panel&&ui_shell_get_panel_layout(shell,action->panel,&layout)==UI_STATUS_OK){
                if(!strcmp(name,"panel-drag"))(void)ui_shell_begin_panel_drag(shell,action->panel);
                else if(!strcmp(name,"panel-split"))(void)ui_shell_begin_splitter_drag(shell,layout.dock_region,action->panel);
                else{layout.collapsed=!layout.collapsed;(void)ui_shell_set_panel_layout(shell,action->panel,&layout);}}
        }}
    else if(!strcmp(name,"tooltip")){tooltip_popup(s,action->id);return;}
    else if(!strcmp(name,"tooltip-hide")){if(s->popup_kind==5)close_popup(s);return;}
    else if(!strcmp(name,"dismiss"))close_popup(s);else return;schedule_refresh(s);
}
static void destroy_chrome(host_window_t *s)
{
    close_popup(s);if(s->view){ui_web_view_destroy(s->view);s->view=NULL;}
    if(s->backend){ui_light_web_backend_destroy(s->backend);s->backend=NULL;}
    if(s->chrome_host){ui_host_destroy(s->chrome_host);s->chrome_host=NULL;}
    free(s->command);free(s->params);free(s->log);s->command=s->params=s->log=NULL;
}
static LRESULT CALLBACK host_proc(HWND hwnd,UINT message,WPARAM wp,LPARAM lp)
{
    host_window_t *s=(host_window_t *)GetWindowLongPtrW(hwnd,GWLP_USERDATA);
    if(message==WM_NCCREATE){s=(host_window_t *)((CREATESTRUCTW *)lp)->lpCreateParams;s->hwnd=hwnd;SetWindowLongPtrW(hwnd,GWLP_USERDATA,(LONG_PTR)s);}
    if(!s)return DefWindowProcW(hwnd,message,wp,lp);
    switch(message){
    case WM_CREATE:{ui_workspace_config_t workspace;ui_host_config_t chrome;
        s->dpi=GetDpiForWindow(hwnd);if(!s->dpi)s->dpi=96;s->params=copy_text("{}");
        memset(&chrome,0,sizeof(chrome));chrome.size=sizeof(chrome);chrome.api_version=UI_FRAMEWORK_API_VERSION;chrome.native_parent=hwnd;
        s->chrome_host=ui_host_create(&chrome);if(!s->chrome_host)return -1;
        s->view=create_view(s,hwnd,&s->backend);if(!s->view){destroy_chrome(s);return -1;}s->web_hwnd=(HWND)ui_web_view_native_handle(s->view);
        memset(&workspace,0,sizeof(workspace));workspace.size=sizeof(workspace);workspace.native_parent=hwnd;
        workspace.user_data=s;workspace.result=on_result;workspace.event=on_event;workspace.progress=on_progress;
        workspace.confirm=on_confirm;workspace.changed=refresh_workspace;workspace.max_permission=UI_ASSISTANT_PERMISSION_DESTRUCTIVE;
        workspace.shell_mode=UI_WORKSPACE_SHELL_WEB;s->workspace=ui_workspace_create(&workspace);
        if(!s->workspace){destroy_chrome(s);return -1;}layout(s);return 0;}
    case WM_SIZE:layout(s);return 0;
    case WM_DPICHANGED:{RECT *rect=(RECT *)lp;s->dpi=LOWORD(wp);close_popup(s);
        SetWindowPos(hwnd,NULL,rect->left,rect->top,rect->right-rect->left,rect->bottom-rect->top,SWP_NOZORDER|SWP_NOACTIVATE);layout(s);return 0;}
    case HOST_ACTION_MESSAGE:{host_action_t *action=(host_action_t *)lp;if(s->workspace&&action)process_action(s,action);action_free(action);return 0;}
    case HOST_REFRESH_MESSAGE:s->refresh_pending=0;layout(s);return 0;
    case WM_COMMAND:switch(LOWORD(wp)){
        case ID_OPEN:open_dialog(s);return 0;case ID_CLOSE:(void)ui_workspace_close(s->workspace,ui_workspace_active(s->workspace),UI_APP_CLOSE_TAB);return 0;
        case ID_EXIT:PostMessageW(hwnd,WM_CLOSE,0,0);return 0;case ID_NEXT:switch_tab(s,1);return 0;case ID_PREVIOUS:switch_tab(s,-1);return 0;}break;
    case UI_WORKSPACE_WAKE_MESSAGE:
        if(!s->workspace)return 0;ui_workspace_poll(s->workspace);
        {const char *error=ui_workspace_last_error(s->workspace);if(error[0]&&strcmp(error,s->reported_error))show_error(s,error);}
        if(s->exiting){if(!ui_workspace_count(s->workspace)){ui_status_t status=ui_workspace_destroy(s->workspace);
                if(status==UI_STATUS_OK){s->workspace=NULL;DestroyWindow(hwnd);}else{s->exiting=0;show_error(s,ui_workspace_last_error(s->workspace));}}
            else{size_t i;for(i=0;i<ui_workspace_count(s->workspace);++i){ui_app_instance_info_t info;
                if(get_instance(s,ui_workspace_instance_at(s->workspace,i),&info)&&!info.closing)s->exiting=0;}}}return 0;
    case WM_CLOSE:
        if(s->confirm_waiting){s->confirm_answer=-1;return 0;}
        if(s->chrome_host&&s->chrome_host->dispatch_depth){PostMessageW(hwnd,WM_CLOSE,0,0);return 0;}
        s->exiting=1;if(ui_workspace_close_all(s->workspace)!=UI_STATUS_OK)s->exiting=0;PostMessageW(hwnd,UI_WORKSPACE_WAKE_MESSAGE,0,0);return 0;
    case WM_DESTROY:{MSG queued;destroy_chrome(s);
        while(PeekMessageW(&queued,hwnd,HOST_ACTION_MESSAGE,HOST_ACTION_MESSAGE,PM_REMOVE))action_free((host_action_t *)queued.lParam);
        PostQuitMessage(0);return 0;}
    }
    if(s->workspace){intptr_t result;if(ui_workspace_handle_message(s->workspace,hwnd,message,(uintptr_t)wp,(intptr_t)lp,&result)==UI_STATUS_OK)return (LRESULT)result;}
    return DefWindowProcW(hwnd,message,wp,lp);
}
static HWND create_host(host_window_t *state,HINSTANCE instance)
{
    WNDCLASSW wc;memset(&wc,0,sizeof(wc));wc.lpfnWndProc=host_proc;wc.hInstance=instance;
    wc.lpszClassName=L"UiFrameworkStandaloneHostV2";wc.hCursor=LoadCursorW(NULL,MAKEINTRESOURCEW(32512));
    if(!RegisterClassW(&wc)&&GetLastError()!=ERROR_CLASS_ALREADY_EXISTS)return NULL;
    return CreateWindowExW(0,wc.lpszClassName,L"C 应用框架 · Web",WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,
        CW_USEDEFAULT,CW_USEDEFAULT,1200,800,NULL,NULL,instance,state);
}
static int host_menu_key(host_window_t *state,const MSG *message)
{
    ui_app_instance_info_t info;ui_input_event_t input={0};HIMC ime;int composing=0;
    if(message->message!=WM_KEYDOWN&&message->message!=WM_SYSKEYDOWN&&message->message!=WM_KEYUP&&message->message!=WM_SYSKEYUP)return 0;
    if(!get_instance(state,ui_workspace_active(state->workspace),&info))return 0;
    ime=ImmGetContext(message->hwnd);if(ime){composing=ImmGetCompositionStringW(ime,GCS_COMPSTR,NULL,0)>0;ImmReleaseContext(message->hwnd,ime);}if(composing)return 0;
    input.size=sizeof(input);input.key_code=(uint32_t)message->wParam;input.kind=(message->message==WM_KEYUP||message->message==WM_SYSKEYUP)?UI_INPUT_KEY_UP:UI_INPUT_KEY_DOWN;
    if(GetKeyState(VK_CONTROL)&0x8000)input.modifiers|=UI_INPUT_MODIFIER_CONTROL;
    if(GetKeyState(VK_SHIFT)&0x8000)input.modifiers|=UI_INPUT_MODIFIER_SHIFT;
    if(GetKeyState(VK_MENU)&0x8000)input.modifiers|=UI_INPUT_MODIFIER_ALT;
    if(ui_host_menu_dispatch_input(info.host,&input)==UI_STATUS_OK)return 1;
    if(input.kind==UI_INPUT_KEY_DOWN){ui_web_view_t *view;
        for(view=info.host->web_views;view;view=view->host_next){HWND native=(HWND)ui_web_view_native_handle(view);
            if(native&&IsChild(native,message->hwnd)&&ui_host_dispatch_shortcut(info.host,input.key_code,input.modifiers,1))return 1;}}
    return 0;
}
#ifndef UI_HOST_TEST
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE previous,PWSTR command,int show)
{
    host_window_t state;HWND hwnd;MSG message={0};int count,i;wchar_t **args;
    ACCEL keys[]={{FVIRTKEY|FCONTROL|FSHIFT,'O',ID_OPEN},{FVIRTKEY|FCONTROL,'W',ID_CLOSE},
        {FVIRTKEY|FCONTROL,VK_TAB,ID_NEXT},{FVIRTKEY|FCONTROL|FSHIFT,VK_TAB,ID_PREVIOUS}};HACCEL accelerators;
    (void)previous;(void)command;if(ui_framework_initialize()!=UI_STATUS_OK)return 1;if(FAILED(CoInitializeEx(NULL,COINIT_APARTMENTTHREADED)))return 1;memset(&state,0,sizeof(state));
    hwnd=create_host(&state,instance);if(!hwnd){CoUninitialize();return 1;}ShowWindow(hwnd,show);UpdateWindow(hwnd);
    args=CommandLineToArgvW(GetCommandLineW(),&count);if(args){for(i=1;i<count;++i)open_path(&state,args[i]);LocalFree(args);}
    accelerators=CreateAcceleratorTableW(keys,(int)(sizeof(keys)/sizeof(keys[0])));
    while(GetMessageW(&message,NULL,0,0)>0){if(!TranslateAcceleratorW(hwnd,accelerators,&message)&&!host_menu_key(&state,&message)){TranslateMessage(&message);DispatchMessageW(&message);}}
    DestroyAcceleratorTable(accelerators);CoUninitialize();return (int)message.wParam;
}
#endif
