#define UI_CAP_INPUT_WORKSPACE
#include "capability_inputs.c"
#include "ui_framework/application.h"
#include "ui_framework/app_data.h"
int wmain(int argc,wchar_t **argv)
{
    HWND parent;char path[32768];ui_workspace_config_t config={0};ui_workspace_t *workspace;
    ui_app_instance_info_t a={0},b={0},reopened={0};uint64_t first=0,second=0,third=0,id=0;model_t m={0},other={0},reopen_model={0};
    ui_cap_input_binding_desc_t binding,old_binding;ui_input_event_t event={0};ui_cap_result_v2_t result={0};
    if(argc!=2||!WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,argv[1],-1,path,sizeof(path),NULL,NULL))return 2;
    origin_case();shortcut_surface_case();
    parent=CreateWindowExW(0,L"STATIC",L"C2 real workspace",WS_OVERLAPPED,-16000,-16000,640,480,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(parent!=NULL);
    config.size=sizeof(config);config.native_parent=parent;workspace=ui_workspace_create(&config);CHECK(workspace!=NULL);if(!workspace)return 1;
    CHECK(ui_workspace_open(workspace,path,&first)==UI_STATUS_OK);CHECK(ui_workspace_open(workspace,path,&second)==UI_STATUS_OK);
    if(!first||!second){fprintf(stderr,"Workspace fixture setup: %s\n",ui_workspace_last_error(workspace));ui_workspace_destroy(workspace);DestroyWindow(parent);return 1;}
    a.size=sizeof(a);b.size=sizeof(b);CHECK(ui_workspace_get_instance(workspace,first,&a)==UI_STATUS_OK);CHECK(ui_workspace_get_instance(workspace,second,&b)==UI_STATUS_OK);
    m.host=a.host;m.async=1;other.host=b.host;register_model(&m);register_model(&other);
    binding=descriptor(&m,UI_CAP_INPUT_SHORTCUT);old_binding=binding;CHECK(binding.resource.instance_id==first);
    CHECK(ui_capability_input_bind(a.host,&binding,&id)==UI_STATUS_OK);event.size=sizeof(event);event.kind=UI_INPUT_KEY_DOWN;event.key_code='K';event.modifiers=1;

#ifdef UI_CAP_INPUT_LIGHT
    {ui_host_config_t chrome_config={0};ui_host_t *chrome;ui_light_web_config_t light={0};ui_web_backend_t *backend;ui_web_view_t *view;
     ui_toolbar_desc_t bar={0};ui_toolbar_item_desc_t tool={0};ui_cap_input_binding_desc_t tool_binding;uint64_t tool_id;ui_rect_t rect={0};ui_input_event_t click={0};
     chrome_config.size=sizeof(chrome_config);chrome_config.api_version=10;chrome=ui_host_create(&chrome_config);light.size=sizeof(light);backend=ui_light_web_backend_create(&light);view=ui_web_view_create(chrome,backend);
     CHECK(ui_web_view_resize(view,180,120,96)==UI_STATUS_OK);CHECK(ui_web_view_load_html(view,"<html><body><button id='tool-tool'>Tool</button></body></html>")==UI_STATUS_OK);
     CHECK(ui_capability_workspace_input_bind(workspace,view)==UI_STATUS_OK);CHECK(view->semantic_action(view,"unrelated-framework-button",workspace)==UI_STATUS_UNSUPPORTED);CHECK(ui_capability_workspace_input_bind(workspace,view)==UI_STATUS_ALREADY_EXISTS);
     bar.size=sizeof(bar);bar.id="bar";bar.title="Bar";bar.visible=1;CHECK(ui_host_register_toolbar(a.host,&bar)==UI_STATUS_OK);
     tool.size=sizeof(tool);tool.id="tool";tool.toolbar_id="bar";tool.title="Tool";tool.command_id="change";CHECK(ui_host_register_toolbar_item(a.host,&tool)==UI_STATUS_OK);
     tool_binding=descriptor(&m,UI_CAP_INPUT_TOOLBAR);tool_binding.control_id="tool";CHECK(ui_capability_input_bind(a.host,&tool_binding,&tool_id)==UI_STATUS_OK);
     view->input_instance=first;CHECK(ui_web_view_get_element_rect(view,"tool-tool",&rect)==UI_STATUS_OK);click.size=sizeof(click);click.pointer_button=1;click.x=rect.x+2;click.y=rect.y+2;
     CHECK(ui_workspace_activate(workspace,first)==UI_STATUS_OK);click.kind=UI_INPUT_POINTER_DOWN;(void)ui_web_view_dispatch_input(view,&click);click.kind=UI_INPUT_POINTER_UP;(void)ui_web_view_dispatch_input(view,&click);CHECK(m.runs==0);
     click.kind=UI_INPUT_POINTER_DOWN;(void)ui_inputs_fixture_light(view,&click,1);click.kind=UI_INPUT_POINTER_UP;(void)ui_inputs_fixture_light(view,&click,1);CHECK(m.runs==1);
     CHECK(ui_workspace_activate(workspace,second)==UI_STATUS_OK);complete(&m);CHECK(m.value==1&&other.value==0);forget(&m);
     click.kind=UI_INPUT_POINTER_DOWN;(void)ui_inputs_fixture_light(view,&click,1);click.kind=UI_INPUT_POINTER_UP;(void)ui_inputs_fixture_light(view,&click,1);CHECK(m.runs==1&&other.runs==0);
     CHECK(ui_capability_workspace_input_unbind(view)==UI_STATUS_OK);ui_web_view_destroy(view);ui_light_web_backend_destroy(backend);ui_host_destroy(chrome);unbind(&m,tool_id);
     m.runs=m.value=m.rev=0;}
#endif
    CHECK(ui_workspace_activate(workspace,first)==UI_STATUS_OK);ui_capabilities_human_begin(a.host,&m);
    CHECK(ui_inputs_route(a.host,UI_CAP_INPUT_SHORTCUT,NULL,"",&event,&m));ui_capabilities_human_end(a.host);CHECK(m.runs==1&&m.value==0);
    CHECK(ui_workspace_activate(workspace,second)==UI_STATUS_OK);complete(&m);
    result.size=sizeof(result);result.version=2;CHECK(ui_capability_get_result_v2(a.host,m.call,&result)==UI_STATUS_OK&&result.outcome==UI_CAP2_OK);
    CHECK(m.value==1&&m.rev==1&&other.value==0&&other.rev==0);forget(&m);
    ui_capabilities_human_begin(a.host,&m);CHECK(ui_inputs_route(a.host,UI_CAP_INPUT_SHORTCUT,NULL,"",&event,&m));ui_capabilities_human_end(a.host);CHECK(m.runs==1&&other.runs==0);
    CHECK(ui_workspace_activate(workspace,first)==UI_STATUS_OK);ui_capabilities_human_begin(a.host,&m);CHECK(ui_inputs_route(a.host,UI_CAP_INPUT_SHORTCUT,NULL,"",&event,&m));ui_capabilities_human_end(a.host);
    CHECK(m.runs==2);CHECK(ui_workspace_close(workspace,first,UI_APP_CLOSE_TAB)==UI_STATUS_OK);ui_workspace_poll(workspace);CHECK(ui_workspace_count(workspace)==2);
    complete(&m);CHECK(ui_capability_get_result_v2(a.host,m.call,&result)==UI_STATUS_OK&&result.outcome==UI_CAP2_OK);ui_workspace_poll(workspace);CHECK(ui_workspace_count(workspace)==1);
    CHECK(ui_workspace_post_app_data(workspace,first,1,&m.call,sizeof(m.call))==UI_STATUS_CANCELLED);
    CHECK(ui_workspace_open(workspace,path,&third)==UI_STATUS_OK&&third!=first);reopened.size=sizeof(reopened);CHECK(ui_workspace_get_instance(workspace,third,&reopened)==UI_STATUS_OK);
    reopen_model.host=reopened.host;register_model(&reopen_model);old_binding.resource.instance_id=first;CHECK(ui_capability_input_bind(reopened.host,&old_binding,&id)==UI_STATUS_NOT_FOUND);
    CHECK(ui_workspace_close(workspace,second,UI_APP_CLOSE_TAB)==UI_STATUS_OK);CHECK(ui_workspace_close(workspace,third,UI_APP_CLOSE_TAB)==UI_STATUS_OK);ui_workspace_poll(workspace);
    CHECK(ui_workspace_count(workspace)==0);ui_workspace_destroy(workspace);DestroyWindow(parent);
    printf("C2 real workspace checks=%u failures=%u\n",checks,failures);return failures?1:0;
}
