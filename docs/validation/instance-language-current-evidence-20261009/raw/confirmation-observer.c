/* Complete application DLL; real confirmation input, not a business command. */
#define wmain stored_state_main
#include "../../tests/stateful_components.c"
#undef wmain
static host_window_t *observed;
static ui_host_t *owner,*active;
static wchar_t *evidence_home;
static int chinese;
static DWORD opened;
static void CALLBACK answer_confirmation(HWND window,UINT message,UINT_PTR timer,DWORD now)
{
    ui_element_presentation_t yes={0},no={0},title={0};(void)message;(void)now;
    if(!observed->confirm_waiting)return;
    yes.size=no.size=title.size=sizeof(yes);
    ui_status_t status=ui_web_view_get_presentation(observed->popup_view,"confirm-no",&no);
    if(status==UI_STATUS_PENDING&&GetTickCount()-opened<10000)return;
    KillTimer(window,timer);
    CHECK(status==UI_STATUS_OK&&no.visible&&no.clip.width>0&&no.clip.height>0);
    CHECK(ui_web_view_get_presentation(observed->popup_view,"confirm-yes",&yes)==UI_STATUS_OK&&!strcmp(yes.text_utf8,chinese?"允许":"Allow"));
    CHECK(ui_web_view_get_presentation(observed->popup_view,"popup-title",&title)==UI_STATUS_OK&&!strcmp(title.text_utf8,chinese?"允许助手执行此操作？":"Allow the assistant to run this operation?"));
    CHECK(!IsWindowEnabled(observed->hwnd));
    CHECK(ui_host_set_language(owner,chinese?"en-US":"zh-CN")==UI_STATUS_OK);
    CHECK(ui_host_set_language(active,chinese?"zh-CN":"en-US")==UI_STATUS_OK);pump(100);
    CHECK(!strcmp(observed->popup_language.language,chinese?"zh-CN":"en-US"));
    CHECK(ui_web_view_get_presentation(observed->popup_view,"confirm-no",&no)==UI_STATUS_OK&&!strcmp(no.text_utf8,chinese?"取消":"Cancel"));
    CHECK(ui_web_view_get_presentation(observed->popup_view,"confirm-yes",&yes)==UI_STATUS_OK&&!strcmp(yes.text_utf8,chinese?"允许":"Allow"));
    CHECK(ui_web_view_get_presentation(observed->popup_view,"popup-title",&title)==UI_STATUS_OK&&!strcmp(title.text_utf8,chinese?"允许助手执行此操作？":"Allow the assistant to run this operation?"));
    screenshot(observed->popup_hwnd,evidence_home,chinese?L"owned-chinese-confirm":L"owned-english-confirm");
    POINT point={MulDiv(no.clip.x+no.clip.width/2,(int)observed->dpi,96),MulDiv(no.clip.y+no.clip.height/2,(int)observed->dpi,96)};DWORD pid=0;
    CHECK(ClientToScreen(observed->popup_hwnd,&point));HWND hit=WindowFromPoint(point);GetWindowThreadProcessId(hit,&pid);CHECK(pid==GetCurrentProcessId());
    CHECK(mouse(point,MOUSEEVENTF_LEFTDOWN));pump(40);CHECK(mouse(point,MOUSEEVENTF_LEFTUP));pump(100);
    printf("OWNED_CONFIRM snapshot=%s active=%s hit%p pid%lu answer%d failures%d\n",chinese?"zh-CN":"en-US",observed->chrome_host->language.language,(void *)hit,pid,observed->confirm_answer,failures);
    CHECK(observed->confirm_answer==-1);if(!observed->confirm_answer)observed->confirm_answer=-1;
}
int wmain(int argc,wchar_t **argv)
{
    host_window_t h={0};ui_app_instance_info_t a={0},b={0};if(argc<5)return 2;setvbuf(stdout,NULL,_IONBF,0);
    CHECK(SUCCEEDED(CoInitializeEx(NULL,COINIT_APARTMENTTHREADED)));CHECK(ui_framework_initialize()==UI_STATUS_OK);
    SetEnvironmentVariableW(L"UI_API7_OSMESA_DLL",argv[2]);SetEnvironmentVariableW(L"UI_API7_BACKEND",argv[3]);SetEnvironmentVariableW(L"UI_STATE_EXAMPLE_HOME",argv[4]);
    HWND root=create_host(&h,GetModuleHandleW(NULL));CHECK(root!=NULL);if(!root)return 2;ShowWindow(root,SW_SHOW);SetWindowPos(root,HWND_TOPMOST,40,40,1200,800,0);SetForegroundWindow(root);layout(&h);pump(200);
    open_path(&h,argv[1]);pump(600);CHECK(get_instance(&h,ui_workspace_active(h.workspace),&a));open_path(&h,argv[1]);pump(600);CHECK(get_instance(&h,ui_workspace_active(h.workspace),&b));if(!a.host||!b.host)return 2;
    observed=&h;owner=a.host;active=b.host;evidence_home=argv[4];CHECK(ui_host_set_language(a.host,"zh-CN")==UI_STATUS_OK);CHECK(ui_host_set_language(b.host,"en-US")==UI_STATUS_OK);pump(200);
    uint64_t target=h.target;HWND content=(HWND)ui_content_slot_native_handle(ui_shell_get_content_slot(ui_host_get_shell(a.host),"viewport"));
    for(int i=0;i<2;++i){chinese=!i;opened=GetTickCount();CHECK(SetTimer(root,71,10,answer_confirmation)!=0);CHECK(!on_confirm(a.instance_id,"test.render","{}",UI_ASSISTANT_PERMISSION_DESTRUCTIVE,&h));CHECK(h.target==target&&ui_workspace_active(h.workspace)==b.instance_id&&IsWindowEnabled(root)&&h.popup_hwnd==NULL);CHECK(content==(HWND)ui_content_slot_native_handle(ui_shell_get_content_slot(ui_host_get_shell(a.host),"viewport")));}
    SendMessageW(root,WM_CLOSE,0,0);ULONGLONG end=GetTickCount64()+10000;while(IsWindow(root)&&GetTickCount64()<end)pump(1);CHECK(!IsWindow(root)&&GetModuleHandleW(L"stateful_components_app.dll")==NULL&&GetCapture()==NULL);CoUninitialize();printf("Complete DLL owned confirmation %ls failures%d\n",argv[3],failures);return failures?1:0;
}
