/* Exercise the real host, resource page, JS bridge, window composition and
   application DLLs in one process. No test-only message transport is exposed. */
#define UI_HOST_TEST 1
#include "../examples/framework_host/main.c"

static int failures;
static host_window_t *confirm_host;
static uint64_t confirm_activate;
static int confirm_allow;

static void check(int condition,const char *text)
{if(!condition){fprintf(stderr,"Web host: %s (Windows error %lu)\n",text,(unsigned long)GetLastError());++failures;}}
static void pump(void)
{
    MSG message;unsigned dispatched=0;
    while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){
        if(message.message==WM_QUIT)continue;TranslateMessage(&message);DispatchMessageW(&message);
        if(++dispatched>2000){check(0,"message queue reaches idle");break;}
    }
}
static int click(ui_web_view_t *view,const char *id)
{
    ui_rect_t rect={0};ui_input_event_t event;int ok=ui_web_view_get_element_rect(view,id,&rect)==UI_STATUS_OK&&rect.width>0&&rect.height>0;
    check(ok,id);if(!ok)return 0;memset(&event,0,sizeof(event));event.size=sizeof(event);
    event.x=rect.x+rect.width/2;event.y=rect.y+rect.height/2;event.pointer_button=1;event.kind=UI_INPUT_POINTER_DOWN;
    check(ui_web_view_dispatch_input(view,&event)==UI_STATUS_OK,"dispatch real Web pointer down");event.kind=UI_INPUT_POINTER_UP;
    check(ui_web_view_dispatch_input(view,&event)==UI_STATUS_OK,"dispatch real Web pointer up");return 1;
}
static unsigned snapshot_value(host_window_t *s,uint64_t id,const char *key)
{
    ui_app_instance_info_t info;const char *snapshot=NULL,*position;char needle[80];unsigned value=(unsigned)-1;
    check(get_instance(s,id,&info),"snapshot instance exists");
    if(!info.assistant)return value;
    check(ui_assistant_get_state_snapshot(info.assistant,&snapshot)==UI_STATUS_OK,"get semantic snapshot");
    (void)snprintf(needle,sizeof(needle),"\"%s\":",key);position=snapshot?strstr(snapshot,needle):NULL;
    if(position)(void)sscanf(position+strlen(needle),"%u",&value);return value;
}
static BOOL CALLBACK reject_native_chrome(HWND window,LPARAM argument)
{
    wchar_t class_name[128];int *count=(int *)argument;GetClassNameW(window,class_name,128);
    if(!wcscmp(class_name,L"SysTabControl32")||!wcscmp(class_name,L"ToolbarWindow32")||!wcscmp(class_name,L"ComboBox"))++*count;
    return TRUE;
}
static void send_selection(host_window_t *s,const char *name,uint64_t id,const char *command)
{
    host_action_t action;char number[32];memset(&action,0,sizeof(action));
    (void)snprintf(number,sizeof(number),"%llu",(unsigned long long)id);
    action.action=(char *)name;action.id=number;action.command=(char *)command;process_action(s,&action);pump();
}
static void CALLBACK confirm_timer(HWND hwnd,UINT message,UINT_PTR timer,DWORD time)
{
    (void)message;(void)time;
    if(!confirm_host||!confirm_host->confirm_waiting)return;KillTimer(hwnd,timer);
    if(confirm_activate)(void)ui_workspace_activate(confirm_host->workspace,confirm_activate);
    (void)click(confirm_host->popup_view,confirm_allow?"confirm-yes":"confirm-no");
}
static void test_bridge(void)
{
    host_action_t *action;json_buffer_t json={0};
    action=parse_action("{\"action\":\"params\",\"params\":\"a\\\"b\\\\c\\n\\u4e2d\\ud83d\\ude00\"}");
    check(action&&action->params&&!strcmp(action->params,"a\"b\\c\n中😀"),"bridge decodes JSON escapes and surrogate pairs");action_free(action);
    check(parse_action("{\"action\":\"close\",\"action\":\"exit\"}")==NULL,"duplicate message fields rejected");
    check(parse_action("{\"action\":\"close\",\"id\":2}")==NULL,"numeric IDs rejected");
    check(parse_action("{\"action\":\"invoke\",\"pointer\":\"1\"}")==NULL,"unknown message fields rejected");
    check(parse_action("{\"action\":\"params\",\"params\":\"\\u0000\"}")==NULL,"embedded NUL rejected");
    check(parse_action("{\"action\":\"params\",\"params\":\"\\ud800\"}")==NULL,"unpaired surrogate rejected");
    check(parse_action("{\"action\":\"close\",}")==NULL,"trailing comma rejected");
    check(read_id("18446744073709551615")==UINT64_MAX&&read_id("18446744073709551616")==0,"uint64 IDs parse without JavaScript precision loss");
    json_string(&json,"<script>\"\\\n中");check(json.text&&!strcmp(json.text,"\"<script>\\\"\\\\\\u000a中\""),"state JSON quotes strings as data");free(json.text);
}

