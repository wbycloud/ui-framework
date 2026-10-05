#include "ui_internal.h"
#include "ui_framework/menus.h"
#include "json_ui.h"
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#include <imm.h>
#endif
#ifdef UI_FRAMEWORK_ENABLE_LIGHT_WEB
#include "ui_framework/light_web.h"
#include "menu_page.h"
#endif

typedef struct menu_group {
    char *path, *title;
    int order;
    uint32_t access_key;
    struct menu_group *next;
} menu_group_t;
typedef struct menu_row {
    ui_menu_model_entry_t entry;
    char path[4096];
    int item_order;
} menu_row_t;
typedef struct menu_rows { menu_row_t rows[UI_MENU_MAX_ITEMS]; size_t count; ui_status_t status; } menu_rows_t;
typedef struct ui_menu_popup {
    ui_host_t *host;
    ui_web_backend_t *backend;
    ui_web_view_t *view;
    void *window, *previous_focus;
    char *path, *detail;
    struct ui_menu_popup *parent, *child, *next_root;
    ui_rect_t parent_row;
    char hover_id[4096];
    size_t page_count;
    char keyboard_focus[4096];
    ui_menu_anchor_t anchor;
    uint64_t target_id, generation;
    int open, toolbar, width, height, keyboard_root;
    size_t first, toolbar_first;
} ui_menu_popup_t;

static ui_menu_popup_t *active_popup(ui_host_t *host)
{ui_menu_popup_t *p=host?(ui_menu_popup_t *)host->menu_popup:NULL;while(p&&p->child&&p->child->open)p=p->child;return p;}
static void close_branch(ui_menu_popup_t *p)
{if(!p)return;close_branch(p->child);p->open=0;p->hover_id[0]=0;
#ifdef _WIN32
 if(p->window){KillTimer((HWND)p->window,1);ShowWindow((HWND)p->window,SW_HIDE);}
#endif
}
#if defined(_WIN32) && defined(UI_FRAMEWORK_ENABLE_LIGHT_WEB)
#ifdef _MSC_VER
__declspec(thread) static ui_menu_popup_t *menu_roots;
__declspec(thread) static HHOOK menu_hook;
#else
static _Thread_local ui_menu_popup_t *menu_roots;
static _Thread_local HHOOK menu_hook;
#endif
static int in_branch(ui_menu_popup_t *p,HWND w)
{for(;p;p=p->child)if(p->open&&p->window&&(w==(HWND)p->window||IsChild((HWND)p->window,w)))return 1;return 0;}
static LRESULT CALLBACK menu_messages(int code,WPARAM wp,LPARAM lp)
{if(code>=0&&wp==PM_REMOVE){MSG *m=(MSG *)lp;ui_menu_popup_t *p;
 for(p=menu_roots;p;p=p->next_root)if(p->open&&!p->detail){
  if(m->message==WM_MOUSEMOVE&&in_branch(p,m->hwnd)){ui_menu_popup_t *q;for(q=p;q;q=q->child)if(q->child&&in_branch(q->child,m->hwnd)){q->hover_id[0]=0;KillTimer((HWND)q->window,1);}}
  if((m->message==WM_LBUTTONDOWN||m->message==WM_RBUTTONDOWN||m->message==WM_MBUTTONDOWN||m->message==WM_NCLBUTTONDOWN)&&!in_branch(p,m->hwnd)){
   POINT pt={(short)LOWORD(m->lParam),(short)HIWORD(m->lParam)},origin={0,0};RECT anchor;if(m->message!=WM_NCLBUTTONDOWN)ClientToScreen(m->hwnd,&pt);ClientToScreen((HWND)p->host->native_parent,&origin);anchor.left=origin.x+MulDiv(p->anchor.rect.x,(int)p->host->dpi,96);anchor.top=origin.y+MulDiv(p->anchor.rect.y,(int)p->host->dpi,96);anchor.right=anchor.left+MulDiv(p->anchor.rect.width,(int)p->host->dpi,96);anchor.bottom=anchor.top+MulDiv(p->anchor.rect.height,(int)p->host->dpi,96);
   if(!PtInRect(&anchor,pt))(void)ui_host_close_menu(p->host);
  }
 }}return CallNextHookEx(menu_hook,code,wp,lp);}
#endif

static void menu_dom_id(const char *id,char out[80])
{uint64_t hash=UINT64_C(14695981039346656037);const unsigned char *p=(const unsigned char *)id;
 if(strlen(id)<64){snprintf(out,80,"menu-item-%s",id);return;}
 for(;*p;++p){hash^=*p;hash*=UINT64_C(1099511628211);}snprintf(out,80,"menu-item-%016llx",(unsigned long long)hash);}
static int valid_anchor(const ui_menu_anchor_t *a)
{return a&&a->size>=sizeof(*a)&&a->kind>=UI_MENU_ANCHOR_HOST&&a->kind<=UI_MENU_ANCHOR_COMPONENT_ROW&&
 a->rect.width>=0&&a->rect.height>=0&&a->rect.width<=32767&&a->rect.height<=32767&&a->rect.x>=-32767&&a->rect.x<=32767&&a->rect.y>=-32767&&a->rect.y<=32767;}
