#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "ui_framework/components.h"
#include "ui_framework/menus.h"
#include "../src/ui_internal.h"
#ifdef UI_EXPERIENCE_WEBVIEW2
#include "ui_framework/webview2.h"
#include <objbase.h>
#endif
static int runtime_mode;
static void pump_for(DWORD ms)
{ULONGLONG start=GetTickCount64();do{MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}Sleep(1);}while(GetTickCount64()-start<ms);}
static ui_status_t presentation(ui_component_t *c,const char *id,ui_element_presentation_t *p)
{ui_status_t status;ULONGLONG start=GetTickCount64();do{status=ui_component_get_presentation(c,id,p);if(status!=UI_STATUS_PENDING)return status;pump_for(1);}while(GetTickCount64()-start<10000);return status;}
static ui_status_t dispatch(ui_component_t *c,const ui_input_event_t *e)
{ui_status_t status=ui_component_dispatch_input(c,e);if(runtime_mode)pump_for(80);return status;}
static int failures,sorts,edit_count,submits;static ui_component_query_t last;static int delayed;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"experience line%d: %s\n",__LINE__,#x);++failures;}}while(0)
static void command(ui_host_t *h,uint64_t request,const char *id,const char *params,const char *origin,void *user)
{(void)origin;(void)user;if(!strcmp(id,"edit")){++edit_count;CHECK(strstr(params,"changed")!=NULL);}else if(!strcmp(id,"submit"))++submits;else ++sorts;ui_host_reply(h,request,1,"{}");}
static void source(ui_component_t *c,const ui_component_query_t *q,void *user)
{
    ui_component_batch_t b={0};ui_row_t rows[128]={0};ui_cell_t cells[128]={0};uint64_t ids[512];size_t i,n=q->count;(void)user;last=*q;
    if(q->kind==UI_QUERY_SELECTION){if(delayed)return;for(i=0;i<n;++i)ids[i]=9007199254741000ULL+(q->sort_direction==-1?99999-q->first-i:q->first+i);
        CHECK(ui_component_submit_selection(c,q->component_generation,q->request_id,ids,n)==UI_STATUS_OK);return;}
    if(q->kind!=UI_QUERY_ROWS)return;if(n>128)n=128;if(q->first+n>100000)n=(size_t)(100000-q->first);
    for(i=0;i<n;++i){rows[i].size=sizeof(rows[i]);rows[i].id=9007199254741000ULL+(q->sort_direction==-1?99999-q->first-i:q->first+i);rows[i].title="row";rows[i].content_version=1;
        cells[i].size=sizeof(cells[i]);cells[i].column_id="name";cells[i].kind=UI_VALUE_TEXT;cells[i].text="editable";rows[i].cells=&cells[i];rows[i].cell_count=1;}
    b.size=sizeof(b);b.component_generation=q->component_generation;b.request_id=q->request_id;b.first=q->first;b.total_count=100000;b.rows=rows;b.row_count=n;
    CHECK(ui_component_submit(c,&b)==UI_STATUS_OK);
}
static int click_mod(ui_component_t *c,const char *id,unsigned modifiers)
{
    ui_element_presentation_t p={0};ui_input_event_t e={0};p.size=sizeof(p);if(presentation(c,id,&p)!=UI_STATUS_OK||!p.visible)return 0;
    e.size=sizeof(e);e.kind=UI_INPUT_POINTER_DOWN;e.pointer_button=1;e.modifiers=modifiers;e.x=p.clip.x+p.clip.width/2;e.y=p.clip.y+p.clip.height/2;if(dispatch(c,&e)!=UI_STATUS_OK)return 0;
    e.kind=UI_INPUT_POINTER_UP;return dispatch(c,&e)==UI_STATUS_OK;
}
static int click(ui_component_t *c,const char *id){return click_mod(c,id,0);}
int main(int argc,char **argv)
{
    ui_host_config_t hc={0};ui_host_t *h;ui_component_t *table,*form;ui_component_desc_t d={0};ui_column_desc_t col={0};ui_field_desc_t f[2]={0};
    ui_command_desc_t cmd={0};ui_component_state_t s={0};uint64_t ids[512],late[2]={4,5};size_t n;ui_component_query_t saved;const char *options[]={"low","medium","high"};ui_cell_t value={0};
    ui_native_shell_t *native=NULL;ui_web_backend_t *backend=NULL;HWND root=NULL;
    runtime_mode=argc==2&&!strcmp(argv[1],"--webview2");
#ifdef UI_EXPERIENCE_WEBVIEW2
    if(runtime_mode){ui_webview2_backend_config_t config={0};ui_status_t status;CHECK(SUCCEEDED(CoInitializeEx(NULL,COINIT_APARTMENTTHREADED)));CHECK(ui_webview2_runtime_status()==UI_STATUS_OK);
        config.size=sizeof(config);config.framework_components=1;backend=ui_webview2_backend_create(&config,&status);CHECK(backend!=NULL&&status==UI_STATUS_OK);if(!backend)return 1;
        root=CreateWindowExW(0,L"STATIC",L"API6 Runtime experience",WS_OVERLAPPEDWINDOW,100,100,1220,940,NULL,NULL,NULL,NULL);CHECK(root!=NULL);ShowWindow(root,SW_SHOWNOACTIVATE);hc.native_parent=root;}
#else
    if(runtime_mode)return 2;
#endif
    hc.size=sizeof(hc);hc.api_version=UI_FRAMEWORK_API_VERSION;h=ui_host_create(&hc);CHECK(h!=NULL);if(!h)return 1;
    if(runtime_mode){ui_layout_desc_t layout={0};ui_panel_desc_t panel={0};ui_native_shell_config_t config={0};layout.size=sizeof(layout);layout.right_sidebar_width=500;CHECK(ui_host_set_layout(h,&layout)==UI_STATUS_OK);CHECK(ui_host_resize(h,1200,900)==UI_STATUS_OK);
        panel.size=sizeof(panel);panel.id="form";panel.title="Form";panel.entry_url="app://form";panel.kind=UI_PANEL_SIDEBAR;CHECK(ui_host_register_panel(h,&panel)==UI_STATUS_OK);config.size=sizeof(config);config.host=h;native=ui_native_shell_create(&config);CHECK(native!=NULL);}
    cmd.size=sizeof(cmd);cmd.id="sort";cmd.title="Sort";cmd.handler=command;CHECK(ui_host_register_command(h,&cmd)==UI_STATUS_OK);
    cmd.id="edit";CHECK(ui_host_register_command(h,&cmd)==UI_STATUS_OK);
    cmd.id="submit";CHECK(ui_host_register_command(h,&cmd)==UI_STATUS_OK);
    col.size=sizeof(col);col.id="name";col.title="Name";col.kind=UI_VALUE_TEXT;col.width=180;
    d.size=sizeof(d);d.id="table";d.kind=UI_COMPONENT_TABLE;d.columns=&col;d.column_count=1;d.source=source;d.sort_command="sort";d.selection_flags=UI_SELECTION_MULTIPLE;d.commands.edit="edit";
    d.web_backend=backend;CHECK(ui_component_register(h,&d,&table)==UI_STATUS_OK);CHECK((runtime_mode?ui_component_mount(table,ui_shell_get_content_slot(ui_host_get_shell(h),NULL)):ui_component_mount_offscreen(table,700,450,96))==UI_STATUS_OK);
    CHECK(ui_component_select_range(table,90000,5,0)==UI_STATUS_OK);CHECK(ui_component_get_selection(table,ids,512,&n)==UI_STATUS_OK&&n==5&&ids[0]==9007199254831000ULL);
    delayed=1;CHECK(ui_component_select_range(table,20,2,0)==UI_STATUS_OK);saved=last;
    CHECK(ui_component_set_sort(table,"name",-1)==UI_STATUS_OK&&sorts==1);CHECK(ui_component_submit_selection(table,saved.component_generation,saved.request_id,late,2)==UI_STATUS_CANCELLED);delayed=0;
    CHECK(ui_component_select_range(table,0,3,0)==UI_STATUS_OK);CHECK(ui_component_get_selection(table,ids,512,&n)==UI_STATUS_OK&&n==3&&ids[0]==9007199254840999ULL);
    CHECK(ui_component_remove_rows(table,&ids[0],1)==UI_STATUS_OK);CHECK(ui_component_get_selection(table,NULL,0,&n)==UI_STATUS_OK&&n==2);
    CHECK(ui_component_select_range(table,0,513,0)==UI_STATUS_LIMIT_EXCEEDED);s.size=sizeof(s);CHECK(ui_component_get_state(table,&s)==UI_STATUS_OK&&s.sort_direction==-1&&s.cached_bytes<=2097152&&s.rendered_nodes<=1024);
    f[0].size=f[1].size=sizeof(f[0]);f[0].id="mode";f[0].title="Mode";f[0].kind=UI_VALUE_ENUM;f[0].options=options;f[0].option_count=3;f[1].id="color";f[1].title="Color";f[1].kind=UI_VALUE_COLOR;
    memset(&d,0,sizeof(d));d.size=sizeof(d);d.id="form";d.kind=UI_COMPONENT_FORM;d.fields=f;d.field_count=2;d.commands.submit="submit";d.web_backend=backend;CHECK(ui_component_register(h,&d,&form)==UI_STATUS_OK);CHECK((runtime_mode?ui_component_mount(form,ui_shell_get_content_slot(ui_host_get_shell(h),"form")):ui_component_mount_offscreen(form,500,650,96))==UI_STATUS_OK);
    CHECK(click(form,"choice-mode"));CHECK(click(form,"option-mode-2"));value.size=sizeof(value);CHECK(ui_component_get_field(form,"mode",&value)==UI_STATUS_OK&&!strcmp(value.text,"high"));
    CHECK(click(form,"color-color"));CHECK(click(form,"swatch-color-1"));CHECK(ui_component_get_field(form,"color",&value)==UI_STATUS_OK&&!strcmp(value.text,"#00ff00ff")&&value.color_rgba==0x00ff00ff);
    {ui_element_presentation_t track={0};ui_input_event_t e={0};track.size=sizeof(track);CHECK(click(form,"color-color"));CHECK(presentation(form,"color-channel-color-a",&track)==UI_STATUS_OK&&track.visible);
 e.size=sizeof(e);e.pointer_button=1;e.kind=UI_INPUT_POINTER_DOWN;e.x=track.rect.x+track.rect.width/2;e.y=track.rect.y+track.rect.height/2;
 CHECK(dispatch(form,&e)==UI_STATUS_OK);e.kind=UI_INPUT_POINTER_MOVE;e.x=track.rect.x+track.rect.width/3;CHECK(dispatch(form,&e)==UI_STATUS_OK);e.kind=UI_INPUT_POINTER_UP;CHECK(dispatch(form,&e)==UI_STATUS_OK);
 CHECK(ui_component_get_field(form,"color",&value)==UI_STATUS_OK&&value.color_rgba!=0x00ff00ff&&submits==0);
 CHECK(click(form,"color-picker-cancel-color"));CHECK(ui_component_get_field(form,"color",&value)==UI_STATUS_OK&&value.color_rgba==0x00ff00ff&&submits==0);
 CHECK(click(form,"color-color"));CHECK(presentation(form,"color-channel-color-r",&track)==UI_STATUS_OK&&track.visible);e.kind=UI_INPUT_POINTER_DOWN;e.x=track.rect.x+track.rect.width/2;e.y=track.rect.y+track.rect.height/2;CHECK(dispatch(form,&e)==UI_STATUS_OK);e.kind=UI_INPUT_POINTER_UP;CHECK(dispatch(form,&e)==UI_STATUS_OK);
 CHECK(click(form,"color-picker-accept-color"));CHECK(ui_component_get_field(form,"color",&value)==UI_STATUS_OK&&value.color_rgba!=0x00ff00ff&&submits==0);}
 CHECK(click(form,"submit")&&submits==1);
    {ui_cell_t six={0};ui_element_presentation_t track={0};ui_input_event_t e={0};CHECK(ui_component_accept_fields(form)==UI_STATUS_OK);six.size=sizeof(six);six.kind=UI_VALUE_COLOR;six.text="#112233";six.color_rgba=0x112233ff;CHECK(ui_component_set_field(form,"color",&six)==UI_STATUS_OK);
     CHECK(click(form,"color-color"));track.size=sizeof(track);CHECK(presentation(form,"color-channel-color-a",&track)==UI_STATUS_OK&&track.visible);e.size=sizeof(e);e.kind=UI_INPUT_POINTER_DOWN;e.pointer_button=1;e.x=track.rect.x+track.rect.width/2;e.y=track.rect.y+track.rect.height/2;CHECK(dispatch(form,&e)==UI_STATUS_OK);
     CHECK(ui_component_get_field(form,"color",&value)==UI_STATUS_OK&&(value.color_rgba>>8)==0x112233&&submits==1);
     e.kind=UI_INPUT_CANCEL;CHECK(dispatch(form,&e)==UI_STATUS_OK);CHECK(ui_component_get_field(form,"color",&value)==UI_STATUS_OK&&!strcmp(value.text,"#112233"));CHECK(click(form,"color-picker-cancel-color")&&submits==1);}
    {ui_input_event_t e={0};e.size=sizeof(e);CHECK(click(form,"field-color"));{ui_element_presentation_t check={0};check.size=sizeof(check);CHECK(presentation(form,"field-color",&check)==UI_STATUS_OK&&check.focused);}e.kind=UI_INPUT_KEY_DOWN;e.key_code='A';e.modifiers=UI_INPUT_MODIFIER_CONTROL;CHECK(dispatch(form,&e)==UI_STATUS_OK);e.kind=UI_INPUT_TEXT;e.modifiers=0;e.text_utf8="#gggggggg";CHECK(dispatch(form,&e)==UI_STATUS_OK);CHECK(ui_component_get_field(form,"color",&value)==UI_STATUS_OK);CHECK(!strcmp(value.text,"#gggggggg"));CHECK(click(form,"submit")&&submits==1);CHECK(ui_component_get_field(form,"color",&value)==UI_STATUS_OK&&*value.error);
        ui_language_info_t previous={0};previous.size=sizeof(previous);CHECK(ui_host_get_language(h,&previous)==UI_STATUS_OK);CHECK(ui_host_set_language(h,"en-US")==UI_STATUS_OK);CHECK(ui_component_get_field(form,"color",&value)==UI_STATUS_OK&&!strcmp(value.text,"#gggggggg")&&!strcmp(value.error,"Color format is #RRGGBB or #RRGGBBAA")&&submits==1);ui_element_presentation_t error_text={0};error_text.size=sizeof(error_text);CHECK(presentation(form,"error-color",&error_text)==UI_STATUS_OK&&error_text.visible&&strstr(error_text.text_utf8,"Color format is #RRGGBB or #RRGGBBAA"));
        CHECK(ui_host_set_language(h,"zh-CN")==UI_STATUS_OK);CHECK(ui_component_get_field(form,"color",&value)==UI_STATUS_OK&&!strcmp(value.text,"#gggggggg")&&!strcmp(value.error,"颜色格式为 #RRGGBB 或 #RRGGBBAA")&&submits==1);CHECK(presentation(form,"error-color",&error_text)==UI_STATUS_OK&&strstr(error_text.text_utf8,"颜色格式为 #RRGGBB 或 #RRGGBBAA"));CHECK(ui_host_reset_language(h)==UI_STATUS_OK);
        CHECK(click(form,"color-color"));CHECK(click(form,"swatch-color-1"));CHECK(click(form,"choice-mode"));e.kind=UI_INPUT_KEY_DOWN;e.key_code=38;e.text_utf8=NULL;CHECK(dispatch(form,&e)==UI_STATUS_OK);e.key_code=13;CHECK(dispatch(form,&e)==UI_STATUS_OK);CHECK(ui_component_get_field(form,"mode",&value)==UI_STATUS_OK&&!strcmp(value.text,"medium"));}
    CHECK(click(table,"sort-name"));CHECK(sorts==2);
    {ui_input_event_t e={0};ui_element_presentation_t p={0};e.size=sizeof(e);e.kind=UI_INPUT_KEY_DOWN;p.size=sizeof(p);
        CHECK(click(table,"cell-9007199254741000-name"));
        e.key_code=40;CHECK(dispatch(table,&e)==UI_STATUS_OK);CHECK(ui_component_get_selection(table,ids,512,&n)==UI_STATUS_OK&&n==1&&ids[0]==9007199254741001ULL);
        e.key_code=13;CHECK(dispatch(table,&e)==UI_STATUS_OK);CHECK(presentation(table,"edit-9007199254741001-name",&p)==UI_STATUS_OK&&p.focused);
        e.key_code='A';e.modifiers=UI_INPUT_MODIFIER_CONTROL;CHECK(dispatch(table,&e)==UI_STATUS_OK);e.kind=UI_INPUT_TEXT;e.modifiers=0;e.text_utf8="changed";CHECK(dispatch(table,&e)==UI_STATUS_OK);e.kind=UI_INPUT_KEY_DOWN;e.key_code=13;CHECK(dispatch(table,&e)==UI_STATUS_OK&&edit_count==1);
        e.key_code=113;CHECK(dispatch(table,&e)==UI_STATUS_OK);e.key_code=27;CHECK(dispatch(table,&e)==UI_STATUS_OK&&edit_count==1);
        e.key_code=40;e.modifiers=UI_INPUT_MODIFIER_SHIFT;CHECK(dispatch(table,&e)==UI_STATUS_OK);CHECK(ui_component_get_selection(table,ids,512,&n)==UI_STATUS_OK&&n==2);e.modifiers=0;
        for(int at=0;at<30;++at)CHECK(dispatch(table,&e)==UI_STATUS_OK);CHECK(ui_component_get_state(table,&s)==UI_STATUS_OK&&s.first>0&&s.rendered_nodes<=1024);
        {char selected_cell[96];CHECK(ui_component_get_selection(table,ids,512,&n)==UI_STATUS_OK&&n==1);snprintf(selected_cell,sizeof(selected_cell),"cell-%llu-name",(unsigned long long)ids[0]);CHECK(click_mod(table,selected_cell,UI_INPUT_MODIFIER_CONTROL));CHECK(ui_component_get_selection(table,ids,512,&n)==UI_STATUS_OK&&n==0);}
    }

    {ui_component_t *readonly;ui_field_desc_t field={0};ui_component_desc_t view={0};ui_element_presentation_t button={0};field.size=sizeof(field);field.id="locked";field.title="Read only RGBA";field.kind=UI_VALUE_COLOR;field.flags=UI_VALUE_READONLY|UI_VALUE_DISABLED;view.size=sizeof(view);view.id="readonly-color";view.kind=UI_COMPONENT_FORM;view.fields=&field;view.field_count=1;view.web_backend=backend;CHECK(ui_component_register(h,&view,&readonly)==UI_STATUS_OK);CHECK((runtime_mode?ui_component_mount(readonly,ui_shell_get_content_slot(ui_host_get_shell(h),NULL)):ui_component_mount_offscreen(readonly,500,650,96))==UI_STATUS_OK);
     button.size=sizeof(button);CHECK(presentation(readonly,"color-locked",&button)==UI_STATUS_OK&&button.visible&&!button.enabled);}
    {ui_component_t *long_form;ui_field_desc_t field={0};ui_component_desc_t view={0};ui_element_presentation_t p={0};ui_input_event_t e={0};const char *long_options[128];char titles[128][128];
     for(size_t i=0;i<128;++i){snprintf(titles[i],128,"Option%zu — 长中文选项完整内容，选择器键盘访问",i);long_options[i]=titles[i];}
     field.size=sizeof(field);field.id="long-mode";field.title="这是需要完整查看的很长属性名称";field.help="选择由应用声明的128项枚举；键盘可以访问末项";field.unit="单位";field.kind=UI_VALUE_ENUM;field.options=long_options;field.option_count=128;
     view.size=sizeof(view);view.id="long-form";view.kind=UI_COMPONENT_FORM;view.fields=&field;view.field_count=1;view.web_backend=backend;CHECK(ui_component_register(h,&view,&long_form)==UI_STATUS_OK);CHECK((runtime_mode?ui_component_mount(long_form,ui_shell_get_content_slot(ui_host_get_shell(h),NULL)):ui_component_mount_offscreen(long_form,240,480,96))==UI_STATUS_OK);
     p.size=sizeof(p);CHECK(presentation(long_form,"help-long-mode",&p)==UI_STATUS_OK&&p.visible&&strstr(p.text_utf8,"128项"));CHECK(click(long_form,"label-long-mode"));e.size=sizeof(e);e.kind=UI_INPUT_KEY_DOWN;e.key_code=VK_F1;CHECK(dispatch(long_form,&e)==UI_STATUS_OK);pump_for(60);CHECK(presentation(long_form,"text-detail-value",&p)==UI_STATUS_OK&&p.visible&&p.clip.height>0&&strstr(p.text_utf8,"很长属性名称"));e.key_code=VK_ESCAPE;CHECK(dispatch(long_form,&e)==UI_STATUS_OK);CHECK(presentation(long_form,"label-long-mode",&p)==UI_STATUS_OK&&p.focused);CHECK(click(long_form,"choice-long-mode"));e.size=sizeof(e);e.kind=UI_INPUT_KEY_DOWN;e.key_code=VK_END;CHECK(dispatch(long_form,&e)==UI_STATUS_OK);CHECK(presentation(long_form,"option-long-mode-127",&p)==UI_STATUS_OK&&p.visible&&p.clip.height>0);e.key_code=VK_RETURN;CHECK(dispatch(long_form,&e)==UI_STATUS_OK);value.size=sizeof(value);CHECK(ui_component_get_field(long_form,"long-mode",&value)==UI_STATUS_OK&&!strcmp(value.text,long_options[127]));
     CHECK(click(long_form,"choice-long-mode"));e.key_code=VK_HOME;CHECK(dispatch(long_form,&e)==UI_STATUS_OK);e.key_code=VK_ESCAPE;CHECK(dispatch(long_form,&e)==UI_STATUS_OK);CHECK(ui_component_get_field(long_form,"long-mode",&value)==UI_STATUS_OK&&!strcmp(value.text,long_options[127]));}
    if(native)ui_native_shell_destroy(native);ui_host_destroy(h);
#ifdef UI_EXPERIENCE_WEBVIEW2
    if(backend){ui_webview2_backend_destroy(backend);pump_for(2000);CoUninitialize();}
#endif
    if(root)DestroyWindow(root);printf("component experience %s: %d failures\n",runtime_mode?"actual WebView2":"light",failures);return failures?1:0;
}
