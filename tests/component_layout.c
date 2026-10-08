/* Public layout/state regression; real visibility/SendInput is a separate probe. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "ui_framework/components.h"
#ifdef UI_EXPERIENCE_WEBVIEW2
#include "ui_framework/webview2.h"
#include <objbase.h>
#endif
static int failures;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"layout line%d: %s\n",__LINE__,#x);++failures;}}while(0)
static void pump(DWORD ms)
{ULONGLONG end=GetTickCount64()+ms;MSG m;do{while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}Sleep(1);}while(GetTickCount64()<end);}
static ui_status_t present(ui_component_t *c,const char *id,ui_element_presentation_t *p)
{ULONGLONG end=GetTickCount64()+10000;ui_status_t s;do{s=ui_component_get_presentation(c,id,p);if(s!=UI_STATUS_PENDING)return s;pump(1);}while(GetTickCount64()<end);return s;}
int main(int argc,char **argv)
{
    ui_host_config_t hc={0};ui_host_t *host;ui_web_backend_t *backend=NULL;HWND root;int runtime=argc>1&&!strcmp(argv[1],"--webview2");char text[4096];size_t used=0;
    CHECK(ui_framework_initialize()==UI_STATUS_OK);root=CreateWindowW(L"STATIC",L"Public layout regression",WS_OVERLAPPEDWINDOW,50,50,800,600,NULL,NULL,NULL,NULL);ShowWindow(root,SW_SHOW);
#ifdef UI_EXPERIENCE_WEBVIEW2
    if(runtime){ui_webview2_backend_config_t config={0};ui_status_t status;CHECK(SUCCEEDED(CoInitializeEx(NULL,COINIT_APARTMENTTHREADED)));config.size=sizeof(config);config.framework_components=1;backend=ui_webview2_backend_create(&config,&status);CHECK(backend&&status==UI_STATUS_OK);if(!backend)return 1;}
#else
    if(runtime)return 2;
#endif
    hc.size=sizeof(hc);hc.api_version=UI_FRAMEWORK_API_VERSION;hc.native_parent=root;host=ui_host_create(&hc);CHECK(host);if(!host)return 1;
    for(int i=1;i<=40;++i)used+=snprintf(text+used,sizeof(text)-used,"LINE %02d abcdefghijklmnopqrstuvwxyz long readable data\r\n",i);snprintf(text+used,sizeof(text)-used,"FINAL LINE 41: COMPLETE END MARKER");
    for(int dpi=96;dpi<=192;dpi+=48)for(int count=1;count<=3;count+=2)for(int width=260;width<=600;width+=340){
        ui_component_t *c;ui_component_desc_t desc={0};ui_field_desc_t fields[3]={0};ui_cell_t value={0},read={0};ui_dialog_layout_t layout={0},got={0};ui_field_layout_t field={0};ui_element_presentation_t p={0};ui_input_event_t event={0};RECT client;HWND dialog;
        CHECK(ui_host_set_dpi(host,(uint32_t)dpi)==UI_STATUS_OK);for(int i=0;i<count;++i){fields[i].size=sizeof(fields[i]);fields[i].id=i==0?"text":i==1?"other":"last";fields[i].title=i==0?"Read-only multiline":"A long field title in a narrow dialog";fields[i].help=count>1?"Long explanation remains independently scrollable and focusable; abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz":"";fields[i].kind=UI_VALUE_TEXT;fields[i].flags=i==0?UI_VALUE_MULTILINE|UI_VALUE_READONLY:0;}
        desc.size=sizeof(desc);desc.id="layout";desc.title="Public layout";desc.kind=UI_COMPONENT_DIALOG;desc.fields=fields;desc.field_count=(size_t)count;desc.web_backend=backend;CHECK(ui_component_register(host,&desc,&c)==UI_STATUS_OK);
        value.size=sizeof(value);value.kind=UI_VALUE_TEXT;value.flags=UI_VALUE_READONLY|UI_VALUE_MULTILINE;value.text=text;CHECK(ui_component_set_field(c,"text",&value)==UI_STATUS_OK);CHECK(ui_component_set_error(c,"text",count>1?"Error text remains focusable":"")==UI_STATUS_OK);
        layout.size=sizeof(layout);layout.preferred_width=width;layout.min_width=240;layout.min_height=120;layout.max_width=640;layout.max_height=240;CHECK(ui_component_set_dialog_layout(c,&layout)==UI_STATUS_OK);got.size=sizeof(got);CHECK(ui_component_get_dialog_layout(c,&got)==UI_STATUS_OK);CHECK(got.preferred_width==width&&got.preferred_height>=120&&got.preferred_height<=240);
        layout.min_width=159;CHECK(ui_component_set_dialog_layout(c,&layout)==UI_STATUS_INVALID_ARGUMENT);layout.min_width=240;layout.min_height=119;CHECK(ui_component_set_dialog_layout(c,&layout)==UI_STATUS_INVALID_ARGUMENT);layout.min_height=120;
        layout.max_width=100;CHECK(ui_component_set_dialog_layout(c,&layout)==UI_STATUS_INVALID_ARGUMENT);CHECK(ui_component_get_dialog_layout(c,&got)==UI_STATUS_OK&&got.max_width==640);layout.max_width=640;
        field.size=sizeof(field);field.id="text";field.visible_rows=6;CHECK(ui_component_set_field_layout(c,&field)==UI_STATUS_OK);field.visible_rows=101;CHECK(ui_component_set_field_layout(c,&field)==UI_STATUS_INVALID_ARGUMENT);field.visible_rows=6;field.height=28;CHECK(ui_component_set_field_layout(c,&field)==UI_STATUS_INVALID_ARGUMENT);field.height=0;CHECK(ui_component_show_dialog(c)==UI_STATUS_OK);pump(runtime?180:20);dialog=FindWindowW(L"UIFrameworkWebDialog3",L"Public layout");CHECK(dialog!=NULL);GetClientRect(dialog,&client);printf("CASE runtime=%d dpi=%d fields=%d client=%ld,%ld\n",runtime,dpi,count,client.right,client.bottom);CHECK(abs(client.right-MulDiv(width,dpi,96))<=1);
        p.size=sizeof(p);CHECK(present(c,"field-text",&p)==UI_STATUS_OK);CHECK(p.rect.height==136);CHECK(present(c,"submit",&p)==UI_STATUS_OK&&p.visible&&p.clip.height>0);CHECK(present(c,"cancel",&p)==UI_STATUS_OK&&p.visible&&p.clip.height>0);
        field.visible_rows=0;field.height=42;CHECK(ui_component_set_field_layout(c,&field)==UI_STATUS_OK);CHECK(present(c,"field-text",&p)==UI_STATUS_OK&&p.rect.height==42);event.size=sizeof(event);event.kind=UI_INPUT_TEXT;event.text_utf8="MUST NOT CHANGE READONLY";(void)ui_component_dispatch_input(c,&event);read.size=sizeof(read);CHECK(ui_component_get_field(c,"text",&read)==UI_STATUS_OK&&!strcmp(read.text,text));
        value.text="UPDATED FIRST\nUPDATED LAST: FULL CONTENT";CHECK(ui_component_set_field(c,"text",&value)==UI_STATUS_OK);CHECK(ui_component_get_field(c,"text",&read)==UI_STATUS_OK&&!strcmp(read.text,value.text));CHECK(ui_component_close_dialog(c)==UI_STATUS_OK);CHECK(IsWindowEnabled(root));CHECK(ui_component_show_dialog(c)==UI_STATUS_OK);CHECK(ui_component_get_field(c,"text",&read)==UI_STATUS_OK&&!strcmp(read.text,value.text));CHECK(ui_component_close_dialog(c)==UI_STATUS_OK);CHECK(ui_component_unregister(c)==UI_STATUS_OK);
    }
    if(!runtime){ui_host_config_t off={0};ui_host_t *owner;ui_component_desc_t desc={0};ui_component_t *c;ui_dialog_layout_t layout={0};ui_element_presentation_t area={0};int initial_width;
        off.size=sizeof(off);off.api_version=UI_FRAMEWORK_API_VERSION;owner=ui_host_create(&off);CHECK(owner);desc.size=sizeof(desc);desc.id="off.dialog";desc.title="Offscreen layout";desc.kind=UI_COMPONENT_DIALOG;CHECK(ui_component_register(owner,&desc,&c)==UI_STATUS_OK);CHECK(ui_component_mount_offscreen(c,240,180,96)==UI_STATUS_OK);
        layout.size=sizeof(layout);layout.preferred_width=300;layout.preferred_height=150;layout.min_width=240;layout.min_height=120;layout.max_width=400;layout.max_height=200;CHECK(ui_component_set_dialog_layout(c,&layout)==UI_STATUS_OK);CHECK(ui_component_show_dialog(c)==UI_STATUS_OK);area.size=sizeof(area);CHECK(ui_component_get_presentation(c,"controls",&area)==UI_STATUS_OK&&area.visible);initial_width=area.rect.width;
        layout.preferred_width=360;layout.preferred_height=160;CHECK(ui_component_set_dialog_layout(c,&layout)==UI_STATUS_OK);CHECK(ui_component_get_presentation(c,"controls",&area)==UI_STATUS_OK&&area.rect.width==initial_width+60);CHECK(ui_component_close_dialog(c)==UI_STATUS_OK);ui_host_destroy(owner);
    }
    ui_host_destroy(host);
#ifdef UI_EXPERIENCE_WEBVIEW2
    if(backend){pump(100);ui_webview2_backend_destroy(backend);CoUninitialize();}
#endif
    DestroyWindow(root);printf("Public layout: runtime=%d failures=%d. State checks only.\n",runtime,failures);return failures?1:0;
}