static int row_compare(const void *a,const void *b)
{
    const menu_row_t *x=(const menu_row_t *)a,*y=(const menu_row_t *)b;
    if(x->entry.order!=y->entry.order)return x->entry.order<y->entry.order?-1:1;
    return strcmp(x->entry.group?x->path:x->entry.id,y->entry.group?y->path:y->entry.id);
}
static menu_group_t *find_group(ui_host_t *host,const char *path)
{menu_group_t *g;for(g=(menu_group_t *)host->menu_groups;g;g=g->next)if(!strcmp(g->path,path))return g;return NULL;}
ui_status_t ui_host_register_menu_group(ui_host_t *host,const ui_menu_group_desc_t *desc)
{
    menu_group_t *g;
    if(!host||!desc||desc->size<offsetof(ui_menu_group_desc_t,access_key)||!desc->path||!*desc->path||strlen(desc->path)>4095||!desc->title)return UI_STATUS_INVALID_ARGUMENT;
    if(find_group(host,desc->path))return UI_STATUS_ALREADY_EXISTS;
    g=(menu_group_t *)calloc(1,sizeof(*g));if(!g)return UI_STATUS_OUT_OF_MEMORY;
    g->path=ui_strdup(desc->path);g->title=ui_strdup(desc->title);g->order=desc->order;
    if(desc->size>=offsetof(ui_menu_group_desc_t,access_key)+sizeof(desc->access_key))g->access_key=desc->access_key;
    if(g->access_key>='a'&&g->access_key<='z')g->access_key-=32;
    if(g->access_key&&!(g->access_key>='A'&&g->access_key<='Z')&&!(g->access_key>='0'&&g->access_key<='9')){free(g->path);free(g->title);free(g);return UI_STATUS_INVALID_ARGUMENT;}
    if(!g->path||!g->title){free(g->path);free(g->title);free(g);return UI_STATUS_OUT_OF_MEMORY;}
    g->next=(menu_group_t *)host->menu_groups;host->menu_groups=g;return ui_host_emit_event(host,"ui.commands.changed","{}");
}
static int active_entry(ui_host_t *host,ui_menu_entry_t *m,ui_command_state_t *state)
{
    state->size=sizeof(*state);if(ui_host_get_command_state(host,m->command_id,state)!=UI_STATUS_OK)return 0;
    state->visible=state->visible&&m->state.visible;state->enabled=state->enabled&&m->state.enabled;
    state->checked=state->checked||m->state.checked;state->busy=state->busy||m->state.busy;return state->visible;
}
static ui_status_t collect_menu(ui_host_t *host,const char *path,menu_rows_t *out)
{
    ui_menu_entry_t *m;size_t prefix=path?strlen(path):0;
    memset(out,0,sizeof(*out));out->status=UI_STATUS_OK;
    for(m=host->menus;m;m=m->next){ui_command_state_t state={0};const char *tail,*slash;menu_row_t *row;size_t i,length;menu_group_t *group;
        if(!active_entry(host,m,&state))continue;
        if(prefix){if(strncmp(m->menu_path,path,prefix))continue;if(m->menu_path[prefix]&&m->menu_path[prefix]!='/')continue;
            tail=m->menu_path+prefix;if(*tail=='/')++tail;}
        else tail=m->menu_path;
        if(*tail){slash=strchr(tail,'/');length=slash?(size_t)(slash-tail):strlen(tail);
            if(prefix+(prefix?1:0)+length+1>sizeof(out->rows[0].path))return UI_STATUS_LIMIT_EXCEEDED;
            for(i=0;i<out->count;++i){menu_row_t *r=&out->rows[i];
                if(r->entry.group&&strlen(r->path)==prefix+(prefix?1:0)+length&&!strncmp(r->path+prefix+(prefix?1:0),tail,length))break;}
            if(i<out->count){if(!find_group(host,out->rows[i].path)&&m->order<out->rows[i].entry.order)out->rows[i].entry.order=m->order;continue;}
        }else length=0;
        if(out->count==UI_MENU_MAX_ITEMS)return UI_STATUS_LIMIT_EXCEEDED;
        row=&out->rows[out->count++];row->entry.size=sizeof(row->entry);row->entry.order=m->order;row->entry.state=state;
        if(*tail){if(prefix)snprintf(row->path,sizeof(row->path),"%s/%.*s",path,(int)length,tail);else snprintf(row->path,sizeof(row->path),"%.*s",(int)length,tail);
            group=find_group(host,row->path);row->entry.group=1;row->entry.path=row->path;row->entry.id=row->path;
            row->entry.title=group?group->title:strrchr(row->path,'/');if(!group)row->entry.title=row->entry.title?row->entry.title+1:row->path;
            if(group){row->entry.order=group->order;row->entry.access_key=group->access_key;}row->entry.command_id="";row->entry.state.enabled=1;row->entry.state.busy=0;
        }else{row->entry.id=m->id;row->entry.title=m->title;row->entry.path=m->menu_path;row->entry.command_id=m->command_id;row->entry.image_id=m->image_id;row->entry.access_key=m->access_key;}
    }
    /* Group strings point into rows, so repair them after sorting. */
    qsort(out->rows,out->count,sizeof(out->rows[0]),row_compare);
    for(size_t i=0;i<out->count;++i)if(out->rows[i].entry.group){menu_row_t *r=&out->rows[i];menu_group_t *g=find_group(host,r->path);
        r->entry.path=r->entry.id=r->path;r->entry.title=g?g->title:strrchr(r->path,'/');if(!g)r->entry.title=r->entry.title?r->entry.title+1:r->path;}
    return UI_STATUS_OK;
}
ui_status_t ui_host_visit_menu(ui_host_t *host,const char *path,ui_menu_visit_fn visit,void *data)
{
    menu_rows_t *rows;ui_status_t status;
    if(!host||!visit||(path&&strlen(path)>4095))return UI_STATUS_INVALID_ARGUMENT;
    rows=(menu_rows_t *)malloc(sizeof(*rows));if(!rows)return UI_STATUS_OUT_OF_MEMORY;
    status=collect_menu(host,path?path:"",rows);if(status!=UI_STATUS_OK){free(rows);return status;}
    ui_dispatch_enter(host);for(size_t i=0;i<rows->count;++i)visit(&rows->rows[i].entry,data);free(rows);ui_dispatch_leave(host);return UI_STATUS_OK;
}
static int tool_compare(const void *a,const void *b)
{
    const menu_row_t *x=(const menu_row_t *)a,*y=(const menu_row_t *)b;int group;
    if(x->entry.order!=y->entry.order)return x->entry.order<y->entry.order?-1:1;
    group=strcmp(x->entry.path,y->entry.path);if(group)return group;
    if(x->item_order!=y->item_order)return x->item_order<y->item_order?-1:1;return strcmp(x->entry.id,y->entry.id);
}
static ui_status_t collect_tools(ui_host_t *host,menu_rows_t *out)
{
    ui_toolbar_item_entry_t *t;memset(out,0,sizeof(*out));
    for(t=host->toolbar_items;t;t=t->next){ui_toolbar_entry_t *bar;ui_command_state_t state={0};menu_row_t *r;state.size=sizeof(state);
        for(bar=host->toolbars;bar&&strcmp(bar->id,t->toolbar_id);bar=bar->next){}
        if(!bar||!bar->visible||!t->state.visible||ui_host_get_command_state(host,t->command_id,&state)!=UI_STATUS_OK||!state.visible)continue;
        if(out->count==UI_MENU_MAX_ITEMS)return UI_STATUS_LIMIT_EXCEEDED;r=&out->rows[out->count++];r->entry.size=sizeof(r->entry);
        r->entry.id=t->id;r->entry.path=bar->id;r->entry.title=t->title;r->entry.command_id=t->command_id;
        r->item_order=t->order;r->entry.order=bar->order;r->entry.image_id=t->image_id;r->entry.state=state;r->entry.state.enabled=state.enabled&&t->state.enabled;
        r->entry.state.busy=state.busy||t->state.busy;r->entry.state.checked=state.checked||t->state.checked;
        /* First sort key is group, second is stable item order. */
        snprintf(r->path,sizeof(r->path),"%010d:%s",t->order,t->id);
    }qsort(out->rows,out->count,sizeof(out->rows[0]),tool_compare);return UI_STATUS_OK;
}
static ui_status_t popup_rows(ui_menu_popup_t *p,menu_rows_t *rows)
{return p->toolbar?collect_tools(p->host,rows):collect_menu(p->host,p->path?p->path:"",rows);}
static int valid_target(ui_menu_popup_t *p)
{
    return !p->host->dispatch_blocked&&p->host->app_active&&!p->host->modal_component&&
        (!p->anchor.component||ui_component_menu_target_valid(p->anchor.component,p->generation,p->anchor.row_id));
}
ui_status_t ui_host_close_menu(ui_host_t *host)
{
    ui_menu_popup_t *p;if(!host)return UI_STATUS_INVALID_ARGUMENT;p=(ui_menu_popup_t *)host->menu_popup;
    host->menu_alt_pending=0;if(!p||!p->open)return UI_STATUS_OK;close_branch(p);host->menu_open_path=NULL;
#ifdef _WIN32
#ifdef UI_FRAMEWORK_ENABLE_LIGHT_WEB
    {ui_menu_popup_t *r;int any=0;for(r=menu_roots;r;r=r->next_root)if(r->open)any=1;if(!any&&menu_hook){UnhookWindowsHookEx(menu_hook);menu_hook=NULL;}}
#endif
    if(host->app_active&&!host->dispatch_blocked&&!host->modal_component&&p->previous_focus&&IsWindow((HWND)p->previous_focus)&&IsWindowEnabled((HWND)p->previous_focus)&&IsWindowVisible((HWND)p->previous_focus))SetFocus((HWND)p->previous_focus);
#endif
    (void)ui_host_emit_event(host,"ui.host.menu_changed","{}");return UI_STATUS_OK;
}
#ifdef UI_FRAMEWORK_ENABLE_LIGHT_WEB
static ui_status_t paint_popup(ui_menu_popup_t *p)
{
    ui_json_t json={0};menu_rows_t *rows=(menu_rows_t *)calloc(1,sizeof(*rows));ui_status_t status;
    size_t page_count=p->page_count,start=p->toolbar_first+p->first,end;
    if(!rows)return UI_STATUS_OUT_OF_MEMORY;
    status=p->detail?UI_STATUS_OK:popup_rows(p,rows);if(status!=UI_STATUS_OK){free(rows);return status;}
    if(page_count<1)page_count=1;if(start>=rows->count&&rows->count){p->first=0;start=p->toolbar_first;}end=start+page_count;if(end>rows->count)end=rows->count;
    uj_add(&json,"{\"title\":");uj_string(&json,p->toolbar?"工具":p->path?p->path:"菜单");uj_add(&json,",\"detail\":");uj_string(&json,p->detail?p->detail:"");
    uj_add(&json,",\"focus\":");uj_string(&json,p->keyboard_focus);p->keyboard_focus[0]=0;
    uj_fmt(&json,",\"dark\":%s,\"first\":%zu,\"total\":%zu,\"back\":%s,\"expanded\":",p->host->menu_dark?"true":"false",start,rows->count,p->parent?"true":"false");uj_string(&json,p->child&&p->child->open?p->child->path:"");uj_add(&json,",\"items\":[");
    for(size_t i=start;i<end;++i){ui_menu_model_entry_t *e=&rows->rows[i].entry;if(i>start)uj_add(&json,",");uj_add(&json,"{\"id\":");uj_string(&json,e->id);{char dom[80];menu_dom_id(e->id,dom);uj_add(&json,",\"dom\":");uj_string(&json,dom);}
        uj_add(&json,",\"title\":");if(p->toolbar){ui_toolbar_entry_t *bar;for(bar=p->host->toolbars;bar&&strcmp(bar->id,e->path);bar=bar->next){}uj_string(&json,bar?bar->title:"");uj_add(&json,",\"section\":");}uj_string(&json,e->title);
        {char shortcut[80]="";ui_command_entry_t *c;for(c=p->host->commands;c&&strcmp(c->id,e->command_id);c=c->next){}if(c&&c->shortcut_key){snprintf(shortcut,sizeof(shortcut),"%s%s%s%c",c->shortcut_modifiers&1?"Ctrl+":"",c->shortcut_modifiers&2?"Shift+":"",c->shortcut_modifiers&4?"Alt+":"",c->shortcut_key>=32&&c->shortcut_key<=126?(char)c->shortcut_key:63);}uj_add(&json,",\"shortcut\":");uj_string(&json,shortcut);}
        uj_fmt(&json,",\"access\":%u,\"group\":%s,\"image\":\"%llu\",\"enabled\":%s,\"checked\":%s,\"busy\":%s}",e->access_key,e->group?"true":"false",(unsigned long long)e->image_id,e->state.enabled?"true":"false",e->state.checked?"true":"false",e->state.busy?"true":"false");}
    uj_add(&json,"]}");free(rows);status=json.failed?UI_STATUS_LIMIT_EXCEEDED:ui_web_view_post_json(p->view,json.data);free(json.data);return status;
}
static ui_status_t create_popup(ui_host_t *,ui_menu_popup_t *,ui_menu_popup_t **);
static ui_status_t show_popup(ui_menu_popup_t *);
static ui_status_t open_child(ui_menu_popup_t *p,const char *path)
{ui_menu_popup_t *child;ui_element_presentation_t row={0};char dom[80];size_t nodes=0,bytes=0;
 if(p->keyboard_root&&!p->parent){char *copy=ui_strdup(path);if(!copy)return UI_STATUS_OUT_OF_MEMORY;free(p->path);p->path=copy;p->keyboard_root=0;p->first=0;return show_popup(p);}
 for(ui_menu_popup_t *q=p;q;q=q->parent){bytes+=(size_t)q->width*q->height*4*p->host->dpi*p->host->dpi/9216;}
 /* Cached panes also count: each reserves its worst page of 10 six-node rows. */
 {ui_menu_popup_t *root=p;while(root->parent)root=root->parent;for(ui_menu_popup_t *q=root;q;q=q->child)nodes+=76;}
 if(nodes+(p->child?0:76)>1024||bytes+(size_t)320*324*4*p->host->dpi*p->host->dpi/9216>32u*1024u*1024u)return UI_STATUS_LIMIT_EXCEEDED;
 if(p->child&&p->child->open&&!strcmp(p->child->path,path))return UI_STATUS_OK;
 row.size=sizeof(row);menu_dom_id(path,dom);if(ui_web_view_get_presentation(p->view,dom,&row)!=UI_STATUS_OK||!row.visible)return UI_STATUS_NOT_FOUND;
 close_branch(p->child);if(!p->child){ui_status_t st=create_popup(p->host,p,&p->child);if(st!=UI_STATUS_OK)return st;}child=p->child;
 free(child->path);child->path=ui_strdup(path);if(!child->path)return UI_STATUS_OUT_OF_MEMORY;child->anchor=p->anchor;child->target_id=p->target_id;child->generation=p->generation;child->parent_row=row.rect;child->first=0;child->keyboard_focus[0]=0;
 {ui_status_t st=show_popup(child);if(st==UI_STATUS_OK)(void)paint_popup(p);return st;}
}
static void popup_message(ui_web_view_t *view,const char *json,void *data)
{
    ui_menu_popup_t *p=(ui_menu_popup_t *)data;char action[32],id[4096];menu_rows_t *rows;size_t i;(void)view;
    if(!p->open||!valid_target(p)){(void)ui_host_close_menu(p->host);return;}
    if(!uj_get(json,"action",action,sizeof(action)))return;
    if(!strcmp(action,"leave")){p->hover_id[0]=0;
#ifdef _WIN32
      if(p->window)KillTimer((HWND)p->window,1);
#endif
      return;}
    if(!strcmp(action,"close")){(void)ui_host_close_menu(p->host);return;}
    if(!strcmp(action,"scroll")){close_branch(p->child);char value[16];int delta=uj_get(json,"delta",value,sizeof(value))?atoi(value):0;
        if(delta<0)p->first=p->first?p->first-1:0;else ++p->first;
        if(paint_popup(p)!=UI_STATUS_OK)(void)ui_host_close_menu(p->host);return;}
    if(!strcmp(action,"back")){if(p->parent){ui_menu_popup_t *parent=p->parent;menu_rows_t *back=(menu_rows_t *)malloc(sizeof(*back));close_branch(p);if(back&&popup_rows(parent,back)==UI_STATUS_OK&&back->count)snprintf(parent->keyboard_focus,sizeof(parent->keyboard_focus),"%s",back->rows[0].entry.id);free(back);(void)paint_popup(parent);}else (void)ui_host_close_menu(p->host);return;}
    rows=(menu_rows_t *)calloc(1,sizeof(*rows));if(!rows)return;
    if(!strcmp(action,"group")){while(p->parent)p=p->parent;close_branch(p->child);char value[16];int delta=uj_get(json,"delta",value,sizeof(value))?atoi(value):0;size_t at=0;
        if(p->toolbar||collect_menu(p->host,"",rows)!=UI_STATUS_OK||!rows->count){free(rows);return;}
        for(i=0;i<rows->count;++i){const char *path=rows->rows[i].entry.path;size_t n=strlen(path);if(p->path&&!strncmp(p->path,path,n)&&(p->path[n]==0||p->path[n]=='/')){at=i;break;}}
        at=(at+rows->count+(delta<0?rows->count-1:1))%rows->count;
        {char *copy=ui_strdup(rows->rows[at].entry.path);free(rows);if(!copy)return;free(p->path);p->path=copy;}p->first=0;
        if(show_popup(p)!=UI_STATUS_OK)(void)ui_host_close_menu(p->host);return;}
    if((strcmp(action,"choose")&&strcmp(action,"hover"))||!uj_get(json,"id",id,sizeof(id))||popup_rows(p,rows)!=UI_STATUS_OK){free(rows);return;}
    for(i=0;i<rows->count;++i){ui_menu_model_entry_t *e=&rows->rows[i].entry;if(strcmp(e->id,id))continue;
        if(!e->state.enabled||e->state.busy)break;
        if(!strcmp(action,"hover")){
#ifdef _WIN32
            if(p->window&&p->child&&p->child->open&&strcmp(p->child->path,e->path)){snprintf(p->hover_id,sizeof(p->hover_id),"%s",id);SetTimer((HWND)p->window,1,160,NULL);free(rows);return;}
#endif
            if(e->group){char *copy=ui_strdup(e->path);free(rows);if(copy){if(open_child(p,copy)!=UI_STATUS_OK)(void)ui_host_close_menu(p->host);free(copy);}return;}close_branch(p->child);free(rows);(void)paint_popup(p);return;}
        if(e->group){char *copy=ui_strdup(e->path);free(rows);if(copy){if(open_child(p,copy)!=UI_STATUS_OK)(void)ui_host_close_menu(p->host);free(copy);}return;}
        {char *command=ui_strdup(e->command_id);ui_json_t params={0};ui_host_t *host=p->host;free(rows);
            uj_fmt(&params,"{\"action\":\"menu\",\"id\":\"%llu\",\"target\":\"%llu\"}",(unsigned long long)p->target_id,(unsigned long long)p->target_id);
            (void)ui_host_close_menu(host);if(command&&!params.failed)(void)ui_host_invoke(host,command,params.data,"menu");free(command);free(params.data);}return;
    }free(rows);
}
#ifdef _WIN32
static LRESULT CALLBACK popup_proc(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    ui_menu_popup_t *p=(ui_menu_popup_t *)GetWindowLongPtrW(window,GWLP_USERDATA);
    if(message==WM_NCCREATE){p=(ui_menu_popup_t *)((CREATESTRUCTW *)lp)->lpCreateParams;SetWindowLongPtrW(window,GWLP_USERDATA,(LONG_PTR)p);return TRUE;}
    if(p&&message==WM_TIMER&&wp==1){char id[4096];KillTimer(window,1);snprintf(id,sizeof(id),"%s",p->hover_id);p->hover_id[0]=0;if(*id){ui_json_t j={0};close_branch(p->child);uj_add(&j,"{\"action\":\"hover\",\"id\":");uj_string(&j,id);uj_add(&j,"}");if(!j.failed)popup_message(p->view,j.data,p);free(j.data);}return 0;}
    if(p&&message==WM_ACTIVATE&&LOWORD(wp)==WA_INACTIVE){ui_menu_popup_t *root=p;while(root->parent)root=root->parent;if(in_branch(root,(HWND)lp)||(HWND)lp==GetAncestor((HWND)p->host->native_parent,GA_ROOT))return 0;root->previous_focus=NULL;}
    if(p&&(message==WM_CLOSE||message==WM_DPICHANGED||(message==WM_ACTIVATE&&LOWORD(wp)==WA_INACTIVE))){(void)ui_host_close_menu(p->host);return 0;}
    return DefWindowProcW(window,message,wp,lp);
}
#endif
static ui_status_t create_popup(ui_host_t *host,ui_menu_popup_t *parent,ui_menu_popup_t **out)
{
    ui_menu_popup_t *p=NULL;ui_light_web_config_t config={0};ui_status_t status;
    p=(ui_menu_popup_t *)calloc(1,sizeof(*p));if(!p)return UI_STATUS_OUT_OF_MEMORY;p->host=host;p->parent=parent;
#ifdef _WIN32
    if(host->native_parent){WNDCLASSW wc={0};wc.lpfnWndProc=popup_proc;wc.hInstance=GetModuleHandleW(L"ui_framework.dll");wc.lpszClassName=L"UIFrameworkRegisteredMenu4";
        wc.hCursor=LoadCursorW(NULL,MAKEINTRESOURCEW(32512));if(!RegisterClassW(&wc)&&GetLastError()!=ERROR_CLASS_ALREADY_EXISTS){free(p);return UI_STATUS_PLATFORM_ERROR;}
        p->window=CreateWindowExW(WS_EX_TOOLWINDOW,wc.lpszClassName,L"",WS_POPUP|WS_BORDER,0,0,1,1,GetAncestor((HWND)host->native_parent,GA_ROOT),NULL,wc.hInstance,p);
        if(!p->window){free(p);return UI_STATUS_PLATFORM_ERROR;}}
#endif
    config.size=sizeof(config);config.parent_hwnd=p->window;p->backend=ui_light_web_backend_create(&config);
    p->view=p->backend?ui_web_view_create(host,p->backend):NULL;status=p->view?ui_web_view_set_message_callback(p->view,popup_message,p):UI_STATUS_OUT_OF_MEMORY;
#ifdef _WIN32
    if(p->view&&p->window)(void)ImmAssociateContextEx((HWND)ui_web_view_native_handle(p->view),NULL,IACE_CHILDREN);
#endif
    if(status==UI_STATUS_OK)status=ui_web_view_load_html(p->view,ui_menu_page);
    if(status!=UI_STATUS_OK){if(p->view)ui_web_view_destroy(p->view);if(p->backend)ui_light_web_backend_destroy(p->backend);
#ifdef _WIN32
        if(p->window)DestroyWindow((HWND)p->window);
#endif
        free(p);return status;}
    if(!parent){host->menu_popup=p;
#ifdef _WIN32
    p->next_root=menu_roots;menu_roots=p;
#endif
    }*out=p;return UI_STATUS_OK;
}
static ui_status_t ensure_popup(ui_host_t *host,ui_menu_popup_t **out)
{if(host->menu_popup){*out=(ui_menu_popup_t *)host->menu_popup;return UI_STATUS_OK;}return create_popup(host,NULL,out);}
static ui_status_t show_popup(ui_menu_popup_t *p)
{
    ui_rect_t rect=p->anchor.rect;void *native=p->host->native_parent;uint32_t dpi=p->host->dpi;ui_status_t status;
    if(p->anchor.kind==UI_MENU_ANCHOR_CONTENT_SLOT){ui_rect_t slot;
        if(!ui_content_slot_belongs_to(p->anchor.slot,p->host)||ui_content_slot_get_rect(p->anchor.slot,&slot)!=UI_STATUS_OK)return UI_STATUS_INVALID_ARGUMENT;
        native=ui_content_slot_native_handle(p->anchor.slot);if(!native){rect.x+=slot.x;rect.y+=slot.y;}
    }else if(p->anchor.kind==UI_MENU_ANCHOR_COMPONENT_ROW){
        status=ui_component_menu_anchor(p->anchor.component,p->host,p->anchor.row_id,&rect,&native,&p->generation);if(status!=UI_STATUS_OK)return status;}
    else if(p->anchor.kind!=UI_MENU_ANCHOR_HOST)return UI_STATUS_INVALID_ARGUMENT;
    {menu_rows_t *rows=(menu_rows_t *)malloc(sizeof(*rows));size_t count=0;if(!rows)return UI_STATUS_OUT_OF_MEMORY;if(!p->detail){status=popup_rows(p,rows);count=rows->count;if(!p->open){p->keyboard_focus[0]=0;for(size_t i=0;i<count;i++)if(rows->rows[i].entry.state.enabled&&!rows->rows[i].entry.state.busy){snprintf(p->keyboard_focus,sizeof(p->keyboard_focus),"%s",rows->rows[i].entry.id);break;}}if(status!=UI_STATUS_OK){free(rows);return status;}}free(rows);p->page_count=count<10?count:10;if(!p->page_count)p->page_count=1;p->width=320;p->height=p->detail?180:(int)p->page_count*28+44;}
#ifdef _WIN32
    if(p->window){POINT point={MulDiv(rect.x,(int)dpi,96),MulDiv(rect.y+rect.height,(int)dpi,96)};MONITORINFO monitor={sizeof(monitor)};
        int width,height,x,y;ClientToScreen((HWND)native,&point);GetMonitorInfoW(MonitorFromPoint(point,MONITOR_DEFAULTTONEAREST),&monitor);
        width=MulDiv(p->width,(int)dpi,96);height=MulDiv(p->height,(int)dpi,96);if(height>monitor.rcWork.bottom-monitor.rcWork.top)height=monitor.rcWork.bottom-monitor.rcWork.top;
        if(width>monitor.rcWork.right-monitor.rcWork.left)width=monitor.rcWork.right-monitor.rcWork.left;
        x=point.x;y=point.y;if(p->parent&&p->parent->window){POINT at={MulDiv(p->parent_row.x,(int)dpi,96),MulDiv(p->parent_row.y,(int)dpi,96)};RECT parent_rect;ClientToScreen((HWND)p->parent->window,&at);GetWindowRect((HWND)p->parent->window,&parent_rect);x=parent_rect.right-2;y=at.y-4;if(x+width>monitor.rcWork.right)x=parent_rect.left-width+2;}
        if(y+height>monitor.rcWork.bottom)y=p->parent?monitor.rcWork.bottom-height:point.y-MulDiv(rect.height,(int)dpi,96)-height;
        if(x+width>monitor.rcWork.right)x=p->parent?monitor.rcWork.right-width:point.x+MulDiv(rect.width,(int)dpi,96)-width;
        if(x<monitor.rcWork.left)x=monitor.rcWork.left;if(y<monitor.rcWork.top)y=monitor.rcWork.top;
        p->width=MulDiv(width,96,(int)dpi);p->height=MulDiv(height,96,(int)dpi);if((p->height-44)/28<(int)p->page_count)p->page_count=p->height>44?(size_t)(p->height-44)/28:1;if(!p->open)p->previous_focus=GetFocus();
        SetWindowPos((HWND)p->window,HWND_TOP,x,y,width,height,SWP_NOACTIVATE);
    }
#endif
    status=ui_web_view_resize(p->view,p->width,p->height,dpi);if(status!=UI_STATUS_OK)return status;p->open=1;
#ifdef _WIN32
    if(p->window){if(!menu_hook&&!p->parent&&!p->detail)menu_hook=SetWindowsHookExW(WH_GETMESSAGE,menu_messages,NULL,GetCurrentThreadId());ShowWindow((HWND)p->window,p->detail||p->parent?SW_SHOWNOACTIVATE:SW_SHOW);
        if(!p->detail&&!p->parent)SetFocus((HWND)ui_web_view_native_handle(p->view));}
#endif
    if(!p->parent&&!p->detail){p->host->menu_open_path=p->path?p->path:"";(void)ui_host_emit_event(p->host,"ui.host.menu_changed","{}");}
    status=paint_popup(p);if(status!=UI_STATUS_OK)(void)ui_host_close_menu(p->host);return status;
}
#endif
ui_status_t ui_host_show_menu(ui_host_t *host,const ui_menu_popup_desc_t *desc)
{
#ifdef UI_FRAMEWORK_ENABLE_LIGHT_WEB
    ui_menu_popup_t *p;menu_rows_t *rows;ui_status_t status;size_t count;
    if(!host||!desc||desc->size<sizeof(*desc)||!desc->path||strlen(desc->path)>4095||!valid_anchor(&desc->anchor))return UI_STATUS_INVALID_ARGUMENT;
    if(host->dispatch_blocked||!host->app_active||host->modal_component)return UI_STATUS_CANCELLED;
    rows=(menu_rows_t *)malloc(sizeof(*rows));if(!rows)return UI_STATUS_OUT_OF_MEMORY;
    status=collect_menu(host,desc->path,rows);count=rows->count;free(rows);if(status!=UI_STATUS_OK)return status;if(!count)return UI_STATUS_NOT_FOUND;
    status=ensure_popup(host,&p);if(status!=UI_STATUS_OK)return status;(void)ui_host_close_menu(host);
    free(p->path);p->path=ui_strdup(desc->path);free(p->detail);p->detail=NULL;if(!p->path)return UI_STATUS_OUT_OF_MEMORY;
    p->keyboard_root=!*desc->path&&desc->anchor.rect.width==0;p->anchor=desc->anchor;p->target_id=desc->target_id;p->toolbar=0;p->toolbar_first=p->first=0;return show_popup(p);
#else
    (void)host;(void)desc;return UI_STATUS_UNSUPPORTED;
#endif
}
ui_status_t ui_host_show_toolbar_menu(ui_host_t *host,const ui_menu_anchor_t *anchor,size_t first)
{
#ifdef UI_FRAMEWORK_ENABLE_LIGHT_WEB
    ui_menu_popup_t *p;menu_rows_t *rows;ui_status_t status;size_t count;
    if(!host||!valid_anchor(anchor))return UI_STATUS_INVALID_ARGUMENT;
    if(host->dispatch_blocked||!host->app_active||host->modal_component)return UI_STATUS_CANCELLED;
    rows=(menu_rows_t *)malloc(sizeof(*rows));if(!rows)return UI_STATUS_OUT_OF_MEMORY;
    status=collect_tools(host,rows);count=rows->count;free(rows);if(status!=UI_STATUS_OK)return status;if(first>=count)return UI_STATUS_NOT_FOUND;
    status=ensure_popup(host,&p);if(status!=UI_STATUS_OK)return status;(void)ui_host_close_menu(host);
    free(p->path);p->path=NULL;free(p->detail);p->detail=NULL;p->anchor=*anchor;p->target_id=0;p->toolbar=1;p->first=0;p->toolbar_first=first;return show_popup(p);
#else
    (void)host;(void)anchor;(void)first;return UI_STATUS_UNSUPPORTED;
#endif
}
ui_status_t ui_host_show_tooltip(ui_host_t *host,const ui_menu_anchor_t *anchor,const char *text)
{
#ifdef UI_FRAMEWORK_ENABLE_LIGHT_WEB
    ui_menu_popup_t *p;ui_status_t status;if(!host||!valid_anchor(anchor)||!text||strlen(text)>4095)return UI_STATUS_INVALID_ARGUMENT;
    status=ensure_popup(host,&p);if(status!=UI_STATUS_OK)return status;if(p->open&&!p->detail)return UI_STATUS_CANCELLED;
    (void)ui_host_close_menu(host);free(p->detail);p->detail=ui_strdup(text);if(!p->detail)return UI_STATUS_OUT_OF_MEMORY;
    free(p->path);p->path=NULL;p->anchor=*anchor;p->toolbar=0;p->first=p->toolbar_first=0;return show_popup(p);
#else
    (void)host;(void)anchor;(void)text;return UI_STATUS_UNSUPPORTED;
#endif
}
ui_status_t ui_host_menu_dispatch_input(ui_host_t *host,const ui_input_event_t *event)
{
 ui_menu_popup_t *p;ui_status_t status;ui_command_entry_t *c;
 if(!host||!event||event->size<sizeof(*event))return UI_STATUS_INVALID_ARGUMENT;p=active_popup(host);
 if(host->dispatch_blocked||!host->app_active||host->modal_component){host->menu_alt_pending=0;host->menu_pressed_key=0;(void)ui_host_close_menu(host);return UI_STATUS_CANCELLED;}
 if(event->kind==UI_INPUT_KEY_UP){
   if(event->key_code==18&&host->menu_alt_pending){ui_menu_popup_desc_t d={0};host->menu_alt_pending=0;d.size=sizeof(d);d.path="";d.anchor.size=sizeof(d.anchor);d.anchor.rect.height=host->layout_rects[UI_LAYOUT_REGION_MENU_BAR].height;return ui_host_show_menu(host,&d);}
   if(host->menu_pressed_key==event->key_code){host->menu_pressed_key=0;return UI_STATUS_OK;}
   return p&&p->open?UI_STATUS_OK:UI_STATUS_NOT_FOUND;
 }
 if(event->kind==UI_INPUT_KEY_DOWN){
   if(event->modifiers&(UI_INPUT_MODIFIER_CONTROL|UI_INPUT_MODIFIER_SHIFT)){host->menu_alt_pending=0;return UI_STATUS_NOT_FOUND;}
   if(event->key_code==18){if(host->menu_pressed_key==18||host->menu_alt_pending)return UI_STATUS_OK;if(p&&p->open&&!p->detail){(void)ui_host_close_menu(host);host->menu_pressed_key=18;}else host->menu_alt_pending=1;return UI_STATUS_OK;}
   host->menu_alt_pending=0;
   if((event->modifiers&UI_INPUT_MODIFIER_ALT)&&(event->key_code<'0'||event->key_code>'Z'))return UI_STATUS_NOT_FOUND;
   for(c=host->commands;c;c=c->next)if(event->modifiers&&c->shortcut_key==event->key_code&&c->shortcut_modifiers==event->modifiers&&c->state.visible&&c->state.enabled&&!c->state.busy)return UI_STATUS_NOT_FOUND;
   if((!p||!p->open)&&(event->modifiers&UI_INPUT_MODIFIER_ALT)){
     menu_rows_t *rows=(menu_rows_t *)malloc(sizeof(*rows));int found=0;if(!rows)return UI_STATUS_OUT_OF_MEMORY;
     status=collect_menu(host,"",rows);if(status==UI_STATUS_OK)for(size_t i=0;i<rows->count;++i)if(rows->rows[i].entry.access_key==event->key_code){found=1;break;}free(rows);if(!found)return UI_STATUS_NOT_FOUND;
     {ui_menu_popup_desc_t d={0};d.size=sizeof(d);d.path="";d.anchor.size=sizeof(d.anchor);d.anchor.rect.height=host->layout_rects[UI_LAYOUT_REGION_MENU_BAR].height;status=ui_host_show_menu(host,&d);if(status!=UI_STATUS_OK)return status;}p=active_popup(host);
   }
   if(host->menu_pressed_key==event->key_code)return UI_STATUS_OK;
   if(!p||!p->open||p->detail)return UI_STATUS_NOT_FOUND;host->menu_pressed_key=event->key_code;
#ifdef UI_FRAMEWORK_ENABLE_LIGHT_WEB
   if(event->key_code>='0'&&event->key_code<='Z'){
     menu_rows_t *rows=(menu_rows_t *)malloc(sizeof(*rows));size_t hits[UI_MENU_MAX_ITEMS],n=0,next=0;
     if(!rows)return UI_STATUS_OUT_OF_MEMORY;status=popup_rows(p,rows);if(status==UI_STATUS_OK)for(size_t i=0;i<rows->count;++i){ui_menu_model_entry_t *e=&rows->rows[i].entry;
       if(e->access_key==event->key_code&&e->state.enabled&&!e->state.busy){ui_element_presentation_t at={0};at.size=sizeof(at);hits[n++]=i;
         if(ui_host_menu_get_item_presentation(host,e->id,&at)==UI_STATUS_OK&&at.focused)next=n;}}
     if(n){ui_menu_model_entry_t *e=&rows->rows[hits[next%n]].entry;
       if(n==1){ui_json_t j={0};uj_add(&j,"{\"action\":\"choose\",\"id\":");uj_string(&j,e->id);uj_add(&j,"}");free(rows);if(!j.failed)popup_message(p->view,j.data,p);status=j.failed?UI_STATUS_LIMIT_EXCEEDED:UI_STATUS_OK;free(j.data);return status;}
       snprintf(p->keyboard_focus,sizeof(p->keyboard_focus),"%s",e->id);p->first=hits[next%n];free(rows);return paint_popup(p);
     }free(rows);host->menu_pressed_key=0;return UI_STATUS_NOT_FOUND;
   }
#endif
 }
 if(!p||!p->open)return UI_STATUS_NOT_FOUND;
 if(!valid_target(p)){(void)ui_host_close_menu(host);return UI_STATUS_CANCELLED;}
 if(event->kind==UI_INPUT_POINTER_DOWN&&(event->x<0||event->y<0||event->x>=p->width||event->y>=p->height))return ui_host_close_menu(host);
 host->menu_routing=1;status=ui_web_view_dispatch_input(p->view,event);host->menu_routing=0;return status;
}
ui_status_t ui_menus_route_input(ui_host_t *host,const void *view_data,const ui_input_event_t *event,int composing)
{(void)view_data;if(!host||host->menu_routing||composing||(event->kind!=UI_INPUT_KEY_DOWN&&event->kind!=UI_INPUT_KEY_UP))return UI_STATUS_NOT_FOUND;return ui_host_menu_dispatch_input(host,event);}
ui_status_t ui_host_menu_get_presentation(ui_host_t *host,const char *id,ui_element_presentation_t *out)
{ui_menu_popup_t *p=active_popup(host);return !p||!p->open?UI_STATUS_NOT_FOUND:ui_web_view_get_presentation(p->view,id,out);}
ui_status_t ui_host_menu_capture_rgba(ui_host_t *host,ui_pixel_buffer_t *out)
{ui_menu_popup_t *p=active_popup(host);return !p||!p->open?UI_STATUS_NOT_FOUND:ui_web_view_capture_rgba(p->view,p->width,p->height,host->dpi,out);}
ui_status_t ui_host_menu_flush(ui_host_t *host,uint32_t budget)
{ui_menu_popup_t *p=active_popup(host);return !p||!p->open?UI_STATUS_NOT_FOUND:ui_web_view_flush(p->view,budget);}
void ui_menus_commands_changed(ui_host_t *host)
{
#ifdef UI_FRAMEWORK_ENABLE_LIGHT_WEB
    ui_menu_popup_t *p=(ui_menu_popup_t *)host->menu_popup;for(;p&&p->open;p=p->child)(void)paint_popup(p);
#else
    (void)host;
#endif
}
void ui_menus_component_invalidated(ui_host_t *host,ui_component_t *c,uint64_t id)
{ui_menu_popup_t *p=(ui_menu_popup_t *)host->menu_popup;if(p&&p->anchor.component==c&&(!id||p->anchor.row_id==id)){(void)ui_host_close_menu(host);p->anchor.component=NULL;}}
static void destroy_popup(ui_menu_popup_t *p)
{if(!p)return;destroy_popup(p->child);if(p->view)ui_web_view_destroy(p->view);
#ifdef UI_FRAMEWORK_ENABLE_LIGHT_WEB
 if(p->backend)ui_light_web_backend_destroy(p->backend);
#endif
#ifdef _WIN32
 if(p->window)DestroyWindow((HWND)p->window);
#endif
 free(p->path);free(p->detail);free(p);}
