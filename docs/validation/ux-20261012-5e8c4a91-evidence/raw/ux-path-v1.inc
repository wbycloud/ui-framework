static void path_states(ui_app_instance_info_t *instance,int narrow)
{
    ui_component_t *form=ui_component_find(instance->host,"form"),*dialog=ui_component_find(instance->host,"dialog"),*table=ui_component_find(instance->host,"table");
    ui_element_presentation_t p={0};ui_cell_t value={0};ui_component_state_t state={0};
    if(assistant_visible(ux_host)){CHECK(chrome_presentation("assistant-toggle",&p)==UI_STATUS_OK);real_click(chrome_point(&p));drain(100);}
    frame_component=form;frame_view=component_view(instance->host,"field-name");CHECK(frame_view!=NULL);
    CHECK(present(form,"field-name",&p)==UI_STATUS_OK&&p.visible);real_click(center(element_screen(frame_view,&p)));drain(80);
    control_key('A');drain(30);INPUT keys[260]={0};unsigned count=0;const char *text="ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-END-UX";
    for(const char *at=text;*at;++at){keys[count].type=keys[count+1].type=INPUT_KEYBOARD;keys[count].ki.wScan=keys[count+1].ki.wScan=(WORD)*at;keys[count].ki.dwFlags=KEYEVENTF_UNICODE;keys[count+1].ki.dwFlags=KEYEVENTF_UNICODE|KEYEVENTF_KEYUP;count+=2;}
    CHECK(SendInput(count,keys,sizeof(INPUT))==count);double value_end=clock_ms()+5000;value.size=sizeof(value);
    do{drain(1);ui_component_get_field(form,"name",&value);}while(clock_ms()<value_end&&strcmp(value.text?value.text:"",text));CHECK(value.text&&!strcmp(value.text,text));
    real_key(VK_HOME,0);drain(100);frame("field-name","home");real_key(VK_END,0);drain(100);frame("field-name","end");
    CHECK(present(form,"submit",&p)==UI_STATUS_OK&&p.visible);RECT rect=element_screen(frame_view,&p);POINT button_point=center(rect);
    CHECK(mouse((POINT){60,1100},0));drain(80);frame("submit","before");CHECK(mouse(button_point,0));drain(80);frame("submit","hover");
    CHECK(mouse(button_point,MOUSEEVENTF_LEFTDOWN));drain(80);frame("submit","down");CHECK(mouse(button_point,MOUSEEVENTF_LEFTUP));drain(150);frame("submit","released");
    CHECK(present(form,"submit",&p)==UI_STATUS_OK&&p.focused);real_key(VK_TAB,0);drain(80);real_key(VK_TAB,1);drain(80);frame("submit","keyboard");
    frame_component=NULL;frame_view=NULL;
    char menu[80];uint64_t hash=1469598103934665603ULL;for(const char *at="State";*at;++at){hash^=(unsigned char)*at;hash*=1099511628211ULL;}snprintf(menu,80,"app-menu-%llu",(unsigned long long)hash);
    CHECK(chrome_presentation(menu,&p)==UI_STATUS_OK);real_click(chrome_point(&p));drain(100);ui_element_presentation_t item={0};item.size=sizeof(item);
    CHECK(ui_host_menu_get_item_presentation(instance->host,"state.properties",&item)==UI_STATUS_OK&&item.visible);HWND popup=active_operation_popup();POINT origin={0,0};ClientToScreen(popup,&origin);
    real_click((POINT){origin.x+MulDiv(item.clip.x+item.clip.width/2,(int)ux_host->dpi,96),origin.y+MulDiv(item.clip.y+item.clip.height/2,(int)ux_host->dpi,96)});
    double ready_end=clock_ms()+5000;ui_status_t ready_status;
    do{drain(1);ready_status=present(dialog,"field-name",&p);}while(clock_ms()<ready_end&&(ready_status!=UI_STATUS_OK||!p.visible));
    CHECK(ready_status==UI_STATUS_OK&&p.visible);frame_component=dialog;frame_view=NULL;
    /* Locate the dialog's view independently of the docked form's same ID. */
    for(ui_web_view_t *v=instance->host->web_views;v;v=v->host_next){HWND root=GetAncestor((HWND)ui_web_view_native_handle(v),GA_ROOT);wchar_t klass[80];GetClassNameW(root,klass,80);if(!wcscmp(klass,L"UIFrameworkWebDialog3")){frame_view=v;break;}}
    CHECK(frame_view!=NULL);frame("field-name","dialog-ready");real_click(center(element_screen(frame_view,&p)));drain(100);frame("field-name","dialog-focused");
    real_key(VK_TAB,0);drain(100);frame("submit","dialog-tab");real_key(VK_TAB,0);drain(100);frame("cancel","dialog-tab");
    real_key(VK_ESCAPE,0);double end=clock_ms()+3000;state.size=sizeof(state);do{drain(1);ui_component_get_state(dialog,&state);}while(clock_ms()<end&&state.modal);CHECK(!state.modal);
    frame_component=NULL;frame_view=NULL;frame("assistant-toggle","dialog-closed");
    value.size=sizeof(value);CHECK(ui_component_get_field(form,"name",&value)==UI_STATUS_OK&&!strcmp(value.text,text));state.size=sizeof(state);CHECK(ui_component_get_state(table,&state)==UI_STATUS_OK&&state.total_count==100000&&state.cached_bytes<=2*1024*1024&&state.rendered_nodes<=1024);
    printf("PATH narrow%d draft-preserved=1 total%llu nodes%zu bytes%zu\n",narrow,(unsigned long long)state.total_count,state.rendered_nodes,state.cached_bytes);
    if(!narrow){
        ui_panel_layout_t panel={0};panel.size=sizeof(panel);CHECK(chrome_presentation("panel-form",&p)==UI_STATUS_OK);real_click(chrome_point(&p));drain(120);
        CHECK(ui_shell_get_panel_layout(ui_host_get_shell(instance->host),"form",&panel)==UI_STATUS_OK&&panel.floating);
        ui_content_slot_t *slot=ui_shell_get_content_slot(ui_host_get_shell(instance->host),"form");HWND content=(HWND)ui_content_slot_native_handle(slot),floating=GetAncestor(content,GA_ROOT);RECT f,r;GetWindowRect(floating,&f);GetWindowRect(ux_host->hwnd,&r);
        POINT begin={f.left+MulDiv(70,(int)ux_host->dpi,96),f.top+MulDiv(14,(int)ux_host->dpi,96)},finish={r.right-20,r.top+250};
        CHECK(mouse(begin,MOUSEEVENTF_LEFTDOWN));drain(60);CHECK(GetCapture()!=NULL);frame("assistant-toggle","float-down");
        for(int step=1;step<=12;++step){POINT point={begin.x+(r.left+120-begin.x)*step/12,begin.y+(r.top+240-begin.y)*step/12};CHECK(mouse(point,0));drain(20);frame("assistant-toggle","float-through-gl");}
        real_key(VK_ESCAPE,0);drain(80);CHECK(mouse((POINT){r.left+120,r.top+240},MOUSEEVENTF_LEFTUP));drain(80);frame("assistant-toggle","float-esc");GetWindowRect(floating,&r);CHECK(!memcmp(&r,&f,sizeof(f)));CHECK(GetCapture()==NULL);
        CHECK(mouse(begin,MOUSEEVENTF_LEFTDOWN));drain(40);CHECK(mouse((POINT){begin.x+30,begin.y+30},0));drain(40);ReleaseCapture();drain(60);CHECK(mouse(begin,MOUSEEVENTF_LEFTUP));frame("assistant-toggle","float-controlled-capture-loss");GetWindowRect(floating,&r);CHECK(!memcmp(&r,&f,sizeof(f)));
        CHECK(mouse(begin,MOUSEEVENTF_LEFTDOWN));drain(40);for(int step=1;step<=12;++step){POINT point={begin.x+(finish.x-begin.x)*step/12,begin.y+(finish.y-begin.y)*step/12};CHECK(mouse(point,0));drain(20);frame("assistant-toggle","dock-held");}
        CHECK(mouse(finish,MOUSEEVENTF_LEFTUP));drain(150);frame("assistant-toggle","dock-release");CHECK(ui_shell_get_panel_layout(ui_host_get_shell(instance->host),"form",&panel)==UI_STATUS_OK&&!panel.floating);CHECK(content==(HWND)ui_content_slot_native_handle(slot));CHECK(GetCapture()==NULL);
        value.size=sizeof(value);CHECK(ui_component_get_field(form,"name",&value)==UI_STATUS_OK&&!strcmp(value.text,text));
    }
}
