#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/ui_internal.h"
#include "ui_framework/shell.h"
static int failures;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"workspace7 line%d: %s\n",__LINE__,#x);++failures;}}while(0)
static void run(int offscreen,int web){
 HWND root=offscreen?NULL:CreateWindowW(L"STATIC",L"Bottom dock and tabs",WS_OVERLAPPEDWINDOW,0,0,1000,700,NULL,NULL,NULL,NULL);
 ui_host_config_t hc={0};ui_host_t *h;ui_native_shell_config_t nc={0};ui_native_shell_t *native;ui_shell_t *s;ui_panel_desc_t p={0};ui_layout_desc_t l={0};ui_panel_layout_t a={0},b={0};ui_rect_t bottom,main,frame;int visible;ui_content_slot_t *saved;size_t bytes=0;unsigned char *data;
 hc.size=sizeof(hc);hc.api_version=UI_FRAMEWORK_API_VERSION;hc.native_parent=root;h=ui_host_create(&hc);CHECK(h!=NULL);if(!h)return;h->run_mode=offscreen?UI_RUN_OFFSCREEN:UI_RUN_WINDOWED;
 l.size=sizeof(l);l.left_sidebar_width=200;l.right_sidebar_width=200;l.main_min_width=300;l.collapsed_tag_width=28;CHECK(ui_host_set_layout(h,&l)==UI_STATUS_OK);CHECK(ui_host_resize(h,1000,700)==UI_STATUS_OK);
 p.size=sizeof(p);p.title="Panel";p.entry_url="";p.kind=UI_PANEL_SIDEBAR;p.dock_region=UI_LAYOUT_REGION_LEFT_SIDEBAR;p.id="a";CHECK(ui_host_register_panel(h,&p)==UI_STATUS_OK);p.id="b";CHECK(ui_host_register_panel(h,&p)==UI_STATUS_OK);p.id="c";CHECK(ui_host_register_panel(h,&p)==UI_STATUS_OK);
 nc.size=sizeof(nc);nc.host=h;native=offscreen?ui_native_shell_create_offscreen(&nc):web?ui_native_shell_create_web(&nc):ui_native_shell_create(&nc);CHECK(native!=NULL);if(!native){ui_host_destroy(h);return;}s=ui_host_get_shell(h);saved=ui_shell_get_content_slot(s,"a");a.size=b.size=sizeof(a);
 CHECK(ui_shell_get_panel_layout(s,"a",&a)==UI_STATUS_OK);a.dock_region=(ui_layout_region_t)9;CHECK(ui_shell_set_panel_layout(s,"a",&a)==UI_STATUS_OK);CHECK(ui_host_get_rect(h,(ui_layout_region_t)9,&bottom)==UI_STATUS_OK&&bottom.width>0&&bottom.height>0);CHECK(ui_host_get_rect(h,UI_LAYOUT_REGION_MAIN,&main)==UI_STATUS_OK&&main.y+main.height<=bottom.y);
 CHECK(ui_shell_get_slot_state(s,"a",&frame,NULL,&visible,NULL)==UI_STATUS_OK&&visible&&frame.y==bottom.y);CHECK(saved==ui_shell_get_content_slot(s,"a"));

 CHECK(ui_shell_set_bottom_height(s,180)==UI_STATUS_OK);CHECK(ui_host_get_rect(h,UI_LAYOUT_REGION_BOTTOM,&bottom)==UI_STATUS_OK&&bottom.height==180);
 CHECK(ui_shell_dock_panel_tab(s,"b","a")==UI_STATUS_OK);CHECK(ui_shell_get_panel_layout(s,"a",&a)==UI_STATUS_OK);CHECK(ui_shell_get_panel_layout(s,"b",&b)==UI_STATUS_OK&&a.tab_group_id&&a.tab_group_id==b.tab_group_id&&b.tab_active&&!a.tab_active);
 CHECK(ui_shell_get_slot_state(s,"a",NULL,NULL,&visible,NULL)==UI_STATUS_OK&&!visible);CHECK(ui_shell_get_slot_state(s,"b",NULL,NULL,&visible,NULL)==UI_STATUS_OK&&visible);
 CHECK(ui_shell_activate_panel(s,"a")==UI_STATUS_OK);CHECK(ui_shell_get_slot_state(s,"a",NULL,NULL,&visible,NULL)==UI_STATUS_OK&&visible);CHECK(saved==ui_shell_get_content_slot(s,"a"));
 CHECK(ui_shell_get_panel_layout(s,"a",&a)==UI_STATUS_OK);a.closed=1;CHECK(ui_shell_set_panel_layout(s,"a",&a)==UI_STATUS_OK);CHECK(ui_shell_get_slot_state(s,"b",NULL,NULL,&visible,NULL)==UI_STATUS_OK&&visible);CHECK(ui_shell_activate_panel(s,"a")==UI_STATUS_OK);
 CHECK(ui_shell_save_layout(s,NULL,0,&bytes)==UI_STATUS_OK&&bytes==96+3*112);data=malloc(bytes);CHECK(data!=NULL);if(data){
 CHECK(ui_shell_save_layout(s,data,bytes,&bytes)==UI_STATUS_OK&&data[4]==2);CHECK(ui_shell_reset_layout(s)==UI_STATUS_OK);CHECK(ui_shell_restore_layout(s,data,bytes)==UI_STATUS_OK);CHECK(ui_shell_get_panel_layout(s,"a",&a)==UI_STATUS_OK&&a.tab_active&&a.tab_group_id);
 data[4]=3;CHECK(ui_shell_restore_layout(s,data,bytes)==UI_STATUS_UNSUPPORTED);data[4]=2;data[84]=1;CHECK(ui_shell_restore_layout(s,data,bytes)==UI_STATUS_INVALID_ARGUMENT);data[84]=0;
 CHECK(ui_shell_get_panel_layout(s,"a",&b)==UI_STATUS_OK&&b.tab_group_id==a.tab_group_id);free(data);}
 {unsigned char partial[64];memset(partial,0xa5,sizeof(partial));*(uint32_t *)partial=55;CHECK(ui_shell_get_panel_layout(s,"a",(ui_panel_layout_t *)partial)==UI_STATUS_OK);CHECK(partial[48]==0xa5&&partial[55]==0xa5);*(uint32_t *)partial=59;CHECK(ui_shell_get_panel_layout(s,"a",(ui_panel_layout_t *)partial)==UI_STATUS_OK);CHECK(partial[56]==0xa5);}
 {ui_layout_region_t region;ui_rect_t preview;CHECK(ui_shell_begin_panel_drag(s,"c")==UI_STATUS_OK);CHECK(ui_shell_update_panel_drag(s,500,720,&preview,&region)==UI_STATUS_OK&&region==UI_LAYOUT_REGION_NONE);CHECK(ui_shell_end_panel_drag(s,0)==UI_STATUS_OK);CHECK(ui_shell_begin_panel_drag(s,"c")==UI_STATUS_OK);CHECK(ui_shell_update_panel_drag(s,bottom.x+30,bottom.y+10,&preview,&region)==UI_STATUS_OK&&region==UI_LAYOUT_REGION_BOTTOM);CHECK(ui_shell_end_panel_drag(s,0)==UI_STATUS_OK);CHECK(ui_shell_get_panel_layout(s,"c",&b)==UI_STATUS_OK&&!b.tab_group_id);
 CHECK(ui_shell_begin_panel_drag(s,"c")==UI_STATUS_OK);CHECK(ui_shell_update_panel_drag(s,bottom.x+30,bottom.y+10,&preview,&region)==UI_STATUS_OK);CHECK(ui_shell_end_panel_drag(s,1)==UI_STATUS_OK);CHECK(ui_shell_get_panel_layout(s,"c",&b)==UI_STATUS_OK&&b.tab_group_id==a.tab_group_id);}
 CHECK(ui_host_resize(h,240,160)==UI_STATUS_OK);CHECK(ui_host_set_dpi(h,192)==UI_STATUS_OK);CHECK(ui_host_get_rect(h,UI_LAYOUT_REGION_BOTTOM,&bottom)==UI_STATUS_OK&&bottom.height>0);CHECK(ui_shell_activate_panel(s,"a")==UI_STATUS_OK);CHECK(saved==ui_shell_get_content_slot(s,"a"));
 CHECK(ui_shell_reset_layout(s)==UI_STATUS_OK);CHECK(ui_host_get_rect(h,UI_LAYOUT_REGION_BOTTOM,&bottom)==UI_STATUS_OK&&bottom.height==0);
 ui_native_shell_destroy(native);ui_host_destroy(h);if(root)DestroyWindow(root);
}
static void registered_bottom(int offscreen,int web){
 HWND root=offscreen?NULL:CreateWindowW(L"STATIC",L"Registered bottom default",WS_OVERLAPPEDWINDOW,0,0,1000,700,NULL,NULL,NULL,NULL);ui_host_config_t hc={0};ui_native_shell_config_t nc={0};ui_panel_desc_t p={0};ui_panel_layout_t layout={0};ui_rect_t r={0};ui_native_shell_t *native;ui_shell_t *shell;ui_host_t *h;ui_status_t status;
 hc.size=sizeof(hc);hc.api_version=UI_FRAMEWORK_API_VERSION;hc.native_parent=root;h=ui_host_create(&hc);CHECK(h!=NULL);if(!h){if(root)DestroyWindow(root);return;}h->run_mode=offscreen?UI_RUN_OFFSCREEN:UI_RUN_WINDOWED;CHECK(ui_host_resize(h,1000,700)==UI_STATUS_OK);
 p.size=sizeof(p);p.id="default-bottom";p.title="Bottom";p.entry_url="";p.kind=UI_PANEL_SIDEBAR;p.dock_region=UI_LAYOUT_REGION_BOTTOM;status=ui_host_register_panel(h,&p);CHECK(status==UI_STATUS_OK);if(status!=UI_STATUS_OK){ui_host_destroy(h);if(root)DestroyWindow(root);return;}
 nc.size=sizeof(nc);nc.host=h;native=offscreen?ui_native_shell_create_offscreen(&nc):web?ui_native_shell_create_web(&nc):ui_native_shell_create(&nc);CHECK(native!=NULL);if(!native){ui_host_destroy(h);if(root)DestroyWindow(root);return;}shell=ui_host_get_shell(h);layout.size=sizeof(layout);
 CHECK(ui_host_get_rect(h,UI_LAYOUT_REGION_BOTTOM,&r)==UI_STATUS_OK&&r.height==220);CHECK(ui_shell_get_panel_layout(shell,p.id,&layout)==UI_STATUS_OK&&layout.dock_region==UI_LAYOUT_REGION_BOTTOM);
 layout.dock_region=UI_LAYOUT_REGION_LEFT_SIDEBAR;CHECK(ui_shell_set_panel_layout(shell,p.id,&layout)==UI_STATUS_OK);CHECK(ui_host_get_rect(h,UI_LAYOUT_REGION_BOTTOM,&r)==UI_STATUS_OK&&r.height==0);CHECK(ui_shell_reset_layout(shell)==UI_STATUS_OK);CHECK(ui_shell_get_panel_layout(shell,p.id,&layout)==UI_STATUS_OK&&layout.dock_region==UI_LAYOUT_REGION_BOTTOM);CHECK(ui_host_get_rect(h,UI_LAYOUT_REGION_BOTTOM,&r)==UI_STATUS_OK&&r.height==220);
 ui_native_shell_destroy(native);ui_host_destroy(h);if(root)DestroyWindow(root);
}
int main(void){CHECK(ui_framework_initialize()==UI_STATUS_OK);run(1,0);run(0,0);registered_bottom(1,0);registered_bottom(0,0);
#ifdef UI_WORKSPACE7_WEB
run(0,1);registered_bottom(0,1);
#endif
printf("API7 bottom/tabs: %d failures\n",failures);return failures?1:0;}
