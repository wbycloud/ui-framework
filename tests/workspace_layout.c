#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/ui_internal.h"
#include "ui_framework/shell.h"
static int failures;
#define CHECK(x) do { if(!(x)){fprintf(stderr,"layout line%d: %s\n",__LINE__,#x);++failures;} } while(0)
static void run(int offscreen)
{
    HWND root=offscreen?NULL:CreateWindowExW(0,L"STATIC",L"layout",WS_OVERLAPPEDWINDOW,0,0,1000,700,NULL,NULL,NULL,NULL);
    ui_host_config_t hc={0};ui_native_shell_config_t nc={0};ui_host_t *h;ui_native_shell_t *native;ui_shell_t *s;
    ui_layout_desc_t l={0};ui_panel_desc_t p={0};ui_panel_layout_t a={0},b={0};ui_rect_t preview;ui_layout_region_t region;size_t bytes=0;unsigned char *data;
    hc.size=sizeof(hc);hc.api_version=UI_FRAMEWORK_API_VERSION;hc.native_parent=root;h=ui_host_create(&hc);CHECK(h!=NULL);if(!h)return;
    h->host_rect=(ui_rect_t){0,0,1000,700};h->run_mode=offscreen?UI_RUN_OFFSCREEN:UI_RUN_WINDOWED;
    l.size=sizeof(l);l.left_sidebar_width=220;l.right_sidebar_width=260;l.left_sidebar_min_width=100;l.left_sidebar_max_width=400;l.collapsed_tag_width=28;l.main_min_width=300;
    CHECK(ui_host_set_layout(h,&l)==UI_STATUS_OK);
    p.size=sizeof(p);p.id="tools";p.title="Tools";p.entry_url="app://layout";p.kind=UI_PANEL_SIDEBAR;p.dock_region=UI_LAYOUT_REGION_LEFT_SIDEBAR;CHECK(ui_host_register_panel(h,&p)==UI_STATUS_OK);
    p.id="tree";CHECK(ui_host_register_panel(h,&p)==UI_STATUS_OK);p.id="properties";p.dock_region=UI_LAYOUT_REGION_RIGHT_SIDEBAR;CHECK(ui_host_register_panel(h,&p)==UI_STATUS_OK);
    nc.size=sizeof(nc);nc.host=h;native=offscreen?ui_native_shell_create_offscreen(&nc):ui_native_shell_create(&nc);CHECK(native!=NULL);if(!native){ui_host_destroy(h);return;}s=ui_host_get_shell(h);
    a.size=b.size=sizeof(a);CHECK(ui_shell_get_panel_layout(s,"tools",&a)==UI_STATUS_OK);a.dock_region=UI_LAYOUT_REGION_RIGHT_SIDEBAR;a.collapsed=1;a.height=180;a.order=10;
    CHECK(ui_shell_set_panel_layout(s,"tools",&a)==UI_STATUS_OK);CHECK(ui_shell_get_panel_layout(s,"tools",&b)==UI_STATUS_OK&&b.collapsed&&b.dock_region==a.dock_region);
    CHECK(ui_shell_save_layout(s,NULL,0,&bytes)==UI_STATUS_OK&&bytes>100);data=(unsigned char *)malloc(bytes);CHECK(data!=NULL);if(!data)return;
    CHECK(ui_shell_save_layout(s,data,bytes-1,&bytes)==UI_STATUS_LIMIT_EXCEEDED);CHECK(ui_shell_save_layout(s,data,bytes,&bytes)==UI_STATUS_OK);
    CHECK(ui_shell_reset_layout(s)==UI_STATUS_OK);CHECK(ui_shell_restore_layout(s,data,bytes)==UI_STATUS_OK);CHECK(ui_shell_get_panel_layout(s,"tools",&b)==UI_STATUS_OK&&b.collapsed&&b.order==10);
    data[4]=99;CHECK(ui_shell_restore_layout(s,data,bytes)==UI_STATUS_UNSUPPORTED);CHECK(ui_shell_get_panel_layout(s,"tools",&b)==UI_STATUS_OK&&b.collapsed);data[4]=2;
    CHECK(ui_shell_restore_layout(s,data,bytes-1)==UI_STATUS_INVALID_ARGUMENT);
    p.id="added";p.dock_region=UI_LAYOUT_REGION_LEFT_SIDEBAR;CHECK(ui_host_register_panel(h,&p)==UI_STATUS_OK);CHECK(ui_shell_refresh(s)==UI_STATUS_OK);CHECK(ui_shell_restore_layout(s,data,bytes)==UI_STATUS_OK);CHECK(ui_shell_get_content_slot(s,"added")!=NULL);
    {unsigned char copy[64];memcpy(copy,data+96,64);memcpy(data+96,data+208,64);CHECK(ui_shell_restore_layout(s,data,bytes)==UI_STATUS_INVALID_ARGUMENT);memcpy(data+96,copy,64);
        data[96]=0xc0;CHECK(ui_shell_restore_layout(s,data,bytes)==UI_STATUS_INVALID_ARGUMENT);memcpy(data+96,copy,64);
        memset(data+96,0,64);strcpy((char *)data+96,"missing-panel");CHECK(ui_shell_restore_layout(s,data,bytes)==UI_STATUS_OK);memcpy(data+96,copy,64);
        {size_t large_bytes=96+512*112;unsigned char *large=(unsigned char *)calloc(1,large_bytes);CHECK(large!=NULL);if(large){memcpy(large,data,96);large[8]=0;large[9]=2;large[12]=(unsigned char)large_bytes;large[13]=(unsigned char)(large_bytes>>8);large[14]=large[15]=0;
            for(int i=0;i<512;++i){memcpy(large+96+i*112,data+96,112);memset(large+96+i*112,0,64);snprintf((char *)large+96+i*112,64,"unknown-%d",i);}CHECK(ui_shell_restore_layout(s,large,large_bytes)==UI_STATUS_OK);free(large);}}}
    free(data);
    CHECK(ui_shell_reset_layout(s)==UI_STATUS_OK);CHECK(ui_shell_resize_splitter(s,UI_LAYOUT_REGION_LEFT_SIDEBAR,NULL,9999)==UI_STATUS_OK&&h->layout.left_sidebar_width<=400);
    CHECK(ui_shell_resize_splitter(s,UI_LAYOUT_REGION_LEFT_SIDEBAR,"tree",120)==UI_STATUS_OK);
    CHECK(ui_shell_begin_panel_drag(s,"tools")==UI_STATUS_OK);CHECK(ui_shell_update_panel_drag(s,995,240,&preview,&region)==UI_STATUS_OK&&region==UI_LAYOUT_REGION_RIGHT_SIDEBAR&&preview.width>0);
    CHECK(ui_shell_end_panel_drag(s,0)==UI_STATUS_OK);CHECK(ui_shell_get_panel_layout(s,"tools",&b)==UI_STATUS_OK&&b.dock_region==UI_LAYOUT_REGION_LEFT_SIDEBAR);
    if(!offscreen){
        CHECK(ui_shell_begin_panel_drag(s,"tools")==UI_STATUS_OK);SendMessageW(root,WM_MOUSEMOVE,MK_LBUTTON,MAKELPARAM(995,240));SendMessageW(root,WM_KEYDOWN,VK_ESCAPE,0);
        CHECK(ui_shell_get_panel_layout(s,"tools",&b)==UI_STATUS_OK&&b.dock_region==UI_LAYOUT_REGION_LEFT_SIDEBAR&&GetCapture()!=root);
        {MINMAXINFO limits={0};ui_panel_layout_t prior={0};prior.size=sizeof(prior);CHECK(ui_shell_get_panel_layout(s,"tools",&prior)==UI_STATUS_OK);a=prior;a.floating=1;a.tab_group_id=0;a.floating_rect=(ui_rect_t){20,20,10,10};CHECK(ui_shell_set_panel_layout(s,"tools",&a)==UI_STATUS_OK);
         CHECK(ui_shell_get_panel_layout(s,"tools",&b)==UI_STATUS_OK&&b.floating_rect.width>=160&&b.floating_rect.height>=120);
         HWND floating=(HWND)ui_content_slot_native_handle(ui_shell_get_content_slot(s,"tools"));HWND frame=GetAncestor(floating,GA_ROOT);SendMessageW(frame,WM_GETMINMAXINFO,0,(LPARAM)&limits);CHECK(limits.ptMinTrackSize.x>=160&&limits.ptMinTrackSize.y>=120);CHECK(ui_shell_set_panel_layout(s,"tools",&prior)==UI_STATUS_OK);}
        CHECK(ui_shell_begin_splitter_drag(s,UI_LAYOUT_REGION_LEFT_SIDEBAR,NULL)==UI_STATUS_OK);SendMessageW(root,WM_MOUSEMOVE,MK_LBUTTON,MAKELPARAM(130,240));CHECK(h->layout.left_sidebar_width==130);
        SendMessageW(root,WM_KEYDOWN,VK_ESCAPE,0);CHECK(h->layout.left_sidebar_width==400);
        CHECK(ui_shell_begin_splitter_drag(s,UI_LAYOUT_REGION_LEFT_SIDEBAR,NULL)==UI_STATUS_OK);SendMessageW(root,WM_MOUSEMOVE,MK_LBUTTON,MAKELPARAM(160,240));SendMessageW(root,WM_LBUTTONUP,0,MAKELPARAM(160,240));CHECK(h->layout.left_sidebar_width==160);
    }
    CHECK(ui_shell_begin_panel_drag(s,"tools")==UI_STATUS_OK);CHECK(ui_shell_update_panel_drag(s,995,240,&preview,&region)==UI_STATUS_OK);CHECK(ui_shell_end_panel_drag(s,1)==UI_STATUS_OK);
    CHECK(ui_shell_get_panel_layout(s,"tools",&b)==UI_STATUS_OK&&b.dock_region==UI_LAYOUT_REGION_RIGHT_SIDEBAR&&!b.floating);
    for(int i=0;i<520;++i){ui_status_t status;CHECK(ui_shell_begin_panel_drag(s,"tools")==UI_STATUS_OK);CHECK(ui_shell_update_panel_drag(s,995,240,&preview,&region)==UI_STATUS_OK);status=ui_shell_end_panel_drag(s,1);CHECK(status==UI_STATUS_OK);if(status!=UI_STATUS_OK)break;}
    a=b;a.floating=1;a.collapsed=0;a.floating_rect=(ui_rect_t){-30000,30000,320,240};a.dpi=144;CHECK(ui_shell_set_panel_layout(s,"tools",&a)==UI_STATUS_OK);
    CHECK(ui_host_set_dpi(h,192)==UI_STATUS_OK);CHECK(ui_shell_get_panel_layout(s,"tools",&b)==UI_STATUS_OK&&b.floating);
    if(!offscreen){RECT wr;HWND window=(HWND)ui_content_slot_native_handle(ui_shell_get_content_slot(s,"tools"));HWND popup=GetAncestor(window,GA_ROOT);CHECK(GetWindowRect(popup,&wr));CHECK(MonitorFromRect(&wr,MONITOR_DEFAULTTONULL)!=NULL);
        SetWindowPos(popup,NULL,-30000,30000,320,240,SWP_NOACTIVATE|SWP_NOZORDER);SendMessageW(root,WM_DISPLAYCHANGE,32,0);CHECK(GetWindowRect(popup,&wr)&&MonitorFromRect(&wr,MONITOR_DEFAULTTONULL)!=NULL);}
    CHECK(ui_shell_reset_layout(s)==UI_STATUS_OK);ui_native_shell_destroy(native);ui_host_destroy(h);if(root)DestroyWindow(root);
}
static void partial_fields(void)
{
    ui_host_config_t hc={0};ui_host_t *h;ui_component_t *c;ui_component_desc_t d={0};ui_component_state_t state;
    hc.size=sizeof(hc);hc.api_version=UI_FRAMEWORK_API_VERSION;h=ui_host_create(&hc);CHECK(h!=NULL);if(!h)return;
    _Static_assert(offsetof(ui_component_desc_t,sort_command)==144&&offsetof(ui_component_desc_t,selection_flags)==152,"current field offsets");
    _Static_assert(offsetof(ui_component_state_t,selected_count)==120&&sizeof(ui_component_query_t)==112,"current state/query");
    d.size=151;d.id="partial-sort";d.kind=UI_COMPONENT_LIST;d.sort_command=(const char *)(uintptr_t)1;d.selection_flags=0xffffffff;
    CHECK(ui_component_register(h,&d,&c)==UI_STATUS_OK);memset(&state,0xa5,sizeof(state));state.size=127;CHECK(ui_component_get_state(c,&state)==UI_STATUS_OK&&state.selected_count==SIZE_MAX/255*165);
    d.size=155;d.id="partial-flags";d.sort_command=NULL;CHECK(ui_component_register(h,&d,&c)==UI_STATUS_OK);
    d.size=156;d.id="bad-flags";CHECK(ui_component_register(h,&d,&c)==UI_STATUS_INVALID_ARGUMENT);ui_host_destroy(h);
}
int main(void){partial_fields();run(0);run(1);printf("workspace layout: %d failures\n",failures);return failures?1:0;}
