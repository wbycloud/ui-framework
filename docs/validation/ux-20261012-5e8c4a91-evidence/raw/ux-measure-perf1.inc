/* Desktop pixel observer. It is not a refresh/scanout timestamp source. */
typedef struct ux_sample {double ms;uint64_t hash;} ux_sample_t;
typedef struct ux_recorder {
    RECT rect;HANDLE ready,stop,thread;volatile LONG quit,armed,done;
    ux_sample_t samples[4096];volatile LONG count;double input_ms,stable_ms;
    HANDLE timer;
} ux_recorder_t;
static ux_recorder_t recorder;
static FILE *measurements,*sample_file;
static uint64_t pixel_hash(const unsigned char *pixels,size_t bytes)
{uint64_t value=1469598103934665603ULL;for(size_t i=0;i<bytes;i+=16){value^=pixels[i];value*=1099511628211ULL;}return value;}
static DWORD WINAPI observe_desktop(void *unused)
{
    HDC screen=GetDC(NULL),memory=CreateCompatibleDC(screen);(void)unused;
    while(!recorder.quit){
        if(WaitForSingleObject(recorder.ready,10)!=WAIT_OBJECT_0)continue;
        if(recorder.quit)break;
        RECT rect=recorder.rect;int width=rect.right-rect.left,height=rect.bottom-rect.top;
        BITMAPINFO info={0};void *pixels;HBITMAP bitmap;HGDIOBJ old;
        info.bmiHeader.biSize=sizeof(info.bmiHeader);info.bmiHeader.biWidth=width;info.bmiHeader.biHeight=-height;
        info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
        bitmap=CreateDIBSection(memory,&info,DIB_RGB_COLORS,&pixels,NULL,0);if(!bitmap){InterlockedExchange(&recorder.done,1);continue;}
        old=SelectObject(memory,bitmap);recorder.count=0;recorder.stable_ms=0;
        do{
            if(BitBlt(memory,0,0,width,height,screen,rect.left,rect.top,SRCCOPY|CAPTUREBLT)){
                GdiFlush();if(recorder.count<4096){ux_sample_t *sample=&recorder.samples[recorder.count];
                    sample->ms=clock_ms();sample->hash=pixel_hash(pixels,(size_t)width*(size_t)height*4);InterlockedIncrement(&recorder.count);}}
            if(!recorder.armed&&recorder.count>1){LONG last=recorder.count-1;
                if(recorder.samples[last].hash!=recorder.samples[last-1].hash||!recorder.stable_ms)recorder.stable_ms=recorder.samples[last].ms;
                if(recorder.samples[last].ms-recorder.stable_ms>=34)InterlockedExchange(&recorder.armed,1);}
            LARGE_INTEGER due;due.QuadPart=-20000;SetWaitableTimer(recorder.timer,&due,0,NULL,NULL,FALSE);HANDLE waits[2]={recorder.stop,recorder.timer};
            if(WaitForMultipleObjects(2,waits,FALSE,100)!=WAIT_OBJECT_0+1)break;
        }while(!recorder.quit);
        SelectObject(memory,old);DeleteObject(bitmap);InterlockedExchange(&recorder.done,1);
    }
    DeleteDC(memory);ReleaseDC(NULL,screen);return 0;
}
static void observer_start(RECT rect)
{
    CHECK(rect.right>rect.left&&rect.bottom>rect.top);if(rect.right<=rect.left)rect.right=rect.left+1;if(rect.bottom<=rect.top)rect.bottom=rect.top+1;
    recorder.rect=rect;recorder.done=recorder.armed=0;ResetEvent(recorder.stop);SetEvent(recorder.ready);
    double end=clock_ms()+3000;while(!recorder.armed&&!recorder.done&&clock_ms()<end)drain(1);CHECK(recorder.armed);
    recorder.input_ms=clock_ms();
}
static void observer_finish(const char *category,unsigned index,double validated_ms,int valid)
{
    double first=-1,max_gap=0;SetEvent(recorder.stop);while(!recorder.done)drain(0);
    for(unsigned i=1;i<(unsigned)recorder.count;++i){ux_sample_t *sample=&recorder.samples[i];
        if(sample->ms>=recorder.input_ms&&sample->hash!=recorder.samples[0].hash&&first<0)first=sample->ms-recorder.input_ms;
        double gap=sample->ms-recorder.samples[i-1].ms;if(gap>max_gap)max_gap=gap;
        fprintf(sample_file,"%s,%u,%.6f,%llu\n",category,index,sample->ms,(unsigned long long)sample->hash);}
    fprintf(measurements,"%s,%u,%.6f,%.6f,%.6f,%.6f,%u,%d,%u\n",category,index,recorder.input_ms,first,
        validated_ms-recorder.input_ms,max_gap,(unsigned)recorder.count,valid,message_count);
    fflush(measurements);fflush(sample_file);
    printf("UX_SAMPLE %s n%u feedbackMs%.3f stateMs%.3f sampleMaxGapMs%.3f valid%d dispatch%u\n",category,index,first,validated_ms-recorder.input_ms,max_gap,valid,message_count);
}
static RECT element_screen(ui_web_view_t *view,const ui_element_presentation_t *p)
{
    POINT origin={0,0};CHECK(ClientToScreen((HWND)ui_web_view_native_handle(view),&origin));
    RECT rect={origin.x+MulDiv(p->clip.x,(int)ux_host->dpi,96),origin.y+MulDiv(p->clip.y,(int)ux_host->dpi,96),
        origin.x+MulDiv(p->clip.x+p->clip.width,(int)ux_host->dpi,96),origin.y+MulDiv(p->clip.y+p->clip.height,(int)ux_host->dpi,96)};
    if(rect.right-rect.left>360)rect.right=rect.left+360;if(rect.bottom-rect.top>120)rect.bottom=rect.top+120;return rect;
}
static ui_web_view_t *component_view(ui_host_t *host,const char *id)
{
    for(ui_web_view_t *view=host->web_views;view;view=view->host_next){ui_element_presentation_t p={0};double end=clock_ms()+3000;ui_status_t status;
        do{p.size=sizeof(p);status=ui_web_view_get_presentation(view,id,&p);if(status!=UI_STATUS_PENDING)break;drain(1);}while(clock_ms()<end);
        if(status==UI_STATUS_OK&&p.visible)return view;}
    return NULL;
}
static POINT center(RECT rect)
{POINT point={(rect.left+rect.right)/2,(rect.top+rect.bottom)/2};return point;}
static void real_click(POINT point)
{CHECK(GetAncestor(WindowFromPoint(point),GA_ROOT)==ux_host->hwnd||GetWindow(GetAncestor(WindowFromPoint(point),GA_ROOT),GW_OWNER)!=NULL);
 CHECK(mouse(point,MOUSEEVENTF_LEFTDOWN));CHECK(mouse(point,MOUSEEVENTF_LEFTUP));}
