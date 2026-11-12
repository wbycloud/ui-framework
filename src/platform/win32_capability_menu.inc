/* HMENU-1: private thread/menu provenance; public WM_COMMAND never admits v2. */
static __declspec(thread) ui_native_shell_t *menu_shells;
static __declspec(thread) HHOOK menu_hook;
static int native_menu_style(HMENU menu)
{
    MENUINFO info={0};int at,count=GetMenuItemCount(menu);
    info.cbSize=sizeof(info);info.fMask=MIM_STYLE;info.dwStyle=MNS_NOTIFYBYPOS;
    if(!SetMenuInfo(menu,&info))return 0;
    for(at=0;at<count;++at){HMENU child=GetSubMenu(menu,at);if(child&&!native_menu_style(child))return 0;}
    return 1;
}
static int menu_contains(HMENU root,HMENU target)
{
    int at,count;if(root==target)return 1;count=GetMenuItemCount(root);
    for(at=0;at<count;++at){HMENU child=GetSubMenu(root,at);if(child&&menu_contains(child,target))return 1;}return 0;
}
static UINT menu_leaf(HMENU menu,UINT position)
{
    MENUITEMINFOW item={0};item.cbSize=sizeof(item);item.fMask=MIIM_ID|MIIM_STATE|MIIM_SUBMENU|MIIM_FTYPE;
    if(!GetMenuItemInfoW(menu,position,TRUE,&item)||item.hSubMenu||(item.fState&(MFS_DISABLED|MFS_GRAYED))||(item.fType&MFT_SEPARATOR))return 0;
    return item.wID;
}
static UINT menu_position(HMENU menu,UINT command)
{int at,count=GetMenuItemCount(menu);for(at=0;at<count;++at)if(menu_leaf(menu,(UINT)at)==command)return (UINT)at;return UINT_MAX;}
static UINT menu_mnemonic(HMENU menu,UINT key)
{
    UINT found=UINT_MAX;int at,count=GetMenuItemCount(menu);if(key>127)return found;if(key>='a'&&key<='z')key-=32;
    for(at=0;at<count;++at){wchar_t text[512];size_t k;if(!menu_leaf(menu,(UINT)at))continue;
      if(!GetMenuStringW(menu,(UINT)at,text,512,MF_BYPOSITION))continue;
      for(k=0;text[k];++k)if(text[k]==L'&'&&text[k+1]){UINT c=(UINT)text[++k];if(c==L'&')continue;if(c>='a'&&c<='z')c-=32;
        if(c==key){if(found!=UINT_MAX)return UINT_MAX;found=(UINT)at;}break;}}
    return found;
}
static int menu_available(ui_native_shell_t *s)
{return s->menu_loop&&s->active&&s->host->app_active&&!s->host->dispatch_blocked&&IsWindow(s->menu_owner)&&GetMenu(s->menu_owner)==s->menu&&IsMenu(s->menu);}
static void menu_record(ui_native_shell_t *s,const MSG *m,int qualified)
{
    UINT position=UINT_MAX,command=0;int mouse=m->message>=WM_MOUSEFIRST&&m->message<=WM_MOUSELAST;
    int keyboard=m->message>=WM_KEYFIRST&&m->message<=WM_KEYLAST;
    if(!mouse&&!keyboard)return;
    ++s->menu_sequence;s->menu_candidate=0;ui_capabilities_human_invalidate(s->host,s);
    if(!qualified||!menu_available(s)){s->menu_mouse_down=0;s->pressed_command=0;return;}
    if(mouse&&s->selected_menu){int hit=MenuItemFromPoint(s->menu_owner,s->selected_menu,m->pt);if(hit>=0)position=(UINT)hit;}
    if(position!=UINT_MAX)command=menu_leaf(s->selected_menu,position);
    if(m->message==WM_LBUTTONDOWN){s->menu_mouse_down=command!=0;s->pressed_command=command;return;}
    if(m->message==WM_LBUTTONUP){int match=s->menu_mouse_down&&command&&(!s->pressed_command||s->pressed_command==command);
      s->menu_mouse_down=0;s->pressed_command=0;if(!match)return;}
    else if(m->message==WM_KEYDOWN||m->message==WM_SYSKEYDOWN){
      if(m->lParam&(1L<<30))return;if(m->wParam!=VK_RETURN&&m->wParam!=VK_SPACE)return;
      position=menu_position(s->selected_menu,s->selected_command);command=menu_leaf(s->selected_menu,position);
    }else if(m->message==WM_CHAR||m->message==WM_SYSCHAR){
      position=menu_mnemonic(s->selected_menu,(UINT)m->wParam);command=menu_leaf(s->selected_menu,position);
    }else return;
    if(!command||!find_command_binding(s,command))return;
    s->candidate_menu=s->selected_menu;s->candidate_position=position;s->candidate_command=command;s->candidate_time=(DWORD)GetMessageTime();s->menu_candidate=1;
}
static LRESULT CALLBACK menu_filter_proc(int code,WPARAM wp,LPARAM lp)
{
    if(code==MSGF_MENU){ui_native_shell_t *s;MSG *m=(MSG *)lp;for(s=menu_shells;s;s=s->menu_filter_next)
      if(menu_available(s)){menu_record(s,m,ui_inputs_current_source());break;}}
    return CallNextHookEx(menu_hook,code,wp,lp);
}
typedef struct menu_guard {ui_native_shell_t *shell;uint64_t generation,sequence;} menu_guard_t;
static int menu_guard_valid(void *data)
{menu_guard_t *g=data;return menu_available(g->shell)&&g->generation==g->shell->menu_generation&&g->sequence==g->shell->menu_sequence;}
static void menu_loop_enter(ui_native_shell_t *s)
{
    if(s->menu_loop)return;s->menu_loop=1;s->menu_candidate=0;
    ++s->host->capability_ui_callbacks;++s->host->assistant_callbacks;ui_dispatch_enter(s->host);
}
static void menu_loop_leave(ui_native_shell_t *s)
{
    if(!s->menu_loop)return;s->menu_loop=0;s->menu_candidate=0;s->menu_mouse_down=0;s->selected_menu=NULL;s->selected_command=0;
    ++s->menu_sequence;ui_capabilities_human_invalidate(s->host,s);
    if(s->menu_titles_pending)(void)PostMessageW(s->menu_owner,WM_APP+0x3d2,(WPARAM)s,0);
    --s->host->capability_ui_callbacks;--s->host->assistant_callbacks;ui_dispatch_leave(s->host);
}
static LRESULT CALLBACK menu_owner_proc(HWND hwnd,UINT message,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR data)
{
    ui_native_shell_t *s=(ui_native_shell_t *)data;(void)id;
    if(message==WM_APP+0x3d2&&wp==(WPARAM)s&&s->menu_titles_pending&&!s->menu_loop){s->menu_titles_pending=0;(void)refresh_menu(s,0);return 0;}
    if(message==WM_NCLBUTTONDOWN&&wp==HTMENU&&GetMenu(hwnd)==s->menu){s->menu_mouse_down=ui_inputs_current_source();s->pressed_command=0;}
    if(message==WM_ENTERMENULOOP&&GetMenu(hwnd)==s->menu&&s->active)menu_loop_enter(s);
    else if(message==WM_MENUSELECT&&s->menu_loop){HMENU menu=(HMENU)lp;UINT flags=HIWORD(wp);
      if(menu&&menu_contains(s->menu,menu)&&flags!=0xffffu){
        if(flags&MF_POPUP){s->selected_menu=GetSubMenu(menu,LOWORD(wp));s->selected_command=0;}
        else{s->selected_menu=menu;s->selected_command=LOWORD(wp);}}}
    else if(message==WM_MENUCOMMAND&&menu_contains(s->menu,(HMENU)lp)){
      UINT position=(UINT)wp,command=menu_leaf((HMENU)lp,position);const char *name=find_command_binding(s,command);size_t at;const char *item=NULL;
      int qualified=s->menu_candidate&&menu_available(s)&&s->candidate_menu==(HMENU)lp&&s->candidate_position==position&&s->candidate_command==command&&s->candidate_time==(DWORD)GetMessageTime();
      menu_guard_t guard={s,s->menu_generation,s->menu_sequence};s->menu_candidate=0;
      for(at=0;at<s->binding_count;++at)if(s->bindings[at].native_id==command){item=s->bindings[at].item_id;break;}
      if(name&&ui_capabilities_nonread(s->host,name)){
        if(qualified&&item){ui_capabilities_human_begin(s->host,s);ui_capabilities_human_guard(s->host,menu_guard_valid,&guard);
          (void)ui_capabilities_ui_action(s->host,item,s);ui_capabilities_human_end(s->host);}
        return 0;
      }
      if(name)(void)ui_host_invoke(s->host,name,"{}","native-menu");return 0;
    }else if(message==WM_EXITMENULOOP||message==WM_CANCELMODE||message==WM_KILLFOCUS||message==WM_NCDESTROY)menu_loop_leave(s);
    return DefSubclassProc(hwnd,message,wp,lp);
}
static int native_menu_attach(ui_native_shell_t *s)
{
    if(s->web_chrome||s->offscreen)return 1;
    if(!menu_hook){menu_hook=SetWindowsHookExW(WH_MSGFILTER,menu_filter_proc,NULL,GetCurrentThreadId());if(!menu_hook)return 0;}
    if(!SetWindowSubclass(s->menu_owner,menu_owner_proc,(UINT_PTR)s,(DWORD_PTR)s)){if(!menu_shells){UnhookWindowsHookEx(menu_hook);menu_hook=NULL;}return 0;}
    s->menu_filter_next=menu_shells;menu_shells=s;return 1;
}
static void native_menu_detach(ui_native_shell_t *s)
{
    ui_native_shell_t **link;RemoveWindowSubclass(s->menu_owner,menu_owner_proc,(UINT_PTR)s);
    for(link=&menu_shells;*link&&*link!=s;link=&(*link)->menu_filter_next){}if(*link)*link=s->menu_filter_next;
    if(!menu_shells&&menu_hook){UnhookWindowsHookEx(menu_hook);menu_hook=NULL;}
}
#ifdef UI_FRAMEWORK_MENU_FIXTURE
/* Logic fixture only: not exported by the production DLL or physical evidence. */
void ui_native_menu_fixture(ui_native_shell_t *s,int action,void *menu,unsigned position,int qualified)
{
    MSG m={0};if(action==0){menu_loop_enter(s);s->selected_menu=(HMENU)menu;s->selected_command=menu_leaf((HMENU)menu,position);}
    else if(action==1){m.message=WM_KEYDOWN;m.wParam=VK_RETURN;menu_record(s,&m,qualified);}
    else if(action==2)menu_loop_leave(s);
}
#endif
