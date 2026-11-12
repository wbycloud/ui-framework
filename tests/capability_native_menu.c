#define UI_CAP_INPUT_WORKSPACE
#include "capability_inputs.c"
#include "ui_framework/native.h"
void ui_native_menu_fixture(ui_native_shell_t *,int,void *,unsigned,int);
static ui_status_t menu_collect(const ui_cap_ui_context_t *c,char *p,size_t capacity,size_t *length,void *data)
{(void)c;(void)data;if(capacity<3)return UI_STATUS_LIMIT_EXCEEDED;memcpy(p,"{}",3);*length=2;return UI_STATUS_OK;}
int main(void)
{
    model_t m={0};ui_menu_item_desc_t item={0};ui_cap_ui_binding_desc_t d={0};ui_native_shell_config_t config={0};ui_native_shell_t *shell;
    HWND parent;HMENU root,menu,wrong;MENUINFO info={0};uint64_t binding;LRESULT result=0;UINT command;
    init(&m);parent=CreateWindowExW(0,L"STATIC",L"Native menu fixture",WS_OVERLAPPEDWINDOW,-16000,-16000,400,300,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(parent!=NULL);m.host->native_parent=parent;
    item.size=sizeof(item);item.id="entry";item.title="&Change";item.menu_path="Actions";item.command_id="change";CHECK(ui_host_register_menu_item(m.host,&item)==UI_STATUS_OK);
    d.size=sizeof(d);d.version=1;d.capability_id="change";d.resource.size=sizeof(d.resource);d.resource.version=2;
    CHECK(ui_capability_resource_ref_v2(m.host,"document",&d.resource)==UI_STATUS_OK);d.menu_item_id="entry";d.purpose=UI_CAP_UI_INVOKE;d.collect=menu_collect;d.observe=observe;d.user_data=&m;
    CHECK(ui_capability_ui_bind(m.host,&d,&binding)==UI_STATUS_OK);
    config.size=sizeof(config);config.host=m.host;config.native_parent=parent;shell=ui_native_shell_create(&config);CHECK(shell!=NULL);if(!shell){DestroyWindow(parent);ui_host_destroy(m.host);return 1;}
    root=GetMenu(parent);menu=GetSubMenu(root,0);CHECK(root&&menu);info.cbSize=sizeof(info);info.fMask=MIM_STYLE;CHECK(GetMenuInfo(menu,&info)&&(info.dwStyle&MNS_NOTIFYBYPOS));command=GetMenuItemID(menu,0);
    SendMessageW(parent,WM_MENUCOMMAND,0,(LPARAM)menu);CHECK(m.runs==0);
    CHECK(ui_native_shell_handle_message(shell,parent,WM_COMMAND,command,0,&result)==UI_STATUS_OK&&m.runs==0);
    ui_native_menu_fixture(shell,0,menu,0,0);ui_native_menu_fixture(shell,1,menu,0,0);SendMessageW(parent,WM_MENUCOMMAND,0,(LPARAM)menu);CHECK(m.runs==0);
    ui_native_menu_fixture(shell,1,menu,0,1);wrong=CreatePopupMenu();AppendMenuW(wrong,MF_STRING,command,L"Wrong");SendMessageW(parent,WM_MENUCOMMAND,0,(LPARAM)wrong);CHECK(m.runs==0);DestroyMenu(wrong);
    SendMessageW(parent,WM_MENUCOMMAND,0,(LPARAM)menu);CHECK(m.runs==1&&m.value==1);SendMessageW(parent,WM_MENUCOMMAND,0,(LPARAM)menu);CHECK(m.runs==1);
    CHECK(ui_capability_forget_v2(m.host,m.call)==UI_STATUS_PENDING);ui_shell_refresh_titles(m.host);CHECK(GetMenu(parent)==root&&IsMenu(menu));
    ui_native_menu_fixture(shell,2,menu,0,0);forget(&m);
    ui_native_menu_fixture(shell,0,menu,0,0);ui_native_menu_fixture(shell,1,menu,0,1);ui_native_menu_fixture(shell,1,menu,0,0);SendMessageW(parent,WM_MENUCOMMAND,0,(LPARAM)menu);CHECK(m.runs==1);
    ui_native_menu_fixture(shell,1,menu,0,1);ui_native_menu_fixture(shell,2,menu,0,0);SendMessageW(parent,WM_MENUCOMMAND,0,(LPARAM)menu);CHECK(m.runs==1);
    ui_native_menu_fixture(shell,0,menu,0,0);ui_native_menu_fixture(shell,1,menu,0,1);m.modal=1;SendMessageW(parent,WM_MENUCOMMAND,0,(LPARAM)menu);CHECK(m.runs==1);m.modal=0;m.host->modal_component=NULL;ui_native_menu_fixture(shell,2,menu,0,0);forget(&m);
    unbind(&m,binding);ui_native_shell_destroy(shell);DestroyWindow(parent);ui_host_destroy(m.host);
    printf("HMENU logic fixture (not physical): checks=%u failures=%u\n",checks,failures);return failures?1:0;
}
