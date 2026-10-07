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
static uint32_t test_dpi=96;
static unsigned last_column_queries;
static int defer_rows;
static ui_component_query_t deferred_query;

static unsigned failures;
static void check(int ok,const char *text){fprintf(stderr,"%s: %s\n",ok?"PASS":"FAIL",text);if(!ok)++failures;}
static void pump(unsigned ms)
{ULONGLONG end=GetTickCount64()+ms;do{MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}Sleep(1);}while(GetTickCount64()<end);}
static void source(ui_component_t *c,const ui_component_query_t *q,void *user)
{
    if(q->kind==UI_QUERY_ROWS&&defer_rows){deferred_query=*q;return;}
    ui_column_desc_t *columns=user;ui_row_t *rows=calloc(128,sizeof(*rows));ui_cell_t (*cells)[64]=calloc(128,sizeof(*cells));char (*titles)[32]=calloc(128,sizeof(*titles)),(*values)[64][32]=calloc(128,sizeof(*values));
    if(q->kind!=UI_QUERY_ROWS||!rows||!cells||!titles||!values){free(rows);free(cells);free(titles);free(values);return;}
    if(columns&&q->first_column+q->column_count==64)++last_column_queries;
    fprintf(stderr,"REPRO_QUERY: first%llu count%zu col%zu+%zu dpi%u\n",(unsigned long long)q->first,q->count,q->first_column,q->column_count,q->dpi);
    size_t count=q->count;if(count>128)count=128;if(q->first>=5000)count=0;else if(count>5000-q->first)count=(size_t)(5000-q->first);
    for(size_t i=0;i<count;++i){rows[i].size=sizeof(rows[i]);rows[i].id=q->first+i+1;snprintf(titles[i],sizeof(titles[i]),"ROW_%04llu",(unsigned long long)(q->first+i));rows[i].title=titles[i];
        if(columns){rows[i].cells=cells[i];rows[i].cell_count=q->column_count;for(size_t k=0;k<q->column_count;++k){size_t j=q->first_column+k;cells[i][k].size=sizeof(cells[i][k]);cells[i][k].column_id=columns[j].id;cells[i][k].kind=UI_VALUE_TEXT;snprintf(values[i][j],sizeof(values[i][j]),"VALUE_%llu_C%zu",(unsigned long long)rows[i].id,j);cells[i][k].text=values[i][j];}}}
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
    HWND parent=(HWND)ui_content_slot_native_handle(slot);POINT start={thumb.clip.x+thumb.clip.width/2,thumb.clip.y+thumb.clip.height/2};start.x=MulDiv(start.x,(int)test_dpi,96);start.y=MulDiv(start.y,(int)test_dpi,96);ClientToScreen(parent,&start);
    HWND target=WindowFromPoint(start);DWORD owner=0;GetWindowThreadProcessId(target,&owner);check(owner==GetCurrentProcessId(),"actual pointer target owned by reproducer");
    wchar_t name[64];RECT bounds;GetClassNameW(target,name,64);GetWindowRect(target,&bounds);fprintf(stderr,"REPRO_HIT: class%ls start%d,%d client%d,%d thumb%d,%d track%d,%d\n",name,start.x,start.y,start.x-bounds.left,start.y-bounds.top,thumb.clip.x,thumb.clip.y,track.rect.x,track.rect.y);
    check(mouse(start,MOUSEEVENTF_LEFTDOWN),"actual mouse down");pump(40);check(GetCapture()==target,"native pointer capture belongs to hit window");
    POINT finish={horizontal?(end?track.rect.x+track.rect.width+100:track.rect.x-100):thumb.clip.x+thumb.clip.width/2,horizontal?thumb.clip.y+thumb.clip.height/2:(end?track.rect.y+track.rect.height+100:track.rect.y-100)};finish.x=MulDiv(finish.x,(int)test_dpi,96);finish.y=MulDiv(finish.y,(int)test_dpi,96);ClientToScreen(parent,&finish);
    check(mouse(finish,0),"actual mouse drag");pump(150);check(mouse(finish,MOUSEEVENTF_LEFTUP),"actual mouse release");pump(100);
    ui_component_state_t s={0};s.size=sizeof(s);ui_component_get_state(c,&s);fprintf(stderr,"REPRO_NATIVE: axis%s end%d first%llu col%zu total%llu nodes%zu bytes%zu status%d\n",axis,end,(unsigned long long)s.first,s.first_column,(unsigned long long)s.total_count,s.rendered_nodes,s.cached_bytes,s.presentation_status);
    check(s.total_count==5000&&s.rendered_nodes<=1024&&s.cached_bytes<=2u*1024u*1024u,"complete source and unchanged budgets");
    const char *item=horizontal?(end?"cell-1-c63":"cell-1-c0"):end?"tree-label-5000":"tree-label-1";
    ui_element_presentation_t row={0};int visible=present(c,item,&row);fprintf(stderr,"REPRO_VISIBLE: id%s visible%d clip%d,%d,%d,%d text%s\n",item,visible,row.clip.x,row.clip.y,row.clip.width,row.clip.height,row.text_utf8);
    check(visible&&strstr(row.text_utf8,horizontal?(end?"VALUE_1_C63":"VALUE_1_C0"):end?"ROW_4999":"ROW_0000")!=NULL,horizontal?"uncached last column visibly reached":end?"last row visibly reached":"first row visibly restored");
}
static void snapshot(HWND root,const char *name)
{
    const char *directory=getenv("UI_NATIVE_SCROLL_EVIDENCE");if(!directory)return;RECT r;GetClientRect(root,&r);
    HDC dc=GetDC(root),mem=CreateCompatibleDC(dc);BITMAPINFO info={0};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=r.right;info.bmiHeader.biHeight=-r.bottom;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;void *pixels=NULL;
    HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,NULL,0);HGDIOBJ old=SelectObject(mem,bitmap);int rendered=PrintWindow(root,mem,PW_CLIENTONLY);char path[1024];snprintf(path,sizeof(path),"%s/%s-%u.bmp",directory,name,test_dpi);FILE *f=fopen(path,"wb");BITMAPFILEHEADER header={0};header.bfType=0x4d42;header.bfOffBits=sizeof(header)+sizeof(info.bmiHeader);header.bfSize=header.bfOffBits+(DWORD)(r.right*r.bottom*4);
    check(rendered&&f&&fwrite(&header,sizeof(header),1,f)==1&&fwrite(&info.bmiHeader,sizeof(info.bmiHeader),1,f)==1&&fwrite(pixels,4,(size_t)r.right*r.bottom,f)==(size_t)r.right*r.bottom,"save actual native client screenshot");if(f)fclose(f);SelectObject(mem,old);DeleteObject(bitmap);DeleteDC(mem);ReleaseDC(root,dc);
}
static void wheel(ui_component_t *c,ui_content_slot_t *slot)
{ui_element_presentation_t rows={0};check(present(c,"rows",&rows),"wheel viewport visible");POINT p={MulDiv(rows.clip.x+20,(int)test_dpi,96),MulDiv(rows.clip.y+20,(int)test_dpi,96)};ClientToScreen((HWND)ui_content_slot_native_handle(slot),&p);check(mouse(p,0),"move real pointer over rows");pump(30);INPUT e={0};e.type=INPUT_MOUSE;e.mi.dwFlags=MOUSEEVENTF_WHEEL;e.mi.mouseData=(DWORD)-WHEEL_DELTA;check(SendInput(1,&e,sizeof(e))==1,"seed return with real wheel");pump(100);ui_component_state_t s={0};s.size=sizeof(s);check(ui_component_get_state(c,&s)==UI_STATUS_OK&&s.first==3,"independent return starts at first3");}
static POINT screen_point(ui_content_slot_t *slot,int x,int y)
{POINT p={MulDiv(x,(int)test_dpi,96),MulDiv(y,(int)test_dpi,96)};ClientToScreen((HWND)ui_content_slot_native_handle(slot),&p);return p;}
static void cancel_drag(ui_component_t *c,ui_content_slot_t *slot,HWND root,int horizontal,int escape)
{ui_element_presentation_t thumb={0},track={0};check(present(c,horizontal?"scroll-h-thumb":"scroll-v-thumb",&thumb)&&present(c,horizontal?"scroll-h-track":"scroll-v-track",&track),"cancel test has visible thumb");ui_component_state_t origin={0},moved={0},restored={0};origin.size=moved.size=restored.size=sizeof(origin);ui_component_get_state(c,&origin);
 POINT start=screen_point(slot,thumb.clip.x+thumb.clip.width/2,thumb.clip.y+thumb.clip.height/2);check(mouse(start,MOUSEEVENTF_LEFTDOWN),"real cancel-test down");pump(30);POINT mid=screen_point(slot,horizontal?track.rect.x+track.rect.width/2:thumb.clip.x+thumb.clip.width/2,horizontal?thumb.clip.y+thumb.clip.height/2:track.rect.y+track.rect.height/2);check(mouse(mid,0),"real cancel-test move");pump(100);ui_component_get_state(c,&moved);check(horizontal?moved.first_column!=origin.first_column:moved.first!=origin.first,"cancel test actually moved");
 if(escape){INPUT key[2]={0};key[0].type=key[1].type=INPUT_KEYBOARD;key[0].ki.wVk=key[1].ki.wVk=VK_ESCAPE;key[1].ki.dwFlags=KEYEVENTF_KEYUP;check(SendInput(2,key,sizeof(INPUT))==2,"actual Esc cancels drag");pump(80);}else{SetCapture(root);pump(80);ReleaseCapture();}
 check(mouse(mid,MOUSEEVENTF_LEFTUP),"release cancelled drag");pump(40);ui_component_get_state(c,&restored);check(restored.first==origin.first&&restored.first_column==origin.first_column,"Esc or actual capture loss restores origin");check(GetCapture()==NULL,"cancel releases native capture");}