int wmain(int argc,wchar_t **argv)
{
    host_window_t host;HWND root,panel_a,panel_b,surface_a,surface_b;ui_app_instance_info_t info;
    uint64_t first,second,cpp=0;ui_rect_t workspace,main,pixel,logical,control;char tab[80];int native_chrome=0;
    HRGN mask;int i,j;const int sizes[][2]={{1920,1080},{1280,720},{800,600},{640,480}};const uint32_t dpis[]={96,144,192};
    test_bridge();if(argc<2){fprintf(stderr,"Usage: web_host_frontend minimal_eda.uapp [cpp_fixture.uapp]\n");return 2;}
    check(ui_framework_initialize()==UI_STATUS_OK,"initialize per-monitor DPI");memset(&host,0,sizeof(host));
    root=create_host(&host,GetModuleHandleW(NULL));check(root!=NULL,"create standalone Web host");if(!root)return 1;
    ShowWindow(root,SW_HIDE);pump();check(ui_workspace_count(host.workspace)==0,"empty start");
    check(GetMenu(root)==NULL,"Web host never installs a Win32 menu");
    check(host.web_hwnd&&IsWindow(host.web_hwnd),"real lightweight Web rendering window");
    check(ui_web_view_get_element_rect(host.view,"workspace",&workspace)==UI_STATUS_OK&&workspace.width>400&&workspace.height>300,"HTML computes initial viewport");
    check(click(host.view,"theme"),"Web theme button exists");pump();check(host.dark,"theme action crosses JS/C bridge");
    open_path(&host,argv[1]);pump();first=ui_workspace_active(host.workspace);
    open_path(&host,argv[1]);pump();second=ui_workspace_active(host.workspace);
    check(first&&second&&first!=second&&ui_workspace_count(host.workspace)==2,"two independent EDA instances");
    EnumChildWindows(root,reject_native_chrome,(LPARAM)&native_chrome);check(!native_chrome,"no native tabs, toolbars or assistant combos");
    check(get_instance(&host,first,&info),"first instance info");panel_a=(HWND)ui_native_shell_panel_handle(info.shell,"eda.properties");
    surface_a=info.host->surfaces?(HWND)ui_surface_native_handle(info.host->surfaces):NULL;
    check(get_instance(&host,second,&info),"second instance info");panel_b=(HWND)ui_native_shell_panel_handle(info.shell,"eda.properties");
    surface_b=info.host->surfaces?(HWND)ui_surface_native_handle(info.host->surfaces):NULL;
    check(panel_a&&panel_b&&panel_a!=panel_b&&surface_a&&surface_b&&surface_a!=surface_b,"independent native panels and OpenGL surfaces");
    (void)snprintf(tab,sizeof(tab),"tab-%llu",(unsigned long long)first);click(host.view,tab);pump();
    check(ui_workspace_active(host.workspace)==first,"actual Web tab click switches instance");
    check(get_instance(&host,first,&info)&&ui_native_shell_panel_handle(info.shell,"eda.properties")==panel_a&&
        (HWND)ui_surface_native_handle(info.host->surfaces)==surface_a,"switch preserves HWND and GL surface");
    (void)snprintf(tab,sizeof(tab),"tab-%llu",(unsigned long long)second);click(host.view,tab);pump();
    send_selection(&host,"select-target",first,NULL);send_selection(&host,"select-command",first,"eda.add_block");
    click(host.view,"invoke");pump();check(snapshot_value(&host,first,"blocks")==1&&snapshot_value(&host,second,"blocks")==0,"assistant targets background instance");
    check(host.target==first&&strstr(host.log?host.log:"","eda.add_block"),"fixed target and routed semantic result");
    click(host.view,"tool-eda.add_block");pump();check(snapshot_value(&host,second,"blocks")==1,"Web toolbar calls active application command");
    {
        char long_text[12001];json_buffer_t popup={0};
        memset(long_text,'x',sizeof(long_text)-1);long_text[sizeof(long_text)-1]=0;
        append_log(&host,long_text);pump();
        check(host.log&&strlen(host.log)>HOST_DISPLAY_TEXT_LIMIT,"full log storage is retained");
        check(ui_web_view_get_element_rect(host.view,"tool-eda.add_block",&control)==UI_STATUS_OK&&control.width>0,
            "long read-only preview does not interrupt chrome refresh");
        check(create_popup(&host,4,500,300),"long detail popup");
        popup_start(&host,&popup,"Long detail",long_text,0);popup_finish(&host,&popup);
        check(click(host.popup_view,"popup-dismiss"),"all detail chunks render before actions");pump();
        check(!host.popup_hwnd,"long detail popup remains interactive");
    }
    click(host.view,"menu");pump();check(host.popup_hwnd&&host.popup_view&&GetWindow(host.popup_hwnd,GW_OWNER)==root,"menu is an owned Web popup");
    click(host.popup_view,"popup-dismiss");pump();check(!host.popup_hwnd,"popup dismiss is deferred out of JS callback");
    send_selection(&host,"select-command",first,"eda.clear");confirm_host=&host;confirm_activate=second;confirm_allow=0;
    SetTimer(root,50,10,confirm_timer);click(host.view,"invoke");pump();check(snapshot_value(&host,first,"blocks")==1,"Web confirmation denial preserves original target");
    confirm_allow=1;SetTimer(root,51,10,confirm_timer);click(host.view,"invoke");pump();
    check(snapshot_value(&host,first,"blocks")==0&&snapshot_value(&host,second,"blocks")==1,"Web confirmation approval routes to submitted target despite tab switch");
    confirm_host=NULL;
    if(argc>2){
        open_path(&host,argv[2]);pump();cpp=ui_workspace_active(host.workspace);
        check(cpp&&cpp!=first&&cpp!=second,"C++ application loads");send_selection(&host,"select-target",cpp,NULL);
        send_selection(&host,"select-command",cpp,"fixture.cpp.count");click(host.view,"begin");pump();
        check(host.transaction_id&&host.transaction_instance==cpp,"begin transaction through Web control");
        click(host.view,"invoke");pump();check(snapshot_value(&host,cpp,"count")==1,"C++ command works without OpenGL");
        click(host.view,"commit");pump();check(host.undo_id&&host.undo_instance==cpp&&!host.transaction_id,"commit records original instance");
        click(host.view,"undo");pump();check(snapshot_value(&host,cpp,"count")==0&&!host.undo_id,"Web undo uses committed original transaction");
    }
    (void)ui_workspace_activate(host.workspace,second);pump();
    for(i=0;i<4;++i)for(j=0;j<3;++j){
        /* Resize a real top window where Windows permits. Full logical/DPI
           matrices are exercised separately without desktop max-track limits. */
        MoveWindow(root,0,0,sizes[i][0],sizes[i][1],FALSE);host.dpi=dpis[j];layout(&host);pump();
        check(ui_web_view_get_element_rect(host.view,"workspace",&workspace)==UI_STATUS_OK,"responsive Web viewport query");
        check(workspace.width>=0&&workspace.height>=0,"responsive viewport has nonnegative size");
        check(get_instance(&host,second,&info),"DPI instance exists");
        (void)ui_host_get_rect(info.host,UI_LAYOUT_REGION_MAIN,&main);
        (void)ui_surface_get_rect(info.host->surfaces,&logical);(void)ui_surface_get_pixel_rect(info.host->surfaces,&pixel);
        check(logical.width==main.width&&logical.height==main.height,"GL tracks logical main area");
        check(pixel.width==MulDiv(main.x+main.width,(int)host.dpi,96)-MulDiv(main.x,(int)host.dpi,96),"GL framebuffer uses physical DPI size");
        mask=CreateRectRgn(0,0,0,0);check(GetWindowRgn(host.web_hwnd,mask)!=ERROR,"composition window has explicit region");
        if(main.width&&main.height)check(!PtInRegion(mask,MulDiv(workspace.x+main.x+main.width/2,(int)host.dpi,96),
            MulDiv(workspace.y+main.y+main.height/2,(int)host.dpi,96)),"Web chrome leaves native canvas input island");DeleteObject(mask);
        check(ui_web_view_get_element_rect(host.view,"menu",&control)==UI_STATUS_OK&&control.width>0,"public Web menu remains reachable");
    }
    host.dpi=GetDpiForWindow(root);layout(&host);pump();ShowWindow(root,SW_MINIMIZE);pump();ShowWindow(root,SW_RESTORE);pump();
    check(IsWindow(surface_a)&&IsWindow(surface_b),"minimize and restore preserve contexts");
    (void)snprintf(tab,sizeof(tab),"close-%llu",(unsigned long long)first);click(host.view,tab);pump();
    check(!get_instance(&host,first,&info)&&ui_workspace_active(host.workspace)==second,"closing background tab preserves active app");
    SendMessageW(root,WM_CLOSE,0,0);pump();check(!IsWindow(root),"host closes through normal lifecycle");
    if(failures){fprintf(stderr,"Web frontend failures: %d\n",failures);return 1;}
    puts("Web host frontend passed.");return 0;
}
