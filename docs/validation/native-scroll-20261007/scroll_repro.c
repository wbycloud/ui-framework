/* Read-only SDK reproducer: synchronous data, actual SendInput, no application
 * workers or business handlers. It diagnoses framework responsibility only. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui_framework/ui.h"
#include "ui_framework/native.h"
#include "ui_framework/shell.h"
#include "ui_framework/components.h"
#include "regression_version.h"

static unsigned failures;
static void check(int ok,const char *text){fprintf(stderr,"%s: %s\n",ok?"PASS":"FAIL",text);if(!ok)++failures;}
static void pump(unsigned ms)
{ULONGLONG end=GetTickCount64()+ms;do{MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}Sleep(1);}while(GetTickCount64()<end);}
static void source(ui_component_t *c,const ui_component_query_t *q,void *user)
{
    ui_column_desc_t *columns=user;ui_row_t *rows=calloc(128,sizeof(*rows));ui_cell_t (*cells)[64]=calloc(128,sizeof(*cells));char (*titles)[32]=calloc(128,sizeof(*titles)),(*values)[64][32]=calloc(128,sizeof(*values));
    if(q->kind!=UI_QUERY_ROWS||!rows||!cells||!titles||!values){free(rows);free(cells);free(titles);free(values);return;}
    fprintf(stderr,"REPRO_QUERY: first%llu count%zu col%zu+%zu dpi%u\n",(unsigned long long)q->first,q->count,q->first_column,q->column_count,q->dpi);
    size_t count=q->count;if(count>128)count=128;if(q->first>=5000)count=0;else if(count>5000-q->first)count=(size_t)(5000-q->first);
    for(size_t i=0;i<count;++i){rows[i].size=sizeof(rows[i]);rows[i].id=q->first+i+1;snprintf(titles[i],sizeof(titles[i]),"ROW_%04llu",(unsigned long long)(q->first+i));rows[i].title=titles[i];
        if(columns){rows[i].cells=cells[i];rows[i].cell_count=64;for(size_t j=0;j<64;++j){cells[i][j].size=sizeof(cells[i][j]);cells[i][j].column_id=columns[j].id;cells[i][j].kind=UI_VALUE_TEXT;snprintf(values[i][j],sizeof(values[i][j]),"VALUE_%llu_C%zu",(unsigned long long)rows[i].id,j);cells[i][j].text=values[i][j];}}}
    ui_component_batch_t b={0};b.size=sizeof(b);b.component_generation=q->component_generation;b.request_id=q->request_id;b.first=q->first;b.parent_id=q->parent_id;b.total_count=5000;b.rows=rows;b.row_count=count;
    check(ui_component_submit(c,&b)==UI_STATUS_OK,"synchronous copied data batch");free(rows);free(cells);free(titles);free(values);
}
static int present(ui_component_t *c,const char *id,ui_element_presentation_t *p)
{memset(p,0,sizeof(*p));p->size=sizeof(*p);return ui_component_get_presentation(c,id,p)==UI_STATUS_OK&&p->visible&&p->clip.width>0&&p->clip.height>0;}
static int mouse(POINT point,DWORD action)
{INPUT events[2]={0};events[0].type=events[1].type=INPUT_MOUSE;events[0].mi.dx=MulDiv(point.x-GetSystemMetrics(SM_XVIRTUALSCREEN),65535,GetSystemMetrics(SM_CXVIRTUALSCREEN)-1);events[0].mi.dy=MulDiv(point.y-GetSystemMetrics(SM_YVIRTUALSCREEN),65535,GetSystemMetrics(SM_CYVIRTUALSCREEN)-1);events[0].mi.dwFlags=MOUSEEVENTF_MOVE|MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_VIRTUALDESK|MOUSEEVENTF_MOVE_NOCOALESCE;events[1].mi.dwFlags=action;return SendInput(action?2:1,events,sizeof(INPUT))==(action?2u:1u);}
static void drag(ui_component_t *c,ui_content_slot_t *slot,int horizontal,int end)
{
    ui_element_presentation_t thumb={0},track={0};const char *axis=horizontal?"scroll-h-thumb":"scroll-v-thumb",*rail=horizontal?"scroll-h-track":"scroll-v-track";
    check(present(c,axis,&thumb)&&present(c,rail,&track),"public scrollbar geometry visible");
    HWND parent=(HWND)ui_content_slot_native_handle(slot);POINT start={thumb.clip.x+thumb.clip.width/2,thumb.clip.y+thumb.clip.height/2};ClientToScreen(parent,&start);
    HWND target=WindowFromPoint(start);DWORD owner=0;GetWindowThreadProcessId(target,&owner);check(owner==GetCurrentProcessId(),"actual pointer target owned by reproducer");
    wchar_t name[64];RECT bounds;GetClassNameW(target,name,64);GetWindowRect(target,&bounds);fprintf(stderr,"REPRO_HIT: class%ls start%d,%d client%d,%d thumb%d,%d track%d,%d\n",name,start.x,start.y,start.x-bounds.left,start.y-bounds.top,thumb.clip.x,thumb.clip.y,track.rect.x,track.rect.y);
    check(mouse(start,MOUSEEVENTF_LEFTDOWN),"actual mouse down");pump(40);check(GetCapture()==target,"native pointer capture belongs to hit window");
    POINT finish={horizontal?(end?track.rect.x+track.rect.width+100:track.rect.x-100):thumb.clip.x+thumb.clip.width/2,horizontal?thumb.clip.y+thumb.clip.height/2:(end?track.rect.y+track.rect.height+100:track.rect.y-100)};ClientToScreen(parent,&finish);
    check(mouse(finish,0),"actual mouse drag");pump(150);check(mouse(finish,MOUSEEVENTF_LEFTUP),"actual mouse release");pump(100);
    ui_component_state_t s={0};s.size=sizeof(s);ui_component_get_state(c,&s);fprintf(stderr,"REPRO_NATIVE: axis%s end%d first%llu col%zu total%llu nodes%zu bytes%zu status%d\n",axis,end,(unsigned long long)s.first,s.first_column,(unsigned long long)s.total_count,s.rendered_nodes,s.cached_bytes,s.presentation_status);
    check(s.total_count==5000&&s.rendered_nodes<=1024&&s.cached_bytes<=2u*1024u*1024u,"complete source and unchanged budgets");
    const char *item=horizontal?"cell-1-c63":end?"tree-label-5000":"tree-label-1";
    ui_element_presentation_t row={0};int visible=present(c,item,&row);fprintf(stderr,"REPRO_VISIBLE: id%s visible%d clip%d,%d,%d,%d text%s\n",item,visible,row.clip.x,row.clip.y,row.clip.width,row.clip.height,row.text_utf8);
    check(visible&&strstr(row.text_utf8,horizontal?"VALUE_1_C63":end?"ROW_4999":"ROW_0000")!=NULL,horizontal?"uncached last column visibly reached":end?"last row visibly reached":"first row visibly restored");
}
/* Public input is a diagnostic control, never an acceptance replacement. */
static void public_control(ui_component_t *c,int horizontal)
{
    ui_element_presentation_t t={0},r={0};if(!present(c,horizontal?"scroll-h-thumb":"scroll-v-thumb",&t)||!present(c,horizontal?"scroll-h-track":"scroll-v-track",&r))return;
    ui_input_event_t e={0};e.size=sizeof(e);e.pointer_button=1;e.kind=UI_INPUT_POINTER_DOWN;e.x=t.clip.x+t.clip.width/2;e.y=t.clip.y+t.clip.height/2;ui_status_t down=ui_component_dispatch_input(c,&e);pump(40);
    e.kind=UI_INPUT_POINTER_MOVE;if(horizontal)e.x=r.rect.x+r.rect.width+100;else e.y=r.rect.y+r.rect.height+100;ui_status_t move=ui_component_dispatch_input(c,&e);pump(150);e.kind=UI_INPUT_POINTER_UP;ui_status_t up=ui_component_dispatch_input(c,&e);pump(100);
    ui_component_state_t s={0};s.size=sizeof(s);ui_component_get_state(c,&s);fprintf(stderr,"REPRO_PUBLIC_CONTROL: down%d move%d up%d first%llu col%zu status%d\n",down,move,up,(unsigned long long)s.first,s.first_column,s.presentation_status);
}
int main(void)
{
    fprintf(stderr,"FRAMEWORK_IDENTITY: SDK%s API%u commit%s\n",KC_REGRESSION_SDK_VERSION,KC_REGRESSION_API,KC_REGRESSION_SDK_COMMIT);
    check(ui_framework_initialize()==UI_STATUS_OK,"initialize current shared framework");
    HWND root=CreateWindowW(L"STATIC",L"Current native scrollbar minimal reproduction",WS_OVERLAPPEDWINDOW,20,20,850,550,NULL,NULL,NULL,NULL);if(!root)return 2;
    ShowWindow(root,SW_SHOW);SetWindowPos(root,HWND_TOPMOST,20,20,0,0,SWP_NOSIZE);SetForegroundWindow(root);SetFocus(root);pump(80);
    ui_host_config_t hc={0};hc.size=sizeof(hc);hc.api_version=UI_FRAMEWORK_API_VERSION;hc.native_parent=root;ui_host_t *host=ui_host_create(&hc);if(!host){DestroyWindow(root);return 2;}
    ui_native_shell_config_t nc={0};nc.size=sizeof(nc);nc.host=host;nc.native_parent=root;nc.menu_owner=root;ui_native_shell_t *native=ui_native_shell_create_web(&nc);if(!native){ui_host_destroy(host);DestroyWindow(root);return 2;}
    check(ui_host_resize(host,800,500)==UI_STATUS_OK&&ui_native_shell_reflow(native)==UI_STATUS_OK,"mount real native content slot");
    ui_content_slot_t *slot=ui_shell_get_content_slot(ui_host_get_shell(host),NULL);ui_column_desc_t columns[64]={0};char ids[64][12];
    for(size_t i=0;i<64;++i){snprintf(ids[i],sizeof(ids[i]),"c%zu",i);columns[i].size=sizeof(columns[i]);columns[i].id=columns[i].title=ids[i];columns[i].width=180;columns[i].kind=UI_VALUE_TEXT;}
    for(int table=0;table<2;++table){ui_component_desc_t d={0};d.size=sizeof(d);d.id=table?"repro.table":"repro.tree";d.title="Synchronous public source";d.kind=table?UI_COMPONENT_TABLE:UI_COMPONENT_TREE;d.source=source;d.user_data=table?columns:NULL;if(table){d.columns=columns;d.column_count=64;}
        ui_component_t *c=NULL;check(ui_component_register(host,&d,&c)==UI_STATUS_OK&&ui_component_mount(c,slot)==UI_STATUS_OK,"register and mount public data component");if(!c)continue;pump(150);
        if(table)drag(c,slot,1,1);else{drag(c,slot,0,1);ui_element_presentation_t row={0};if(present(c,"rows",&row)){POINT p={row.clip.x+20,row.clip.y+20};ClientToScreen((HWND)ui_content_slot_native_handle(slot),&p);mouse(p,0);pump(40);INPUT wheel={0};wheel.type=INPUT_MOUSE;wheel.mi.dwFlags=MOUSEEVENTF_WHEEL;wheel.mi.mouseData=(DWORD)-WHEEL_DELTA;check(SendInput(1,&wheel,sizeof(wheel))==1,"seed independent return test through real wheel");pump(120);}ui_component_state_t s={0};s.size=sizeof(s);ui_component_get_state(c,&s);check(s.first>0,"return test starts away from first row");drag(c,slot,0,0);}public_control(c,table);check(ui_component_unregister(c)==UI_STATUS_OK,"destroy all component resources after callbacks");
        d.id=table?"control.table":"control.tree";c=NULL;check(ui_component_register(host,&d,&c)==UI_STATUS_OK&&ui_component_mount_offscreen(c,800,500,96)==UI_STATUS_OK,"public input control on same template without native focus");if(c){public_control(c,table);check(ui_component_unregister(c)==UI_STATUS_OK,"destroy diagnostic control");}}
    ui_native_shell_destroy(native);ui_host_destroy(host);DestroyWindow(root);fprintf(stderr,"REPRO_RESULT: %u failures\n",failures);return failures?1:0;
}