static void pending_geometry(ui_component_t *c,ui_content_slot_t *slot,void *data)
{ui_element_presentation_t rail={0},thumb={0},loading={0},pending={0},complete={0};check(present(c,"scroll-v-track",&rail)&&present(c,"scroll-v-thumb",&thumb),"pending test scrollbar visible");POINT start=screen_point(slot,thumb.clip.x+thumb.clip.width/2,thumb.clip.y+thumb.clip.height/2);check(mouse(start,MOUSEEVENTF_LEFTDOWN),"actual pending-test down");pump(30);defer_rows=1;POINT mid=screen_point(slot,thumb.clip.x+thumb.clip.width/2,rail.rect.y+rail.rect.height/2);check(mouse(mid,0),"actual pending-test drag");pump(50);check(ui_component_set_visible(c,1)==UI_STATUS_OK,"refresh unchanged visible pending component");check(present(c,"empty-state",&loading),"async source actually displays loading placeholder");check(present(c,"scroll-v-track",&pending)&&pending.rect.height==rail.rect.height,"loading does not shrink complete-source track");defer_rows=0;source(c,&deferred_query,data);pump(70);check(present(c,"scroll-v-track",&complete)&&complete.rect.height==pending.rect.height,"async delivery does not move track under pointer");check(mouse(mid,MOUSEEVENTF_LEFTUP),"actual pending-test up");pump(30);drag(c,slot,0,0);}
static void resize_column(ui_component_t *c,ui_content_slot_t *slot)
{ui_element_presentation_t grip={0};int width=0;check(present(c,"resize-c0",&grip),"native header resize grip visible");POINT start=screen_point(slot,grip.clip.x+grip.clip.width/2,grip.clip.y+grip.clip.height/2);HWND target=WindowFromPoint(start);DWORD pid=0;GetWindowThreadProcessId(target,&pid);check(pid==GetCurrentProcessId(),"native resize target belongs to test");check(mouse(start,MOUSEEVENTF_LEFTDOWN),"actual header resize down");pump(40);check(GetCapture()==target,"same HWND header capture");POINT end=start;end.x+=MulDiv(64,(int)test_dpi,96);check(mouse(end,0),"actual header resize move");pump(100);check(mouse(end,MOUSEEVENTF_LEFTUP),"actual header resize up");pump(80);check(ui_component_get_column_width(c,"c0",&width)==UI_STATUS_OK&&width>=243&&width<=245,"actual mouse sets logical width at program DPI");check(GetCapture()==NULL,"header resize capture released");check(ui_component_get_column_width(c,"c1",&width)==UI_STATUS_OK&&width==180,"adjacent declared width preserved");check(ui_component_reset_columns(c,"c0")==UI_STATUS_OK,"explicit single-column reset");pump(50);}
int main(void)
{
    check(ui_framework_initialize()==UI_STATUS_OK,"initialize current shared framework");HWND root=CreateWindowW(L"STATIC",L"Native common scrollbar regression",WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,20,20,900,650,NULL,NULL,NULL,NULL);if(!root)return 2;
    /* STARTUPINFO may hide the first show; require the actual visible surface. */
    ShowWindow(root,SW_SHOW);ShowWindow(root,SW_SHOW);SetWindowPos(root,HWND_TOPMOST,20,20,0,0,SWP_NOSIZE);SetForegroundWindow(root);SetFocus(root);pump(80);check(IsWindowVisible(root),"native root visible");
    ui_host_config_t hc={0};hc.size=sizeof(hc);hc.api_version=UI_FRAMEWORK_API_VERSION;hc.native_parent=root;ui_host_t *host=ui_host_create(&hc);if(!host){DestroyWindow(root);return 2;}
    ui_native_shell_config_t nc={0};nc.size=sizeof(nc);nc.host=host;nc.native_parent=root;nc.menu_owner=root;ui_native_shell_t *shell=ui_native_shell_create_web(&nc);if(!shell){ui_host_destroy(host);DestroyWindow(root);return 2;}
    ui_column_desc_t columns[64]={0};char ids[64][12];for(size_t i=0;i<64;++i){snprintf(ids[i],sizeof(ids[i]),"c%zu",i);columns[i].size=sizeof(columns[i]);columns[i].id=columns[i].title=ids[i];columns[i].width=180;columns[i].kind=UI_VALUE_TEXT;}
    for(unsigned dpi=96;dpi<=192;dpi+=48){test_dpi=dpi;SetWindowPos(root,NULL,20,20,MulDiv(850,(int)dpi,96),MulDiv(550,(int)dpi,96),SWP_NOZORDER);check(ui_host_set_dpi(host,dpi)==UI_STATUS_OK&&ui_host_resize(host,800,500)==UI_STATUS_OK&&ui_native_shell_reflow(shell)==UI_STATUS_OK,"program DPI native slot reflow");
        ui_content_slot_t *slot=ui_shell_get_content_slot(ui_host_get_shell(host),NULL);
        for(int kind=1;kind<=3;++kind){int table=kind==2;last_column_queries=0;ui_component_desc_t d={0};d.size=sizeof(d);d.id="native.scroll";d.title="Synchronous complete source";d.kind=(ui_component_kind_t)kind;d.source=source;d.user_data=table?columns:NULL;if(table){d.columns=columns;d.column_count=64;}ui_component_t *c=NULL;check(ui_component_register(host,&d,&c)==UI_STATUS_OK&&ui_component_mount(c,slot)==UI_STATUS_OK,"register native component");if(!c)continue;pump(100);snapshot(root,table?"table-initial":kind==1?"tree-initial":"list-initial");
            if(table){check(last_column_queries==0,"last column initially uncached");resize_column(c,slot);drag(c,slot,1,1);check(last_column_queries>0,"last column queried on demand");snapshot(root,"table-last");drag(c,slot,1,0);}else{wheel(c,slot);drag(c,slot,0,0);drag(c,slot,0,1);snapshot(root,kind==1?"tree-last":"list-last");drag(c,slot,0,0);}
            SetFocus(root);pump(30);SetForegroundWindow(root);check(ui_host_resize(host,740,460)==UI_STATUS_OK&&ui_native_shell_reflow(shell)==UI_STATUS_OK,"resize without component remount");pump(70);drag(c,slot,table,1);drag(c,slot,table,0);check(ui_host_resize(host,800,500)==UI_STATUS_OK&&ui_native_shell_reflow(shell)==UI_STATUS_OK,"restore viewport");if(dpi==96){/* TREE renders a loading placeholder between async pages; LIST retains its old page. */if(kind==1)pending_geometry(c,slot,d.user_data);cancel_drag(c,slot,root,table,1);cancel_drag(c,slot,root,table,0);}check(ui_component_unregister(c)==UI_STATUS_OK,"destroy callbacks and native component");check(GetCapture()==NULL,"native capture released after close");}
    }
    ui_native_shell_destroy(shell);ui_host_destroy(host);DestroyWindow(root);fprintf(stderr,"NATIVE_SCROLL_RESULT: %u failures\n",failures);return failures?1:0;
}