static void control_key(WORD key)
{INPUT keys[4]={0};for(int i=0;i<4;++i){keys[i].type=INPUT_KEYBOARD;keys[i].ki.wVk=i==0||i==3?VK_CONTROL:key;keys[i].ki.dwFlags=i>=2?KEYEVENTF_KEYUP:0;}CHECK(SendInput(4,keys,sizeof(INPUT))==4);}
static HWND operation_popup;
static BOOL CALLBACK find_operation_popup(HWND hwnd,LPARAM unused)
{wchar_t klass[80];(void)unused;GetClassNameW(hwnd,klass,80);if(IsWindowVisible(hwnd)&&(!wcscmp(klass,L"UIFrameworkRegisteredMenu4")||!wcscmp(klass,L"UIFrameworkWebDialog3")))operation_popup=hwnd;return TRUE;}
static HWND active_operation_popup(void)
{operation_popup=NULL;EnumThreadWindows(GetCurrentThreadId(),find_operation_popup,0);return operation_popup;}
static void measure_scene(ui_app_instance_info_t *instance,const wchar_t *package)
{
    ui_component_t *table=ui_component_find(instance->host,"table"),*form=ui_component_find(instance->host,"form"),*dialog=ui_component_find(instance->host,"dialog");
    ui_web_view_t *form_view=component_view(instance->host,"field-name"),*table_view=component_view(instance->host,"sort-name");
    ui_element_presentation_t p={0};ui_component_state_t state={0};wchar_t path[4096];char menu_id[80];uint64_t hash=1469598103934665603ULL;
    const unsigned char *name=(const unsigned char *)"State";for(;*name;++name){hash^=*name;hash*=1099511628211ULL;}snprintf(menu_id,sizeof(menu_id),"app-menu-%llu",(unsigned long long)hash);
    CHECK(table&&form&&dialog&&form_view&&table_view);if(!table||!form||!dialog||!form_view||!table_view)return;
    swprintf(path,4096,L"%s\\measurements.csv",output);CHECK(!_wfopen_s(&measurements,path,L"wb"));swprintf(path,4096,L"%s\\desktop-samples.csv",output);CHECK(!_wfopen_s(&sample_file,path,L"wb"));
    if(!measurements||!sample_file)return;
    fputs("category,index,inputQpcMs,firstDesktopChangeMs,validatedStateMs,maxSampleGapMs,samples,valid,dispatch\n",measurements);
    fputs("category,index,qpcMs,pixelHash\n",sample_file);
    recorder.ready=CreateEventW(NULL,FALSE,FALSE,NULL);recorder.stop=CreateEventW(NULL,TRUE,FALSE,NULL);recorder.timer=CreateWaitableTimerExW(NULL,NULL,0x2,TIMER_ALL_ACCESS);CHECK(recorder.timer!=NULL);
    recorder.thread=CreateThread(NULL,0,observe_desktop,NULL,0,NULL);CHECK(recorder.thread!=NULL);
    for(unsigned i=0;i<30;++i){
        const char *category="host-hover";SetEnvironmentVariableA("UI_UX_PHASE",category);
        CHECK(chrome_presentation(i%2?"window-min":"assistant-toggle",&p)==UI_STATUS_OK);RECT rect=element_screen(ux_host->view,&p);
        observer_start(rect);CHECK(mouse(center(rect),0));double end=clock_ms()+1000;do{drain(1);}while(clock_ms()<end&&recorder.count<5);
        observer_finish(category,i,clock_ms(),1);
    }
    CHECK(mouse((POINT){60,1100},0));drain(30);
    for(unsigned i=0;i<30;++i){
        const char *category="host-click";SetEnvironmentVariableA("UI_UX_PHASE",category);int before=assistant_visible(ux_host);
        CHECK(chrome_presentation("assistant-toggle",&p)==UI_STATUS_OK);RECT rect=element_screen(ux_host->view,&p);observer_start(rect);real_click(center(rect));
        double end=clock_ms()+3000;do{drain(1);}while(clock_ms()<end&&assistant_visible(ux_host)==before);
        observer_finish(category,i,clock_ms(),assistant_visible(ux_host)!=before);CHECK(assistant_visible(ux_host)!=before);
    }
    if(assistant_visible(ux_host)){CHECK(chrome_presentation("assistant-toggle",&p)==UI_STATUS_OK);real_click(chrome_point(&p));drain(70);}
    for(unsigned i=0;i<20;++i){
        const char *category="menu-open";SetEnvironmentVariableA("UI_UX_PHASE",category);
        CHECK(chrome_presentation(menu_id,&p)==UI_STATUS_OK);RECT rect=element_screen(ux_host->view,&p);rect.bottom+=120;observer_start(rect);real_click(chrome_point(&p));
        ui_element_presentation_t item={0};double end=clock_ms()+3000;ui_status_t status;
        do{drain(1);item.size=sizeof(item);status=ui_host_menu_get_item_presentation(instance->host,"state.properties",&item);}while(clock_ms()<end&&(status!=UI_STATUS_OK||!item.visible));
        observer_finish(category,i,clock_ms(),status==UI_STATUS_OK&&item.visible);CHECK(status==UI_STATUS_OK&&item.visible);
        CHECK(ui_host_close_menu(instance->host)==UI_STATUS_OK);drain(30);
    }
    for(unsigned i=0;i<20;++i){
        const char *category="dialog-open";SetEnvironmentVariableA("UI_UX_PHASE",category);
        CHECK(chrome_presentation(menu_id,&p)==UI_STATUS_OK);real_click(chrome_point(&p));drain(40);
        ui_element_presentation_t item={0};item.size=sizeof(item);CHECK(ui_host_menu_get_item_presentation(instance->host,"state.properties",&item)==UI_STATUS_OK&&item.visible);
        HWND popup=active_operation_popup();CHECK(popup!=NULL);POINT origin={0,0};ClientToScreen(popup,&origin);
        POINT point={origin.x+MulDiv(item.clip.x+item.clip.width/2,(int)ux_host->dpi,96),origin.y+MulDiv(item.clip.y+item.clip.height/2,(int)ux_host->dpi,96)};
        RECT window;GetWindowRect(ux_host->hwnd,&window);RECT rect={window.left+300,window.top+200,window.left+660,window.top+320};
        observer_start(rect);real_click(point);double end=clock_ms()+3000;state.size=sizeof(state);
        do{drain(1);CHECK(ui_component_get_state(dialog,&state)==UI_STATUS_OK);}while(clock_ms()<end&&!state.modal);
        observer_finish(category,i,clock_ms(),state.modal);CHECK(state.modal);
        real_key(VK_ESCAPE,0);end=clock_ms()+3000;do{drain(1);ui_component_get_state(dialog,&state);}while(clock_ms()<end&&state.modal);CHECK(!state.modal);drain(30);
    }
    SetEnvironmentVariableA("UI_UX_PHASE","long-input");CHECK(present(form,"field-name",&p)==UI_STATUS_OK);
    RECT field_rect=element_screen(form_view,&p);real_click(center(field_rect));drain(30);
    const char *text="ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-END-UX-20261012";
    for(unsigned i=0;i<20;++i){
        control_key('A');drain(10);observer_start(field_rect);INPUT keys[400]={0};unsigned n=0;
        for(const unsigned char *at=(const unsigned char *)text;*at;++at){keys[n].type=keys[n+1].type=INPUT_KEYBOARD;keys[n].ki.wScan=keys[n+1].ki.wScan=*at;keys[n].ki.dwFlags=KEYEVENTF_UNICODE;keys[n+1].ki.dwFlags=KEYEVENTF_UNICODE|KEYEVENTF_KEYUP;n+=2;}
        CHECK(SendInput(n,keys,sizeof(INPUT))==n);ui_cell_t value={0};double end=clock_ms()+5000;value.size=sizeof(value);
        do{drain(1);ui_component_get_field(form,"name",&value);}while(clock_ms()<end&&strcmp(value.text?value.text:"",text));
        int valid=value.text&&!strcmp(value.text,text);observer_finish("long-input",i,clock_ms(),valid);CHECK(valid);
    }
    for(unsigned i=0;i<30;++i){
        const char *category=i%2?"text-home":"text-end";SetEnvironmentVariableA("UI_UX_PHASE",category);observer_start(field_rect);real_key(i%2?VK_HOME:VK_END,0);
        drain(2);ui_element_presentation_t check={0};CHECK(present(form,"field-name",&check)==UI_STATUS_OK&&check.focused);
        observer_finish(category,i,clock_ms(),check.focused);
    }
    CHECK(present(table,"scroll-v-track",&p)==UI_STATUS_OK);RECT rail_rect=element_screen(table_view,&p);rail_rect.left-=250;POINT wheel={rail_rect.left+20,rail_rect.top+30};
    for(unsigned i=0;i<30;++i){
        const char *category="table-wheel";SetEnvironmentVariableA("UI_UX_PHASE",category);state.size=sizeof(state);ui_component_get_state(table,&state);uint64_t before=state.first;
        observer_start(rail_rect);CHECK(mouse(wheel,0));INPUT input={0};input.type=INPUT_MOUSE;input.mi.dwFlags=MOUSEEVENTF_WHEEL;input.mi.mouseData=(DWORD)(i%2?360:-360);CHECK(SendInput(1,&input,sizeof(input))==1);
        double end=clock_ms()+3000;do{drain(1);ui_component_get_state(table,&state);}while(clock_ms()<end&&state.first==before);
        observer_finish(category,i,clock_ms(),state.first!=before);CHECK(state.total_count==100000&&state.cached_bytes<=2*1024*1024&&state.rendered_nodes<=1024);
    }
    for(unsigned i=0;i<8;++i){
        const char *category="scrollbar-drag";SetEnvironmentVariableA("UI_UX_PHASE",category);ui_element_presentation_t thumb={0},track={0};
        CHECK(present(table,"scroll-v-thumb",&thumb)==UI_STATUS_OK&&present(table,"scroll-v-track",&track)==UI_STATUS_OK);
        RECT thumb_rect=element_screen(table_view,&thumb);POINT begin=center(thumb_rect),origin={0,0};ClientToScreen((HWND)ui_web_view_native_handle(table_view),&origin);
        POINT finish={begin.x,origin.y+MulDiv(i%2?track.rect.y:track.rect.y+track.rect.height,(int)ux_host->dpi,96)};
        observer_start(rail_rect);CHECK(mouse(begin,MOUSEEVENTF_LEFTDOWN));drain(1);
        double started=clock_ms();
        for(int step=1;step<=20;++step){POINT point={begin.x,begin.y+(finish.y-begin.y)*step/20};CHECK(mouse(point,0));
            while(clock_ms()<started+step*1000.0/60.0)drain(1);}
        CHECK(mouse(finish,MOUSEEVENTF_LEFTUP));drain(2);state.size=sizeof(state);CHECK(ui_component_get_state(table,&state)==UI_STATUS_OK);
        observer_finish(category,i,clock_ms(),state.total_count==100000);CHECK(GetCapture()==NULL);
        printf("UX_RANGE first%llu rows%zu total%llu generation%llu\n",(unsigned long long)state.first,state.row_count,(unsigned long long)state.total_count,(unsigned long long)state.generation);
    }
    for(unsigned i=0;i<20;++i){
        const char *category="column-width";SetEnvironmentVariableA("UI_UX_PHASE",category);int before=0,after=0;CHECK(ui_component_get_column_width(table,"name",&before)==UI_STATUS_OK);
        CHECK(present(table,"resize-name",&p)==UI_STATUS_OK);RECT rect=element_screen(table_view,&p);POINT begin=center(rect),finish=begin;finish.x+=i%2?-40:40;rect.left-=180;
        observer_start(rect);CHECK(mouse(begin,MOUSEEVENTF_LEFTDOWN));drain(1);CHECK(mouse(finish,0));drain(1);CHECK(mouse(finish,MOUSEEVENTF_LEFTUP));
        double end=clock_ms()+3000;do{drain(1);ui_component_get_column_width(table,"name",&after);}while(clock_ms()<end&&before==after);
        observer_finish(category,i,clock_ms(),after==before+(i%2?-40:40));CHECK(after==before+(i%2?-40:40));CHECK(GetCapture()==NULL);
    }
    for(unsigned i=0;i<6;++i){
        const char *category="float-drag-gl";SetEnvironmentVariableA("UI_UX_PHASE",category);ui_panel_layout_t panel={0};panel.size=sizeof(panel);
        CHECK(chrome_presentation("panel-form",&p)==UI_STATUS_OK);real_click(chrome_point(&p));
        double end=clock_ms()+3000;do{drain(1);ui_shell_get_panel_layout(ui_host_get_shell(instance->host),"form",&panel);}while(clock_ms()<end&&!panel.floating);CHECK(panel.floating);
        ui_content_slot_t *slot=ui_shell_get_content_slot(ui_host_get_shell(instance->host),"form");HWND content=(HWND)ui_content_slot_native_handle(slot),floating=GetAncestor(content,GA_ROOT);RECT float_rect,root_rect;GetWindowRect(floating,&float_rect);GetWindowRect(ux_host->hwnd,&root_rect);
        CHECK(floating!=ux_host->hwnd);POINT begin={float_rect.left+80,float_rect.top+14},finish={root_rect.left+600,root_rect.top+260},cross={root_rect.left+120,root_rect.top+240};RECT rect={root_rect.left+70,root_rect.top+130,root_rect.left+310,root_rect.top+250};
        observer_start(rect);CHECK(mouse(begin,MOUSEEVENTF_LEFTDOWN));drain(1);double started=clock_ms();
        for(int step=1;step<=20;++step){POINT from=step<=10?begin:cross,to=step<=10?cross:finish;int progress=step<=10?step:step-10;
            POINT point={from.x+(to.x-from.x)*progress/10,from.y+(to.y-from.y)*progress/10};CHECK(mouse(point,0));while(clock_ms()<started+step*1000.0/60.0)drain(1);}
        CHECK(mouse(finish,MOUSEEVENTF_LEFTUP));drain(10);ui_shell_get_panel_layout(ui_host_get_shell(instance->host),"form",&panel);
        observer_finish(category,i,clock_ms(),panel.floating&&content==(HWND)ui_content_slot_native_handle(slot));CHECK(panel.floating&&content==(HWND)ui_content_slot_native_handle(slot));
        category="dock";SetEnvironmentVariableA("UI_UX_PHASE",category);GetWindowRect(floating,&float_rect);begin=(POINT){float_rect.left+80,float_rect.top+14};finish=(POINT){root_rect.right-20,root_rect.top+250};
        observer_start(rect);CHECK(mouse(begin,MOUSEEVENTF_LEFTDOWN));drain(1);started=clock_ms();
        for(int step=1;step<=20;++step){POINT point={begin.x+(finish.x-begin.x)*step/20,begin.y+(finish.y-begin.y)*step/20};CHECK(mouse(point,0));while(clock_ms()<started+step*1000.0/60.0)drain(1);}
        CHECK(mouse(finish,MOUSEEVENTF_LEFTUP));end=clock_ms()+3000;do{drain(1);ui_shell_get_panel_layout(ui_host_get_shell(instance->host),"form",&panel);}while(clock_ms()<end&&panel.floating);
        observer_finish(category,i,clock_ms(),!panel.floating);CHECK(!panel.floating);CHECK(content==(HWND)ui_content_slot_native_handle(slot));
    }
    for(unsigned i=0;i<20;++i){
        const char *category="window-resize";SetEnvironmentVariableA("UI_UX_PHASE",category);RECT window,after;GetWindowRect(ux_host->hwnd,&window);
        POINT begin={window.right-2,window.top+(window.bottom-window.top)/2},finish=begin;finish.x+=i%2?-20:20;RECT rect={window.right-300,window.top+60,window.right,window.top+180};
        observer_start(rect);INPUT input[4]={0};POINT points[4]={begin,begin,finish,finish};
        for(int j=0;j<4;++j){input[j].type=INPUT_MOUSE;input[j].mi.dx=MulDiv(points[j].x,65535,GetSystemMetrics(SM_CXSCREEN)-1);input[j].mi.dy=MulDiv(points[j].y,65535,GetSystemMetrics(SM_CYSCREEN)-1);
            input[j].mi.dwFlags=MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MOVE|(j==1?MOUSEEVENTF_LEFTDOWN:j==3?MOUSEEVENTF_LEFTUP:0);}
        CHECK(SendInput(4,input,sizeof(INPUT))==4);double end=clock_ms()+3000;do{drain(1);GetWindowRect(ux_host->hwnd,&after);}while(clock_ms()<end&&after.right==window.right);
        observer_finish(category,i,clock_ms(),after.right-window.right==(i%2?-20:20));CHECK(after.right-window.right==(i%2?-20:20));
    }
    open_path(ux_host,package);drain(600);uint64_t first=instance->instance_id,second=ui_workspace_active(ux_host->workspace);
    for(unsigned i=0;i<30;++i){
        const char *category="instance-switch";SetEnvironmentVariableA("UI_UX_PHASE",category);char id[80];uint64_t expected=i%2?second:first;snprintf(id,80,"tab-%llu",(unsigned long long)expected);
        CHECK(chrome_presentation(id,&p)==UI_STATUS_OK);RECT rect=element_screen(ux_host->view,&p);observer_start(rect);real_click(center(rect));
        double end=clock_ms()+3000;do{drain(1);}while(clock_ms()<end&&ui_workspace_active(ux_host->workspace)!=expected);
        observer_finish(category,i,clock_ms(),ui_workspace_active(ux_host->workspace)==expected);CHECK(ui_workspace_active(ux_host->workspace)==expected);
    }
    CHECK(ui_workspace_activate(ux_host->workspace,first)==UI_STATUS_OK);drain(120);
    SetEnvironmentVariableA("UI_UX_PHASE","idle");FILETIME created,exited,kernel0,user0,kernel1,user1;GetProcessTimes(GetCurrentProcess(),&created,&exited,&kernel0,&user0);double start=clock_ms();drain(2000);GetProcessTimes(GetCurrentProcess(),&created,&exited,&kernel1,&user1);
    ULARGE_INTEGER k0,k1,u0,u1;k0.LowPart=kernel0.dwLowDateTime;k0.HighPart=kernel0.dwHighDateTime;k1.LowPart=kernel1.dwLowDateTime;k1.HighPart=kernel1.dwHighDateTime;u0.LowPart=user0.dwLowDateTime;u0.HighPart=user0.dwHighDateTime;u1.LowPart=user1.dwLowDateTime;u1.HighPart=user1.dwHighDateTime;
    printf("UX_IDLE elapsedMs%.3f hostCpuMs%.3f observerThreadDormant=1\n",clock_ms()-start,(double)(k1.QuadPart-k0.QuadPart+u1.QuadPart-u0.QuadPart)/10000.0);
    recorder.quit=1;SetEvent(recorder.ready);SetEvent(recorder.stop);CHECK(WaitForSingleObject(recorder.thread,3000)==WAIT_OBJECT_0);CloseHandle(recorder.thread);CloseHandle(recorder.ready);CloseHandle(recorder.stop);CloseHandle(recorder.timer);fclose(measurements);fclose(sample_file);
    CHECK(ui_component_get_state(table,&state)==UI_STATUS_OK&&state.total_count==100000);
}
