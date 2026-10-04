/* Versioned layout persistence and logical gestures. Included after the shell
 * implementation so both native and Web chrome retain their existing slots. */
#define LAYOUT_HEADER 80u
#define LAYOUT_RECORD 104u
static uint32_t layout_read(const unsigned char *p)
{return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static void layout_write(unsigned char *p,uint32_t n)
{p[0]=(unsigned char)n;p[1]=(unsigned char)(n>>8);p[2]=(unsigned char)(n>>16);p[3]=(unsigned char)(n>>24);}
static int layout_allowed(ui_native_shell_t *s)
{return s&&!s->host->dispatch_blocked&&!s->host->modal_component;}
static ui_status_t layout_changed(ui_native_shell_t *s)
{
    HWND focus=s->offscreen?NULL:GetFocus();ui_status_t status=ui_native_shell_reflow(s);
    if(focus&&s->active&&!s->host->modal_component&&IsWindowVisible(focus)&&IsWindowEnabled(focus))SetFocus(focus);
    if(status==UI_STATUS_OK)(void)ui_host_emit_event(s->host,"ui.shell.changed","{}");return status;
}
static ui_rect_t panel_dock_rect(ui_native_shell_t *s,ui_native_panel_t *panel,ui_rect_t side)
{
    size_t i,count=0;int before=0,total=0,min_height=28,room;ui_rect_t r=side;
    for(i=0;i<s->panel_count;++i){ui_native_panel_t *p=&s->panels[i];int weight;
        if(p->kind!=UI_PANEL_SIDEBAR||p->manual_floating||p->dock_region!=panel->dock_region)continue;
        weight=p->collapsed?0:(p->height>28?p->height-28:100);total+=weight;++count;
        if(p->order<panel->order||(p->order==panel->order&&i<(size_t)(panel-s->panels)))before+=weight;}
    if(!count)return r;if(side.height<(int)count*28)min_height=side.height/(int)count;
    room=side.height-(int)count*min_height;{
        int ordinal=0,weight=panel->collapsed?0:(panel->height>28?panel->height-28:100);
        for(i=0;i<s->panel_count;++i){ui_native_panel_t *p=&s->panels[i];
            if(p->kind==UI_PANEL_SIDEBAR&&!p->manual_floating&&p->dock_region==panel->dock_region&&
              (p->order<panel->order||(p->order==panel->order&&i<(size_t)(panel-s->panels))))++ordinal;}
        r.y+=ordinal*min_height+(total?(int)((int64_t)room*before/total):0);
        r.height=min_height+(total?(int)((int64_t)room*(before+weight)/total)-(int)((int64_t)room*before/total):room/(int)count);
    }return r;
}
static void position_panel(ui_native_shell_t *s,ui_native_panel_t *p,HWND window)
{
    RECT wr;MONITORINFO monitor={0};POINT origin={0,0};uint32_t dpi=p->saved_dpi?p->saved_dpi:s->host->dpi;
    int width=logical_to_pixels(p->saved_float.width?p->saved_float.width:(p->preferred_width>0?p->preferred_width:320),dpi);
    int height=logical_to_pixels(p->saved_float.height?p->saved_float.height:240,dpi);
    if(p->saved_float.width){origin.x=logical_to_pixels(p->saved_float.x,dpi);origin.y=logical_to_pixels(p->saved_float.y,dpi);}
    else{ClientToScreen(s->parent,&origin);origin.x+=40;origin.y+=48;}
    wr=(RECT){origin.x,origin.y,origin.x+width,origin.y+height};monitor.cbSize=sizeof(monitor);
    if(GetMonitorInfoW(MonitorFromRect(&wr,MONITOR_DEFAULTTONEAREST),&monitor)){
        RECT work=monitor.rcWork;if(width>work.right-work.left)width=work.right-work.left;if(height>work.bottom-work.top)height=work.bottom-work.top;
        if(origin.x>work.right-width)origin.x=work.right-width;if(origin.x<work.left)origin.x=work.left;
        if(origin.y>work.bottom-height)origin.y=work.bottom-height;if(origin.y<work.top)origin.y=work.top;
    }
    (void)SetWindowPos(window,NULL,origin.x,origin.y,width,height,SWP_NOACTIVATE|SWP_NOZORDER);p->positioned=1;
}
ui_status_t ui_shell_get_panel_layout(ui_shell_t *shell,const char *id,ui_panel_layout_t *out)
{
    ui_native_panel_t *p;RECT r;uint32_t dpi;
    if(!shell||!shell->native||!id||!out||out->size<sizeof(*out))return UI_STATUS_INVALID_ARGUMENT;
    p=find_panel(shell->native,id);if(!p)return UI_STATUS_NOT_FOUND;
    out->dock_region=p->dock_region;out->floating=p->kind==UI_PANEL_FLOATING||p->manual_floating;
    out->collapsed=p->collapsed;out->closed=p->closed;out->order=p->order;out->height=p->height;
    out->floating_rect=p->saved_float;out->dpi=p->saved_dpi?p->saved_dpi:shell->native->host->dpi;
    if(!shell->native->offscreen&&p->floating&&GetWindowRect(p->popup?p->popup:p->hwnd,&r)){
        dpi=p->floating_dpi?p->floating_dpi:out->dpi;out->dpi=dpi;
        out->floating_rect=(ui_rect_t){MulDiv(r.left,96,(int)dpi),MulDiv(r.top,96,(int)dpi),MulDiv(r.right-r.left,96,(int)dpi),MulDiv(r.bottom-r.top,96,(int)dpi)};
    }return UI_STATUS_OK;
}
static int valid_panel_layout(const ui_panel_layout_t *p)
{
    return p&&p->size>=sizeof(*p)&&(p->dock_region==UI_LAYOUT_REGION_LEFT_SIDEBAR||p->dock_region==UI_LAYOUT_REGION_RIGHT_SIDEBAR)&&
      (p->floating==0||p->floating==1)&&(p->collapsed==0||p->collapsed==1)&&(p->closed==0||p->closed==1)&&
      p->order>=0&&p->order<=511&&p->height>=0&&p->height<=32767&&p->dpi>0&&p->dpi<=768&&
      p->floating_rect.x>=-1000000&&p->floating_rect.x<=1000000&&p->floating_rect.y>=-1000000&&p->floating_rect.y<=1000000&&
      p->floating_rect.width>=0&&p->floating_rect.width<=32767&&p->floating_rect.height>=0&&p->floating_rect.height<=32767;
}
static void assign_panel(ui_native_panel_t *p,const ui_panel_layout_t *v)
{p->kind=UI_PANEL_SIDEBAR;p->dock_region=v->dock_region;p->manual_floating=v->floating;p->collapsed=v->collapsed;p->closed=v->closed;p->order=v->order;p->height=v->height;p->saved_float=v->floating_rect;p->saved_dpi=v->dpi;p->positioned=0;}
ui_status_t ui_shell_set_panel_layout(ui_shell_t *shell,const char *id,const ui_panel_layout_t *v)
{
    ui_native_shell_t *s;ui_native_panel_t *p;if(!shell||!shell->native||!id||!valid_panel_layout(v))return UI_STATUS_INVALID_ARGUMENT;
    s=shell->native;if(!layout_allowed(s))return UI_STATUS_CANCELLED;p=find_panel(s,id);if(!p)return UI_STATUS_NOT_FOUND;
    s->layout_enabled=1;assign_panel(p,v);return layout_changed(s);
}
ui_status_t ui_shell_save_layout(ui_shell_t *shell,void *buffer,size_t capacity,size_t *required)
{
    ui_native_shell_t *s;unsigned char *data=(unsigned char *)buffer;size_t i,j,n;int fields[16];
    if(!shell||!shell->native||!required)return UI_STATUS_INVALID_ARGUMENT;s=shell->native;n=LAYOUT_HEADER+s->panel_count*LAYOUT_RECORD;
    if(n>65536||s->panel_count>512)return UI_STATUS_LIMIT_EXCEEDED;*required=n;for(i=0;i<s->panel_count;++i){if(strlen(s->panels[i].id)>63)return UI_STATUS_LIMIT_EXCEEDED;if(!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,s->panels[i].id,-1,NULL,0))return UI_STATUS_INVALID_ARGUMENT;}
    if(!buffer)return UI_STATUS_OK;if(capacity<n)return UI_STATUS_LIMIT_EXCEEDED;memset(data,0,n);memcpy(data,"ULYT",4);
    layout_write(data+4,1);layout_write(data+8,(uint32_t)s->panel_count);layout_write(data+12,(uint32_t)n);
    _Static_assert(offsetof(ui_layout_desc_t,right_sidebar_no_collapse)==64,"layout v1 field span");
    memcpy(fields,&s->host->layout.menu_bar_height,sizeof(fields));for(j=0;j<16;++j)layout_write(data+16+j*4,(uint32_t)fields[j]);
    for(i=0;i<s->panel_count;++i){ui_panel_layout_t v={0};unsigned char *r=data+LAYOUT_HEADER+i*LAYOUT_RECORD;v.size=sizeof(v);
        (void)ui_shell_get_panel_layout(shell,s->panels[i].id,&v);memcpy(r,s->panels[i].id,strlen(s->panels[i].id));
        layout_write(r+64,(uint32_t)v.dock_region);layout_write(r+68,(uint32_t)(v.floating|v.collapsed<<1|v.closed<<2));
        layout_write(r+72,(uint32_t)v.order);layout_write(r+76,(uint32_t)v.height);layout_write(r+80,(uint32_t)v.floating_rect.x);layout_write(r+84,(uint32_t)v.floating_rect.y);
        layout_write(r+88,(uint32_t)v.floating_rect.width);layout_write(r+92,(uint32_t)v.floating_rect.height);layout_write(r+96,v.dpi);
    }return UI_STATUS_OK;
}
static ui_panel_layout_t read_panel_layout(const unsigned char *r)
{
    ui_panel_layout_t v={0};uint32_t flags=layout_read(r+68);v.size=sizeof(v);v.dock_region=(ui_layout_region_t)layout_read(r+64);
    v.floating=(int)(flags&1);v.collapsed=(int)((flags>>1)&1);v.closed=(int)((flags>>2)&1);v.order=(int)layout_read(r+72);v.height=(int)layout_read(r+76);
    v.floating_rect=(ui_rect_t){(int)layout_read(r+80),(int)layout_read(r+84),(int)layout_read(r+88),(int)layout_read(r+92)};v.dpi=layout_read(r+96);return v;
}
ui_status_t ui_shell_restore_layout(ui_shell_t *shell,const void *buffer,size_t bytes)
{
    const unsigned char *data=(const unsigned char *)buffer;ui_native_shell_t *s;uint32_t count;size_t i,j;ui_layout_desc_t layout={0};int fields[16];
    ui_host_config_t config={0};ui_host_t *validator;ui_status_t status;
    if(!shell||!shell->native||!data||bytes<LAYOUT_HEADER||bytes>65536||memcmp(data,"ULYT",4))return UI_STATUS_INVALID_ARGUMENT;
    if(layout_read(data+4)!=1)return UI_STATUS_UNSUPPORTED;s=shell->native;if(!layout_allowed(s))return UI_STATUS_CANCELLED;
    count=layout_read(data+8);if(count>512||bytes!=LAYOUT_HEADER+(size_t)count*LAYOUT_RECORD||layout_read(data+12)!=bytes)return UI_STATUS_INVALID_ARGUMENT;
    for(i=0;i<count;++i){const unsigned char *r=data+LAYOUT_HEADER+i*LAYOUT_RECORD;ui_panel_layout_t v=read_panel_layout(r);
        if(!r[0]||!memchr(r,0,64)||!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,(const char *)r,-1,NULL,0)||layout_read(r+68)>7||layout_read(r+100)||!valid_panel_layout(&v))return UI_STATUS_INVALID_ARGUMENT;
        for(j=0;j<i;++j)if(!strcmp((const char *)r,(const char *)(data+LAYOUT_HEADER+j*LAYOUT_RECORD)))return UI_STATUS_INVALID_ARGUMENT;
    }
    layout.size=sizeof(layout);for(j=0;j<16;++j){fields[j]=(int)layout_read(data+16+j*4);if(fields[j]<0||fields[j]>32767)return UI_STATUS_INVALID_ARGUMENT;}memcpy(&layout.menu_bar_height,fields,sizeof(fields));
    config.size=sizeof(config);config.api_version=UI_FRAMEWORK_API_VERSION;validator=ui_host_create(&config);if(!validator)return UI_STATUS_OUT_OF_MEMORY;
    status=ui_host_set_layout(validator,&layout);ui_host_destroy(validator);if(status!=UI_STATUS_OK)return status;
    s->layout_enabled=1;for(i=0;i<count;++i){const unsigned char *r=data+LAYOUT_HEADER+i*LAYOUT_RECORD;ui_native_panel_t *p=find_panel(s,(const char *)r);ui_panel_layout_t v=read_panel_layout(r);if(p)assign_panel(p,&v);}
    status=ui_host_set_layout(s->host,&layout);return status==UI_STATUS_OK?layout_changed(s):status;
}
ui_status_t ui_shell_reset_layout(ui_shell_t *shell)
{
    ui_native_shell_t *s;size_t i;if(!shell||!shell->native)return UI_STATUS_INVALID_ARGUMENT;s=shell->native;if(!layout_allowed(s))return UI_STATUS_CANCELLED;
    if(s->drag_active)(void)ui_shell_end_panel_drag(shell,0);s->layout_enabled=1;
    for(i=0;i<s->panel_count;++i){ui_native_panel_t *p=&s->panels[i];ui_panel_entry_t *e;for(e=s->host->panels;e&&strcmp(e->id,p->id);e=e->next){}
        if(e){p->kind=e->kind;p->dock_region=e->dock_region;p->manual_floating=p->collapsed=p->closed=p->positioned=0;p->order=(int)i;p->height=0;p->saved_float=(ui_rect_t){0,0,0,0};p->saved_dpi=0;}}
    {ui_status_t status=ui_host_set_layout(s->host,&s->default_layout);return status==UI_STATUS_OK?layout_changed(s):status;}
}
ui_status_t ui_shell_resize_splitter(ui_shell_t *shell,ui_layout_region_t region,const char *id,int extent)
{
    ui_native_shell_t *s;ui_layout_desc_t l;int min,max;if(!shell||!shell->native||(region!=UI_LAYOUT_REGION_LEFT_SIDEBAR&&region!=UI_LAYOUT_REGION_RIGHT_SIDEBAR))return UI_STATUS_INVALID_ARGUMENT;
    s=shell->native;if(!layout_allowed(s))return UI_STATUS_CANCELLED;s->layout_enabled=1;l=s->host->layout;
    if(id){ui_native_panel_t *p=find_panel(s,id),*next=NULL;ui_rect_t r;size_t i;if(!p||p->dock_region!=region||p->floating)return UI_STATUS_NOT_FOUND;
        for(i=0;i<s->panel_count;++i){ui_native_panel_t *n=&s->panels[i];if(n->dock_region==region&&!n->floating&&n!=p&&n->order>p->order&&(!next||n->order<next->order))next=n;}
        if(!next)return UI_STATUS_NOT_FOUND;r=p->slot->frame_rect;min=72;max=r.height+next->slot->frame_rect.height-72;if(max<min)return UI_STATUS_CANCELLED;
        if(extent<min)extent=min;if(extent>max)extent=max;
        for(i=0;i<s->panel_count;++i){ui_native_panel_t *n=&s->panels[i];if(n->dock_region==region&&!n->floating&&!n->collapsed)n->height=n->slot->frame_rect.height;}
        p->height=extent;next->height=r.height+next->slot->frame_rect.height-extent;
    }else{int available=s->host->host_rect.width-(l.main_min_width?l.main_min_width:400)-28;
        min=region==UI_LAYOUT_REGION_LEFT_SIDEBAR?l.left_sidebar_min_width:l.right_sidebar_min_width;if(!min)min=80;
        max=region==UI_LAYOUT_REGION_LEFT_SIDEBAR?l.left_sidebar_max_width:l.right_sidebar_max_width;if(!max)max=32767;if(max>available)max=available;if(max<min)return UI_STATUS_CANCELLED;
        if(extent<min)extent=min;if(extent>max)extent=max;
        if(region==UI_LAYOUT_REGION_LEFT_SIDEBAR)l.left_sidebar_width=l.left_sidebar_preferred_width=extent;else l.right_sidebar_width=l.right_sidebar_preferred_width=extent;
        {ui_status_t status=ui_host_set_layout(s->host,&l);if(status!=UI_STATUS_OK)return status;}}
    return layout_changed(s);
}
static void clamp_floating_windows(ui_native_shell_t *s)
{
    size_t i;if(s->offscreen||!s->layout_enabled)return;
    for(i=0;i<s->panel_count;++i){ui_native_panel_t *p=&s->panels[i];HWND window=p->popup?p->popup:p->hwnd;RECT r;MONITORINFO m={0};int width,height,x,y;
        if(!p->floating||!window||!GetWindowRect(window,&r))continue;m.cbSize=sizeof(m);if(!GetMonitorInfoW(MonitorFromRect(&r,MONITOR_DEFAULTTONEAREST),&m))continue;
        width=r.right-r.left;height=r.bottom-r.top;if(width>m.rcWork.right-m.rcWork.left)width=m.rcWork.right-m.rcWork.left;if(height>m.rcWork.bottom-m.rcWork.top)height=m.rcWork.bottom-m.rcWork.top;
        x=r.left;y=r.top;if(x>m.rcWork.right-width)x=m.rcWork.right-width;if(x<m.rcWork.left)x=m.rcWork.left;if(y>m.rcWork.bottom-height)y=m.rcWork.bottom-height;if(y<m.rcWork.top)y=m.rcWork.top;
        if(x!=r.left||y!=r.top||width!=r.right-r.left||height!=r.bottom-r.top)(void)SetWindowPos(window,NULL,x,y,width,height,SWP_NOACTIVATE|SWP_NOZORDER);
    }
}
ui_status_t ui_shell_begin_panel_drag(ui_shell_t *shell,const char *id)
{
    ui_native_shell_t *s;if(!shell||!shell->native||!id||strlen(id)>63)return UI_STATUS_INVALID_ARGUMENT;s=shell->native;
    if(!layout_allowed(s)||!s->active)return UI_STATUS_CANCELLED;if(s->drag_active||s->split_active)return UI_STATUS_ALREADY_EXISTS;if(!find_panel(s,id))return UI_STATUS_NOT_FOUND;if(s->panel_count>512)return UI_STATUS_LIMIT_EXCEEDED;
    s->drag_active=1;s->drag_region=UI_LAYOUT_REGION_NONE;strcpy(s->drag_id,id);if(!s->offscreen){s->gesture_focus=GetFocus();SetFocus(s->parent);SetCapture(s->parent);}return UI_STATUS_OK;
}
ui_status_t ui_shell_update_panel_drag(ui_shell_t *shell,int x,int y,ui_rect_t *preview,ui_layout_region_t *region)
{
    ui_native_shell_t *s;ui_rect_t r;int width;if(!shell||!shell->native||!preview||!region)return UI_STATUS_INVALID_ARGUMENT;s=shell->native;
    if(!s->drag_active)return UI_STATUS_NOT_FOUND;if(!layout_allowed(s)||!s->active){(void)ui_shell_end_panel_drag(shell,0);return UI_STATUS_CANCELLED;}
    s->drag_x=x;s->drag_y=y;s->drag_region=UI_LAYOUT_REGION_NONE;r=(ui_rect_t){x-20,y-14,320,240};width=s->host->host_rect.width;
    if(y>=0&&y<s->host->host_rect.height&&x>=0&&x<width){
        if(x<width/4){s->drag_region=UI_LAYOUT_REGION_LEFT_SIDEBAR;r=(ui_rect_t){0,0,width/4,s->host->host_rect.height};}
        else if(x>=width*3/4){s->drag_region=UI_LAYOUT_REGION_RIGHT_SIDEBAR;r=(ui_rect_t){width*3/4,0,width-width*3/4,s->host->host_rect.height};}}
    *preview=r;*region=s->drag_region;
    if(!s->offscreen){RECT px=logical_rect_to_pixels(&r,s->host->dpi);if(!s->drag_preview)s->drag_preview=CreateWindowExW(WS_EX_TRANSPARENT|WS_EX_NOACTIVATE,L"STATIC",L"",WS_CHILD|SS_GRAYRECT,s->drag_x,s->drag_y,1,1,s->parent,NULL,NULL,NULL);
        if(s->drag_preview)SetWindowPos(s->drag_preview,HWND_TOP,px.left,px.top,px.right-px.left,px.bottom-px.top,SWP_SHOWWINDOW|SWP_NOACTIVATE);}
    return UI_STATUS_OK;
}
ui_status_t ui_shell_end_panel_drag(ui_shell_t *shell,int commit)
{
    ui_native_shell_t *s;ui_panel_layout_t v={0};char id[64];ui_status_t status=UI_STATUS_OK;
    if(!shell||!shell->native)return UI_STATUS_INVALID_ARGUMENT;s=shell->native;if(!s->drag_active)return UI_STATUS_NOT_FOUND;
    strcpy(id,s->drag_id);s->drag_active=0;if(s->drag_preview){DestroyWindow(s->drag_preview);s->drag_preview=NULL;}if(!s->offscreen&&GetCapture()==s->parent)ReleaseCapture();
    if(commit){if(!layout_allowed(s)||!s->active)return UI_STATUS_CANCELLED;v.size=sizeof(v);status=ui_shell_get_panel_layout(shell,id,&v);if(status!=UI_STATUS_OK)return status;
        v.floating=s->drag_region==UI_LAYOUT_REGION_NONE;v.closed=v.collapsed=0;
        if(!v.floating){size_t i,j,count=0,indices[512],insert=0;v.dock_region=s->drag_region;
            for(i=0;i<s->panel_count;++i){ui_native_panel_t *p=&s->panels[i];if(!strcmp(p->id,id)||p->dock_region!=v.dock_region)continue;
                j=count;while(j&&s->panels[indices[j-1]].order>p->order){indices[j]=indices[j-1];--j;}indices[j]=i;++count;}
            for(i=0;i<count;++i){ui_native_panel_t *p=&s->panels[indices[i]];if(!p->floating&&!p->closed&&s->drag_y>=p->slot->frame_rect.y+p->slot->frame_rect.height/2)insert=i+1;}
            for(i=0;i<count;++i)s->panels[indices[i]].order=(int)(i+(i>=insert));v.order=(int)insert;
        }
        else{POINT p={logical_to_pixels(s->drag_x,s->host->dpi),logical_to_pixels(s->drag_y,s->host->dpi)};if(!s->offscreen)ClientToScreen(s->parent,&p);v.dpi=s->host->dpi;v.floating_rect=(ui_rect_t){MulDiv(p.x,96,(int)v.dpi),MulDiv(p.y,96,(int)v.dpi),320,240};}
        status=ui_shell_set_panel_layout(shell,id,&v);
    }if(s->gesture_focus&&s->active&&layout_allowed(s)&&IsWindowVisible(s->gesture_focus)&&IsWindowEnabled(s->gesture_focus))SetFocus(s->gesture_focus);s->gesture_focus=NULL;return status;
}

