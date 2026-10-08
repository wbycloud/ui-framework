/* A public-C dialog; no custom HTML, private headers or application DLL. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ui_framework/components.h"
static ui_host_t *host;
static ui_component_t *dialog;
static void submit(ui_host_t *h,uint64_t request,const char *id,const char *params,const char *source,void *user)
{(void)id;(void)params;(void)source;(void)user;(void)ui_component_accept_fields(dialog);(void)ui_component_close_dialog(dialog);(void)ui_host_reply(h,request,1,"{}");}
static LRESULT CALLBACK window_proc(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    if(message==WM_KEYDOWN&&wp==VK_RETURN&&dialog){(void)ui_component_show_dialog(dialog);return 0;}
    if(message==WM_CLOSE){if(host){ui_host_destroy(host);host=NULL;dialog=NULL;}DestroyWindow(window);return 0;}
    if(message==WM_DESTROY){PostQuitMessage(0);return 0;}
    return DefWindowProcW(window,message,wp,lp);
}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE previous,PWSTR arguments,int show)
{
    WNDCLASSW wc={0};HWND window;ui_host_config_t config={0};ui_component_desc_t desc={0};ui_field_desc_t fields[2]={0};ui_command_desc_t command={0};ui_dialog_layout_t layout={0};ui_field_layout_t field_layout={0};ui_cell_t value={0};MSG message;
    (void)previous;(void)arguments;if(ui_framework_initialize()!=UI_STATUS_OK)return 1;
    wc.lpfnWndProc=window_proc;wc.hInstance=instance;wc.hCursor=LoadCursorW(NULL,MAKEINTRESOURCEW(32512));wc.hbrBackground=GetSysColorBrush(COLOR_WINDOW);wc.lpszClassName=L"UIFrameworkDialogLayoutExample";if(!RegisterClassW(&wc))return 2;
    window=CreateWindowW(wc.lpszClassName,L"Public dialog layout - Enter to reopen",WS_OVERLAPPEDWINDOW,100,100,800,600,NULL,NULL,instance,NULL);if(!window)return 3;
    config.size=sizeof(config);config.api_version=UI_FRAMEWORK_API_VERSION;config.native_parent=window;host=ui_host_create(&config);if(!host)return 4;
    command.size=sizeof(command);command.id="accept";command.title="Accept";command.handler=submit;if(ui_host_register_command(host,&command)!=UI_STATUS_OK)return 5;
    fields[0].size=sizeof(fields[0]);fields[0].id="name";fields[0].title="Name";fields[0].kind=UI_VALUE_TEXT;fields[0].flags=UI_VALUE_REQUIRED;
    fields[1].size=sizeof(fields[1]);fields[1].id="notes";fields[1].title="Notes";fields[1].kind=UI_VALUE_TEXT;fields[1].flags=UI_VALUE_MULTILINE;fields[1].help="The field scrolls internally; additional fields use the outer container.";
    desc.size=sizeof(desc);desc.id="example.dialog";desc.title="Public dialog layout";desc.kind=UI_COMPONENT_DIALOG;desc.fields=fields;desc.field_count=2;desc.commands.submit="accept";if(ui_component_register(host,&desc,&dialog)!=UI_STATUS_OK)return 6;
    layout.size=sizeof(layout);layout.preferred_width=420;layout.min_width=240;layout.min_height=120;layout.max_width=640;layout.max_height=480;if(ui_component_set_dialog_layout(dialog,&layout)!=UI_STATUS_OK)return 7;
    field_layout.size=sizeof(field_layout);field_layout.id="notes";field_layout.visible_rows=6;if(ui_component_set_field_layout(dialog,&field_layout)!=UI_STATUS_OK)return 8;
    value.size=sizeof(value);value.kind=UI_VALUE_TEXT;value.text="Example";if(ui_component_set_field(dialog,"name",&value)!=UI_STATUS_OK)return 9;value.flags=UI_VALUE_MULTILINE;value.text="First line\nSecond line\nFinal line";if(ui_component_set_field(dialog,"notes",&value)!=UI_STATUS_OK)return 10;
    ShowWindow(window,show);if(ui_component_show_dialog(dialog)!=UI_STATUS_OK)return 11;
    while(GetMessageW(&message,NULL,0,0)>0){TranslateMessage(&message);DispatchMessageW(&message);}return 0;
}
