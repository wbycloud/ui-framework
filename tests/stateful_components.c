/* Fresh-process durable state test. Header width changes use real SendInput. */
#define UI_HOST_TEST 1
#include "../examples/framework_host/main.c"
static int failures;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"state line%d: %s\n",__LINE__,#x);++failures;}}while(0)
static void pump(DWORD ms)
{ULONGLONG end=GetTickCount64()+ms;while(GetTickCount64()<end){MSG m;unsigned n=0;ULONGLONG start=GetTickCount64();while(n<3000&&GetTickCount64()-start<100&&PeekMessageW(&m,NULL,0,0,PM_REMOVE)){++n;if(m.message!=WM_QUIT){TranslateMessage(&m);DispatchMessageW(&m);}}if(!n)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}}
static ui_status_t present(ui_component_t *c,const char *id,ui_element_presentation_t *p)
{ULONGLONG end=GetTickCount64()+10000;ui_status_t status;do{p->size=sizeof(*p);status=ui_component_get_presentation(c,id,p);if(status!=UI_STATUS_PENDING)return status;pump(1);}while(GetTickCount64()<end);return status;}
static int mouse(POINT point,DWORD action)
{INPUT e[2]={0};e[0].type=e[1].type=INPUT_MOUSE;e[0].mi.dx=MulDiv(point.x-GetSystemMetrics(SM_XVIRTUALSCREEN),65535,GetSystemMetrics(SM_CXVIRTUALSCREEN)-1);e[0].mi.dy=MulDiv(point.y-GetSystemMetrics(SM_YVIRTUALSCREEN),65535,GetSystemMetrics(SM_CYVIRTUALSCREEN)-1);e[0].mi.dwFlags=MOUSEEVENTF_MOVE|MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_VIRTUALDESK|MOUSEEVENTF_MOVE_NOCOALESCE;e[1].mi.dwFlags=action;return SendInput(action?2:1,e,sizeof(INPUT))==(action?2u:1u);}
static void resize_column(ui_app_instance_info_t *info,ui_component_t *table,uint32_t dpi)
{
    ui_element_presentation_t p={0};ui_rect_t slot_rect={0};POINT start,finish;DWORD pid=0;HWND hit;int before=0,after=0;
    CHECK(present(table,"resize-name",&p)==UI_STATUS_OK&&p.visible&&p.clip.width>0);
    /* Main-slot Web view is positioned in host coordinates; presentation is
     * view-local. The borrowed main container is the application's parent. */
    CHECK(ui_content_slot_get_rect(ui_shell_get_content_slot(ui_host_get_shell(info->host),NULL),&slot_rect)==UI_STATUS_OK);
    start=(POINT){MulDiv(slot_rect.x+p.clip.x+p.clip.width/2,(int)dpi,96),MulDiv(slot_rect.y+p.clip.y+p.clip.height/2,(int)dpi,96)};
    CHECK(ClientToScreen((HWND)ui_content_slot_native_handle(ui_shell_get_content_slot(ui_host_get_shell(info->host),NULL)),&start));hit=WindowFromPoint(start);GetWindowThreadProcessId(hit,&pid);CHECK(pid==GetCurrentProcessId());
    CHECK(ui_component_get_column_width(table,"name",&before)==UI_STATUS_OK);printf("HEADER_GEOMETRY width%d grip%d,%d %dx%d screen%ld,%ld\n",before,p.clip.x,p.clip.y,p.clip.width,p.clip.height,start.x,start.y);CHECK(mouse(start,MOUSEEVENTF_LEFTDOWN));pump(40);
    {wchar_t name[80];GUITHREADINFO gui={0};gui.cbSize=sizeof(gui);DWORD thread=GetWindowThreadProcessId(hit,NULL);CHECK(GetGUIThreadInfo(thread,&gui));GetClassNameW(hit,name,80);
     printf("CAPTURE hit%p class%ls thread%lu ours%lu GetCapture%p target-thread-capture%p\n",(void *)hit,name,thread,GetCurrentThreadId(),(void *)GetCapture(),(void *)gui.hwndCapture);
     /* Light owns native capture on this UI thread. Runtime's browser owns
      * DOM pointer capture, not a framework-thread HWND capture contract. */
     if(!wcscmp(name,L"UIFrameworkLightWeb"))CHECK(GetCapture()==hit);
    }
    finish=start;finish.x+=MulDiv(80,(int)dpi,96);CHECK(mouse(finish,0));pump(80);CHECK(mouse(finish,MOUSEEVENTF_LEFTUP));pump(80);
    CHECK(ui_component_get_column_width(table,"name",&after)==UI_STATUS_OK&&after==before+80);CHECK(GetCapture()==NULL);printf("ACTUAL_HEADER dpi%u width%d -> %d hit%p pid%lu\n",dpi,before,after,(void *)hit,pid);
}
static void screenshot(HWND window,const wchar_t *home,const wchar_t *mode)
{RECT r;HDC dc;HBITMAP bitmap;HGDIOBJ old;BITMAPINFO info={0};BITMAPFILEHEADER file={0};void *pixels;FILE *out=NULL;wchar_t path[4096];GetWindowRect(window,&r);int w=r.right-r.left,h=r.bottom-r.top;
 dc=CreateCompatibleDC(NULL);info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=w;info.bmiHeader.biHeight=-h;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,NULL,0);CHECK(bitmap!=NULL);if(!bitmap){DeleteDC(dc);return;}old=SelectObject(dc,bitmap);CHECK(PrintWindow(window,dc,2));GdiFlush();swprintf(path,4096,L"%s\\%s.bmp",home,mode);file.bfType=0x4d42;file.bfOffBits=sizeof(file)+sizeof(info.bmiHeader);file.bfSize=file.bfOffBits+(DWORD)w*(DWORD)h*4;CHECK(!_wfopen_s(&out,path,L"wb"));if(out){CHECK(fwrite(&file,sizeof(file),1,out)==1&&fwrite(&info.bmiHeader,sizeof(info.bmiHeader),1,out)==1&&fwrite(pixels,4,(size_t)w*h,out)==(size_t)w*h);fclose(out);}SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);}