void ui_menus_destroy(ui_host_t *host)
{ui_menu_popup_t *p=(ui_menu_popup_t *)host->menu_popup;menu_group_t *g=(menu_group_t *)host->menu_groups;if(p){(void)ui_host_close_menu(host);
#if defined(_WIN32) && defined(UI_FRAMEWORK_ENABLE_LIGHT_WEB)
 ui_menu_popup_t **link=&menu_roots;while(*link&&*link!=p)link=&(*link)->next_root;if(*link)*link=p->next_root;
#endif
 destroy_popup(p);host->menu_popup=NULL;}while(g){menu_group_t *next=g->next;free(g->path);free(g->title);free(g);g=next;}host->menu_groups=NULL;}

void ui_menus_hide_tooltip(ui_host_t *host)
{ui_menu_popup_t *p=host?(ui_menu_popup_t *)host->menu_popup:NULL;if(p&&p->detail)(void)ui_host_close_menu(host);}

ui_status_t ui_host_hide_tooltip(ui_host_t *host)
{if(!host)return UI_STATUS_INVALID_ARGUMENT;ui_menus_hide_tooltip(host);return UI_STATUS_OK;}
ui_status_t ui_host_menu_get_capabilities(ui_host_t *host,uint64_t *out)
{if(!host||!out)return UI_STATUS_INVALID_ARGUMENT;*out=UI_MENU_CAP_MODEL;
#ifdef UI_FRAMEWORK_ENABLE_LIGHT_WEB
*out|=UI_MENU_CAP_POPUP|UI_MENU_CAP_OFFSCREEN|UI_MENU_CAP_ACCESS_KEYS;
#endif
return UI_STATUS_OK;}

ui_status_t ui_host_menu_get_item_presentation(ui_host_t *host,const char *id,ui_element_presentation_t *out)
{char dom[80];if(!host||!id)return UI_STATUS_INVALID_ARGUMENT;menu_dom_id(id,dom);return ui_host_menu_get_presentation(host,dom,out);}