static void end_splitter(ui_native_shell_t *s,int commit)
{
    void *data=s->split_snapshot;size_t bytes=s->split_bytes;s->split_snapshot=NULL;s->split_active=0;
    if(GetCapture()==s->parent)ReleaseCapture();if(!commit&&layout_allowed(s))(void)ui_shell_restore_layout(&s->shell,data,bytes);free(data);
    if(s->gesture_focus&&s->active&&layout_allowed(s)&&IsWindowVisible(s->gesture_focus)&&IsWindowEnabled(s->gesture_focus))SetFocus(s->gesture_focus);s->gesture_focus=NULL;
}
ui_status_t ui_shell_begin_splitter_drag(ui_shell_t *shell,ui_layout_region_t region,const char *id)
{
    ui_native_shell_t *s;ui_status_t status;size_t n=0;
    if(!shell||!shell->native||(region!=UI_LAYOUT_REGION_LEFT_SIDEBAR&&region!=UI_LAYOUT_REGION_RIGHT_SIDEBAR)||(id&&strlen(id)>63))return UI_STATUS_INVALID_ARGUMENT;
    s=shell->native;if(s->offscreen)return UI_STATUS_UNSUPPORTED;if(!s->active||!layout_allowed(s))return UI_STATUS_CANCELLED;
    if(s->drag_active||s->split_active)return UI_STATUS_ALREADY_EXISTS;if(id&&!find_panel(s,id))return UI_STATUS_NOT_FOUND;
    status=ui_shell_save_layout(shell,NULL,0,&n);if(status!=UI_STATUS_OK)return status;s->split_snapshot=malloc(n);if(!s->split_snapshot)return UI_STATUS_OUT_OF_MEMORY;
    status=ui_shell_save_layout(shell,s->split_snapshot,n,&n);if(status!=UI_STATUS_OK){free(s->split_snapshot);s->split_snapshot=NULL;return status;}
    s->split_bytes=n;s->split_active=1;s->split_region=region;s->split_id[0]=0;if(id)strcpy(s->split_id,id);
    s->gesture_focus=GetFocus();SetFocus(s->parent);SetCapture(s->parent);return UI_STATUS_OK;
}
static LRESULT CALLBACK layout_subclass(HWND window,UINT message,WPARAM wp,LPARAM lp,UINT_PTR subclass,DWORD_PTR data)
{
    ui_native_shell_t *s=(ui_native_shell_t *)data;int handled=0;(void)subclass;
    if(message==WM_DISPLAYCHANGE||(message==WM_SETTINGCHANGE&&wp==SPI_SETWORKAREA))clamp_floating_windows(s);
    if(s->drag_active||s->split_active){ui_dispatch_enter(s->host);
        if(message==WM_MOUSEMOVE){int x=MulDiv((short)LOWORD(lp),96,(int)s->host->dpi),y=MulDiv((short)HIWORD(lp),96,(int)s->host->dpi);handled=1;
            if(!layout_allowed(s)||!s->active){if(s->drag_active)(void)ui_shell_end_panel_drag(&s->shell,0);if(s->split_active)end_splitter(s,0);}
            else if(s->drag_active){ui_rect_t r;ui_layout_region_t region;(void)ui_shell_update_panel_drag(&s->shell,x,y,&r,&region);}
            else if(s->split_id[0]){ui_native_panel_t *p=find_panel(s,s->split_id);if(p)(void)ui_shell_resize_splitter(&s->shell,s->split_region,s->split_id,y-p->slot->frame_rect.y);}
            else (void)ui_shell_resize_splitter(&s->shell,s->split_region,NULL,s->split_region==UI_LAYOUT_REGION_LEFT_SIDEBAR?x:s->host->host_rect.width-x);
        }else if(message==WM_LBUTTONUP){handled=1;if(s->drag_active)(void)ui_shell_end_panel_drag(&s->shell,1);if(s->split_active)end_splitter(s,1);}
        else if((message==WM_KEYDOWN&&wp==VK_ESCAPE)||message==WM_CANCELMODE||message==WM_CAPTURECHANGED){handled=1;if(s->drag_active)(void)ui_shell_end_panel_drag(&s->shell,0);if(s->split_active)end_splitter(s,0);}
        ui_dispatch_leave(s->host);
    }
    return handled?0:DefSubclassProc(window,message,wp,lp);
}
