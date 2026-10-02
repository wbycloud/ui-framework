#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui_framework/application.h"

enum { ID_OPEN=0x1001, ID_CLOSE, ID_EXIT, ID_NEXT, ID_PREVIOUS,
       ID_TARGET=0x1101, ID_COMMAND, ID_INVOKE, ID_CANCEL, ID_SNAPSHOT,
       ID_ASSISTANT_TOGGLE, ID_PARAMS, ID_BEGIN, ID_COMMIT, ID_ROLLBACK, ID_UNDO };
typedef struct host_window {
    HWND hwnd, tabs, close_tab, empty, panel, toggle;
    HWND target, command, params, schema, log, invoke, cancel, snapshot;
    HWND target_label, command_label, params_label, begin, commit, rollback, undo;
    HFONT font;
    HMENU empty_menu;
    ui_workspace_t *workspace;
    uint32_t dpi;
    uint64_t last_id, last_request, completed_id, completed_request;
    uint64_t transaction_id, transaction_instance;
    uint64_t undo_id, undo_instance;
    int refreshing, exiting, assistant_expanded, assistant_scroll;
    char reported_error[512];
} host_window_t;

static wchar_t *to_wide(const char *s)
{
    int n; wchar_t *p;
    if (!s) s = "";
    n = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s, -1, NULL, 0);
    if (!n) return NULL;
    p = (wchar_t *)malloc((size_t)n*sizeof(*p));
    if (p) MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s, -1, p, n);
    return p;
}
static char *to_utf8(const wchar_t *s)
{
    int n = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, s, -1, NULL, 0, NULL, NULL);
    char *p;
    if (!n) return NULL;
    p = (char *)malloc((size_t)n);
    if (p) WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, s, -1, p, n, NULL, NULL);
    return p;
}
static void text_utf8(HWND hwnd, const char *text)
{
    wchar_t *wide = to_wide(text); if (wide) { SetWindowTextW(hwnd, wide); free(wide); }
}
static void append_log(host_window_t *s, const char *text)
{
    wchar_t *wide = to_wide(text); int length = GetWindowTextLengthW(s->log);
    if (!wide) return;
    if (length > 32000) { SetWindowTextW(s->log,L""); length=0; }
    SendMessageW(s->log, EM_SETSEL, (WPARAM)length, (LPARAM)length);
    SendMessageW(s->log, EM_REPLACESEL, FALSE, (LPARAM)wide);
    SendMessageW(s->log, EM_REPLACESEL, FALSE, (LPARAM)L"\r\n"); free(wide);
}
static void show_error(host_window_t *s, const char *text)
{
    wchar_t *wide=to_wide(text); append_log(s,text);
    (void)snprintf(s->reported_error,sizeof(s->reported_error),"%s",text);
    MessageBoxW(s->hwnd,wide?wide:L"操作失败",L"应用框架",MB_OK|MB_ICONERROR); free(wide);
}
static HMENU common_menu(void)
{
    HMENU menu=CreatePopupMenu();
    AppendMenuW(menu,MF_STRING,ID_OPEN,L"打开应用…\tCtrl+O");
    AppendMenuW(menu,MF_STRING,ID_CLOSE,L"关闭当前标签\tCtrl+W");
    AppendMenuW(menu,MF_STRING,ID_NEXT,L"下一个标签\tCtrl+Tab");
    AppendMenuW(menu,MF_STRING,ID_PREVIOUS,L"上一个标签\tCtrl+Shift+Tab");
    AppendMenuW(menu,MF_SEPARATOR,0,NULL);
    AppendMenuW(menu,MF_STRING,ID_EXIT,L"退出"); return menu;
}
static void install_menu(host_window_t *s)
{
    ui_app_instance_info_t info; HMENU menu=s->empty_menu;
    ZeroMemory(&info,sizeof(info)); info.size=sizeof(info);
    if (ui_workspace_get_instance(s->workspace,ui_workspace_active(s->workspace),&info)==UI_STATUS_OK)
        menu=(HMENU)ui_native_shell_menu_handle(info.shell);
    if (!menu) menu=s->empty_menu;
    if (GetMenuState(menu,ID_OPEN,MF_BYCOMMAND)==(UINT)-1)
        InsertMenuW(menu,0,MF_BYPOSITION|MF_POPUP,(UINT_PTR)common_menu(),L"框架");
    SetMenu(s->hwnd,menu); DrawMenuBar(s->hwnd);
}
static uint64_t selected_target(host_window_t *s)
{
    LRESULT index=SendMessageW(s->target,CB_GETCURSEL,0,0);
    return index==CB_ERR?0:(uint64_t)SendMessageW(s->target,CB_GETITEMDATA,(WPARAM)index,0);
}
static char *selected_command(host_window_t *s)
{
    LRESULT index=SendMessageW(s->command,CB_GETCURSEL,0,0);
    wchar_t buffer[256];
    if (index==CB_ERR || SendMessageW(s->command,CB_GETLBTEXTLEN,(WPARAM)index,0)>=256) return NULL;
    SendMessageW(s->command,CB_GETLBTEXT,(WPARAM)index,(LPARAM)buffer);
    return to_utf8(buffer);
}
static void visit_command(const ui_assistant_command_desc_t *command, void *data)
{
    host_window_t *s=(host_window_t *)data; wchar_t *wide=to_wide(command->id);
    if (wide) { SendMessageW(s->command,CB_ADDSTRING,0,(LPARAM)wide); free(wide); }
}
typedef struct schema_query { const char *id; HWND edit; } schema_query_t;
static void visit_schema(const ui_assistant_command_desc_t *command, void *data)
{
    schema_query_t *q=(schema_query_t *)data;
    if (strcmp(command->id,q->id)==0) text_utf8(q->edit,command->params_schema_json);
}
static void refresh_schema(host_window_t *s)
{
    char *command=selected_command(s); ui_app_instance_info_t info;
    ZeroMemory(&info,sizeof(info)); info.size=sizeof(info); SetWindowTextW(s->schema,L"");
    if (command && ui_workspace_get_instance(s->workspace,selected_target(s),&info)==UI_STATUS_OK) {
        schema_query_t query={command,s->schema};
        (void)ui_assistant_visit_commands(info.assistant,visit_schema,&query);
    }
    free(command);
}
static void refresh_commands(host_window_t *s)
{
    ui_app_instance_info_t info; char *old=selected_command(s); wchar_t *old_wide;
    ZeroMemory(&info,sizeof(info)); info.size=sizeof(info);
    SendMessageW(s->command,CB_RESETCONTENT,0,0);
    if (ui_workspace_get_instance(s->workspace,selected_target(s),&info)==UI_STATUS_OK) {
        (void)ui_assistant_visit_commands(info.assistant,visit_command,s);
        EnableWindow(s->invoke,!info.closing);
    } else EnableWindow(s->invoke,FALSE);
    SendMessageW(s->command,CB_SETCURSEL,0,0);
    old_wide=to_wide(old);
    if (old && old_wide) {
        LRESULT index=SendMessageW(s->command,CB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)old_wide);
        if (index!=CB_ERR) SendMessageW(s->command,CB_SETCURSEL,(WPARAM)index,0);
    }
    free(old); free(old_wide); refresh_schema(s);
}
static void layout(host_window_t *s);
static void refresh_workspace(void *data)
{
    host_window_t *s=(host_window_t *)data; size_t i,count;
    uint64_t target=selected_target(s); int target_index=-1;
    if (!s->workspace || s->refreshing) return;
    s->refreshing=1;
    {
        ui_app_instance_info_t info;ZeroMemory(&info,sizeof(info));info.size=sizeof(info);
        if(s->transaction_id && ui_workspace_get_instance(s->workspace,s->transaction_instance,&info)!=UI_STATUS_OK)
            s->transaction_id=s->transaction_instance=0;
        if(s->undo_id && ui_workspace_get_instance(s->workspace,s->undo_instance,&info)!=UI_STATUS_OK)
            s->undo_id=s->undo_instance=0;
        if(s->last_request && ui_workspace_get_instance(s->workspace,s->last_id,&info)!=UI_STATUS_OK){
            s->last_request=0;EnableWindow(s->cancel,FALSE);
        }
    }
    SendMessageW(s->tabs,TCM_DELETEALLITEMS,0,0);
    SendMessageW(s->target,CB_RESETCONTENT,0,0);
    count=ui_workspace_count(s->workspace);
    for (i=0;i<count;++i) {
        uint64_t id=ui_workspace_instance_at(s->workspace,i);
        ui_app_instance_info_t info; TCITEMW tab; wchar_t label[320],*name;
        ZeroMemory(&info,sizeof(info)); info.size=sizeof(info);
        if (ui_workspace_get_instance(s->workspace,id,&info)!=UI_STATUS_OK) continue;
        name=to_wide(info.name_utf8);
        (void)swprintf(label,320,L"%ls #%llu%ls",name?name:L"应用",(unsigned long long)id,
                       info.closing?L"（关闭中）":L""); free(name);
        ZeroMemory(&tab,sizeof(tab)); tab.mask=TCIF_TEXT|TCIF_PARAM;
        tab.pszText=label; tab.lParam=(LPARAM)id;
        SendMessageW(s->tabs,TCM_INSERTITEMW,(WPARAM)i,(LPARAM)&tab);
        if (info.active) SendMessageW(s->tabs,TCM_SETCURSEL,(WPARAM)i,0);
        { LRESULT index=SendMessageW(s->target,CB_ADDSTRING,0,(LPARAM)label);
          SendMessageW(s->target,CB_SETITEMDATA,(WPARAM)index,(LPARAM)id);
          if (id==target) target_index=(int)index;
          if (target_index<0 && info.active && !target) target_index=(int)index; }
    }
    if (target_index<0 && count) target_index=0;
    SendMessageW(s->target,CB_SETCURSEL,(WPARAM)target_index,0);
    refresh_commands(s); install_menu(s);
    ShowWindow(s->empty,count?SW_HIDE:SW_SHOW); EnableWindow(s->close_tab,count!=0);
    layout(s); s->refreshing=0;
}
static void on_result(uint64_t id,const ui_result_t *result,void *data)
{
    host_window_t *s=(host_window_t *)data; char line[4096];
    (void)snprintf(line,sizeof(line),"[#%llu / request %llu] %s: %s %s",
        (unsigned long long)id,(unsigned long long)result->request_id,result->command_id,
        result->success?"OK":"FAILED",result->result_json);
    append_log(s,line); s->completed_id=id; s->completed_request=result->request_id;
    if (s->last_id==id && s->last_request==result->request_id) { s->last_request=0; EnableWindow(s->cancel,FALSE); }
}
static void on_event(uint64_t id,const char *event,const char *json,void *data)
{
    char line[2048]; host_window_t *s=(host_window_t *)data;
    if (strncmp(event,"ui.host.",8)==0) return;
    (void)snprintf(line,sizeof(line),"[#%llu] %s %s",(unsigned long long)id,event,json);
    append_log(s,line);
}
static void on_progress(uint64_t id,uint64_t request,int percent,const char *text,void *data)
{
    char line[1024]; host_window_t *s=(host_window_t *)data;
    (void)snprintf(line,sizeof(line),"[#%llu / request %llu] %d%% %s",
        (unsigned long long)id,(unsigned long long)request,percent,text); append_log(s,line);
}
static int on_confirm(uint64_t id,const char *command,const char *json,
    ui_assistant_permission_t permission,void *data)
{
    host_window_t *s=(host_window_t *)data; wchar_t *wc=to_wide(command),*wj=to_wide(json),prompt[1800];
    (void)permission;
    (void)swprintf(prompt,1800,L"允许助手操作应用实例 #%llu？\n\n命令：%.200ls\n参数：%.1200ls",
        (unsigned long long)id,wc?wc:L"",wj?wj:L""); free(wc); free(wj);
    return MessageBoxW(s->hwnd,prompt,L"确认助手操作",MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2)==IDYES;
}
static void set_font(host_window_t *s)
{
    HWND controls[]={s->tabs,s->close_tab,s->empty,s->toggle,s->target,s->command,s->params,
        s->schema,s->log,s->invoke,s->cancel,s->snapshot,s->target_label,s->command_label,
        s->params_label,s->begin,s->commit,s->rollback,s->undo};
    HFONT font=CreateFontW(-MulDiv(14,(int)s->dpi,96),0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,
        DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,
        DEFAULT_PITCH|FF_DONTCARE,L"Microsoft YaHei UI"); size_t i;
    if (!font) return;
    for (i=0;i<sizeof(controls)/sizeof(controls[0]);++i) SendMessageW(controls[i],WM_SETFONT,(WPARAM)font,TRUE);
    if (s->font) DeleteObject(s->font); s->font=font;
}
static void move_control(host_window_t *s,HWND hwnd,int x,int y,int width,int height)
{
    if (GetParent(hwnd)==s->panel) y-=s->assistant_scroll;
    MoveWindow(hwnd,MulDiv(x,(int)s->dpi,96),MulDiv(y,(int)s->dpi,96),
        MulDiv(width,(int)s->dpi,96),MulDiv(height,(int)s->dpi,96),TRUE);
}
static void layout(host_window_t *s)
{
    RECT client; int width,height,sidebar,main_width,full;
    ui_rect_t rect;
    if (!s->workspace) return;
    GetClientRect(s->hwnd,&client); width=MulDiv(client.right,96,(int)s->dpi);
    height=MulDiv(client.bottom,96,(int)s->dpi);
    full=s->assistant_expanded>0 || (s->assistant_expanded==0 && width>=800);
    sidebar=full?320:32;
    if (full && width<720) sidebar=width>360?width-360:width/2;
    main_width=width-sidebar; if(main_width<0)main_width=0;
    move_control(s,s->tabs,0,0,width>100?width-100:0,34);
    move_control(s,s->close_tab,width>100?width-100:0,2,100,30);
    move_control(s,s->empty,24,60,main_width>48?main_width-48:0,100);
    move_control(s,s->panel,main_width,34,sidebar,height>34?height-34:0);
    {
        SCROLLINFO scroll; int content_height=full?(height>640?height-34:606):(height>34?height-34:1);
        ZeroMemory(&scroll,sizeof(scroll));scroll.cbSize=sizeof(scroll);
        scroll.fMask=SIF_RANGE|SIF_PAGE|SIF_POS;scroll.nMin=0;scroll.nMax=content_height-1;
        scroll.nPage=(UINT)(height>34?height-34:0);scroll.nPos=full?s->assistant_scroll:0;
        SetScrollInfo(s->panel,SB_VERT,&scroll,TRUE);
        s->assistant_scroll=GetScrollPos(s->panel,SB_VERT);
    }
    move_control(s,s->toggle,2,2,sidebar>4?sidebar-4:0,30);
    SetWindowTextW(s->toggle,full?L"助手 / 收起":L"AI");
    { HWND controls[]={s->target,s->command,s->params,s->schema,s->log,s->invoke,s->cancel,
        s->snapshot,s->target_label,s->command_label,s->params_label,s->begin,s->commit,s->rollback,s->undo};
      size_t i; for(i=0;i<sizeof(controls)/sizeof(controls[0]);++i)ShowWindow(controls[i],full?SW_SHOW:SW_HIDE); }
    if(full) {
        RECT panel_client;int inner,half,quarter;
        GetClientRect(s->panel,&panel_client);inner=MulDiv(panel_client.right,96,(int)s->dpi)-16;
        if(inner<16)inner=16;half=(inner-4)/2;quarter=(inner-12)/4;
        move_control(s,s->target_label,8,38,inner,20); move_control(s,s->target,8,60,inner,180);
        move_control(s,s->command_label,8,92,inner,20); move_control(s,s->command,8,114,inner,180);
        move_control(s,s->schema,8,148,inner,54); move_control(s,s->params_label,8,208,inner,20);
        move_control(s,s->params,8,230,inner,60);
        move_control(s,s->invoke,8,296,half,30); move_control(s,s->cancel,12+half,296,half,30);
        move_control(s,s->snapshot,8,330,inner,28);
        move_control(s,s->begin,8,362,quarter,28); move_control(s,s->commit,12+quarter,362,quarter,28);
        move_control(s,s->rollback,16+2*quarter,362,quarter,28); move_control(s,s->undo,20+3*quarter,362,quarter,28);
        move_control(s,s->log,8,396,inner,height>640?height-438:204);
    }
    rect.x=0;rect.y=34;rect.width=main_width;rect.height=height>34?height-34:0;
    (void)ui_workspace_set_rect(s->workspace,&rect,s->dpi);
}
static void open_path(host_window_t *s,const wchar_t *path)
{
    char *utf8=to_utf8(path);uint64_t id;ui_status_t status;
    if(!utf8)return;
    status=ui_workspace_open(s->workspace,utf8,&id);free(utf8);
    if(status!=UI_STATUS_OK)show_error(s,ui_workspace_last_error(s->workspace));
}
static void open_dialog(host_window_t *s)
{
    OPENFILENAMEW file; wchar_t path[32768]=L"";
    ZeroMemory(&file,sizeof(file));file.lStructSize=sizeof(file);file.hwndOwner=s->hwnd;
    file.lpstrFilter=L"应用包 (*.uapp)\0*.uapp\0\0";file.lpstrFile=path;file.nMaxFile=32768;
    file.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
    if(GetOpenFileNameW(&file))open_path(s,path);
}
static void switch_tab(host_window_t *s,int direction)
{
    size_t count=ui_workspace_count(s->workspace),i,attempt;
    if(!count)return;
    for(i=0;i<count;++i)if(ui_workspace_instance_at(s->workspace,i)==ui_workspace_active(s->workspace))break;
    for(attempt=0;attempt<count;++attempt){
        i=direction>0?(i+1)%count:(i+count-1)%count;
        if(ui_workspace_activate(s->workspace,ui_workspace_instance_at(s->workspace,i))==UI_STATUS_OK)break;
    }
}
static void invoke_selected(host_window_t *s)
{
    uint64_t id=selected_target(s),request=0;char *command=selected_command(s),*json;
    int length=GetWindowTextLengthW(s->params);wchar_t *wide=(wchar_t *)malloc(((size_t)length+1)*sizeof(*wide));
    ui_status_t status;
    if(!command||!wide){free(command);free(wide);return;}
    GetWindowTextW(s->params,wide,length+1);json=to_utf8(wide);free(wide);
    if(!json){free(command);return;}
    status=ui_workspace_invoke(s->workspace,id,command,json,&request);free(command);free(json);
    if(status!=UI_STATUS_OK){char line[160];(void)snprintf(line,sizeof(line),"Command failed (status %d)",(int)status);append_log(s,line);return;}
    if(s->completed_id!=id||s->completed_request!=request){s->last_id=id;s->last_request=request;EnableWindow(s->cancel,TRUE);}
}
static void transaction_action(host_window_t *s,UINT action)
{
    ui_app_instance_info_t info;ui_status_t status;uint64_t id=selected_target(s);
    ZeroMemory(&info,sizeof(info));info.size=sizeof(info);
    if(ui_workspace_get_instance(s->workspace,id,&info)!=UI_STATUS_OK||info.closing)return;
    if(action==ID_BEGIN){
        if(s->transaction_id){append_log(s,"Finish the existing transaction first.");return;}
        status=ui_assistant_begin_transaction(info.assistant,"Assistant edit",&s->transaction_id);
        if(status==UI_STATUS_OK)s->transaction_instance=id;
    } else if(action==ID_UNDO) {
        if(!s->undo_id||s->undo_instance!=id){append_log(s,"Select the committed transaction's original application instance.");return;}
        status=ui_assistant_undo_transaction(info.assistant,s->undo_id);
        if(status==UI_STATUS_OK)s->undo_id=s->undo_instance=0;
    } else {
        if(!s->transaction_id||s->transaction_instance!=id){append_log(s,"Select the transaction's original application instance.");return;}
        if(action==ID_COMMIT)status=ui_assistant_commit_transaction(info.assistant,s->transaction_id);
        else if(action==ID_ROLLBACK)status=ui_assistant_rollback_transaction(info.assistant,s->transaction_id);
        else status=UI_STATUS_INVALID_ARGUMENT;
        if(status==UI_STATUS_OK){
            if(action==ID_COMMIT){s->undo_id=s->transaction_id;s->undo_instance=id;}
            s->transaction_id=s->transaction_instance=0;
        }
    }
    {char line[100];(void)snprintf(line,sizeof(line),"Transaction status: %d",(int)status);append_log(s,line);}
}
static HWND control(HWND parent,const wchar_t *class_name,const wchar_t *text,DWORD style,int id)
{
    return CreateWindowExW(0,class_name,text,WS_CHILD|WS_VISIBLE|style,0,0,1,1,parent,(HMENU)(INT_PTR)id,GetModuleHandleW(NULL),NULL);
}
static void create_controls(host_window_t *s)
{
    s->tabs=control(s->hwnd,WC_TABCONTROLW,L"",TCS_SINGLELINE|TCS_FOCUSNEVER,0);
    s->close_tab=control(s->hwnd,L"BUTTON",L"关闭标签",BS_PUSHBUTTON,ID_CLOSE);
    s->empty=control(s->hwnd,L"STATIC",L"应用框架\n\n选择“框架 → 打开应用”，加载 .uapp 应用包。\n可同时打开多个应用，并在上方标签切换。",SS_LEFT,0);
    /* Panel uses the same parent WndProc for child notifications via WM_COMMAND. */
    s->panel=control(s->hwnd,L"STATIC",L"",SS_NOTIFY|WS_VSCROLL,0);
    s->toggle=control(s->panel,L"BUTTON",L"助手",BS_PUSHBUTTON,ID_ASSISTANT_TOGGLE);
    s->target_label=control(s->panel,L"STATIC",L"目标应用实例",SS_LEFT,0);
    s->target=control(s->panel,L"COMBOBOX",L"",CBS_DROPDOWNLIST|WS_VSCROLL,ID_TARGET);
    s->command_label=control(s->panel,L"STATIC",L"语义命令",SS_LEFT,0);
    s->command=control(s->panel,L"COMBOBOX",L"",CBS_DROPDOWNLIST|WS_VSCROLL,ID_COMMAND);
    s->schema=control(s->panel,L"EDIT",L"",ES_MULTILINE|ES_READONLY|WS_BORDER|WS_VSCROLL,0);
    s->params_label=control(s->panel,L"STATIC",L"参数（JSON）",SS_LEFT,0);
    s->params=control(s->panel,L"EDIT",L"{}",ES_MULTILINE|WS_BORDER|WS_TABSTOP,ID_PARAMS);
    s->invoke=control(s->panel,L"BUTTON",L"调用",BS_PUSHBUTTON,ID_INVOKE);
    s->cancel=control(s->panel,L"BUTTON",L"取消任务",BS_PUSHBUTTON,ID_CANCEL);
    s->snapshot=control(s->panel,L"BUTTON",L"查看状态快照",BS_PUSHBUTTON,ID_SNAPSHOT);
    s->begin=control(s->panel,L"BUTTON",L"开始",BS_PUSHBUTTON,ID_BEGIN);
    s->commit=control(s->panel,L"BUTTON",L"提交",BS_PUSHBUTTON,ID_COMMIT);
    s->rollback=control(s->panel,L"BUTTON",L"回滚",BS_PUSHBUTTON,ID_ROLLBACK);
    s->undo=control(s->panel,L"BUTTON",L"撤销",BS_PUSHBUTTON,ID_UNDO);
    s->log=control(s->panel,L"EDIT",L"",ES_MULTILINE|ES_READONLY|WS_BORDER|WS_VSCROLL|ES_AUTOVSCROLL,0);
    SendMessageW(s->params,EM_SETLIMITTEXT,8192,0);EnableWindow(s->cancel,FALSE);
}
static LRESULT CALLBACK panel_subclass(HWND hwnd,UINT message,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR data)
{
    host_window_t *s=(host_window_t *)data;
    (void)id;
    if(message==WM_COMMAND)return SendMessageW(s->hwnd,message,wp,lp);
    if(message==WM_VSCROLL||message==WM_MOUSEWHEEL){
        SCROLLINFO scroll;int position;
        ZeroMemory(&scroll,sizeof(scroll));scroll.cbSize=sizeof(scroll);scroll.fMask=SIF_ALL;
        GetScrollInfo(hwnd,SB_VERT,&scroll);position=scroll.nPos;
        if(message==WM_MOUSEWHEEL)position-=GET_WHEEL_DELTA_WPARAM(wp)/WHEEL_DELTA*48;
        else switch(LOWORD(wp)){
            case SB_LINEUP:position-=24;break;case SB_LINEDOWN:position+=24;break;
            case SB_PAGEUP:position-=(int)scroll.nPage;break;case SB_PAGEDOWN:position+=(int)scroll.nPage;break;
            case SB_THUMBTRACK:case SB_THUMBPOSITION:position=scroll.nTrackPos;break;
            default:break;
        }
        scroll.fMask=SIF_POS;scroll.nPos=position;SetScrollInfo(hwnd,SB_VERT,&scroll,TRUE);
        s->assistant_scroll=GetScrollPos(hwnd,SB_VERT);layout(s);return 0;
    }
    if(message==WM_NCDESTROY)RemoveWindowSubclass(hwnd,panel_subclass,1);
    return DefSubclassProc(hwnd,message,wp,lp);
}
static LRESULT CALLBACK host_proc(HWND hwnd,UINT message,WPARAM wp,LPARAM lp)
{
    host_window_t *s=(host_window_t *)GetWindowLongPtrW(hwnd,GWLP_USERDATA);
    if(message==WM_NCCREATE){s=(host_window_t *)((CREATESTRUCTW *)lp)->lpCreateParams;s->hwnd=hwnd;SetWindowLongPtrW(hwnd,GWLP_USERDATA,(LONG_PTR)s);}
    if(!s)return DefWindowProcW(hwnd,message,wp,lp);
    switch(message){
    case WM_CREATE:{ui_workspace_config_t config;
        create_controls(s);SetWindowSubclass(s->panel,panel_subclass,1,(DWORD_PTR)s);
        s->dpi=GetDpiForWindow(hwnd);if(!s->dpi)s->dpi=96;set_font(s);
        s->empty_menu=CreateMenu();AppendMenuW(s->empty_menu,MF_POPUP,(UINT_PTR)common_menu(),L"框架");SetMenu(hwnd,s->empty_menu);
        ZeroMemory(&config,sizeof(config));config.size=sizeof(config);config.native_parent=hwnd;
        config.user_data=s;config.result=on_result;config.event=on_event;config.progress=on_progress;
        config.confirm=on_confirm;config.changed=refresh_workspace;config.max_permission=UI_ASSISTANT_PERMISSION_DESTRUCTIVE;
        s->workspace=ui_workspace_create(&config);if(!s->workspace)return -1;
        layout(s);return 0;}
    case WM_SIZE:layout(s);return 0;
    case WM_DPICHANGED:{RECT *rect=(RECT *)lp;s->dpi=LOWORD(wp);set_font(s);
        SetWindowPos(hwnd,NULL,rect->left,rect->top,rect->right-rect->left,rect->bottom-rect->top,SWP_NOZORDER|SWP_NOACTIVATE);layout(s);return 0;}
    case WM_NOTIFY:{NMHDR *header=(NMHDR *)lp;
        if(header->hwndFrom==s->tabs&&header->code==TCN_SELCHANGE){
            TCITEMW item;ZeroMemory(&item,sizeof(item));item.mask=TCIF_PARAM;
            SendMessageW(s->tabs,TCM_GETITEMW,SendMessageW(s->tabs,TCM_GETCURSEL,0,0),(LPARAM)&item);
            if(ui_workspace_activate(s->workspace,(uint64_t)item.lParam)!=UI_STATUS_OK)
                refresh_workspace(s);
            return 0;}break;}
    case WM_COMMAND:
        switch(LOWORD(wp)){
        case ID_OPEN:open_dialog(s);return 0;
        case ID_CLOSE:(void)ui_workspace_close(s->workspace,ui_workspace_active(s->workspace),UI_APP_CLOSE_TAB);return 0;
        case ID_EXIT:SendMessageW(hwnd,WM_CLOSE,0,0);return 0;
        case ID_NEXT:switch_tab(s,1);return 0;
        case ID_PREVIOUS:switch_tab(s,-1);return 0;
        case ID_ASSISTANT_TOGGLE:{RECT client;int width;
            GetClientRect(hwnd,&client);width=MulDiv(client.right,96,(int)s->dpi);
            s->assistant_expanded=(s->assistant_expanded>0 ||
                (s->assistant_expanded==0&&width>=800))?-1:1;layout(s);return 0;}
        case ID_TARGET:if(HIWORD(wp)==CBN_SELCHANGE)refresh_commands(s);return 0;
        case ID_COMMAND:if(HIWORD(wp)==CBN_SELCHANGE)refresh_schema(s);return 0;
        case ID_INVOKE:invoke_selected(s);return 0;
        case ID_CANCEL:if(s->last_request)(void)ui_workspace_cancel(s->workspace,s->last_id,s->last_request);return 0;
        case ID_SNAPSHOT:{ui_app_instance_info_t info;const char *snapshot;
            ZeroMemory(&info,sizeof(info));info.size=sizeof(info);
            if(ui_workspace_get_instance(s->workspace,selected_target(s),&info)==UI_STATUS_OK&&
                ui_assistant_get_state_snapshot(info.assistant,&snapshot)==UI_STATUS_OK)append_log(s,snapshot);return 0;}
        case ID_BEGIN:case ID_COMMIT:case ID_ROLLBACK:case ID_UNDO:transaction_action(s,LOWORD(wp));return 0;
        }break;
    case UI_WORKSPACE_WAKE_MESSAGE:
        ui_workspace_poll(s->workspace);
        {const char *error=ui_workspace_last_error(s->workspace);
         if(error[0] && strcmp(error,s->reported_error)!=0)show_error(s,error);}
        if(s->exiting){
            if(ui_workspace_count(s->workspace)==0){
                ui_status_t status=ui_workspace_destroy(s->workspace);
                if(status==UI_STATUS_OK){s->workspace=NULL;DestroyWindow(hwnd);}
                else{s->exiting=0;show_error(s,ui_workspace_last_error(s->workspace));}
            }else{size_t i;for(i=0;i<ui_workspace_count(s->workspace);++i){ui_app_instance_info_t info;
                ZeroMemory(&info,sizeof(info));info.size=sizeof(info);
                (void)ui_workspace_get_instance(s->workspace,ui_workspace_instance_at(s->workspace,i),&info);
                if(!info.closing)s->exiting=0;}}
        }return 0;
    case WM_CLOSE:
        s->exiting=1;
        if(ui_workspace_close_all(s->workspace)!=UI_STATUS_OK)s->exiting=0;
        PostMessageW(hwnd,UI_WORKSPACE_WAKE_MESSAGE,0,0);return 0;
    case WM_DESTROY:
        SetMenu(hwnd,NULL);if(s->empty_menu)DestroyMenu(s->empty_menu);
        if(s->font)DeleteObject(s->font);PostQuitMessage(0);return 0;
    }
    if(s->workspace){intptr_t result;
        if(ui_workspace_handle_message(s->workspace,hwnd,message,(uintptr_t)wp,(intptr_t)lp,&result)==UI_STATUS_OK)return (LRESULT)result;}
    return DefWindowProcW(hwnd,message,wp,lp);
}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE previous,PWSTR command,int show)
{
    host_window_t state;WNDCLASSW wc;INITCOMMONCONTROLSEX controls;HWND hwnd;MSG message;
    int count,i;wchar_t **args;ACCEL keys[]={
        {FVIRTKEY|FCONTROL,'O',ID_OPEN},{FVIRTKEY|FCONTROL,'W',ID_CLOSE},
        {FVIRTKEY|FCONTROL,VK_TAB,ID_NEXT},{FVIRTKEY|FCONTROL|FSHIFT,VK_TAB,ID_PREVIOUS}};
    HACCEL accelerators;
    (void)previous;(void)command;
    if(ui_framework_initialize()!=UI_STATUS_OK)return 1;
    ZeroMemory(&state,sizeof(state));ZeroMemory(&wc,sizeof(wc));
    controls.dwSize=sizeof(controls);controls.dwICC=ICC_TAB_CLASSES|ICC_BAR_CLASSES;InitCommonControlsEx(&controls);
    wc.lpfnWndProc=host_proc;wc.hInstance=instance;wc.lpszClassName=L"UiFrameworkStandaloneHostV1";
    wc.hCursor=LoadCursorW(NULL,MAKEINTRESOURCEW(32512));wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
    if(!RegisterClassW(&wc))return 1;
    hwnd=CreateWindowExW(0,wc.lpszClassName,L"C 应用框架",WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,
        CW_USEDEFAULT,CW_USEDEFAULT,1200,800,NULL,NULL,instance,&state);if(!hwnd)return 1;
    ShowWindow(hwnd,show);UpdateWindow(hwnd);
    args=CommandLineToArgvW(GetCommandLineW(),&count);
    if(args){for(i=1;i<count;++i)open_path(&state,args[i]);LocalFree(args);}
    accelerators=CreateAcceleratorTableW(keys,sizeof(keys)/sizeof(keys[0]));
    while(GetMessageW(&message,NULL,0,0)>0){
        if(!TranslateAcceleratorW(hwnd,accelerators,&message)){TranslateMessage(&message);DispatchMessageW(&message);}
    }
    DestroyAcceleratorTable(accelerators);return (int)message.wParam;
}
