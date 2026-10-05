/* Internal standalone-host chrome. Paths are host state, never app state. */
static int recent_equal(const char *a,const char *b)
{
    wchar_t wa[HOST_RECENT_PATH],wb[HOST_RECENT_PATH];
    return MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,a,-1,wa,HOST_RECENT_PATH)&&
        MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,b,-1,wb,HOST_RECENT_PATH)&&
        CompareStringOrdinal(wa,-1,wb,-1,TRUE)==CSTR_EQUAL;
}
static void recent_load(host_window_t *s)
{
    FILE *f=NULL;uint32_t count=0;recent_app_t *items;char magic[4];size_t i,j;int valid=1;
    if(!*s->recent_file||_wfopen_s(&f,s->recent_file,L"rb"))return;
    items=(recent_app_t *)calloc(HOST_RECENT_CAPACITY,sizeof(*items));if(!items){fclose(f);return;}
    if(fread(magic,1,4,f)!=4||memcmp(magic,"UHR1",4)||fread(&count,4,1,f)!=1||count>HOST_RECENT_CAPACITY)valid=0;
    for(i=0;valid&&i<count;++i){uint32_t n=0;wchar_t path[HOST_RECENT_PATH];
        if(fread(&n,4,1,f)!=1||!n||n>=HOST_RECENT_PATH||fread(items[i].path,1,n,f)!=n||
            memchr(items[i].path,0,n)||!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,items[i].path,-1,path,HOST_RECENT_PATH)||
            (!(path[0]&&path[1]==L':'&&path[2]==L'\\')&&!(path[0]==L'\\'&&path[1]==L'\\'))){valid=0;break;}
        for(j=0;j<i;++j)if(recent_equal(items[i].path,items[j].path))valid=0;
    }
    if(valid&&fgetc(f)!=EOF)valid=0;
    if(valid){memcpy(s->recent,items,sizeof(s->recent));s->recent_count=count;}
    else append_log(s,"最近应用记录格式无效，已忽略；应用包没有被修改。");
    free(items);fclose(f);
}
static void recent_initialize(host_window_t *s)
{
#ifndef UI_HOST_TEST
    if(!*s->recent_file){wchar_t directory[4096];
        if(SUCCEEDED(SHGetFolderPathW(NULL,CSIDL_LOCAL_APPDATA,NULL,SHGFP_TYPE_CURRENT,directory))){
            size_t n=wcslen(directory);if(n+48<4096){wcscat_s(directory,4096,L"\\UiFramework");CreateDirectoryW(directory,NULL);
                swprintf(s->recent_file,4096,L"%s\\host-recent-v1.bin",directory);}}}
#endif
    recent_load(s);
}
static void recent_save(host_window_t *s)
{
    FILE *f=NULL;wchar_t temporary[4096];uint32_t count=(uint32_t)s->recent_count;size_t i;int ok=1;
    if(!*s->recent_file)return;if(wcslen(s->recent_file)+24>=4096)return;
    swprintf(temporary,4096,L"%s.%lu.tmp",s->recent_file,GetCurrentProcessId());
    if(_wfopen_s(&f,temporary,L"wb")){append_log(s,"无法保存最近应用记录；本次打开仍然成功。");return;}
    if(fwrite("UHR1",1,4,f)!=4||fwrite(&count,4,1,f)!=1)ok=0;
    for(i=0;ok&&i<s->recent_count;++i){uint32_t n=(uint32_t)strlen(s->recent[i].path);
        if(fwrite(&n,4,1,f)!=1||fwrite(s->recent[i].path,1,n,f)!=n)ok=0;}
    if(fclose(f))ok=0;
    if(ok)ok=MoveFileExW(temporary,s->recent_file,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;
    if(!ok){DeleteFileW(temporary);append_log(s,"最近应用记录保存失败；本次打开仍然成功。");}
}
static void recent_record(host_window_t *s,const wchar_t *path)
{
    wchar_t full[32768];char *utf8;DWORD n;size_t i;
    n=GetFullPathNameW(path,32768,full,NULL);if(!n||n>=32768)return;
    utf8=to_utf8(full);if(!utf8)return;
    if(strlen(utf8)>=HOST_RECENT_PATH){append_log(s,"应用已打开，但路径超过最近记录的4095字节上限。");free(utf8);return;}
    for(i=0;i<s->recent_count;++i)if(recent_equal(utf8,s->recent[i].path))break;
    if(i==s->recent_count){if(s->recent_count<HOST_RECENT_CAPACITY)++s->recent_count;else --i;}
    if(i)memmove(&s->recent[1],&s->recent[0],i*sizeof(s->recent[0]));
    strcpy_s(s->recent[0].path,HOST_RECENT_PATH,utf8);free(utf8);recent_save(s);
}
static void tab_window(host_window_t *s,int width,size_t *start,size_t *visible,int *tab_width)
{
    size_t count=ui_workspace_count(s->workspace),active=0,i;int compact=width<400,available=width-(compact?28+28+12+32+108:34+34+24+40+138),slots;
    *tab_width=180;if(available<0)available=0;
    if(count&&count<=(size_t)(available/88)){*visible=count;*tab_width=available/(int)count;if(*tab_width>180)*tab_width=180;}
    else{available-=compact?28:34;slots=available/88;if(slots<0)slots=0;*visible=(size_t)slots;*tab_width=slots?available/slots:88;if(*tab_width>180)*tab_width=180;}
    if(*visible>count)*visible=count;
    for(i=0;i<count;++i)if(ui_workspace_instance_at(s->workspace,i)==ui_workspace_active(s->workspace)){active=i;break;}
    *start=*visible?(active/(*visible))*(*visible):0;
}
static int chrome_element(host_window_t *s,const char *id,POINT p)
{
    ui_element_presentation_t e={0};e.size=sizeof(e);
    return s->view&&ui_web_view_get_presentation(s->view,id,&e)==UI_STATUS_OK&&e.visible&&
        p.x>=e.clip.x&&p.x<e.clip.x+e.clip.width&&p.y>=e.clip.y&&p.y<e.clip.y+e.clip.height;
}
static LRESULT chrome_hit(host_window_t *s,LPARAM position)
{
    POINT p={(short)LOWORD(position),(short)HIWORD(position)},logical;RECT r;int border;size_t i,start,visible;int width;char id[80];
    GetWindowRect(s->hwnd,&r);border=GetSystemMetricsForDpi(SM_CXFRAME,s->dpi?s->dpi:96)+GetSystemMetricsForDpi(SM_CXPADDEDBORDER,s->dpi?s->dpi:96);
    if(!IsZoomed(s->hwnd)){int l=p.x<r.left+border,rr=p.x>=r.right-border,t=p.y<r.top+border,b=p.y>=r.bottom-border;
        if(t)return l?HTTOPLEFT:rr?HTTOPRIGHT:HTTOP;if(b)return l?HTBOTTOMLEFT:rr?HTBOTTOMRIGHT:HTBOTTOM;if(l)return HTLEFT;if(rr)return HTRIGHT;}
    ScreenToClient(s->hwnd,&p);logical.x=MulDiv(p.x,96,(int)(s->dpi?s->dpi:96));logical.y=MulDiv(p.y,96,(int)(s->dpi?s->dpi:96));
    if(logical.y<0||logical.y>=40)return HTCLIENT;
    if(chrome_element(s,"window-max",logical))return HTMAXBUTTON;
    if(chrome_element(s,"recent",logical)||chrome_element(s,"open",logical)||chrome_element(s,"tab-more",logical)||
        chrome_element(s,"assistant-toggle",logical)||chrome_element(s,"window-min",logical)||chrome_element(s,"window-close",logical))return HTCLIENT;
    GetClientRect(s->hwnd,&r);tab_window(s,MulDiv(r.right,96,(int)s->dpi),&start,&visible,&width);
    for(i=start;i<start+visible;++i){uint64_t instance=ui_workspace_instance_at(s->workspace,i);
        snprintf(id,sizeof(id),"tab-%llu",(unsigned long long)instance);if(chrome_element(s,id,logical))return HTCLIENT;
        snprintf(id,sizeof(id),"close-%llu",(unsigned long long)instance);if(chrome_element(s,id,logical))return HTCLIENT;}
    return HTCAPTION;
}
static LRESULT CALLBACK chrome_web_proc(HWND hwnd,UINT message,WPARAM wp,LPARAM lp)
{
    host_window_t *s=(host_window_t *)GetWindowLongPtrW(GetParent(hwnd),GWLP_USERDATA);
    if(message==WM_NCHITTEST&&s){LRESULT hit=chrome_hit(s,lp);if(hit!=HTCLIENT)return HTTRANSPARENT;}
    return s&&s->web_proc?CallWindowProcW(s->web_proc,hwnd,message,wp,lp):DefWindowProcW(hwnd,message,wp,lp);
}
static void anchor_popup(host_window_t *s,const char *id)
{
    ui_rect_t at={0};POINT origin={0,0};RECT r;MONITORINFO mi={sizeof(mi)};int x,y,w,h;
    if(ui_web_view_get_element_rect(s->view,id,&at)!=UI_STATUS_OK)return;
    ClientToScreen(s->web_hwnd,&origin);s->menu_anchor=(RECT){origin.x+MulDiv(at.x,(int)s->dpi,96),origin.y+MulDiv(at.y,(int)s->dpi,96),
        origin.x+MulDiv(at.x+at.width,(int)s->dpi,96),origin.y+MulDiv(at.y+at.height,(int)s->dpi,96)};
    GetWindowRect(s->popup_hwnd,&r);w=r.right-r.left;h=r.bottom-r.top;x=s->menu_anchor.left;y=s->menu_anchor.bottom;
    if(GetMonitorInfoW(MonitorFromRect(&s->menu_anchor,MONITOR_DEFAULTTONEAREST),&mi)){if(x+w>mi.rcWork.right)x=mi.rcWork.right-w;if(x<mi.rcWork.left)x=mi.rcWork.left;
        if(y+h>mi.rcWork.bottom)y=s->menu_anchor.top-h;if(y<mi.rcWork.top)y=mi.rcWork.top;}
    SetWindowPos(s->popup_hwnd,NULL,x,y,w,h,SWP_NOZORDER|SWP_NOACTIVATE);
}