static void invoke(ui_host_t *host,const char *command)
{CHECK(ui_host_invoke(host,command,"{}","state-test")!=0);pump(100);}
int wmain(int argc,wchar_t **argv)
{
    host_window_t h={0};HWND root;ui_app_instance_info_t info={0},other={0};ui_component_t *table,*form;ui_panel_layout_t panel={0};ui_cell_t draft={0};ui_component_state_t state={0};wchar_t provider[4096];int width=0;uint32_t dpi=argc>6?(uint32_t)_wtoi(argv[6]):96;const wchar_t *mode;
    if(argc<6)return 2;mode=argv[5];setvbuf(stdout,NULL,_IONBF,0);CHECK(SUCCEEDED(CoInitializeEx(NULL,COINIT_APARTMENTTHREADED)));CHECK(ui_framework_initialize()==UI_STATUS_OK);
    CHECK(GetFullPathNameW(argv[2],4096,provider,NULL)>0);SetEnvironmentVariableW(L"UI_API7_OSMESA_DLL",provider);SetEnvironmentVariableW(L"UI_API7_BACKEND",argv[3]);SetEnvironmentVariableW(L"UI_STATE_EXAMPLE_HOME",argv[4]);
    root=create_host(&h,GetModuleHandleW(NULL));CHECK(root!=NULL);if(!root)return 2;h.dpi=dpi;ShowWindow(root,SW_SHOW);SetWindowPos(root,HWND_TOPMOST,40,40,MulDiv(1200,(int)dpi,96),MulDiv(800,(int)dpi,96),0);SetForegroundWindow(root);layout(&h);
    if(!wcscmp(mode,L"init-failures")){const wchar_t *flags[]={L"UI_API7_FAIL_CREATE",L"UI_API7_FAIL_MOUNT"};char *package=to_utf8(argv[1]);for(int i=0;i<2;++i){uint64_t id=0;SetEnvironmentVariableW(flags[i],L"1");CHECK(ui_workspace_open(h.workspace,package,&id)==UI_STATUS_PLATFORM_ERROR);SetEnvironmentVariableW(flags[i],NULL);pump(100);CHECK(ui_workspace_count(h.workspace)==0&&GetModuleHandleW(L"stateful_components_app.dll")==NULL);}free(package);}
    open_path(&h,argv[1]);pump(500);CHECK(get_instance(&h,ui_workspace_active(h.workspace),&info));if(!info.host){fprintf(stderr,"LOAD %s\n",ui_workspace_last_error(h.workspace));return 2;}
    table=ui_component_find(info.host,"table");form=ui_component_find(info.host,"form");CHECK(table&&form);if(!table||!form)return 2;state.size=sizeof(state);CHECK(ui_component_get_state(table,&state)==UI_STATUS_OK&&state.total_count==100000&&state.cached_bytes<=2*1024*1024&&state.rendered_nodes<=1024);
    CHECK(ui_component_get_column_width(table,"name",&width)==UI_STATUS_OK);panel.size=sizeof(panel);CHECK(ui_shell_get_panel_layout(ui_host_get_shell(info.host),"tree",&panel)==UI_STATUS_OK);
    printf("RESTORED mode%ls dpi%u name%d treeCollapsed%d\n",mode,dpi,width,panel.collapsed);
    if(!wcscmp(mode,L"write")){CHECK(width==200&&!panel.collapsed);screenshot(root,argv[4],L"before");resize_column(&info,table,dpi);CHECK(ui_component_set_column_width(table,"c63",210)==UI_STATUS_OK);panel.collapsed=1;CHECK(ui_shell_set_panel_layout(ui_host_get_shell(info.host),"tree",&panel)==UI_STATUS_OK);invoke(info.host,"state.save");CHECK(!strcmp(ui_host_status_text(info.host),"State saved: profile A"));screenshot(root,argv[4],L"saved");}
    else if(!wcscmp(mode,L"restore")){ui_element_presentation_t header={0},cell={0};CHECK(width==280&&panel.collapsed);CHECK(ui_component_get_column_width(table,"c63",&width)==UI_STATUS_OK&&width==210);CHECK(present(table,"sort-name",&header)==UI_STATUS_OK);CHECK(present(table,"cell-9007199254741001-name",&cell)==UI_STATUS_OK&&cell.visible&&!strcmp(cell.text_utf8,"editable"));printf("STABLE_COLUMN name headerX%d cellX%d text%s\n",header.rect.x,cell.rect.x,cell.text_utf8);CHECK(header.rect.x==cell.rect.x);screenshot(root,argv[4],argc>7?argv[7]:L"restored");}
    else if(!wcscmp(mode,L"profile-b")){CHECK(width==360&&!panel.collapsed);}
    else if(!wcscmp(mode,L"evolve")){CHECK(width==280&&panel.collapsed);CHECK(ui_component_get_column_width(table,"c63",&width)==UI_STATUS_NOT_FOUND);CHECK(ui_component_get_column_width(table,"added",&width)==UI_STATUS_OK&&width==180);}
    else if(!wcscmp(mode,L"reset")){CHECK(width==280);invoke(info.host,"state.reset-column");CHECK(ui_component_get_column_width(table,"name",&width)==UI_STATUS_OK&&width==200);CHECK(ui_component_set_column_width(table,"name",280)==UI_STATUS_OK);invoke(info.host,"state.reset");CHECK(ui_component_get_column_width(table,"name",&width)==UI_STATUS_OK&&width==200);CHECK(ui_shell_get_panel_layout(ui_host_get_shell(info.host),"tree",&panel)==UI_STATUS_OK&&!panel.collapsed);invoke(info.host,"state.save");}
    else if(!wcscmp(mode,L"invalid")||!wcscmp(mode,L"defaults")){CHECK(width==200&&!panel.collapsed);if(!wcscmp(mode,L"invalid"))CHECK(strstr(ui_host_status_text(info.host),"State rejected")!=NULL);}
    else if(!wcscmp(mode,L"init-failures")){CHECK(width==200&&!panel.collapsed);}
    else if(!wcscmp(mode,L"locked-profile")){char *package=to_utf8(argv[1]);uint64_t id=0;SetEnvironmentVariableW(L"UI_STATE_EXAMPLE_PROFILE",L"A");CHECK(ui_workspace_open(h.workspace,package,&id)==UI_STATUS_ALREADY_EXISTS);SetEnvironmentVariableW(L"UI_STATE_EXAMPLE_PROFILE",NULL);free(package);CHECK(ui_workspace_count(h.workspace)==1&&ui_component_get_column_width(table,"name",&width)==UI_STATUS_OK&&width==280);}
    else if(!wcscmp(mode,L"isolation")){CHECK(width==280);open_path(&h,argv[1]);pump(500);CHECK(get_instance(&h,ui_workspace_active(h.workspace),&other)&&other.instance_id!=info.instance_id);ui_component_t *second=ui_component_find(other.host,"table");CHECK(ui_component_get_column_width(second,"name",&width)==UI_STATUS_OK&&width==200);resize_column(&other,second,dpi);resize_column(&other,second,dpi);invoke(other.host,"state.save");CHECK(!strcmp(ui_host_status_text(other.host),"State saved: profile B"));CHECK(ui_component_get_column_width(table,"name",&width)==UI_STATUS_OK&&width==280);}
    else if(!wcscmp(mode,L"reload")){uint64_t selected[2]={0};size_t count=0;ui_element_presentation_t p={0};HWND content=(HWND)ui_content_slot_native_handle(ui_shell_get_content_slot(ui_host_get_shell(info.host),"viewport"));draft.size=sizeof(draft);draft.kind=UI_VALUE_TEXT;draft.text="unsaved survives state restore";draft.flags=UI_VALUE_MODIFIED;CHECK(ui_component_set_field(form,"name",&draft)==UI_STATUS_OK);CHECK(ui_component_select(table,9007199254741001ULL)==UI_STATUS_OK);invoke(info.host,"state.load");CHECK(ui_component_get_field(form,"name",&draft)==UI_STATUS_OK&&!strcmp(draft.text,"unsaved survives state restore"));CHECK(ui_component_get_selection(table,selected,2,&count)==UI_STATUS_OK&&count==1&&selected[0]==9007199254741001ULL);CHECK(IsWindow(content)&&content==(HWND)ui_content_slot_native_handle(ui_shell_get_content_slot(ui_host_get_shell(info.host),"viewport")));invoke(info.host,"test.render");CHECK(strstr(h.result_summary,ui_language_text(info.host,"操作完成"))!=NULL);CHECK(present(ui_component_find(info.host,"viewport"),"preview-pixels",&p)==UI_STATUS_OK&&p.visible&&p.clip.width>0&&p.clip.height>0);}
    else CHECK(0);
    SendMessageW(root,WM_CLOSE,0,0);ULONGLONG end=GetTickCount64()+10000;while(IsWindow(root)&&GetTickCount64()<end)pump(10);CHECK(!IsWindow(root)&&GetCapture()==NULL&&GetModuleHandleW(L"stateful_components_app.dll")==NULL);pump(300);CoUninitialize();printf("Stateful actual %ls / %ls: %d failures\n",argv[3],mode,failures);return failures?1:0;
}
